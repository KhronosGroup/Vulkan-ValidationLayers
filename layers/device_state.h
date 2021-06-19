/* Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (C) 2015-2021 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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
 *
 * Author: Courtney Goeltzenleuchter <courtneygo@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 * Author: Tobias Hector <tobias.hector@amd.com>
 */
#pragma once
#include "base_node.h"
#include "layer_chassis_dispatch.h"
#include <vector>

struct DeviceFeatures {
    VkPhysicalDeviceFeatures core;
    VkPhysicalDeviceVulkan11Features core11;
    VkPhysicalDeviceVulkan12Features core12;

#if defined(VK_NV_scissor_exclusive)
    VkPhysicalDeviceExclusiveScissorFeaturesNV exclusive_scissor_features;
#endif
#if defined(VK_NV_shading_rate_image)
    VkPhysicalDeviceShadingRateImageFeaturesNV shading_rate_image_features;
#endif
#if defined(VK_NV_mesh_shader)
    VkPhysicalDeviceMeshShaderFeaturesNV mesh_shader_features;
#endif
#if defined(VK_EXT_inline_uniform_block)
    VkPhysicalDeviceInlineUniformBlockFeaturesEXT inline_uniform_block_features;
#endif
#if defined(VK_EXT_transform_feedback)
    VkPhysicalDeviceTransformFeedbackFeaturesEXT transform_feedback_features;
#endif
    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT vtx_attrib_divisor_features;
#if defined(VK_EXT_buffer_device_address)
    VkPhysicalDeviceBufferDeviceAddressFeaturesEXT buffer_device_address_ext_features;
#endif
#if defined(VK_NV_cooperative_matrix)
    VkPhysicalDeviceCooperativeMatrixFeaturesNV cooperative_matrix_features;
#endif
#if defined(VK_NV_compute_shader_derivatives)
    VkPhysicalDeviceComputeShaderDerivativesFeaturesNV compute_shader_derivatives_features;
#endif
#if defined(VK_NV_fragment_shader_barycentric)
    VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV fragment_shader_barycentric_features;
#endif
#if defined(VK_NV_shader_image_footprint)
    VkPhysicalDeviceShaderImageFootprintFeaturesNV shader_image_footprint_features;
#endif
    VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT fragment_shader_interlock_features;
    VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT demote_to_helper_invocation_features;
    VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT texel_buffer_alignment_features;
#if defined(VK_KHR_pipeline_executable_properties)
    VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR pipeline_exe_props_features;
#endif
#if defined(VK_NV_dedicated_allocation_image_aliasing)
    VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV dedicated_allocation_image_aliasing_features;
#endif
    VkPhysicalDevicePerformanceQueryFeaturesKHR performance_query_features;
#if defined(VK_AMD_device_coherent_memory)
    VkPhysicalDeviceCoherentMemoryFeaturesAMD device_coherent_memory_features;
#endif
    VkPhysicalDeviceYcbcrImageArraysFeaturesEXT ycbcr_image_array_features;
#if defined(VK_KHR_ray_query)
    VkPhysicalDeviceRayQueryFeaturesKHR ray_query_features;
#endif
#if defined(VK_KHR_ray_tracing_pipeline)
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_pipeline_features;
#endif
#if defined(VK_KHR_acceleration_structure)
    VkPhysicalDeviceAccelerationStructureFeaturesKHR ray_tracing_acceleration_structure_features;
#endif
    VkPhysicalDeviceRobustness2FeaturesEXT robustness2_features;
#if defined(VK_EXT_fragment_density_map)
    VkPhysicalDeviceFragmentDensityMapFeaturesEXT fragment_density_map_features;
#endif
#if defined(VK_EXT_fragment_density_map2)
    VkPhysicalDeviceFragmentDensityMap2FeaturesEXT fragment_density_map2_features;
#endif
    VkPhysicalDeviceASTCDecodeFeaturesEXT astc_decode_features;
    VkPhysicalDeviceCustomBorderColorFeaturesEXT custom_border_color_features;
#if defined(VK_EXT_pipeline_creation_cache_control)
    VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT pipeline_creation_cache_control_features;
#endif
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state_features;
    VkPhysicalDeviceMultiviewFeatures multiview_features;
#if defined(VK_KHR_portability_subset)
    VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_subset_features;
#endif
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragment_shading_rate_features;
#if defined(VK_INTEL_shader_integer_functions2)
    VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL shader_integer_functions2_features;
#endif
#if defined(VK_NV_shader_sm_builtins)
    VkPhysicalDeviceShaderSMBuiltinsFeaturesNV shader_sm_builtins_features;
#endif
    VkPhysicalDeviceShaderAtomicFloatFeaturesEXT shader_atomic_float_features;
    VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT shader_image_atomic_int64_features;
    VkPhysicalDeviceShaderClockFeaturesKHR shader_clock_features;
#if defined(VK_EXT_conditional_rendering)
    VkPhysicalDeviceConditionalRenderingFeaturesEXT conditional_rendering_features;
#endif
#if defined(VK_KHR_workgroup_memory_explicit_layout)
    VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR workgroup_memory_explicit_layout_features;
#endif
    VkPhysicalDeviceSynchronization2FeaturesKHR synchronization2_features;
    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extended_dynamic_state2_features;
    VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT vertex_input_dynamic_state_features;
#if defined(VK_NV_inherited_viewport_scissor)
    VkPhysicalDeviceInheritedViewportScissorFeaturesNV inherited_viewport_scissor_features;
#endif
#if defined(VK_EXT_provoking_vertex)
    VkPhysicalDeviceProvokingVertexFeaturesEXT provoking_vertex_features;
#endif
#if defined(VK_EXT_multi_draw)
    VkPhysicalDeviceMultiDrawFeaturesEXT multi_draw_features;
#endif
    VkPhysicalDeviceColorWriteEnableFeaturesEXT color_write_features;
#if defined(VK_EXT_shader_atomic_float2)
    VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT shader_atomic_float2_features;
#endif
#if defined(VK_KHR_present_id)
    VkPhysicalDevicePresentIdFeaturesKHR present_id_features;
#endif
#if defined(VK_KHR_present_wait)
    VkPhysicalDevicePresentWaitFeaturesKHR present_wait_features;
#endif
#if defined(VK_NV_ray_tracing_motion_blur)
    VkPhysicalDeviceRayTracingMotionBlurFeaturesNV ray_tracing_motion_blur_features;
    VkPhysicalDeviceShaderIntegerDotProductFeaturesKHR shader_integer_dot_product_features;
    VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT primitive_topology_list_restart_features;
#endif
    VkPhysicalDeviceSubgroupSizeControlFeaturesEXT subgroup_size_control_features;
#if !defined(VULKANSC)
    VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT rgba10x6_formats_features;
    VkPhysicalDeviceMaintenance4FeaturesKHR maintenance4_features;
    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_features;
#endif
    // If a new feature is added here that involves a SPIR-V capability add also in spirv_validation_generator.py
    // This is known by checking the table in the spec or if the struct is in a <spirvcapability> in vk.xml
};

