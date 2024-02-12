/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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

// clang-format off

[[maybe_unused]] static const char *kVUID_BestPractices_CreateInstance_ExtensionMismatch =
    "BestPractices-vkCreateInstance-extension-mismatch";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateDevice_ExtensionMismatch =
    "BestPractices-vkCreateDevice-extension-mismatch";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateDevice_API_Mismatch =
    "BestPractices-vkCreateDevice-API-version-mismatch";
[[maybe_unused]] static const char *kVUID_BestPractices_DevLimit_MustQueryCount = "BestPractices-DevLimit-MustQueryCount";
[[maybe_unused]] static const char *kVUID_BestPractices_DevLimit_CountMismatch = "BestPractices-DevLimit-CountMismatch";
[[maybe_unused]] static const char *kVUID_BestPractices_DevLimit_MissingQueryCount = "BestPractices-DevLimit-MissingQueryCount";
[[maybe_unused]] static const char *kVUID_BestPractices_SharingModeExclusive =
    "BestPractices-vkCreateBuffer-sharing-mode-exclusive";
[[maybe_unused]] static const char *kVUID_BestPractices_RenderPass_Attatchment =
    "BestPractices-vkCreateRenderPass-attatchment";
[[maybe_unused]] static const char *kVUID_BestPractices_AllocateMemory_TooManyObjects =
    "BestPractices-vkAllocateMemory-too-many-objects";
[[maybe_unused]] static const char *kVUID_BestPractices_CreatePipelines_MultiplePipelines =
    "BestPractices-vkCreatePipelines-multiple-pipelines-no-cache";
[[maybe_unused]] static const char *kVUID_BestPractices_PipelineStageFlags = "BestPractices-pipeline-stage-flags";
[[maybe_unused]] static const char *kVUID_BestPractices_CmdDraw_InstanceCountZero =
    "BestPractices-vkCmdDraw-instance-count-zero";
[[maybe_unused]] static const char *kVUID_BestPractices_CmdDraw_DrawCountZero = "BestPractices-vkCmdDraw-draw-count-zero";
[[maybe_unused]] static const char *kVUID_BestPractices_CmdDispatch_GroupCountZero =
    "BestPractices-vkCmdDispatch-group-count-zero";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateDevice_PDFeaturesNotCalled =
    "BestPractices-vkCreateDevice-physical-device-features-not-retrieved";
[[maybe_unused]] static const char *kVUID_BestPractices_Swapchain_GetSurfaceNotCalled =
    "BestPractices-vkCreateSwapchainKHR-surface-not-retrieved";
[[maybe_unused]] static const char *kVUID_BestPractices_DisplayPlane_PropertiesNotCalled =
    "BestPractices-vkGetDisplayPlaneSupportedDisplaysKHR-properties-not-retrieved";
[[maybe_unused]] static const char *kVUID_BestPractices_MemTrack_InvalidState = "BestPractices-MemTrack-InvalidState";
[[maybe_unused]] static const char *kVUID_BestPractices_BindAccelNV_NoMemReqQuery =
    "BestPractices-BindAccelerationStructureMemoryNV-requirements-not-retrieved";
[[maybe_unused]] static const char *kVUID_BestPractices_GetVideoSessionMemReqCountNotRetrieved =
    "BestPractices-vkGetVideoSessionMemoryRequirementsKHR-count-not-retrieved";
[[maybe_unused]] static const char *kVUID_BestPractices_BindVideoSessionMemReqCountNotRetrieved =
    "BestPractices-vkBindVideoSessionMemoryKHR-requirements-count-not-retrieved";
[[maybe_unused]] static const char *kVUID_BestPractices_BindVideoSessionMemReqNotAllBindingsRetrieved =
    "BestPractices-vkBindVideoSessionMemoryKHR-requirements-not-all-retrieved";
[[maybe_unused]] static const char *kVUID_BestPractices_DrawState_VtxIndexOutOfBounds =
    "BestPractices-DrawState-VtxIndexOutOfBounds";
