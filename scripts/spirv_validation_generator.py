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

import os,re,sys,string,json
import xml.etree.ElementTree as etree
from generator import *
from collections import namedtuple
from common_codegen import *

# This is a workaround to use a Python 2.7 and 3.x compatible syntax
from io import open

class SpirvValidationHelperOutputGeneratorOptions(GeneratorOptions):
    def __init__(self,
                 conventions = None,
                 filename = None,
                 directory = '.',
                 genpath = None,
                 apiname = 'vulkan',
                 profile = None,
                 versions = '.*',
                 emitversions = '.*',
                 defaultExtensions = 'vulkan',
                 addExtensions = None,
                 removeExtensions = None,
                 emitExtensions = None,
                 emitSpirv = None,
                 emitFormats = None,
                 sortProcedure = regSortFeatures,
                 genFuncPointers = True,
                 protectFile = True,
                 protectFeature = False,
                 apicall = 'VKAPI_ATTR ',
                 apientry = 'VKAPI_CALL ',
                 apientryp = 'VKAPI_PTR *',
                 indentFuncProto = True,
                 indentFuncPointer = False,
                 alignFuncParam = 48,
                 expandEnumerants = False):
        GeneratorOptions.__init__(self,
                conventions = conventions,
                filename = filename,
                directory = directory,
                genpath = genpath,
                apiname = apiname,
                profile = profile,
                versions = versions,
                emitversions = emitversions,
                defaultExtensions = defaultExtensions,
                addExtensions = addExtensions,
                removeExtensions = removeExtensions,
                emitExtensions = emitExtensions,
                emitSpirv = emitSpirv,
                emitFormats = emitFormats,
                sortProcedure = sortProcedure)
        self.genFuncPointers = genFuncPointers
        self.protectFile     = protectFile
        self.protectFeature  = protectFeature
        self.apicall         = apicall
        self.apientry        = apientry
        self.apientryp       = apientryp
        self.indentFuncProto = indentFuncProto
        self.indentFuncPointer = indentFuncPointer
        self.alignFuncParam  = alignFuncParam
        self.expandEnumerants = expandEnumerants
