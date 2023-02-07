// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See spirv_validation_generator.py for modifications


/***************************************************************************
 *
 * Copyright (c) 2020-2023 The Khronos Group Inc.
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
 * This file is related to anything that is found in the Vulkan XML related
 * to SPIR-V. Anything related to the SPIR-V grammar belongs in spirv_grammar_helper
 *
 ****************************************************************************/

#include <string>
#include <functional>
#include <spirv/unified1/spirv.hpp>
#include "vk_extension_helper.h"
#include "state_tracker/shader_module.h"
#include "state_tracker/device_state.h"
#include "core_checks/core_validation.h"

struct FeaturePointer {
    // Callable object to test if this feature is enabled in the given aggregate feature struct
    const std::function<VkBool32(const DeviceFeatures &)> IsEnabled;

    // Test if feature pointer is populated
    explicit operator bool() const { return static_cast<bool>(IsEnabled); }

    // Default and nullptr constructor to create an empty FeaturePointer
    FeaturePointer() : IsEnabled(nullptr) {}
    FeaturePointer(std::nullptr_t ptr) : IsEnabled(nullptr) {}

    // Constructors to populate FeaturePointer based on given pointer to member
    FeaturePointer(VkBool32 VkPhysicalDeviceFeatures::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.core.*ptr; }) {}
    FeaturePointer(VkBool32 VkPhysicalDeviceVulkan11Features::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.core11.*ptr; }) {}
    FeaturePointer(VkBool32 VkPhysicalDeviceVulkan12Features::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.core12.*ptr; }) {}
    FeaturePointer(VkBool32 VkPhysicalDeviceVulkan13Features::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.core13.*ptr; }) {}
    FeaturePointer(VkBool32 VkPhysicalDeviceTransformFeedbackFeaturesEXT::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.transform_feedback_features.*ptr; }) {}
    FeaturePointer(VkBool32 VkPhysicalDeviceCooperativeMatrixFeaturesNV::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.cooperative_matrix_features.*ptr; }) {}
    FeaturePointer(VkBool32 VkPhysicalDeviceComputeShaderDerivativesFeaturesNV::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.compute_shader_derivatives_features.*ptr; }) {}
    FeaturePointer(VkBool32 VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.fragment_shader_barycentric_features.*ptr; }) {}
    FeaturePointer(VkBool32 VkPhysicalDeviceShaderImageFootprintFeaturesNV::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.shader_image_footprint_features.*ptr; }) {}
    FeaturePointer(VkBool32 VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.fragment_shader_interlock_features.*ptr; }) {}
    FeaturePointer(VkBool32 VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.demote_to_helper_invocation_features.*ptr; }) {}
    FeaturePointer(VkBool32 VkPhysicalDeviceRayQueryFeaturesKHR::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.ray_query_features.*ptr; }) {}
    FeaturePointer(VkBool32 VkPhysicalDeviceRayTracingPipelineFeaturesKHR::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.ray_tracing_pipeline_features.*ptr; }) {}
    FeaturePointer(VkBool32 VkPhysicalDeviceAccelerationStructureFeaturesKHR::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.ray_tracing_acceleration_structure_features.*ptr; }) {}
    FeaturePointer(VkBool32 VkPhysicalDeviceFragmentDensityMapFeaturesEXT::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.fragment_density_map_features.*ptr; }) {}
    FeaturePointer(VkBool32 VkPhysicalDeviceBufferDeviceAddressFeaturesEXT::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.buffer_device_address_ext_features.*ptr; }) {}
    FeaturePointer(VkBool32 VkPhysicalDeviceFragmentShadingRateFeaturesKHR::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.fragment_shading_rate_features.*ptr; }) {}
    FeaturePointer(VkBool32 VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.shader_integer_functions2_features.*ptr; }) {}
    FeaturePointer(VkBool32 VkPhysicalDeviceShaderSMBuiltinsFeaturesNV::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.shader_sm_builtins_features.*ptr; }) {}
    FeaturePointer(VkBool32 VkPhysicalDeviceShadingRateImageFeaturesNV::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.shading_rate_image_features.*ptr; }) {}
    FeaturePointer(VkBool32 VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.shader_atomic_float_features.*ptr; }) {}
    FeaturePointer(VkBool32 VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.shader_image_atomic_int64_features.*ptr; }) {}
    FeaturePointer(VkBool32 VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.workgroup_memory_explicit_layout_features.*ptr; }) {}
    FeaturePointer(VkBool32 VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.shader_atomic_float2_features.*ptr; }) {}
    FeaturePointer(VkBool32 VkPhysicalDeviceRayTracingMotionBlurFeaturesNV::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.ray_tracing_motion_blur_features.*ptr; }) {}
    FeaturePointer(VkBool32 VkPhysicalDeviceShaderIntegerDotProductFeaturesKHR::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.shader_integer_dot_product_features.*ptr; }) {}
    FeaturePointer(VkBool32 VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.shader_subgroup_uniform_control_flow_features.*ptr; }) {}
    FeaturePointer(VkBool32 VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.ray_tracing_maintenance1_features.*ptr; }) {}
    FeaturePointer(VkBool32 VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM::*ptr)
        : IsEnabled([=](const DeviceFeatures &features) { return features.shader_core_builtins_features.*ptr; }) {}
};

// Each instance of the struct will only have a singel field non-null
struct RequiredSpirvInfo {
    uint32_t version;
    FeaturePointer feature;
    ExtEnabled DeviceExtensions::*extension;
    const char* property; // For human readability and make some capabilities unique
};

