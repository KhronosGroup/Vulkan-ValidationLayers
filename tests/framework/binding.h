/*
 * Copyright (c) 2015-2016, 2020-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2016, 2020-2023 Valve Corporation
 * Copyright (c) 2015-2016, 2020-2023 LunarG, Inc.
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

#include <algorithm>
#include <cassert>
#include <iterator>
#include <memory>
#include <vector>

#include "generated/vk_function_pointers.h"
#include "generated/vk_extension_helper.h"
#include "test_common.h"

namespace vkt {

template <class Dst, class Src>
std::vector<Dst> MakeVkHandles(const std::vector<Src> &v) {
    std::vector<Dst> handles;
    handles.reserve(v.size());
    std::transform(v.begin(), v.end(), std::back_inserter(handles), [](const Src &o) { return o.handle(); });
    return handles;
}

template <class Dst, class Src>
std::vector<Dst> MakeVkHandles(const std::vector<Src *> &v) {
    std::vector<Dst> handles;
    handles.reserve(v.size());
    std::transform(v.begin(), v.end(), std::back_inserter(handles),
                   [](const Src *o) { return (o) ? o->handle() : VK_NULL_HANDLE; });
    return handles;
}

class PhysicalDevice;
class Device;
class Queue;
class DeviceMemory;
class Fence;
class Semaphore;
class Event;
class QueryPool;
class Buffer;
class BufferView;
class Image;
class ImageView;
class DepthStencilView;
class Pipeline;
class PipelineDelta;
class Sampler;
class DescriptorSetLayout;
class PipelineLayout;
class DescriptorSetPool;
class DescriptorSet;
class CommandBuffer;
class CommandPool;

std::vector<VkLayerProperties> GetGlobalLayers();
std::vector<VkExtensionProperties> GetGlobalExtensions();
std::vector<VkExtensionProperties> GetGlobalExtensions(const char *pLayerName);

namespace internal {

template <typename T>
class Handle {
  public:
    const T &handle() const noexcept { return handle_; }
    bool initialized() const noexcept { return (handle_ != T{}); }

    operator T() const noexcept { return handle(); }
    operator bool() const noexcept { return initialized(); }

  protected:
    typedef T handle_type;

    explicit Handle() noexcept : handle_{} {}
    explicit Handle(T handle) noexcept : handle_(handle) {}

    // handles are non-copyable
    Handle(const Handle &) = delete;
    Handle &operator=(const Handle &) = delete;

    // handles can be moved out
    Handle(Handle &&src) noexcept : handle_{src.handle_} { src.handle_ = {}; }
    Handle &operator=(Handle &&src) noexcept {
        handle_ = src.handle_;
        src.handle_ = {};
        return *this;
    }

    void init(T handle) noexcept {
        assert(!initialized());
        handle_ = handle;
    }

  protected:
    T handle_;
};

template <typename T>
class NonDispHandle : public Handle<T> {
  protected:
    explicit NonDispHandle() noexcept : Handle<T>(), dev_handle_(VK_NULL_HANDLE) {}
    explicit NonDispHandle(VkDevice dev, T handle) noexcept : Handle<T>(handle), dev_handle_(dev) {}

    NonDispHandle(NonDispHandle &&src) noexcept : Handle<T>(std::move(src)) {
        dev_handle_ = src.dev_handle_;
        src.dev_handle_ = VK_NULL_HANDLE;
    }
    NonDispHandle &operator=(NonDispHandle &&src) noexcept {
        Handle<T>::operator=(std::move(src));
        dev_handle_ = src.dev_handle_;
        src.dev_handle_ = VK_NULL_HANDLE;
        return *this;
    }

    const VkDevice &device() const noexcept { return dev_handle_; }

    void init(VkDevice dev, T handle) noexcept {
        assert(!Handle<T>::initialized() && dev_handle_ == VK_NULL_HANDLE);
        Handle<T>::init(handle);
        dev_handle_ = dev;
    }

    void destroy() noexcept { dev_handle_ = VK_NULL_HANDLE; }

  private:
    VkDevice dev_handle_;
};

}  // namespace internal

class PhysicalDevice : public internal::Handle<VkPhysicalDevice> {
  public:
    explicit PhysicalDevice(VkPhysicalDevice phy)
        : Handle(phy),
          properties_(properties()),
          limits_(properties_.limits),
          memory_properties_(memory_properties()),
          queue_properties_(queue_properties()) {}

    VkPhysicalDeviceFeatures features() const;

    bool set_memory_type(const uint32_t type_bits, VkMemoryAllocateInfo *info, const VkMemoryPropertyFlags properties,
                         const VkMemoryPropertyFlags forbid = 0) const;

    // vkEnumerateDeviceExtensionProperties()
    std::vector<VkExtensionProperties> extensions(const char *pLayerName = nullptr) const;

    // vkEnumerateLayers()
    std::vector<VkLayerProperties> layers() const;

    const VkPhysicalDeviceProperties properties_;
    const VkPhysicalDeviceLimits limits_;
    const VkPhysicalDeviceMemoryProperties memory_properties_;
    const std::vector<VkQueueFamilyProperties> queue_properties_;

  private:
    void add_extension_dependencies(uint32_t dependency_count, VkExtensionProperties *depencency_props,
                                    std::vector<VkExtensionProperties> &ext_list);
    VkPhysicalDeviceProperties properties() const;
    std::vector<VkQueueFamilyProperties> queue_properties() const;
    VkPhysicalDeviceMemoryProperties memory_properties() const;
};

class QueueCreateInfoArray {
  private:
    std::vector<VkDeviceQueueCreateInfo> queue_info_;
    std::vector<std::vector<float>> queue_priorities_;

  public:
    QueueCreateInfoArray(const std::vector<VkQueueFamilyProperties> &queue_props);
    size_t size() const { return queue_info_.size(); }
    const VkDeviceQueueCreateInfo *data() const { return queue_info_.data(); }
};

class Device : public internal::Handle<VkDevice> {
  public:
    explicit Device(VkPhysicalDevice phy) : phy_(phy) { init(); }
    explicit Device(VkPhysicalDevice phy, const VkDeviceCreateInfo &info) : phy_(phy) { init(info); }
    explicit Device(VkPhysicalDevice phy, std::vector<const char *> &extension_names, VkPhysicalDeviceFeatures *features = nullptr,
                    void *create_device_pnext = nullptr)
        : phy_(phy) {
        init(extension_names, features, create_device_pnext);
    }

    ~Device() noexcept;
    void destroy() noexcept;

    // vkCreateDevice()
    void init(const VkDeviceCreateInfo &info);
    void init(std::vector<const char *> &extensions, VkPhysicalDeviceFeatures *features = nullptr,
              void *create_device_pnext = nullptr);  // all queues, all extensions, etc
    void init() {
        std::vector<const char *> extensions;
        init(extensions);
    };

    VkDevice device() { return handle(); }
    const PhysicalDevice &phy() const { return phy_; }

    std::vector<const char *> GetEnabledExtensions() { return enabled_extensions_; }
    bool IsEnabledExtension(const char *extension);

    // vkGetDeviceProcAddr()
    PFN_vkVoidFunction get_proc(const char *name) const { return vk::GetDeviceProcAddr(handle(), name); }

    // vkGetDeviceQueue()
    const std::vector<Queue *> &graphics_queues() const { return queues_[GRAPHICS]; }
    const std::vector<Queue *> &compute_queues() { return queues_[COMPUTE]; }
    const std::vector<Queue *> &dma_queues() { return queues_[DMA]; }

    typedef std::vector<std::unique_ptr<Queue>> QueueFamilyQueues;
    typedef std::vector<QueueFamilyQueues> QueueFamilies;
    const QueueFamilyQueues &queue_family_queues(uint32_t queue_family) const;

    // Find a queue family with and without desired capabilities
    std::optional<uint32_t> QueueFamilyMatching(VkQueueFlags with, VkQueueFlags without, bool all_bits = true);
    std::optional<uint32_t> QueueFamilyWithoutCapabilities(VkQueueFlags capabilities) {
        // an all_bits match with 0 matches all
        return QueueFamilyMatching(VkQueueFlags(0), capabilities, true /* all_bits with */);
    }

    uint32_t graphics_queue_node_index_;

    const PhysicalDevice phy_;

    struct Format {
        VkFormat format;
        VkImageTiling tiling;
        VkFlags features;
    };
    // vkGetFormatInfo()
    VkFormatProperties format_properties(VkFormat format);
    const std::vector<Format> &formats() const { return formats_; }

    // vkDeviceWaitIdle()
    void wait();

    // vkWaitForFences()
    VkResult wait(const std::vector<const Fence *> &fences, bool wait_all, uint64_t timeout);
    VkResult wait(const Fence &fence) { return wait(std::vector<const Fence *>(1, &fence), true, (uint64_t)-1); }

    // vkUpdateDescriptorSets()
    void update_descriptor_sets(const std::vector<VkWriteDescriptorSet> &writes, const std::vector<VkCopyDescriptorSet> &copies);
    void update_descriptor_sets(const std::vector<VkWriteDescriptorSet> &writes) {
        return update_descriptor_sets(writes, std::vector<VkCopyDescriptorSet>());
    }

    static VkWriteDescriptorSet write_descriptor_set(const DescriptorSet &set, uint32_t binding, uint32_t array_element,
                                                     VkDescriptorType type, uint32_t count,
                                                     const VkDescriptorImageInfo *image_info);
    static VkWriteDescriptorSet write_descriptor_set(const DescriptorSet &set, uint32_t binding, uint32_t array_element,
                                                     VkDescriptorType type, uint32_t count,
                                                     const VkDescriptorBufferInfo *buffer_info);
    static VkWriteDescriptorSet write_descriptor_set(const DescriptorSet &set, uint32_t binding, uint32_t array_element,
                                                     VkDescriptorType type, uint32_t count, const VkBufferView *buffer_views);
    static VkWriteDescriptorSet write_descriptor_set(const DescriptorSet &set, uint32_t binding, uint32_t array_element,
                                                     VkDescriptorType type, const std::vector<VkDescriptorImageInfo> &image_info);
    static VkWriteDescriptorSet write_descriptor_set(const DescriptorSet &set, uint32_t binding, uint32_t array_element,
                                                     VkDescriptorType type, const std::vector<VkDescriptorBufferInfo> &buffer_info);
    static VkWriteDescriptorSet write_descriptor_set(const DescriptorSet &set, uint32_t binding, uint32_t array_element,
                                                     VkDescriptorType type, const std::vector<VkBufferView> &buffer_views);

    static VkCopyDescriptorSet copy_descriptor_set(const DescriptorSet &src_set, uint32_t src_binding, uint32_t src_array_element,
                                                   const DescriptorSet &dst_set, uint32_t dst_binding, uint32_t dst_array_element,
                                                   uint32_t count);

  private:
    enum QueueIndex {
        GRAPHICS,
        COMPUTE,
        DMA,
        QUEUE_COUNT,
    };

    void init_queues(const VkDeviceCreateInfo &info);
    void init_formats();

    std::vector<const char *> enabled_extensions_;

    QueueFamilies queue_families_;
    std::vector<Queue *> queues_[QUEUE_COUNT];
    std::vector<Format> formats_;
};

