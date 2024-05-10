/* Copyright (c) 2018-2024 The Khronos Group Inc.
 * Copyright (c) 2018-2024 Valve Corporation
 * Copyright (c) 2018-2024 LunarG, Inc.
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

#include <cmath>
#include <fstream>
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
#include <unistd.h>
#endif
#include "utils/cast_utils.h"
#include "utils/shader_utils.h"
#include "utils/hash_util.h"
#include "gpu_validation/gpu_constants.h"
#include "gpu_validation/gpu_validation.h"
#include "gpu_validation/gpu_subclasses.h"
#include "state_tracker/device_state.h"
#include "state_tracker/shader_object_state.h"
#include "spirv-tools/instrument.hpp"
#include "spirv-tools/linker.hpp"
#include "generated/layer_chassis_dispatch.h"
#include "gpu_vuids.h"
#include "containers/custom_containers.h"
// Generated shaders
#include "gpu_shaders/gpu_error_header.h"
#include "generated/gpu_pre_draw_vert.h"
#include "generated/gpu_pre_dispatch_comp.h"
#include "generated/gpu_pre_trace_rays_rgen.h"
#include "generated/gpu_inst_shader_hash.h"

namespace gpuav {

std::shared_ptr<vvl::Buffer> Validator::CreateBufferState(VkBuffer handle, const VkBufferCreateInfo *pCreateInfo) {
    return std::make_shared<Buffer>(*this, handle, pCreateInfo, *desc_heap);
}

std::shared_ptr<vvl::BufferView> Validator::CreateBufferViewState(const std::shared_ptr<vvl::Buffer> &bf, VkBufferView bv,
                                                                  const VkBufferViewCreateInfo *ci,
                                                                  VkFormatFeatureFlags2KHR buf_ff) {
    return std::make_shared<BufferView>(bf, bv, ci, buf_ff, *desc_heap);
}

std::shared_ptr<vvl::ImageView> Validator::CreateImageViewState(const std::shared_ptr<vvl::Image> &image_state, VkImageView iv,
                                                                const VkImageViewCreateInfo *ci, VkFormatFeatureFlags2KHR ff,
                                                                const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props) {
    return std::make_shared<ImageView>(image_state, iv, ci, ff, cubic_props, *desc_heap);
}

std::shared_ptr<vvl::Sampler> Validator::CreateSamplerState(VkSampler s, const VkSamplerCreateInfo *ci) {
    return std::make_shared<Sampler>(s, ci, *desc_heap);
}

std::shared_ptr<vvl::DescriptorSet> Validator::CreateDescriptorSet(VkDescriptorSet set, vvl::DescriptorPool *pool,
                                                                   const std::shared_ptr<vvl::DescriptorSetLayout const> &layout,
                                                                   uint32_t variable_count) {
    return std::static_pointer_cast<vvl::DescriptorSet>(std::make_shared<DescriptorSet>(set, pool, layout, variable_count, this));
}

std::shared_ptr<vvl::CommandBuffer> Validator::CreateCmdBufferState(VkCommandBuffer handle,
                                                                    const VkCommandBufferAllocateInfo *pCreateInfo,
                                                                    const vvl::CommandPool *pool) {
    return std::static_pointer_cast<vvl::CommandBuffer>(std::make_shared<CommandBuffer>(*this, handle, pCreateInfo, pool));
}

std::shared_ptr<vvl::Queue> Validator::CreateQueue(VkQueue q, uint32_t index, VkDeviceQueueCreateFlags flags,
                                                   const VkQueueFamilyProperties &queueFamilyProperties) {
    return std::static_pointer_cast<vvl::Queue>(std::make_shared<Queue>(*this, q, index, flags, queueFamilyProperties));
}

// Perform initializations that can be done at Create Device time.
void Validator::CreateDevice(const VkDeviceCreateInfo *pCreateInfo, const Location &loc) {
    // Add the callback hooks for the functions that are either broadly or deeply used and that the ValidationStateTracker refactor
    // would be messier without.
    // TODO: Find a good way to do this hooklessly.
    SetSetImageViewInitialLayoutCallback(
        [](vvl::CommandBuffer *cb_state, const vvl::ImageView &iv_state, VkImageLayout layout) -> void {
            cb_state->SetImageViewInitialLayout(iv_state, layout);
        });

    // Set up a stub implementation of the descriptor heap in case we abort.
    desc_heap.emplace(*this, 0);

    validation_bindings_ = {
        // Error output buffer
        {glsl::kBindingInstErrorBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        // Current bindless buffer
        {glsl::kBindingInstBindlessDescriptor, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        // Buffer holding buffer device addresses
        {glsl::kBindingInstBufferDeviceAddress, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        // Buffer holding action command index in command buffer
        {glsl::kBindingInstActionIndex, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr},
        // Buffer holding a resource index from the per command buffer command resources list
        {glsl::kBindingInstCmdResourceIndex, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr},
        // Commands errors counts buffer
        {glsl::kBindingInstCmdErrorsCount, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };

    // TODO: Such a call is expected to be the first thing happening in this function,
    // but moving it at the top breaks GPU-AV. Try to fix it
    BaseClass::CreateDevice(pCreateInfo, loc);

    if (api_version < VK_API_VERSION_1_1) {
        ReportSetupProblem(device, loc, "GPU-Assisted validation requires Vulkan 1.1 or later.  GPU-Assisted Validation disabled.");
        aborted = true;
        return;
    }

    DispatchGetPhysicalDeviceFeatures(physical_device, &supported_features);
    if (!supported_features.fragmentStoresAndAtomics || !supported_features.vertexPipelineStoresAndAtomics) {
        ReportSetupProblem(device, loc,
                           "GPU-Assisted validation requires fragmentStoresAndAtomics and vertexPipelineStoresAndAtomics.  "
                           "GPU-Assisted Validation disabled.");
        aborted = true;
        return;
    }

    shaderInt64 = supported_features.shaderInt64;
    bda_validation_possible = ((IsExtEnabled(device_extensions.vk_ext_buffer_device_address) ||
                                IsExtEnabled(device_extensions.vk_khr_buffer_device_address)) &&
                               shaderInt64 && enabled_features.bufferDeviceAddress);
    if (!bda_validation_possible) {
        if (gpuav_settings.validate_bda) {
            if (!shaderInt64) {
                LogWarning("WARNING-GPU-Assisted-Validation", device, loc,
                           "Buffer device address validation option was enabled, but required features shaderInt64 is not enabled. "
                           "Disabling option.");
            } else {
                LogWarning("WARNING-GPU-Assisted-Validation", device, loc,
                           "Buffer device address validation option was enabled, but required buffer device address extension "
                           "and/or features are not enabled. Disabling option.");
            }
        }
        gpuav_settings.validate_bda = false;
    }

    if (gpuav_settings.validate_ray_query) {
        if (!enabled_features.rayQuery) {
            gpuav_settings.validate_ray_query = false;
            LogWarning("WARNING-GPU-Assisted-Validation", device, loc,
                       "Ray query validation option was enabled, but required feature rayQuery is not enabled. "
                       "Disabling option.");
        }
    }

    // gpu_pre_copy_buffer_to_image.comp relies on uint8_t buffers to perform validation
    if (gpuav_settings.validate_buffer_copies) {
        if (!enabled_features.uniformAndStorageBuffer8BitAccess) {
            gpuav_settings.validate_buffer_copies = false;
            LogWarning("WARNING-GPU-Assisted-Validation", device, loc,
                       "gpuav_validate_copies option was enabled, but uniformAndStorageBuffer8BitAccess feature is not available. "
                       "Disabling option.");
        }
    }

    if (IsExtEnabled(device_extensions.vk_ext_descriptor_buffer)) {
        LogWarning("WARNING-GPU-Assisted-Validation", device, loc,
                   "VK_EXT_descriptor_buffer is enabled, but GPU-AV does not currently support validation of descriptor buffers. "
                   "Use of descriptor buffers will result in no descriptor checking");
    }

    output_buffer_byte_size = glsl::kErrorBufferByteSize;

    if (gpuav_settings.validate_descriptors && !force_buffer_device_address) {
        gpuav_settings.validate_descriptors = false;
        LogWarning("WARNING-GPU-Assisted-Validation", device, loc,
                   "Buffer Device Address + feature is not available.  No descriptor checking will be attempted");
    }

    if (gpuav_settings.IsBufferValidationEnabled() && (phys_dev_props.limits.maxPushConstantsSize < 4 * sizeof(uint32_t))) {
        gpuav_settings.SetBufferValidationEnabled(false);
        LogWarning("WARNING-GPU-Assisted-Validation", device, loc,
                   "Device does not support the minimum range of push constants (32 bytes).  No indirect buffer checking will be "
                   "attempted");
    }

    if (gpuav_settings.validate_descriptors) {
        VkPhysicalDeviceDescriptorIndexingProperties desc_indexing_props = vku::InitStructHelper();
        VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&desc_indexing_props);
        DispatchGetPhysicalDeviceProperties2(physical_device, &props2);

        uint32_t num_descs = desc_indexing_props.maxUpdateAfterBindDescriptorsInAllPools;
        if (num_descs == 0 || num_descs > glsl::kDebugInputBindlessMaxDescriptors) {
            num_descs = glsl::kDebugInputBindlessMaxDescriptors;
        }

        desc_heap.emplace(*this, num_descs);
    }

    if (gpuav_settings.vma_linear_output) {
        VkBufferCreateInfo output_buffer_create_info = vku::InitStructHelper();
        output_buffer_create_info.size = output_buffer_byte_size;
        output_buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VmaAllocationCreateInfo alloc_create_info = {};
        alloc_create_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        uint32_t mem_type_index;
        VkResult result =
            vmaFindMemoryTypeIndexForBufferInfo(vmaAllocator, &output_buffer_create_info, &alloc_create_info, &mem_type_index);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, loc, "Unable to find memory type index");
            aborted = true;
            return;
        }
        VmaPoolCreateInfo pool_create_info = {};
        pool_create_info.memoryTypeIndex = mem_type_index;
        pool_create_info.blockSize = 0;
        pool_create_info.maxBlockCount = 0;
        pool_create_info.flags = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT;
        result = vmaCreatePool(vmaAllocator, &pool_create_info, &output_buffer_pool);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, loc, "Unable to create VMA memory pool");
            aborted = true;
            return;
        }
    }

    if (gpuav_settings.cache_instrumented_shaders) {
        auto tmp_path = GetTempFilePath();
        instrumented_shader_cache_path = tmp_path + "/instrumented_shader_cache";
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
        instrumented_shader_cache_path += "-" + std::to_string(getuid());
#endif
        instrumented_shader_cache_path += ".bin";

        std::ifstream file_stream(instrumented_shader_cache_path, std::ifstream::in | std::ifstream::binary);
        if (file_stream) {
            char inst_shader_hash[sizeof(INST_SHADER_GIT_HASH)];
            file_stream.read(inst_shader_hash, sizeof(inst_shader_hash));
            if (!strncmp(inst_shader_hash, INST_SHADER_GIT_HASH, sizeof(INST_SHADER_GIT_HASH))) {
                uint32_t num_shaders = 0;
                file_stream.read(reinterpret_cast<char *>(&num_shaders), sizeof(uint32_t));
                for (uint32_t i = 0; i < num_shaders; ++i) {
                    uint32_t hash;
                    uint32_t shader_length;
                    std::vector<uint32_t> shader_code;
                    file_stream.read(reinterpret_cast<char *>(&hash), sizeof(uint32_t));
                    file_stream.read(reinterpret_cast<char *>(&shader_length), sizeof(uint32_t));
                    shader_code.resize(shader_length);
                    file_stream.read(reinterpret_cast<char *>(shader_code.data()), 4 * shader_length);
                    instrumented_shaders.emplace(hash, std::make_pair(shader_length, std::move(shader_code)));
                }
            }
            file_stream.close();
        }
    }

    // Create command indices buffer
    {
        VkBufferCreateInfo buffer_info = vku::InitStructHelper();
        buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        buffer_info.size = cst::indices_count * sizeof(uint32_t);
        VmaAllocationCreateInfo alloc_info = {};
        assert(output_buffer_pool);
        alloc_info.pool = output_buffer_pool;
        alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        VkResult result =
            vmaCreateBuffer(vmaAllocator, &buffer_info, &alloc_info, &indices_buffer.buffer, &indices_buffer.allocation, nullptr);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, loc, "Unable to allocate device memory for command indices. Device could become unstable.",
                               true);
            aborted = true;
            return;
        }

        uint32_t *indices_ptr = nullptr;
        result = vmaMapMemory(vmaAllocator, indices_buffer.allocation, reinterpret_cast<void **>(&indices_ptr));
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, loc, "Unable to map device memory for command indices buffer.");
            aborted = true;
            return;
        }

        for (uint32_t i = 0; i < cst::indices_count; ++i) {
            indices_ptr[i] = i;
        }

        vmaUnmapMemory(vmaAllocator, indices_buffer.allocation);
    }
}

void PreDrawResources::SharedResources::Destroy(Validator &validator) {
    if (shader_module != VK_NULL_HANDLE) {
        DispatchDestroyShaderModule(validator.device, shader_module, nullptr);
        shader_module = VK_NULL_HANDLE;
    }
    if (ds_layout != VK_NULL_HANDLE) {
        DispatchDestroyDescriptorSetLayout(validator.device, ds_layout, nullptr);
        ds_layout = VK_NULL_HANDLE;
    }
    if (pipeline_layout != VK_NULL_HANDLE) {
        DispatchDestroyPipelineLayout(validator.device, pipeline_layout, nullptr);
        pipeline_layout = VK_NULL_HANDLE;
    }
    auto to_destroy = renderpass_to_pipeline.snapshot();
    for (auto &entry : to_destroy) {
        DispatchDestroyPipeline(validator.device, entry.second, nullptr);
        renderpass_to_pipeline.erase(entry.first);
    }
    if (shader_object != VK_NULL_HANDLE) {
        DispatchDestroyShaderEXT(validator.device, shader_object, nullptr);
        shader_object = VK_NULL_HANDLE;
    }
}

void PreDispatchResources::SharedResources::Destroy(Validator &validator) {
    if (ds_layout != VK_NULL_HANDLE) {
        DispatchDestroyDescriptorSetLayout(validator.device, ds_layout, nullptr);
        ds_layout = VK_NULL_HANDLE;
    }
    if (pipeline_layout != VK_NULL_HANDLE) {
        DispatchDestroyPipelineLayout(validator.device, pipeline_layout, nullptr);
        pipeline_layout = VK_NULL_HANDLE;
    }
    if (pipeline != VK_NULL_HANDLE) {
        DispatchDestroyPipeline(validator.device, pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;
    }
    if (shader_object != VK_NULL_HANDLE) {
        DispatchDestroyShaderEXT(validator.device, shader_object, nullptr);
        shader_object = VK_NULL_HANDLE;
    }
}

void PreTraceRaysResources::SharedResources::Destroy(Validator &validator) {
    if (ds_layout != VK_NULL_HANDLE) {
        DispatchDestroyDescriptorSetLayout(validator.device, ds_layout, nullptr);
        ds_layout = VK_NULL_HANDLE;
    }
    if (pipeline_layout != VK_NULL_HANDLE) {
        DispatchDestroyPipelineLayout(validator.device, pipeline_layout, nullptr);
        pipeline_layout = VK_NULL_HANDLE;
    }
    if (pipeline != VK_NULL_HANDLE) {
        DispatchDestroyPipeline(validator.device, pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;
    }
    if (sbt_buffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(validator.vmaAllocator, sbt_buffer, sbt_allocation);
        sbt_buffer = VK_NULL_HANDLE;
        sbt_allocation = VK_NULL_HANDLE;
        sbt_address = 0;
    }
    if (sbt_pool) {
        vmaDestroyPool(validator.vmaAllocator, sbt_pool);
        sbt_pool = VK_NULL_HANDLE;
    }
}

void PreCopyBufferToImageResources::SharedResources::Destroy(Validator &validator) {
    if (ds_layout != VK_NULL_HANDLE) {
        DispatchDestroyDescriptorSetLayout(validator.device, ds_layout, nullptr);
        ds_layout = VK_NULL_HANDLE;
    }
    if (pipeline_layout != VK_NULL_HANDLE) {
        DispatchDestroyPipelineLayout(validator.device, pipeline_layout, nullptr);
        pipeline_layout = VK_NULL_HANDLE;
    }
    if (pipeline != VK_NULL_HANDLE) {
        DispatchDestroyPipeline(validator.device, pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;
    }
    if (copy_regions_pool != VK_NULL_HANDLE) {
        vmaDestroyPool(validator.vmaAllocator, copy_regions_pool);
        copy_regions_pool = VK_NULL_HANDLE;
    }
}

void RestorablePipelineState::Create(vvl::CommandBuffer &cb_state, VkPipelineBindPoint bind_point) {
    pipeline_bind_point = bind_point;
    const auto lv_bind_point = ConvertToLvlBindPoint(bind_point);

    LastBound &last_bound = cb_state.lastBound[lv_bind_point];
    if (last_bound.pipeline_state) {
        pipeline = last_bound.pipeline_state->VkHandle();
        pipeline_layout = last_bound.pipeline_layout;
        descriptor_sets.reserve(last_bound.per_set.size());
        for (std::size_t i = 0; i < last_bound.per_set.size(); i++) {
            const auto &bound_descriptor_set = last_bound.per_set[i].bound_descriptor_set;
            if (bound_descriptor_set) {
                descriptor_sets.push_back(std::make_pair(bound_descriptor_set->VkHandle(), static_cast<uint32_t>(i)));
                if (bound_descriptor_set->IsPushDescriptor()) {
                    push_descriptor_set_index = static_cast<uint32_t>(i);
                }
                dynamic_offsets.push_back(last_bound.per_set[i].dynamicOffsets);
            }
        }

        if (last_bound.push_descriptor_set) {
            push_descriptor_set_writes = last_bound.push_descriptor_set->GetWrites();
        }
        const auto &pipeline_layout = last_bound.pipeline_state->PipelineLayoutState();
        if (pipeline_layout->push_constant_ranges == cb_state.push_constant_data_ranges) {
            push_constants_data = cb_state.push_constant_data;
            push_constants_ranges = pipeline_layout->push_constant_ranges;
        }
    } else {
        assert(shader_objects.empty());
        if (lv_bind_point == BindPoint_Graphics) {
            shader_objects = last_bound.GetAllBoundGraphicsShaders();
        } else if (lv_bind_point == BindPoint_Compute) {
            auto compute_shader = last_bound.GetShaderState(ShaderObjectStage::COMPUTE);
            if (compute_shader) {
                shader_objects.emplace_back(compute_shader);
            }
        }
    }
}

void RestorablePipelineState::Restore(VkCommandBuffer command_buffer) const {
    if (pipeline != VK_NULL_HANDLE) {
        DispatchCmdBindPipeline(command_buffer, pipeline_bind_point, pipeline);
        if (!descriptor_sets.empty()) {
            for (std::size_t i = 0; i < descriptor_sets.size(); i++) {
                VkDescriptorSet descriptor_set = descriptor_sets[i].first;
                if (descriptor_set != VK_NULL_HANDLE) {
                    DispatchCmdBindDescriptorSets(command_buffer, pipeline_bind_point, pipeline_layout, descriptor_sets[i].second,
                                                  1, &descriptor_set, static_cast<uint32_t>(dynamic_offsets[i].size()),
                                                  dynamic_offsets[i].data());
                }
            }
        }
        if (!push_descriptor_set_writes.empty()) {
            DispatchCmdPushDescriptorSetKHR(command_buffer, pipeline_bind_point, pipeline_layout, push_descriptor_set_index,
                                            static_cast<uint32_t>(push_descriptor_set_writes.size()),
                                            reinterpret_cast<const VkWriteDescriptorSet *>(push_descriptor_set_writes.data()));
        }
        if (!push_constants_data.empty()) {
            for (const auto &push_constant_range : *push_constants_ranges) {
                if (push_constant_range.size == 0) continue;
                DispatchCmdPushConstants(command_buffer, pipeline_layout, push_constant_range.stageFlags,
                                         push_constant_range.offset, push_constant_range.size, push_constants_data.data());
            }
        }
    }
    if (!shader_objects.empty()) {
        std::vector<VkShaderStageFlagBits> stages;
        std::vector<VkShaderEXT> shaders;
        for (const vvl::ShaderObject *shader_obj : shader_objects) {
            stages.emplace_back(shader_obj->create_info.stage);
            shaders.emplace_back(shader_obj->VkHandle());
        }
        DispatchCmdBindShadersEXT(command_buffer, static_cast<uint32_t>(shader_objects.size()), stages.data(), shaders.data());
    }
}

void CommandResources::Destroy(Validator &validator) {
    if (instrumentation_desc_set != VK_NULL_HANDLE) {
        validator.desc_set_manager->PutBackDescriptorSet(instrumentation_desc_pool, instrumentation_desc_set);
        instrumentation_desc_set = VK_NULL_HANDLE;
        instrumentation_desc_pool = VK_NULL_HANDLE;
    }
}

void PreDrawResources::Destroy(Validator &validator) {
    if (buffer_desc_set != VK_NULL_HANDLE) {
        validator.desc_set_manager->PutBackDescriptorSet(desc_pool, buffer_desc_set);
        buffer_desc_set = VK_NULL_HANDLE;
        desc_pool = VK_NULL_HANDLE;
    }

    CommandResources::Destroy(validator);
}

void PreDispatchResources::Destroy(Validator &validator) {
    if (indirect_buffer_desc_set != VK_NULL_HANDLE) {
        validator.desc_set_manager->PutBackDescriptorSet(desc_pool, indirect_buffer_desc_set);
        indirect_buffer_desc_set = VK_NULL_HANDLE;
        desc_pool = VK_NULL_HANDLE;
    }

    CommandResources::Destroy(validator);
}

void PreTraceRaysResources::Destroy(Validator &validator) { CommandResources::Destroy(validator); }

void PreCopyBufferToImageResources::Destroy(Validator &validator) {
    if (desc_set != VK_NULL_HANDLE) {
        validator.desc_set_manager->PutBackDescriptorSet(desc_pool, desc_set);
        desc_set = VK_NULL_HANDLE;
        desc_pool = VK_NULL_HANDLE;
    }

    if (copy_src_regions_buffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(validator.vmaAllocator, copy_src_regions_buffer, copy_src_regions_allocation);
        copy_src_regions_buffer = VK_NULL_HANDLE;
        copy_src_regions_allocation = VK_NULL_HANDLE;
    }

    CommandResources::Destroy(validator);
}

}  // namespace gpuav