// clang-format off
static const std::unordered_multimap<uint32_t, RequiredSpirvInfo> spirvCapabilities = {
    {spv::CapabilityAtomicFloat16AddEXT, {0, &VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderBufferFloat16AtomicAdd, nullptr, ""}},
    {spv::CapabilityAtomicFloat16AddEXT, {0, &VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderSharedFloat16AtomicAdd, nullptr, ""}},
    {spv::CapabilityAtomicFloat16MinMaxEXT, {0, &VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderBufferFloat16AtomicMinMax, nullptr, ""}},
    {spv::CapabilityAtomicFloat16MinMaxEXT, {0, &VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderSharedFloat16AtomicMinMax, nullptr, ""}},
    {spv::CapabilityAtomicFloat32AddEXT, {0, &VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderBufferFloat32AtomicAdd, nullptr, ""}},
    {spv::CapabilityAtomicFloat32AddEXT, {0, &VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderSharedFloat32AtomicAdd, nullptr, ""}},
    {spv::CapabilityAtomicFloat32AddEXT, {0, &VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderImageFloat32AtomicAdd, nullptr, ""}},
    {spv::CapabilityAtomicFloat32MinMaxEXT, {0, &VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderBufferFloat32AtomicMinMax, nullptr, ""}},
    {spv::CapabilityAtomicFloat32MinMaxEXT, {0, &VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderSharedFloat32AtomicMinMax, nullptr, ""}},
    {spv::CapabilityAtomicFloat32MinMaxEXT, {0, &VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderImageFloat32AtomicMinMax, nullptr, ""}},
    {spv::CapabilityAtomicFloat64AddEXT, {0, &VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderBufferFloat64AtomicAdd, nullptr, ""}},
    {spv::CapabilityAtomicFloat64AddEXT, {0, &VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderSharedFloat64AtomicAdd, nullptr, ""}},
    {spv::CapabilityAtomicFloat64MinMaxEXT, {0, &VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderBufferFloat64AtomicMinMax, nullptr, ""}},
    {spv::CapabilityAtomicFloat64MinMaxEXT, {0, &VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderSharedFloat64AtomicMinMax, nullptr, ""}},
    {spv::CapabilityClipDistance, {0, &VkPhysicalDeviceFeatures::shaderClipDistance, nullptr, ""}},
    // Not found in current SPIR-V Headers
    //    {spv::CapabilityClusterCullingShadingHUAWEI, {0, &VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI::clustercullingShader, nullptr, ""}},
    {spv::CapabilityComputeDerivativeGroupLinearNV, {0, &VkPhysicalDeviceComputeShaderDerivativesFeaturesNV::computeDerivativeGroupLinear, nullptr, ""}},
    {spv::CapabilityComputeDerivativeGroupQuadsNV, {0, &VkPhysicalDeviceComputeShaderDerivativesFeaturesNV::computeDerivativeGroupQuads, nullptr, ""}},
    {spv::CapabilityCooperativeMatrixNV, {0, &VkPhysicalDeviceCooperativeMatrixFeaturesNV::cooperativeMatrix, nullptr, ""}},
    {spv::CapabilityCoreBuiltinsARM, {0, &VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM::shaderCoreBuiltins, nullptr, ""}},
    {spv::CapabilityCullDistance, {0, &VkPhysicalDeviceFeatures::shaderCullDistance, nullptr, ""}},
    {spv::CapabilityDemoteToHelperInvocationEXT, {0, &VkPhysicalDeviceVulkan13Features::shaderDemoteToHelperInvocation, nullptr, ""}},
    {spv::CapabilityDemoteToHelperInvocationEXT, {0, &VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT::shaderDemoteToHelperInvocation, nullptr, ""}},
    {spv::CapabilityDenormFlushToZero, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderDenormFlushToZeroFloat16 & VK_TRUE) != 0"}},
    {spv::CapabilityDenormFlushToZero, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderDenormFlushToZeroFloat32 & VK_TRUE) != 0"}},
    {spv::CapabilityDenormFlushToZero, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderDenormFlushToZeroFloat64 & VK_TRUE) != 0"}},
    {spv::CapabilityDenormPreserve, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderDenormPreserveFloat16 & VK_TRUE) != 0"}},
    {spv::CapabilityDenormPreserve, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderDenormPreserveFloat32 & VK_TRUE) != 0"}},
    {spv::CapabilityDenormPreserve, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderDenormPreserveFloat64 & VK_TRUE) != 0"}},
    {spv::CapabilityDerivativeControl, {VK_API_VERSION_1_0, nullptr, nullptr, ""}},
    {spv::CapabilityDeviceGroup, {VK_API_VERSION_1_1, nullptr, nullptr, ""}},
    {spv::CapabilityDeviceGroup, {0, nullptr, &DeviceExtensions::vk_khr_device_group, ""}},
    {spv::CapabilityDotProductInput4x8BitKHR, {0, &VkPhysicalDeviceVulkan13Features::shaderIntegerDotProduct, nullptr, ""}},
    {spv::CapabilityDotProductInput4x8BitKHR, {0, &VkPhysicalDeviceShaderIntegerDotProductFeaturesKHR::shaderIntegerDotProduct, nullptr, ""}},
    {spv::CapabilityDotProductInput4x8BitPackedKHR, {0, &VkPhysicalDeviceVulkan13Features::shaderIntegerDotProduct, nullptr, ""}},
    {spv::CapabilityDotProductInput4x8BitPackedKHR, {0, &VkPhysicalDeviceShaderIntegerDotProductFeaturesKHR::shaderIntegerDotProduct, nullptr, ""}},
    {spv::CapabilityDotProductInputAllKHR, {0, &VkPhysicalDeviceVulkan13Features::shaderIntegerDotProduct, nullptr, ""}},
    {spv::CapabilityDotProductInputAllKHR, {0, &VkPhysicalDeviceShaderIntegerDotProductFeaturesKHR::shaderIntegerDotProduct, nullptr, ""}},
    {spv::CapabilityDotProductKHR, {0, &VkPhysicalDeviceVulkan13Features::shaderIntegerDotProduct, nullptr, ""}},
    {spv::CapabilityDotProductKHR, {0, &VkPhysicalDeviceShaderIntegerDotProductFeaturesKHR::shaderIntegerDotProduct, nullptr, ""}},
    {spv::CapabilityDrawParameters, {0, &VkPhysicalDeviceVulkan11Features::shaderDrawParameters, nullptr, ""}},
    {spv::CapabilityDrawParameters, {0, nullptr, &DeviceExtensions::vk_khr_shader_draw_parameters, ""}},
    {spv::CapabilityFloat16, {0, &VkPhysicalDeviceVulkan12Features::shaderFloat16, nullptr, ""}},
    {spv::CapabilityFloat16, {0, nullptr, &DeviceExtensions::vk_amd_gpu_shader_half_float, ""}},
    {spv::CapabilityFloat64, {0, &VkPhysicalDeviceFeatures::shaderFloat64, nullptr, ""}},
    {spv::CapabilityFragmentBarycentricKHR, {0, &VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR::fragmentShaderBarycentric, nullptr, ""}},
    {spv::CapabilityFragmentBarycentricNV, {0, &VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV::fragmentShaderBarycentric, nullptr, ""}},
    {spv::CapabilityFragmentDensityEXT, {0, &VkPhysicalDeviceFragmentDensityMapFeaturesEXT::fragmentDensityMap, nullptr, ""}},
    {spv::CapabilityFragmentFullyCoveredEXT, {0, nullptr, &DeviceExtensions::vk_ext_conservative_rasterization, ""}},
    {spv::CapabilityFragmentMaskAMD, {0, nullptr, &DeviceExtensions::vk_amd_shader_fragment_mask, ""}},
    {spv::CapabilityFragmentShaderPixelInterlockEXT, {0, &VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::fragmentShaderPixelInterlock, nullptr, ""}},
    {spv::CapabilityFragmentShaderSampleInterlockEXT, {0, &VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::fragmentShaderSampleInterlock, nullptr, ""}},
    {spv::CapabilityFragmentShaderShadingRateInterlockEXT, {0, &VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::fragmentShaderShadingRateInterlock, nullptr, ""}},
    {spv::CapabilityFragmentShaderShadingRateInterlockEXT, {0, &VkPhysicalDeviceShadingRateImageFeaturesNV::shadingRateImage, nullptr, ""}},
    {spv::CapabilityFragmentShadingRateKHR, {0, &VkPhysicalDeviceFragmentShadingRateFeaturesKHR::pipelineFragmentShadingRate, nullptr, ""}},
    {spv::CapabilityFragmentShadingRateKHR, {0, &VkPhysicalDeviceFragmentShadingRateFeaturesKHR::primitiveFragmentShadingRate, nullptr, ""}},
    {spv::CapabilityFragmentShadingRateKHR, {0, &VkPhysicalDeviceFragmentShadingRateFeaturesKHR::attachmentFragmentShadingRate, nullptr, ""}},
    {spv::CapabilityGeometry, {0, &VkPhysicalDeviceFeatures::geometryShader, nullptr, ""}},
    {spv::CapabilityGeometryPointSize, {0, &VkPhysicalDeviceFeatures::shaderTessellationAndGeometryPointSize, nullptr, ""}},
    {spv::CapabilityGeometryShaderPassthroughNV, {0, nullptr, &DeviceExtensions::vk_nv_geometry_shader_passthrough, ""}},
    {spv::CapabilityGeometryStreams, {0, &VkPhysicalDeviceTransformFeedbackFeaturesEXT::geometryStreams, nullptr, ""}},
    {spv::CapabilityGroupNonUniform, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan11Properties::subgroupSupportedOperations & VK_SUBGROUP_FEATURE_BASIC_BIT) != 0"}},
    {spv::CapabilityGroupNonUniformArithmetic, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan11Properties::subgroupSupportedOperations & VK_SUBGROUP_FEATURE_ARITHMETIC_BIT) != 0"}},
    {spv::CapabilityGroupNonUniformBallot, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan11Properties::subgroupSupportedOperations & VK_SUBGROUP_FEATURE_BALLOT_BIT) != 0"}},
    {spv::CapabilityGroupNonUniformClustered, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan11Properties::subgroupSupportedOperations & VK_SUBGROUP_FEATURE_CLUSTERED_BIT) != 0"}},
    {spv::CapabilityGroupNonUniformPartitionedNV, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan11Properties::subgroupSupportedOperations & VK_SUBGROUP_FEATURE_PARTITIONED_BIT_NV) != 0"}},
    {spv::CapabilityGroupNonUniformQuad, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan11Properties::subgroupSupportedOperations & VK_SUBGROUP_FEATURE_QUAD_BIT) != 0"}},
    {spv::CapabilityGroupNonUniformShuffle, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan11Properties::subgroupSupportedOperations & VK_SUBGROUP_FEATURE_SHUFFLE_BIT) != 0"}},
    {spv::CapabilityGroupNonUniformShuffleRelative, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan11Properties::subgroupSupportedOperations & VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT) != 0"}},
    {spv::CapabilityGroupNonUniformVote, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan11Properties::subgroupSupportedOperations & VK_SUBGROUP_FEATURE_VOTE_BIT) != 0"}},
    {spv::CapabilityImage1D, {VK_API_VERSION_1_0, nullptr, nullptr, ""}},
    {spv::CapabilityImageBuffer, {VK_API_VERSION_1_0, nullptr, nullptr, ""}},
    {spv::CapabilityImageCubeArray, {0, &VkPhysicalDeviceFeatures::imageCubeArray, nullptr, ""}},
    {spv::CapabilityImageFootprintNV, {0, &VkPhysicalDeviceShaderImageFootprintFeaturesNV::imageFootprint, nullptr, ""}},
    {spv::CapabilityImageGatherBiasLodAMD, {0, nullptr, &DeviceExtensions::vk_amd_texture_gather_bias_lod, ""}},
    {spv::CapabilityImageGatherExtended, {0, &VkPhysicalDeviceFeatures::shaderImageGatherExtended, nullptr, ""}},
    {spv::CapabilityImageMSArray, {0, &VkPhysicalDeviceFeatures::shaderStorageImageMultisample, nullptr, ""}},
    {spv::CapabilityImageQuery, {VK_API_VERSION_1_0, nullptr, nullptr, ""}},
    {spv::CapabilityImageReadWriteLodAMD, {0, nullptr, &DeviceExtensions::vk_amd_shader_image_load_store_lod, ""}},
    {spv::CapabilityInputAttachment, {VK_API_VERSION_1_0, nullptr, nullptr, ""}},
    {spv::CapabilityInputAttachmentArrayDynamicIndexing, {0, &VkPhysicalDeviceVulkan12Features::shaderInputAttachmentArrayDynamicIndexing, nullptr, ""}},
    {spv::CapabilityInputAttachmentArrayNonUniformIndexing, {0, &VkPhysicalDeviceVulkan12Features::shaderInputAttachmentArrayNonUniformIndexing, nullptr, ""}},
    {spv::CapabilityInt16, {0, &VkPhysicalDeviceFeatures::shaderInt16, nullptr, ""}},
    {spv::CapabilityInt64, {0, &VkPhysicalDeviceFeatures::shaderInt64, nullptr, ""}},
    {spv::CapabilityInt64Atomics, {0, &VkPhysicalDeviceVulkan12Features::shaderBufferInt64Atomics, nullptr, ""}},
    {spv::CapabilityInt64Atomics, {0, &VkPhysicalDeviceVulkan12Features::shaderSharedInt64Atomics, nullptr, ""}},
    {spv::CapabilityInt64Atomics, {0, &VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT::shaderImageInt64Atomics, nullptr, ""}},
    {spv::CapabilityInt64ImageEXT, {0, &VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT::shaderImageInt64Atomics, nullptr, ""}},
    {spv::CapabilityInt8, {0, &VkPhysicalDeviceVulkan12Features::shaderInt8, nullptr, ""}},
    {spv::CapabilityIntegerFunctions2INTEL, {0, &VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL::shaderIntegerFunctions2, nullptr, ""}},
    {spv::CapabilityInterpolationFunction, {0, &VkPhysicalDeviceFeatures::sampleRateShading, nullptr, ""}},
    {spv::CapabilityMatrix, {VK_API_VERSION_1_0, nullptr, nullptr, ""}},
    {spv::CapabilityMeshShadingEXT, {0, nullptr, &DeviceExtensions::vk_ext_mesh_shader, ""}},
    {spv::CapabilityMeshShadingNV, {0, nullptr, &DeviceExtensions::vk_nv_mesh_shader, ""}},
    {spv::CapabilityMinLod, {0, &VkPhysicalDeviceFeatures::shaderResourceMinLod, nullptr, ""}},
    {spv::CapabilityMultiView, {0, &VkPhysicalDeviceVulkan11Features::multiview, nullptr, ""}},
    {spv::CapabilityMultiViewport, {0, &VkPhysicalDeviceFeatures::multiViewport, nullptr, ""}},
    {spv::CapabilityPerViewAttributesNV, {0, nullptr, &DeviceExtensions::vk_nvx_multiview_per_view_attributes, ""}},
    {spv::CapabilityPhysicalStorageBufferAddresses, {0, &VkPhysicalDeviceVulkan12Features::bufferDeviceAddress, nullptr, ""}},
    {spv::CapabilityPhysicalStorageBufferAddresses, {0, &VkPhysicalDeviceBufferDeviceAddressFeaturesEXT::bufferDeviceAddress, nullptr, ""}},
    {spv::CapabilityRayCullMaskKHR, {0, &VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR::rayTracingMaintenance1, nullptr, ""}},
    {spv::CapabilityRayQueryKHR, {0, &VkPhysicalDeviceRayQueryFeaturesKHR::rayQuery, nullptr, ""}},
    {spv::CapabilityRayTracingKHR, {0, &VkPhysicalDeviceRayTracingPipelineFeaturesKHR::rayTracingPipeline, nullptr, ""}},
    {spv::CapabilityRayTracingMotionBlurNV, {0, &VkPhysicalDeviceRayTracingMotionBlurFeaturesNV::rayTracingMotionBlur, nullptr, ""}},
    {spv::CapabilityRayTracingNV, {0, nullptr, &DeviceExtensions::vk_nv_ray_tracing, ""}},
    {spv::CapabilityRayTracingOpacityMicromapEXT, {0, nullptr, &DeviceExtensions::vk_ext_opacity_micromap, ""}},
    {spv::CapabilityRayTraversalPrimitiveCullingKHR, {0, &VkPhysicalDeviceRayTracingPipelineFeaturesKHR::rayTraversalPrimitiveCulling, nullptr, ""}},
    {spv::CapabilityRayTraversalPrimitiveCullingKHR, {0, &VkPhysicalDeviceRayQueryFeaturesKHR::rayQuery, nullptr, ""}},
    {spv::CapabilityRoundingModeRTE, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderRoundingModeRTEFloat16 & VK_TRUE) != 0"}},
    {spv::CapabilityRoundingModeRTE, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderRoundingModeRTEFloat32 & VK_TRUE) != 0"}},
    {spv::CapabilityRoundingModeRTE, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderRoundingModeRTEFloat64 & VK_TRUE) != 0"}},
    {spv::CapabilityRoundingModeRTZ, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderRoundingModeRTZFloat16 & VK_TRUE) != 0"}},
    {spv::CapabilityRoundingModeRTZ, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderRoundingModeRTZFloat32 & VK_TRUE) != 0"}},
    {spv::CapabilityRoundingModeRTZ, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderRoundingModeRTZFloat64 & VK_TRUE) != 0"}},
    {spv::CapabilityRuntimeDescriptorArray, {0, &VkPhysicalDeviceVulkan12Features::runtimeDescriptorArray, nullptr, ""}},
    {spv::CapabilitySampleMaskOverrideCoverageNV, {0, nullptr, &DeviceExtensions::vk_nv_sample_mask_override_coverage, ""}},
    {spv::CapabilitySampleMaskPostDepthCoverage, {0, nullptr, &DeviceExtensions::vk_ext_post_depth_coverage, ""}},
    {spv::CapabilitySampleRateShading, {0, &VkPhysicalDeviceFeatures::sampleRateShading, nullptr, ""}},
    {spv::CapabilitySampled1D, {VK_API_VERSION_1_0, nullptr, nullptr, ""}},
    {spv::CapabilitySampledBuffer, {VK_API_VERSION_1_0, nullptr, nullptr, ""}},
    {spv::CapabilitySampledCubeArray, {0, &VkPhysicalDeviceFeatures::imageCubeArray, nullptr, ""}},
    {spv::CapabilitySampledImageArrayDynamicIndexing, {0, &VkPhysicalDeviceFeatures::shaderSampledImageArrayDynamicIndexing, nullptr, ""}},
    {spv::CapabilitySampledImageArrayNonUniformIndexing, {0, &VkPhysicalDeviceVulkan12Features::shaderSampledImageArrayNonUniformIndexing, nullptr, ""}},
    {spv::CapabilityShader, {VK_API_VERSION_1_0, nullptr, nullptr, ""}},
    {spv::CapabilityShaderClockKHR, {0, nullptr, &DeviceExtensions::vk_khr_shader_clock, ""}},
    {spv::CapabilityShaderInvocationReorderNV, {0, nullptr, &DeviceExtensions::vk_nv_ray_tracing_invocation_reorder, ""}},
    {spv::CapabilityShaderLayer, {0, &VkPhysicalDeviceVulkan12Features::shaderOutputLayer, nullptr, ""}},
    {spv::CapabilityShaderNonUniform, {VK_API_VERSION_1_2, nullptr, nullptr, ""}},
    {spv::CapabilityShaderNonUniform, {0, nullptr, &DeviceExtensions::vk_ext_descriptor_indexing, ""}},
    {spv::CapabilityShaderSMBuiltinsNV, {0, &VkPhysicalDeviceShaderSMBuiltinsFeaturesNV::shaderSMBuiltins, nullptr, ""}},
    {spv::CapabilityShaderViewportIndex, {0, &VkPhysicalDeviceVulkan12Features::shaderOutputViewportIndex, nullptr, ""}},
    {spv::CapabilityShaderViewportIndexLayerEXT, {0, nullptr, &DeviceExtensions::vk_ext_shader_viewport_index_layer, ""}},
    {spv::CapabilityShaderViewportIndexLayerNV, {0, nullptr, &DeviceExtensions::vk_nv_viewport_array2, ""}},
    {spv::CapabilityShaderViewportMaskNV, {0, nullptr, &DeviceExtensions::vk_nv_viewport_array2, ""}},
    {spv::CapabilityShadingRateNV, {0, &VkPhysicalDeviceShadingRateImageFeaturesNV::shadingRateImage, nullptr, ""}},
    {spv::CapabilitySignedZeroInfNanPreserve, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderSignedZeroInfNanPreserveFloat16 & VK_TRUE) != 0"}},
    {spv::CapabilitySignedZeroInfNanPreserve, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderSignedZeroInfNanPreserveFloat32 & VK_TRUE) != 0"}},
    {spv::CapabilitySignedZeroInfNanPreserve, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderSignedZeroInfNanPreserveFloat64 & VK_TRUE) != 0"}},
    {spv::CapabilitySparseResidency, {0, &VkPhysicalDeviceFeatures::shaderResourceResidency, nullptr, ""}},
    {spv::CapabilityStencilExportEXT, {0, nullptr, &DeviceExtensions::vk_ext_shader_stencil_export, ""}},
    {spv::CapabilityStorageBuffer16BitAccess, {0, &VkPhysicalDeviceVulkan11Features::storageBuffer16BitAccess, nullptr, ""}},
    {spv::CapabilityStorageBuffer8BitAccess, {0, &VkPhysicalDeviceVulkan12Features::storageBuffer8BitAccess, nullptr, ""}},
    {spv::CapabilityStorageBufferArrayDynamicIndexing, {0, &VkPhysicalDeviceFeatures::shaderStorageBufferArrayDynamicIndexing, nullptr, ""}},
    {spv::CapabilityStorageBufferArrayNonUniformIndexing, {0, &VkPhysicalDeviceVulkan12Features::shaderStorageBufferArrayNonUniformIndexing, nullptr, ""}},
    {spv::CapabilityStorageImageArrayDynamicIndexing, {0, &VkPhysicalDeviceFeatures::shaderStorageImageArrayDynamicIndexing, nullptr, ""}},
    {spv::CapabilityStorageImageArrayNonUniformIndexing, {0, &VkPhysicalDeviceVulkan12Features::shaderStorageImageArrayNonUniformIndexing, nullptr, ""}},
    {spv::CapabilityStorageImageExtendedFormats, {VK_API_VERSION_1_0, nullptr, nullptr, ""}},
    {spv::CapabilityStorageImageMultisample, {0, &VkPhysicalDeviceFeatures::shaderStorageImageMultisample, nullptr, ""}},
    {spv::CapabilityStorageImageReadWithoutFormat, {0, &VkPhysicalDeviceFeatures::shaderStorageImageReadWithoutFormat, nullptr, ""}},
    {spv::CapabilityStorageImageReadWithoutFormat, {VK_API_VERSION_1_3, nullptr, nullptr, ""}},
    {spv::CapabilityStorageImageReadWithoutFormat, {0, nullptr, &DeviceExtensions::vk_khr_format_feature_flags2, ""}},
    {spv::CapabilityStorageImageWriteWithoutFormat, {0, &VkPhysicalDeviceFeatures::shaderStorageImageWriteWithoutFormat, nullptr, ""}},
    {spv::CapabilityStorageImageWriteWithoutFormat, {VK_API_VERSION_1_3, nullptr, nullptr, ""}},
    {spv::CapabilityStorageImageWriteWithoutFormat, {0, nullptr, &DeviceExtensions::vk_khr_format_feature_flags2, ""}},
    {spv::CapabilityStorageInputOutput16, {0, &VkPhysicalDeviceVulkan11Features::storageInputOutput16, nullptr, ""}},
    {spv::CapabilityStoragePushConstant16, {0, &VkPhysicalDeviceVulkan11Features::storagePushConstant16, nullptr, ""}},
    {spv::CapabilityStoragePushConstant8, {0, &VkPhysicalDeviceVulkan12Features::storagePushConstant8, nullptr, ""}},
    {spv::CapabilityStorageTexelBufferArrayDynamicIndexing, {0, &VkPhysicalDeviceVulkan12Features::shaderStorageTexelBufferArrayDynamicIndexing, nullptr, ""}},
    {spv::CapabilityStorageTexelBufferArrayNonUniformIndexing, {0, &VkPhysicalDeviceVulkan12Features::shaderStorageTexelBufferArrayNonUniformIndexing, nullptr, ""}},
    {spv::CapabilitySubgroupBallotKHR, {0, nullptr, &DeviceExtensions::vk_ext_shader_subgroup_ballot, ""}},
    {spv::CapabilitySubgroupVoteKHR, {0, nullptr, &DeviceExtensions::vk_ext_shader_subgroup_vote, ""}},
    {spv::CapabilityTessellation, {0, &VkPhysicalDeviceFeatures::tessellationShader, nullptr, ""}},
    {spv::CapabilityTessellationPointSize, {0, &VkPhysicalDeviceFeatures::shaderTessellationAndGeometryPointSize, nullptr, ""}},
    // Not found in current SPIR-V Headers
    //    {spv::CapabilityTextureBlockMatchQCOM, {0, &VkPhysicalDeviceImageProcessingFeaturesQCOM::textureBlockMatch, nullptr, ""}},
    // Not found in current SPIR-V Headers
    //    {spv::CapabilityTextureBoxFilterQCOM, {0, &VkPhysicalDeviceImageProcessingFeaturesQCOM::textureBoxFilter, nullptr, ""}},
    // Not found in current SPIR-V Headers
    //    {spv::CapabilityTextureSampleWeightedQCOM, {0, &VkPhysicalDeviceImageProcessingFeaturesQCOM::textureSampleWeighted, nullptr, ""}},
    {spv::CapabilityTransformFeedback, {0, &VkPhysicalDeviceTransformFeedbackFeaturesEXT::transformFeedback, nullptr, ""}},
    {spv::CapabilityUniformAndStorageBuffer16BitAccess, {0, &VkPhysicalDeviceVulkan11Features::uniformAndStorageBuffer16BitAccess, nullptr, ""}},
    {spv::CapabilityUniformAndStorageBuffer8BitAccess, {0, &VkPhysicalDeviceVulkan12Features::uniformAndStorageBuffer8BitAccess, nullptr, ""}},
    {spv::CapabilityUniformBufferArrayDynamicIndexing, {0, &VkPhysicalDeviceFeatures::shaderUniformBufferArrayDynamicIndexing, nullptr, ""}},
    {spv::CapabilityUniformBufferArrayNonUniformIndexing, {0, &VkPhysicalDeviceVulkan12Features::shaderUniformBufferArrayNonUniformIndexing, nullptr, ""}},
    {spv::CapabilityUniformTexelBufferArrayDynamicIndexing, {0, &VkPhysicalDeviceVulkan12Features::shaderUniformTexelBufferArrayDynamicIndexing, nullptr, ""}},
    {spv::CapabilityUniformTexelBufferArrayNonUniformIndexing, {0, &VkPhysicalDeviceVulkan12Features::shaderUniformTexelBufferArrayNonUniformIndexing, nullptr, ""}},
    {spv::CapabilityVariablePointers, {0, &VkPhysicalDeviceVulkan11Features::variablePointers, nullptr, ""}},
    {spv::CapabilityVariablePointersStorageBuffer, {0, &VkPhysicalDeviceVulkan11Features::variablePointersStorageBuffer, nullptr, ""}},
    {spv::CapabilityVulkanMemoryModel, {0, &VkPhysicalDeviceVulkan12Features::vulkanMemoryModel, nullptr, ""}},
    {spv::CapabilityVulkanMemoryModelDeviceScope, {0, &VkPhysicalDeviceVulkan12Features::vulkanMemoryModelDeviceScope, nullptr, ""}},
    {spv::CapabilityWorkgroupMemoryExplicitLayout16BitAccessKHR, {0, &VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::workgroupMemoryExplicitLayout16BitAccess, nullptr, ""}},
    {spv::CapabilityWorkgroupMemoryExplicitLayout8BitAccessKHR, {0, &VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::workgroupMemoryExplicitLayout8BitAccess, nullptr, ""}},
    {spv::CapabilityWorkgroupMemoryExplicitLayoutKHR, {0, &VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::workgroupMemoryExplicitLayout, nullptr, ""}},
};
// clang-format on