[[maybe_unused]] static const char *kVUID_BestPractices_DrawState_ClearCmdBeforeDraw =
    "BestPractices-DrawState-ClearCmdBeforeDraw";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateCommandPool_CommandBufferReset =
    "BestPractices-vkCreateCommandPool-command-buffer-reset";
[[maybe_unused]] static const char *kVUID_BestPractices_AllocateCommandBuffers_UnusableSecondary =
    "BestPractices-vkAllocateCommandBuffers-unusable-secondary";
[[maybe_unused]] static const char *kVUID_BestPractices_BeginCommandBuffer_SimultaneousUse =
    "BestPractices-vkBeginCommandBuffer-simultaneous-use";
[[maybe_unused]] static const char *kVUID_BestPractices_AllocateMemory_SmallAllocation =
    "BestPractices-vkAllocateMemory-small-allocation";
[[maybe_unused]] static const char *kVUID_BestPractices_SmallDedicatedAllocation =
    "BestPractices-vkBindMemory-small-dedicated-allocation";
[[maybe_unused]] static const char *kVUID_BestPractices_NonLazyTransientImage =
    "BestPractices-vkBindImageMemory-non-lazy-transient-image";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateRenderPass_ImageRequiresMemory =
    "BestPractices-vkCreateRenderPass-image-requires-memory";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateFramebuffer_AttachmentShouldBeTransient =
    "BestPractices-vkCreateFramebuffer-attachment-should-be-transient";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateFramebuffer_AttachmentShouldNotBeTransient =
    "BestPractices-vkCreateFramebuffer-attachment-should-not-be-transient";
[[maybe_unused]] static const char *kVUID_BestPractices_CreatePipelines_TooManyInstancedVertexBuffers =
    "BestPractices-vkCreateGraphicsPipelines-too-many-instanced-vertex-buffers";
[[maybe_unused]] static const char *kVUID_BestPractices_ClearAttachments_ClearAfterLoad =
    "BestPractices-vkCmdClearAttachments-clear-after-load";
[[maybe_unused]] static const char *kVUID_BestPractices_Error_Result = "BestPractices-Error-Result";
[[maybe_unused]] static const char *kVUID_BestPractices_Failure_Result = "BestPractices-Failure-Result";
[[maybe_unused]] static const char *kVUID_BestPractices_Verbose_Success_Logging = "BestPractices-Verbose-Success-Logging";
[[maybe_unused]] static const char *kVUID_BestPractices_SuboptimalSwapchain = "BestPractices-SuboptimalSwapchain";
[[maybe_unused]] static const char *kVUID_BestPractices_SuboptimalSwapchainImageCount =
    "BestPractices-vkCreateSwapchainKHR-suboptimal-swapchain-image-count";
[[maybe_unused]] static const char *kVUID_BestPractices_NoVkSwapchainPresentModesCreateInfoEXTProvided =
    "BestPractices-vkCreateSwapchainKHR-no-VkSwapchainPresentModesCreateInfoEXT-provided";
[[maybe_unused]] static const char *kVUID_BestPractices_SubpassResolve_NonOptimalFormat =
    "BestPractices-vkCreateRenderPass-SubpassResolve-NonOptimalFormat";

[[maybe_unused]] static const char *kVUID_BestPractices_Swapchain_InvalidCount = "BestPractices-SwapchainInvalidCount";
[[maybe_unused]] static const char *kVUID_BestPractices_Swapchain_PriorCount = "BestPractices-SwapchainPriorCount";
[[maybe_unused]] static const char *kVUID_BestPractices_DepthBiasNoAttachment = "BestPractices-DepthBiasNoAttachment";
[[maybe_unused]] static const char *kVUID_BestPractices_SpirvDeprecated_WorkgroupSize =
    "BestPractices-SpirvDeprecated_WorkgroupSize";
