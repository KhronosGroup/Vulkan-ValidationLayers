/* Copyright (c) 2021-2022 The Khronos Group Inc.
 * Copyright (c) 2021-2022 Valve Corporation
 * Copyright (c) 2021-2022 LunarG, Inc.
 * Copyright (C) 2021-2022 Google Inc.
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
 * Author: Jeremy Gebben <jeremyg@lunarg.com>
 */
#include "sync_vuid_maps.h"
#include "core_error_location.h"
#include "device_state.h"
#include "core_validation.h"
#include <cassert>
#include <algorithm>
#include <array>
#include <vector>

namespace sync_vuid_maps {
using core_error::Field;
using core_error::Func;
using core_error::Struct;
using core_error::FindVUID;
using core_error::Key;
using core_error::Entry;

const std::map<VkPipelineStageFlags2KHR, std::string> kFeatureNameMap{
    {VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT_KHR, "geometryShader"},
    {VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT_KHR, "tessellationShader"},
    {VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT_KHR, "tessellationShader"},
    {VK_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT, "conditionalRendering"},
    {VK_PIPELINE_STAGE_2_FRAGMENT_DENSITY_PROCESS_BIT_EXT, "fragmentDensity"},
    {VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT, "transformFeedback"},
    {VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV, "meshShader"},
    {VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV, "taskShader"},
    {VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR, "shadingRate"},
    {VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, "rayTracing"},
    {VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR, "rayTracing"},
};

// commonvalidity/pipeline_stage_common.txt
// commonvalidity/stage_mask_2_common.txt
// commonvalidity/stage_mask_common.txt
static const std::map<VkPipelineStageFlags2KHR, std::vector<Entry>> kStageMaskErrors{
    {VK_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT,
     {
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstStageMask), "VUID-VkBufferMemoryBarrier2-dstStageMask-03931"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcStageMask), "VUID-VkBufferMemoryBarrier2-srcStageMask-03931"},
         {Key(Func::vkCmdPipelineBarrier, Field::dstStageMask), "VUID-vkCmdPipelineBarrier-dstStageMask-04092"},
         {Key(Func::vkCmdPipelineBarrier, Field::srcStageMask), "VUID-vkCmdPipelineBarrier-srcStageMask-04092"},
         {Key(Func::vkCmdResetEvent2, Field::stageMask), "VUID-vkCmdResetEvent2-stageMask-03931"},
         {Key(Func::vkCmdResetEvent, Field::stageMask), "VUID-vkCmdResetEvent-stageMask-04092"},
         {Key(Func::vkCmdSetEvent, Field::stageMask), "VUID-vkCmdSetEvent-stageMask-04092"},
         {Key(Func::vkCmdWaitEvents, Field::dstStageMask), "VUID-vkCmdWaitEvents-dstStageMask-04092"},
         {Key(Func::vkCmdWaitEvents, Field::srcStageMask), "VUID-vkCmdWaitEvents-srcStageMask-04092"},
         {Key(Func::vkCmdWriteTimestamp2, Field::stage), "VUID-vkCmdWriteTimestamp2-stage-03931"},
         {Key(Func::vkCmdWriteTimestamp, Field::pipelineStage), "VUID-vkCmdWriteTimestamp-pipelineStage-04077"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstStageMask), "VUID-VkImageMemoryBarrier2-dstStageMask-03931"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcStageMask), "VUID-VkImageMemoryBarrier2-srcStageMask-03931"},
         {Key(Struct::VkMemoryBarrier2, Field::dstStageMask), "VUID-VkMemoryBarrier2-dstStageMask-03931"},
         {Key(Struct::VkMemoryBarrier2, Field::srcStageMask), "VUID-VkMemoryBarrier2-srcStageMask-03931"},
         {Key(Struct::VkSemaphoreSubmitInfo, Field::stageMask), "VUID-VkSemaphoreSubmitInfo-stageMask-03931"},
         {Key(Struct::VkSubmitInfo, Field::pWaitDstStageMask), "VUID-VkSubmitInfo-pWaitDstStageMask-04092"},
         {Key(Struct::VkSubpassDependency, Field::srcStageMask), "VUID-VkSubpassDependency-srcStageMask-04092"},
         {Key(Struct::VkSubpassDependency, Field::dstStageMask), "VUID-VkSubpassDependency-dstStageMask-04092"},
         {Key(Struct::VkSubpassDependency2, Field::srcStageMask), "VUID-VkSubpassDependency2-srcStageMask-04092"},
         {Key(Struct::VkSubpassDependency2, Field::dstStageMask), "VUID-VkSubpassDependency2-dstStageMask-04092"},
     }},
    {VK_PIPELINE_STAGE_2_FRAGMENT_DENSITY_PROCESS_BIT_EXT,
     {
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstStageMask), "VUID-VkBufferMemoryBarrier2-dstStageMask-03932"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcStageMask), "VUID-VkBufferMemoryBarrier2-srcStageMask-03932"},
         {Key(Func::vkCmdPipelineBarrier, Field::dstStageMask), "VUID-vkCmdPipelineBarrier-dstStageMask-04093"},
         {Key(Func::vkCmdPipelineBarrier, Field::srcStageMask), "VUID-vkCmdPipelineBarrier-srcStageMask-04093"},
         {Key(Func::vkCmdResetEvent2, Field::stageMask), "VUID-vkCmdResetEvent2-stageMask-03932"},
         {Key(Func::vkCmdResetEvent, Field::stageMask), "VUID-vkCmdResetEvent-stageMask-04093"},
         {Key(Func::vkCmdSetEvent, Field::stageMask), "VUID-vkCmdSetEvent-stageMask-04093"},
         {Key(Func::vkCmdWaitEvents, Field::dstStageMask), "VUID-vkCmdWaitEvents-dstStageMask-04093"},
         {Key(Func::vkCmdWaitEvents, Field::srcStageMask), "VUID-vkCmdWaitEvents-srcStageMask-04093"},
         {Key(Func::vkCmdWriteTimestamp2, Field::stage), "VUID-vkCmdWriteTimestamp2-stage-03932"},
         {Key(Func::vkCmdWriteTimestamp, Field::pipelineStage), "VUID-vkCmdWriteTimestamp-pipelineStage-04078"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstStageMask), "VUID-VkImageMemoryBarrier2-dstStageMask-03932"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcStageMask), "VUID-VkImageMemoryBarrier2-srcStageMask-03932"},
         {Key(Struct::VkMemoryBarrier2, Field::dstStageMask), "VUID-VkMemoryBarrier2-dstStageMask-03932"},
         {Key(Struct::VkMemoryBarrier2, Field::srcStageMask), "VUID-VkMemoryBarrier2-srcStageMask-03932"},
         {Key(Struct::VkSemaphoreSubmitInfo, Field::stageMask), "VUID-VkSemaphoreSubmitInfo-stageMask-03932"},
         {Key(Struct::VkSubmitInfo, Field::pWaitDstStageMask), "VUID-VkSubmitInfo-pWaitDstStageMask-04093"},
         {Key(Struct::VkSubpassDependency, Field::srcStageMask), "VUID-VkSubpassDependency-srcStageMask-04093"},
         {Key(Struct::VkSubpassDependency, Field::dstStageMask), "VUID-VkSubpassDependency-dstStageMask-04093"},
         {Key(Struct::VkSubpassDependency2, Field::srcStageMask), "VUID-VkSubpassDependency2-srcStageMask-04093"},
         {Key(Struct::VkSubpassDependency2, Field::dstStageMask), "VUID-VkSubpassDependency2-dstStageMask-04093"},
     }},
    {VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
     {
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstStageMask), "VUID-VkBufferMemoryBarrier2-dstStageMask-04956"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcStageMask), "VUID-VkBufferMemoryBarrier2-srcStageMask-04956"},
         {Key(Func::vkCmdPipelineBarrier, Field::dstStageMask), "VUID-vkCmdPipelineBarrier-dstStageMask-04097"},
         {Key(Func::vkCmdPipelineBarrier, Field::srcStageMask), "VUID-vkCmdPipelineBarrier-srcStageMask-04097"},
         {Key(Func::vkCmdResetEvent2, Field::stageMask), "VUID-vkCmdResetEvent2-stageMask-04956"},
         {Key(Func::vkCmdResetEvent, Field::stageMask), "VUID-vkCmdResetEvent-stageMask-04097"},
         {Key(Func::vkCmdSetEvent, Field::stageMask), "VUID-vkCmdSetEvent-stageMask-04097"},
         {Key(Func::vkCmdWaitEvents, Field::dstStageMask), "VUID-vkCmdWaitEvents-dstStageMask-04097"},
         {Key(Func::vkCmdWaitEvents, Field::srcStageMask), "VUID-vkCmdWaitEvents-srcStageMask-04097"},
         {Key(Func::vkCmdWriteTimestamp2, Field::stage), "VUID-vkCmdWriteTimestamp2-stage-04956"},
         {Key(Func::vkCmdWriteTimestamp, Field::pipelineStage), "VUID-vkCmdWriteTimestamp-pipelineStage-04081"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstStageMask), "VUID-VkImageMemoryBarrier2-dstStageMask-04956"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcStageMask), "VUID-VkImageMemoryBarrier2-srcStageMask-04956"},
         {Key(Struct::VkMemoryBarrier2, Field::dstStageMask), "VUID-VkMemoryBarrier2-dstStageMask-04956"},
         {Key(Struct::VkMemoryBarrier2, Field::srcStageMask), "VUID-VkMemoryBarrier2-srcStageMask-04956"},
         {Key(Struct::VkSemaphoreSubmitInfo, Field::stageMask), "VUID-VkSemaphoreSubmitInfo-stageMask-04956"},
         {Key(Struct::VkSubmitInfo, Field::pWaitDstStageMask), "VUID-VkSubmitInfo-pWaitDstStageMask-04097"},
         {Key(Struct::VkSubpassDependency, Field::srcStageMask), "VUID-VkSubpassDependency-srcStageMask-04097"},
         {Key(Struct::VkSubpassDependency, Field::dstStageMask), "VUID-VkSubpassDependency-dstStageMask-04097"},
         {Key(Struct::VkSubpassDependency2, Field::srcStageMask), "VUID-VkSubpassDependency2-srcStageMask-04097"},
         {Key(Struct::VkSubpassDependency2, Field::dstStageMask), "VUID-VkSubpassDependency2-dstStageMask-04097"},
     }},
    {VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT_KHR,
     {
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstStageMask), "VUID-VkBufferMemoryBarrier2-dstStageMask-03929"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcStageMask), "VUID-VkBufferMemoryBarrier2-srcStageMask-03929"},
         {Key(Func::vkCmdPipelineBarrier, Field::dstStageMask), "VUID-vkCmdPipelineBarrier-dstStageMask-04090"},
         {Key(Func::vkCmdPipelineBarrier, Field::srcStageMask), "VUID-vkCmdPipelineBarrier-srcStageMask-04090"},
         {Key(Func::vkCmdResetEvent2, Field::stageMask), "VUID-vkCmdResetEvent2-stageMask-03929"},
         {Key(Func::vkCmdResetEvent, Field::stageMask), "VUID-vkCmdResetEvent-stageMask-04090"},
         {Key(Func::vkCmdSetEvent, Field::stageMask), "VUID-vkCmdSetEvent-stageMask-04090"},
         {Key(Func::vkCmdWaitEvents, Field::dstStageMask), "VUID-vkCmdWaitEvents-dstStageMask-04090"},
         {Key(Func::vkCmdWaitEvents, Field::srcStageMask), "VUID-vkCmdWaitEvents-srcStageMask-04090"},
         {Key(Func::vkCmdWriteTimestamp2, Field::stage), "VUID-vkCmdWriteTimestamp2-stage-03929"},
         {Key(Func::vkCmdWriteTimestamp, Field::pipelineStage), "VUID-vkCmdWriteTimestamp-pipelineStage-04075"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstStageMask), "VUID-VkImageMemoryBarrier2-dstStageMask-03929"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcStageMask), "VUID-VkImageMemoryBarrier2-srcStageMask-03929"},
         {Key(Struct::VkMemoryBarrier2, Field::dstStageMask), "VUID-VkMemoryBarrier2-dstStageMask-03929"},
         {Key(Struct::VkMemoryBarrier2, Field::srcStageMask), "VUID-VkMemoryBarrier2-srcStageMask-03929"},
         {Key(Struct::VkSemaphoreSubmitInfo, Field::stageMask), "VUID-VkSemaphoreSubmitInfo-stageMask-03929"},
         {Key(Struct::VkSubmitInfo, Field::pWaitDstStageMask), "VUID-VkSubmitInfo-pWaitDstStageMask-04090"},
         {Key(Struct::VkSubpassDependency, Field::srcStageMask), "VUID-VkSubpassDependency-srcStageMask-04090"},
         {Key(Struct::VkSubpassDependency, Field::dstStageMask), "VUID-VkSubpassDependency-dstStageMask-04090"},
         {Key(Struct::VkSubpassDependency2, Field::srcStageMask), "VUID-VkSubpassDependency2-srcStageMask-04090"},
         {Key(Struct::VkSubpassDependency2, Field::dstStageMask), "VUID-VkSubpassDependency2-dstStageMask-04090"},
     }},
    {VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV,
     {
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstStageMask), "VUID-VkBufferMemoryBarrier2-dstStageMask-03934"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcStageMask), "VUID-VkBufferMemoryBarrier2-srcStageMask-03934"},
         {Key(Func::vkCmdPipelineBarrier, Field::dstStageMask), "VUID-vkCmdPipelineBarrier-dstStageMask-04095"},
         {Key(Func::vkCmdPipelineBarrier, Field::srcStageMask), "VUID-vkCmdPipelineBarrier-srcStageMask-04095"},
         {Key(Func::vkCmdResetEvent2, Field::stageMask), "VUID-vkCmdResetEvent2-stageMask-03934"},
         {Key(Func::vkCmdResetEvent, Field::stageMask), "VUID-vkCmdResetEvent-stageMask-04095"},
         {Key(Func::vkCmdSetEvent, Field::stageMask), "VUID-vkCmdSetEvent-stageMask-04095"},
         {Key(Func::vkCmdWaitEvents, Field::dstStageMask), "VUID-vkCmdWaitEvents-dstStageMask-04095"},
         {Key(Func::vkCmdWaitEvents, Field::srcStageMask), "VUID-vkCmdWaitEvents-srcStageMask-04095"},
         {Key(Func::vkCmdWriteTimestamp2, Field::stage), "VUID-vkCmdWriteTimestamp2-stage-03934"},
         {Key(Func::vkCmdWriteTimestamp, Field::pipelineStage), "VUID-vkCmdWriteTimestamp-pipelineStage-04080"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstStageMask), "VUID-VkImageMemoryBarrier2-dstStageMask-03934"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcStageMask), "VUID-VkImageMemoryBarrier2-srcStageMask-03934"},
         {Key(Struct::VkMemoryBarrier2, Field::dstStageMask), "VUID-VkMemoryBarrier2-dstStageMask-03934"},
         {Key(Struct::VkMemoryBarrier2, Field::srcStageMask), "VUID-VkMemoryBarrier2-srcStageMask-03934"},
         {Key(Struct::VkSemaphoreSubmitInfo, Field::stageMask), "VUID-VkSemaphoreSubmitInfo-stageMask-03934"},
         {Key(Struct::VkSubmitInfo, Field::pWaitDstStageMask), "VUID-VkSubmitInfo-pWaitDstStageMask-04095"},
         {Key(Struct::VkSubpassDependency, Field::srcStageMask), "VUID-VkSubpassDependency-srcStageMask-04095"},
         {Key(Struct::VkSubpassDependency, Field::dstStageMask), "VUID-VkSubpassDependency-dstStageMask-04095"},
         {Key(Struct::VkSubpassDependency2, Field::srcStageMask), "VUID-VkSubpassDependency2-srcStageMask-04095"},
         {Key(Struct::VkSubpassDependency2, Field::dstStageMask), "VUID-VkSubpassDependency2-dstStageMask-04095"},
     }},
    {VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV,
     {
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstStageMask), "VUID-VkBufferMemoryBarrier2-dstStageMask-03935"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcStageMask), "VUID-VkBufferMemoryBarrier2-srcStageMask-03935"},
         {Key(Func::vkCmdPipelineBarrier, Field::dstStageMask), "VUID-vkCmdPipelineBarrier-dstStageMask-04096"},
         {Key(Func::vkCmdPipelineBarrier, Field::srcStageMask), "VUID-vkCmdPipelineBarrier-srcStageMask-04096"},
         {Key(Func::vkCmdResetEvent2, Field::stageMask), "VUID-vkCmdResetEvent2-stageMask-03935"},
         {Key(Func::vkCmdResetEvent, Field::stageMask), "VUID-vkCmdResetEvent-stageMask-04096"},
         {Key(Func::vkCmdSetEvent, Field::stageMask), "VUID-vkCmdSetEvent-stageMask-04096"},
         {Key(Func::vkCmdWaitEvents, Field::dstStageMask), "VUID-vkCmdWaitEvents-dstStageMask-04096"},
         {Key(Func::vkCmdWaitEvents, Field::srcStageMask), "VUID-vkCmdWaitEvents-srcStageMask-04096"},
         {Key(Func::vkCmdWriteTimestamp2, Field::stage), "VUID-vkCmdWriteTimestamp2-stage-03935"},
         {Key(Func::vkCmdWriteTimestamp, Field::pipelineStage), "VUID-vkCmdWriteTimestamp-pipelineStage-04080"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstStageMask), "VUID-VkImageMemoryBarrier2-dstStageMask-03935"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcStageMask), "VUID-VkImageMemoryBarrier2-srcStageMask-03935"},
         {Key(Struct::VkMemoryBarrier2, Field::dstStageMask), "VUID-VkMemoryBarrier2-dstStageMask-03935"},
         {Key(Struct::VkMemoryBarrier2, Field::srcStageMask), "VUID-VkMemoryBarrier2-srcStageMask-03935"},
         {Key(Struct::VkSemaphoreSubmitInfo, Field::stageMask), "VUID-VkSemaphoreSubmitInfo-stageMask-03935"},
         {Key(Struct::VkSubmitInfo, Field::pWaitDstStageMask), "VUID-VkSubmitInfo-pWaitDstStageMask-04096"},
         {Key(Struct::VkSubpassDependency, Field::srcStageMask), "VUID-VkSubpassDependency-srcStageMask-04096"},
         {Key(Struct::VkSubpassDependency, Field::dstStageMask), "VUID-VkSubpassDependency-dstStageMask-04096"},
         {Key(Struct::VkSubpassDependency2, Field::srcStageMask), "VUID-VkSubpassDependency2-srcStageMask-04096"},
         {Key(Struct::VkSubpassDependency2, Field::dstStageMask), "VUID-VkSubpassDependency2-dstStageMask-04096"},
     }},
    {VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT_KHR,
     {
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstStageMask), "VUID-VkBufferMemoryBarrier2-dstStageMask-03930"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcStageMask), "VUID-VkBufferMemoryBarrier2-srcStageMask-03930"},
         {Key(Func::vkCmdPipelineBarrier, Field::dstStageMask), "VUID-vkCmdPipelineBarrier-dstStageMask-04091"},
         {Key(Func::vkCmdPipelineBarrier, Field::srcStageMask), "VUID-vkCmdPipelineBarrier-srcStageMask-04091"},
         {Key(Func::vkCmdResetEvent2, Field::stageMask), "VUID-vkCmdResetEvent2-stageMask-03930"},
         {Key(Func::vkCmdResetEvent, Field::stageMask), "VUID-vkCmdResetEvent-stageMask-04091"},
         {Key(Func::vkCmdSetEvent, Field::stageMask), "VUID-vkCmdSetEvent-stageMask-04091"},
         {Key(Func::vkCmdWaitEvents, Field::dstStageMask), "VUID-vkCmdWaitEvents-dstStageMask-04091"},
         {Key(Func::vkCmdWaitEvents, Field::srcStageMask), "VUID-vkCmdWaitEvents-srcStageMask-04091"},
         {Key(Func::vkCmdWriteTimestamp2, Field::stage), "VUID-vkCmdWriteTimestamp2-stage-03930"},
         {Key(Func::vkCmdWriteTimestamp, Field::pipelineStage), "VUID-vkCmdWriteTimestamp-pipelineStage-04076"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstStageMask), "VUID-VkImageMemoryBarrier2-dstStageMask-03930"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcStageMask), "VUID-VkImageMemoryBarrier2-srcStageMask-03930"},
         {Key(Struct::VkMemoryBarrier2, Field::dstStageMask), "VUID-VkMemoryBarrier2-dstStageMask-03930"},
         {Key(Struct::VkMemoryBarrier2, Field::srcStageMask), "VUID-VkMemoryBarrier2-srcStageMask-03930"},
         {Key(Struct::VkSemaphoreSubmitInfo, Field::stageMask), "VUID-VkSemaphoreSubmitInfo-stageMask-03930"},
         {Key(Struct::VkSubmitInfo, Field::pWaitDstStageMask), "VUID-VkSubmitInfo-pWaitDstStageMask-04091"},
         {Key(Struct::VkSubpassDependency, Field::srcStageMask), "VUID-VkSubpassDependency-srcStageMask-04091"},
         {Key(Struct::VkSubpassDependency, Field::dstStageMask), "VUID-VkSubpassDependency-dstStageMask-04091"},
         {Key(Struct::VkSubpassDependency2, Field::srcStageMask), "VUID-VkSubpassDependency2-srcStageMask-04091"},
         {Key(Struct::VkSubpassDependency2, Field::dstStageMask), "VUID-VkSubpassDependency2-dstStageMask-04091"},
     }},
    {
        VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT_KHR,
        {
            {Key(Struct::VkBufferMemoryBarrier2, Field::dstStageMask), "VUID-VkBufferMemoryBarrier2-dstStageMask-03930"},
            {Key(Struct::VkBufferMemoryBarrier2, Field::srcStageMask), "VUID-VkBufferMemoryBarrier2-srcStageMask-03930"},
            {Key(Func::vkCmdPipelineBarrier, Field::dstStageMask), "VUID-vkCmdPipelineBarrier-dstStageMask-04091"},
            {Key(Func::vkCmdPipelineBarrier, Field::srcStageMask), "VUID-vkCmdPipelineBarrier-srcStageMask-04091"},
            {Key(Func::vkCmdResetEvent2, Field::stageMask), "VUID-vkCmdResetEvent2-stageMask-03930"},
            {Key(Func::vkCmdResetEvent, Field::stageMask), "VUID-vkCmdResetEvent-stageMask-04091"},
            {Key(Func::vkCmdSetEvent, Field::stageMask), "VUID-vkCmdSetEvent-stageMask-04091"},
            {Key(Func::vkCmdWaitEvents, Field::dstStageMask), "VUID-vkCmdWaitEvents-dstStageMask-04091"},
            {Key(Func::vkCmdWaitEvents, Field::srcStageMask), "VUID-vkCmdWaitEvents-srcStageMask-04091"},
            {Key(Func::vkCmdWriteTimestamp2, Field::stage), "VUID-vkCmdWriteTimestamp2-stage-03930"},
            {Key(Func::vkCmdWriteTimestamp, Field::pipelineStage), "VUID-vkCmdWriteTimestamp-pipelineStage-04076"},
            {Key(Struct::VkImageMemoryBarrier2, Field::dstStageMask), "VUID-VkImageMemoryBarrier2-dstStageMask-03930"},
            {Key(Struct::VkImageMemoryBarrier2, Field::srcStageMask), "VUID-VkImageMemoryBarrier2-srcStageMask-03930"},
            {Key(Struct::VkMemoryBarrier2, Field::dstStageMask), "VUID-VkMemoryBarrier2-dstStageMask-03930"},
            {Key(Struct::VkMemoryBarrier2, Field::srcStageMask), "VUID-VkMemoryBarrier2-srcStageMask-03930"},
            {Key(Struct::VkSemaphoreSubmitInfo, Field::stageMask), "VUID-VkSemaphoreSubmitInfo-stageMask-03930"},
            {Key(Struct::VkSubmitInfo, Field::pWaitDstStageMask), "VUID-VkSubmitInfo-pWaitDstStageMask-04091"},
            {Key(Struct::VkSubpassDependency, Field::srcStageMask), "VUID-VkSubpassDependency-srcStageMask-04091"},
            {Key(Struct::VkSubpassDependency, Field::dstStageMask), "VUID-VkSubpassDependency-dstStageMask-04091"},
            {Key(Struct::VkSubpassDependency2, Field::srcStageMask), "VUID-VkSubpassDependency2-srcStageMask-04091"},
            {Key(Struct::VkSubpassDependency2, Field::dstStageMask), "VUID-VkSubpassDependency2-dstStageMask-04091"},
        },
    },
    {VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT,
     {
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstStageMask), "VUID-VkBufferMemoryBarrier2-dstStageMask-03933"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcStageMask), "VUID-VkBufferMemoryBarrier2-srcStageMask-03933"},
         {Key(Func::vkCmdPipelineBarrier, Field::dstStageMask), "VUID-vkCmdPipelineBarrier-dstStageMask-04094"},
         {Key(Func::vkCmdPipelineBarrier, Field::srcStageMask), "VUID-vkCmdPipelineBarrier-srcStageMask-04094"},
         {Key(Func::vkCmdResetEvent2, Field::stageMask), "VUID-vkCmdResetEvent2-stageMask-03933"},
         {Key(Func::vkCmdResetEvent, Field::stageMask), "VUID-vkCmdResetEvent-stageMask-04094"},
         {Key(Func::vkCmdSetEvent, Field::stageMask), "VUID-vkCmdSetEvent-stageMask-04094"},
         {Key(Func::vkCmdWaitEvents, Field::dstStageMask), "VUID-vkCmdWaitEvents-dstStageMask-04094"},
         {Key(Func::vkCmdWaitEvents, Field::srcStageMask), "VUID-vkCmdWaitEvents-srcStageMask-04094"},
         {Key(Func::vkCmdWriteTimestamp2, Field::stage), "VUID-vkCmdWriteTimestamp2-stage-03933"},
         {Key(Func::vkCmdWriteTimestamp, Field::pipelineStage), "VUID-vkCmdWriteTimestamp-pipelineStage-04079"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstStageMask), "VUID-VkImageMemoryBarrier2-dstStageMask-03933"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcStageMask), "VUID-VkImageMemoryBarrier2-srcStageMask-03933"},
         {Key(Struct::VkMemoryBarrier2, Field::dstStageMask), "VUID-VkMemoryBarrier2-dstStageMask-03933"},
         {Key(Struct::VkMemoryBarrier2, Field::srcStageMask), "VUID-VkMemoryBarrier2-srcStageMask-03933"},
         {Key(Struct::VkSemaphoreSubmitInfo, Field::stageMask), "VUID-VkSemaphoreSubmitInfo-stageMask-03933"},
         {Key(Struct::VkSubmitInfo, Field::pWaitDstStageMask), "VUID-VkSubmitInfo-pWaitDstStageMask-04094"},
         {Key(Struct::VkSubpassDependency, Field::srcStageMask), "VUID-VkSubpassDependency-srcStageMask-04094"},
         {Key(Struct::VkSubpassDependency, Field::dstStageMask), "VUID-VkSubpassDependency-dstStageMask-04094"},
         {Key(Struct::VkSubpassDependency2, Field::srcStageMask), "VUID-VkSubpassDependency2-srcStageMask-04094"},
         {Key(Struct::VkSubpassDependency2, Field::dstStageMask), "VUID-VkSubpassDependency2-dstStageMask-04094"},
     }},
    // special case for the NONE stage. This entry omits the synchronization2
    // commands because they shouldn't be called unless synchronization2 is enabled.
    {VK_PIPELINE_STAGE_2_NONE_KHR,
     {
         {Key(Func::vkCmdPipelineBarrier, Field::srcStageMask), "VUID-vkCmdPipelineBarrier-srcStageMask-03937"},
         {Key(Func::vkCmdPipelineBarrier, Field::dstStageMask), "VUID-vkCmdPipelineBarrier-dstStageMask-03937"},
         {Key(Func::vkCmdResetEvent, Field::stageMask), "VUID-vkCmdResetEvent-stageMask-03937"},
         {Key(Func::vkCmdSetEvent, Field::stageMask), "VUID-vkCmdSetEvent-stageMask-03937"},
         {Key(Func::vkCmdWaitEvents, Field::srcStageMask), "VUID-vkCmdWaitEvents-srcStageMask-03937"},
         {Key(Func::vkCmdWaitEvents, Field::dstStageMask), "VUID-vkCmdWaitEvents-dstStageMask-03937"},
         {Key(Func::vkCmdWriteTimestamp, Field::pipelineStage), "VUID-vkCmdWriteTimestamp-pipelineStage-04074"},
         {Key(Struct::VkSubmitInfo, Field::pWaitDstStageMask), "VUID-VkSubmitInfo-pWaitDstStageMask-03937"},
         {Key(Struct::VkSubpassDependency, Field::srcStageMask), "VUID-VkSubpassDependency-srcStageMask-03937"},
         {Key(Struct::VkSubpassDependency, Field::dstStageMask), "VUID-VkSubpassDependency-dstStageMask-03937"},
         {Key(Struct::VkSubpassDependency2, Field::srcStageMask), "VUID-VkSubpassDependency2-srcStageMask-03937"},
         {Key(Struct::VkSubpassDependency2, Field::dstStageMask), "VUID-VkSubpassDependency2-dstStageMask-03937"},
     }},
};

