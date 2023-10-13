/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
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

#include "binding.h"

#include <string.h>  // memset(), memcmp()
#include <cassert>

#include <vulkan/utility/vk_format_utils.h>
#include <vulkan/utility/vk_struct_helper.hpp>

#define NON_DISPATCHABLE_HANDLE_INIT(create_func, dev, ...)                                                \
    do {                                                                                                   \
        assert(!initialized());                                                                            \
        handle_type handle;                                                                                \
        auto result = create_func(dev.handle(), __VA_ARGS__, NULL, &handle);                               \
        ASSERT_TRUE((result == VK_SUCCESS) || (result == VK_ERROR_VALIDATION_FAILED_EXT) ||                \
                    (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) || (result == VK_ERROR_OUT_OF_HOST_MEMORY)); \
        if (result == VK_SUCCESS) {                                                                        \
            NonDispHandle::init(dev.handle(), handle);                                                     \
        }                                                                                                  \
    } while (0)

#define NON_DISPATCHABLE_HANDLE_DTOR(cls, destroy_func)        \
    void cls::destroy() noexcept {                             \
        if (!initialized()) {                                  \
            return;                                            \
        }                                                      \
        destroy_func(device(), handle(), NULL);                \
        handle_ = VK_NULL_HANDLE;                              \
        internal::NonDispHandle<decltype(handle_)>::destroy(); \
    }                                                          \
    cls::~cls() noexcept { destroy(); }

