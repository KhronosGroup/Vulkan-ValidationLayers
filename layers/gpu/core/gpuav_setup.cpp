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
#include <cstring>
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__GNU__)
#include <unistd.h>
#endif
#include "gpu/core/gpuav_constants.h"
#include "gpu/core/gpuav.h"
#include "gpu/resources/gpuav_subclasses.h"
#include "generated/layer_chassis_dispatch.h"
#include "gpu/shaders/gpu_error_header.h"
#include "gpu/shaders/gpu_shaders_constants.h"
#include "generated/chassis.h"
#include "gpu/core/gpu_shader_cache_hash.h"

namespace gpuav {

std::shared_ptr<vvl::Buffer> Validator::CreateBufferState(VkBuffer handle, const VkBufferCreateInfo *create_info) {
    return std::make_shared<Buffer>(*this, handle, create_info, *desc_heap_);
}

std::shared_ptr<vvl::BufferView> Validator::CreateBufferViewState(const std::shared_ptr<vvl::Buffer> &buffer, VkBufferView handle,
                                                                  const VkBufferViewCreateInfo *create_info,
                                                                  VkFormatFeatureFlags2 format_features) {
    return std::make_shared<BufferView>(buffer, handle, create_info, format_features, *desc_heap_);
}

std::shared_ptr<vvl::ImageView> Validator::CreateImageViewState(const std::shared_ptr<vvl::Image> &image_state, VkImageView handle,
                                                                const VkImageViewCreateInfo *create_info,
                                                                VkFormatFeatureFlags2 format_features,
                                                                const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props) {
    return std::make_shared<ImageView>(image_state, handle, create_info, format_features, cubic_props, *desc_heap_);
}

std::shared_ptr<vvl::Sampler> Validator::CreateSamplerState(VkSampler handle, const VkSamplerCreateInfo *create_info) {
    return std::make_shared<Sampler>(handle, create_info, *desc_heap_);
}

std::shared_ptr<vvl::AccelerationStructureKHR> Validator::CreateAccelerationStructureState(
    VkAccelerationStructureKHR handle, const VkAccelerationStructureCreateInfoKHR *create_info,
    std::shared_ptr<vvl::Buffer> &&buf_state) {
    return std::make_shared<AccelerationStructureKHR>(handle, create_info, std::move(buf_state), *desc_heap_);
}

std::shared_ptr<vvl::DescriptorSet> Validator::CreateDescriptorSet(VkDescriptorSet handle, vvl::DescriptorPool *pool,
                                                                   const std::shared_ptr<vvl::DescriptorSetLayout const> &layout,
                                                                   uint32_t variable_count) {
    return std::static_pointer_cast<vvl::DescriptorSet>(
        std::make_shared<DescriptorSet>(handle, pool, layout, variable_count, this));
}

std::shared_ptr<vvl::CommandBuffer> Validator::CreateCmdBufferState(VkCommandBuffer handle,
                                                                    const VkCommandBufferAllocateInfo *allocate_info,
                                                                    const vvl::CommandPool *pool) {
    return std::static_pointer_cast<vvl::CommandBuffer>(std::make_shared<CommandBuffer>(*this, handle, allocate_info, pool));
}

static std::vector<VkExtensionProperties> GetExtensions(VkPhysicalDevice physical_device) {
    VkResult err;
    uint32_t extension_count = 512;
    std::vector<VkExtensionProperties> extensions(extension_count);
    for (;;) {
        err = DispatchEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, extensions.data());
        if (err == VK_SUCCESS) {
            extensions.resize(extension_count);
            return extensions;
        } else if (err == VK_INCOMPLETE) {
            extension_count *= 2;  // wasn't enough space, increase it
            extensions.resize(extension_count);
        } else {
            return {};
        }
    }
}

static bool IsExtensionAvailable(const char *extension_name, const std::vector<VkExtensionProperties> &available_extensions) {
    for (const VkExtensionProperties &ext : available_extensions) {
        if (strncmp(extension_name, ext.extensionName, VK_MAX_EXTENSION_NAME_SIZE) == 0) {
            return true;
        }
    }

    return false;
}