class Queue : public internal::Handle<VkQueue> {
  public:
    explicit Queue(VkQueue queue, uint32_t index) : Handle(queue) { family_index_ = index; }

    // vkQueueSubmit()
    VkResult submit(const std::vector<const CommandBuffer *> &cmds, const Fence &fence, bool expect_success = true);
    VkResult submit(const CommandBuffer &cmd, const Fence &fence, bool expect_success = true);
    VkResult submit(const CommandBuffer &cmd, bool expect_success = true);
    // vkQueueSubmit2()
    VkResult submit2(const std::vector<const CommandBuffer *> &cmds, const Fence &fence, bool expect_success = true);
    VkResult submit2(const CommandBuffer &cmd, const Fence &fence, bool expect_success = true);

    // vkQueueWaitIdle()
    VkResult wait();

    uint32_t get_family_index() const { return family_index_; }

  private:
    uint32_t family_index_;
};

class DeviceMemory : public internal::NonDispHandle<VkDeviceMemory> {
  public:
    DeviceMemory() = default;
    DeviceMemory(const Device &dev, const VkMemoryAllocateInfo &info) { init(dev, info); }
    ~DeviceMemory() noexcept;
    void destroy() noexcept;
    DeviceMemory &operator=(DeviceMemory &&) = default;

    // vkAllocateMemory()
    // Fails the test when allocation is unsuccessful
    void init(const Device &dev, const VkMemoryAllocateInfo &info);
    // Does not fail the test when allocation is unsuccessful and instead returns error code
    VkResult try_init(const Device &dev, const VkMemoryAllocateInfo &info);

    // vkMapMemory()
    const void *map(VkFlags flags) const;
    void *map(VkFlags flags);
    const void *map() const { return map(0); }
    void *map() { return map(0); }

    // vkUnmapMemory()
    void unmap() const;
	const auto &get_memory_allocate_info() { return memory_allocate_info_; }

        static VkMemoryAllocateInfo get_resource_alloc_info(const vkt::Device &dev, const VkMemoryRequirements &reqs,
                                                            VkMemoryPropertyFlags mem_props, void *alloc_info_pnext = nullptr);

      private:
        VkMemoryAllocateInfo memory_allocate_info_{};
};

class Fence : public internal::NonDispHandle<VkFence> {
  public:
    Fence() = default;
    Fence(const Device &dev) { init(dev, create_info()); }
    Fence(const Device &dev, const VkFenceCreateInfo &info) { init(dev, info); }
    ~Fence() noexcept;
    void destroy() noexcept;

    // vkCreateFence()
    void init(const Device &dev, const VkFenceCreateInfo &info);

