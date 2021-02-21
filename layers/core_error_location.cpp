#include "core_error_location.h"
#include <map>

#define FUNC_ENTRY(_v) \
    { ErrFunc::_v, #_v }
const std::string& CoreErrorLocation::String(ErrFunc func) {
    static const std::map<ErrFunc, std::string> table{
        {ErrFunc::Empty, ""},
        FUNC_ENTRY(vkQueueSubmit),
        FUNC_ENTRY(vkQueueSubmit2KHR),
        FUNC_ENTRY(vkCmdSetEvent),
        FUNC_ENTRY(vkCmdSetEvent2KHR),
        FUNC_ENTRY(vkCmdResetEvent),
        FUNC_ENTRY(vkCmdResetEvent2KHR),
        FUNC_ENTRY(vkCmdPipelineBarrier),
        FUNC_ENTRY(vkCmdPipelineBarrier2KHR),
        FUNC_ENTRY(vkCmdWaitEvents),
        FUNC_ENTRY(vkCmdWaitEvents2KHR),
        FUNC_ENTRY(vkCmdWriteTimestamp2),
        FUNC_ENTRY(vkCmdWriteTimestamp2KHR),
        FUNC_ENTRY(vkCreateRenderPass),
        FUNC_ENTRY(vkCreateRenderPass2),
        FUNC_ENTRY(vkQueueBindSparse),
        FUNC_ENTRY(vkSignalSemaphore),
    };
    const auto entry = table.find(func);
    assert(entry != table.end());
    return entry->second;
}

#define REFPAGE_ENTRY(_v) \
    { RefPage::_v, #_v }
const std::string& CoreErrorLocation::String(RefPage refpage) {
    static const std::map<RefPage, std::string> table{
        {RefPage::Empty, ""},
        REFPAGE_ENTRY(VkMemoryBarrier),
        REFPAGE_ENTRY(VkMemoryBarrier2KHR),
        REFPAGE_ENTRY(VkBufferMemoryBarrier),
        REFPAGE_ENTRY(VkImageMemoryBarrier),
        REFPAGE_ENTRY(VkBufferMemoryBarrier2KHR),
        REFPAGE_ENTRY(VkImageMemoryBarrier2KHR),
        REFPAGE_ENTRY(VkSubmitInfo),
        REFPAGE_ENTRY(VkSubmitInfo2KHR),
        REFPAGE_ENTRY(VkCommandBufferSubmitInfoKHR),
        REFPAGE_ENTRY(vkCmdSetEvent),
        REFPAGE_ENTRY(vkCmdSetEvent2KHR),
        REFPAGE_ENTRY(vkCmdResetEvent),
        REFPAGE_ENTRY(vkCmdResetEvent2KHR),
        REFPAGE_ENTRY(vkCmdPipelineBarrier),
        REFPAGE_ENTRY(vkCmdPipelineBarrier2KHR),
        REFPAGE_ENTRY(vkCmdWaitEvents),
        REFPAGE_ENTRY(vkCmdWaitEvents2KHR),
        REFPAGE_ENTRY(vkCmdWriteTimestamp2),
        REFPAGE_ENTRY(vkCmdWriteTimestamp2KHR),
        REFPAGE_ENTRY(VkSubpassDependency),
        REFPAGE_ENTRY(VkSubpassDependency2),
        REFPAGE_ENTRY(VkBindSparseInfo),
        REFPAGE_ENTRY(VkSemaphoreSignalInfo),
    };
    const auto entry = table.find(refpage);
    assert(entry != table.end());
    return entry->second;
}

#define FIELD_ENTRY(_v) \
    { Field::_v, #_v }
const std::string& CoreErrorLocation::String(Field field) {
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
    };
    const auto entry = table.find(field);
    assert(entry != table.end());
    return entry->second;
}

CoreErrorLocationCapture::CoreErrorLocationCapture(const CoreErrorLocation& loc) { Capture(loc, 1); }

const CoreErrorLocation* CoreErrorLocationCapture::Capture(const CoreErrorLocation& loc, CaptureStore::size_type depth) {
    const CoreErrorLocation* prev_capture = nullptr;
    if (loc.prev) {
        prev_capture = Capture(*loc.prev, depth + 1);
    } else {
        capture.reserve(depth);
    }

    capture.emplace_back(loc);
    capture.back().prev = prev_capture;
    return &(capture.back());
}

void CoreErrorLocation::AppendFields(std::stringstream* out) const {
    if (prev) {
        prev->AppendFields(out);
        *out << ".";
    }
    *out << String(field_name);
    if (index != CoreErrorLocation::kNoIndex) {
        *out << "[" << index << "]";
    }
}
