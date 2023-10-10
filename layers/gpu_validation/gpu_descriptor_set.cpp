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

#include "gpu_validation/gpu_descriptor_set.h"
#include "gpu_validation/gpu_validation.h"
#include "gpu_shaders/gpu_shaders_constants.h"

using cvdescriptorset::DescriptorClass;

namespace gpuav_glsl {

struct BindingLayout {
    uint32_t count;
    uint32_t state_start;
};

struct DescriptorState {
    DescriptorState() : id(0), extra_data(0) {}
    DescriptorState(cvdescriptorset::DescriptorClass dc, uint32_t id_, uint32_t extra_data_ = 1)
        : id(ClassToShaderBits(dc) | id_), extra_data(extra_data_) {}
    uint32_t id;
    uint32_t extra_data;

    static uint32_t ClassToShaderBits(DescriptorClass dc) {
        using namespace gpuav_glsl;
        switch (dc) {
            case DescriptorClass::PlainSampler:
                return (kSamplerDesc << kDescBitShift);
            case DescriptorClass::ImageSampler:
                return (kImageSamplerDesc << kDescBitShift);
            case DescriptorClass::Image:
                return (kImageDesc << kDescBitShift);
            case DescriptorClass::TexelBuffer:
                return (kTexelDesc << kDescBitShift);
            case DescriptorClass::GeneralBuffer:
                return (kBufferDesc << kDescBitShift);
            case DescriptorClass::InlineUniform:
                return (kInlineUniformDesc << kDescBitShift);
            case DescriptorClass::AccelerationStructure:
                return (kAccelDesc << kDescBitShift);
            default:
                assert(false);
        }
        return 0;
    }
};

} // namespace gpuav_glsl


gpuav_state::DescriptorSet::DescriptorSet(const VkDescriptorSet set, DESCRIPTOR_POOL_STATE *pool,
                                          const std::shared_ptr<cvdescriptorset::DescriptorSetLayout const> &layout,
                                          uint32_t variable_count, ValidationStateTracker *state_data)
    : cvdescriptorset::DescriptorSet(set, pool, layout, variable_count, state_data) {}

gpuav_state::DescriptorSet::~DescriptorSet() {
    Destroy();
    GpuAssisted *gv_dev = static_cast<GpuAssisted *>(state_data_);
    vmaDestroyBuffer(gv_dev->vmaAllocator, layout_.buffer, layout_.allocation);
}

VkDeviceAddress gpuav_state::DescriptorSet::GetLayoutState() {
    auto guard = Lock();
    if (layout_.device_addr != 0) {
        return layout_.device_addr;
    }
    uint32_t num_bindings = (GetBindingCount() > 0) ? GetLayout()->GetMaxBinding() + 1 : 0;
    GpuAssisted *gv_dev = static_cast<GpuAssisted *>(state_data_);
    VkBufferCreateInfo buffer_info = vku::InitStruct<VkBufferCreateInfo>();
    // 1 uvec2 to store num_bindings and 1 for each binding's data
    buffer_info.size = (1 + num_bindings) * sizeof(gpuav_glsl::BindingLayout);
    buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    VkResult result =
        vmaCreateBuffer(gv_dev->vmaAllocator, &buffer_info, &alloc_info, &layout_.buffer, &layout_.allocation, nullptr);
    if (result != VK_SUCCESS) {
        return 0;
    }
    gpuav_glsl::BindingLayout *layout_data;
    result = vmaMapMemory(gv_dev->vmaAllocator, layout_.allocation, reinterpret_cast<void **>(&layout_data));
    assert(result == VK_SUCCESS);
    memset(layout_data, 0, static_cast<size_t>(buffer_info.size));

    layout_data[0].count = num_bindings;
    layout_data[0].state_start = 0;

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
    // 1 = reserved
    // 2 = number of descriptors in binding 0  = 3
    // 3 = start of init data for binding 0 = 0
    // 4 = number of descriptors in binding 1 = 1
    // 5 = start of init data for binding 1 = 4
    // 6 = number of descriptors in binding 2 = 0 (ignored)
    // 7 = start of init data for binding 2 = 0 (ignored)
    // 8 = number of descriptors in binding 3 = 2
    // 9 = start of init data for binding 3 =  5
    uint32_t state_start = 0;
    for (size_t i = 0; i < bindings_.size(); i++) {
        auto &binding = bindings_[i];
        if (VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT == binding->type) {
            layout_data[binding->binding + 1] = {1, state_start};
            state_start += 1;
        } else {
            layout_data[binding->binding + 1] = {binding->count, state_start};
            state_start += binding->count;
        }
    }
    auto buffer_device_address_info = vku::InitStruct<VkBufferDeviceAddressInfo>();
    buffer_device_address_info.buffer = layout_.buffer;

    // We cannot rely on device_extensions here, since we may be enabling BDA support even
    // though the application has not requested it.
    if (gv_dev->api_version >= VK_API_VERSION_1_2) {
        layout_.device_addr = DispatchGetBufferDeviceAddress(gv_dev->device, &buffer_device_address_info);
    } else {
        layout_.device_addr = DispatchGetBufferDeviceAddressKHR(gv_dev->device, &buffer_device_address_info);
    }
    assert(layout_.device_addr != 0);

    result = vmaFlushAllocation(gv_dev->vmaAllocator, layout_.allocation, 0, VK_WHOLE_SIZE);
    // No good way to handle this error, we should still try to unmap.
    assert(result == VK_SUCCESS);
    vmaUnmapMemory(gv_dev->vmaAllocator, layout_.allocation);

    return layout_.device_addr;
}