// clang-format off
static const std::unordered_multimap<std::string, RequiredSpirvInfo> spirvExtensions = {
    {"SPV_AMD_gcn_shader", {0, nullptr, &DeviceExtensions::vk_amd_gcn_shader, ""}},
    {"SPV_AMD_gpu_shader_half_float", {0, nullptr, &DeviceExtensions::vk_amd_gpu_shader_half_float, ""}},
    {"SPV_AMD_gpu_shader_int16", {0, nullptr, &DeviceExtensions::vk_amd_gpu_shader_int16, ""}},
    {"SPV_AMD_shader_ballot", {0, nullptr, &DeviceExtensions::vk_amd_shader_ballot, ""}},
    {"SPV_AMD_shader_early_and_late_fragment_tests", {0, nullptr, &DeviceExtensions::vk_amd_shader_early_and_late_fragment_tests, ""}},
    {"SPV_AMD_shader_explicit_vertex_parameter", {0, nullptr, &DeviceExtensions::vk_amd_shader_explicit_vertex_parameter, ""}},
    {"SPV_AMD_shader_fragment_mask", {0, nullptr, &DeviceExtensions::vk_amd_shader_fragment_mask, ""}},
    {"SPV_AMD_shader_image_load_store_lod", {0, nullptr, &DeviceExtensions::vk_amd_shader_image_load_store_lod, ""}},
    {"SPV_AMD_shader_trinary_minmax", {0, nullptr, &DeviceExtensions::vk_amd_shader_trinary_minmax, ""}},
    {"SPV_AMD_texture_gather_bias_lod", {0, nullptr, &DeviceExtensions::vk_amd_texture_gather_bias_lod, ""}},
    {"SPV_EXT_demote_to_helper_invocation", {VK_API_VERSION_1_3, nullptr, nullptr, ""}},
    {"SPV_EXT_demote_to_helper_invocation", {0, nullptr, &DeviceExtensions::vk_ext_shader_demote_to_helper_invocation, ""}},
    {"SPV_EXT_descriptor_indexing", {VK_API_VERSION_1_2, nullptr, nullptr, ""}},
    {"SPV_EXT_descriptor_indexing", {0, nullptr, &DeviceExtensions::vk_ext_descriptor_indexing, ""}},
    {"SPV_EXT_fragment_invocation_density", {0, nullptr, &DeviceExtensions::vk_ext_fragment_density_map, ""}},
    {"SPV_EXT_fragment_shader_interlock", {0, nullptr, &DeviceExtensions::vk_ext_fragment_shader_interlock, ""}},
    {"SPV_EXT_mesh_shader", {0, nullptr, &DeviceExtensions::vk_ext_mesh_shader, ""}},
    {"SPV_EXT_physical_storage_buffer", {0, nullptr, &DeviceExtensions::vk_ext_buffer_device_address, ""}},
    {"SPV_EXT_shader_atomic_float16_add", {0, nullptr, &DeviceExtensions::vk_ext_shader_atomic_float2, ""}},
    {"SPV_EXT_shader_atomic_float_add", {0, nullptr, &DeviceExtensions::vk_ext_shader_atomic_float, ""}},
    {"SPV_EXT_shader_atomic_float_min_max", {0, nullptr, &DeviceExtensions::vk_ext_shader_atomic_float2, ""}},
    {"SPV_EXT_shader_image_int64", {0, nullptr, &DeviceExtensions::vk_ext_shader_image_atomic_int64, ""}},
    {"SPV_EXT_shader_stencil_export", {0, nullptr, &DeviceExtensions::vk_ext_shader_stencil_export, ""}},
    {"SPV_EXT_shader_viewport_index_layer", {VK_API_VERSION_1_2, nullptr, nullptr, ""}},
    {"SPV_EXT_shader_viewport_index_layer", {0, nullptr, &DeviceExtensions::vk_ext_shader_viewport_index_layer, ""}},
    {"SPV_GOOGLE_decorate_string", {0, nullptr, &DeviceExtensions::vk_google_decorate_string, ""}},
    {"SPV_GOOGLE_hlsl_functionality1", {0, nullptr, &DeviceExtensions::vk_google_hlsl_functionality1, ""}},
    {"SPV_GOOGLE_user_type", {0, nullptr, &DeviceExtensions::vk_google_user_type, ""}},
    {"SPV_INTEL_shader_integer_functions", {0, nullptr, &DeviceExtensions::vk_intel_shader_integer_functions2, ""}},
    {"SPV_KHR_16bit_storage", {VK_API_VERSION_1_1, nullptr, nullptr, ""}},
    {"SPV_KHR_16bit_storage", {0, nullptr, &DeviceExtensions::vk_khr_16bit_storage, ""}},
    {"SPV_KHR_8bit_storage", {VK_API_VERSION_1_2, nullptr, nullptr, ""}},
    {"SPV_KHR_8bit_storage", {0, nullptr, &DeviceExtensions::vk_khr_8bit_storage, ""}},
    {"SPV_KHR_device_group", {VK_API_VERSION_1_1, nullptr, nullptr, ""}},
    {"SPV_KHR_device_group", {0, nullptr, &DeviceExtensions::vk_khr_device_group, ""}},
    {"SPV_KHR_float_controls", {VK_API_VERSION_1_2, nullptr, nullptr, ""}},
    {"SPV_KHR_float_controls", {0, nullptr, &DeviceExtensions::vk_khr_shader_float_controls, ""}},
    {"SPV_KHR_fragment_shader_barycentric", {0, nullptr, &DeviceExtensions::vk_khr_fragment_shader_barycentric, ""}},
    {"SPV_KHR_fragment_shading_rate", {0, nullptr, &DeviceExtensions::vk_khr_fragment_shading_rate, ""}},
    {"SPV_KHR_integer_dot_product", {VK_API_VERSION_1_3, nullptr, nullptr, ""}},
    {"SPV_KHR_integer_dot_product", {0, nullptr, &DeviceExtensions::vk_khr_shader_integer_dot_product, ""}},
    {"SPV_KHR_multiview", {VK_API_VERSION_1_1, nullptr, nullptr, ""}},
    {"SPV_KHR_multiview", {0, nullptr, &DeviceExtensions::vk_khr_multiview, ""}},
    {"SPV_KHR_non_semantic_info", {VK_API_VERSION_1_3, nullptr, nullptr, ""}},
    {"SPV_KHR_non_semantic_info", {0, nullptr, &DeviceExtensions::vk_khr_shader_non_semantic_info, ""}},
    {"SPV_KHR_physical_storage_buffer", {VK_API_VERSION_1_2, nullptr, nullptr, ""}},
    {"SPV_KHR_physical_storage_buffer", {0, nullptr, &DeviceExtensions::vk_khr_buffer_device_address, ""}},
    {"SPV_KHR_post_depth_coverage", {0, nullptr, &DeviceExtensions::vk_ext_post_depth_coverage, ""}},
    {"SPV_KHR_ray_cull_mask", {0, nullptr, &DeviceExtensions::vk_khr_ray_tracing_maintenance1, ""}},
    {"SPV_KHR_ray_query", {0, nullptr, &DeviceExtensions::vk_khr_ray_query, ""}},
    {"SPV_KHR_ray_tracing", {0, nullptr, &DeviceExtensions::vk_khr_ray_tracing_pipeline, ""}},
    {"SPV_KHR_shader_ballot", {0, nullptr, &DeviceExtensions::vk_ext_shader_subgroup_ballot, ""}},
    {"SPV_KHR_shader_clock", {0, nullptr, &DeviceExtensions::vk_khr_shader_clock, ""}},
    {"SPV_KHR_shader_draw_parameters", {VK_API_VERSION_1_1, nullptr, nullptr, ""}},
    {"SPV_KHR_shader_draw_parameters", {0, nullptr, &DeviceExtensions::vk_khr_shader_draw_parameters, ""}},
    {"SPV_KHR_storage_buffer_storage_class", {VK_API_VERSION_1_1, nullptr, nullptr, ""}},
    {"SPV_KHR_storage_buffer_storage_class", {0, nullptr, &DeviceExtensions::vk_khr_storage_buffer_storage_class, ""}},
    {"SPV_KHR_subgroup_uniform_control_flow", {VK_API_VERSION_1_3, nullptr, nullptr, ""}},
    {"SPV_KHR_subgroup_uniform_control_flow", {0, nullptr, &DeviceExtensions::vk_khr_shader_subgroup_uniform_control_flow, ""}},
    {"SPV_KHR_subgroup_vote", {0, nullptr, &DeviceExtensions::vk_ext_shader_subgroup_vote, ""}},
    {"SPV_KHR_terminate_invocation", {VK_API_VERSION_1_3, nullptr, nullptr, ""}},
    {"SPV_KHR_terminate_invocation", {0, nullptr, &DeviceExtensions::vk_khr_shader_terminate_invocation, ""}},
    {"SPV_KHR_variable_pointers", {VK_API_VERSION_1_1, nullptr, nullptr, ""}},
    {"SPV_KHR_variable_pointers", {0, nullptr, &DeviceExtensions::vk_khr_variable_pointers, ""}},
    {"SPV_KHR_vulkan_memory_model", {VK_API_VERSION_1_2, nullptr, nullptr, ""}},
    {"SPV_KHR_vulkan_memory_model", {0, nullptr, &DeviceExtensions::vk_khr_vulkan_memory_model, ""}},
    {"SPV_KHR_workgroup_memory_explicit_layout", {0, nullptr, &DeviceExtensions::vk_khr_workgroup_memory_explicit_layout, ""}},
    {"SPV_NVX_multiview_per_view_attributes", {0, nullptr, &DeviceExtensions::vk_nvx_multiview_per_view_attributes, ""}},
    {"SPV_NV_compute_shader_derivatives", {0, nullptr, &DeviceExtensions::vk_nv_compute_shader_derivatives, ""}},
    {"SPV_NV_cooperative_matrix", {0, nullptr, &DeviceExtensions::vk_nv_cooperative_matrix, ""}},
    {"SPV_NV_fragment_shader_barycentric", {0, nullptr, &DeviceExtensions::vk_nv_fragment_shader_barycentric, ""}},
    {"SPV_NV_geometry_shader_passthrough", {0, nullptr, &DeviceExtensions::vk_nv_geometry_shader_passthrough, ""}},
    {"SPV_NV_mesh_shader", {0, nullptr, &DeviceExtensions::vk_nv_mesh_shader, ""}},
    {"SPV_NV_ray_tracing", {0, nullptr, &DeviceExtensions::vk_nv_ray_tracing, ""}},
    {"SPV_NV_sample_mask_override_coverage", {0, nullptr, &DeviceExtensions::vk_nv_sample_mask_override_coverage, ""}},
    {"SPV_NV_shader_image_footprint", {0, nullptr, &DeviceExtensions::vk_nv_shader_image_footprint, ""}},
    {"SPV_NV_shader_invocation_reorder", {0, nullptr, &DeviceExtensions::vk_nv_ray_tracing_invocation_reorder, ""}},
    {"SPV_NV_shader_sm_builtins", {0, nullptr, &DeviceExtensions::vk_nv_shader_sm_builtins, ""}},
    {"SPV_NV_shader_subgroup_partitioned", {0, nullptr, &DeviceExtensions::vk_nv_shader_subgroup_partitioned, ""}},
    {"SPV_NV_shading_rate", {0, nullptr, &DeviceExtensions::vk_nv_shading_rate_image, ""}},
    {"SPV_NV_viewport_array2", {0, nullptr, &DeviceExtensions::vk_nv_viewport_array2, ""}},
    {"SPV_QCOM_image_processing", {0, nullptr, &DeviceExtensions::vk_qcom_image_processing, ""}},
};
// clang-format on

