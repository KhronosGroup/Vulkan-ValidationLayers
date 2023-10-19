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
// NOLINTBEGIN
#include <string>
#include <string_view>
#include <functional>
#include <spirv/unified1/spirv.hpp>
#include "vk_extension_helper.h"
#include "state_tracker/shader_module.h"
#include "state_tracker/device_state.h"
#include "core_checks/core_validation.h"

struct FeaturePointer {
    // Callable object to test if this feature is enabled in the given aggregate feature struct
    const std::function<bool(const DeviceFeatures &)> IsEnabled;

    // Test if feature pointer is populated
    explicit operator bool() const { return static_cast<bool>(IsEnabled); }

    // Default and nullptr constructor to create an empty FeaturePointer
    FeaturePointer() : IsEnabled(nullptr) {}
    FeaturePointer(std::nullptr_t ptr) : IsEnabled(nullptr) {}
    FeaturePointer(bool DeviceFeatures::*ptr) : IsEnabled([=](const DeviceFeatures &features) { return features.*ptr; }) {}
};

// Each instance of the struct will only have a singel field non-null
struct RequiredSpirvInfo {
    uint32_t version;
    FeaturePointer feature;
    ExtEnabled DeviceExtensions::*extension;
    const char *property;  // For human readability and make some capabilities unique
};