static gpuav_glsl::DescriptorState GetInData(const cvdescriptorset::BufferDescriptor &desc) {
    auto buffer_state = static_cast<const gpuav_state::Buffer *>(desc.GetBufferState());
    if (!buffer_state) {
        return gpuav_glsl::DescriptorState(DescriptorClass::GeneralBuffer, gpuav_glsl::kDebugInputBindlessSkipId, vvl::kU32Max);
    }
    return gpuav_glsl::DescriptorState(DescriptorClass::GeneralBuffer, buffer_state->id, static_cast<uint32_t>(buffer_state->createInfo.size));
}

static gpuav_glsl::DescriptorState GetInData(const cvdescriptorset::TexelDescriptor &desc) {
    auto buffer_view_state = static_cast<const gpuav_state::BufferView *>(desc.GetBufferViewState());
    if (!buffer_view_state) {
        return gpuav_glsl::DescriptorState(DescriptorClass::TexelBuffer, gpuav_glsl::kDebugInputBindlessSkipId, vvl::kU32Max);
    }
    auto view_size = buffer_view_state->Size();
    uint32_t res_size = static_cast<uint32_t>(view_size / vkuFormatElementSize(buffer_view_state->create_info.format));
    return gpuav_glsl::DescriptorState(DescriptorClass::TexelBuffer, buffer_view_state->id, res_size);
}

static gpuav_glsl::DescriptorState GetInData(const cvdescriptorset::ImageDescriptor &desc) {
    auto image_state = static_cast<const gpuav_state::ImageView *>(desc.GetImageViewState());
    return gpuav_glsl::DescriptorState(DescriptorClass::Image,
                                       image_state ? image_state->id : gpuav_glsl::kDebugInputBindlessSkipId);
}

static gpuav_glsl::DescriptorState GetInData(const cvdescriptorset::SamplerDescriptor &desc) {
    auto sampler_state = static_cast<const gpuav_state::Sampler *>(desc.GetSamplerState());
    return gpuav_glsl::DescriptorState(DescriptorClass::PlainSampler, sampler_state->id);
}

static gpuav_glsl::DescriptorState GetInData(const cvdescriptorset::ImageSamplerDescriptor &desc) {
    auto image_state = static_cast<const gpuav_state::ImageView *>(desc.GetImageViewState());
    auto sampler_state = static_cast<const gpuav_state::Sampler *>(desc.GetSamplerState());
    return gpuav_glsl::DescriptorState(DescriptorClass::ImageSampler,
                                       image_state ? image_state->id : gpuav_glsl::kDebugInputBindlessSkipId,
                                       sampler_state ? sampler_state->id : 0);
}

