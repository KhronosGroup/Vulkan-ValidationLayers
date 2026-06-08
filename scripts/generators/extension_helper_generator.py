#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2026 The Khronos Group Inc.
# Copyright (c) 2015-2026 Valve Corporation
# Copyright (c) 2015-2026 LunarG, Inc.
# Copyright (c) 2015-2026 Google Inc.
# Copyright (c) 2023-2025 RasterGrid Kft.
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
from base_generator import BaseGenerator
from generators.generator_utils import PlatformGuardHelper

# Need pyparsing because the Vulkan-Headers use it in dependencyBNF
from pyparsing import ParseResults
# From the Vulkan-Headers
from parse_dependency import dependencyBNF

def parseExpr(expr): return dependencyBNF().parse_string(expr, parseAll=True)

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

def exprToOrGroups(pr: ParseResults) -> list:
    """
    Convert a parsed dependency expression into a list of "OR groups" that are AND'ed
    together (i.e. conjunctive normal form): the expression is satisfied only if every
    group is satisfied, and a group is satisfied if any one of its members is present.

    Every 'depends' expression in the spec reduces to this shape - an AND of single
    values and/or parenthesized OR-groups of plain values, with no deeper nesting -
    once 'VK_VERSION_1_x' OR-alternatives are stripped out (see the caller). Anything
    else raises so that a future spec expression this can't represent (which would need
    full CNF conversion, e.g. distributing OR over AND) is caught here at generation
    time, instead of silently producing an incorrect requirement check.
    """

    # Walk the expression with dependCheck, building a normalized tree where each node is
    # either a bare leaf name or (operator, [child, ...]). A group's operator is only known
    # once it is fully parsed, so collapsing a redundant single-child group (e.g. '((A))' ->
    # 'A') and flattening chains of the same operator (e.g. 'A+(B+C)' -> ('+', [A, B, C]))
    # both happen here, in finalize(), rather than as the tree is built.
    def finalize(op, children):
        if op is None:
            return children[0]
        flat = []
        for child in children:
            flat.extend(child[1] if isinstance(child, tuple) and child[0] == op else [child])
        return (op, flat)

    stack = [[None, []]]  # [operator, children] per nesting level; stack[0] is the root

    def on_token(name):
        stack[-1][1].append(name)

    def on_op(o):
        level = stack[-1]
        assert level[0] in (None, o), f'Mixed operators at one nesting level of dependency expression: {pr}'
        level[0] = o

    def start_group():
        stack.append([None, []])

    def end_group():
        op, children = stack.pop()
        stack[-1][1].append(finalize(op, children))

    dependCheck(pr, on_token, on_op, start_group, end_group)
    node = finalize(*stack[0])

    if isinstance(node, str):
        return [[node]]

    def as_or_group(n):
        """A node that must be a flat OR-group of plain values: either a single leaf, or values joined by ','."""
        if isinstance(n, str):
            return [n]
        op, values = n
        assert op == ',' and all(isinstance(v, str) for v in values), \
            f'Nested expression where a flat OR-group is expected, full CNF conversion needed to handle: {pr}'
        return values

    op, children = node
    if op == ',':
        return [as_or_group(node)]
    return [as_or_group(child) for child in children]

def exprToCpp(pr: ParseResults, opt = lambda x: x) -> str:
    r = []
    printExt = lambda x: r.append(opt(x))
    printOp = lambda x: r.append(' && ' if x == '+' else ' || ')
    openParen = lambda: r.append('(')
    closeParen = lambda: r.append(')')
    dependCheck(pr, printExt, printOp, openParen, closeParen)
    return ''.join(r)


class ExtensionHelperOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)
        # [ Feature name | name in struct InstanceExtensions ]
        self.fieldName = dict()
        # [ Extension name : List[OR-group of Extension | Version] ] - the groups are AND'ed together,
        # and a group is satisfied if any one of its members is enabled (conjunctive normal form)
        self.requiredExpression = dict()

    def generate(self):
        for extension in self.vk.extensions.values():
            self.fieldName[extension.name] = extension.name.lower()
            self.requiredExpression[extension.name] = list()
            if extension.depends is not None:
                # This is a work around for https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5372
                temp = re.sub(r',VK_VERSION_1_\d+', '', extension.depends)
                # It can look like (VK_KHR_timeline_semaphore,VK_VERSION_1_2) or (VK_VERSION_1_2,VK_KHR_timeline_semaphore)
                temp = re.sub(r'VK_VERSION_1_\d+,', '', temp)
                for orGroup in exprToOrGroups(parseExpr(temp)):
                    features = [self.vk.extensions[name] if name in self.vk.extensions else self.vk.versions[name] for name in orGroup]
                    self.requiredExpression[extension.name].append(features)
        for version in self.vk.versions.keys():
            self.fieldName[version] = version.lower().replace('version', 'feature_version')

        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2015-2026 The Khronos Group Inc.
            * Copyright (c) 2015-2026 Valve Corporation
            * Copyright (c) 2015-2026 LunarG, Inc.
            * Copyright (c) 2015-2026 Google Inc.
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

        if self.filename == 'vk_extension_helper.h':
            self.generateHeader()
        elif self.filename == 'vk_extension_helper.cpp':
            self.generateSource()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateHeader(self):
        out = []
        out.append('''
            #pragma once

            #include <string>
            #include <utility>
            #include <vector>
            #include <cassert>

            #include <vulkan/vulkan.h>
            #include "containers/custom_containers.h"
            #include "generated/vk_api_version.h"
            #include "generated/error_location_helper.h"

            // Extensions (unlike functions, struct, etc) are passed in as strings.
            // The goal is to turn the string to a enum and pass that around the layers.
            // Only when we need to print an error do we try and turn it back into a string
            vvl::Extension GetExtension(std::string extension);

            enum ExtEnabled : unsigned char {
                kNotSupported,
                kNotEnabled,            // Extension is supported, but not enabled
                kEnabledByCreateinfo,  // Extension is passed at vkCreateDevice/vkCreateInstance time
                kEnabledByApiLevel,  // the API version implicitly enabled it
                kEnabledByInteraction,  // is implicity enabled by another extension
            };

            // Map of promoted extension information per version (a separate map exists for instance and device extensions).
            // The map is keyed by the version number (e.g. VK_API_VERSION_1_1) and each value is a pair consisting of the
            // version string (e.g. "VK_VERSION_1_1") and the set of name of the promoted extensions.
            using PromotedExtensionInfoMap = vvl::unordered_map<uint32_t, std::pair<const char*, vvl::unordered_set<vvl::Extension>>>;
            const PromotedExtensionInfoMap& GetInstancePromotionInfoMap();
            const PromotedExtensionInfoMap& GetDevicePromotionInfoMap();

            /*
            This function is a helper to know if the extension is enabled.

            Times to use it
            - To determine the VUID
            - The VU mentions the use of the extension
            - Extension exposes property limits being validated
            - Checking not enabled
                - if (!IsExtEnabled(...)) { }
            - Special extensions that being EXPOSED alters the VUs
                - IsExtEnabled(extensions.vk_khr_portability_subset)
            - Special extensions that alter behaviour of enabled
                - IsExtEnabled(extensions.vk_khr_maintenance*)

            Times to NOT use it
                - If checking if a struct or enum is being used. There are a stateless checks
                  to make sure the new Structs/Enums are not being used without this enabled.
                - If checking if the extension's feature enable status, because if the feature
                  is enabled, then we already validated that extension is enabled.
                - Some variables (ex. viewMask) require the extension to be used if non-zero
            */
            [[maybe_unused]] static bool IsExtEnabled(ExtEnabled extension) { return (extension == kEnabledByCreateinfo || extension == kEnabledByApiLevel || extension == kEnabledByInteraction); }

            [[maybe_unused]] static bool IsExtSupported(ExtEnabled extension) { return (extension != kNotSupported); }

            [[maybe_unused]] static bool IsExtEnabledByCreateinfo(ExtEnabled extension) { return (extension == kEnabledByCreateinfo); }
            ''')

        out.append('\nstruct InstanceExtensions {\n')
        out.append('    APIVersion api_version{};\n')
        for version in self.vk.versions.keys():
            out.append(f'    ExtEnabled {self.fieldName[version]}{{kNotSupported}};\n')

        out.extend([f'    ExtEnabled {ext.name.lower()}{{kNotSupported}};\n' for ext in self.vk.extensions.values() if ext.instance])

        out.append('''
            struct Requirement {
                const ExtEnabled InstanceExtensions::*enabled;
                const char *name;
            };
            // Only one requirement needs to be satisfied
            using RequirementOrGroup = small_vector<Requirement, 1>;
            using RequirementVec = std::vector<RequirementOrGroup>;
            struct Info {
                Info(ExtEnabled InstanceExtensions::*state_, const RequirementVec requirements_)
                    : state(state_), requirements(requirements_) {}
                ExtEnabled InstanceExtensions::*state;
                RequirementVec requirements;
            };

            const Info &GetInfo(vvl::Extension extension_name) const;

            InstanceExtensions() = default;
            InstanceExtensions(APIVersion requested_api_version, const VkInstanceCreateInfo *pCreateInfo);

            };
            ''')

        out.append('\nstruct DeviceExtensions : public InstanceExtensions {\n')
        for version in self.vk.versions.keys():
            out.append(f'    ExtEnabled {self.fieldName[version]}{{kNotSupported}};\n')

        out.extend([f'    ExtEnabled {ext.name.lower()}{{kNotSupported}};\n' for ext in self.vk.extensions.values() if ext.device])

        out.append('''
            struct Requirement {
                const ExtEnabled DeviceExtensions::*enabled;
                const char *name;
            };
            // Only one requirement needs to be satisfied
            using RequirementOrGroup = std::vector<Requirement>;
            using RequirementVec = std::vector<RequirementOrGroup>;
            struct Info {
                Info(ExtEnabled DeviceExtensions::*state_, const RequirementVec requirements_)
                    : state(state_), requirements(requirements_) {}
                ExtEnabled DeviceExtensions::*state;
                RequirementVec requirements;
            };

            const Info &GetInfo(vvl::Extension extension_name) const;

            DeviceExtensions() = default;
            DeviceExtensions(const InstanceExtensions &instance_ext) : InstanceExtensions(instance_ext) {}

            DeviceExtensions(const InstanceExtensions &instance_extensions, APIVersion requested_api_version,
                                                const VkDeviceCreateInfo *pCreateInfo = nullptr);
            DeviceExtensions(const InstanceExtensions &instance_ext, APIVersion requested_api_version, const std::vector<VkExtensionProperties> &props);
            };

            const InstanceExtensions::Info &GetInstanceVersionMap(const char* version);
            const DeviceExtensions::Info &GetDeviceVersionMap(const char* version);

            ''')

        out.append('''
            constexpr bool IsInstanceExtension(vvl::Extension extension) {
                switch (extension) {
            ''')
        out.extend([f'case vvl::Extension::_{x.name}:\n' for x in self.vk.extensions.values() if x.instance])
        out.append('''    return true;''')
        out.append('''default: return false;
            }
        }\n''')

        out.append('''
            constexpr bool IsDeviceExtension(vvl::Extension extension) {
                switch (extension) {
            ''')
        out.extend([f'case vvl::Extension::_{x.name}:\n' for x in self.vk.extensions.values() if x.device])
        out.append('''    return true;''')
        out.append('''default: return false;
            }
        }\n''')

        self.write(''.join(out))

    def generateSource(self):
        guard_helper = PlatformGuardHelper()
        out = []
        out.append('''
            #include "vk_extension_helper.h"

            vvl::Extension GetExtension(std::string extension) {
                static const vvl::unordered_map<std::string, vvl::Extension> extension_map {
            ''')
        for extension in self.vk.extensions.values():
            out.append(f'    {{"{extension.name}", vvl::Extension::_{extension.name}}},\n')
        out.append('''    };
                const auto it = extension_map.find(extension);
                return (it == extension_map.end()) ? vvl::Extension::Empty : it->second;
            }

            const PromotedExtensionInfoMap &GetInstancePromotionInfoMap() {
                static const PromotedExtensionInfoMap promoted_map = {
            ''')

        for version in self.vk.versions.keys():
            promoted_ext_list = [x for x in self.vk.extensions.values() if x.promotedTo == version and getattr(x, 'instance')]
            if len(promoted_ext_list) > 0:
                out.append(f'{{{version.replace("VERSION", "API_VERSION")},{{"{version}",{{')
                out.extend(['    %s,\n' % f'vvl::Extension::_{ext.name}' for ext in promoted_ext_list])
                out.append('}}},\n')

        out.append('''
                };
                return promoted_map;
            }

            const PromotedExtensionInfoMap &GetDevicePromotionInfoMap() {
                static const PromotedExtensionInfoMap promoted_map = {
            ''')

        for version in self.vk.versions.keys():
            promoted_ext_list = [x for x in self.vk.extensions.values() if x.promotedTo == version and getattr(x, 'device')]
            if len(promoted_ext_list) > 0:
                out.append(f'{{{version.replace("VERSION", "API_VERSION")},{{"{version}",{{')
                out.extend(['    %s,\n' % f'vvl::Extension::_{ext.name}' for ext in promoted_ext_list])
                out.append('}}},\n')

        out.append('''
                };
                return promoted_map;
            }

            const InstanceExtensions::Info &GetInstanceVersionMap(const char* version) {
                static const InstanceExtensions::Info empty_info{nullptr, InstanceExtensions::RequirementVec()};
                static const vvl::unordered_map<std::string_view, InstanceExtensions::Info> version_map = {
            ''')
        for version in self.vk.versions.keys():
            out.append(f'{{"{version}", InstanceExtensions::Info(&InstanceExtensions::{self.fieldName[version]}, {{}})}},\n')
        out.append('''};
                const auto info = version_map.find(version);
                return (info != version_map.cend()) ? info->second : empty_info;
            }

            const DeviceExtensions::Info &GetDeviceVersionMap(const char* version) {
                static const DeviceExtensions::Info empty_info{nullptr, DeviceExtensions::RequirementVec()};
                static const vvl::unordered_map<std::string_view, DeviceExtensions::Info> version_map = {
            ''')
        for version in self.vk.versions.keys():
            out.append(f'{{"{version}", DeviceExtensions::Info(&DeviceExtensions::{self.fieldName[version]}, {{}})}},\n')
        out.append('''};
                const auto info = version_map.find(version);
                return (info != version_map.cend()) ? info->second : empty_info;
            }

            InstanceExtensions::InstanceExtensions(APIVersion requested_api_version, const VkInstanceCreateInfo* pCreateInfo) {
                // Initialize struct data, robust to invalid pCreateInfo
                api_version = NormalizeApiVersion(requested_api_version);
                if (!api_version.Valid()) {
                    return;
                }

                const auto promotion_info_map = GetInstancePromotionInfoMap();
                for (const auto& version_it : promotion_info_map) {
                    auto info = GetInstanceVersionMap(version_it.second.first);
                    if (api_version >= version_it.first) {
                        if (info.state) this->*(info.state) = kEnabledByCreateinfo;
                        for (const auto& extension : version_it.second.second) {
                            info = GetInfo(extension);
                            assert(info.state);
                            if (info.state) this->*(info.state) = kEnabledByApiLevel;
                        }
                    }
                }

                // CreateInfo takes precedence over promoted
                if (pCreateInfo && pCreateInfo->ppEnabledExtensionNames) {
                    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
                        if (!pCreateInfo->ppEnabledExtensionNames[i]) continue;
                        vvl::Extension extension = GetExtension(pCreateInfo->ppEnabledExtensionNames[i]);
                        auto info = GetInfo(extension);
                        if (info.state) this->*(info.state) = kEnabledByCreateinfo;
                    }
                }
            }

            DeviceExtensions::DeviceExtensions(const InstanceExtensions& instance_ext,
                                               APIVersion requested_api_version,
                                               const VkDeviceCreateInfo* pCreateInfo)
                : InstanceExtensions(instance_ext) {

                auto api_version = NormalizeApiVersion(requested_api_version);
                if (!api_version.Valid()) {
                    return;
                }

                const auto promotion_info_map = GetDevicePromotionInfoMap();
                for (const auto& version_it : promotion_info_map) {
                    auto info = GetDeviceVersionMap(version_it.second.first);
                    if (api_version >= version_it.first) {
                        if (info.state) this->*(info.state) = kEnabledByCreateinfo;
                        for (const auto& extension : version_it.second.second) {
                            info = GetInfo(extension);
                            assert(info.state);
                            if (info.state) this->*(info.state) = kEnabledByApiLevel;
                        }
                    }
                }

                // CreateInfo takes precedence over promoted
                if (pCreateInfo && pCreateInfo->ppEnabledExtensionNames) {
                    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
                        if (!pCreateInfo->ppEnabledExtensionNames[i]) continue;
                        vvl::Extension extension = GetExtension(pCreateInfo->ppEnabledExtensionNames[i]);
                        auto info = GetInfo(extension);
                        if (info.state) this->*(info.state) = kEnabledByCreateinfo;
                    }
                }

                // Workaround for functions being introduced by multiple extensions, until the layer is fixed to handle this correctly
                // See https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5579 and
                // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5600
                {
                    constexpr std::array shader_object_interactions = {
                        vvl::Extension::_VK_EXT_extended_dynamic_state,
                        vvl::Extension::_VK_EXT_extended_dynamic_state2,
                        vvl::Extension::_VK_EXT_extended_dynamic_state3,
                        vvl::Extension::_VK_EXT_vertex_input_dynamic_state,
                    };
                    auto info = GetInfo(vvl::Extension::_VK_EXT_shader_object);
                    if (info.state) {
                        if (IsExtEnabled(this->*(info.state))) {
                            for (auto interaction_ext : shader_object_interactions) {
                                info = GetInfo(interaction_ext);
                                assert(info.state);
                                if (this->*(info.state) != kEnabledByCreateinfo) {
                                    this->*(info.state) = kEnabledByInteraction;
                                }
                            }
                        }
                    }
                }
            }

            DeviceExtensions::DeviceExtensions(const InstanceExtensions& instance_ext,
                                               APIVersion requested_api_version,
                                               const std::vector<VkExtensionProperties> &props)
                : InstanceExtensions(instance_ext) {

                auto api_version = NormalizeApiVersion(requested_api_version);
                if (!api_version.Valid()) {
                    return;
                }

                const auto promotion_info_map = GetDevicePromotionInfoMap();
                for (const auto& version_it : promotion_info_map) {
                    auto info = GetDeviceVersionMap(version_it.second.first);
                    if (api_version >= version_it.first) {
                        if (info.state) this->*(info.state) = kEnabledByCreateinfo;
                        for (const auto& extension : version_it.second.second) {
                            info = GetInfo(extension);
                            assert(info.state);
                            if (info.state) this->*(info.state) = kEnabledByApiLevel;
                        }
                    }
                }
                for (const auto &prop : props) {
                    vvl::Extension extension = GetExtension(prop.extensionName);
                    auto info = GetInfo(extension);
                    if (info.state) this->*(info.state) = kEnabledByCreateinfo;
                }

                // Workaround for functions being introduced by multiple extensions, until the layer is fixed to handle this correctly
                // See https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5579 and
                // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5600
                {
                    constexpr std::array shader_object_interactions = {
                        vvl::Extension::_VK_EXT_extended_dynamic_state,
                        vvl::Extension::_VK_EXT_extended_dynamic_state2,
                        vvl::Extension::_VK_EXT_extended_dynamic_state3,
                        vvl::Extension::_VK_EXT_vertex_input_dynamic_state,
                    };
                    auto info = GetInfo(vvl::Extension::_VK_EXT_shader_object);
                    if (info.state) {
                        if (IsExtEnabled(this->*(info.state))) {
                            for (auto interaction_ext : shader_object_interactions) {
                                info = GetInfo(interaction_ext);
                                assert(info.state);
                                if (this->*(info.state) != kEnabledByCreateinfo) {
                                    this->*(info.state) = kEnabledByInteraction;
                                }
                            }
                        }
                    }
                }
            }
    ''')

        out.append('''
            using InstanceExtensionsInfoMap = vvl::unordered_map<vvl::Extension, InstanceExtensions::Info>;
            static const InstanceExtensionsInfoMap& GetInstanceInfoMap() {
                using Info = InstanceExtensions::Info;
                static const InstanceExtensionsInfoMap info_map = {
            ''')
        for extension in [x for x in self.vk.extensions.values() if x.instance]:
            out.extend(guard_helper.add_guard(extension.protect))
            reqs = ''
            # This is only done to match whitespace from before code we refactored
            if self.requiredExpression[extension.name]:
                reqs += '{\n'
                reqs += ',\n'.join(['{' + ','.join(f'{{&InstanceExtensions::{self.fieldName[feature.name]}, {feature.nameString}}}' for feature in orGroup) + '}'
                                    for orGroup in self.requiredExpression[extension.name]])
                reqs += '}'
            out.append(f'{{vvl::Extension::_{extension.name}, Info(&InstanceExtensions::{extension.name.lower()}, {{{reqs}}})}},\n')
        out.extend(guard_helper.add_guard(None))
        out.append('''
                };

                return info_map;
            }

            using DeviceExtensionsInfoMap = vvl::unordered_map<vvl::Extension, DeviceExtensions::Info>;
            static const DeviceExtensionsInfoMap& GetDeviceInfoMap() {
                using Info = DeviceExtensions::Info;
                static const DeviceExtensionsInfoMap info_map = {
            ''')
        for extension in [x for x in self.vk.extensions.values() if x.device]:
            out.extend(guard_helper.add_guard(extension.protect))
            reqs = ''
            # This is only done to match whitespace from before code we refactored
            if self.requiredExpression[extension.name]:
                reqs += '{\n'
                reqs += ',\n'.join(['{' + ','.join(f'{{&DeviceExtensions::{self.fieldName[feature.name]}, {feature.nameString}}}' for feature in orGroup) + '}'
                                    for orGroup in self.requiredExpression[extension.name]])
                reqs += '}'
            out.append(f'{{vvl::Extension::_{extension.name}, Info(&DeviceExtensions::{extension.name.lower()}, {{{reqs}}})}},\n')
        out.extend(guard_helper.add_guard(None))


        out.append('''
                };
                return info_map;
            }
        ''')

        out.append('''
            const InstanceExtensions::Info &InstanceExtensions::GetInfo(vvl::Extension extension_name) const {
                static const InstanceExtensions::Info empty_info{nullptr, RequirementVec()};
                const auto &ext_map = GetInstanceInfoMap();
                const auto info = ext_map.find(extension_name);
                return (info != ext_map.cend()) ? info->second : empty_info;
            }

            const DeviceExtensions::Info &DeviceExtensions::GetInfo(vvl::Extension extension_name) const {
                static const DeviceExtensions::Info empty_info{nullptr, RequirementVec()};
                const auto &ext_map = GetDeviceInfoMap();
                const auto info = ext_map.find(extension_name);
                return (info != ext_map.cend()) ? info->second : empty_info;
            }
        ''')
        self.write(''.join(out))