namespace vkt {

VkPhysicalDeviceProperties PhysicalDevice::properties() const {
    VkPhysicalDeviceProperties info;

    vk::GetPhysicalDeviceProperties(handle(), &info);

    return info;
}

std::vector<VkQueueFamilyProperties> PhysicalDevice::queue_properties() const {
    std::vector<VkQueueFamilyProperties> info;
    uint32_t count;

    // Call once with NULL data to receive count
    vk::GetPhysicalDeviceQueueFamilyProperties(handle(), &count, NULL);
    info.resize(count);
    vk::GetPhysicalDeviceQueueFamilyProperties(handle(), &count, info.data());

    return info;
}

VkPhysicalDeviceMemoryProperties PhysicalDevice::memory_properties() const {
    VkPhysicalDeviceMemoryProperties info;

    vk::GetPhysicalDeviceMemoryProperties(handle(), &info);

    return info;
}

VkPhysicalDeviceFeatures PhysicalDevice::features() const {
    VkPhysicalDeviceFeatures features;
    vk::GetPhysicalDeviceFeatures(handle(), &features);
    return features;
}

/*
 * Return list of Global layers available
 */
std::vector<VkLayerProperties> GetGlobalLayers() {
    VkResult err;
    uint32_t layer_count = 32;
    std::vector<VkLayerProperties> layers(layer_count);
    for (;;) {
        err = vk::EnumerateInstanceLayerProperties(&layer_count, layers.data());
        if (err == VK_SUCCESS) {
            layers.resize(layer_count);
            return layers;
        } else if (err == VK_INCOMPLETE) {
            layer_count *= 2;  // wasn't enough space, increase it
            layers.resize(layer_count);
        } else {
            return {};
        }
    }
}

/*
 * Return list of Global extensions provided by the ICD / Loader
 */
std::vector<VkExtensionProperties> GetGlobalExtensions() { return GetGlobalExtensions(nullptr); }

/*
 * Return list of Global extensions provided by the specified layer
 * If pLayerName is NULL, will return extensions implemented by the loader /
 * ICDs
 */
std::vector<VkExtensionProperties> GetGlobalExtensions(const char *pLayerName) {
    VkResult err;
    uint32_t extension_count = 32;
    std::vector<VkExtensionProperties> extensions(extension_count);
    for (;;) {
        err = vk::EnumerateInstanceExtensionProperties(pLayerName, &extension_count, extensions.data());
        if (err == VK_SUCCESS) {
            extensions.resize(extension_count);
            return extensions;
        } else if (err == VK_INCOMPLETE) {
            extension_count *= 2;  // wasn't enough space, increase it
            extensions.resize(extension_count);
        } else {
            return {};
        }
    }
}

/*
 * Return list of PhysicalDevice extensions provided by the specified layer
 * If pLayerName is NULL, will return extensions for ICD / loader.
 */
std::vector<VkExtensionProperties> PhysicalDevice::extensions(const char *pLayerName) const {
    VkResult err;
    uint32_t extension_count = 512;
    std::vector<VkExtensionProperties> extensions(extension_count);
    for (;;) {
        err = vk::EnumerateDeviceExtensionProperties(handle(), pLayerName, &extension_count, extensions.data());
        if (err == VK_SUCCESS) {
            extensions.resize(extension_count);
            return extensions;
        } else if (err == VK_INCOMPLETE) {
            extension_count *= 2;  // wasn't enough space, increase it
            extensions.resize(extension_count);
        } else {
            return {};
        }
    }
}

bool PhysicalDevice::set_memory_type(const uint32_t type_bits, VkMemoryAllocateInfo *info, const VkFlags properties,
                                     const VkFlags forbid) const {
    uint32_t type_mask = type_bits;
    // Search memtypes to find first index with those properties
    for (uint32_t i = 0; i < memory_properties_.memoryTypeCount; i++) {
        if ((type_mask & 1) == 1) {
            // Type is available, does it match user properties?
            if ((memory_properties_.memoryTypes[i].propertyFlags & properties) == properties &&
                (memory_properties_.memoryTypes[i].propertyFlags & forbid) == 0 &&
                (memory_properties_.memoryHeaps[memory_properties_.memoryTypes[i].heapIndex].size >= info->allocationSize)) {
                info->memoryTypeIndex = i;
                return true;
            }
        }
        type_mask >>= 1;
    }
    // No memory types matched, return failure
    return false;
}

/*
 * Return list of PhysicalDevice layers
 */
std::vector<VkLayerProperties> PhysicalDevice::layers() const {
    VkResult err;
    uint32_t layer_count = 32;
    std::vector<VkLayerProperties> layers(layer_count);
    for (;;) {
        err = vk::EnumerateDeviceLayerProperties(handle(), &layer_count, layers.data());
        if (err == VK_SUCCESS) {
            layers.resize(layer_count);
            return layers;
        } else if (err == VK_INCOMPLETE) {
            layer_count *= 2;  // wasn't enough space, increase it
            layers.resize(layer_count);
        } else {
            return {};
        }
    }
}

QueueCreateInfoArray::QueueCreateInfoArray(const std::vector<VkQueueFamilyProperties> &queue_props)
    : queue_info_(), queue_priorities_() {
    queue_info_.reserve(queue_props.size());

    for (uint32_t i = 0; i < (uint32_t)queue_props.size(); ++i) {
        if (queue_props[i].queueCount > 0) {
            VkDeviceQueueCreateInfo qi = vku::InitStructHelper();
            qi.queueFamilyIndex = i;
            qi.queueCount = queue_props[i].queueCount;
            queue_priorities_.emplace_back(qi.queueCount, 0.0f);
            qi.pQueuePriorities = queue_priorities_[i].data();
            queue_info_.push_back(qi);
        }
    }
}

void Device::destroy() noexcept {
    if (!initialized()) return;
    vk::DestroyDevice(handle(), NULL);
    handle_ = VK_NULL_HANDLE;
}

Device::~Device() noexcept { destroy(); }

void Device::init(std::vector<const char *> &extensions, VkPhysicalDeviceFeatures *features, void *create_device_pnext) {
    // request all queues
    QueueCreateInfoArray queue_info(phy_.queue_properties_);
    for (uint32_t i = 0; i < (uint32_t)phy_.queue_properties_.size(); i++) {
        if (phy_.queue_properties_[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphics_queue_node_index_ = i;
            break;
        }
    }
    // Only request creation with queuefamilies that have at least one queue
    std::vector<VkDeviceQueueCreateInfo> create_queue_infos;
    auto qci = queue_info.data();
    for (uint32_t j = 0; j < queue_info.size(); ++j) {
        if (qci[j].queueCount) {
            create_queue_infos.push_back(qci[j]);
        }
    }

    enabled_extensions_ = extensions;

    VkDeviceCreateInfo dev_info = vku::InitStructHelper(create_device_pnext);
    dev_info.queueCreateInfoCount = static_cast<uint32_t>(create_queue_infos.size());
    dev_info.pQueueCreateInfos = create_queue_infos.data();
    dev_info.enabledLayerCount = 0;
    dev_info.ppEnabledLayerNames = NULL;
    dev_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    dev_info.ppEnabledExtensionNames = extensions.data();

    VkPhysicalDeviceFeatures all_features;
    // Let VkPhysicalDeviceFeatures2 take priority over VkPhysicalDeviceFeatures,
    // since it supports extensions

    if (!(vku::FindStructInPNextChain<VkPhysicalDeviceFeatures2>(dev_info.pNext))) {
        if (features) {
            dev_info.pEnabledFeatures = features;
        } else {
            // request all supportable features enabled
            all_features = phy().features();
            dev_info.pEnabledFeatures = &all_features;
        }
    }

    init(dev_info);
}

void Device::init(const VkDeviceCreateInfo &info) {
    VkDevice dev;

    ASSERT_EQ(VK_SUCCESS, vk::CreateDevice(phy_.handle(), &info, NULL, &dev));
    Handle::init(dev);

    init_queues(info);
    init_formats();
}

void Device::init_queues(const VkDeviceCreateInfo &info) {
    uint32_t queue_node_count = phy_.queue_properties_.size();

    queue_families_.resize(queue_node_count);
    for (uint32_t i = 0; i < info.queueCreateInfoCount; i++) {
        const auto &queue_create_info = info.pQueueCreateInfos[i];
        auto queue_family_i = queue_create_info.queueFamilyIndex;
        const auto &queue_family_prop = phy_.queue_properties_[queue_family_i];

        QueueFamilyQueues &queue_storage = queue_families_[queue_family_i];
        queue_storage.reserve(queue_create_info.queueCount);
        for (uint32_t queue_i = 0; queue_i < queue_create_info.queueCount; ++queue_i) {
            // TODO: Need to add support for separate MEMMGR and work queues,
            // including synchronization
            VkQueue queue = VK_NULL_HANDLE;
            vk::GetDeviceQueue(handle(), queue_family_i, queue_i, &queue);

            // Store single copy of the queue object that will self destruct
            queue_storage.emplace_back(new Queue(queue, queue_family_i));

            if (queue_family_prop.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                queues_[GRAPHICS].push_back(queue_storage.back().get());
            }

            if (queue_family_prop.queueFlags & VK_QUEUE_COMPUTE_BIT) {
                queues_[COMPUTE].push_back(queue_storage.back().get());
            }

            if (queue_family_prop.queueFlags & VK_QUEUE_TRANSFER_BIT) {
                queues_[DMA].push_back(queue_storage.back().get());
            }
        }
    }

    ASSERT_TRUE(!queues_[GRAPHICS].empty() || !queues_[COMPUTE].empty() || !queues_[DMA].empty());
}

const Device::QueueFamilyQueues &Device::queue_family_queues(uint32_t queue_family) const {
    assert(queue_family < queue_families_.size());
    return queue_families_[queue_family];
}

std::optional<uint32_t> Device::QueueFamilyMatching(VkQueueFlags with, VkQueueFlags without, bool all_bits) {
    for (uint32_t i = 0; i < phy_.queue_properties_.size(); i++) {
        const auto flags = phy_.queue_properties_[i].queueFlags;
        const bool matches = all_bits ? (flags & with) == with : (flags & with) != 0;
        if (matches && ((flags & without) == 0) && (phy_.queue_properties_[i].queueCount > 0)) {
            return i;
        }
    }
    return {};
}

void Device::init_formats() {
    // For each 1.0 core format, undefined = first, 12x12_SRGB_BLOCK = last
    for (int f = VK_FORMAT_UNDEFINED; f <= VK_FORMAT_ASTC_12x12_SRGB_BLOCK; f++) {
        const VkFormat fmt = static_cast<VkFormat>(f);
        const VkFormatProperties format_props = format_properties(fmt);

        if (format_props.linearTilingFeatures) {
            const Format tmp = {fmt, VK_IMAGE_TILING_LINEAR, format_props.linearTilingFeatures};
            formats_.push_back(tmp);
        }

        if (format_props.optimalTilingFeatures) {
            const Format tmp = {fmt, VK_IMAGE_TILING_OPTIMAL, format_props.optimalTilingFeatures};
            formats_.push_back(tmp);
        }
    }

    ASSERT_TRUE(!formats_.empty());
}

bool Device::IsEnabledExtension(const char *extension) {
    const auto is_x = [&extension](const char *enabled_extension) { return strcmp(extension, enabled_extension) == 0; };
    return std::any_of(enabled_extensions_.begin(), enabled_extensions_.end(), is_x);
}

VkFormatProperties Device::format_properties(VkFormat format) {
    VkFormatProperties data;
    vk::GetPhysicalDeviceFormatProperties(phy().handle(), format, &data);

    return data;
}

void Device::wait() { ASSERT_EQ(VK_SUCCESS, vk::DeviceWaitIdle(handle())); }

VkResult Device::wait(const std::vector<const Fence *> &fences, bool wait_all, uint64_t timeout) {
    const std::vector<VkFence> fence_handles = MakeVkHandles<VkFence>(fences);
    VkResult err =
        vk::WaitForFences(handle(), static_cast<uint32_t>(fence_handles.size()), fence_handles.data(), wait_all, timeout);
    EXPECT_TRUE(err == VK_SUCCESS || err == VK_TIMEOUT);

    return err;
}

void Device::update_descriptor_sets(const std::vector<VkWriteDescriptorSet> &writes,
                                    const std::vector<VkCopyDescriptorSet> &copies) {
    vk::UpdateDescriptorSets(handle(), static_cast<uint32_t>(writes.size()), writes.data(), static_cast<uint32_t>(copies.size()),
                             copies.data());
}

VkResult Queue::submit(const std::vector<const CommandBuffer *> &cmds, const Fence &fence, bool expect_success) {
    const std::vector<VkCommandBuffer> cmd_handles = MakeVkHandles<VkCommandBuffer>(cmds);
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = nullptr;
    submit_info.pWaitDstStageMask = nullptr;
    submit_info.commandBufferCount = static_cast<uint32_t>(cmd_handles.size());
    submit_info.pCommandBuffers = cmd_handles.data();
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = nullptr;

    VkResult result = vk::QueueSubmit(handle(), 1, &submit_info, fence.handle());
    if (expect_success) {
        EXPECT_EQ(VK_SUCCESS, result);
    }
    return result;
}

VkResult Queue::submit(const CommandBuffer &cmd, const Fence &fence, bool expect_success) {
    return submit(std::vector<const CommandBuffer *>(1, &cmd), fence, expect_success);
}

VkResult Queue::submit(const CommandBuffer &cmd, bool expect_success) {
    Fence fence;
    return submit(cmd, fence, expect_success);
}

VkResult Queue::submit2(const std::vector<const CommandBuffer *> &cmds, const Fence &fence, bool expect_success) {
    std::vector<VkCommandBufferSubmitInfo> cmd_submit_infos;
    for (size_t i = 0; i < cmds.size(); i++) {
        VkCommandBufferSubmitInfo cmd_submit_info = vku::InitStructHelper();
        cmd_submit_info.deviceMask = 0;
        cmd_submit_info.commandBuffer = cmds[i]->handle();
        cmd_submit_infos.push_back(cmd_submit_info);
    }

    VkSubmitInfo2 submit_info = vku::InitStructHelper();
    submit_info.flags = 0;
    submit_info.waitSemaphoreInfoCount = 0;
    submit_info.pWaitSemaphoreInfos = nullptr;
    submit_info.signalSemaphoreInfoCount = 0;
    submit_info.pSignalSemaphoreInfos = nullptr;
    submit_info.commandBufferInfoCount = static_cast<uint32_t>(cmd_submit_infos.size());
    submit_info.pCommandBufferInfos = cmd_submit_infos.data();

    // requires synchronization2 to be enabled
    VkResult result = vk::QueueSubmit2(handle(), 1, &submit_info, fence.handle());
    if (expect_success) {
        EXPECT_EQ(VK_SUCCESS, result);
    }
    return result;
}

VkResult Queue::submit2(const CommandBuffer &cmd, const Fence &fence, bool expect_success) {
    return submit2(std::vector<const CommandBuffer *>(1, &cmd), fence, expect_success);
}

VkResult Queue::wait() {
    VkResult result = vk::QueueWaitIdle(handle());
    EXPECT_EQ(VK_SUCCESS, result);
    return result;
}

void DeviceMemory::destroy() noexcept {
    if (!initialized()) {
        return;
    }

    vk::FreeMemory(device(), handle(), NULL);
    handle_ = VK_NULL_HANDLE;
}

DeviceMemory::~DeviceMemory() noexcept { destroy(); }

void DeviceMemory::init(const Device &dev, const VkMemoryAllocateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::AllocateMemory, dev, &info);
    memory_allocate_info_ = info;
}

VkResult DeviceMemory::try_init(const Device &dev, const VkMemoryAllocateInfo &info) {
    assert(!initialized());
    VkDeviceMemory handle = VK_NULL_HANDLE;
    auto result = vk::AllocateMemory(dev.handle(), &info, nullptr, &handle);
    if (result == VK_SUCCESS) {
        NonDispHandle::init(dev.handle(), handle);
    }
    return result;
}

const void *DeviceMemory::map(VkFlags flags) const {
    void *data;
    VkResult result = vk::MapMemory(device(), handle(), 0, VK_WHOLE_SIZE, flags, &data);
    EXPECT_EQ(VK_SUCCESS, result);
    if (result != VK_SUCCESS) data = NULL;
    return data;
}

void *DeviceMemory::map(VkFlags flags) {
    void *data;
    VkResult result = vk::MapMemory(device(), handle(), 0, VK_WHOLE_SIZE, flags, &data);
    EXPECT_EQ(VK_SUCCESS, result);
    if (result != VK_SUCCESS) data = NULL;

    return data;
}

void DeviceMemory::unmap() const { vk::UnmapMemory(device(), handle()); }

VkMemoryAllocateInfo DeviceMemory::get_resource_alloc_info(const Device &dev, const VkMemoryRequirements &reqs,
                                                           VkMemoryPropertyFlags mem_props, void *alloc_info_pnext) {
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(alloc_info_pnext);
    alloc_info.allocationSize = reqs.size;
    EXPECT_TRUE(dev.phy().set_memory_type(reqs.memoryTypeBits, &alloc_info, mem_props));
    return alloc_info;
}

NON_DISPATCHABLE_HANDLE_DTOR(Fence, vk::DestroyFence)

void Fence::init(const Device &dev, const VkFenceCreateInfo &info) { NON_DISPATCHABLE_HANDLE_INIT(vk::CreateFence, dev, &info); }

VkResult Fence::wait(uint64_t timeout) const {
    VkFence fence = handle();
    return vk::WaitForFences(device(), 1, &fence, VK_TRUE, timeout);
}

VkResult Fence::reset() {
    VkFence fence = handle();
    return vk::ResetFences(device(), 1, &fence);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
VkResult Fence::export_handle(HANDLE &win32_handle, VkExternalFenceHandleTypeFlagBits handle_type) {
    VkFenceGetWin32HandleInfoKHR ghi = vku::InitStructHelper();
    ghi.fence = handle();
    ghi.handleType = handle_type;
    return vk::GetFenceWin32HandleKHR(device(), &ghi, &win32_handle);
}

VkResult Fence::import_handle(HANDLE win32_handle, VkExternalFenceHandleTypeFlagBits handle_type, VkFenceImportFlags flags) {
    VkImportFenceWin32HandleInfoKHR ifi = vku::InitStructHelper();
    ifi.fence = handle();
    ifi.handleType = handle_type;
    ifi.handle = win32_handle;
    ifi.flags = flags;
    return vk::ImportFenceWin32HandleKHR(device(), &ifi);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

VkResult Fence::export_handle(int &fd_handle, VkExternalFenceHandleTypeFlagBits handle_type) {
    VkFenceGetFdInfoKHR gfi = vku::InitStructHelper();
    gfi.fence = handle();
    gfi.handleType = handle_type;
    return vk::GetFenceFdKHR(device(), &gfi, &fd_handle);
}

VkResult Fence::import_handle(int fd_handle, VkExternalFenceHandleTypeFlagBits handle_type, VkFenceImportFlags flags) {
    VkImportFenceFdInfoKHR ifi = vku::InitStructHelper();
    ifi.fence = handle();
    ifi.handleType = handle_type;
    ifi.fd = fd_handle;
    ifi.flags = flags;
    return vk::ImportFenceFdKHR(device(), &ifi);
}

NON_DISPATCHABLE_HANDLE_DTOR(Semaphore, vk::DestroySemaphore)

void Semaphore::init(const Device &dev, const VkSemaphoreCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateSemaphore, dev, &info);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
VkResult Semaphore::export_handle(HANDLE &win32_handle, VkExternalSemaphoreHandleTypeFlagBits handle_type) {
    win32_handle = nullptr;
    VkSemaphoreGetWin32HandleInfoKHR ghi = vku::InitStructHelper();
    ghi.semaphore = handle();
    ghi.handleType = handle_type;
    return vk::GetSemaphoreWin32HandleKHR(device(), &ghi, &win32_handle);
}

VkResult Semaphore::import_handle(HANDLE win32_handle, VkExternalSemaphoreHandleTypeFlagBits handle_type,
                                  VkSemaphoreImportFlags flags) {
    VkImportSemaphoreWin32HandleInfoKHR ihi = vku::InitStructHelper();
    ihi.semaphore = handle();
    ihi.handleType = handle_type;
    ihi.handle = win32_handle;
    ihi.flags = flags;
    return vk::ImportSemaphoreWin32HandleKHR(device(), &ihi);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

VkResult Semaphore::export_handle(int &fd_handle, VkExternalSemaphoreHandleTypeFlagBits handle_type) {
    fd_handle = -1;
    VkSemaphoreGetFdInfoKHR ghi = vku::InitStructHelper();
    ghi.semaphore = handle();
    ghi.handleType = handle_type;
    return vk::GetSemaphoreFdKHR(device(), &ghi, &fd_handle);
}

VkResult Semaphore::import_handle(int fd_handle, VkExternalSemaphoreHandleTypeFlagBits handle_type, VkSemaphoreImportFlags flags) {
    // Import opaque handle exported above
    VkImportSemaphoreFdInfoKHR ihi = vku::InitStructHelper();
    ihi.semaphore = handle();
    ihi.handleType = handle_type;
    ihi.fd = fd_handle;
    ihi.flags = flags;
    return vk::ImportSemaphoreFdKHR(device(), &ihi);
}

NON_DISPATCHABLE_HANDLE_DTOR(Event, vk::DestroyEvent)

void Event::init(const Device &dev, const VkEventCreateInfo &info) { NON_DISPATCHABLE_HANDLE_INIT(vk::CreateEvent, dev, &info); }

void Event::set() { ASSERT_EQ(VK_SUCCESS, vk::SetEvent(device(), handle())); }

void Event::cmd_set(const CommandBuffer &cmd, VkPipelineStageFlags stage_mask) {
    vk::CmdSetEvent(cmd.handle(), handle(), stage_mask);
}

void Event::cmd_reset(const CommandBuffer &cmd, VkPipelineStageFlags stage_mask) {
    vk::CmdResetEvent(cmd.handle(), handle(), stage_mask);
}

void Event::cmd_wait(const CommandBuffer &cmd, VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
                     const std::vector<VkMemoryBarrier> &memory_barriers, const std::vector<VkBufferMemoryBarrier> &buffer_barriers,
                     const std::vector<VkImageMemoryBarrier> &image_barriers) {
    VkEvent event_handle = handle();
    vk::CmdWaitEvents(cmd.handle(), 1, &event_handle, src_stage_mask, dst_stage_mask, static_cast<uint32_t>(memory_barriers.size()),
                      memory_barriers.data(), static_cast<uint32_t>(buffer_barriers.size()), buffer_barriers.data(),
                      static_cast<uint32_t>(image_barriers.size()), image_barriers.data());
}

void Event::reset() { ASSERT_EQ(VK_SUCCESS, vk::ResetEvent(device(), handle())); }

NON_DISPATCHABLE_HANDLE_DTOR(QueryPool, vk::DestroyQueryPool)

void QueryPool::init(const Device &dev, const VkQueryPoolCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateQueryPool, dev, &info);
}

VkResult QueryPool::results(uint32_t first, uint32_t count, size_t size, void *data, size_t stride) {
    VkResult err = vk::GetQueryPoolResults(device(), handle(), first, count, size, data, stride, 0);
    EXPECT_TRUE(err == VK_SUCCESS || err == VK_NOT_READY);

    return err;
}

NON_DISPATCHABLE_HANDLE_DTOR(Buffer, vk::DestroyBuffer)

void Buffer::init(const Device &dev, const VkBufferCreateInfo &info, VkMemoryPropertyFlags mem_props, void *alloc_info_pnext) {
    init_no_mem(dev, info);

    auto alloc_info = DeviceMemory::get_resource_alloc_info(dev, memory_requirements(), mem_props, alloc_info_pnext);
    internal_mem_.init(dev, alloc_info);

    bind_memory(internal_mem_, 0);
}

void Buffer::init_no_mem(const Device &dev, const VkBufferCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateBuffer, dev, &info);
    create_info_ = info;
}

VkMemoryRequirements Buffer::memory_requirements() const {
    VkMemoryRequirements reqs;

    vk::GetBufferMemoryRequirements(device(), handle(), &reqs);

    return reqs;
}

void Buffer::allocate_and_bind_memory(const Device &dev, VkMemoryPropertyFlags mem_props, void *alloc_info_pnext) {
    assert(!internal_mem_.initialized());
    internal_mem_.init(dev, DeviceMemory::get_resource_alloc_info(dev, memory_requirements(), mem_props, alloc_info_pnext));
    bind_memory(internal_mem_, 0);
}

void Buffer::bind_memory(const DeviceMemory &mem, VkDeviceSize mem_offset) {
    const auto result = vk::BindBufferMemory(device(), handle(), mem.handle(), mem_offset);
    // Allow successful calls and the calls that cause validation errors (but not actual Vulkan errors).
    // In the case of a validation error, it's part of the test logic how to handle it.
    ASSERT_TRUE(result == VK_SUCCESS || result == VK_ERROR_VALIDATION_FAILED_EXT);
}

VkDeviceAddress Buffer::address() const {
    VkBufferDeviceAddressInfo bdai = vku::InitStructHelper();
    bdai.buffer = handle();
    if (vk::GetBufferDeviceAddressKHR) {
        return vk::GetBufferDeviceAddressKHR(device(), &bdai);
    } else {
        return vk::GetBufferDeviceAddress(device(), &bdai);
    }
}

NON_DISPATCHABLE_HANDLE_DTOR(BufferView, vk::DestroyBufferView)

void BufferView::init(const Device &dev, const VkBufferViewCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateBufferView, dev, &info);
}

NON_DISPATCHABLE_HANDLE_DTOR(Image, vk::DestroyImage)

Image::Image(const Device &dev, const VkImageCreateInfo &info, VkMemoryPropertyFlags mem_props, void *alloc_info_pnext)
    : format_features_(0) {
    init(dev, info, mem_props, alloc_info_pnext);
}

void Image::init(const Device &dev, const VkImageCreateInfo &info, VkMemoryPropertyFlags mem_props, void *alloc_info_pnext) {
    init_no_mem(dev, info);

    if (initialized()) {
        auto alloc_info = DeviceMemory::get_resource_alloc_info(dev, memory_requirements(), mem_props, alloc_info_pnext);
        internal_mem_.init(dev, alloc_info);
        bind_memory(internal_mem_, 0);
    }
}

void Image::init_no_mem(const Device &dev, const VkImageCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateImage, dev, &info);
    if (initialized()) {
        init_info(dev, info);
    }
}