static gpuav_glsl::DescriptorState GetInData(const cvdescriptorset::AccelerationStructureDescriptor &ac) {
    uint32_t id;
    if (ac.is_khr()) {
        auto ac_state = static_cast<const gpuav_state::AccelerationStructureKHR *>(ac.GetAccelerationStructureStateKHR());
        id = ac_state ? ac_state->id : gpuav_glsl::kDebugInputBindlessSkipId;
    } else {
        auto ac_state = static_cast<const gpuav_state::AccelerationStructureNV *>(ac.GetAccelerationStructureStateNV());
        id = ac_state ? ac_state->id : gpuav_glsl::kDebugInputBindlessSkipId;
    }
    return gpuav_glsl::DescriptorState(DescriptorClass::AccelerationStructure, id);
}

static gpuav_glsl::DescriptorState GetInData(const cvdescriptorset::MutableDescriptor &desc) {

    auto desc_class = desc.ActiveClass();
    switch (desc_class) {
        case DescriptorClass::GeneralBuffer: {
            auto buffer_state = std::static_pointer_cast<const gpuav_state::Buffer>(desc.GetSharedBufferState());
            if (!buffer_state) {
                return gpuav_glsl::DescriptorState(desc_class, gpuav_glsl::kDebugInputBindlessSkipId, vvl::kU32Max);
            }
            return gpuav_glsl::DescriptorState(desc_class, buffer_state->id, static_cast<uint32_t>(buffer_state->createInfo.size));
        }
        case DescriptorClass::TexelBuffer: {
            auto buffer_view_state = std::static_pointer_cast<const gpuav_state::BufferView>(desc.GetSharedBufferViewState());
            if (!buffer_view_state) {
                return gpuav_glsl::DescriptorState(desc_class, gpuav_glsl::kDebugInputBindlessSkipId, vvl::kU32Max);
            }
            auto view_size = buffer_view_state->Size();
            uint32_t res_size = static_cast<uint32_t>(view_size / vkuFormatElementSize(buffer_view_state->create_info.format));
            return gpuav_glsl::DescriptorState(desc_class, buffer_view_state->id, res_size);
        }
        case DescriptorClass::PlainSampler: {
            auto sampler_state = std::static_pointer_cast<const gpuav_state::Sampler>(desc.GetSharedSamplerState());
            return gpuav_glsl::DescriptorState(desc_class, sampler_state->id);
        }
        case DescriptorClass::ImageSampler: {
            auto image_state = std::static_pointer_cast<const gpuav_state::ImageView>(desc.GetSharedImageViewState());
            auto sampler_state = std::static_pointer_cast<const gpuav_state::Sampler>(desc.GetSharedSamplerState());
            // image can be null in some cases, but the sampler can't
            return gpuav_glsl::DescriptorState(desc_class, image_state ? image_state->id : gpuav_glsl::kDebugInputBindlessSkipId,
                                               sampler_state ? sampler_state->id : 0);
        }
        case DescriptorClass::Image: {
            auto image_state = std::static_pointer_cast<const gpuav_state::ImageView>(desc.GetSharedImageViewState());
            return gpuav_glsl::DescriptorState(desc_class, image_state ? image_state->id : gpuav_glsl::kDebugInputBindlessSkipId);
        }
        case DescriptorClass::AccelerationStructure: {
            uint32_t id;
            if (desc.IsAccelerationStructureKHR()) {
                auto ac_state = static_cast<const gpuav_state::AccelerationStructureKHR *>(desc.GetAccelerationStructureStateKHR());
                id = ac_state ? ac_state->id : gpuav_glsl::kDebugInputBindlessSkipId;
            } else {
                auto ac_state = static_cast<const gpuav_state::AccelerationStructureNV *>(desc.GetAccelerationStructureStateNV());
                id = ac_state ? ac_state->id : gpuav_glsl::kDebugInputBindlessSkipId;
            }
            return gpuav_glsl::DescriptorState(desc_class, id);
        }
        default:
            assert(false);
            break;
    }
    return gpuav_glsl::DescriptorState(desc_class, gpuav_glsl::kDebugInputBindlessSkipId, vvl::kU32Max);
}

