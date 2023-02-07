/* Copyright (c) 2021-2022 The Khronos Group Inc.
 * Copyright (c) 2021-2023 Valve Corporation
 * Copyright (c) 2021-2023 LunarG, Inc.
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
 */
#include "core_error_location.h"
#include <map>

namespace core_error {
#define FUNC_ENTRY(_v) \
    { Func::_v, #_v }
const std::string& String(Func func) {
    static const std::map<Func, std::string> table{
        {Func::Empty, ""},
        FUNC_ENTRY(vkQueueSubmit),
        FUNC_ENTRY(vkQueueSubmit2),
        FUNC_ENTRY(vkCmdSetEvent),
        FUNC_ENTRY(vkCmdSetEvent2),
        FUNC_ENTRY(vkCmdResetEvent),
        FUNC_ENTRY(vkCmdResetEvent2),
        FUNC_ENTRY(vkCmdPipelineBarrier),
        FUNC_ENTRY(vkCmdPipelineBarrier2),
        FUNC_ENTRY(vkCmdWaitEvents),
        FUNC_ENTRY(vkCmdWaitEvents2),
        FUNC_ENTRY(vkCmdWriteTimestamp),
        FUNC_ENTRY(vkCmdWriteTimestamp2),
        FUNC_ENTRY(vkCreateRenderPass),
        FUNC_ENTRY(vkCreateRenderPass2),
        FUNC_ENTRY(vkQueueBindSparse),
        FUNC_ENTRY(vkSignalSemaphore),
        FUNC_ENTRY(vkQueuePresentKHR),
    };
    const auto entry = table.find(func);
    assert(entry != table.end());
    return entry->second;
}

#define STRUCT_ENTRY(_v) \
    { Struct::_v, #_v }
const std::string& String(Struct structure) {
    static const std::map<Struct, std::string> table{
        {Struct::Empty, ""},
        STRUCT_ENTRY(VkMemoryBarrier),
        STRUCT_ENTRY(VkMemoryBarrier2),
        STRUCT_ENTRY(VkBufferMemoryBarrier),
        STRUCT_ENTRY(VkImageMemoryBarrier),
        STRUCT_ENTRY(VkBufferMemoryBarrier2),
        STRUCT_ENTRY(VkImageMemoryBarrier2),
        STRUCT_ENTRY(VkSubmitInfo),
        STRUCT_ENTRY(VkSubmitInfo2),
        STRUCT_ENTRY(VkCommandBufferSubmitInfo),
        STRUCT_ENTRY(VkSubpassDependency),
        STRUCT_ENTRY(VkSubpassDependency2),
        STRUCT_ENTRY(VkBindSparseInfo),
        STRUCT_ENTRY(VkSemaphoreSignalInfo),
        STRUCT_ENTRY(VkPresentInfoKHR),
    };
    const auto entry = table.find(structure);
    assert(entry != table.end());
    return entry->second;
}

#define FIELD_ENTRY(_v) \
    { Field::_v, #_v }
const std::string& String(Field field) {
    static const std::map<Field, std::string> table{
        {Field::Empty, ""},
        FIELD_ENTRY(oldLayout),
        FIELD_ENTRY(newLayout),
        FIELD_ENTRY(image),
        FIELD_ENTRY(buffer),
        FIELD_ENTRY(pMemoryBarriers),
        FIELD_ENTRY(pBufferMemoryBarriers),
        FIELD_ENTRY(pImageMemoryBarriers),
        FIELD_ENTRY(offset),
        FIELD_ENTRY(size),
        FIELD_ENTRY(subresourceRange),
        FIELD_ENTRY(srcAccessMask),
        FIELD_ENTRY(dstAccessMask),
        FIELD_ENTRY(srcStageMask),
        FIELD_ENTRY(dstStageMask),
        FIELD_ENTRY(pNext),
        FIELD_ENTRY(pWaitDstStageMask),
        FIELD_ENTRY(pWaitSemaphores),
        FIELD_ENTRY(pSignalSemaphores),
        FIELD_ENTRY(pWaitSemaphoreInfos),
        FIELD_ENTRY(pWaitSemaphoreValues),
        FIELD_ENTRY(pSignalSemaphoreInfos),
        FIELD_ENTRY(pSignalSemaphoreValues),
        FIELD_ENTRY(stage),
        FIELD_ENTRY(stageMask),
        FIELD_ENTRY(value),
        FIELD_ENTRY(pCommandBuffers),
        FIELD_ENTRY(pSubmits),
        FIELD_ENTRY(pCommandBufferInfos),
        FIELD_ENTRY(semaphore),
        FIELD_ENTRY(commandBuffer),
        FIELD_ENTRY(dependencyFlags),
        FIELD_ENTRY(pDependencyInfo),
        FIELD_ENTRY(pDependencyInfos),
        FIELD_ENTRY(srcQueueFamilyIndex),
        FIELD_ENTRY(dstQueueFamilyIndex),
        FIELD_ENTRY(queryPool),
        FIELD_ENTRY(pDependencies),
        FIELD_ENTRY(pipelineStage),
    };
    const auto entry = table.find(field);
    assert(entry != table.end());
    return entry->second;
}

LocationCapture::LocationCapture(const Location& loc) { Capture(loc, 1); }

const Location* LocationCapture::Capture(const Location& loc, CaptureStore::size_type depth) {
    const Location* prev_capture = nullptr;
    if (loc.prev) {
        prev_capture = Capture(*loc.prev, depth + 1);
    } else {
        capture.reserve(depth);
    }

    capture.emplace_back(loc);
    capture.back().prev = prev_capture;
    return &(capture.back());
}

void Location::AppendFields(std::ostream& out) const {
    if (prev) {
        prev->AppendFields(out);
        out << ".";
    }
    out << String(field);
    if (index != Location::kNoIndex) {
        out << "[" << index << "]";
    }
}

bool operator==(const Key& key, const Location& loc) {
    assert(key.function != Func::Empty || key.structure != Struct::Empty);
    assert(loc.function != Func::Empty);
    if (key.function != Func::Empty) {
        if (key.function != loc.function) {
            return false;
        }
    }
    if (key.structure != Struct::Empty) {
        if (key.structure != loc.structure) {
            return false;
        }
    }
    if (key.field == Field::Empty) {
        return true;
    }
    if (key.field == loc.field) {
        return true;
    }
    if (key.recurse_field) {
        const Location *prev = loc.prev;
        while (prev != nullptr) {
            if (key.field == prev->field) {
                return true;
            }
            prev = prev->prev;
        }
    }
    return false;
}

}  // namespace core_error
