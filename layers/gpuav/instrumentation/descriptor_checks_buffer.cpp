/* Copyright (c) 2026 LunarG, Inc.
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

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <cstring>
#include <vector>
#include <vulkan/utility/vk_struct_helper.hpp>
#include "drawdispatch/drawdispatch_vuids.h"
#include "gpuav/core/gpuav.h"
#include "gpuav/descriptor_validation/gpuav_descriptor_validation.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/resources/gpuav_shader_resources.h"
#include "gpuav/resources/gpuav_vulkan_objects.h"
#include "gpuav/shaders/gpuav_error_codes.h"
#include "gpuav/shaders/gpuav_error_header.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/descriptor_hashing.h"
#include "utils/descriptor_utils.h"
#include "utils/lock_utils.h"

namespace gpuav {

// We hope people don't ever use Descriptor Buffer and Heap together or we will have double allocation for no reason
struct DescriptorChecksBufferHashTable {
    DescriptorChecksBufferHashTable(CommandBufferSubState& cb, VkDeviceSize size) {
        buffer_range = cb.gpu_resources_manager.GetDeviceLocalBufferRange(size);
    }
    vko::BufferRange buffer_range{};
};

void RegisterDescriptorChecksBufferValidation(Validator& gpuav, CommandBufferSubState& cb) {
    if (!gpuav.gpuav_settings.shader_instrumentation.descriptor_checks) {
        return;
    }
    if (!gpuav.enabled_features.descriptorBuffer) {
        return;
    }
    // The only check requires descriptor hashing
    if (!gpuav.global_settings.descriptor_hashing) {
        return;
    }

    cb.on_instrumentation_common_desc_update_functions.emplace_back([&gpuav](CommandBufferSubState& cb, const LastBound& last_bound,
                                                                             const Location&,
                                                                             CommonDescriptorUpdate& out_update) mutable {
        // Only way this can happen if you have no descriptor (push constant only)
        // in which case, no one will care about this buffer so can skip
        if (last_bound.ds_slots.empty()) {
            return;
        }

        assert(last_bound.desc_set_pipeline_layout);
        const vvl::PipelineLayout& pipeline_layout = *last_bound.desc_set_pipeline_layout;

        const uint32_t buffer_size = sizeof(VkDeviceAddress) * 2;
        vko::BufferRange output_range = cb.gpu_resources_manager.GetHostCoherentBufferRange(buffer_size);

        // DescriptorBufferEncoding
        VkDeviceAddress* buffer_encoding = (VkDeviceAddress*)output_range.offset_mapped_ptr;

        // DescriptorBufferSets
        const size_t buffer_sets_size = sizeof(VkDeviceAddress) * last_bound.ds_slots.size();
        vko::BufferRange buffer_sets_range = cb.gpu_resources_manager.GetHostCoherentBufferRange(buffer_sets_size);
        buffer_encoding[0] = buffer_sets_range.offset_address;
        auto buffer_sets_ptr = (VkDeviceAddress*)buffer_sets_range.offset_mapped_ptr;

        for (uint32_t ds_i = 0; ds_i < last_bound.ds_slots.size(); ds_i++) {
            const auto& ds_slot = last_bound.ds_slots[ds_i];

            bool skip_set = false;
            const vvl::DescriptorSetLayout* dsl_state = pipeline_layout.set_layouts.list[ds_i].get();
            if (!dsl_state || dsl_state->Destroyed()) {
                // PositiveDescriptorBuffer.DestroyDescriptor
                // TODO - Handle this as the information should be saved somewhere (GPU Dump has same issue)
                skip_set = true;
            } else {
                const bool is_embedded =
                    (dsl_state->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT) != 0;
                const bool is_push_descriptor =
                    (dsl_state->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR) != 0;
                if (is_embedded || is_push_descriptor) {
                    skip_set = true;
                }

                if (!ds_slot.descriptor_buffer_binding.has_value() && !is_push_descriptor) {
                    // This means this is just not a proper Descriptor Buffer set
                    // Also happens if they only have a Set = 1 and Set = 0 is empty/null
                    continue;
                }
            }

            const size_t set_encoding_size = sizeof(glsl::DescriptorBufferSetEncoding);
            vko::BufferRange set_encoding_range = cb.gpu_resources_manager.GetHostCoherentBufferRange(set_encoding_size);
            auto set_encoding_ptr = (glsl::DescriptorBufferSetEncoding*)set_encoding_range.offset_mapped_ptr;

            if (skip_set) {
                // Need way to skip in shader which won't know this information
                set_encoding_ptr->skip = 1;
            } else {
                VkDeviceAddress buffer_address =
                    cb.base.descriptor_buffer.binding_info[ds_slot.descriptor_buffer_binding->index].address;
                buffer_address += ds_slot.descriptor_buffer_binding->offset;

                const VkDescriptorSetLayout dsl_handle = dsl_state->VkHandle();

                std::vector<glsl::DescriptorBufferBindingEncoding> binding_encoding;
                for (const auto& dsl_binding : dsl_state->GetBindings()) {
                    VkDeviceSize binding_offset = 0;
                    DispatchGetDescriptorSetLayoutBindingOffsetEXT(gpuav.device, dsl_handle, dsl_binding.binding, &binding_offset);
                    // there is no practical way the offset can be over 4GB
                    binding_encoding.emplace_back(
                        glsl::DescriptorBufferBindingEncoding{dsl_binding.binding, (uint32_t)binding_offset});
                }

                const size_t binding_encoding_size = sizeof(glsl::DescriptorBufferBindingEncoding) * binding_encoding.size();
                vko::BufferRange binding_encoding_range =
                    cb.gpu_resources_manager.GetHostCoherentBufferRange(binding_encoding_size);
                memcpy(binding_encoding_range.offset_mapped_ptr, binding_encoding.data(), binding_encoding_size);

                set_encoding_ptr->buffer_address = buffer_address;
                set_encoding_ptr->bindings = binding_encoding_range.offset_address;
                set_encoding_ptr->binding_count = (uint32_t)binding_encoding.size();
                set_encoding_ptr->skip = 0;
            }

            buffer_sets_ptr[ds_i] = set_encoding_range.offset_address;
        }

        {
            assert(gpuav.global_settings.descriptor_hashing);
            ReadLockGuard guard(gpuav.device_state->descriptor_hashing->map_lock);
            DescriptorChecksBufferHashTable& hash_table_state =
                cb.shared_resources_cache.GetOrCreate<DescriptorChecksBufferHashTable>(
                    cb, gpuav.device_state->descriptor_hashing->table.Size());
            buffer_encoding[1] = hash_table_state.buffer_range.offset_address;
        }

        out_update.buffer = output_range.buffer;
        out_update.offset = output_range.offset;
        out_update.range = output_range.size;
        out_update.address = output_range.offset_address;
        out_update.binding = glsl::kBindingInstDescriptorBuffer;
    });

    cb.on_pre_cb_submission_functions.emplace_back(
        [](Validator& gpuav, CommandBufferSubState& cb, VkCommandBuffer per_submission_cb) {
            if (!gpuav.global_settings.descriptor_hashing) {
                return;
            }

            DescriptorChecksBufferHashTable* hash_table_state = cb.shared_resources_cache.TryGet<DescriptorChecksBufferHashTable>();
            if (!hash_table_state) {
                return;
            }

            // Same as |descriptor_checks_heap.cpp|
            vvl::DescriptorHashing& descriptor_hashing = *gpuav.device_state->descriptor_hashing;
            ReadLockGuard guard(descriptor_hashing.map_lock);
            const VkDeviceSize table_size = descriptor_hashing.table.Size();

            vko::BufferRange staging_buffer = cb.gpu_resources_manager.GetHostCachedBufferRange(table_size);
            memcpy(staging_buffer.offset_mapped_ptr, descriptor_hashing.table.slots.data(), static_cast<size_t>(table_size));
            cb.gpu_resources_manager.FlushAllocation(staging_buffer);

            vko::CmdSynchronizedCopyBufferRange(per_submission_cb, hash_table_state->buffer_range, staging_buffer);
        });

    cb.on_instrumentation_error_logger_register_functions.emplace_back(
        [](Validator& gpuav, CommandBufferSubState& cb, const LastBound& last_bound) {
            CommandBufferSubState::InstrumentationErrorLogger inst_error_logger =
                [](Validator& gpuav, const Location& loc, const uint32_t* error_record,
                   const InstrumentedShader* instrumented_shader, std::string& out_error_msg, std::string& out_vuid_msg) {
                    using namespace glsl;

                    bool error_found = false;
                    if (GetErrorGroup(error_record) != kErrorGroup_InstDescriptorBuffer) {
                        return error_found;
                    }
                    ReadLockGuard guard(gpuav.device_state->descriptor_hashing->map_lock);
                    if (gpuav.device_state->descriptor_hashing->table.limit_reported) {
                        // If we have hit the limit, we are likely going to spam errors and want them to see
                        // the DESCRIPTOR-HASHING-LIMIT only instead
                        return error_found;
                    }
                    error_found = true;

                    std::ostringstream ss;

                    const uint32_t encoded_set_index = error_record[kInst_LogError_ParameterOffset_1];
                    const uint32_t descriptor_set = encoded_set_index >> kInst_DescriptorIndexing_SetShift;
                    const uint32_t descriptor_index = encoded_set_index & kInst_DescriptorIndexing_IndexMask;
                    const uint32_t descriptor_binding = error_record[kInst_LogError_ParameterOffset_2];
                    // TODO - Get the OpVariable out as well
                    ss << "[Set " << std::dec << descriptor_set << ", Binding " << descriptor_binding << ", Index "
                       << descriptor_index << "] ";

                    const uint32_t error_sub_code = GetSubError(error_record);
                    // TODO - Print the actual address and how it got to it (similar to GPU Dump)
                    switch (error_sub_code) {
                        case kErrorSubCode_DescriptorBuffer_DescriptorHashing_None: {
                            ss << "is accessing the descriptor buffer but there is no valid descriptor at that location.";
                            out_vuid_msg = vvl::CreateActionVuid(loc.function, vvl::ActionVUID::DESCRIPTOR_BUFFER_08116);
                            error_found = true;
                        } break;
                        case kErrorSubCode_DescriptorBuffer_DescriptorHashing_Wrong: {
                            const uint32_t slot_index = error_record[kInst_LogError_ParameterOffset_0];
                            const auto& entry = gpuav.device_state->descriptor_hashing->table.slots[slot_index].entry;
                            ss << "is accessing the  descriptor buffer but there is a descriptor there of the wrong type: \n";
                            entry.Describe(*gpuav.device_state, ss);
                            out_vuid_msg = vvl::CreateActionVuid(loc.function, vvl::ActionVUID::DESCRIPTOR_BUFFER_08116);
                            error_found = true;
                        } break;
                    }
                    ss << '\n';
                    out_error_msg += ss.str();
                    return error_found;
                };

            return inst_error_logger;
        });
}

}  // namespace gpuav