static inline const char* string_SpvCapability(uint32_t input_value) {
    switch ((spv::Capability)input_value) {
         case spv::CapabilityAtomicFloat16AddEXT:
            return "AtomicFloat16AddEXT";
         case spv::CapabilityAtomicFloat16MinMaxEXT:
            return "AtomicFloat16MinMaxEXT";
         case spv::CapabilityAtomicFloat32AddEXT:
            return "AtomicFloat32AddEXT";
         case spv::CapabilityAtomicFloat32MinMaxEXT:
            return "AtomicFloat32MinMaxEXT";
         case spv::CapabilityAtomicFloat64AddEXT:
            return "AtomicFloat64AddEXT";
         case spv::CapabilityAtomicFloat64MinMaxEXT:
            return "AtomicFloat64MinMaxEXT";
         case spv::CapabilityClipDistance:
            return "ClipDistance";
         case spv::CapabilityComputeDerivativeGroupLinearNV:
            return "ComputeDerivativeGroupLinearNV";
         case spv::CapabilityComputeDerivativeGroupQuadsNV:
            return "ComputeDerivativeGroupQuadsNV";
         case spv::CapabilityCooperativeMatrixNV:
            return "CooperativeMatrixNV";
         case spv::CapabilityCoreBuiltinsARM:
            return "CoreBuiltinsARM";
         case spv::CapabilityCullDistance:
            return "CullDistance";
         case spv::CapabilityDemoteToHelperInvocationEXT:
            return "DemoteToHelperInvocationEXT";
         case spv::CapabilityDenormFlushToZero:
            return "DenormFlushToZero";
         case spv::CapabilityDenormPreserve:
            return "DenormPreserve";
         case spv::CapabilityDerivativeControl:
            return "DerivativeControl";
         case spv::CapabilityDeviceGroup:
            return "DeviceGroup";
         case spv::CapabilityDotProductInput4x8BitKHR:
            return "DotProductInput4x8BitKHR";
         case spv::CapabilityDotProductInput4x8BitPackedKHR:
            return "DotProductInput4x8BitPackedKHR";
         case spv::CapabilityDotProductInputAllKHR:
            return "DotProductInputAllKHR";
         case spv::CapabilityDotProductKHR:
            return "DotProductKHR";
         case spv::CapabilityDrawParameters:
            return "DrawParameters";
         case spv::CapabilityFloat16:
            return "Float16";
         case spv::CapabilityFloat64:
            return "Float64";
         case spv::CapabilityFragmentBarycentricKHR:
            return "FragmentBarycentricKHR";
         case spv::CapabilityFragmentDensityEXT:
            return "FragmentDensityEXT";
         case spv::CapabilityFragmentFullyCoveredEXT:
            return "FragmentFullyCoveredEXT";
         case spv::CapabilityFragmentMaskAMD:
            return "FragmentMaskAMD";
         case spv::CapabilityFragmentShaderPixelInterlockEXT:
            return "FragmentShaderPixelInterlockEXT";
         case spv::CapabilityFragmentShaderSampleInterlockEXT:
            return "FragmentShaderSampleInterlockEXT";
         case spv::CapabilityFragmentShaderShadingRateInterlockEXT:
            return "FragmentShaderShadingRateInterlockEXT";
         case spv::CapabilityFragmentShadingRateKHR:
            return "FragmentShadingRateKHR";
         case spv::CapabilityGeometry:
            return "Geometry";
         case spv::CapabilityGeometryPointSize:
            return "GeometryPointSize";
         case spv::CapabilityGeometryShaderPassthroughNV:
            return "GeometryShaderPassthroughNV";
         case spv::CapabilityGeometryStreams:
            return "GeometryStreams";
         case spv::CapabilityGroupNonUniform:
            return "GroupNonUniform";
         case spv::CapabilityGroupNonUniformArithmetic:
            return "GroupNonUniformArithmetic";
         case spv::CapabilityGroupNonUniformBallot:
            return "GroupNonUniformBallot";
         case spv::CapabilityGroupNonUniformClustered:
            return "GroupNonUniformClustered";
         case spv::CapabilityGroupNonUniformPartitionedNV:
            return "GroupNonUniformPartitionedNV";
         case spv::CapabilityGroupNonUniformQuad:
            return "GroupNonUniformQuad";
         case spv::CapabilityGroupNonUniformShuffle:
            return "GroupNonUniformShuffle";
         case spv::CapabilityGroupNonUniformShuffleRelative:
            return "GroupNonUniformShuffleRelative";
         case spv::CapabilityGroupNonUniformVote:
            return "GroupNonUniformVote";
         case spv::CapabilityImage1D:
            return "Image1D";
         case spv::CapabilityImageBuffer:
            return "ImageBuffer";
         case spv::CapabilityImageCubeArray:
            return "ImageCubeArray";
         case spv::CapabilityImageFootprintNV:
            return "ImageFootprintNV";
         case spv::CapabilityImageGatherBiasLodAMD:
            return "ImageGatherBiasLodAMD";
         case spv::CapabilityImageGatherExtended:
            return "ImageGatherExtended";
         case spv::CapabilityImageMSArray:
            return "ImageMSArray";
         case spv::CapabilityImageQuery:
            return "ImageQuery";
         case spv::CapabilityImageReadWriteLodAMD:
            return "ImageReadWriteLodAMD";
         case spv::CapabilityInputAttachment:
            return "InputAttachment";
         case spv::CapabilityInputAttachmentArrayDynamicIndexing:
            return "InputAttachmentArrayDynamicIndexing";
         case spv::CapabilityInputAttachmentArrayNonUniformIndexing:
            return "InputAttachmentArrayNonUniformIndexing";
         case spv::CapabilityInt16:
            return "Int16";
         case spv::CapabilityInt64:
            return "Int64";
         case spv::CapabilityInt64Atomics:
            return "Int64Atomics";
         case spv::CapabilityInt64ImageEXT:
            return "Int64ImageEXT";
         case spv::CapabilityInt8:
            return "Int8";
         case spv::CapabilityIntegerFunctions2INTEL:
            return "IntegerFunctions2INTEL";
         case spv::CapabilityInterpolationFunction:
            return "InterpolationFunction";
         case spv::CapabilityMatrix:
            return "Matrix";
         case spv::CapabilityMeshShadingEXT:
            return "MeshShadingEXT";
         case spv::CapabilityMeshShadingNV:
            return "MeshShadingNV";
         case spv::CapabilityMinLod:
            return "MinLod";
         case spv::CapabilityMultiView:
            return "MultiView";
         case spv::CapabilityMultiViewport:
            return "MultiViewport";
         case spv::CapabilityPerViewAttributesNV:
            return "PerViewAttributesNV";
         case spv::CapabilityPhysicalStorageBufferAddresses:
            return "PhysicalStorageBufferAddresses";
         case spv::CapabilityRayCullMaskKHR:
            return "RayCullMaskKHR";
         case spv::CapabilityRayQueryKHR:
            return "RayQueryKHR";
         case spv::CapabilityRayTracingKHR:
            return "RayTracingKHR";
         case spv::CapabilityRayTracingMotionBlurNV:
            return "RayTracingMotionBlurNV";
         case spv::CapabilityRayTracingNV:
            return "RayTracingNV";
         case spv::CapabilityRayTracingOpacityMicromapEXT:
            return "RayTracingOpacityMicromapEXT";
         case spv::CapabilityRayTraversalPrimitiveCullingKHR:
            return "RayTraversalPrimitiveCullingKHR";
         case spv::CapabilityRoundingModeRTE:
            return "RoundingModeRTE";
         case spv::CapabilityRoundingModeRTZ:
            return "RoundingModeRTZ";
         case spv::CapabilityRuntimeDescriptorArray:
            return "RuntimeDescriptorArray";
         case spv::CapabilitySampleMaskOverrideCoverageNV:
            return "SampleMaskOverrideCoverageNV";
         case spv::CapabilitySampleMaskPostDepthCoverage:
            return "SampleMaskPostDepthCoverage";
         case spv::CapabilitySampleRateShading:
            return "SampleRateShading";
         case spv::CapabilitySampled1D:
            return "Sampled1D";
         case spv::CapabilitySampledBuffer:
            return "SampledBuffer";
         case spv::CapabilitySampledCubeArray:
            return "SampledCubeArray";
         case spv::CapabilitySampledImageArrayDynamicIndexing:
            return "SampledImageArrayDynamicIndexing";
         case spv::CapabilitySampledImageArrayNonUniformIndexing:
            return "SampledImageArrayNonUniformIndexing";
         case spv::CapabilityShader:
            return "Shader";
         case spv::CapabilityShaderClockKHR:
            return "ShaderClockKHR";
         case spv::CapabilityShaderInvocationReorderNV:
            return "ShaderInvocationReorderNV";
         case spv::CapabilityShaderLayer:
            return "ShaderLayer";
         case spv::CapabilityShaderNonUniform:
            return "ShaderNonUniform";
         case spv::CapabilityShaderSMBuiltinsNV:
            return "ShaderSMBuiltinsNV";
         case spv::CapabilityShaderViewportIndex:
            return "ShaderViewportIndex";
         case spv::CapabilityShaderViewportIndexLayerEXT:
            return "ShaderViewportIndexLayerEXT";
         case spv::CapabilityShaderViewportMaskNV:
            return "ShaderViewportMaskNV";
         case spv::CapabilitySignedZeroInfNanPreserve:
            return "SignedZeroInfNanPreserve";
         case spv::CapabilitySparseResidency:
            return "SparseResidency";
         case spv::CapabilityStencilExportEXT:
            return "StencilExportEXT";
         case spv::CapabilityStorageBuffer16BitAccess:
            return "StorageBuffer16BitAccess";
         case spv::CapabilityStorageBuffer8BitAccess:
            return "StorageBuffer8BitAccess";
         case spv::CapabilityStorageBufferArrayDynamicIndexing:
            return "StorageBufferArrayDynamicIndexing";
         case spv::CapabilityStorageBufferArrayNonUniformIndexing:
            return "StorageBufferArrayNonUniformIndexing";
         case spv::CapabilityStorageImageArrayDynamicIndexing:
            return "StorageImageArrayDynamicIndexing";
         case spv::CapabilityStorageImageArrayNonUniformIndexing:
            return "StorageImageArrayNonUniformIndexing";
         case spv::CapabilityStorageImageExtendedFormats:
            return "StorageImageExtendedFormats";
         case spv::CapabilityStorageImageMultisample:
            return "StorageImageMultisample";
         case spv::CapabilityStorageImageReadWithoutFormat:
            return "StorageImageReadWithoutFormat";
         case spv::CapabilityStorageImageWriteWithoutFormat:
            return "StorageImageWriteWithoutFormat";
         case spv::CapabilityStorageInputOutput16:
            return "StorageInputOutput16";
         case spv::CapabilityStoragePushConstant16:
            return "StoragePushConstant16";
         case spv::CapabilityStoragePushConstant8:
            return "StoragePushConstant8";
         case spv::CapabilityStorageTexelBufferArrayDynamicIndexing:
            return "StorageTexelBufferArrayDynamicIndexing";
         case spv::CapabilityStorageTexelBufferArrayNonUniformIndexing:
            return "StorageTexelBufferArrayNonUniformIndexing";
         case spv::CapabilitySubgroupBallotKHR:
            return "SubgroupBallotKHR";
         case spv::CapabilitySubgroupVoteKHR:
            return "SubgroupVoteKHR";
         case spv::CapabilityTessellation:
            return "Tessellation";
         case spv::CapabilityTessellationPointSize:
            return "TessellationPointSize";
         case spv::CapabilityTransformFeedback:
            return "TransformFeedback";
         case spv::CapabilityUniformAndStorageBuffer16BitAccess:
            return "UniformAndStorageBuffer16BitAccess";
         case spv::CapabilityUniformAndStorageBuffer8BitAccess:
            return "UniformAndStorageBuffer8BitAccess";
         case spv::CapabilityUniformBufferArrayDynamicIndexing:
            return "UniformBufferArrayDynamicIndexing";
         case spv::CapabilityUniformBufferArrayNonUniformIndexing:
            return "UniformBufferArrayNonUniformIndexing";
         case spv::CapabilityUniformTexelBufferArrayDynamicIndexing:
            return "UniformTexelBufferArrayDynamicIndexing";
         case spv::CapabilityUniformTexelBufferArrayNonUniformIndexing:
            return "UniformTexelBufferArrayNonUniformIndexing";
         case spv::CapabilityVariablePointers:
            return "VariablePointers";
         case spv::CapabilityVariablePointersStorageBuffer:
            return "VariablePointersStorageBuffer";
         case spv::CapabilityVulkanMemoryModel:
            return "VulkanMemoryModel";
         case spv::CapabilityVulkanMemoryModelDeviceScope:
            return "VulkanMemoryModelDeviceScope";
         case spv::CapabilityWorkgroupMemoryExplicitLayout16BitAccessKHR:
            return "WorkgroupMemoryExplicitLayout16BitAccessKHR";
         case spv::CapabilityWorkgroupMemoryExplicitLayout8BitAccessKHR:
            return "WorkgroupMemoryExplicitLayout8BitAccessKHR";
         case spv::CapabilityWorkgroupMemoryExplicitLayoutKHR:
            return "WorkgroupMemoryExplicitLayoutKHR";
        default:
            return "Unhandled OpCapability";
    };
};

