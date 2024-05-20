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

#include "gpu/core/gpuav.h"
#include "gpu/cmd_validation/gpuav_cmd_validation_common.h"
#include "gpu/resources/gpuav_subclasses.h"
// Generated shaders
#include "generated/cmd_validation_trace_rays_rgen.h"

namespace gpuav {

std::unique_ptr<CommandResources> Validator::AllocatePreTraceRaysValidationResources(const Location &loc,
                                                                                     VkCommandBuffer cmd_buffer,
                                                                                     VkDeviceAddress indirect_data_address) {
    auto cb_node = GetWrite<CommandBuffer>(cmd_buffer);
    if (!cb_node) {
        InternalError(cmd_buffer, loc, "Unrecognized command buffer");
        return nullptr;
    }

    if (!gpuav_settings.validate_indirect_trace_rays_buffers) {
        CommandResources cmd_resources = AllocateActionCommandResources(cb_node, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, loc);
        auto cmd_resources_ptr = std::make_unique<CommandResources>(cmd_resources);
        return cmd_resources_ptr;
    }

    auto trace_rays_resources = std::make_unique<PreTraceRaysResources>();

    {
        PreTraceRaysResources::SharedResources *shared_resources =
            GetSharedTraceRaysValidationResources(cb_node->GetValidationCmdCommonDescriptorSetLayout(), loc);
        if (!shared_resources) {
            return nullptr;
        }

        trace_rays_resources->indirect_data_address = indirect_data_address;

        // Save current ray tracing pipeline state
        RestorablePipelineState restorable_state(*cb_node, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);

        // Push info needed for validation:
        // - the device address indirect data is read from
        // - the limits to check against
        uint32_t push_constants[PreTraceRaysResources::push_constant_words] = {};
        const uint64_t ray_query_dimension_max_width = static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupCount[0]) *
                                                       static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupSize[0]);
        const uint64_t ray_query_dimension_max_height = static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupCount[1]) *
                                                        static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupSize[1]);
        const uint64_t ray_query_dimension_max_depth = static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupCount[2]) *
                                                       static_cast<uint64_t>(phys_dev_props.limits.maxComputeWorkGroupSize[2]);
        // Need to put the buffer reference first otherwise it is incorrect, probably an alignment issue
        push_constants[0] = static_cast<uint32_t>(trace_rays_resources->indirect_data_address) & vvl::kU32Max;
        push_constants[1] = static_cast<uint32_t>(trace_rays_resources->indirect_data_address >> 32) & vvl::kU32Max;
        push_constants[2] = static_cast<uint32_t>(std::min<uint64_t>(ray_query_dimension_max_width, vvl::kU32Max));
        push_constants[3] = static_cast<uint32_t>(std::min<uint64_t>(ray_query_dimension_max_height, vvl::kU32Max));
        push_constants[4] = static_cast<uint32_t>(std::min<uint64_t>(ray_query_dimension_max_depth, vvl::kU32Max));

        DispatchCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, shared_resources->pipeline);
        BindValidationCmdsCommonDescSet(cb_node, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, shared_resources->pipeline_layout,
                                        cb_node->trace_rays_index, static_cast<uint32_t>(cb_node->per_command_resources.size()));
        DispatchCmdPushConstants(cmd_buffer, shared_resources->pipeline_layout, VK_SHADER_STAGE_RAYGEN_BIT_KHR, 0,
                                 sizeof(push_constants), push_constants);
        VkStridedDeviceAddressRegionKHR ray_gen_sbt{};
        assert(shared_resources->sbt_address != 0);
        ray_gen_sbt.deviceAddress = shared_resources->sbt_address;
        ray_gen_sbt.stride = shared_resources->shader_group_handle_size_aligned;
        ray_gen_sbt.size = shared_resources->shader_group_handle_size_aligned;

        VkStridedDeviceAddressRegionKHR empty_sbt{};
        DispatchCmdTraceRaysKHR(cmd_buffer, &ray_gen_sbt, &empty_sbt, &empty_sbt, &empty_sbt, 1, 1, 1);

        CommandResources cmd_resources = AllocateActionCommandResources(cb_node, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, loc);
        if (aborted) {
            return nullptr;
        }
        CommandResources &base = *trace_rays_resources;
        base = cmd_resources;

        // Restore the previous ray tracing pipeline state.
        restorable_state.Restore(cmd_buffer);
    }

    return trace_rays_resources;
}

