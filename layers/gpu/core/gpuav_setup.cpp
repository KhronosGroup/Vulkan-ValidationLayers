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
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__GNU__)
#include <unistd.h>
#endif
#include "utils/cast_utils.h"
#include "state_tracker/shader_stage_state.h"
#include "utils/hash_util.h"
#include "gpu/core/gpuav_constants.h"
#include "gpu/error_message/gpuav_vuids.h"
#include "gpu/core/gpuav.h"
#include "gpu/resources/gpuav_subclasses.h"
#include "state_tracker/device_state.h"
#include "state_tracker/shader_object_state.h"
#include "spirv-tools/instrument.hpp"
#include "spirv-tools/linker.hpp"
#include "generated/layer_chassis_dispatch.h"
#include "containers/custom_containers.h"
#include "gpu/shaders/gpu_error_header.h"
#include "generated/chassis.h"

namespace gpuav {

std::shared_ptr<vvl::Buffer> Validator::CreateBufferState(VkBuffer handle, const VkBufferCreateInfo *pCreateInfo) {
    return std::make_shared<Buffer>(*this, handle, pCreateInfo, *desc_heap_);
}

std::shared_ptr<vvl::BufferView> Validator::CreateBufferViewState(const std::shared_ptr<vvl::Buffer> &bf, VkBufferView bv,
                                                                  const VkBufferViewCreateInfo *ci,
                                                                  VkFormatFeatureFlags2KHR buf_ff) {
    return std::make_shared<BufferView>(bf, bv, ci, buf_ff, *desc_heap_);
}

std::shared_ptr<vvl::ImageView> Validator::CreateImageViewState(const std::shared_ptr<vvl::Image> &image_state, VkImageView iv,
                                                                const VkImageViewCreateInfo *ci, VkFormatFeatureFlags2KHR ff,
                                                                const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props) {
    return std::make_shared<ImageView>(image_state, iv, ci, ff, cubic_props, *desc_heap_);
}

std::shared_ptr<vvl::Sampler> Validator::CreateSamplerState(VkSampler s, const VkSamplerCreateInfo *ci) {
    return std::make_shared<Sampler>(s, ci, *desc_heap_);
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

std::shared_ptr<vvl::Queue> Validator::CreateQueue(VkQueue q, uint32_t family_index, uint32_t queue_index,
                                                   VkDeviceQueueCreateFlags flags,
                                                   const VkQueueFamilyProperties &queueFamilyProperties) {
    return std::static_pointer_cast<vvl::Queue>(
        std::make_shared<Queue>(*this, q, family_index, queue_index, flags, queueFamilyProperties));
}

void Validator::PreCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator, VkDevice *pDevice,
                                          const RecordObject &record_obj, vku::safe_VkDeviceCreateInfo *modified_create_info) {
    BaseClass::PreCallRecordCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice, record_obj, modified_create_info);

    auto add_missing_features = [this, &record_obj, modified_create_info]() {
        if (force_buffer_device_address_) {
            // Add buffer device address feature
            if (auto *bda_features = const_cast<VkPhysicalDeviceBufferDeviceAddressFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceBufferDeviceAddressFeatures>(modified_create_info))) {
                InternalWarning(device, record_obj.location,
                                "Forcing VkPhysicalDeviceBufferDeviceAddressFeatures::bufferDeviceAddress to VK_TRUE");
                bda_features->bufferDeviceAddress = VK_TRUE;
            } else {
                InternalWarning(
                    device, record_obj.location,
                    "Adding a VkPhysicalDeviceBufferDeviceAddressFeatures to pNext with bufferDeviceAddress set to VK_TRUE");
                VkPhysicalDeviceBufferDeviceAddressFeatures new_bda_features = vku::InitStructHelper();
                new_bda_features.bufferDeviceAddress = VK_TRUE;
                vku::AddToPnext(*modified_create_info, new_bda_features);
            }
        }
    };

    if (api_version > VK_API_VERSION_1_1) {
        if (auto *features12 = const_cast<VkPhysicalDeviceVulkan12Features *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(modified_create_info->pNext))) {
            if (force_buffer_device_address_ && !features12->bufferDeviceAddress) {
                InternalWarning(device, record_obj.location,
                                "Forcing VkPhysicalDeviceVulkan12Features::bufferDeviceAddress to VK_TRUE");
                features12->bufferDeviceAddress = VK_TRUE;
            }
        } else {
            add_missing_features();
        }
    } else if (api_version == VK_API_VERSION_1_1) {
        // Add our new extensions (will only add if found)
        const std::string_view bda_ext{"VK_KHR_buffer_device_address"};
        vku::AddExtension(*modified_create_info, bda_ext.data());
        add_missing_features();
    } else {
        force_buffer_device_address_ = false;
    }
}

