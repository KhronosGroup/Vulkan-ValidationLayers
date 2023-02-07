/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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

#ifndef BEST_PRACTICES_ERROR_ENUMS_H_
#define BEST_PRACTICES_ERROR_ENUMS_H_

// clang-format off

[[maybe_unused]] static const char *kVUID_BestPractices_CreateInstance_ExtensionMismatch =
    "UNASSIGNED-BestPractices-vkCreateInstance-extension-mismatch";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateDevice_ExtensionMismatch =
    "UNASSIGNED-BestPractices-vkCreateDevice-extension-mismatch";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateInstance_DeprecatedExtension =
    "UNASSIGNED-BestPractices-vkCreateInstance-deprecated-extension";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateDevice_DeprecatedExtension =
    "UNASSIGNED-BestPractices-vkCreateDevice-deprecated-extension";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateInstance_SpecialUseExtension_CADSupport =
    "UNASSIGNED-BestPractices-vkCreateInstance-specialuse-extension-cadsupport";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateInstance_SpecialUseExtension_D3DEmulation =
    "UNASSIGNED-BestPractices-vkCreateInstance-specialuse-extension-d3demulation";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateInstance_SpecialUseExtension_DevTools =
    "UNASSIGNED-BestPractices-vkCreateInstance-specialuse-extension-devtools";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateInstance_SpecialUseExtension_Debugging =
    "UNASSIGNED-BestPractices-vkCreateInstance-specialuse-extension-debugging";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateInstance_SpecialUseExtension_GLEmulation =
    "UNASSIGNED-BestPractices-vkCreateInstance-specialuse-extension-glemulation";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateDevice_SpecialUseExtension_CADSupport =
    "UNASSIGNED-BestPractices-vkCreateDevice-specialuse-extension-cadsupport";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateDevice_SpecialUseExtension_D3DEmulation =
    "UNASSIGNED-BestPractices-vkCreateDevice-specialuse-extension-d3demulation";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateDevice_SpecialUseExtension_DevTools =
    "UNASSIGNED-BestPractices-vkCreateDevice-specialuse-extension-devtools";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateDevice_SpecialUseExtension_Debugging =
    "UNASSIGNED-BestPractices-vkCreateDevice-specialuse-extension-debugging";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateDevice_SpecialUseExtension_GLEmulation =
    "UNASSIGNED-BestPractices-vkCreateDevice-specialuse-extension-glemulation";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateDevice_API_Mismatch =
    "UNASSIGNED-BestPractices-vkCreateDevice-API-version-mismatch";
[[maybe_unused]] static const char *kVUID_BestPractices_DevLimit_MustQueryCount = "UNASSIGNED-BestPractices-DevLimit-MustQueryCount";
[[maybe_unused]] static const char *kVUID_BestPractices_DevLimit_CountMismatch = "UNASSIGNED-BestPractices-DevLimit-CountMismatch";
[[maybe_unused]] static const char *kVUID_BestPractices_DevLimit_MissingQueryCount = "UNASSIGNED-BestPractices-DevLimit-MissingQueryCount";
[[maybe_unused]] static const char *kVUID_BestPractices_SharingModeExclusive =
    "UNASSIGNED-BestPractices-vkCreateBuffer-sharing-mode-exclusive";
[[maybe_unused]] static const char *kVUID_BestPractices_RenderPass_Attatchment =
    "UNASSIGNED-BestPractices-vkCreateRenderPass-attatchment";
[[maybe_unused]] static const char *kVUID_BestPractices_AllocateMemory_TooManyObjects =
    "UNASSIGNED-BestPractices-vkAllocateMemory-too-many-objects";
[[maybe_unused]] static const char *kVUID_BestPractices_CreatePipelines_MultiplePipelines =
    "UNASSIGNED-BestPractices-vkCreatePipelines-multiple-pipelines-no-cache";
