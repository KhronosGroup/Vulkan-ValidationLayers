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
#include "state_tracker/buffer_state.h"
#include "generated/layer_chassis_dispatch.h"
#include "state_tracker/state_tracker.h"

static VkExternalMemoryHandleTypeFlags GetExternalHandleTypes(const VkBufferCreateInfo *create_info) {
    const auto *external_memory_info = vku::FindStructInPNextChain<VkExternalMemoryBufferCreateInfo>(create_info->pNext);
    return external_memory_info ? external_memory_info->handleTypes : 0;
}

static VkMemoryRequirements GetMemoryRequirements(ValidationStateTracker *dev_data, VkBuffer buffer) {
    VkMemoryRequirements result{};
    DispatchGetBufferMemoryRequirements(dev_data->device, buffer, &result);
    return result;
}

static VkBufferUsageFlags2KHR GetBufferUsageFlags(const VkBufferCreateInfo &create_info) {
    const auto *usage_flags2 = vku::FindStructInPNextChain<VkBufferUsageFlags2CreateInfoKHR>(create_info.pNext);
    return usage_flags2 ? usage_flags2->usage : create_info.usage;
}

BUFFER_STATE::BUFFER_STATE(ValidationStateTracker *dev_data, VkBuffer buff, const VkBufferCreateInfo *pCreateInfo)
    : BINDABLE(buff, kVulkanObjectTypeBuffer, (pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_BINDING_BIT) != 0,
               (pCreateInfo->flags & VK_BUFFER_CREATE_PROTECTED_BIT) == 0, GetExternalHandleTypes(pCreateInfo)),
      safe_create_info(pCreateInfo),
      createInfo(*safe_create_info.ptr()),
      requirements(GetMemoryRequirements(dev_data, buff)),
      usage(GetBufferUsageFlags(createInfo)),
      supported_video_profiles(dev_data->video_profile_cache_.Get(dev_data, vku::FindStructInPNextChain<VkVideoProfileListInfoKHR>(pCreateInfo->pNext))) {
    if (pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_BINDING_BIT) {
        tracker_.emplace<BindableSparseMemoryTracker>(&requirements,
                                                      (pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT) != 0);
        SetMemoryTracker(&std::get<BindableSparseMemoryTracker>(tracker_));
    } else {
        tracker_.emplace<BindableLinearMemoryTracker>(&requirements);
        SetMemoryTracker(&std::get<BindableLinearMemoryTracker>(tracker_));
    }
}

#ifdef VK_USE_PLATFORM_METAL_EXT
static bool GetMetalExport(const VkBufferViewCreateInfo *info) {
    bool retval = false;
    auto export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(info->pNext);
    while (export_metal_object_info) {
        if (export_metal_object_info->exportObjectType == VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT) {
            retval = true;
            break;
        }
        export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(export_metal_object_info->pNext);
    }
    return retval;
}
#endif

BUFFER_VIEW_STATE::BUFFER_VIEW_STATE(const std::shared_ptr<BUFFER_STATE> &bf, VkBufferView bv, const VkBufferViewCreateInfo *ci,
                                     VkFormatFeatureFlags2KHR buf_ff)
    : BASE_NODE(bv, kVulkanObjectTypeBufferView),
      create_info(*ci),
      buffer_state(bf),
#ifdef VK_USE_PLATFORM_METAL_EXT
      metal_bufferview_export(GetMetalExport(ci)),
#endif
      buf_format_features(buf_ff) {
}