// clang-format off
static const std::unordered_multimap<uint32_t, RequiredSpirvInfo> spirvCapabilities = {
    {spv::CapabilityMatrix, {VK_API_VERSION_1_0, nullptr, nullptr, ""}},
    {spv::CapabilityShader, {VK_API_VERSION_1_0, nullptr, nullptr, ""}},
    {spv::CapabilityInputAttachment, {VK_API_VERSION_1_0, nullptr, nullptr, ""}},
    {spv::CapabilitySampled1D, {VK_API_VERSION_1_0, nullptr, nullptr, ""}},
    {spv::CapabilityImage1D, {VK_API_VERSION_1_0, nullptr, nullptr, ""}},
    {spv::CapabilitySampledBuffer, {VK_API_VERSION_1_0, nullptr, nullptr, ""}},
    {spv::CapabilityImageBuffer, {VK_API_VERSION_1_0, nullptr, nullptr, ""}},
    {spv::CapabilityImageQuery, {VK_API_VERSION_1_0, nullptr, nullptr, ""}},
    {spv::CapabilityDerivativeControl, {VK_API_VERSION_1_0, nullptr, nullptr, ""}},
    {spv::CapabilityGeometry, {0, &DeviceFeatures::geometryShader, nullptr, ""}},
    {spv::CapabilityTessellation, {0, &DeviceFeatures::tessellationShader, nullptr, ""}},
    {spv::CapabilityFloat64, {0, &DeviceFeatures::shaderFloat64, nullptr, ""}},
    {spv::CapabilityInt64, {0, &DeviceFeatures::shaderInt64, nullptr, ""}},
    {spv::CapabilityInt64Atomics, {0, &DeviceFeatures::shaderBufferInt64Atomics, nullptr, ""}},
    {spv::CapabilityInt64Atomics, {0, &DeviceFeatures::shaderSharedInt64Atomics, nullptr, ""}},
    {spv::CapabilityInt64Atomics, {0, &DeviceFeatures::shaderImageInt64Atomics, nullptr, ""}},
    {spv::CapabilityAtomicFloat16AddEXT, {0, &DeviceFeatures::shaderBufferFloat16AtomicAdd, nullptr, ""}},
    {spv::CapabilityAtomicFloat16AddEXT, {0, &DeviceFeatures::shaderSharedFloat16AtomicAdd, nullptr, ""}},
    {spv::CapabilityAtomicFloat32AddEXT, {0, &DeviceFeatures::shaderBufferFloat32AtomicAdd, nullptr, ""}},
    {spv::CapabilityAtomicFloat32AddEXT, {0, &DeviceFeatures::shaderSharedFloat32AtomicAdd, nullptr, ""}},
    {spv::CapabilityAtomicFloat32AddEXT, {0, &DeviceFeatures::shaderImageFloat32AtomicAdd, nullptr, ""}},
    {spv::CapabilityAtomicFloat64AddEXT, {0, &DeviceFeatures::shaderBufferFloat64AtomicAdd, nullptr, ""}},
    {spv::CapabilityAtomicFloat64AddEXT, {0, &DeviceFeatures::shaderSharedFloat64AtomicAdd, nullptr, ""}},
    {spv::CapabilityAtomicFloat16MinMaxEXT, {0, &DeviceFeatures::shaderBufferFloat16AtomicMinMax, nullptr, ""}},
    {spv::CapabilityAtomicFloat16MinMaxEXT, {0, &DeviceFeatures::shaderSharedFloat16AtomicMinMax, nullptr, ""}},
    {spv::CapabilityAtomicFloat32MinMaxEXT, {0, &DeviceFeatures::shaderBufferFloat32AtomicMinMax, nullptr, ""}},
    {spv::CapabilityAtomicFloat32MinMaxEXT, {0, &DeviceFeatures::shaderSharedFloat32AtomicMinMax, nullptr, ""}},
    {spv::CapabilityAtomicFloat32MinMaxEXT, {0, &DeviceFeatures::shaderImageFloat32AtomicMinMax, nullptr, ""}},
    {spv::CapabilityAtomicFloat64MinMaxEXT, {0, &DeviceFeatures::shaderBufferFloat64AtomicMinMax, nullptr, ""}},
    {spv::CapabilityAtomicFloat64MinMaxEXT, {0, &DeviceFeatures::shaderSharedFloat64AtomicMinMax, nullptr, ""}},
    {spv::CapabilityInt64ImageEXT, {0, &DeviceFeatures::shaderImageInt64Atomics, nullptr, ""}},
    {spv::CapabilityInt16, {0, &DeviceFeatures::shaderInt16, nullptr, ""}},
    {spv::CapabilityTessellationPointSize, {0, &DeviceFeatures::shaderTessellationAndGeometryPointSize, nullptr, ""}},
    {spv::CapabilityGeometryPointSize, {0, &DeviceFeatures::shaderTessellationAndGeometryPointSize, nullptr, ""}},
    {spv::CapabilityImageGatherExtended, {0, &DeviceFeatures::shaderImageGatherExtended, nullptr, ""}},
    {spv::CapabilityStorageImageMultisample, {0, &DeviceFeatures::shaderStorageImageMultisample, nullptr, ""}},
    {spv::CapabilityUniformBufferArrayDynamicIndexing, {0, &DeviceFeatures::shaderUniformBufferArrayDynamicIndexing, nullptr, ""}},
    {spv::CapabilitySampledImageArrayDynamicIndexing, {0, &DeviceFeatures::shaderSampledImageArrayDynamicIndexing, nullptr, ""}},
    {spv::CapabilityStorageBufferArrayDynamicIndexing, {0, &DeviceFeatures::shaderStorageBufferArrayDynamicIndexing, nullptr, ""}},
    {spv::CapabilityStorageImageArrayDynamicIndexing, {0, &DeviceFeatures::shaderStorageImageArrayDynamicIndexing, nullptr, ""}},
    {spv::CapabilityClipDistance, {0, &DeviceFeatures::shaderClipDistance, nullptr, ""}},
    {spv::CapabilityCullDistance, {0, &DeviceFeatures::shaderCullDistance, nullptr, ""}},
    {spv::CapabilityImageCubeArray, {0, &DeviceFeatures::imageCubeArray, nullptr, ""}},
    {spv::CapabilitySampleRateShading, {0, &DeviceFeatures::sampleRateShading, nullptr, ""}},
    {spv::CapabilitySparseResidency, {0, &DeviceFeatures::shaderResourceResidency, nullptr, ""}},
    {spv::CapabilityMinLod, {0, &DeviceFeatures::shaderResourceMinLod, nullptr, ""}},
    {spv::CapabilitySampledCubeArray, {0, &DeviceFeatures::imageCubeArray, nullptr, ""}},
    {spv::CapabilityImageMSArray, {0, &DeviceFeatures::shaderStorageImageMultisample, nullptr, ""}},
    {spv::CapabilityStorageImageExtendedFormats, {VK_API_VERSION_1_0, nullptr, nullptr, ""}},
    {spv::CapabilityInterpolationFunction, {0, &DeviceFeatures::sampleRateShading, nullptr, ""}},
    {spv::CapabilityStorageImageReadWithoutFormat, {0, &DeviceFeatures::shaderStorageImageReadWithoutFormat, nullptr, ""}},
    {spv::CapabilityStorageImageReadWithoutFormat, {VK_API_VERSION_1_3, nullptr, nullptr, ""}},
    {spv::CapabilityStorageImageReadWithoutFormat, {0, nullptr, &DeviceExtensions::vk_khr_format_feature_flags2, ""}},
    {spv::CapabilityStorageImageWriteWithoutFormat, {0, &DeviceFeatures::shaderStorageImageWriteWithoutFormat, nullptr, ""}},
    {spv::CapabilityStorageImageWriteWithoutFormat, {VK_API_VERSION_1_3, nullptr, nullptr, ""}},
    {spv::CapabilityStorageImageWriteWithoutFormat, {0, nullptr, &DeviceExtensions::vk_khr_format_feature_flags2, ""}},
    {spv::CapabilityMultiViewport, {0, &DeviceFeatures::multiViewport, nullptr, ""}},
    {spv::CapabilityDrawParameters, {0, &DeviceFeatures::shaderDrawParameters, nullptr, ""}},
    {spv::CapabilityDrawParameters, {0, nullptr, &DeviceExtensions::vk_khr_shader_draw_parameters, ""}},
    {spv::CapabilityMultiView, {0, &DeviceFeatures::multiview, nullptr, ""}},
    {spv::CapabilityDeviceGroup, {VK_API_VERSION_1_1, nullptr, nullptr, ""}},
    {spv::CapabilityDeviceGroup, {0, nullptr, &DeviceExtensions::vk_khr_device_group, ""}},
    {spv::CapabilityVariablePointersStorageBuffer, {0, &DeviceFeatures::variablePointersStorageBuffer, nullptr, ""}},
    {spv::CapabilityVariablePointers, {0, &DeviceFeatures::variablePointers, nullptr, ""}},
    {spv::CapabilityShaderClockKHR, {0, nullptr, &DeviceExtensions::vk_khr_shader_clock, ""}},
    {spv::CapabilityStencilExportEXT, {0, nullptr, &DeviceExtensions::vk_ext_shader_stencil_export, ""}},
    {spv::CapabilitySubgroupBallotKHR, {0, nullptr, &DeviceExtensions::vk_ext_shader_subgroup_ballot, ""}},
    {spv::CapabilitySubgroupVoteKHR, {0, nullptr, &DeviceExtensions::vk_ext_shader_subgroup_vote, ""}},
    {spv::CapabilityImageReadWriteLodAMD, {0, nullptr, &DeviceExtensions::vk_amd_shader_image_load_store_lod, ""}},
    {spv::CapabilityImageGatherBiasLodAMD, {0, nullptr, &DeviceExtensions::vk_amd_texture_gather_bias_lod, ""}},
    {spv::CapabilityFragmentMaskAMD, {0, nullptr, &DeviceExtensions::vk_amd_shader_fragment_mask, ""}},
    {spv::CapabilitySampleMaskOverrideCoverageNV, {0, nullptr, &DeviceExtensions::vk_nv_sample_mask_override_coverage, ""}},
    {spv::CapabilityGeometryShaderPassthroughNV, {0, nullptr, &DeviceExtensions::vk_nv_geometry_shader_passthrough, ""}},
    {spv::CapabilityShaderViewportIndex, {0, &DeviceFeatures::shaderOutputViewportIndex, nullptr, ""}},
    {spv::CapabilityShaderLayer, {0, &DeviceFeatures::shaderOutputLayer, nullptr, ""}},
    {spv::CapabilityShaderViewportIndexLayerEXT, {0, nullptr, &DeviceExtensions::vk_ext_shader_viewport_index_layer, ""}},
    {spv::CapabilityShaderViewportIndexLayerNV, {0, nullptr, &DeviceExtensions::vk_nv_viewport_array2, ""}},
    {spv::CapabilityShaderViewportMaskNV, {0, nullptr, &DeviceExtensions::vk_nv_viewport_array2, ""}},
    {spv::CapabilityPerViewAttributesNV, {0, nullptr, &DeviceExtensions::vk_nvx_multiview_per_view_attributes, ""}},
    {spv::CapabilityStorageBuffer16BitAccess, {0, &DeviceFeatures::storageBuffer16BitAccess, nullptr, ""}},
    {spv::CapabilityUniformAndStorageBuffer16BitAccess, {0, &DeviceFeatures::uniformAndStorageBuffer16BitAccess, nullptr, ""}},
    {spv::CapabilityStoragePushConstant16, {0, &DeviceFeatures::storagePushConstant16, nullptr, ""}},
    {spv::CapabilityStorageInputOutput16, {0, &DeviceFeatures::storageInputOutput16, nullptr, ""}},
    {spv::CapabilityGroupNonUniform, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan11Properties::subgroupSupportedOperations & VK_SUBGROUP_FEATURE_BASIC_BIT) != 0"}},
    {spv::CapabilityGroupNonUniformVote, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan11Properties::subgroupSupportedOperations & VK_SUBGROUP_FEATURE_VOTE_BIT) != 0"}},
    {spv::CapabilityGroupNonUniformArithmetic, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan11Properties::subgroupSupportedOperations & VK_SUBGROUP_FEATURE_ARITHMETIC_BIT) != 0"}},
    {spv::CapabilityGroupNonUniformBallot, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan11Properties::subgroupSupportedOperations & VK_SUBGROUP_FEATURE_BALLOT_BIT) != 0"}},
    {spv::CapabilityGroupNonUniformShuffle, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan11Properties::subgroupSupportedOperations & VK_SUBGROUP_FEATURE_SHUFFLE_BIT) != 0"}},
    {spv::CapabilityGroupNonUniformShuffleRelative, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan11Properties::subgroupSupportedOperations & VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT) != 0"}},
    {spv::CapabilityGroupNonUniformClustered, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan11Properties::subgroupSupportedOperations & VK_SUBGROUP_FEATURE_CLUSTERED_BIT) != 0"}},
    {spv::CapabilityGroupNonUniformQuad, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan11Properties::subgroupSupportedOperations & VK_SUBGROUP_FEATURE_QUAD_BIT) != 0"}},
    {spv::CapabilityGroupNonUniformPartitionedNV, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan11Properties::subgroupSupportedOperations & VK_SUBGROUP_FEATURE_PARTITIONED_BIT_NV) != 0"}},
    {spv::CapabilitySampleMaskPostDepthCoverage, {0, nullptr, &DeviceExtensions::vk_ext_post_depth_coverage, ""}},
    {spv::CapabilityShaderNonUniform, {VK_API_VERSION_1_2, nullptr, nullptr, ""}},
    {spv::CapabilityShaderNonUniform, {0, nullptr, &DeviceExtensions::vk_ext_descriptor_indexing, ""}},
    {spv::CapabilityRuntimeDescriptorArray, {0, &DeviceFeatures::runtimeDescriptorArray, nullptr, ""}},
    {spv::CapabilityInputAttachmentArrayDynamicIndexing, {0, &DeviceFeatures::shaderInputAttachmentArrayDynamicIndexing, nullptr, ""}},
    {spv::CapabilityUniformTexelBufferArrayDynamicIndexing, {0, &DeviceFeatures::shaderUniformTexelBufferArrayDynamicIndexing, nullptr, ""}},
    {spv::CapabilityStorageTexelBufferArrayDynamicIndexing, {0, &DeviceFeatures::shaderStorageTexelBufferArrayDynamicIndexing, nullptr, ""}},
    {spv::CapabilityUniformBufferArrayNonUniformIndexing, {0, &DeviceFeatures::shaderUniformBufferArrayNonUniformIndexing, nullptr, ""}},
    {spv::CapabilitySampledImageArrayNonUniformIndexing, {0, &DeviceFeatures::shaderSampledImageArrayNonUniformIndexing, nullptr, ""}},
    {spv::CapabilityStorageBufferArrayNonUniformIndexing, {0, &DeviceFeatures::shaderStorageBufferArrayNonUniformIndexing, nullptr, ""}},
    {spv::CapabilityStorageImageArrayNonUniformIndexing, {0, &DeviceFeatures::shaderStorageImageArrayNonUniformIndexing, nullptr, ""}},
    {spv::CapabilityInputAttachmentArrayNonUniformIndexing, {0, &DeviceFeatures::shaderInputAttachmentArrayNonUniformIndexing, nullptr, ""}},
    {spv::CapabilityUniformTexelBufferArrayNonUniformIndexing, {0, &DeviceFeatures::shaderUniformTexelBufferArrayNonUniformIndexing, nullptr, ""}},
    {spv::CapabilityStorageTexelBufferArrayNonUniformIndexing, {0, &DeviceFeatures::shaderStorageTexelBufferArrayNonUniformIndexing, nullptr, ""}},
    {spv::CapabilityFragmentFullyCoveredEXT, {0, nullptr, &DeviceExtensions::vk_ext_conservative_rasterization, ""}},
    {spv::CapabilityFloat16, {0, &DeviceFeatures::shaderFloat16, nullptr, ""}},
    {spv::CapabilityFloat16, {0, nullptr, &DeviceExtensions::vk_amd_gpu_shader_half_float, ""}},
    {spv::CapabilityInt8, {0, &DeviceFeatures::shaderInt8, nullptr, ""}},
    {spv::CapabilityStorageBuffer8BitAccess, {0, &DeviceFeatures::storageBuffer8BitAccess, nullptr, ""}},
    {spv::CapabilityUniformAndStorageBuffer8BitAccess, {0, &DeviceFeatures::uniformAndStorageBuffer8BitAccess, nullptr, ""}},
    {spv::CapabilityStoragePushConstant8, {0, &DeviceFeatures::storagePushConstant8, nullptr, ""}},
    {spv::CapabilityVulkanMemoryModel, {0, &DeviceFeatures::vulkanMemoryModel, nullptr, ""}},
    {spv::CapabilityVulkanMemoryModelDeviceScope, {0, &DeviceFeatures::vulkanMemoryModelDeviceScope, nullptr, ""}},
    {spv::CapabilityDenormPreserve, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderDenormPreserveFloat16 & VK_TRUE) != 0"}},
    {spv::CapabilityDenormPreserve, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderDenormPreserveFloat32 & VK_TRUE) != 0"}},
    {spv::CapabilityDenormPreserve, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderDenormPreserveFloat64 & VK_TRUE) != 0"}},
    {spv::CapabilityDenormFlushToZero, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderDenormFlushToZeroFloat16 & VK_TRUE) != 0"}},
    {spv::CapabilityDenormFlushToZero, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderDenormFlushToZeroFloat32 & VK_TRUE) != 0"}},
    {spv::CapabilityDenormFlushToZero, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderDenormFlushToZeroFloat64 & VK_TRUE) != 0"}},
    {spv::CapabilitySignedZeroInfNanPreserve, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderSignedZeroInfNanPreserveFloat16 & VK_TRUE) != 0"}},
    {spv::CapabilitySignedZeroInfNanPreserve, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderSignedZeroInfNanPreserveFloat32 & VK_TRUE) != 0"}},
    {spv::CapabilitySignedZeroInfNanPreserve, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderSignedZeroInfNanPreserveFloat64 & VK_TRUE) != 0"}},
    {spv::CapabilityRoundingModeRTE, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderRoundingModeRTEFloat16 & VK_TRUE) != 0"}},
    {spv::CapabilityRoundingModeRTE, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderRoundingModeRTEFloat32 & VK_TRUE) != 0"}},
    {spv::CapabilityRoundingModeRTE, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderRoundingModeRTEFloat64 & VK_TRUE) != 0"}},
    {spv::CapabilityRoundingModeRTZ, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderRoundingModeRTZFloat16 & VK_TRUE) != 0"}},
    {spv::CapabilityRoundingModeRTZ, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderRoundingModeRTZFloat32 & VK_TRUE) != 0"}},
    {spv::CapabilityRoundingModeRTZ, {0, nullptr, nullptr, "(VkPhysicalDeviceVulkan12Properties::shaderRoundingModeRTZFloat64 & VK_TRUE) != 0"}},
    {spv::CapabilityComputeDerivativeGroupQuadsNV, {0, &DeviceFeatures::computeDerivativeGroupQuads, nullptr, ""}},
    {spv::CapabilityComputeDerivativeGroupLinearNV, {0, &DeviceFeatures::computeDerivativeGroupLinear, nullptr, ""}},
    {spv::CapabilityFragmentBarycentricNV, {0, &DeviceFeatures::fragmentShaderBarycentric, nullptr, ""}},
    {spv::CapabilityImageFootprintNV, {0, &DeviceFeatures::imageFootprint, nullptr, ""}},
    {spv::CapabilityShadingRateNV, {0, &DeviceFeatures::shadingRateImage, nullptr, ""}},
    {spv::CapabilityMeshShadingNV, {0, nullptr, &DeviceExtensions::vk_nv_mesh_shader, ""}},
    {spv::CapabilityRayTracingKHR, {0, &DeviceFeatures::rayTracingPipeline, nullptr, ""}},
    {spv::CapabilityRayQueryKHR, {0, &DeviceFeatures::rayQuery, nullptr, ""}},
    {spv::CapabilityRayTraversalPrimitiveCullingKHR, {0, &DeviceFeatures::rayTraversalPrimitiveCulling, nullptr, ""}},
    {spv::CapabilityRayTraversalPrimitiveCullingKHR, {0, &DeviceFeatures::rayQuery, nullptr, ""}},
    {spv::CapabilityRayCullMaskKHR, {0, &DeviceFeatures::rayTracingMaintenance1, nullptr, ""}},
    {spv::CapabilityRayTracingNV, {0, nullptr, &DeviceExtensions::vk_nv_ray_tracing, ""}},
    {spv::CapabilityRayTracingMotionBlurNV, {0, &DeviceFeatures::rayTracingMotionBlur, nullptr, ""}},
    {spv::CapabilityTransformFeedback, {0, &DeviceFeatures::transformFeedback, nullptr, ""}},
    {spv::CapabilityGeometryStreams, {0, &DeviceFeatures::geometryStreams, nullptr, ""}},
    {spv::CapabilityFragmentDensityEXT, {0, &DeviceFeatures::fragmentDensityMap, nullptr, ""}},
    {spv::CapabilityPhysicalStorageBufferAddresses, {0, &DeviceFeatures::bufferDeviceAddress, nullptr, ""}},
    {spv::CapabilityPhysicalStorageBufferAddresses, {0, &DeviceFeatures::bufferDeviceAddress, nullptr, ""}},
    {spv::CapabilityCooperativeMatrixNV, {0, &DeviceFeatures::cooperativeMatrix, nullptr, ""}},
    {spv::CapabilityIntegerFunctions2INTEL, {0, &DeviceFeatures::shaderIntegerFunctions2, nullptr, ""}},
    {spv::CapabilityShaderSMBuiltinsNV, {0, &DeviceFeatures::shaderSMBuiltins, nullptr, ""}},
    {spv::CapabilityFragmentShaderSampleInterlockEXT, {0, &DeviceFeatures::fragmentShaderSampleInterlock, nullptr, ""}},
    {spv::CapabilityFragmentShaderPixelInterlockEXT, {0, &DeviceFeatures::fragmentShaderPixelInterlock, nullptr, ""}},
    {spv::CapabilityFragmentShaderShadingRateInterlockEXT, {0, &DeviceFeatures::fragmentShaderShadingRateInterlock, nullptr, ""}},
    {spv::CapabilityFragmentShaderShadingRateInterlockEXT, {0, &DeviceFeatures::shadingRateImage, nullptr, ""}},
    {spv::CapabilityDemoteToHelperInvocationEXT, {0, &DeviceFeatures::shaderDemoteToHelperInvocation, nullptr, ""}},
    {spv::CapabilityDemoteToHelperInvocationEXT, {0, &DeviceFeatures::shaderDemoteToHelperInvocation, nullptr, ""}},
    {spv::CapabilityFragmentShadingRateKHR, {0, &DeviceFeatures::pipelineFragmentShadingRate, nullptr, ""}},
    {spv::CapabilityFragmentShadingRateKHR, {0, &DeviceFeatures::primitiveFragmentShadingRate, nullptr, ""}},
    {spv::CapabilityFragmentShadingRateKHR, {0, &DeviceFeatures::attachmentFragmentShadingRate, nullptr, ""}},
    {spv::CapabilityWorkgroupMemoryExplicitLayoutKHR, {0, &DeviceFeatures::workgroupMemoryExplicitLayout, nullptr, ""}},
    {spv::CapabilityWorkgroupMemoryExplicitLayout8BitAccessKHR, {0, &DeviceFeatures::workgroupMemoryExplicitLayout8BitAccess, nullptr, ""}},
    {spv::CapabilityWorkgroupMemoryExplicitLayout16BitAccessKHR, {0, &DeviceFeatures::workgroupMemoryExplicitLayout16BitAccess, nullptr, ""}},
    {spv::CapabilityDotProductInputAllKHR, {0, &DeviceFeatures::shaderIntegerDotProduct, nullptr, ""}},
    {spv::CapabilityDotProductInputAllKHR, {0, &DeviceFeatures::shaderIntegerDotProduct, nullptr, ""}},
    {spv::CapabilityDotProductInput4x8BitKHR, {0, &DeviceFeatures::shaderIntegerDotProduct, nullptr, ""}},
    {spv::CapabilityDotProductInput4x8BitKHR, {0, &DeviceFeatures::shaderIntegerDotProduct, nullptr, ""}},
    {spv::CapabilityDotProductInput4x8BitPackedKHR, {0, &DeviceFeatures::shaderIntegerDotProduct, nullptr, ""}},
    {spv::CapabilityDotProductInput4x8BitPackedKHR, {0, &DeviceFeatures::shaderIntegerDotProduct, nullptr, ""}},
    {spv::CapabilityDotProductKHR, {0, &DeviceFeatures::shaderIntegerDotProduct, nullptr, ""}},
    {spv::CapabilityDotProductKHR, {0, &DeviceFeatures::shaderIntegerDotProduct, nullptr, ""}},
    {spv::CapabilityFragmentBarycentricKHR, {0, &DeviceFeatures::fragmentShaderBarycentric, nullptr, ""}},
    {spv::CapabilityTextureSampleWeightedQCOM, {0, &DeviceFeatures::textureSampleWeighted, nullptr, ""}},
    {spv::CapabilityTextureBoxFilterQCOM, {0, &DeviceFeatures::textureBoxFilter, nullptr, ""}},
    {spv::CapabilityTextureBlockMatchQCOM, {0, &DeviceFeatures::textureBlockMatch, nullptr, ""}},
    // Not found in current SPIR-V Headers
    //    {spv::CapabilityTextureBlockMatch2QCOM, {0, &DeviceFeatures::textureBlockMatch2, nullptr, ""}},
    {spv::CapabilityMeshShadingEXT, {0, nullptr, &DeviceExtensions::vk_ext_mesh_shader, ""}},
    {spv::CapabilityRayTracingOpacityMicromapEXT, {0, nullptr, &DeviceExtensions::vk_ext_opacity_micromap, ""}},
    {spv::CapabilityCoreBuiltinsARM, {0, &DeviceFeatures::shaderCoreBuiltins, nullptr, ""}},
    {spv::CapabilityShaderInvocationReorderNV, {0, nullptr, &DeviceExtensions::vk_nv_ray_tracing_invocation_reorder, ""}},
    // Not found in current SPIR-V Headers
    //    {spv::CapabilityClusterCullingShadingHUAWEI, {0, &DeviceFeatures::clustercullingShader, nullptr, ""}},
    {spv::CapabilityRayTracingPositionFetchKHR, {0, &DeviceFeatures::rayTracingPositionFetch, nullptr, ""}},
    {spv::CapabilityTileImageColorReadAccessEXT, {0, &DeviceFeatures::shaderTileImageColorReadAccess, nullptr, ""}},
    {spv::CapabilityTileImageDepthReadAccessEXT, {0, &DeviceFeatures::shaderTileImageDepthReadAccess, nullptr, ""}},
    {spv::CapabilityTileImageStencilReadAccessEXT, {0, &DeviceFeatures::shaderTileImageStencilReadAccess, nullptr, ""}},
    {spv::CapabilityCooperativeMatrixKHR, {0, &DeviceFeatures::cooperativeMatrix, nullptr, ""}},
    // Not found in current SPIR-V Headers
    //    {spv::CapabilityShaderEnqueueAMDX, {0, &DeviceFeatures::shaderEnqueue, nullptr, ""}},
};
// clang-format on

