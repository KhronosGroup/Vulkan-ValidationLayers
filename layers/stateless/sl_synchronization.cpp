/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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

#include "stateless/stateless_validation.h"
#include "generated/enum_flag_bits.h"

namespace stateless {
bool Device::manual_PreCallValidateCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo,
                                                   const VkAllocationCallbacks *pAllocator, VkSemaphore *pSemaphore,
                                                   const Context &context) const {
    bool skip = false;
#ifdef VK_USE_PLATFORM_METAL_EXT
    skip |= ExportMetalObjectsPNextUtil(VK_EXPORT_METAL_OBJECT_TYPE_METAL_SHARED_EVENT_BIT_EXT,
                                        "VUID-VkSemaphoreCreateInfo-pNext-06789", context.error_obj.location,
                                        "VK_EXPORT_METAL_OBJECT_TYPE_METAL_SHARED_EVENT_BIT_EXT", pCreateInfo->pNext);
#endif  // VK_USE_PLATFORM_METAL_EXT
    return skip;
}
bool Device::manual_PreCallValidateCreateEvent(VkDevice device, const VkEventCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkEvent *pEvent,
                                               const Context &context) const {
    bool skip = false;
#ifdef VK_USE_PLATFORM_METAL_EXT
    skip |= ExportMetalObjectsPNextUtil(VK_EXPORT_METAL_OBJECT_TYPE_METAL_SHARED_EVENT_BIT_EXT,
                                        "VUID-VkEventCreateInfo-pNext-06790", context.error_obj.location,
                                        "VK_EXPORT_METAL_OBJECT_TYPE_METAL_SHARED_EVENT_BIT_EXT", pCreateInfo->pNext);
#endif  // VK_USE_PLATFORM_METAL_EXT
    return skip;
}

bool Device::ValidateDependencyInfo(const Context &context, const VkDependencyInfo &dep_info, const Location &loc) const {
    bool skip = false;

    constexpr std::array allowed_structs = {VK_STRUCTURE_TYPE_MEMORY_BARRIER_ACCESS_FLAGS_3_KHR};

    for (uint32_t i = 0; i < dep_info.memoryBarrierCount; ++i) {
        const Location barrier_loc = loc.dot(Struct::VkMemoryBarrier2, Field::pMemoryBarriers, i);
        const VkMemoryBarrier2 &memory_barrier = dep_info.pMemoryBarriers[i];
        skip |= context.ValidateStructPnext(barrier_loc, memory_barrier.pNext, allowed_structs.size(),
                                            allowed_structs.data(), GeneratedVulkanHeaderVersion,
                                            "VUID-VkDependencyInfo-pMemoryBarriers-10606",
                                            "VUID-VkDependencyInfo-pMemoryBarriers-10605");
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo,
                                                       const Context &context) const {
    bool skip = false;

    skip |= ValidateDependencyInfo(context, *pDependencyInfo, context.error_obj.location.dot(Field::pDependencyInfo));

    return skip;
}

bool Device::manual_PreCallValidateCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                                const VkDependencyInfo *pDependencyInfo, const Context &context) const {
    bool skip = false;

    skip |= ValidateDependencyInfo(context, *pDependencyInfo, context.error_obj.location.dot(Field::pDependencyInfo));

    return skip;
}

bool Device::manual_PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                  const VkDependencyInfo *pDependencyInfos, const Context &context) const {
    bool skip = false;

    for (uint32_t i = 0; i < eventCount; ++i) {
        skip |= ValidateDependencyInfo(context, pDependencyInfos[i], context.error_obj.location.dot(Field::pDependencyInfos, i));
    }

    return skip;
}

}  // namespace stateless
