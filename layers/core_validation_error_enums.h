/* Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (C) 2015-2022 Google Inc.
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
 * Author: Camden Stocker <camden@lunarg.com>
 */
#ifndef CORE_VALIDATION_ERROR_ENUMS_H_
#define CORE_VALIDATION_ERROR_ENUMS_H_

// Suppress unused warning on Linux
#if defined(__GNUC__)
#define DECORATE_UNUSED __attribute__((unused))
#else
#define DECORATE_UNUSED
#endif

// clang-format off

static const char DECORATE_UNUSED *kVUID_Core_Bound_Resource_FreedMemoryAccess = "UNASSIGNED-CoreValidation-BoundResourceFreedMemoryAccess";
static const char DECORATE_UNUSED *kVUID_Core_MemTrack_InvalidState = "UNASSIGNED-CoreValidation-MemTrack-InvalidState";

static const char DECORATE_UNUSED *kVUID_Core_DrawState_CommandBufferSingleSubmitViolation = "UNASSIGNED-CoreValidation-DrawState-CommandBufferSingleSubmitViolation";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_DescriptorSetNotBound = "UNASSIGNED-CoreValidation-DrawState-DescriptorSetNotBound";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_ExtensionNotEnabled = "UNASSIGNED-CoreValidation-DrawState-ExtensionNotEnabled";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidCommandBuffer = "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidCommandBufferSimultaneousUse = "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBufferSimultaneousUse";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidDescriptorSet = "UNASSIGNED-CoreValidation-DrawState-InvalidDescriptorSet";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidEvent = "UNASSIGNED-CoreValidation-DrawState-InvalidEvent";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidExtents = "UNASSIGNED-CoreValidation-DrawState-InvalidExtents";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidImageAspect = "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidImageLayout = "UNASSIGNED-CoreValidation-DrawState-InvalidImageLayout";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidPipelineCreateState = "UNASSIGNED-CoreValidation-DrawState-InvalidPipelineCreateState";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidQuery = "UNASSIGNED-CoreValidation-DrawState-InvalidQuery";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_QueryNotReset = "UNASSIGNED-CoreValidation-DrawState-QueryNotReset";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidRenderpass = "UNASSIGNED-CoreValidation-DrawState-InvalidRenderpass";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidSecondaryCommandBuffer = "UNASSIGNED-CoreValidation-DrawState-InvalidSecondaryCommandBuffer";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_MismatchedImageType = "UNASSIGNED-CoreValidation-DrawState-MismatchedImageType";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_NoActiveRenderpass = "UNASSIGNED-CoreValidation-DrawState-NoActiveRenderpass";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_NoEndCommandBuffer = "UNASSIGNED-CoreValidation-DrawState-NoEndCommandBuffer";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_PipelineLayoutsIncompatible = "UNASSIGNED-CoreValidation-DrawState-PipelineLayoutsIncompatible";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_QueueForwardProgress = "UNASSIGNED-CoreValidation-DrawState-QueueForwardProgress";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_SwapchainImagesNotFound = "UNASSIGNED-CoreValidation-DrawState-SwapchainImagesNotFound";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_SwapchainUnsupportedQueue = "UNASSIGNED-CoreValidation-DrawState-SwapchainUnsupportedQueue";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidImageView = "UNASSIGNED-CoreValidation-DrawState-InvalidImageView";