[[maybe_unused]] static const char *kVUID_BestPractices_ImageCreateFlags = "BestPractices-ImageCreateFlags";
[[maybe_unused]] static const char *kVUID_BestPractices_TransitionUndefinedToReadOnly =
    "BestPractices-TransitionUndefinedToReadOnly";
[[maybe_unused]] static const char *kVUID_BestPractices_SemaphoreCount = "BestPractices-SemaphoreCount";
[[maybe_unused]] static const char *kVUID_BestPractices_PushConstants = "BestPractices-PushConstants";
[[maybe_unused]] static const char *kVUID_BestPractices_EmptyDescriptorPool =
    "BestPractices-EmptyDescriptorPool";
[[maybe_unused]] static const char *kVUID_BestPractices_ClearValueWithoutLoadOpClear = "BestPractices-vkCmdBeginRenderPass-ClearValueWithoutLoadOpClear";
[[maybe_unused]] static const char *kVUID_BestPractices_ClearValueCountHigherThanAttachmentCount = "BestPractices-vkCmdBeginRenderPass-ClearValueCountHigherThanAttachmentCount";
[[maybe_unused]] static const char *kVUID_BestPractices_StoreOpDontCareThenLoadOpLoad = "BestPractices-vkCmdBeginRenderPass-StoreOpDontCareThenLoadOpLoad";
[[maybe_unused]] static const char *kVUID_BestPractices_ConcurrentUsageOfExclusiveImage = "BestPractices-ConcurrentUsageOfExclusiveImage";
[[maybe_unused]] static const char *kVUID_BestPractices_ImageBarrierAccessLayout =
    "BestPractices-ImageBarrierAccessLayout";
[[maybe_unused]] static const char *kVUID_BestPractices_DrawState_SwapchainImagesNotFound = "BestPractices-DrawState-SwapchainImagesNotFound";
[[maybe_unused]] static const char *kVUID_BestPractices_DrawState_MismatchedImageType = "BestPractices-DrawState-MismatchedImageType";
[[maybe_unused]] static const char *kVUID_BestPractices_DrawState_InvalidExtents = "BestPractices-DrawState-InvalidExtents";
[[maybe_unused]] static const char *kVUID_BestPractices_DrawState_InvalidCommandBufferSimultaneousUse = "BestPractices-DrawState-InvalidCommandBufferSimultaneousUse";
[[maybe_unused]] static const char *kVUID_BestPractices_Pipeline_NoRendering = "BestPractices-Pipeline-NoRendering";
[[maybe_unused]] static const char *kVUID_BestPractices_QueryPool_Unavailable = "BestPractices-QueryPool-Unavailable";
[[maybe_unused]] static const char *kVUID_BestPractices_Shader_MissingInputAttachment =
    "BestPractices-Shader-MissingInputAttachment";
[[maybe_unused]] static const char *kVUID_BestPractices_RenderingInfo_ResolveModeNone = "BestPractices-VkRenderingInfo-ResolveModeNone";

// Arm-specific best practice
[[maybe_unused]] static const char *kVUID_BestPractices_AllocateDescriptorSets_SuboptimalReuse =
    "BestPractices-vkAllocateDescriptorSets-suboptimal-reuse";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateComputePipelines_ComputeThreadGroupAlignment =
    "BestPractices-vkCreateComputePipelines-compute-thread-group-alignment";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateComputePipelines_ComputeWorkGroupSize =
    "BestPractices-vkCreateComputePipelines-compute-work-group-size";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateComputePipelines_ComputeSpatialLocality =
    "BestPractices-vkCreateComputePipelines-compute-spatial-locality";
