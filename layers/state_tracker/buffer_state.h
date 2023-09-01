/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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
class VideoProfileDesc;

class BUFFER_STATE : public BINDABLE {
  public:
    const safe_VkBufferCreateInfo safe_create_info;
    const VkBufferCreateInfo &createInfo;
    const VkMemoryRequirements requirements;
    VkDeviceAddress deviceAddress = 0;
    // VkBufferUsageFlags2CreateInfoKHR can be used instead over the VkBufferCreateInfo::usage
    const VkBufferUsageFlags2KHR usage;

    vvl::unordered_set<std::shared_ptr<const VideoProfileDesc>> supported_video_profiles;

    BUFFER_STATE(ValidationStateTracker *dev_data, VkBuffer buff, const VkBufferCreateInfo *pCreateInfo);

    BUFFER_STATE(BUFFER_STATE const &rh_obj) = delete;

    // This destructor is needed because BINDABLE depends on the tracker_ variant defined in this
    // class. So we need to do the Destroy() work before tracker_ is destroyed.
    virtual ~BUFFER_STATE() {
        if (!Destroyed()) {
            BINDABLE::Destroy();
        }
    }

    VkBuffer buffer() const { return handle_.Cast<VkBuffer>(); }
    std::optional<VkDeviceSize> ComputeValidSize(VkDeviceSize offset, VkDeviceSize size) const {
        return ::ComputeValidSize(offset, size, createInfo.size);
    }
    VkDeviceSize ComputeSize(VkDeviceSize offset, VkDeviceSize size) const {
        std::optional<VkDeviceSize> valid_size = ComputeValidSize(offset, size);
        return valid_size.has_value() ? *valid_size : VkDeviceSize(0);
    }
    static VkDeviceSize ComputeSize(const std::shared_ptr<const BUFFER_STATE> &buffer_state, VkDeviceSize offset,
                                    VkDeviceSize size) {
        return buffer_state ? buffer_state->ComputeSize(offset, size) : VkDeviceSize(0);
    }

    sparse_container::range<VkDeviceAddress> DeviceAddressRange() const { return {deviceAddress, deviceAddress + createInfo.size}; }

  private:
    std::variant<std::monostate, BindableLinearMemoryTracker, BindableSparseMemoryTracker> tracker_;
};

class BUFFER_VIEW_STATE : public BASE_NODE {
  public:
    const VkBufferViewCreateInfo create_info;
    std::shared_ptr<BUFFER_STATE> buffer_state;
#ifdef VK_USE_PLATFORM_METAL_EXT
    const bool metal_bufferview_export;
#endif  // VK_USE_PLATFORM_METAL_EXT
    // Format features that matter when accessing the buffer
    // both as a buffer (ex OpLoad) or image (ex OpImageWrite)
    const VkFormatFeatureFlags2KHR buf_format_features;

    BUFFER_VIEW_STATE(const std::shared_ptr<BUFFER_STATE> &bf, VkBufferView bv, const VkBufferViewCreateInfo *ci,
                      VkFormatFeatureFlags2KHR buf_ff);

    void LinkChildNodes() override {
        // Connect child node(s), which cannot safely be done in the constructor.
        buffer_state->AddParent(this);
    }
    virtual ~BUFFER_VIEW_STATE() {
        if (!Destroyed()) {
            Destroy();
        }
    }

    BUFFER_VIEW_STATE(const BUFFER_VIEW_STATE &rh_obj) = delete;

    VkBufferView buffer_view() const { return handle_.Cast<VkBufferView>(); }

    void Destroy() override {
        if (buffer_state) {
            buffer_state->RemoveParent(this);
            buffer_state = nullptr;
        }
        BASE_NODE::Destroy();
    }
    bool Invalid() const override { return Destroyed() || !buffer_state || buffer_state->Invalid(); }

    VkDeviceSize Size() const {
        VkDeviceSize size = create_info.range;
        if (size == VK_WHOLE_SIZE) {
            size = buffer_state->createInfo.size - create_info.offset;
        }
        return size;
    }
};