// clang-format off
static const std::unordered_multimap<std::string_view, RequiredSpirvInfo> spirvExtensions = {
    {"SPV_KHR_variable_pointers", {VK_API_VERSION_1_1, nullptr, nullptr, ""}},
    {"SPV_KHR_variable_pointers", {0, nullptr, &DeviceExtensions::vk_khr_variable_pointers, ""}},
    {"SPV_AMD_shader_explicit_vertex_parameter", {0, nullptr, &DeviceExtensions::vk_amd_shader_explicit_vertex_parameter, ""}},
    {"SPV_AMD_gcn_shader", {0, nullptr, &DeviceExtensions::vk_amd_gcn_shader, ""}},
    {"SPV_AMD_gpu_shader_half_float", {0, nullptr, &DeviceExtensions::vk_amd_gpu_shader_half_float, ""}},
    {"SPV_AMD_gpu_shader_int16", {0, nullptr, &DeviceExtensions::vk_amd_gpu_shader_int16, ""}},
    {"SPV_AMD_shader_ballot", {0, nullptr, &DeviceExtensions::vk_amd_shader_ballot, ""}},
    {"SPV_AMD_shader_fragment_mask", {0, nullptr, &DeviceExtensions::vk_amd_shader_fragment_mask, ""}},
    {"SPV_AMD_shader_image_load_store_lod", {0, nullptr, &DeviceExtensions::vk_amd_shader_image_load_store_lod, ""}},
    {"SPV_AMD_shader_trinary_minmax", {0, nullptr, &DeviceExtensions::vk_amd_shader_trinary_minmax, ""}},
    {"SPV_AMD_texture_gather_bias_lod", {0, nullptr, &DeviceExtensions::vk_amd_texture_gather_bias_lod, ""}},
    {"SPV_AMD_shader_early_and_late_fragment_tests", {0, nullptr, &DeviceExtensions::vk_amd_shader_early_and_late_fragment_tests, ""}},
    {"SPV_KHR_shader_draw_parameters", {VK_API_VERSION_1_1, nullptr, nullptr, ""}},
    {"SPV_KHR_shader_draw_parameters", {0, nullptr, &DeviceExtensions::vk_khr_shader_draw_parameters, ""}},
    {"SPV_KHR_8bit_storage", {VK_API_VERSION_1_2, nullptr, nullptr, ""}},
    {"SPV_KHR_8bit_storage", {0, nullptr, &DeviceExtensions::vk_khr_8bit_storage, ""}},
    {"SPV_KHR_16bit_storage", {VK_API_VERSION_1_1, nullptr, nullptr, ""}},
    {"SPV_KHR_16bit_storage", {0, nullptr, &DeviceExtensions::vk_khr_16bit_storage, ""}},
    {"SPV_KHR_shader_clock", {0, nullptr, &DeviceExtensions::vk_khr_shader_clock, ""}},
    {"SPV_KHR_float_controls", {VK_API_VERSION_1_2, nullptr, nullptr, ""}},
    {"SPV_KHR_float_controls", {0, nullptr, &DeviceExtensions::vk_khr_shader_float_controls, ""}},
    {"SPV_KHR_storage_buffer_storage_class", {VK_API_VERSION_1_1, nullptr, nullptr, ""}},
    {"SPV_KHR_storage_buffer_storage_class", {0, nullptr, &DeviceExtensions::vk_khr_storage_buffer_storage_class, ""}},
    {"SPV_KHR_post_depth_coverage", {0, nullptr, &DeviceExtensions::vk_ext_post_depth_coverage, ""}},
    {"SPV_EXT_shader_stencil_export", {0, nullptr, &DeviceExtensions::vk_ext_shader_stencil_export, ""}},
    {"SPV_KHR_shader_ballot", {0, nullptr, &DeviceExtensions::vk_ext_shader_subgroup_ballot, ""}},
    {"SPV_KHR_subgroup_vote", {0, nullptr, &DeviceExtensions::vk_ext_shader_subgroup_vote, ""}},
    {"SPV_NV_sample_mask_override_coverage", {0, nullptr, &DeviceExtensions::vk_nv_sample_mask_override_coverage, ""}},
    {"SPV_NV_geometry_shader_passthrough", {0, nullptr, &DeviceExtensions::vk_nv_geometry_shader_passthrough, ""}},
    {"SPV_NV_mesh_shader", {0, nullptr, &DeviceExtensions::vk_nv_mesh_shader, ""}},
    {"SPV_NV_viewport_array2", {0, nullptr, &DeviceExtensions::vk_nv_viewport_array2, ""}},
    {"SPV_NV_shader_subgroup_partitioned", {0, nullptr, &DeviceExtensions::vk_nv_shader_subgroup_partitioned, ""}},
    {"SPV_NV_shader_invocation_reorder", {0, nullptr, &DeviceExtensions::vk_nv_ray_tracing_invocation_reorder, ""}},
    {"SPV_EXT_shader_viewport_index_layer", {VK_API_VERSION_1_2, nullptr, nullptr, ""}},
    {"SPV_EXT_shader_viewport_index_layer", {0, nullptr, &DeviceExtensions::vk_ext_shader_viewport_index_layer, ""}},
    {"SPV_NVX_multiview_per_view_attributes", {0, nullptr, &DeviceExtensions::vk_nvx_multiview_per_view_attributes, ""}},
    {"SPV_EXT_descriptor_indexing", {VK_API_VERSION_1_2, nullptr, nullptr, ""}},
    {"SPV_EXT_descriptor_indexing", {0, nullptr, &DeviceExtensions::vk_ext_descriptor_indexing, ""}},
    {"SPV_KHR_vulkan_memory_model", {VK_API_VERSION_1_2, nullptr, nullptr, ""}},
    {"SPV_KHR_vulkan_memory_model", {0, nullptr, &DeviceExtensions::vk_khr_vulkan_memory_model, ""}},
    {"SPV_NV_compute_shader_derivatives", {0, nullptr, &DeviceExtensions::vk_nv_compute_shader_derivatives, ""}},
    {"SPV_NV_fragment_shader_barycentric", {0, nullptr, &DeviceExtensions::vk_nv_fragment_shader_barycentric, ""}},
    {"SPV_NV_shader_image_footprint", {0, nullptr, &DeviceExtensions::vk_nv_shader_image_footprint, ""}},
    {"SPV_NV_shading_rate", {0, nullptr, &DeviceExtensions::vk_nv_shading_rate_image, ""}},
    {"SPV_NV_ray_tracing", {0, nullptr, &DeviceExtensions::vk_nv_ray_tracing, ""}},
    {"SPV_KHR_ray_tracing", {0, nullptr, &DeviceExtensions::vk_khr_ray_tracing_pipeline, ""}},
    {"SPV_KHR_ray_query", {0, nullptr, &DeviceExtensions::vk_khr_ray_query, ""}},
    {"SPV_KHR_ray_cull_mask", {0, nullptr, &DeviceExtensions::vk_khr_ray_tracing_maintenance1, ""}},
    {"SPV_GOOGLE_hlsl_functionality1", {0, nullptr, &DeviceExtensions::vk_google_hlsl_functionality1, ""}},
    {"SPV_GOOGLE_user_type", {0, nullptr, &DeviceExtensions::vk_google_user_type, ""}},
    {"SPV_GOOGLE_decorate_string", {0, nullptr, &DeviceExtensions::vk_google_decorate_string, ""}},
    {"SPV_EXT_fragment_invocation_density", {0, nullptr, &DeviceExtensions::vk_ext_fragment_density_map, ""}},
    {"SPV_KHR_physical_storage_buffer", {VK_API_VERSION_1_2, nullptr, nullptr, ""}},
    {"SPV_KHR_physical_storage_buffer", {0, nullptr, &DeviceExtensions::vk_khr_buffer_device_address, ""}},
    {"SPV_EXT_physical_storage_buffer", {0, nullptr, &DeviceExtensions::vk_ext_buffer_device_address, ""}},
    {"SPV_NV_cooperative_matrix", {0, nullptr, &DeviceExtensions::vk_nv_cooperative_matrix, ""}},
    {"SPV_NV_shader_sm_builtins", {0, nullptr, &DeviceExtensions::vk_nv_shader_sm_builtins, ""}},
    {"SPV_EXT_fragment_shader_interlock", {0, nullptr, &DeviceExtensions::vk_ext_fragment_shader_interlock, ""}},
    {"SPV_EXT_demote_to_helper_invocation", {VK_API_VERSION_1_3, nullptr, nullptr, ""}},
    {"SPV_EXT_demote_to_helper_invocation", {0, nullptr, &DeviceExtensions::vk_ext_shader_demote_to_helper_invocation, ""}},
    {"SPV_KHR_fragment_shading_rate", {0, nullptr, &DeviceExtensions::vk_khr_fragment_shading_rate, ""}},
    {"SPV_KHR_non_semantic_info", {VK_API_VERSION_1_3, nullptr, nullptr, ""}},
    {"SPV_KHR_non_semantic_info", {0, nullptr, &DeviceExtensions::vk_khr_shader_non_semantic_info, ""}},
    {"SPV_EXT_shader_image_int64", {0, nullptr, &DeviceExtensions::vk_ext_shader_image_atomic_int64, ""}},
    {"SPV_KHR_terminate_invocation", {VK_API_VERSION_1_3, nullptr, nullptr, ""}},
    {"SPV_KHR_terminate_invocation", {0, nullptr, &DeviceExtensions::vk_khr_shader_terminate_invocation, ""}},
    {"SPV_KHR_multiview", {VK_API_VERSION_1_1, nullptr, nullptr, ""}},
    {"SPV_KHR_multiview", {0, nullptr, &DeviceExtensions::vk_khr_multiview, ""}},
    {"SPV_KHR_workgroup_memory_explicit_layout", {0, nullptr, &DeviceExtensions::vk_khr_workgroup_memory_explicit_layout, ""}},
    {"SPV_EXT_shader_atomic_float_add", {0, nullptr, &DeviceExtensions::vk_ext_shader_atomic_float, ""}},
    {"SPV_KHR_fragment_shader_barycentric", {0, nullptr, &DeviceExtensions::vk_khr_fragment_shader_barycentric, ""}},
    {"SPV_KHR_subgroup_uniform_control_flow", {VK_API_VERSION_1_3, nullptr, nullptr, ""}},
    {"SPV_KHR_subgroup_uniform_control_flow", {0, nullptr, &DeviceExtensions::vk_khr_shader_subgroup_uniform_control_flow, ""}},
    {"SPV_EXT_shader_atomic_float_min_max", {0, nullptr, &DeviceExtensions::vk_ext_shader_atomic_float2, ""}},
    {"SPV_EXT_shader_atomic_float16_add", {0, nullptr, &DeviceExtensions::vk_ext_shader_atomic_float2, ""}},
    {"SPV_EXT_fragment_fully_covered", {0, nullptr, &DeviceExtensions::vk_ext_conservative_rasterization, ""}},
    {"SPV_KHR_integer_dot_product", {VK_API_VERSION_1_3, nullptr, nullptr, ""}},
    {"SPV_KHR_integer_dot_product", {0, nullptr, &DeviceExtensions::vk_khr_shader_integer_dot_product, ""}},
    {"SPV_INTEL_shader_integer_functions2", {0, nullptr, &DeviceExtensions::vk_intel_shader_integer_functions2, ""}},
    {"SPV_KHR_device_group", {VK_API_VERSION_1_1, nullptr, nullptr, ""}},
    {"SPV_KHR_device_group", {0, nullptr, &DeviceExtensions::vk_khr_device_group, ""}},
    {"SPV_QCOM_image_processing", {0, nullptr, &DeviceExtensions::vk_qcom_image_processing, ""}},
    {"SPV_QCOM_image_processing2", {0, nullptr, &DeviceExtensions::vk_qcom_image_processing2, ""}},
    {"SPV_EXT_mesh_shader", {0, nullptr, &DeviceExtensions::vk_ext_mesh_shader, ""}},
    {"SPV_KHR_ray_tracing_position_fetch", {0, nullptr, &DeviceExtensions::vk_khr_ray_tracing_position_fetch, ""}},
    {"SPV_EXT_shader_tile_image", {0, nullptr, &DeviceExtensions::vk_ext_shader_tile_image, ""}},
    {"SPV_EXT_opacity_micromap", {0, nullptr, &DeviceExtensions::vk_ext_opacity_micromap, ""}},
    {"SPV_KHR_cooperative_matrix", {0, nullptr, &DeviceExtensions::vk_khr_cooperative_matrix, ""}},
    {"SPV_ARM_core_builtins", {0, nullptr, &DeviceExtensions::vk_arm_shader_core_builtins, ""}},
    {"SPV_AMDX_shader_enqueue", {0, nullptr, &DeviceExtensions::vk_amdx_shader_enqueue, ""}},
};
// clang-format on

