#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2025 The Khronos Group Inc.
# Copyright (c) 2015-2025 Valve Corporation
# Copyright (c) 2015-2025 LunarG, Inc.
# Copyright (c) 2015-2025 Google Inc.
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
from base_generator import BaseGenerator
from generators.generator_utils import PlatformGuardHelper

class FeatureNotPresentGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

    def generate(self):
        out = []
        out.append(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
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
            ****************************************************************************/\n\n''')
        out.append(self.generateSource())
        self.write("".join(out))

    def generateSource(self):
        out = []
        guard_helper = PlatformGuardHelper()
        out.append('''
            #include "chassis/dispatch_object.h"
            #include "generated/dispatch_functions.h"
            #include "error_message/error_location.h"

            namespace vvl {
            namespace dispatch {

            void Instance::ReportErrorFeatureNotPresent(VkPhysicalDevice gpu, const VkDeviceCreateInfo &create_info) {
                std::stringstream ss;
                ss << "returned VK_ERROR_FEATURE_NOT_PRESENT because the following features were not supported on this physical device:\\n";

                // First do 1.0 VkPhysicalDeviceFeatures
                {
                    const auto *features2 = vku::FindStructInPNextChain<VkPhysicalDeviceFeatures2>(create_info.pNext);
                    const VkPhysicalDeviceFeatures &enabling = create_info.pEnabledFeatures ? *create_info.pEnabledFeatures : features2->features;

                    VkPhysicalDeviceFeatures supported = {};
                    DispatchGetPhysicalDeviceFeatures(gpu, &supported);
        ''')

        for member in self.vk.structs['VkPhysicalDeviceFeatures'].members:
            out.append(f'''if (enabling.{member.name} && !supported.{member.name}) {{
                               ss << "VkPhysicalDeviceFeatures::{member.name} is not supported\\n";
                           }}
                        ''')
        out.append('}')
        out.append('''
                    VkPhysicalDeviceFeatures2 features_2 = vku::InitStructHelper();
                    for (const VkBaseInStructure* current = static_cast<const VkBaseInStructure*>(create_info.pNext); current != nullptr; current = current->pNext) {
                        switch(current->sType) {
                    ''')

        feature_structs = self.vk.structs['VkPhysicalDeviceFeatures2'].extendedBy
        for extending_struct_name in feature_structs:
            extending_struct = self.vk.structs[extending_struct_name]
            out.extend(guard_helper.add_guard(extending_struct.protect))
            out.append(f'''case {extending_struct.sType}: {{
                            {extending_struct_name} supported = vku::InitStructHelper();
                            features_2.pNext = &supported;
                            DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                            const {extending_struct_name} *enabling = reinterpret_cast<const {extending_struct_name} *>(current);
                       ''')

            for member in [x for x in extending_struct.members if x.type == 'VkBool32']:
                out.append(f'''if (enabling->{member.name} && !supported.{member.name}) {{
                                ss << "{extending_struct_name}::{member.name} is not supported\\n";
                            }}
                            ''')
            out.append('break;\n}\n')
            out.extend(guard_helper.add_guard(None))
        out.append('''
                        default:
                            break;
                        }
                    }

                    Location loc(vvl::Func::vkCreateDevice);
                    LogWarning("WARNING-vkCreateDevice-FeatureNotPresent", instance, loc, "%s", ss.str().c_str());
                }  // ReportErrorFeatureNotPresent
                }  // namespace dispatch
                }  // namespace vvl
                ''')
        return "".join(out)