[[maybe_unused]] static const char *kVUID_BestPractices_PipelineStageFlags = "UNASSIGNED-BestPractices-pipeline-stage-flags";
[[maybe_unused]] static const char *kVUID_BestPractices_CmdDraw_InstanceCountZero =
    "UNASSIGNED-BestPractices-vkCmdDraw-instance-count-zero";
[[maybe_unused]] static const char *kVUID_BestPractices_CmdDraw_DrawCountZero = "UNASSIGNED-BestPractices-vkCmdDraw-draw-count-zero";
[[maybe_unused]] static const char *kVUID_BestPractices_CmdDispatch_GroupCountZero =
    "UNASSIGNED-BestPractices-vkCmdDispatch-group-count-zero";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateDevice_PDFeaturesNotCalled =
    "UNASSIGNED-BestPractices-vkCreateDevice-physical-device-features-not-retrieved";
[[maybe_unused]] static const char *kVUID_BestPractices_Swapchain_GetSurfaceNotCalled =
    "UNASSIGNED-BestPractices-vkCreateSwapchainKHR-surface-not-retrieved";
[[maybe_unused]] static const char *kVUID_BestPractices_DisplayPlane_PropertiesNotCalled =
    "UNASSIGNED-BestPractices-vkGetDisplayPlaneSupportedDisplaysKHR-properties-not-retrieved";
[[maybe_unused]] static const char *kVUID_BestPractices_MemTrack_InvalidState = "UNASSIGNED-BestPractices-MemTrack-InvalidState";
[[maybe_unused]] static const char *kVUID_BestPractices_BufferMemReqNotCalled =
    "UNASSIGNED-BestPractices-vkBindBufferMemory-requirements-not-retrieved";
[[maybe_unused]] static const char *kVUID_BestPractices_ImageMemReqNotCalled =
    "UNASSIGNED-BestPractices-vkBindImageMemory-requirements-not-retrieved";
[[maybe_unused]] static const char *kVUID_BestPractices_BindAccelNV_NoMemReqQuery =
    "UNASSIGNED-BestPractices-BindAccelerationStructureMemoryNV-requirements-not-retrieved";
[[maybe_unused]] static const char *kVUID_BestPractices_GetVideoSessionMemReqCountNotRetrieved =
    "UNASSIGNED-BestPractices-vkGetVideoSessionMemoryRequirementsKHR-count-not-retrieved";
[[maybe_unused]] static const char *kVUID_BestPractices_BindVideoSessionMemReqCountNotRetrieved =
    "UNASSIGNED-BestPractices-vkBindVideoSessionMemoryKHR-requirements-count-not-retrieved";
[[maybe_unused]] static const char *kVUID_BestPractices_BindVideoSessionMemReqNotAllBindingsRetrieved =
    "UNASSIGNED-BestPractices-vkBindVideoSessionMemoryKHR-requirements-not-all-retrieved";
[[maybe_unused]] static const char *kVUID_BestPractices_DrawState_VtxIndexOutOfBounds =
    "UNASSIGNED-BestPractices-DrawState-VtxIndexOutOfBounds";
[[maybe_unused]] static const char *kVUID_BestPractices_DrawState_ClearCmdBeforeDraw =
    "UNASSIGNED-BestPractices-DrawState-ClearCmdBeforeDraw";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateCommandPool_CommandBufferReset =
    "UNASSIGNED-BestPractices-vkCreateCommandPool-command-buffer-reset";
[[maybe_unused]] static const char *kVUID_BestPractices_AllocateCommandBuffers_UnusableSecondary =
    "UNASSIGNED-BestPractices-vkAllocateCommandBuffers-unusable-secondary";
[[maybe_unused]] static const char *kVUID_BestPractices_BeginCommandBuffer_SimultaneousUse =
    "UNASSIGNED-BestPractices-vkBeginCommandBuffer-simultaneous-use";
[[maybe_unused]] static const char *kVUID_BestPractices_AllocateMemory_SmallAllocation =
    "UNASSIGNED-BestPractices-vkAllocateMemory-small-allocation";