static inline const char *string_SpvCapability(uint32_t input_value) {
    switch ((spv::Capability)input_value) {
        case spv::CapabilityMatrix:
            return "Matrix";
        case spv::CapabilityShader:
            return "Shader";
        case spv::CapabilityInputAttachment:
            return "InputAttachment";
        case spv::CapabilitySampled1D:
            return "Sampled1D";
        case spv::CapabilityImage1D:
            return "Image1D";
        case spv::CapabilitySampledBuffer:
            return "SampledBuffer";
        case spv::CapabilityImageBuffer:
            return "ImageBuffer";
        case spv::CapabilityImageQuery:
            return "ImageQuery";
        case spv::CapabilityDerivativeControl:
            return "DerivativeControl";
        case spv::CapabilityGeometry:
            return "Geometry";
        case spv::CapabilityTessellation:
            return "Tessellation";
        case spv::CapabilityFloat64:
            return "Float64";
        case spv::CapabilityInt64:
            return "Int64";
        case spv::CapabilityInt64Atomics:
            return "Int64Atomics";
        case spv::CapabilityAtomicFloat16AddEXT:
            return "AtomicFloat16AddEXT";
        case spv::CapabilityAtomicFloat32AddEXT:
            return "AtomicFloat32AddEXT";
        case spv::CapabilityAtomicFloat64AddEXT:
            return "AtomicFloat64AddEXT";
        case spv::CapabilityAtomicFloat16MinMaxEXT:
            return "AtomicFloat16MinMaxEXT";
        case spv::CapabilityAtomicFloat32MinMaxEXT:
            return "AtomicFloat32MinMaxEXT";
        case spv::CapabilityAtomicFloat64MinMaxEXT:
            return "AtomicFloat64MinMaxEXT";
        case spv::CapabilityInt64ImageEXT:
            return "Int64ImageEXT";
        case spv::CapabilityInt16:
            return "Int16";
        case spv::CapabilityTessellationPointSize:
            return "TessellationPointSize";
        case spv::CapabilityGeometryPointSize:
            return "GeometryPointSize";
        case spv::CapabilityImageGatherExtended:
            return "ImageGatherExtended";
        case spv::CapabilityStorageImageMultisample:
            return "StorageImageMultisample";
        case spv::CapabilityUniformBufferArrayDynamicIndexing:
            return "UniformBufferArrayDynamicIndexing";
        case spv::CapabilitySampledImageArrayDynamicIndexing:
            return "SampledImageArrayDynamicIndexing";
        case spv::CapabilityStorageBufferArrayDynamicIndexing:
            return "StorageBufferArrayDynamicIndexing";
        case spv::CapabilityStorageImageArrayDynamicIndexing:
            return "StorageImageArrayDynamicIndexing";
        case spv::CapabilityClipDistance:
            return "ClipDistance";
        case spv::CapabilityCullDistance:
            return "CullDistance";
        case spv::CapabilityImageCubeArray:
            return "ImageCubeArray";
        case spv::CapabilitySampleRateShading:
            return "SampleRateShading";
        case spv::CapabilitySparseResidency:
            return "SparseResidency";
        case spv::CapabilityMinLod:
            return "MinLod";
        case spv::CapabilitySampledCubeArray:
            return "SampledCubeArray";
        case spv::CapabilityImageMSArray:
            return "ImageMSArray";
        case spv::CapabilityStorageImageExtendedFormats:
            return "StorageImageExtendedFormats";
        case spv::CapabilityInterpolationFunction:
            return "InterpolationFunction";
        case spv::CapabilityStorageImageReadWithoutFormat:
            return "StorageImageReadWithoutFormat";
        case spv::CapabilityStorageImageWriteWithoutFormat:
            return "StorageImageWriteWithoutFormat";
        case spv::CapabilityMultiViewport:
            return "MultiViewport";
        case spv::CapabilityDrawParameters:
            return "DrawParameters";
        case spv::CapabilityMultiView:
            return "MultiView";
        case spv::CapabilityDeviceGroup:
            return "DeviceGroup";
        case spv::CapabilityVariablePointersStorageBuffer:
            return "VariablePointersStorageBuffer";
        case spv::CapabilityVariablePointers:
            return "VariablePointers";
        case spv::CapabilityShaderClockKHR:
            return "ShaderClockKHR";
        case spv::CapabilityStencilExportEXT:
            return "StencilExportEXT";
        case spv::CapabilitySubgroupBallotKHR:
            return "SubgroupBallotKHR";
        case spv::CapabilitySubgroupVoteKHR:
            return "SubgroupVoteKHR";
        case spv::CapabilityImageReadWriteLodAMD:
            return "ImageReadWriteLodAMD";
        case spv::CapabilityImageGatherBiasLodAMD:
            return "ImageGatherBiasLodAMD";
        case spv::CapabilityFragmentMaskAMD:
            return "FragmentMaskAMD";
        case spv::CapabilitySampleMaskOverrideCoverageNV:
            return "SampleMaskOverrideCoverageNV";
        case spv::CapabilityGeometryShaderPassthroughNV:
            return "GeometryShaderPassthroughNV";
        case spv::CapabilityShaderViewportIndex:
            return "ShaderViewportIndex";
        case spv::CapabilityShaderLayer:
            return "ShaderLayer";
        case spv::CapabilityShaderViewportIndexLayerEXT:
            return "ShaderViewportIndexLayerEXT";
        case spv::CapabilityShaderViewportMaskNV:
            return "ShaderViewportMaskNV";
        case spv::CapabilityPerViewAttributesNV:
            return "PerViewAttributesNV";
        case spv::CapabilityStorageBuffer16BitAccess:
            return "StorageBuffer16BitAccess";
        case spv::CapabilityUniformAndStorageBuffer16BitAccess:
            return "UniformAndStorageBuffer16BitAccess";
        case spv::CapabilityStoragePushConstant16:
            return "StoragePushConstant16";
        case spv::CapabilityStorageInputOutput16:
            return "StorageInputOutput16";
        case spv::CapabilityGroupNonUniform:
            return "GroupNonUniform";
        case spv::CapabilityGroupNonUniformVote:
            return "GroupNonUniformVote";
        case spv::CapabilityGroupNonUniformArithmetic:
            return "GroupNonUniformArithmetic";
        case spv::CapabilityGroupNonUniformBallot:
            return "GroupNonUniformBallot";
        case spv::CapabilityGroupNonUniformShuffle:
            return "GroupNonUniformShuffle";
        case spv::CapabilityGroupNonUniformShuffleRelative:
            return "GroupNonUniformShuffleRelative";
        case spv::CapabilityGroupNonUniformClustered:
            return "GroupNonUniformClustered";
        case spv::CapabilityGroupNonUniformQuad:
            return "GroupNonUniformQuad";
        case spv::CapabilityGroupNonUniformPartitionedNV:
            return "GroupNonUniformPartitionedNV";
        case spv::CapabilitySampleMaskPostDepthCoverage:
            return "SampleMaskPostDepthCoverage";
        case spv::CapabilityShaderNonUniform:
            return "ShaderNonUniform";
        case spv::CapabilityRuntimeDescriptorArray:
            return "RuntimeDescriptorArray";
        case spv::CapabilityInputAttachmentArrayDynamicIndexing:
            return "InputAttachmentArrayDynamicIndexing";
        case spv::CapabilityUniformTexelBufferArrayDynamicIndexing:
            return "UniformTexelBufferArrayDynamicIndexing";
        case spv::CapabilityStorageTexelBufferArrayDynamicIndexing:
            return "StorageTexelBufferArrayDynamicIndexing";
        case spv::CapabilityUniformBufferArrayNonUniformIndexing:
            return "UniformBufferArrayNonUniformIndexing";
        case spv::CapabilitySampledImageArrayNonUniformIndexing:
            return "SampledImageArrayNonUniformIndexing";
        case spv::CapabilityStorageBufferArrayNonUniformIndexing:
            return "StorageBufferArrayNonUniformIndexing";
        case spv::CapabilityStorageImageArrayNonUniformIndexing:
            return "StorageImageArrayNonUniformIndexing";
        case spv::CapabilityInputAttachmentArrayNonUniformIndexing:
            return "InputAttachmentArrayNonUniformIndexing";
        case spv::CapabilityUniformTexelBufferArrayNonUniformIndexing:
            return "UniformTexelBufferArrayNonUniformIndexing";
        case spv::CapabilityStorageTexelBufferArrayNonUniformIndexing:
            return "StorageTexelBufferArrayNonUniformIndexing";
        case spv::CapabilityFragmentFullyCoveredEXT:
            return "FragmentFullyCoveredEXT";
        case spv::CapabilityFloat16:
            return "Float16";
        case spv::CapabilityInt8:
            return "Int8";
        case spv::CapabilityStorageBuffer8BitAccess:
            return "StorageBuffer8BitAccess";
        case spv::CapabilityUniformAndStorageBuffer8BitAccess:
            return "UniformAndStorageBuffer8BitAccess";
        case spv::CapabilityStoragePushConstant8:
            return "StoragePushConstant8";
        case spv::CapabilityVulkanMemoryModel:
            return "VulkanMemoryModel";
        case spv::CapabilityVulkanMemoryModelDeviceScope:
            return "VulkanMemoryModelDeviceScope";
        case spv::CapabilityDenormPreserve:
            return "DenormPreserve";
        case spv::CapabilityDenormFlushToZero:
            return "DenormFlushToZero";
        case spv::CapabilitySignedZeroInfNanPreserve:
            return "SignedZeroInfNanPreserve";
        case spv::CapabilityRoundingModeRTE:
            return "RoundingModeRTE";
        case spv::CapabilityRoundingModeRTZ:
            return "RoundingModeRTZ";
        case spv::CapabilityComputeDerivativeGroupQuadsNV:
            return "ComputeDerivativeGroupQuadsNV";
        case spv::CapabilityComputeDerivativeGroupLinearNV:
            return "ComputeDerivativeGroupLinearNV";
        case spv::CapabilityImageFootprintNV:
            return "ImageFootprintNV";
        case spv::CapabilityMeshShadingNV:
            return "MeshShadingNV";
        case spv::CapabilityRayTracingKHR:
            return "RayTracingKHR";
        case spv::CapabilityRayQueryKHR:
            return "RayQueryKHR";
        case spv::CapabilityRayTraversalPrimitiveCullingKHR:
            return "RayTraversalPrimitiveCullingKHR";
        case spv::CapabilityRayCullMaskKHR:
            return "RayCullMaskKHR";
        case spv::CapabilityRayTracingNV:
            return "RayTracingNV";
        case spv::CapabilityRayTracingMotionBlurNV:
            return "RayTracingMotionBlurNV";
        case spv::CapabilityTransformFeedback:
            return "TransformFeedback";
        case spv::CapabilityGeometryStreams:
            return "GeometryStreams";
        case spv::CapabilityFragmentDensityEXT:
            return "FragmentDensityEXT";
        case spv::CapabilityPhysicalStorageBufferAddresses:
            return "PhysicalStorageBufferAddresses";
        case spv::CapabilityCooperativeMatrixNV:
            return "CooperativeMatrixNV";
        case spv::CapabilityIntegerFunctions2INTEL:
            return "IntegerFunctions2INTEL";
        case spv::CapabilityShaderSMBuiltinsNV:
            return "ShaderSMBuiltinsNV";
        case spv::CapabilityFragmentShaderSampleInterlockEXT:
            return "FragmentShaderSampleInterlockEXT";
        case spv::CapabilityFragmentShaderPixelInterlockEXT:
            return "FragmentShaderPixelInterlockEXT";
        case spv::CapabilityFragmentShaderShadingRateInterlockEXT:
            return "FragmentShaderShadingRateInterlockEXT";
        case spv::CapabilityDemoteToHelperInvocationEXT:
            return "DemoteToHelperInvocationEXT";
        case spv::CapabilityFragmentShadingRateKHR:
            return "FragmentShadingRateKHR";
        case spv::CapabilityWorkgroupMemoryExplicitLayoutKHR:
            return "WorkgroupMemoryExplicitLayoutKHR";
        case spv::CapabilityWorkgroupMemoryExplicitLayout8BitAccessKHR:
            return "WorkgroupMemoryExplicitLayout8BitAccessKHR";
        case spv::CapabilityWorkgroupMemoryExplicitLayout16BitAccessKHR:
            return "WorkgroupMemoryExplicitLayout16BitAccessKHR";
        case spv::CapabilityDotProductInputAllKHR:
            return "DotProductInputAllKHR";
        case spv::CapabilityDotProductInput4x8BitKHR:
            return "DotProductInput4x8BitKHR";
        case spv::CapabilityDotProductInput4x8BitPackedKHR:
            return "DotProductInput4x8BitPackedKHR";
        case spv::CapabilityDotProductKHR:
            return "DotProductKHR";
        case spv::CapabilityFragmentBarycentricKHR:
            return "FragmentBarycentricKHR";
        case spv::CapabilityTextureSampleWeightedQCOM:
            return "TextureSampleWeightedQCOM";
        case spv::CapabilityTextureBoxFilterQCOM:
            return "TextureBoxFilterQCOM";
        case spv::CapabilityTextureBlockMatchQCOM:
            return "TextureBlockMatchQCOM";
        case spv::CapabilityMeshShadingEXT:
            return "MeshShadingEXT";
        case spv::CapabilityRayTracingOpacityMicromapEXT:
            return "RayTracingOpacityMicromapEXT";
        case spv::CapabilityCoreBuiltinsARM:
            return "CoreBuiltinsARM";
        case spv::CapabilityShaderInvocationReorderNV:
            return "ShaderInvocationReorderNV";
        case spv::CapabilityRayTracingPositionFetchKHR:
            return "RayTracingPositionFetchKHR";
        case spv::CapabilityTileImageColorReadAccessEXT:
            return "TileImageColorReadAccessEXT";
        case spv::CapabilityTileImageDepthReadAccessEXT:
            return "TileImageDepthReadAccessEXT";
        case spv::CapabilityTileImageStencilReadAccessEXT:
            return "TileImageStencilReadAccessEXT";
        case spv::CapabilityCooperativeMatrixKHR:
            return "CooperativeMatrixKHR";
        default:
            return "Unhandled OpCapability";
    };
}