void Validator::PreCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator, VkDevice *pDevice,
                                          const RecordObject &record_obj, vku::safe_VkDeviceCreateInfo *modified_create_info) {
    BaseClass::PreCallRecordCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice, record_obj, modified_create_info);

    // In PreCallRecord this is all about trying to turn on as many feature/extension as possible on behalf of the app

    std::vector<VkExtensionProperties> available_extensions = GetExtensions(physicalDevice);

    // Force bufferDeviceAddress feature if available
    // ---
    auto add_bda_feature = [this, &record_obj, modified_create_info]() {
        // Add buffer device address feature
        if (auto *bda_features = const_cast<VkPhysicalDeviceBufferDeviceAddressFeatures *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceBufferDeviceAddressFeatures>(modified_create_info))) {
            if (!bda_features->bufferDeviceAddress) {
                InternalWarning(device, record_obj.location,
                                "Forcing VkPhysicalDeviceBufferDeviceAddressFeatures::bufferDeviceAddress to VK_TRUE");
                bda_features->bufferDeviceAddress = VK_TRUE;
            }
        } else {
            InternalWarning(
                device, record_obj.location,
                "Adding a VkPhysicalDeviceBufferDeviceAddressFeatures to pNext with bufferDeviceAddress set to VK_TRUE");
            VkPhysicalDeviceBufferDeviceAddressFeatures new_bda_features = vku::InitStructHelper();
            new_bda_features.bufferDeviceAddress = VK_TRUE;
            vku::AddToPnext(*modified_create_info, new_bda_features);
        }
    };

    if (api_version >= VK_API_VERSION_1_2) {
        if (auto *features12 = const_cast<VkPhysicalDeviceVulkan12Features *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(modified_create_info->pNext))) {
            if (!features12->bufferDeviceAddress) {
                InternalWarning(device, record_obj.location,
                                "Forcing VkPhysicalDeviceVulkan12Features::bufferDeviceAddress to VK_TRUE");
                features12->bufferDeviceAddress = VK_TRUE;
            }
        } else {
            add_bda_feature();
        }
    } else if (IsExtensionAvailable(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, available_extensions)) {
        // Add our new extensions, only add if not found
        vku::AddExtension(*modified_create_info, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
        add_bda_feature();
    }

    // Force uniformAndStorageBuffer8BitAccess feature if available and needed
    // ---
    if (gpuav_settings.validate_buffer_copies) {
        VkPhysicalDevice8BitStorageFeatures eight_bit_feature = vku::InitStructHelper();
        VkPhysicalDeviceFeatures2 features_2 = vku::InitStructHelper(&eight_bit_feature);
        DispatchGetPhysicalDeviceFeatures2(physicalDevice, &features_2);
        // uniformAndStorageBuffer8BitAccess is optional in 1.2. Only force on if available.
        if (eight_bit_feature.uniformAndStorageBuffer8BitAccess) {
            auto add_8bit_access_feature = [this, &record_obj, modified_create_info]() {
                // Add uniformAndStorageBuffer8BitAccess feature
                if (auto *eight_bit_access_feature = const_cast<VkPhysicalDevice8BitStorageFeatures *>(
                        vku::FindStructInPNextChain<VkPhysicalDevice8BitStorageFeatures>(modified_create_info))) {
                    if (!eight_bit_access_feature->uniformAndStorageBuffer8BitAccess) {
                        InternalWarning(
                            device, record_obj.location,
                            "Forcing VkPhysicalDevice8BitStorageFeatures::uniformAndStorageBuffer8BitAccess to VK_TRUE");
                        eight_bit_access_feature->uniformAndStorageBuffer8BitAccess = VK_TRUE;
                    }
                } else {
                    InternalWarning(device, record_obj.location,
                                    "Adding a VkPhysicalDevice8BitStorageFeatures to pNext with uniformAndStorageBuffer8BitAccess "
                                    "set to VK_TRUE");
                    VkPhysicalDevice8BitStorageFeatures new_bda_features = vku::InitStructHelper();
                    new_bda_features.uniformAndStorageBuffer8BitAccess = VK_TRUE;
                    vku::AddToPnext(*modified_create_info, new_bda_features);
                }
            };

            if (api_version >= VK_API_VERSION_1_2) {
                if (auto *features12 = const_cast<VkPhysicalDeviceVulkan12Features *>(
                        vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(modified_create_info->pNext))) {
                    if (!features12->uniformAndStorageBuffer8BitAccess) {
                        InternalWarning(device, record_obj.location,
                                        "Forcing VkPhysicalDeviceVulkan12Features::uniformAndStorageBuffer8BitAccess to VK_TRUE");
                        features12->uniformAndStorageBuffer8BitAccess = VK_TRUE;
                    }
                } else {
                    add_8bit_access_feature();
                }
            } else if (IsExtensionAvailable(VK_KHR_8BIT_STORAGE_EXTENSION_NAME, available_extensions)) {
                // Add our new extensions, only if not found
                vku::AddExtension(*modified_create_info, VK_KHR_8BIT_STORAGE_EXTENSION_NAME);
                add_8bit_access_feature();
            }
        }
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
    desc_heap_.emplace(*this, 0, loc);

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

    // Currently, both GPU-AV and DebugPrintf set their own instrumentation_bindings_ that this call will use
    BaseClass::PostCreateDevice(pCreateInfo, loc);
    // We might fail in parent class device creation if global requirements are not met
    if (aborted_) return;

    // Need the device to be created before we can query features for settings
    InitSettings(loc);

    if (gpuav_settings.shader_instrumentation.bindless_descriptor) {
        VkPhysicalDeviceDescriptorIndexingProperties desc_indexing_props = vku::InitStructHelper();
        VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&desc_indexing_props);
        DispatchGetPhysicalDeviceProperties2Helper(physical_device, &props2);

        uint32_t num_descs = desc_indexing_props.maxUpdateAfterBindDescriptorsInAllPools;
        if (num_descs == 0 || num_descs > glsl::kDebugInputBindlessMaxDescriptors) {
            num_descs = glsl::kDebugInputBindlessMaxDescriptors;
        }

        desc_heap_.emplace(*this, num_descs, loc);
    }

    VkBufferCreateInfo error_buffer_ci = vku::InitStructHelper();
    error_buffer_ci.size = glsl::kErrorBufferByteSize;
    error_buffer_ci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VmaAllocationCreateInfo alloc_create_info = {};
    alloc_create_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    uint32_t mem_type_index;
    VkResult result = vmaFindMemoryTypeIndexForBufferInfo(vma_allocator_, &error_buffer_ci, &alloc_create_info, &mem_type_index);
    if (result != VK_SUCCESS) {
        InternalError(device, loc, "Unable to find memory type index.", true);
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
        InternalError(device, loc, "Unable to create VMA memory pool.", true);
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
            ShaderCacheHash shader_cache_hash(gpuav_settings.shader_instrumentation);
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
        indices_buffer_alignment_ = sizeof(uint32_t) * static_cast<uint32_t>(phys_dev_props.limits.minStorageBufferOffsetAlignment);
        VkBufferCreateInfo buffer_info = vku::InitStructHelper();
        buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        buffer_info.size = cst::indices_count * indices_buffer_alignment_;
        VmaAllocationCreateInfo alloc_info = {};
        assert(output_buffer_pool_);
        alloc_info.pool = output_buffer_pool_;
        alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        result = vmaCreateBuffer(vma_allocator_, &buffer_info, &alloc_info, &indices_buffer_.buffer, &indices_buffer_.allocation,
                                 nullptr);
        if (result != VK_SUCCESS) {
            InternalError(device, loc, "Unable to allocate device memory for command indices.", true);
            return;
        }

        uint32_t *indices_ptr = nullptr;
        result = vmaMapMemory(vma_allocator_, indices_buffer_.allocation, reinterpret_cast<void **>(&indices_ptr));
        if (result != VK_SUCCESS) {
            InternalError(device, loc, "Unable to map device memory for command indices buffer.", true);
            return;
        }

        for (uint32_t i = 0; i < buffer_info.size / sizeof(uint32_t); ++i) {
            indices_ptr[i] = i / (indices_buffer_alignment_ / sizeof(uint32_t));
        }

        vmaUnmapMemory(vma_allocator_, indices_buffer_.allocation);
    }
}

