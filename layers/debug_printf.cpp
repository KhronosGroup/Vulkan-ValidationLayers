/* Copyright (c) 2020-2022 The Khronos Group Inc.
 * Copyright (c) 2020-2022 Valve Corporation
 * Copyright (c) 2020-2022 LunarG, Inc.
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
 *
 * Author: Tony Barbour <tony@lunarg.com>
 */

#include "debug_printf.h"
#include "spirv-tools/optimizer.hpp"
#include "spirv-tools/instrument.hpp"
#include <iostream>
#include "layer_chassis_dispatch.h"
#include "cmd_buffer_state.h"

// Perform initializations that can be done at Create Device time.
void DebugPrintf::CreateDevice(const VkDeviceCreateInfo *pCreateInfo) {
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
    transform(verbose_string.begin(), verbose_string.end(), verbose_string.begin(), ::tolower);
    verbose = verbose_string.length() ? !verbose_string.compare("true") : false;

    std::string stdout_string = getLayerOption("khronos_validation.printf_to_stdout");
    transform(stdout_string.begin(), stdout_string.end(), stdout_string.begin(), ::tolower);
    use_stdout = stdout_string.length() ? !stdout_string.compare("true") : false;
    if (getenv("DEBUG_PRINTF_TO_STDOUT")) use_stdout = true;

    // GpuAssistedBase::CreateDevice will set up bindings
    VkDescriptorSetLayoutBinding binding = {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
                                            VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_MESH_BIT_NV |
                                                VK_SHADER_STAGE_TASK_BIT_NV | VK_SHADER_STAGE_COMPUTE_BIT |
                                                kShaderStageAllRayTracing,
                                            NULL};
    bindings_.push_back(binding);

    GpuAssistedBase::CreateDevice(pCreateInfo);

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
void DebugPrintf::DestroyBuffer(DPFBufferInfo &buffer_info) {
    vmaDestroyBuffer(vmaAllocator, buffer_info.output_mem_block.buffer, buffer_info.output_mem_block.allocation);
    if (buffer_info.desc_set != VK_NULL_HANDLE) {
        desc_set_manager->PutBackDescriptorSet(buffer_info.desc_pool, buffer_info.desc_set);
    }
}

// Call the SPIR-V Optimizer to run the instrumentation pass on the shader.
bool DebugPrintf::InstrumentShader(const VkShaderModuleCreateInfo *pCreateInfo, std::vector<uint32_t> &new_pgm,
                                   uint32_t *unique_shader_id) {
    if (aborted) return false;
    if (pCreateInfo->pCode[0] != spv::MagicNumber) return false;

    // Load original shader SPIR-V
    uint32_t num_words = static_cast<uint32_t>(pCreateInfo->codeSize / 4);
    new_pgm.clear();
    new_pgm.reserve(num_words);
    new_pgm.insert(new_pgm.end(), &pCreateInfo->pCode[0], &pCreateInfo->pCode[num_words]);

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
        [this](spv_message_level_t level, const char *, const spv_position_t &position, const char *message) -> void {
        switch (level) {
            case SPV_MSG_FATAL:
            case SPV_MSG_INTERNAL_ERROR:
            case SPV_MSG_ERROR:
                this->LogError(this->device, "UNASSIGNED-Debug-Printf", "Error during shader instrumentation: line %zu: %s",
                               position.index, message);
                break;
            default:
                break;
        }
    };
    optimizer.SetMessageConsumer(debug_printf_console_message_consumer);
    optimizer.RegisterPass(CreateInstDebugPrintfPass(desc_set_bind_index, unique_shader_module_id));
    bool pass = optimizer.Run(new_pgm.data(), new_pgm.size(), &new_pgm, opt_options);
    if (!pass) {
        ReportSetupProblem(device, "Failure to instrument shader.  Proceeding with non-instrumented shader.");
    }
    *unique_shader_id = unique_shader_module_id++;
    return pass;
}
// Create the instrumented shader data to provide to the driver.
void DebugPrintf::PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule,
                                                  void *csm_state_data) {
    create_shader_module_api_state *csm_state = reinterpret_cast<create_shader_module_api_state *>(csm_state_data);
    bool pass = InstrumentShader(pCreateInfo, csm_state->instrumented_pgm, &csm_state->unique_shader_id);
    if (pass) {
        csm_state->instrumented_create_info.pCode = csm_state->instrumented_pgm.data();
        csm_state->instrumented_create_info.codeSize = csm_state->instrumented_pgm.size() * sizeof(uint32_t);
    }
}