void Image::init_info(const Device &dev, const VkImageCreateInfo &info) {
    create_info_ = info;

    for (std::vector<Device::Format>::const_iterator it = dev.formats().begin(); it != dev.formats().end(); it++) {
        if (memcmp(&it->format, &create_info_.format, sizeof(it->format)) == 0 && it->tiling == create_info_.tiling) {
            format_features_ = it->features;
            break;
        }
    }
}

VkMemoryRequirements Image::memory_requirements() const {
    VkMemoryRequirements reqs;

    vk::GetImageMemoryRequirements(device(), handle(), &reqs);

    return reqs;
}

void Image::allocate_and_bind_memory(const Device &dev, VkMemoryPropertyFlags mem_props, void *alloc_info_pnext) {
    assert(!internal_mem_.initialized());
    internal_mem_.init(dev, DeviceMemory::get_resource_alloc_info(dev, memory_requirements(), mem_props, alloc_info_pnext));
    bind_memory(internal_mem_, 0);
}

void Image::bind_memory(const DeviceMemory &mem, VkDeviceSize mem_offset) {
    const auto result = vk::BindImageMemory(device(), handle(), mem.handle(), mem_offset);
    // Allow successful calls and the calls that cause validation errors (but not actual Vulkan errors).
    // In the case of a validation error, it's part of the test logic how to handle it.
    ASSERT_TRUE(result == VK_SUCCESS || result == VK_ERROR_VALIDATION_FAILED_EXT);
}

