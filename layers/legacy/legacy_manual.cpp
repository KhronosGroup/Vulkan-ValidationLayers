/***************************************************************************
 *
 * Copyright (c) 2025 The Khronos Group Inc.
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
 ****************************************************************************/

#include "generated/dispatch_functions.h"
#include "generated/legacy.h"

namespace legacy {

static const char* kLegacyExtensionVUID = "WARNING-legacy-extension";

bool Instance::ValidateLegacyExtensions(const Location& loc, vvl::Extension extension, APIVersion version) const {
    bool skip = false;
    const auto dep_info = GetExtensionData(extension);
    if (dep_info.reason != Reason::Empty) {
        auto reason_to_string = [](Reason reason) {
            switch (reason) {
                case Reason::Promoted:
                    return "promoted to";
                case Reason::Obsoleted:
                    return "obsoleted by";
                case Reason::Superseded:
                    return "superseded by";
                default:
                    return "";
            }
        };

        if ((dep_info.target.version == vvl::Version::_VK_VERSION_1_1 && (version >= VK_API_VERSION_1_1)) ||
            (dep_info.target.version == vvl::Version::_VK_VERSION_1_2 && (version >= VK_API_VERSION_1_2)) ||
            (dep_info.target.version == vvl::Version::_VK_VERSION_1_3 && (version >= VK_API_VERSION_1_3)) ||
            (dep_info.target.version == vvl::Version::_VK_VERSION_1_4 && (version >= VK_API_VERSION_1_4))) {
            skip |= LogWarning(kLegacyExtensionVUID, instance, loc,
                               "Attempting to enable legacy extension %s, but this extension has been %s %s.", String(extension),
                               reason_to_string(dep_info.reason), String(dep_info.target).c_str());
        } else if (dep_info.target.version == vvl::Version::Empty) {
            if (dep_info.target.extension == vvl::Extension::Empty) {
                skip |= LogWarning(kLegacyExtensionVUID, instance, loc,
                                   "Attempting to enable legacy extension %s, but this extension has been deprecated "
                                   "without replacement.",
                                   String(extension));
            } else {
                skip |= LogWarning(kLegacyExtensionVUID, instance, loc,
                                   "Attempting to enable legacy extension %s, but this extension has been %s %s.",
                                   String(extension), reason_to_string(dep_info.reason), String(dep_info.target).c_str());
            }
        }
    }
    return skip;
}

bool Instance::PreCallValidateCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                             VkInstance* pInstance, const ErrorObject& error_obj) const {
    bool skip = false;

    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        vvl::Extension extension = GetExtension(pCreateInfo->ppEnabledExtensionNames[i]);
        const uint32_t specified_version =
            pCreateInfo->pApplicationInfo ? pCreateInfo->pApplicationInfo->apiVersion : VK_API_VERSION_1_0;
        skip |= ValidateLegacyExtensions(error_obj.location, extension, specified_version);
    }

    return skip;
}

bool Instance::PreCallValidateCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                           const ErrorObject& error_obj) const {
    bool skip = false;

    VkPhysicalDeviceProperties physical_device_properties{};
    DispatchGetPhysicalDeviceProperties(physicalDevice, &physical_device_properties);
    uint32_t device_api_version = physical_device_properties.apiVersion;

    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        const char* extension_name = pCreateInfo->ppEnabledExtensionNames[i];

        APIVersion extension_api_version = std::min(api_version, APIVersion(device_api_version));

        vvl::Extension extension = GetExtension(extension_name);
        if (IsInstanceExtension(extension)) {
            extension_api_version = api_version;
        }

        skip |= ValidateLegacyExtensions(error_obj.location, extension, extension_api_version);
    }

    return skip;
}

}  // namespace legacy