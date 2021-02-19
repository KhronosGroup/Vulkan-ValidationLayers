/* Copyright (c) 2021 The Khronos Group Inc.
 * Copyright (c) 2021 Valve Corporation
 * Copyright (c) 2021 LunarG, Inc.
 * Copyright (C) 2021 Google Inc.
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
#include "state_tracker.h"
#include <cassert>
#include <algorithm>
#include <array>
#include <vector>
#include <unordered_map>

namespace sync_vuid_maps {

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

static std::string MakeKey(const std::string &refpage, const std::string &field_name) {
    std::string key(refpage);
    key += "-";
    key += field_name;
    return key;
}

template <typename Table>
static const std::string &FindVUID(const std::string &refpage, const std::string &field_name, const Table &table) {
    static const std::string empty;
    auto key = MakeKey(refpage, field_name);
    const auto pos =
        std::find_if(table.begin(), table.end(), [&key](const std::string &entry) { return entry.find(key) != std::string::npos; });

    return (pos != table.end()) ? *pos : empty;
}

template <typename Table>
static const std::string &FindVUID(const RefPage &refpage, const Field &field_name, const Table &table) {
    const auto &str_ref = CoreErrorLocation::String(refpage);
    const auto &str_field = CoreErrorLocation::String(field_name);
    return FindVUID(str_ref, str_field, table);
}

// commonvalidity/pipeline_stage_common.txt
// commonvalidity/stage_mask_2_common.txt
// commonvalidity/stage_mask_common.txt
// cmdbuffers.txt VkSubmitInfo-pWaitDstStageMask
// renderpass.txt VkSubpassDependency2-dstStageMask
// renderpass.txt VkSubpassDependency2-srcStageMask
// renderpass.txt VkSubpassDependency-dstStageMask
// renderpass.txt VkSubpassDependency-srcStageMask

static const std::map<VkPipelineStageFlags2KHR, std::vector<std::string>> kStageMaskErrors{
    {VK_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT,
     {"VUID-VkBufferMemoryBarrier2KHR-dstStageMask-03931",
      "VUID-VkBufferMemoryBarrier2KHR-srcStageMask-03931",
      "VUID-vkCmdPipelineBarrier-dstStageMask-04092",
      "VUID-vkCmdPipelineBarrier-srcStageMask-04092",
      "VUID-vkCmdResetEvent2KHR-stageMask-03931",
      "VUID-vkCmdResetEvent-stageMask-04092",
      "VUID-vkCmdSetEvent-stageMask-04092",
      "VUID-vkCmdWaitEvents-dstStageMask-04092",
      "VUID-vkCmdWaitEvents-srcStageMask-04092",
      "VUID-vkCmdWriteTimestamp2KHR-stageMask-03931",
      "VUID-vkCmdWriteTimestamp-pipelineStage-04077",
      "VUID-VkImageMemoryBarrier2KHR-dstStageMask-03931",
      "VUID-VkImageMemoryBarrier2KHR-srcStageMask-03931",
      "VUID-VkMemoryBarrier2KHR-dstStageMask-03931",
      "VUID-VkMemoryBarrier2KHR-srcStageMask-03931",
      "VUID-VkSemaphoreSubmitInfoKHR-stageMask-03931",
      "UNASSIGNED-CoreChecks-VkSubmitInfo-pWaitDstStageMask-conditionalRendering",
      "UNASSIGNED-CoreChecks-VkSubpassDependency-srcStageMask-conditionalRendering",
      "UNASSIGNED-CoreChecks-VkSubpassDependency-dstStageMask-conditionalRendering",
      "UNASSIGNED-CoreChecks-VkSubpassDependency2-srcStageMask-conditionalRendering",
      "UNASSIGNED-CoreChecks-VkSubpassDependency2-dstStageMask-conditionalRendering"}},
    {VK_PIPELINE_STAGE_2_FRAGMENT_DENSITY_PROCESS_BIT_EXT,
     {"VUID-VkBufferMemoryBarrier2KHR-dstStageMask-03932",
      "VUID-VkBufferMemoryBarrier2KHR-srcStageMask-03932",
      "VUID-vkCmdPipelineBarrier-dstStageMask-04093",
      "VUID-vkCmdPipelineBarrier-srcStageMask-04093",
      "VUID-vkCmdResetEvent2KHR-stageMask-03932",
      "VUID-vkCmdResetEvent-stageMask-04093",
      "VUID-vkCmdSetEvent-stageMask-04093",
      "VUID-vkCmdWaitEvents-dstStageMask-04093",
      "VUID-vkCmdWaitEvents-srcStageMask-04093",
      "VUID-vkCmdWriteTimestamp2KHR-stageMask-03932",
      "VUID-vkCmdWriteTimestamp-pipelineStage-04078",
      "VUID-VkImageMemoryBarrier2KHR-dstStageMask-03932",
      "VUID-VkImageMemoryBarrier2KHR-srcStageMask-03932",
      "VUID-VkMemoryBarrier2KHR-dstStageMask-03932",
      "VUID-VkMemoryBarrier2KHR-srcStageMask-03932",
      "VUID-VkSemaphoreSubmitInfoKHR-stageMask-03932",
      "UNASSIGNED-CoreChecks-VkSubmitInfo-pWaitDstStageMask-fragmentDensity",
      "UNASSIGNED-CoreChecks-VkSubpassDependency-srcStageMask-fragmentDensity",
      "UNASSIGNED-CoreChecks-VkSubpassDependency-dstStageMask-fragmentDensity",
      "UNASSIGNED-CoreChecks-VkSubpassDependency2-srcStageMask-fragmentDensity",
      "UNASSIGNED-CoreChecks-VkSubpassDependency2-dstStageMask-fragmentDensity"}},
    {VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
     {"VUID-VkBufferMemoryBarrier2KHR-dstStageMask-03936",
      "VUID-VkBufferMemoryBarrier2KHR-srcStageMask-03936",
      "VUID-vkCmdPipelineBarrier-dstStageMask-04097",
      "VUID-vkCmdPipelineBarrier-srcStageMask-04097",
      "VUID-vkCmdResetEvent2KHR-stageMask-03936",
      "VUID-vkCmdResetEvent-stageMask-04097",
      "VUID-vkCmdSetEvent-stageMask-04097",
      "VUID-vkCmdWaitEvents-dstStageMask-04097",
      "VUID-vkCmdWaitEvents-srcStageMask-04097",
      "VUID-vkCmdWriteTimestamp2KHR-stageMask-03936",
      "VUID-vkCmdWriteTimestamp-pipelineStage-04081",
      "VUID-VkImageMemoryBarrier2KHR-dstStageMask-03936",
      "VUID-VkImageMemoryBarrier2KHR-srcStageMask-03936",
      "VUID-VkMemoryBarrier2KHR-dstStageMask-03936",
      "VUID-VkMemoryBarrier2KHR-srcStageMask-03936",
      "VUID-VkSemaphoreSubmitInfoKHR-stageMask-03936",
      "UNASSIGNED-CoreChecks-VkSubmitInfo-pWaitDstStageMask-shadingRate",
      "UNASSIGNED-CoreChecks-VkSubpassDependency-srcStageMask-shadingRate",
      "UNASSIGNED-CoreChecks-VkSubpassDependency-dstStageMask-shadingRate",
      "UNASSIGNED-CoreChecks-VkSubpassDependency2-srcStageMask-shadingRate",
      "UNASSIGNED-CoreChecks-VkSubpassDependency2-dstStageMask-shadingRate"}},
    {VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT_KHR,
     {"VUID-VkBufferMemoryBarrier2KHR-dstStageMask-03929",
      "VUID-VkBufferMemoryBarrier2KHR-srcStageMask-03929",
      "VUID-vkCmdPipelineBarrier-dstStageMask-04090",
      "VUID-vkCmdPipelineBarrier-srcStageMask-04090",
      "VUID-vkCmdResetEvent2KHR-stageMask-03929",
      "VUID-vkCmdResetEvent-stageMask-04090",
      "VUID-vkCmdSetEvent-stageMask-04090",
      "VUID-vkCmdWaitEvents-dstStageMask-04090",
      "VUID-vkCmdWaitEvents-srcStageMask-04090",
      "VUID-vkCmdWriteTimestamp2KHR-stageMask-03929",
      "VUID-vkCmdWriteTimestamp-pipelineStage-04075",
      "VUID-VkImageMemoryBarrier2KHR-dstStageMask-03929",
      "VUID-VkImageMemoryBarrier2KHR-srcStageMask-03929",
      "VUID-VkMemoryBarrier2KHR-dstStageMask-03929",
      "VUID-VkMemoryBarrier2KHR-srcStageMask-03929",
      "VUID-VkSemaphoreSubmitInfoKHR-stageMask-03929",
      "VUID-VkSubmitInfo-pWaitDstStageMask-00076",
      "VUID-VkSubpassDependency-srcStageMask-00860",
      "VUID-VkSubpassDependency-dstStageMask-00861",
      "VUID-VkSubpassDependency2-srcStageMask-03080",
      "VUID-VkSubpassDependency2-dstStageMask-03081"}},
    {VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV,
     {"VUID-VkBufferMemoryBarrier2KHR-dstStageMask-03934",
      "VUID-VkBufferMemoryBarrier2KHR-srcStageMask-03934",
      "VUID-vkCmdPipelineBarrier-dstStageMask-04095",
      "VUID-vkCmdPipelineBarrier-srcStageMask-04095",
      "VUID-vkCmdResetEvent2KHR-stageMask-03934",
      "VUID-vkCmdResetEvent-stageMask-04095",
      "VUID-vkCmdSetEvent-stageMask-04095",
      "VUID-vkCmdWaitEvents-dstStageMask-04095",
      "VUID-vkCmdWaitEvents-srcStageMask-04095",
      "VUID-vkCmdWriteTimestamp2KHR-stageMask-03934",
      "VUID-vkCmdWriteTimestamp-pipelineStage-04080",
      "VUID-VkImageMemoryBarrier2KHR-dstStageMask-03934",
      "VUID-VkImageMemoryBarrier2KHR-srcStageMask-03934",
      "VUID-VkMemoryBarrier2KHR-dstStageMask-03934",
      "VUID-VkMemoryBarrier2KHR-srcStageMask-03934",
      "VUID-VkSemaphoreSubmitInfoKHR-stageMask-03934",
      "VUID-VkSubmitInfo-pWaitDstStageMask-02089",
      "VUID-VkSubpassDependency-srcStageMask-02099",
      "VUID-VkSubpassDependency-dstStageMask-02101",
      "VUID-VkSubpassDependency2-srcStageMask-02103",
      "VUID-VkSubpassDependency2-dstStageMask-02105"}},
    {VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV,
     {"VUID-VkBufferMemoryBarrier2KHR-dstStageMask-03935",
      "VUID-VkBufferMemoryBarrier2KHR-srcStageMask-03935",
      "VUID-vkCmdPipelineBarrier-dstStageMask-04096",
      "VUID-vkCmdPipelineBarrier-srcStageMask-04096",
      "VUID-vkCmdResetEvent2KHR-stageMask-03935",
      "VUID-vkCmdResetEvent-stageMask-04096",
      "VUID-vkCmdSetEvent-stageMask-04096",
      "VUID-vkCmdWaitEvents-dstStageMask-04096",
      "VUID-vkCmdWaitEvents-srcStageMask-04096",
      "VUID-vkCmdWriteTimestamp2KHR-stageMask-03935",
      "VUID-vkCmdWriteTimestamp-pipelineStage-04080",
      "VUID-VkImageMemoryBarrier2KHR-dstStageMask-03935",
      "VUID-VkImageMemoryBarrier2KHR-srcStageMask-03935",
      "VUID-VkMemoryBarrier2KHR-dstStageMask-03935",
      "VUID-VkMemoryBarrier2KHR-srcStageMask-03935",
      "VUID-VkSemaphoreSubmitInfoKHR-stageMask-03935",
      "VUID-VkSubmitInfo-pWaitDstStageMask-02090",
      "VUID-VkSubpassDependency-srcStageMask-02100",
      "VUID-VkSubpassDependency-dstStageMask-02102",
      "VUID-VkSubpassDependency2-srcStageMask-02104",
      "VUID-VkSubpassDependency2-dstStageMask-02106"}},
    {VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT_KHR,
     {"VUID-VkBufferMemoryBarrier2KHR-dstStageMask-03930",
      "VUID-VkBufferMemoryBarrier2KHR-srcStageMask-03930",
      "VUID-vkCmdPipelineBarrier-dstStageMask-04091",
      "VUID-vkCmdPipelineBarrier-srcStageMask-04091",
      "VUID-vkCmdResetEvent2KHR-stageMask-03930",
      "VUID-vkCmdResetEvent-stageMask-04091",
      "VUID-vkCmdSetEvent-stageMask-04091",
      "VUID-vkCmdWaitEvents-dstStageMask-04091",
      "VUID-vkCmdWaitEvents-srcStageMask-04091",
      "VUID-vkCmdWriteTimestamp2KHR-stageMask-03930",
      "VUID-vkCmdWriteTimestamp-pipelineStage-04076",
      "VUID-VkImageMemoryBarrier2KHR-dstStageMask-03930",
      "VUID-VkImageMemoryBarrier2KHR-srcStageMask-03930",
      "VUID-VkMemoryBarrier2KHR-dstStageMask-03930",
      "VUID-VkMemoryBarrier2KHR-srcStageMask-03930",
      "VUID-VkSemaphoreSubmitInfoKHR-stageMask-03930",
      "VUID-VkSubmitInfo-pWaitDstStageMask-00077",
      "VUID-VkSubpassDependency-srcStageMask-00862",
      "VUID-VkSubpassDependency-dstStageMask-00863",
      "VUID-VkSubpassDependency2-srcStageMask-03082",
      "VUID-VkSubpassDependency2-dstStageMask-03083"}},
    {VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT_KHR,
     {"VUID-VkBufferMemoryBarrier2KHR-dstStageMask-03930",
      "VUID-VkBufferMemoryBarrier2KHR-srcStageMask-03930",
      "VUID-vkCmdPipelineBarrier-dstStageMask-04091",
      "VUID-vkCmdPipelineBarrier-srcStageMask-04091",
      "VUID-vkCmdResetEvent2KHR-stageMask-03930",
      "VUID-vkCmdResetEvent-stageMask-04091",
      "VUID-vkCmdSetEvent-stageMask-04091",
      "VUID-vkCmdWaitEvents-dstStageMask-04091",
      "VUID-vkCmdWaitEvents-srcStageMask-04091",
      "VUID-vkCmdWriteTimestamp2KHR-pipelineStage-04076",
      "VUID-vkCmdWriteTimestamp-pipelineStage-04076",
      "VUID-VkImageMemoryBarrier2KHR-dstStageMask-03930",
      "VUID-VkImageMemoryBarrier2KHR-srcStageMask-03930",
      "VUID-VkMemoryBarrier2KHR-dstStageMask-03930",
      "VUID-VkMemoryBarrier2KHR-srcStageMask-03930",
      "VUID-VkSemaphoreSubmitInfoKHR-stageMask-03930",
      "VUID-VkSubmitInfo-pWaitDstStageMask-00077",
      "VUID-VkSubpassDependency-srcStageMask-00862",
      "VUID-VkSubpassDependency-dstStageMask-00863",
      "VUID-VkSubpassDependency2-srcStageMask-03082",
      "VUID-VkSubpassDependency2-dstStageMask-03083"}},
    {VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT,
     {"VUID-VkBufferMemoryBarrier2KHR-dstStageMask-03933",
      "VUID-VkBufferMemoryBarrier2KHR-srcStageMask-03933",
      "VUID-vkCmdPipelineBarrier-dstStageMask-04094",
      "VUID-vkCmdPipelineBarrier-srcStageMask-04094",
      "VUID-vkCmdResetEvent2KHR-stageMask-03933",
      "VUID-vkCmdResetEvent-stageMask-04094",
      "VUID-vkCmdSetEvent-stageMask-04094",
      "VUID-vkCmdWaitEvents-dstStageMask-04094",
      "VUID-vkCmdWaitEvents-srcStageMask-04094",
      "VUID-vkCmdWriteTimestamp2KHR-stageMask-03933",
      "VUID-vkCmdWriteTimestamp-pipelineStage-04079",
      "VUID-VkImageMemoryBarrier2KHR-dstStageMask-03933",
      "VUID-VkImageMemoryBarrier2KHR-srcStageMask-03933",
      "VUID-VkMemoryBarrier2KHR-dstStageMask-03933",
      "VUID-VkMemoryBarrier2KHR-srcStageMask-03933",
      "VUID-VkSemaphoreSubmitInfoKHR-stageMask-03933",
      "UNASSIGNED-CoreChecks-VkSubmitInfo-pWaitDstStageMask-transformFeedback",
      "UNASSIGNED-CoreChecks-VkSubpassDependency-srcStageMask-transformFeedback",
      "UNASSIGNED-CoreChecks-VkSubpassDependency-dstStageMask-transformFeedback",
      "UNASSIGNED-CoreChecks-VkSubpassDependency2-srcStageMask-transformFeedback",
      "UNASSIGNED-CoreChecks-VkSubpassDependency2-dstStageMask-transformFeedback"}},
    // special case for VK_PIPELINE_STAGE_NONE_KHR. This entry omits the synchronization2
    // commands because they shouldn't be called unless synchronization2 is enabled.
    {0,
     {"VUID-vkCmdPipelineBarrier-srcStageMask-03937", "VUID-vkCmdPipelineBarrier-dstStageMask-03937",
      "VUID-vkCmdResetEvent-stageMask-03937", "VUID-vkCmdSetEvent-stageMask-03937", "VUID-vkCmdWaitEvents-srcStageMask-03937",
      "VUID-vkCmdWaitEvents-dstStageMask-03937"}},
};

const std::string &GetBadFeatureVUID(const CoreErrorLocation &loc, VkPipelineStageFlags2KHR bit) {
    const auto entry = kStageMaskErrors.find(bit);
    assert(entry != kStageMaskErrors.end());
    const auto &result = FindVUID(loc.refpage, loc.field_name, entry->second);
    assert(!result.empty());
    return result;
}

// commonvalidity/access_mask_2_common.txt
static const std::map<VkAccessFlags2KHR, std::array<std::string, 6>> kAccessMask2Common = {
    {VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT_KHR,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03900",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03900",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03900",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03900",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03900",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03900",
     }},
    {VK_ACCESS_2_INDEX_READ_BIT_KHR,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03901",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03901",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03901",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03901",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03901",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03901",
     }},
    {VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT_KHR,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03902",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03902",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03902",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03902",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03902",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03902",
     }},
    {VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT_KHR,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03903",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03903",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03903",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03903",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03903",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03903",
     }},
    {VK_ACCESS_2_UNIFORM_READ_BIT_KHR,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03904",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03904",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03904",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03904",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03904",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03904",
     }},
    {VK_ACCESS_2_SHADER_SAMPLED_READ_BIT_KHR,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03905",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03905",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03905",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03905",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03905",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03905",
     }},
    {VK_ACCESS_2_SHADER_STORAGE_READ_BIT_KHR,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03906",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03906",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03906",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03906",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03906",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03906",
     }},
    {VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT_KHR,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03907",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03907",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03907",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03907",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03907",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03907",
     }},
    {VK_ACCESS_2_SHADER_READ_BIT_KHR,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03908",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03908",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03908",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03908",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03908",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03908",
     }},
    {VK_ACCESS_2_SHADER_WRITE_BIT_KHR,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03909",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03909",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03909",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03909",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03909",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03909",
     }},
    {VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT_KHR,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03910",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03910",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03910",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03910",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03910",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03910",
     }},
    {VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03911",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03911",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03911",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03911",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03911",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03911",
     }},
    {VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT_KHR,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03912",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03912",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03912",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03912",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03912",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03912",
     }},
    {VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT_KHR,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03913",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03913",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03913",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03913",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03913",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03913",
     }},
    {VK_ACCESS_2_TRANSFER_READ_BIT_KHR,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03914",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03914",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03914",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03914",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03914",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03914",
     }},
    {VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03915",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03915",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03915",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03915",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03915",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03915",
     }},
    {VK_ACCESS_2_HOST_READ_BIT_KHR,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03916",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03916",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03916",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03916",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03916",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03916",
     }},
    {VK_ACCESS_2_HOST_WRITE_BIT_KHR,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03917",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03917",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03917",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03917",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03917",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03917",
     }},
    {VK_ACCESS_2_CONDITIONAL_RENDERING_READ_BIT_EXT,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03918",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03918",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03918",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03918",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03918",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03918",
     }},
    {VK_ACCESS_2_FRAGMENT_DENSITY_MAP_READ_BIT_EXT,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03919",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03919",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03919",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03919",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03919",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03919",
     }},
    {VK_ACCESS_2_TRANSFORM_FEEDBACK_WRITE_BIT_EXT,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03920",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03920",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03920",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03920",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03920",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03920",
     }},
    {VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03920",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03920",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03920",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03920",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03920",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03920",
     }},
    {VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03921",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03921",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03921",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03921",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03921",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03921",
     }},
    {VK_ACCESS_2_SHADING_RATE_IMAGE_READ_BIT_NV,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03922",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03922",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03922",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03922",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03922",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03922",
     }},
    {VK_ACCESS_2_COMMAND_PREPROCESS_READ_BIT_NV,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03923",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03923",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03923",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03923",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03923",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03923",
     }},
    {VK_ACCESS_2_COMMAND_PREPROCESS_WRITE_BIT_NV,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03924",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03924",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03924",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03924",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03924",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03924",
     }},
    {VK_PIPELINE_STAGE_2_COMMAND_PREPROCESS_BIT_NV,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03925",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03925",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03925",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03925",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03925",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03925",
     }},
    {VK_ACCESS_2_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03926",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03926",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03926",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03926",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03926",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03926",
     }},
    {VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03927",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03927",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03927",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03927",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03927",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03927",
     }},
    {VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
     {
         "VUID-VkMemoryBarrier2KHR-srcAccessMask-03928",
         "VUID-VkMemoryBarrier2KHR-dstAccessMask-03928",
         "VUID-VkBufferMemoryBarrier2KHR-srcAccessMask-03928",
         "VUID-VkBufferMemoryBarrier2KHR-dstAccessMask-03928",
         "VUID-VkImageMemoryBarrier2KHR-srcAccessMask-03928",
         "VUID-VkImageMemoryBarrier2KHR-dstAccessMask-03928",
     }},
};

// commonvalidity/fine_sync_commands_common.txt
static const std::vector<std::string> kFineSyncCommon = {
    "VUID-vkCmdPipelineBarrier-srcAccessMask-02815", "VUID-vkCmdPipelineBarrier-dstAccessMask-02816",
    "VUID-vkCmdWaitEvents-srcAccessMask-02815",      "VUID-vkCmdWaitEvents-dstAccessMask-02816",
    "VUID-VkSubpassDependency-srcAccessMask-00868",  "VUID-VkSubpassDependency-dstAccessMask-00869",
    "VUID-VkSubpassDependency2-srcAccessMask-03088", "VUID-VkSubpassDependency2-dstAccessMask-03089",
};
const std::string &GetBadAccessFlagsVUID(const CoreErrorLocation &loc, VkAccessFlags2KHR bit) {
    const auto entry = kAccessMask2Common.find(bit);
    if (entry != kAccessMask2Common.end()) {
        const auto &result = FindVUID(loc.refpage, loc.field_name, entry->second);
        if (!result.empty()) {
            return result;
        }
    }
    const auto &result2 = FindVUID(loc.refpage, loc.field_name, kFineSyncCommon);
    assert(!result2.empty());
    return result2;
}

static const std::vector<std::string> kQueueCapErrors{
    "VUID-VkSubmitInfo-pWaitDstStageMask-00066",
    "VUID-vkCmdBeginRenderPass-srcStageMask-00901",
    "VUID-vkCmdBeginRenderPass-dstStageMask-00901",
    "VUID-vkCmdSetEvent-stageMask-4098",
    "VUID-vkCmdResetEvent-stageMask-4098",
    "VUID-vkCmdWaitEvents-srcStageMask-4098",
    "VUID-vkCmdWaitEvents-dstStageMask-4098",
    "VUID-vkCmdPipelineBarrier-srcStageMask-4098",
    "VUID-vkCmdPipelineBarrier-dstStageMask-4098",
    "VUID-vkCmdWriteTimestamp-pipelineStage-04074",
    "VUID-vkCmdBeginRenderPass2-srcStageMask-03101",
    "VUID-vkCmdBeginRenderPass2-dstStageMask-03101",
    "VUID-vkCmdSetEvent2KHR-srcStageMask-03827",
    "VUID-vkCmdSetEvent2KHR-dstStageMask-03828",
    "VUID-vkCmdPipelineBarrier2KHR-srcStageMask-03849",
    "VUID-vkCmdPipelineBarrier2KHR-dstStageMask-03850",
    "VUID-vkCmdWaitEvents2KHR-srcStageMask-03842",
    "VUID-vkCmdWaitEvents2KHR-dstStageMask-03843",
    "VUID-vkCmdWriteTimestamp2KHR-stage-03860",
    "VUID-vkQueueSubmit2KHR-stageMask-03869",  // TODO: src
    "VUID-vkQueueSubmit2KHR-stageMask-03870",  // TODO: dst
};

const std::string &GetStageQueueCapVUID(const CoreErrorLocation &loc, VkPipelineStageFlags2KHR bit) {
    // no per-bit lookups needed
    return FindVUID(loc.refpage, loc.field_name, kQueueCapErrors);
}

static const std::map<QueueError, std::vector<std::string>> kBarrierQueueErrors{
    {QueueError::kSrcOrDstMustBeIgnore,
     {
         // this isn't an error for synchronization2, so we don't need the 2KHR versions
         "VUID-VkBufferMemoryBarrier-synchronization2-03853",
         "VUID-VkImageMemoryBarrier-synchronization2-03857",
     }},

    {QueueError::kSpecialOrIgnoreOnly,
     {
         "VUID-VkBufferMemoryBarrier2KHR-buffer-04088",
         "VUID-VkBufferMemoryBarrier-buffer-04088",
         "VUID-VkImageMemoryBarrier2KHR-image-04071",
         "VUID-VkImageMemoryBarrier-image-04071",
     }},
    {QueueError::kSrcAndDstValidOrSpecial,
     {
         "VUID-VkBufferMemoryBarrier2KHR-buffer-04089",
         "VUID-VkBufferMemoryBarrier-buffer-04089",
         "VUID-VkImageMemoryBarrier2KHR-image-04072",
         "VUID-VkImageMemoryBarrier-image-04072",
     }},

    {QueueError::kSrcAndDestMustBeIgnore,
     {
         // this isn't an error for synchronization2, so we don't need the 2KHR versions
         "VUID-VkBufferMemoryBarrier-synchronization2-03852",
         "VUID-VkImageMemoryBarrier-synchronization2-03856",
     }},
    {QueueError::kSrcAndDstBothValid,
     {
         "VUID-VkBufferMemoryBarrier2KHR-buffer-04086",
         "VUID-VkBufferMemoryBarrier-buffer-04086",
         "VUID-VkImageMemoryBarrier2KHR-image-04069",
         "VUID-VkImageMemoryBarrier-image-04069",
     }},
    {QueueError::kSubmitQueueMustMatchSrcOrDst,
     {
         "UNASSIGNED-CoreValidation-VkImageMemoryBarrier-sharing-mode-exclusive-same-family",
         "UNASSIGNED-CoreValidation-VkImageMemoryBarrier2KHR-sharing-mode-exclusive-same-family",
         "UNASSIGNED-CoreValidation-VkBufferMemoryBarrier-sharing-mode-exclusive-same-family",
         "UNASSIGNED-CoreValidation-VkBufferMemoryBarrier2KHR-sharing-mode-exclusive-same-family",
     }},
};

const std::map<QueueError, std::string> kQueueErrorSummary{
    {QueueError::kSrcOrDstMustBeIgnore, "Source or destination queue family must be ignored."},
    {QueueError::kSpecialOrIgnoreOnly, "Source or destination queue family must be special or ignored."},
    {QueueError::kSrcAndDstValidOrSpecial, "Destination queue family must be valid, ignored, or special."},
    {QueueError::kSrcAndDestMustBeIgnore, "Source and destination queue family must both be ignored."},
    {QueueError::kSrcAndDstBothValid, "Source and destination queue family must both be valid."},
    {QueueError::kSubmitQueueMustMatchSrcOrDst,
     "Source or destination queue family must match submit queue family, if not ignored."},
};

const std::string &GetBarrierQueueVUID(const CoreErrorLocation &loc, QueueError error) {
    const auto entry = kBarrierQueueErrors.find(error);
    assert(entry != kBarrierQueueErrors.end());

    // NOTE we ignore field_name here because of inconsistencies in the VUIDs.
    const auto &result = FindVUID(loc.refpage, Field::Empty, entry->second);
    assert(!result.empty());
    return result;
}

static const std::map<VkImageLayout, std::array<std::string, 2>> kImageLayoutErrors{
    {VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
     {"VUID-VkImageMemoryBarrier-oldLayout-01208", "VUID-VkImageMemoryBarrier2KHR-oldLayout-01208"}},
    {VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
     {"VUID-VkImageMemoryBarrier-oldLayout-01209", "VUID-VkImageMemoryBarrier2KHR-oldLayout-01209"}},
    {VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
     {"VUID-VkImageMemoryBarrier-oldLayout-01210", "VUID-VkImageMemoryBarrier2KHR-oldLayout-01210"}},
    {VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
     {"VUID-VkImageMemoryBarrier-oldLayout-01211", "VUID-VkImageMemoryBarrier2KHR-oldLayout-01211"}},
    {VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
     {"VUID-VkImageMemoryBarrier-oldLayout-01212", "VUID-VkImageMemoryBarrier2KHR-oldLayout-01212"}},
    {VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
     {"VUID-VkImageMemoryBarrier-oldLayout-01213", "VUID-VkImageMemoryBarrier2KHR-oldLayout-01213"}},
    {VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV,
     {"VUID-VkImageMemoryBarrier-oldLayout-02088", "VUID-VkImageMemoryBarrier2KHR-oldLayout-02088"}},
    {VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
     {"VUID-VkImageMemoryBarrier-oldLayout-01658", "VUID-VkImageMemoryBarrier2KHR-oldLayout-01658"}},
    {VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
     {"VUID-VkImageMemoryBarrier-oldLayout-01659", "VUID-VkImageMemoryBarrier2KHR-oldLayout-01659"}},
};

const std::string &GetBadImageLayoutVUID(const CoreErrorLocation &loc, VkImageLayout layout) {
    const auto entry = kImageLayoutErrors.find(layout);
    assert(entry != kImageLayoutErrors.end());

    // NOTE we ignore field_name here these VUIDs always use oldLayout
    const auto &result = FindVUID(loc.refpage, Field::Empty, entry->second);
    assert(!result.empty());
    return result;
}

static const std::map<BufferError, std::array<std::string, 2>> kBufferErrors{
    {BufferError::kNoMemory, {"VUID-VkBufferMemoryBarrier2KHR-buffer-01931", "VUID-VkBufferMemoryBarrier-buffer-01931"}},
    {BufferError::kOffsetTooBig, {"VUID-VkBufferMemoryBarrier-offset-01187", "VUID-VkBufferMemoryBarrier2KHR-offset-01187"}},
    {BufferError::kSizeOutOfRange, {"VUID-VkBufferMemoryBarrier-size-01189", "VUID-VkBufferMemoryBarrier2KHR-size-01189"}},
    {BufferError::kSizeZero, {"VUID-VkBufferMemoryBarrier-size-01188", "VUID-VkBufferMemoryBarrier2KHR-size-01188"}},
};

const std::string &GetBufferBarrierVUID(const CoreErrorLocation &loc, BufferError error) {
    const auto entry = kBufferErrors.find(error);
    assert(entry != kBufferErrors.end());

    const auto &result = FindVUID(loc.refpage, Field::Empty, entry->second);
    assert(!result.empty());
    return result;
}

static const std::map<ImageError, std::vector<std::string>> kImageErrors{
    {ImageError::kNoMemory, {"VUID-VkImageMemoryBarrier-image-01932", "VUID-VkImageMemoryBarrier2KHR-image-01932"}},
    {ImageError::kConflictingLayout,
     {"VUID-VkImageMemoryBarrier-oldLayout-01197", "VUID-VkImageMemoryBarrier2KHR-oldLayout-01197"}},
    {ImageError::kBadLayout, {"VUID-VkImageMemoryBarrier-newLayout-01198", "VUID-VkImageMemoryBarrier2KHR-newLayout-01198"}},
    {ImageError::kNotColorAspect, {"VUID-VkImageMemoryBarrier-image-01671", "VUID-VkImageMemoryBarrier2KHR-image-01671"}},
    {ImageError::kNotColorAspectYcbcr, {"VUID-VkImageMemoryBarrier-image-02902", "VUID-VkImageMemoryBarrier2KHR-image-02902"}},
    {ImageError::kBadMultiplanarAspect, {"VUID-VkImageMemoryBarrier-image-01672", "VUID-VkImageMemoryBarrier2KHR-image-01672"}},
    {ImageError::kBadPlaneCount, {"VUID-VkImageMemoryBarrier-image-01673", "VUID-VkImageMemoryBarrier2KHR-image-01673"}},
    {ImageError::kNotDepthOrStencilAspect, {"VUID-VkImageMemoryBarrier-image-03319", "VUID-VkImageMemoryBarrier2KHR-image-03319"}},
    {ImageError::kNotDepthAndStencilAspect, {"VUID-VkImageMemoryBarrier-image-01207", "VUID-VkImageMemoryBarrier2KHR-image-01207"}},
    {ImageError::kNotSeparateDepthAndStencilAspect,
     {"VUID-VkImageMemoryBarrier-image-03320", "VUID-VkImageMemoryBarrier2KHR-image-03320"}},
    {ImageError::kRenderPassMismatch, {"VUID-vkCmdPipelineBarrier-image-04073", "VUID-vkCmdPipelineBarrier2KHR-image-04073"}},
    {ImageError::kRenderPassLayoutChange,
     {"VUID-vkCmdPipelineBarrier-oldLayout-01181", "VUID-vkCmdPipelineBarrier2KHR-oldLayout-01181"}},
};

const std::string &GetImageBarrierVUID(const CoreErrorLocation &loc, ImageError error) {
    const auto entry = kImageErrors.find(error);
    assert(entry != kImageErrors.end());

    const auto &result = FindVUID(loc.refpage, Field::Empty, entry->second);
    if (!result.empty()) {
        return result;
    }
    // hack to handle refpage vs. function mismatches in these VUIDS
    const auto &str_func = loc.StringFuncName();
    const auto &result2 = FindVUID(str_func, "", entry->second);
    assert(!result2.empty());
    return result2;
}

const SubresourceRangeErrorCodes& GetSubResourceVUIDs(const CoreErrorLocation &loc) {
    static const SubresourceRangeErrorCodes v1 {
        "VUID-VkImageMemoryBarrier-subresourceRange-01486",
        "VUID-VkImageMemoryBarrier-subresourceRange-01724",
        "VUID-VkImageMemoryBarrier-subresourceRange-01488",
        "VUID-VkImageMemoryBarrier-subresourceRange-01725",
    };
    static const SubresourceRangeErrorCodes v2 {
        "VUID-VkImageMemoryBarrier2KHR-subresourceRange-01486",
        "VUID-VkImageMemoryBarrier2KHR-subresourceRange-01724",
        "VUID-VkImageMemoryBarrier2KHR-subresourceRange-01488"
        "VUID-VkImageMemoryBarrier2KHR-subresourceRange-01725",
    };
    return (loc.refpage == RefPage::VkImageMemoryBarrier2KHR) ? v2 : v1;
}

static const std::map<SubmitError, std::vector<std::string>> kSubmitErrors{
    {SubmitError::kTimelineSemSmallValue,
     {
         "VUID-VkSemaphoreSignalInfo-value-03258",
         "VUID-VkBindSparseInfo-pSignalSemaphores-03249",
         "VUID-VkSubmitInfo-pSignalSemaphores-03242",
         "VUID-VkSubmitInfo2KHR-semaphore-03881",
         "VUID-VkSubmitInfo2KHR-semaphore-03882",
     }},
    {SubmitError::kSemAlreadySignalled,
     {
         "VUID-vkQueueSubmit-pSignalSemaphores-00067",
         "VUID-vkQueueBindSparse-pSignalSemaphores-01115",
         "VUID-vkQueueSubmit2KHR-semaphore-03868",
     }},
    {SubmitError::kBinaryCannotBeSignalled,
     {
         "VUID-vkQueueSubmit-pWaitSemaphores-00069",
         "VUID-vkQueueSubmit2KHR-semaphore-03872",
     }},
    {SubmitError::kTimelineCannotBeSignalled,
     {
         "VUID-vkQueueSubmit-pWaitSemaphores-03238",
         "VUID-vkQueueSubmit2KHR-semaphore-03873",
     }},
    {SubmitError::kTimelineSemMaxDiff,
     {
         "VUID-VkBindSparseInfo-pWaitSemaphores-03250",
         "VUID-VkBindSparseInfo-pSignalSemaphores-03251",
         "VUID-VkSubmitInfo-pWaitSemaphores-03243",
         "VUID-VkSubmitInfo-pSignalSemaphores-03244",
         "VUID-VkSemaphoreSignalInfo-value-03260",
         "VUID-VkSubmitInfo2KHR-semaphore-03883",
         "VUID-VkSubmitInfo2KHR-semaphore-03884",
     }},
    {SubmitError::kProtectedFeatureDisabled,
     {
         "VUID-VkProtectedSubmitInfo-protectedSubmit-01816",
         "VUID-VkSubmitInfo2KHR-flags-03885",
     }},
    {SubmitError::kBadUnprotectedSubmit,
     {
         "VUID-VkSubmitInfo-pNext-04148",
         "VUID-VkSubmitInfo2KHR-flags-03886",
     }},
    {SubmitError::kBadProtectedSubmit,
     {
         "VUID-VkSubmitInfo-pNext-04120",
         "VUID-VkSubmitInfo2KHR-flags-03887",
     }},
    {SubmitError::kCmdNotSimultaneous,
     {
         "VUID-vkQueueSubmit-pCommandBuffers-00071",
         "VUID-vkQueueSubmit2KHR-commandBuffer-03875",
     }},
    {SubmitError::kReusedOneTimeCmd,
     {
         "VUID-vkQueueSubmit-pCommandBuffers-00072",
         "VUID-vkQueueSubmit2KHR-commandBuffer-03876",
     }},
    {SubmitError::kSecondaryCmdNotSimultaneous,
     {
         "VUID-vkQueueSubmit-pCommandBuffers-00073",
         "VUID-vkQueueSubmit2KHR-commandBuffer-03877",
     }},
    {SubmitError::kCmdWrongQueueFamily,
     {
         "VUID-vkQueueSubmit-pCommandBuffers-00074",
         "VUID-vkQueueSubmit2KHR-commandBuffer-03878",
     }},
    {SubmitError::kSecondaryCmdInSubmit,
     {
         "VUID-VkSubmitInfo-pCommandBuffers-00075",
         "VUID-VkCommandBufferSubmitInfoKHR-commandBuffer-03890",
     }},
    {SubmitError::kHostStageMask,
     {
         "VUID-VkSubmitInfo-pWaitDstStageMask-00078",
         "VUID-vkCmdSetEvent-stageMask-01149",
         "VUID-vkCmdResetEvent-stageMask-01153",
         "VUID-vkCmdResetEvent2KHR-stageMask-03830",
     }},
};

const std::string &GetQueueSubmitVUID(const CoreErrorLocation &loc, SubmitError error) {
    const auto entry = kSubmitErrors.find(error);
    assert(entry != kSubmitErrors.end());

    const auto &ref_result = FindVUID(loc.refpage, loc.field_name, entry->second);
    if (!ref_result.empty()) {
        return ref_result;
    }
    const auto &result = FindVUID(loc.StringFuncName(), loc.StringField(), entry->second);
    assert(!result.empty());
    return result;
}

};  // namespace sync_vuid_maps