    // vkGetFenceStatus()
    VkResult status() const { return vk::GetFenceStatus(device(), handle()); }
    VkResult wait(uint64_t timeout) const;

    VkResult reset();

#ifdef VK_USE_PLATFORM_WIN32_KHR
    VkResult export_handle(HANDLE &win32_handle, VkExternalFenceHandleTypeFlagBits handle_type);
    VkResult import_handle(HANDLE win32_handle, VkExternalFenceHandleTypeFlagBits handle_type, VkFenceImportFlags flags = 0);
#endif
    VkResult export_handle(int &fd_handle, VkExternalFenceHandleTypeFlagBits handle_type);
    VkResult import_handle(int fd_handle, VkExternalFenceHandleTypeFlagBits handle_type, VkFenceImportFlags flags = 0);

    static VkFenceCreateInfo create_info(VkFenceCreateFlags flags);
    static VkFenceCreateInfo create_info();
};

class Semaphore : public internal::NonDispHandle<VkSemaphore> {
  public:
    Semaphore() = default;
    Semaphore(const Device &dev) { init(dev, vku::InitStruct<VkSemaphoreCreateInfo>()); }
    Semaphore(const Device &dev, const VkSemaphoreCreateInfo &info) { init(dev, info); }
    ~Semaphore() noexcept;
    void destroy() noexcept;

    // vkCreateSemaphore()
    void init(const Device &dev, const VkSemaphoreCreateInfo &info);

#ifdef VK_USE_PLATFORM_WIN32_KHR
    VkResult export_handle(HANDLE &win32_handle, VkExternalSemaphoreHandleTypeFlagBits handle_type);
    VkResult import_handle(HANDLE win32_handle, VkExternalSemaphoreHandleTypeFlagBits handle_type,
                           VkSemaphoreImportFlags flags = 0);
#endif
    VkResult export_handle(int &fd_handle, VkExternalSemaphoreHandleTypeFlagBits handle_type);
    VkResult import_handle(int fd_handle, VkExternalSemaphoreHandleTypeFlagBits handle_type, VkSemaphoreImportFlags flags = 0);

    static VkSemaphoreCreateInfo create_info(VkFlags flags);
};

class Event : public internal::NonDispHandle<VkEvent> {
  public:
    Event() = default;
    Event(const Device &dev) { init(dev, vku::InitStruct<VkEventCreateInfo>()); }
    Event(const Device &dev, const VkEventCreateInfo &info) { init(dev, info); }
    ~Event() noexcept;
    void destroy() noexcept;

    // vkCreateEvent()
    void init(const Device &dev, const VkEventCreateInfo &info);

    // vkGetEventStatus()
    // vkSetEvent()
    // vkResetEvent()
    VkResult status() const { return vk::GetEventStatus(device(), handle()); }
    void set();
    void cmd_set(const CommandBuffer &cmd, VkPipelineStageFlags stage_mask);
    void cmd_reset(const CommandBuffer &cmd, VkPipelineStageFlags stage_mask);
    void cmd_wait(const CommandBuffer &cmd, VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
                  const std::vector<VkMemoryBarrier> &memory_barriers, const std::vector<VkBufferMemoryBarrier> &buffer_barriers,
                  const std::vector<VkImageMemoryBarrier> &image_barriers);
    void reset();

    static VkEventCreateInfo create_info(VkFlags flags);
};

class QueryPool : public internal::NonDispHandle<VkQueryPool> {
  public:
    QueryPool() = default;
    QueryPool(const Device &dev, const VkQueryPoolCreateInfo &info) { init(dev, info); }
    ~QueryPool() noexcept;
    void destroy() noexcept;

    // vkCreateQueryPool()
    void init(const Device &dev, const VkQueryPoolCreateInfo &info);

    // vkGetQueryPoolResults()
    VkResult results(uint32_t first, uint32_t count, size_t size, void *data, size_t stride);

    static VkQueryPoolCreateInfo create_info(VkQueryType type, uint32_t slot_count);
};

struct NoMemT {};
static constexpr NoMemT no_mem{};

class Buffer : public internal::NonDispHandle<VkBuffer> {
  public:
    explicit Buffer() : NonDispHandle(), create_info_(vku::InitStruct<decltype(create_info_)>()) {}
    explicit Buffer(const Device &dev, const VkBufferCreateInfo &info, VkMemoryPropertyFlags mem_props = 0,
                    void *alloc_info_pnext = nullptr) {
        init(dev, info, mem_props, alloc_info_pnext);
    }
    explicit Buffer(const Device &dev, VkDeviceSize size, VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VkMemoryPropertyFlags mem_props = 0,
                    void *alloc_info_pnext = nullptr) {
        init(dev, size, usage, mem_props, alloc_info_pnext);
    }
    explicit Buffer(const Device &dev, const VkBufferCreateInfo &info, NoMemT) { init_no_mem(dev, info); }
    Buffer(Buffer &&rhs) noexcept : NonDispHandle(std::move(rhs)) {
        create_info_ = std::move(rhs.create_info_);
        internal_mem_ = std::move(rhs.internal_mem_);
    }
    Buffer &operator=(Buffer &&rhs) noexcept {
        if (&rhs == this) {
            return *this;
        }
        destroy();
        internal_mem_.destroy();
        NonDispHandle::operator=(std::move(rhs));
        create_info_ = std::move(rhs.create_info_);
        internal_mem_ = std::move(rhs.internal_mem_);
        return *this;
    }
    ~Buffer() noexcept;
    void destroy() noexcept;

    // vkCreateBuffer()
    void init(const Device &dev, const VkBufferCreateInfo &info, VkMemoryPropertyFlags mem_props = 0,
              void *alloc_info_pnext = nullptr);
    void init(const Device &dev, VkDeviceSize size, VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
              VkMemoryPropertyFlags mem_props = 0, void *alloc_info_pnext = nullptr,
              const std::vector<uint32_t> &queue_families = {}) {
        init(dev, create_info(size, usage, &queue_families), mem_props, alloc_info_pnext);
    }
    void init_no_mem(const Device &dev, const VkBufferCreateInfo &info);

    // get the internal memory
    const DeviceMemory &memory() const { return internal_mem_; }
    DeviceMemory &memory() { return internal_mem_; }

    // vkGetObjectMemoryRequirements()
    VkMemoryRequirements memory_requirements() const;

    // Allocate and bind memory
    // The assumption that this object was created in no_mem configuration
    void allocate_and_bind_memory(const Device &dev, VkMemoryPropertyFlags mem_props = 0, void *alloc_info_pnext = nullptr);

    // Bind to existing memory object
    void bind_memory(const DeviceMemory &mem, VkDeviceSize mem_offset);

    const VkBufferCreateInfo &create_info() const { return create_info_; }
    static VkBufferCreateInfo create_info(VkDeviceSize size, VkFlags usage, const std::vector<uint32_t> *queue_families = nullptr,
                                          void *create_info_pnext = nullptr);