const std::string &GetBadFeatureVUID(const Location &loc, VkPipelineStageFlags2KHR bit) {
    const auto &result = FindVUID(bit, loc, kStageMaskErrors);
    assert(!result.empty());
    if (result.empty()) {
        static const std::string unhandled("UNASSIGNED-CoreChecks-unhandle-pipeline-stage-feature");
        return unhandled;
    }
    return result;
}

// commonvalidity/access_mask_2_common.txt
static const std::map<VkAccessFlags2KHR, std::array<Entry, 6>> kAccessMask2Common{
    {VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT_KHR,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03900"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03900"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03900"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03900"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03900"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03900"},
     }}},
    {VK_ACCESS_2_INDEX_READ_BIT_KHR,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03901"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03901"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03901"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03901"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03901"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03901"},
     }}},
    {VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT_KHR,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03902"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03902"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03902"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03902"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03902"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03902"},
     }}},
    {VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT_KHR,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03903"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03903"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03903"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03903"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03903"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03903"},
     }}},
    {VK_ACCESS_2_UNIFORM_READ_BIT_KHR,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03904"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03904"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03904"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03904"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03904"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03904"},
     }}},
    {VK_ACCESS_2_SHADER_SAMPLED_READ_BIT_KHR,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03905"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03905"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03905"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03905"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03905"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03905"},
     }}},
    {VK_ACCESS_2_SHADER_STORAGE_READ_BIT_KHR,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03906"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03906"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03906"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03906"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03906"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03906"},
     }}},
    {VK_ACCESS_2_SHADER_BINDING_TABLE_READ_BIT_KHR,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "UNASSIGNED-VkMemoryBarrier2-srcAccessMask-SHADER_BINDING_TABLE_READ"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "UNASSIGNED-VkMemoryBarrier2-dstAccessMask-SHADER_BINDING_TABLE_READ"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "UNASSIGNED-VkBufferMemoryBarrier2-srcAccessMask-SHADER_BINDING_TABLE_READ"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "UNASSIGNED-VkBufferMemoryBarrier2-dstAccessMask-SHADER_BINDING_TABLE_READ"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "UNASSIGNED-VkImageMemoryBarrier2-srcAccessMask-SHADER_BINDING_TABLE_READ"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "UNASSIGNED-VkImageMemoryBarrier2-dstAccessMask-SHADER_BINDING_TABLE_READ"},
     }}},
    {VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT_KHR,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03907"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03907"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03907"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03907"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03907"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03907"},
     }}},
    {VK_ACCESS_2_SHADER_READ_BIT_KHR,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03908"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03908"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03908"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03908"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03908"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03908"},
     }}},
    {VK_ACCESS_2_SHADER_WRITE_BIT_KHR,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03909"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03909"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03909"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03909"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03909"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03909"},
     }}},
    {VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT_KHR,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03910"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03910"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03910"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03910"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03910"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03910"},
     }}},
    {VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03911"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03911"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03911"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03911"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03911"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03911"},
     }}},
    {VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT_KHR,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03912"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03912"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03912"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03912"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03912"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03912"},
     }}},
    {VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT_KHR,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03913"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03913"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03913"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03913"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03913"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03913"},
     }}},
    {VK_ACCESS_2_TRANSFER_READ_BIT_KHR,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03914"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03914"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03914"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03914"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03914"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03914"},
     }}},
    {VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03915"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03915"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03915"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03915"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03915"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03915"},
     }}},
    {VK_ACCESS_2_HOST_READ_BIT_KHR,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03916"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03916"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03916"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03916"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03916"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03916"},
     }}},
    {VK_ACCESS_2_HOST_WRITE_BIT_KHR,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03917"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03917"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03917"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03917"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03917"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03917"},
     }}},
    {VK_ACCESS_2_CONDITIONAL_RENDERING_READ_BIT_EXT,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03918"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03918"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03918"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03918"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03918"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03918"},
     }}},
    {VK_ACCESS_2_FRAGMENT_DENSITY_MAP_READ_BIT_EXT,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03919"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03919"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03919"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03919"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03919"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03919"},
     }}},
    {VK_ACCESS_2_TRANSFORM_FEEDBACK_WRITE_BIT_EXT,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03920"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03920"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03920"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03920"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03920"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03920"},
     }}},
    {VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-04747"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-04747"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-04747"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-04747"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-04747"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-04747"},
     }}},
    {VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03920"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03920"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03920"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03920"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03920"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03920"},
     }}},
    {VK_ACCESS_2_SHADING_RATE_IMAGE_READ_BIT_NV,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03922"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03922"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03922"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03922"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03922"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03922"},
     }}},
    {VK_ACCESS_2_COMMAND_PREPROCESS_READ_BIT_NV,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03923"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03923"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03923"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03923"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03923"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03923"},
     }}},
    {VK_ACCESS_2_COMMAND_PREPROCESS_WRITE_BIT_NV,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03924"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03924"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03924"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03924"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03924"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03924"},
     }}},
    {VK_PIPELINE_STAGE_2_COMMAND_PREPROCESS_BIT_NV,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03925"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03925"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03925"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03925"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03925"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03925"},
     }}},
    {VK_ACCESS_2_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03926"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03926"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03926"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03926"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03926"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03926"},
     }}},
    {VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03927"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03927"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03927"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03927"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03927"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03927"},
     }}},
    {VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-03928"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-03928"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-03928"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-03928"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-03928"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-03928"},
     }}},
    {VK_ACCESS_2_VIDEO_DECODE_READ_BIT_KHR,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-04858"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-04858"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-04858"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-04858"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-04858"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-04858"},
     }}},
    {VK_ACCESS_2_VIDEO_DECODE_WRITE_BIT_KHR,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-04859"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-04859"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-04859"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-04859"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-04859"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-04859"},
     }}},
    {VK_ACCESS_2_VIDEO_ENCODE_READ_BIT_KHR,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-04860"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-04860"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-04860"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-04860"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-04860"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-04860"},
     }}},
    {VK_ACCESS_2_VIDEO_ENCODE_WRITE_BIT_KHR,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-04861"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-04861"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-04861"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-04861"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-04861"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-04861"},
     }}},
    {VK_ACCESS_2_INVOCATION_MASK_READ_BIT_HUAWEI,
     {{
         {Key(Struct::VkMemoryBarrier2, Field::srcAccessMask), "VUID-VkMemoryBarrier2-srcAccessMask-04994"},
         {Key(Struct::VkMemoryBarrier2, Field::dstAccessMask), "VUID-VkMemoryBarrier2-dstAccessMask-04994"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::srcAccessMask), "VUID-VkBufferMemoryBarrier2-srcAccessMask-04994"},
         {Key(Struct::VkBufferMemoryBarrier2, Field::dstAccessMask), "VUID-VkBufferMemoryBarrier2-dstAccessMask-04994"},
         {Key(Struct::VkImageMemoryBarrier2, Field::srcAccessMask), "VUID-VkImageMemoryBarrier2-srcAccessMask-04994"},
         {Key(Struct::VkImageMemoryBarrier2, Field::dstAccessMask), "VUID-VkImageMemoryBarrier2-dstAccessMask-04994"},
     }}},
};