VkSubresourceLayout Image::subresource_layout(const VkImageSubresource &subres) const {
    VkSubresourceLayout data;
    size_t size = sizeof(data);
    vk::GetImageSubresourceLayout(device(), handle(), &subres, &data);
    if (size != sizeof(data)) memset(&data, 0, sizeof(data));

    return data;
}

VkSubresourceLayout Image::subresource_layout(const VkImageSubresourceLayers &subrescopy) const {
    VkSubresourceLayout data;
    VkImageSubresource subres = subresource(subrescopy.aspectMask, subrescopy.mipLevel, subrescopy.baseArrayLayer);
    size_t size = sizeof(data);
    vk::GetImageSubresourceLayout(device(), handle(), &subres, &data);
    if (size != sizeof(data)) memset(&data, 0, sizeof(data));

    return data;
}

bool Image::transparent() const {
    return (create_info_.tiling == VK_IMAGE_TILING_LINEAR && create_info_.samples == VK_SAMPLE_COUNT_1_BIT &&
            !(create_info_.usage & (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)));
}

VkImageAspectFlags Image::aspect_mask(VkFormat format) {
    VkImageAspectFlags image_aspect;
    if (vkuFormatIsDepthAndStencil(format)) {
        image_aspect = VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;
    } else if (vkuFormatIsDepthOnly(format)) {
        image_aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    } else if (vkuFormatIsStencilOnly(format)) {
        image_aspect = VK_IMAGE_ASPECT_STENCIL_BIT;
    } else {  // color
        image_aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    return image_aspect;
}

NON_DISPATCHABLE_HANDLE_DTOR(ImageView, vk::DestroyImageView)

void ImageView::init(const Device &dev, const VkImageViewCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateImageView, dev, &info);
}

