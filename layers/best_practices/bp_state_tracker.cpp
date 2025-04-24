/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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

#include "best_practices/best_practices_validation.h"
#include "best_practices/bp_state.h"

void BestPractices::RecordGetImageMemoryRequirementsState(vvl::Image& image_state, const VkImageMemoryRequirementsInfo2* pInfo) {
    auto& sub_state = bp_state::SubState(image_state);
    const VkImagePlaneMemoryRequirementsInfo* plane_info =
        (pInfo) ? vku::FindStructInPNextChain<VkImagePlaneMemoryRequirementsInfo>(pInfo->pNext) : nullptr;

    if (plane_info != nullptr) {
        // Multi-plane image
        if (plane_info->planeAspect == VK_IMAGE_ASPECT_PLANE_0_BIT) {
            sub_state.memory_requirements_checked[0] = true;
        } else if (plane_info->planeAspect == VK_IMAGE_ASPECT_PLANE_1_BIT) {
            sub_state.memory_requirements_checked[1] = true;
        } else if (plane_info->planeAspect == VK_IMAGE_ASPECT_PLANE_2_BIT) {
            sub_state.memory_requirements_checked[2] = true;
        }
    } else if (!image_state.disjoint) {
        // Single Plane image
        sub_state.memory_requirements_checked[0] = true;
    }
}

void BestPractices::PostCallRecordGetImageMemoryRequirements(VkDevice device, VkImage image,
                                                             VkMemoryRequirements* pMemoryRequirements,
                                                             const RecordObject& record_obj) {
    if (auto image_state = Get<vvl::Image>(image)) {
        RecordGetImageMemoryRequirementsState(*image_state, nullptr);
    }
}

void BestPractices::PostCallRecordGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                              VkMemoryRequirements2* pMemoryRequirements,
                                                              const RecordObject& record_obj) {
    if (auto image_state = Get<vvl::Image>(pInfo->image)) {
        RecordGetImageMemoryRequirementsState(*image_state, pInfo);
    }
}

void BestPractices::PostCallRecordGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                                 VkMemoryRequirements2* pMemoryRequirements,
                                                                 const RecordObject& record_obj) {
    PostCallRecordGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements, record_obj);
}

void BestPractices::PostCallRecordGetImageSparseMemoryRequirements(VkDevice device, VkImage image,
                                                                   uint32_t* pSparseMemoryRequirementCount,
                                                                   VkSparseImageMemoryRequirements* pSparseMemoryRequirements,
                                                                   const RecordObject& record_obj) {
    if (auto image_state = Get<vvl::Image>(image)) {
        auto& sub_state = bp_state::SubState(*image_state);
        sub_state.get_sparse_reqs_called = true;
    }
}

void BestPractices::PostCallRecordGetImageSparseMemoryRequirements2(VkDevice device,
                                                                    const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                                    uint32_t* pSparseMemoryRequirementCount,
                                                                    VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                                    const RecordObject& record_obj) {
    if (auto image_state = Get<vvl::Image>(pInfo->image)) {
        auto& sub_state = bp_state::SubState(*image_state);
        sub_state.get_sparse_reqs_called = true;
    }
}

void BestPractices::PostCallRecordGetImageSparseMemoryRequirements2KHR(VkDevice device,
                                                                       const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                                       uint32_t* pSparseMemoryRequirementCount,
                                                                       VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                                       const RecordObject& record_obj) {
    PostCallRecordGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements,
                                                    record_obj);
}