bool CoreChecks::ValidateShaderCapabilitiesAndExtensions(const Instruction &insn) const {
    bool skip = false;

    if (insn.Opcode() == spv::OpCapability) {
        // All capabilities are generated so if it is not in the list it is not supported by Vulkan
        if (spirvCapabilities.count(insn.Word(1)) == 0) {
            skip |= LogError(device, "VUID-VkShaderModuleCreateInfo-pCode-01090",
                "vkCreateShaderModule(): A SPIR-V Capability (%s) was declared that is not supported by Vulkan.", string_SpvCapability(insn.Word(1)));
            return skip; // no known capability to validate
        }

        // Each capability has one or more requirements to check
        // Only one item has to be satisfied and an error only occurs
        // when all are not satisfied
        auto caps = spirvCapabilities.equal_range(insn.Word(1));
        bool has_support = false;
        for (auto it = caps.first; (it != caps.second) && (has_support == false); ++it) {
            if (it->second.version) {
                if (api_version >= it->second.version) {
                    has_support = true;
                }
            } else if (it->second.feature) {
                if (it->second.feature.IsEnabled(enabled_features)) {
                    has_support = true;
                }
            } else if (it->second.extension) {
                // kEnabledByApiLevel is not valid as some extension are promoted with feature bits to be used.
                // If the new Api Level gives support, it will be caught in the "it->second.version" check instead.
                if (IsExtEnabledByCreateinfo(device_extensions.*(it->second.extension))) {
                    has_support = true;
                }
            } else if (it->second.property) {
                // support is or'ed as only one has to be supported (if applicable)
                switch (insn.Word(1)) {
                    case spv::CapabilityDenormFlushToZero:
                        has_support |= ((phys_dev_props_core12.shaderDenormFlushToZeroFloat16 & VK_TRUE) != 0);
                        has_support |= ((phys_dev_props_core12.shaderDenormFlushToZeroFloat32 & VK_TRUE) != 0);
                        has_support |= ((phys_dev_props_core12.shaderDenormFlushToZeroFloat64 & VK_TRUE) != 0);
                        break;
                    case spv::CapabilityDenormPreserve:
                        has_support |= ((phys_dev_props_core12.shaderDenormPreserveFloat16 & VK_TRUE) != 0);
                        has_support |= ((phys_dev_props_core12.shaderDenormPreserveFloat32 & VK_TRUE) != 0);
                        has_support |= ((phys_dev_props_core12.shaderDenormPreserveFloat64 & VK_TRUE) != 0);
                        break;
                    case spv::CapabilityGroupNonUniform:
                        has_support |= ((phys_dev_props_core11.subgroupSupportedOperations & VK_SUBGROUP_FEATURE_BASIC_BIT) != 0);
                        break;
                    case spv::CapabilityGroupNonUniformArithmetic:
                        has_support |= ((phys_dev_props_core11.subgroupSupportedOperations & VK_SUBGROUP_FEATURE_ARITHMETIC_BIT) != 0);
                        break;
                    case spv::CapabilityGroupNonUniformBallot:
                        has_support |= ((phys_dev_props_core11.subgroupSupportedOperations & VK_SUBGROUP_FEATURE_BALLOT_BIT) != 0);
                        break;
                    case spv::CapabilityGroupNonUniformClustered:
                        has_support |= ((phys_dev_props_core11.subgroupSupportedOperations & VK_SUBGROUP_FEATURE_CLUSTERED_BIT) != 0);
                        break;
                    case spv::CapabilityGroupNonUniformPartitionedNV:
                        has_support |= ((phys_dev_props_core11.subgroupSupportedOperations & VK_SUBGROUP_FEATURE_PARTITIONED_BIT_NV) != 0);
                        break;
                    case spv::CapabilityGroupNonUniformQuad:
                        has_support |= ((phys_dev_props_core11.subgroupSupportedOperations & VK_SUBGROUP_FEATURE_QUAD_BIT) != 0);
                        break;
                    case spv::CapabilityGroupNonUniformShuffle:
                        has_support |= ((phys_dev_props_core11.subgroupSupportedOperations & VK_SUBGROUP_FEATURE_SHUFFLE_BIT) != 0);
                        break;
                    case spv::CapabilityGroupNonUniformShuffleRelative:
                        has_support |= ((phys_dev_props_core11.subgroupSupportedOperations & VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT) != 0);
                        break;
                    case spv::CapabilityGroupNonUniformVote:
                        has_support |= ((phys_dev_props_core11.subgroupSupportedOperations & VK_SUBGROUP_FEATURE_VOTE_BIT) != 0);
                        break;
                    case spv::CapabilityRoundingModeRTE:
                        has_support |= ((phys_dev_props_core12.shaderRoundingModeRTEFloat16 & VK_TRUE) != 0);
                        has_support |= ((phys_dev_props_core12.shaderRoundingModeRTEFloat32 & VK_TRUE) != 0);
                        has_support |= ((phys_dev_props_core12.shaderRoundingModeRTEFloat64 & VK_TRUE) != 0);
                        break;
                    case spv::CapabilityRoundingModeRTZ:
                        has_support |= ((phys_dev_props_core12.shaderRoundingModeRTZFloat16 & VK_TRUE) != 0);
                        has_support |= ((phys_dev_props_core12.shaderRoundingModeRTZFloat32 & VK_TRUE) != 0);
                        has_support |= ((phys_dev_props_core12.shaderRoundingModeRTZFloat64 & VK_TRUE) != 0);
                        break;
                    case spv::CapabilitySignedZeroInfNanPreserve:
                        has_support |= ((phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat16 & VK_TRUE) != 0);
                        has_support |= ((phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat32 & VK_TRUE) != 0);
                        has_support |= ((phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat64 & VK_TRUE) != 0);
                        break;
                    default:
                        break;
                }
            }
        }

        if (has_support == false) {
            skip |= LogError(device, "VUID-VkShaderModuleCreateInfo-pCode-01091",
                "vkCreateShaderModule(): The SPIR-V Capability (%s) was declared, but none of the requirements were met to use it.", string_SpvCapability(insn.Word(1)));
        }

        // Portability checks
        if (IsExtEnabled(device_extensions.vk_khr_portability_subset)) {
            if ((VK_FALSE == enabled_features.portability_subset_features.shaderSampleRateInterpolationFunctions) &&
                (spv::CapabilityInterpolationFunction == insn.Word(1))) {
                skip |= LogError(device, "VUID-RuntimeSpirv-shaderSampleRateInterpolationFunctions-06325",
                                    "Invalid shader capability (portability error): interpolation functions are not supported "
                                    "by this platform");
            }
        }
    } else if (insn.Opcode() == spv::OpExtension) {
        static const std::string spv_prefix = "SPV_";
        std::string extension_name = insn.GetAsString(1);

        if (0 == extension_name.compare(0, spv_prefix.size(), spv_prefix)) {
            if (spirvExtensions.count(extension_name) == 0) {
                skip |= LogError(device, "VUID-VkShaderModuleCreateInfo-pCode-04146",
                    "vkCreateShaderModule(): A SPIR-V Extension (%s) was declared that is not supported by Vulkan.", extension_name.c_str());
                return skip; // no known extension to validate
            }
        } else {
            skip |= LogError(device, "VUID-VkShaderModuleCreateInfo-pCode-04146",
                "vkCreateShaderModule(): The SPIR-V code uses the '%s' extension which is not a SPIR-V extension. Please use a SPIR-V"
                " extension (https://github.com/KhronosGroup/SPIRV-Registry) for OpExtension instructions. Non-SPIR-V extensions can be"
                " recorded in SPIR-V using the OpSourceExtension instruction.", extension_name.c_str());
            return skip; // no known extension to validate
        }

        // Each SPIR-V Extension has one or more requirements to check
        // Only one item has to be satisfied and an error only occurs
        // when all are not satisfied
        auto ext = spirvExtensions.equal_range(extension_name);
        bool has_support = false;
        for (auto it = ext.first; (it != ext.second) && (has_support == false); ++it) {
            if (it->second.version) {
                if (api_version >= it->second.version) {
                    has_support = true;
                }
            } else if (it->second.feature) {
                if (it->second.feature.IsEnabled(enabled_features)) {
                    has_support = true;
                }
            } else if (it->second.extension) {
                if (IsExtEnabled(device_extensions.*(it->second.extension))) {
                    has_support = true;
                }
            } else if (it->second.property) {
                // support is or'ed as only one has to be supported (if applicable)
                switch (insn.Word(1)) {
                    default:
                        break;
                }
            }
        }

        if (has_support == false) {
            skip |= LogError(device, "VUID-VkShaderModuleCreateInfo-pCode-04147",
                "vkCreateShaderModule(): The SPIR-V Extension (%s) was declared, but none of the requirements were met to use it.", extension_name.c_str());
        }
    } //spv::OpExtension
    return skip;
}

