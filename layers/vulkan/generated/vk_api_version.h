// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See api_version_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google Inc.
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
// NOLINTBEGIN
#pragma once
#include <vulkan/vulkan.h>
#include <sstream>
#include <iomanip>

#define VK_VERSION_1_1_NAME "VK_VERSION_1_1"
#define VK_VERSION_1_2_NAME "VK_VERSION_1_2"
#define VK_VERSION_1_3_NAME "VK_VERSION_1_3"

#define VVL_UNRECOGNIZED_API_VERSION 0xFFFFFFFF

class APIVersion {
  public:
    APIVersion() : api_version_(VVL_UNRECOGNIZED_API_VERSION) {}
    APIVersion(uint32_t api_version) : api_version_(api_version) {}
    APIVersion& operator=(uint32_t api_version) {
        api_version_ = api_version;
        return *this;
    }
    bool Valid() const { return api_version_ != VVL_UNRECOGNIZED_API_VERSION; }
    uint32_t Value() const { return api_version_; }
    uint32_t Major() const { return VK_API_VERSION_MAJOR(api_version_); }
    uint32_t Minor() const { return VK_API_VERSION_MINOR(api_version_); }
    uint32_t Patch() const { return VK_API_VERSION_PATCH(api_version_); }
    bool operator<(APIVersion api_version) const { return api_version_ < api_version.api_version_; }
    bool operator<=(APIVersion api_version) const { return api_version_ <= api_version.api_version_; }
    bool operator>(APIVersion api_version) const { return api_version_ > api_version.api_version_; }
    bool operator>=(APIVersion api_version) const { return api_version_ >= api_version.api_version_; }
    bool operator==(APIVersion api_version) const { return api_version_ == api_version.api_version_; }
    bool operator!=(APIVersion api_version) const { return api_version_ != api_version.api_version_; }

  private:
    uint32_t api_version_;
};

static inline APIVersion NormalizeApiVersion(APIVersion specified_version) {
    if (specified_version < VK_API_VERSION_1_1)
        return VK_API_VERSION_1_0;
    else if (specified_version < VK_API_VERSION_1_2)
        return VK_API_VERSION_1_1;
    else if (specified_version < VK_API_VERSION_1_3)
        return VK_API_VERSION_1_2;
    else
        return VK_API_VERSION_1_3;
}

// Convert integer API version to a string
static inline std::string StringAPIVersion(APIVersion version) {
    std::stringstream version_name;
    if (!version.Valid()) {
        return "<unrecognized>";
    }
    version_name << version.Major() << "." << version.Minor() << "." << version.Patch() << " (0x" << std::setfill('0')
                 << std::setw(8) << std::hex << version.Value() << ")";
    return version_name.str();
}
// NOLINTEND
