#!/usr/bin/python3 -i
#
# Copyright (c) 2025-2026 The Khronos Group Inc.
# Copyright (c) 2025-2026 Valve Corporation
# Copyright (c) 2025-2026 LunarG, Inc.
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

class LegacyGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

        self.all_device_extensions = set()
        self.all_instance_extensions = set()

    def generate(self):
        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2025-2026 The Khronos Group Inc.
            * Copyright (c) 2025-2026 Valve Corporation
            * Copyright (c) 2025-2026 LunarG, Inc.
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

        if self.filename == 'legacy.h':
            self.generateHeader()
        elif self.filename == 'legacy.cpp':
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

            namespace legacy {

            enum class Reason {
                Empty = 0,
                Promoted,
                Obsoleted,
                Superseded,
            };

            struct ExtensionData {
                Reason reason;
                vvl::Requirement target;
            };

            ExtensionData GetExtensionData(vvl::Extension extension);

            // We currently only check if the extension is enabled, if we decide in the future to check for support, instance extensions
            // we can try and use DispatchEnumerateInstanceExtensionProperties, but will likely run into many loader related issues.
            class Instance : public vvl::BaseInstance {
            public:
                Instance(vvl::DispatchInstance *dispatch) : BaseInstance(dispatch, LayerObjectTypeLegacy) {}

                // Special functions done in legacy_manual.cpp
                bool PreCallValidateCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                                VkInstance* pInstance, const ErrorObject& error_obj) const override;
                bool PreCallValidateCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                                const ErrorObject& error_obj) const override;
                bool ValidateLegacyExtensions(const Location& loc, vvl::Extension extension, APIVersion version) const;

        ''')

        for command in [x for x in self.vk.commands.values() if x.legacy and x.instance]:
            prototype = (command.cPrototype.split('VKAPI_CALL ')[1])[2:-1]
            prePrototype = prototype.replace(')', ', const ErrorObject& error_obj)')
            out.append(f'bool PreCallValidate{prePrototype} const override;\n')

        out.append('''
            };

            class Device : public vvl::BaseDevice {
            public:
                Device(vvl::DispatchDevice *dev, Instance *instance_vo)
                    : BaseDevice(dev, instance_vo, LayerObjectTypeLegacy), instance(instance_vo) {}
                ~Device() {}
                Instance *instance;

        ''')

        out.append('\n')

        for command in [x for x in self.vk.commands.values() if x.legacy and x.device]:
            # There is really no good use to warn developer both the create and destroy are superseded
            if command.name.startswith('vkDestroy'):
                continue

            prototype = (command.cPrototype.split('VKAPI_CALL ')[1])[2:-1]
            prePrototype = prototype.replace(')', ', const ErrorObject& error_obj)')
            out.append(f'bool PreCallValidate{prePrototype} const override;\n')

        out.append('};')
        out.append('}  // namespace legacy')
        self.write(''.join(out))

    def generateSource(self):
        out = []
        out.append('''
            #include "legacy.h"

            namespace legacy {
        ''')

        # Currently we only validate the incoming commands, not the <deprecate> tags around flags/enums/etc
        #
        # 1. It is easier to just do this as must things marked as "legacy" can't be used without being
        #    called in a vulkan command anyway.
        # 2. If we do decide to add them one day, we need to be VERY cautious as there are things like
        #    VkPipelineCreateFlags that is "supersededby" by VkPipelineCreateFlags2, but really we don't
        #    want to report those (as discussed in https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/8072).
        for command in [x for x in self.vk.commands.values() if x.legacy]:
            # There is really no good use to warn developer both the create and destroy are superseded
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

                if command.legacy.supersededBy:
                    replacement = f'which contains {command.legacy.supersededBy} that can be used instead'

                out.append(f'''
                    {logic} (api_version >= {command.legacy.version.nameApi}) {{
                        reported = true;
                        LogWarning("WARNING-{command.legacy.link}", {objName}, error_obj.location,
                            "{command.name} is a legacy command and this {handleName} was created with {command.legacy.version.name} {replacement}.\\nSee more information about this legacy in the specification: https://docs.vulkan.org/spec/latest/appendices/legacy.html#{command.legacy.link}");
                    }}''')

            for extension in command.legacy.extensions:
                logic = 'if' if firstCheck else 'else if'
                if firstCheck:
                    firstCheck = False

                if command.legacy.supersededBy:
                    # Currenty the |supersededBy| only has the version
                    # Slightly hacky way to check, will be fine until we have some strange legacy combo
                    new_command = command.legacy.supersededBy # backup value
                    if new_command[-3:].isupper() and new_command[-3:].isalpha():
                        new_command = command.legacy.supersededBy
                    elif (command.legacy.supersededBy + 'KHR') in self.vk.commands:
                        new_command += 'KHR'
                    elif (command.legacy.supersededBy + 'EXT') in self.vk.commands:
                        new_command += 'EXT'
                    else:
                        print(f'WARNING - need to fix supersededBy logic for {command.name} with {command.legacy.supersededBy}')
                    replacement = f'which contains {new_command} that can be used instead'

                out.append(f'''
                    {logic} (IsExtEnabled(extensions.{extension.lower()})) {{
                        reported = true;
                        LogWarning("WARNING-{command.legacy.link}", {objName}, error_obj.location,
                            "{command.name} is a legacy command and this {handleName} enabled the {extension} extension {replacement}.\\nSee more information about this legacy in the specification: https://docs.vulkan.org/spec/latest/appendices/legacy.html#{command.legacy.link}");
                    }}''')

            # For things mark as legacy in Vulkan 1.0
            if command.legacy.version is None and len(command.legacy.extensions) == 0:
                out.append(f'''
                           LogWarning("WARNING-{command.legacy.link}", {objName}, error_obj.location,
                            "{command.name} is a legacy command.\\nSee more information about this superseding in the specification: https://docs.vulkan.org/spec/latest/appendices/legacy.html#{command.legacy.link}");
                    ''')

            out.append('''
                return false;
            }
            ''')

        out.append('''
            ExtensionData GetExtensionData(vvl::Extension extension_name) {
                static const ExtensionData empty_data{Reason::Empty, vvl::Extension::Empty};
                static const vvl::unordered_map<vvl::Extension, ExtensionData> legacy_extensions = {
            ''')

        for extension in self.vk.extensions.values():
            target = None
            reason = None
            if extension.promotedTo is not None:
                reason = 'Reason::Promoted'
                target = extension.promotedTo
            elif extension.obsoletedBy is not None:
                reason = 'Reason::Obsoleted'
                target = extension.obsoletedBy
            elif extension.deprecatedBy is not None:
                reason = 'Reason::Superseded'
                target = extension.deprecatedBy
            else:
                continue

            if len(target) == 0:
                target = 'vvl::Extension::Empty'
            elif 'VERSION' in target:
                target = f'vvl::Version::_{target}'
            else:
                target = f'vvl::Extension::_{target}'

            out.append(f'    {{vvl::Extension::_{extension.name}, {{{reason}, {{{target}}}}}}},\n')
        out.append('''    };

                auto it = legacy_extensions.find(extension_name);
                return (it == legacy_extensions.end()) ? empty_data : it->second;
            }
            ''')

        out.append('}  // namespace legacy')
        self.write(''.join(out))
