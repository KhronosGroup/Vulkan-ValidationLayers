/* Copyright (c) 2024-2025 LunarG, Inc.
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

#include "containers/container_utils.h"
#include "gpuav/core/gpuav.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/resources/gpuav_vulkan_objects.h"
#include "gpuav/shaders/gpuav_error_codes.h"
#include "gpuav/shaders/gpuav_error_header.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "gpuav/spirv/vertex_attribute_fetch_oob_pass.h"
#include "state_tracker/shader_module.h"
#include "utils/action_command_utils.h"
#include "utils/image_utils.h"

namespace gpuav {
struct VertexAttributeFetchLimit {
    // Default value indicates that no vertex buffer attribute fetching will be OOB
    VkDeviceSize max_vertex_attributes_count = std::numeric_limits<VkDeviceSize>::max();
    vvl::VertexBufferBinding binding_info{};
    VkVertexInputAttributeDescription attribute{};
    uint32_t instance_rate_divisor = vvl::kNoIndex32;
};

// Computes vertex attributes fetching limits based on the set of bound vertex buffers.
// Used to detect out of bounds indices in index buffers.
static std::pair<std::optional<VertexAttributeFetchLimit>, std::optional<VertexAttributeFetchLimit>> GetVertexAttributeFetchLimits(
    const vvl::CommandBuffer &cb_state) {
    const LastBound &last_bound = cb_state.GetLastBoundGraphics();
    const vvl::Pipeline *pipeline_state = last_bound.pipeline_state;

    const bool dynamic_vertex_input = last_bound.IsDynamic(CB_DYNAMIC_STATE_VERTEX_INPUT_EXT);

    const auto &vertex_binding_descriptions =
        dynamic_vertex_input ? cb_state.dynamic_state_value.vertex_bindings : pipeline_state->vertex_input_state->bindings;

    std::optional<VertexAttributeFetchLimit> vertex_attribute_fetch_limit_vertex_input_rate;
    std::optional<VertexAttributeFetchLimit> vertex_attribute_fetch_limit_instance_input_rate;

    small_vector<uint32_t, 32> vertex_shader_used_locations;
    {
        const ::spirv::EntryPoint *vertex_entry_point = last_bound.GetVertexEntryPoint();
        if (!vertex_entry_point) {
            return {std::optional<VertexAttributeFetchLimit>{}, std::optional<VertexAttributeFetchLimit>{}};
        }
        for (const ::spirv::StageInterfaceVariable &interface_var : vertex_entry_point->stage_interface_variables) {
            for (const ::spirv::InterfaceSlot &interface_slot : interface_var.interface_slots) {
                const uint32_t location = interface_slot.Location();
                if (std::find(vertex_shader_used_locations.begin(), vertex_shader_used_locations.end(), location) ==
                    vertex_shader_used_locations.end()) {
                    vertex_shader_used_locations.emplace_back(location);
                }
            }
        }
    }

    for (const auto &[binding, vertex_binding_desc] : vertex_binding_descriptions) {
        const vvl::VertexBufferBinding *vbb = vvl::Find(cb_state.current_vertex_buffer_binding_info, binding);
        if (!vbb) {
            // Validation error
            continue;
        }

        for (const auto &[location, attrib] : vertex_binding_desc.locations) {
            if (std::find(vertex_shader_used_locations.begin(), vertex_shader_used_locations.end(), location) ==
                vertex_shader_used_locations.end()) {
                continue;
            }
            const VkDeviceSize attribute_size = GetVertexInputFormatSize(attrib.desc.format);

            VkDeviceSize vertex_buffer_remaining_size =
                vbb->effective_size > attrib.desc.offset ? vbb->effective_size - attrib.desc.offset : 0;

            VkDeviceSize vertex_attributes_count = 0;
            if (vbb->stride > 0) {
                vertex_attributes_count = vertex_buffer_remaining_size / vbb->stride;

                if (vertex_buffer_remaining_size > vertex_attributes_count * vbb->stride) {
                    vertex_buffer_remaining_size -= vertex_attributes_count * vbb->stride;
                } else {
                    vertex_buffer_remaining_size = 0;
                }

                // maybe room for one more attribute but not full stride - not having stride space does not matter for last element
                if (vertex_buffer_remaining_size >= attribute_size) {
                    vertex_attributes_count += 1;
                }
            } else {
                // For the current attribute type, if stride is 0, the same vertex data chunk will be accessed by all vertex shader
                // instances See https://docs.vulkan.org/spec/latest/chapters/fxvertex.html#fxvertex-input-address-calculation
                if (vertex_buffer_remaining_size >= attribute_size) {
                    // attribute won't be limiting
                    continue;
                } else {
                    // Vertex attribute does not fit in vertex buffer
                    vertex_attributes_count = 0;
                }
            }

            if (vertex_binding_desc.desc.inputRate == VK_VERTEX_INPUT_RATE_VERTEX) {
                if (!vertex_attribute_fetch_limit_vertex_input_rate.has_value()) {
                    vertex_attribute_fetch_limit_vertex_input_rate = VertexAttributeFetchLimit{};
                }

                vertex_attribute_fetch_limit_vertex_input_rate->max_vertex_attributes_count =
                    std::min(vertex_attribute_fetch_limit_vertex_input_rate->max_vertex_attributes_count, vertex_attributes_count);
                if (vertex_attribute_fetch_limit_vertex_input_rate->max_vertex_attributes_count == vertex_attributes_count) {
                    vertex_attribute_fetch_limit_vertex_input_rate->binding_info = *vbb;
                    vertex_attribute_fetch_limit_vertex_input_rate->attribute.location = attrib.desc.location;
                    vertex_attribute_fetch_limit_vertex_input_rate->attribute.binding = attrib.desc.binding;
                    vertex_attribute_fetch_limit_vertex_input_rate->attribute.format = attrib.desc.format;
                    vertex_attribute_fetch_limit_vertex_input_rate->attribute.offset = attrib.desc.offset;
                }
            } else if (vertex_binding_desc.desc.inputRate == VK_VERTEX_INPUT_RATE_INSTANCE) {
                if (!vertex_attribute_fetch_limit_instance_input_rate.has_value()) {
                    vertex_attribute_fetch_limit_instance_input_rate = VertexAttributeFetchLimit{};
                }

                vertex_attribute_fetch_limit_instance_input_rate->max_vertex_attributes_count =
                    std::min(vertex_attribute_fetch_limit_instance_input_rate->max_vertex_attributes_count,
                             vertex_attributes_count * vertex_binding_desc.desc.divisor);
                if (vertex_attribute_fetch_limit_instance_input_rate->max_vertex_attributes_count ==
                    (vertex_attributes_count * vertex_binding_desc.desc.divisor)) {
                    vertex_attribute_fetch_limit_instance_input_rate->binding_info = *vbb;
                    vertex_attribute_fetch_limit_instance_input_rate->attribute.location = attrib.desc.location;
                    vertex_attribute_fetch_limit_instance_input_rate->attribute.binding = attrib.desc.binding;
                    vertex_attribute_fetch_limit_instance_input_rate->attribute.format = attrib.desc.format;
                    vertex_attribute_fetch_limit_instance_input_rate->attribute.offset = attrib.desc.offset;
                    vertex_attribute_fetch_limit_instance_input_rate->instance_rate_divisor = vertex_binding_desc.desc.divisor;
                }
            }
        }
    }
    return {vertex_attribute_fetch_limit_vertex_input_rate, vertex_attribute_fetch_limit_instance_input_rate};
}

void RegisterVertexAttributeFetchOobValidation(Validator &gpuav, CommandBufferSubState &cb) {
    if (!gpuav.gpuav_settings.shader_instrumentation.vertex_attribute_fetch_oob) {
        return;
    }

    struct ErrorInfo {
        std::optional<VertexAttributeFetchLimit> vertex_attribute_fetch_limit_vertex_input_rate{};
        std::optional<VertexAttributeFetchLimit> vertex_attribute_fetch_limit_instance_input_rate{};
        std::optional<vvl::IndexBufferBinding> index_buffer_binding{};
    };

    // Used to communicate error info between lambdas
    auto error_info = std::make_shared<ErrorInfo>();

    cb.on_instrumentation_error_logger_register_functions.emplace_back([error_info](Validator &gpuav, CommandBufferSubState &cb,
                                                                                    const LastBound &last_bound) {
        auto local_error_info = std::make_shared<ErrorInfo>();
        *local_error_info = *error_info;
        CommandBufferSubState::InstrumentationErrorLogger inst_error_logger = [local_error_info = std::move(local_error_info)](
                                                                                  Validator &gpuav, const Location &loc,
                                                                                  const uint32_t *error_record,
                                                                                  std::string &out_error_msg,
                                                                                  std::string &out_vuid_msg) {
            if (GetErrorGroup(error_record) != glsl::kErrorGroupInstIndexedDraw) {
                return false;
            }

            const uint32_t error_sub_code = GetSubError(error_record);
            if (error_sub_code != glsl::kErrorSubCode_IndexedDraw_OOBVertexIndex &&
                error_sub_code != glsl::kErrorSubCode_IndexedDraw_OOBInstanceIndex) {
                return false;
            }

            switch (loc.function) {
                case vvl::Func::vkCmdDrawIndexed:
                    out_vuid_msg = "VUID-vkCmdDrawIndexed-None-02721";
                    break;
                case vvl::Func::vkCmdDrawIndexedIndirectCount:
                case vvl::Func::vkCmdDrawIndexedIndirectCountKHR:
                    out_vuid_msg = "VUID-vkCmdDrawIndexedIndirectCount-None-02721";
                    break;
                case vvl::Func::vkCmdDrawIndexedIndirect:
                    out_vuid_msg = "VUID-vkCmdDrawIndexedIndirect-None-02721";
                    break;
                case vvl::Func::vkCmdDrawMultiIndexedEXT:
                    out_vuid_msg = "VUID-vkCmdDrawMultiIndexedEXT-None-02721";
                    break;
                default:
                    return false;
            }

            assert(local_error_info->vertex_attribute_fetch_limit_vertex_input_rate.has_value() ||
                   local_error_info->vertex_attribute_fetch_limit_instance_input_rate.has_value());
            assert(local_error_info->index_buffer_binding.has_value());

            auto add_vertex_buffer_binding_info =
                [&gpuav, error_sub_code](const VertexAttributeFetchLimit &vertex_attribute_fetch_limit, std::string &out) {
                    out += "Vertex Buffer (";
                    out += gpuav.FormatHandle(vertex_attribute_fetch_limit.binding_info.buffer);
                    out += ") binding info:\n";
                    out += "  - Binding: ";
                    out += std::to_string(vertex_attribute_fetch_limit.attribute.binding);
                    out += '\n';
                    out += "  - Offset: ";
                    out += std::to_string(vertex_attribute_fetch_limit.binding_info.offset);
                    out += " bytes\n";
                    out += "  - Effective Size: ";
                    out += std::to_string(vertex_attribute_fetch_limit.binding_info.effective_size);
                    out += " bytes\n";
                    out += "  - Vertices Count: ";
                    out += std::to_string(vertex_attribute_fetch_limit.max_vertex_attributes_count);
                    out += '\n';
                    out += "  - Stride: ";
                    out += std::to_string(vertex_attribute_fetch_limit.binding_info.stride);
                    out += " bytes\n";
                    if (error_sub_code == glsl::kErrorSubCode_IndexedDraw_OOBInstanceIndex) {
                        if (vertex_attribute_fetch_limit.instance_rate_divisor != vvl::kNoIndex32) {
                            out += "  - Instance rate divisor: ";
                            out += std::to_string(vertex_attribute_fetch_limit.instance_rate_divisor);
                            out += '\n';
                        }
                    }
                };

            auto add_vertex_attribute_info = [](const VertexAttributeFetchLimit &vertex_attribute_fetch_limit, std::string &out) {
                out += "The following VkVertexInputAttributeDescription caused OOB access:\n";
                out += "  - Location: ";
                out += std::to_string(vertex_attribute_fetch_limit.attribute.location);
                out += '\n';
                out += "  - Binding: ";
                out += std::to_string(vertex_attribute_fetch_limit.attribute.binding);
                out += '\n';
                out += "  - Format: ";
                out += string_VkFormat(vertex_attribute_fetch_limit.attribute.format);
                out += '\n';
                out += "  - Offset: ";
                out += std::to_string(vertex_attribute_fetch_limit.attribute.offset);
                out += " bytes\n";
            };

            if (error_sub_code == glsl::kErrorSubCode_IndexedDraw_OOBVertexIndex) {
                out_error_msg += "Vertex index ";
                const uint32_t oob_vertex_index = error_record[glsl::kHeaderStageInfoOffset_0];
                out_error_msg += std::to_string(oob_vertex_index);
            } else if (error_sub_code == glsl::kErrorSubCode_IndexedDraw_OOBInstanceIndex) {
                out_error_msg += "Instance index ";
                const uint32_t oob_instance_index = error_record[glsl::kHeaderStageInfoOffset_1];
                out_error_msg += std::to_string(oob_instance_index);
                const uint32_t instance_rate_divisor =
                    local_error_info->vertex_attribute_fetch_limit_instance_input_rate->instance_rate_divisor;
                if (instance_rate_divisor > 1 && instance_rate_divisor != vvl::kNoIndex32) {
                    out_error_msg += " (or ";
                    out_error_msg += std::to_string(oob_instance_index / instance_rate_divisor);
                    out_error_msg += " if divided by instance rate divisor of ";
                    out_error_msg += std::to_string(instance_rate_divisor);
                    out_error_msg += ")";
                }
            }

            if (error_sub_code == glsl::kErrorSubCode_IndexedDraw_OOBVertexIndex) {
                out_error_msg += ", using VK_VERTEX_INPUT_RATE_VERTEX, has caused an OOB access within a bound vertex buffer.\n";

            } else if (error_sub_code == glsl::kErrorSubCode_IndexedDraw_OOBInstanceIndex) {
                out_error_msg += ", using VK_VERTEX_INPUT_RATE_INSTANCE, has caused an OOB access within a bound vertex buffer.\n";
            }

            if (error_sub_code == glsl::kErrorSubCode_IndexedDraw_OOBVertexIndex) {
                add_vertex_buffer_binding_info(*local_error_info->vertex_attribute_fetch_limit_vertex_input_rate, out_error_msg);
                add_vertex_attribute_info(*local_error_info->vertex_attribute_fetch_limit_vertex_input_rate, out_error_msg);

            } else if (error_sub_code == glsl::kErrorSubCode_IndexedDraw_OOBInstanceIndex) {
                add_vertex_buffer_binding_info(*local_error_info->vertex_attribute_fetch_limit_instance_input_rate, out_error_msg);
                add_vertex_attribute_info(*local_error_info->vertex_attribute_fetch_limit_instance_input_rate, out_error_msg);
            }

            if (error_sub_code == glsl::kErrorSubCode_IndexedDraw_OOBVertexIndex) {
                const uint32_t index_bits_size = GetIndexBitsSize(local_error_info->index_buffer_binding->index_type);
                const uint32_t max_indices_in_buffer =
                    static_cast<uint32_t>(local_error_info->index_buffer_binding->size / (index_bits_size / 8u));
                out_error_msg += "Index Buffer (";
                out_error_msg += gpuav.FormatHandle(local_error_info->index_buffer_binding->buffer);
                out_error_msg += ") binding info:\n";
                out_error_msg += "  - Type: ";
                out_error_msg += string_VkIndexType(local_error_info->index_buffer_binding->index_type);
                out_error_msg += '\n';
                out_error_msg += "  - Offset: ";
                out_error_msg += std::to_string(local_error_info->index_buffer_binding->offset);
                out_error_msg += " bytes\n";
                out_error_msg += "  - Size: ";
                out_error_msg += std::to_string(local_error_info->index_buffer_binding->size);
                out_error_msg += " bytes (sizeof(";
                out_error_msg += string_VkIndexType(local_error_info->index_buffer_binding->index_type);
                out_error_msg += ") * ";
                out_error_msg += std::to_string(max_indices_in_buffer);
                out_error_msg += ")\n";
            }
            out_error_msg +=
                "Note: Vertex buffer binding size is the effective, valid one, based on how the VkBuffer was created and "
                "the vkCmdBindVertexBuffers parameters. So it can be clamped up to 0 if binding was invalid.";

            return true;
        };

        return inst_error_logger;
    });

    cb.on_instrumentation_desc_set_update_functions.emplace_back(
        [&gpuav, error_info](CommandBufferSubState &cb, VkPipelineBindPoint bind_point, const Location &loc,
                             VkDescriptorBufferInfo &out_buffer_info, uint32_t &out_dst_binding) {
            if (!vvl::IsCommandDrawVertex(loc.function)) {
                return;
            }

            if (vvl::IsCommandDrawVertexIndexed(loc.function)) {
                vko::BufferRange vertex_attribute_fetch_limits_buffer_range =
                    cb.gpu_resources_manager.GetHostCoherentBufferRange(4 * sizeof(uint32_t));
                if (vertex_attribute_fetch_limits_buffer_range.buffer == VK_NULL_HANDLE) {
                    return;
                }

                auto vertex_attribute_fetch_limits_buffer_ptr =
                    (uint32_t *)vertex_attribute_fetch_limits_buffer_range.offset_mapped_ptr;

                const auto [vertex_attribute_fetch_limit_vertex_input_rate, vertex_attribute_fetch_limit_instance_input_rate] =
                    GetVertexAttributeFetchLimits(cb.base);
                if (vertex_attribute_fetch_limit_vertex_input_rate.has_value()) {
                    vertex_attribute_fetch_limits_buffer_ptr[0] = 1u;
                    vertex_attribute_fetch_limits_buffer_ptr[1] =
                        (uint32_t)vertex_attribute_fetch_limit_vertex_input_rate->max_vertex_attributes_count;
                } else {
                    vertex_attribute_fetch_limits_buffer_ptr[0] = 0u;
                }

                if (vertex_attribute_fetch_limit_instance_input_rate.has_value()) {
                    vertex_attribute_fetch_limits_buffer_ptr[2] = 1u;
                    vertex_attribute_fetch_limits_buffer_ptr[3] =
                        (uint32_t)vertex_attribute_fetch_limit_instance_input_rate->max_vertex_attributes_count;
                } else {
                    vertex_attribute_fetch_limits_buffer_ptr[2] = 0u;
                }

                error_info->vertex_attribute_fetch_limit_vertex_input_rate = vertex_attribute_fetch_limit_vertex_input_rate;
                error_info->vertex_attribute_fetch_limit_instance_input_rate = vertex_attribute_fetch_limit_instance_input_rate;
                error_info->index_buffer_binding = cb.base.index_buffer_binding;

                out_buffer_info.buffer = vertex_attribute_fetch_limits_buffer_range.buffer;
                out_buffer_info.offset = vertex_attribute_fetch_limits_buffer_range.offset;
                out_buffer_info.range = vertex_attribute_fetch_limits_buffer_range.size;
            } else {
                // Point all non-indexed draws to our global buffer that will bypass the check in shader
                VertexAttributeFetchOff &resource = gpuav.shared_resources_cache.GetOrCreate<VertexAttributeFetchOff>(gpuav);
                if (!resource.valid) {
                    return;
                }
                out_buffer_info.buffer = resource.buffer.VkHandle();
                out_buffer_info.offset = 0;
                out_buffer_info.range = VK_WHOLE_SIZE;
            }

            out_dst_binding = glsl::kBindingInstVertexAttributeFetchLimits;
        });
}

}  // namespace gpuav