// commonvalidity/fine_sync_commands_common.txt
static const std::vector<Entry> kFineSyncCommon = {
    {Key(Func::vkCmdPipelineBarrier, Field::srcAccessMask), "VUID-vkCmdPipelineBarrier-srcAccessMask-02815"},
    {Key(Func::vkCmdPipelineBarrier, Field::dstAccessMask), "VUID-vkCmdPipelineBarrier-dstAccessMask-02816"},
    {Key(Func::vkCmdWaitEvents, Field::srcAccessMask), "VUID-vkCmdWaitEvents-srcAccessMask-02815"},
    {Key(Func::vkCmdWaitEvents, Field::dstAccessMask), "VUID-vkCmdWaitEvents-dstAccessMask-02816"},
    {Key(Struct::VkSubpassDependency, Field::srcAccessMask), "VUID-VkSubpassDependency-srcAccessMask-00868"},
    {Key(Struct::VkSubpassDependency, Field::dstAccessMask), "VUID-VkSubpassDependency-dstAccessMask-00869"},
    {Key(Struct::VkSubpassDependency2, Field::srcAccessMask), "VUID-VkSubpassDependency2-srcAccessMask-03088"},
    {Key(Struct::VkSubpassDependency2, Field::dstAccessMask), "VUID-VkSubpassDependency2-dstAccessMask-03089"},
};
const std::string &GetBadAccessFlagsVUID(const Location &loc, VkAccessFlags2KHR bit) {
    const auto &result = FindVUID(bit, loc, kAccessMask2Common);
    if (!result.empty()) {
        return result;
    }
    const auto &result2 = FindVUID(loc, kFineSyncCommon);
    assert(!result2.empty());
    if (result2.empty()) {
        static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-bad-access-flags");
        return unhandled;
    }
    return result2;
}