    VkBufferMemoryBarrier buffer_memory_barrier(VkFlags output_mask, VkFlags input_mask, VkDeviceSize offset,
                                                VkDeviceSize size) const {
        VkBufferMemoryBarrier barrier = vku::InitStructHelper();
        barrier.buffer = handle();
        barrier.srcAccessMask = output_mask;
        barrier.dstAccessMask = input_mask;
        barrier.offset = offset;
        barrier.size = size;
        if (create_info_.sharingMode == VK_SHARING_MODE_CONCURRENT) {
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        }
        return barrier;
    }

    VkBufferMemoryBarrier2KHR buffer_memory_barrier(VkPipelineStageFlags2KHR src_stage, VkPipelineStageFlags2KHR dst_stage,
                                                    VkAccessFlags2KHR src_access, VkAccessFlags2KHR dst_access, VkDeviceSize offset,
                                                    VkDeviceSize size) const {
        VkBufferMemoryBarrier2KHR barrier = vku::InitStructHelper();
        barrier.buffer = handle();
        barrier.srcStageMask = src_stage;
        barrier.dstStageMask = dst_stage;
        barrier.srcAccessMask = src_access;
        barrier.dstAccessMask = dst_access;
        barrier.offset = offset;
        barrier.size = size;
        if (create_info_.sharingMode == VK_SHARING_MODE_CONCURRENT) {
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        }
        return barrier;
    }

    [[nodiscard]] VkDeviceAddress address() const;

  private:
    VkBufferCreateInfo create_info_;
    DeviceMemory internal_mem_;
};

class BufferView : public internal::NonDispHandle<VkBufferView> {
  public:
    BufferView() = default;
    BufferView(const Device &dev, const VkBufferViewCreateInfo &info) { init(dev, info); }
    ~BufferView() noexcept;
    void destroy() noexcept;

    // vkCreateBufferView()
    void init(const Device &dev, const VkBufferViewCreateInfo &info);
    static VkBufferViewCreateInfo createInfo(VkBuffer buffer, VkFormat format, VkDeviceSize offset = 0,
                                             VkDeviceSize range = VK_WHOLE_SIZE);
};

inline VkBufferViewCreateInfo BufferView::createInfo(VkBuffer buffer, VkFormat format, VkDeviceSize offset, VkDeviceSize range) {
    VkBufferViewCreateInfo info = vku::InitStructHelper();
    info.flags = VkFlags(0);
    info.buffer = buffer;
    info.format = format;
    info.offset = offset;
    info.range = range;
    return info;
}

class Image : public internal::NonDispHandle<VkImage> {
  public:
    explicit Image() : NonDispHandle(), format_features_(0) {}
    explicit Image(const Device &dev, const VkImageCreateInfo &info) : format_features_(0) { init(dev, info); }
    explicit Image(const Device &dev, const VkImageCreateInfo &info, VkMemoryPropertyFlags mem_props,
                   void *alloc_info_pnext = nullptr);
    explicit Image(const Device &dev, const VkImageCreateInfo &info, NoMemT) : format_features_(0) { init_no_mem(dev, info); }

    ~Image() noexcept;
    void destroy() noexcept;

    // vkCreateImage()
    void init(const Device &dev, const VkImageCreateInfo &info, VkMemoryPropertyFlags mem_props, void *alloc_info_pnext = nullptr);
    void init(const Device &dev, const VkImageCreateInfo &info) { init(dev, info, 0); }
    void init_no_mem(const Device &dev, const VkImageCreateInfo &info);

    // get the internal memory
    const DeviceMemory &memory() const { return internal_mem_; }
    DeviceMemory &memory() { return internal_mem_; }

    // vkGetObjectMemoryRequirements()
    VkMemoryRequirements memory_requirements() const;

    // Allocate and bind memory
    // The assumption that this object was created in no_mem configuration
    void allocate_and_bind_memory(const Device &dev, VkMemoryPropertyFlags mem_props = 0, void *alloc_info_pnext = nullptr);

    // Bind to existing memory object
    void bind_memory(const DeviceMemory &mem, VkDeviceSize mem_offset);

    // vkGetImageSubresourceLayout()
    VkSubresourceLayout subresource_layout(const VkImageSubresource &subres) const;
    VkSubresourceLayout subresource_layout(const VkImageSubresourceLayers &subres) const;

    bool transparent() const;
    bool copyable() const { return (format_features_ & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT); }

    VkImageAspectFlags aspect_mask() const { return aspect_mask(create_info_.format); }

    VkImageSubresourceRange subresource_range() const { return subresource_range(create_info_, aspect_mask()); }
    VkImageSubresourceRange subresource_range(VkImageAspectFlags aspect) const { return subresource_range(create_info_, aspect); }

    VkExtent3D extent() const { return create_info_.extent; }
    VkFormat format() const { return create_info_.format; }
    VkImageUsageFlags usage() const { return create_info_.usage; }
    VkSharingMode sharing_mode() const { return create_info_.sharingMode; }
    VkImageMemoryBarrier image_memory_barrier(VkFlags output_mask, VkFlags input_mask, VkImageLayout old_layout,
                                              VkImageLayout new_layout, const VkImageSubresourceRange &range,
                                              uint32_t srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                              uint32_t dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED) const {
        VkImageMemoryBarrier barrier = vku::InitStructHelper();
        barrier.srcAccessMask = output_mask;
        barrier.dstAccessMask = input_mask;
        barrier.oldLayout = old_layout;
        barrier.newLayout = new_layout;
        barrier.image = handle();
        barrier.subresourceRange = range;
        barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
        barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;
        return barrier;
    }

    VkImageMemoryBarrier2KHR image_memory_barrier(VkPipelineStageFlags2KHR src_stage, VkPipelineStageFlags2KHR dst_stage,
                                                  VkAccessFlags2KHR src_access, VkAccessFlags2KHR dst_access,
                                                  VkImageLayout old_layout, VkImageLayout new_layout,
                                                  const VkImageSubresourceRange &range,
                                                  uint32_t srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                                  uint32_t dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED) const {
        VkImageMemoryBarrier2KHR barrier = vku::InitStructHelper();
        barrier.srcStageMask = src_stage;
        barrier.dstStageMask = dst_stage;
        barrier.srcAccessMask = src_access;
        barrier.dstAccessMask = dst_access;
        barrier.oldLayout = old_layout;
        barrier.newLayout = new_layout;
        barrier.image = handle();
        barrier.subresourceRange = range;
        barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
        barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;
        return barrier;
    }

    static VkImageCreateInfo create_info();
    static VkImageSubresource subresource(VkImageAspectFlags aspect, uint32_t mip_level, uint32_t array_layer);
    static VkImageSubresource subresource(const VkImageSubresourceRange &range, uint32_t mip_level, uint32_t array_layer);
    static VkImageSubresourceLayers subresource(VkImageAspectFlags aspect, uint32_t mip_level, uint32_t array_layer,
                                                uint32_t array_size);
    static VkImageSubresourceLayers subresource(const VkImageSubresourceRange &range, uint32_t mip_level, uint32_t array_layer,
                                                uint32_t array_size);
    static VkImageSubresourceRange subresource_range(VkImageAspectFlags aspect_mask, uint32_t base_mip_level, uint32_t mip_levels,
                                                     uint32_t base_array_layer, uint32_t num_layers);
    static VkImageSubresourceRange subresource_range(const VkImageCreateInfo &info, VkImageAspectFlags aspect_mask);
    static VkImageSubresourceRange subresource_range(const VkImageSubresource &subres);

