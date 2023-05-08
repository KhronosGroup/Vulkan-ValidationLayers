/* Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
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

#include "gpu_validation/gv_descriptor_sets.h"
#include "gpu_validation/gpu_validation.h"

gpuav_state::DescriptorSet::DescriptorSet(const VkDescriptorSet set, DESCRIPTOR_POOL_STATE *pool,
                                          const std::shared_ptr<cvdescriptorset::DescriptorSetLayout const> &layout,
                                          uint32_t variable_count, ValidationStateTracker *state_data)
    : cvdescriptorset::DescriptorSet(set, pool, layout, variable_count, state_data) {}

std::shared_ptr<gpuav_state::DescriptorSet::State> gpuav_state::DescriptorSet::GetCurrentState() {
    auto guard = Lock();
    GpuAssisted *gv_dev = static_cast<GpuAssisted *>(state_data_);
    uint32_t cur_version = current_version_.load();
    if (last_used_state_ && last_used_state_->version == cur_version) {
        return last_used_state_;
    }
    auto next_state = std::make_shared<State>();
    next_state->version = cur_version;
    next_state->allocator = gv_dev->vmaAllocator;

    uint32_t descriptor_count = 0;  // Number of descriptors, including all array elements
    uint32_t binding_count = 0;     // Number of bindings based on the max binding number used
    if (GetBindingCount() > 0) {
        binding_count += GetLayout()->GetMaxBinding() + 1;
        for (const auto &binding : *this) {
            // Shader instrumentation is tracking inline uniform blocks as scalers. Don't try to validate inline uniform
            // blocks
            if (binding->type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT) {
                descriptor_count++;
                // LogWarning(device, "UNASSIGNED-GPU-Assisted Validation Warning",
                //            "VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT descriptors will not be validated by GPU assisted "
                //            "validation");
            } else {
                descriptor_count += binding->count;
            }
        }
    }

    // For each set, allocate an input buffer that describes the descriptor set and its update status as follows
    // Word 0 = the number of bindings in the descriptor set - note that the bindings can be sparse and this is the largest
    // binding number + 1 which we'll refer to as N. Words 1 through Word N = the number of descriptors in each binding.
    // Words N+1 through Word N+N = the index where the size and update status of each binding + index pair starts -
    // unwritten is size 0 So for descriptor set:
    //    Binding
    //       0 Array[3]
    //       1 Non Array
    //       3 Array[2]
    // offset 0 = number of bindings in the descriptor set = 4
    // 1 = number of descriptors in binding 0  = 3
    // 2 = number of descriptors in binding 1 = 1
    // 3 = number of descriptors in binding 2 = 0 (ignored)
    // 4 = number of descriptors in binding 3 = 2
    // 5 = start of init data for binding 0 = 9
    // 6 = start of init data for binding 1 = 12
    // 7 = start of init data for binding 2 = 0 (ignored)
    // 8 = start of init data for binding 3 =  13
    // 9 =  Is set 0 binding 0 index 0 written
    // 10 = Is set 0 binding 0 index 1 written
    // 11 = Is set 0 binding 0 index 2 written
    // 12 = is set 1 binding 1 index 0 written
    // 13 = is set 3 binding 3 index 0 written
    // 14 = is set 3 binding 3 index 1 written
    //
    // Once the buffer is complete, write its buffer device address into the address buffer
    VkBufferCreateInfo buffer_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_info.size = (1 + binding_count + binding_count + descriptor_count) * 4;
    buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    // The descriptor state buffer can be very large (4mb+ in some games). Allocating it as HOST_CACHED
    // and manually flushing it at the end of the state updates is faster than using HOST_COHERENT.
    VmaAllocationCreateInfo alloc_info{};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    VkResult result =
        vmaCreateBuffer(next_state->allocator, &buffer_info, &alloc_info, &next_state->buffer, &next_state->allocation, nullptr);
    if (result != VK_SUCCESS) {
        return nullptr;
    }
    uint32_t *descriptor_data_ptr;
    result = vmaMapMemory(next_state->allocator, next_state->allocation, reinterpret_cast<void **>(&descriptor_data_ptr));
    assert(result == VK_SUCCESS);
    memset(descriptor_data_ptr, 0, static_cast<size_t>(buffer_info.size));
    *descriptor_data_ptr = binding_count;
    auto num_descriptors_ptr = descriptor_data_ptr + 1;
    auto start_index_ptr = num_descriptors_ptr + binding_count;
    auto written_index = 1 + binding_count + binding_count;
    for (auto &binding : *this) {
        // Note: the start index needs to start after element 0, which is a
        // separate field in the SPIR-V structure.
        *(start_index_ptr + binding->binding) = written_index - 1;
        if (VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT == binding->type) {
            *(num_descriptors_ptr + binding->binding) = 1;
            descriptor_data_ptr[written_index++] = vvl::kU32Max;
            continue;
        }
        *(num_descriptors_ptr + binding->binding) = binding->count;

        SetBindingState(descriptor_data_ptr, written_index, binding.get());

        written_index += binding->count;
    }
    auto buffer_device_address_info = LvlInitStruct<VkBufferDeviceAddressInfo>();
    buffer_device_address_info.buffer = next_state->buffer;

    // We cannot rely on device_extensions here, since we may be enabling BDA support even
    // though the application has not requested it.
    if (gv_dev->api_version >= VK_API_VERSION_1_2) {
        next_state->device_addr = DispatchGetBufferDeviceAddress(gv_dev->device, &buffer_device_address_info);
    } else {
        next_state->device_addr = DispatchGetBufferDeviceAddressKHR(gv_dev->device, &buffer_device_address_info);
    }
    assert(next_state->device_addr != 0);

    // Flush the descriptor state buffer before unmapping so that the new state is visible to the GPU
    result = vmaFlushAllocation(next_state->allocator, next_state->allocation, 0, VK_WHOLE_SIZE);
    // No good way to handle this error, we should still try to unmap.
    assert(result == VK_SUCCESS);
    vmaUnmapMemory(next_state->allocator, next_state->allocation);

    last_used_state_ = next_state;
    return next_state;
}

gpuav_state::DescriptorSet::State::~State() { vmaDestroyBuffer(allocator, buffer, allocation); }

void gpuav_state::DescriptorSet::SetBindingState(uint32_t *data, uint32_t index,
                                                 const cvdescriptorset::DescriptorBinding *binding) {
    switch (binding->descriptor_class) {
        case cvdescriptorset::DescriptorClass::GeneralBuffer: {
            auto buffer_binding = static_cast<const cvdescriptorset::BufferBinding *>(binding);
            for (uint32_t di = 0; di < buffer_binding->count; di++) {
                const auto &desc = buffer_binding->descriptors[di];
                if (!buffer_binding->updated[di]) {
                    data[index++] = 0;
                    continue;
                }
                auto buffer = desc.GetBuffer();
                if (buffer == VK_NULL_HANDLE) {
                    data[index++] = vvl::kU32Max;
                } else {
                    auto buffer_state = desc.GetBufferState();
                    data[index++] = static_cast<uint32_t>(buffer_state->createInfo.size);
                }
            }
            break;
        }
        case cvdescriptorset::DescriptorClass::TexelBuffer: {
            auto texel_binding = static_cast<const cvdescriptorset::TexelBinding *>(binding);
            for (uint32_t di = 0; di < texel_binding->count; di++) {
                const auto &desc = texel_binding->descriptors[di];
                if (!texel_binding->updated[di]) {
                    data[index++] = 0;
                    continue;
                }
                auto buffer_view = desc.GetBufferView();
                if (buffer_view == VK_NULL_HANDLE) {
                    data[index++] = vvl::kU32Max;
                } else {
                    auto buffer_view_state = desc.GetBufferViewState();
                    data[index++] = static_cast<uint32_t>(buffer_view_state->buffer_state->createInfo.size);
                }
            }
            break;
        }
        case cvdescriptorset::DescriptorClass::Mutable: {
            auto mutable_binding = static_cast<const cvdescriptorset::MutableBinding *>(binding);
            for (uint32_t di = 0; di < mutable_binding->count; di++) {
                const auto &desc = mutable_binding->descriptors[di];
                if (!mutable_binding->updated[di]) {
                    data[index++] = 0;
                    continue;
                }
                switch (desc.ActiveType()) {
                    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                        data[index++] = static_cast<uint32_t>(desc.GetBufferSize());
                        break;
                    default:
                        data[index++] = 1;
                        break;
                }
            }
            break;
        }
        default: {
            for (uint32_t i = 0; i < binding->count; i++, index++) {
                data[index] = static_cast<uint32_t>(binding->updated[i]);
            }
            break;
        }
    }
}

void gpuav_state::DescriptorSet::PerformPushDescriptorsUpdate(uint32_t write_count, const VkWriteDescriptorSet *write_descs) {
    cvdescriptorset::DescriptorSet::PerformPushDescriptorsUpdate(write_count, write_descs);
    current_version_++;
}

void gpuav_state::DescriptorSet::PerformWriteUpdate(const VkWriteDescriptorSet &write_desc) {
    cvdescriptorset::DescriptorSet::PerformWriteUpdate(write_desc);
    current_version_++;
}

void gpuav_state::DescriptorSet::PerformCopyUpdate(const VkCopyDescriptorSet &copy_desc,
                                                   const cvdescriptorset::DescriptorSet &src_set) {
    cvdescriptorset::DescriptorSet::PerformCopyUpdate(copy_desc, src_set);
    current_version_++;
}