static const std::vector<Entry> kQueueCapErrors{
    {Key(Struct::VkSubmitInfo, Field::pWaitDstStageMask), "VUID-vkQueueSubmit-pWaitDstStageMask-00066"},
    {Key(Struct::VkSubpassDependency, Field::srcStageMask), "VUID-vkCmdBeginRenderPass-srcStageMask-06451"},
    {Key(Struct::VkSubpassDependency, Field::dstStageMask), "VUID-vkCmdBeginRenderPass-dstStageMask-06452"},
    {Key(Func::vkCmdSetEvent, Field::stageMask), "VUID-vkCmdSetEvent-stageMask-06457"},
    {Key(Func::vkCmdResetEvent, Field::stageMask), "VUID-vkCmdResetEvent-stageMask-06458"},
    {Key(Func::vkCmdWaitEvents, Field::srcStageMask), "VUID-vkCmdWaitEvents-srcStageMask-06459"},
    {Key(Func::vkCmdWaitEvents, Field::dstStageMask), "VUID-vkCmdWaitEvents-dstStageMask-06460"},
    {Key(Func::vkCmdPipelineBarrier, Field::srcStageMask), "VUID-vkCmdPipelineBarrier-srcStageMask-06461"},
    {Key(Func::vkCmdPipelineBarrier, Field::dstStageMask), "VUID-vkCmdPipelineBarrier-dstStageMask-06462"},
    {Key(Func::vkCmdWriteTimestamp, Field::pipelineStage), "VUID-vkCmdWriteTimestamp-pipelineStage-04074"},
    {Key(Struct::VkSubpassDependency2, Field::srcStageMask), "VUID-vkCmdBeginRenderPass2-srcStageMask-06453"},
    {Key(Struct::VkSubpassDependency2, Field::dstStageMask), "VUID-vkCmdBeginRenderPass2-dstStageMask-06454"},
    {Key(Func::vkCmdSetEvent, Field::srcStageMask), "VUID-vkCmdSetEvent2-srcStageMask-03827"},
    {Key(Func::vkCmdSetEvent, Field::dstStageMask), "VUID-vkCmdSetEvent2-dstStageMask-03828"},
    {Key(Func::vkCmdPipelineBarrier2, Field::srcStageMask), "VUID-vkCmdPipelineBarrier2-srcStageMask-03849"},
    {Key(Func::vkCmdPipelineBarrier2, Field::dstStageMask), "VUID-vkCmdPipelineBarrier2-dstStageMask-03850"},
    {Key(Func::vkCmdWaitEvents2, Field::srcStageMask), "VUID-vkCmdWaitEvents2-srcStageMask-03842"},
    {Key(Func::vkCmdWaitEvents2, Field::dstStageMask), "VUID-vkCmdWaitEvents2-dstStageMask-03843"},
    {Key(Func::vkCmdWriteTimestamp2, Field::stage), "VUID-vkCmdWriteTimestamp2-stage-03860"},
    {Key(Func::vkQueueSubmit2, Field::pSignalSemaphoreInfos, true), "VUID-vkQueueSubmit2-stageMask-03869"},
    {Key(Func::vkQueueSubmit2, Field::pWaitSemaphoreInfos, true), "VUID-vkQueueSubmit2-stageMask-03870"},
};