    static VkImageAspectFlags aspect_mask(VkFormat format);

    static VkExtent2D extent(int32_t width, int32_t height);
    static VkExtent2D extent(const VkExtent3D &extent);

    static VkExtent3D extent(int32_t width, int32_t height, int32_t depth);

  private:
    void init_info(const Device &dev, const VkImageCreateInfo &info);

    VkImageCreateInfo create_info_;
    VkFlags format_features_;

    DeviceMemory internal_mem_;
};

class ImageView : public internal::NonDispHandle<VkImageView> {
  public:
    explicit ImageView() = default;
    explicit ImageView(const Device &dev, const VkImageViewCreateInfo &info) { init(dev, info); }
    ~ImageView() noexcept;
    void destroy() noexcept;

    // vkCreateImageView()
    void init(const Device &dev, const VkImageViewCreateInfo &info);
};

class AccelerationStructure : public internal::NonDispHandle<VkAccelerationStructureNV> {
  public:
    explicit AccelerationStructure(const Device &dev, const VkAccelerationStructureCreateInfoNV &info, bool init_memory = true) {
        init(dev, info, init_memory);
    }
    ~AccelerationStructure() noexcept;
    void destroy() noexcept;

    // vkCreateAccelerationStructureNV
    void init(const Device &dev, const VkAccelerationStructureCreateInfoNV &info, bool init_memory = true);
    // vkGetAccelerationStructureMemoryRequirementsNV()
    VkMemoryRequirements2 memory_requirements() const;
    VkMemoryRequirements2 build_scratch_memory_requirements() const;

    uint64_t opaque_handle() const { return opaque_handle_; }

    const VkAccelerationStructureInfoNV &info() const { return info_; }

    const VkDevice &dev() const { return device(); }

    [[nodiscard]] vkt::Buffer create_scratch_buffer(const Device &device, VkBufferCreateInfo *pCreateInfo = nullptr,
                                                    bool buffer_device_address = false) const;

  private:
    VkAccelerationStructureInfoNV info_;
    DeviceMemory memory_;
    uint64_t opaque_handle_;
};

class ShaderModule : public internal::NonDispHandle<VkShaderModule> {
  public:
    ~ShaderModule() noexcept;
    void destroy() noexcept;

    // vkCreateShaderModule()
    void init(const Device &dev, const VkShaderModuleCreateInfo &info);
    VkResult init_try(const Device &dev, const VkShaderModuleCreateInfo &info);

    static VkShaderModuleCreateInfo create_info(size_t code_size, const uint32_t *code, VkFlags flags);
};

class Shader : public internal::NonDispHandle<VkShaderEXT> {
  public:
    Shader(const Device &dev, const VkShaderCreateInfoEXT &info) { init(dev, info); }
    Shader(const Device &dev, const VkShaderStageFlagBits stage, const std::vector<uint32_t> &spv,
           const VkDescriptorSetLayout *descriptorSetLayout = nullptr, const VkPushConstantRange* pushConstRange = nullptr);
    Shader(const Device &dev, const VkShaderStageFlagBits stage, const std::vector<uint8_t> &binary,
           const VkDescriptorSetLayout *descriptorSetLayout = nullptr, const VkPushConstantRange *pushConstRange = nullptr);
    Shader(const Device &dev, const VkShaderStageFlagBits stage, const std::vector<uint32_t> &spv,
           VkShaderCreateFlagsEXT flags);
    ~Shader() noexcept;
    void destroy() noexcept;

    // vkCreateShaderModule()
    void init(const Device &dev, const VkShaderCreateInfoEXT &info);
    VkResult init_try(const Device &dev, const VkShaderCreateInfoEXT &info);
};

class Pipeline : public internal::NonDispHandle<VkPipeline> {
  public:
    Pipeline() = default;
    Pipeline(const Device &dev, const VkGraphicsPipelineCreateInfo &info) { init(dev, info); }
    Pipeline(const Device &dev, const VkGraphicsPipelineCreateInfo &info, const VkPipeline basePipeline) {
        init(dev, info, basePipeline);
    }
    Pipeline(const Device &dev, const VkComputePipelineCreateInfo &info) { init(dev, info); }
    ~Pipeline() noexcept;
    void destroy() noexcept;

    // vkCreateGraphicsPipeline()
    void init(const Device &dev, const VkGraphicsPipelineCreateInfo &info);
    // vkCreateGraphicsPipelineDerivative()
    void init(const Device &dev, const VkGraphicsPipelineCreateInfo &info, const VkPipeline basePipeline);
    // vkCreateComputePipeline()
    void init(const Device &dev, const VkComputePipelineCreateInfo &info);
    // vkLoadPipeline()
    void init(const Device &dev, size_t size, const void *data);
    // vkLoadPipelineDerivative()
    void init(const Device &dev, size_t size, const void *data, VkPipeline basePipeline);

    // vkCreateGraphicsPipeline with error return
    VkResult init_try(const Device &dev, const VkGraphicsPipelineCreateInfo &info);

    // vkStorePipeline()
    size_t store(size_t size, void *data);
};

class PipelineLayout : public internal::NonDispHandle<VkPipelineLayout> {
  public:
    PipelineLayout() noexcept : NonDispHandle() {}
    PipelineLayout(const Device &dev, VkPipelineLayoutCreateInfo &info,
                   const std::vector<const DescriptorSetLayout *> &layouts) {
        init(dev, info, layouts);
    }
    PipelineLayout(const Device &dev, VkPipelineLayoutCreateInfo &info) {
        init(dev, info);
    }
    PipelineLayout(const Device &dev, const std::vector<const DescriptorSetLayout *> &layouts = {},
                   const std::vector<VkPushConstantRange> &push_constant_ranges = {},
                   VkPipelineLayoutCreateFlags flags = static_cast<VkPipelineLayoutCreateFlags>(0)) {
        VkPipelineLayoutCreateInfo info = vku::InitStructHelper();
        info.flags = flags;
        info.pushConstantRangeCount = static_cast<uint32_t>(push_constant_ranges.size());
        info.pPushConstantRanges = push_constant_ranges.data();

        init(dev, info, layouts);
    }
    ~PipelineLayout() noexcept;
    void destroy() noexcept;

    // Move constructor for Visual Studio 2013
    PipelineLayout(PipelineLayout &&src) noexcept : NonDispHandle(std::move(src)){};

    PipelineLayout &operator=(PipelineLayout &&src) noexcept {
        this->~PipelineLayout();
        this->NonDispHandle::operator=(std::move(src));
        return *this;
    };

