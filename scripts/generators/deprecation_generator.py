#!/usr/bin/python3 -i
#
# Copyright (c) 2025 The Khronos Group Inc.
# Copyright (c) 2025 Valve Corporation
# Copyright (c) 2025 LunarG, Inc.
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

class DeprecationGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

        self.all_device_extensions = set()
        self.all_instance_extensions = set()

        # Try and provide a mapping of the "new" function to replace
        # (This should really be in the vk.xml)
        self.replacement = {
            "vkGetPhysicalDeviceFeatures" : {
                "version" : "vkGetPhysicalDeviceFeatures2",
                "extension" : "vkGetPhysicalDeviceFeatures2KHR",
            },
            "vkGetPhysicalDeviceFormatProperties" : {
                "version" : "vkGetPhysicalDeviceFormatProperties2",
                "extension" : "vkGetPhysicalDeviceFormatProperties2KHR",
            },
            "vkGetPhysicalDeviceImageFormatProperties" : {
                "version" : "vkGetPhysicalDeviceImageFormatProperties2",
                "extension" : "vkGetPhysicalDeviceImageFormatProperties2KHR",
            },
            "vkGetPhysicalDeviceProperties" : {
                "version" : "vkGetPhysicalDeviceProperties2",
                "extension" : "vkGetPhysicalDeviceProperties2KHR",
            },
            "vkGetPhysicalDeviceQueueFamilyProperties" : {
                "version" : "vkGetPhysicalDeviceQueueFamilyProperties2",
                "extension" : "vkGetPhysicalDeviceQueueFamilyProperties2KHR",
            },
            "vkGetPhysicalDeviceMemoryProperties" : {
                "version" : "vkGetPhysicalDeviceMemoryProperties2",
                "extension" : "vkGetPhysicalDeviceMemoryProperties2KHR",
            },
            "vkGetPhysicalDeviceSparseImageFormatProperties" : {
                "version" : "vkGetPhysicalDeviceSparseImageFormatProperties2",
                "extension" : "vkGetPhysicalDeviceSparseImageFormatProperties2KHR",
            },
            "vkCreateRenderPass" : {
                "version" : "vkCreateRenderPass2",
                "extension" : "vkCreateRenderPass2KHR",
            },
            "vkCmdBeginRenderPass" : {
                "version" : "vkCmdBeginRenderPass2",
                "extension" : "vkCmdBeginRenderPass2KHR",
            },
            "vkCmdNextSubpass" : {
                "version" : "vkCmdNextSubpass2",
                "extension" : "vkCmdNextSubpass2KHR",
            },
            "vkCmdEndRenderPass" : {
                "version" : "vkCmdEndRenderPass2",
                "extension" : "vkCmdEndRenderPass2KHR",
            },
        }

    def generate(self):
        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

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
            ****************************************************************************/\n''')
        self.write('// NOLINTBEGIN') # Wrap for clang-tidy to ignore

        for extensions in [x.legacy.extensions for x in self.vk.commands.values() if x.legacy and x.legacy.extensions and x.instance]:
            self.all_instance_extensions.update(extensions)
        for extensions in [x.legacy.extensions for x in self.vk.commands.values() if x.legacy and x.legacy.extensions and x.device]:
            self.all_device_extensions.update(extensions)

        if self.filename == 'deprecation.h':
            self.generateHeader()
        elif self.filename == 'deprecation.cpp':
            self.generateSource()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateHeader(self):
        out = []
        out.append('''
            #pragma once

            #include <vulkan/vulkan.h>
            #include "chassis/validation_object.h"

            namespace deprecation {

            // We currently only check if the extension is enabled, if we decide in the future to check for support, instance extensions
            // we can try and use DispatchEnumerateInstanceExtensionProperties, but will likely run into many loader related issues.
            class Instance : public vvl::base::Instance {
                using BaseClass = vvl::base::Instance;

            public:
                Instance(vvl::dispatch::Instance *dispatch) : BaseClass(dispatch, LayerObjectTypeDeprecation) {}

        ''')

        for command in [x for x in self.vk.commands.values() if x.legacy and x.instance]:
            prototype = (command.cPrototype.split('VKAPI_CALL ')[1])[2:-1]
            prePrototype = prototype.replace(')', ', const ErrorObject& error_obj)')
            out.append(f'bool PreCallValidate{prePrototype} const override;\n')

        out.append('''
            };

            class Device : public vvl::base::Device {
                using BaseClass = vvl::base::Device;

            public:
                Device(vvl::dispatch::Device *dev, Instance *instance_vo)
                    : BaseClass(dev, instance_vo, LayerObjectTypeDeprecation), instance(instance_vo) {}
                ~Device() {}
                Instance *instance;

        ''')

        out.append('\n')

        for command in [x for x in self.vk.commands.values() if x.legacy and x.device]:
            # There is really no good use to warn developer both the create and destroy are deprecated
            if command.name.startswith('vkDestroy'):
                continue

            prototype = (command.cPrototype.split('VKAPI_CALL ')[1])[2:-1]
            prePrototype = prototype.replace(')', ', const ErrorObject& error_obj)')
            out.append(f'bool PreCallValidate{prePrototype} const override;\n')

        out.append('};')
        out.append('}  // namespace deprecation')
        self.write(''.join(out))

    def generateSource(self):
        out = []
        out.append('''
            #include "deprecation.h"

            namespace deprecation {
        ''')

        for command in [x for x in self.vk.commands.values() if x.legacy]:
            # There is really no good use to warn developer both the create and destroy are deprecated
            if command.name.startswith('vkDestroy'):
                continue

            className = 'Device' if command.device else 'Instance'
            handleName = 'VkDevice' if command.device else 'VkInstance'
            objName = 'device' if command.device else 'physicalDevice'
            replacement = "which contains the new feature to replace it"

            prototype = (command.cPrototype.split('VKAPI_CALL ')[1])[2:-1]
            prePrototype = prototype.replace(')', ', const ErrorObject& error_obj)')
            out.append(f'''
                bool {className}::PreCallValidate{prePrototype} const {{
                    static bool reported = false;
                    if (reported) return false;
                ''')

            firstCheck = True
            if command.legacy.version:
                logic = 'if' if firstCheck else 'else if'
                if firstCheck:
                    firstCheck = False

                if command.name in self.replacement:
                    replacement = f'which contains {self.replacement[command.name]["version"]} that can be used instead'

                out.append(f'''
                    {logic} (api_version >= {command.legacy.version.nameApi}) {{
                        reported = true;
                        LogWarning("WARNING-{command.legacy.link}", {objName}, error_obj.location,
                            "{command.name} is deprecated and this {handleName} was created with {command.legacy.version.name} {replacement}.\\nSee more information about this deprecation in the specification: https://docs.vulkan.org/spec/latest/appendices/legacy.html#{command.legacy.link}");
                    }}''')

            for extension in command.legacy.extensions:
                logic = 'if' if firstCheck else 'else if'
                if firstCheck:
                    firstCheck = False

                if command.name in self.replacement:
                    replacement = f'which contains {self.replacement[command.name]["extension"]} that can be used instead'

                out.append(f'''
                    {logic} (IsExtEnabled(extensions.{extension.lower()})) {{
                        reported = true;
                        LogWarning("WARNING-{command.legacy.link}", {objName}, error_obj.location,
                            "{command.name} is deprecated and this {handleName} enabled the {extension} extension {replacement}.\\nSee more information about this deprecation in the specification: https://docs.vulkan.org/spec/latest/appendices/legacy.html#{command.legacy.link}");
                    }}''')

            # For things deprecated in Vulkan 1.0
            if command.legacy.version is None and len(command.legacy.extensions) == 0:
                out.append(f'''
                           LogWarning("WARNING-{command.legacy.link}", {objName}, error_obj.location,
                            "{command.name} is deprecated.\\nSee more information about this deprecation in the specification: https://docs.vulkan.org/spec/latest/appendices/legacy.html#{command.legacy.link}");
                    ''')

            out.append('''
                return false;
            }
            ''')

        out.append('}  // namespace deprecation')
        self.write(''.join(out))
