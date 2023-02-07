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
#include "state_tracker/device_memory_state.h"
#include "range_vector.h"

class ValidationStateTracker;
class VideoProfileDesc;

class BUFFER_STATE : public BINDABLE {
  public:
    const safe_VkBufferCreateInfo safe_create_info;
    const VkBufferCreateInfo &createInfo;
    VkDeviceAddress deviceAddress;
    const VkMemoryRequirements requirements;
    const VkMemoryRequirements *const memory_requirements_pointer = &requirements;
    bool memory_requirements_checked;

    vvl::unordered_set<std::shared_ptr<const VideoProfileDesc>> supported_video_profiles;

    BUFFER_STATE(ValidationStateTracker *dev_data, VkBuffer buff, const VkBufferCreateInfo *pCreateInfo);

    BUFFER_STATE(BUFFER_STATE const &rh_obj) = delete;

    VkBuffer buffer() const { return handle_.Cast<VkBuffer>(); }

    sparse_container::range<VkDeviceAddress> DeviceAddressRange() const { return {deviceAddress, deviceAddress + createInfo.size}; }
};

using BUFFER_STATE_LINEAR = MEMORY_TRACKED_RESOURCE_STATE<BUFFER_STATE, BindableLinearMemoryTracker>;
template <bool IS_RESIDENT>
using BUFFER_STATE_SPARSE = MEMORY_TRACKED_RESOURCE_STATE<BUFFER_STATE, BindableSparseMemoryTracker<IS_RESIDENT>>;

#ifdef VK_USE_PLATFORM_METAL_EXT
static bool GetMetalExport(const VkBufferViewCreateInfo *info) {
    bool retval = false;
    auto export_metal_object_info = LvlFindInChain<VkExportMetalObjectCreateInfoEXT>(info->pNext);
    while (export_metal_object_info) {
        if (export_metal_object_info->exportObjectType == VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT) {
            retval = true;
            break;
        }
        export_metal_object_info = LvlFindInChain<VkExportMetalObjectCreateInfoEXT>(export_metal_object_info->pNext);
    }
    return retval;
}
#endif

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
                      VkFormatFeatureFlags2KHR buf_ff)
        : BASE_NODE(bv, kVulkanObjectTypeBufferView),
          create_info(*ci),
          buffer_state(bf),
#ifdef VK_USE_PLATFORM_METAL_EXT
          metal_bufferview_export(GetMetalExport(ci)),
#endif
          buf_format_features(buf_ff) {
    }

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
};