    // vCreatePipelineLayout()
    void init(const Device &dev, VkPipelineLayoutCreateInfo &info, const std::vector<const DescriptorSetLayout *> &layouts);
    void init(const Device &dev, VkPipelineLayoutCreateInfo &info);
};

class Sampler : public internal::NonDispHandle<VkSampler> {
  public:
    Sampler() = default;
    Sampler(const Device &dev, const VkSamplerCreateInfo &info) { init(dev, info); }
    ~Sampler() noexcept;
    void destroy() noexcept;

    // vkCreateSampler()
    void init(const Device &dev, const VkSamplerCreateInfo &info);
};

class DescriptorSetLayout : public internal::NonDispHandle<VkDescriptorSetLayout> {
  public:
    DescriptorSetLayout() noexcept : NonDispHandle(){};
    DescriptorSetLayout(const Device &dev, const VkDescriptorSetLayoutCreateInfo &info) { init(dev, info); }
    DescriptorSetLayout(const Device &dev, const std::vector<VkDescriptorSetLayoutBinding> &descriptor_set_bindings = {},
                        VkDescriptorSetLayoutCreateFlags flags = 0, void *pNext = nullptr) {
        VkDescriptorSetLayoutCreateInfo info = vku::InitStructHelper(pNext);
        info.flags = flags;
        info.bindingCount = static_cast<uint32_t>(descriptor_set_bindings.size());
        info.pBindings = descriptor_set_bindings.data();
        init(dev, info);
    }

    ~DescriptorSetLayout() noexcept;
    void destroy() noexcept;

    // Move constructor for Visual Studio 2013
    DescriptorSetLayout(DescriptorSetLayout &&src) noexcept : NonDispHandle(std::move(src)){};

    DescriptorSetLayout &operator=(DescriptorSetLayout &&src) noexcept {
        this->~DescriptorSetLayout();
        this->NonDispHandle::operator=(std::move(src));
        return *this;
    }

    // vkCreateDescriptorSetLayout()
    void init(const Device &dev, const VkDescriptorSetLayoutCreateInfo &info);
};

class DescriptorPool : public internal::NonDispHandle<VkDescriptorPool> {
  public:
    DescriptorPool() = default;
    DescriptorPool(const Device &dev, const VkDescriptorPoolCreateInfo &info) { init(dev, info); }
    ~DescriptorPool() noexcept;
    void destroy() noexcept;

    // vkCreateDescriptorPool()
    void init(const Device &dev, const VkDescriptorPoolCreateInfo &info);

    // vkResetDescriptorPool()
    void reset();

    // vkFreeDescriptorSet()
    void setDynamicUsage(bool isDynamic) { dynamic_usage_ = isDynamic; }
    bool getDynamicUsage() { return dynamic_usage_; }

    // vkAllocateDescriptorSets()
    std::vector<DescriptorSet *> alloc_sets(const Device &dev, const std::vector<const DescriptorSetLayout *> &layouts);
    std::vector<DescriptorSet *> alloc_sets(const Device &dev, const DescriptorSetLayout &layout, uint32_t count);
    DescriptorSet *alloc_sets(const Device &dev, const DescriptorSetLayout &layout);

    template <typename PoolSizes>
    static VkDescriptorPoolCreateInfo create_info(VkDescriptorPoolCreateFlags flags, uint32_t max_sets,
                                                  const PoolSizes &pool_sizes);

  private:
    // Track whether this pool's usage is VK_DESCRIPTOR_POOL_USAGE_DYNAMIC
    bool dynamic_usage_;
};

template <typename PoolSizes>
inline VkDescriptorPoolCreateInfo DescriptorPool::create_info(VkDescriptorPoolCreateFlags flags, uint32_t max_sets,
                                                              const PoolSizes &pool_sizes) {
    VkDescriptorPoolCreateInfo info = vku::InitStructHelper();
    info.flags = flags;
    info.maxSets = max_sets;
    info.poolSizeCount = pool_sizes.size();
    info.pPoolSizes = (info.poolSizeCount) ? pool_sizes.data() : nullptr;
    return info;
}

class DescriptorSet : public internal::NonDispHandle<VkDescriptorSet> {
  public:
    ~DescriptorSet() noexcept;
    void destroy() noexcept;

    explicit DescriptorSet() : NonDispHandle() {}
    explicit DescriptorSet(const Device &dev, DescriptorPool *pool, VkDescriptorSet set) : NonDispHandle(dev.handle(), set) {
        containing_pool_ = pool;
    }

  private:
    DescriptorPool *containing_pool_;
};

class CommandPool : public internal::NonDispHandle<VkCommandPool> {
  public:
    ~CommandPool() noexcept;
    void destroy() noexcept;

    explicit CommandPool() : NonDispHandle() {}
    explicit CommandPool(const Device &dev, const VkCommandPoolCreateInfo &info) { init(dev, info); }
    explicit CommandPool(const Device &dev, uint32_t queue_family_index, VkCommandPoolCreateFlags flags = 0) {
        init(dev, vkt::CommandPool::create_info(queue_family_index, flags));
    }

    void init(const Device &dev, const VkCommandPoolCreateInfo &info);

    static VkCommandPoolCreateInfo create_info(uint32_t queue_family_index, VkCommandPoolCreateFlags flags);
};

inline VkCommandPoolCreateInfo CommandPool::create_info(uint32_t queue_family_index, VkCommandPoolCreateFlags flags) {
    VkCommandPoolCreateInfo info = vku::InitStructHelper();
    info.queueFamilyIndex = queue_family_index;
    info.flags = flags;
    return info;
}

class CommandBuffer : public internal::Handle<VkCommandBuffer> {
  public:
    ~CommandBuffer() noexcept;
    void destroy() noexcept;

    explicit CommandBuffer() : Handle() {}
    explicit CommandBuffer(const Device &dev, const VkCommandBufferAllocateInfo &info) { init(dev, info); }
    explicit CommandBuffer(Device *device, const CommandPool *pool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                           Queue *queue = nullptr) {
        Init(device, pool, level, queue);
    }

    // vkAllocateCommandBuffers()
    void init(const Device &dev, const VkCommandBufferAllocateInfo &info);
    void Init(Device *device, const CommandPool *pool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
              Queue *queue = nullptr);

    // vkBeginCommandBuffer()
    void begin(const VkCommandBufferBeginInfo *info);
    void begin();

    // vkEndCommandBuffer()
    // vkResetCommandBuffer()
    void end();
    void reset(VkCommandBufferResetFlags flags);
    void reset() { reset(VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT); }

    static VkCommandBufferAllocateInfo create_info(VkCommandPool const &pool);

    void BeginRenderPass(const VkRenderPassBeginInfo &info, VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);
    void NextSubpass(VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);
    void EndRenderPass();
    void BeginRendering(const VkRenderingInfoKHR &renderingInfo);
    void BeginRenderingColor(const VkImageView imageView);
    void EndRendering();