vartype vartype_lookup(char intype) {
    switch (intype) {
        case 'd':
        case 'i':
            return varsigned;
            break;

        case 'f':
        case 'F':
        case 'a':
        case 'A':
        case 'e':
        case 'E':
        case 'g':
        case 'G':
            return varfloat;
            break;

        case 'u':
        case 'x':
        case 'o':
        default:
            return varunsigned;
            break;
    }
}

std::vector<DPFSubstring> DebugPrintf::ParseFormatString(const std::string format_string) {
    const char types[] = {'d', 'i', 'o', 'u', 'x', 'X', 'a', 'A', 'e', 'E', 'f', 'F', 'g', 'G', 'v', '\0'};
    std::vector<DPFSubstring> parsed_strings;
    size_t pos = 0;
    size_t begin = 0;
    size_t percent = 0;

    while (begin < format_string.length()) {
        DPFSubstring substring;

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

std::string DebugPrintf::FindFormatString(std::vector<uint32_t> pgm, uint32_t string_id) {
    std::string format_string;
    SHADER_MODULE_STATE module_state(pgm);
    if (module_state.words.size() > 0) {
        for (const auto &insn : module_state) {
            if (insn.opcode() == spv::OpString) {
                uint32_t offset = insn.offset();
                if (pgm[offset + 1] == string_id) {
                    format_string = reinterpret_cast<char *>(&pgm[offset + 2]);
                    break;
                }
            }
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

void snprintf_with_malloc(std::stringstream &shader_message, DPFSubstring substring, size_t needed, void *values) {
    char *buffer = static_cast<char *>(malloc((needed + 1) * sizeof(char)));  // Add 1 for terminator
    if (substring.longval) {
        snprintf(buffer, needed, substring.string.c_str(), substring.longval);
    } else if (!substring.needs_value) {
        snprintf(buffer, needed, substring.string.c_str());
    } else {
        switch (substring.type) {
            case varunsigned:
                needed = snprintf(buffer, needed, substring.string.c_str(), *static_cast<uint32_t *>(values) - 1);
                break;

            case varsigned:
                needed = snprintf(buffer, needed, substring.string.c_str(), *static_cast<int32_t *>(values) - 1);
                break;

            case varfloat:
                needed = snprintf(buffer, needed, substring.string.c_str(), *static_cast<float *>(values) - 1);
                break;
        }
    }
    shader_message << buffer;
    free(buffer);
}

void DebugPrintf::AnalyzeAndGenerateMessages(VkCommandBuffer command_buffer, VkQueue queue, DPFBufferInfo &buffer_info,
                                             uint32_t operation_index, uint32_t *const debug_output_buffer) {
    // Word         Content
    //    0         Size of output record, including this word
    //    1         Shader ID
    //    2         Instruction Position
    //    3         Stage Ordinal
    //    4         Stage - specific Info Word 0
    //    5         Stage - specific Info Word 1
    //    6         Stage - specific Info Word 2
    //    7         Printf Format String Id
    //    8         Printf Values Word 0 (optional)
    //    9         Printf Values Word 1 (optional)
    uint32_t expect = debug_output_buffer[0];
    if (!expect) return;

    uint32_t index = 1;
    while (debug_output_buffer[index]) {
        std::stringstream shader_message;
        VkShaderModule shader_module_handle = VK_NULL_HANDLE;
        VkPipeline pipeline_handle = VK_NULL_HANDLE;
        std::vector<uint32_t> pgm;

        DPFOutputRecord *debug_record = reinterpret_cast<DPFOutputRecord *>(&debug_output_buffer[index]);
        // Lookup the VkShaderModule handle and SPIR-V code used to create the shader, using the unique shader ID value returned
        // by the instrumented shader.
        auto it = shader_map.find(debug_record->shader_id);
        if (it != shader_map.end()) {
            shader_module_handle = it->second.shader_module;
            pipeline_handle = it->second.pipeline;
            pgm = it->second.pgm;
        }
        // Search through the shader source for the printf format string for this invocation
        auto format_string = FindFormatString(pgm, debug_record->format_string_id);
        // Break the format string into strings with 1 or 0 value
        auto format_substrings = ParseFormatString(format_string);
        void *values = static_cast<void *>(&debug_record->values);
        const uint32_t static_size = 1024;
        // Sprintf each format substring into a temporary string then add that to the message
        for (auto &substring : format_substrings) {
            char temp_string[static_size];
            size_t needed = 0;
            std::vector<std::string> format_strings = { "%ul", "%lu", "%lx" };
            size_t ul_pos = 0;
            bool print_hex = true;
            for (auto ul_string : format_strings) {
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
                needed = snprintf(temp_string, static_size, substring.string.c_str(), substring.longval);
            } else {
                if (substring.needs_value) {
                    switch (substring.type) {
                        case varunsigned:
                            needed = snprintf(temp_string, static_size, substring.string.c_str(), *static_cast<uint32_t *>(values));
                            break;

                        case varsigned:
                            needed = snprintf(temp_string, static_size, substring.string.c_str(), *static_cast<int32_t *>(values));
                            break;

                        case varfloat:
                            needed = snprintf(temp_string, static_size, substring.string.c_str(), *static_cast<float *>(values));
                            break;
                    }
                    values = static_cast<uint32_t *>(values) + 1;
                } else {
                    needed = snprintf(temp_string, static_size, substring.string.c_str());
                }
            }

            if (needed < static_size) {
                shader_message << temp_string;
            } else {
                // Static buffer not big enough for message, use malloc to get enough
                snprintf_with_malloc(shader_message, substring, needed, values);
            }
        }

        if (verbose) {
            std::string stage_message;
            std::string common_message;
            std::string filename_message;
            std::string source_message;
            UtilGenerateStageMessage(&debug_output_buffer[index], stage_message);
            UtilGenerateCommonMessage(report_data, command_buffer, &debug_output_buffer[index], shader_module_handle,
                                      pipeline_handle, buffer_info.pipeline_bind_point, operation_index, common_message);
            UtilGenerateSourceMessages(pgm, &debug_output_buffer[index], true, filename_message, source_message);
            if (use_stdout) {
                std::cout << "UNASSIGNED-DEBUG-PRINTF " << common_message.c_str() << " " << stage_message.c_str() << " "
                          << shader_message.str().c_str() << " " << filename_message.c_str() << " " << source_message.c_str();
            } else {
                LogInfo(queue, "UNASSIGNED-DEBUG-PRINTF", "%s %s %s %s%s", common_message.c_str(), stage_message.c_str(),
                        shader_message.str().c_str(), filename_message.c_str(), source_message.c_str());
            }
        } else {
            if (use_stdout) {
                std::cout << shader_message.str();
            } else {
                // Don't let LogInfo process any '%'s in the string
                LogInfo(device, "UNASSIGNED-DEBUG-PRINTF", "%s", shader_message.str().c_str());
            }
        }
        index += debug_record->size;
    }
    if ((index - 1) != expect) {
        LogWarning(device, "UNASSIGNED-DEBUG-PRINTF",
                   "WARNING - Debug Printf message was truncated, likely due to a buffer size that was too small for the message");
    }
    memset(debug_output_buffer, 0, 4 * (debug_output_buffer[0] + 1));
}

// For the given command buffer, map its debug data buffers and read their contents for analysis.
void debug_printf_state::CommandBuffer::Process(VkQueue queue) {
    auto *device_state = static_cast<DebugPrintf *>(dev_data);
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

void DebugPrintf::PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                       uint32_t firstVertex, uint32_t firstInstance) {
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void DebugPrintf::PreCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                               const VkMultiDrawInfoEXT *pVertexInfo, uint32_t instanceCount,
                                               uint32_t firstInstance, uint32_t stride) {
    for(uint32_t i = 0; i < drawCount; i++) {
        AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
    }
}

void DebugPrintf::PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                              uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void DebugPrintf::PreCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                      const VkMultiDrawIndexedInfoEXT *pIndexInfo, uint32_t instanceCount,
                                                      uint32_t firstInstance, uint32_t stride, const int32_t *pVertexOffset) {
    for (uint32_t i = 0; i < drawCount; i++) {
        AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
    }
}

void DebugPrintf::PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                               uint32_t stride) {
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void DebugPrintf::PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      uint32_t count, uint32_t stride) {
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void DebugPrintf::PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) {
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE);
}

void DebugPrintf::PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE);
}

