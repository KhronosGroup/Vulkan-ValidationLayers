/* Copyright (c) 2015-2019 The Khronos Group Inc.
 * Copyright (c) 2015-2019 Valve Corporation
 * Copyright (c) 2015-2019 LunarG, Inc.
 * Copyright (C) 2015-2019 Google Inc.
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

#endif