class QUEUE_FAMILY_PERF_COUNTERS {
  public:
    std::vector<VkPerformanceCounterKHR> counters;
};

class PHYSICAL_DEVICE_STATE : public BASE_NODE {
  public:
    uint32_t queue_family_known_count = 1;  // spec implies one QF must always be supported
    const std::vector<VkQueueFamilyProperties> queue_family_properties;
    // TODO These are currently used by CoreChecks, but should probably be refactored
    bool vkGetPhysicalDeviceDisplayPlanePropertiesKHR_called = false;
    uint32_t display_plane_property_count = 0;

    // Map of queue family index to QUEUE_FAMILY_PERF_COUNTERS
    layer_data::unordered_map<uint32_t, std::unique_ptr<QUEUE_FAMILY_PERF_COUNTERS>> perf_counters;

    PHYSICAL_DEVICE_STATE(VkPhysicalDevice phys_dev)
        : BASE_NODE(phys_dev, kVulkanObjectTypePhysicalDevice), queue_family_properties(GetQueueFamilyProps(phys_dev)) {}

    VkPhysicalDevice PhysDev() const { return handle_.Cast<VkPhysicalDevice>(); }

  private:
    const std::vector<VkQueueFamilyProperties> GetQueueFamilyProps(VkPhysicalDevice phys_dev) {
        std::vector<VkQueueFamilyProperties> result;
        uint32_t count;
        DispatchGetPhysicalDeviceQueueFamilyProperties(phys_dev, &count, nullptr);
        result.resize(count);
        DispatchGetPhysicalDeviceQueueFamilyProperties(phys_dev, &count, result.data());
        return result;
    }
};

class DISPLAY_MODE_STATE : public BASE_NODE {
  public:
    const VkPhysicalDevice physical_device;

    DISPLAY_MODE_STATE(VkDisplayModeKHR dm, VkPhysicalDevice phys_dev)
        : BASE_NODE(dm, kVulkanObjectTypeDisplayModeKHR), physical_device(phys_dev) {}

    VkDisplayModeKHR display_mode() const { return handle_.Cast<VkDisplayModeKHR>(); }
};