void DebugPrintf::PreCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                               uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                               uint32_t groupCountZ) {
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE);
}

void DebugPrintf::PreCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                  uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                  uint32_t groupCountZ) {
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE);
}

void DebugPrintf::PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride) {
    ValidationStateTracker::PreCallRecordCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                                 maxDrawCount, stride);
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void DebugPrintf::PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                    uint32_t stride) {
    ValidationStateTracker::PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                              maxDrawCount, stride);
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void DebugPrintf::PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                              uint32_t maxDrawCount, uint32_t stride) {
    ValidationStateTracker::PreCallRecordCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer,
                                                                        countBufferOffset, maxDrawCount, stride);
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void DebugPrintf::PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride) {
    ValidationStateTracker::PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                                     maxDrawCount, stride);
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void DebugPrintf::PreCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                           uint32_t firstInstance, VkBuffer counterBuffer,
                                                           VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                           uint32_t vertexStride) {
    ValidationStateTracker::PreCallRecordCmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer,
                                                                     counterBufferOffset, counterOffset, vertexStride);
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void DebugPrintf::PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) {
    ValidationStateTracker::PreCallRecordCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void DebugPrintf::PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          uint32_t drawCount, uint32_t stride) {
    ValidationStateTracker::PreCallRecordCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void DebugPrintf::PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                               uint32_t maxDrawCount, uint32_t stride) {
    ValidationStateTracker::PreCallRecordCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer,
                                                                         countBufferOffset, maxDrawCount, stride);
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void DebugPrintf::PreCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                              VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                              VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                              VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                              VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                              VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                              uint32_t width, uint32_t height, uint32_t depth) {
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV);
}

