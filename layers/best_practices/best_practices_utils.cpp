/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
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

#include "best_practices/best_practices_validation.h"
#include "best_practices/best_practices_error_enums.h"

struct VendorSpecificInfo {
    EnableFlags vendor_id;
    std::string name;
};

const std::map<BPVendorFlagBits, VendorSpecificInfo> kVendorInfo = {{kBPVendorArm, {vendor_specific_arm, "Arm"}},
                                                                    {kBPVendorAMD, {vendor_specific_amd, "AMD"}},
                                                                    {kBPVendorIMG, {vendor_specific_img, "IMG"}},
                                                                    {kBPVendorNVIDIA, {vendor_specific_nvidia, "NVIDIA"}}};

ReadLockGuard BestPractices::ReadLock() const {
    if (fine_grained_locking) {
        return ReadLockGuard(validation_object_mutex, std::defer_lock);
    } else {
        return ReadLockGuard(validation_object_mutex);
    }
}

WriteLockGuard BestPractices::WriteLock() {
    if (fine_grained_locking) {
        return WriteLockGuard(validation_object_mutex, std::defer_lock);
    } else {
        return WriteLockGuard(validation_object_mutex);
    }
}

std::shared_ptr<vvl::CommandBuffer> BestPractices::CreateCmdBufferState(VkCommandBuffer cb,
                                                                        const VkCommandBufferAllocateInfo* pCreateInfo,
                                                                        const vvl::CommandPool* pool) {
    return std::static_pointer_cast<vvl::CommandBuffer>(std::make_shared<bp_state::CommandBuffer>(this, cb, pCreateInfo, pool));
}

bp_state::CommandBuffer::CommandBuffer(BestPractices* bp, VkCommandBuffer cb, const VkCommandBufferAllocateInfo* pCreateInfo,
                                       const vvl::CommandPool* pool)
    : vvl::CommandBuffer(bp, cb, pCreateInfo, pool) {}

bool BestPractices::VendorCheckEnabled(BPVendorFlags vendors) const {
    for (const auto& vendor : kVendorInfo) {
        if (vendors & vendor.first && enabled[vendor.second.vendor_id]) {
            return true;
        }
    }
    return false;
}

const char* BestPractices::VendorSpecificTag(BPVendorFlags vendors) const {
    // Cache built vendor tags in a map
    static vvl::unordered_map<BPVendorFlags, std::string> tag_map;

    auto res = tag_map.find(vendors);
    if (res == tag_map.end()) {
        // Build the vendor tag string
        std::stringstream vendor_tag;

        vendor_tag << "[";
        bool first_vendor = true;
        for (const auto& vendor : kVendorInfo) {
            if (vendors & vendor.first) {
                if (!first_vendor) {
                    vendor_tag << ", ";
                }
                vendor_tag << vendor.second.name;
                first_vendor = false;
            }
        }
        vendor_tag << "]";

        tag_map[vendors] = vendor_tag.str();
        res = tag_map.find(vendors);
    }

    return res->second.c_str();
}

// Despite the return code being successful this can be a useful utility for some developers in niche debugging situation.
void BestPractices::LogPositiveSuccessCode(const RecordObject& record_obj) const {
    assert(record_obj.result > VK_SUCCESS);

    LogVerbose(kVUID_BestPractices_Verbose_Success_Logging, instance, record_obj.location, "Returned %s.",
               string_VkResult(record_obj.result));
}

void BestPractices::LogErrorCode(const RecordObject& record_obj) const {
    assert(record_obj.result < VK_SUCCESS);  // Anything less than VK_SUCCESS is an error.

    // Despite being error codes log these results as informational.
    // That is because they are returned frequently during window resizing.
    // They are expected to occur during the normal application lifecycle.
    constexpr std::array common_failure_codes = {VK_ERROR_OUT_OF_DATE_KHR, VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT};
    const auto result_string = string_VkResult(record_obj.result);

    if (IsValueIn(record_obj.result, common_failure_codes)) {
        LogInfo(kVUID_BestPractices_Failure_Result, instance, record_obj.location, "Returned error %s.", result_string);
    } else {
        LogWarning(kVUID_BestPractices_Error_Result, instance, record_obj.location, "Returned error %s.", result_string);
    }
}
