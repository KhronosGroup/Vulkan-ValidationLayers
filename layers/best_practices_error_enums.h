/* Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (C) 2015-2022 Google Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
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
 * Author: Camden Stocker <camden@lunarg.com>
 * Author: Nadav Geva <nadav.geva@amd.com>
 */

#ifndef BEST_PRACTICES_ERROR_ENUMS_H_
#define BEST_PRACTICES_ERROR_ENUMS_H_

// Suppress unused warning on Linux
#if defined(__GNUC__)
#define DECORATE_UNUSED __attribute__((unused))
#else
#define DECORATE_UNUSED
#endif

static const char DECORATE_UNUSED *kVUID_BestPractices_CreateInstance_ExtensionMismatch =
    "UNASSIGNED-BestPractices-vkCreateInstance-extension-mismatch";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateDevice_ExtensionMismatch =
    "UNASSIGNED-BestPractices-vkCreateDevice-extension-mismatch";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateInstance_DeprecatedExtension =
    "UNASSIGNED-BestPractices-vkCreateInstance-deprecated-extension";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateDevice_DeprecatedExtension =
    "UNASSIGNED-BestPractices-vkCreateDevice-deprecated-extension";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateInstance_SpecialUseExtension_CADSupport =
    "UNASSIGNED-BestPractices-vkCreateInstance-specialuse-extension-cadsupport";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateInstance_SpecialUseExtension_D3DEmulation =
    "UNASSIGNED-BestPractices-vkCreateInstance-specialuse-extension-d3demulation";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateInstance_SpecialUseExtension_DevTools =
    "UNASSIGNED-BestPractices-vkCreateInstance-specialuse-extension-devtools";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateInstance_SpecialUseExtension_Debugging =
    "UNASSIGNED-BestPractices-vkCreateInstance-specialuse-extension-debugging";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateInstance_SpecialUseExtension_GLEmulation =
    "UNASSIGNED-BestPractices-vkCreateInstance-specialuse-extension-glemulation";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateDevice_SpecialUseExtension_CADSupport =
    "UNASSIGNED-BestPractices-vkCreateDevice-specialuse-extension-cadsupport";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateDevice_SpecialUseExtension_D3DEmulation =
    "UNASSIGNED-BestPractices-vkCreateDevice-specialuse-extension-d3demulation";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateDevice_SpecialUseExtension_DevTools =
    "UNASSIGNED-BestPractices-vkCreateDevice-specialuse-extension-devtools";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateDevice_SpecialUseExtension_Debugging =
    "UNASSIGNED-BestPractices-vkCreateDevice-specialuse-extension-debugging";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateDevice_SpecialUseExtension_GLEmulation =
    "UNASSIGNED-BestPractices-vkCreateDevice-specialuse-extension-glemulation";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateDevice_API_Mismatch =
    "UNASSIGNED-BestPractices-vkCreateDevice-API-version-mismatch";
static const char DECORATE_UNUSED *kVUID_BestPractices_SharingModeExclusive =
    "UNASSIGNED-BestPractices-vkCreateBuffer-sharing-mode-exclusive";
static const char DECORATE_UNUSED *kVUID_BestPractices_RenderPass_Attatchment =
    "UNASSIGNED-BestPractices-vkCreateRenderPass-attatchment";
static const char DECORATE_UNUSED *kVUID_BestPractices_AllocateMemory_TooManyObjects =
    "UNASSIGNED-BestPractices-vkAllocateMemory-too-many-objects";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreatePipelines_MultiplePipelines =
    "UNASSIGNED-BestPractices-vkCreatePipelines-multiple-pipelines-no-cache";
static const char DECORATE_UNUSED *kVUID_BestPractices_PipelineStageFlags = "UNASSIGNED-BestPractices-pipeline-stage-flags";
static const char DECORATE_UNUSED *kVUID_BestPractices_CmdDraw_InstanceCountZero =
    "UNASSIGNED-BestPractices-vkCmdDraw-instance-count-zero";
static const char DECORATE_UNUSED *kVUID_BestPractices_CmdDraw_DrawCountZero = "UNASSIGNED-BestPractices-vkCmdDraw-draw-count-zero";
static const char DECORATE_UNUSED *kVUID_BestPractices_CmdDispatch_GroupCountZero =
    "UNASSIGNED-BestPractices-vkCmdDispatch-group-count-zero";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateDevice_PDFeaturesNotCalled =
    "UNASSIGNED-BestPractices-vkCreateDevice-physical-device-features-not-retrieved";
