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

#include "best_practices_validation.h"

#include "best_practices/my_example_practice.h"

BestPracticesTracker::BestPracticesTracker() {
    container_type = LayerObjectTypeBestPractices;

    // here we add best practices
    auto example_practice = std::unique_ptr<BestPracticeBase>(new MyExampleBestPractices(*this));
    practices.emplace_back(std::move(example_practice));

    // add reverse lookups for vendors which agree with particular checks
    // TODO: maybe reverse lookup is unneeded, we could define vendor agreement within each check class itself?
    for (const auto& practice : practices) {
        for (const auto& vendor : vendor_practices) {
            // if (vendor -> {check-id}) for a particular vendor contains this practice, then the practice "agrees" with the vendor
            if (vendor.second.find(practice->id()) != vendor.second.end()) {
                practice->agree(vendor.first);
            }
        }
    }
}

const std::map<BPVendorFlagBits, VendorSpecificInfo> BestPracticesTracker::initVendorInfo() {
    return {
        // here we define the names and enablements of each vendor
        {kBPVendorKhronos, {&CHECK_ENABLED::vendor_specific_khronos, "Khronos"}},
        {kBPVendorArm, {&CHECK_ENABLED::vendor_specific_arm, "Arm"}},
        {kBPVendorExample1, {&CHECK_ENABLED::vendor_specific_test, "Example1"}},
        {kBPVendorExample2, {&CHECK_ENABLED::vendor_specific_test2, "Example2"}},
    };
}

const std::map<BPVendorFlagBits, std::set<std::string>> BestPracticesTracker::initVendorPractices() {
    return {
        // here we define vendors which agree with particular best practices
        {kBPVendorKhronos, {MyExampleBestPractices::ID}},
        {kBPVendorArm, {MyExampleBestPractices::ID}},
        {kBPVendorExample1, {MyExampleBestPractices::ID}},
        {kBPVendorExample2, {MyExampleBestPractices::ID}},
    };
}

void BestPracticesTracker::ValidateReturnCodes(const char* api_name, VkResult result, const std::vector<VkResult>& error_codes,
                                               const std::vector<VkResult>& success_codes) const {
    auto error = std::find(error_codes.begin(), error_codes.end(), result);
    if (error != error_codes.end()) {
        LogWarning(instance, kVUID_BestPractices_Error_Result, "%s(): Returned error %s.", api_name, string_VkResult(result));
        return;
    }
    auto success = std::find(success_codes.begin(), success_codes.end(), result);
    if (success != success_codes.end()) {
        LogInfo(instance, kVUID_BestPractices_NonSuccess_Result, "%s(): Returned non-success return code %s.", api_name,
                string_VkResult(result));
    }
}
