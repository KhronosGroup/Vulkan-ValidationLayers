/* Copyright (c) 2018-2023 The Khronos Group Inc.
 * Copyright (c) 2018-2023 Valve Corporation
 * Copyright (c) 2018-2023 LunarG, Inc.
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
#include "gpu_validation/gpu_validation.h"
#include "spirv-tools/instrument.hpp"
#include "spirv-tools/linker.hpp"
#include "generated/layer_chassis_dispatch.h"
#include "gpu_vuids.h"
#include "containers/custom_containers.h"
// Generated shaders
#include "generated/gpu_pre_draw_vert.h"
#include "generated/gpu_pre_dispatch_comp.h"
#include "generated/gpu_pre_trace_rays_rgen.h"
#include "generated/gpu_as_inspection_comp.h"
#include "generated/inst_functions_comp.h"
#include "generated/gpu_inst_shader_hash.h"

std::shared_ptr<vvl::Buffer> gpuav::Validator::CreateBufferState(VkBuffer buf, const VkBufferCreateInfo *pCreateInfo) {
    return std::make_shared<Buffer>(this, buf, pCreateInfo, *desc_heap);
}

std::shared_ptr<vvl::BufferView> gpuav::Validator::CreateBufferViewState(const std::shared_ptr<vvl::Buffer> &bf, VkBufferView bv,
                                                                         const VkBufferViewCreateInfo *ci,
                                                                         VkFormatFeatureFlags2KHR buf_ff) {
    return std::make_shared<BufferView>(bf, bv, ci, buf_ff, *desc_heap);
}

std::shared_ptr<vvl::ImageView> gpuav::Validator::CreateImageViewState(
    const std::shared_ptr<vvl::Image> &image_state, VkImageView iv, const VkImageViewCreateInfo *ci, VkFormatFeatureFlags2KHR ff,
    const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props) {
    return std::make_shared<ImageView>(image_state, iv, ci, ff, cubic_props, *desc_heap);
}

std::shared_ptr<vvl::AccelerationStructureNV> gpuav::Validator::CreateAccelerationStructureState(
    VkAccelerationStructureNV as, const VkAccelerationStructureCreateInfoNV *ci) {
    return std::make_shared<AccelerationStructureNV>(device, as, ci, *desc_heap);
}

std::shared_ptr<vvl::AccelerationStructureKHR> gpuav::Validator::CreateAccelerationStructureState(
    VkAccelerationStructureKHR as, const VkAccelerationStructureCreateInfoKHR *ci, std::shared_ptr<vvl::Buffer> &&buf_state,
    VkDeviceAddress address) {
    return std::make_shared<AccelerationStructureKHR>(as, ci, std::move(buf_state), address, *desc_heap);
}

std::shared_ptr<vvl::Sampler> gpuav::Validator::CreateSamplerState(VkSampler s, const VkSamplerCreateInfo *ci) {
    return std::make_shared<Sampler>(s, ci, *desc_heap);
}

std::shared_ptr<vvl::DescriptorSet> gpuav::Validator::CreateDescriptorSet(
    VkDescriptorSet set, vvl::DescriptorPool *pool, const std::shared_ptr<vvl::DescriptorSetLayout const> &layout,
    uint32_t variable_count) {
    return std::static_pointer_cast<vvl::DescriptorSet>(std::make_shared<DescriptorSet>(set, pool, layout, variable_count, this));
}

std::shared_ptr<vvl::CommandBuffer> gpuav::Validator::CreateCmdBufferState(VkCommandBuffer cb,
                                                                           const VkCommandBufferAllocateInfo *pCreateInfo,
                                                                           const vvl::CommandPool *pool) {
    return std::static_pointer_cast<vvl::CommandBuffer>(std::make_shared<CommandBuffer>(this, cb, pCreateInfo, pool));
}

// Perform initializations that can be done at Create Device time.
void gpuav::Validator::CreateDevice(const VkDeviceCreateInfo *pCreateInfo) {
    // Add the callback hooks for the functions that are either broadly or deeply used and that the ValidationStateTracker refactor
    // would be messier without.
    // TODO: Find a good way to do this hooklessly.
    SetSetImageViewInitialLayoutCallback(
        [](vvl::CommandBuffer *cb_state, const vvl::ImageView &iv_state, VkImageLayout layout) -> void {
            cb_state->SetImageViewInitialLayout(iv_state, layout);
        });

    // BaseClass::CreateDevice will set up bindings
    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
                                            VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT |
                                                VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT |
                                                gpu_tracker::kShaderStageAllRayTracing,
                                            NULL};
    // Set up a stub implementation of the descriptor heap in case we abort.
    desc_heap.emplace(*this, 0);
    bindings_.push_back(binding);
    for (auto i = 1; i < 3; i++) {
        binding.binding = i;
        bindings_.push_back(binding);
    }
    BaseClass::CreateDevice(pCreateInfo);
    Location loc(vvl::Func::vkCreateDevice);

    validate_instrumented_shaders = (GetEnvironment("VK_LAYER_GPUAV_VALIDATE_INSTRUMENTED_SHADERS").size() > 0);

    if (api_version < VK_API_VERSION_1_1) {
        ReportSetupProblem(device, "GPU-Assisted validation requires Vulkan 1.1 or later.  GPU-Assisted Validation disabled.");
        aborted = true;
        return;
    }

    DispatchGetPhysicalDeviceFeatures(physical_device, &supported_features);
    if (!supported_features.fragmentStoresAndAtomics || !supported_features.vertexPipelineStoresAndAtomics) {
        ReportSetupProblem(device,
                           "GPU-Assisted validation requires fragmentStoresAndAtomics and vertexPipelineStoresAndAtomics.  "
                           "GPU-Assisted Validation disabled.");
        aborted = true;
        return;
    }

    shaderInt64 = supported_features.shaderInt64;
    if ((IsExtEnabled(device_extensions.vk_ext_buffer_device_address) ||
         IsExtEnabled(device_extensions.vk_khr_buffer_device_address)) &&
        !shaderInt64) {
        LogWarning("WARNING-GPU-Assisted-Validation", device, loc,
                   "shaderInt64 feature is not available.  No buffer device address checking will be attempted");
    }
    buffer_device_address_enabled = ((IsExtEnabled(device_extensions.vk_ext_buffer_device_address) ||
                                      IsExtEnabled(device_extensions.vk_khr_buffer_device_address)) &&
                                     shaderInt64 && enabled_features.bufferDeviceAddress);

    if (buffer_device_address_enabled) {
        VkBufferCreateInfo buffer_info = vku::InitStructHelper();
        buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VmaAllocationCreateInfo alloc_info = {};
        // We need 2 words per address (address and size), 1 word for the start of sizes index, 2 words for the address section
        // bounds, and 2 more words for the size section bounds
        app_bda_buffer_size =
            (1 + (gpuav_settings.gpuav_max_buffer_device_addresses + 2) + (gpuav_settings.gpuav_max_buffer_device_addresses + 2)) *
            8;  // 64 bit words
        buffer_info.size = app_bda_buffer_size;
        // This buffer could be very large if an application uses many buffers. Allocating it as HOST_CACHED
        // and manually flushing it at the end of the state updates is faster than using HOST_COHERENT.
        alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        VkResult result = vmaCreateBuffer(vmaAllocator, &buffer_info, &alloc_info, &app_buffer_device_addresses.buffer,
                                          &app_buffer_device_addresses.allocation, nullptr);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(
                device, "Unable to allocate device memory for buffer device address data. Device could become unstable.", true);
            aborted = true;
            return;
        }
    }

    if (IsExtEnabled(device_extensions.vk_ext_descriptor_buffer)) {
        LogWarning("WARNING-GPU-Assisted-Validation", device, loc,
                   "VK_EXT_descriptor_buffer is enabled, but GPU-AV does not currently support validation of descriptor buffers. "
                   "Use of descriptor buffers will result in no descriptor checking");
    }

    output_buffer_size = sizeof(uint32_t) * (glsl::kInstMaxOutCnt + spvtools::kDebugOutputDataOffset);

    if (gpuav_settings.validate_descriptors && !force_buffer_device_address) {
        gpuav_settings.validate_descriptors = false;
        LogWarning("WARNING-GPU-Assisted-Validation", device, loc,
                   "Buffer Device Address + feature is not available.  No descriptor checking will be attempted");
    }

    if (gpuav_settings.validate_indirect_buffer && (phys_dev_props.limits.maxPushConstantsSize < 4 * sizeof(uint32_t))) {
        gpuav_settings.validate_indirect_buffer = false;
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
        output_buffer_create_info.size = output_buffer_size;
        output_buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VmaAllocationCreateInfo alloc_create_info = {};
        alloc_create_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        uint32_t mem_type_index;
        vmaFindMemoryTypeIndexForBufferInfo(vmaAllocator, &output_buffer_create_info, &alloc_create_info, &mem_type_index);
        VmaPoolCreateInfo pool_create_info = {};
        pool_create_info.memoryTypeIndex = mem_type_index;
        pool_create_info.blockSize = 0;
        pool_create_info.maxBlockCount = 0;
        pool_create_info.flags = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT;
        VkResult result = vmaCreatePool(vmaAllocator, &pool_create_info, &output_buffer_pool);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Unable to create VMA memory pool");
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

    CreateAccelerationStructureBuildValidationState(pCreateInfo);
}

void gpuav::Validator::CreateAccelerationStructureBuildValidationState(const VkDeviceCreateInfo *pCreateInfo) {
    if (aborted) {
        return;
    }

    auto &as_validation_state = acceleration_structure_validation_state;
    if (as_validation_state.initialized) {
        return;
    }

    if (!IsExtEnabled(device_extensions.vk_nv_ray_tracing)) {
        return;
    }
    Location loc(vvl::Func::vkCreateDevice);

    // Cannot use this validation without a queue that supports graphics
    auto pd_state = Get<vvl::PhysicalDevice>(physical_device);
    bool graphics_queue_exists = false;
    uint32_t graphics_queue_family = 0;
    for (uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; i++) {
        auto qfi = pCreateInfo->pQueueCreateInfos[i].queueFamilyIndex;
        if (pd_state->queue_family_properties[qfi].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphics_queue_family = qfi;
            graphics_queue_exists = true;
            break;
        }
    }
    if (!graphics_queue_exists) {
        LogWarning("WARNING-GPU-Assisted-Validation", device, loc, "No queue that supports graphics, GPU-AV aborted.");
        aborted = true;
        return;
    }

    // Outline:
    //   - Create valid bottom level acceleration structure which acts as replacement
    //      - Create and load vertex buffer
    //      - Create and load index buffer
    //      - Create, allocate memory for, and bind memory for acceleration structure
    //      - Query acceleration structure handle
    //      - Create command pool and command buffer
    //      - Record build acceleration structure command
    //      - Submit command buffer and wait for completion
    //      - Cleanup
    //  - Create compute pipeline for validating instance buffers
    //      - Create descriptor set layout
    //      - Create pipeline layout
    //      - Create pipeline
    //      - Cleanup

    VkResult result = VK_SUCCESS;

    VkBuffer vbo = VK_NULL_HANDLE;
    VmaAllocation vbo_allocation = VK_NULL_HANDLE;
    if (result == VK_SUCCESS) {
        VkBufferCreateInfo vbo_ci = vku::InitStructHelper();
        vbo_ci.size = sizeof(float) * 9;
        vbo_ci.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;

        VmaAllocationCreateInfo vbo_ai = {};
        vbo_ai.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        vbo_ai.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        result = vmaCreateBuffer(vmaAllocator, &vbo_ci, &vbo_ai, &vbo, &vbo_allocation, nullptr);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Failed to create vertex buffer for acceleration structure build validation.");
        }
    }

    if (result == VK_SUCCESS) {
        uint8_t *mapped_vbo_buffer = nullptr;
        result = vmaMapMemory(vmaAllocator, vbo_allocation, reinterpret_cast<void **>(&mapped_vbo_buffer));
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Failed to map vertex buffer for acceleration structure build validation.");
        } else {
            constexpr std::array vertices = {1.0f, 0.0f, 0.0f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f};
            std::memcpy(mapped_vbo_buffer, (uint8_t *)vertices.data(), sizeof(vertices[0]) * vertices.size());
            vmaUnmapMemory(vmaAllocator, vbo_allocation);
        }
    }

    VkBuffer ibo = VK_NULL_HANDLE;
    VmaAllocation ibo_allocation = VK_NULL_HANDLE;
    if (result == VK_SUCCESS) {
        VkBufferCreateInfo ibo_ci = vku::InitStructHelper();
        ibo_ci.size = sizeof(uint32_t) * 3;
        ibo_ci.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;

        VmaAllocationCreateInfo ibo_ai = {};
        ibo_ai.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        ibo_ai.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        result = vmaCreateBuffer(vmaAllocator, &ibo_ci, &ibo_ai, &ibo, &ibo_allocation, nullptr);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Failed to create index buffer for acceleration structure build validation.");
        }
    }

    if (result == VK_SUCCESS) {
        uint8_t *mapped_ibo_buffer = nullptr;
        result = vmaMapMemory(vmaAllocator, ibo_allocation, reinterpret_cast<void **>(&mapped_ibo_buffer));
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Failed to map index buffer for acceleration structure build validation.");
        } else {
            constexpr std::array<uint32_t, 3> indicies = {0, 1, 2};
            std::memcpy(mapped_ibo_buffer, (uint8_t *)indicies.data(), sizeof(indicies[0]) * indicies.size());
            vmaUnmapMemory(vmaAllocator, ibo_allocation);
        }
    }

    VkGeometryNV geometry = vku::InitStructHelper();
    geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
    geometry.geometry.triangles = vku::InitStructHelper();
    geometry.geometry.triangles.vertexData = vbo;
    geometry.geometry.triangles.vertexOffset = 0;
    geometry.geometry.triangles.vertexCount = 3;
    geometry.geometry.triangles.vertexStride = 12;
    geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    geometry.geometry.triangles.indexData = ibo;
    geometry.geometry.triangles.indexOffset = 0;
    geometry.geometry.triangles.indexCount = 3;
    geometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
    geometry.geometry.triangles.transformData = VK_NULL_HANDLE;
    geometry.geometry.triangles.transformOffset = 0;
    geometry.geometry.aabbs = vku::InitStructHelper();

    VkAccelerationStructureCreateInfoNV as_ci = vku::InitStructHelper();
    as_ci.info = vku::InitStructHelper();
    as_ci.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    as_ci.info.instanceCount = 0;
    as_ci.info.geometryCount = 1;
    as_ci.info.pGeometries = &geometry;
    if (result == VK_SUCCESS) {
        result = DispatchCreateAccelerationStructureNV(device, &as_ci, nullptr, &as_validation_state.replacement_as);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Failed to create acceleration structure for acceleration structure build validation.");
        }
    }

    VkMemoryRequirements2 as_mem_requirements = {};
    if (result == VK_SUCCESS) {
        VkAccelerationStructureMemoryRequirementsInfoNV as_mem_requirements_info = vku::InitStructHelper();
        as_mem_requirements_info.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
        as_mem_requirements_info.accelerationStructure = as_validation_state.replacement_as;

        DispatchGetAccelerationStructureMemoryRequirementsNV(device, &as_mem_requirements_info, &as_mem_requirements);
    }

    VmaAllocationInfo as_memory_ai = {};
    if (result == VK_SUCCESS) {
        VmaAllocationCreateInfo as_memory_aci = {};
        as_memory_aci.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        result = vmaAllocateMemory(vmaAllocator, &as_mem_requirements.memoryRequirements, &as_memory_aci,
                                   &as_validation_state.replacement_as_allocation, &as_memory_ai);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device,
                               "Failed to alloc acceleration structure memory for acceleration structure build validation.");
        }
    }

    if (result == VK_SUCCESS) {
        VkBindAccelerationStructureMemoryInfoNV as_bind_info = vku::InitStructHelper();
        as_bind_info.accelerationStructure = as_validation_state.replacement_as;
        as_bind_info.memory = as_memory_ai.deviceMemory;
        as_bind_info.memoryOffset = as_memory_ai.offset;

        result = DispatchBindAccelerationStructureMemoryNV(device, 1, &as_bind_info);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Failed to bind acceleration structure memory for acceleration structure build validation.");
        }
    }

    if (result == VK_SUCCESS) {
        result = DispatchGetAccelerationStructureHandleNV(device, as_validation_state.replacement_as, sizeof(uint64_t),
                                                          &as_validation_state.replacement_as_handle);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Failed to get acceleration structure handle for acceleration structure build validation.");
        }
    }

    VkMemoryRequirements2 scratch_mem_requirements = {};
    if (result == VK_SUCCESS) {
        VkAccelerationStructureMemoryRequirementsInfoNV scratch_mem_requirements_info = vku::InitStructHelper();
        scratch_mem_requirements_info.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
        scratch_mem_requirements_info.accelerationStructure = as_validation_state.replacement_as;

        DispatchGetAccelerationStructureMemoryRequirementsNV(device, &scratch_mem_requirements_info, &scratch_mem_requirements);
    }

    VkBuffer scratch = VK_NULL_HANDLE;
    VmaAllocation scratch_allocation = {};
    if (result == VK_SUCCESS) {
        VkBufferCreateInfo scratch_ci = vku::InitStructHelper();
        scratch_ci.size = scratch_mem_requirements.memoryRequirements.size;
        scratch_ci.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
        VmaAllocationCreateInfo scratch_aci = {};
        scratch_aci.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        result = vmaCreateBuffer(vmaAllocator, &scratch_ci, &scratch_aci, &scratch, &scratch_allocation, nullptr);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Failed to create scratch buffer for acceleration structure build validation.");
        }
    }

    VkCommandPool command_pool = VK_NULL_HANDLE;
    if (result == VK_SUCCESS) {
        VkCommandPoolCreateInfo command_pool_ci = vku::InitStructHelper();
        command_pool_ci.queueFamilyIndex = 0;

        result = DispatchCreateCommandPool(device, &command_pool_ci, nullptr, &command_pool);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Failed to create command pool for acceleration structure build validation.");
        }
    }

    VkCommandBuffer command_buffer = VK_NULL_HANDLE;

    if (result == VK_SUCCESS) {
        VkCommandBufferAllocateInfo command_buffer_ai = vku::InitStructHelper();
        command_buffer_ai.commandPool = command_pool;
        command_buffer_ai.commandBufferCount = 1;
        command_buffer_ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        result = DispatchAllocateCommandBuffers(device, &command_buffer_ai, &command_buffer);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Failed to create command buffer for acceleration structure build validation.");
        }

        // Hook up command buffer dispatch
        vkSetDeviceLoaderData(device, command_buffer);
    }

    if (result == VK_SUCCESS) {
        VkCommandBufferBeginInfo command_buffer_bi = vku::InitStructHelper();

        result = DispatchBeginCommandBuffer(command_buffer, &command_buffer_bi);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Failed to begin command buffer for acceleration structure build validation.");
        }
    }

    if (result == VK_SUCCESS) {
        DispatchCmdBuildAccelerationStructureNV(command_buffer, &as_ci.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                                as_validation_state.replacement_as, VK_NULL_HANDLE, scratch, 0);
        DispatchEndCommandBuffer(command_buffer);
    }

    VkQueue queue = VK_NULL_HANDLE;
    if (result == VK_SUCCESS) {
        DispatchGetDeviceQueue(device, graphics_queue_family, 0, &queue);

        // Hook up queue dispatch
        vkSetDeviceLoaderData(device, queue);

        VkSubmitInfo submit_info = vku::InitStructHelper();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;
        result = DispatchQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Failed to submit command buffer for acceleration structure build validation.");
        }
    }

    if (result == VK_SUCCESS) {
        result = DispatchQueueWaitIdle(queue);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Failed to wait for queue idle for acceleration structure build validation.");
        }
    }

    if (vbo != VK_NULL_HANDLE) {
        vmaDestroyBuffer(vmaAllocator, vbo, vbo_allocation);
    }
    if (ibo != VK_NULL_HANDLE) {
        vmaDestroyBuffer(vmaAllocator, ibo, ibo_allocation);
    }
    if (scratch != VK_NULL_HANDLE) {
        vmaDestroyBuffer(vmaAllocator, scratch, scratch_allocation);
    }
    if (command_pool != VK_NULL_HANDLE) {
        DispatchDestroyCommandPool(device, command_pool, nullptr);
    }

    if (debug_desc_layout == VK_NULL_HANDLE) {
        ReportSetupProblem(device, "Failed to find descriptor set layout for acceleration structure build validation.");
        result = VK_INCOMPLETE;
    }

    if (result == VK_SUCCESS) {
        VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
        pipeline_layout_ci.setLayoutCount = 1;
        pipeline_layout_ci.pSetLayouts = &debug_desc_layout;
        result = DispatchCreatePipelineLayout(device, &pipeline_layout_ci, 0, &as_validation_state.pipeline_layout);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Failed to create pipeline layout for acceleration structure build validation.");
        }
    }

    VkShaderModule shader_module = VK_NULL_HANDLE;
    if (result == VK_SUCCESS) {
        VkShaderModuleCreateInfo shader_module_ci = vku::InitStructHelper();
        shader_module_ci.codeSize = sizeof(gpu_as_inspection_comp);
        shader_module_ci.pCode = gpu_as_inspection_comp;

        result = DispatchCreateShaderModule(device, &shader_module_ci, nullptr, &shader_module);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Failed to create compute shader module for acceleration structure build validation.");
        }
    }

    if (result == VK_SUCCESS) {
        VkPipelineShaderStageCreateInfo pipeline_stage_ci = vku::InitStructHelper();
        pipeline_stage_ci.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        pipeline_stage_ci.module = shader_module;
        pipeline_stage_ci.pName = "main";

        VkComputePipelineCreateInfo pipeline_ci = vku::InitStructHelper();
        pipeline_ci.stage = pipeline_stage_ci;
        pipeline_ci.layout = as_validation_state.pipeline_layout;

        result = DispatchCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &as_validation_state.pipeline);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Failed to create compute pipeline for acceleration structure build validation.");
        }
    }

    if (shader_module != VK_NULL_HANDLE) {
        DispatchDestroyShaderModule(device, shader_module, nullptr);
    }

    if (result == VK_SUCCESS) {
        as_validation_state.initialized = true;
        LogInfo("WARNING-GPU-Assisted-Validation", device, loc, "Acceleration Structure Building GPU Validation Enabled.");
    } else {
        aborted = true;
    }
}

void gpuav::Validator::Destroy(AccelerationStructureBuildValidationInfo &as_validation_info) {
    vmaDestroyBuffer(vmaAllocator, as_validation_info.buffer, as_validation_info.buffer_allocation);

    if (as_validation_info.descriptor_set != VK_NULL_HANDLE) {
        desc_set_manager->PutBackDescriptorSet(as_validation_info.descriptor_pool, as_validation_info.descriptor_set);
    }
}

void gpuav::CommonDrawResources::Destroy(VkDevice device) {
    if (shader_module != VK_NULL_HANDLE) {
        DispatchDestroyShaderModule(device, shader_module, nullptr);
        shader_module = VK_NULL_HANDLE;
    }
    if (ds_layout != VK_NULL_HANDLE) {
        DispatchDestroyDescriptorSetLayout(device, ds_layout, nullptr);
        ds_layout = VK_NULL_HANDLE;
    }
    if (pipeline_layout != VK_NULL_HANDLE) {
        DispatchDestroyPipelineLayout(device, pipeline_layout, nullptr);
        pipeline_layout = VK_NULL_HANDLE;
    }
    auto to_destroy = renderpass_to_pipeline.snapshot();
    for (auto &entry : to_destroy) {
        DispatchDestroyPipeline(device, entry.second, nullptr);
        renderpass_to_pipeline.erase(entry.first);
    }
    if (shader_object != VK_NULL_HANDLE) {
        DispatchDestroyShaderEXT(device, shader_object, nullptr);
        shader_object = VK_NULL_HANDLE;
    }
    initialized = false;
}

void gpuav::CommonDispatchResources::Destroy(VkDevice device) {
    if (shader_module != VK_NULL_HANDLE) {
        DispatchDestroyShaderModule(device, shader_module, nullptr);
        shader_module = VK_NULL_HANDLE;
    }
    if (ds_layout != VK_NULL_HANDLE) {
        DispatchDestroyDescriptorSetLayout(device, ds_layout, nullptr);
        ds_layout = VK_NULL_HANDLE;
    }
    if (pipeline_layout != VK_NULL_HANDLE) {
        DispatchDestroyPipelineLayout(device, pipeline_layout, nullptr);
        pipeline_layout = VK_NULL_HANDLE;
    }
    if (pipeline != VK_NULL_HANDLE) {
        DispatchDestroyPipeline(device, pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;
    }
    if (shader_object != VK_NULL_HANDLE) {
        DispatchDestroyShaderEXT(device, shader_object, nullptr);
        shader_object = VK_NULL_HANDLE;
    }
    initialized = false;
}

void gpuav::CommonTraceRaysResources::Destroy(VkDevice device, VmaAllocator &vmaAllocator) {
    if (shader_module != VK_NULL_HANDLE) {
        DispatchDestroyShaderModule(device, shader_module, nullptr);
        shader_module = VK_NULL_HANDLE;
    }
    if (ds_layout != VK_NULL_HANDLE) {
        DispatchDestroyDescriptorSetLayout(device, ds_layout, nullptr);
        ds_layout = VK_NULL_HANDLE;
    }
    if (pipeline_layout != VK_NULL_HANDLE) {
        DispatchDestroyPipelineLayout(device, pipeline_layout, nullptr);
        pipeline_layout = VK_NULL_HANDLE;
    }
    if (pipeline != VK_NULL_HANDLE) {
        DispatchDestroyPipeline(device, pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;
    }
    if (sbt_buffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(vmaAllocator, sbt_buffer, sbt_allocation);
        sbt_buffer = VK_NULL_HANDLE;
        sbt_allocation = VK_NULL_HANDLE;
        sbt_address = 0;
    }
    if (sbt_pool) {
        vmaDestroyPool(vmaAllocator, sbt_pool);
        sbt_pool = VK_NULL_HANDLE;
    }

    initialized = false;
}

void gpuav::AccelerationStructureBuildValidationState::Destroy(VkDevice device, VmaAllocator &vmaAllocator) {
    if (pipeline != VK_NULL_HANDLE) {
        DispatchDestroyPipeline(device, pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;
    }
    if (pipeline_layout != VK_NULL_HANDLE) {
        DispatchDestroyPipelineLayout(device, pipeline_layout, nullptr);
        pipeline_layout = VK_NULL_HANDLE;
    }
    if (replacement_as != VK_NULL_HANDLE) {
        DispatchDestroyAccelerationStructureNV(device, replacement_as, nullptr);
        replacement_as = VK_NULL_HANDLE;
    }
    if (replacement_as_allocation != VK_NULL_HANDLE) {
        vmaFreeMemory(vmaAllocator, replacement_as_allocation);
        replacement_as_allocation = VK_NULL_HANDLE;
    }
    initialized = false;
}

void gpuav::RestorablePipelineState::Create(vvl::CommandBuffer *cb_state, VkPipelineBindPoint bind_point) {
    pipeline_bind_point = bind_point;
    const auto lv_bind_point = ConvertToLvlBindPoint(bind_point);

    LastBound &last_bound = cb_state->lastBound[lv_bind_point];
    if (last_bound.pipeline_state) {
        pipeline = last_bound.pipeline_state->pipeline();
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
        if (pipeline_layout->push_constant_ranges == cb_state->push_constant_data_ranges) {
            push_constants_data = cb_state->push_constant_data;
            push_constants_ranges = pipeline_layout->push_constant_ranges;
        }
    }
}

void gpuav::RestorablePipelineState::Restore(VkCommandBuffer command_buffer) const {
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
}

void gpuav::CommandResources::Destroy(gpuav::Validator &validator) {
    if (output_mem_block.buffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(validator.vmaAllocator, output_mem_block.buffer, output_mem_block.allocation);
    }
    if (output_buffer_desc_set != VK_NULL_HANDLE) {
        validator.desc_set_manager->PutBackDescriptorSet(output_buffer_desc_pool, output_buffer_desc_set);
    }
    output_mem_block.buffer = VK_NULL_HANDLE;
    output_mem_block.allocation = VK_NULL_HANDLE;
    output_buffer_desc_set = VK_NULL_HANDLE;
}

void gpuav::PreDrawResources::Destroy(gpuav::Validator &validator) {
    if (buffer_desc_set != VK_NULL_HANDLE) {
        validator.desc_set_manager->PutBackDescriptorSet(desc_pool, buffer_desc_set);
        buffer_desc_set = VK_NULL_HANDLE;
        desc_pool = VK_NULL_HANDLE;
    }

    CommandResources::Destroy(validator);
}

void gpuav::PreDispatchResources::Destroy(gpuav::Validator &validator) {
    if (indirect_buffer_desc_set != VK_NULL_HANDLE) {
        validator.desc_set_manager->PutBackDescriptorSet(desc_pool, indirect_buffer_desc_set);
        indirect_buffer_desc_set = VK_NULL_HANDLE;
        desc_pool = VK_NULL_HANDLE;
    }

    CommandResources::Destroy(validator);
}

void gpuav::PreTraceRaysResources::Destroy(gpuav::Validator &validator) {
    if (desc_set != VK_NULL_HANDLE) {
        validator.desc_set_manager->PutBackDescriptorSet(desc_pool, desc_set);
        desc_set = VK_NULL_HANDLE;
        desc_pool = VK_NULL_HANDLE;
    }

    CommandResources::Destroy(validator);
}