void DebugPrintf::PreCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                               const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, uint32_t width,
                                               uint32_t height, uint32_t depth) {
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
}

void DebugPrintf::PreCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                       const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                       const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                       const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                       const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                                       VkDeviceAddress indirectDeviceAddress) {
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
}

void DebugPrintf::PreCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress) {
    AllocateDebugPrintfResources(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
}

void DebugPrintf::AllocateDebugPrintfResources(const VkCommandBuffer cmd_buffer, const VkPipelineBindPoint bind_point) {
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

    auto cb_node = GetWrite<debug_printf_state::CommandBuffer>(cmd_buffer);
    if (!cb_node) {
        ReportSetupProblem(device, "Unrecognized command buffer");
        aborted = true;
        return;
    }

    const auto lv_bind_point = ConvertToLvlBindPoint(bind_point);
    const auto *pipeline_state = cb_node->lastBound[lv_bind_point].pipeline_state;

    // TODO (ncesario) remove once VK_EXT_graphics_pipeline_library support is added for debug printf
    if (pipeline_state && pipeline_state->IsGraphicsLibrary()) {
        ReportSetupProblem(device, "Debug printf does not currently support VK_EXT_graphics_pipeline_library");
        aborted = true;
        return;
    }

    // Allocate memory for the output block that the gpu will use to return values for printf
    DPFDeviceMemoryBlock output_block = {};
    VkBufferCreateInfo buffer_info = LvlInitStruct<VkBufferCreateInfo>();
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

    auto desc_writes = LvlInitStruct<VkWriteDescriptorSet>();
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

    if (pipeline_state) {
        const auto &pipeline_layout = pipeline_state->PipelineLayoutState();
        if (pipeline_layout->set_layouts.size() <= desc_set_bind_index) {
            DispatchCmdBindDescriptorSets(cmd_buffer, bind_point, pipeline_layout->layout(), desc_set_bind_index, 1,
                                          desc_sets.data(), 0, nullptr);
        }
        // Record buffer and memory info in CB state tracking
        cb_node->buffer_infos.emplace_back(output_block, desc_sets[0], desc_pool, bind_point);
    } else {
        ReportSetupProblem(device, "Unable to find pipeline state");
        vmaDestroyBuffer(vmaAllocator, output_block.buffer, output_block.allocation);
        aborted = true;
        return;
    }
}

std::shared_ptr<CMD_BUFFER_STATE> DebugPrintf::CreateCmdBufferState(VkCommandBuffer cb,
                                                                    const VkCommandBufferAllocateInfo *pCreateInfo,
                                                                    const COMMAND_POOL_STATE *pool) {
    return std::static_pointer_cast<CMD_BUFFER_STATE>(
        std::make_shared<debug_printf_state::CommandBuffer>(this, cb, pCreateInfo, pool));
}

debug_printf_state::CommandBuffer::CommandBuffer(DebugPrintf *dp, VkCommandBuffer cb,
                                                 const VkCommandBufferAllocateInfo *pCreateInfo, const COMMAND_POOL_STATE *pool)
    : gpu_utils_state::CommandBuffer(dp, cb, pCreateInfo, pool) {}

void debug_printf_state::CommandBuffer::Reset() {
    CMD_BUFFER_STATE::Reset();
    auto debug_printf = static_cast<DebugPrintf *>(dev_data);
    // Free the device memory and descriptor set(s) associated with a command buffer.
    if (debug_printf->aborted) {
        return;
    }
    for (auto &buffer_info : buffer_infos) {
        debug_printf->DestroyBuffer(buffer_info);
    }
    buffer_infos.clear();
}
