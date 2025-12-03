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

#include "gpuav/core/gpuav.h"
#include "gpuav/error_message/gpuav_vuids.h"
#include "gpuav/resources/gpuav_shader_resources.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/shaders/gpuav_error_codes.h"
#include "gpuav/shaders/gpuav_error_header.h"

namespace gpuav {

// Returns the number of bytes to hold 32 bit aligned array of bits.
static uint32_t BitBufferSize(uint32_t num_bits) {
    static constexpr uint32_t kBitsPerWord = 32;
    return (((num_bits + (kBitsPerWord - 1)) & ~(kBitsPerWord - 1)) / kBitsPerWord) * sizeof(uint32_t);
}

DescriptorHeap::DescriptorHeap(Validator& gpuav, uint32_t max_descriptors) : max_descriptors_(max_descriptors), buffer_(gpuav) {
    // If max_descriptors_ is 0, GPU-AV aborted during vkCreateDevice(). We still need to
    // support calls into this class as no-ops if this happens.
    if (max_descriptors_ == 0) {
        return;
    }

    VkBufferCreateInfo buffer_info = vku::InitStruct<VkBufferCreateInfo>();
    buffer_info.size = BitBufferSize(max_descriptors_ + 1);  // add extra entry since 0 is the invalid id.
    buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    const bool success = buffer_.Create(&buffer_info, &alloc_info);
    if (!success) {
        return;
    }

    gpu_heap_state_ = (uint32_t*)buffer_.GetMappedPtr();
    memset(gpu_heap_state_, 0, static_cast<size_t>(buffer_info.size));
}

DescriptorHeap::~DescriptorHeap() {
    if (max_descriptors_ > 0) {
        buffer_.Destroy();
        gpu_heap_state_ = nullptr;
    }
}

DescriptorId DescriptorHeap::NextId(const VulkanTypedHandle& handle) {
    if (max_descriptors_ == 0) {
        return 0;
    }
    DescriptorId result;

    // NOTE: valid ids are in the range [1, max_descriptors_] (inclusive)
    // 0 is the invalid id.
    std::lock_guard guard(lock_);
    if (alloc_map_.size() >= max_descriptors_) {
        return 0;
    }
    do {
        result = next_id_++;
        if (next_id_ > max_descriptors_) {
            next_id_ = 1;
        }
    } while (alloc_map_.count(result) > 0);
    alloc_map_[result] = handle;
    gpu_heap_state_[result / 32] |= 1u << (result & 31);
    return result;
}

void DescriptorHeap::DeleteId(DescriptorId id) {
    if (max_descriptors_ > 0) {
        std::lock_guard guard(lock_);
        // Note: We don't mess with next_id_ here because ids should be assigned in LRU order.
        gpu_heap_state_[id / 32] &= ~(1u << (id & 31));
        alloc_map_.erase(id);
    }
}

struct DescriptorChecksCbState {
    vko::BufferRange last_bound_desc_sets_state_ssbo;
};

void DescriptorChecksOnFinishDeviceSetup(Validator& gpuav) {
    if (!gpuav.gpuav_settings.shader_instrumentation.descriptor_checks) {
        gpuav.shared_resources_cache.GetOrCreate<DescriptorHeap>(gpuav, 0);
        return;
    }

    VkPhysicalDeviceDescriptorIndexingProperties desc_indexing_props = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&desc_indexing_props);
    DispatchGetPhysicalDeviceProperties2Helper(gpuav.api_version, gpuav.physical_device, &props2);

    uint32_t num_descs = desc_indexing_props.maxUpdateAfterBindDescriptorsInAllPools;
    if (num_descs == 0 || num_descs > glsl::kDebugInputBindlessMaxDescriptors) {
        num_descs = glsl::kDebugInputBindlessMaxDescriptors;
    }

    gpuav.shared_resources_cache.GetOrCreate<DescriptorHeap>(gpuav, num_descs);
}

