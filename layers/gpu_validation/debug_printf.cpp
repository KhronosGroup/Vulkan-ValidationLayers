/* Copyright (c) 2020-2024 The Khronos Group Inc.
 * Copyright (c) 2020-2024 Valve Corporation
 * Copyright (c) 2020-2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gpu_validation/debug_printf.h"
#include "spirv-tools/instrument.hpp"
#include <iostream>
#include "generated/layer_chassis_dispatch.h"
#include "utils/shader_utils.h"

// Perform initializations that can be done at Create Device time.
void debug_printf::Validator::CreateDevice(const VkDeviceCreateInfo *pCreateInfo) {
    if (enabled[gpu_validation]) {
        ReportSetupProblem(device,
                           "Debug Printf cannot be enabled when gpu assisted validation is enabled.  "
                           "Debug Printf disabled.");
        aborted = true;
        return;
    }
    const char *size_string = getLayerOption("khronos_validation.printf_buffer_size");
    output_buffer_size = *size_string ? atoi(size_string) : 1024;

    std::string verbose_string = getLayerOption("khronos_validation.printf_verbose");
    vvl::ToLower(verbose_string);
    verbose = !verbose_string.compare("true");

    std::string stdout_string = getLayerOption("khronos_validation.printf_to_stdout");
    vvl::ToLower(stdout_string);
    use_stdout = !stdout_string.compare("true");
    if (!GetEnvironment("DEBUG_PRINTF_TO_STDOUT").empty()) use_stdout = true;

    // GpuAssistedBase::CreateDevice will set up bindings
    VkDescriptorSetLayoutBinding binding = {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
                                            VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_MESH_BIT_EXT |
                                                VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_COMPUTE_BIT |
                                                gpu_tracker::kShaderStageAllRayTracing,
                                            NULL};
    bindings_.push_back(binding);

    BaseClass::CreateDevice(pCreateInfo);

    if (phys_dev_props.apiVersion < VK_API_VERSION_1_1) {
        ReportSetupProblem(device, "Debug Printf requires Vulkan 1.1 or later.  Debug Printf disabled.");
        aborted = true;
        return;
    }

    DispatchGetPhysicalDeviceFeatures(physical_device, &supported_features);
    if (!supported_features.fragmentStoresAndAtomics || !supported_features.vertexPipelineStoresAndAtomics) {
        ReportSetupProblem(device,
                           "Debug Printf requires fragmentStoresAndAtomics and vertexPipelineStoresAndAtomics.  "
                           "Debug Printf disabled.");
        aborted = true;
        return;
    }
}

// Free the device memory and descriptor set associated with a command buffer.
void debug_printf::Validator::DestroyBuffer(BufferInfo &buffer_info) {
    vmaDestroyBuffer(vmaAllocator, buffer_info.output_mem_block.buffer, buffer_info.output_mem_block.allocation);
    if (buffer_info.desc_set != VK_NULL_HANDLE) {
        desc_set_manager->PutBackDescriptorSet(buffer_info.desc_pool, buffer_info.desc_set);
    }
}

// Call the SPIR-V Optimizer to run the instrumentation pass on the shader.
bool debug_printf::Validator::InstrumentShader(const vvl::span<const uint32_t> &input, std::vector<uint32_t> &new_pgm,
                                               uint32_t unique_shader_id, const Location &loc) {
    if (aborted) return false;
    if (input[0] != spv::MagicNumber) return false;

    // Load original shader SPIR-V
    new_pgm.clear();
    new_pgm.reserve(input.size());
    new_pgm.insert(new_pgm.end(), &input.front(), &input.back() + 1);

    // Call the optimizer to instrument the shader.
    // Use the unique_shader_module_id as a shader ID so we can look up its handle later in the shader_map.
    // If descriptor indexing is enabled, enable length checks and updated descriptor checks
    using namespace spvtools;
    spv_target_env target_env = PickSpirvEnv(api_version, IsExtEnabled(device_extensions.vk_khr_spirv_1_4));
    spvtools::ValidatorOptions val_options;
    AdjustValidatorOptions(device_extensions, enabled_features, val_options);
    spvtools::OptimizerOptions opt_options;
    opt_options.set_run_validator(true);
    opt_options.set_validator_options(val_options);
    Optimizer optimizer(target_env);
    const spvtools::MessageConsumer debug_printf_console_message_consumer =
        [this, loc](spv_message_level_t level, const char *, const spv_position_t &position, const char *message) -> void {
        switch (level) {
            case SPV_MSG_FATAL:
            case SPV_MSG_INTERNAL_ERROR:
            case SPV_MSG_ERROR:
                this->LogError("UNASSIGNED-Debug-Printf", this->device, loc,
                               "Error during shader instrumentation in spirv-opt: line %zu: %s", position.index, message);
                break;
            default:
                break;
        }
    };
    optimizer.SetMessageConsumer(debug_printf_console_message_consumer);
    optimizer.RegisterPass(CreateInstDebugPrintfPass(desc_set_bind_index, unique_shader_id));
    const bool pass = optimizer.Run(new_pgm.data(), new_pgm.size(), &new_pgm, opt_options);
    if (!pass) {
        ReportSetupProblem(device, "Failure to instrument shader in spirv-opt. Proceeding with non-instrumented shader.");
    }
    return pass;
}
// Create the instrumented shader data to provide to the driver.
void debug_printf::Validator::PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkShaderModule *pShaderModule, const RecordObject &record_obj,
                                                              void *csm_state_data) {
    ValidationStateTracker::PreCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, record_obj,
                                                            csm_state_data);
    create_shader_module_api_state *csm_state = static_cast<create_shader_module_api_state *>(csm_state_data);
    csm_state->unique_shader_id = unique_shader_module_id++;
    const bool pass = InstrumentShader(vvl::make_span(pCreateInfo->pCode, pCreateInfo->codeSize / sizeof(uint32_t)),
                                       csm_state->instrumented_spirv, csm_state->unique_shader_id, record_obj.location);
    if (pass) {
        csm_state->instrumented_create_info.pCode = csm_state->instrumented_spirv.data();
        csm_state->instrumented_create_info.codeSize = csm_state->instrumented_spirv.size() * sizeof(uint32_t);
    }
}

void debug_printf::Validator::PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                            const VkShaderCreateInfoEXT *pCreateInfos,
                                                            const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                                            const RecordObject &record_obj, void *csm_state_data) {
    ValidationStateTracker::PreCallRecordCreateShadersEXT(device, createInfoCount, pCreateInfos, pAllocator, pShaders, record_obj,
                                                          csm_state_data);
    BaseClass::PreCallRecordCreateShadersEXT(device, createInfoCount, pCreateInfos, pAllocator, pShaders, record_obj,
                                             csm_state_data);
    create_shader_object_api_state *csm_state = static_cast<create_shader_object_api_state *>(csm_state_data);
    for (uint32_t i = 0; i < createInfoCount; ++i) {
        csm_state->unique_shader_ids[i] = unique_shader_module_id++;
        const bool pass = InstrumentShader(
            vvl::make_span(static_cast<const uint32_t *>(pCreateInfos[i].pCode), pCreateInfos[i].codeSize / sizeof(uint32_t)),
            csm_state->instrumented_spirv[i], csm_state->unique_shader_ids[i], record_obj.location);
        if (pass) {
            csm_state->instrumented_create_info[i].pCode = csm_state->instrumented_spirv[i].data();
            csm_state->instrumented_create_info[i].codeSize = csm_state->instrumented_spirv[i].size() * sizeof(uint32_t);
        }
    }
}

static debug_printf::vartype vartype_lookup(char intype) {
    switch (intype) {
        case 'd':
        case 'i':
            return debug_printf::varsigned;
            break;

        case 'f':
        case 'F':
        case 'a':
        case 'A':
        case 'e':
        case 'E':
        case 'g':
        case 'G':
            return debug_printf::varfloat;
            break;

        case 'u':
        case 'x':
        case 'o':
        default:
            return debug_printf::varunsigned;
            break;
    }
}

std::vector<debug_printf::Substring> debug_printf::Validator::ParseFormatString(const std::string &format_string) {
    const char types[] = {'d', 'i', 'o', 'u', 'x', 'X', 'a', 'A', 'e', 'E', 'f', 'F', 'g', 'G', 'v', '\0'};
    std::vector<Substring> parsed_strings;
    size_t pos = 0;
    size_t begin = 0;
    size_t percent = 0;

    while (begin < format_string.length()) {
        Substring substring;

        // Find a percent sign
        pos = percent = format_string.find_first_of('%', pos);
        if (pos == std::string::npos) {
            // End of the format string   Push the rest of the characters
            substring.string = format_string.substr(begin, format_string.length());
            substring.needs_value = false;
            parsed_strings.push_back(substring);
            break;
        }
        pos++;
        if (format_string[pos] == '%') {
            pos++;
            continue;  // %% - skip it
        }
        // Find the type of the value
        pos = format_string.find_first_of(types, pos);
        if (pos == format_string.npos) {
            // This really shouldn't happen with a legal value string
            pos = format_string.length();
        } else {
            char tempstring[32];
            int count = 0;
            std::string specifier = {};

            if (format_string[pos] == 'v') {
                // Vector must be of size 2, 3, or 4
                // and format %v<size><type>
                specifier = format_string.substr(percent, pos - percent);
                count = atoi(&format_string[pos + 1]);
                pos += 2;

                // skip v<count>, handle long
                specifier.push_back(format_string[pos]);
                if (format_string[pos + 1] == 'l') {
                    specifier.push_back('l');
                    pos++;
                }

                // Take the preceding characters, and the percent through the type
                substring.string = format_string.substr(begin, percent - begin);
                substring.string += specifier;
                substring.needs_value = true;
                substring.type = vartype_lookup(specifier.back());
                parsed_strings.push_back(substring);

                // Continue with a comma separated list
                snprintf(tempstring, sizeof(tempstring), ", %s", specifier.c_str());
                substring.string = tempstring;
                for (int i = 0; i < (count - 1); i++) {
                    parsed_strings.push_back(substring);
                }
            } else {
                // Single non-vector value
                if (format_string[pos + 1] == 'l') pos++;  // Save long size
                substring.string = format_string.substr(begin, pos - begin + 1);
                substring.needs_value = true;
                substring.type = vartype_lookup(format_string[pos]);
                parsed_strings.push_back(substring);
            }
            begin = pos + 1;
        }
    }
    return parsed_strings;
}

std::string debug_printf::Validator::FindFormatString(vvl::span<const uint32_t> pgm, uint32_t string_id) {
    std::string format_string;
    spirv::Module module_state(pgm);
    if (module_state.words_.empty()) {
        return {};
    }

    for (const spirv::Instruction *insn : module_state.static_data_.debug_string_inst) {
        if (insn->Word(1) == string_id) {
            format_string = insn->GetAsString(2);
            break;
        }
    }

    return format_string;
}

// GCC and clang don't like using variables as format strings in sprintf.
// #pragma GCC is recognized by both compilers
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
#endif

void debug_printf::Validator::AnalyzeAndGenerateMessages(VkCommandBuffer command_buffer, VkQueue queue, BufferInfo &buffer_info,
                                                         uint32_t operation_index, uint32_t *const debug_output_buffer) {
    // Word         Content
    //    0         Must be zero
    //    1         Size of output record, including this word
    //    2         Shader ID
    //    3         Instruction Position
    //    4         Printf Format String Id
    //    5         Printf Values Word 0 (optional)
    //    6         Printf Values Word 1 (optional)
    uint32_t expect = debug_output_buffer[1];
    if (!expect) return;

    // TODO - have Loc passed in correctly
    Location loc(vvl::Func::vkQueueSubmit);

    uint32_t index = spvtools::kDebugOutputDataOffset;
    while (debug_output_buffer[index]) {
        std::stringstream shader_message;
        VkShaderModule shader_module_handle = VK_NULL_HANDLE;
        VkPipeline pipeline_handle = VK_NULL_HANDLE;
        VkShaderEXT shader_object_handle = VK_NULL_HANDLE;
        vvl::span<const uint32_t> pgm;

        OutputRecord *debug_record = reinterpret_cast<OutputRecord *>(&debug_output_buffer[index]);
        // Lookup the VkShaderModule handle and SPIR-V code used to create the shader, using the unique shader ID value returned
        // by the instrumented shader.
        auto it = shader_map.find(debug_record->shader_id);
        if (it != shader_map.end()) {
            shader_module_handle = it->second.shader_module;
            pipeline_handle = it->second.pipeline;
            shader_object_handle = it->second.shader_object;
            pgm = it->second.pgm;
        }
        assert(pgm.size() != 0);
        // Search through the shader source for the printf format string for this invocation
        const auto format_string = FindFormatString(pgm, debug_record->format_string_id);
        // Break the format string into strings with 1 or 0 value
        auto format_substrings = ParseFormatString(format_string);
        void *values = static_cast<void *>(&debug_record->values);
        // Sprintf each format substring into a temporary string then add that to the message
        for (auto &substring : format_substrings) {
            std::string temp_string;
            size_t needed = 0;
            std::vector<std::string> format_strings = {"%ul", "%lu", "%lx"};
            size_t ul_pos = 0;
            bool print_hex = true;
            for (const auto &ul_string : format_strings) {
                ul_pos = substring.string.find(ul_string);
                if (ul_pos != std::string::npos) {
                    if (ul_string == "%lu") print_hex = false;
                    break;
                }
            }
            if (ul_pos != std::string::npos) {
                // Unsigned 64 bit value
                substring.longval = *static_cast<uint64_t *>(values);
                values = static_cast<uint64_t *>(values) + 1;
                if (print_hex) {
                    substring.string.replace(ul_pos + 1, 2, PRIx64);
                } else {
                    substring.string.replace(ul_pos + 1, 2, PRIu64);
                }
                // +1 for null terminator
                needed = std::snprintf(nullptr, 0, substring.string.c_str(), substring.longval) + 1;
                temp_string.resize(needed);
                std::snprintf(&temp_string[0], needed, substring.string.c_str(), substring.longval);
            } else {
                if (substring.needs_value) {
                    switch (substring.type) {
                        case varunsigned:
                            // +1 for null terminator
                            needed = std::snprintf(nullptr, 0, substring.string.c_str(), *static_cast<uint32_t *>(values)) + 1;
                            temp_string.resize(needed);
                            std::snprintf(&temp_string[0], needed, substring.string.c_str(), *static_cast<uint32_t *>(values));
                            break;

                        case varsigned:
                            needed = std::snprintf(nullptr, 0, substring.string.c_str(), *static_cast<int32_t *>(values)) + 1;
                            temp_string.resize(needed);
                            std::snprintf(&temp_string[0], needed, substring.string.c_str(), *static_cast<int32_t *>(values));
                            break;

                        case varfloat:
                            needed = std::snprintf(nullptr, 0, substring.string.c_str(), *static_cast<float *>(values)) + 1;
                            temp_string.resize(needed);
                            std::snprintf(&temp_string[0], needed, substring.string.c_str(), *static_cast<float *>(values));
                            break;
                    }
                    values = static_cast<uint32_t *>(values) + 1;
                } else {
                    needed = std::snprintf(nullptr, 0, substring.string.c_str()) + 1;
                    temp_string.resize(needed);
                    std::snprintf(&temp_string[0], needed, substring.string.c_str());
                }
            }
            shader_message << temp_string.c_str();
        }

        if (verbose) {
            std::string common_message;
            std::string filename_message;
            std::string source_message;
            UtilGenerateCommonMessage(report_data, command_buffer, &debug_output_buffer[index], shader_module_handle,
                                      pipeline_handle, shader_object_handle, buffer_info.pipeline_bind_point, operation_index, common_message);
            UtilGenerateSourceMessages(pgm, &debug_output_buffer[index], true, filename_message, source_message);
            if (use_stdout) {
                std::cout << "WARNING-DEBUG-PRINTF " << common_message.c_str() << " "
                          << shader_message.str().c_str() << " " << filename_message.c_str() << " " << source_message.c_str();
            } else {
                LogInfo("WARNING-DEBUG-PRINTF", queue, loc, "%s %s %s%s", common_message.c_str(),
                        shader_message.str().c_str(), filename_message.c_str(), source_message.c_str());
            }
        } else {
            if (use_stdout) {
                std::cout << shader_message.str();
            } else {
                // Don't let LogInfo process any '%'s in the string
                LogInfo("WARNING-DEBUG-PRINTF", device, loc, "%s", shader_message.str().c_str());
            }
        }
        index += debug_record->size;
    }
    if ((index - spvtools::kDebugOutputDataOffset) != expect) {
        LogWarning("WARNING-DEBUG-PRINTF", device, loc,
                   "WARNING - Debug Printf message was truncated, likely due to a buffer size that was too small for the message");
    }
    memset(debug_output_buffer, 0, 4 * (debug_output_buffer[spvtools::kDebugOutputSizeOffset] + spvtools::kDebugOutputDataOffset));
}

// For the given command buffer, map its debug data buffers and read their contents for analysis.
void debug_printf::CommandBuffer::Process(VkQueue queue, const Location &loc) {
    auto *device_state = static_cast<debug_printf::Validator *>(dev_data);
    if (has_draw_cmd || has_trace_rays_cmd || has_dispatch_cmd) {
        auto &gpu_buffer_list = buffer_infos;
        uint32_t draw_index = 0;
        uint32_t compute_index = 0;
        uint32_t ray_trace_index = 0;

        for (auto &buffer_info : gpu_buffer_list) {
            char *data;

            uint32_t operation_index = 0;
            if (buffer_info.pipeline_bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
                operation_index = draw_index;
                draw_index++;
            } else if (buffer_info.pipeline_bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
                operation_index = compute_index;
                compute_index++;
            } else if (buffer_info.pipeline_bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
                operation_index = ray_trace_index;
                ray_trace_index++;
            } else {
                assert(false);
            }

            VkResult result = vmaMapMemory(device_state->vmaAllocator, buffer_info.output_mem_block.allocation, (void **)&data);
            if (result == VK_SUCCESS) {
                device_state->AnalyzeAndGenerateMessages(commandBuffer(), queue, buffer_info, operation_index, (uint32_t *)data);
                vmaUnmapMemory(device_state->vmaAllocator, buffer_info.output_mem_block.allocation);
            }
        }
    }
}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

void debug_printf::Validator::PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                                   uint32_t firstVertex, uint32_t firstInstance, const RecordObject &record_obj) {
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void debug_printf::Validator::PreCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                           const VkMultiDrawInfoEXT *pVertexInfo, uint32_t instanceCount,
                                                           uint32_t firstInstance, uint32_t stride,
                                                           const RecordObject &record_obj) {
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void debug_printf::Validator::PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount,
                                                          uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset,
                                                          uint32_t firstInstance, const RecordObject &record_obj) {
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void debug_printf::Validator::PreCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                                  const VkMultiDrawIndexedInfoEXT *pIndexInfo,
                                                                  uint32_t instanceCount, uint32_t firstInstance, uint32_t stride,
                                                                  const int32_t *pVertexOffset, const RecordObject &record_obj) {
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void debug_printf::Validator::PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           uint32_t count, uint32_t stride, const RecordObject &record_obj) {
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void debug_printf::Validator::PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, uint32_t count, uint32_t stride,
                                                                  const RecordObject &record_obj) {
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void debug_printf::Validator::PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z,
                                                       const RecordObject &record_obj) {
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE);
}

void debug_printf::Validator::PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               const RecordObject &record_obj) {
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE);
}

void debug_printf::Validator::PreCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                           uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                           uint32_t groupCountZ, const RecordObject &record_obj) {
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE);
}

void debug_printf::Validator::PreCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX,
                                                              uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX,
                                                              uint32_t groupCountY, uint32_t groupCountZ,
                                                              const RecordObject &record_obj) {
    PreCallRecordCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ,
                                 record_obj);
}

void debug_printf::Validator::PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                   VkDeviceSize offset, VkBuffer countBuffer,
                                                                   VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                   uint32_t stride, const RecordObject &record_obj) {
    PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                      record_obj);
}

void debug_printf::Validator::PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                                uint32_t maxDrawCount, uint32_t stride,
                                                                const RecordObject &record_obj) {
    ValidationStateTracker::PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                              maxDrawCount, stride, record_obj);
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void debug_printf::Validator::PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                          VkDeviceSize offset, VkBuffer countBuffer,
                                                                          VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                          uint32_t stride, const RecordObject &record_obj) {
    PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                             record_obj);
}

void debug_printf::Validator::PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                       VkDeviceSize offset, VkBuffer countBuffer,
                                                                       VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                       uint32_t stride, const RecordObject &record_obj) {
    ValidationStateTracker::PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                                     maxDrawCount, stride, record_obj);
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void debug_printf::Validator::PreCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                                       uint32_t firstInstance, VkBuffer counterBuffer,
                                                                       VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                                       uint32_t vertexStride, const RecordObject &record_obj) {
    ValidationStateTracker::PreCallRecordCmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer,
                                                                     counterBufferOffset, counterOffset, vertexStride, record_obj);
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void debug_printf::Validator::PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                                              const RecordObject &record_obj) {
    ValidationStateTracker::PreCallRecordCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask, record_obj);
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void debug_printf::Validator::PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                      VkDeviceSize offset, uint32_t drawCount, uint32_t stride,
                                                                      const RecordObject &record_obj) {
    ValidationStateTracker::PreCallRecordCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride, record_obj);
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void debug_printf::Validator::PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                           VkDeviceSize offset, VkBuffer countBuffer,
                                                                           VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                           uint32_t stride, const RecordObject &record_obj) {
    ValidationStateTracker::PreCallRecordCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer,
                                                                         countBufferOffset, maxDrawCount, stride, record_obj);
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void debug_printf::Validator::PreCallRecordCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX,
                                                               uint32_t groupCountY, uint32_t groupCountZ,
                                                               const RecordObject &record_obj) {
    ValidationStateTracker::PreCallRecordCmdDrawMeshTasksEXT(commandBuffer, groupCountX, groupCountY, groupCountZ, record_obj);
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void debug_printf::Validator::PreCallRecordCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                       VkDeviceSize offset, uint32_t drawCount, uint32_t stride,
                                                                       const RecordObject &record_obj) {
    ValidationStateTracker::PreCallRecordCmdDrawMeshTasksIndirectEXT(commandBuffer, buffer, offset, drawCount, stride, record_obj);
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void debug_printf::Validator::PreCallRecordCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                            VkDeviceSize offset, VkBuffer countBuffer,
                                                                            VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                            uint32_t stride, const RecordObject &record_obj) {
    ValidationStateTracker::PreCallRecordCmdDrawMeshTasksIndirectCountEXT(commandBuffer, buffer, offset, countBuffer,
                                                                          countBufferOffset, maxDrawCount, stride, record_obj);
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void debug_printf::Validator::PreCallRecordCmdTraceRaysNV(
    VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer, VkDeviceSize raygenShaderBindingOffset,
    VkBuffer missShaderBindingTableBuffer, VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
    VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset, VkDeviceSize hitShaderBindingStride,
    VkBuffer callableShaderBindingTableBuffer, VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
    uint32_t width, uint32_t height, uint32_t depth, const RecordObject &record_obj) {
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV);
}

void debug_printf::Validator::PreCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                                           const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                           const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                           const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                           const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                                           uint32_t width, uint32_t height, uint32_t depth,
                                                           const RecordObject &record_obj) {
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
}

void debug_printf::Validator::PreCallRecordCmdTraceRaysIndirectKHR(
    VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, VkDeviceAddress indirectDeviceAddress,
    const RecordObject &record_obj) {
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
}

void debug_printf::Validator::PreCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer,
                                                                    VkDeviceAddress indirectDeviceAddress,
                                                                    const RecordObject &record_obj) {
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
}

void debug_printf::Validator::AllocateDebugPrintfResources(const VkCommandBuffer cmd_buffer, const VkPipelineBindPoint bind_point) {
    if (bind_point != VK_PIPELINE_BIND_POINT_GRAPHICS && bind_point != VK_PIPELINE_BIND_POINT_COMPUTE &&
        bind_point != VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
        return;
    }
    VkResult result;

    if (aborted) return;

    std::vector<VkDescriptorSet> desc_sets;
    VkDescriptorPool desc_pool = VK_NULL_HANDLE;
    result = desc_set_manager->GetDescriptorSets(1, &desc_pool, debug_desc_layout, &desc_sets);
    assert(result == VK_SUCCESS);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, "Unable to allocate descriptor sets.  Device could become unstable.");
        aborted = true;
        return;
    }

    VkDescriptorBufferInfo output_desc_buffer_info = {};
    output_desc_buffer_info.range = output_buffer_size;

    auto cb_node = GetWrite<debug_printf::CommandBuffer>(cmd_buffer);
    if (!cb_node) {
        ReportSetupProblem(device, "Unrecognized command buffer");
        aborted = true;
        return;
    }

    const auto lv_bind_point = ConvertToLvlBindPoint(bind_point);
    const auto &last_bound = cb_node->lastBound[lv_bind_point];
    const auto *pipeline_state = last_bound.pipeline_state;

    if (!pipeline_state && !last_bound.HasShaderObjects()) {
        ReportSetupProblem(device, "Neither pipeline state nor shader object states were found, aborting Debug Printf");
        aborted = true;
        return;
    }

    // Allocate memory for the output block that the gpu will use to return values for printf
    DeviceMemoryBlock output_block = {};
    VkBufferCreateInfo buffer_info = vku::InitStructHelper();
    buffer_info.size = output_buffer_size;
    buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    result = vmaCreateBuffer(vmaAllocator, &buffer_info, &alloc_info, &output_block.buffer, &output_block.allocation, nullptr);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, "Unable to allocate device memory.  Device could become unstable.");
        aborted = true;
        return;
    }

    // Clear the output block to zeros so that only printf values from the gpu will be present
    uint32_t *data;
    result = vmaMapMemory(vmaAllocator, output_block.allocation, reinterpret_cast<void **>(&data));
    if (result == VK_SUCCESS) {
        memset(data, 0, output_buffer_size);
        vmaUnmapMemory(vmaAllocator, output_block.allocation);
    }

    VkWriteDescriptorSet desc_writes = vku::InitStructHelper();
    const uint32_t desc_count = 1;

    // Write the descriptor
    output_desc_buffer_info.buffer = output_block.buffer;
    output_desc_buffer_info.offset = 0;

    desc_writes.descriptorCount = 1;
    desc_writes.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    desc_writes.pBufferInfo = &output_desc_buffer_info;
    desc_writes.dstSet = desc_sets[0];
    desc_writes.dstBinding = 3;
    DispatchUpdateDescriptorSets(device, desc_count, &desc_writes, 0, NULL);

    const auto pipeline_layout =
        pipeline_state ? pipeline_state->PipelineLayoutState() : Get<vvl::PipelineLayout>(last_bound.pipeline_layout);
    if (pipeline_layout) {
        // If GPL is used, it's possible the pipeline layout used at pipeline creation time is null. If CmdBindDescriptorSets has
        // not been called yet (i.e., state.pipeline_null), then fall back to the layout associated with pre-raster state.
        // PipelineLayoutState should be used for the purposes of determining the number of sets in the layout, but this layout
        // may be a "pseudo layout" used to represent the union of pre-raster and fragment shader layouts, and therefore have a
        // null handle.
        const auto pipeline_layout_handle =
            (last_bound.pipeline_layout) ? last_bound.pipeline_layout : pipeline_state->PreRasterPipelineLayoutState()->layout();
        if (pipeline_layout->set_layouts.size() <= desc_set_bind_index) {
            DispatchCmdBindDescriptorSets(cmd_buffer, bind_point, pipeline_layout_handle, desc_set_bind_index, 1, desc_sets.data(),
                                          0, nullptr);
        }
    } else {
        // If no pipeline layout was bound when using shader objects that don't use any descriptor set, bind the debug pipeline
        // layout
        DispatchCmdBindDescriptorSets(cmd_buffer, bind_point, debug_pipeline_layout, desc_set_bind_index, 1, desc_sets.data(), 0,
                                      nullptr);
    }
    // Record buffer and memory info in CB state tracking
    cb_node->buffer_infos.emplace_back(output_block, desc_sets[0], desc_pool, bind_point);
}

std::shared_ptr<vvl::CommandBuffer> debug_printf::Validator::CreateCmdBufferState(VkCommandBuffer cb,
                                                                                  const VkCommandBufferAllocateInfo *pCreateInfo,
                                                                                  const vvl::CommandPool *pool) {
    return std::static_pointer_cast<vvl::CommandBuffer>(std::make_shared<debug_printf::CommandBuffer>(this, cb, pCreateInfo, pool));
}

debug_printf::CommandBuffer::CommandBuffer(debug_printf::Validator *dp, VkCommandBuffer cb,
                                           const VkCommandBufferAllocateInfo *pCreateInfo, const vvl::CommandPool *pool)
    : gpu_tracker::CommandBuffer(dp, cb, pCreateInfo, pool) {}

debug_printf::CommandBuffer::~CommandBuffer() { Destroy(); }

void debug_printf::CommandBuffer::Destroy() {
    ResetCBState();
    vvl::CommandBuffer::Destroy();
}

void debug_printf::CommandBuffer::Reset() {
    vvl::CommandBuffer::Reset();
    ResetCBState();
}

void debug_printf::CommandBuffer::ResetCBState() {
    auto debug_printf = static_cast<debug_printf::Validator *>(dev_data);
    // Free the device memory and descriptor set(s) associated with a command buffer.
    if (debug_printf->aborted) {
        return;
    }
    for (auto &buffer_info : buffer_infos) {
        debug_printf->DestroyBuffer(buffer_info);
    }
    buffer_infos.clear();
}
