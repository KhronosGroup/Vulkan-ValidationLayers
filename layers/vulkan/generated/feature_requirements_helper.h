// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See feature_requirements.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
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
 ****************************************************************************/

// NOLINTBEGIN

#include "vk_api_version.h"

#include <vulkan/vulkan.h>

namespace vkt {
enum class Feature {
    // VkPhysicalDevice16BitStorageFeatures, VkPhysicalDeviceVulkan11Features
    storageBuffer16BitAccess,
    // VkPhysicalDevice16BitStorageFeatures, VkPhysicalDeviceVulkan11Features
    storageInputOutput16,
    // VkPhysicalDevice16BitStorageFeatures, VkPhysicalDeviceVulkan11Features
    storagePushConstant16,
    // VkPhysicalDevice16BitStorageFeatures, VkPhysicalDeviceVulkan11Features
    uniformAndStorageBuffer16BitAccess,
    // VkPhysicalDevice4444FormatsFeaturesEXT
    formatA4B4G4R4,
    // VkPhysicalDevice4444FormatsFeaturesEXT
    formatA4R4G4B4,
    // VkPhysicalDevice8BitStorageFeatures, VkPhysicalDeviceVulkan12Features
    storageBuffer8BitAccess,
    // VkPhysicalDevice8BitStorageFeatures, VkPhysicalDeviceVulkan12Features
    storagePushConstant8,
    // VkPhysicalDevice8BitStorageFeatures, VkPhysicalDeviceVulkan12Features
    uniformAndStorageBuffer8BitAccess,
    // VkPhysicalDeviceASTCDecodeFeaturesEXT
    decodeModeSharedExponent,
    // VkPhysicalDeviceAccelerationStructureFeaturesKHR
    accelerationStructure,
    // VkPhysicalDeviceAccelerationStructureFeaturesKHR
    accelerationStructureCaptureReplay,
    // VkPhysicalDeviceAccelerationStructureFeaturesKHR
    accelerationStructureHostCommands,
    // VkPhysicalDeviceAccelerationStructureFeaturesKHR
    accelerationStructureIndirectBuild,
    // VkPhysicalDeviceAccelerationStructureFeaturesKHR
    descriptorBindingAccelerationStructureUpdateAfterBind,
    // VkPhysicalDeviceAddressBindingReportFeaturesEXT
    reportAddressBinding,
    // VkPhysicalDeviceAmigoProfilingFeaturesSEC
    amigoProfiling,
    // VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT
    attachmentFeedbackLoopDynamicState,
    // VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT
    attachmentFeedbackLoopLayout,
    // VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT
    advancedBlendCoherentOperations,
    // VkPhysicalDeviceBorderColorSwizzleFeaturesEXT
    borderColorSwizzle,
    // VkPhysicalDeviceBorderColorSwizzleFeaturesEXT
    borderColorSwizzleFromImage,
    // VkPhysicalDeviceBufferDeviceAddressFeatures, VkPhysicalDeviceVulkan12Features
    bufferDeviceAddress,
    // VkPhysicalDeviceBufferDeviceAddressFeatures, VkPhysicalDeviceVulkan12Features
    bufferDeviceAddressCaptureReplay,
    // VkPhysicalDeviceBufferDeviceAddressFeatures, VkPhysicalDeviceVulkan12Features
    bufferDeviceAddressMultiDevice,
    // VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI
    clustercullingShader,
    // VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI
    multiviewClusterCullingShader,
    // VkPhysicalDeviceCoherentMemoryFeaturesAMD
    deviceCoherentMemory,
    // VkPhysicalDeviceColorWriteEnableFeaturesEXT
    colorWriteEnable,
    // VkPhysicalDeviceComputeShaderDerivativesFeaturesNV
    computeDerivativeGroupLinear,
    // VkPhysicalDeviceComputeShaderDerivativesFeaturesNV
    computeDerivativeGroupQuads,
    // VkPhysicalDeviceConditionalRenderingFeaturesEXT
    conditionalRendering,
    // VkPhysicalDeviceConditionalRenderingFeaturesEXT
    inheritedConditionalRendering,
    // VkPhysicalDeviceCooperativeMatrixFeaturesKHR, VkPhysicalDeviceCooperativeMatrixFeaturesNV
    cooperativeMatrix,
    // VkPhysicalDeviceCooperativeMatrixFeaturesKHR, VkPhysicalDeviceCooperativeMatrixFeaturesNV
    cooperativeMatrixRobustBufferAccess,
    // VkPhysicalDeviceCopyMemoryIndirectFeaturesNV
    indirectCopy,
    // VkPhysicalDeviceCornerSampledImageFeaturesNV
    cornerSampledImage,
    // VkPhysicalDeviceCoverageReductionModeFeaturesNV
    coverageReductionMode,
    // VkPhysicalDeviceCubicClampFeaturesQCOM
    cubicRangeClamp,
    // VkPhysicalDeviceCubicWeightsFeaturesQCOM
    selectableCubicWeights,
    // VkPhysicalDeviceCudaKernelLaunchFeaturesNV
    cudaKernelLaunchFeatures,
    // VkPhysicalDeviceCustomBorderColorFeaturesEXT
    customBorderColorWithoutFormat,
    // VkPhysicalDeviceCustomBorderColorFeaturesEXT
    customBorderColors,
    // VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV
    dedicatedAllocationImageAliasing,
    // VkPhysicalDeviceDepthBiasControlFeaturesEXT
    depthBiasControl,
    // VkPhysicalDeviceDepthBiasControlFeaturesEXT
    depthBiasExact,
    // VkPhysicalDeviceDepthBiasControlFeaturesEXT
    floatRepresentation,
    // VkPhysicalDeviceDepthBiasControlFeaturesEXT
    leastRepresentableValueForceUnormRepresentation,
    // VkPhysicalDeviceDepthClampZeroOneFeaturesEXT
    depthClampZeroOne,
    // VkPhysicalDeviceDepthClipControlFeaturesEXT
    depthClipControl,
    // VkPhysicalDeviceDepthClipEnableFeaturesEXT
    depthClipEnable,
    // VkPhysicalDeviceDescriptorBufferFeaturesEXT
    descriptorBuffer,
    // VkPhysicalDeviceDescriptorBufferFeaturesEXT
    descriptorBufferCaptureReplay,
    // VkPhysicalDeviceDescriptorBufferFeaturesEXT
    descriptorBufferImageLayoutIgnored,
    // VkPhysicalDeviceDescriptorBufferFeaturesEXT
    descriptorBufferPushDescriptors,
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    descriptorBindingPartiallyBound,
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    descriptorBindingSampledImageUpdateAfterBind,
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    descriptorBindingStorageBufferUpdateAfterBind,
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    descriptorBindingStorageImageUpdateAfterBind,
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    descriptorBindingStorageTexelBufferUpdateAfterBind,
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    descriptorBindingUniformBufferUpdateAfterBind,
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    descriptorBindingUniformTexelBufferUpdateAfterBind,
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    descriptorBindingUpdateUnusedWhilePending,
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    descriptorBindingVariableDescriptorCount,
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    runtimeDescriptorArray,
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    shaderInputAttachmentArrayDynamicIndexing,
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    shaderInputAttachmentArrayNonUniformIndexing,
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    shaderSampledImageArrayNonUniformIndexing,
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    shaderStorageBufferArrayNonUniformIndexing,
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    shaderStorageImageArrayNonUniformIndexing,
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    shaderStorageTexelBufferArrayDynamicIndexing,
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    shaderStorageTexelBufferArrayNonUniformIndexing,
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    shaderUniformBufferArrayNonUniformIndexing,
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    shaderUniformTexelBufferArrayDynamicIndexing,
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    shaderUniformTexelBufferArrayNonUniformIndexing,
    // VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV
    descriptorPoolOverallocation,
    // VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE
    descriptorSetHostMapping,
    // VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV
    deviceGeneratedCompute,
    // VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV
    deviceGeneratedComputeCaptureReplay,
    // VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV
    deviceGeneratedComputePipelines,
    // VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV
    deviceGeneratedCommands,
    // VkPhysicalDeviceDeviceMemoryReportFeaturesEXT
    deviceMemoryReport,
    // VkPhysicalDeviceDiagnosticsConfigFeaturesNV
    diagnosticsConfig,
    // VkPhysicalDeviceDisplacementMicromapFeaturesNV
    displacementMicromap,
    // VkPhysicalDeviceDynamicRenderingFeatures, VkPhysicalDeviceVulkan13Features
    dynamicRendering,
    // VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT
    dynamicRenderingUnusedAttachments,
    // VkPhysicalDeviceExclusiveScissorFeaturesNV
    exclusiveScissor,
    // VkPhysicalDeviceExtendedDynamicState2FeaturesEXT
    extendedDynamicState2,
    // VkPhysicalDeviceExtendedDynamicState2FeaturesEXT
    extendedDynamicState2LogicOp,
    // VkPhysicalDeviceExtendedDynamicState2FeaturesEXT
    extendedDynamicState2PatchControlPoints,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3AlphaToCoverageEnable,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3AlphaToOneEnable,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3ColorBlendAdvanced,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3ColorBlendEnable,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3ColorBlendEquation,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3ColorWriteMask,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3ConservativeRasterizationMode,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3CoverageModulationMode,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3CoverageModulationTable,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3CoverageModulationTableEnable,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3CoverageReductionMode,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3CoverageToColorEnable,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3CoverageToColorLocation,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3DepthClampEnable,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3DepthClipEnable,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3DepthClipNegativeOneToOne,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3ExtraPrimitiveOverestimationSize,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3LineRasterizationMode,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3LineStippleEnable,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3LogicOpEnable,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3PolygonMode,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3ProvokingVertexMode,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3RasterizationSamples,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3RasterizationStream,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3RepresentativeFragmentTestEnable,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3SampleLocationsEnable,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3SampleMask,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3ShadingRateImageEnable,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3TessellationDomainOrigin,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3ViewportSwizzle,
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    extendedDynamicState3ViewportWScalingEnable,
    // VkPhysicalDeviceExtendedDynamicStateFeaturesEXT
    extendedDynamicState,
    // VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV
    extendedSparseAddressSpace,
    // VkPhysicalDeviceExternalFormatResolveFeaturesANDROID
    externalFormatResolve,
    // VkPhysicalDeviceExternalMemoryRDMAFeaturesNV
    externalMemoryRDMA,
    // VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX
    screenBufferImport,
    // VkPhysicalDeviceFaultFeaturesEXT
    deviceFault,
    // VkPhysicalDeviceFaultFeaturesEXT
    deviceFaultVendorBinary,
    // VkPhysicalDeviceFeatures
    alphaToOne,
    // VkPhysicalDeviceFeatures
    depthBiasClamp,
    // VkPhysicalDeviceFeatures
    depthBounds,
    // VkPhysicalDeviceFeatures
    depthClamp,
    // VkPhysicalDeviceFeatures
    drawIndirectFirstInstance,
    // VkPhysicalDeviceFeatures
    dualSrcBlend,
    // VkPhysicalDeviceFeatures
    fillModeNonSolid,
    // VkPhysicalDeviceFeatures
    fragmentStoresAndAtomics,
    // VkPhysicalDeviceFeatures
    fullDrawIndexUint32,
    // VkPhysicalDeviceFeatures
    geometryShader,
    // VkPhysicalDeviceFeatures
    imageCubeArray,
    // VkPhysicalDeviceFeatures
    independentBlend,
    // VkPhysicalDeviceFeatures
    inheritedQueries,
    // VkPhysicalDeviceFeatures
    largePoints,
    // VkPhysicalDeviceFeatures
    logicOp,
    // VkPhysicalDeviceFeatures
    multiDrawIndirect,
    // VkPhysicalDeviceFeatures
    multiViewport,
    // VkPhysicalDeviceFeatures
    occlusionQueryPrecise,
    // VkPhysicalDeviceFeatures
    pipelineStatisticsQuery,
    // VkPhysicalDeviceFeatures
    robustBufferAccess,
    // VkPhysicalDeviceFeatures
    sampleRateShading,
    // VkPhysicalDeviceFeatures
    samplerAnisotropy,
    // VkPhysicalDeviceFeatures
    shaderClipDistance,
    // VkPhysicalDeviceFeatures
    shaderCullDistance,
    // VkPhysicalDeviceFeatures
    shaderFloat64,
    // VkPhysicalDeviceFeatures
    shaderImageGatherExtended,
    // VkPhysicalDeviceFeatures
    shaderInt16,
    // VkPhysicalDeviceFeatures
    shaderInt64,
    // VkPhysicalDeviceFeatures
    shaderResourceMinLod,
    // VkPhysicalDeviceFeatures
    shaderResourceResidency,
    // VkPhysicalDeviceFeatures
    shaderSampledImageArrayDynamicIndexing,
    // VkPhysicalDeviceFeatures
    shaderStorageBufferArrayDynamicIndexing,
    // VkPhysicalDeviceFeatures
    shaderStorageImageArrayDynamicIndexing,
    // VkPhysicalDeviceFeatures
    shaderStorageImageExtendedFormats,
    // VkPhysicalDeviceFeatures
    shaderStorageImageMultisample,
    // VkPhysicalDeviceFeatures
    shaderStorageImageReadWithoutFormat,
    // VkPhysicalDeviceFeatures
    shaderStorageImageWriteWithoutFormat,
    // VkPhysicalDeviceFeatures
    shaderTessellationAndGeometryPointSize,
    // VkPhysicalDeviceFeatures
    shaderUniformBufferArrayDynamicIndexing,
    // VkPhysicalDeviceFeatures
    sparseBinding,
    // VkPhysicalDeviceFeatures
    sparseResidency16Samples,
    // VkPhysicalDeviceFeatures
    sparseResidency2Samples,
    // VkPhysicalDeviceFeatures
    sparseResidency4Samples,
    // VkPhysicalDeviceFeatures
    sparseResidency8Samples,
    // VkPhysicalDeviceFeatures
    sparseResidencyAliased,
    // VkPhysicalDeviceFeatures
    sparseResidencyBuffer,
    // VkPhysicalDeviceFeatures
    sparseResidencyImage2D,
    // VkPhysicalDeviceFeatures
    sparseResidencyImage3D,
    // VkPhysicalDeviceFeatures
    tessellationShader,
    // VkPhysicalDeviceFeatures
    textureCompressionASTC_LDR,
    // VkPhysicalDeviceFeatures
    textureCompressionBC,
    // VkPhysicalDeviceFeatures
    textureCompressionETC2,
    // VkPhysicalDeviceFeatures
    variableMultisampleRate,
    // VkPhysicalDeviceFeatures
    vertexPipelineStoresAndAtomics,
    // VkPhysicalDeviceFeatures
    wideLines,
    // VkPhysicalDeviceFragmentDensityMap2FeaturesEXT
    fragmentDensityMapDeferred,
    // VkPhysicalDeviceFragmentDensityMapFeaturesEXT
    fragmentDensityMap,
    // VkPhysicalDeviceFragmentDensityMapFeaturesEXT
    fragmentDensityMapDynamic,
    // VkPhysicalDeviceFragmentDensityMapFeaturesEXT
    fragmentDensityMapNonSubsampledImages,
    // VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM
    fragmentDensityMapOffset,
    // VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR
    fragmentShaderBarycentric,
    // VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT
    fragmentShaderPixelInterlock,
    // VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT
    fragmentShaderSampleInterlock,
    // VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT
    fragmentShaderShadingRateInterlock,
    // VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV
    fragmentShadingRateEnums,
    // VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV
    noInvocationFragmentShadingRates,
    // VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV
    supersampleFragmentShadingRates,
    // VkPhysicalDeviceFragmentShadingRateFeaturesKHR
    attachmentFragmentShadingRate,
    // VkPhysicalDeviceFragmentShadingRateFeaturesKHR
    pipelineFragmentShadingRate,
    // VkPhysicalDeviceFragmentShadingRateFeaturesKHR
    primitiveFragmentShadingRate,
    // VkPhysicalDeviceFrameBoundaryFeaturesEXT
    frameBoundary,
    // VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR
    globalPriorityQuery,
    // VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT
    graphicsPipelineLibrary,
    // VkPhysicalDeviceHostImageCopyFeaturesEXT
    hostImageCopy,
    // VkPhysicalDeviceHostQueryResetFeatures, VkPhysicalDeviceVulkan12Features
    hostQueryReset,
    // VkPhysicalDeviceImage2DViewOf3DFeaturesEXT
    image2DViewOf3D,
    // VkPhysicalDeviceImage2DViewOf3DFeaturesEXT
    sampler2DViewOf3D,
    // VkPhysicalDeviceImageCompressionControlFeaturesEXT
    imageCompressionControl,
    // VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT
    imageCompressionControlSwapchain,
    // VkPhysicalDeviceImageProcessing2FeaturesQCOM
    textureBlockMatch2,
    // VkPhysicalDeviceImageProcessingFeaturesQCOM
    textureBlockMatch,
    // VkPhysicalDeviceImageProcessingFeaturesQCOM
    textureBoxFilter,
    // VkPhysicalDeviceImageProcessingFeaturesQCOM
    textureSampleWeighted,
    // VkPhysicalDeviceImageRobustnessFeatures, VkPhysicalDeviceVulkan13Features
    robustImageAccess,
    // VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT
    imageSlicedViewOf3D,
    // VkPhysicalDeviceImageViewMinLodFeaturesEXT
    minLod,
    // VkPhysicalDeviceImagelessFramebufferFeatures, VkPhysicalDeviceVulkan12Features
    imagelessFramebuffer,
    // VkPhysicalDeviceIndexTypeUint8FeaturesEXT
    indexTypeUint8,
    // VkPhysicalDeviceInheritedViewportScissorFeaturesNV
    inheritedViewportScissor2D,
    // VkPhysicalDeviceInlineUniformBlockFeatures, VkPhysicalDeviceVulkan13Features
    descriptorBindingInlineUniformBlockUpdateAfterBind,
    // VkPhysicalDeviceInlineUniformBlockFeatures, VkPhysicalDeviceVulkan13Features
    inlineUniformBlock,
    // VkPhysicalDeviceInvocationMaskFeaturesHUAWEI
    invocationMask,
    // VkPhysicalDeviceLegacyDitheringFeaturesEXT
    legacyDithering,
    // VkPhysicalDeviceLineRasterizationFeaturesEXT
    bresenhamLines,
    // VkPhysicalDeviceLineRasterizationFeaturesEXT
    rectangularLines,
    // VkPhysicalDeviceLineRasterizationFeaturesEXT
    smoothLines,
    // VkPhysicalDeviceLineRasterizationFeaturesEXT
    stippledBresenhamLines,
    // VkPhysicalDeviceLineRasterizationFeaturesEXT
    stippledRectangularLines,
    // VkPhysicalDeviceLineRasterizationFeaturesEXT
    stippledSmoothLines,
    // VkPhysicalDeviceLinearColorAttachmentFeaturesNV
    linearColorAttachment,
    // VkPhysicalDeviceMaintenance4Features, VkPhysicalDeviceVulkan13Features
    maintenance4,
    // VkPhysicalDeviceMaintenance5FeaturesKHR
    maintenance5,
    // VkPhysicalDeviceMaintenance6FeaturesKHR
    maintenance6,
    // VkPhysicalDeviceMemoryDecompressionFeaturesNV
    memoryDecompression,
    // VkPhysicalDeviceMemoryPriorityFeaturesEXT
    memoryPriority,
    // VkPhysicalDeviceMeshShaderFeaturesEXT
    meshShaderQueries,
    // VkPhysicalDeviceMeshShaderFeaturesEXT
    multiviewMeshShader,
    // VkPhysicalDeviceMeshShaderFeaturesEXT
    primitiveFragmentShadingRateMeshShader,
    // VkPhysicalDeviceMeshShaderFeaturesEXT, VkPhysicalDeviceMeshShaderFeaturesNV
    meshShader,
    // VkPhysicalDeviceMeshShaderFeaturesEXT, VkPhysicalDeviceMeshShaderFeaturesNV
    taskShader,
    // VkPhysicalDeviceMultiDrawFeaturesEXT
    multiDraw,
    // VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT
    multisampledRenderToSingleSampled,
    // VkPhysicalDeviceMultiviewFeatures, VkPhysicalDeviceVulkan11Features
    multiview,
    // VkPhysicalDeviceMultiviewFeatures, VkPhysicalDeviceVulkan11Features
    multiviewGeometryShader,
    // VkPhysicalDeviceMultiviewFeatures, VkPhysicalDeviceVulkan11Features
    multiviewTessellationShader,
    // VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM
    multiviewPerViewRenderAreas,
    // VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM
    multiviewPerViewViewports,
    // VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT
    mutableDescriptorType,
    // VkPhysicalDeviceNestedCommandBufferFeaturesEXT
    nestedCommandBuffer,
    // VkPhysicalDeviceNestedCommandBufferFeaturesEXT
    nestedCommandBufferRendering,
    // VkPhysicalDeviceNestedCommandBufferFeaturesEXT
    nestedCommandBufferSimultaneousUse,
    // VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT
    nonSeamlessCubeMap,
    // VkPhysicalDeviceOpacityMicromapFeaturesEXT
    micromap,
    // VkPhysicalDeviceOpacityMicromapFeaturesEXT
    micromapCaptureReplay,
    // VkPhysicalDeviceOpacityMicromapFeaturesEXT
    micromapHostCommands,
    // VkPhysicalDeviceOpticalFlowFeaturesNV
    opticalFlow,
    // VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT
    pageableDeviceLocalMemory,
    // VkPhysicalDevicePerStageDescriptorSetFeaturesNV
    dynamicPipelineLayout,
    // VkPhysicalDevicePerStageDescriptorSetFeaturesNV
    perStageDescriptorSet,
    // VkPhysicalDevicePerformanceQueryFeaturesKHR
    performanceCounterMultipleQueryPools,
    // VkPhysicalDevicePerformanceQueryFeaturesKHR
    performanceCounterQueryPools,
    // VkPhysicalDevicePipelineCreationCacheControlFeatures, VkPhysicalDeviceVulkan13Features
    pipelineCreationCacheControl,
    // VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR
    pipelineExecutableInfo,
    // VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT
    pipelineLibraryGroupHandles,
    // VkPhysicalDevicePipelinePropertiesFeaturesEXT
    pipelinePropertiesIdentifier,
    // VkPhysicalDevicePipelineProtectedAccessFeaturesEXT
    pipelineProtectedAccess,
    // VkPhysicalDevicePipelineRobustnessFeaturesEXT
    pipelineRobustness,
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    constantAlphaColorBlendFactors,
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    events,
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    imageView2DOn3DImage,
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    imageViewFormatReinterpretation,
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    imageViewFormatSwizzle,
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    multisampleArrayImage,
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    mutableComparisonSamplers,
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    pointPolygons,
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    samplerMipLodBias,
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    separateStencilMaskRef,
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    shaderSampleRateInterpolationFunctions,
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    tessellationIsolines,
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    tessellationPointMode,
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    triangleFans,
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    vertexAttributeAccessBeyondStride,
    // VkPhysicalDevicePresentBarrierFeaturesNV
    presentBarrier,
    // VkPhysicalDevicePresentIdFeaturesKHR
    presentId,
    // VkPhysicalDevicePresentWaitFeaturesKHR
    presentWait,
    // VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT
    primitiveTopologyListRestart,
    // VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT
    primitiveTopologyPatchListRestart,
    // VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT
    primitivesGeneratedQuery,
    // VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT
    primitivesGeneratedQueryWithNonZeroStreams,
    // VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT
    primitivesGeneratedQueryWithRasterizerDiscard,
    // VkPhysicalDevicePrivateDataFeatures, VkPhysicalDeviceVulkan13Features
    privateData,
    // VkPhysicalDeviceProtectedMemoryFeatures, VkPhysicalDeviceVulkan11Features
    protectedMemory,
    // VkPhysicalDeviceProvokingVertexFeaturesEXT
    provokingVertexLast,
    // VkPhysicalDeviceProvokingVertexFeaturesEXT
    transformFeedbackPreservesProvokingVertex,
    // VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT
    formatRgba10x6WithoutYCbCrSampler,
    // VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT
    rasterizationOrderColorAttachmentAccess,
    // VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT
    rasterizationOrderDepthAttachmentAccess,
    // VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT
    rasterizationOrderStencilAttachmentAccess,
    // VkPhysicalDeviceRayQueryFeaturesKHR
    rayQuery,
    // VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV
    rayTracingInvocationReorder,
    // VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR
    rayTracingMaintenance1,
    // VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR
    rayTracingPipelineTraceRaysIndirect2,
    // VkPhysicalDeviceRayTracingMotionBlurFeaturesNV
    rayTracingMotionBlur,
    // VkPhysicalDeviceRayTracingMotionBlurFeaturesNV
    rayTracingMotionBlurPipelineTraceRaysIndirect,
    // VkPhysicalDeviceRayTracingPipelineFeaturesKHR
    rayTracingPipeline,
    // VkPhysicalDeviceRayTracingPipelineFeaturesKHR
    rayTracingPipelineShaderGroupHandleCaptureReplay,
    // VkPhysicalDeviceRayTracingPipelineFeaturesKHR
    rayTracingPipelineShaderGroupHandleCaptureReplayMixed,
    // VkPhysicalDeviceRayTracingPipelineFeaturesKHR
    rayTracingPipelineTraceRaysIndirect,
    // VkPhysicalDeviceRayTracingPipelineFeaturesKHR
    rayTraversalPrimitiveCulling,
    // VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR
    rayTracingPositionFetch,
    // VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG
    relaxedLineRasterization,
    // VkPhysicalDeviceRenderPassStripedFeaturesARM
    renderPassStriped,
    // VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV
    representativeFragmentTest,
    // VkPhysicalDeviceRobustness2FeaturesEXT
    nullDescriptor,
    // VkPhysicalDeviceRobustness2FeaturesEXT
    robustBufferAccess2,
    // VkPhysicalDeviceRobustness2FeaturesEXT
    robustImageAccess2,
    // VkPhysicalDeviceSamplerYcbcrConversionFeatures, VkPhysicalDeviceVulkan11Features
    samplerYcbcrConversion,
    // VkPhysicalDeviceScalarBlockLayoutFeatures, VkPhysicalDeviceVulkan12Features
    scalarBlockLayout,
    // VkPhysicalDeviceSchedulingControlsFeaturesARM
    schedulingControls,
    // VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures, VkPhysicalDeviceVulkan12Features
    separateDepthStencilLayouts,
    // VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT
    shaderBufferFloat16AtomicAdd,
    // VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT
    shaderBufferFloat16AtomicMinMax,
    // VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT
    shaderBufferFloat16Atomics,
    // VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT
    shaderBufferFloat32AtomicMinMax,
    // VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT
    shaderBufferFloat64AtomicMinMax,
    // VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT
    shaderImageFloat32AtomicMinMax,
    // VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT
    shaderSharedFloat16AtomicAdd,
    // VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT
    shaderSharedFloat16AtomicMinMax,
    // VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT
    shaderSharedFloat16Atomics,
    // VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT
    shaderSharedFloat32AtomicMinMax,
    // VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT
    shaderSharedFloat64AtomicMinMax,
    // VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT
    sparseImageFloat32AtomicMinMax,
    // VkPhysicalDeviceShaderAtomicFloatFeaturesEXT
    shaderBufferFloat32AtomicAdd,
    // VkPhysicalDeviceShaderAtomicFloatFeaturesEXT
    shaderBufferFloat32Atomics,
    // VkPhysicalDeviceShaderAtomicFloatFeaturesEXT
    shaderBufferFloat64AtomicAdd,
    // VkPhysicalDeviceShaderAtomicFloatFeaturesEXT
    shaderBufferFloat64Atomics,
    // VkPhysicalDeviceShaderAtomicFloatFeaturesEXT
    shaderImageFloat32AtomicAdd,
    // VkPhysicalDeviceShaderAtomicFloatFeaturesEXT
    shaderImageFloat32Atomics,
    // VkPhysicalDeviceShaderAtomicFloatFeaturesEXT
    shaderSharedFloat32AtomicAdd,
    // VkPhysicalDeviceShaderAtomicFloatFeaturesEXT
    shaderSharedFloat32Atomics,
    // VkPhysicalDeviceShaderAtomicFloatFeaturesEXT
    shaderSharedFloat64AtomicAdd,
    // VkPhysicalDeviceShaderAtomicFloatFeaturesEXT
    shaderSharedFloat64Atomics,
    // VkPhysicalDeviceShaderAtomicFloatFeaturesEXT
    sparseImageFloat32AtomicAdd,
    // VkPhysicalDeviceShaderAtomicFloatFeaturesEXT
    sparseImageFloat32Atomics,
    // VkPhysicalDeviceShaderAtomicInt64Features, VkPhysicalDeviceVulkan12Features
    shaderBufferInt64Atomics,
    // VkPhysicalDeviceShaderAtomicInt64Features, VkPhysicalDeviceVulkan12Features
    shaderSharedInt64Atomics,
    // VkPhysicalDeviceShaderClockFeaturesKHR
    shaderDeviceClock,
    // VkPhysicalDeviceShaderClockFeaturesKHR
    shaderSubgroupClock,
    // VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM
    shaderCoreBuiltins,
    // VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures, VkPhysicalDeviceVulkan13Features
    shaderDemoteToHelperInvocation,
    // VkPhysicalDeviceShaderDrawParametersFeatures, VkPhysicalDeviceVulkan11Features
    shaderDrawParameters,
    // VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD
    shaderEarlyAndLateFragmentTests,
    // VkPhysicalDeviceShaderEnqueueFeaturesAMDX
    shaderEnqueue,
    // VkPhysicalDeviceShaderFloat16Int8Features, VkPhysicalDeviceVulkan12Features
    shaderFloat16,
    // VkPhysicalDeviceShaderFloat16Int8Features, VkPhysicalDeviceVulkan12Features
    shaderInt8,
    // VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT
    shaderImageInt64Atomics,
    // VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT
    sparseImageInt64Atomics,
    // VkPhysicalDeviceShaderImageFootprintFeaturesNV
    imageFootprint,
    // VkPhysicalDeviceShaderIntegerDotProductFeatures, VkPhysicalDeviceVulkan13Features
    shaderIntegerDotProduct,
    // VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL
    shaderIntegerFunctions2,
    // VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT
    shaderModuleIdentifier,
    // VkPhysicalDeviceShaderObjectFeaturesEXT
    shaderObject,
    // VkPhysicalDeviceShaderSMBuiltinsFeaturesNV
    shaderSMBuiltins,
    // VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures, VkPhysicalDeviceVulkan12Features
    shaderSubgroupExtendedTypes,
    // VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR
    shaderSubgroupUniformControlFlow,
    // VkPhysicalDeviceShaderTerminateInvocationFeatures, VkPhysicalDeviceVulkan13Features
    shaderTerminateInvocation,
    // VkPhysicalDeviceShaderTileImageFeaturesEXT
    shaderTileImageColorReadAccess,
    // VkPhysicalDeviceShaderTileImageFeaturesEXT
    shaderTileImageDepthReadAccess,
    // VkPhysicalDeviceShaderTileImageFeaturesEXT
    shaderTileImageStencilReadAccess,
    // VkPhysicalDeviceShadingRateImageFeaturesNV
    shadingRateCoarseSampleOrder,
    // VkPhysicalDeviceShadingRateImageFeaturesNV
    shadingRateImage,
    // VkPhysicalDeviceSubgroupSizeControlFeatures, VkPhysicalDeviceVulkan13Features
    computeFullSubgroups,
    // VkPhysicalDeviceSubgroupSizeControlFeatures, VkPhysicalDeviceVulkan13Features
    subgroupSizeControl,
    // VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT
    subpassMergeFeedback,
    // VkPhysicalDeviceSubpassShadingFeaturesHUAWEI
    subpassShading,
    // VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT
    swapchainMaintenance1,
    // VkPhysicalDeviceSynchronization2Features, VkPhysicalDeviceVulkan13Features
    synchronization2,
    // VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT
    texelBufferAlignment,
    // VkPhysicalDeviceTextureCompressionASTCHDRFeatures, VkPhysicalDeviceVulkan13Features
    textureCompressionASTC_HDR,
    // VkPhysicalDeviceTilePropertiesFeaturesQCOM
    tileProperties,
    // VkPhysicalDeviceTimelineSemaphoreFeatures, VkPhysicalDeviceVulkan12Features
    timelineSemaphore,
    // VkPhysicalDeviceTransformFeedbackFeaturesEXT
    geometryStreams,
    // VkPhysicalDeviceTransformFeedbackFeaturesEXT
    transformFeedback,
    // VkPhysicalDeviceUniformBufferStandardLayoutFeatures, VkPhysicalDeviceVulkan12Features
    uniformBufferStandardLayout,
    // VkPhysicalDeviceVariablePointersFeatures, VkPhysicalDeviceVulkan11Features
    variablePointers,
    // VkPhysicalDeviceVariablePointersFeatures, VkPhysicalDeviceVulkan11Features
    variablePointersStorageBuffer,
    // VkPhysicalDeviceVertexAttributeDivisorFeaturesKHR
    vertexAttributeInstanceRateDivisor,
    // VkPhysicalDeviceVertexAttributeDivisorFeaturesKHR
    vertexAttributeInstanceRateZeroDivisor,
    // VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT
    vertexInputDynamicState,
    // VkPhysicalDeviceVideoMaintenance1FeaturesKHR
    videoMaintenance1,
    // VkPhysicalDeviceVulkan12Features
    descriptorIndexing,
    // VkPhysicalDeviceVulkan12Features
    drawIndirectCount,
    // VkPhysicalDeviceVulkan12Features
    samplerFilterMinmax,
    // VkPhysicalDeviceVulkan12Features
    samplerMirrorClampToEdge,
    // VkPhysicalDeviceVulkan12Features
    shaderOutputLayer,
    // VkPhysicalDeviceVulkan12Features
    shaderOutputViewportIndex,
    // VkPhysicalDeviceVulkan12Features
    subgroupBroadcastDynamicId,
    // VkPhysicalDeviceVulkan12Features, VkPhysicalDeviceVulkanMemoryModelFeatures
    vulkanMemoryModel,
    // VkPhysicalDeviceVulkan12Features, VkPhysicalDeviceVulkanMemoryModelFeatures
    vulkanMemoryModelAvailabilityVisibilityChains,
    // VkPhysicalDeviceVulkan12Features, VkPhysicalDeviceVulkanMemoryModelFeatures
    vulkanMemoryModelDeviceScope,
    // VkPhysicalDeviceVulkan13Features, VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures
    shaderZeroInitializeWorkgroupMemory,
    // VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR
    workgroupMemoryExplicitLayout,
    // VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR
    workgroupMemoryExplicitLayout16BitAccess,
    // VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR
    workgroupMemoryExplicitLayout8BitAccess,
    // VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR
    workgroupMemoryExplicitLayoutScalarBlockLayout,
    // VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT
    ycbcr2plane444Formats,
    // VkPhysicalDeviceYcbcrDegammaFeaturesQCOM
    ycbcrDegamma,
    // VkPhysicalDeviceYcbcrImageArraysFeaturesEXT
    ycbcrImageArrays,
};

struct FeatureAndName {
    VkBool32 *feature;
    const char *name;
};

// Find or add the correct VkPhysicalDeviceFeature struct in `pnext_chain` based on `feature`,
// a vkt::Feature enum value, and set feature to VK_TRUE
FeatureAndName AddFeature(APIVersion api_version, vkt::Feature feature, void **inout_pnext_chain);

}  // namespace vkt

// NOLINTEND