void AccelerationStructure::destroy() noexcept {
    if (!initialized()) {
        return;
    }
    PFN_vkDestroyAccelerationStructureNV vkDestroyAccelerationStructureNV =
        (PFN_vkDestroyAccelerationStructureNV)vk::GetDeviceProcAddr(device(), "vkDestroyAccelerationStructureNV");
    assert(vkDestroyAccelerationStructureNV != nullptr);

    vkDestroyAccelerationStructureNV(device(), handle(), nullptr);
    handle_ = VK_NULL_HANDLE;
}
AccelerationStructure::~AccelerationStructure() noexcept { destroy(); }

VkMemoryRequirements2 AccelerationStructure::memory_requirements() const {
    PFN_vkGetAccelerationStructureMemoryRequirementsNV vkGetAccelerationStructureMemoryRequirementsNV =
        (PFN_vkGetAccelerationStructureMemoryRequirementsNV)vk::GetDeviceProcAddr(device(),
                                                                                  "vkGetAccelerationStructureMemoryRequirementsNV");
    assert(vkGetAccelerationStructureMemoryRequirementsNV != nullptr);
    VkMemoryRequirements2 memoryRequirements = {};
    VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo = vku::InitStructHelper();
    memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
    memoryRequirementsInfo.accelerationStructure = handle();
    vkGetAccelerationStructureMemoryRequirementsNV(device(), &memoryRequirementsInfo, &memoryRequirements);
    return memoryRequirements;
}

VkMemoryRequirements2 AccelerationStructure::build_scratch_memory_requirements() const {
    PFN_vkGetAccelerationStructureMemoryRequirementsNV vkGetAccelerationStructureMemoryRequirementsNV =
        (PFN_vkGetAccelerationStructureMemoryRequirementsNV)vk::GetDeviceProcAddr(device(),
                                                                                  "vkGetAccelerationStructureMemoryRequirementsNV");
    assert(vkGetAccelerationStructureMemoryRequirementsNV != nullptr);

    VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo = vku::InitStructHelper();
    memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
    memoryRequirementsInfo.accelerationStructure = handle();

    VkMemoryRequirements2 memoryRequirements = {};
    vkGetAccelerationStructureMemoryRequirementsNV(device(), &memoryRequirementsInfo, &memoryRequirements);
    return memoryRequirements;
}