// At this point extensions/features may have been turned on by us in PreCallRecord.
// Now that we have all the information, here is where we might disable GPU-AV settings that are missing requirements
void Validator::InitSettings(const Location &loc) {
    VkPhysicalDeviceFeatures supported_features{};
    DispatchGetPhysicalDeviceFeatures(physical_device, &supported_features);

    GpuAVSettings::ShaderInstrumentation &shader_instrumentation = gpuav_settings.shader_instrumentation;
    if (shader_instrumentation.bindless_descriptor) {
        if (!enabled_features.bufferDeviceAddress) {
            shader_instrumentation.bindless_descriptor = false;
            InternalWarning(device, loc,
                            "Descriptors Indexing Validation optin was enabled. but the bufferDeviceAddress was not supported "
                            "[Disabling gpuav_descriptor_checks]");
        }
    }

    if (shader_instrumentation.buffer_device_address) {
        const bool bda_validation_possible = ((IsExtEnabled(device_extensions.vk_ext_buffer_device_address) ||
                                               IsExtEnabled(device_extensions.vk_khr_buffer_device_address)) &&
                                              enabled_features.shaderInt64 && enabled_features.bufferDeviceAddress);
        if (!bda_validation_possible) {
            shader_instrumentation.buffer_device_address = false;
            if (!enabled_features.shaderInt64) {
                InternalWarning(
                    device, loc,
                    "Buffer device address validation option was enabled, but the shaderInt64 feature is not supported. "
                    "[Disabling gpuav_buffer_address_oob].");
            } else {
                InternalWarning(device, loc,
                                "Buffer device address validation option was enabled, but required buffer device address extension "
                                "and/or features are not enabled. [Disabling gpuav_buffer_address_oob]");
            }
        }
    }

    if (shader_instrumentation.ray_query) {
        if (!enabled_features.rayQuery) {
            // TODO - Force on if possible, issue is we need to potentially enable all the dependency extensions
            shader_instrumentation.ray_query = false;
            InternalWarning(device, loc,
                            "Ray Query validation option was enabled, but the rayQuery feature is not enabled. "
                            "[Disabling gpuav_validate_ray_query]");
        }
    }

    // copy_buffer_to_image.comp relies on uint8_t buffers to perform validation
    if (gpuav_settings.validate_buffer_copies) {
        if (!enabled_features.uniformAndStorageBuffer8BitAccess) {
            gpuav_settings.validate_buffer_copies = false;
            InternalWarning(device, loc,
                            "Buffer copies option was enabled, but the uniformAndStorageBuffer8BitAccess feature is not supported. "
                            "[Disabling gpuav_buffer_copies]");
        }
    }

    if (IsExtEnabled(device_extensions.vk_ext_descriptor_buffer)) {
        InternalWarning(
            device, loc,
            "VK_EXT_descriptor_buffer is enabled, but GPU-AV does not currently support validation of descriptor buffers. "
            "[Disabling shader_instrumentation_enabled]");
        // Because of VUs like VUID-VkPipelineLayoutCreateInfo-pSetLayouts-08008 we currently would need to rework the entire shader
        // instrumentation logic
        gpuav_settings.DisableShaderInstrumentationAndOptions();
    }

    if (gpuav_settings.IsBufferValidationEnabled()) {
        if (phys_dev_props.limits.maxPushConstantsSize < 4 * sizeof(uint32_t)) {
            gpuav_settings.SetBufferValidationEnabled(false);
            InternalWarning(
                device, loc,
                "Device does not support the minimum range of push constants (32 bytes). No indirect buffer checking will be "
                "attempted");
        }
    }

    // If we have turned off all the possible things to instrument, turn off everything fully
    if (!gpuav_settings.IsShaderInstrumentationEnabled()) {
        gpuav_settings.DisableShaderInstrumentationAndOptions();
    }
}

}  // namespace gpuav
