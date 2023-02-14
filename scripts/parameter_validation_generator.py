#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2023 The Khronos Group Inc.
# Copyright (c) 2015-2023 Valve Corporation
# Copyright (c) 2015-2023 LunarG, Inc.
# Copyright (c) 2015-2023 Google Inc.
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

# Helper for iterating over a list where each element is possibly a single element or another 1-dimensional list
# Generates (setter, deleter, element) for each element where:
#  - element = the next element in the list
#  - setter(x) = a function that will set the entry in `lines` corresponding to `element` to `x`
#  - deleter() = a function that will delete the entry corresponding to `element` in `lines`
def multi_string_iter(lines):
    for i, ul in enumerate(lines):
        if not isinstance(ul, list):
            def setter(x): lines[i] = x
            def deleter(): del(lines[i])
            yield (setter, deleter, ul)
        else:
            for j, l in enumerate(lines[i]):
                def setter(x): lines[i][j] = x
                def deleter(): del(lines[i][j])
                yield (setter, deleter, l)

# ParameterValidationGeneratorOptions - subclass of GeneratorOptions.
#
# Adds options used by ParameterValidationOutputGenerator object during Parameter validation layer generation.
#
# Additional members
#   protectFile - True if multiple inclusion protection should be
#     generated (based on the filename) around the entire header.
#   protectFeature - True if #ifndef..#endif protection should be
#     generated around a feature interface in the header file.
#   genFuncPointers - True if function pointer typedefs should be
#     generated
#   protectProto - If conditional protection should be generated
#     around prototype declarations, set to either '#ifdef'
#     to require opt-in (#ifdef protectProtoStr) or '#ifndef'
#     to require opt-out (#ifndef protectProtoStr). Otherwise
#     set to None.
#   protectProtoStr - #ifdef/#ifndef symbol to use around prototype
#     declarations, if protectProto is set
#   apicall - string to use for the function declaration prefix,
#     such as APICALL on Windows.
#   apientry - string to use for the calling convention macro,
#     in typedefs, such as APIENTRY.
#   apientryp - string to use for the calling convention macro
#     in function pointer typedefs, such as APIENTRYP.
#   indentFuncProto - True if prototype declarations should put each
#     parameter on a separate line
#   indentFuncPointer - True if typedefed function pointers should put each
#     parameter on a separate line
#   alignFuncParam - if nonzero and parameters are being put on a
#     separate line, align parameter names at the specified column
class ParameterValidationGeneratorOptions(GeneratorOptions):
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
                 sortProcedure = regSortFeatures,
                 apicall = 'VKAPI_ATTR ',
                 apientry = 'VKAPI_CALL ',
                 apientryp = 'VKAPI_PTR *',
                 indentFuncProto = True,
                 indentFuncPointer = False,
                 alignFuncParam = 48,
                 expandEnumerants = False,
                 valid_usage_path = ''):
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
                sortProcedure = sortProcedure)
        self.apicall         = apicall
        self.apientry        = apientry
        self.apientryp       = apientryp
        self.indentFuncProto = indentFuncProto
        self.indentFuncPointer = indentFuncPointer
        self.alignFuncParam  = alignFuncParam
        self.expandEnumerants = expandEnumerants
        self.valid_usage_path = valid_usage_path