    void BeginVideoCoding(const VkVideoBeginCodingInfoKHR &beginInfo);
    void ControlVideoCoding(const VkVideoCodingControlInfoKHR &controlInfo);
    void DecodeVideo(const VkVideoDecodeInfoKHR &decodeInfo);
    void EndVideoCoding(const VkVideoEndCodingInfoKHR &endInfo);

    void QueueCommandBuffer(bool check_success = true);
    void QueueCommandBuffer(const Fence &fence, bool check_success = true, bool submit_2 = false);

    void SetEvent(Event &event, VkPipelineStageFlags stageMask) { event.cmd_set(*this, stageMask); }
    void ResetEvent(Event &event, VkPipelineStageFlags stageMask) { event.cmd_reset(*this, stageMask); }
    void WaitEvents(uint32_t eventCount, const VkEvent *pEvents, VkPipelineStageFlags srcStageMask,
                    VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                    uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers) {
        vk::CmdWaitEvents(handle(), eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers,
                          bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    }

  private:
    VkDevice dev_handle_;
    VkCommandPool cmd_pool_;
    Queue *m_queue;
};

class RenderPass : public internal::NonDispHandle<VkRenderPass> {
  public:
    RenderPass() = default;
    // vkCreateRenderPass()
    RenderPass(const Device &dev, const VkRenderPassCreateInfo &info) { init(dev, info); }
    // vkCreateRenderPass2()
    RenderPass(const Device &dev, const VkRenderPassCreateInfo2 &info, bool khr = false) { init(dev, info, khr); }
    ~RenderPass() noexcept;
    void destroy() noexcept;

    // vkCreateRenderPass()
    void init(const Device &dev, const VkRenderPassCreateInfo &info);
    // vkCreateRenderPass2()
    void init(const Device &dev, const VkRenderPassCreateInfo2 &info, bool khr = false);
};


class Framebuffer : public internal::NonDispHandle<VkFramebuffer> {
  public:
    Framebuffer() = default;
    Framebuffer(const Device &dev, const VkFramebufferCreateInfo &info) { init(dev, info); }
    ~Framebuffer() noexcept;
    void destroy() noexcept;

    // vkCreateFramebuffer()
    void init(const Device &dev, const VkFramebufferCreateInfo &info);
};

class SamplerYcbcrConversion : public internal::NonDispHandle<VkSamplerYcbcrConversion> {
  public:
    SamplerYcbcrConversion() = default;
    SamplerYcbcrConversion(const Device &dev, VkFormat format, bool khr = false) : khr_(khr) {
        init(dev, DefaultConversionInfo(format), khr);
    }
    SamplerYcbcrConversion(const Device &dev, const VkSamplerYcbcrConversionCreateInfo &info, bool khr = false) : khr_(khr) {
        init(dev, info, khr);
    }
    ~SamplerYcbcrConversion() noexcept;
    void destroy() noexcept;

    void init(const Device &dev, const VkSamplerYcbcrConversionCreateInfo &info, bool khr);
    VkSamplerYcbcrConversionInfo ConversionInfo();

    static VkSamplerYcbcrConversionCreateInfo DefaultConversionInfo(VkFormat format);

