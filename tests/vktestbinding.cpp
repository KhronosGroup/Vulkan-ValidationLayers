/*
 * Copyright (c) 2015-2020 The Khronos Group Inc.
 * Copyright (c) 2015-2020 Valve Corporation
 * Copyright (c) 2015-2020 LunarG, Inc.
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
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Tony Barbour <tony@LunarG.com>
 */

#include "vktestbinding.h"

#include <string.h>  // memset(), memcmp()
#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

#include "test_common.h"
#include "vk_typemap_helper.h"

namespace {

#define NON_DISPATCHABLE_HANDLE_INIT(create_func, dev, ...)                              \
    do {                                                                                 \
        handle_type handle;                                                              \
        if (EXPECT(create_func(dev.handle(), __VA_ARGS__, NULL, &handle) == VK_SUCCESS)) \
            NonDispHandle::init(dev.handle(), handle);                                   \
    } while (0)

#define NON_DISPATCHABLE_HANDLE_DTOR(cls, destroy_func)            \
    cls::~cls() NOEXCEPT {                                         \
        if (initialized()) destroy_func(device(), handle(), NULL); \
    }

#define STRINGIFY(x) #x
#define EXPECT(expr) ((expr) ? true : expect_failure(STRINGIFY(expr), __FILE__, __LINE__, __FUNCTION__))

vk_testing::ErrorCallback error_callback;

bool expect_failure(const char *expr, const char *file, unsigned int line, const char *function) {
    if (error_callback) {
        error_callback(expr, file, line, function);
    } else {
        std::cerr << file << ":" << line << ": " << function << ": Expectation `" << expr << "' failed.\n";
    }

    return false;
}

}  // namespace

namespace vk_testing {

void set_error_callback(ErrorCallback callback) { error_callback = callback; }

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
    uint32_t layer_count;
    std::vector<VkLayerProperties> layers;

    do {
        err = vk::EnumerateInstanceLayerProperties(&layer_count, nullptr);
        assert(!err);
        if (err || 0 == layer_count) return {};

        layers.resize(layer_count);
        err = vk::EnumerateInstanceLayerProperties(&layer_count, layers.data());
    } while (VK_INCOMPLETE == err);

    assert(!err);
    if (err) return {};
    layers.resize(layer_count);

    return layers;
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
    uint32_t extension_count;
    std::vector<VkExtensionProperties> extensions;

    do {
        err = vk::EnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
        assert(!err);
        if (err || 0 == extension_count) return {};

        extensions.resize(extension_count);
        err = vk::EnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());
    } while (VK_INCOMPLETE == err);

    assert(!err);
    if (err) return {};
    extensions.resize(extension_count);

    return extensions;
}

/*
 * Return list of PhysicalDevice extensions provided by the specified layer
 * If pLayerName is NULL, will return extensions for ICD / loader.
 */
std::vector<VkExtensionProperties> PhysicalDevice::extensions(const char *pLayerName) const {
    VkResult err;
    uint32_t extension_count;
    std::vector<VkExtensionProperties> extensions;
    do {
        err = vk::EnumerateDeviceExtensionProperties(handle(), pLayerName, &extension_count, nullptr);
        if (err || 0 == extension_count) return {};

        extensions.resize(extension_count);
        err = vk::EnumerateDeviceExtensionProperties(handle(), pLayerName, &extension_count, extensions.data());
    } while (VK_INCOMPLETE == err);

    if (err) return {};
    extensions.resize(extension_count);

    return extensions;
}