void AccelerationStructure::init(const Device &dev, const VkAccelerationStructureCreateInfoNV &info, bool init_memory) {
    PFN_vkCreateAccelerationStructureNV vkCreateAccelerationStructureNV =
        (PFN_vkCreateAccelerationStructureNV)vk::GetDeviceProcAddr(dev.handle(), "vkCreateAccelerationStructureNV");
    assert(vkCreateAccelerationStructureNV != nullptr);

    NON_DISPATCHABLE_HANDLE_INIT(vkCreateAccelerationStructureNV, dev, &info);

    info_ = info.info;

    if (init_memory) {
        memory_.init(dev, DeviceMemory::get_resource_alloc_info(dev, memory_requirements().memoryRequirements,
                                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));

        PFN_vkBindAccelerationStructureMemoryNV vkBindAccelerationStructureMemoryNV =
            (PFN_vkBindAccelerationStructureMemoryNV)vk::GetDeviceProcAddr(dev.handle(), "vkBindAccelerationStructureMemoryNV");
        assert(vkBindAccelerationStructureMemoryNV != nullptr);

        VkBindAccelerationStructureMemoryInfoNV bind_info = vku::InitStructHelper();
        bind_info.accelerationStructure = handle();
        bind_info.memory = memory_.handle();
        ASSERT_EQ(VK_SUCCESS, vkBindAccelerationStructureMemoryNV(dev.handle(), 1, &bind_info));

        PFN_vkGetAccelerationStructureHandleNV vkGetAccelerationStructureHandleNV =
            (PFN_vkGetAccelerationStructureHandleNV)vk::GetDeviceProcAddr(dev.handle(), "vkGetAccelerationStructureHandleNV");
        assert(vkGetAccelerationStructureHandleNV != nullptr);
        ASSERT_EQ(VK_SUCCESS, vkGetAccelerationStructureHandleNV(dev.handle(), handle(), sizeof(uint64_t), &opaque_handle_));
    }
}
vkt::Buffer AccelerationStructure::create_scratch_buffer(const Device &device, VkBufferCreateInfo *pCreateInfo /*= nullptr*/,
                                                         bool buffer_device_address /*= false*/) const {
    VkMemoryRequirements scratch_buffer_memory_requirements = build_scratch_memory_requirements().memoryRequirements;
    VkBufferCreateInfo create_info = {};
    create_info.size = scratch_buffer_memory_requirements.size;
    if (pCreateInfo) {
        create_info.sType = pCreateInfo->sType;
        create_info.usage = pCreateInfo->usage;
    } else {
        create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        create_info.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
        if (buffer_device_address) create_info.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    }

    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    void *pNext = nullptr;
    if (buffer_device_address) {
        alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        pNext = &alloc_flags;
    }

    return vkt::Buffer(device, create_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pNext);
}

NON_DISPATCHABLE_HANDLE_DTOR(ShaderModule, vk::DestroyShaderModule)

void ShaderModule::init(const Device &dev, const VkShaderModuleCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateShaderModule, dev, &info);
}

VkResult ShaderModule::init_try(const Device &dev, const VkShaderModuleCreateInfo &info) {
    VkShaderModule mod;

    VkResult err = vk::CreateShaderModule(dev.handle(), &info, NULL, &mod);
    if (err == VK_SUCCESS) NonDispHandle::init(dev.handle(), mod);

    return err;
}

NON_DISPATCHABLE_HANDLE_DTOR(Shader, vk::DestroyShaderEXT)

void Shader::init(const Device &dev, const VkShaderCreateInfoEXT &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateShadersEXT, dev, 1u, &info);
}

VkResult Shader::init_try(const Device &dev, const VkShaderCreateInfoEXT &info) {
    VkShaderEXT mod;

    VkResult err = vk::CreateShadersEXT(dev.handle(), 1u, &info, NULL, &mod);
    if (err == VK_SUCCESS) NonDispHandle::init(dev.handle(), mod);

    return err;
}

Shader::Shader(const Device &dev, const VkShaderStageFlagBits stage, const std::vector<uint32_t> &spv,
               const VkDescriptorSetLayout *descriptorSetLayout, const VkPushConstantRange *pushConstRange) {
    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = stage;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";
    if (descriptorSetLayout) {
        createInfo.setLayoutCount = 1u;
        createInfo.pSetLayouts = descriptorSetLayout;
    }
    if (pushConstRange) {
        createInfo.pushConstantRangeCount = 1u;
        createInfo.pPushConstantRanges = pushConstRange;
    }
    init(dev, createInfo);
}

Shader::Shader(const Device &dev, const VkShaderStageFlagBits stage, const std::vector<uint8_t> &binary,
               const VkDescriptorSetLayout *descriptorSetLayout, const VkPushConstantRange *pushConstRange) {
    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = stage;
    createInfo.codeType = VK_SHADER_CODE_TYPE_BINARY_EXT;
    createInfo.codeSize = binary.size();
    createInfo.pCode = binary.data();
    createInfo.pName = "main";
    if (descriptorSetLayout) {
        createInfo.setLayoutCount = 1u;
        createInfo.pSetLayouts = descriptorSetLayout;
    }
    if (pushConstRange) {
        createInfo.pushConstantRangeCount = 1u;
        createInfo.pPushConstantRanges = pushConstRange;
    }
    init(dev, createInfo);
}

Shader::Shader(const Device &dev, const VkShaderStageFlagBits stage, const std::vector<uint32_t> &spv,
               VkShaderCreateFlagsEXT flags) {
    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.flags = flags;
    createInfo.stage = stage;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";
    init(dev, createInfo);
}

NON_DISPATCHABLE_HANDLE_DTOR(Pipeline, vk::DestroyPipeline)

void Pipeline::init(const Device &dev, const VkGraphicsPipelineCreateInfo &info) {
    VkPipelineCache cache;
    VkPipelineCacheCreateInfo ci = vku::InitStructHelper();
    VkResult err = vk::CreatePipelineCache(dev.handle(), &ci, NULL, &cache);
    if (err == VK_SUCCESS) {
        NON_DISPATCHABLE_HANDLE_INIT(vk::CreateGraphicsPipelines, dev, cache, 1, &info);
        vk::DestroyPipelineCache(dev.handle(), cache, NULL);
    }
}

VkResult Pipeline::init_try(const Device &dev, const VkGraphicsPipelineCreateInfo &info) {
    VkPipeline pipe;
    VkPipelineCache cache;
    VkPipelineCacheCreateInfo ci = vku::InitStructHelper();
    VkResult err = vk::CreatePipelineCache(dev.handle(), &ci, NULL, &cache);
    EXPECT_EQ(VK_SUCCESS, err);
    if (err == VK_SUCCESS) {
        err = vk::CreateGraphicsPipelines(dev.handle(), cache, 1, &info, NULL, &pipe);
        if (err == VK_SUCCESS) {
            NonDispHandle::init(dev.handle(), pipe);
        }
        vk::DestroyPipelineCache(dev.handle(), cache, NULL);
    }

    return err;
}

void Pipeline::init(const Device &dev, const VkComputePipelineCreateInfo &info) {
    VkPipelineCache cache;
    VkPipelineCacheCreateInfo ci = vku::InitStructHelper();
    VkResult err = vk::CreatePipelineCache(dev.handle(), &ci, NULL, &cache);
    if (err == VK_SUCCESS) {
        NON_DISPATCHABLE_HANDLE_INIT(vk::CreateComputePipelines, dev, cache, 1, &info);
        vk::DestroyPipelineCache(dev.handle(), cache, NULL);
    }
}

NON_DISPATCHABLE_HANDLE_DTOR(PipelineLayout, vk::DestroyPipelineLayout)

void PipelineLayout::init(const Device &dev, VkPipelineLayoutCreateInfo &info,
                          const std::vector<const DescriptorSetLayout *> &layouts) {
    const std::vector<VkDescriptorSetLayout> layout_handles = MakeVkHandles<VkDescriptorSetLayout>(layouts);
    info.setLayoutCount = static_cast<uint32_t>(layout_handles.size());
    info.pSetLayouts = layout_handles.data();

    init(dev, info);
}