// Will return the Vulkan format for a given SPIR-V image format value
// Note: will return VK_FORMAT_UNDEFINED if non valid input
// This was in vk_format_utils but the SPIR-V Header dependency was an issue
//   see https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/4647
VkFormat CoreChecks::CompatibleSpirvImageFormat(uint32_t spirv_image_format) const {
    switch (spirv_image_format) {
        case spv::ImageFormatR8:
            return VK_FORMAT_R8_UNORM;
        case spv::ImageFormatR8Snorm:
            return VK_FORMAT_R8_SNORM;
        case spv::ImageFormatR8ui:
            return VK_FORMAT_R8_UINT;
        case spv::ImageFormatR8i:
            return VK_FORMAT_R8_SINT;
        case spv::ImageFormatRg8:
            return VK_FORMAT_R8G8_UNORM;
        case spv::ImageFormatRg8Snorm:
            return VK_FORMAT_R8G8_SNORM;
        case spv::ImageFormatRg8ui:
            return VK_FORMAT_R8G8_UINT;
        case spv::ImageFormatRg8i:
            return VK_FORMAT_R8G8_SINT;
        case spv::ImageFormatRgba8:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case spv::ImageFormatRgba8Snorm:
            return VK_FORMAT_R8G8B8A8_SNORM;
        case spv::ImageFormatRgba8ui:
            return VK_FORMAT_R8G8B8A8_UINT;
        case spv::ImageFormatRgba8i:
            return VK_FORMAT_R8G8B8A8_SINT;
        case spv::ImageFormatRgb10A2:
            return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        case spv::ImageFormatRgb10a2ui:
            return VK_FORMAT_A2B10G10R10_UINT_PACK32;
        case spv::ImageFormatR16:
            return VK_FORMAT_R16_UNORM;
        case spv::ImageFormatR16Snorm:
            return VK_FORMAT_R16_SNORM;
        case spv::ImageFormatR16ui:
            return VK_FORMAT_R16_UINT;
        case spv::ImageFormatR16i:
            return VK_FORMAT_R16_SINT;
        case spv::ImageFormatR16f:
            return VK_FORMAT_R16_SFLOAT;
        case spv::ImageFormatRg16:
            return VK_FORMAT_R16G16_UNORM;
        case spv::ImageFormatRg16Snorm:
            return VK_FORMAT_R16G16_SNORM;
        case spv::ImageFormatRg16ui:
            return VK_FORMAT_R16G16_UINT;
        case spv::ImageFormatRg16i:
            return VK_FORMAT_R16G16_SINT;
        case spv::ImageFormatRg16f:
            return VK_FORMAT_R16G16_SFLOAT;
        case spv::ImageFormatRgba16:
            return VK_FORMAT_R16G16B16A16_UNORM;
        case spv::ImageFormatRgba16Snorm:
            return VK_FORMAT_R16G16B16A16_SNORM;
        case spv::ImageFormatRgba16ui:
            return VK_FORMAT_R16G16B16A16_UINT;
        case spv::ImageFormatRgba16i:
            return VK_FORMAT_R16G16B16A16_SINT;
        case spv::ImageFormatRgba16f:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case spv::ImageFormatR32ui:
            return VK_FORMAT_R32_UINT;
        case spv::ImageFormatR32i:
            return VK_FORMAT_R32_SINT;
        case spv::ImageFormatR32f:
            return VK_FORMAT_R32_SFLOAT;
        case spv::ImageFormatRg32ui:
            return VK_FORMAT_R32G32_UINT;
        case spv::ImageFormatRg32i:
            return VK_FORMAT_R32G32_SINT;
        case spv::ImageFormatRg32f:
            return VK_FORMAT_R32G32_SFLOAT;
        case spv::ImageFormatRgba32ui:
            return VK_FORMAT_R32G32B32A32_UINT;
        case spv::ImageFormatRgba32i:
            return VK_FORMAT_R32G32B32A32_SINT;
        case spv::ImageFormatRgba32f:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case spv::ImageFormatR64ui:
            return VK_FORMAT_R64_UINT;
        case spv::ImageFormatR64i:
            return VK_FORMAT_R64_SINT;
        case spv::ImageFormatR11fG11fB10f:
            return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        default:
            return VK_FORMAT_UNDEFINED;
    };
}

