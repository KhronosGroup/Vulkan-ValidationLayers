/* Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (C) 2015-2022 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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
 * Author: Courtney Goeltzenleuchter <courtneygo@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 * Author: Tobias Hector <tobias.hector@amd.com>
 * Author: Jeremy Gebben <jeremyg@lunarg.com>
 */
#pragma once
#include "device_memory_state.h"
#include "range_vector.h"

class ValidationStateTracker;

class BUFFER_STATE : public BINDABLE_NEW {
  public:
    const safe_VkBufferCreateInfo safe_create_info;
    const VkBufferCreateInfo &createInfo;
    VkDeviceAddress deviceAddress;
    const VkMemoryRequirements requirements;
    bool memory_requirements_checked;

    BUFFER_STATE(ValidationStateTracker *dev_data, VkBuffer buff, const VkBufferCreateInfo *pCreateInfo);

    BUFFER_STATE(BUFFER_STATE const &rh_obj) = delete;

    void Destroy() override { BINDABLE_NEW::Destroy(); }

    VkBuffer buffer() const { return handle_.Cast<VkBuffer>(); }

    sparse_container::range<VkDeviceAddress> DeviceAddressRange() const {
        return {deviceAddress, deviceAddress + createInfo.size};
    }

    bool DoesBoundMemoryOverlap(const sparse_container::range<VkDeviceSize> &src_region, const BUFFER_STATE *dst,
                                const sparse_container::range<VkDeviceSize> &dst_region) const;

  protected:
    virtual BindableMemoryTracker::BoundMemoryRange GetBoundMemoryRange(
        const sparse_container::range<VkDeviceSize> &region) const = 0;

    static VkMemoryRequirements GetMemoryRequirements(ValidationStateTracker *dev_data, VkBuffer buffer);
};

template <typename BindableMemoryTrackerType>
class BUFFER_STATE_FINAL final : public BUFFER_STATE {
  public:
    BUFFER_STATE_FINAL(ValidationStateTracker *dev_data, VkBuffer buff, const VkBufferCreateInfo *pCreateInfo)
        : BUFFER_STATE(dev_data, buff, pCreateInfo), bound_memory_tracker_(GetMemoryRequirements(dev_data, buff)) {}

    BUFFER_STATE_FINAL(BUFFER_STATE_FINAL const &rh_obj) = delete;

    virtual ~BUFFER_STATE_FINAL() {
        // Need to unbind children nodes before ~BindableMemoryTrackerType
        if (!Destroyed()) {
            Destroy();
        }
    }

    void Destroy() override {
        auto state = bound_memory_tracker_.GetDeviceMemoryState();
        for (auto &s : state) s->RemoveParent(this);

        BUFFER_STATE::Destroy();
    }

    //---------------------------------------------------------------------------------------------------------
    // Kept for backwards compatibility. We should do a bigger rework on memory with current binding tracking
    const MEM_BINDING *Binding() const override { return bound_memory_tracker_.Binding(); }
    unsigned CountDeviceMemory(VkDeviceMemory mem) const override {
        return bound_memory_tracker_.GetDeviceMemoryHandle() == mem ? 1 : 0;
    }
    //---------------------------------------------------------------------------------------------------------

    // Binds a range of the resource memory [resource_offset, resource_offset + size)
    void BindMemoryRange(std::shared_ptr<DEVICE_MEMORY_STATE> &mem_state, VkDeviceMemory mem, VkDeviceSize mem_offset,
                         VkDeviceSize resource_offset, VkDeviceSize size) override {
        if (bound_memory_tracker_.is_sparse()) {
            // Since we don't know which ranges will be removed, we need to unbind everything and rebind later
            for (auto &mem : bound_memory_tracker_.GetDeviceMemoryState()) {
                mem->RemoveParent(this);
            }
        }
        bound_memory_tracker_.BindMemoryRange(mem_state, mem_offset, resource_offset, size);

        if (bound_memory_tracker_.is_sparse()) {
            for (auto &mem : bound_memory_tracker_.GetDeviceMemoryState()) {
                mem->AddParent(this);
            }
        } else {
            mem_state->AddParent(this);
        }
    }

    BindableMemoryTracker::BoundMemoryRange GetBoundMemoryRange(
        const sparse_container::range<VkDeviceSize> &region) const override {
        return bound_memory_tracker_.GetBoundMemoryRange(region);
    }

    bool FullRangeBound() const override { return bound_memory_tracker_.FullRangeBound(); }

  private:
    BindableMemoryTrackerType bound_memory_tracker_;
};

class BUFFER_VIEW_STATE : public BASE_NODE {
  public:
    const VkBufferViewCreateInfo create_info;
    std::shared_ptr<BUFFER_STATE> buffer_state;
    // Format features that matter when accessing the buffer (OpLoad, OpStore,
    // OpAtomicLoad, etc...)
    const VkFormatFeatureFlags2KHR buf_format_features;
    // Format features that matter when accessing the buffer as a image
    // (OpImageRead, OpImageWrite, etc...)
    const VkFormatFeatureFlags2KHR img_format_features;

    BUFFER_VIEW_STATE(const std::shared_ptr<BUFFER_STATE> &bf, VkBufferView bv, const VkBufferViewCreateInfo *ci,
                      VkFormatFeatureFlags2KHR buf_ff, VkFormatFeatureFlags2KHR img_ff)
        : BASE_NODE(bv, kVulkanObjectTypeBufferView),
          create_info(*ci),
          buffer_state(bf),
          buf_format_features(buf_ff),
          img_format_features(img_ff) {}

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