// Perform initializations that can be done at Create Device time.
void Validator::PostCreateDevice(const VkDeviceCreateInfo *pCreateInfo, const Location &loc) {
    // Add the callback hooks for the functions that are either broadly or deeply used and that the ValidationStateTracker refactor
    // would be messier without.
    // TODO: Find a good way to do this hooklessly.
    SetSetImageViewInitialLayoutCallback(
        [](vvl::CommandBuffer *cb_state, const vvl::ImageView &iv_state, VkImageLayout layout) -> void {
            cb_state->SetImageViewInitialLayout(iv_state, layout);
        });

    // Set up a stub implementation of the descriptor heap in case we abort.
    desc_heap_.emplace(*this, 0);

    instrumentation_bindings_ = {
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
    BaseClass::PostCreateDevice(pCreateInfo, loc);

    if (api_version < VK_API_VERSION_1_1) {
        InternalError(device, loc, "GPU-Assisted validation requires Vulkan 1.1 or later. Aborting GPU-AV.");
        return;
    }

    VkPhysicalDeviceFeatures supported_features{};
    DispatchGetPhysicalDeviceFeatures(physical_device, &supported_features);

    if (!supported_features.fragmentStoresAndAtomics) {
        InternalError(device, loc, "GPU-Assisted validation requires fragmentStoresAndAtomics. Aborting GPU-AV.");
        return;
    }
    if (!supported_features.vertexPipelineStoresAndAtomics) {
        InternalError(device, loc, "GPU-Assisted validation requires vertexPipelineStoresAndAtomics. Aborting GPU-AV.");
        return;
    }

    bda_validation_possible = ((IsExtEnabled(device_extensions.vk_ext_buffer_device_address) ||
                                IsExtEnabled(device_extensions.vk_khr_buffer_device_address)) &&
                               supported_features.shaderInt64 && enabled_features.bufferDeviceAddress);
    if (!bda_validation_possible) {
        if (gpuav_settings.validate_bda) {
            if (!supported_features.shaderInt64) {
                InternalWarning(
                    device, loc,
                    "Buffer device address validation option was enabled, but required features shaderInt64 is not enabled. "
                    "Disabling option.");
            } else {
                InternalWarning(device, loc,
                                "Buffer device address validation option was enabled, but required buffer device address extension "
                                "and/or features are not enabled. Disabling option.");
            }
        }
        gpuav_settings.validate_bda = false;
    }

    if (gpuav_settings.validate_ray_query) {
        if (!enabled_features.rayQuery) {
            gpuav_settings.validate_ray_query = false;
            InternalWarning(device, loc,
                            "Ray query validation option was enabled, but required feature rayQuery is not enabled. "
                            "Disabling option.");
        }
    }

    // copy_buffer_to_image.comp relies on uint8_t buffers to perform validation
    if (gpuav_settings.validate_buffer_copies) {
        if (!enabled_features.uniformAndStorageBuffer8BitAccess) {
            gpuav_settings.validate_buffer_copies = false;
            InternalWarning(
                device, loc,
                "gpuav_validate_copies option was enabled, but uniformAndStorageBuffer8BitAccess feature is not available. "
                "Disabling option.");
        }
    }

    if (IsExtEnabled(device_extensions.vk_ext_descriptor_buffer)) {
        InternalWarning(
            device, loc,
            "VK_EXT_descriptor_buffer is enabled, but GPU-AV does not currently support validation of descriptor buffers. "
            "Use of descriptor buffers will result in no descriptor checking");
    }

    if (gpuav_settings.validate_descriptors && !force_buffer_device_address_) {
        gpuav_settings.validate_descriptors = false;
        InternalWarning(device, loc, "Buffer Device Address + feature is not available.  No descriptor checking will be attempted");
    }

    if (gpuav_settings.IsBufferValidationEnabled() && (phys_dev_props.limits.maxPushConstantsSize < 4 * sizeof(uint32_t))) {
        gpuav_settings.SetBufferValidationEnabled(false);
        InternalWarning(
            device, loc,
            "Device does not support the minimum range of push constants (32 bytes).  No indirect buffer checking will be "
            "attempted");
    }

    if (gpuav_settings.validate_descriptors) {
        VkPhysicalDeviceDescriptorIndexingProperties desc_indexing_props = vku::InitStructHelper();
        VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&desc_indexing_props);
        DispatchGetPhysicalDeviceProperties2Helper(physical_device, &props2);

        uint32_t num_descs = desc_indexing_props.maxUpdateAfterBindDescriptorsInAllPools;
        if (num_descs == 0 || num_descs > glsl::kDebugInputBindlessMaxDescriptors) {
            num_descs = glsl::kDebugInputBindlessMaxDescriptors;
        }

        desc_heap_.emplace(*this, num_descs);
    }

    VkBufferCreateInfo error_buffer_ci = vku::InitStructHelper();
    error_buffer_ci.size = glsl::kErrorBufferByteSize;
    error_buffer_ci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VmaAllocationCreateInfo alloc_create_info = {};
    alloc_create_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    uint32_t mem_type_index;
    VkResult result = vmaFindMemoryTypeIndexForBufferInfo(vma_allocator_, &error_buffer_ci, &alloc_create_info, &mem_type_index);
    if (result != VK_SUCCESS) {
        InternalError(device, loc, "Unable to find memory type index. Aborting GPU-AV.");
        return;
    }
    VmaPoolCreateInfo vma_pool_ci = {};
    vma_pool_ci.memoryTypeIndex = mem_type_index;
    vma_pool_ci.blockSize = 0;
    vma_pool_ci.maxBlockCount = 0;
    if (gpuav_settings.vma_linear_output) {
        vma_pool_ci.flags = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT;
    }
    result = vmaCreatePool(vma_allocator_, &vma_pool_ci, &output_buffer_pool_);
    if (result != VK_SUCCESS) {
        InternalError(device, loc, "Unable to create VMA memory pool. Aborting GPU-AV.");
        return;
    }

    if (gpuav_settings.cache_instrumented_shaders) {
        auto tmp_path = GetTempFilePath();
        instrumented_shader_cache_path_ = tmp_path + "/instrumented_shader_cache";
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__GNU__)
        instrumented_shader_cache_path_ += "-" + std::to_string(getuid());
#endif
        instrumented_shader_cache_path_ += ".bin";

        std::ifstream file_stream(instrumented_shader_cache_path_, std::ifstream::in | std::ifstream::binary);
        if (file_stream) {
            ShaderCacheHash shader_cache_hash(gpuav_settings);
            char inst_shader_hash[sizeof(shader_cache_hash)];
            file_stream.read(inst_shader_hash, sizeof(inst_shader_hash));
            if (std::memcmp(inst_shader_hash, reinterpret_cast<char *>(&shader_cache_hash), sizeof(shader_cache_hash)) == 0) {
                uint32_t num_shaders = 0;
                file_stream.read(reinterpret_cast<char *>(&num_shaders), sizeof(uint32_t));
                for (uint32_t i = 0; i < num_shaders; ++i) {
                    uint32_t hash;
                    uint32_t spirv_dwords_count;
                    std::vector<uint32_t> shader_code;
                    file_stream.read(reinterpret_cast<char *>(&hash), sizeof(uint32_t));
                    file_stream.read(reinterpret_cast<char *>(&spirv_dwords_count), sizeof(uint32_t));
                    shader_code.resize(spirv_dwords_count);
                    file_stream.read(reinterpret_cast<char *>(shader_code.data()), 4 * spirv_dwords_count);
                    instrumented_shaders_cache_.Add(hash, std::move(shader_code));
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
        assert(output_buffer_pool_);
        alloc_info.pool = output_buffer_pool_;
        alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        result = vmaCreateBuffer(vma_allocator_, &buffer_info, &alloc_info, &indices_buffer_.buffer, &indices_buffer_.allocation,
                                 nullptr);
        if (result != VK_SUCCESS) {
            InternalError(device, loc, "Unable to allocate device memory for command indices. Aborting GPU-AV.", true);
            return;
        }

        uint32_t *indices_ptr = nullptr;
        result = vmaMapMemory(vma_allocator_, indices_buffer_.allocation, reinterpret_cast<void **>(&indices_ptr));
        if (result != VK_SUCCESS) {
            InternalError(device, loc, "Unable to map device memory for command indices buffer. Aborting GPU-AV.");
            return;
        }

        for (uint32_t i = 0; i < cst::indices_count; ++i) {
            indices_ptr[i] = i;
        }

        vmaUnmapMemory(vma_allocator_, indices_buffer_.allocation);
    }
}

}  // namespace gpuav