static const char DECORATE_UNUSED *kVUID_BestPractices_Swapchain_GetSurfaceNotCalled =
    "UNASSIGNED-BestPractices-vkCreateSwapchainKHR-surface-not-retrieved";
static const char DECORATE_UNUSED *kVUID_BestPractices_DisplayPlane_PropertiesNotCalled =
    "UNASSIGNED-BestPractices-vkGetDisplayPlaneSupportedDisplaysKHR-properties-not-retrieved";
static const char DECORATE_UNUSED *kVUID_BestPractices_BufferMemReqNotCalled =
    "UNASSIGNED-BestPractices-vkBindBufferMemory-requirements-not-retrieved";
static const char DECORATE_UNUSED *kVUID_BestPractices_ImageMemReqNotCalled =
    "UNASSIGNED-BestPractices-vkBindImageMemory-requirements-not-retrieved";
static const char DECORATE_UNUSED *kVUID_BestPractices_BindAccelNV_NoMemReqQuery =
    "UNASSIGNED-BestPractices-BindAccelerationStructureMemoryNV-requirements-not-retrieved";
static const char DECORATE_UNUSED *kVUID_BestPractices_DrawState_VtxIndexOutOfBounds =
    "UNASSIGNED-BestPractices-DrawState-VtxIndexOutOfBounds";
static const char DECORATE_UNUSED *kVUID_BestPractices_DrawState_ClearCmdBeforeDraw =
    "UNASSIGNED-BestPractices-DrawState-ClearCmdBeforeDraw";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateCommandPool_CommandBufferReset =
    "UNASSIGNED-BestPractices-vkCreateCommandPool-command-buffer-reset";
static const char DECORATE_UNUSED *kVUID_BestPractices_BeginCommandBuffer_SimultaneousUse =
    "UNASSIGNED-BestPractices-vkBeginCommandBuffer-simultaneous-use";
static const char DECORATE_UNUSED *kVUID_BestPractices_AllocateMemory_SmallAllocation =
    "UNASSIGNED-BestPractices-vkAllocateMemory-small-allocation";
static const char DECORATE_UNUSED *kVUID_BestPractices_SmallDedicatedAllocation =
    "UNASSIGNED-BestPractices-vkBindMemory-small-dedicated-allocation";
static const char DECORATE_UNUSED *kVUID_BestPractices_NonLazyTransientImage =
    "UNASSIGNED-BestPractices-vkBindImageMemory-non-lazy-transient-image";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateRenderPass_ImageRequiresMemory =
    "UNASSIGNED-BestPractices-vkCreateRenderPass-image-requires-memory";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateFramebuffer_AttachmentShouldBeTransient =
    "UNASSIGNED-BestPractices-vkCreateFramebuffer-attachment-should-be-transient";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateFramebuffer_AttachmentShouldNotBeTransient =
    "UNASSIGNED-BestPractices-vkCreateFramebuffer-attachment-should-not-be-transient";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreatePipelines_TooManyInstancedVertexBuffers =
    "UNASSIGNED-BestPractices-vkCreateGraphicsPipelines-too-many-instanced-vertex-buffers";
static const char DECORATE_UNUSED *kVUID_BestPractices_ClearAttachments_ClearAfterLoad =
    "UNASSIGNED-BestPractices-vkCmdClearAttachments-clear-after-load";
static const char DECORATE_UNUSED *kVUID_BestPractices_Error_Result = "UNASSIGNED-BestPractices-Error-Result";
static const char DECORATE_UNUSED *kVUID_BestPractices_Failure_Result = "UNASSIGNED-BestPractices-Failure-Result";
static const char DECORATE_UNUSED *kVUID_BestPractices_NonSuccess_Result = "UNASSIGNED-BestPractices-NonSuccess-Result";
static const char DECORATE_UNUSED *kVUID_BestPractices_SuboptimalSwapchain = "UNASSIGNED-BestPractices-SuboptimalSwapchain";
static const char DECORATE_UNUSED *kVUID_BestPractices_SuboptimalSwapchainImageCount =
    "UNASSIGNED-BestPractices-vkCreateSwapchainKHR-suboptimal-swapchain-image-count";