void PipelineLayout::init(const Device &dev, VkPipelineLayoutCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreatePipelineLayout, dev, &info);
}

NON_DISPATCHABLE_HANDLE_DTOR(Sampler, vk::DestroySampler)

void Sampler::init(const Device &dev, const VkSamplerCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateSampler, dev, &info);
}

NON_DISPATCHABLE_HANDLE_DTOR(DescriptorSetLayout, vk::DestroyDescriptorSetLayout)

void DescriptorSetLayout::init(const Device &dev, const VkDescriptorSetLayoutCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateDescriptorSetLayout, dev, &info);
}

NON_DISPATCHABLE_HANDLE_DTOR(DescriptorPool, vk::DestroyDescriptorPool)

void DescriptorPool::init(const Device &dev, const VkDescriptorPoolCreateInfo &info) {
    setDynamicUsage(info.flags & VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateDescriptorPool, dev, &info);
}

void DescriptorPool::reset() { ASSERT_EQ(VK_SUCCESS, vk::ResetDescriptorPool(device(), handle(), 0)); }

std::vector<DescriptorSet *> DescriptorPool::alloc_sets(const Device &dev,
                                                        const std::vector<const DescriptorSetLayout *> &layouts) {
    const std::vector<VkDescriptorSetLayout> layout_handles = MakeVkHandles<VkDescriptorSetLayout>(layouts);

    std::vector<VkDescriptorSet> set_handles;
    set_handles.resize(layout_handles.size());

    VkDescriptorSetAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.descriptorSetCount = static_cast<uint32_t>(layout_handles.size());
    alloc_info.descriptorPool = handle();
    alloc_info.pSetLayouts = layout_handles.data();
    VkResult err = vk::AllocateDescriptorSets(device(), &alloc_info, set_handles.data());
    EXPECT_EQ(VK_SUCCESS, err);

    std::vector<DescriptorSet *> sets;
    for (std::vector<VkDescriptorSet>::const_iterator it = set_handles.begin(); it != set_handles.end(); it++) {
        // do descriptor sets need memories bound?
        DescriptorSet *descriptorSet = new DescriptorSet(dev, this, *it);
        sets.push_back(descriptorSet);
    }
    return sets;
}

std::vector<DescriptorSet *> DescriptorPool::alloc_sets(const Device &dev, const DescriptorSetLayout &layout, uint32_t count) {
    return alloc_sets(dev, std::vector<const DescriptorSetLayout *>(count, &layout));
}

DescriptorSet *DescriptorPool::alloc_sets(const Device &dev, const DescriptorSetLayout &layout) {
    std::vector<DescriptorSet *> set = alloc_sets(dev, layout, 1);
    return (set.empty()) ? NULL : set[0];
}
void DescriptorSet::destroy() noexcept {
    if (!initialized()) {
        return;
    }
    // Only call vk::Free* on sets allocated from pool with usage *_DYNAMIC
    if (containing_pool_->getDynamicUsage()) {
        VkDescriptorSet sets[1] = {handle()};
        ASSERT_EQ(VK_SUCCESS, vk::FreeDescriptorSets(device(), containing_pool_->handle(), 1, sets));
    }
    handle_ = VK_NULL_HANDLE;
}
DescriptorSet::~DescriptorSet() noexcept { destroy(); }

NON_DISPATCHABLE_HANDLE_DTOR(CommandPool, vk::DestroyCommandPool)

void CommandPool::init(const Device &dev, const VkCommandPoolCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateCommandPool, dev, &info);
}

void CommandBuffer::destroy() noexcept {
    if (!initialized()) {
        return;
    }
    VkCommandBuffer cmds[] = {handle()};
    vk::FreeCommandBuffers(dev_handle_, cmd_pool_, 1, cmds);
    handle_ = VK_NULL_HANDLE;
}
CommandBuffer::~CommandBuffer() noexcept { destroy(); }

void CommandBuffer::init(const Device &dev, const VkCommandBufferAllocateInfo &info) {
    VkCommandBuffer cmd;

    // Make sure commandPool is set
    assert(info.commandPool);

    ASSERT_EQ(VK_SUCCESS, vk::AllocateCommandBuffers(dev.handle(), &info, &cmd));
    Handle::init(cmd);
    dev_handle_ = dev.handle();
    cmd_pool_ = info.commandPool;
}

void CommandBuffer::Init(Device *device, const CommandPool *pool, VkCommandBufferLevel level, Queue *queue) {
    if (queue) {
        m_queue = queue;
    } else {
        m_queue = device->graphics_queues()[0];
    }
    assert(m_queue);

    auto create_info = CommandBuffer::create_info(pool->handle());
    create_info.level = level;
    init(*device, create_info);
}

void CommandBuffer::begin(const VkCommandBufferBeginInfo *info) { ASSERT_EQ(VK_SUCCESS, vk::BeginCommandBuffer(handle(), info)); }

void CommandBuffer::begin() {
    VkCommandBufferBeginInfo info = vku::InitStructHelper();
    VkCommandBufferInheritanceInfo hinfo = vku::InitStructHelper();
    info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    info.pInheritanceInfo = &hinfo;
    hinfo.renderPass = VK_NULL_HANDLE;
    hinfo.subpass = 0;
    hinfo.framebuffer = VK_NULL_HANDLE;
    hinfo.occlusionQueryEnable = VK_FALSE;
    hinfo.queryFlags = 0;
    hinfo.pipelineStatistics = 0;

    begin(&info);
}

void CommandBuffer::end() { ASSERT_EQ(VK_SUCCESS, vk::EndCommandBuffer(handle())); }

void CommandBuffer::reset(VkCommandBufferResetFlags flags) { ASSERT_EQ(VK_SUCCESS, vk::ResetCommandBuffer(handle(), flags)); }

VkCommandBufferAllocateInfo CommandBuffer::create_info(VkCommandPool const &pool) {
    VkCommandBufferAllocateInfo info = vku::InitStructHelper();
    info.commandPool = pool;
    info.commandBufferCount = 1;
    return info;
}

void CommandBuffer::BeginRenderPass(const VkRenderPassBeginInfo &info, VkSubpassContents contents) {
    vk::CmdBeginRenderPass(handle(), &info, contents);
}

void CommandBuffer::NextSubpass(VkSubpassContents contents) { vk::CmdNextSubpass(handle(), contents); }

void CommandBuffer::EndRenderPass() { vk::CmdEndRenderPass(handle()); }

void CommandBuffer::BeginRendering(const VkRenderingInfoKHR &renderingInfo) {
    if (vk::CmdBeginRenderingKHR) {
        vk::CmdBeginRenderingKHR(handle(), &renderingInfo);
    } else {
        vk::CmdBeginRendering(handle(), &renderingInfo);
    }
}

