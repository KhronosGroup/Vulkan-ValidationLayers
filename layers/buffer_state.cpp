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
#include "buffer_state.h"
#include "layer_chassis_dispatch.h"
#include "state_tracker.h"

static VkExternalMemoryHandleTypeFlags GetExternalHandleType(const VkBufferCreateInfo *create_info) {
    const auto *external_memory_info = LvlFindInChain<VkExternalMemoryBufferCreateInfo>(create_info->pNext);
    return external_memory_info ? external_memory_info->handleTypes : 0;
}

static VkMemoryRequirements GetMemoryRequirements(ValidationStateTracker *dev_data, VkBuffer buffer) {
    VkMemoryRequirements result{};
    DispatchGetBufferMemoryRequirements(dev_data->device, buffer, &result);
    return result;
}

BUFFER_STATE::BUFFER_STATE(ValidationStateTracker *dev_data, VkBuffer buff, const VkBufferCreateInfo *pCreateInfo)
    : BINDABLE(buff, kVulkanObjectTypeBuffer, (pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_BINDING_BIT) != 0,
               (pCreateInfo->flags & VK_BUFFER_CREATE_PROTECTED_BIT) == 0, GetExternalHandleType(pCreateInfo)),
          safe_create_info(pCreateInfo),
          createInfo(*safe_create_info.ptr()),
          deviceAddress(0),
          requirements(GetMemoryRequirements(dev_data, buff)),
          memory_requirements_checked(false) {}