static const char DECORATE_UNUSED *kVUID_BestPractices_Swapchain_InvalidCount = "UNASSIGNED-BestPractices-SwapchainInvalidCount";
static const char DECORATE_UNUSED *kVUID_BestPractices_DepthBiasNoAttachment = "UNASSIGNED-BestPractices-DepthBiasNoAttachment";
static const char DECORATE_UNUSED *kVUID_BestPractices_SpirvDeprecated_WorkgroupSize =
    "UNASSIGNED-BestPractices-SpirvDeprecated_WorkgroupSize";
static const char DECORATE_UNUSED *kVUID_BestPractices_ImageCreateFlags = "UNASSIGNED-BestPractices-ImageCreateFlags";
static const char DECORATE_UNUSED *kVUID_BestPractices_TransitionUndefinedToReadOnly =
    "UNASSIGNED-BestPractices-TransitionUndefinedToReadOnly";
static const char DECORATE_UNUSED *kVUID_BestPractices_EmptyDescriptorPool =
    "UNASSIGNED-BestPractices-EmptyDescriptorPool";

// Arm-specific best practice
static const char DECORATE_UNUSED *kVUID_BestPractices_AllocateDescriptorSets_SuboptimalReuse =
    "UNASSIGNED-BestPractices-vkAllocateDescriptorSets-suboptimal-reuse";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateComputePipelines_ComputeThreadGroupAlignment =
    "UNASSIGNED-BestPractices-vkCreateComputePipelines-compute-thread-group-alignment";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateComputePipelines_ComputeWorkGroupSize =
    "UNASSIGNED-BestPractices-vkCreateComputePipelines-compute-work-group-size";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateComputePipelines_ComputeSpatialLocality =
    "UNASSIGNED-BestPractices-vkCreateComputePipelines-compute-spatial-locality";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreatePipelines_MultisampledBlending =
    "UNASSIGNED-BestPractices-vkCreatePipelines-multisampled-blending";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateImage_TooLargeSampleCount =
    "UNASSIGNED-BestPractices-vkCreateImage-too-large-sample-count";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateImage_NonTransientMSImage =
    "UNASSIGNED-BestPractices-vkCreateImage-non-transient-ms-image";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateSampler_DifferentWrappingModes =
    "UNASSIGNED-BestPractices-vkCreateSampler-different-wrapping-modes";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateSampler_LodClamping =
    "UNASSIGNED-BestPractices-vkCreateSampler-lod-clamping";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateSampler_LodBias = "UNASSIGNED-BestPractices-vkCreateSampler-lod-bias";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateSampler_BorderClampColor =
    "UNASSIGNED-BestPractices-vkCreateSampler-border-clamp-color";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateSampler_UnnormalizedCoordinates =
    "UNASSIGNED-BestPractices-vkCreateSampler-unnormalized-coordinates";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateSampler_Anisotropy =
    "UNASSIGNED-BestPractices-vkCreateSampler-anisotropy";
static const char DECORATE_UNUSED *kVUID_BestPractices_CmdResolveImage_ResolvingImage =
    "UNASSIGNED-BestPractices-vkCmdResolveImage-resolving-image";
static const char DECORATE_UNUSED *kVUID_BestPractices_CmdResolveImage2KHR_ResolvingImage =
    "UNASSIGNED-BestPractices-vkCmdResolveImage2KHR-resolving-image";
static const char DECORATE_UNUSED *kVUID_BestPractices_CmdResolveImage2_ResolvingImage =
    "UNASSIGNED-BestPractices-vkCmdResolveImage2-resolving-image";
static const char DECORATE_UNUSED *kVUID_BestPractices_CmdDrawIndexed_ManySmallIndexedDrawcalls =
    "UNASSIGNED-BestPractices-vkCmdDrawIndexed-many-small-indexed-drawcalls";
static const char DECORATE_UNUSED *kVUID_BestPractices_CmdDrawIndexed_SparseIndexBuffer =
    "UNASSIGNED-BestPractices-vkCmdDrawIndexed-sparse-index-buffer";
static const char DECORATE_UNUSED *kVUID_BestPractices_CmdDrawIndexed_PostTransformCacheThrashing =
    "UNASSIGNED-BestPractices-vkCmdDrawIndexed-post-transform-cache-thrashing";