template <typename Binding>
void FillBindingInData(const Binding &binding, gpuav_glsl::DescriptorState *data, uint32_t &index) {
    for (uint32_t di = 0; di < binding.count; di++) {
        if (!binding.updated[di]) {
            data[index++] = gpuav_glsl::DescriptorState();
        } else {
            data[index++] = GetInData(binding.descriptors[di]);
        }
    }
}

// Inline Uniforms are currently treated as a single descriptor. Writes to any offsets cause the whole range to be valid.
template <>
void FillBindingInData(const cvdescriptorset::InlineUniformBinding &binding, gpuav_glsl::DescriptorState *data, uint32_t &index) {
    data[index++] = gpuav_glsl::DescriptorState(DescriptorClass::InlineUniform, gpuav_glsl::kDebugInputBindlessSkipId, vvl::kU32Max);
}

std::shared_ptr<gpuav_state::DescriptorSet::State> gpuav_state::DescriptorSet::GetCurrentState() {
    using namespace cvdescriptorset;
    auto guard = Lock();
    GpuAssisted *gv_dev = static_cast<GpuAssisted *>(state_data_);
    uint32_t cur_version = current_version_.load();
    if (last_used_state_ && last_used_state_->version == cur_version) {
        return last_used_state_;
    }
    auto next_state = std::make_shared<State>();
    next_state->set = GetSet();
    next_state->version = cur_version;
    next_state->allocator = gv_dev->vmaAllocator;

    uint32_t descriptor_count = 0;  // Number of descriptors, including all array elements
    if (GetBindingCount() > 0) {
        for (const auto &binding : *this) {
            // Shader instrumentation is tracking inline uniform blocks as scalars. Don't try to validate inline uniform
            // blocks
            if (binding->type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT) {
                descriptor_count++;
            } else {
                descriptor_count += binding->count;
            }
        }
    }
    if (descriptor_count == 0) {
        // no descriptors case, return a dummy state object
        last_used_state_ = next_state;
        return last_used_state_;
    }

    VkBufferCreateInfo buffer_info = vku::InitStruct<VkBufferCreateInfo>();
    buffer_info.size = descriptor_count  * sizeof(gpuav_glsl::DescriptorState);
    buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    // The descriptor state buffer can be very large (4mb+ in some games). Allocating it as HOST_CACHED
    // and manually flushing it at the end of the state updates is faster than using HOST_COHERENT.
    VmaAllocationCreateInfo alloc_info{};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    VkResult result =
        vmaCreateBuffer(next_state->allocator, &buffer_info, &alloc_info, &next_state->buffer, &next_state->allocation, nullptr);
    if (result != VK_SUCCESS) {
        return nullptr;
    }
    gpuav_glsl::DescriptorState *data{nullptr};
    result = vmaMapMemory(next_state->allocator, next_state->allocation, reinterpret_cast<void **>(&data));
    assert(result == VK_SUCCESS);
    uint32_t index = 0;
    for (uint32_t i = 0; i < bindings_.size(); i++) {
        const auto &binding = *bindings_[i];
        switch (binding.descriptor_class) {
            case DescriptorClass::InlineUniform:
                FillBindingInData(static_cast<const InlineUniformBinding &>(binding), data, index);
                break;
            case DescriptorClass::GeneralBuffer:
                FillBindingInData(static_cast<const cvdescriptorset::BufferBinding &>(binding), data, index);
                break;
            case DescriptorClass::TexelBuffer:
                FillBindingInData(static_cast<const TexelBinding &>(binding), data, index);
                break;
            case DescriptorClass::Mutable:
                FillBindingInData(static_cast<const MutableBinding &>(binding), data, index);
                break;
            case PlainSampler:
                FillBindingInData(static_cast<const SamplerBinding &>(binding), data, index);
                break;
            case ImageSampler:
                FillBindingInData(static_cast<const ImageSamplerBinding &>(binding), data, index);
                break;
            case Image:
                FillBindingInData(static_cast<const ImageBinding &>(binding), data, index);
                break;
            case AccelerationStructure:
                FillBindingInData(static_cast<const AccelerationStructureBinding &>(binding), data, index);
                break;
            default:
                assert(false);
        }
    }
    VkBufferDeviceAddressInfo buffer_device_address_info = vku::InitStructHelper();
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
