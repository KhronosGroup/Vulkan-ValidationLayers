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

#include "gpu/debug_printf/debug_printf.h"
#include <vulkan/vulkan_core.h>
#include "generated/layer_chassis_dispatch.h"
#include "chassis/chassis_modification_state.h"
#include "gpu/shaders/gpu_error_header.h"
#include "gpu/shaders/gpu_shaders_constants.h"
#include "gpu/resources/gpuav_subclasses.h"
#include "gpu/core/gpuav.h"

#include <iostream>

namespace debug_printf {

void Validator::PreCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator, VkDevice *pDevice,
                                          const RecordObject &record_obj, vku::safe_VkDeviceCreateInfo *modified_create_info) {
    BaseClass::PreCallRecordCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice, record_obj, modified_create_info);
}

// Perform initializations that can be done at Create Device time.
void Validator::PostCreateDevice(const VkDeviceCreateInfo *pCreateInfo, const Location &loc) {
    if (enabled[gpu_validation]) {
        InternalError(device, loc, "Debug Printf cannot be enabled when gpu assisted validation is enabled.");
        return;
    }

    instrumentation_bindings_.emplace_back(VkDescriptorSetLayoutBinding{
        gpuav::glsl::kBindingInstDebugPrintf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr});

    // Currently, both GPU-AV and DebugPrintf set their own instrumentation_bindings_ that this call will use
    BaseClass::PostCreateDevice(pCreateInfo, loc);
    // We might fail in parent class device creation if global requirements are not met
    if (aborted_) return;
}

}  // namespace debug_printf

namespace gpuav {
namespace debug_printf {

enum NumericType {
    NumericTypeUnknown = 0,
    NumericTypeFloat = 1,
    NumericTypeSint = 2,
    NumericTypeUint = 4,
};

static NumericType NumericTypeLookup(char specifier) {
    switch (specifier) {
        case 'd':
        case 'i':
            return NumericTypeSint;
            break;

        case 'f':
        case 'F':
        case 'a':
        case 'A':
        case 'e':
        case 'E':
        case 'g':
        case 'G':
            return NumericTypeFloat;
            break;

        case 'u':
        case 'x':
        case 'o':
        default:
            return NumericTypeUint;
            break;
    }
}

struct Substring {
    std::string string;
    bool needs_value = false;  // if value from buffer needed to print arguments
    NumericType type = NumericTypeUnknown;
    bool is_64_bit = false;
};

static std::vector<Substring> ParseFormatString(const std::string &format_string) {
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
            parsed_strings.emplace_back(substring);
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
            substring.needs_value = true;

            // We are just taking vector and creating a list of scalar that snprintf can handle
            if (format_string[pos] == 'v') {
                // Vector must be of size 2, 3, or 4
                // and format %v<size><type>
                std::string specifier = format_string.substr(percent, pos - percent);
                const int vec_size = atoi(&format_string[pos + 1]);
                pos += 2;

                // skip v<count>, handle long
                specifier.push_back(format_string[pos]);
                if (format_string[pos + 1] == 'l') {
                    // catches %ul
                    substring.is_64_bit = true;
                    specifier.push_back('l');
                    pos++;
                } else if (format_string[pos] == 'l') {
                    // catches %lu and lx
                    substring.is_64_bit = true;
                    specifier.push_back(format_string[pos + 1]);
                    pos++;
                }

                // Take the preceding characters, and the percent through the type
                substring.string = format_string.substr(begin, percent - begin);
                substring.string += specifier;
                substring.type = NumericTypeLookup(specifier.back());
                parsed_strings.emplace_back(substring);

                // Continue with a comma separated list
                char temp_string[32];
                snprintf(temp_string, sizeof(temp_string), ", %s", specifier.c_str());
                substring.string = temp_string;
                for (int i = 0; i < (vec_size - 1); i++) {
                    parsed_strings.emplace_back(substring);
                }
            } else {
                // Single non-vector value
                if (format_string[pos - 1] == 'l') {
                    substring.is_64_bit = true;  // finds %lu since we skipped the 'l' to find the 'u'
                } else if (format_string[pos + 1] == 'l') {
                    substring.is_64_bit = true;
                    pos++;  // Save long size
                }
                substring.string = format_string.substr(begin, pos - begin + 1);
                substring.type = NumericTypeLookup(format_string[pos]);
                parsed_strings.emplace_back(substring);
            }
            begin = pos + 1;
        }
    }
    return parsed_strings;
}

