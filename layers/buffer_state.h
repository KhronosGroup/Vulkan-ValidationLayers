/* Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (C) 2015-2021 Google Inc.
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

class BUFFER_STATE : public BINDABLE {
  public:
    VkBufferCreateInfo createInfo;
    VkDeviceAddress deviceAddress;
    VkMemoryRequirements requirements;
    bool memory_requirements_checked;

    BUFFER_STATE(VkBuffer buff, const VkBufferCreateInfo *pCreateInfo)
        : BINDABLE(buff, kVulkanObjectTypeBuffer, (pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_BINDING_BIT) != 0,
                   (pCreateInfo->flags & VK_BUFFER_CREATE_PROTECTED_BIT) == 0, GetExternalHandleType(pCreateInfo)),
          createInfo(*pCreateInfo),
          deviceAddress(0),
          requirements(),
          memory_requirements_checked(false) {
        if ((createInfo.sharingMode == VK_SHARING_MODE_CONCURRENT) && (createInfo.queueFamilyIndexCount > 0)) {
            uint32_t *pQueueFamilyIndices = new uint32_t[createInfo.queueFamilyIndexCount];
            for (uint32_t i = 0; i < createInfo.queueFamilyIndexCount; i++) {
                pQueueFamilyIndices[i] = pCreateInfo->pQueueFamilyIndices[i];
            }
            createInfo.pQueueFamilyIndices = pQueueFamilyIndices;
        }
    }

    BUFFER_STATE(BUFFER_STATE const &rh_obj) = delete;

    VkBuffer buffer() const { return handle_.Cast<VkBuffer>(); }

    ~BUFFER_STATE() {
        if ((createInfo.sharingMode == VK_SHARING_MODE_CONCURRENT) && (createInfo.queueFamilyIndexCount > 0)) {
            delete[] createInfo.pQueueFamilyIndices;
            createInfo.pQueueFamilyIndices = nullptr;
        }
        Destroy();
    };

  private:
    static inline VkExternalMemoryHandleTypeFlags GetExternalHandleType(const VkBufferCreateInfo *pCreateInfo) {
        auto *external_memory_info = LvlFindInChain<VkExternalMemoryBufferCreateInfo>(pCreateInfo->pNext);
        return external_memory_info ? external_memory_info->handleTypes : 0;
    }
};

class BUFFER_VIEW_STATE : public BASE_NODE {
  public:
    VkBufferViewCreateInfo create_info;
    std::shared_ptr<BUFFER_STATE> buffer_state;
    VkFormatFeatureFlags format_features;
    BUFFER_VIEW_STATE(const std::shared_ptr<BUFFER_STATE> &bf, VkBufferView bv, const VkBufferViewCreateInfo *ci)
        : BASE_NODE(bv, kVulkanObjectTypeBufferView), create_info(*ci), buffer_state(bf) {
        if (buffer_state) {
            buffer_state->AddParent(this);
        }
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
        }
        BASE_NODE::Destroy();
    }
};
