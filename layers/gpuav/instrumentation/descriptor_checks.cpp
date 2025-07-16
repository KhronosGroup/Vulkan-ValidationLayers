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
#include "gpuav/resources/gpuav_shader_resources.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/shaders/root_node.h"

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
        gpuav.shared_resources_manager.GetOrCreate<DescriptorHeap>(gpuav, 0);
        return;
    }

    VkPhysicalDeviceDescriptorIndexingProperties desc_indexing_props = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&desc_indexing_props);
    DispatchGetPhysicalDeviceProperties2Helper(gpuav.api_version, gpuav.physical_device, &props2);

    uint32_t num_descs = desc_indexing_props.maxUpdateAfterBindDescriptorsInAllPools;
    if (num_descs == 0 || num_descs > glsl::kDebugInputBindlessMaxDescriptors) {
        num_descs = glsl::kDebugInputBindlessMaxDescriptors;
    }

    gpuav.shared_resources_manager.GetOrCreate<DescriptorHeap>(gpuav, num_descs);
}

void RegisterDescriptorChecksValidation(Validator& gpuav, CommandBufferSubState& cb) {
    if (!gpuav.gpuav_settings.shader_instrumentation.descriptor_checks) {
        return;
    }

    DescriptorSetBindings& desc_set_bindings = cb.shared_resources_cache.GetOrCreate<DescriptorSetBindings>();

    desc_set_bindings.on_update_bound_descriptor_sets.emplace_back(
        [](Validator& gpuav, CommandBufferSubState& cb, DescriptorSetBindings::BindingCommand& desc_binding_cmd) {
            DescriptorChecksCbState& dc_cb_state = cb.shared_resources_cache.GetOrCreate<DescriptorChecksCbState>();
            dc_cb_state.last_bound_desc_sets_state_ssbo =
                cb.gpu_resources_manager.GetHostVisibleBufferRange(sizeof(glsl::BoundDescriptorSetsStateSSBO));
            dc_cb_state.last_bound_desc_sets_state_ssbo.Clear();
            auto desc_state_ssbo =
                static_cast<glsl::BoundDescriptorSetsStateSSBO*>(dc_cb_state.last_bound_desc_sets_state_ssbo.offset_mapped_ptr);
            desc_state_ssbo->descriptor_init_status = gpuav.shared_resources_manager.Get<DescriptorHeap>().GetDeviceAddress();

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
        [dummy_buffer_range = vko::BufferRange{}](CommandBufferSubState& cb, VkPipelineBindPoint,
                                                  glsl::RootNode* root_node) mutable {
            DescriptorChecksCbState* dc_cb_state = cb.shared_resources_cache.TryGet<DescriptorChecksCbState>();
            if (dc_cb_state) {
                root_node->bound_desc_sets_state_ssbo = dc_cb_state->last_bound_desc_sets_state_ssbo.offset_address;
            } else {
                // Eventually, no descriptor set was bound in command buffer.
                // Instrumenation descriptor set is already defined at this point and needs a binding,
                // so just provide a dummy buffer
                if (dummy_buffer_range.buffer == VK_NULL_HANDLE) {
                    dummy_buffer_range = cb.gpu_resources_manager.GetDeviceLocalBufferRange(64);
                }
                // #ARNO_TODO in this case, no access should be done in shader, it should be ok to have this be NULL
                root_node->bound_desc_sets_state_ssbo = dummy_buffer_range.offset_address;
            }
            assert(root_node->bound_desc_sets_state_ssbo);
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
