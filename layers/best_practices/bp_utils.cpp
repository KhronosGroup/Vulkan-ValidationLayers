/* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
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

#include "best_practices/bp_utils.h"

struct VendorSpecificInfo {
    EnableFlags vendor_id;
    std::string name;
};

static const auto& GetVendorInfo() {
    static const std::map<BPVendorFlagBits, VendorSpecificInfo> kVendorInfo = {
        {kBPVendorArm, {vendor_specific_arm, "Arm"}},
        {kBPVendorAMD, {vendor_specific_amd, "AMD"}},
        {kBPVendorIMG, {vendor_specific_img, "IMG"}},
        {kBPVendorNVIDIA, {vendor_specific_nvidia, "NVIDIA"}}};

    return kVendorInfo;
}

bool IsVendorCheckEnabled(const ValidationEnabled& enabled, BPVendorFlags vendors) {
    for (const auto& vendor : GetVendorInfo()) {
        if (vendors & vendor.first && enabled[vendor.second.vendor_id]) {
            return true;
        }
    }
    return false;
}

const char* VendorSpecificTag(BPVendorFlagBits vendor) {
    switch (vendor) {
        case kBPVendorArm:
            return "[Arm]";
        case kBPVendorAMD:
            return "[AMD]";
        case kBPVendorIMG:
            return "[IMG]";
        case kBPVendorNVIDIA:
            return "[NVIDIA]";
    }
    assert(false);
    return "[Unknown vendor]";
}

std::string VendorSpecificTag(BPVendorFlags vendors) {
    std::string tag = "[";
    bool first_vendor = true;
    for (const auto& vendor : GetVendorInfo()) {
        if (vendors & vendor.first) {
            if (!first_vendor) {
                tag += ", ";
            }
            tag += vendor.second.name;
            first_vendor = false;
        }
    }
    tag += "]";
    return tag;
}
