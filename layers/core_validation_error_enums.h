/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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
#ifndef CORE_VALIDATION_ERROR_ENUMS_H_
#define CORE_VALIDATION_ERROR_ENUMS_H_

// clang-format off

[[maybe_unused]] static const char *kVUID_Core_Bound_Resource_FreedMemoryAccess = "UNASSIGNED-CoreValidation-BoundResourceFreedMemoryAccess";

[[maybe_unused]] static const char *kVUID_Core_DrawState_CommandBufferSingleSubmitViolation = "UNASSIGNED-CoreValidation-DrawState-CommandBufferSingleSubmitViolation";
[[maybe_unused]] static const char *kVUID_Core_DrawState_ExtensionNotEnabled = "UNASSIGNED-CoreValidation-DrawState-ExtensionNotEnabled";
[[maybe_unused]] static const char *kVUID_Core_DrawState_InvalidCommandBuffer = "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer";
[[maybe_unused]] static const char *kVUID_Core_DrawState_InvalidCommandBufferSimultaneousUse = "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBufferSimultaneousUse";
[[maybe_unused]] static const char *kVUID_Core_DrawState_InvalidDescriptorSet = "UNASSIGNED-CoreValidation-DrawState-InvalidDescriptorSet";
[[maybe_unused]] static const char *kVUID_Core_DrawState_InvalidEvent = "UNASSIGNED-CoreValidation-DrawState-InvalidEvent";
[[maybe_unused]] static const char *kVUID_Core_DrawState_InvalidImageAspect = "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect";
[[maybe_unused]] static const char *kVUID_Core_DrawState_InvalidImageLayout = "UNASSIGNED-CoreValidation-DrawState-InvalidImageLayout";
[[maybe_unused]] static const char *kVUID_Core_DrawState_InvalidPipelineCreateState = "UNASSIGNED-CoreValidation-DrawState-InvalidPipelineCreateState";
[[maybe_unused]] static const char *kVUID_Core_DrawState_InvalidQuery = "UNASSIGNED-CoreValidation-DrawState-InvalidQuery";
[[maybe_unused]] static const char *kVUID_Core_DrawState_QueryNotReset = "UNASSIGNED-CoreValidation-DrawState-QueryNotReset";
[[maybe_unused]] static const char *kVUID_Core_DrawState_InvalidRenderpass = "UNASSIGNED-CoreValidation-DrawState-InvalidRenderpass";
[[maybe_unused]] static const char *kVUID_Core_DrawState_InvalidSecondaryCommandBuffer = "UNASSIGNED-CoreValidation-DrawState-InvalidSecondaryCommandBuffer";
[[maybe_unused]] static const char *kVUID_Core_DrawState_NoActiveRenderpass = "UNASSIGNED-CoreValidation-DrawState-NoActiveRenderpass";
[[maybe_unused]] static const char *kVUID_Core_DrawState_NoEndCommandBuffer = "UNASSIGNED-CoreValidation-DrawState-NoEndCommandBuffer";
[[maybe_unused]] static const char *kVUID_Core_DrawState_QueueForwardProgress = "UNASSIGNED-CoreValidation-DrawState-QueueForwardProgress";
[[maybe_unused]] static const char *kVUID_Core_DrawState_InvalidImageView = "UNASSIGNED-CoreValidation-DrawState-InvalidImageView";

[[maybe_unused]] static const char *kVUID_Core_Shader_InconsistentSpirv = "UNASSIGNED-CoreValidation-Shader-InconsistentSpirv";
[[maybe_unused]] static const char *kVUID_Core_Shader_InputNotProduced = "UNASSIGNED-CoreValidation-Shader-InputNotProduced";
[[maybe_unused]] static const char *kVUID_Core_Shader_InterfaceTypeMismatch = "UNASSIGNED-CoreValidation-Shader-InterfaceTypeMismatch";
[[maybe_unused]] static const char *kVUID_Core_Shader_AtomicFeature = "UNASSIGNED-CoreValidation-Shader-AtomicFeature";
[[maybe_unused]] static const char *kVUID_Core_Shader_OutputNotConsumed = "UNASSIGNED-CoreValidation-Shader-OutputNotConsumed";
[[maybe_unused]] static const char *kVUID_Core_Shader_NoAlphaAtLocation0WithAlphaToCoverage = "UNASSIGNED-CoreValidation-Shader-NoAlphaAtLocation0WithAlphaToCoverage";
[[maybe_unused]] static const char *kVUID_Core_Shader_CooperativeMatrixType = "UNASSIGNED-CoreValidation-Shader-CooperativeMatrixType";
[[maybe_unused]] static const char *kVUID_Core_Shader_CooperativeMatrixMulAdd = "UNASSIGNED-CoreValidation-Shader-CooperativeMatrixMulAdd";
[[maybe_unused]] static const char *kVUID_Core_Shader_MaxComputeWorkGroupSize = "UNASSIGNED-CoreValidation-Shader-MaxComputeWorkGroupSize";