# ParameterValidationOutputGenerator - subclass of OutputGenerator.
# Generates param checker layer code.
#
# ---- methods ----
# ParamCheckerOutputGenerator(errFile, warnFile, diagFile) - args as for
#   OutputGenerator. Defines additional internal state.
# ---- methods overriding base class ----
# beginFile(genOpts)
# endFile()
# beginFeature(interface, emit)
# endFeature()
# genType(typeinfo,name)
# genStruct(typeinfo,name)
# genGroup(groupinfo,name)
# genEnum(enuminfo, name)
# genCmd(cmdinfo)
class ParameterValidationOutputGenerator(OutputGenerator):
    """Generate Parameter Validation code based on XML element attributes"""
    # This is an ordered list of sections in the header file.
    ALL_SECTIONS = ['command']

    # ValidationObject (i.e., not just StatelessValidation) needs access to these values.
    # Future "script refactoring efforts" should try to move common code outside of this file so it can better be consumed by
    # the chassis and parameter validation generation scripts.
    VALID_PARAM_VALUES_PATH = 'valid_param_values.h'

    def __init__(self,
                 errFile = sys.stderr,
                 warnFile = sys.stderr,
                 diagFile = sys.stdout):
        OutputGenerator.__init__(self, errFile, warnFile, diagFile)
        self.INDENT_SPACES = 4
        self.declarations = []
        self.specializations = []

        # Template specialization declarations, which must appear at namespace scope (i.e., not inside a class declaration)
        self.specFilePath = ParameterValidationOutputGenerator.VALID_PARAM_VALUES_PATH
        self.categoryFilePath = 'valid_param_values.cpp'

        inline_custom_source_preamble = """
"""

        # These functions have additional, custom-written checks in the utils cpp file. CodeGen will automatically add a call
        # to those functions of the form 'bool manual_PreCallValidateAPIName', where the 'vk' is dropped.
        # see 'manual_PreCallValidateCreateGraphicsPipelines' as an example.
        self.functions_with_manual_checks = [
            'vkCreateInstance',
            'vkCreateDevice',
            'vkCreateQueryPool',
            'vkCreateRenderPass',
            'vkCreateRenderPass2',
            'vkCreateRenderPass2KHR',
            'vkCreateBuffer',
            'vkCreateImage',
            'vkCreatePipelineLayout',
            'vkCreateGraphicsPipelines',
            'vkCreateComputePipelines',
            'vkCreateRayTracingPipelinesNV',
            'vkCreateRayTracingPipelinesKHR',
            'vkCreateSampler',
            'vkCreateDescriptorSetLayout',
            'vkCreateBufferView',
            'vkCreateSemaphore',
            'vkCreateEvent',
            'vkFreeDescriptorSets',
            'vkUpdateDescriptorSets',
            'vkBeginCommandBuffer',
            'vkCmdSetViewport',
            'vkCmdSetScissor',
            'vkCmdSetLineWidth',
            'vkCmdDrawIndirect',
            'vkCmdDrawIndexedIndirect',
            'vkCmdDrawMultiEXT',
            'vkCmdDrawMultiIndexedEXT',
            'vkCmdClearAttachments',
            'vkCmdBindIndexBuffer',
            'vkCmdCopyBuffer',
            'vkCmdUpdateBuffer',
            'vkCmdFillBuffer',
            'vkCreateSwapchainKHR',
            'vkCreateSharedSwapchainsKHR',
            'vkQueuePresentKHR',
            'vkCreateDescriptorPool',
            'vkCmdDispatch',
            'vkCmdDispatchIndirect',
            'vkCmdDispatchBaseKHR',
            'vkCmdPushDescriptorSetKHR',
            'vkCmdSetExclusiveScissorNV',
            'vkCmdSetViewportShadingRatePaletteNV',
            'vkCmdSetCoarseSampleOrderNV',
            'vkCmdDrawMeshTasksNV',
            'vkCmdDrawMeshTasksIndirectNV',
            'vkCmdDrawMeshTasksIndirectCountNV',
            'vkCmdDrawMeshTasksEXT',
            'vkCmdDrawMeshTasksIndirectEXT',
            'vkAllocateMemory',
            'vkCreateAccelerationStructureNV',
            'vkCreateAccelerationStructureKHR',
            'vkGetAccelerationStructureHandleNV',
            'vkGetPhysicalDeviceImageFormatProperties',
            'vkGetPhysicalDeviceImageFormatProperties2',
            'vkGetPhysicalDeviceImageFormatProperties2KHR',
            'vkCmdBuildAccelerationStructureNV',
            'vkCreateFramebuffer',
            'vkCmdSetLineStippleEXT',
            'vkSetDebugUtilsObjectNameEXT',
            'vkSetDebugUtilsObjectTagEXT',
            'vkCmdSetViewportWScalingNV',
            'vkAcquireNextImageKHR',
            'vkAcquireNextImage2KHR',
            'vkCmdBindTransformFeedbackBuffersEXT',
            'vkCmdBeginTransformFeedbackEXT',
            'vkCmdEndTransformFeedbackEXT',
            'vkCmdDrawIndirectByteCountEXT',
            'vkCreateSamplerYcbcrConversion',
            'vkCreateSamplerYcbcrConversionKHR',
            'vkImportSemaphoreFdKHR',
            'vkGetSemaphoreFdKHR',
            'vkImportFenceFdKHR',
            'vkGetFenceFdKHR',
            'vkImportFenceWin32HandleKHR',
            'vkGetFenceWin32HandleKHR',
            'vkImportSemaphoreWin32HandleKHR',
            'vkGetSemaphoreWin32HandleKHR',
            'vkCmdBindVertexBuffers',
            'vkCreateImageView',
            'vkCopyAccelerationStructureToMemoryKHR',
            'vkCmdCopyAccelerationStructureToMemoryKHR',
            'vkCopyAccelerationStructureKHR',
            'vkCmdCopyAccelerationStructureKHR',
            'vkCopyMemoryToAccelerationStructureKHR',
            'vkCmdCopyMemoryToAccelerationStructureKHR',
            'vkCmdDrawIndirectCount',
            'vkCmdDrawIndirectCountAMD',
            'vkCmdDrawIndirectCountKHR',
            'vkCmdDrawIndexedIndirectCount',
            'vkCmdDrawIndexedIndirectCountAMD',
            'vkCmdDrawIndexedIndirectCountKHR',
            'vkCmdWriteAccelerationStructuresPropertiesKHR',
            'vkWriteAccelerationStructuresPropertiesKHR',
            'vkGetRayTracingCaptureReplayShaderGroupHandlesKHR',
            'vkCmdTraceRaysKHR',
            'vkCmdTraceRaysNV',
            'vkCmdTraceRaysIndirectKHR',
            'vkCmdTraceRaysIndirect2KHR',
            'vkCmdBuildAccelerationStructureIndirectKHR',
            'vkGetDeviceAccelerationStructureCompatibilityKHR',
            'vkCmdSetViewportWithCountEXT',
            'vkCmdSetViewportWithCount',
            'vkCmdSetScissorWithCountEXT',
            'vkCmdSetScissorWithCount',
            'vkCmdBindVertexBuffers2EXT',
            'vkCmdBindVertexBuffers2',
            'vkCmdCopyBuffer2KHR',
            'vkCmdCopyBuffer2',
            'vkCmdBuildAccelerationStructuresKHR',
            'vkCmdBuildAccelerationStructuresIndirectKHR',
            'vkBuildAccelerationStructuresKHR',
            'vkGetAccelerationStructureBuildSizesKHR',
            'vkCmdWriteAccelerationStructuresPropertiesNV',
            'vkCreateDisplayModeKHR',
            'vkCmdSetVertexInputEXT',
            'vkCmdPushConstants',
            'vkMergePipelineCaches',
            'vkCmdClearColorImage',
            'vkCmdBeginRenderPass',
            'vkCmdBeginRenderPass2KHR',
            'vkCmdBeginRenderPass2',
            'vkCmdBeginRendering',
            'vkCmdBeginRenderingKHR',
            'vkCmdSetDiscardRectangleEXT',
            'vkGetQueryPoolResults',
            'vkCmdBeginConditionalRenderingEXT',
            'vkGetDeviceImageMemoryRequirementsKHR',
            'vkGetDeviceImageSparseMemoryRequirementsKHR',
            'vkCreateWin32SurfaceKHR',
            'vkCreateWaylandSurfaceKHR',
            'vkGetPhysicalDeviceSurfaceFormatsKHR',
            'vkGetPhysicalDeviceSurfacePresentModesKHR',
            'vkGetPhysicalDeviceSurfaceCapabilities2KHR',
            'vkGetPhysicalDeviceSurfaceFormats2KHR',
            'vkGetPhysicalDeviceSurfacePresentModes2EXT',
            'vkExportMetalObjectsEXT',
            ]

        # Commands to ignore
        self.blacklist = [
            'vkGetInstanceProcAddr',
            'vkGetDeviceProcAddr',
            'vkEnumerateInstanceVersion',
            'vkEnumerateInstanceLayerProperties',
            'vkEnumerateInstanceExtensionProperties',
            'vkEnumerateDeviceLayerProperties',
            'vkEnumerateDeviceExtensionProperties',
            'vkGetDeviceGroupSurfacePresentModes2EXT'
            ]

        # Structure fields to ignore
        self.structMemberBlacklist = { 'VkWriteDescriptorSet' : ['dstSet'], 'VkAccelerationStructureGeometryKHR' :['geometry'], 'VkDescriptorDataEXT' :['pSampler'] }
        # Validation conditions for some special case struct members that are conditionally validated
        self.structMemberValidationConditions = { 'VkPipelineColorBlendStateCreateInfo' : { 'logicOp' : '{}logicOpEnable == VK_TRUE' } }
        # FlagBits that should also be array
        self.flagBitsAsArray = ['VkShaderStageFlags']
        # Header version
        self.headerVersion = None
        # Internal state - accumulators for different inner block text
        self.validation = []                              # Text comprising the main per-api parameter validation routines
        self.stypes = []                                  # Values from the VkStructureType enumeration
        self.structTypes = dict()                         # Map of Vulkan struct typename to required VkStructureType
        self.handleTypes = set()                          # Set of handle type names
        self.commands = []                                # List of CommandData records for all Vulkan commands
        self.structMembers = []                           # List of StructMemberData records for all Vulkan structs
        self.validatedStructs = dict()                    # Map of structs type names to generated validation code for that struct type
        self.enumRanges = set()                           # Set of enum names
        self.enum_values_definitions = dict()             # [enum, string] containing enumerated type map definitions
        self.flag_values_definitions = dict()             # [flag, string] containing flag type map definitions
        self.flag_array_values_definitions = dict()       # [flag, string] containing flag type map definitions to be used as an array
        self.stype_version_dict = dict()                  # String containing structtype to version map data
        self.flags = set()                                # Map of flags typenames
        self.flagBits = dict()                            # Map of flag bits typename to list of values
        self.newFlags = set()                             # Map of flags typenames /defined in the current feature/
        self.required_extensions = dict()                 # Dictionary of required extensions for each item in the current extension
        self.extension_type = ''                          # Type of active feature (extension), device or instance
        self.extension_names = dict()                     # Dictionary of extension names to extension name defines
        self.structextends_list = []                      # List of extensions which extend another struct
        self.struct_feature_protect = dict()              # Dictionary of structnames and FeatureExtraProtect strings
        self.valid_vuids = set()                          # Set of all valid VUIDs
        self.vuid_dict = dict()                           # VUID dictionary (from JSON)
        self.alias_dict = dict()                          # Dict of cmd|struct aliases
        self.header_file = False                          # Header file generation flag
        self.source_file = False                          # Source file generation flag
        self.enum_file = False                            # Enum helper header file generation flag
        self.instance_extension_list = ''                 # List of instance extension name defines
        self.device_extension_list = ''                   # List of device extension name defines
        self.returnedonly_structs = []                    # List of structs with 'returnonly' attribute
        self.called_types = set()                         # Set of types called via function/struct - not in list == app never passes in to validate
        # Named tuples to store struct and command data
        self.CommandParam = namedtuple('CommandParam', ['type', 'name', 'ispointer', 'isstaticarray', 'isbool', 'israngedenum',
                                                        'isconst', 'isoptional', 'iscount', 'noautovalidity',
                                                        'len', 'extstructs', 'condition', 'cdecl'])
        self.CommandData = namedtuple('CommandData', ['name', 'params', 'cdecl', 'extension_type', 'result', 'promotion_info'])
        self.StructMemberData = namedtuple('StructMemberData', ['name', 'members'])
        self.extension_number_map = dict()                # Mapping from extnumber -> extension element
        self.extension_enums = dict()                     # Mapping from enum -> extension name

    #
    # Generate Copyright comment block for file
    def GenerateCopyright(self, start_year = '2015'):
        from datetime import datetime
        curr_year = datetime.now().year
        year = f'{start_year}-{curr_year}' if start_year is not None else curr_year
        return f'''/* *** THIS FILE IS GENERATED - DO NOT EDIT! ***
 * See parameter_validation_generator.py for modifications
 *
 * Copyright (c) {year} The Khronos Group Inc.
 * Copyright (c) {year} LunarG, Inc.
 * Copyright (C) {year} Google Inc.
 * Copyright (c) {year} Valve Corporation
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
 */\n\n'''
    #
    # Increases the global indent variable
    def incIndent(self, indent):
        inc = ' ' * self.INDENT_SPACES
        if indent:
            return indent + inc
        return inc
    #
    # Decreases the global indent variable
    def decIndent(self, indent):
        if indent and (len(indent) > self.INDENT_SPACES):
            return indent[:-self.INDENT_SPACES]
        return ''
    #
    # Walk the JSON-derived dict and find all "vuid" key values
    def ExtractVUIDs(self, d):
        if hasattr(d, 'items'):
            for k, v in d.items():
                if k == "vuid":
                    yield v
                elif isinstance(v, dict):
                    for s in self.ExtractVUIDs(v):
                        yield s
                elif isinstance (v, list):
                    for l in v:
                        for s in self.ExtractVUIDs(l):
                            yield s
    #
    # Called at file creation time
    def beginFile(self, genOpts):
        OutputGenerator.beginFile(self, genOpts)
        self.header_file = (genOpts.filename == 'parameter_validation.h')
        self.source_file = (genOpts.filename == 'parameter_validation.cpp')
        self.enum_file = (genOpts.filename == 'enum_flag_bits.h')

        if not self.header_file and not self.source_file and not self.enum_file:
            print("Error: Output Filenames have changed, update generator source.\n")
            sys.exit(1)

        if self.source_file or self.header_file or self.enum_file:
            # Output Copyright text
            s = self.GenerateCopyright()
            write(s, file=self.outFile)

        if self.header_file or self.enum_file:
            write('#pragma once\n', file=self.outFile)

        if self.enum_file:
            write('#include <array>', file=self.outFile)
            write('#include "vulkan/vulkan.h"\n', file=self.outFile)

        if not self.source_file:
            return

        stype_map = ''
        stype_version_dict = dict()
        # Create contents of Structs->API version unordered map
        root = self.registry.reg
        for node in root.findall('feature'):
            version_name = node.get('name')
            version_name = version_name.replace('VK_', 'VK_API_')
            for enum_item in node.iter('enum'):
                if enum_item.get('extends') == "VkStructureType":
                    struct_type_id = enum_item.get('name')
                    # This captures all sType in core (or promoted)
                    self.stype_version_dict[struct_type_id] = version_name
        for extensions in root.findall('extensions'):
            for extension in extensions.findall('extension'):
                extensionName = extension.get('name')
                promotedTo = extension.get('promotedto')
                # TODO Issue 5103 - this is being used to remove false positive currently
                promotedToCore = promotedTo is not None and 'VK_VERSION' in promotedTo
                self.extension_number_map[extension.get('number')] = extension
                for enum in extension.findall('*/enum'):
                    enum_name = enum.get('name')
                    alias = enum.get('alias')
                    if enum_name not in self.extension_enums:
                        self.extension_enums[enum_name] = set()
                    self.extension_enums[enum_name].add(extensionName)
                    if alias is not None:
                        if alias not in self.extension_enums:
                            self.extension_enums[alias] = set()
                        self.extension_enums[alias].add(extensionName)

                for entry in extension.iterfind('require/enum[@extends="VkStructureType"]'):
                    if (entry.get('comment') is None or 'typo' not in entry.get('comment')):
                        alias = entry.get('alias')
                        if (alias is not None and promotedToCore):
                            self.stype_version_dict[alias] = extensionName

        # Build map of structure type names to VkStructureType enum values
        # Find all types of category "struct"
        for struct in self.registry.tree.iterfind('types/type[@category="struct"]'):
            # Check if struct has member named "sType" of type "VkStructureType" which has values defined
            stype = struct.find('member[name="sType"][type="VkStructureType"][@values]')
            if stype is not None:
                # Store VkStructureType value for this type
                self.structTypes[struct.get('name')] = stype.get('values')

        self.valid_usage_path = genOpts.valid_usage_path
        vu_json_filename = os.path.join(self.valid_usage_path + os.sep, 'validusage.json')
        if os.path.isfile(vu_json_filename):
            json_file = open(vu_json_filename, 'r', encoding='utf-8')
            self.vuid_dict = json.load(json_file)
            json_file.close()
        if len(self.vuid_dict) == 0:
            print("Error: Could not find, or error loading %s/validusage.json\n", vu_json_filename)
            sys.exit(1)
        #
        # Build a set of all vuid text strings found in validusage.json
        for json_vuid_string in self.ExtractVUIDs(self.vuid_dict):
            self.valid_vuids.add(json_vuid_string)
        #
        # Headers
        write('#include "chassis.h"', file=self.outFile)
        self.newline()
        write('#include "stateless/stateless_validation.h"', file=self.outFile)
        write('#include "enum_flag_bits.h"', file=self.outFile)
        self.newline()
    #
    # Called at end-time for final content output
    def endFile(self):
        if self.enum_file:
            # Write the declaration for the HeaderVersion
            if self.headerVersion:
                write('const uint32_t GeneratedVulkanHeaderVersion = {};'.format(self.headerVersion), file=self.outFile)

            # Don't need flag/enum lists if app can never call it to be validated
            # But need to save everything as not all information is known until endFile()
            for flag, string in self.flag_values_definitions.items():
                if flag == 'VkGeometryInstanceFlagsKHR':
                    # only called in VkAccelerationStructureInstanceKHR which is never called anywhere explicitly
                    continue
                flagBits = flag.replace('Flags', 'FlagBits')
                if flag in self.called_types or flagBits in self.called_types:
                    write(string, file=self.outFile)

            for flag, string in self.flag_array_values_definitions.items():
                # These are custom selected flags, so will always write
                write(string, file=self.outFile)

        elif self.source_file:
            api_func  = 'bool StatelessValidation::CheckPromotedApiAgainstVulkanVersion(VkInstance instance, const char *api_name, const uint32_t promoted_version) const {\n'
            api_func += '    bool skip = false;\n'
            api_func += '    if (api_version < promoted_version) {\n'
            api_func += '        skip = LogError(instance,\n'
            api_func += '                        kVUID_PVError_ApiVersionViolation, "Attempted to call %s() with an effective API version of %s"\n'
            api_func += '                        "but this API was not promoted until version %s.", api_name, StringAPIVersion(api_version).c_str(),\n'
            api_func += '                        StringAPIVersion(promoted_version).c_str());\n'
            api_func += '    }\n'
            api_func += '    return skip;\n'
            api_func += '}\n\n'
            api_func += 'bool StatelessValidation::CheckPromotedApiAgainstVulkanVersion(VkPhysicalDevice pdev, const char *api_name, const uint32_t promoted_version) const {\n'
            api_func += '    bool skip = false;\n'
            api_func += '    const auto &target_pdev = physical_device_properties_map.find(pdev);\n'
            api_func += '    if (target_pdev != physical_device_properties_map.end()) {\n'
            api_func += '        auto effective_api_version = std::min(target_pdev->second->apiVersion, api_version);\n'
            api_func += '        if (effective_api_version < promoted_version) {\n'
            api_func += '            skip = LogError(instance,\n'
            api_func += '                            kVUID_PVError_ApiVersionViolation, "Attempted to call %s() with an effective API version of %s, "\n'
            api_func += '                            "which is the minimum of version requested in pApplicationInfo (%s) and supported by this physical device (%s), "\n'
            api_func += '                            "but this API was not promoted until version %s.", api_name, StringAPIVersion(effective_api_version).c_str(),\n'
            api_func += '                            StringAPIVersion(api_version).c_str(), StringAPIVersion(target_pdev->second->apiVersion).c_str(),\n'
            api_func += '                            StringAPIVersion(promoted_version).c_str());\n'
            api_func += '        }\n'
            api_func += '    }\n'
            api_func += '    return skip;\n'
            api_func += '}\n'
            write(api_func, file=self.outFile)

            pnext_handler  = 'bool StatelessValidation::ValidatePnextStructContents(const char *api_name, const ParameterName &parameter_name,\n'
            pnext_handler += '                                                      const VkBaseOutStructure* header, const char *pnext_vuid, bool is_physdev_api, bool is_const_param) const {\n'
            pnext_handler += '    bool skip = false;\n'
            pnext_handler += '    switch(header->sType) {\n'

            with open(os.path.join(self.genOpts.directory, self.categoryFilePath), mode='w', encoding='utf-8', newline='\n') as fd:
                preamble = f'''{self.GenerateCopyright(None)}
#include "chassis.h"
#include "hash_vk_types.h"
'''
                write(preamble, file=fd)
                for enum, string in self.enum_values_definitions.items():
                    if enum in self.called_types:
                        write(string, file=fd)

            # Do some processing here to extract data from validatedstructs...
            for item in self.structextends_list:
                postProcSpec = {}
                postProcSpec['ppp'] = '' if not item else '{postProcPrefix}'
                postProcSpec['pps'] = '' if not item else '{postProcSuffix}'
                postProcSpec['ppi'] = '' if not item else '{postProcInsert}'

                pnext_case = '\n'
                pnext_check = ''
                protect = ''
                # Guard struct cases with feature ifdefs, if necessary
                if item in self.struct_feature_protect.keys():
                    protect = self.struct_feature_protect[item]
                    pnext_case += '#ifdef %s\n' % protect
                pnext_case += '        // Validation code for %s structure members\n' % item
                pnext_case += '        case %s: { // Covers VUID-%s-sType-sType\n' % (self.structTypes[item], item)
                # pNext version/extension-enabled checks
                ver_info = ''
                struct_type = self.structTypes[item]
                if struct_type in self.stype_version_dict.keys():
                    ver_info = self.stype_version_dict[struct_type]
                else:
                    struct_type[:-4]
                    if struct_type[:-4] in self.stype_version_dict.values():
                        ver_info = self.stype_version_dict[struct_type[:-4]]
                    else:
                        ver_info = None
                api_check = False
                if ver_info is not None:
                    if 'VK_API_VERSION_' in ver_info:
                        api_check = True
                        api_version = ver_info;
                        pnext_check += '            if (api_version < %s) {\n' % ver_info
                        pnext_check += '                skip |= LogError(\n'
                        pnext_check += '                           instance, pnext_vuid,\n'
                        pnext_check += '                           "%%s: Includes a pNext pointer (%%s) to a VkStructureType (%s) which was added in %s but the "\n' % (struct_type, ver_info)
                        pnext_check += '                           "current effective API version is %s.",\n'
                        pnext_check += '                           api_name, parameter_name.get_name().c_str(), StringAPIVersion(api_version).c_str());\n'
                        pnext_check += '            }\n'
                    else:
                        # Dependent on enabled extension
                        ext_name = ver_info
                        ext_name_define = self.extension_names[ver_info]
                        table_type = ''
                        if ext_name_define in self.instance_extension_list:
                            table_type = 'instance'
                        elif ext_name_define in self.device_extension_list:
                            table_type = 'device'
                        else:
                            print("Error in parameter_validation_generator.py CodeGen.")
                        pnext_check += '            if (is_const_param) {\n'
                        if table_type == 'device':
                            pnext_check += f'                if ((is_physdev_api && !SupportedByPdev(physical_device, {ext_name_define})) || (!is_physdev_api && !IsExtEnabled({table_type}_extensions.{ext_name.lower()}))) {{\n'
                        else:
                            pnext_check += '                if (!%s_extensions.%s) {\n' % (table_type, ext_name.lower())
                        pnext_check += '                        skip |= LogError(\n'
                        pnext_check += '                               instance, pnext_vuid,\n'
                        pnext_check += '                               "%%s: Includes a pNext pointer (%%s) to a VkStructureType (%s), but its parent extension "\n' % struct_type
                        pnext_check += '                               "%s has not been enabled.",\n' % ext_name
                        pnext_check += '                               api_name, parameter_name.get_name().c_str());\n'
                        pnext_check += '                }\n'
                        pnext_check += '            }\n'
                        pnext_check += '\n'
                expr = self.expandStructCode(item, item, 'structure->', '', '                ', [], postProcSpec)
                struct_validation_source = self.ScrubStructCode(expr)
                if struct_validation_source != '':
                    pnext_check += '            if (is_const_param) {\n'
                    struct_validation_source = '                %s *structure = (%s *) header;\n' % (item, item) + struct_validation_source
                    struct_validation_source += '            }\n'
                pnext_case += '%s%s' % (pnext_check, struct_validation_source)
                pnext_case += '        } break;\n'
                if protect:
                    pnext_case += '#endif // %s\n' % protect
                # Skip functions containing no validation
                if struct_validation_source or pnext_check != '':
                    pnext_handler += pnext_case;
                else:
                    pnext_handler += '\n        // No Validation code for %s structure members  -- Covers VUID-%s-sType-sType\n' % (item, item)
            pnext_handler += '        default:\n'
            pnext_handler += '            skip = false;\n'
            pnext_handler += '    }\n'
            pnext_handler += '    return skip;\n'
            pnext_handler += '}\n'
            write(pnext_handler, file=self.outFile)
            self.newline()

            ext_template  = 'bool StatelessValidation::OutputExtensionError(const std::string &api_name, const std::string &extension_name) const {\n'
            ext_template += '    return LogError(instance,\n'
            ext_template += '                    kVUID_PVError_ExtensionNotEnabled, "Attempted to call %s() but its required extension %s has not been enabled\\n",\n'
            ext_template += '                    api_name.c_str(), extension_name.c_str());\n'
            ext_template += '}\n'
            write(ext_template, file=self.outFile)
            self.newline()
            commands_text = '\n'.join(self.validation)
            write(commands_text, file=self.outFile)
            self.newline()
        if self.header_file:
            # Output declarations and record intercepted procedures
            write('\n'.join(self.declarations), file=self.outFile)

            # Specializations need to appear outside of the class definition
            with open(os.path.join(self.genOpts.directory, self.specFilePath), mode='w', encoding='utf-8', newline='\n') as fd:
                write(self.GenerateCopyright(None), file=fd)
                write('\n'.join(self.specializations), file=fd)
        # Finish processing in superclass
        OutputGenerator.endFile(self)
    #
    # Processing at beginning of each feature or extension
    def beginFeature(self, interface, emit):
        # Start processing in superclass
        OutputGenerator.beginFeature(self, interface, emit)
        # C-specific
        # Accumulate includes, defines, types, enums, function pointer typedefs, end function prototypes separately for this
        # feature. They're only printed in endFeature().
        self.stypes = []
        self.commands = []
        self.structMembers = []
        self.newFlags = set()
        self.featureExtraProtect = GetFeatureProtect(interface)
        # Get base list of extension dependencies for all items in this extension
        base_required_extensions = []
        if "VK_VERSION_1" not in self.featureName:
            index = 0
            while interface[0][index].tag == 'comment':
                index += 1
            nameElem = interface[0][index + 1]
            name = nameElem.get('name')
            # Save Name Define to get correct enable name later
            self.extension_names[self.featureName] = name
            # This extension is the first dependency for this command
            base_required_extensions.append(self.featureName)
        # Add any defined extension dependencies to the base dependency list for this extension
        requires = interface.get('requires')
        if requires is not None:
            # Comma (',') will be replaced with plus ('+') soon here to harmonize meaning: ','==OR '+'==AND in <require> tag and requires="" attr
            base_required_extensions.extend(requires.split(','))
        # Build dictionary of extension dependencies for each item in this extension
        self.required_extensions = dict()
        for require_element in interface.findall('require'):
            # Copy base extension dependency list
            required_extensions = list(base_required_extensions)
            # Add any additional extension dependencies specified in this require block
            additional_extensions = require_element.get('extension')
            # require tags must split here by '+' as it is an AND operation according registry.adoc:
            #    == Attributes of tag:require tags
            #    attr:extension - optional, and only for tag:require tags.
            #    String containing one or more API extension names separated by , or +.
            #    Interfaces in the tag are only required if enabled extensions satisfy
            #    the logical expression in the string, where , is interpreted as a
            #    logical OR and '+' as a logical AND.
            if additional_extensions:
                required_extensions.extend(additional_extensions.split('+'))
            # Save full extension list for all named items
            for element in require_element.findall('*[@name]'):
                self.required_extensions[element.get('name')] = required_extensions

        # And note if this is an Instance or Device extension
        self.extension_type = interface.get('type')
        if interface.tag == 'extension':
            index = 0
            while interface[0][index].tag == 'comment':
                index += 1
            name_elem = interface[0][index + 1]
            name_definition = name_elem.get('name')
            if 'EXTENSION_NAME' not in name_definition:
                print("Error in vk.xml file -- extension name is not available")
            if interface.get('type') == 'instance':
                self.instance_extension_list += '%s, ' % GetNameDefine(interface)
            else:
                self.device_extension_list += '%s, ' % GetNameDefine(interface)

    #
    # Called at the end of each extension (feature)
    def endFeature(self):
        if self.header_file:
            return
        # C-specific
        # Actually write the interface to the output file.
        if (self.emit):
            # If type declarations are needed by other features based on this one, it may be necessary to suppress the ExtraProtect,
            # or move it below the 'for section...' loop.
            ifdef = ''
            if (self.featureExtraProtect is not None):
                ifdef = '#ifdef %s\n' % self.featureExtraProtect
                self.validation.append(ifdef)
            # Generate the struct member checking code from the captured data
            self.processStructMemberData()
            # Generate the command parameter checking code from the captured data
            self.processCmdData()
            # Write the declarations for the VkFlags values combining all flag bits
            for flag in sorted(self.newFlags):
                flagBits = flag.replace('Flags', 'FlagBits')
                if flagBits in self.flagBits:
                    bits = self.flagBits[flagBits]
                    decl = 'const {} All{} = {};'.format(flag, flagBits, '|'.join(bits))
                    self.flag_values_definitions[flag] = Guarded(self.featureExtraProtect, decl)
                    if flag in self.flagBitsAsArray:
                        decl = '[[maybe_unused]] constexpr std::array All%s = {%s};' % (flag, ','.join(bits))
                        self.flag_array_values_definitions[flag] = Guarded(self.featureExtraProtect, decl)

            endif = '\n'
            if (self.featureExtraProtect is not None):
                endif = '#endif // %s\n' % self.featureExtraProtect
            self.validation.append(endif)
        # Finish processing in superclass
        OutputGenerator.endFeature(self)
    #
    # Type generation
    def genType(self, typeinfo, name, alias):
        # record the name/alias pair
        if alias is not None:
            self.alias_dict[name]=alias
        OutputGenerator.genType(self, typeinfo, name, alias)
        typeElem = typeinfo.elem
        # If the type is a struct type, traverse the embedded <member> tags generating a structure. Otherwise, emit the tag text.
        category = typeElem.get('category')
        if (category == 'struct' or category == 'union'):
            self.genStruct(typeinfo, name, alias)
        elif (category == 'handle'):
            self.handleTypes.add(name)
        elif (category == 'bitmask'):
            self.flags.add(name)
            self.newFlags.add(name)
        elif (category == 'define'):
            if name == 'VK_HEADER_VERSION':
                nameElem = typeElem.find('name')
                self.headerVersion = noneStr(nameElem.tail).strip()
    #
    # Struct parameter check generation.
    # This is a special case of the <type> tag where the contents are interpreted as a set of <member> tags instead of freeform C
    # type declarations. The <member> tags are just like <param> tags - they are a declaration of a struct or union member.
    # Only simple member declarations are supported (no nested structs etc.)
    def genStruct(self, typeinfo, typeName, alias):
        if self.header_file:
            return
        # alias has already been recorded in genType, above
        OutputGenerator.genStruct(self, typeinfo, typeName, alias)

        conditions = self.structMemberValidationConditions[typeName] if typeName in self.structMemberValidationConditions else None
        members = typeinfo.elem.findall('.//member')
        if self.featureExtraProtect is not None:
            self.struct_feature_protect[typeName] = self.featureExtraProtect
        #
        # Iterate over members once to get length parameters for arrays
        lens = set()
        for member in members:
            len = self.getLen(member)
            if len:
                lens.add(len)
        #
        # Generate member info
        membersInfo = []
        returned_only = typeinfo.elem.attrib.get('returnedonly') is not None
        for member in members:
            # Get the member's type and name
            info = self.getTypeNameTuple(member)
            type = info[0]
            name = info[1]
            stypeValue = ''
            cdecl = self.makeCParamDecl(member, 0)
            ispointer = self.paramIsPointer(member)
            isconst = True if 'const' in cdecl else False

            # Store pointer/array/string info -- Check for parameter name in lens set
            iscount = False
            if name in lens:
                iscount = True
            # The pNext members are not tagged as optional, but are treated as optional for parameter NULL checks.  Static array
            # members are also treated as optional to skip NULL pointer validation, as they won't be NULL.
            isstaticarray = self.paramIsStaticArray(member)
            isoptional = False
            if self.paramIsOptional(member) or (name == 'pNext') or (isstaticarray):
                isoptional = True
            # Determine if value should be ignored by code generation.
            noautovalidity = False
            if (member.attrib.get('noautovalidity') is not None) or ((typeName in self.structMemberBlacklist) and (name in self.structMemberBlacklist[typeName])):
                noautovalidity = True

            # Some types are marked as noautovalidity, but stateless_validation.h will still want them for manual validation
            noautovalidity_type_exceptions = [
                "VkQueryPipelineStatisticFlags",
                "VkBorderColor"
            ]
            # Store all types that are from incoming calls if auto validity
            # non-const pointers don't have auto gen code as used for return values
            if (noautovalidity == False) or (type in noautovalidity_type_exceptions):
                if not returned_only and (not ispointer or isconst):
                    self.called_types.add(type)

            # enum file just needs the called_types
            if self.enum_file:
                continue

            structextends = False
            membersInfo.append(self.CommandParam(type=type, name=name,
                                                ispointer=ispointer,
                                                isstaticarray=isstaticarray,
                                                isbool=True if type == 'VkBool32' else False,
                                                israngedenum=True if type in self.enumRanges else False,
                                                isconst=isconst,
                                                isoptional=isoptional,
                                                iscount=iscount,
                                                noautovalidity=noautovalidity,
                                                len=self.getLen(member),
                                                extstructs=self.registry.validextensionstructs[typeName] if name == 'pNext' else None,
                                                condition=conditions[name] if conditions and name in conditions else None,
                                                cdecl=cdecl))
        # If this struct extends another, keep its name in list for further processing
        if typeinfo.elem.attrib.get('structextends') is not None:
            self.structextends_list.append(typeName)
        # Returnedonly structs should have most of their members ignored -- on entry, we only care about validating the sType and
        # pNext members. Everything else will be overwritten by the callee.
        if returned_only:
            self.returnedonly_structs.append(typeName)
            membersInfo = [m for m in membersInfo if m.name in ('sType', 'pNext')]
        self.structMembers.append(self.StructMemberData(name=typeName, members=membersInfo))
    #
    # Capture group (e.g. C "enum" type) info to be used for param check code generation.
    # These are concatenated together with other types.
    def genGroup(self, groupinfo, groupName, alias):
        if not self.header_file:
            # record the name/alias pair
            if alias is not None:
                self.alias_dict[groupName]=alias
            OutputGenerator.genGroup(self, groupinfo, groupName, alias)
            groupElem = groupinfo.elem
            # Store the sType values
            if groupName == 'VkStructureType':
                for elem in groupElem.findall('enum'):
                    self.stypes.append(elem.get('name'))
            elif 'FlagBits' in groupName:
                bits = []
                for elem in groupElem.findall('enum'):
                    if elem.get('supported') != 'disabled' and elem.get('alias') is None:
                        bits.append(elem.get('name'))
                if bits:
                    self.flagBits[groupName] = bits
            else:
                # Determine if begin/end ranges are needed (we don't do this for VkStructureType, which has a more finely grained check)
                expandName = re.sub(r'([0-9a-z_])([A-Z0-9][^A-Z0-9]?)',r'\1_\2',groupName).upper()
                expandPrefix = expandName
                expandSuffix = ''
                expandSuffixMatch = re.search(r'[A-Z][A-Z]+$',groupName)
                if expandSuffixMatch:
                    expandSuffix = '_' + expandSuffixMatch.group()
                    # Strip off the suffix from the prefix
                    expandPrefix = expandName.rsplit(expandSuffix, 1)[0]
                isEnum = ('FLAG_BITS' not in expandPrefix)
                if isEnum and self.source_file:
                    self.enumRanges.add(groupName)
                    # Create definition for a list containing valid enum values for this enumerated type
                    enum_entry = ''
                    if self.featureExtraProtect is not None:
                        enum_entry = '#ifdef %s\n' % self.featureExtraProtect
                    enum_entry_map = {}
                    for enum in groupElem:
                        name = enum.get('name')
                        if name is not None and enum.get('alias') is None and enum.get('supported') != 'disabled':
                            enum_map_key = set(['core'])
                            extnumber = enum.get('extnumber')

                            if extnumber is not None:
                                # Find the actual, "promoted to" extension
                                ext = self.extension_number_map[extnumber]
                                enum_map_key = set([ext.get('name')])

                                if name in self.extension_enums:
                                    enum_map_key = enum_map_key.union(self.extension_enums[name])

                            for k in sorted(enum_map_key):
                                if k not in enum_entry_map:
                                    enum_entry_map[k] = f'{name}, '
                                else:
                                    enum_entry_map[k] += f'{name}, '
                    if alias is None:
                        enum_entry += f'''
template<>
std::vector<{groupName}> ValidationObject::ValidParamValues() const {{
    // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
    //      Ideally "values" would be something like a static variable that is built once and this function returns
    //      a span of the container. This does not work for applications which create and destroy many instances and
    //      devices over the lifespan of the project (e.g., VLT).
    constexpr std::array Core{groupName}Enums = {{ {enum_entry_map["core"]} }};
    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<{groupName}>> Extended{groupName}Enums = {{\n'''
                        for k,v in enum_entry_map.items():
                            if k != 'core':
                                enum_entry += f'        {{ &DeviceExtensions::{k.lower()}, {{ {v} }} }},\n'
                        enum_entry += f'''    }};
    std::vector<{groupName}> values(Core{groupName}Enums.cbegin(), Core{groupName}Enums.cend());
    std::set<{groupName}> unique_exts;
    for (const auto& [extension, enums]: Extended{groupName}Enums) {{
        if (IsExtEnabled(device_extensions.*extension)) {{
            unique_exts.insert(enums.cbegin(), enums.cend());
        }}
    }}
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}}
'''
                    if self.featureExtraProtect is not None:
                        enum_entry += '\n#endif // %s' % self.featureExtraProtect
                    self.enum_values_definitions[groupName] = enum_entry
        elif self.header_file:
            if groupName != 'VkStructureType' and 'FlagBits' not in groupName:
                # Determine if begin/end ranges are needed (we don't do this for VkStructureType, which has a more finely grained check)
                expandName = re.sub(r'([0-9a-z_])([A-Z0-9][^A-Z0-9]?)',r'\1_\2',groupName).upper()
                isEnum = ('FLAG_BITS' not in expandName)
                if isEnum and alias is None:
                    if self.featureExtraProtect is not None: self.specializations += [ f'#ifdef {self.featureExtraProtect}'  ]
                    self.specializations += [ f'template<> std::vector<{groupName}> ValidationObject::ValidParamValues() const;' ]
                    if self.featureExtraProtect is not None: self.specializations += [ f'#endif // {self.featureExtraProtect}'  ]
    #
    # Capture command parameter info to be used for param check code generation.
    def genCmd(self, cmdinfo, name, alias):
        # record the name/alias pair
        if alias is not None:
            self.alias_dict[name]=alias
        OutputGenerator.genCmd(self, cmdinfo, name, alias)
        decls = self.makeCDecls(cmdinfo.elem)
        typedef = decls[1]
        typedef = typedef.split(')',1)[1]
        if self.header_file:
            if name not in self.blacklist:
                if (self.featureExtraProtect is not None):
                    self.declarations += [ '#ifdef %s' % self.featureExtraProtect ]
                # Strip off 'vk' from API name
                decl = '%s%s' % ('bool PreCallValidate', decls[0].split("VKAPI_CALL vk")[1])
                decl_terminator =  ' const override;'
                if 'ValidationCache' in name:
                    decl_terminator = ' const;'
                decl = str(decl).replace(';', decl_terminator)
                self.declarations += [ decl ]
                if (self.featureExtraProtect is not None):
                    self.declarations += [ '#endif' ]
        if self.source_file or self.enum_file:
            if name not in self.blacklist:
                params = cmdinfo.elem.findall('param')
                # Get list of array lengths
                lens = set()
                for param in params:
                    len = self.getLen(param)
                    if len:
                        lens.add(len)
                # Get param info
                paramsInfo = []
                for param in params:
                    paramInfo = self.getTypeNameTuple(param)
                    cdecl = self.makeCParamDecl(param, 0)
                    ispointer = self.paramIsPointer(param)
                    isconst = True if 'const' in cdecl else False
                    # non-const pointers don't have auto gen code as used for return values
                    if not ispointer or isconst:
                        self.called_types.add(paramInfo[0])
                    # Check for parameter name in lens set
                    iscount = False
                    if paramInfo[1] in lens:
                        iscount = True
                    paramsInfo.append(self.CommandParam(type=paramInfo[0], name=paramInfo[1],
                                                        ispointer=ispointer,
                                                        isstaticarray=self.paramIsStaticArray(param),
                                                        isbool=True if paramInfo[0] == 'VkBool32' else False,
                                                        israngedenum=True if paramInfo[0] in self.enumRanges else False,
                                                        isconst=isconst,
                                                        isoptional=self.paramIsOptional(param),
                                                        iscount=iscount,
                                                        noautovalidity=True if param.attrib.get('noautovalidity') is not None else False,
                                                        len=self.getLen(param),
                                                        extstructs=None,
                                                        condition=None,
                                                        cdecl=cdecl))
                # Save return value information, if any
                result_type = ''
                promotion_info = ''
                resultinfo = cmdinfo.elem.find('proto/type')
                if (resultinfo is not None and resultinfo.text != 'void'):
                    result_type = resultinfo.text
                if "VK_VERSION" in self.featureName and "VK_VERSION_1_0" != self.featureName:
                    if ('VkInstance' == paramsInfo[0].type or 'VkPhysicalDevice' == paramsInfo[0].type):
                        promotion_info = [paramsInfo[0].name, self.featureName]
                self.commands.append(self.CommandData(name=name, params=paramsInfo, cdecl=self.makeCDecls(cmdinfo.elem)[0], extension_type=self.extension_type, result=result_type, promotion_info=promotion_info))
    #
    # Check if the parameter passed in is a pointer
    def paramIsPointer(self, param):
        ispointer = 0
        paramtype = param.find('type')
        if (paramtype.tail is not None) and ('*' in paramtype.tail):
            ispointer = paramtype.tail.count('*')
        elif paramtype.text[:4] == 'PFN_':
            # Treat function pointer typedefs as a pointer to a single value
            ispointer = 1
        return ispointer
    #
    # Check if the parameter passed in is a static array
    def paramIsStaticArray(self, param):
        isstaticarray = 0
        paramname = param.find('name')
        if (paramname.tail is not None) and ('[' in paramname.tail):
            isstaticarray = paramname.tail.count('[')
        return isstaticarray
    #
    # Check if the parameter passed in is optional
    # Returns a list of Boolean values for comma separated len attributes (len='false,true')
    def paramIsOptional(self, param):
        # See if the handle is optional
        isoptional = False
        # Simple, if it's optional, return true
        optString = param.attrib.get('optional')
        if optString:
            if optString == 'true':
                isoptional = True
            elif ',' in optString:
                opts = []
                for opt in optString.split(','):
                    val = opt.strip()
                    if val == 'true':
                        opts.append(True)
                    elif val == 'false':
                        opts.append(False)
                    else:
                        print('Unrecognized len attribute value',val)
                isoptional = opts
        return isoptional
    #
    # Check if the handle passed in is optional
    # Uses the same logic as ValidityOutputGenerator.isHandleOptional
    def isHandleOptional(self, param, lenParam):
        # Simple, if it's optional, return true
        if param.isoptional:
            return True
        # If no validity is being generated, it usually means that validity is complex and not absolute, so let's say yes.
        if param.noautovalidity:
            return True
        # If the parameter is an array and we haven't already returned, find out if any of the len parameters are optional
        if lenParam and lenParam.isoptional:
            return True
        return False
    #
    # Retrieve the value of the len tag
    def getLen(self, param):
        result = None
        # Default to altlen when available to avoid LaTeX markup
        if 'altlen' in param.attrib:
            len = param.attrib.get('altlen')
        else:
            len = param.attrib.get('len')
        if len and len != 'null-terminated':
            # Only first level is supported for multidimensional arrays. Conveniently, this also strips the trailing
            # 'null-terminated' from arrays of strings
            len = len.split(',')[0]
            # Convert scope notation to pointer access
            result = str(len).replace('::', '->')
        elif self.paramIsStaticArray(param):
            # For static arrays get length from inside []
            array_match = re.search(r'\[(\d+)\]', param.find('name').tail)
            if array_match:
                result = array_match.group(1)
        return result
    #
    # Retrieve the type and name for a parameter
    def getTypeNameTuple(self, param):
        type = ''
        name = ''
        for elem in param:
            if elem.tag == 'type':
                type = noneStr(elem.text)
            elif elem.tag == 'name':
                name = noneStr(elem.text)
        return (type, name)
    #
    # Find a named parameter in a parameter list
    def getParamByName(self, params, name):
        for param in params:
            if param.name == name:
                return param
        return None
    #
    # Get the length paramater record for the specified length expression
    def getLenParam(self, params, length):
        # First check if any element of params matches length exactly
        lenParam = self.getParamByName(params, length)
        if not lenParam:
            # Otherwise, look for any elements of params that appear within length
            len_candidates = [p for p in params if re.search(r'\b{}\b'.format(p.name), length)]
            # 0 or 1 matches are expected, >1 would require a special case and/or explicit validation
            if len(len_candidates) == 0:
                lenParam = None
            elif len(len_candidates) == 1:
                lenParam = len_candidates[0]
            else:
                raise Exception('Cannot determine length parameter for len attribute value {}'.format(length))
        return lenParam
    #
    # Convert a vulkan.h command declaration into a parameter_validation.h definition
    def getCmdDef(self, cmd):
        # Strip the trailing ';' and split into individual lines
        lines = cmd.cdecl[:-1].split('\n')
        cmd_hdr = '\n'.join(lines)
        return cmd_hdr
    #
    # Generate the code to check for a NULL dereference before calling the
    # validation function
    def genCheckedLengthCall(self, name, exprs):
        count = name.count('->')
        if count:
            checkedExpr = []
            localIndent = ''
            elements = name.split('->')
            # Open the if expression blocks
            for i in range(0, count):
                checkedExpr.append(localIndent + 'if ({} != nullptr) {{\n'.format('->'.join(elements[0:i+1])))
                localIndent = self.incIndent(localIndent)
            # Add the validation expression
            for expr in exprs:
                checkedExpr.append(localIndent + expr)
            # Close the if blocks
            for i in range(0, count):
                localIndent = self.decIndent(localIndent)
                checkedExpr.append(localIndent + '}\n')
            return [checkedExpr]
        # No if statements were required
        return exprs
    #
    # Generate code to check for a specific condition before executing validation code
    def genConditionalCall(self, prefix, condition, exprs):
        checkedExpr = []
        localIndent = ''
        formattedCondition = condition.format(prefix)
        checkedExpr.append(localIndent + 'if ({})\n'.format(formattedCondition))
        checkedExpr.append(localIndent + '{\n')
        localIndent = self.incIndent(localIndent)
        for expr in exprs:
            checkedExpr.append(localIndent + expr)
        localIndent = self.decIndent(localIndent)
        checkedExpr.append(localIndent + '}\n')
        return [checkedExpr]
    #
    # Get VUID identifier from implicit VUID tag
    def GetVuid(self, name, suffix):
        vuid_string = 'VUID-%s-%s' % (name, suffix)
        vuid = "kVUIDUndefined"
        if '->' in vuid_string:
           return vuid
        if vuid_string in self.valid_vuids:
            vuid = "\"%s\"" % vuid_string
        else:
            if name in self.alias_dict:
                alias_string = 'VUID-%s-%s' % (self.alias_dict[name], suffix)
                if alias_string in self.valid_vuids:
                    vuid = "\"%s\"" % alias_string
        return vuid
    #
    # Generate the sType check string
    def makeStructTypeCheck(self, prefix, value, lenValue, valueRequired, lenValueRequired, lenPtrRequired, funcPrintName, lenPrintName, valuePrintName, postProcSpec, struct_type_name):
        checkExpr = []
        stype = self.structTypes[value.type]
        vuid_name = struct_type_name if struct_type_name is not None else funcPrintName
        stype_vuid = self.GetVuid(value.type, "sType-sType")
        param_vuid = self.GetVuid(vuid_name, "%s-parameter" % value.name)

        if lenValue:
            count_required_vuid = self.GetVuid(vuid_name, "%s-arraylength" % value.len)

            # This is an array of struct pointers
            if value.ispointer == 2:
                checkExpr.append('skip |= ValidateStructPointerTypeArray("{}", {ppp}"{ldn}"{pps}, {ppp}"{dn}"{pps}, "{sv}", {pf}{ln}, {pf}{vn}, {sv}, {}, {}, {}, {}, {});\n'.format(
                    funcPrintName, lenValueRequired, valueRequired, stype_vuid, param_vuid, count_required_vuid, ln=lenValue.name, ldn=lenPrintName, dn=valuePrintName, vn=value.name, sv=stype, pf=prefix, **postProcSpec))
            # This is an array with a pointer to a count value
            elif lenValue.ispointer:
                # When the length parameter is a pointer, there is an extra Boolean parameter in the function call to indicate if it is required
                checkExpr.append('skip |= ValidateStructTypeArray("{}", {ppp}"{ldn}"{pps}, {ppp}"{dn}"{pps}, "{sv}", {pf}{ln}, {pf}{vn}, {sv}, {}, {}, {}, {}, {}, {});\n'.format(
                    funcPrintName, lenPtrRequired, lenValueRequired, valueRequired, stype_vuid, param_vuid, count_required_vuid, ln=value.len, ldn=lenPrintName, dn=valuePrintName, vn=value.name, sv=stype, pf=prefix, **postProcSpec))
            # This is an array with an integer count value
            else:
                checkExpr.append('skip |= ValidateStructTypeArray("{}", {ppp}"{ldn}"{pps}, {ppp}"{dn}"{pps}, "{sv}", {pf}{ln}, {pf}{vn}, {sv}, {}, {}, {}, {}, {});\n'.format(
                    funcPrintName, lenValueRequired, valueRequired, stype_vuid, param_vuid, count_required_vuid, ln=value.len, ldn=lenPrintName, dn=valuePrintName, vn=value.name, sv=stype, pf=prefix, **postProcSpec))
        # This is an individual struct
        else:
            checkExpr.append('skip |= ValidateStructType("{}", {ppp}"{}"{pps}, "{sv}", {}{vn}, {sv}, {}, {}, {});\n'.format(
                funcPrintName, valuePrintName, prefix, valueRequired, param_vuid, stype_vuid, vn=value.name, sv=stype, vt=value.type, **postProcSpec))
        return checkExpr
    #
    # Generate the handle check string
    def makeHandleCheck(self, prefix, value, lenValue, valueRequired, lenValueRequired, funcPrintName, lenPrintName, valuePrintName, postProcSpec):
        checkExpr = []
        if lenValue:
            if lenValue.ispointer:
                # This is assumed to be an output array with a pointer to a count value
                raise Exception('Unsupported parameter validation case: Output handle array elements are not NULL checked')
            else:
                count_required_vuid = self.GetVuid(funcPrintName, "%s-arraylength" % (value.len))
                # This is an array with an integer count value
                checkExpr.append('skip |= ValidateHandleArray("{}", {ppp}"{ldn}"{pps}, {ppp}"{dn}"{pps}, {pf}{ln}, {pf}{vn}, {}, {}, {});\n'.format(
                    funcPrintName, lenValueRequired, valueRequired, count_required_vuid, ln=value.len, ldn=lenPrintName, dn=valuePrintName, vn=value.name, pf=prefix, **postProcSpec))
        else:
            # This is assumed to be an output handle pointer
            raise Exception('Unsupported parameter validation case: Output handles are not NULL checked')
        return checkExpr
    #
    # Generate check string for an array of VkFlags values
    def makeFlagsArrayCheck(self, prefix, value, lenValueRequired, callerName, lenPrintName, valuePrintName, postProcSpec):
        checkExpr = []
        flagBitsName = value.type.replace('Flags', 'FlagBits')
        if not flagBitsName in self.flagBits:
            raise Exception('Unsupported parameter validation case: array of reserved VkFlags')
        else:
            allFlags = 'All' + flagBitsName
            array_required_vuid = self.GetVuid(callerName, "%s-parameter" % (value.name))
            checkExpr.append('skip |= ValidateFlagsArray("{}", {ppp}"{}"{pps}, {ppp}"{}"{pps}, "{}", {}, {pf}{}, {pf}{}, {}, {});\n'.format(callerName, lenPrintName, valuePrintName, flagBitsName, allFlags, value.len, value.name, lenValueRequired, array_required_vuid, pf=prefix, **postProcSpec))
        return checkExpr
    #
    # Generate pNext check string
    def makeStructNextCheck(self, prefix, value, funcPrintName, valuePrintName, postProcSpec, struct_type_name):
        checkExpr = []
        # Generate an array of acceptable VkStructureType values for pNext
        extStructCount = 0
        extStructVar = 'nullptr'
        extStructNames = 'nullptr'
        extStructData = 'nullptr'
        pNextVuid = self.GetVuid(struct_type_name, "pNext-pNext")
        sTypeVuid = self.GetVuid(struct_type_name, "sType-unique")
        if value.extstructs:
            extStructVar = 'allowed_structs_{}'.format(struct_type_name)
            extStructCount = '{}.size()'.format(extStructVar)
            extStructData = '{}.data()'.format(extStructVar)
            if struct_type_name == 'VkInstanceCreateInfo':
                value.extstructs.append('VkInstanceLayerSettingsEXT')
                self.structTypes['VkInstanceLayerSettingsEXT'] = 'VK_STRUCTURE_TYPE_INSTANCE_LAYER_SETTINGS_EXT'
            extStructNames = '"' + ', '.join(value.extstructs) + '"'
            checkExpr.append('constexpr std::array {} = {{ {} }};\n'.format(extStructVar, ', '.join([self.structTypes[s] for s in value.extstructs])))
        checkExpr.append('skip |= ValidateStructPnext("{}", {ppp}"{}"{pps}, {}, {}{}, {}, {}, GeneratedVulkanHeaderVersion, {}, {});\n'.format(
            funcPrintName, valuePrintName, extStructNames, prefix, value.name, extStructCount, extStructData, pNextVuid, sTypeVuid, **postProcSpec))
        return checkExpr
    #
    # Generate the pointer check string
    def makePointerCheck(self, prefix, value, lenValue, valueRequired, lenValueRequired, lenPtrRequired, funcPrintName, lenPrintName, valuePrintName, postProcSpec, struct_type_name):
        checkExpr = []
        vuid_tag_name = struct_type_name if struct_type_name is not None else funcPrintName
        if lenValue:
            length_deref = '->' in value.len
            count_required_vuid = self.GetVuid(vuid_tag_name, "%s-arraylength" % (value.len))
            array_required_vuid = self.GetVuid(vuid_tag_name, "%s-parameter" % (value.name))
            # TODO: Remove workaround for missing optional tag in vk.xml
            if array_required_vuid == '"VUID-VkFramebufferCreateInfo-pAttachments-parameter"':
                return []
            # This is an array with a pointer to a count value
            if lenValue.ispointer and not length_deref:
                # If count and array parameters are optional, there will be no validation
                if valueRequired == 'true' or lenPtrRequired == 'true' or lenValueRequired == 'true':
                    # When the length parameter is a pointer, there is an extra Boolean parameter in the function call to indicate if it is required
                    checkExpr.append('skip |= ValidateArray("{}", {ppp}"{ldn}"{pps}, {ppp}"{dn}"{pps}, {pf}{ln}, &{pf}{vn}, {}, {}, {}, {}, {});\n'.format(
                        funcPrintName, lenPtrRequired, lenValueRequired, valueRequired, count_required_vuid, array_required_vuid, ln=value.len, ldn=lenPrintName, dn=valuePrintName, vn=value.name, pf=prefix, **postProcSpec))
            # This is an array with an integer count value
            else:
                # If count and array parameters are optional, there will be no validation
                if valueRequired == 'true' or lenValueRequired == 'true':
                    if value.type != 'char':
                        # A valid VU can't use '->' in the middle so the generated VUID from the spec uses '::' instead
                        count_required_vuid = self.GetVuid(vuid_tag_name, "%s-arraylength" % (value.len.replace('->', '::')))
                        checkExpr.append('skip |= ValidateArray("{}", {ppp}"{ldn}"{pps}, {ppp}"{dn}"{pps}, {pf}{ln}, &{pf}{vn}, {}, {}, {}, {});\n'.format(
                            funcPrintName, lenValueRequired, valueRequired, count_required_vuid, array_required_vuid, ln=value.len, ldn=lenPrintName, dn=valuePrintName, vn=value.name, pf=prefix, **postProcSpec))
                    else:
                        # Arrays of strings receive special processing
                        checkExpr.append('skip |= ValidateStringArray("{}", {ppp}"{ldn}"{pps}, {ppp}"{dn}"{pps}, {pf}{ln}, {pf}{vn}, {}, {}, {}, {});\n'.format(
                            funcPrintName, lenValueRequired, valueRequired, count_required_vuid, array_required_vuid, ln=value.len, ldn=lenPrintName, dn=valuePrintName, vn=value.name, pf=prefix, **postProcSpec))
            if checkExpr:
                if lenValue and length_deref:
                    # Add checks to ensure the validation call does not dereference a NULL pointer to obtain the count
                    checkExpr = self.genCheckedLengthCall(value.len, checkExpr)
        # This is an individual struct that is not allowed to be NULL
        elif not value.isoptional:
            # Function pointers need a reinterpret_cast to void*
            ptr_required_vuid = self.GetVuid(vuid_tag_name, "%s-parameter" % (value.name))
            if value.type[:4] == 'PFN_':
                allocator_dict = {'pfnAllocation': '"VUID-VkAllocationCallbacks-pfnAllocation-00632"',
                                  'pfnReallocation': '"VUID-VkAllocationCallbacks-pfnReallocation-00633"',
                                  'pfnFree': '"VUID-VkAllocationCallbacks-pfnFree-00634"',
                                 }
                vuid = allocator_dict.get(value.name)
                if vuid is not None:
                    ptr_required_vuid = vuid
                checkExpr.append('skip |= ValidateRequiredPointer("{}", {ppp}"{}"{pps}, reinterpret_cast<const void*>({}{}), {});\n'.format(funcPrintName, valuePrintName, prefix, value.name, ptr_required_vuid, **postProcSpec))
            else:
                checkExpr.append('skip |= ValidateRequiredPointer("{}", {ppp}"{}"{pps}, {}{}, {});\n'.format(funcPrintName, valuePrintName, prefix, value.name, ptr_required_vuid, **postProcSpec))
        else:
            # Special case for optional internal allocation function pointers.
            if (value.type, value.name) == ('PFN_vkInternalAllocationNotification', 'pfnInternalAllocation'):
                checkExpr.extend(self.internalAllocationCheck(funcPrintName, prefix, value.name, 'pfnInternalFree', postProcSpec))
            elif (value.type, value.name) == ('PFN_vkInternalFreeNotification', 'pfnInternalFree'):
                checkExpr.extend(self.internalAllocationCheck(funcPrintName, prefix, value.name, 'pfnInternalAllocation', postProcSpec))
        return checkExpr

    #
    # Generate internal allocation function pointer check.
    def internalAllocationCheck(self, funcPrintName, prefix, name, complementaryName, postProcSpec):
        checkExpr = []
        vuid = '"VUID-VkAllocationCallbacks-pfnInternalAllocation-00635"'
        checkExpr.append('if ({}{} != nullptr)'.format(prefix, name))
        checkExpr.append('{')
        local_indent = self.incIndent('')
        # Function pointers need a reinterpret_cast to void*
        checkExpr.append(local_indent + 'skip |= ValidateRequiredPointer("{}", {ppp}"{}{}"{pps}, reinterpret_cast<const void*>({}{}), {});\n'.format(funcPrintName, prefix, complementaryName, prefix, complementaryName, vuid, **postProcSpec))
        checkExpr.append('}\n')
        return checkExpr

    #
    # Process struct member validation code, performing name substitution if required
    def processStructMemberCode(self, line, funcName, memberNamePrefix, memberDisplayNamePrefix, postProcSpec):
        # Build format specifier list
        kwargs = {}
        if '{postProcPrefix}' in line:
            # If we have a tuple that includes a format string and format parameters, need to use ParameterName class
            if type(memberDisplayNamePrefix) is tuple:
                kwargs['postProcPrefix'] = 'ParameterName('
            else:
                kwargs['postProcPrefix'] = postProcSpec['ppp']
        if '{postProcSuffix}' in line:
            # If we have a tuple that includes a format string and format parameters, need to use ParameterName class
            if type(memberDisplayNamePrefix) is tuple:
                kwargs['postProcSuffix'] = ', ParameterName::IndexVector{{ {}{} }})'.format(postProcSpec['ppi'], memberDisplayNamePrefix[1])
            else:
                kwargs['postProcSuffix'] = postProcSpec['pps']
        if '{postProcInsert}' in line:
            # If we have a tuple that includes a format string and format parameters, need to use ParameterName class
            if type(memberDisplayNamePrefix) is tuple:
                kwargs['postProcInsert'] = '{}{}, '.format(postProcSpec['ppi'], memberDisplayNamePrefix[1])
            else:
                kwargs['postProcInsert'] = postProcSpec['ppi']
        if '{funcName}' in line:
            kwargs['funcName'] = funcName
        if '{valuePrefix}' in line:
            kwargs['valuePrefix'] = memberNamePrefix
        if '{displayNamePrefix}' in line:
            # Check for a tuple that includes a format string and format parameters to be used with the ParameterName class
            if type(memberDisplayNamePrefix) is tuple:
                kwargs['displayNamePrefix'] = memberDisplayNamePrefix[0]
            else:
                kwargs['displayNamePrefix'] = memberDisplayNamePrefix

        if kwargs:
            # Need to escape the C++ curly braces
            if 'IndexVector' in line:
                line = line.replace('IndexVector{ ', 'IndexVector{{ ')
                line = line.replace(' }),', ' }}),')
            return line.format(**kwargs)
        return line
    #
    # Process struct member validation code, stripping metadata
    def ScrubStructCode(self, code):
        scrubbed_lines = ''
        for line in code:
            if 'ValidateStructPnext(' in line:
                continue
            if 'allowed_structs' in line:
                continue
            if 'xml-driven validation' in line:
                continue
            line = line.replace('{postProcPrefix}', '')
            line = line.replace('{postProcSuffix}', '')
            line = line.replace('{postProcInsert}', '')
            line = line.replace('{funcName}', '')
            line = line.replace('{valuePrefix}', '')
            line = line.replace('{displayNamePrefix}', '')
            line = line.replace('{IndexVector}', '')
            line = line.replace('local_data->', '')
            scrubbed_lines += line
        return scrubbed_lines
    #
    # Process struct validation code for inclusion in function or parent struct validation code
    def expandStructCode(self, item_type, funcName, memberNamePrefix, memberDisplayNamePrefix, indent, output, postProcSpec):
        lines = self.validatedStructs[item_type]
        for line in lines:
            if output:
                output[-1] += '\n'
            if type(line) is list:
                for sub in line:
                    output.append(self.processStructMemberCode(indent + sub, funcName, memberNamePrefix, memberDisplayNamePrefix, postProcSpec))
            else:
                output.append(self.processStructMemberCode(indent + line, funcName, memberNamePrefix, memberDisplayNamePrefix, postProcSpec))
        return output
    #
    # Process struct pointer/array validation code, performing name substitution if required
    def expandStructPointerCode(self, prefix, value, lenValue, funcName, valueDisplayName, postProcSpec):
        expr = []
        expr.append('if ({}{} != nullptr)\n'.format(prefix, value.name))
        expr.append('{')
        indent = self.incIndent(None)
        if lenValue:
            # Need to process all elements in the array
            indexName = value.len.replace('Count', 'Index')
            expr[-1] += '\n'
            if lenValue.ispointer:
                # If the length value is a pointer, de-reference it for the count.
                expr.append(indent + 'for (uint32_t {iname} = 0; {iname} < *{}{}; ++{iname})\n'.format(prefix, value.len, iname=indexName))
            else:
                expr.append(indent + 'for (uint32_t {iname} = 0; {iname} < {}{}; ++{iname})\n'.format(prefix, value.len, iname=indexName))
            expr.append(indent + '{')
            indent = self.incIndent(indent)
            # Prefix for value name to display in error message
            if value.ispointer == 2:
                memberNamePrefix = '{}{}[{}]->'.format(prefix, value.name, indexName)
                memberDisplayNamePrefix = ('{}[%i]->'.format(valueDisplayName), indexName)
            else:
                memberNamePrefix = '{}{}[{}].'.format(prefix, value.name, indexName)
                memberDisplayNamePrefix = ('{}[%i].'.format(valueDisplayName), indexName)
        else:
            memberNamePrefix = '{}{}->'.format(prefix, value.name)
            memberDisplayNamePrefix = '{}->'.format(valueDisplayName)
        # Expand the struct validation lines
        expr = self.expandStructCode(value.type, funcName, memberNamePrefix, memberDisplayNamePrefix, indent, expr, postProcSpec)
        if lenValue:
            # Close if and for scopes
            indent = self.decIndent(indent)
            expr.append(indent + '}\n')
        expr.append('}\n')
        return expr
    #
    # Generate the parameter checking code
    def genFuncBody(self, funcName, values, valuePrefix, displayNamePrefix, structTypeName, is_phys_device = False):
        lines = []    # Generated lines of code
        unused = []   # Unused variable names
        duplicateCountVuid = [] # prevent duplicate VUs being generated

        # TODO Using a regex in this context is not ideal. Would be nicer if usedLines were a list of objects with "settings" (such as "is_phys_device")
        validate_pnext_rx = re.compile(r'(.*ValidateStructPnext\(.*)(\).*\n*)', re.M)

        for value in values:
            usedLines = []
            lenParam = None
            #
            # Prefix and suffix for post processing of parameter names for struct members.  Arrays of structures need special processing to include the array index in the full parameter name.
            postProcSpec = {}
            postProcSpec['ppp'] = '' if not structTypeName else '{postProcPrefix}'
            postProcSpec['pps'] = '' if not structTypeName else '{postProcSuffix}'
            postProcSpec['ppi'] = '' if not structTypeName else '{postProcInsert}'
            #
            # Generate the full name of the value, which will be printed in the error message, by adding the variable prefix to the value name
            valueDisplayName = '{}{}'.format(displayNamePrefix, value.name)
            #
            # Check for NULL pointers, ignore the in-out count parameters that
            # will be validated with their associated array
            if (value.ispointer or value.isstaticarray) and not value.iscount:
                # Parameters for function argument generation
                req = 'true'    # Parameter cannot be NULL
                cpReq = 'true'  # Count pointer cannot be NULL
                cvReq = 'true'  # Count value cannot be 0
                lenDisplayName = None # Name of length parameter to print with validation messages; parameter name with prefix applied
                countRequiredVuid = None # If there is a count required VUID to check
                # Generate required/optional parameter strings for the pointer and count values
                if value.isoptional:
                    req = 'false'
                if value.len:
                    # The parameter is an array with an explicit count parameter
                    lenParam = self.getLenParam(values, value.len)
                    if lenParam:
                        lenDisplayName = value.len.replace(lenParam.name, displayNamePrefix + lenParam.name)
                        if lenParam.ispointer:
                            # Count parameters that are pointers are inout
                            if type(lenParam.isoptional) is list:
                                if lenParam.isoptional[0]:
                                    cpReq = 'false'
                                if lenParam.isoptional[1]:
                                    cvReq = 'false'
                            else:
                                if lenParam.isoptional:
                                    cpReq = 'false'
                                # In case of count as field in another struct, look up field to see if count is optional.
                                len_deref = value.len.split('->')
                                if len(len_deref) == 2:
                                    struct_fields = next((struct.members for struct in self.structMembers if struct.name == lenParam.type), None)
                                    if struct_fields:
                                        len_field_name = len_deref[1]
                                        struct_field = next((field for field in struct_fields if field.name == len_field_name), None)
                                        if struct_field and struct_field.isoptional:
                                            cvReq = 'false'
                        else:
                            vuidNameTag = structTypeName if structTypeName is not None else funcName
                            vuidName = self.GetVuid(vuidNameTag, "%s-arraylength" % (lenParam.name))
                            arrayVuidExceptions = ["\"VUID-vkCmdBindVertexBuffers2-bindingCount-arraylength\""] # This VUID is considered special, as it is the only one whose names ends in "-arraylength" but has special conditions allowing bindingCount to be 0.
                            if vuidName in arrayVuidExceptions:
                                continue
                            if lenParam.isoptional:
                                cvReq = 'false'
                            elif value.noautovalidity:
                                # Handle edge case where XML expresses a non-optional non-pointer value length with noautovalidity
                                # ex: <param noautovalidity="true"len="commandBufferCount">
                                vuidNameTag = structTypeName if structTypeName is not None else funcName
                                countRequiredVuid = self.GetVuid(vuidNameTag, "%s-arraylength" % (lenParam.name))
                                if countRequiredVuid in duplicateCountVuid:
                                    countRequiredVuid = None
                                else:
                                    duplicateCountVuid.append(countRequiredVuid)
                    else:
                        # Do not generate length checks for constant sized arrays
                        cpReq = 'false'
                        cvReq = 'false'

                #
                # The parameter will not be processed when tagged as 'noautovalidity'
                # For the pointer to struct case, the struct pointer will not be validated, but any
                # members not tagged as 'noautovalidity' will be validated
                # We special-case the custom allocator checks, as they are explicit but can be auto-generated.
                AllocatorFunctions = ['PFN_vkAllocationFunction', 'PFN_vkReallocationFunction', 'PFN_vkFreeFunction', 'PFN_vkInternalAllocationNotification', 'PFN_vkInternalFreeNotification']
                if value.noautovalidity and value.type not in AllocatorFunctions and not countRequiredVuid:
                    # Log a diagnostic message when validation cannot be automatically generated and must be implemented manually
                    self.logMsg('diag', 'ParameterValidation: No validation for {} {}'.format(structTypeName if structTypeName else funcName, value.name))
                elif countRequiredVuid:
                    usedLines.append('skip |= ValidateArray("{}", {ppp}"{ldn}"{pps}, "", {pf}{ln}, &{pf}{vn}, true, false, {}, kVUIDUndefined);\n'.format(
                        funcName, countRequiredVuid, pf=valuePrefix, ldn=lenDisplayName, ln=value.len, vn=value.name, **postProcSpec))
                else:
                    if value.type in self.structTypes:
                        # If this is a pointer to a struct with an sType field, verify the type
                        usedLines += self.makeStructTypeCheck(valuePrefix, value, lenParam, req, cvReq, cpReq, funcName, lenDisplayName, valueDisplayName, postProcSpec, structTypeName)
                    # If this is an input handle array that is not allowed to contain NULL handles, verify that none of the handles are VK_NULL_HANDLE
                    elif value.type in self.handleTypes and value.isconst and not self.isHandleOptional(value, lenParam):
                        usedLines += self.makeHandleCheck(valuePrefix, value, lenParam, req, cvReq, funcName, lenDisplayName, valueDisplayName, postProcSpec)
                    elif value.type in self.flags and value.isconst:
                        callerName = structTypeName if structTypeName else funcName
                        usedLines += self.makeFlagsArrayCheck(valuePrefix, value, cvReq, callerName, lenDisplayName, valueDisplayName, postProcSpec)
                    elif value.isbool and value.isconst:
                        usedLines.append('skip |= ValidateBool32Array("{}", {ppp}"{}"{pps}, {ppp}"{}"{pps}, {pf}{}, {pf}{}, {}, {});\n'.format(funcName, lenDisplayName, valueDisplayName, value.len, value.name, cvReq, req, pf=valuePrefix, **postProcSpec))
                    elif value.israngedenum and value.isconst:
                        prefix = postProcSpec.get('ppp', '')
                        suffix = postProcSpec.get('pps', '')
                        usedLines.append(f'skip |= ValidateRangedEnumArray("{funcName}", {prefix}"{lenDisplayName}"{suffix}, {prefix}"{valueDisplayName}"{suffix}, "{value.type}", {valuePrefix}{value.len}, {valuePrefix}{value.name}, {cvReq}, {req});\n')
                    elif value.name == 'pNext':
                        usedLines += self.makeStructNextCheck(valuePrefix, value, funcName, valueDisplayName, postProcSpec, structTypeName)
                    else:
                        usedLines += self.makePointerCheck(valuePrefix, value, lenParam, req, cvReq, cpReq, funcName, lenDisplayName, valueDisplayName, postProcSpec, structTypeName)
                    # If this is a pointer to a struct (input), see if it contains members that need to be checked
                    if value.type in self.validatedStructs:
                        if value.isconst: # or value.type in self.returnedonly_structs:
                            usedLines.append(self.expandStructPointerCode(valuePrefix, value, lenParam, funcName, valueDisplayName, postProcSpec))
                        elif value.type in self.returnedonly_structs:
                            usedLines.append(self.expandStructPointerCode(valuePrefix, value, lenParam, funcName, valueDisplayName, postProcSpec))

                    is_const_str = 'true' if value.isconst else 'false'
                    is_phys_device_str = 'true' if is_phys_device else 'false'
                    for setter, _, elem in multi_string_iter(usedLines):
                        elem = re.sub(r', (true|false)', '', elem)
                        m = validate_pnext_rx.match(elem)
                        if m is not None:
                            setter(f'{m.group(1)}, {is_phys_device_str}, {is_const_str}{m.group(2)}')

            # Non-pointer types
            else:
                # The parameter will not be processes when tagged as 'noautovalidity'
                # For the struct case, the struct type will not be validated, but any
                # members not tagged as 'noautovalidity' will be validated
                if value.noautovalidity:
                    # Log a diagnostic message when validation cannot be automatically generated and must be implemented manually
                    self.logMsg('diag', 'ParameterValidation: No validation for {} {}'.format(structTypeName if structTypeName else funcName, value.name))
                else:
                    vuid_name_tag = structTypeName if structTypeName is not None else funcName
                    if value.type in self.structTypes:
                        stype = self.structTypes[value.type]
                        vuid = self.GetVuid(value.type, "sType-sType")
                        undefined_vuid = '"kVUIDUndefined"'
                        usedLines.append('skip |= ValidateStructType("{}", {ppp}"{}"{pps}, "{sv}", &({}{vn}), {sv}, false, kVUIDUndefined, {});\n'.format(
                            funcName, valueDisplayName, valuePrefix, vuid, vn=value.name, sv=stype, vt=value.type, **postProcSpec))
                    elif value.type in self.handleTypes:
                        if not self.isHandleOptional(value, None):
                            usedLines.append('skip |= ValidateRequiredHandle("{}", {ppp}"{}"{pps}, {}{});\n'.format(funcName, valueDisplayName, valuePrefix, value.name, **postProcSpec))
                    elif value.type in self.flags and value.type.replace('Flags', 'FlagBits') not in self.flagBits:
                        vuid = self.GetVuid(vuid_name_tag, "%s-zerobitmask" % (value.name))
                        usedLines.append('skip |= ValidateReservedFlags("{}", {ppp}"{}"{pps}, {pf}{}, {});\n'.format(funcName, valueDisplayName, value.name, vuid, pf=valuePrefix, **postProcSpec))
                    elif value.type in self.flags or value.type in self.flagBits:
                        if value.type in self.flags:
                            flagBitsName = value.type.replace('Flags', 'FlagBits')
                            flagsType = 'kOptionalFlags' if value.isoptional else 'kRequiredFlags'
                            invalidVuid = self.GetVuid(vuid_name_tag, "%s-parameter" % (value.name))
                            zeroVuid = self.GetVuid(vuid_name_tag, "%s-requiredbitmask" % (value.name))
                        elif value.type in self.flagBits:
                            flagBitsName = value.type
                            flagsType = 'kOptionalSingleBit' if value.isoptional else 'kRequiredSingleBit'
                            invalidVuid = self.GetVuid(vuid_name_tag, "%s-parameter" % (value.name))
                            zeroVuid = invalidVuid
                        allFlagsName = 'All' + flagBitsName

                        invalid_vuid = self.GetVuid(vuid_name_tag, "%s-parameter" % (value.name))
                        allFlagsName = 'All' + flagBitsName
                        zeroVuidArg = '' if value.isoptional else ', ' + zeroVuid
                        usedLines.append('skip |= ValidateFlags("{}", {ppp}"{}"{pps}, "{}", {}, {pf}{}, {}, {}{});\n'.format(funcName, valueDisplayName, flagBitsName, allFlagsName, value.name, flagsType, invalidVuid, zeroVuidArg, pf=valuePrefix, **postProcSpec))
                    elif value.isbool:
                        usedLines.append('skip |= ValidateBool32("{}", {ppp}"{}"{pps}, {}{});\n'.format(funcName, valueDisplayName, valuePrefix, value.name, **postProcSpec))
                    elif value.israngedenum:
                        vuid = self.GetVuid(vuid_name_tag, "%s-parameter" % (value.name))
                        prefix = postProcSpec.get('ppp', '')
                        suffix = postProcSpec.get('pps', '')
                        usedLines.append(f'skip |= ValidateRangedEnum("{funcName}", {prefix}"{valueDisplayName}"{suffix}, "{value.type}", {valuePrefix}{value.name}, {vuid});\n')
                    # If this is a struct, see if it contains members that need to be checked
                    if value.type in self.validatedStructs:
                        memberNamePrefix = '{}{}.'.format(valuePrefix, value.name)
                        memberDisplayNamePrefix = '{}.'.format(valueDisplayName)
                        usedLines.append(self.expandStructCode(value.type, funcName, memberNamePrefix, memberDisplayNamePrefix, '', [], postProcSpec))
            # Append the parameter check to the function body for the current command
            if usedLines:
                # Apply special conditional checks
                if value.condition:
                    usedLines = self.genConditionalCall(valuePrefix, value.condition, usedLines)
                lines += usedLines
            elif not value.iscount:
                # If no expression was generated for this value, it is unreferenced by the validation function, unless
                # it is an array count, which is indirectly referenced for array valiadation.
                unused.append(value.name)
        if not lines:
            lines.append('// No xml-driven validation\n')
        return lines, unused
    #
    # Generate the struct member check code from the captured data
    def processStructMemberData(self):
        indent = self.incIndent(None)
        for struct in self.structMembers:
            #
            # The string returned by genFuncBody will be nested in an if check for a NULL pointer, so needs its indent incremented
            lines, unused = self.genFuncBody('{funcName}', struct.members, '{valuePrefix}', '{displayNamePrefix}', struct.name)
            if lines:
                self.validatedStructs[struct.name] = lines
    #
    # Generate the command param check code from the captured data
    def processCmdData(self):
        indent = self.incIndent(None)
        for command in self.commands:
            # Skip first parameter if it is a dispatch handle (everything except vkCreateInstance)
            startIndex = 0 if command.name == 'vkCreateInstance' else 1
            lines, unused = self.genFuncBody(command.name, command.params[startIndex:], '', '', None, is_phys_device = command.params[0].type == 'VkPhysicalDevice')
            # Cannot validate extension dependencies for device extension APIs having a physical device as their dispatchable object
            if (command.name in self.required_extensions) and (self.extension_type != 'device' or command.params[0].type != 'VkPhysicalDevice'):
                for ext in self.required_extensions[command.name]:
                    if ',' in ext:
                        extor_list = ext.split(',')
                        ext_test = ''
                        ext_name_define = ''
                        for extor in extor_list:
                            for extension in self.registry.extensions:
                                if extension.attrib['name'] == extor:
                                    if ext_name_define != '':
                                        ext_name_define += ' " or " '
                                    ext_name_define += GetNameDefine(extension)
                                    break
                            if ext_test != '':
                                ext_test += ' || '
                            ext_test += 'IsExtEnabled(device_extensions.%s)' % (extor.lower())
                        ext_test = 'if (!(%s)) skip |= OutputExtensionError("%s", %s);\n' % (ext_test, command.name, ext_name_define)
                        lines.insert(0, ext_test)
                    else:
                        ext_name_define = ''
                        for extension in self.registry.extensions:
                            if extension.attrib['name'] == ext:
                                ext_name_define = GetNameDefine(extension)
                                break
                        ext_test = ''
                        if command.params[0].type in ["VkInstance", "VkPhysicalDevice"] or command.name == 'vkCreateInstance':
                            ext_test = 'if (!instance_extensions.%s) skip |= OutputExtensionError("%s", %s);\n' % (ext.lower(), command.name, ext_name_define)
                        else:
                            ext_test = 'if (!IsExtEnabled(device_extensions.%s)) skip |= OutputExtensionError("%s", %s);\n' % (ext.lower(), command.name, ext_name_define)
                        lines.insert(0, ext_test)
            if lines:
                func_sig = self.getCmdDef(command) + ' const {\n'
                func_sig = func_sig.split('VKAPI_CALL vk')[1]
                cmdDef = 'bool StatelessValidation::PreCallValidate' + func_sig
                cmdDef += '%sbool skip = false;\n' % indent
                if isinstance(command.promotion_info, list):
                    version_flag = command.promotion_info[1]
                    version_id = version_flag.replace('VK_VERSION', 'VK_API_VERSION')
                    cmdDef += '%s if (CheckPromotedApiAgainstVulkanVersion(%s, "%s", %s)) return true;\n' % (indent, command.promotion_info[0], command.name, version_id)
                for line in lines:
                    if type(line) is list:
                        for sub in line:
                            cmdDef += indent + sub
                    else:
                        cmdDef += indent + line
                # Insert call to custom-written function if present
                if command.name in self.functions_with_manual_checks:
                    # Generate parameter list for manual fcn and down-chain calls
                    params_text = ''
                    for param in command.params:
                        params_text += '%s, ' % param.name
                    params_text = params_text[:-2] + ');\n'
                    cmdDef += '    if (!skip) skip |= manual_PreCallValidate'+ command.name[2:] + '(' + params_text
                cmdDef += '%sreturn skip;\n' % indent
                cmdDef += '}\n'
                self.validation.append(cmdDef)