[[maybe_unused]] static const char *kVUID_BestPractices_SmallDedicatedAllocation =
    "UNASSIGNED-BestPractices-vkBindMemory-small-dedicated-allocation";
[[maybe_unused]] static const char *kVUID_BestPractices_NonLazyTransientImage =
    "UNASSIGNED-BestPractices-vkBindImageMemory-non-lazy-transient-image";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateRenderPass_ImageRequiresMemory =
    "UNASSIGNED-BestPractices-vkCreateRenderPass-image-requires-memory";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateFramebuffer_AttachmentShouldBeTransient =
    "UNASSIGNED-BestPractices-vkCreateFramebuffer-attachment-should-be-transient";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateFramebuffer_AttachmentShouldNotBeTransient =
    "UNASSIGNED-BestPractices-vkCreateFramebuffer-attachment-should-not-be-transient";
[[maybe_unused]] static const char *kVUID_BestPractices_CreatePipelines_TooManyInstancedVertexBuffers =
    "UNASSIGNED-BestPractices-vkCreateGraphicsPipelines-too-many-instanced-vertex-buffers";
[[maybe_unused]] static const char *kVUID_BestPractices_ClearAttachments_ClearAfterLoad =
    "UNASSIGNED-BestPractices-vkCmdClearAttachments-clear-after-load";
[[maybe_unused]] static const char *kVUID_BestPractices_Error_Result = "UNASSIGNED-BestPractices-Error-Result";
[[maybe_unused]] static const char *kVUID_BestPractices_Failure_Result = "UNASSIGNED-BestPractices-Failure-Result";
[[maybe_unused]] static const char *kVUID_BestPractices_NonSuccess_Result = "UNASSIGNED-BestPractices-NonSuccess-Result";
[[maybe_unused]] static const char *kVUID_BestPractices_SuboptimalSwapchain = "UNASSIGNED-BestPractices-SuboptimalSwapchain";
[[maybe_unused]] static const char *kVUID_BestPractices_SuboptimalSwapchainImageCount =
    "UNASSIGNED-BestPractices-vkCreateSwapchainKHR-suboptimal-swapchain-image-count";
[[maybe_unused]] static const char *kVUID_BestPractices_Swapchain_InvalidCount = "UNASSIGNED-BestPractices-SwapchainInvalidCount";
[[maybe_unused]] static const char *kVUID_BestPractices_DepthBiasNoAttachment = "UNASSIGNED-BestPractices-DepthBiasNoAttachment";
[[maybe_unused]] static const char *kVUID_BestPractices_SpirvDeprecated_WorkgroupSize =
    "UNASSIGNED-BestPractices-SpirvDeprecated_WorkgroupSize";
[[maybe_unused]] static const char *kVUID_BestPractices_ImageCreateFlags = "UNASSIGNED-BestPractices-ImageCreateFlags";
[[maybe_unused]] static const char *kVUID_BestPractices_TransitionUndefinedToReadOnly =
    "UNASSIGNED-BestPractices-TransitionUndefinedToReadOnly";
[[maybe_unused]] static const char *kVUID_BestPractices_SemaphoreCount = "UNASSIGNED-BestPractices-SemaphoreCount";
[[maybe_unused]] static const char *kVUID_BestPractices_EmptyDescriptorPool =
    "UNASSIGNED-BestPractices-EmptyDescriptorPool";
