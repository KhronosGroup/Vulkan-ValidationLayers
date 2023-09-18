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
import re
from generators.base_generator import BaseGenerator

from pyparsing import ParseResults
# From the Vulkan-Headers
from parse_dependency import dependencyBNF

def parseExpr(expr): return dependencyBNF().parseString(expr, parseAll=True)

def dependCheck(pr: ParseResults, token, op, start_group, end_group) -> None:
    """
    Run a set of callbacks on a boolean expression.

    token: run on a non-operator, non-parenthetical token
    op: run on an operator token
    start_group: run on a '(' token
    end_group: run on a ')' token
    """

    for r in pr:
        if isinstance(r, ParseResults):
            start_group()
            dependCheck(r, token, op, start_group, end_group)
            end_group()
        elif r in ',+':
            op(r)
        else:
            token(r)

def exprValues(pr: ParseResults) -> list:
    """
    Return a list of all "values" (i.e., non-operators) in the parsed expression.
    """

    values = []
    dependCheck(pr, lambda x: values.append(x), lambda x: None, lambda: None, lambda: None)
    return values

def exprToCpp(pr: ParseResults, opt = lambda x: x) -> str:
    r = []
    printExt = lambda x: r.append(opt(x))
    printOp = lambda x: r.append(' && ' if x == '+' else ' || ')
    openParen = lambda: r.append('(')
    closeParen = lambda: r.append(')')
    dependCheck(pr, printExt, printOp, openParen, closeParen)
    return ''.join(r)

# This class is a container for any source code, data, or other behavior that is necessary to
# customize the generator script for a specific target API variant (e.g. Vulkan SC). As such,
# all of these API-specific interfaces and their use in the generator script are part of the
# contract between this repository and its downstream users. Changing or removing any of these
# interfaces or their use in the generator script will have downstream effects and thus
# should be avoided unless absolutely necessary.
class APISpecific:
    # Returns dictionary of version field names
    @staticmethod
    def getVersionFieldNameDict(targetApiName: str) -> dict[str, str]:
        match targetApiName:

            # Vulkan specific version field names
            case 'vulkan':
                return {
                    'VK_VERSION_1_1': 'vk_feature_version_1_1',
                    'VK_VERSION_1_2': 'vk_feature_version_1_2',
                    'VK_VERSION_1_3': 'vk_feature_version_1_3'
                }


    # Returns dictionary of promoted extension array variable names
    @staticmethod
    def getPromotedExtensionArrayName(targetApiName: str, scope: str) -> dict[str, str]:
        match targetApiName:

            # Vulkan specific promoted extension array variable names
            case 'vulkan':
                return {
                    'VK_VERSION_1_1': f'V_1_1_promoted_{scope}_apis',
                    'VK_VERSION_1_2': f'V_1_2_promoted_{scope}_apis',
                    'VK_VERSION_1_3': f'V_1_3_promoted_{scope}_apis'
                }


class ExtensionHelperOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

    def generate(self):
        # [ Feature name | name in struct InstanceExtensions ]
        fieldName = dict()
        # [ Extension name : List[Extension | Version] ]
        requiredExpression = dict()
        for extension in self.vk.extensions.values():
            fieldName[extension.name] = extension.name.lower()
            requiredExpression[extension.name] = list()
            if extension.depends is not None:
                # This is a work around for https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5372
                temp = re.sub(r',VK_VERSION_1_\d+', '', extension.depends)
                for reqs in exprValues(parseExpr(temp)):
                    feature = self.vk.extensions[reqs] if reqs in self.vk.extensions else self.vk.versions[reqs]
                    requiredExpression[extension.name].append(feature)
        for version, field in APISpecific.getVersionFieldNameDict(self.targetApiName).items():
            fieldName[version] = field

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

            #include <string>
            #include <utility>
            #include <set>
            #include <array>
            #include <vector>
            #include <cassert>

            #include <vulkan/vulkan.h>
            #include "containers/custom_containers.h"
            #include "generated/vk_api_version.h"

            enum ExtEnabled : unsigned char {
                kNotEnabled,
                kEnabledByCreateinfo,
                kEnabledByApiLevel,
                kEnabledByInteraction,
            };

            /*
            This function is a helper to know if the extension is enabled.

            Times to use it
            - To determine the VUID
            - The VU mentions the use of the extension
            - Extension exposes property limits being validated
            - Checking not enabled
                - if (!IsExtEnabled(...)) { }
            - Special extensions that being EXPOSED alters the VUs
                - IsExtEnabled(device_extensions.vk_khr_portability_subset)
            - Special extensions that alter behaviour of enabled
                - IsExtEnabled(device_extensions.vk_khr_maintenance*)

            Times to NOT use it
                - If checking if a struct or enum is being used. There are a stateless checks
                  to make sure the new Structs/Enums are not being used without this enabled.
                - If checking if the extension's feature enable status, because if the feature
                  is enabled, then we already validated that extension is enabled.
                - Some variables (ex. viewMask) require the extension to be used if non-zero
            */
            [[maybe_unused]] static bool IsExtEnabled(ExtEnabled extension) { return (extension != kNotEnabled); }

            [[maybe_unused]] static bool IsExtEnabledByCreateinfo(ExtEnabled extension) { return (extension == kEnabledByCreateinfo); }
            ''')

        out.append('\nstruct InstanceExtensions {\n')
        for name in APISpecific.getVersionFieldNameDict(self.targetApiName).values():
            out.append(f'    ExtEnabled {name}{{kNotEnabled}};\n')

        out.extend([f'    ExtEnabled {ext.name.lower()}{{kNotEnabled}};\n' for ext in self.vk.extensions.values() if ext.instance])

        out.append('''
            struct InstanceReq {
                const ExtEnabled InstanceExtensions::*enabled;
                const char *name;
            };
            typedef std::vector<InstanceReq> InstanceReqVec;
            struct InstanceInfo {
                InstanceInfo(ExtEnabled InstanceExtensions::*state_, const InstanceReqVec requirements_)
                    : state(state_), requirements(requirements_) {}
                ExtEnabled InstanceExtensions::*state;
                InstanceReqVec requirements;
            };

            typedef vvl::unordered_map<std::string,InstanceInfo> InstanceInfoMap;
            static const InstanceInfoMap &get_info_map() {
                static const InstanceInfoMap info_map = {
            ''')
        for version, name in APISpecific.getVersionFieldNameDict(self.targetApiName).items():
            out.append(f'{{"{version}", InstanceInfo(&InstanceExtensions::{name}, {{}})}},\n')

        for extension in [x for x in self.vk.extensions.values() if x.instance]:
            out.extend([f'#ifdef {extension.protect}\n'] if extension.protect else [])
            reqs = ''
            # This is only done to match whitespace from before code we refactored
            if requiredExpression[extension.name]:
                reqs += '{\n'
                reqs += ',\n'.join([f'{{&InstanceExtensions::{fieldName[feature.name]}, {feature.nameString}}}' for feature in requiredExpression[extension.name]])
                reqs += '}'
            out.append(f'{{{extension.nameString}, InstanceInfo(&InstanceExtensions::{extension.name.lower()}, {{{reqs}}})}},\n')
            out.extend(['#endif\n'] if extension.protect else [])

        out.append('''
                };
                return info_map;
            }

            static const InstanceInfo &get_info(const char *name) {
                static const InstanceInfo empty_info{nullptr, InstanceReqVec()};
                const auto &ext_map = InstanceExtensions::get_info_map();
                const auto info = ext_map.find(name);
                if (info != ext_map.cend()) {
                    return info->second;
                }
                return empty_info;
            }

            APIVersion InitFromInstanceCreateInfo(APIVersion requested_api_version, const VkInstanceCreateInfo *pCreateInfo) {
            ''')
        promoted_var_names = APISpecific.getPromotedExtensionArrayName(self.targetApiName, 'instance')
        for version_name, promoted_var_name in promoted_var_names.items():
            promoted_ext_list = [x for x in self.vk.extensions.values() if x.promotedTo == version_name and x.instance]
            out.append(f'constexpr std::array<const char*, {len(promoted_ext_list)}> {promoted_var_name} = {{\n')
            out.extend(['    %s,\n' % ext.nameString for ext in promoted_ext_list])
            out.append('};\n')

        out.append('''
            // Initialize struct data, robust to invalid pCreateInfo
            auto api_version = NormalizeApiVersion(requested_api_version);''')
        for version_name, promoted_var_name in promoted_var_names.items():
            out.append(f'''
                if (api_version >= {version_name.replace('_VERSION_', '_API_VERSION_')}) {{
                    auto info = get_info("{version_name}");
                    if (info.state) this->*(info.state) = kEnabledByCreateinfo;
                    for (auto promoted_ext : {promoted_var_name}) {{
                        info = get_info(promoted_ext);
                        assert(info.state);
                        if (info.state) this->*(info.state) = kEnabledByApiLevel;
                    }}
                }}''')

        out.append('''
            // CreateInfo takes precedence over promoted
            if (pCreateInfo && pCreateInfo->ppEnabledExtensionNames) {
                for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
                    if (!pCreateInfo->ppEnabledExtensionNames[i]) continue;
                    auto info = get_info(pCreateInfo->ppEnabledExtensionNames[i]);
                    if (info.state) this->*(info.state) = kEnabledByCreateinfo;
                }
            }
            return api_version;
            }
            };
            ''')

        out.append('static const std::set<std::string> kInstanceExtensionNames = {\n')
        for extension in [x for x in self.vk.extensions.values() if x.instance]:
            out.extend([f'#ifdef {extension.protect}\n'] if extension.protect else [])
            out.append(f'    {extension.nameString},\n')
            out.extend(['#endif\n'] if extension.protect else [])
        out.append('};\n')

        out.append('\nstruct DeviceExtensions : public InstanceExtensions {\n')
        out.extend([f'    ExtEnabled {feature_field}{{kNotEnabled}};\n' for feature_field in APISpecific.getVersionFieldNameDict(self.targetApiName).values()])

        out.extend([f'    ExtEnabled {ext.name.lower()}{{kNotEnabled}};\n' for ext in self.vk.extensions.values() if ext.device])

        out.append('''
            struct DeviceReq {
                const ExtEnabled DeviceExtensions::*enabled;
                const char *name;
            };
            typedef std::vector<DeviceReq> DeviceReqVec;
            struct DeviceInfo {
                DeviceInfo(ExtEnabled DeviceExtensions::*state_, const DeviceReqVec requirements_)
                    : state(state_), requirements(requirements_) {}
                ExtEnabled DeviceExtensions::*state;
                DeviceReqVec requirements;
            };

            typedef vvl::unordered_map<std::string, DeviceInfo> DeviceInfoMap;
            static const DeviceInfoMap &get_info_map() {
                static const DeviceInfoMap info_map = {
            ''')
        for version, field in APISpecific.getVersionFieldNameDict(self.targetApiName).items():
            out.append(f'{{"{version}", DeviceInfo(&DeviceExtensions::{field}, {{}})}},\n')

        for extension in [x for x in self.vk.extensions.values() if x.device]:
            out.extend([f'#ifdef {extension.protect}\n'] if extension.protect else [])
            reqs = ''
            # This is only done to match whitespace from before code we refactored
            if requiredExpression[extension.name]:
                reqs += '{\n'
                reqs += ',\n'.join([f'{{&DeviceExtensions::{fieldName[feature.name]}, {feature.nameString}}}' for feature in requiredExpression[extension.name]])
                reqs += '}'
            out.append(f'{{{extension.nameString}, DeviceInfo(&DeviceExtensions::{extension.name.lower()}, {{{reqs}}})}},\n')
            out.extend(['#endif\n'] if extension.protect else [])

        out.append('''
                };

                return info_map;
            }

            static const DeviceInfo &get_info(const char *name) {
                static const DeviceInfo empty_info{nullptr, DeviceReqVec()};
                const auto &ext_map = DeviceExtensions::get_info_map();
                const auto info = ext_map.find(name);
                if (info != ext_map.cend()) {
                    return info->second;
                }
                return empty_info;
            }

            DeviceExtensions() = default;
            DeviceExtensions(const InstanceExtensions &instance_ext) : InstanceExtensions(instance_ext) {}

            APIVersion InitFromDeviceCreateInfo(const InstanceExtensions *instance_extensions, APIVersion requested_api_version,
                                                const VkDeviceCreateInfo *pCreateInfo = nullptr) {
                // Initialize: this to defaults,  base class fields to input.
                assert(instance_extensions);
                *this = DeviceExtensions(*instance_extensions);
            ''')
        promoted_var_names = APISpecific.getPromotedExtensionArrayName(self.targetApiName, 'device')
        for version_name, promoted_var_name in promoted_var_names.items():
            promoted_ext_list = [x for x in self.vk.extensions.values() if x.promotedTo == version_name and x.device]
            out.append(f'constexpr std::array<const char*, {len(promoted_ext_list)}> {promoted_var_name} = {{\n')
            out.extend(['    %s,\n' % ext.nameString for ext in promoted_ext_list])
            out.append('};\n')

        out.append('''
            // Initialize struct data, robust to invalid pCreateInfo
            auto api_version = NormalizeApiVersion(requested_api_version);''')
        for version_name, promoted_var_name in promoted_var_names.items():
            out.append(f'''
                if (api_version >= {version_name.replace('_VERSION_', '_API_VERSION_')}) {{
                    auto info = get_info("{version_name}");
                    if (info.state) this->*(info.state) = kEnabledByCreateinfo;
                    for (auto promoted_ext : {promoted_var_name}) {{
                        info = get_info(promoted_ext);
                        assert(info.state);
                        if (info.state) this->*(info.state) = kEnabledByApiLevel;
                    }}
                }}''')

        out.append('''
            // CreateInfo takes precedence over promoted
            if (pCreateInfo && pCreateInfo->ppEnabledExtensionNames) {
                for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
                    if (!pCreateInfo->ppEnabledExtensionNames[i]) continue;
                    auto info = get_info(pCreateInfo->ppEnabledExtensionNames[i]);
                    if (info.state) this->*(info.state) = kEnabledByCreateinfo;
                }
            }
            // Workaround for functions being introduced by multiple extensions, until the layer is fixed to handle this correctly
            // See https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5579 and https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5600
            {
                constexpr std::array shader_object_interactions = {
                    VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
                    VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME,
                    VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,
                    VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME,
                };
                auto info = get_info(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
                if (info.state) {
                    if (this->*(info.state) != kNotEnabled) {
                        for (auto interaction_ext : shader_object_interactions) {
                            info = get_info(interaction_ext);
                            assert(info.state);
                            if (this->*(info.state) != kEnabledByCreateinfo) {
                                this->*(info.state) = kEnabledByInteraction;
                            }
                        }
                    }
                }
            }
            return api_version;
            }
            };

            ''')
        out.append('static const std::set<std::string> kDeviceExtensionNames = {\n')
        for extension in [x for x in self.vk.extensions.values() if x.device]:
            out.extend([f'#ifdef {extension.protect}\n'] if extension.protect else [])
            out.append(f'    {extension.nameString},\n')
            out.extend(['#endif\n'] if extension.protect else [])
        out.append('};\n')

        out.append('// NOLINTEND') # Wrap for clang-tidy to ignore
        self.write(''.join(out))
