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

        for extensions in [x.deprecate.extensions for x in self.vk.commands.values() if x.deprecate and x.deprecate.extensions and x.instance]:
            self.all_instance_extensions.update(extensions)
        for extensions in [x.deprecate.extensions for x in self.vk.commands.values() if x.deprecate and x.deprecate.extensions and x.device]:
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

            class Instance : public vvl::base::Instance {
                using BaseClass = vvl::base::Instance;

            public:
                Instance(vvl::dispatch::Instance *dispatch) : BaseClass(dispatch, LayerObjectTypeDeprecation) {}

                void PostCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                                  VkInstance* pInstance, const RecordObject& record_obj) override;

        ''')

        for extension in sorted(self.all_instance_extensions):
            out.append(f'bool supported_{extension.lower()} = false;')
        out.append('\n')

        for command in [x for x in self.vk.commands.values() if x.deprecate and x.instance]:
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

                void FinishDeviceSetup(const VkDeviceCreateInfo *pCreateInfo, const Location &loc) override;
        ''')

        for extension in sorted(self.all_device_extensions):
            out.append(f'bool supported_{extension.lower()} = false;')
        out.append('\n')

        for command in [x for x in self.vk.commands.values() if x.deprecate and x.device]:
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
            #include "generated/dispatch_functions.h"

            namespace deprecation {

            void Instance::PostCallRecordCreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                                            VkInstance *pInstance, const RecordObject &record_obj) {
                if (record_obj.result != VK_SUCCESS) {
                    return;
                }

                // Tried to use DispatchEnumerateInstanceExtensionProperties but ran into many loader related issues
                // For now, just check if they have enabled the extension (instead of if it supported)

        ''')
        for extension in sorted(self.all_instance_extensions):
            out.append(f'''
                if (IsExtEnabled(extensions.{extension.lower()})) {{
                    supported_{extension.lower()} = true;
                }}
            ''')
        out.append('''
            }
            ''')

        out.append('''

            void Device::FinishDeviceSetup(const VkDeviceCreateInfo* pCreateInfo, const Location& loc) {
                std::vector<VkExtensionProperties> ext_props{};
                uint32_t ext_count = 0;
                DispatchEnumerateDeviceExtensionProperties(physical_device, nullptr, &ext_count, nullptr);
                ext_props.resize(ext_count);
                DispatchEnumerateDeviceExtensionProperties(physical_device, nullptr, &ext_count, ext_props.data());
                for (const auto& prop : ext_props) {
                    vvl::Extension extension = GetExtension(prop.extensionName);
        ''')
        for extension in sorted(self.all_device_extensions):
            out.append(f'''
                if (extension == vvl::Extension::_{extension}) {{
                    supported_{extension.lower()} = true;
                }}
            ''')

        out.append('''
                }
            }
            ''')

        for command in [x for x in self.vk.commands.values() if x.deprecate]:
            # There is really no good use to warn developer both the create and destroy are deprecated
            if command.name.startswith('vkDestroy'):
                continue

            className = 'Device' if command.device else 'Instance'
            objName = 'device' if command.device else 'physicalDevice'

            prototype = (command.cPrototype.split('VKAPI_CALL ')[1])[2:-1]
            prePrototype = prototype.replace(')', ', const ErrorObject& error_obj)')
            out.append(f'''
                bool {className}::PreCallValidate{prePrototype} const {{
                    static bool reported = false;
                    if (reported) return false;
                ''')

            firstCheck = True
            if command.deprecate.version:
                logic = 'if' if firstCheck else 'else if'
                if firstCheck:
                    firstCheck = False

                out.append(f'''
                    {logic} (api_version >= {command.deprecate.version.nameApi}) {{
                        reported = true;
                        LogWarning("WARNING-{command.deprecate.link}", {objName}, error_obj.location,
                            "{command.name} is deprecated and this {objName} supports {command.deprecate.version.name}\\nSee more information about this deprecation in the specification: https://docs.vulkan.org/spec/latest/appendices/deprecation.html#{command.deprecate.link}");
                    }}''')

            for extension in command.deprecate.extensions:
                logic = 'if' if firstCheck else 'else if'
                if firstCheck:
                    firstCheck = False

                out.append(f'''
                    {logic} (supported_{extension.lower()}) {{
                        reported = true;
                        LogWarning("WARNING-{command.deprecate.link}", {objName}, error_obj.location,
                            "{command.name} is deprecated and this {objName} supports {extension}\\nSee more information about this deprecation in the specification: https://docs.vulkan.org/spec/latest/appendices/deprecation.html#{command.deprecate.link}");
                    }}''')

            out.append('''
                return false;
            }
            ''')

        out.append('}  // namespace deprecation')
        self.write(''.join(out))