void RegisterDescriptorChecksValidation(Validator& gpuav, CommandBufferSubState& cb) {
    if (!gpuav.gpuav_settings.shader_instrumentation.descriptor_checks) {
        return;
    }

    cb.on_instrumentation_error_logger_register_functions.emplace_back([](Validator& gpuav, CommandBufferSubState& cb,
                                                                          const LastBound& last_bound) {
        // We want to grab the last (current) element in descriptor_binding_commands, but as a std::vector, the reference might be
        // garbage later, so just hold the index for later. It is possible to have no descriptor sets bound, for example if using
        // push constants.
        uint32_t descriptor_binding_index = vvl::kNoIndex32;
        DescriptorSetBindings* desc_set_bindings = cb.shared_resources_cache.TryGet<DescriptorSetBindings>();
        if (desc_set_bindings && !desc_set_bindings->descriptor_set_binding_commands.empty()) {
            descriptor_binding_index = uint32_t(desc_set_bindings->descriptor_set_binding_commands.size() - 1);
        }

        CommandBufferSubState::InstrumentationErrorLogger inst_error_logger = [&cb, descriptor_binding_index](
                                                                                  Validator& gpuav, const Location& loc,
                                                                                  const uint32_t* error_record,
                                                                                  std::string& out_error_msg,
                                                                                  std::string& out_vuid_msg) {
            using namespace glsl;

            bool error_found = false;
            if (GetErrorGroup(error_record) != kErrorGroupInstDescriptorIndexingOOB) {
                return error_found;
            }
            error_found = true;

            std::ostringstream strm;
            const GpuVuid& vuid = GetGpuVuid(loc.function);

            if (descriptor_binding_index == vvl::kNoIndex32) {
                assert(false);  // This means we have hit a situtation where there are no descriptors bound
                return false;
            }
            const DescriptorSetBindings& desc_set_bindings = cb.shared_resources_cache.Get<DescriptorSetBindings>();
            const auto& descriptor_sets =
                desc_set_bindings.descriptor_set_binding_commands[descriptor_binding_index].bound_descriptor_sets;

            // Currently we only encode the descriptor index here and save the binding in a parameter slot
            // The issue becomes if the user has kErrorSubCodeDescriptorIndexingBounds then we can't back track to the exact binding
            // because they have gone over it
            const uint32_t encoded_set_index = error_record[kInstDescriptorIndexingSetAndIndexOffset];
            const uint32_t set_num = encoded_set_index >> kInstDescriptorIndexingSetShift;
            const uint32_t descriptor_index = encoded_set_index & kInstDescriptorIndexingIndexMask;
            const uint32_t binding_num = error_record[kInstDescriptorIndexingParamOffset_1];

            const uint32_t array_length = error_record[kInstDescriptorIndexingParamOffset_0];

            const uint32_t error_sub_code = GetSubError(error_record);
            switch (error_sub_code) {
                case kErrorSubCodeDescriptorIndexingBounds: {
                    strm << "(set = " << set_num << ", binding = " << binding_num << ") Index of " << descriptor_index
                         << " used to index descriptor array of length " << array_length << ".";
                    out_vuid_msg = vuid.descriptor_index_oob_10068;
                    error_found = true;
                } break;

                case kErrorSubCodeDescriptorIndexingUninitialized: {
                    const auto& dsl = descriptor_sets[set_num]->Layout();
                    strm << "(set = " << set_num << ", binding = " << binding_num << ") Descriptor index " << descriptor_index
                         << " is uninitialized.";

                    if (descriptor_index == 0 && array_length == 1) {
                        strm << " (There is no array, but descriptor is viewed as having an array of length 1)";
                    }

                    const VkDescriptorBindingFlags binding_flags = dsl.GetDescriptorBindingFlagsFromBinding(binding_num);
                    if (binding_flags & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT) {
                        strm << " VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT was used and the original descriptorCount ("
                             << dsl.GetDescriptorCountFromBinding(binding_num)
                             << ") could have been reduced during AllocateDescriptorSets.";
                    } else if (gpuav.enabled_features.nullDescriptor) {
                        strm << " nullDescriptor feature is on, but vkUpdateDescriptorSets was not called with VK_NULL_HANDLE for "
                                "this "
                                "descriptor.";
                    }

                    out_vuid_msg = vuid.invalid_descriptor_08114;
                    error_found = true;
                } break;

                case kErrorSubCodeDescriptorIndexingDestroyed: {
                    strm << "(set = " << set_num << ", binding = " << binding_num << ") Descriptor index " << descriptor_index
                         << " references a resource that was destroyed.";

                    if (descriptor_index == 0 && array_length == 1) {
                        strm << " (There is no array, but descriptor is viewed as having an array of length 1)";
                    }

                    out_vuid_msg = "UNASSIGNED-Descriptor destroyed";
                    error_found = true;
                } break;
            }
            out_error_msg += strm.str();
            return error_found;
        };

        return inst_error_logger;
    });

    cb.on_instrumentation_error_logger_register_functions.emplace_back([](Validator& gpuav, CommandBufferSubState& cb,
                                                                          const LastBound& last_bound) {
        // We want to grab the last (current) element in descriptor_binding_commands, but as a std::vector, the reference might be
        // garbage later, so just hold the index for later. It is possible to have no descriptor sets bound, for example if using
        // push constants.
        uint32_t descriptor_binding_index = vvl::kNoIndex32;
        DescriptorSetBindings* desc_set_bindings = cb.shared_resources_cache.TryGet<DescriptorSetBindings>();
        if (desc_set_bindings && !desc_set_bindings->descriptor_set_binding_commands.empty()) {
            descriptor_binding_index = uint32_t(desc_set_bindings->descriptor_set_binding_commands.size() - 1);
        }

        bool uses_shader_object = last_bound.pipeline_state == nullptr;

        CommandBufferSubState::InstrumentationErrorLogger inst_error_logger = [&cb, descriptor_binding_index, uses_shader_object](
                                                                                  Validator& gpuav, const Location& loc,
                                                                                  const uint32_t* error_record,
                                                                                  std::string& out_error_msg,
                                                                                  std::string& out_vuid_msg) {
            using namespace glsl;
            bool error_found = false;
            if (GetErrorGroup(error_record) != kErrorGroupInstDescriptorClass) {
                return error_found;
            }
            error_found = true;
            std::ostringstream strm;
            const GpuVuid& vuid = GetGpuVuid(loc.function);

            if (descriptor_binding_index == vvl::kNoIndex32) {
                assert(false);  // This means we have hit a situtation where there are no descriptors bound
                return false;
            }
            const DescriptorSetBindings& desc_set_bindings = cb.shared_resources_cache.Get<DescriptorSetBindings>();
            const auto& descriptor_sets =
                desc_set_bindings.descriptor_set_binding_commands[descriptor_binding_index].bound_descriptor_sets;

            const uint32_t encoded_set_index = error_record[kInstDescriptorIndexingSetAndIndexOffset];
            const uint32_t set_num = encoded_set_index >> kInstDescriptorIndexingSetShift;
            const uint32_t global_descriptor_index = encoded_set_index & kInstDescriptorIndexingIndexMask;

            const auto descriptor_set_state = descriptor_sets[set_num];
            auto [binding_num, desc_index] = descriptor_set_state->GetBindingAndIndex(global_descriptor_index);

            const auto* binding_state = descriptor_set_state->GetBinding(binding_num);

            strm << "(set = " << set_num << ", binding = " << binding_num << ", index " << desc_index << ") ";

            const uint32_t error_sub_code = GetSubError(error_record);
            switch (error_sub_code) {
                case kErrorSubCodeDescriptorClassGeneralBufferBounds:
                case kErrorSubCodeDescriptorClassGeneralBufferCoopMatBounds: {
                    if (binding_state->descriptor_class != vvl::DescriptorClass::GeneralBuffer) {
                        assert(false);
                        return false;
                    }
                    const vvl::Buffer* buffer_state =
                        static_cast<const vvl::BufferBinding*>(binding_state)->descriptors[desc_index].GetBufferState();
                    if (buffer_state) {
                        const uint32_t byte_offset = error_record[kInstDescriptorIndexingParamOffset_0];
                        const uint32_t resource_size = error_record[kInstDescriptorIndexingParamOffset_1];
                        strm << " access out of bounds. The descriptor buffer (" << gpuav.FormatHandle(buffer_state->Handle())
                             << ") size is " << buffer_state->create_info.size << " bytes, " << resource_size
                             << " bytes were bound, and the highest out of bounds access was at [" << byte_offset << "] bytes";
                    } else {
                        // This will only get called when using nullDescriptor without bindless
                        strm << "Trying to access a null descriptor, but vkUpdateDescriptorSets was not called with VK_NULL_HANDLE "
                                "for "
                                "this descriptor. ";
                    }
                    if (error_sub_code == kErrorSubCodeDescriptorClassGeneralBufferCoopMatBounds) {
                        strm << "\nFor VK_KHR_cooperative_matrix this is invalid unless cooperativeMatrixRobustBufferAccess is "
                                "enabled.";
                    }

                    if (binding_state->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
                        binding_state->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
                        out_vuid_msg = uses_shader_object ? vuid.uniform_access_oob_08612 : vuid.uniform_access_oob_06935;
                    } else {
                        out_vuid_msg = uses_shader_object ? vuid.storage_access_oob_08613 : vuid.storage_access_oob_06936;
                    }
                } break;

                case kErrorSubCodeDescriptorClassTexelBufferBounds: {
                    if (binding_state->descriptor_class != vvl::DescriptorClass::TexelBuffer) {
                        assert(false);
                        return false;
                    }

                    const vvl::BufferView* buffer_view_state =
                        static_cast<const vvl::TexelBinding*>(binding_state)->descriptors[desc_index].GetBufferViewState();
                    if (buffer_view_state) {
                        const uint32_t byte_offset = error_record[kInstDescriptorIndexingParamOffset_0];
                        const uint32_t resource_size = error_record[kInstDescriptorIndexingParamOffset_1];

                        strm << " access out of bounds. The descriptor texel buffer ("
                             << gpuav.FormatHandle(buffer_view_state->Handle()) << ") size is " << resource_size
                             << " texels and the highest out of bounds access was at [" << byte_offset << "] bytes";
                    } else {
                        // This will only get called when using nullDescriptor without bindless
                        strm << "Trying to access a null descriptor, but vkUpdateDescriptorSets was not called with VK_NULL_HANDLE "
                                "for "
                                "this descriptor. ";
                    }

                    // https://gitlab.khronos.org/vulkan/vulkan/-/issues/3977
                    out_vuid_msg = "UNASSIGNED-Descriptor Texel Buffer texel out of bounds";
                } break;

                default:
                    error_found = false;
                    assert(false);  // other OOB checks are not implemented yet
            }

            out_error_msg += strm.str();
            return error_found;
        };

        return inst_error_logger;
    });

    DescriptorSetBindings& desc_set_bindings = cb.shared_resources_cache.GetOrCreate<DescriptorSetBindings>();

    desc_set_bindings.on_update_bound_descriptor_sets.emplace_back(
        [](Validator& gpuav, CommandBufferSubState& cb, DescriptorSetBindings::BindingCommand& desc_binding_cmd) {
            DescriptorChecksCbState& dc_cb_state = cb.shared_resources_cache.GetOrCreate<DescriptorChecksCbState>();
            dc_cb_state.last_bound_desc_sets_state_ssbo =
                cb.gpu_resources_manager.GetHostCoherentBufferRange(sizeof(glsl::BoundDescriptorSetsStateSSBO));
            dc_cb_state.last_bound_desc_sets_state_ssbo.Clear();
            auto desc_state_ssbo =
                static_cast<glsl::BoundDescriptorSetsStateSSBO*>(dc_cb_state.last_bound_desc_sets_state_ssbo.offset_mapped_ptr);
            desc_state_ssbo->descriptor_init_status = gpuav.shared_resources_cache.Get<DescriptorHeap>().GetDeviceAddress();

            for (size_t bound_ds_i = 0; bound_ds_i < desc_binding_cmd.bound_descriptor_sets.size(); ++bound_ds_i) {
                auto& bound_ds = desc_binding_cmd.bound_descriptor_sets[bound_ds_i];
                // Account for gaps in descriptor sets bindings
                if (!bound_ds) {
                    continue;
                }

                if (bound_ds->IsUpdateAfterBind()) {
                    continue;
                }

                desc_state_ssbo->descriptor_set_types[bound_ds_i] = SubState(*bound_ds).GetTypeAddress(gpuav);
            }

            desc_binding_cmd.descritpor_state_ssbo = dc_cb_state.last_bound_desc_sets_state_ssbo;
        });

    cb.on_instrumentation_desc_set_update_functions.emplace_back(
        [dummy_buffer_range = vko::BufferRange{}](CommandBufferSubState& cb, VkPipelineBindPoint, const Location&,
                                                  VkDescriptorBufferInfo& out_buffer_info, uint32_t& out_dst_binding) mutable {
            DescriptorChecksCbState* dc_cb_state = cb.shared_resources_cache.TryGet<DescriptorChecksCbState>();
            if (dc_cb_state) {
                out_buffer_info.buffer = dc_cb_state->last_bound_desc_sets_state_ssbo.buffer;
                out_buffer_info.offset = dc_cb_state->last_bound_desc_sets_state_ssbo.offset;
                out_buffer_info.range = dc_cb_state->last_bound_desc_sets_state_ssbo.size;
            } else {
                // Eventually, no descriptor set was bound in command buffer.
                // Instrumenation descriptor set is already defined at this point and needs a binding,
                // so just provide a dummy buffer
                if (dummy_buffer_range.buffer == VK_NULL_HANDLE) {
                    dummy_buffer_range = cb.gpu_resources_manager.GetDeviceLocalBufferRange(64);
                }
                out_buffer_info.buffer = dummy_buffer_range.buffer;
                out_buffer_info.offset = dummy_buffer_range.offset;
                out_buffer_info.range = dummy_buffer_range.size;
            }

            out_dst_binding = glsl::kBindingInstDescriptorIndexingOOB;
        });

    // For every descriptor binding command, update a GPU buffer holding the type of each bound descriptor set
    cb.on_pre_cb_submission_functions.emplace_back([](Validator& gpuav, CommandBufferSubState& cb, VkCommandBuffer) {
        DescriptorSetBindings& desc_set_bindings = cb.shared_resources_cache.Get<DescriptorSetBindings>();
        for (DescriptorSetBindings::BindingCommand& desc_binding_cmd : desc_set_bindings.descriptor_set_binding_commands) {
            auto desc_state_ssbo_ptr =
                static_cast<glsl::BoundDescriptorSetsStateSSBO*>(desc_binding_cmd.descritpor_state_ssbo.offset_mapped_ptr);
            for (size_t bound_ds_i = 0; bound_ds_i < desc_binding_cmd.bound_descriptor_sets.size(); ++bound_ds_i) {
                auto& bound_ds = desc_binding_cmd.bound_descriptor_sets[bound_ds_i];
                // Account for gaps in descriptor sets bindings
                if (!bound_ds) {
                    continue;
                }
                DescriptorSetSubState& desc_set_state = SubState(*bound_ds);
                desc_state_ssbo_ptr->descriptor_set_types[bound_ds_i] = desc_set_state.GetTypeAddress(gpuav);
            }
        }
    });
}

}  // namespace gpuav