const std::string &GetStageQueueCapVUID(const Location &loc, VkPipelineStageFlags2KHR bit) {
    // no per-bit lookups needed
    const auto &result = FindVUID(loc, kQueueCapErrors);
    if (result.empty()) {
        static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-queue-capabilities");
        return unhandled;
    }
    return result;
}

static const std::map<QueueError, std::vector<Entry>> kBarrierQueueErrors{
    {QueueError::kSrcOrDstMustBeIgnore,
     {
         // this isn't an error for synchronization2, so we don't need the 2KHR versions
         {Key(Struct::VkBufferMemoryBarrier), "VUID-VkBufferMemoryBarrier-synchronization2-03853"},
         {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-synchronization2-03857"},
     }},

    {QueueError::kSpecialOrIgnoreOnly,
     {
         {Key(Struct::VkBufferMemoryBarrier2), "VUID-VkBufferMemoryBarrier2-buffer-04088"},
         {Key(Struct::VkBufferMemoryBarrier), "VUID-VkBufferMemoryBarrier-buffer-04088"},
         {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-image-04071"},
         {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-image-04071"},
     }},
    {QueueError::kSrcAndDstValidOrSpecial,
     {
         {Key(Struct::VkBufferMemoryBarrier2), "VUID-VkBufferMemoryBarrier2-buffer-04089"},
         {Key(Struct::VkBufferMemoryBarrier), "VUID-VkBufferMemoryBarrier-buffer-04089"},
         {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-image-04072"},
         {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-image-04072"},
     }},

    {QueueError::kSrcAndDestMustBeIgnore,
     {
         // this isn't an error for synchronization2, so we don't need the 2KHR versions
         {Key(Struct::VkBufferMemoryBarrier), "VUID-VkBufferMemoryBarrier-synchronization2-03852"},
         {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-synchronization2-03856"},
     }},
    {QueueError::kSrcAndDstBothValid,
     {
         {Key(Struct::VkBufferMemoryBarrier2), "VUID-VkBufferMemoryBarrier2-buffer-04086"},
         {Key(Struct::VkBufferMemoryBarrier), "VUID-VkBufferMemoryBarrier-buffer-04086"},
         {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-image-04069"},
         {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-image-04069"},
     }},
    {QueueError::kSubmitQueueMustMatchSrcOrDst,
     {
         {Key(Struct::VkImageMemoryBarrier), kVUID_Core_ImageMemoryBarrier_SharingModeExclusiveSameFamily},
         {Key(Struct::VkImageMemoryBarrier2), kVUID_Core_ImageMemoryBarrier2_SharingModeExclusiveSameFamily},
         {Key(Struct::VkBufferMemoryBarrier), kVUID_Core_BufferMemoryBarrier_SharingModeExclusiveSameFamily},
         {Key(Struct::VkBufferMemoryBarrier2), kVUID_Core_BufferMemoryBarrier2_SharingModeExclusiveSameFamily},
     }},
};

const std::map<QueueError, std::string> kQueueErrorSummary{
    {QueueError::kSrcOrDstMustBeIgnore, "Source or destination queue family must be ignored."},
    {QueueError::kSpecialOrIgnoreOnly, "Source or destination queue family must be special or ignored."},
    {QueueError::kSrcAndDstValidOrSpecial, "Source and destination queue family must be valid, ignored, or special."},
    {QueueError::kSrcAndDestMustBeIgnore, "Source and destination queue family must both be ignored."},
    {QueueError::kSrcAndDstBothValid, "Source and destination queue family must both be valid."},
    {QueueError::kSubmitQueueMustMatchSrcOrDst,
     "Source or destination queue family must match submit queue family, if not ignored."},
};

const std::string &GetBarrierQueueVUID(const Location &loc, QueueError error) {
    const auto &result = FindVUID(error, loc, kBarrierQueueErrors);
    assert(!result.empty());
    if (result.empty()) {
        static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-queue-error");
        return unhandled;
    }
    return result;
}

static const std::map<VkImageLayout, std::array<Entry, 2>> kImageLayoutErrors{
    {VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
     {{
         {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-oldLayout-01208"},
         {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-oldLayout-01208"},
     }}},
    {VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
     {{
         {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-oldLayout-01209"},
         {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-oldLayout-01209"},
     }}},
    {VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
     {{
         {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-oldLayout-01210"},
         {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-oldLayout-01210"},
     }}},
    {VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
     {{
         {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-oldLayout-01211"},
         {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-oldLayout-01211"},
     }}},
    {VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
     {{
         {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-oldLayout-01212"},
         {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-oldLayout-01212"},
     }}},
    {VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
     {{
         {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-oldLayout-01213"},
         {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-oldLayout-01213"},
     }}},
    {VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV,
     {{
         {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-oldLayout-02088"},
         {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-oldLayout-02088"},
     }}},
    {VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
     {{
         {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-oldLayout-01658"},
         {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-oldLayout-01658"},
     }}},
    {VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
     {{
         {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-oldLayout-01659"},
         {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-oldLayout-01659"},
     }}},
    {VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT,
     {{
         {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07006"},
         {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07006"},
     }}},
};

const std::string &GetBadImageLayoutVUID(const Location &loc, VkImageLayout layout) {
    const auto &result = FindVUID(layout, loc, kImageLayoutErrors);
    assert(!result.empty());
    if (result.empty()) {
        static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-bad-image-layout");
        return unhandled;
    }
    return result;
}

static const std::map<BufferError, std::array<Entry, 2>> kBufferErrors{
    {BufferError::kNoMemory,
     {{
         {Key(Struct::VkBufferMemoryBarrier2), "VUID-VkBufferMemoryBarrier2-buffer-01931"},
         {Key(Struct::VkBufferMemoryBarrier), "VUID-VkBufferMemoryBarrier-buffer-01931"},
     }}},
    {BufferError::kOffsetTooBig,
     {{
         {Key(Struct::VkBufferMemoryBarrier), "VUID-VkBufferMemoryBarrier-offset-01187"},
         {Key(Struct::VkBufferMemoryBarrier2), "VUID-VkBufferMemoryBarrier2-offset-01187"},
     }}},
    {BufferError::kSizeOutOfRange,
     {{
         {Key(Struct::VkBufferMemoryBarrier), "VUID-VkBufferMemoryBarrier-size-01189"},
         {Key(Struct::VkBufferMemoryBarrier2), "VUID-VkBufferMemoryBarrier2-size-01189"},
     }}},
    {BufferError::kSizeZero,
     {{
         {Key(Struct::VkBufferMemoryBarrier), "VUID-VkBufferMemoryBarrier-size-01188"},
         {Key(Struct::VkBufferMemoryBarrier2), "VUID-VkBufferMemoryBarrier2-size-01188"},
     }}},
    {BufferError::kQueueFamilyExternal,
     {{
         {Key(Struct::VkBufferMemoryBarrier), "VUID-VkBufferMemoryBarrier-srcQueueFamilyIndex-04087"},
         {Key(Struct::VkBufferMemoryBarrier2), "VUID-VkBufferMemoryBarrier2-srcQueueFamilyIndex-04087"},
     }}},
};

const std::string &GetBufferBarrierVUID(const Location &loc, BufferError error) {
    const auto &result = FindVUID(error, loc, kBufferErrors);
    assert(!result.empty());
    if (result.empty()) {
         static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-buffer-barrier-error");
         return unhandled;
    }
    return result;
}

static const std::map<ImageError, std::vector<Entry>> kImageErrors{
    {ImageError::kNoMemory,
     {
         {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-image-01932"},
         {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-image-01932"},
     }},
    {ImageError::kConflictingLayout,
     {
         {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-oldLayout-01197"},
         {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-oldLayout-01197"},
     }},
    {ImageError::kBadLayout,
     {
         {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-newLayout-01198"},
         {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-newLayout-01198"},
     }},
    {ImageError::kBadAttFeedbackLoopLayout,
     {
         {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-attachmentFeedbackLoopLayout-07313"},
         {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-attachmentFeedbackLoopLayout-07313"},
     }},
    {ImageError::kNotColorAspect,
     {
         {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-image-01671"},
         {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-image-01671"},
     }},
    {ImageError::kNotColorAspectYcbcr,
     {
         {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-image-02902"},
         {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-image-02902"},
     }},
    {ImageError::kBadMultiplanarAspect,
     {
         {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-image-01672"},
         {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-image-01672"},
     }},
    {ImageError::kBadPlaneCount,
     {
         {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-image-01673"},
         {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-image-01673"},
     }},
    {ImageError::kNotDepthOrStencilAspect,
     {
         {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-image-03319"},
         {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-image-03319"},
     }},
    {ImageError::kNotDepthAndStencilAspect,
     {
         {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-image-01207"},
         {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-image-01207"},
     }},
    {ImageError::kNotSeparateDepthAndStencilAspect,
     {
         {Key(Struct::VkImageMemoryBarrier), "VUID-VkImageMemoryBarrier-image-03320"},
         {Key(Struct::VkImageMemoryBarrier2), "VUID-VkImageMemoryBarrier2-image-03320"},
     }},
    {ImageError::kRenderPassMismatch,
     {
         {Key(Func::vkCmdPipelineBarrier), "VUID-vkCmdPipelineBarrier-image-04073"},
         {Key(Func::vkCmdPipelineBarrier2), "VUID-vkCmdPipelineBarrier2-image-04073"},
     }},
    {ImageError::kRenderPassLayoutChange,
     {
         {Key(Func::vkCmdPipelineBarrier), "VUID-vkCmdPipelineBarrier-oldLayout-01181"},
         {Key(Func::vkCmdPipelineBarrier2), "VUID-vkCmdPipelineBarrier2-oldLayout-01181"},
     }},
};

const std::string &GetImageBarrierVUID(const Location &loc, ImageError error) {
    const auto &result = FindVUID(error, loc, kImageErrors);
    assert(!result.empty());
    if (result.empty()) {
         static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-image-barrier-error");
         return unhandled;
    }
    return result;
}

const SubresourceRangeErrorCodes &GetSubResourceVUIDs(const Location &loc) {
    static const SubresourceRangeErrorCodes v1 {
        "VUID-VkImageMemoryBarrier-subresourceRange-01486",
        "VUID-VkImageMemoryBarrier-subresourceRange-01724",
        "VUID-VkImageMemoryBarrier-subresourceRange-01488",
        "VUID-VkImageMemoryBarrier-subresourceRange-01725",
    };
    static const SubresourceRangeErrorCodes v2 {
        "VUID-VkImageMemoryBarrier2-subresourceRange-01486",
        "VUID-VkImageMemoryBarrier2-subresourceRange-01724",
        "VUID-VkImageMemoryBarrier2-subresourceRange-01488"
        "VUID-VkImageMemoryBarrier2-subresourceRange-01725",
    };
    return (loc.structure == Struct::VkImageMemoryBarrier2) ? v2 : v1;
}

static const std::map<SubmitError, std::vector<Entry>> kSubmitErrors{
    {SubmitError::kTimelineSemSmallValue,
     {
         {Key(Struct::VkSemaphoreSignalInfo), "VUID-VkSemaphoreSignalInfo-value-03258"},
         {Key(Struct::VkBindSparseInfo), "VUID-VkBindSparseInfo-pSignalSemaphores-03249"},
         {Key(Struct::VkSubmitInfo), "VUID-VkSubmitInfo-pSignalSemaphores-03242"},
         {Key(Struct::VkSubmitInfo2), "VUID-VkSubmitInfo2-semaphore-03881"},
         {Key(Struct::VkSubmitInfo2), "VUID-VkSubmitInfo2-semaphore-03882"},
     }},
    {SubmitError::kSemAlreadySignalled,
     {
         {Key(Func::vkQueueSubmit), "VUID-vkQueueSubmit-pSignalSemaphores-00067"},
         {Key(Func::vkQueueBindSparse), "VUID-vkQueueBindSparse-pSignalSemaphores-01115"},
         {Key(Func::vkQueueSubmit2), "VUID-vkQueueSubmit2-semaphore-03868"},
     }},
    {SubmitError::kBinaryCannotBeSignalled,
     {
         {Key(Func::vkQueueSubmit), "VUID-vkQueueSubmit-pWaitSemaphores-00069"},
         {Key(Func::vkQueueSubmit2), "VUID-vkQueueSubmit2-semaphore-03872"},
     }},
    {SubmitError::kTimelineCannotBeSignalled,
     {
         {Key(Func::vkQueueSubmit), "VUID-vkQueueSubmit-pWaitSemaphores-03238"},
         {Key(Func::vkQueueSubmit2), "VUID-vkQueueSubmit2-semaphore-03873"},
     }},
    {SubmitError::kTimelineSemMaxDiff,
     {
         {Key(Struct::VkBindSparseInfo, Field::pWaitSemaphores), "VUID-VkBindSparseInfo-pWaitSemaphores-03250"},
         {Key(Struct::VkBindSparseInfo, Field::pSignalSemaphores), "VUID-VkBindSparseInfo-pSignalSemaphores-03251"},
         {Key(Struct::VkSubmitInfo, Field::pWaitSemaphores), "VUID-VkSubmitInfo-pWaitSemaphores-03243"},
         {Key(Struct::VkSubmitInfo, Field::pSignalSemaphores), "VUID-VkSubmitInfo-pSignalSemaphores-03244"},
         {Key(Struct::VkSemaphoreSignalInfo), "VUID-VkSemaphoreSignalInfo-value-03260"},
         {Key(Struct::VkSubmitInfo2, Field::pWaitSemaphoreInfos, true), "VUID-VkSubmitInfo2-semaphore-03883"},
         {Key(Struct::VkSubmitInfo2, Field::pSignalSemaphoreInfos, true), "VUID-VkSubmitInfo2-semaphore-03884"},
     }},
    {SubmitError::kProtectedFeatureDisabled,
     {
         {Key(Struct::VkProtectedSubmitInfo), "VUID-vkQueueSubmit-queue-06448"},
         {Key(Struct::VkSubmitInfo2), "VUID-vkQueueSubmit2-queue-06447"},
     }},
    {SubmitError::kBadUnprotectedSubmit,
     {
         {Key(Struct::VkSubmitInfo), "VUID-VkSubmitInfo-pNext-04148"},
         {Key(Struct::VkSubmitInfo2), "VUID-VkSubmitInfo2-flags-03886"},
     }},
    {SubmitError::kBadProtectedSubmit,
     {
         {Key(Struct::VkSubmitInfo), "VUID-VkSubmitInfo-pNext-04120"},
         {Key(Struct::VkSubmitInfo2), "VUID-VkSubmitInfo2-flags-03887"},
     }},
    {SubmitError::kCmdNotSimultaneous,
     {
         {Key(Func::vkQueueSubmit), "VUID-vkQueueSubmit-pCommandBuffers-00071"},
         {Key(Func::vkQueueSubmit2), "VUID-vkQueueSubmit2-commandBuffer-03875"},
     }},
    {SubmitError::kReusedOneTimeCmd,
     {
         {Key(Func::vkQueueSubmit), "VUID-vkQueueSubmit-pCommandBuffers-00072"},
         {Key(Func::vkQueueSubmit2), "VUID-vkQueueSubmit2-commandBuffer-03876"},
     }},
    {SubmitError::kSecondaryCmdNotSimultaneous,
     {
         {Key(Func::vkQueueSubmit2), "VUID-vkQueueSubmit-pCommandBuffers-00073"},
         {Key(Func::vkQueueSubmit2), "VUID-vkQueueSubmit2-commandBuffer-03877"},
     }},
    {SubmitError::kCmdWrongQueueFamily,
     {
         {Key(Func::vkQueueSubmit), "VUID-vkQueueSubmit-pCommandBuffers-00074"},
         {Key(Func::vkQueueSubmit2), "VUID-vkQueueSubmit2-commandBuffer-03878"},
     }},
    {SubmitError::kSecondaryCmdInSubmit,
     {
         {Key(Func::vkQueueSubmit), "VUID-VkSubmitInfo-pCommandBuffers-00075"},
         {Key(Func::vkQueueSubmit2), "VUID-VkCommandBufferSubmitInfo-commandBuffer-03890"},
     }},
    {SubmitError::kHostStageMask,
     {
         {Key(Struct::VkSubmitInfo), "VUID-VkSubmitInfo-pWaitDstStageMask-00078"},
         {Key(Func::vkCmdSetEvent), "VUID-vkCmdSetEvent-stageMask-01149"},
         {Key(Func::vkCmdResetEvent), "VUID-vkCmdResetEvent-stageMask-01153"},
         {Key(Func::vkCmdResetEvent2), "VUID-vkCmdResetEvent2-stageMask-03830"},
     }},
};

const std::string &GetQueueSubmitVUID(const Location &loc, SubmitError error) {
    const auto &result = FindVUID(error, loc, kSubmitErrors);
    assert(!result.empty());
    if (result.empty()) {
         static const std::string unhandled("UNASSIGNED-CoreChecks-unhandled-submit-error");
         return unhandled;
    }
    return result;
}

};  // namespace sync_vuid_maps