    bool khr_ = false;
};

inline VkBufferCreateInfo Buffer::create_info(VkDeviceSize size, VkFlags usage, const std::vector<uint32_t> *queue_families,
                                              void *create_info_pnext) {
    VkBufferCreateInfo info = vku::InitStructHelper(create_info_pnext);
    info.size = size;
    info.usage = usage;

    if (queue_families && queue_families->size() > 1) {
        info.sharingMode = VK_SHARING_MODE_CONCURRENT;
        info.queueFamilyIndexCount = static_cast<uint32_t>(queue_families->size());
        info.pQueueFamilyIndices = queue_families->data();
    }

    return info;
}

inline VkFenceCreateInfo Fence::create_info(VkFenceCreateFlags flags) {
    VkFenceCreateInfo info = vku::InitStructHelper();
    info.flags = flags;
    return info;
}

inline VkFenceCreateInfo Fence::create_info() {
    VkFenceCreateInfo info = vku::InitStructHelper();
    return info;
}

inline VkSemaphoreCreateInfo Semaphore::create_info(VkFlags flags) {
    VkSemaphoreCreateInfo info = vku::InitStructHelper();
    info.flags = flags;
    return info;
}

inline VkEventCreateInfo Event::create_info(VkFlags flags) {
    VkEventCreateInfo info = vku::InitStructHelper();
    info.flags = flags;
    return info;
}

inline VkQueryPoolCreateInfo QueryPool::create_info(VkQueryType type, uint32_t slot_count) {
    VkQueryPoolCreateInfo info = vku::InitStructHelper();
    info.queryType = type;
    info.queryCount = slot_count;
    return info;
}

inline VkImageCreateInfo Image::create_info() {
    VkImageCreateInfo info = vku::InitStructHelper();
    info.extent.width = 1;
    info.extent.height = 1;
    info.extent.depth = 1;
    info.mipLevels = 1;
    info.arrayLayers = 1;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    return info;
}

inline VkImageSubresource Image::subresource(VkImageAspectFlags aspect, uint32_t mip_level, uint32_t array_layer) {
    VkImageSubresource subres = {};
    if (aspect == 0) {
        assert(false && "Invalid VkImageAspectFlags");
    }
    subres.aspectMask = aspect;
    subres.mipLevel = mip_level;
    subres.arrayLayer = array_layer;
    return subres;
}

inline VkImageSubresource Image::subresource(const VkImageSubresourceRange &range, uint32_t mip_level, uint32_t array_layer) {
    return subresource(range.aspectMask, range.baseMipLevel + mip_level, range.baseArrayLayer + array_layer);
}

inline VkImageSubresourceLayers Image::subresource(VkImageAspectFlags aspect, uint32_t mip_level, uint32_t array_layer,
                                                   uint32_t array_size) {
    VkImageSubresourceLayers subres = {};
    switch (aspect) {
        case VK_IMAGE_ASPECT_COLOR_BIT:
        case VK_IMAGE_ASPECT_DEPTH_BIT:
        case VK_IMAGE_ASPECT_STENCIL_BIT:
        case VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT:
            /* valid */
            break;
        default:
            assert(false && "Invalid VkImageAspectFlags");
    }
    subres.aspectMask = aspect;
    subres.mipLevel = mip_level;
    subres.baseArrayLayer = array_layer;
    subres.layerCount = array_size;
    return subres;
}

inline VkImageSubresourceLayers Image::subresource(const VkImageSubresourceRange &range, uint32_t mip_level, uint32_t array_layer,
                                                   uint32_t array_size) {
    return subresource(range.aspectMask, range.baseMipLevel + mip_level, range.baseArrayLayer + array_layer, array_size);
}

inline VkImageSubresourceRange Image::subresource_range(VkImageAspectFlags aspect_mask, uint32_t base_mip_level,
                                                        uint32_t mip_levels, uint32_t base_array_layer, uint32_t num_layers) {
    VkImageSubresourceRange range = {};
    if (aspect_mask == 0) {
        assert(false && "Invalid VkImageAspectFlags");
    }
    range.aspectMask = aspect_mask;
    range.baseMipLevel = base_mip_level;
    range.levelCount = mip_levels;
    range.baseArrayLayer = base_array_layer;
    range.layerCount = num_layers;
    return range;
}

inline VkImageSubresourceRange Image::subresource_range(const VkImageCreateInfo &info, VkImageAspectFlags aspect_mask) {
    return subresource_range(aspect_mask, 0, info.mipLevels, 0, info.arrayLayers);
}

inline VkImageSubresourceRange Image::subresource_range(const VkImageSubresource &subres) {
    return subresource_range(subres.aspectMask, subres.mipLevel, 1, subres.arrayLayer, 1);
}

inline VkExtent2D Image::extent(int32_t width, int32_t height) {
    VkExtent2D extent = {};
    extent.width = width;
    extent.height = height;
    return extent;
}

inline VkExtent2D Image::extent(const VkExtent3D &extent) { return Image::extent(extent.width, extent.height); }

inline VkExtent3D Image::extent(int32_t width, int32_t height, int32_t depth) {
    VkExtent3D extent = {};
    extent.width = width;
    extent.height = height;
    extent.depth = depth;
    return extent;
}

inline VkShaderModuleCreateInfo ShaderModule::create_info(size_t code_size, const uint32_t *code, VkFlags flags) {
    VkShaderModuleCreateInfo info = vku::InitStructHelper();
    info.codeSize = code_size;
    info.pCode = code;
    info.flags = flags;
    return info;
}

inline VkWriteDescriptorSet Device::write_descriptor_set(const DescriptorSet &set, uint32_t binding, uint32_t array_element,
                                                         VkDescriptorType type, uint32_t count,
                                                         const VkDescriptorImageInfo *image_info) {
    VkWriteDescriptorSet write = vku::InitStructHelper();
    write.dstSet = set.handle();
    write.dstBinding = binding;
    write.dstArrayElement = array_element;
    write.descriptorCount = count;
    write.descriptorType = type;
    write.pImageInfo = image_info;
    return write;
}

inline VkWriteDescriptorSet Device::write_descriptor_set(const DescriptorSet &set, uint32_t binding, uint32_t array_element,
                                                         VkDescriptorType type, uint32_t count,
                                                         const VkDescriptorBufferInfo *buffer_info) {
    VkWriteDescriptorSet write = vku::InitStructHelper();
    write.dstSet = set.handle();
    write.dstBinding = binding;
    write.dstArrayElement = array_element;
    write.descriptorCount = count;
    write.descriptorType = type;
    write.pBufferInfo = buffer_info;
    return write;
}

inline VkWriteDescriptorSet Device::write_descriptor_set(const DescriptorSet &set, uint32_t binding, uint32_t array_element,
                                                         VkDescriptorType type, uint32_t count, const VkBufferView *buffer_views) {
    VkWriteDescriptorSet write = vku::InitStructHelper();
    write.dstSet = set.handle();
    write.dstBinding = binding;
    write.dstArrayElement = array_element;
    write.descriptorCount = count;
    write.descriptorType = type;
    write.pTexelBufferView = buffer_views;
    return write;
}

inline VkWriteDescriptorSet Device::write_descriptor_set(const DescriptorSet &set, uint32_t binding, uint32_t array_element,
                                                         VkDescriptorType type,
                                                         const std::vector<VkDescriptorImageInfo> &image_info) {
    return write_descriptor_set(set, binding, array_element, type, static_cast<uint32_t>(image_info.size()), &image_info[0]);
}

inline VkWriteDescriptorSet Device::write_descriptor_set(const DescriptorSet &set, uint32_t binding, uint32_t array_element,
                                                         VkDescriptorType type,
                                                         const std::vector<VkDescriptorBufferInfo> &buffer_info) {
    return write_descriptor_set(set, binding, array_element, type, static_cast<uint32_t>(buffer_info.size()), &buffer_info[0]);
}

inline VkWriteDescriptorSet Device::write_descriptor_set(const DescriptorSet &set, uint32_t binding, uint32_t array_element,
                                                         VkDescriptorType type, const std::vector<VkBufferView> &buffer_views) {
    return write_descriptor_set(set, binding, array_element, type, static_cast<uint32_t>(buffer_views.size()), &buffer_views[0]);
}

inline VkCopyDescriptorSet Device::copy_descriptor_set(const DescriptorSet &src_set, uint32_t src_binding,
                                                       uint32_t src_array_element, const DescriptorSet &dst_set,
                                                       uint32_t dst_binding, uint32_t dst_array_element, uint32_t count) {
    VkCopyDescriptorSet copy = vku::InitStructHelper();
    copy.srcSet = src_set.handle();
    copy.srcBinding = src_binding;
    copy.srcArrayElement = src_array_element;
    copy.dstSet = dst_set.handle();
    copy.dstBinding = dst_binding;
    copy.dstArrayElement = dst_array_element;
    copy.descriptorCount = count;

    return copy;
}

struct GraphicsPipelineLibraryStage {
    vvl::span<const uint32_t> spv;
    VkShaderModuleCreateInfo shader_ci;
    VkPipelineShaderStageCreateInfo stage_ci;

    GraphicsPipelineLibraryStage(vvl::span<const uint32_t> spv, VkShaderStageFlagBits stage, const char *name = "main") : spv(spv) {
        shader_ci = vku::InitStructHelper();
        shader_ci.codeSize = spv.size() * sizeof(uint32_t);
        shader_ci.pCode = spv.data();

        stage_ci = vku::InitStructHelper(&shader_ci);
        stage_ci.stage = stage;
        stage_ci.module = VK_NULL_HANDLE;
        stage_ci.pName = name;
    }
};

struct GraphicsPipelineFromLibraries {
    vvl::span<VkPipeline> libs;
    VkPipelineLibraryCreateInfoKHR link_info;
    vkt::Pipeline pipe;

    GraphicsPipelineFromLibraries(const Device &dev, vvl::span<VkPipeline> libs, VkPipelineLayout layout)
        : GraphicsPipelineFromLibraries(libs) {
        VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
        exe_pipe_ci.layout = layout;
        pipe.init(dev, exe_pipe_ci);
        pipe.initialized();
    }

    GraphicsPipelineFromLibraries(const Device &dev, vvl::span<VkPipeline> libs, VkGraphicsPipelineCreateInfo &ci)
        : GraphicsPipelineFromLibraries(libs) {
        link_info.pNext = ci.pNext;
        ci.pNext = &link_info;
        pipe.init(dev, ci);
        pipe.initialized();
    }

    operator VkPipeline() const { return pipe.handle(); }
    operator bool() const { return pipe.initialized(); }

  private:
    GraphicsPipelineFromLibraries(vvl::span<VkPipeline> libs) : libs(libs) {
        link_info = vku::InitStructHelper();
        link_info.libraryCount = static_cast<uint32_t>(libs.size());
        link_info.pLibraries = libs.data();
    }
};

}  // namespace vkt
