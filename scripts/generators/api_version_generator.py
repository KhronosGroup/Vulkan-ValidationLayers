#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2023 The Khronos Group Inc.
# Copyright (c) 2015-2023 Valve Corporation
# Copyright (c) 2015-2023 LunarG, Inc.
# Copyright (c) 2015-2023 Google Inc.
# Copyright (c) 2023-2023 RasterGrid Kft.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
from generators.base_generator import BaseGenerator

class ApiVersionOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

    def generate(self):
        out = []
        out.append(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See {os.path.basename(__file__)} for modifications

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
****************************************************************************/\n''')
        out.append('// NOLINTBEGIN') # Wrap for clang-tidy to ignore

        out.append('''
#pragma once
#include <vulkan/vulkan.h>
''')

        out.extend([f'#define {x.name}_NAME "{x.name}"\n' for x in self.vk.versions.values()])

        out.append('''
#define VVL_UNRECOGNIZED_API_VERSION 0xFFFFFFFF

class APIVersion {
  public:
    APIVersion() : api_version_(VVL_UNRECOGNIZED_API_VERSION) {}

    APIVersion(uint32_t api_version) : api_version_(api_version) {}

    APIVersion& operator=(uint32_t api_version) {
        api_version_ = api_version;
        return *this;
    }

    bool valid() const { return api_version_ != VVL_UNRECOGNIZED_API_VERSION; }
    uint32_t value() const { return api_version_; }
    uint32_t major() const { return VK_API_VERSION_MAJOR(api_version_); }
    uint32_t minor() const { return VK_API_VERSION_MINOR(api_version_); }
    uint32_t patch() const { return VK_API_VERSION_PATCH(api_version_); }

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
''')

        out.append('// NOLINTEND') # Wrap for clang-tidy to ignore
        self.write(''.join(out))
