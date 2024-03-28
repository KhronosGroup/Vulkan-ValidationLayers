/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
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
#include "containers/range_vector.h"

class ValidationStateTracker;

namespace vvl {

class VideoProfileDesc;

class Buffer : public Bindable {
  public:
    const vku::safe_VkBufferCreateInfo safe_create_info;
    const VkBufferCreateInfo &create_info;

    const VkMemoryRequirements requirements;
    VkDeviceAddress deviceAddress = 0;
    // VkBufferUsageFlags2CreateInfoKHR can be used instead over the VkBufferCreateInfo::usage
    const VkBufferUsageFlags2KHR usage;

    unordered_set<std::shared_ptr<const VideoProfileDesc>> supported_video_profiles;

    Buffer(ValidationStateTracker &dev_data, VkBuffer handle, const VkBufferCreateInfo *pCreateInfo);

    Buffer(Buffer const &rh_obj) = delete;

    // This destructor is needed because Bindable depends on the tracker_ variant defined in this
    // class. So we need to do the Destroy() work before tracker_ is destroyed.
    virtual ~Buffer() {
        if (!Destroyed()) {
            Bindable::Destroy();
        }
    }

    VkBuffer VkHandle() const { return handle_.Cast<VkBuffer>(); }
    std::optional<VkDeviceSize> ComputeValidSize(VkDeviceSize offset, VkDeviceSize size) const {
        return ::ComputeValidSize(offset, size, create_info.size);
    }
    VkDeviceSize ComputeSize(VkDeviceSize offset, VkDeviceSize size) const {
        std::optional<VkDeviceSize> valid_size = ComputeValidSize(offset, size);
        return valid_size.has_value() ? *valid_size : VkDeviceSize(0);
    }
    static VkDeviceSize ComputeSize(const std::shared_ptr<const Buffer> &buffer_state, VkDeviceSize offset,
                                    VkDeviceSize size) {
        return buffer_state ? buffer_state->ComputeSize(offset, size) : VkDeviceSize(0);
    }

    sparse_container::range<VkDeviceAddress> DeviceAddressRange() const {
        return {deviceAddress, deviceAddress + create_info.size};
    }

  private:
    std::variant<std::monostate, BindableLinearMemoryTracker, BindableSparseMemoryTracker> tracker_;
};

class BufferView : public StateObject {
  public:
    const vku::safe_VkBufferViewCreateInfo safe_create_info;
    const VkBufferViewCreateInfo &create_info;

    std::shared_ptr<Buffer> buffer_state;
#ifdef VK_USE_PLATFORM_METAL_EXT
    const bool metal_bufferview_export;
#endif  // VK_USE_PLATFORM_METAL_EXT
    // Format features that matter when accessing the buffer
    // both as a buffer (ex OpLoad) or image (ex OpImageWrite)
    const VkFormatFeatureFlags2KHR buffer_format_features;

    BufferView(const std::shared_ptr<Buffer> &bf, VkBufferView handle, const VkBufferViewCreateInfo *pCreateInfo,
               VkFormatFeatureFlags2KHR format_features);

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

    void Destroy() override {
        if (buffer_state) {
            buffer_state->RemoveParent(this);
            buffer_state = nullptr;
        }
        StateObject::Destroy();
    }
    bool Invalid() const override { return Destroyed() || !buffer_state || buffer_state->Invalid(); }

    VkDeviceSize Size() const {
        VkDeviceSize size = create_info.range;
        if (size == VK_WHOLE_SIZE) {
            size = buffer_state->create_info.size - create_info.offset;
        }
        return size;
    }
};

}  // namespace vvl