[[maybe_unused]] static const char *kVUID_BestPractices_DescriptorTypeNotInPool = "UNASSIGNED-BestPractices-DescriptorTypeNotInPool";
[[maybe_unused]] static const char *kVUID_BestPractices_ClearValueWithoutLoadOpClear = "UNASSIGNED-BestPractices-vkCmdBeginRenderPass-ClearValueWithoutLoadOpClear";
[[maybe_unused]] static const char *kVUID_BestPractices_ClearValueCountHigherThanAttachmentCount = "UNASSIGNED-BestPractices-vkCmdBeginRenderPass-ClearValueCountHigherThanAttachmentCount";
[[maybe_unused]] static const char *kVUID_BestPractices_StoreOpDontCareThenLoadOpLoad = "UNASSIGNED-BestPractices-vkCmdBeginRenderPass-StoreOpDontCareThenLoadOpLoad";
[[maybe_unused]] static const char *kVUID_BestPractices_ConcurrentUsageOfExclusiveImage = "UNASSIGNED-BestPractices-ConcurrentUsageOfExclusiveImage";
[[maybe_unused]] static const char *kVUID_BestPractices_ImageBarrierAccessLayout =
    "UNASSIGNED-BestPractices-ImageBarrierAccessLayout";
[[maybe_unused]] static const char *kVUID_BestPractices_DrawState_SwapchainImagesNotFound = "UNASSIGNED-BestPractices-DrawState-SwapchainImagesNotFound";
[[maybe_unused]] static const char *kVUID_BestPractices_DrawState_MismatchedImageType = "UNASSIGNED-BestPractices-DrawState-MismatchedImageType";
[[maybe_unused]] static const char *kVUID_BestPractices_DrawState_InvalidExtents = "UNASSIGNED-BestPractices-DrawState-InvalidExtents";

// Arm-specific best practice
[[maybe_unused]] static const char *kVUID_BestPractices_AllocateDescriptorSets_SuboptimalReuse =
    "UNASSIGNED-BestPractices-vkAllocateDescriptorSets-suboptimal-reuse";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateComputePipelines_ComputeThreadGroupAlignment =
    "UNASSIGNED-BestPractices-vkCreateComputePipelines-compute-thread-group-alignment";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateComputePipelines_ComputeWorkGroupSize =
    "UNASSIGNED-BestPractices-vkCreateComputePipelines-compute-work-group-size";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateComputePipelines_ComputeSpatialLocality =
    "UNASSIGNED-BestPractices-vkCreateComputePipelines-compute-spatial-locality";
[[maybe_unused]] static const char *kVUID_BestPractices_CreatePipelines_MultisampledBlending =
    "UNASSIGNED-BestPractices-vkCreatePipelines-multisampled-blending";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateImage_TooLargeSampleCount =
    "UNASSIGNED-BestPractices-vkCreateImage-too-large-sample-count";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateImage_NonTransientMSImage =
    "UNASSIGNED-BestPractices-vkCreateImage-non-transient-ms-image";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateSampler_DifferentWrappingModes =
    "UNASSIGNED-BestPractices-vkCreateSampler-different-wrapping-modes";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateSampler_LodClamping =
    "UNASSIGNED-BestPractices-vkCreateSampler-lod-clamping";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateSampler_LodBias = "UNASSIGNED-BestPractices-vkCreateSampler-lod-bias";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateSampler_BorderClampColor =
    "UNASSIGNED-BestPractices-vkCreateSampler-border-clamp-color";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateSampler_UnnormalizedCoordinates =
    "UNASSIGNED-BestPractices-vkCreateSampler-unnormalized-coordinates";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateSampler_Anisotropy =
    "UNASSIGNED-BestPractices-vkCreateSampler-anisotropy";
[[maybe_unused]] static const char *kVUID_BestPractices_CmdResolveImage_ResolvingImage =
    "UNASSIGNED-BestPractices-vkCmdResolveImage-resolving-image";
[[maybe_unused]] static const char *kVUID_BestPractices_CmdDrawIndexed_ManySmallIndexedDrawcalls =
    "UNASSIGNED-BestPractices-vkCmdDrawIndexed-many-small-indexed-drawcalls";
[[maybe_unused]] static const char *kVUID_BestPractices_CmdDrawIndexed_SparseIndexBuffer =
    "UNASSIGNED-BestPractices-vkCmdDrawIndexed-sparse-index-buffer";