static std::string FindFormatString(const std::vector<gpu::spirv::Instruction> &instructions, uint32_t string_id) {
    std::string format_string;
    for (const auto &insn : instructions) {
        if (insn.Opcode() == spv::OpString && insn.Word(1) == string_id) {
            format_string = insn.GetAsString(2);
            break;
        }
        // if here, seen all OpString and can return early
        if (insn.Opcode() == spv::OpFunction) break;
    }
    return format_string;
}

// GCC and clang don't like using variables as format strings in sprintf.
// #pragma GCC is recognized by both compilers
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
#endif

// The contents each "printf" is writting to the output buffer streams
struct OutputRecord {
    uint32_t size;
    uint32_t shader_id;
    uint32_t instruction_position;
    uint32_t format_string_id;
    uint32_t double_bitmask;  // used to distinguish if float is 1 or 2 dwords
    uint32_t signed_8_bitmask;   // used to distinguish if signed int is a int8_t
    uint32_t signed_16_bitmask;  // used to distinguish if signed int is a int16_t
    uint32_t stage_id;
    uint32_t stage_info_0;
    uint32_t stage_info_1;
    uint32_t stage_info_2;
    uint32_t values;  // place holder to be casted to get rest of items in record
};

void AnalyzeAndGenerateMessage(Validator &gpuav, VkCommandBuffer command_buffer, VkQueue queue,
                               gpuav::DebugPrintfBufferInfo &buffer_info, uint32_t *const debug_output_buffer,
                               const Location &loc) {
    uint32_t output_record_counts = debug_output_buffer[gpuav::kDebugPrintfOutputBufferSize];
    if (!output_record_counts) return;

    uint32_t index = gpuav::kDebugPrintfOutputBufferData;  // get first OutputRecord index
    while (debug_output_buffer[index]) {
        std::stringstream shader_message;

        OutputRecord *debug_record = reinterpret_cast<OutputRecord *>(&debug_output_buffer[index]);
        // Lookup the VkShaderModule handle and SPIR-V code used to create the shader, using the unique shader ID value returned
        // by the instrumented shader.
        const gpu::GpuAssistedShaderTracker *tracker_info = nullptr;
        auto it = gpuav.shader_map_.find(debug_record->shader_id);
        if (it != gpuav.shader_map_.end()) {
            tracker_info = &it->second;
        }

        // without the instrumented spirv, there is nothing valuable to print out
        if (!tracker_info || tracker_info->instrumented_spirv.empty()) {
            gpuav.InternalWarning(queue, loc, "Can't find instructions from any handles in shader_map");
            return;
        }

        std::vector<gpu::spirv::Instruction> instructions;
        gpu::spirv::GenerateInstructions(tracker_info->instrumented_spirv, instructions);

        // Search through the shader source for the printf format string for this invocation
        const std::string format_string = FindFormatString(instructions, debug_record->format_string_id);
        // Break the format string into strings with 1 or 0 value
        auto format_substrings = ParseFormatString(format_string);
        void *current_value = static_cast<void *>(&debug_record->values);
        // Sprintf each format substring into a temporary string then add that to the message
        for (size_t i = 0; i < format_substrings.size(); i++) {
            auto &substring = format_substrings[i];
            std::string temp_string;
            size_t needed = 0;

            if (substring.needs_value) {
                if (substring.is_64_bit) {
                    assert(substring.type != NumericTypeSint);  // not supported
                    if (substring.type == NumericTypeUint) {
                        std::vector<std::string> format_strings = {"%ul", "%lu", "%lx"};
                        for (const auto &ul_string : format_strings) {
                            size_t ul_pos = substring.string.find(ul_string);
                            if (ul_pos == std::string::npos) continue;
                            if (ul_string != "%lu") {
                                substring.string.replace(ul_pos + 1, 2, PRIx64);
                            } else {
                                substring.string.replace(ul_pos + 1, 2, PRIu64);
                            }
                            break;
                        }

                        const uint64_t value = *static_cast<uint64_t *>(current_value);
                        // +1 for null terminator
                        needed = std::snprintf(nullptr, 0, substring.string.c_str(), value) + 1;
                        temp_string.resize(needed);
                        std::snprintf(&temp_string[0], needed, substring.string.c_str(), value);
                    }
                } else {
                    if (substring.type == NumericTypeUint) {
                        // +1 for null terminator
                        const uint32_t value = *static_cast<uint32_t *>(current_value);
                        needed = std::snprintf(nullptr, 0, substring.string.c_str(), value) + 1;
                        temp_string.resize(needed);
                        std::snprintf(&temp_string[0], needed, substring.string.c_str(), value);

                    } else if (substring.type == NumericTypeSint) {
                        // When dealing with signed int, we need to know which size the int was to print the correct value
                        if (debug_record->signed_8_bitmask & (1 << i)) {
                            const int8_t value = *static_cast<int8_t *>(current_value);
                            needed = std::snprintf(nullptr, 0, substring.string.c_str(), value) + 1;
                            temp_string.resize(needed);
                            std::snprintf(&temp_string[0], needed, substring.string.c_str(), value);
                        } else if (debug_record->signed_16_bitmask & (1 << i)) {
                            const int16_t value = *static_cast<int16_t *>(current_value);
                            needed = std::snprintf(nullptr, 0, substring.string.c_str(), value) + 1;
                            temp_string.resize(needed);
                            std::snprintf(&temp_string[0], needed, substring.string.c_str(), value);
                        } else {
                            const int32_t value = *static_cast<int32_t *>(current_value);
                            needed = std::snprintf(nullptr, 0, substring.string.c_str(), value) + 1;
                            temp_string.resize(needed);
                            std::snprintf(&temp_string[0], needed, substring.string.c_str(), value);
                        }

                    } else if (substring.type == NumericTypeFloat) {
                        // On the CPU printf the "%f" is used for 16, 32, and 64-bit floats,
                        // but we need to store the 64-bit floats in 2 dwords in our GPU side buffer.
                        // Using the bitmask, we know if the incoming float was 64-bit or not.
                        // This is much simpler than enforcing a %lf which doesn't line up with how the CPU side works
                        if (debug_record->double_bitmask & (1 << i)) {
                            substring.is_64_bit = true;
                            const double value = *static_cast<double *>(current_value);
                            needed = std::snprintf(nullptr, 0, substring.string.c_str(), value) + 1;
                            temp_string.resize(needed);
                            std::snprintf(&temp_string[0], needed, substring.string.c_str(), value);
                        } else {
                            const float value = *static_cast<float *>(current_value);
                            needed = std::snprintf(nullptr, 0, substring.string.c_str(), value) + 1;
                            temp_string.resize(needed);
                            std::snprintf(&temp_string[0], needed, substring.string.c_str(), value);
                        }
                    }
                }

                const uint32_t offset = substring.is_64_bit ? 2 : 1;
                current_value = static_cast<uint32_t *>(current_value) + offset;

            } else {
                // incase where someone just printing a string with no arguments to it
                needed = std::snprintf(nullptr, 0, substring.string.c_str()) + 1;
                temp_string.resize(needed);
                std::snprintf(&temp_string[0], needed, substring.string.c_str());
            }

            shader_message << temp_string.c_str();
        }

        const bool use_stdout = gpuav.gpuav_settings.debug_printf_to_stdout;
        if (gpuav.gpuav_settings.debug_printf_verbose) {
            std::string debug_info_message = gpuav.GenerateDebugInfoMessage(
                command_buffer, instructions, debug_record->stage_id, debug_record->stage_info_0, debug_record->stage_info_1,
                debug_record->stage_info_2, debug_record->instruction_position, tracker_info, debug_record->shader_id,
                buffer_info.pipeline_bind_point, buffer_info.action_command_index, true);
            if (use_stdout) {
                std::cout << "WARNING-DEBUG-PRINTF " << shader_message.str() << '\n' << debug_info_message;
            } else {
                gpuav.LogInfo("WARNING-DEBUG-PRINTF", queue, loc, "%s\n%s", shader_message.str().c_str(),
                              debug_info_message.c_str());
            }

        } else {
            if (use_stdout) {
                std::cout << shader_message.str();
            } else {
                // Don't let LogInfo process any '%'s in the string
                gpuav.LogInfo("WARNING-DEBUG-PRINTF", queue, loc, "%s", shader_message.str().c_str());
            }
        }
        index += debug_record->size;
    }
    if ((index - gpuav::kDebugPrintfOutputBufferData) != output_record_counts) {
        std::stringstream message;
        message << "Debug Printf message was truncated due to a buffer size (" << gpuav.gpuav_settings.debug_printf_buffer_size
                << ") being too small for the messages. (This can be adjusted with VK_LAYER_PRINTF_BUFFER_SIZE or vkconfig)";
        gpuav.InternalWarning(queue, loc, message.str().c_str());
    }

    // Only memset what is needed, in case we are only using a small portion of a large buffer_size.
    // At the same time we want to make sure we don't memset past the actual VkBuffer allocation
    uint32_t clear_size =
        sizeof(uint32_t) * (debug_output_buffer[gpuav::kDebugPrintfOutputBufferSize] + gpuav::kDebugPrintfOutputBufferData);
    clear_size = std::min(gpuav.gpuav_settings.debug_printf_buffer_size, clear_size);
    memset(debug_output_buffer, 0, clear_size);
}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