#
# SpirvValidationHelperOutputGenerator - Generate SPIR-V validation
# for SPIR-V extensions and capabilities
class SpirvValidationHelperOutputGenerator(OutputGenerator):
    def __init__(self,
                 errFile = sys.stderr,
                 warnFile = sys.stderr,
                 diagFile = sys.stdout):
        OutputGenerator.__init__(self, errFile, warnFile, diagFile)
        self.extensions = dict()
        self.capabilities = dict()
        self.formats = dict()

        # Sometimes the Vulkan-Headers XML will mention new SPIR-V capability or extensions
        # That require an update of the SPIRV-Headers which might not be ready to pull in.
        # These 2 arrays SHOULD be empty when possible and when the SPIR-V Headers are updated these
        # should be attempted to be cleared
        self.capabilityExcludeList = [
            'TextureBlockMatchQCOM',
            'TextureBoxFilterQCOM',
            'TextureSampleWeightedQCOM',
            'ClusterCullingShadingHUAWEI',
        ]

        # There are some enums that share the same value in the SPIR-V header.
        # This array remove the duplicate to not print out, usually due to being the older value given
        self.capabilityAliasList = [
          'ShaderViewportIndexLayerNV',
          'ShadingRateNV',
          'FragmentBarycentricNV',
        ]

        # This is a list that maps the Vulkan struct a feature field is with the internal
        # state tracker's enabled features value
        #
        # If a new SPIR-V Capability is added to uses a new feature struct, it will need to be
        # added here with the name added in 'DeviceFeatures' struct
        self.featureMap = [
          # {'vulkan' : <Vulkan Spec Feature Struct Name>, 'layer' : <Name of variable in CoreChecks DeviceFeatures>},
            {'vulkan' : 'VkPhysicalDeviceFeatures', 'layer' : 'core'},
            {'vulkan' : 'VkPhysicalDeviceVulkan11Features', 'layer' : 'core11'},
            {'vulkan' : 'VkPhysicalDeviceVulkan12Features', 'layer' : 'core12'},
            {'vulkan' : 'VkPhysicalDeviceVulkan13Features', 'layer' : 'core13'},
            {'vulkan' : 'VkPhysicalDeviceTransformFeedbackFeaturesEXT', 'layer' : 'transform_feedback_features'},
            {'vulkan' : 'VkPhysicalDeviceCooperativeMatrixFeaturesNV', 'layer' : 'cooperative_matrix_features'},
            {'vulkan' : 'VkPhysicalDeviceComputeShaderDerivativesFeaturesNV', 'layer' : 'compute_shader_derivatives_features'},
            {'vulkan' : 'VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV', 'layer' : 'fragment_shader_barycentric_features'},
            {'vulkan' : 'VkPhysicalDeviceShaderImageFootprintFeaturesNV', 'layer' : 'shader_image_footprint_features'},
            {'vulkan' : 'VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT', 'layer' : 'fragment_shader_interlock_features'},
            {'vulkan' : 'VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT', 'layer' : 'demote_to_helper_invocation_features'},
            {'vulkan' : 'VkPhysicalDeviceRayQueryFeaturesKHR', 'layer' : 'ray_query_features'},
            {'vulkan' : 'VkPhysicalDeviceRayTracingPipelineFeaturesKHR', 'layer' : 'ray_tracing_pipeline_features'},
            {'vulkan' : 'VkPhysicalDeviceAccelerationStructureFeaturesKHR', 'layer' : 'ray_tracing_acceleration_structure_features'},
            {'vulkan' : 'VkPhysicalDeviceFragmentDensityMapFeaturesEXT', 'layer' : 'fragment_density_map_features'},
            {'vulkan' : 'VkPhysicalDeviceBufferDeviceAddressFeaturesEXT', 'layer' : 'buffer_device_address_ext_features'},
            {'vulkan' : 'VkPhysicalDeviceFragmentShadingRateFeaturesKHR', 'layer' : 'fragment_shading_rate_features'},
            {'vulkan' : 'VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL', 'layer' : 'shader_integer_functions2_features'},
            {'vulkan' : 'VkPhysicalDeviceShaderSMBuiltinsFeaturesNV', 'layer' : 'shader_sm_builtins_features'},
            {'vulkan' : 'VkPhysicalDeviceShadingRateImageFeaturesNV', 'layer' : 'shading_rate_image_features'},
            {'vulkan' : 'VkPhysicalDeviceShaderAtomicFloatFeaturesEXT', 'layer' : 'shader_atomic_float_features'},
            {'vulkan' : 'VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT', 'layer' : 'shader_image_atomic_int64_features'},
            {'vulkan' : 'VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR', 'layer' : 'workgroup_memory_explicit_layout_features'},
            {'vulkan' : 'VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT', 'layer' : 'shader_atomic_float2_features'},
            {'vulkan' : 'VkPhysicalDeviceRayTracingMotionBlurFeaturesNV', 'layer' : 'ray_tracing_motion_blur_features'},
            {'vulkan' : 'VkPhysicalDeviceShaderIntegerDotProductFeaturesKHR', 'layer' : 'shader_integer_dot_product_features'},
            {'vulkan' : 'VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR', 'layer' : 'shader_subgroup_uniform_control_flow_features'},
            {'vulkan' : 'VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR', 'layer' : 'ray_tracing_maintenance1_features'},
            {'vulkan' : 'VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM', 'layer' : 'shader_core_builtins_features'},
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
    # Called at beginning of processing as file is opened
    def beginFile(self, genOpts):
        OutputGenerator.beginFile(self, genOpts)
        # File Comment
        file_comment = '// *** THIS FILE IS GENERATED - DO NOT EDIT ***\n'
        file_comment += '// See spirv_validation_generator.py for modifications\n'
        write(file_comment, file=self.outFile)
        # Copyright Statement
        copyright = ''
        copyright += '\n'
        copyright += '/***************************************************************************\n'
        copyright += ' *\n'
        copyright += ' * Copyright (c) 2020-2023 The Khronos Group Inc.\n'
        copyright += ' *\n'
        copyright += ' * Licensed under the Apache License, Version 2.0 (the "License");\n'
        copyright += ' * you may not use this file except in compliance with the License.\n'
        copyright += ' * You may obtain a copy of the License at\n'
        copyright += ' *\n'
        copyright += ' *     http://www.apache.org/licenses/LICENSE-2.0\n'
        copyright += ' *\n'
        copyright += ' * Unless required by applicable law or agreed to in writing, software\n'
        copyright += ' * distributed under the License is distributed on an "AS IS" BASIS,\n'
        copyright += ' * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n'
        copyright += ' * See the License for the specific language governing permissions and\n'
        copyright += ' * limitations under the License.\n'
        copyright += ' *\n'
        copyright += ' * This file is related to anything that is found in the Vulkan XML related\n'
        copyright += ' * to SPIR-V. Anything related to the SPIR-V grammar belongs in spirv_grammar_helper\n'
        copyright += ' *\n'
        copyright += ' ****************************************************************************/\n'
        write(copyright, file=self.outFile)
        write('#include <string>', file=self.outFile)
        write('#include <functional>', file=self.outFile)
        write('#include <spirv/unified1/spirv.hpp>', file=self.outFile)
        write('#include "vk_extension_helper.h"', file=self.outFile)
        write('#include "state_tracker/shader_module.h"', file=self.outFile)
        write('#include "state_tracker/device_state.h"', file=self.outFile)
        write('#include "core_checks/core_validation.h"', file=self.outFile)
        write(self.featurePointer(), file=self.outFile)
        write(self.mapStructDeclarations(), file=self.outFile)
    #
    # Write generated file content to output file
    def endFile(self):
        write(self.capabilityStruct(), file=self.outFile)
        write(self.extensionStruct(), file=self.outFile)
        write(self.enumHelper(), file=self.outFile)
        write(self.validateFunction(), file=self.outFile)
        write(self.formatHelper(), file=self.outFile)
        # Finish processing in superclass
        OutputGenerator.endFile(self)
    #
    # Processing point at beginning of each extension definition
    def beginFeature(self, interface, emit):
        OutputGenerator.beginFeature(self, interface, emit)
        self.featureExtraProtect = GetFeatureProtect(interface)
    #
    # Capture all SPIR-V elements from registry
    def genSpirv(self, spirvinfo, spirvName, alias):
        OutputGenerator.genSpirv(self, spirvinfo, spirvName, alias)
        spirvElem = spirvinfo.elem
        name = spirvElem.get('name')
        enables = []
        for elem in spirvElem:
            if elem.tag != 'enable':
                self.logMsg('error', 'should only be <enable> tags in ' + name)
            # Each <enable> holds only one possible requirment
            # This internal python dict is to represent the struct generated later
            enable = {
                'version' : None,
                'feature' : None,
                'extension' : None,
                'property' : None,
            }
            if 'version' in elem.attrib:
                enable['version'] = elem.attrib['version']
            elif 'feature' in elem.attrib:
                if elem.attrib['struct'] in self.promotedFeatures:
                    continue
                enable['feature'] = {
                    'feature' : elem.attrib['feature'],
                    'struct' : elem.attrib['struct']
                }
            elif 'extension' in elem.attrib:
                enable['extension'] = elem.attrib['extension']
            elif 'property' in elem.attrib:
                enable['property'] =  {
                    'property' : elem.attrib['property'],
                    'member' : elem.attrib['member'],
                    'value' : elem.attrib['value']
                }
            else:
                self.logMsg('error', 'No known attributes in <enable> for ' + name)
            enables.append(enable)
        if spirvElem.tag == 'spirvcapability':
            self.capabilities[name] = enables
        elif spirvElem.tag == 'spirvextension':
            self.extensions[name] = enables
    #
    # Capture all Format elements from registry
    def genFormat(self, format, formatinfo, alias):
        OutputGenerator.genFormat(self, format, formatinfo, alias)
        elem = format.elem
        formatName = elem.get('name')

        spirvImageFormat = elem.find('spirvimageformat')
        if (spirvImageFormat is not None):
            self.formats[formatName] = spirvImageFormat.get('name')
    #
    # Creates SPIR-V image format helper
    def formatHelper(self):
        output = '\n// Will return the Vulkan format for a given SPIR-V image format value\n'
        output += '// Note: will return VK_FORMAT_UNDEFINED if non valid input\n'
        output += '// This was in vk_format_utils but the SPIR-V Header dependency was an issue\n'
        output += '//   see https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/4647\n'
        output += 'VkFormat CoreChecks::CompatibleSpirvImageFormat(uint32_t spirv_image_format) const {\n'
        output += '    switch (spirv_image_format) {\n'
        for f, spirvFormat in sorted(self.formats.items()):
            output += '        case spv::ImageFormat{}:\n'.format(spirvFormat)
            output += '            return {};\n'.format(f)
        output += '        default:\n'
        output += '            return VK_FORMAT_UNDEFINED;\n'
        output += '     }\n'
        output += '}'
        return output
    #
    # Creates the Enum string helpers for better error messages. Same idea of vk_enum_string_helper.h but for SPIR-V
    def enumHelper(self):
        output =  'static inline const char* string_SpvCapability(uint32_t input_value) {\n'
        output += '    switch ((spv::Capability)input_value) {\n'
        for name, enables in sorted(self.capabilities.items()):
            if (name not in self.capabilityAliasList) and (name not in self.capabilityExcludeList):
                output += '         case spv::Capability' + name + ':\n'
                output += '            return \"' + name + '\";\n'
        output += '        default:\n'
        output += '            return \"Unhandled OpCapability\";\n'
        output += '    };\n'
        output += '};'
        return output
    #
    # Creates the FeaturePointer struct to map features with those in the layers state tracker
    def featurePointer(self):
        output = '\n'
        output += 'struct FeaturePointer {\n'
        output += '    // Callable object to test if this feature is enabled in the given aggregate feature struct\n'
        output += '    const std::function<VkBool32(const DeviceFeatures &)> IsEnabled;\n'
        output += '\n'
        output += '    // Test if feature pointer is populated\n'
        output += '    explicit operator bool() const { return static_cast<bool>(IsEnabled); }\n'
        output += '\n'
        output += '    // Default and nullptr constructor to create an empty FeaturePointer\n'
        output += '    FeaturePointer() : IsEnabled(nullptr) {}\n'
        output += '    FeaturePointer(std::nullptr_t ptr) : IsEnabled(nullptr) {}\n'
        output += '\n'
        output += '    // Constructors to populate FeaturePointer based on given pointer to member\n'
        for feature in self.featureMap:
            output += '    FeaturePointer(VkBool32 ' + feature['vulkan'] + '::*ptr)\n'
            output += '        : IsEnabled([=](const DeviceFeatures &features) { return features.' + feature['layer'] + '.*ptr; }) {}\n'
        output += '};\n'
        return output
    #
    # Declare the struct that contains requirement for the spirv info
    def mapStructDeclarations(self):
        output = '// Each instance of the struct will only have a singel field non-null\n'
        output += 'struct RequiredSpirvInfo {\n'
        output += '    uint32_t version;\n'
        output += '    FeaturePointer feature;\n'
        output += '    ExtEnabled DeviceExtensions::*extension;\n'
        output += '    const char* property; // For human readability and make some capabilities unique\n'
        output += '};\n'
        return output
    #
    # Creates the value of the struct declared in mapStructDeclarations()
    def createMapValue(self, name, enable, isExtension):
        output = ''
        if enable['version'] != None:
            # Version should be VK_VERSION_x_x as defined in header but need to get as VK_API_VERSION_x_x
            version = enable['version'].replace('VK_VERSION', 'VK_API_VERSION')
            output = '{' + version + ', nullptr, nullptr, ""}'
        elif enable['feature'] != None:
            output = '{0, &' + enable['feature']['struct'] + '::'  + enable['feature']['feature'] + ', nullptr, ""}'
        elif enable['extension'] != None:
            # All fields in DeviceExtensions should just be the extension name lowercase
            output = '{0, nullptr, &DeviceExtensions::' + enable['extension'].lower() + ', ""}'
        elif enable['property'] != None:
            propertyStruct = enable['property']['property']
            # Need to make sure to return a boolean value to prevent compiler warning for implicit conversions
            propertyLogic = "(" + propertyStruct + '::' + enable['property']['member'] + ' & ' + enable['property']['value'] + ") != 0"
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
            output = '{0, nullptr, nullptr, "' + propertyLogic + '"}'
        else:
            output = '{0, nullptr, nullptr, ""}'
        return output
    #
    # Build the struct with all the requirments for the spirv capabilities
    def capabilityStruct(self):
        output = '// clang-format off\n'
        output += 'static const std::unordered_multimap<uint32_t, RequiredSpirvInfo> spirvCapabilities = {\n'

        # Sort so the order is the same on Windows and Unix
        for name, enables in sorted(self.capabilities.items()):
            for enable in enables:
                # Prepend with comment and comment out line if in exclude list as explained in declaration
                if name in self.capabilityExcludeList:
                    output += '    // Not found in current SPIR-V Headers\n    //'
                output += '    {spv::Capability' + name + ', ' + self.createMapValue(name, enable, False) + '},\n'
        output += '};\n'
        output += '// clang-format on\n'
        return output
    #
    # Build the struct with all the requirments for the spirv extensions
    def extensionStruct(self):
        output = '// clang-format off\n'
        output += 'static const std::unordered_multimap<std::string, RequiredSpirvInfo> spirvExtensions = {\n'

        # Sort so the order is the same on Windows and Unix
        for name, enables in sorted(self.extensions.items()):
            for enable in enables:
                output += '    {\"' + name + '\", ' + self.createMapValue(name, enable, True) + '},\n'
        output += '};\n'
        output += '// clang-format on\n'
        return output
    #
    # The main function to validate all the extensions and capabilities
    def validateFunction(self):
        output = '''
bool CoreChecks::ValidateShaderCapabilitiesAndExtensions(const Instruction &insn) const {
    bool skip = false;

    if (insn.Opcode() == spv::OpCapability) {
        // All capabilities are generated so if it is not in the list it is not supported by Vulkan
        if (spirvCapabilities.count(insn.Word(1)) == 0) {
            skip |= LogError(device, "VUID-VkShaderModuleCreateInfo-pCode-01090",
                "vkCreateShaderModule(): A SPIR-V Capability (%s) was declared that is not supported by Vulkan.", string_SpvCapability(insn.Word(1)));
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
                switch (insn.Word(1)) {'''

        for name, infos in sorted(self.propertyInfo.items()):
            # Only capabilities here (all items in array are the same)
            if infos[0]['isExtension'] == True:
                continue

            # use triple-tick syntax to keep tab alignment for generated code
            output += '''
                    case spv::Capability{}:'''.format(name)
            for info in infos:
                # Need to string replace property string to create valid C++ logic
                logic = info['logic'].replace('::', '.')
                logic = logic.replace(info['struct'], self.propertyMap[info['struct']])
                output += '''
                        has_support |= ({});'''.format(logic)
            output += '''
                        break;'''

        output += '''
                    default:
                        break;
                }
            }
        }

        if (has_support == false) {
            skip |= LogError(device, "VUID-VkShaderModuleCreateInfo-pCode-01091",
                "vkCreateShaderModule(): The SPIR-V Capability (%s) was declared, but none of the requirements were met to use it.", string_SpvCapability(insn.Word(1)));
        }

        // Portability checks
        if (IsExtEnabled(device_extensions.vk_khr_portability_subset)) {
            if ((VK_FALSE == enabled_features.portability_subset_features.shaderSampleRateInterpolationFunctions) &&
                (spv::CapabilityInterpolationFunction == insn.Word(1))) {
                skip |= LogError(device, "VUID-RuntimeSpirv-shaderSampleRateInterpolationFunctions-06325",
                                    "Invalid shader capability (portability error): interpolation functions are not supported "
                                    "by this platform");
            }
        }
    } else if (insn.Opcode() == spv::OpExtension) {
        static const std::string spv_prefix = "SPV_";
        std::string extension_name = insn.GetAsString(1);

        if (0 == extension_name.compare(0, spv_prefix.size(), spv_prefix)) {
            if (spirvExtensions.count(extension_name) == 0) {
                skip |= LogError(device, "VUID-VkShaderModuleCreateInfo-pCode-04146",
                    "vkCreateShaderModule(): A SPIR-V Extension (%s) was declared that is not supported by Vulkan.", extension_name.c_str());
                return skip; // no known extension to validate
            }
        } else {
            skip |= LogError(device, "VUID-VkShaderModuleCreateInfo-pCode-04146",
                "vkCreateShaderModule(): The SPIR-V code uses the '%s' extension which is not a SPIR-V extension. Please use a SPIR-V"
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
            } else if (it->second.property) {
                // support is or'ed as only one has to be supported (if applicable)
                switch (insn.Word(1)) {'''

        for name, infos in sorted(self.propertyInfo.items()):
            # Only extensions here (all items in array are the same)
            if infos[0]['isExtension'] == False:
                continue

            # use triple-tick syntax to keep tab alignment for generated code
            output += '''
                    case spv::Capability{}:'''.format(name)
            for info in infos:
                # Need to string replace property string to create valid C++ logic
                logic = info['logic'].replace('::', '.')
                logic = logic.replace(info['struct'], self.propertyMap[info['struct']])
                output += '''
                    has_support |= ({});'''.format(logic)
            output += '''
                    break;'''

        output += '''
                    default:
                        break;
                }
            }
        }

        if (has_support == false) {
            skip |= LogError(device, "VUID-VkShaderModuleCreateInfo-pCode-04147",
                "vkCreateShaderModule(): The SPIR-V Extension (%s) was declared, but none of the requirements were met to use it.", extension_name.c_str());
        }
    } //spv::OpExtension
    return skip;
}'''
        return output
