#!/usr/bin/python3 -i
#
# Copyright (c) 2020-2023 The Khronos Group Inc.
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

import sys
import os
from generators.vulkan_object import SpirvEnables
from generators.base_generator import BaseGenerator

#
# Generate SPIR-V validation for SPIR-V extensions and capabilities
class SpirvValidationHelperOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

        # Sometimes the Vulkan-Headers XML will mention new SPIR-V capability or extensions
        # That require an update of the SPIRV-Headers which might not be ready to pull in.
        # These 2 arrays SHOULD be empty when possible and when the SPIR-V Headers are updated these
        # should be attempted to be cleared
        self.capabilityExcludeList = [
            'ClusterCullingShadingHUAWEI',
            'ShaderEnqueueAMDX',
            'TextureBlockMatch2QCOM',
        ]

        # There are some enums that share the same value in the SPIR-V header.
        # This array remove the duplicate to not print out, usually due to being the older value given
        self.capabilityAliasList = [
          'ShaderViewportIndexLayerNV',
          'ShadingRateNV',
          'FragmentBarycentricNV',
        ]

        # Promoted features structure in state_tracker.cpp are put in the VkPhysicalDeviceVulkan*Features structs
        # but the XML can still list them. This list all promoted structs to ignore since they are aliased.
        # Tried to generate these, but no reliable way from vk.xml
        self.promotedFeatures = [
            # 1.1
            "VkPhysicalDevice16BitStorageFeatures",
            "VkPhysicalDeviceMultiviewFeatures",
            "VkPhysicalDeviceVariablePointersFeatures",
            "VkPhysicalDeviceProtectedMemoryFeatures",
            "VkPhysicalDeviceSamplerYcbcrConversionFeatures",
            "VkPhysicalDeviceShaderDrawParametersFeatures",
            # 1.2
            "VkPhysicalDevice8BitStorageFeatures",
            "VkPhysicalDeviceShaderFloat16Int8Features",
            "VkPhysicalDeviceDescriptorIndexingFeatures",
            "VkPhysicalDeviceScalarBlockLayoutFeatures",
            "VkPhysicalDeviceImagelessFramebufferFeatures",
            "VkPhysicalDeviceUniformBufferStandardLayoutFeatures",
            "VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures",
            "VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures",
            "VkPhysicalDeviceTimelineSemaphoreFeatures",
            "VkPhysicalDeviceBufferDeviceAddressFeatures",
            "VkPhysicalDeviceShaderAtomicInt64Features",
            "VkPhysicalDeviceVulkanMemoryModelFeatures",
            # 1.3
            "VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures",
            "VkPhysicalDeviceShaderIntegerDotProductFeatures",
        ]

        # Properties are harder to handle genearted without generating a template for every property struct type
        # The simpler solution is create strings that will be printed out as static comparisons at compile time
        # The Map is used to map Vulkan property structs with the state tracker variable name
        self.propertyInfo = dict()
        self.propertyMap = {
            'VkPhysicalDeviceVulkan11Properties' : 'phys_dev_props_core11',
            'VkPhysicalDeviceVulkan12Properties' : 'phys_dev_props_core12',
        }

    #
    # Creates the value of the struct declared in RequiredSpirvInfo
    def createMapValue(self, name: str, enable: SpirvEnables, isExtension: bool) -> str:
        out = []
        if enable.version is not None:
            # Version should be VK_VERSION_x_x as defined in header but need to get as VK_API_VERSION_x_x
            version = enable.version.replace('VK_VERSION', 'VK_API_VERSION')
            out.append(f'{{{version}, nullptr, nullptr, ""}}')
        elif enable.feature is not None:
            out.append(f'{{0, &DeviceFeatures::{enable.feature}, nullptr, ""}}')
        elif enable.extension is not None:
            # All fields in DeviceExtensions should just be the extension name lowercase
            out.append(f'{{0, nullptr, &DeviceExtensions::{enable.extension.lower()}, ""}}')
        elif enable.property is not None:
            propertyStruct = enable.property
            # Need to make sure to return a boolean value to prevent compiler warning for implicit conversions
            propertyLogic = f'({propertyStruct}::{enable.member} & {enable.value}) != 0'
            # Property might have multiple items per capability/extension
            if name not in self.propertyInfo:
                self.propertyInfo[name] = []
            # Save info later to be printed out
            self.propertyInfo[name].append({
                "logic" : propertyLogic,
                "struct" : propertyStruct,
                "isExtension" : isExtension
            })
            # For properties, this string is just for human readableness
            out.append(f'{{0, nullptr, nullptr, "{propertyLogic}"}}')
        else:
            out.append('{0, nullptr, nullptr, ""}')
        return "".join(out)

    def generate(self):
        out = []
        out.append(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2020-2023 The Khronos Group Inc.
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
            *
            * This file is related to anything that is found in the Vulkan XML related
            * to SPIR-V. Anything related to the SPIR-V grammar belongs in spirv_grammar_helper
            *
            ****************************************************************************/
            ''')
        out.append('// NOLINTBEGIN') # Wrap for clang-tidy to ignore
        out.append('''
            #include <string>
            #include <string_view>
            #include <functional>
            #include <spirv/unified1/spirv.hpp>
            #include "vk_extension_helper.h"
            #include "state_tracker/shader_module.h"
            #include "state_tracker/device_state.h"
            #include "core_checks/core_validation.h"
            ''')

        #
        # Creates the FeaturePointer struct to map features with those in the layers state tracker
        out.append('''
            struct FeaturePointer {
                // Callable object to test if this feature is enabled in the given aggregate feature struct
                const std::function<bool(const DeviceFeatures &)> IsEnabled;

                // Test if feature pointer is populated
                explicit operator bool() const { return static_cast<bool>(IsEnabled); }

                // Default and nullptr constructor to create an empty FeaturePointer
                FeaturePointer() : IsEnabled(nullptr) {}
                FeaturePointer(std::nullptr_t ptr) : IsEnabled(nullptr) {}
                FeaturePointer(bool DeviceFeatures::*ptr)
                    : IsEnabled([=](const DeviceFeatures &features) { return features.*ptr; }) {}
            };
            ''')

        out.append('''
            // Each instance of the struct will only have a singel field non-null
            struct RequiredSpirvInfo {
                uint32_t version;
                FeaturePointer feature;
                ExtEnabled DeviceExtensions::*extension;
                const char* property; // For human readability and make some capabilities unique
            };

            ''')

        #
        # Build the struct with all the requirments for the spirv capabilities
        out.append('// clang-format off\n')
        out.append('static const std::unordered_multimap<uint32_t, RequiredSpirvInfo> spirvCapabilities = {\n')
        for spirv in [x for x in self.vk.spirv if x.capability]:
            for enable in [x for x in spirv.enable if x.struct is None or x.struct not in self.promotedFeatures]:
                if spirv.name in self.capabilityExcludeList:
                    out.append('    // Not found in current SPIR-V Headers\n    //')
                out.append(f'    {{spv::Capability{spirv.name}, {self.createMapValue(spirv.name, enable, False)}}},\n')
        out.append('};\n')
        out.append('// clang-format on\n')
        out.append('\n')

        #
        # Build the struct with all the requirments for the spirv extensions
        out.append('// clang-format off\n')
        out.append('static const std::unordered_multimap<std::string_view, RequiredSpirvInfo> spirvExtensions = {\n')
        for spirv in [x for x in self.vk.spirv if x.extension]:
            for enable in spirv.enable:
                out.append(f'    {{"{spirv.name}", {self.createMapValue(spirv.name, enable, True)}}},\n')
        out.append('};\n')
        out.append('// clang-format on\n')
        out.append('\n')

        #
        # Creates the Enum string helpers for better error messages. Same idea of vk_enum_string_helper.h but for SPIR-V
        out.append('static inline const char* string_SpvCapability(uint32_t input_value) {\n')
        out.append('    switch ((spv::Capability)input_value) {\n')
        for spirv in [x for x in self.vk.spirv if x.capability]:
            if (spirv.name not in self.capabilityAliasList) and (spirv.name not in self.capabilityExcludeList):
                out.append(f'         case spv::Capability{spirv.name}:\n')
                out.append(f'            return "{spirv.name}";\n')
        out.append('        default:\n')
        out.append('            return \"Unhandled OpCapability\";\n')
        out.append('    };\n')
        out.append('}\n')
        #
        # Creates SPIR-V image format helper
        out.append('''
            // Will return the Vulkan format for a given SPIR-V image format value
            // Note: will return VK_FORMAT_UNDEFINED if non valid input
            // This was in vk_format_utils but the SPIR-V Header dependency was an issue
            //   see https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/4647
            VkFormat CoreChecks::CompatibleSpirvImageFormat(uint32_t spirv_image_format) const {
                switch (spirv_image_format) {
            ''')
        for format in [x for x in self.vk.formats.values() if x.spirvImageFormat]:
            out.append(f'        case spv::ImageFormat{format.spirvImageFormat}:\n')
            out.append(f'            return {format.name};\n')
        out.append('        default:\n')
        out.append('            return VK_FORMAT_UNDEFINED;\n')
        out.append('    };\n')
        out.append('}\n')


        out.append('''
// clang-format off
static inline const char* SpvCapabilityRequirements(uint32_t capability) {
    static const vvl::unordered_map<uint32_t, std::string_view> table {
''')
        for spirv in [x for x in self.vk.spirv if x.capability and x.name not in self.capabilityExcludeList]:
            requirment = ''
            for index, enable in enumerate([x for x in spirv.enable if x.struct is None or x.struct not in self.promotedFeatures]):
                requirment += ' OR ' if (index != 0) else ''
                if enable.version is not None:
                    requirment += enable.version
                elif enable.feature is not None:
                    requirment += f'{enable.struct}::{enable.feature}'
                elif enable.extension is not None:
                    requirment += enable.extension
                elif enable.property is not None:
                    requirment += f'({enable.property}::{enable.member} == {enable.value})'
            out.append(f'    {{spv::Capability{spirv.name}, "{requirment}"}},\n')
        out.append('''    };

    // VUs before catch unknown capabilities
    const auto entry = table.find(capability);
    return entry->second.data();
}
// clang-format on
''')

        out.append('''
// clang-format off
static inline const char* SpvExtensionRequirments(std::string_view extension) {
    static const vvl::unordered_map<std::string_view, std::string_view> table {
''')
        for spirv in [x for x in self.vk.spirv if x.extension]:
            requirment = ''
            for index, enable in enumerate(spirv.enable):
                requirment += ' OR ' if (index != 0) else ''
                if enable.version is not None:
                    requirment += enable.version
                elif enable.feature is not None:
                    requirment += f'{enable.struct}::{enable.feature}'
                elif enable.extension is not None:
                    requirment += enable.extension
                elif enable.property is not None:
                    requirment += f'({enable.property}::{enable.member} == {enable.value})'
            out.append(f'    {{"{spirv.name}", "{requirment}"}},\n')
        out.append('''    };

    // VUs before catch unknown extensions
    const auto entry = table.find(extension);
    return entry->second.data();
}
// clang-format on
''')

        # The chance of an SPIR-V extension having a property as a requirement is low
        # Instead of writting complex (and more confusing) code, just go back match what
        # we do for capabilities if one is ever added to the XML
        if len([infos for infos in self.propertyInfo.values() if infos[0]['isExtension']]) > 0:
            print("Error: XML has added a property requirement to a SPIR-V Extension")
            sys.exit(1)

        #
        # The main function to validate all the extensions and capabilities
        out.append('''
            bool CoreChecks::ValidateShaderCapabilitiesAndExtensions(const Instruction &insn, const bool pipeline, const Location& loc) const {
                bool skip = false;

                if (insn.Opcode() == spv::OpCapability) {
                    // All capabilities are generated so if it is not in the list it is not supported by Vulkan
                    if (spirvCapabilities.count(insn.Word(1)) == 0) {
                        const char *vuid = pipeline ? "VUID-VkShaderModuleCreateInfo-pCode-08739" : "VUID-VkShaderCreateInfoEXT-pCode-08739";
                        skip |= LogError(vuid, device, loc,
                            "SPIR-V has Capability (%s) declared, but this is not supported by Vulkan.", string_SpvCapability(insn.Word(1)));
                        return skip; // no known capability to validate
                    }

                    // Each capability has one or more requirements to check
                    // Only one item has to be satisfied and an error only occurs
                    // when all are not satisfied
                    auto caps = spirvCapabilities.equal_range(insn.Word(1));
                    bool has_support = false;
                    for (auto it = caps.first; (it != caps.second) && (has_support == false); ++it) {
                        if (it->second.version) {
                            if (api_version >= it->second.version) {
                                has_support = true;
                            }
                        } else if (it->second.feature) {
                            if (it->second.feature.IsEnabled(enabled_features)) {
                                has_support = true;
                            }
                        } else if (it->second.extension) {
                            // kEnabledByApiLevel is not valid as some extension are promoted with feature bits to be used.
                            // If the new Api Level gives support, it will be caught in the "it->second.version" check instead.
                            if (IsExtEnabledByCreateinfo(device_extensions.*(it->second.extension))) {
                                has_support = true;
                            }
                        } else if (it->second.property) {
                            // support is or'ed as only one has to be supported (if applicable)
                            switch (insn.Word(1)) {''')

        for name, infos in sorted(self.propertyInfo.items()):
            # Only capabilities here (all items in array are the same)
            if infos[0]['isExtension']:
                continue

            # use triple-tick syntax to keep tab alignment for generated code
            out.append(f'''
                    case spv::Capability{name}:''')
            for info in infos:
                # Need to string replace property string to create valid C++ logic
                logic = info['logic'].replace('::', '.')
                logic = logic.replace(info['struct'], self.propertyMap[info['struct']])
                out.append(f'''
                        has_support |= ({logic});''')
            out.append('''
                        break;''')

        out.append('''
                        default:
                            break;
                    }
                }
            }
            ''')

        out.append('''
                if (has_support == false) {
                    const char *vuid = pipeline ? "VUID-VkShaderModuleCreateInfo-pCode-08740" : "VUID-VkShaderCreateInfoEXT-pCode-08740";
                    skip |= LogError(vuid, device, loc,
                        "SPIR-V Capability %s was declared, but one of the following requirements is required (%s).", string_SpvCapability(insn.Word(1)), SpvCapabilityRequirements(insn.Word(1)));
                }

                // Portability checks
                if (IsExtEnabled(device_extensions.vk_khr_portability_subset)) {
                    if ((VK_FALSE == enabled_features.shaderSampleRateInterpolationFunctions) &&
                        (spv::CapabilityInterpolationFunction == insn.Word(1))) {
                        skip |= LogError("VUID-RuntimeSpirv-shaderSampleRateInterpolationFunctions-06325", device, loc,
                                            "SPIR-V (portability error) InterpolationFunction Capability are not supported "
                                            "by this platform");
                    }
                }
            } else if (insn.Opcode() == spv::OpExtension) {
                static const std::string spv_prefix = "SPV_";
                std::string extension_name = insn.GetAsString(1);

                if (0 == extension_name.compare(0, spv_prefix.size(), spv_prefix)) {
                    if (spirvExtensions.count(extension_name) == 0) {
                        const char *vuid = pipeline ? "VUID-VkShaderModuleCreateInfo-pCode-08741" : "VUID-VkShaderCreateInfoEXT-pCode-08741";
                        skip |= LogError(vuid, device,loc,
                            "SPIR-V Extension %s was declared, but that is not supported by Vulkan.", extension_name.c_str());
                        return skip; // no known extension to validate
                    }
                } else {
                    const char *vuid = pipeline ? "VUID-VkShaderModuleCreateInfo-pCode-08741" : "VUID-VkShaderCreateInfoEXT-pCode-08741";
                    skip |= LogError( vuid, device,loc,
                        "SPIR-V Extension %s was declared, but this is not a SPIR-V extension. Please use a SPIR-V"
                        " extension (https://github.com/KhronosGroup/SPIRV-Registry) for OpExtension instructions. Non-SPIR-V extensions can be"
                        " recorded in SPIR-V using the OpSourceExtension instruction.", extension_name.c_str());
                    return skip; // no known extension to validate
                }

                // Each SPIR-V Extension has one or more requirements to check
                // Only one item has to be satisfied and an error only occurs
                // when all are not satisfied
                auto ext = spirvExtensions.equal_range(extension_name);
                bool has_support = false;
                for (auto it = ext.first; (it != ext.second) && (has_support == false); ++it) {
                    if (it->second.version) {
                        if (api_version >= it->second.version) {
                            has_support = true;
                        }
                    } else if (it->second.feature) {
                        if (it->second.feature.IsEnabled(enabled_features)) {
                            has_support = true;
                        }
                    } else if (it->second.extension) {
                        if (IsExtEnabled(device_extensions.*(it->second.extension))) {
                            has_support = true;
                        }
                    }
                }

                if (has_support == false) {
                    const char *vuid = pipeline ? "VUID-VkShaderModuleCreateInfo-pCode-08742" : "VUID-VkShaderCreateInfoEXT-pCode-08742";
                    skip |= LogError(vuid, device, loc,
                        "SPIR-V Extension %s was declared, but one of the following requirements is required (%s).", extension_name.c_str(), SpvExtensionRequirments(extension_name));
                }
            } //spv::OpExtension
            return skip;
            }
            ''')
        out.append('// NOLINTEND') # Wrap for clang-tidy to ignore
        self.write("".join(out))