void AllocateResources(Validator &gpuav, CommandBuffer &cb_state, const VkPipelineBindPoint bind_point, const Location &loc) {
    if (gpuav.enabled[debug_printf_validation]) return;
    assert(bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS || bind_point == VK_PIPELINE_BIND_POINT_COMPUTE ||
           bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);

    const auto lv_bind_point = ConvertToLvlBindPoint(bind_point);
    const auto &last_bound = cb_state.lastBound[lv_bind_point];
    const auto *pipeline_state = last_bound.pipeline_state;

    // If there is no DebugPrintf instrumented, there is no reason to allocate buffers
    if (pipeline_state) {
        if (!pipeline_state->instrumentation_data.was_instrumented && (pipeline_state->linking_shaders == 0)) {
            cb_state.action_command_count++;
            return;
        }
    } else if (last_bound.HasShaderObjects()) {
        // TODO - Add same skip for shader object
    } else {
        gpuav.InternalError(cb_state.Handle(), loc, "Neither pipeline state nor shader object states were found.");
        return;
    }

    // TODO - use the following when we combine the two command buffer classes
    // VkDescriptorSet printf_desc_set = cb_state->gpu_resources_manager.GetManagedDescriptorSet(GetDebugDescriptorSetLayout());
    std::vector<VkDescriptorSet> desc_sets;
    VkDescriptorPool desc_pool = VK_NULL_HANDLE;
    VkResult result = gpuav.desc_set_manager_->GetDescriptorSets(1, &desc_pool, gpuav.GetDebugDescriptorSetLayout(), &desc_sets);
    if (result != VK_SUCCESS) {
        gpuav.InternalError(cb_state.Handle(), loc, "Unable to allocate descriptor sets.");
        return;
    }

    // Allocate memory for the output block that the gpu will use to return values for printf
    gpu::DeviceMemoryBlock output_block = {};
    VkBufferCreateInfo buffer_info = vku::InitStructHelper();
    buffer_info.size = gpuav.gpuav_settings.debug_printf_buffer_size;
    buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    result =
        vmaCreateBuffer(gpuav.vma_allocator_, &buffer_info, &alloc_info, &output_block.buffer, &output_block.allocation, nullptr);
    if (result != VK_SUCCESS) {
        gpuav.InternalError(cb_state.Handle(), loc, "Unable to allocate device memory.", true);
        return;
    }

    // Clear the output block to zeros so that only printf values from the gpu will be present
    uint32_t *data;
    result = vmaMapMemory(gpuav.vma_allocator_, output_block.allocation, reinterpret_cast<void **>(&data));
    if (result != VK_SUCCESS) {
        gpuav.InternalError(cb_state.Handle(), loc, "Unable to allocate map memory.", true);
        return;
    }
    memset(data, 0, gpuav.gpuav_settings.debug_printf_buffer_size);
    vmaUnmapMemory(gpuav.vma_allocator_, output_block.allocation);

    // Write the descriptor
    VkDescriptorBufferInfo output_desc_buffer_info = {};
    output_desc_buffer_info.range = gpuav.gpuav_settings.debug_printf_buffer_size;
    output_desc_buffer_info.buffer = output_block.buffer;
    output_desc_buffer_info.offset = 0;

    VkWriteDescriptorSet desc_writes = vku::InitStructHelper();
    desc_writes.descriptorCount = 1;
    desc_writes.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    desc_writes.pBufferInfo = &output_desc_buffer_info;
    desc_writes.dstSet = desc_sets[0];
    desc_writes.dstBinding = gpuav::glsl::kBindingInstDebugPrintf;
    DispatchUpdateDescriptorSets(gpuav.device, 1, &desc_writes, 0, nullptr);

    const auto pipeline_layout = pipeline_state ? pipeline_state->PipelineLayoutState()
                                                : gpuav.Get<vvl::PipelineLayout>(last_bound.desc_set_pipeline_layout);
    if (pipeline_layout) {
        // If GPL is used, it's possible the pipeline layout used at pipeline creation time is null. If CmdBindDescriptorSets has
        // not been called yet (i.e., state.pipeline_null), then fall back to the layout associated with pre-raster state.
        // PipelineLayoutState should be used for the purposes of determining the number of sets in the layout, but this layout
        // may be a "pseudo layout" used to represent the union of pre-raster and fragment shader layouts, and therefore have a
        // null handle.
        const auto pipeline_layout_handle = (last_bound.desc_set_pipeline_layout)
                                                ? last_bound.desc_set_pipeline_layout
                                                : pipeline_state->PreRasterPipelineLayoutState()->VkHandle();
        if (pipeline_layout->set_layouts.size() <= gpuav.desc_set_bind_index_) {
            DispatchCmdBindDescriptorSets(cb_state.VkHandle(), bind_point, pipeline_layout_handle, gpuav.desc_set_bind_index_, 1,
                                          desc_sets.data(), 0, nullptr);
        }
    } else {
        // If no pipeline layout was bound when using shader objects that don't use any descriptor set, bind the debug pipeline
        // layout
        DispatchCmdBindDescriptorSets(cb_state.VkHandle(), bind_point, gpuav.GetDebugPipelineLayout(), gpuav.desc_set_bind_index_,
                                      1, desc_sets.data(), 0, nullptr);
    }
    // Record buffer and memory info in CB state tracking
    cb_state.buffer_infos.emplace_back(output_block, desc_sets[0], desc_pool, bind_point, cb_state.action_command_count++);
}

}  // namespace debug_printf
}  // namespace gpuav