PreTraceRaysResources::SharedResources *Validator::GetSharedTraceRaysValidationResources(
    VkDescriptorSetLayout error_output_desc_layout, const Location &loc) {
    if (auto shared_resources = shared_validation_resources_map.find(typeid(PreTraceRaysResources::SharedResources));
        shared_resources != shared_validation_resources_map.end()) {
        return reinterpret_cast<PreTraceRaysResources::SharedResources *>(shared_resources->second.get());
    }

    auto shared_resources = std::make_unique<PreTraceRaysResources::SharedResources>();

    VkResult result = VK_SUCCESS;

    VkPushConstantRange push_constant_range = {};
    push_constant_range.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    push_constant_range.offset = 0;
    push_constant_range.size = PreTraceRaysResources::push_constant_words * sizeof(uint32_t);

    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    pipeline_layout_ci.pushConstantRangeCount = 1;
    pipeline_layout_ci.pPushConstantRanges = &push_constant_range;
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &error_output_desc_layout;
    result = DispatchCreatePipelineLayout(device, &pipeline_layout_ci, nullptr, &shared_resources->pipeline_layout);
    if (result != VK_SUCCESS) {
        InternalError(device, loc, "Unable to create pipeline layout. Aborting GPU-AV");
        return nullptr;
    }

    VkShaderModuleCreateInfo shader_module_ci = vku::InitStructHelper();
    shader_module_ci.codeSize = cmd_validation_trace_rays_rgen_size * sizeof(uint32_t);
    shader_module_ci.pCode = cmd_validation_trace_rays_rgen;
    VkShaderModule validation_shader = VK_NULL_HANDLE;
    result = DispatchCreateShaderModule(device, &shader_module_ci, nullptr, &validation_shader);
    if (result != VK_SUCCESS) {
        InternalError(device, loc, "Unable to create ray tracing shader module. Aborting GPU-AV");
        return nullptr;
    }

    // Create pipeline
    VkPipelineShaderStageCreateInfo pipeline_stage_ci = vku::InitStructHelper();
    pipeline_stage_ci.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    pipeline_stage_ci.module = validation_shader;
    pipeline_stage_ci.pName = "main";
    VkRayTracingShaderGroupCreateInfoKHR raygen_group_ci = vku::InitStructHelper();
    raygen_group_ci.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    raygen_group_ci.generalShader = 0;
    raygen_group_ci.closestHitShader = VK_SHADER_UNUSED_KHR;
    raygen_group_ci.anyHitShader = VK_SHADER_UNUSED_KHR;
    raygen_group_ci.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineCreateInfoKHR rt_pipeline_create_info{};
    rt_pipeline_create_info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    rt_pipeline_create_info.stageCount = 1;
    rt_pipeline_create_info.pStages = &pipeline_stage_ci;
    rt_pipeline_create_info.groupCount = 1;
    rt_pipeline_create_info.pGroups = &raygen_group_ci;
    rt_pipeline_create_info.maxPipelineRayRecursionDepth = 1;
    rt_pipeline_create_info.layout = shared_resources->pipeline_layout;
    result = DispatchCreateRayTracingPipelinesKHR(device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rt_pipeline_create_info, nullptr,
                                                  &shared_resources->pipeline);

    DispatchDestroyShaderModule(device, validation_shader, nullptr);

    if (result != VK_SUCCESS) {
        InternalError(device, loc, "Failed to create ray tracing pipeline for pre trace rays validation. Aborting GPU-AV");
        return nullptr;
    }

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rt_pipeline_props = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&rt_pipeline_props);
    DispatchGetPhysicalDeviceProperties2(physical_device, &props2);

    // Get shader group handles to fill shader binding table (SBT)
    const uint32_t shader_group_size_aligned =
        Align(rt_pipeline_props.shaderGroupHandleSize, rt_pipeline_props.shaderGroupHandleAlignment);
    const uint32_t sbt_size = 1 * shader_group_size_aligned;
    std::vector<uint8_t> sbt_host_storage(sbt_size);
    result = DispatchGetRayTracingShaderGroupHandlesKHR(device, shared_resources->pipeline, 0, rt_pipeline_create_info.groupCount,
                                                        sbt_size, sbt_host_storage.data());
    if (result != VK_SUCCESS) {
        InternalError(device, loc, "Failed to call vkGetRayTracingShaderGroupHandlesKHR. Aborting GPU-AV");
        return nullptr;
    }

    // Allocate buffer to store SBT, and fill it with sbt_host_storage
    VkBufferCreateInfo buffer_info = vku::InitStructHelper();
    buffer_info.size = 4096;
    buffer_info.usage =
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    uint32_t mem_type_index = 0;
    vmaFindMemoryTypeIndexForBufferInfo(vmaAllocator, &buffer_info, &alloc_info, &mem_type_index);
    VmaPoolCreateInfo pool_create_info = {};
    pool_create_info.memoryTypeIndex = mem_type_index;
    pool_create_info.blockSize = 0;
    pool_create_info.maxBlockCount = 0;
    pool_create_info.flags = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT;
    result = vmaCreatePool(vmaAllocator, &pool_create_info, &shared_resources->sbt_pool);
    if (result != VK_SUCCESS) {
        InternalError(device, loc, "Unable to create VMA memory pool for SBT. Aborting GPU-AV");
        return nullptr;
    }

    alloc_info.pool = shared_resources->sbt_pool;
    result = vmaCreateBuffer(vmaAllocator, &buffer_info, &alloc_info, &shared_resources->sbt_buffer,
                             &shared_resources->sbt_allocation, nullptr);
    if (result != VK_SUCCESS) {
        InternalError(device, loc, "Unable to allocate device memory for shader binding table. Aborting GPU-AV.", true);
        return nullptr;
    }

    uint8_t *mapped_sbt = nullptr;
    result = vmaMapMemory(vmaAllocator, shared_resources->sbt_allocation, reinterpret_cast<void **>(&mapped_sbt));

    if (result != VK_SUCCESS) {
        InternalError(device, loc,
                      "Failed to map shader binding table when creating trace rays validation resources. Aborting GPU-AV", true);
        return nullptr;
    }

    std::memcpy(mapped_sbt, sbt_host_storage.data(), rt_pipeline_props.shaderGroupHandleSize);

    vmaUnmapMemory(vmaAllocator, shared_resources->sbt_allocation);

    shared_resources->shader_group_handle_size_aligned = shader_group_size_aligned;

    // Retrieve SBT address
    const VkDeviceAddress sbt_address = GetBufferDeviceAddress(shared_resources->sbt_buffer, loc);
    assert(sbt_address != 0);
    if (sbt_address == 0) {
        InternalError(device, loc, "Retrieved SBT buffer device address is null. Aborting GPU-AV.");
        return nullptr;
    }
    assert(sbt_address == Align(sbt_address, static_cast<VkDeviceAddress>(rt_pipeline_props.shaderGroupBaseAlignment)));
    shared_resources->sbt_address = sbt_address;

    const auto elt =
        shared_validation_resources_map.insert({typeid(PreTraceRaysResources::SharedResources), std::move(shared_resources)});

    assert(elt.second);

    return reinterpret_cast<PreTraceRaysResources::SharedResources *>(elt.first->second.get());
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

void PreTraceRaysResources::Destroy(Validator &validator) { CommandResources::Destroy(validator); }

}  // namespace gpuav