[[maybe_unused]] static const char *kVUID_Core_Swapchain_PriorCount = "UNASSIGNED-CoreValidation-SwapchainPriorCount";
[[maybe_unused]] static const char *kVUID_Core_Swapchain_PreTransform = "UNASSIGNED-CoreValidation-SwapchainPreTransform";
[[maybe_unused]] static const char *kVUID_Core_BindImageMemory_Swapchain = "UNASSIGNED-CoreValidation-BindImageMemory-Swapchain";

[[maybe_unused]] static const char *kVUID_Core_Image_InvalidFormatLimitsViolation = "UNASSIGNED-CoreValidation-Image-InvalidFormatLimitsViolation";

[[maybe_unused]] static const char *kVUID_Core_PushDescriptorUpdate_TemplateType = "UNASSIGNED-CoreValidation-vkCmdPushDescriptorSetWithTemplateKHR-descriptorUpdateTemplate-templateType";
[[maybe_unused]] static const char *kVUID_Core_PushDescriptorUpdate_Template_SetMismatched = "UNASSIGNED-CoreValidation-vkCmdPushDescriptorSetWithTemplateKHR-set";
[[maybe_unused]] static const char *kVUID_Core_PushDescriptorUpdate_Template_LayoutMismatched = "UNASSIGNED-CoreValidation-vkCmdPushDescriptorSetWithTemplateKHR-layout";

[[maybe_unused]] static const char *kVUID_Core_CmdBuildAccelNV_NoScratchMemReqQuery = "UNASSIGNED-CoreValidation-vkCmdBuildAccelerationStructureNV-scratch-requirements";
[[maybe_unused]] static const char *kVUID_Core_CmdBuildAccelNV_NoUpdateMemReqQuery = "UNASSIGNED-CoreValidation-vkCmdBuildAccelerationStructureNV-update-requirements";

[[maybe_unused]] static const char *kVUID_Core_CreatInstance_Status = "UNASSIGNED-khronos-validation-createinstance-status-message";
[[maybe_unused]] static const char *kVUID_Core_CreateInstance_Debug_Warning = "UNASSIGNED-khronos-Validation-debug-build-warning-message";
[[maybe_unused]] static const char *kVUID_Core_CreateInstance_Locking_Warning = "UNASSIGNED-khronos-Validation-fine-grained-locking-warning-message";

[[maybe_unused]] static const char *kVUID_Core_ImageMemoryBarrier_SharingModeExclusiveSameFamily = "UNASSIGNED-CoreValidation-VkImageMemoryBarrier-sharing-mode-exclusive-same-family";
[[maybe_unused]] static const char *kVUID_Core_ImageMemoryBarrier2_SharingModeExclusiveSameFamily = "UNASSIGNED-CoreValidation-VkImageMemoryBarrier2KHR-sharing-mode-exclusive-same-family";
[[maybe_unused]] static const char *kVUID_Core_BufferMemoryBarrier_SharingModeExclusiveSameFamily = "UNASSIGNED-CoreValidation-VkBufferMemoryBarrier-sharing-mode-exclusive-same-family";
[[maybe_unused]] static const char *kVUID_Core_BufferMemoryBarrier2_SharingModeExclusiveSameFamily = "UNASSIGNED-CoreValidation-VkBufferMemoryBarrier2KHR-sharing-mode-exclusive-same-family";

[[maybe_unused]] static const char *kVUID_Core_invalidDepthStencilFormat = "UNASSIGNED-CoreValidation-depthStencil-format";

// clang-format on

#undef DECORATE_UNUSED

#endif  // CORE_VALIDATION_ERROR_ENUMS_H_