[[maybe_unused]] static const char *kVUID_BestPractices_CmdDrawIndexed_PostTransformCacheThrashing =
    "UNASSIGNED-BestPractices-vkCmdDrawIndexed-post-transform-cache-thrashing";
[[maybe_unused]] static const char *kVUID_BestPractices_BeginCommandBuffer_OneTimeSubmit =
    "UNASSIGNED-BestPractices-vkBeginCommandBuffer-one-time-submit";
[[maybe_unused]] static const char *kVUID_BestPractices_BeginRenderPass_ZeroSizeRenderArea =
    "UNASSIGNED-BestPractices-vkCmdBeginRenderPass-zero-size-render-area";
[[maybe_unused]] static const char *kVUID_BestPractices_BeginRenderPass_AttachmentNeedsReadback =
    "UNASSIGNED-BestPractices-vkCmdBeginRenderPass-attachment-needs-readback";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateSwapchain_PresentMode =
    "UNASSIGNED-BestPractices-vkCreateSwapchainKHR-swapchain-presentmode-not-fifo";
[[maybe_unused]] static const char *kVUID_BestPractices_CreatePipelines_DepthBias_Zero =
    "UNASSIGNED-BestPractices-vkCreatePipelines-depthbias-zero";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateDevice_RobustBufferAccess =
    "UNASSIGNED-BestPractices-vkCreateDevice-RobustBufferAccess";
[[maybe_unused]] static const char *kVUID_BestPractices_EndRenderPass_DepthPrePassUsage =
    "UNASSIGNED-BestPractices-vkCmdEndRenderPass-depth-pre-pass-usage";
[[maybe_unused]] static const char *kVUID_BestPractices_EndRenderPass_RedundantAttachmentOnTile =
    "UNASSIGNED-BestPractices-vkCmdEndRenderPass-redundant-attachment-on-tile";
[[maybe_unused]] static const char *kVUID_BestPractices_RenderPass_RedundantStore =
    "UNASSIGNED-BestPractices-RenderPass-redundant-store";
[[maybe_unused]] static const char *kVUID_BestPractices_RenderPass_RedundantClear =
    "UNASSIGNED-BestPractices-RenderPass-redundant-clear";
[[maybe_unused]] static const char *kVUID_BestPractices_RenderPass_InefficientClear =
    "UNASSIGNED-BestPractices-RenderPass-inefficient-clear";
[[maybe_unused]] static const char *kVUID_BestPractices_RenderPass_BlitImage_LoadOpLoad =
    "UNASSIGNED-BestPractices-RenderPass-blitimage-loadopload";
[[maybe_unused]] static const char *kVUID_BestPractices_RenderPass_CopyImage_LoadOpLoad =
    "UNASSIGNED-BestPractices-RenderPass-copyimage-loadopload";
[[maybe_unused]] static const char *kVUID_BestPractices_RenderPass_ResolveImage_LoadOpLoad =
    "UNASSIGNED-BestPractices-RenderPass-resolveimage-loadopload";


// AMD-specific best practice
[[maybe_unused]] static const char *kVUID_BestPractices_CmdBuffer_AvoidTinyCmdBuffers =
    "UNASSIGNED-BestPractices-VkCommandBuffer-AvoidTinyCmdBuffers";
[[maybe_unused]] static const char *kVUID_BestPractices_CmdBuffer_AvoidSecondaryCmdBuffers =
    "UNASSIGNED-BestPractices-VkCommandBuffer-AvoidSecondaryCmdBuffers";
[[maybe_unused]] static const char *kVUID_BestPractices_CmdBuffer_AvoidSmallSecondaryCmdBuffers =
    "UNASSIGNED-BestPractices-VkCommandBuffer-AvoidSmallSecondaryCmdBuffers";
[[maybe_unused]] static const char *kVUID_BestPractices_CmdBuffer_AvoidClearSecondaryCmdBuffers =
    "UNASSIGNED-BestPractices-VkCommandBuffer-AvoidClearSecondaryCmdBuffers";