static const char DECORATE_UNUSED *kVUID_BestPractices_BeginCommandBuffer_OneTimeSubmit =
    "UNASSIGNED-BestPractices-vkBeginCommandBuffer-one-time-submit";
static const char DECORATE_UNUSED *kVUID_BestPractices_BeginRenderPass_ZeroSizeRenderArea =
    "UNASSIGNED-BestPractices-vkCmdBeginRenderPass-zero-size-render-area";
static const char DECORATE_UNUSED *kVUID_BestPractices_BeginRenderPass_AttachmentNeedsReadback =
    "UNASSIGNED-BestPractices-vkCmdBeginRenderPass-attachment-needs-readback";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateSwapchain_PresentMode =
    "UNASSIGNED-BestPractices-vkCreateSwapchainKHR-swapchain-presentmode-not-fifo";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreatePipelines_DepthBias_Zero =
    "UNASSIGNED-BestPractices-vkCreatePipelines-depthbias-zero";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateDevice_RobustBufferAccess =
    "UNASSIGNED-BestPractices-vkCreateDevice-RobustBufferAccess";
static const char DECORATE_UNUSED *kVUID_BestPractices_EndRenderPass_DepthPrePassUsage =
    "UNASSIGNED-BestPractices-vkCmdEndRenderPass-depth-pre-pass-usage";
static const char DECORATE_UNUSED *kVUID_BestPractices_EndRenderPass_RedundantAttachmentOnTile =
    "UNASSIGNED-BestPractices-vkCmdEndRenderPass-redundant-attachment-on-tile";
static const char DECORATE_UNUSED *kVUID_BestPractices_RenderPass_RedundantStore =
    "UNASSIGNED-BestPractices-RenderPass-redundant-store";
static const char DECORATE_UNUSED *kVUID_BestPractices_RenderPass_RedundantClear =
    "UNASSIGNED-BestPractices-RenderPass-redundant-clear";
static const char DECORATE_UNUSED *kVUID_BestPractices_RenderPass_InefficientClear =
    "UNASSIGNED-BestPractices-RenderPass-inefficient-clear";
static const char DECORATE_UNUSED *kVUID_BestPractices_RenderPass_BlitImage_LoadOpLoad =
    "UNASSIGNED-BestPractices-RenderPass-blitimage-loadopload";
static const char DECORATE_UNUSED *kVUID_BestPractices_RenderPass_CopyImage_LoadOpLoad =
    "UNASSIGNED-BestPractices-RenderPass-copyimage-loadopload";
static const char DECORATE_UNUSED *kVUID_BestPractices_RenderPass_ResolveImage_LoadOpLoad =
    "UNASSIGNED-BestPractices-RenderPass-resolveimage-loadopload";


// AMD-specific best practice
static const char DECORATE_UNUSED *kVUID_BestPractices_CmdBuffer_AvoidTinyCmdBuffers =
    "UNASSIGNED-BestPractices-VkCommandBuffer-AvoidTinyCmdBuffers";
static const char DECORATE_UNUSED *kVUID_BestPractices_CmdBuffer_AvoidSecondaryCmdBuffers =
    "UNASSIGNED-BestPractices-VkCommandBuffer-AvoidSecondaryCmdBuffers";
static const char DECORATE_UNUSED *kVUID_BestPractices_CmdBuffer_AvoidSmallSecondaryCmdBuffers =
    "UNASSIGNED-BestPractices-VkCommandBuffer-AvoidSmallSecondaryCmdBuffers";
static const char DECORATE_UNUSED *kVUID_BestPractices_CmdBuffer_AvoidClearSecondaryCmdBuffers =
    "UNASSIGNED-BestPractices-VkCommandBuffer-AvoidClearSecondaryCmdBuffers";
static const char DECORATE_UNUSED *kVUID_BestPractices_DrawState_AvoidVertexBindEveryDraw =
    "UNASSIGNED-BestPractices-DrawState-AvoidVertexBindEveryDraw";
static const char DECORATE_UNUSED *kVUID_BestPractices_CmdPool_DisparateSizedCmdBuffers =
    "UNASSIGNED-BestPractices-CmdPool-DisparateSizedCmdBuffers";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreatePipelines_TooManyPipelines =
    "UNASSIGNED-BestPractices-CreatePipelines-TooManyPipelines";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreatePipelines_MultiplePipelineCaches =
    "UNASSIGNED-BestPractices-vkCreatePipelines-multiple-pipelines-caches";
