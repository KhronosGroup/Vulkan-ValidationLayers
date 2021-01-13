/* Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (C) 2015-2021 Google Inc.
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
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateInstance_SpecialUseExtension =
    "UNASSIGNED-BestPractices-vkCreateInstance-specialuse-extension";
static const char DECORATE_UNUSED *kVUID_BestPractices_CreateDevice_SpecialUseExtension =
    "UNASSIGNED-BestPractices-vkCreateDevice-specialuse-extension";
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
static const char DECORATE_UNUSED *kVUID_BestPractices_CmdDrawIndexed_ManySmallIndexedDrawcalls =
    "UNASSIGNED-BestPractices-vkCmdDrawIndexed-many-small-indexed-drawcalls";
static const char DECORATE_UNUSED *kVUID_BestPractices_CmdDrawIndexed_SparseIndexBuffer =
    "UNASSIGNED-BestPractices-vkCmdDrawIndexed-sparse-index-buffer";
static const char DECORATE_UNUSED *kVUID_BestPractices_CmdDrawIndexed_PostTransformCacheThrashing =
    "UNASSIGNED-BestPractices-vkCmdDrawIndexed-post-transform-cache-thrashing";
static const char DECORATE_UNUSED *kVUID_BestPractices_BeginCommandBuffer_OneTimeSubmit =
    "UNASSIGNED-BestPractices-vkBeginCommandBuffer-one-time-submit";
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

#endif