[[maybe_unused]] static const char *kVUID_BestPractices_DrawState_AvoidVertexBindEveryDraw =
    "UNASSIGNED-BestPractices-DrawState-AvoidVertexBindEveryDraw";
[[maybe_unused]] static const char *kVUID_BestPractices_CmdPool_DisparateSizedCmdBuffers =
    "UNASSIGNED-BestPractices-CmdPool-DisparateSizedCmdBuffers";
[[maybe_unused]] static const char *kVUID_BestPractices_CreatePipelines_TooManyPipelines =
    "UNASSIGNED-BestPractices-CreatePipelines-TooManyPipelines";
[[maybe_unused]] static const char *kVUID_BestPractices_CreatePipelines_MultiplePipelineCaches =
    "UNASSIGNED-BestPractices-vkCreatePipelines-multiple-pipelines-caches";
[[maybe_unused]] static const char *kVUID_BestPractices_vkImage_DontUseMutableRenderTargets =
    "UNASSIGNED-BestPractices-vkImage-DontUseMutableRenderTargets";
[[maybe_unused]] static const char *kVUID_BestPractices_vkImage_AvoidImageToImageCopy =
    "UNASSIGNED-BestPractices-vkImage-AvoidImageToImageCopy";
[[maybe_unused]] static const char *kVUID_BestPractices_vkImage_AvoidConcurrentRenderTargets =
    "UNASSIGNED-BestPractices-vkImage-AvoidConcurrentRenderTargets";
[[maybe_unused]] static const char *kVUID_BestPractices_vkImage_DontUseStorageRenderTargets =
    "UNASSIGNED-BestPractices-vkImage-DontUseStorageRenderTargets";
[[maybe_unused]] static const char *kVUID_BestPractices_vkImage_AvoidGeneral = "UNASSIGNED-BestPractices-vkImage-AvoidGeneral";
[[maybe_unused]] static const char *kVUID_BestPractices_CreatePipelines_AvoidPrimitiveRestart =
    "UNASSIGNED-BestPractices-CreatePipelines-AvoidPrimitiveRestart";
[[maybe_unused]] static const char *kVUID_BestPractices_CreatePipelines_MinimizeNumDynamicStates =
    "UNASSIGNED-BestPractices-CreatePipelines-MinimizeNumDynamicStates";
[[maybe_unused]] static const char *kVUID_BestPractices_CreatePipelinesLayout_KeepLayoutSmall =
    "UNASSIGNED-BestPractices-CreatePipelinesLayout-KeepLayoutSmall";
[[maybe_unused]] static const char *kVUID_BestPractices_UpdateDescriptors_AvoidCopyingDescriptors =
    "UNASSIGNED-BestPractices-UpdateDescriptors-AvoidCopyingDescriptors";
[[maybe_unused]] static const char *kVUID_BestPractices_UpdateDescriptors_PreferNonTemplate =
    "UNASSIGNED-BestPractices-UpdateDescriptors-PreferNonTemplate";
[[maybe_unused]] static const char *kVUID_BestPractices_ClearAttachment_FastClearValues =
    "UNASSIGNED-BestPractices-ClearAttachment-FastClearValues";
[[maybe_unused]] static const char *kVUID_BestPractices_ClearAttachment_ClearImage =
    "UNASSIGNED-BestPractices-ClearAttachment-ClearImage";
[[maybe_unused]] static const char *kVUID_BestPractices_CmdBuffer_backToBackBarrier =
    "UNASSIGNED-BestPractices-CmdBuffer-backToBackBarrier";
[[maybe_unused]] static const char *kVUID_BestPractices_CmdBuffer_highBarrierCount =
    "UNASSIGNED-BestPractices-CmdBuffer-highBarrierCount";
[[maybe_unused]] static const char *kVUID_BestPractices_PipelineBarrier_readToReadBarrier =
    "UNASSIGNED-BestPractices-PipelineBarrier-readToReadBarrier";
