/* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (C) 2015-2026 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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
#include <variant>
#include "state_tracker/device_memory_state.h"
#include "state_tracker/device_range_state.h"
#include "containers/range.h"

namespace vvl {

class BufferSubState;
class BufferViewSubState;
class BufferAddressRangeSubState;
class DeviceState;
class VideoProfileDesc;

class Buffer : public Bindable, public SubStateManager<BufferSubState> {
    // We normally want to allow full access to the |create_info|
    // but due to things, such as VkBufferUsageFlags2CreateInfo, it is really easy
    // to not realize that |create_info.usage| is the wrong usage and you need to check the pNext.
    // The answer is for these cases, to force a getter function for the entire |create_info| to prevent bugs
    const vku::safe_VkBufferCreateInfo safe_create_info;
    const VkBufferCreateInfo &create_info;

  public:
    const VkMemoryRequirements requirements;
    VkDeviceAddress deviceAddress = 0;
    // VkBufferUsageFlags2CreateInfo can be used instead of the VkBufferCreateInfo::usage
    const VkBufferUsageFlags2 usage;

    unordered_set<std::shared_ptr<const VideoProfileDesc>> supported_video_profiles;

    Buffer(DeviceState &dev_data, VkBuffer handle, const VkBufferCreateInfo *pCreateInfo);

    Buffer(Buffer const &rh_obj) = delete;

    // This destructor is needed because Bindable depends on the tracker_ variant defined in this
    // class. So we need to do the Destroy() work before tracker_ is destroyed.
    virtual ~Buffer() {
        if (!Destroyed()) {
            Destroy();
        }
    }

    void Destroy() override;
    void NotifyInvalidate(const StateObject::NodeList &invalid_nodes, bool unlink) override;

    VkBuffer VkHandle() const { return handle_.Cast<VkBuffer>(); }

    VkBufferCreateFlags GetFlags() const { return create_info.flags; }
    VkDeviceSize GetSize() const { return create_info.size; }
    VkSharingMode GetSharingMode() const { return create_info.sharingMode; }
    uint32_t GetQueueFamilyIndexCount() const { return create_info.queueFamilyIndexCount; }
    const uint32_t* GetQueueFamilyIndices() const { return create_info.pQueueFamilyIndices; }

    vvl::range<VkDeviceAddress> DeviceAddressRange() const { return {deviceAddress, deviceAddress + create_info.size}; }

    // This function is only used for comparing Imported External Dedicated Memory
    bool CompareCreateInfo(const Buffer &other) const;

    // Used to help unify the way we print the BDA info
    std::string Describe(const Logger& dev_data) const;

  private:
    std::variant<std::monostate, BindableLinearMemoryTracker, BindableSparseMemoryTracker> tracker_;
};

class BufferSubState {
  public:
    explicit BufferSubState(Buffer &buf) : base(buf) {}
    BufferSubState(const BufferSubState &) = delete;
    BufferSubState &operator=(const BufferSubState &) = delete;
    virtual ~BufferSubState() {}
    virtual void Destroy() {}
    virtual void NotifyInvalidate(const StateObject::NodeList&, bool) {}

    Buffer &base;
};

class BufferView : public StateObject, public SubStateManager<BufferViewSubState> {
  public:
    const vku::safe_VkBufferViewCreateInfo safe_create_info;
    const VkBufferViewCreateInfo &create_info;

    std::shared_ptr<Buffer> buffer_state;
#ifdef VK_USE_PLATFORM_METAL_EXT
    const bool metal_bufferview_export;
#endif  // VK_USE_PLATFORM_METAL_EXT
    // Format features that matter when accessing the buffer
    // both as a buffer (ex OpLoad) or image (ex OpImageWrite)
    const VkFormatFeatureFlags2 buffer_format_features;

    BufferView(const std::shared_ptr<Buffer> &bf, VkBufferView handle, const VkBufferViewCreateInfo *pCreateInfo,
               VkFormatFeatureFlags2 format_features);

    void LinkChildNodes() override {
        // Connect child node(s), which cannot safely be done in the constructor.
        buffer_state->AddParent(this);
    }
    virtual ~BufferView() {
        if (!Destroyed()) {
            Destroy();
        }
    }

    BufferView(const BufferView &rh_obj) = delete;

    VkBufferView VkHandle() const { return handle_.Cast<VkBufferView>(); }

    void Destroy() override;

    void NotifyInvalidate(const StateObject::NodeList &invalid_nodes, bool unlink) override;

    bool Invalid() const override { return Destroyed() || !buffer_state || buffer_state->Invalid(); }

    VkDeviceSize Size() const {
        VkDeviceSize size = create_info.range;
        if (size == VK_WHOLE_SIZE) {
            size = buffer_state->GetSize() - create_info.offset;
        }
        return size;
    }
};

class BufferViewSubState {
  public:
    explicit BufferViewSubState(BufferView &buf) : base(buf) {}
    BufferViewSubState(const BufferViewSubState &) = delete;
    BufferViewSubState &operator=(const BufferViewSubState &) = delete;
    virtual ~BufferViewSubState() {}
    virtual void Destroy() {}
    virtual void NotifyInvalidate(const StateObject::NodeList&, bool) {}

    BufferView &base;
};

// As the API went from using VkBuffer to VkDevicesAddress/VkDeviceSize.
// The issue become when there are 2 VkBuffer that can actually be tied to it.
//
// Example:
//   VkDeviceMemory is from [0x1000, 0x2000]
//   VkBuffer A is bound to [0x1000, 0x2000]
//   VkBuffer B is also bound to [0x1000, 0x2000]
//
// In this case it is valid to delete VkBuffer A or B, but once both are destroyed, the VkDeviceAddress becomes invalid.
//
// We agreed (https://gitlab.khronos.org/vulkan/vulkan/-/issues/4665) that the case like
//
//   VkDeviceMemory is from [0x1000, 0x2000]
//   VkBuffer A is bound to [0x1000, 0x2000]
//   VkBuffer B is bound to [0x1000, 0x1800]
//   VkBuffer C is bound to [0x1800, 0x2000]
//
// If VkBuffer A is destroyed, it is now invalid to use the range [0x1000, 0x2000] regardless of the various sub-buffers covering it
class BufferAddressRange : public StateObject {
  public:
    // We use a small_vector because we can assume most apps don't do buffer memory aliasing
    // But it is common for middleware to want a copy of the buffer, so we set the size to 2
    small_vector<vvl::Buffer*, 2> buffer_states;

    // This holds the information we will want to print any error message
    // We "trick" VkHandleInfo by providing it a pointer instead of a normal vulkan handle
    InternalDeviceRange internal_range;

    BufferAddressRange(small_vector<vvl::Buffer*, 2> buffer_states, const vvl::range<VkDeviceAddress> range,
                       VkBufferUsageFlags2 usage);

    void LinkChildNodes() override {
        // Connect child node(s), which cannot safely be done in the constructor.
        for (const auto &buffer_state : buffer_states) {
            buffer_state->AddParent(this);
        }
    }
    virtual ~BufferAddressRange() {
        if (!Destroyed()) {
            Destroy();
        }
    }

    BufferAddressRange(const BufferAddressRange &rh_obj) = delete;

    void Destroy() override;

    void NotifyInvalidate(const StateObject::NodeList &invalid_nodes, bool unlink) override;

    // Only invalid if ALL the buffers in the range are invalid
    bool Invalid() const override {
        if (Destroyed()) {
            return true;
        }
        for (const auto *buffer_state : buffer_states) {
            if (buffer_state && !buffer_state->Invalid()) {
                return false;
            }
        }
        return true;
    }
};

}  // namespace vvl