[[maybe_unused]] static const char *kVUID_BestPractices_CreatePipelines_MultisampledBlending =
    "BestPractices-vkCreatePipelines-multisampled-blending";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateImage_TooLargeSampleCount =
    "BestPractices-vkCreateImage-too-large-sample-count";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateImage_NonTransientMSImage =
    "BestPractices-vkCreateImage-non-transient-ms-image";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateSampler_DifferentWrappingModes =
    "BestPractices-vkCreateSampler-different-wrapping-modes";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateSampler_LodClamping =
    "BestPractices-vkCreateSampler-lod-clamping";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateSampler_LodBias = "BestPractices-vkCreateSampler-lod-bias";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateSampler_BorderClampColor =
    "BestPractices-vkCreateSampler-border-clamp-color";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateSampler_UnnormalizedCoordinates =
    "BestPractices-vkCreateSampler-unnormalized-coordinates";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateSampler_Anisotropy =
    "BestPractices-vkCreateSampler-anisotropy";
[[maybe_unused]] static const char *kVUID_BestPractices_CmdResolveImage_ResolvingImage =
    "BestPractices-vkCmdResolveImage-resolving-image";
[[maybe_unused]] static const char *kVUID_BestPractices_CmdDrawIndexed_ManySmallIndexedDrawcalls =
    "BestPractices-vkCmdDrawIndexed-many-small-indexed-drawcalls";
[[maybe_unused]] static const char *kVUID_BestPractices_CmdDrawIndexed_SparseIndexBuffer =
    "BestPractices-vkCmdDrawIndexed-sparse-index-buffer";
[[maybe_unused]] static const char *kVUID_BestPractices_CmdDrawIndexed_PostTransformCacheThrashing =
    "BestPractices-vkCmdDrawIndexed-post-transform-cache-thrashing";
[[maybe_unused]] static const char *kVUID_BestPractices_BeginCommandBuffer_OneTimeSubmit =
    "BestPractices-vkBeginCommandBuffer-one-time-submit";
[[maybe_unused]] static const char *kVUID_BestPractices_BeginRenderPass_ZeroSizeRenderArea =
    "BestPractices-vkCmdBeginRenderPass-zero-size-render-area";
[[maybe_unused]] static const char *kVUID_BestPractices_BeginRenderPass_AttachmentNeedsReadback =
    "BestPractices-vkCmdBeginRenderPass-attachment-needs-readback";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateSwapchain_PresentMode =
    "BestPractices-vkCreateSwapchainKHR-swapchain-presentmode-not-fifo";
[[maybe_unused]] static const char *kVUID_BestPractices_CreatePipelines_DepthBias_Zero =
    "BestPractices-vkCreatePipelines-depthbias-zero";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateDevice_RobustBufferAccess =
    "BestPractices-vkCreateDevice-RobustBufferAccess";
[[maybe_unused]] static const char *kVUID_BestPractices_EndRenderPass_DepthPrePassUsage =
    "BestPractices-vkCmdEndRenderPass-depth-pre-pass-usage";
[[maybe_unused]] static const char *kVUID_BestPractices_EndRenderPass_RedundantAttachmentOnTile =
    "BestPractices-vkCmdEndRenderPass-redundant-attachment-on-tile";
[[maybe_unused]] static const char *kVUID_BestPractices_RenderPass_RedundantStore =
    "BestPractices-RenderPass-redundant-store";
[[maybe_unused]] static const char *kVUID_BestPractices_RenderPass_RedundantClear =
    "BestPractices-RenderPass-redundant-clear";
[[maybe_unused]] static const char *kVUID_BestPractices_RenderPass_InefficientClear =
    "BestPractices-RenderPass-inefficient-clear";
[[maybe_unused]] static const char *kVUID_BestPractices_RenderPass_BlitImage_LoadOpLoad =
    "BestPractices-RenderPass-blitimage-loadopload";
[[maybe_unused]] static const char *kVUID_BestPractices_RenderPass_CopyImage_LoadOpLoad =
    "BestPractices-RenderPass-copyimage-loadopload";
[[maybe_unused]] static const char *kVUID_BestPractices_RenderPass_ResolveImage_LoadOpLoad =
    "BestPractices-RenderPass-resolveimage-loadopload";


