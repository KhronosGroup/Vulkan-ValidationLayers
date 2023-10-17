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
#pragma once

// clang-format off

[[maybe_unused]] static const char *kVUID_Core_Bound_Resource_FreedMemoryAccess = "UNASSIGNED-CoreValidation-BoundResourceFreedMemoryAccess";

[[maybe_unused]] static const char *kVUID_Core_DrawState_CommandBufferSingleSubmitViolation = "UNASSIGNED-CoreValidation-DrawState-CommandBufferSingleSubmitViolation";
[[maybe_unused]] static const char *kVUID_Core_DrawState_ExtensionNotEnabled = "UNASSIGNED-CoreValidation-DrawState-ExtensionNotEnabled";
[[maybe_unused]] static const char *kVUID_Core_DrawState_InvalidImageAspect = "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect";
[[maybe_unused]] static const char *kVUID_Core_DrawState_InvalidImageLayout = "UNASSIGNED-CoreValidation-DrawState-InvalidImageLayout";
[[maybe_unused]] static const char *kVUID_Core_DrawState_InvalidRenderpass = "UNASSIGNED-CoreValidation-DrawState-InvalidRenderpass";
[[maybe_unused]] static const char *kVUID_Core_DrawState_InvalidSecondaryCommandBuffer = "UNASSIGNED-CoreValidation-DrawState-InvalidSecondaryCommandBuffer";
[[maybe_unused]] static const char *kVUID_Core_DrawState_QueueForwardProgress = "UNASSIGNED-CoreValidation-DrawState-QueueForwardProgress";
[[maybe_unused]] static const char *kVUID_Core_DrawState_InvalidImageView = "UNASSIGNED-CoreValidation-DrawState-InvalidImageView";
[[maybe_unused]] static const char *kVUID_Core_QueryPool_NotReset = "UNASSIGNED-CoreValidation-QueryPool-NotReset";
[[maybe_unused]] static const char *kVUID_Core_DrawState_BlendOperationAdvanced = "UNASSIGNED-CoreValidation-DrawState-BlendOperationAdvanced";

[[maybe_unused]] static const char *kVUID_Core_Shader_AllowVaryingSubgroupSize = "UNASSIGNED-CoreValidation-Shader-AllowVaryingSubgroupSize";
[[maybe_unused]] static const char *kVUID_Core_Shader_RequireFullSubgroups = "UNASSIGNED-CoreValidation-Shader-RequireFullSubgroups";

[[maybe_unused]] static const char *kVUID_Core_BindImageMemory_Swapchain = "UNASSIGNED-CoreValidation-BindImageMemory-Swapchain";

// TODO - Need to be moved to Best Practice
[[maybe_unused]] static const char *kVUID_Core_Shader_OutputNotConsumed = "UNASSIGNED-CoreValidation-Shader-OutputNotConsumed";
[[maybe_unused]] static const char *kVUID_Core_Swapchain_PriorCount = "UNASSIGNED-CoreValidation-SwapchainPriorCount";
[[maybe_unused]] static const char *kVUID_Core_Swapchain_PreTransform = "UNASSIGNED-CoreValidation-SwapchainPreTransform";
[[maybe_unused]] static const char *kVUID_Core_Image_InvalidFormatLimitsViolation = "UNASSIGNED-CoreValidation-Image-InvalidFormatLimitsViolation";
[[maybe_unused]] static const char *kVUID_Core_CmdBuildAccelNV_NoScratchMemReqQuery = "UNASSIGNED-CoreValidation-vkCmdBuildAccelerationStructureNV-scratch-requirements";
[[maybe_unused]] static const char *kVUID_Core_CmdBuildAccelNV_NoUpdateMemReqQuery = "UNASSIGNED-CoreValidation-vkCmdBuildAccelerationStructureNV-update-requirements";

// These are UNASSIGNED VUID won't have a proper spec VU for special reasons (see where used for more info)
[[maybe_unused]] static const char *kVUID_Core_invalidDepthStencilFormat = "UNASSIGNED-CoreValidation-depthStencil-format";

// clang-format on

#undef DECORATE_UNUSED
