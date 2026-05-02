/* Copyright (c) 2023-2026 The Khronos Group Inc.
 * Copyright (c) 2023-2026 Valve Corporation
 * Copyright (c) 2023-2026 LunarG, Inc.
 * Copyright (c) 2025 Arm Limited.
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
#include <vulkan/vulkan_core.h>
#include <mutex>

#include "generated/gpuav_offline_spirv.h"
#include "gpuav/core/gpuav.h"
#include "gpuav/core/gpuav_validation_pipeline.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/resources/gpuav_shader_resources.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "gpuav/shaders/setup/descriptor_encoding_update.h"
#include "state_tracker/descriptor_sets.h"
#include "containers/limits.h"
#include "utils/image_utils.h"

using vvl::DescriptorClass;

namespace gpuav {

DescriptorSetSubState::DescriptorSetSubState(const vvl::DescriptorSet& set, Validator& state_data)
    : vvl::DescriptorSetSubState(set), descriptor_encodings_(state_data) {
    BuildBindingLayouts();
}

DescriptorSetSubState::~DescriptorSetSubState() { descriptor_encodings_.Destroy(); }

void DescriptorSetSubState::BuildBindingLayouts() {
    const uint32_t binding_count = (base.GetBindingCount() > 0) ? base.GetLayout()->GetMaxBinding() + 1 : 0;

    binding_layouts_.resize(binding_count);
    uint32_t start = 0;
    for (const auto& binding : base) {
        if (binding->type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
            binding_layouts_[binding->binding] = {start, 1};
            start++;
        } else {
            binding_layouts_[binding->binding] = {start, binding->count};
            start += binding->count;
        }
    }
}

void DescriptorSetSubState::CreateDescriptorEncodingBuffer() {
    VkBufferCreateInfo buffer_info = vku::InitStruct<VkBufferCreateInfo>();
    buffer_info.size = base.GetNonInlineDescriptorCount() * sizeof(glsl::DescriptorEncoding);
    buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    // The descriptor state buffer can be very large (4mb+ in some games). Allocating it as HOST_CACHED
    // and manually flushing it at the end of the state updates is faster than using HOST_COHERENT.
    VmaAllocationCreateInfo alloc_info{};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    const VkResult result = descriptor_encodings_.Create(&buffer_info, &alloc_info);
    if (result != VK_SUCCESS) {
        return;
    }
}

template <typename StateObject>
DescriptorId GetId(const StateObject* obj, bool allow_null = true) {
    if (!obj) {
        return allow_null ? glsl::kNullDescriptor : 0;
    }
    auto& sub_state = SubState(*obj);
    return sub_state.Id();
}

static glsl::DescriptorEncoding GetDescriptorEncoding(const vvl::BufferDescriptor& desc) {
    return glsl::DescriptorEncoding(DescriptorClass::GeneralBuffer, GetId(desc.GetBufferState()),
                                    static_cast<uint32_t>(desc.GetEffectiveRange()));
}

static glsl::DescriptorEncoding GetDescriptorEncoding(const vvl::TexelDescriptor& desc) {
    auto* buffer_view_state = desc.GetBufferViewState();
    uint32_t res_size = vvl::kNoIndex32;
    if (buffer_view_state) {
        auto view_size = buffer_view_state->Size();
        res_size = static_cast<uint32_t>(view_size / GetTexelBufferFormatSize(buffer_view_state->create_info.format));
    }
    return glsl::DescriptorEncoding(DescriptorClass::TexelBuffer, GetId(buffer_view_state), res_size);
}

static glsl::DescriptorEncoding GetDescriptorEncoding(const vvl::ImageDescriptor& desc) {
    return glsl::DescriptorEncoding(DescriptorClass::Image, GetId(desc.GetImageViewState()));
}

static glsl::DescriptorEncoding GetDescriptorEncoding(const vvl::TensorDescriptor& desc) {
    auto tensor_view_state = static_cast<const vvl::TensorView*>(desc.GetTensorViewState());
    return glsl::DescriptorEncoding(DescriptorClass::Tensor,
                                    tensor_view_state ? tensor_view_state->GetId() : glsl::kNullDescriptor);
}

static glsl::DescriptorEncoding GetDescriptorEncoding(const vvl::SamplerDescriptor& desc) {
    return glsl::DescriptorEncoding(DescriptorClass::PlainSampler, GetId(desc.GetSamplerState()));
}

static glsl::DescriptorEncoding GetDescriptorEncoding(const vvl::ImageSamplerDescriptor& desc) {
    // image can be null in some cases, but the sampler can't
    return glsl::DescriptorEncoding(DescriptorClass::ImageSampler, GetId(desc.GetImageViewState()),
                                    GetId(desc.GetSamplerState(), false));
}

static glsl::DescriptorEncoding GetDescriptorEncoding(const vvl::AccelerationStructureDescriptor& ac) {
    uint32_t id = ac.IsKHR() ? GetId(ac.GetAccelerationStructureStateKHR()) : GetId(ac.GetAccelerationStructureStateNV());
    glsl::DescriptorEncoding encoding(DescriptorClass::AccelerationStructure, id);
    encoding.dwords_1_2 = SubState(*ac.GetAccelerationStructureStateKHR()).gpu_state.offset_address;
    return encoding;
}

static glsl::DescriptorEncoding GetDescriptorEncoding(const vvl::MutableDescriptor& desc) {
    auto desc_class = desc.ActiveClass();
    switch (desc_class) {
        case DescriptorClass::GeneralBuffer: {
            auto buffer_state = desc.GetSharedBufferState();
            return glsl::DescriptorEncoding(desc_class, GetId(buffer_state.get()),
                                            buffer_state ? static_cast<uint32_t>(buffer_state->GetSize()) : vvl::kNoIndex32);
        }
        case DescriptorClass::TexelBuffer: {
            auto buffer_view_state = desc.GetSharedBufferViewState();
            uint32_t res_size = vvl::kNoIndex32;
            if (buffer_view_state) {
                auto view_size = buffer_view_state->Size();
                res_size = static_cast<uint32_t>(view_size / GetTexelBufferFormatSize(buffer_view_state->create_info.format));
            }
            return glsl::DescriptorEncoding(desc_class, GetId(buffer_view_state.get()), res_size);
        }
        case DescriptorClass::PlainSampler: {
            return glsl::DescriptorEncoding(desc_class, GetId(desc.GetSharedSamplerState().get()));
        }
        case DescriptorClass::ImageSampler: {
            // image can be null in some cases, but the sampler can't
            return glsl::DescriptorEncoding(desc_class, GetId(desc.GetSharedImageViewState().get()),
                                            GetId(desc.GetSharedSamplerState().get(), false));
        }
        case DescriptorClass::Image: {
            return glsl::DescriptorEncoding(DescriptorClass::Image, GetId(desc.GetSharedImageViewState().get()));
        }
        case DescriptorClass::Tensor: {
            auto tensor_state = std::static_pointer_cast<const vvl::Tensor>(desc.GetSharedTensor());
            return glsl::DescriptorEncoding(desc_class, tensor_state ? tensor_state->GetId() : glsl::kNullDescriptor);
        }
        case DescriptorClass::AccelerationStructure: {
            uint32_t id =
                desc.IsKHR() ? GetId(desc.GetAccelerationStructureStateKHR()) : GetId(desc.GetAccelerationStructureStateNV());
            return glsl::DescriptorEncoding(DescriptorClass::AccelerationStructure, id);
        }
        case DescriptorClass::InlineUniform:
        case DescriptorClass::Mutable:
        case DescriptorClass::Invalid:
            assert(false);
            break;
    }
    // If unsupported descriptor, act as if it is null and skip
    return glsl::DescriptorEncoding(desc_class, glsl::kNullDescriptor, vvl::kNoIndex32);
}

template <typename Binding>
void GetBindingEncodings(const Binding& binding, glsl::DescriptorEncoding* descriptor_encodings, uint32_t& index) {
    for (uint32_t di = 0; di < binding.count; di++) {
        if (!binding.updated[di]) {
            descriptor_encodings[index++] = glsl::DescriptorEncoding();
        } else {
            descriptor_encodings[index++] = GetDescriptorEncoding(binding.descriptors[di]);
        }
    }
}

// Inline Uniforms are currently treated as a single descriptor. Writes to any offsets cause the whole range to be valid.
template <>
void GetBindingEncodings(const vvl::InlineUniformBinding& binding, glsl::DescriptorEncoding* descriptor_encodings,
                         uint32_t& index) {
    // While not techincally a "null descriptor" we want to skip it as if it is one
    descriptor_encodings[index++] =
        glsl::DescriptorEncoding(DescriptorClass::InlineUniform, glsl::kNullDescriptor, vvl::kNoIndex32);
}

static void GetBindingEncodingsHelper(const vvl::DescriptorBinding* binding, glsl::DescriptorEncoding* descriptor_encodings,
                                      uint32_t& index) {
    switch (binding->descriptor_class) {
        case DescriptorClass::InlineUniform:
            GetBindingEncodings(static_cast<const vvl::InlineUniformBinding&>(*binding), descriptor_encodings, index);
            break;
        case DescriptorClass::GeneralBuffer:
            GetBindingEncodings(static_cast<const vvl::BufferBinding&>(*binding), descriptor_encodings, index);
            break;
        case DescriptorClass::TexelBuffer:
            GetBindingEncodings(static_cast<const vvl::TexelBinding&>(*binding), descriptor_encodings, index);
            break;
        case DescriptorClass::Mutable:
            GetBindingEncodings(static_cast<const vvl::MutableBinding&>(*binding), descriptor_encodings, index);
            break;
        case DescriptorClass::PlainSampler:
            GetBindingEncodings(static_cast<const vvl::SamplerBinding&>(*binding), descriptor_encodings, index);
            break;
        case DescriptorClass::ImageSampler:
            GetBindingEncodings(static_cast<const vvl::ImageSamplerBinding&>(*binding), descriptor_encodings, index);
            break;
        case DescriptorClass::Image:
            GetBindingEncodings(static_cast<const vvl::ImageBinding&>(*binding), descriptor_encodings, index);
            break;
        case DescriptorClass::AccelerationStructure:
            GetBindingEncodings(static_cast<const vvl::AccelerationStructureBinding&>(*binding), descriptor_encodings, index);
            break;
        case DescriptorClass::Tensor:
            GetBindingEncodings(static_cast<const vvl::TensorBinding&>(*binding), descriptor_encodings, index);
            break;
        case DescriptorClass::Invalid:
            assert(false);
            break;
    }
}

static std::tuple<uint32_t, uint32_t, vko::BufferRange> GetWriteDescriptorSetEncodings(
    gpuav::CommandBufferSubState& cb_sub_state, const vvl::DescriptorSet& set, const VkWriteDescriptorSet& write_desc,
    const std::vector<spirv::BindingLayout>& binding_layouts) {
    const auto* binding = set.GetBinding(write_desc.dstBinding);
    if (!binding) {
        return {0, 0, vko::BufferRange{}};
    }

    vko::BufferRange desc_set_encodings =
        cb_sub_state.gpu_resources_manager.GetHostCoherentBufferRange(binding->count * sizeof(glsl::DescriptorEncoding));

    auto desc_set_encodings_ptr = (glsl::DescriptorEncoding*)desc_set_encodings.offset_mapped_ptr;
    uint32_t index = 0;
    GetBindingEncodingsHelper(binding, desc_set_encodings_ptr, index);

    return {binding_layouts[write_desc.dstBinding].start, binding->count, desc_set_encodings};
}

struct DescriptorEncodingUpdateShader {
    static size_t GetSpirvSize() { return setup_descriptor_encoding_update_comp_size * sizeof(uint32_t); }
    static const uint32_t* GetSpirv() { return setup_descriptor_encoding_update_comp; }

    glsl::DescriptorEncodingUpdateShaderPushData push_constants{};

    static std::vector<VkDescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings() { return {}; }

    std::vector<VkWriteDescriptorSet> GetDescriptorWrites() const { return {}; }
};

void DescriptorSetSubState::PerformPushDescriptorsUpdate(vvl::CommandBuffer& cb, uint32_t write_count,
                                                         const VkWriteDescriptorSet* write_descs) {
    std::lock_guard guard(state_lock_);

    gpuav::CommandBufferSubState& cb_sub_state = gpuav::SubState(cb);

    valpipe::ComputePipeline<DescriptorEncodingUpdateShader>& descriptor_encoding_update_pipeline =
        cb_sub_state.gpuav_.shared_resources_cache.GetOrCreate<valpipe::ComputePipeline<DescriptorEncodingUpdateShader>>(
            cb_sub_state.gpuav_, Location(vvl::Func::Empty));

    if (!descriptor_encoding_update_pipeline.valid) {
        return;
    }

    valpipe::RestorablePipelineState restorable_state(cb_sub_state, VK_PIPELINE_BIND_POINT_COMPUTE);

    DispatchCmdBindPipeline(cb.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, descriptor_encoding_update_pipeline.pipeline);

    for (const VkWriteDescriptorSet& write_desc : vvl::make_span(write_descs, write_count)) {
        const auto [start_binding, binding_count, desc_set_encodings] =
            GetWriteDescriptorSetEncodings(cb_sub_state, base, write_desc, binding_layouts_);
        if (binding_count == 0) {
            continue;
        }

        DescriptorEncodingUpdateShader shader_resources;
        // Create buffer just in time
        if (descriptor_encodings_.IsDestroyed()) {
            CreateDescriptorEncodingBuffer();
        }
        assert(descriptor_encodings_.Address());
        shader_resources.push_constants.cb_desc_encodings_ptr = descriptor_encodings_.Address();
        shader_resources.push_constants.staged_desc_encodings_ptr = desc_set_encodings.offset_address;
        shader_resources.push_constants.start_binding = start_binding;

        if (!descriptor_encoding_update_pipeline.BindShaderResources(cb_sub_state.gpuav_, cb_sub_state, shader_resources)) {
            return;
        }

        {
            VkBufferMemoryBarrier barrier_write_after_read = vku::InitStructHelper();
            barrier_write_after_read.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier_write_after_read.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            barrier_write_after_read.buffer = descriptor_encodings_.VkHandle();
            barrier_write_after_read.offset = 0;
            barrier_write_after_read.size = VK_WHOLE_SIZE;

            DispatchCmdPipelineBarrier(cb.VkHandle(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 1, &barrier_write_after_read, 0,
                                       nullptr);
        }

        DispatchCmdDispatch(cb.VkHandle(), binding_count, 1, 1);

        {
            VkBufferMemoryBarrier barrier_read_after_write = vku::InitStructHelper();
            barrier_read_after_write.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            barrier_read_after_write.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier_read_after_write.buffer = descriptor_encodings_.VkHandle();
            barrier_read_after_write.offset = 0;
            barrier_read_after_write.size = VK_WHOLE_SIZE;

            DispatchCmdPipelineBarrier(cb.VkHandle(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 0, nullptr, 1,
                                       &barrier_read_after_write, 0, nullptr);
        }
    }
}

VkDeviceAddress DescriptorSetSubState::GetDescriptorEncodingsAddress(Validator& gpuav) {
    std::lock_guard guard(state_lock_);
    const uint32_t current_version = current_version_.load();

    if (base.GetNonInlineDescriptorCount() == 0) {
        // no descriptors
        return 0;
    }

    // Create buffer just in time
    if (descriptor_encodings_.IsDestroyed()) {
        CreateDescriptorEncodingBuffer();
    } else if (last_used_version_ == current_version) {
        // nothing has changed
        return descriptor_encodings_.Address();
    }

    auto desc_set_encodings_ptr = (glsl::DescriptorEncoding*)descriptor_encodings_.GetMappedPtr();

    uint32_t index = 0;
    for (const auto& binding : base) {
        GetBindingEncodingsHelper(binding.get(), desc_set_encodings_ptr, index);
    }

    // Flush the descriptor encodings before unmapping so that they are visible to the GPU
    descriptor_encodings_.FlushAllocation();

    last_used_version_ = current_version;

    return descriptor_encodings_.Address();
}

void DescriptorSetSubState::NotifyUpdate() { current_version_++; }

}  // namespace gpuav