void CommandBuffer::BeginRenderingColor(const VkImageView imageView) {
    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageView = imageView;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfoKHR renderingInfo = vku::InitStructHelper();
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &color_attachment;
    renderingInfo.layerCount = 1;
    renderingInfo.renderArea = {{0, 0}, {1, 1}};

    BeginRendering(renderingInfo);
}

void CommandBuffer::EndRendering() {
    if (vk::CmdEndRenderingKHR) {
        vk::CmdEndRenderingKHR(handle());
    } else {
        vk::CmdEndRendering(handle());
    }
}

void CommandBuffer::BeginVideoCoding(const VkVideoBeginCodingInfoKHR &beginInfo) {
    PFN_vkCmdBeginVideoCodingKHR vkCmdBeginVideoCodingKHR =
        (PFN_vkCmdBeginVideoCodingKHR)vk::GetDeviceProcAddr(dev_handle_, "vkCmdBeginVideoCodingKHR");
    assert(vkCmdBeginVideoCodingKHR);

    vkCmdBeginVideoCodingKHR(handle(), &beginInfo);
}

void CommandBuffer::ControlVideoCoding(const VkVideoCodingControlInfoKHR &controlInfo) {
    PFN_vkCmdControlVideoCodingKHR vkCmdControlVideoCodingKHR =
        (PFN_vkCmdControlVideoCodingKHR)vk::GetDeviceProcAddr(dev_handle_, "vkCmdControlVideoCodingKHR");
    assert(vkCmdControlVideoCodingKHR);

    vkCmdControlVideoCodingKHR(handle(), &controlInfo);
}

void CommandBuffer::DecodeVideo(const VkVideoDecodeInfoKHR &decodeInfo) {
    PFN_vkCmdDecodeVideoKHR vkCmdDecodeVideoKHR =
        (PFN_vkCmdDecodeVideoKHR)vk::GetDeviceProcAddr(dev_handle_, "vkCmdDecodeVideoKHR");
    assert(vkCmdDecodeVideoKHR);

    vkCmdDecodeVideoKHR(handle(), &decodeInfo);
}

void CommandBuffer::EndVideoCoding(const VkVideoEndCodingInfoKHR &endInfo) {
    PFN_vkCmdEndVideoCodingKHR vkCmdEndVideoCodingKHR =
        (PFN_vkCmdEndVideoCodingKHR)vk::GetDeviceProcAddr(dev_handle_, "vkCmdEndVideoCodingKHR");
    assert(vkCmdEndVideoCodingKHR);

    vkCmdEndVideoCodingKHR(handle(), &endInfo);
}

void CommandBuffer::QueueCommandBuffer(bool check_success) {
    vkt::Fence null_fence;
    QueueCommandBuffer(null_fence, check_success);
}

void CommandBuffer::QueueCommandBuffer(const vkt::Fence &fence, bool check_success, bool submit_2) {
    VkResult err = VK_SUCCESS;
    (void)err;

    if (submit_2) {
        err = m_queue->submit2(*this, fence, check_success);
    } else {
        err = m_queue->submit(*this, fence, check_success);
    }
    if (check_success) {
        assert(err == VK_SUCCESS);
    }

    err = m_queue->wait();
    if (check_success) {
        assert(err == VK_SUCCESS);
    }

    // TODO: Determine if we really want this serialization here
    // Wait for work to finish before cleaning up.
    vk::DeviceWaitIdle(dev_handle_);
}

void RenderPass::init(const Device &dev, const VkRenderPassCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateRenderPass, dev, &info);
}

void RenderPass::init(const Device &dev, const VkRenderPassCreateInfo2 &info, bool khr) {
    if (!khr) {
        NON_DISPATCHABLE_HANDLE_INIT(vk::CreateRenderPass2, dev, &info);
    } else {
        auto vkCreateRenderPass2KHR =
            reinterpret_cast<PFN_vkCreateRenderPass2KHR>(vk::GetDeviceProcAddr(dev.handle(), "vkCreateRenderPass2KHR"));
        ASSERT_NE(vkCreateRenderPass2KHR, nullptr);
        NON_DISPATCHABLE_HANDLE_INIT(vkCreateRenderPass2KHR, dev, &info);
    }
}

NON_DISPATCHABLE_HANDLE_DTOR(RenderPass, vk::DestroyRenderPass)

void Framebuffer::init(const Device &dev, const VkFramebufferCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateFramebuffer, dev, &info);
}

NON_DISPATCHABLE_HANDLE_DTOR(Framebuffer, vk::DestroyFramebuffer)

void SamplerYcbcrConversion::init(const Device &dev, const VkSamplerYcbcrConversionCreateInfo &info, bool khr) {
    if (!khr) {
        NON_DISPATCHABLE_HANDLE_INIT(vk::CreateSamplerYcbcrConversion, dev, &info);
    } else {
        auto vkCreateSamplerYcbcrConversionKHR = reinterpret_cast<PFN_vkCreateSamplerYcbcrConversionKHR>(
            vk::GetDeviceProcAddr(dev.handle(), "vkCreateSamplerYcbcrConversionKHR"));
        ASSERT_NE(vkCreateSamplerYcbcrConversionKHR, nullptr);
        NON_DISPATCHABLE_HANDLE_INIT(vkCreateSamplerYcbcrConversionKHR, dev, &info);
    }
}

VkSamplerYcbcrConversionInfo SamplerYcbcrConversion::ConversionInfo() {
    VkSamplerYcbcrConversionInfo ycbcr_info = vku::InitStructHelper();
    ycbcr_info.conversion = handle();
    return ycbcr_info;
}

// static
VkSamplerYcbcrConversionCreateInfo SamplerYcbcrConversion::DefaultConversionInfo(VkFormat format) {
    VkSamplerYcbcrConversionCreateInfo ycbcr_create_info = vku::InitStructHelper();
    ycbcr_create_info.format = format;
    ycbcr_create_info.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    ycbcr_create_info.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    ycbcr_create_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                    VK_COMPONENT_SWIZZLE_IDENTITY};
    ycbcr_create_info.xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    ycbcr_create_info.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    ycbcr_create_info.chromaFilter = VK_FILTER_NEAREST;
    ycbcr_create_info.forceExplicitReconstruction = false;
    return ycbcr_create_info;
}

void SamplerYcbcrConversion::destroy() noexcept {
    if (!initialized()) {
        return;
    }
    if (!khr_) {
        vk::DestroySamplerYcbcrConversion(device(), handle(), nullptr);
    } else {
        auto vkDestroySamplerYcbcrConversionKHR = reinterpret_cast<PFN_vkDestroySamplerYcbcrConversionKHR>(
            vk::GetDeviceProcAddr(device(), "vkDestroySamplerYcbcrConversionKHR"));
        assert(vkDestroySamplerYcbcrConversionKHR != nullptr);
        vkDestroySamplerYcbcrConversionKHR(device(), handle(), nullptr);
    }
    handle_ = VK_NULL_HANDLE;
}

SamplerYcbcrConversion::~SamplerYcbcrConversion() noexcept { destroy(); }

}  // namespace vkt