// clang-format off
static inline const char* SpvCapabilityRequirements(uint32_t capability) {
    static const vvl::unordered_map<uint32_t, std::string_view> table {
    {spv::CapabilityMatrix, "VK_VERSION_1_0"},
    {spv::CapabilityShader, "VK_VERSION_1_0"},
    {spv::CapabilityInputAttachment, "VK_VERSION_1_0"},
    {spv::CapabilitySampled1D, "VK_VERSION_1_0"},
    {spv::CapabilityImage1D, "VK_VERSION_1_0"},
    {spv::CapabilitySampledBuffer, "VK_VERSION_1_0"},
    {spv::CapabilityImageBuffer, "VK_VERSION_1_0"},
    {spv::CapabilityImageQuery, "VK_VERSION_1_0"},
    {spv::CapabilityDerivativeControl, "VK_VERSION_1_0"},
    {spv::CapabilityGeometry, "VkPhysicalDeviceFeatures::geometryShader"},
    {spv::CapabilityTessellation, "VkPhysicalDeviceFeatures::tessellationShader"},
    {spv::CapabilityFloat64, "VkPhysicalDeviceFeatures::shaderFloat64"},
    {spv::CapabilityInt64, "VkPhysicalDeviceFeatures::shaderInt64"},
    {spv::CapabilityInt64Atomics, "VkPhysicalDeviceVulkan12Features::shaderBufferInt64Atomics OR VkPhysicalDeviceVulkan12Features::shaderSharedInt64Atomics OR VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT::shaderImageInt64Atomics"},
    {spv::CapabilityAtomicFloat16AddEXT, "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderBufferFloat16AtomicAdd OR VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderSharedFloat16AtomicAdd"},
    {spv::CapabilityAtomicFloat32AddEXT, "VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderBufferFloat32AtomicAdd OR VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderSharedFloat32AtomicAdd OR VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderImageFloat32AtomicAdd"},
    {spv::CapabilityAtomicFloat64AddEXT, "VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderBufferFloat64AtomicAdd OR VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderSharedFloat64AtomicAdd"},
    {spv::CapabilityAtomicFloat16MinMaxEXT, "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderBufferFloat16AtomicMinMax OR VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderSharedFloat16AtomicMinMax"},
    {spv::CapabilityAtomicFloat32MinMaxEXT, "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderBufferFloat32AtomicMinMax OR VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderSharedFloat32AtomicMinMax OR VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderImageFloat32AtomicMinMax"},
    {spv::CapabilityAtomicFloat64MinMaxEXT, "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderBufferFloat64AtomicMinMax OR VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderSharedFloat64AtomicMinMax"},
    {spv::CapabilityInt64ImageEXT, "VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT::shaderImageInt64Atomics"},
    {spv::CapabilityInt16, "VkPhysicalDeviceFeatures::shaderInt16"},
    {spv::CapabilityTessellationPointSize, "VkPhysicalDeviceFeatures::shaderTessellationAndGeometryPointSize"},
    {spv::CapabilityGeometryPointSize, "VkPhysicalDeviceFeatures::shaderTessellationAndGeometryPointSize"},
    {spv::CapabilityImageGatherExtended, "VkPhysicalDeviceFeatures::shaderImageGatherExtended"},
    {spv::CapabilityStorageImageMultisample, "VkPhysicalDeviceFeatures::shaderStorageImageMultisample"},
    {spv::CapabilityUniformBufferArrayDynamicIndexing, "VkPhysicalDeviceFeatures::shaderUniformBufferArrayDynamicIndexing"},
    {spv::CapabilitySampledImageArrayDynamicIndexing, "VkPhysicalDeviceFeatures::shaderSampledImageArrayDynamicIndexing"},
    {spv::CapabilityStorageBufferArrayDynamicIndexing, "VkPhysicalDeviceFeatures::shaderStorageBufferArrayDynamicIndexing"},
    {spv::CapabilityStorageImageArrayDynamicIndexing, "VkPhysicalDeviceFeatures::shaderStorageImageArrayDynamicIndexing"},
    {spv::CapabilityClipDistance, "VkPhysicalDeviceFeatures::shaderClipDistance"},
    {spv::CapabilityCullDistance, "VkPhysicalDeviceFeatures::shaderCullDistance"},
    {spv::CapabilityImageCubeArray, "VkPhysicalDeviceFeatures::imageCubeArray"},
    {spv::CapabilitySampleRateShading, "VkPhysicalDeviceFeatures::sampleRateShading"},
    {spv::CapabilitySparseResidency, "VkPhysicalDeviceFeatures::shaderResourceResidency"},
    {spv::CapabilityMinLod, "VkPhysicalDeviceFeatures::shaderResourceMinLod"},
    {spv::CapabilitySampledCubeArray, "VkPhysicalDeviceFeatures::imageCubeArray"},
    {spv::CapabilityImageMSArray, "VkPhysicalDeviceFeatures::shaderStorageImageMultisample"},
    {spv::CapabilityStorageImageExtendedFormats, "VK_VERSION_1_0"},
    {spv::CapabilityInterpolationFunction, "VkPhysicalDeviceFeatures::sampleRateShading"},
    {spv::CapabilityStorageImageReadWithoutFormat, "VkPhysicalDeviceFeatures::shaderStorageImageReadWithoutFormat OR VK_VERSION_1_3 OR VK_KHR_format_feature_flags2"},
    {spv::CapabilityStorageImageWriteWithoutFormat, "VkPhysicalDeviceFeatures::shaderStorageImageWriteWithoutFormat OR VK_VERSION_1_3 OR VK_KHR_format_feature_flags2"},
    {spv::CapabilityMultiViewport, "VkPhysicalDeviceFeatures::multiViewport"},
    {spv::CapabilityDrawParameters, "VkPhysicalDeviceVulkan11Features::shaderDrawParameters OR VK_KHR_shader_draw_parameters"},
    {spv::CapabilityMultiView, "VkPhysicalDeviceVulkan11Features::multiview"},
    {spv::CapabilityDeviceGroup, "VK_VERSION_1_1 OR VK_KHR_device_group"},
    {spv::CapabilityVariablePointersStorageBuffer, "VkPhysicalDeviceVulkan11Features::variablePointersStorageBuffer"},
    {spv::CapabilityVariablePointers, "VkPhysicalDeviceVulkan11Features::variablePointers"},
    {spv::CapabilityShaderClockKHR, "VK_KHR_shader_clock"},
    {spv::CapabilityStencilExportEXT, "VK_EXT_shader_stencil_export"},
    {spv::CapabilitySubgroupBallotKHR, "VK_EXT_shader_subgroup_ballot"},
    {spv::CapabilitySubgroupVoteKHR, "VK_EXT_shader_subgroup_vote"},
    {spv::CapabilityImageReadWriteLodAMD, "VK_AMD_shader_image_load_store_lod"},
    {spv::CapabilityImageGatherBiasLodAMD, "VK_AMD_texture_gather_bias_lod"},
    {spv::CapabilityFragmentMaskAMD, "VK_AMD_shader_fragment_mask"},
    {spv::CapabilitySampleMaskOverrideCoverageNV, "VK_NV_sample_mask_override_coverage"},
    {spv::CapabilityGeometryShaderPassthroughNV, "VK_NV_geometry_shader_passthrough"},
    {spv::CapabilityShaderViewportIndex, "VkPhysicalDeviceVulkan12Features::shaderOutputViewportIndex"},
    {spv::CapabilityShaderLayer, "VkPhysicalDeviceVulkan12Features::shaderOutputLayer"},
    {spv::CapabilityShaderViewportIndexLayerEXT, "VK_EXT_shader_viewport_index_layer"},
    {spv::CapabilityShaderViewportIndexLayerNV, "VK_NV_viewport_array2"},
    {spv::CapabilityShaderViewportMaskNV, "VK_NV_viewport_array2"},
    {spv::CapabilityPerViewAttributesNV, "VK_NVX_multiview_per_view_attributes"},
    {spv::CapabilityStorageBuffer16BitAccess, "VkPhysicalDeviceVulkan11Features::storageBuffer16BitAccess"},
    {spv::CapabilityUniformAndStorageBuffer16BitAccess, "VkPhysicalDeviceVulkan11Features::uniformAndStorageBuffer16BitAccess"},
    {spv::CapabilityStoragePushConstant16, "VkPhysicalDeviceVulkan11Features::storagePushConstant16"},
    {spv::CapabilityStorageInputOutput16, "VkPhysicalDeviceVulkan11Features::storageInputOutput16"},
    {spv::CapabilityGroupNonUniform, "(VkPhysicalDeviceVulkan11Properties::subgroupSupportedOperations == VK_SUBGROUP_FEATURE_BASIC_BIT)"},
    {spv::CapabilityGroupNonUniformVote, "(VkPhysicalDeviceVulkan11Properties::subgroupSupportedOperations == VK_SUBGROUP_FEATURE_VOTE_BIT)"},
    {spv::CapabilityGroupNonUniformArithmetic, "(VkPhysicalDeviceVulkan11Properties::subgroupSupportedOperations == VK_SUBGROUP_FEATURE_ARITHMETIC_BIT)"},
    {spv::CapabilityGroupNonUniformBallot, "(VkPhysicalDeviceVulkan11Properties::subgroupSupportedOperations == VK_SUBGROUP_FEATURE_BALLOT_BIT)"},
    {spv::CapabilityGroupNonUniformShuffle, "(VkPhysicalDeviceVulkan11Properties::subgroupSupportedOperations == VK_SUBGROUP_FEATURE_SHUFFLE_BIT)"},
    {spv::CapabilityGroupNonUniformShuffleRelative, "(VkPhysicalDeviceVulkan11Properties::subgroupSupportedOperations == VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT)"},
    {spv::CapabilityGroupNonUniformClustered, "(VkPhysicalDeviceVulkan11Properties::subgroupSupportedOperations == VK_SUBGROUP_FEATURE_CLUSTERED_BIT)"},
    {spv::CapabilityGroupNonUniformQuad, "(VkPhysicalDeviceVulkan11Properties::subgroupSupportedOperations == VK_SUBGROUP_FEATURE_QUAD_BIT)"},
    {spv::CapabilityGroupNonUniformPartitionedNV, "(VkPhysicalDeviceVulkan11Properties::subgroupSupportedOperations == VK_SUBGROUP_FEATURE_PARTITIONED_BIT_NV)"},
    {spv::CapabilitySampleMaskPostDepthCoverage, "VK_EXT_post_depth_coverage"},
    {spv::CapabilityShaderNonUniform, "VK_VERSION_1_2 OR VK_EXT_descriptor_indexing"},
    {spv::CapabilityRuntimeDescriptorArray, "VkPhysicalDeviceVulkan12Features::runtimeDescriptorArray"},
    {spv::CapabilityInputAttachmentArrayDynamicIndexing, "VkPhysicalDeviceVulkan12Features::shaderInputAttachmentArrayDynamicIndexing"},
    {spv::CapabilityUniformTexelBufferArrayDynamicIndexing, "VkPhysicalDeviceVulkan12Features::shaderUniformTexelBufferArrayDynamicIndexing"},
    {spv::CapabilityStorageTexelBufferArrayDynamicIndexing, "VkPhysicalDeviceVulkan12Features::shaderStorageTexelBufferArrayDynamicIndexing"},
    {spv::CapabilityUniformBufferArrayNonUniformIndexing, "VkPhysicalDeviceVulkan12Features::shaderUniformBufferArrayNonUniformIndexing"},
    {spv::CapabilitySampledImageArrayNonUniformIndexing, "VkPhysicalDeviceVulkan12Features::shaderSampledImageArrayNonUniformIndexing"},
    {spv::CapabilityStorageBufferArrayNonUniformIndexing, "VkPhysicalDeviceVulkan12Features::shaderStorageBufferArrayNonUniformIndexing"},
    {spv::CapabilityStorageImageArrayNonUniformIndexing, "VkPhysicalDeviceVulkan12Features::shaderStorageImageArrayNonUniformIndexing"},
    {spv::CapabilityInputAttachmentArrayNonUniformIndexing, "VkPhysicalDeviceVulkan12Features::shaderInputAttachmentArrayNonUniformIndexing"},
    {spv::CapabilityUniformTexelBufferArrayNonUniformIndexing, "VkPhysicalDeviceVulkan12Features::shaderUniformTexelBufferArrayNonUniformIndexing"},
    {spv::CapabilityStorageTexelBufferArrayNonUniformIndexing, "VkPhysicalDeviceVulkan12Features::shaderStorageTexelBufferArrayNonUniformIndexing"},
    {spv::CapabilityFragmentFullyCoveredEXT, "VK_EXT_conservative_rasterization"},
    {spv::CapabilityFloat16, "VkPhysicalDeviceVulkan12Features::shaderFloat16 OR VK_AMD_gpu_shader_half_float"},
    {spv::CapabilityInt8, "VkPhysicalDeviceVulkan12Features::shaderInt8"},
    {spv::CapabilityStorageBuffer8BitAccess, "VkPhysicalDeviceVulkan12Features::storageBuffer8BitAccess"},
    {spv::CapabilityUniformAndStorageBuffer8BitAccess, "VkPhysicalDeviceVulkan12Features::uniformAndStorageBuffer8BitAccess"},
    {spv::CapabilityStoragePushConstant8, "VkPhysicalDeviceVulkan12Features::storagePushConstant8"},
    {spv::CapabilityVulkanMemoryModel, "VkPhysicalDeviceVulkan12Features::vulkanMemoryModel"},
    {spv::CapabilityVulkanMemoryModelDeviceScope, "VkPhysicalDeviceVulkan12Features::vulkanMemoryModelDeviceScope"},
    {spv::CapabilityDenormPreserve, "(VkPhysicalDeviceVulkan12Properties::shaderDenormPreserveFloat16 == VK_TRUE) OR (VkPhysicalDeviceVulkan12Properties::shaderDenormPreserveFloat32 == VK_TRUE) OR (VkPhysicalDeviceVulkan12Properties::shaderDenormPreserveFloat64 == VK_TRUE)"},
    {spv::CapabilityDenormFlushToZero, "(VkPhysicalDeviceVulkan12Properties::shaderDenormFlushToZeroFloat16 == VK_TRUE) OR (VkPhysicalDeviceVulkan12Properties::shaderDenormFlushToZeroFloat32 == VK_TRUE) OR (VkPhysicalDeviceVulkan12Properties::shaderDenormFlushToZeroFloat64 == VK_TRUE)"},
    {spv::CapabilitySignedZeroInfNanPreserve, "(VkPhysicalDeviceVulkan12Properties::shaderSignedZeroInfNanPreserveFloat16 == VK_TRUE) OR (VkPhysicalDeviceVulkan12Properties::shaderSignedZeroInfNanPreserveFloat32 == VK_TRUE) OR (VkPhysicalDeviceVulkan12Properties::shaderSignedZeroInfNanPreserveFloat64 == VK_TRUE)"},
    {spv::CapabilityRoundingModeRTE, "(VkPhysicalDeviceVulkan12Properties::shaderRoundingModeRTEFloat16 == VK_TRUE) OR (VkPhysicalDeviceVulkan12Properties::shaderRoundingModeRTEFloat32 == VK_TRUE) OR (VkPhysicalDeviceVulkan12Properties::shaderRoundingModeRTEFloat64 == VK_TRUE)"},
    {spv::CapabilityRoundingModeRTZ, "(VkPhysicalDeviceVulkan12Properties::shaderRoundingModeRTZFloat16 == VK_TRUE) OR (VkPhysicalDeviceVulkan12Properties::shaderRoundingModeRTZFloat32 == VK_TRUE) OR (VkPhysicalDeviceVulkan12Properties::shaderRoundingModeRTZFloat64 == VK_TRUE)"},
    {spv::CapabilityComputeDerivativeGroupQuadsNV, "VkPhysicalDeviceComputeShaderDerivativesFeaturesNV::computeDerivativeGroupQuads"},
    {spv::CapabilityComputeDerivativeGroupLinearNV, "VkPhysicalDeviceComputeShaderDerivativesFeaturesNV::computeDerivativeGroupLinear"},
    {spv::CapabilityFragmentBarycentricNV, "VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV::fragmentShaderBarycentric"},
    {spv::CapabilityImageFootprintNV, "VkPhysicalDeviceShaderImageFootprintFeaturesNV::imageFootprint"},
    {spv::CapabilityShadingRateNV, "VkPhysicalDeviceShadingRateImageFeaturesNV::shadingRateImage"},
    {spv::CapabilityMeshShadingNV, "VK_NV_mesh_shader"},
    {spv::CapabilityRayTracingKHR, "VkPhysicalDeviceRayTracingPipelineFeaturesKHR::rayTracingPipeline"},
    {spv::CapabilityRayQueryKHR, "VkPhysicalDeviceRayQueryFeaturesKHR::rayQuery"},
    {spv::CapabilityRayTraversalPrimitiveCullingKHR, "VkPhysicalDeviceRayTracingPipelineFeaturesKHR::rayTraversalPrimitiveCulling OR VkPhysicalDeviceRayQueryFeaturesKHR::rayQuery"},
    {spv::CapabilityRayCullMaskKHR, "VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR::rayTracingMaintenance1"},
    {spv::CapabilityRayTracingNV, "VK_NV_ray_tracing"},
    {spv::CapabilityRayTracingMotionBlurNV, "VkPhysicalDeviceRayTracingMotionBlurFeaturesNV::rayTracingMotionBlur"},
    {spv::CapabilityTransformFeedback, "VkPhysicalDeviceTransformFeedbackFeaturesEXT::transformFeedback"},
    {spv::CapabilityGeometryStreams, "VkPhysicalDeviceTransformFeedbackFeaturesEXT::geometryStreams"},
    {spv::CapabilityFragmentDensityEXT, "VkPhysicalDeviceFragmentDensityMapFeaturesEXT::fragmentDensityMap"},
    {spv::CapabilityPhysicalStorageBufferAddresses, "VkPhysicalDeviceVulkan12Features::bufferDeviceAddress OR VkPhysicalDeviceBufferDeviceAddressFeaturesEXT::bufferDeviceAddress"},
    {spv::CapabilityCooperativeMatrixNV, "VkPhysicalDeviceCooperativeMatrixFeaturesNV::cooperativeMatrix"},
    {spv::CapabilityIntegerFunctions2INTEL, "VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL::shaderIntegerFunctions2"},
    {spv::CapabilityShaderSMBuiltinsNV, "VkPhysicalDeviceShaderSMBuiltinsFeaturesNV::shaderSMBuiltins"},
    {spv::CapabilityFragmentShaderSampleInterlockEXT, "VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::fragmentShaderSampleInterlock"},
    {spv::CapabilityFragmentShaderPixelInterlockEXT, "VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::fragmentShaderPixelInterlock"},
    {spv::CapabilityFragmentShaderShadingRateInterlockEXT, "VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::fragmentShaderShadingRateInterlock OR VkPhysicalDeviceShadingRateImageFeaturesNV::shadingRateImage"},
    {spv::CapabilityDemoteToHelperInvocationEXT, "VkPhysicalDeviceVulkan13Features::shaderDemoteToHelperInvocation OR VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT::shaderDemoteToHelperInvocation"},
    {spv::CapabilityFragmentShadingRateKHR, "VkPhysicalDeviceFragmentShadingRateFeaturesKHR::pipelineFragmentShadingRate OR VkPhysicalDeviceFragmentShadingRateFeaturesKHR::primitiveFragmentShadingRate OR VkPhysicalDeviceFragmentShadingRateFeaturesKHR::attachmentFragmentShadingRate"},
    {spv::CapabilityWorkgroupMemoryExplicitLayoutKHR, "VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::workgroupMemoryExplicitLayout"},
    {spv::CapabilityWorkgroupMemoryExplicitLayout8BitAccessKHR, "VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::workgroupMemoryExplicitLayout8BitAccess"},
    {spv::CapabilityWorkgroupMemoryExplicitLayout16BitAccessKHR, "VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::workgroupMemoryExplicitLayout16BitAccess"},
    {spv::CapabilityDotProductInputAllKHR, "VkPhysicalDeviceVulkan13Features::shaderIntegerDotProduct OR VkPhysicalDeviceShaderIntegerDotProductFeaturesKHR::shaderIntegerDotProduct"},
    {spv::CapabilityDotProductInput4x8BitKHR, "VkPhysicalDeviceVulkan13Features::shaderIntegerDotProduct OR VkPhysicalDeviceShaderIntegerDotProductFeaturesKHR::shaderIntegerDotProduct"},
    {spv::CapabilityDotProductInput4x8BitPackedKHR, "VkPhysicalDeviceVulkan13Features::shaderIntegerDotProduct OR VkPhysicalDeviceShaderIntegerDotProductFeaturesKHR::shaderIntegerDotProduct"},
    {spv::CapabilityDotProductKHR, "VkPhysicalDeviceVulkan13Features::shaderIntegerDotProduct OR VkPhysicalDeviceShaderIntegerDotProductFeaturesKHR::shaderIntegerDotProduct"},
    {spv::CapabilityFragmentBarycentricKHR, "VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR::fragmentShaderBarycentric"},
    {spv::CapabilityTextureSampleWeightedQCOM, "VkPhysicalDeviceImageProcessingFeaturesQCOM::textureSampleWeighted"},
    {spv::CapabilityTextureBoxFilterQCOM, "VkPhysicalDeviceImageProcessingFeaturesQCOM::textureBoxFilter"},
    {spv::CapabilityTextureBlockMatchQCOM, "VkPhysicalDeviceImageProcessingFeaturesQCOM::textureBlockMatch"},
    {spv::CapabilityMeshShadingEXT, "VK_EXT_mesh_shader"},
    {spv::CapabilityRayTracingOpacityMicromapEXT, "VK_EXT_opacity_micromap"},
    {spv::CapabilityCoreBuiltinsARM, "VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM::shaderCoreBuiltins"},
    {spv::CapabilityShaderInvocationReorderNV, "VK_NV_ray_tracing_invocation_reorder"},
    {spv::CapabilityRayTracingPositionFetchKHR, "VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR::rayTracingPositionFetch"},
    {spv::CapabilityTileImageColorReadAccessEXT, "VkPhysicalDeviceShaderTileImageFeaturesEXT::shaderTileImageColorReadAccess"},
    {spv::CapabilityTileImageDepthReadAccessEXT, "VkPhysicalDeviceShaderTileImageFeaturesEXT::shaderTileImageDepthReadAccess"},
    {spv::CapabilityTileImageStencilReadAccessEXT, "VkPhysicalDeviceShaderTileImageFeaturesEXT::shaderTileImageStencilReadAccess"},
    {spv::CapabilityCooperativeMatrixKHR, "VkPhysicalDeviceCooperativeMatrixFeaturesKHR::cooperativeMatrix"},
    };

    // VUs before catch unknown capabilities
    const auto entry = table.find(capability);
    return entry->second.data();
}
// clang-format on