[[maybe_unused]] static const char *kVUID_BestPractices_Submission_ReduceNumberOfSubmissions =
    "UNASSIGNED-BestPractices-Submission-ReduceNumberOfSubmissions";
[[maybe_unused]] static const char *kVUID_BestPractices_Pipeline_SortAndBind = "UNASSIGNED-BestPractices-Pipeline-SortAndBind";
[[maybe_unused]] static const char *kVUID_BestPractices_Pipeline_WorkPerPipelineChange =
    "UNASSIGNED-BestPractices-Pipeline-WorkPerPipelineChange";
[[maybe_unused]] static const char *kVUID_BestPractices_SyncObjects_HighNumberOfFences =
    "UNASSIGNED-BestPractices-SyncObjects-HighNumberOfFences";
[[maybe_unused]] static const char *kVUID_BestPractices_SyncObjects_HighNumberOfSemaphores =
    "UNASSIGNED-BestPractices-SyncObjects-HighNumberOfSemaphores";
[[maybe_unused]] static const char *kVUID_BestPractices_DynamicRendering_NotSupported =
    "UNASSIGNED-BestPractices-DynamicRendering-NotSupported";

// Imagination Technologies best practices
[[maybe_unused]] static const char *kVUID_BestPractices_Texture_Format_PVRTC_Outdated =
    "UNASSIGNED-BestPractices-Texture-Format-PVRTC-Outdated";

// NVIDIA-specific best practices
[[maybe_unused]] static const char *kVUID_BestPractices_CreateDevice_PageableDeviceLocalMemory =
    "UNASSIGNED-BestPractices-CreateDevice-PageableDeviceLocalMemory";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateImage_TilingLinear =
    "UNASSIGNED-BestPractices-CreateImage-TilingLinear";
[[maybe_unused]] static const char *kVUID_BestPractices_CreateImage_Depth32Format =
    "UNASSIGNED-BestPractices-CreateImage-Depth32Format";
[[maybe_unused]] static const char *kVUID_BestPractices_QueueBindSparse_NotAsync =
    "UNASSIGNED-BestPractices-QueueBindSparse-NotAsync";
[[maybe_unused]] static const char *kVUID_BestPractices_AccelerationStructure_NotAsync =
    "UNASSIGNED-BestPractices-AccelerationStructure-NotAsync";
[[maybe_unused]] static const char *kVUID_BestPractices_AllocateMemory_SetPriority =
    "UNASSIGNED-BestPractices-AllocateMemory-SetPriority";
[[maybe_unused]] static const char *kVUID_BestPractices_AllocateMemory_ReuseAllocations =
    "UNASSIGNED-BestPractices-AllocateMemory-ReuseAllocations";
[[maybe_unused]] static const char *kVUID_BestPractices_BindMemory_NoPriority =
    "UNASSIGNED-BestPractices-BindMemory-NoPriority";
[[maybe_unused]] static const char *kVUID_BestPractices_CreatePipelineLayout_SeparateSampler =
    "UNASSIGNED-BestPractices-CreatePipelineLayout-SeparateSampler";
[[maybe_unused]] static const char *kVUID_BestPractices_CreatePipelinesLayout_LargePipelineLayout =
    "UNASSIGNED-BestPractices-CreatePipelineLayout-LargePipelineLayout";
[[maybe_unused]] static const char *kVUID_BestPractices_BindPipeline_SwitchTessGeometryMesh =
    "UNASSIGNED-BestPractices-BindPipeline-SwitchTessGeometryMesh";
[[maybe_unused]] static const char *kVUID_BestPractices_Zcull_LessGreaterRatio =
    "UNASSIGNED-BestPractices-Zcull-LessGreaterRatio";
[[maybe_unused]] static const char *kVUID_BestPractices_ClearColor_NotCompressed =
    "UNASSIGNED-BestPractices-ClearColor-NotCompressed";

// clang-format on

#endif
