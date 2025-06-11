/* Copyright (c) 2023-2025 The Khronos Group Inc.
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
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

#include "gpuav/descriptor_validation/gpuav_descriptor_set.h"
#include <mutex>

#include "gpuav/core/gpuav.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/resources/gpuav_shader_resources.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "state_tracker/descriptor_sets.h"
#include "containers/limits.h"

#include "profiling/profiling.h"

using vvl::DescriptorClass;

namespace gpuav {

DescriptorSetSubState::DescriptorSetSubState(const vvl::DescriptorSet &set, Validator &state_data)
    : vvl::DescriptorSetSubState(set), input_buffer_(state_data) {
    BuildBindingLayouts();
}

DescriptorSetSubState::~DescriptorSetSubState() { input_buffer_.Destroy(); }

void DescriptorSetSubState::BuildBindingLayouts() {
    const uint32_t binding_count = (base.GetBindingCount() > 0) ? base.GetLayout()->GetMaxBinding() + 1 : 0;

    binding_layouts_.resize(binding_count);
    uint32_t start = 0;
    for (const auto &binding : base) {
        if (binding->type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
            binding_layouts_[binding->binding] = {start, 1};
            start++;
        } else {
            binding_layouts_[binding->binding] = {start, binding->count};
            start += binding->count;
        }
    }
}

template <typename StateObject>
DescriptorId GetId(const StateObject *obj, bool allow_null = true) {
    if (!obj) {
        return allow_null ? glsl::kNullDescriptor : 0;
    }
    auto &sub_state = SubState(*obj);
    return sub_state.Id();
}

static glsl::DescriptorState GetInData(const vvl::BufferDescriptor &desc) {
    return glsl::DescriptorState(DescriptorClass::GeneralBuffer, GetId(desc.GetBufferState()),
                                 static_cast<uint32_t>(desc.GetEffectiveRange()));
}

static glsl::DescriptorState GetInData(const vvl::TexelDescriptor &desc) {
    auto *buffer_view_state = desc.GetBufferViewState();
    uint32_t res_size = vvl::kU32Max;
    if (buffer_view_state) {
        auto view_size = buffer_view_state->Size();
        res_size = static_cast<uint32_t>(view_size / GetTexelBufferFormatSize(buffer_view_state->create_info.format));
    }
    return glsl::DescriptorState(DescriptorClass::TexelBuffer, GetId(buffer_view_state), res_size);
}

static glsl::DescriptorState GetInData(const vvl::ImageDescriptor &desc) {
    return glsl::DescriptorState(DescriptorClass::Image, GetId(desc.GetImageViewState()));
}

static glsl::DescriptorState GetInData(const vvl::SamplerDescriptor &desc) {
    return glsl::DescriptorState(DescriptorClass::PlainSampler, GetId(desc.GetSamplerState()));
}

static glsl::DescriptorState GetInData(const vvl::ImageSamplerDescriptor &desc) {
    // image can be null in some cases, but the sampler can't
    return glsl::DescriptorState(DescriptorClass::ImageSampler, GetId(desc.GetImageViewState()),
                                 GetId(desc.GetSamplerState(), false));
}

static glsl::DescriptorState GetInData(const vvl::AccelerationStructureDescriptor &ac) {
    uint32_t id = ac.IsKHR() ? GetId(ac.GetAccelerationStructureStateKHR()) : GetId(ac.GetAccelerationStructureStateNV());
    return glsl::DescriptorState(DescriptorClass::AccelerationStructure, id);
}

static glsl::DescriptorState GetInData(const vvl::MutableDescriptor &desc) {
    auto desc_class = desc.ActiveClass();
    switch (desc_class) {
        case DescriptorClass::GeneralBuffer: {
            auto buffer_state = desc.GetSharedBufferState();
            return glsl::DescriptorState(desc_class, GetId(buffer_state.get()),
                                         buffer_state ? static_cast<uint32_t>(buffer_state->create_info.size) : vvl::kU32Max);
        }
        case DescriptorClass::TexelBuffer: {
            auto buffer_view_state = desc.GetSharedBufferViewState();
            uint32_t res_size = vvl::kU32Max;
            if (buffer_view_state) {
                auto view_size = buffer_view_state->Size();
                res_size = static_cast<uint32_t>(view_size / GetTexelBufferFormatSize(buffer_view_state->create_info.format));
            }
            return glsl::DescriptorState(desc_class, GetId(buffer_view_state.get()), res_size);
        }
        case DescriptorClass::PlainSampler: {
            return glsl::DescriptorState(desc_class, GetId(desc.GetSharedSamplerState().get()));
        }
        case DescriptorClass::ImageSampler: {
            // image can be null in some cases, but the sampler can't
            return glsl::DescriptorState(desc_class, GetId(desc.GetSharedImageViewState().get()),
                                         GetId(desc.GetSharedSamplerState().get(), false));
        }
        case DescriptorClass::Image: {
            return glsl::DescriptorState(DescriptorClass::Image, GetId(desc.GetSharedImageViewState().get()));
        }
        case DescriptorClass::AccelerationStructure: {
            uint32_t id =
                desc.IsKHR() ? GetId(desc.GetAccelerationStructureStateKHR()) : GetId(desc.GetAccelerationStructureStateNV());
            return glsl::DescriptorState(DescriptorClass::AccelerationStructure, id);
        }
        case DescriptorClass::InlineUniform:
        case DescriptorClass::Mutable:
        case DescriptorClass::Invalid:
            assert(false);
            break;
    }
    // If unsupported descriptor, act as if it is null and skip
    return glsl::DescriptorState(desc_class, glsl::kNullDescriptor, vvl::kU32Max);
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
    // While not techincally a "null descriptor" we want to skip it as if it is one
    data[index++] = glsl::DescriptorState(DescriptorClass::InlineUniform, glsl::kNullDescriptor, vvl::kU32Max);
}

VkDeviceAddress DescriptorSetSubState::GetTypeAddress(Validator &gpuav, const Location &loc) {
    std::lock_guard guard(state_lock_);
    const uint32_t current_version = current_version_.load();

    // Will be empty on first time getting the state
    if (input_buffer_.Address() != 0) {
        if (last_used_version_ == current_version) {
            return input_buffer_.Address();  // nothing has changed
        } else {
            // will replace (descriptor array size might have change, so need to resize buffer)
            input_buffer_.Destroy();
        }
    }

    last_used_version_ = current_version;

    if (base.GetNonInlineDescriptorCount() == 0) {
        // no descriptors case, return a dummy state object
        return input_buffer_.Address();
    }

    VkBufferCreateInfo buffer_info = vku::InitStruct<VkBufferCreateInfo>();
    buffer_info.size = base.GetNonInlineDescriptorCount() * sizeof(glsl::DescriptorState);
    buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    // The descriptor state buffer can be very large (4mb+ in some games). Allocating it as HOST_CACHED
    // and manually flushing it at the end of the state updates is faster than using HOST_COHERENT.
    VmaAllocationCreateInfo alloc_info{};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    const bool success = input_buffer_.Create(&buffer_info, &alloc_info);
    if (!success) {
        return 0;
    }

    auto data = (glsl::DescriptorState *)input_buffer_.GetMappedPtr();

    uint32_t index = 0;
    for (const auto &binding : base) {
        switch (binding->descriptor_class) {
            case DescriptorClass::InlineUniform:
                FillBindingInData(static_cast<const vvl::InlineUniformBinding &>(*binding), data, index);
                break;
            case DescriptorClass::GeneralBuffer:
                FillBindingInData(static_cast<const vvl::BufferBinding &>(*binding), data, index);
                break;
            case DescriptorClass::TexelBuffer:
                FillBindingInData(static_cast<const vvl::TexelBinding &>(*binding), data, index);
                break;
            case DescriptorClass::Mutable:
                FillBindingInData(static_cast<const vvl::MutableBinding &>(*binding), data, index);
                break;
            case DescriptorClass::PlainSampler:
                FillBindingInData(static_cast<const vvl::SamplerBinding &>(*binding), data, index);
                break;
            case DescriptorClass::ImageSampler:
                FillBindingInData(static_cast<const vvl::ImageSamplerBinding &>(*binding), data, index);
                break;
            case DescriptorClass::Image:
                FillBindingInData(static_cast<const vvl::ImageBinding &>(*binding), data, index);
                break;
            case DescriptorClass::AccelerationStructure:
                FillBindingInData(static_cast<const vvl::AccelerationStructureBinding &>(*binding), data, index);
                break;
            case DescriptorClass::Invalid:
                gpuav.InternalError(gpuav.device, loc, "Unknown DescriptorClass");
        }
    }

    // Flush the descriptor state buffer before unmapping so that the new state is visible to the GPU
    input_buffer_.FlushAllocation();

    return input_buffer_.Address();
}

void DescriptorSetSubState::NotifyUpdate() { current_version_++; }

}  // namespace gpuav