bool PhysicalDevice::set_memory_type(const uint32_t type_bits, VkMemoryAllocateInfo *info, const VkFlags properties,
                                     const VkFlags forbid) const {
    uint32_t type_mask = type_bits;
    // Search memtypes to find first index with those properties
    for (uint32_t i = 0; i < memory_properties_.memoryTypeCount; i++) {
        if ((type_mask & 1) == 1) {
            // Type is available, does it match user properties?
            if ((memory_properties_.memoryTypes[i].propertyFlags & properties) == properties &&
                (memory_properties_.memoryTypes[i].propertyFlags & forbid) == 0) {
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
    uint32_t layer_count;
    std::vector<VkLayerProperties> layers;
    do {
        err = vk::EnumerateDeviceLayerProperties(handle(), &layer_count, nullptr);
        if (err || 0 == layer_count) return {};

        layers.resize(layer_count);
        err = vk::EnumerateDeviceLayerProperties(handle(), &layer_count, layers.data());
    } while (VK_INCOMPLETE == err);

    if (err) return {};
    layers.resize(layer_count);

    return layers;
}

QueueCreateInfoArray::QueueCreateInfoArray(const std::vector<VkQueueFamilyProperties> &queue_props)
    : queue_info_(), queue_priorities_() {
    queue_info_.reserve(queue_props.size());

    for (uint32_t i = 0; i < (uint32_t)queue_props.size(); ++i) {
        if (queue_props[i].queueCount > 0) {
            VkDeviceQueueCreateInfo qi = {};
            qi.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            qi.pNext = NULL;
            qi.queueFamilyIndex = i;
            qi.queueCount = queue_props[i].queueCount;
            queue_priorities_.emplace_back(qi.queueCount, 0.0f);
            qi.pQueuePriorities = queue_priorities_[i].data();
            queue_info_.push_back(qi);
        }
    }
}

Device::~Device() NOEXCEPT {
    if (!initialized()) return;

    vk::DestroyDevice(handle(), NULL);
}

void Device::init(std::vector<const char *> &extensions, VkPhysicalDeviceFeatures *features, void *create_device_pnext) {
    // request all queues
    const std::vector<VkQueueFamilyProperties> queue_props = phy_.queue_properties();
    QueueCreateInfoArray queue_info(phy_.queue_properties());
    for (uint32_t i = 0; i < (uint32_t)queue_props.size(); i++) {
        if (queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
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

    VkDeviceCreateInfo dev_info = {};
    dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dev_info.pNext = create_device_pnext;
    dev_info.queueCreateInfoCount = create_queue_infos.size();
    dev_info.pQueueCreateInfos = create_queue_infos.data();
    dev_info.enabledLayerCount = 0;
    dev_info.ppEnabledLayerNames = NULL;
    dev_info.enabledExtensionCount = extensions.size();
    dev_info.ppEnabledExtensionNames = extensions.data();

    VkPhysicalDeviceFeatures all_features;
    // Let VkPhysicalDeviceFeatures2 take priority over VkPhysicalDeviceFeatures,
    // since it supports extensions

    if (!(lvl_find_in_chain<VkPhysicalDeviceFeatures2>(dev_info.pNext))) {
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

    if (EXPECT(vk::CreateDevice(phy_.handle(), &info, NULL, &dev) == VK_SUCCESS)) Handle::init(dev);

    init_queues();
    init_formats();
}

void Device::init_queues() {
    uint32_t queue_node_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(phy_.handle(), &queue_node_count, NULL);
    EXPECT(queue_node_count >= 1);

    std::vector<VkQueueFamilyProperties> queue_props(queue_node_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(phy_.handle(), &queue_node_count, queue_props.data());

    queue_families_.resize(queue_node_count);
    for (uint32_t queue_family_i = 0; queue_family_i < queue_node_count; ++queue_family_i) {
        const auto &queue_prop = queue_props[queue_family_i];

        QueueFamilyQueues &queue_storage = queue_families_[queue_family_i];
        queue_storage.reserve(queue_prop.queueCount);
        for (uint32_t queue_i = 0; queue_i < queue_prop.queueCount; ++queue_i) {
            // TODO: Need to add support for separate MEMMGR and work queues,
            // including synchronization
            VkQueue queue;
            vk::GetDeviceQueue(handle(), queue_family_i, queue_i, &queue);

            // Store single copy of the queue object that will self destruct
            queue_storage.emplace_back(new Queue(queue, queue_family_i));

            if (queue_prop.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                queues_[GRAPHICS].push_back(queue_storage.back().get());
            }

            if (queue_prop.queueFlags & VK_QUEUE_COMPUTE_BIT) {
                queues_[COMPUTE].push_back(queue_storage.back().get());
            }

            if (queue_prop.queueFlags & VK_QUEUE_TRANSFER_BIT) {
                queues_[DMA].push_back(queue_storage.back().get());
            }
        }
    }

    EXPECT(!queues_[GRAPHICS].empty() || !queues_[COMPUTE].empty());
}

const Device::QueueFamilyQueues &Device::queue_family_queues(uint32_t queue_family) const {
    assert(queue_family < queue_families_.size());
    return queue_families_[queue_family];
}

void Device::init_formats() {
    // For each 1.0 core format, undefined = first, 12x12_SRGB_BLOCK = last
    for (int f = VK_FORMAT_UNDEFINED; f <= VK_FORMAT_ASTC_12x12_SRGB_BLOCK; f++) {
        const VkFormat fmt = static_cast<VkFormat>(f);
        const VkFormatProperties props = format_properties(fmt);

        if (props.linearTilingFeatures) {
            const Format tmp = {fmt, VK_IMAGE_TILING_LINEAR, props.linearTilingFeatures};
            formats_.push_back(tmp);
        }

        if (props.optimalTilingFeatures) {
            const Format tmp = {fmt, VK_IMAGE_TILING_OPTIMAL, props.optimalTilingFeatures};
            formats_.push_back(tmp);
        }
    }

    EXPECT(!formats_.empty());
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

void Device::wait() { EXPECT(vk::DeviceWaitIdle(handle()) == VK_SUCCESS); }

VkResult Device::wait(const std::vector<const Fence *> &fences, bool wait_all, uint64_t timeout) {
    const std::vector<VkFence> fence_handles = MakeVkHandles<VkFence>(fences);
    VkResult err = vk::WaitForFences(handle(), fence_handles.size(), fence_handles.data(), wait_all, timeout);
    EXPECT(err == VK_SUCCESS || err == VK_TIMEOUT);

    return err;
}

void Device::update_descriptor_sets(const std::vector<VkWriteDescriptorSet> &writes,
                                    const std::vector<VkCopyDescriptorSet> &copies) {
    vk::UpdateDescriptorSets(handle(), writes.size(), writes.data(), copies.size(), copies.data());
}

VkResult Queue::submit(const std::vector<const CommandBuffer *> &cmds, const Fence &fence, bool expect_success) {
    const std::vector<VkCommandBuffer> cmd_handles = MakeVkHandles<VkCommandBuffer>(cmds);
    VkSubmitInfo submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = NULL;
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = NULL;
    submit_info.pWaitDstStageMask = NULL;
    submit_info.commandBufferCount = (uint32_t)cmd_handles.size();
    submit_info.pCommandBuffers = cmd_handles.data();
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = NULL;

    VkResult result = vk::QueueSubmit(handle(), 1, &submit_info, fence.handle());
    if (expect_success) EXPECT(result == VK_SUCCESS);
    return result;
}

VkResult Queue::submit(const CommandBuffer &cmd, const Fence &fence, bool expect_success) {
    return submit(std::vector<const CommandBuffer *>(1, &cmd), fence, expect_success);
}

VkResult Queue::submit(const CommandBuffer &cmd, bool expect_success) {
    Fence fence;
    return submit(cmd, fence);
}

VkResult Queue::wait() {
    VkResult result = vk::QueueWaitIdle(handle());
    EXPECT(result == VK_SUCCESS);
    return result;
}

DeviceMemory::~DeviceMemory() NOEXCEPT {
    if (initialized()) vk::FreeMemory(device(), handle(), NULL);
}

void DeviceMemory::init(const Device &dev, const VkMemoryAllocateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::AllocateMemory, dev, &info);
}

const void *DeviceMemory::map(VkFlags flags) const {
    void *data;
    if (!EXPECT(vk::MapMemory(device(), handle(), 0, VK_WHOLE_SIZE, flags, &data) == VK_SUCCESS)) data = NULL;

    return data;
}

void *DeviceMemory::map(VkFlags flags) {
    void *data;
    if (!EXPECT(vk::MapMemory(device(), handle(), 0, VK_WHOLE_SIZE, flags, &data) == VK_SUCCESS)) data = NULL;

    return data;
}

void DeviceMemory::unmap() const { vk::UnmapMemory(device(), handle()); }

VkMemoryAllocateInfo DeviceMemory::get_resource_alloc_info(const Device &dev, const VkMemoryRequirements &reqs,
                                                           VkMemoryPropertyFlags mem_props) {
    // Find appropriate memory type for given reqs
    VkPhysicalDeviceMemoryProperties dev_mem_props = dev.phy().memory_properties();
    uint32_t mem_type_index = 0;
    for (mem_type_index = 0; mem_type_index < dev_mem_props.memoryTypeCount; ++mem_type_index) {
        if (mem_props == (mem_props & dev_mem_props.memoryTypes[mem_type_index].propertyFlags)) break;
    }
    // If we exceeded types, then this device doesn't have the memory we need
    assert(mem_type_index < dev_mem_props.memoryTypeCount);
    VkMemoryAllocateInfo info = alloc_info(reqs.size, mem_type_index);
    EXPECT(dev.phy().set_memory_type(reqs.memoryTypeBits, &info, mem_props));
    return info;
}

NON_DISPATCHABLE_HANDLE_DTOR(Fence, vk::DestroyFence)

void Fence::init(const Device &dev, const VkFenceCreateInfo &info) { NON_DISPATCHABLE_HANDLE_INIT(vk::CreateFence, dev, &info); }

VkResult Fence::wait(uint64_t timeout) const {
    VkFence fence = handle();
    return vk::WaitForFences(device(), 1, &fence, VK_TRUE, timeout);
}

NON_DISPATCHABLE_HANDLE_DTOR(Semaphore, vk::DestroySemaphore)

void Semaphore::init(const Device &dev, const VkSemaphoreCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateSemaphore, dev, &info);
}

NON_DISPATCHABLE_HANDLE_DTOR(Event, vk::DestroyEvent)

void Event::init(const Device &dev, const VkEventCreateInfo &info) { NON_DISPATCHABLE_HANDLE_INIT(vk::CreateEvent, dev, &info); }

void Event::set() { EXPECT(vk::SetEvent(device(), handle()) == VK_SUCCESS); }

void Event::reset() { EXPECT(vk::ResetEvent(device(), handle()) == VK_SUCCESS); }

NON_DISPATCHABLE_HANDLE_DTOR(QueryPool, vk::DestroyQueryPool)

void QueryPool::init(const Device &dev, const VkQueryPoolCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateQueryPool, dev, &info);
}

VkResult QueryPool::results(uint32_t first, uint32_t count, size_t size, void *data, size_t stride) {
    VkResult err = vk::GetQueryPoolResults(device(), handle(), first, count, size, data, stride, 0);
    EXPECT(err == VK_SUCCESS || err == VK_NOT_READY);

    return err;
}

NON_DISPATCHABLE_HANDLE_DTOR(Buffer, vk::DestroyBuffer)

void Buffer::init(const Device &dev, const VkBufferCreateInfo &info, VkMemoryPropertyFlags mem_props) {
    init_no_mem(dev, info);

    internal_mem_.init(dev, DeviceMemory::get_resource_alloc_info(dev, memory_requirements(), mem_props));
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

void Buffer::bind_memory(const DeviceMemory &mem, VkDeviceSize mem_offset) {
    EXPECT(vk::BindBufferMemory(device(), handle(), mem.handle(), mem_offset) == VK_SUCCESS);
}

NON_DISPATCHABLE_HANDLE_DTOR(BufferView, vk::DestroyBufferView)

void BufferView::init(const Device &dev, const VkBufferViewCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateBufferView, dev, &info);
}

NON_DISPATCHABLE_HANDLE_DTOR(Image, vk::DestroyImage)

void Image::init(const Device &dev, const VkImageCreateInfo &info, VkMemoryPropertyFlags mem_props) {
    init_no_mem(dev, info);

    if (initialized()) {
        internal_mem_.init(dev, DeviceMemory::get_resource_alloc_info(dev, memory_requirements(), mem_props));
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

void Image::bind_memory(const DeviceMemory &mem, VkDeviceSize mem_offset) {
    EXPECT(vk::BindImageMemory(device(), handle(), mem.handle(), mem_offset) == VK_SUCCESS);
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

NON_DISPATCHABLE_HANDLE_DTOR(ImageView, vk::DestroyImageView)

void ImageView::init(const Device &dev, const VkImageViewCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateImageView, dev, &info);
}

AccelerationStructure::~AccelerationStructure() {
    if (initialized()) {
        if (isKHR) {
            PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR =
                (PFN_vkDestroyAccelerationStructureKHR)vk::GetDeviceProcAddr(device(), "vkDestroyAccelerationStructureKHR");
            assert(vkDestroyAccelerationStructureKHR != nullptr);
            vkDestroyAccelerationStructureKHR(device(), handle(), nullptr);
        } else {
            PFN_vkDestroyAccelerationStructureNV vkDestroyAccelerationStructureNV =
                (PFN_vkDestroyAccelerationStructureNV)vk::GetDeviceProcAddr(device(), "vkDestroyAccelerationStructureNV");
            assert(vkDestroyAccelerationStructureNV != nullptr);

            vkDestroyAccelerationStructureNV(device(), handle(), nullptr);
        }
    }
}

VkMemoryRequirements2 AccelerationStructure::memory_requirements(bool isKHR) const {
    PFN_vkGetAccelerationStructureMemoryRequirementsNV vkGetAccelerationStructureMemoryRequirementsNV =
        (PFN_vkGetAccelerationStructureMemoryRequirementsNV)vk::GetDeviceProcAddr(device(),
                                                                                  "vkGetAccelerationStructureMemoryRequirementsNV");
    assert(!(!isKHR && vkGetAccelerationStructureMemoryRequirementsNV == nullptr));

    PFN_vkGetAccelerationStructureMemoryRequirementsKHR vkGetAccelerationStructureMemoryRequirementsKHR =
        (PFN_vkGetAccelerationStructureMemoryRequirementsKHR)vk::GetDeviceProcAddr(
            device(), "vkGetAccelerationStructureMemoryRequirementsKHR");
    assert(!(isKHR && vkGetAccelerationStructureMemoryRequirementsKHR == nullptr));
    VkMemoryRequirements2 memoryRequirements = {};
    if (isKHR) {
        VkAccelerationStructureMemoryRequirementsInfoKHR memoryRequirementsInfo = {};
        memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR;
        memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_KHR;
        memoryRequirementsInfo.accelerationStructure = handle();
        memoryRequirementsInfo.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;
        vkGetAccelerationStructureMemoryRequirementsKHR(device(), &memoryRequirementsInfo, &memoryRequirements);
    } else {
        VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo = {};
        memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
        memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
        memoryRequirementsInfo.accelerationStructure = handle();
        vkGetAccelerationStructureMemoryRequirementsNV(device(), &memoryRequirementsInfo, &memoryRequirements);
    }
    return memoryRequirements;
}

VkMemoryRequirements2 AccelerationStructure::build_scratch_memory_requirements() const {
    PFN_vkGetAccelerationStructureMemoryRequirementsNV vkGetAccelerationStructureMemoryRequirementsNV =
        (PFN_vkGetAccelerationStructureMemoryRequirementsNV)vk::GetDeviceProcAddr(device(),
                                                                                  "vkGetAccelerationStructureMemoryRequirementsNV");
    assert(vkGetAccelerationStructureMemoryRequirementsNV != nullptr);

    VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo = {};
    memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
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

        VkBindAccelerationStructureMemoryInfoNV bind_info = {};
        bind_info.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
        bind_info.accelerationStructure = handle();
        bind_info.memory = memory_.handle();
        EXPECT(vkBindAccelerationStructureMemoryNV(dev.handle(), 1, &bind_info) == VK_SUCCESS);

        PFN_vkGetAccelerationStructureHandleNV vkGetAccelerationStructureHandleNV =
            (PFN_vkGetAccelerationStructureHandleNV)vk::GetDeviceProcAddr(dev.handle(), "vkGetAccelerationStructureHandleNV");
        assert(vkGetAccelerationStructureHandleNV != nullptr);
        EXPECT(vkGetAccelerationStructureHandleNV(dev.handle(), handle(), sizeof(uint64_t), &opaque_handle_) == VK_SUCCESS);
    }
}

void AccelerationStructure::initKHR(const Device &dev, const VkAccelerationStructureCreateInfoKHR &info, bool init_memory) {
    PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR =
        (PFN_vkCreateAccelerationStructureKHR)vk::GetDeviceProcAddr(dev.handle(), "vkCreateAccelerationStructureKHR");
    assert(vkCreateAccelerationStructureKHR != nullptr);
    NON_DISPATCHABLE_HANDLE_INIT(vkCreateAccelerationStructureKHR, dev, &info);
    if (init_memory) {
        memory_.init(dev, DeviceMemory::get_resource_alloc_info(dev, memory_requirements(true).memoryRequirements,
                                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
        PFN_vkBindAccelerationStructureMemoryKHR vkBindAccelerationStructureMemoryKHR =
            (PFN_vkBindAccelerationStructureMemoryKHR)vk::GetDeviceProcAddr(dev.handle(), "vkBindAccelerationStructureMemoryKHR");
        assert(vkBindAccelerationStructureMemoryKHR != nullptr);
        VkBindAccelerationStructureMemoryInfoKHR bind_info = {};
        bind_info.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_KHR;
        bind_info.accelerationStructure = handle();
        bind_info.memory = memory_.handle();
        EXPECT(vkBindAccelerationStructureMemoryKHR(dev.handle(), 1, &bind_info) == VK_SUCCESS);
    }
}
void AccelerationStructure::create_scratch_buffer(const Device &dev, Buffer *buffer, VkBufferCreateInfo *pCreateInfo) {
    VkMemoryRequirements scratch_buffer_memory_requirements = build_scratch_memory_requirements().memoryRequirements;

    VkBufferCreateInfo create_info = {};
    create_info.size = scratch_buffer_memory_requirements.size;
    if (pCreateInfo) {
        create_info.sType = pCreateInfo->sType;
        create_info.usage = pCreateInfo->usage;
    } else {
        create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        create_info.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
    }
    buffer->init(dev, create_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
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

NON_DISPATCHABLE_HANDLE_DTOR(Pipeline, vk::DestroyPipeline)

void Pipeline::init(const Device &dev, const VkGraphicsPipelineCreateInfo &info) {
    VkPipelineCache cache;
    VkPipelineCacheCreateInfo ci;
    memset((void *)&ci, 0, sizeof(VkPipelineCacheCreateInfo));
    ci.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VkResult err = vk::CreatePipelineCache(dev.handle(), &ci, NULL, &cache);
    if (err == VK_SUCCESS) {
        NON_DISPATCHABLE_HANDLE_INIT(vk::CreateGraphicsPipelines, dev, cache, 1, &info);
        vk::DestroyPipelineCache(dev.handle(), cache, NULL);
    }
}

VkResult Pipeline::init_try(const Device &dev, const VkGraphicsPipelineCreateInfo &info) {
    VkPipeline pipe;
    VkPipelineCache cache;
    VkPipelineCacheCreateInfo ci;
    memset((void *)&ci, 0, sizeof(VkPipelineCacheCreateInfo));
    ci.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VkResult err = vk::CreatePipelineCache(dev.handle(), &ci, NULL, &cache);
    EXPECT(err == VK_SUCCESS);
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
    VkPipelineCacheCreateInfo ci;
    memset((void *)&ci, 0, sizeof(VkPipelineCacheCreateInfo));
    ci.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
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
    info.setLayoutCount = layout_handles.size();
    info.pSetLayouts = layout_handles.data();

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

void DescriptorPool::reset() { EXPECT(vk::ResetDescriptorPool(device(), handle(), 0) == VK_SUCCESS); }

std::vector<DescriptorSet *> DescriptorPool::alloc_sets(const Device &dev,
                                                        const std::vector<const DescriptorSetLayout *> &layouts) {
    const std::vector<VkDescriptorSetLayout> layout_handles = MakeVkHandles<VkDescriptorSetLayout>(layouts);

    std::vector<VkDescriptorSet> set_handles;
    set_handles.resize(layout_handles.size());

    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorSetCount = layout_handles.size();
    alloc_info.descriptorPool = handle();
    alloc_info.pSetLayouts = layout_handles.data();
    VkResult err = vk::AllocateDescriptorSets(device(), &alloc_info, set_handles.data());
    EXPECT(err == VK_SUCCESS);

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

DescriptorSet::~DescriptorSet() NOEXCEPT {
    if (initialized()) {
        // Only call vk::Free* on sets allocated from pool with usage *_DYNAMIC
        if (containing_pool_->getDynamicUsage()) {
            VkDescriptorSet sets[1] = {handle()};
            EXPECT(vk::FreeDescriptorSets(device(), containing_pool_->GetObj(), 1, sets) == VK_SUCCESS);
        }
    }
}

NON_DISPATCHABLE_HANDLE_DTOR(CommandPool, vk::DestroyCommandPool)

void CommandPool::init(const Device &dev, const VkCommandPoolCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateCommandPool, dev, &info);
}

CommandBuffer::~CommandBuffer() NOEXCEPT {
    if (initialized()) {
        VkCommandBuffer cmds[] = {handle()};
        vk::FreeCommandBuffers(dev_handle_, cmd_pool_, 1, cmds);
    }
}

void CommandBuffer::init(const Device &dev, const VkCommandBufferAllocateInfo &info) {
    VkCommandBuffer cmd;

    // Make sure commandPool is set
    assert(info.commandPool);

    if (EXPECT(vk::AllocateCommandBuffers(dev.handle(), &info, &cmd) == VK_SUCCESS)) {
        Handle::init(cmd);
        dev_handle_ = dev.handle();
        cmd_pool_ = info.commandPool;
    }
}

void CommandBuffer::begin(const VkCommandBufferBeginInfo *info) { EXPECT(vk::BeginCommandBuffer(handle(), info) == VK_SUCCESS); }

void CommandBuffer::begin() {
    VkCommandBufferBeginInfo info = {};
    VkCommandBufferInheritanceInfo hinfo = {};
    info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pInheritanceInfo = &hinfo;
    hinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    hinfo.pNext = NULL;
    hinfo.renderPass = VK_NULL_HANDLE;
    hinfo.subpass = 0;
    hinfo.framebuffer = VK_NULL_HANDLE;
    hinfo.occlusionQueryEnable = VK_FALSE;
    hinfo.queryFlags = 0;
    hinfo.pipelineStatistics = 0;

    begin(&info);
}

void CommandBuffer::end() { EXPECT(vk::EndCommandBuffer(handle()) == VK_SUCCESS); }

void CommandBuffer::reset(VkCommandBufferResetFlags flags) { EXPECT(vk::ResetCommandBuffer(handle(), flags) == VK_SUCCESS); }

}  // namespace vk_testing