// clang-format off
static inline const char* SpvExtensionRequirments(std::string_view extension) {
    static const vvl::unordered_map<std::string_view, std::string_view> table {
    {"SPV_KHR_variable_pointers", "VK_VERSION_1_1 OR VK_KHR_variable_pointers"},
    {"SPV_AMD_shader_explicit_vertex_parameter", "VK_AMD_shader_explicit_vertex_parameter"},
    {"SPV_AMD_gcn_shader", "VK_AMD_gcn_shader"},
    {"SPV_AMD_gpu_shader_half_float", "VK_AMD_gpu_shader_half_float"},
    {"SPV_AMD_gpu_shader_int16", "VK_AMD_gpu_shader_int16"},
    {"SPV_AMD_shader_ballot", "VK_AMD_shader_ballot"},
    {"SPV_AMD_shader_fragment_mask", "VK_AMD_shader_fragment_mask"},
    {"SPV_AMD_shader_image_load_store_lod", "VK_AMD_shader_image_load_store_lod"},
    {"SPV_AMD_shader_trinary_minmax", "VK_AMD_shader_trinary_minmax"},
    {"SPV_AMD_texture_gather_bias_lod", "VK_AMD_texture_gather_bias_lod"},
    {"SPV_AMD_shader_early_and_late_fragment_tests", "VK_AMD_shader_early_and_late_fragment_tests"},
    {"SPV_KHR_shader_draw_parameters", "VK_VERSION_1_1 OR VK_KHR_shader_draw_parameters"},
    {"SPV_KHR_8bit_storage", "VK_VERSION_1_2 OR VK_KHR_8bit_storage"},
    {"SPV_KHR_16bit_storage", "VK_VERSION_1_1 OR VK_KHR_16bit_storage"},
    {"SPV_KHR_shader_clock", "VK_KHR_shader_clock"},
    {"SPV_KHR_float_controls", "VK_VERSION_1_2 OR VK_KHR_shader_float_controls"},
    {"SPV_KHR_storage_buffer_storage_class", "VK_VERSION_1_1 OR VK_KHR_storage_buffer_storage_class"},
    {"SPV_KHR_post_depth_coverage", "VK_EXT_post_depth_coverage"},
    {"SPV_EXT_shader_stencil_export", "VK_EXT_shader_stencil_export"},
    {"SPV_KHR_shader_ballot", "VK_EXT_shader_subgroup_ballot"},
    {"SPV_KHR_subgroup_vote", "VK_EXT_shader_subgroup_vote"},
    {"SPV_NV_sample_mask_override_coverage", "VK_NV_sample_mask_override_coverage"},
    {"SPV_NV_geometry_shader_passthrough", "VK_NV_geometry_shader_passthrough"},
    {"SPV_NV_mesh_shader", "VK_NV_mesh_shader"},
    {"SPV_NV_viewport_array2", "VK_NV_viewport_array2"},
    {"SPV_NV_shader_subgroup_partitioned", "VK_NV_shader_subgroup_partitioned"},
    {"SPV_NV_shader_invocation_reorder", "VK_NV_ray_tracing_invocation_reorder"},
    {"SPV_EXT_shader_viewport_index_layer", "VK_VERSION_1_2 OR VK_EXT_shader_viewport_index_layer"},
    {"SPV_NVX_multiview_per_view_attributes", "VK_NVX_multiview_per_view_attributes"},
    {"SPV_EXT_descriptor_indexing", "VK_VERSION_1_2 OR VK_EXT_descriptor_indexing"},
    {"SPV_KHR_vulkan_memory_model", "VK_VERSION_1_2 OR VK_KHR_vulkan_memory_model"},
    {"SPV_NV_compute_shader_derivatives", "VK_NV_compute_shader_derivatives"},
    {"SPV_NV_fragment_shader_barycentric", "VK_NV_fragment_shader_barycentric"},
    {"SPV_NV_shader_image_footprint", "VK_NV_shader_image_footprint"},
    {"SPV_NV_shading_rate", "VK_NV_shading_rate_image"},
    {"SPV_NV_ray_tracing", "VK_NV_ray_tracing"},
    {"SPV_KHR_ray_tracing", "VK_KHR_ray_tracing_pipeline"},
    {"SPV_KHR_ray_query", "VK_KHR_ray_query"},
    {"SPV_KHR_ray_cull_mask", "VK_KHR_ray_tracing_maintenance1"},
    {"SPV_GOOGLE_hlsl_functionality1", "VK_GOOGLE_hlsl_functionality1"},
    {"SPV_GOOGLE_user_type", "VK_GOOGLE_user_type"},
    {"SPV_GOOGLE_decorate_string", "VK_GOOGLE_decorate_string"},
    {"SPV_EXT_fragment_invocation_density", "VK_EXT_fragment_density_map"},
    {"SPV_KHR_physical_storage_buffer", "VK_VERSION_1_2 OR VK_KHR_buffer_device_address"},
    {"SPV_EXT_physical_storage_buffer", "VK_EXT_buffer_device_address"},
    {"SPV_NV_cooperative_matrix", "VK_NV_cooperative_matrix"},
    {"SPV_NV_shader_sm_builtins", "VK_NV_shader_sm_builtins"},
    {"SPV_EXT_fragment_shader_interlock", "VK_EXT_fragment_shader_interlock"},
    {"SPV_EXT_demote_to_helper_invocation", "VK_VERSION_1_3 OR VK_EXT_shader_demote_to_helper_invocation"},
    {"SPV_KHR_fragment_shading_rate", "VK_KHR_fragment_shading_rate"},
    {"SPV_KHR_non_semantic_info", "VK_VERSION_1_3 OR VK_KHR_shader_non_semantic_info"},
    {"SPV_EXT_shader_image_int64", "VK_EXT_shader_image_atomic_int64"},
    {"SPV_KHR_terminate_invocation", "VK_VERSION_1_3 OR VK_KHR_shader_terminate_invocation"},
    {"SPV_KHR_multiview", "VK_VERSION_1_1 OR VK_KHR_multiview"},
    {"SPV_KHR_workgroup_memory_explicit_layout", "VK_KHR_workgroup_memory_explicit_layout"},
    {"SPV_EXT_shader_atomic_float_add", "VK_EXT_shader_atomic_float"},
    {"SPV_KHR_fragment_shader_barycentric", "VK_KHR_fragment_shader_barycentric"},
    {"SPV_KHR_subgroup_uniform_control_flow", "VK_VERSION_1_3 OR VK_KHR_shader_subgroup_uniform_control_flow"},
    {"SPV_EXT_shader_atomic_float_min_max", "VK_EXT_shader_atomic_float2"},
    {"SPV_EXT_shader_atomic_float16_add", "VK_EXT_shader_atomic_float2"},
    {"SPV_EXT_fragment_fully_covered", "VK_EXT_conservative_rasterization"},
    {"SPV_KHR_integer_dot_product", "VK_VERSION_1_3 OR VK_KHR_shader_integer_dot_product"},
    {"SPV_INTEL_shader_integer_functions2", "VK_INTEL_shader_integer_functions2"},
    {"SPV_KHR_device_group", "VK_VERSION_1_1 OR VK_KHR_device_group"},
    {"SPV_QCOM_image_processing", "VK_QCOM_image_processing"},
    {"SPV_QCOM_image_processing2", "VK_QCOM_image_processing2"},
    {"SPV_EXT_mesh_shader", "VK_EXT_mesh_shader"},
    {"SPV_KHR_ray_tracing_position_fetch", "VK_KHR_ray_tracing_position_fetch"},
    {"SPV_EXT_shader_tile_image", "VK_EXT_shader_tile_image"},
    {"SPV_EXT_opacity_micromap", "VK_EXT_opacity_micromap"},
    {"SPV_KHR_cooperative_matrix", "VK_KHR_cooperative_matrix"},
    {"SPV_ARM_core_builtins", "VK_ARM_shader_core_builtins"},
    {"SPV_AMDX_shader_enqueue", "VK_AMDX_shader_enqueue"},
    };

    // VUs before catch unknown extensions
    const auto entry = table.find(extension);
    return entry->second.data();
}
// clang-format on

