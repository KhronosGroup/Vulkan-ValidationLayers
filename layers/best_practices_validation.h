/* Copyright (c) 2015-2020 The Khronos Group Inc.
 * Copyright (c) 2015-2020 Valve Corporation
 * Copyright (c) 2015-2020 LunarG, Inc.
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
 * Author: Camden Stocker <camden@lunarg.com>
 */

#pragma once

// Include code-generated functions
#include "chassis.h"
#include "best_practices_chassis.h"
#include "state_tracker.h"
#include "best_practices_error_enums.h"
#include <stdarg.h>
#include <limits.h>
#include <string>
#include <bitset>

typedef enum {
    // TODO what should the default vendor be called?
    kBPVendorKhronos = 0x00000001,
    kBPVendorArm = 0x00000002,
    kBPVendorExample1 = 0x00000004,
    kBPVendorExample2 = 0x00000008,
} BPVendorFlagBits;
typedef VkFlags BPVendorFlags;

struct VendorSpecificInfo {
    bool CHECK_ENABLED::*check;
    std::string name;
};

class ManualFnAPICallHookInterface {
  public:
    virtual void ManualPostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                                            VkDescriptorSet* pDescriptorSets, VkResult result,
                                                            void* ads_state) const {}
    virtual void ManualPostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                                    VkResult result) const {}
    virtual void ManualPostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                                     VkFence fence, VkResult result) const {}
    virtual void ManualPostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo, VkResult result) const {}
};

// forward declaration of BestPractices for BestPracticeBase
class BestPracticesTracker;

class BestPracticeBase : public virtual BestPracticesAPICallHookInterface, public virtual ManualFnAPICallHookInterface {
  public:
    BestPracticeBase(BestPracticesTracker& tracker) : tracker(tracker) {}

    virtual const std::string id() const = 0;

    void agree(BPVendorFlagBits vendors) { _agreeing_vendors |= vendors; }

    void disagree(BPVendorFlagBits vendors) { _agreeing_vendors &= ~vendors; }

    BPVendorFlags agreeing_vendors() const { return _agreeing_vendors; }

  protected:
    BestPracticesTracker& tracker;

  private:
    BPVendorFlags _agreeing_vendors = 0;
};

class BestPracticesTracker : public ValidationStateTracker {
  public:
    BestPracticesTracker();

    // extra helper functions

    void ValidateReturnCodes(const char* api_name, VkResult result, const std::vector<VkResult>& success_codes,
                             const std::vector<VkResult>& error_codes) const;

    const std::map<BPVendorFlagBits, VendorSpecificInfo> vendor_info = initVendorInfo();
    const std::map<BPVendorFlagBits, std::set<std::string>> vendor_practices = initVendorPractices();

    const std::map<BPVendorFlagBits, VendorSpecificInfo> initVendorInfo();
    const std::map<BPVendorFlagBits, std::set<std::string>> initVendorPractices();

    bool VendorCheckEnabled(BPVendorFlags vendors) const {
        for (const auto& vendor : vendor_info) {
            if (vendors & vendor.first && enabled.*(vendor.second.check)) {
                return true;
            }
        }
        return false;
    }

    const char* VendorSpecificTag(BPVendorFlags vendors) {
        // Cache built vendor tags in a map
        static std::unordered_map<BPVendorFlags, std::string> tag_map;

        auto res = tag_map.find(vendors);
        if (res == tag_map.end()) {
            // Build the vendor tag string
            std::stringstream vendor_tag;

            vendor_tag << "[";
            bool first_vendor = true;
            for (const auto& vendor : vendor_info) {
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

    bool shouldCheckPractice(BPVendorFlags agreement) const {
        for (const auto& vendor : vendor_info) {
            // for each enabled vendor
            // (khronos is always enabled)
            const bool vendor_enabled = enabled.*(vendor.second.check);
            const bool vendor_agrees = vendor.first & agreement;
            if (vendor_enabled && vendor_agrees) {
                return true;
            }
        }
        return false;
    }

    template <typename T>
    T foreachPractice(std::function<T(T, T)> accumulator, std::function<T(BestPracticeBase&)> f) const {
        T acc;
        for (size_t i = 0; i < practices.size(); i++) {
            if (!shouldCheckPractice(practices[i]->agreeing_vendors())) continue;
            if (i <= 0)
                acc = f(*practices[i]);
            else
                acc = accumulator(acc, f(*practices[i]));
        }
        return acc;
    }

    bool foreachPractice(std::function<bool(BestPracticeBase&)> f) const {
        return foreachPractice<bool>([](bool a, bool b) { return a | b; }, f);
    }

    void foreachPractice(std::function<void(BestPracticeBase&)> f) const {
        for (auto& practice : practices) {
            if (shouldCheckPractice(practice->agreeing_vendors())) f(*practice);
        }
    }

  private:
    std::vector<std::unique_ptr<BestPracticeBase>> practices = {};
};