// Will return the Vulkan format for a given SPIR-V image format value
// Note: will return VK_FORMAT_UNDEFINED if non valid input
// This was in vk_format_utils but the SPIR-V Header dependency was an issue
//   see https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/4647
VkFormat CoreChecks::CompatibleSpirvImageFormat(uint32_t spirv_image_format) const {
    switch (spirv_image_format) {
        case spv::ImageFormatRgb10a2ui:
            return VK_FORMAT_A2B10G10R10_UINT_PACK32;
        case spv::ImageFormatRgb10A2:
            return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        case spv::ImageFormatR11fG11fB10f:
            return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        case spv::ImageFormatRgba16f:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case spv::ImageFormatRgba16i:
            return VK_FORMAT_R16G16B16A16_SINT;
        case spv::ImageFormatRgba16Snorm:
            return VK_FORMAT_R16G16B16A16_SNORM;
        case spv::ImageFormatRgba16ui:
            return VK_FORMAT_R16G16B16A16_UINT;
        case spv::ImageFormatRgba16:
            return VK_FORMAT_R16G16B16A16_UNORM;
        case spv::ImageFormatRg16f:
            return VK_FORMAT_R16G16_SFLOAT;
        case spv::ImageFormatRg16i:
            return VK_FORMAT_R16G16_SINT;
        case spv::ImageFormatRg16Snorm:
            return VK_FORMAT_R16G16_SNORM;
        case spv::ImageFormatRg16ui:
            return VK_FORMAT_R16G16_UINT;
        case spv::ImageFormatRg16:
            return VK_FORMAT_R16G16_UNORM;
        case spv::ImageFormatR16f:
            return VK_FORMAT_R16_SFLOAT;
        case spv::ImageFormatR16i:
            return VK_FORMAT_R16_SINT;
        case spv::ImageFormatR16Snorm:
            return VK_FORMAT_R16_SNORM;
        case spv::ImageFormatR16ui:
            return VK_FORMAT_R16_UINT;
        case spv::ImageFormatR16:
            return VK_FORMAT_R16_UNORM;
        case spv::ImageFormatRgba32f:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case spv::ImageFormatRgba32i:
            return VK_FORMAT_R32G32B32A32_SINT;
        case spv::ImageFormatRgba32ui:
            return VK_FORMAT_R32G32B32A32_UINT;
        case spv::ImageFormatRg32f:
            return VK_FORMAT_R32G32_SFLOAT;
        case spv::ImageFormatRg32i:
            return VK_FORMAT_R32G32_SINT;
        case spv::ImageFormatRg32ui:
            return VK_FORMAT_R32G32_UINT;
        case spv::ImageFormatR32f:
            return VK_FORMAT_R32_SFLOAT;
        case spv::ImageFormatR32i:
            return VK_FORMAT_R32_SINT;
        case spv::ImageFormatR32ui:
            return VK_FORMAT_R32_UINT;
        case spv::ImageFormatR64i:
            return VK_FORMAT_R64_SINT;
        case spv::ImageFormatR64ui:
            return VK_FORMAT_R64_UINT;
        case spv::ImageFormatRgba8i:
            return VK_FORMAT_R8G8B8A8_SINT;
        case spv::ImageFormatRgba8Snorm:
            return VK_FORMAT_R8G8B8A8_SNORM;
        case spv::ImageFormatRgba8ui:
            return VK_FORMAT_R8G8B8A8_UINT;
        case spv::ImageFormatRgba8:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case spv::ImageFormatRg8i:
            return VK_FORMAT_R8G8_SINT;
        case spv::ImageFormatRg8Snorm:
            return VK_FORMAT_R8G8_SNORM;
        case spv::ImageFormatRg8ui:
            return VK_FORMAT_R8G8_UINT;
        case spv::ImageFormatRg8:
            return VK_FORMAT_R8G8_UNORM;
        case spv::ImageFormatR8i:
            return VK_FORMAT_R8_SINT;
        case spv::ImageFormatR8Snorm:
            return VK_FORMAT_R8_SNORM;
        case spv::ImageFormatR8ui:
            return VK_FORMAT_R8_UINT;
        case spv::ImageFormatR8:
            return VK_FORMAT_R8_UNORM;
        default:
            return VK_FORMAT_UNDEFINED;
     }
}
