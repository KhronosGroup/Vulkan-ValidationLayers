/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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
 */
#pragma once
#include "state_tracker/base_node.h"
#include "layer_chassis_dispatch.h"
#include <vector>

struct DeviceFeatures {
    VkPhysicalDeviceFeatures core;
    VkPhysicalDeviceVulkan11Features core11;
    VkPhysicalDeviceVulkan12Features core12;
    VkPhysicalDeviceVulkan13Features core13;

    VkPhysicalDeviceExclusiveScissorFeaturesNV exclusive_scissor_features;
    VkPhysicalDeviceShadingRateImageFeaturesNV shading_rate_image_features;
    VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_features;
    VkPhysicalDeviceDescriptorBufferFeaturesEXT descriptor_buffer_features;
    VkPhysicalDeviceTransformFeedbackFeaturesEXT transform_feedback_features;
    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT vtx_attrib_divisor_features;
    VkPhysicalDeviceBufferDeviceAddressFeaturesEXT buffer_device_address_ext_features;
    VkPhysicalDeviceCooperativeMatrixFeaturesNV cooperative_matrix_features;
    VkPhysicalDeviceComputeShaderDerivativesFeaturesNV compute_shader_derivatives_features;
    VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV fragment_shader_barycentric_features;
    VkPhysicalDeviceShaderImageFootprintFeaturesNV shader_image_footprint_features;
    VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT fragment_shader_interlock_features;
    VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT demote_to_helper_invocation_features;
    VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT texel_buffer_alignment_features;
    VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR pipeline_exe_props_features;
    VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV dedicated_allocation_image_aliasing_features;
    VkPhysicalDevicePerformanceQueryFeaturesKHR performance_query_features;
    VkPhysicalDeviceCoherentMemoryFeaturesAMD device_coherent_memory_features;
    VkPhysicalDeviceYcbcrImageArraysFeaturesEXT ycbcr_image_array_features;
    VkPhysicalDeviceRayQueryFeaturesKHR ray_query_features;
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_pipeline_features;
    VkPhysicalDeviceAccelerationStructureFeaturesKHR ray_tracing_acceleration_structure_features;
    VkPhysicalDeviceRobustness2FeaturesEXT robustness2_features;
    VkPhysicalDeviceFragmentDensityMapFeaturesEXT fragment_density_map_features;
    VkPhysicalDeviceFragmentDensityMap2FeaturesEXT fragment_density_map2_features;
    VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM fragment_density_map_offset_features;
    VkPhysicalDeviceASTCDecodeFeaturesEXT astc_decode_features;
    VkPhysicalDeviceCustomBorderColorFeaturesEXT custom_border_color_features;
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state_features;
    VkPhysicalDeviceMultiviewFeatures multiview_features;
    VkPhysicalDevicePortabilitySubsetFeaturesKHR portability_subset_features;
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragment_shading_rate_features;
    VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV fragment_shading_rate_enums_features;
    VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL shader_integer_functions2_features;
    VkPhysicalDeviceShaderSMBuiltinsFeaturesNV shader_sm_builtins_features;
    VkPhysicalDeviceShaderAtomicFloatFeaturesEXT shader_atomic_float_features;
    VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT shader_image_atomic_int64_features;
    VkPhysicalDeviceShaderClockFeaturesKHR shader_clock_features;
    VkPhysicalDeviceConditionalRenderingFeaturesEXT conditional_rendering_features;
    VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR workgroup_memory_explicit_layout_features;
    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extended_dynamic_state2_features;
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extended_dynamic_state3_features;
    VkPhysicalDeviceDepthClipEnableFeaturesEXT depth_clip_enable_features;
    VkPhysicalDeviceDepthClipControlFeaturesEXT depth_clip_control_features;
    VkPhysicalDeviceLineRasterizationFeaturesEXT line_rasterization_features;
    VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT vertex_input_dynamic_state_features;
    VkPhysicalDeviceInheritedViewportScissorFeaturesNV inherited_viewport_scissor_features;
    VkPhysicalDeviceProvokingVertexFeaturesEXT provoking_vertex_features;
    VkPhysicalDeviceMultiDrawFeaturesEXT multi_draw_features;
    VkPhysicalDeviceColorWriteEnableFeaturesEXT color_write_features;
    VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT shader_atomic_float2_features;
    VkPhysicalDevicePresentIdFeaturesKHR present_id_features;
    VkPhysicalDevicePresentWaitFeaturesKHR present_wait_features;
    VkPhysicalDeviceRayTracingMotionBlurFeaturesNV ray_tracing_motion_blur_features;
    VkPhysicalDeviceShaderIntegerDotProductFeaturesKHR shader_integer_dot_product_features;
    VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT primitive_topology_list_restart_features;
    VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT rgba10x6_formats_features;
    VkPhysicalDeviceImageViewMinLodFeaturesEXT image_view_min_lod_features;
    VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT primitives_generated_query_features;
    VkPhysicalDeviceImage2DViewOf3DFeaturesEXT image_2d_view_of_3d_features;
    VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT graphics_pipeline_library_features;
    VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR shader_subgroup_uniform_control_flow_features;
    VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR ray_tracing_maintenance1_features;
    VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT non_seamless_cube_map_features;
    VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT multisampled_render_to_single_sampled_features;
    VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT shader_module_identifier_features;
    VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT attachment_feedback_loop_layout_features;
    VkPhysicalDevicePipelineProtectedAccessFeaturesEXT pipeline_protected_access_features;
    VkPhysicalDeviceLinearColorAttachmentFeaturesNV linear_color_attachment_features;
    VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM shader_core_builtins_features;
    VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT pipeline_library_group_handles_features;
    // If a new feature is added here that involves a SPIR-V capability add also in spirv_validation_generator.py
    // This is known by checking the table in the spec or if the struct is in a <spirvcapability> in vk.xml
};

class QUEUE_FAMILY_PERF_COUNTERS {
  public:
    std::vector<VkPerformanceCounterKHR> counters;
};

class SURFACELESS_QUERY_STATE {
  public:
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
    VkSurfaceCapabilitiesKHR capabilities;
};

class PHYSICAL_DEVICE_STATE : public BASE_NODE {
  public:
    uint32_t queue_family_known_count = 1;  // spec implies one QF must always be supported
    const std::vector<VkQueueFamilyProperties> queue_family_properties;
    // TODO These are currently used by CoreChecks, but should probably be refactored
    bool vkGetPhysicalDeviceDisplayPlanePropertiesKHR_called = false;
    uint32_t display_plane_property_count = 0;

    // Map of queue family index to QUEUE_FAMILY_PERF_COUNTERS
    vvl::unordered_map<uint32_t, std::unique_ptr<QUEUE_FAMILY_PERF_COUNTERS>> perf_counters;

    // Surfaceless Query extension needs 'global' surface_state data
    SURFACELESS_QUERY_STATE surfaceless_query_state{};

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
