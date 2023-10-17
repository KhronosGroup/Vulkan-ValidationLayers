#!/usr/bin/python3 -i
#
# Copyright (c) 2023 The Khronos Group Inc.
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

# For detecting VkPhysicalDevice*Features* structs
featuresStructPattern = re.compile(r'VkPhysicalDevice.*Features[A-Z0-9]*')

#
# StateTrackerHelperOutputGenerator - Generate helpers to discover enabled features.
class StateTrackerHelperOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

        # Features of the VkPhysicalDeviceBufferDeviceAddressFeaturesEXT have
        # the same name as Vulkan 1.2 and
        # VkPhysicalDeviceBufferDeviceAddressFeaturesKHR features, but are
        # semantically different.  They are given a suffix to be distinguished.
        self.identical_but_different_features = {
            'VkPhysicalDeviceBufferDeviceAddressFeaturesEXT': 'EXT',
        }

    def generate(self):
        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2023 Google Inc.
            * Copyright (c) 2023 LunarG, Inc.
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

        if self.filename == 'state_tracker_helper.h':
            self.generateHeader()
        elif self.filename == 'state_tracker_helper.cpp':
            self.generateSource()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def addToDict(self, dictionary, name, value):
        if name not in dictionary:
            dictionary[name] = set()
        dictionary[name].add(value)

    def generateHeader(self):
        # Map feature names to structs that have them
        featureMap = dict()
        for struct, info in self.vk.structs.items():
            if featuresStructPattern.match(struct) is None:
                continue

            suffix = ''
            if struct in self.identical_but_different_features:
                suffix = self.identical_but_different_features[struct]

            for feature in info.members:
                if feature.type == 'VkBool32':
                    self.addToDict(featureMap, feature.name + suffix, struct)

        # Generate a comment for every feature regarding where it may be coming from, then sort the
        # features by that comment.  That ensures features of the same struct end up together.
        featuresAndOrigins = sorted([(', '.join(sorted(structs)), feature)
                                     for feature, structs in featureMap.items()])

        out = []
        out.append('''
            #pragma once

            // Union of all features defined in VkPhysicalDevice*Features* structs
            struct DeviceFeatures {
            ''')
        for origins, feature in featuresAndOrigins:
            out.append(f'// {origins}\n')
            out.append(f'bool {feature};\n')

        out.append('''};

            void GetEnabledDeviceFeatures(const VkDeviceCreateInfo *pCreateInfo, DeviceFeatures *features, const APIVersion &api_version);
            ''')
        self.write("".join(out))

    def generateSource(self):
        out = []
        # Handle pCreateInfo->pEnabledFeatures first, it's the only one not necessarily found in the
        # pNext chain.
        out.append('''
            #include "state_tracker/state_tracker.h"

            void GetEnabledDeviceFeatures(const VkDeviceCreateInfo *pCreateInfo, DeviceFeatures *features, const APIVersion &api_version) {
                // Initialize all to false
                *features = {};

                // handle VkPhysicalDeviceFeatures specially as it is not part of the pNext chain,
                // and when it is (through VkPhysicalDeviceFeatures2), it requires an extra indirection.
                const VkPhysicalDeviceFeatures *core_features = pCreateInfo->pEnabledFeatures;
                if (core_features == nullptr) {
                    const VkPhysicalDeviceFeatures2 *features2 = vku::FindStructInPNextChain<VkPhysicalDeviceFeatures2>(pCreateInfo->pNext);
                    if (features2 != nullptr) {
                        core_features = &features2->features;
                    }
                }
                if (core_features != nullptr) {
            ''')
        for member in self.vk.structs['VkPhysicalDeviceFeatures'].members:
            if member.type == 'VkBool32':
                out.append(f'features->{member.name} = core_features->{member.name} == VK_TRUE;\n')
        out.append('}\n')

        # Handle every features struct in the pnext chain.
        out.append('''
                for(const VkBaseInStructure *pNext = reinterpret_cast<const VkBaseInStructure *>(pCreateInfo->pNext);
                    pNext != nullptr; pNext = pNext->pNext) {
                    switch (pNext->sType) {
            ''')

        for struct, info in self.vk.structs.items():
            if featuresStructPattern.match(struct) is None:
                continue
            if info.sType is None:
                # Only features struct without an sType is VkPhysicalDeviceFeatures
                assert(struct == 'VkPhysicalDeviceFeatures')
                continue
            if struct == 'VkPhysicalDeviceFeatures2':
                # VkPhysicalDeviceFeatures2 is already handled
                continue

            suffix = ''
            if struct in self.identical_but_different_features:
                suffix = self.identical_but_different_features[struct]

            out.extend([f'#ifdef {info.protect}\n'] if info.protect else [])
            out.append(f'case {info.sType}:')
            out.append('{\n')
            out.append(f'const {struct} *enabled = reinterpret_cast<const {struct} *>(pNext);\n')

            for member in info.members:
                if member.type == 'VkBool32':
                    feature = member.name
                    out.append(f'features->{feature}{suffix} |= enabled->{feature} == VK_TRUE;\n')

            out.append('break;\n}\n')
            out.extend([f'#endif //{info.protect}\n'] if info.protect else [])

        out.append('''
                    default:
                        break;
                    }
                }
            ''')

        # Handle Extension Feature Aliases:
        extension_feature_alises = {
                'VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME': ['shaderDrawParameters'],
                'VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME': ['drawIndirectCount'],
                'VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME': ['samplerMirrorClampToEdge'],
                'VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME': ['descriptorIndexing'],
                'VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME': ['samplerFilterMinmax'],
                'VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME': ['shaderOutputViewportIndex', 'shaderOutputLayer'],
        }
        out.append('''
                // Some older extensions were made without features, but equivalent features were
                // added to the core spec when they were promoted.  When those extensions are
                // enabled, treat validation rules as if the corresponding feature is enabled.
                for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
                    const char *extension = pCreateInfo->ppEnabledExtensionNames[i];
            ''')
        for ext, features in sorted(extension_feature_alises.items()):
            out.append(f'if (0 == strncmp(extension, {ext}, VK_MAX_EXTENSION_NAME_SIZE))')
            out.append('{\n')
            for feature in features:
                out.append(f'    features->{feature} = true;\n')
            out.append('}\n')
        out.append('}\n')

        # Finally, handle oddities:
        out.append('''
                // texelBufferAlignment was not promoted to VkPhysicalDeviceVulkan13Features
                // but the feature is automatically enabled.
                // Setting the feature explicitly to 'false' doesn't change that
                if (api_version >= VK_API_VERSION_1_3) {
                    features->texelBufferAlignment = true;
                }
            }
            ''')

        self.write("".join(out))