bool CoreChecks::ValidateShaderCapabilitiesAndExtensions(const Instruction &insn, const bool pipeline, const Location &loc) const {
    bool skip = false;

    if (insn.Opcode() == spv::OpCapability) {
        // All capabilities are generated so if it is not in the list it is not supported by Vulkan
        if (spirvCapabilities.count(insn.Word(1)) == 0) {
            const char *vuid = pipeline ? "VUID-VkShaderModuleCreateInfo-pCode-08739" : "VUID-VkShaderCreateInfoEXT-pCode-08739";
            skip |= LogError(vuid, device, loc, "SPIR-V has Capability (%s) declared, but this is not supported by Vulkan.",
                             string_SpvCapability(insn.Word(1)));
            return skip;  // no known capability to validate
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
                        has_support |=
                            ((phys_dev_props_core11.subgroupSupportedOperations & VK_SUBGROUP_FEATURE_ARITHMETIC_BIT) != 0);
                        break;
                    case spv::CapabilityGroupNonUniformBallot:
                        has_support |= ((phys_dev_props_core11.subgroupSupportedOperations & VK_SUBGROUP_FEATURE_BALLOT_BIT) != 0);
                        break;
                    case spv::CapabilityGroupNonUniformClustered:
                        has_support |=
                            ((phys_dev_props_core11.subgroupSupportedOperations & VK_SUBGROUP_FEATURE_CLUSTERED_BIT) != 0);
                        break;
                    case spv::CapabilityGroupNonUniformPartitionedNV:
                        has_support |=
                            ((phys_dev_props_core11.subgroupSupportedOperations & VK_SUBGROUP_FEATURE_PARTITIONED_BIT_NV) != 0);
                        break;
                    case spv::CapabilityGroupNonUniformQuad:
                        has_support |= ((phys_dev_props_core11.subgroupSupportedOperations & VK_SUBGROUP_FEATURE_QUAD_BIT) != 0);
                        break;
                    case spv::CapabilityGroupNonUniformShuffle:
                        has_support |= ((phys_dev_props_core11.subgroupSupportedOperations & VK_SUBGROUP_FEATURE_SHUFFLE_BIT) != 0);
                        break;
                    case spv::CapabilityGroupNonUniformShuffleRelative:
                        has_support |=
                            ((phys_dev_props_core11.subgroupSupportedOperations & VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT) != 0);
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
            const char *vuid = pipeline ? "VUID-VkShaderModuleCreateInfo-pCode-08740" : "VUID-VkShaderCreateInfoEXT-pCode-08740";
            skip |= LogError(vuid, device, loc,
                             "SPIR-V Capability %s was declared, but one of the following requirements is required (%s).",
                             string_SpvCapability(insn.Word(1)), SpvCapabilityRequirements(insn.Word(1)));
        }

        // Portability checks
        if (IsExtEnabled(device_extensions.vk_khr_portability_subset)) {
            if ((VK_FALSE == enabled_features.shaderSampleRateInterpolationFunctions) &&
                (spv::CapabilityInterpolationFunction == insn.Word(1))) {
                skip |= LogError("VUID-RuntimeSpirv-shaderSampleRateInterpolationFunctions-06325", device, loc,
                                 "SPIR-V (portability error) InterpolationFunction Capability are not supported "
                                 "by this platform");
            }
        }
    } else if (insn.Opcode() == spv::OpExtension) {
        static const std::string spv_prefix = "SPV_";
        std::string extension_name = insn.GetAsString(1);

        if (0 == extension_name.compare(0, spv_prefix.size(), spv_prefix)) {
            if (spirvExtensions.count(extension_name) == 0) {
                const char *vuid =
                    pipeline ? "VUID-VkShaderModuleCreateInfo-pCode-08741" : "VUID-VkShaderCreateInfoEXT-pCode-08741";
                skip |= LogError(vuid, device, loc, "SPIR-V Extension %s was declared, but that is not supported by Vulkan.",
                                 extension_name.c_str());
                return skip;  // no known extension to validate
            }
        } else {
            const char *vuid = pipeline ? "VUID-VkShaderModuleCreateInfo-pCode-08741" : "VUID-VkShaderCreateInfoEXT-pCode-08741";
            skip |= LogError(vuid, device, loc,
                             "SPIR-V Extension %s was declared, but this is not a SPIR-V extension. Please use a SPIR-V"
                             " extension (https://github.com/KhronosGroup/SPIRV-Registry) for OpExtension instructions. Non-SPIR-V "
                             "extensions can be"
                             " recorded in SPIR-V using the OpSourceExtension instruction.",
                             extension_name.c_str());
            return skip;  // no known extension to validate
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
            }
        }

        if (has_support == false) {
            const char *vuid = pipeline ? "VUID-VkShaderModuleCreateInfo-pCode-08742" : "VUID-VkShaderCreateInfoEXT-pCode-08742";
            skip |= LogError(vuid, device, loc,
                             "SPIR-V Extension %s was declared, but one of the following requirements is required (%s).",
                             extension_name.c_str(), SpvExtensionRequirments(extension_name));
        }
    }  // spv::OpExtension
    return skip;
}
// NOLINTEND