// AMD-specific best practice
[[maybe_unused]] static const char *kVUID_BestPractices_CmdBuffer_AvoidTinyCmdBuffers =
    "BestPractices-VkCommandBuffer-AvoidTinyCmdBuffers";
[[maybe_unused]] static const char *kVUID_BestPractices_CmdBuffer_AvoidSecondaryCmdBuffers =
    "BestPractices-VkCommandBuffer-AvoidSecondaryCmdBuffers";
[[maybe_unused]] static const char *kVUID_BestPractices_CmdBuffer_AvoidSmallSecondaryCmdBuffers =
    "BestPractices-VkCommandBuffer-AvoidSmallSecondaryCmdBuffers";
[[maybe_unused]] static const char *kVUID_BestPractices_CmdBuffer_AvoidClearSecondaryCmdBuffers =
    "BestPractices-VkCommandBuffer-AvoidClearSecondaryCmdBuffers";
[[maybe_unused]] static const char *kVUID_BestPractices_DrawState_AvoidVertexBindEveryDraw =
    "BestPractices-DrawState-AvoidVertexBindEveryDraw";
[[maybe_unused]] static const char *kVUID_BestPractices_CmdPool_DisparateSizedCmdBuffers =
    "BestPractices-CmdPool-DisparateSizedCmdBuffers";
[[maybe_unused]] static const char *kVUID_BestPractices_CreatePipelines_TooManyPipelines =
    "BestPractices-CreatePipelines-TooManyPipelines";
[[maybe_unused]] static const char *kVUID_BestPractices_CreatePipelines_MultiplePipelineCaches =
    "BestPractices-vkCreatePipelines-multiple-pipelines-caches";
[[maybe_unused]] static const char *kVUID_BestPractices_vkImage_DontUseMutableRenderTargets =
    "BestPractices-vkImage-DontUseMutableRenderTargets";
[[maybe_unused]] static const char *kVUID_BestPractices_vkImage_AvoidImageToImageCopy =
    "BestPractices-vkImage-AvoidImageToImageCopy";
[[maybe_unused]] static const char *kVUID_BestPractices_vkImage_AvoidConcurrentRenderTargets =
    "BestPractices-vkImage-AvoidConcurrentRenderTargets";
[[maybe_unused]] static const char *kVUID_BestPractices_vkImage_DontUseStorageRenderTargets =
    "BestPractices-vkImage-DontUseStorageRenderTargets";
[[maybe_unused]] static const char *kVUID_BestPractices_vkImage_AvoidGeneral = "BestPractices-vkImage-AvoidGeneral";
[[maybe_unused]] static const char *kVUID_BestPractices_CreatePipelines_AvoidPrimitiveRestart =
    "BestPractices-CreatePipelines-AvoidPrimitiveRestart";
[[maybe_unused]] static const char *kVUID_BestPractices_CreatePipelines_MinimizeNumDynamicStates =
    "BestPractices-CreatePipelines-MinimizeNumDynamicStates";
[[maybe_unused]] static const char *kVUID_BestPractices_CreatePipelinesLayout_KeepLayoutSmall =
    "BestPractices-CreatePipelinesLayout-KeepLayoutSmall";
[[maybe_unused]] static const char *kVUID_BestPractices_UpdateDescriptors_AvoidCopyingDescriptors =
    "BestPractices-UpdateDescriptors-AvoidCopyingDescriptors";
[[maybe_unused]] static const char *kVUID_BestPractices_UpdateDescriptors_PreferNonTemplate =
    "BestPractices-UpdateDescriptors-PreferNonTemplate";
[[maybe_unused]] static const char *kVUID_BestPractices_ClearAttachment_FastClearValues =
    "BestPractices-ClearAttachment-FastClearValues";
[[maybe_unused]] static const char *kVUID_BestPractices_ClearAttachment_ClearImage =
    "BestPractices-ClearAttachment-ClearImage";
[[maybe_unused]] static const char *kVUID_BestPractices_CmdBuffer_backToBackBarrier =
    "BestPractices-CmdBuffer-backToBackBarrier";