static const char DECORATE_UNUSED *kVUID_BestPractices_vkImage_DontUseMutableRenderTargets =
    "UNASSIGNED-BestPractices-vkImage-DontUseMutableRenderTargets";
static const char DECORATE_UNUSED *kVUID_BestPractices_vkImage_AvoidImageToImageCopy =
    "UNASSIGNED-BestPractices-vkImage-AvoidImageToImageCopy";
static const char DECORATE_UNUSED *kVUID_BestPractices_vkImage_AvoidConcurrentRenderTargets =
    "UNASSIGNED-BestPractices-vkImage-AvoidConcurrentRenderTargets";
static const char DECORATE_UNUSED *kVUID_BestPractices_vkImage_DontUseStorageRenderTargets =
    "UNASSIGNED-BestPractices-vkImage-DontUseStorageRenderTargets";
static const char DECORATE_UNUSED *kVUID_BestPractices_vkImage_AvoidGeneral = "UNASSIGNED-BestPractices-vkImage-AvoidGeneral";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreatePipelines_AvoidPrimitiveRestart =
    "UNASSIGNED-BestPractices-CreatePipelines-AvoidPrimitiveRestart";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreatePipelines_MinimizeNumDynamicStates =
    "UNASSIGNED-BestPractices-CreatePipelines-MinimizeNumDynamicStates";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreatePipelinesLayout_KeepLayoutSmall =
    "UNASSIGNED-BestPractices-CreatePipelinesLayout-KeepLayoutSmall";
static const char DECORATE_UNUSED *kVUID_BestPractices_UpdateDescriptors_AvoidCopyingDescriptors =
    "UNASSIGNED-BestPractices-UpdateDescriptors-AvoidCopyingDescriptors";
static const char DECORATE_UNUSED *kVUID_BestPractices_UpdateDescriptors_PreferNonTemplate =
    "UNASSIGNED-BestPractices-UpdateDescriptors-PreferNonTemplate";
static const char DECORATE_UNUSED *kVUID_BestPractices_ClearAttachment_FastClearValues =
    "UNASSIGNED-BestPractices-ClearAttachment-FastClearValues";
static const char DECORATE_UNUSED *kVUID_BestPractices_ClearAttachment_ClearImage =
    "UNASSIGNED-BestPractices-ClearAttachment-ClearImage";
static const char DECORATE_UNUSED *kVUID_BestPractices_CmdBuffer_backToBackBarrier =
    "UNASSIGNED-BestPractices-CmdBuffer-backToBackBarrier";
static const char DECORATE_UNUSED *kVUID_BestPractices_CmdBuffer_highBarrierCount =
    "UNASSIGNED-BestPractices-CmdBuffer-highBarrierCount";
static const char DECORATE_UNUSED *kVUID_BestPractices_PipelineBarrier_readToReadBarrier =
    "UNASSIGNED-BestPractices-PipelineBarrier-readToReadBarrier";
static const char DECORATE_UNUSED *kVUID_BestPractices_Submission_ReduceNumberOfSubmissions =
    "UNASSIGNED-BestPractices-Submission-ReduceNumberOfSubmissions";
static const char DECORATE_UNUSED *kVUID_BestPractices_Pipeline_SortAndBind = "UNASSIGNED-BestPractices-Pipeline-SortAndBind";
static const char DECORATE_UNUSED *kVUID_BestPractices_Pipeline_WorkPerPipelineChange =
    "UNASSIGNED-BestPractices-Pipeline-WorkPerPipelineChange";
static const char DECORATE_UNUSED *kVUID_BestPractices_SyncObjects_HighNumberOfFences =
    "UNASSIGNED-BestPractices-SyncObjects-HighNumberOfFences";
static const char DECORATE_UNUSED *kVUID_BestPractices_SyncObjects_HighNumberOfSemaphores =
    "UNASSIGNED-BestPractices-SyncObjects-HighNumberOfSemaphores";
static const char DECORATE_UNUSED *kVUID_BestPractices_DynamicRendering_NotSupported =
    "UNASSIGNED-BestPractices-DynamicRendering-NotSupported";

// Imagination Technologies best practices
static const char DECORATE_UNUSED *kVUID_BestPractices_Texture_Format_PVRTC_Outdated =
    "UNASSIGNED-BestPractices-Texture-Format-PVRTC-Outdated";
#endif
