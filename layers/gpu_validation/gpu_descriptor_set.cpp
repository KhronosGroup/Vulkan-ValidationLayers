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

using vvl::DescriptorClass;

namespace gpuav {
namespace glsl {

struct BindingLayout {
    uint32_t count;
    uint32_t state_start;
};

struct DescriptorState {
    DescriptorState() : id(0), extra_data(0) {}
    DescriptorState(vvl::DescriptorClass dc, uint32_t id_, uint32_t extra_data_ = 1)
        : id(ClassToShaderBits(dc) | id_), extra_data(extra_data_) {}
    uint32_t id;
    uint32_t extra_data;

    static uint32_t ClassToShaderBits(DescriptorClass dc) {
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

}  // namespace glsl
}  // namespace gpuav

// Returns the number of bytes to hold 32 bit aligned array of bits.
static uint32_t BitBufferSize(uint32_t num_bits) {
    static constexpr uint32_t kBitsPerWord = 32;
    return (((num_bits + (kBitsPerWord - 1)) & ~(kBitsPerWord - 1))/kBitsPerWord) * sizeof(uint32_t);
}

gpuav::DescriptorSet::DescriptorSet(const VkDescriptorSet set, vvl::DescriptorPool *pool,
                                    const std::shared_ptr<vvl::DescriptorSetLayout const> &layout, uint32_t variable_count,
                                    ValidationStateTracker *state_data)
    : vvl::DescriptorSet(set, pool, layout, variable_count, state_data) {}

gpuav::DescriptorSet::~DescriptorSet() {
    Destroy();
    Validator *gv_dev = static_cast<Validator *>(state_data_);
    vmaDestroyBuffer(gv_dev->vmaAllocator, layout_.buffer, layout_.allocation);
}

VkDeviceAddress gpuav::DescriptorSet::GetLayoutState() {
    auto guard = Lock();
    if (layout_.device_addr != 0) {
        return layout_.device_addr;
    }
    uint32_t num_bindings = (GetBindingCount() > 0) ? GetLayout()->GetMaxBinding() + 1 : 0;
    Validator *gv_dev = static_cast<Validator *>(state_data_);
    VkBufferCreateInfo buffer_info = vku::InitStruct<VkBufferCreateInfo>();
    // 1 uvec2 to store num_bindings and 1 for each binding's data
    buffer_info.size = (1 + num_bindings) * sizeof(glsl::BindingLayout);
    buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    VkResult result =
        vmaCreateBuffer(gv_dev->vmaAllocator, &buffer_info, &alloc_info, &layout_.buffer, &layout_.allocation, nullptr);
    if (result != VK_SUCCESS) {
        return 0;
    }
    glsl::BindingLayout *layout_data;
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

namespace gpuav {
static glsl::DescriptorState GetInData(const vvl::BufferDescriptor &desc) {
    auto buffer_state = static_cast<const Buffer *>(desc.GetBufferState());
    if (!buffer_state) {
        return glsl::DescriptorState(DescriptorClass::GeneralBuffer, glsl::kDebugInputBindlessSkipId, vvl::kU32Max);
    }
    return glsl::DescriptorState(DescriptorClass::GeneralBuffer, buffer_state->id,
                                 static_cast<uint32_t>(buffer_state->createInfo.size));
}

static glsl::DescriptorState GetInData(const vvl::TexelDescriptor &desc) {
    auto buffer_view_state = static_cast<const BufferView *>(desc.GetBufferViewState());
    if (!buffer_view_state) {
        return glsl::DescriptorState(DescriptorClass::TexelBuffer, glsl::kDebugInputBindlessSkipId, vvl::kU32Max);
    }
    auto view_size = buffer_view_state->Size();
    uint32_t res_size = static_cast<uint32_t>(view_size / vkuFormatElementSize(buffer_view_state->create_info.format));
    return glsl::DescriptorState(DescriptorClass::TexelBuffer, buffer_view_state->id, res_size);
}

static glsl::DescriptorState GetInData(const vvl::ImageDescriptor &desc) {
    auto image_state = static_cast<const ImageView *>(desc.GetImageViewState());
    return glsl::DescriptorState(DescriptorClass::Image, image_state ? image_state->id : glsl::kDebugInputBindlessSkipId);
}

static glsl::DescriptorState GetInData(const vvl::SamplerDescriptor &desc) {
    auto sampler_state = static_cast<const Sampler *>(desc.GetSamplerState());
    return glsl::DescriptorState(DescriptorClass::PlainSampler, sampler_state->id);
}

static glsl::DescriptorState GetInData(const vvl::ImageSamplerDescriptor &desc) {
    auto image_state = static_cast<const ImageView *>(desc.GetImageViewState());
    auto sampler_state = static_cast<const Sampler *>(desc.GetSamplerState());
    return glsl::DescriptorState(DescriptorClass::ImageSampler, image_state ? image_state->id : glsl::kDebugInputBindlessSkipId,
                                 sampler_state ? sampler_state->id : 0);
}

static glsl::DescriptorState GetInData(const vvl::AccelerationStructureDescriptor &ac) {
    uint32_t id;
    if (ac.is_khr()) {
        auto ac_state = static_cast<const AccelerationStructureKHR *>(ac.GetAccelerationStructureStateKHR());
        id = ac_state ? ac_state->id : glsl::kDebugInputBindlessSkipId;
    } else {
        auto ac_state = static_cast<const AccelerationStructureNV *>(ac.GetAccelerationStructureStateNV());
        id = ac_state ? ac_state->id : glsl::kDebugInputBindlessSkipId;
    }
    return glsl::DescriptorState(DescriptorClass::AccelerationStructure, id);
}

static glsl::DescriptorState GetInData(const vvl::MutableDescriptor &desc) {
    auto desc_class = desc.ActiveClass();
    switch (desc_class) {
        case DescriptorClass::GeneralBuffer: {
            auto buffer_state = std::static_pointer_cast<const Buffer>(desc.GetSharedBufferState());
            if (!buffer_state) {
                return glsl::DescriptorState(desc_class, glsl::kDebugInputBindlessSkipId, vvl::kU32Max);
            }
            return glsl::DescriptorState(desc_class, buffer_state->id, static_cast<uint32_t>(buffer_state->createInfo.size));
        }
        case DescriptorClass::TexelBuffer: {
            auto buffer_view_state = std::static_pointer_cast<const BufferView>(desc.GetSharedBufferViewState());
            if (!buffer_view_state) {
                return glsl::DescriptorState(desc_class, glsl::kDebugInputBindlessSkipId, vvl::kU32Max);
            }
            auto view_size = buffer_view_state->Size();
            uint32_t res_size = static_cast<uint32_t>(view_size / vkuFormatElementSize(buffer_view_state->create_info.format));
            return glsl::DescriptorState(desc_class, buffer_view_state->id, res_size);
        }
        case DescriptorClass::PlainSampler: {
            auto sampler_state = std::static_pointer_cast<const Sampler>(desc.GetSharedSamplerState());
            return glsl::DescriptorState(desc_class, sampler_state->id);
        }
        case DescriptorClass::ImageSampler: {
            auto image_state = std::static_pointer_cast<const ImageView>(desc.GetSharedImageViewState());
            auto sampler_state = std::static_pointer_cast<const Sampler>(desc.GetSharedSamplerState());
            // image can be null in some cases, but the sampler can't
            return glsl::DescriptorState(desc_class, image_state ? image_state->id : glsl::kDebugInputBindlessSkipId,
                                         sampler_state ? sampler_state->id : 0);
        }
        case DescriptorClass::Image: {
            auto image_state = std::static_pointer_cast<const ImageView>(desc.GetSharedImageViewState());
            return glsl::DescriptorState(desc_class, image_state ? image_state->id : glsl::kDebugInputBindlessSkipId);
        }
        case DescriptorClass::AccelerationStructure: {
            uint32_t id;
            if (desc.IsAccelerationStructureKHR()) {
                auto ac_state = static_cast<const AccelerationStructureKHR *>(desc.GetAccelerationStructureStateKHR());
                id = ac_state ? ac_state->id : glsl::kDebugInputBindlessSkipId;
            } else {
                auto ac_state = static_cast<const AccelerationStructureNV *>(desc.GetAccelerationStructureStateNV());
                id = ac_state ? ac_state->id : glsl::kDebugInputBindlessSkipId;
            }
            return glsl::DescriptorState(desc_class, id);
        }
        default:
            assert(false);
            break;
    }
    return glsl::DescriptorState(desc_class, glsl::kDebugInputBindlessSkipId, vvl::kU32Max);
}

template <typename Binding>
void FillBindingInData(const Binding &binding, glsl::DescriptorState *data, uint32_t &index) {
    for (uint32_t di = 0; di < binding.count; di++) {
        if (!binding.updated[di]) {
            data[index++] = glsl::DescriptorState();
        } else {
            data[index++] = GetInData(binding.descriptors[di]);
        }
    }
}

// Inline Uniforms are currently treated as a single descriptor. Writes to any offsets cause the whole range to be valid.
template <>
void FillBindingInData(const vvl::InlineUniformBinding &binding, glsl::DescriptorState *data, uint32_t &index) {
    data[index++] = glsl::DescriptorState(DescriptorClass::InlineUniform, glsl::kDebugInputBindlessSkipId, vvl::kU32Max);
}
}  // namespace gpuav

std::shared_ptr<gpuav::DescriptorSet::State> gpuav::DescriptorSet::GetCurrentState() {
    auto guard = Lock();
    Validator *gv_dev = static_cast<Validator *>(state_data_);
    uint32_t cur_version = current_version_.load();
    if (last_used_state_ && last_used_state_->version == cur_version) {
        return last_used_state_;
    }
    auto next_state = std::make_shared<State>();
    next_state->set = VkHandle();
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
    buffer_info.size = descriptor_count * sizeof(glsl::DescriptorState);
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
    glsl::DescriptorState *data{nullptr};
    result = vmaMapMemory(next_state->allocator, next_state->allocation, reinterpret_cast<void **>(&data));
    assert(result == VK_SUCCESS);
    uint32_t index = 0;
    for (uint32_t i = 0; i < bindings_.size(); i++) {
        const auto &binding = *bindings_[i];
        switch (binding.descriptor_class) {
            case DescriptorClass::InlineUniform:
                FillBindingInData(static_cast<const vvl::InlineUniformBinding &>(binding), data, index);
                break;
            case DescriptorClass::GeneralBuffer:
                FillBindingInData(static_cast<const vvl::BufferBinding &>(binding), data, index);
                break;
            case DescriptorClass::TexelBuffer:
                FillBindingInData(static_cast<const vvl::TexelBinding &>(binding), data, index);
                break;
            case DescriptorClass::Mutable:
                FillBindingInData(static_cast<const vvl::MutableBinding &>(binding), data, index);
                break;
            case DescriptorClass::PlainSampler:
                FillBindingInData(static_cast<const vvl::SamplerBinding &>(binding), data, index);
                break;
            case DescriptorClass::ImageSampler:
                FillBindingInData(static_cast<const vvl::ImageSamplerBinding &>(binding), data, index);
                break;
            case DescriptorClass::Image:
                FillBindingInData(static_cast<const vvl::ImageBinding &>(binding), data, index);
                break;
            case DescriptorClass::AccelerationStructure:
                FillBindingInData(static_cast<const vvl::AccelerationStructureBinding &>(binding), data, index);
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

std::shared_ptr<gpuav::DescriptorSet::State> gpuav::DescriptorSet::GetOutputState() {
    auto guard = Lock();
    Validator *gv_dev = static_cast<Validator *>(state_data_);
    uint32_t cur_version = current_version_.load();
    if (output_state_) {
        return output_state_;
    }
    auto next_state = std::make_shared<State>();
    next_state->set = VkHandle();
    next_state->version = cur_version;
    next_state->allocator = gv_dev->vmaAllocator;

    uint32_t descriptor_count = 0;  // Number of descriptors, including all array elements
    for (const auto &binding : *this) {
        // Shader instrumentation is tracking inline uniform blocks as scalers. Don't try to validate inline uniform
        // blocks
        if (binding->type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT) {
            descriptor_count++;
        } else {
            descriptor_count += binding->count;
        }
    }
    if (descriptor_count == 0) {
        // no descriptors case, return a dummy state object
        output_state_ = next_state;
        return output_state_;
    }

    VkBufferCreateInfo buffer_info = vku::InitStructHelper();
    buffer_info.size = descriptor_count * sizeof(uint32_t);
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
    uint32_t *data{};
    result = vmaMapMemory(next_state->allocator, next_state->allocation, reinterpret_cast<void **>(&data));
    assert(result == VK_SUCCESS);
    memset(data, 0, static_cast<size_t>(buffer_info.size));

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

    output_state_ = next_state;
    return next_state;
}

std::map<uint32_t, std::vector<uint32_t>> gpuav::DescriptorSet::State::UsedDescriptors(const gpuav::DescriptorSet &set) const {
    std::map<uint32_t, std::vector<uint32_t>> used_descs;
    if (!allocation) {
        return used_descs;
    }

    glsl::BindingLayout *layout_data;
    [[maybe_unused]] auto result = vmaMapMemory(allocator, set.layout_.allocation, reinterpret_cast<void **>(&layout_data));

    uint32_t *data{nullptr};
    result = vmaMapMemory(allocator, allocation, reinterpret_cast<void **>(&data));
    result = vmaInvalidateAllocation(allocator, allocation, 0, VK_WHOLE_SIZE);

    uint32_t max_binding = layout_data[0].count;
    for (uint32_t binding = 0; binding < max_binding; binding++) {
        uint32_t count = layout_data[binding + 1].count;
        uint32_t start = layout_data[binding + 1].state_start;
        for (uint32_t i = 0; i < count; i++) {
            uint32_t pos = start + i;
            if (data[pos]) {
                auto map_result = used_descs.emplace(binding, std::vector<uint32_t>());
                map_result.first->second.emplace_back(i);
            }
        }
    }

    vmaUnmapMemory(allocator, allocation);
    vmaUnmapMemory(allocator, set.layout_.allocation);
    return used_descs;
}

gpuav::DescriptorSet::State::~State() { vmaDestroyBuffer(allocator, buffer, allocation); }

void gpuav::DescriptorSet::PerformPushDescriptorsUpdate(uint32_t write_count, const VkWriteDescriptorSet *write_descs) {
    vvl::DescriptorSet::PerformPushDescriptorsUpdate(write_count, write_descs);
    current_version_++;
}

void gpuav::DescriptorSet::PerformWriteUpdate(const VkWriteDescriptorSet &write_desc) {
    vvl::DescriptorSet::PerformWriteUpdate(write_desc);
    current_version_++;
}

void gpuav::DescriptorSet::PerformCopyUpdate(const VkCopyDescriptorSet &copy_desc, const vvl::DescriptorSet &src_set) {
    vvl::DescriptorSet::PerformCopyUpdate(copy_desc, src_set);
    current_version_++;
}

gpuav::DescriptorHeap::DescriptorHeap(gpuav::Validator &gpu_dev, uint32_t max_descriptors)
    : max_descriptors_(max_descriptors), allocator_(gpu_dev.vmaAllocator) {
    // If max_descriptors_ is 0, GPU-AV aborted during vkCreateDevice(). We still need to
    // support calls into this class as no-ops if this happens.
    if (max_descriptors_ == 0) {
        return;
    }

    VkBufferCreateInfo buffer_info = vku::InitStruct<VkBufferCreateInfo>();
    buffer_info.size = BitBufferSize(max_descriptors_ + 1); // add extra entry since 0 is the invalid id.
    buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR;

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    [[maybe_unused]] VkResult result;
    result = vmaCreateBuffer(allocator_, &buffer_info, &alloc_info, &buffer_, &allocation_, nullptr);
    assert(result == VK_SUCCESS);

    result = vmaMapMemory(allocator_, allocation_, reinterpret_cast<void **>(&gpu_heap_state_));
    assert(result == VK_SUCCESS);
    memset(gpu_heap_state_, 0, static_cast<size_t>(buffer_info.size));

    auto buffer_device_address_info = vku::InitStruct<VkBufferDeviceAddressInfo>();
    buffer_device_address_info.buffer = buffer_;
    // We cannot rely on device_extensions here, since we may be enabling BDA support even
    // though the application has not requested it.
    if (gpu_dev.api_version >= VK_API_VERSION_1_2) {
        device_address_ = DispatchGetBufferDeviceAddress(gpu_dev.device, &buffer_device_address_info);
    } else {
        device_address_ = DispatchGetBufferDeviceAddressKHR(gpu_dev.device, &buffer_device_address_info);
    }
    assert(device_address_ != 0);
}

gpuav::DescriptorHeap::~DescriptorHeap() {
    if (max_descriptors_ > 0) {
        vmaUnmapMemory(allocator_, allocation_);
        gpu_heap_state_ = nullptr;
        vmaDestroyBuffer(allocator_, buffer_, allocation_);
    }
}

gpuav::DescriptorId gpuav::DescriptorHeap::NextId(const VulkanTypedHandle &handle) {
    if (max_descriptors_ == 0) {
        return 0;
    }
    DescriptorId result;

    // NOTE: valid ids are in the range [1, max_descriptors_] (inclusive)
    // 0 is the invalid id.
    auto guard = Lock();
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
    gpu_heap_state_[result/32] |= 1u << (result & 31);
    return result;
}

void gpuav::DescriptorHeap::DeleteId(gpuav::DescriptorId id) {
    if (max_descriptors_ > 0) {
        auto guard = Lock();
        // Note: We don't mess with next_id_ here because ids should be signed in LRU order.
        gpu_heap_state_[id/32] &= ~(1u << (id & 31));
        alloc_map_.erase(id);
    }
}