static const char DECORATE_UNUSED *kVUID_Core_Shader_InconsistentSpirv = "UNASSIGNED-CoreValidation-Shader-InconsistentSpirv";
static const char DECORATE_UNUSED *kVUID_Core_Shader_InconsistentVi = "UNASSIGNED-CoreValidation-Shader-InconsistentVi";
static const char DECORATE_UNUSED *kVUID_Core_Shader_InputAttachmentTypeMismatch = "UNASSIGNED-CoreValidation-Shader-InputAttachmentTypeMismatch";
static const char DECORATE_UNUSED *kVUID_Core_Shader_InputNotProduced = "UNASSIGNED-CoreValidation-Shader-InputNotProduced";
static const char DECORATE_UNUSED *kVUID_Core_Shader_InterfaceTypeMismatch = "UNASSIGNED-CoreValidation-Shader-InterfaceTypeMismatch";
static const char DECORATE_UNUSED *kVUID_Core_Shader_AtomicFeature = "UNASSIGNED-CoreValidation-Shader-AtomicFeature";
static const char DECORATE_UNUSED *kVUID_Core_Shader_MissingInputAttachment = "UNASSIGNED-CoreValidation-Shader-MissingInputAttachment";
static const char DECORATE_UNUSED *kVUID_Core_Shader_OutputNotConsumed = "UNASSIGNED-CoreValidation-Shader-OutputNotConsumed";
static const char DECORATE_UNUSED *kVUID_Core_Shader_MissingPointSizeBuiltIn = "UNASSIGNED-CoreValidation-Shader-PointSizeMissing";
static const char DECORATE_UNUSED *kVUID_Core_Shader_PointSizeBuiltInOverSpecified = "UNASSIGNED-CoreValidation-Shader-PointSizeOverSpecified";
static const char DECORATE_UNUSED *kVUID_Core_Shader_NoAlphaAtLocation0WithAlphaToCoverage = "UNASSIGNED-CoreValidation-Shader-NoAlphaAtLocation0WithAlphaToCoverage";
static const char DECORATE_UNUSED *kVUID_Core_Shader_CooperativeMatrixType = "UNASSIGNED-CoreValidation-Shader-CooperativeMatrixType";
static const char DECORATE_UNUSED *kVUID_Core_Shader_CooperativeMatrixMulAdd = "UNASSIGNED-CoreValidation-Shader-CooperativeMatrixMulAdd";
static const char DECORATE_UNUSED *kVUID_Core_Shader_InvalidExtension = "UNASSIGNED-CoreValidation-Shader-InvalidSpirvExtension";
static const char DECORATE_UNUSED *kVUID_Core_Shader_MaxComputeWorkGroupSize = "UNASSIGNED-CoreValidation-Shader-MaxComputeWorkGroupSize";
static const char DECORATE_UNUSED *kVUID_Stateless_InvalidShaderStagesArray = "UNASSIGNED-Stateless-InvalidShaderStagesArray";

static const char DECORATE_UNUSED *kVUID_Core_DevLimit_CountMismatch = "UNASSIGNED-CoreValidation-DevLimitCountMismatch";
static const char DECORATE_UNUSED *kVUID_Core_DevLimit_MissingQueryCount = "UNASSIGNED-CoreValidation-DevLimit-MissingQueryCount";
static const char DECORATE_UNUSED *kVUID_Core_DevLimit_MustQueryCount = "UNASSIGNED-CoreValidation-DevLimit-MustQueryCount";

static const char DECORATE_UNUSED *kVUID_Core_Swapchain_PriorCount = "UNASSIGNED-CoreValidation-SwapchainPriorCount";
static const char DECORATE_UNUSED *kVUID_Core_Swapchain_PreTransform = "UNASSIGNED-CoreValidation-SwapchainPreTransform";
static const char DECORATE_UNUSED *kVUID_Core_BindImageMemory_Swapchain = "UNASSIGNED-CoreValidation-BindImageMemory-Swapchain";
static const char DECORATE_UNUSED *kVUID_Core_NonAcquiredSwapchainImageUsed = "UNASSIGNED-CoreValidation-NonAcquiredSwapchainImageUsed";

static const char DECORATE_UNUSED *kVUID_Core_Image_InvalidFormatLimitsViolation = "UNASSIGNED-CoreValidation-Image-InvalidFormatLimitsViolation";

static const char DECORATE_UNUSED *kVUID_Core_PushDescriptorUpdate_TemplateType = "UNASSIGNED-CoreValidation-vkCmdPushDescriptorSetWithTemplateKHR-descriptorUpdateTemplate-templateType";
static const char DECORATE_UNUSED *kVUID_Core_PushDescriptorUpdate_Template_SetMismatched = "UNASSIGNED-CoreValidation-vkCmdPushDescriptorSetWithTemplateKHR-set";
static const char DECORATE_UNUSED *kVUID_Core_PushDescriptorUpdate_Template_LayoutMismatched = "UNASSIGNED-CoreValidation-vkCmdPushDescriptorSetWithTemplateKHR-layout";

static const char DECORATE_UNUSED *kVUID_Core_CmdBuildAccelNV_NoScratchMemReqQuery = "UNASSIGNED-CoreValidation-vkCmdBuildAccelerationStructureNV-scratch-requirements";
static const char DECORATE_UNUSED *kVUID_Core_CmdBuildAccelNV_NoUpdateMemReqQuery = "UNASSIGNED-CoreValidation-vkCmdBuildAccelerationStructureNV-update-requirements";