[[maybe_unused]] static const char *kVUID_BestPractices_CmdBuffer_highBarrierCount =
    "BestPractices-CmdBuffer-highBarrierCount";
[[maybe_unused]] static const char *kVUID_BestPractices_PipelineBarrier_readToReadBarrier =
    "BestPractices-PipelineBarrier-readToReadBarrier";
[[maybe_unused]] static const char *kVUID_BestPractices_Submission_ReduceNumberOfSubmissions =
    "BestPractices-Submission-ReduceNumberOfSubmissions";
[[maybe_unused]] static const char *kVUID_BestPractices_Pipeline_SortAndBind = "BestPractices-Pipeline-SortAndBind";
[[maybe_unused]] static const char *kVUID_BestPractices_Pipeline_WorkPerPipelineChange =
    "BestPractices-Pipeline-WorkPerPipelineChange";
[[maybe_unused]] static const char *kVUID_BestPractices_SyncObjects_HighNumberOfFences =
    "BestPractices-SyncObjects-HighNumberOfFences";
[[maybe_unused]] static const char *kVUID_BestPractices_SyncObjects_HighNumberOfSemaphores =
    "BestPractices-SyncObjects-HighNumberOfSemaphores";
[[maybe_unused]] static const char *kVUID_BestPractices_DynamicRendering_NotSupported =
    "BestPractices-DynamicRendering-NotSupported";
[[maybe_unused]] static const char *kVUID_BestPractices_LocalWorkgroup_Multiple64 =
    "BestPractices-LocalWorkgroup-Multiple64";

// Imagination Technologies best practices
[[maybe_unused]] static const char *kVUID_BestPractices_Texture_Format_PVRTC_Outdated =
    "BestPractices-Texture-Format-PVRTC-Outdated";

// NVIDIA-specific best practices
[[maybe_unused]] static const char *kVUID_BestPractices_CreateDevice_PageableDeviceLocalMemory =
    "BestPractices-CreateDevice-PageableDeviceLocalMemory";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateImage_TilingLinear =
    "BestPractices-CreateImage-TilingLinear";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateImage_Depth32Format =
    "BestPractices-CreateImage-Depth32Format";
[[maybe_unused]] static const char *kVUID_BestPractices_QueueBindSparse_NotAsync =
    "BestPractices-QueueBindSparse-NotAsync";
[[maybe_unused]] static const char *kVUID_BestPractices_AccelerationStructure_NotAsync =
    "BestPractices-AccelerationStructure-NotAsync";
[[maybe_unused]] static const char *kVUID_BestPractices_AllocateMemory_SetPriority =
    "BestPractices-AllocateMemory-SetPriority";
[[maybe_unused]] static const char *kVUID_BestPractices_AllocateMemory_ReuseAllocations =
    "BestPractices-AllocateMemory-ReuseAllocations";
[[maybe_unused]] static const char *kVUID_BestPractices_BindMemory_NoPriority =
    "BestPractices-BindMemory-NoPriority";
[[maybe_unused]] static const char *kVUID_BestPractices_CreatePipelineLayout_SeparateSampler =
    "BestPractices-CreatePipelineLayout-SeparateSampler";
[[maybe_unused]] static const char *kVUID_BestPractices_CreatePipelinesLayout_LargePipelineLayout =
    "BestPractices-CreatePipelineLayout-LargePipelineLayout";
[[maybe_unused]] static const char *kVUID_BestPractices_BindPipeline_SwitchTessGeometryMesh =
    "BestPractices-BindPipeline-SwitchTessGeometryMesh";
[[maybe_unused]] static const char *kVUID_BestPractices_Zcull_LessGreaterRatio =
    "BestPractices-Zcull-LessGreaterRatio";
[[maybe_unused]] static const char *kVUID_BestPractices_ClearColor_NotCompressed =
    "BestPractices-ClearColor-NotCompressed";

// clang-format on