static const char DECORATE_UNUSED *kVUID_Core_CreatInstance_Status = "UNASSIGNED-khronos-validation-createinstance-status-message";
static const char DECORATE_UNUSED *kVUID_Core_CreateInstance_Debug_Warning = "UNASSIGNED-khronos-Validation-debug-build-warning-message";
static const char DECORATE_UNUSED *kVUID_Core_CreateInstance_Locking_Warning = "UNASSIGNED-khronos-Validation-fine-grained-locking-warning-message";

static const char DECORATE_UNUSED *kVUID_Core_ImageMemoryBarrier_SharingModeExclusiveSameFamily = "UNASSIGNED-CoreValidation-VkImageMemoryBarrier-sharing-mode-exclusive-same-family";
static const char DECORATE_UNUSED *kVUID_Core_ImageMemoryBarrier2_SharingModeExclusiveSameFamily = "UNASSIGNED-CoreValidation-VkImageMemoryBarrier2KHR-sharing-mode-exclusive-same-family";
static const char DECORATE_UNUSED *kVUID_Core_BufferMemoryBarrier_SharingModeExclusiveSameFamily = "UNASSIGNED-CoreValidation-VkBufferMemoryBarrier-sharing-mode-exclusive-same-family";
static const char DECORATE_UNUSED *kVUID_Core_BufferMemoryBarrier2_SharingModeExclusiveSameFamily = "UNASSIGNED-CoreValidation-VkBufferMemoryBarrier2KHR-sharing-mode-exclusive-same-family";

// Pending VUIDs from https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/4585
static const char DECORATE_UNUSED *kVUID_Core_CmdDraw_VertexInput = "UNASSIGNED-vkCmdDraw-vertexInput-not-specified";
static const char DECORATE_UNUSED *kVUID_Core_CmdDrawMultiEXT_VertexInput = "UNASSIGNED-vkCmdDrawMultiEXT-vertexInput-not-specified";
static const char DECORATE_UNUSED *kVUID_Core_CmdDrawIndexed_VertexInput = "UNASSIGNED-vkCmdDrawIndexed-vertexInput-not-specified";
static const char DECORATE_UNUSED *kVUID_Core_CmdDrawMultiIndexedEXT_VertexInput = "UNASSIGNED-vkCmdDrawMultiIndexedEXT-vertexInput-not-specified";
static const char DECORATE_UNUSED *kVUID_Core_CmdDrawIndirect_VertexInput = "UNASSIGNED-vkCmdDrawIndirect-vertexInput-not-specified";
static const char DECORATE_UNUSED *kVUID_Core_CmdDrawIndexedIndirect_VertexInput = "UNASSIGNED-vkCmdDrawIndexedIndirect-vertexInput-not-specified";
static const char DECORATE_UNUSED *kVUID_Core_CmdDrawIndirectCount_VertexInput = "UNASSIGNED-vkCmdDrawIndirectCount-vertexInput-not-specified";
static const char DECORATE_UNUSED *kVUID_Core_CmdDrawIndexedIndirectCount_VertexInput = "UNASSIGNED-vkCmdDrawIndexedIndirectCount-vertexInput-not-specified";
static const char DECORATE_UNUSED *kVUID_Core_CmdDrawMeshTasksNV_VertexInput = "UNASSIGNED-vkCmdDrawMeshTasksNV-vertexInput-not-specified";
static const char DECORATE_UNUSED *kVUID_Core_CmdDrawMeshTasksIndirectNV_VertexInput = "UNASSIGNED-vkCmdDrawMeshTasksIndirectNV-vertexInput-not-specified";
static const char DECORATE_UNUSED *kVUID_Core_CmdDrawMeshTasksIndirectCountNV_VertexInput = "UNASSIGNED-vkCmdDrawMeshTasksIndirectCountNV-vertexInput-not-specified";
static const char DECORATE_UNUSED *kVUID_Core_CmdDrawIndirectByteCountEXT_VertexInput = "UNASSIGNED-vkCmdDrawIndirectByteCountEXT-vertexInput-not-specified";

static const char DECORATE_UNUSED *kVUID_Core_invalidDepthStencilFormat = "UNASSIGNED-CoreValidation-depthStencil-format";

static const char DECORATE_UNUSED *kVUID_Core_VkBindImageMemoryInfo_pNext_missing_VkBindImagePlaneMemoryInfo = "UNASSIGNED-VkBindImageMemoryInfo-pNext-missing-VkBindImagePlaneMemoryInfo";

static const char DECORATE_UNUSED *kVUID_Core_InputDescriptorInvalid = "UNASSIGNED-input-attachment-descriptor-not-in-subpass";

// clang-format on

#undef DECORATE_UNUSED

#endif  // CORE_VALIDATION_ERROR_ENUMS_H_
