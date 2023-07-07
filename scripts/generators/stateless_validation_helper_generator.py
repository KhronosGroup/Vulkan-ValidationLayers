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
import sys
import re

from common_codegen import (exprToCpp, exprValues, parseExpr)
from generators.generator_utils import (fileIsGeneratedWarning)
from generators.vulkan_object import (Member)
from generators.base_generator import BaseGenerator
from typing import List


class StatelessValidationHelperOutputGenerator(BaseGenerator):
    def __init__(self,
                 errFile = sys.stderr,
                 warnFile = sys.stderr,
                 diagFile = sys.stdout):
        BaseGenerator.__init__(self, errFile, warnFile, diagFile)
        self.INDENT_SPACES = 4
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
            'vkFreeCommandBuffers',
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
            'vkGetMemoryFdKHR',
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
            'vkCreateXcbSurfaceKHR',
            'vkCreateXlibSurfaceKHR',
            'vkGetPhysicalDeviceSurfaceFormatsKHR',
            'vkGetPhysicalDeviceSurfacePresentModesKHR',
            'vkGetPhysicalDeviceSurfaceCapabilities2KHR',
            'vkGetPhysicalDeviceSurfaceFormats2KHR',
            'vkGetPhysicalDeviceSurfacePresentModes2EXT',
            'vkExportMetalObjectsEXT',
            'vkCmdSetDiscardRectangleEnableEXT',
            'vkCmdSetDiscardRectangleModeEXT',
            'vkCmdSetExclusiveScissorEnableNV',
            'vkGetMemoryWin32HandlePropertiesKHR',
            'vkGetMemoryFdPropertiesKHR',
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

        # Validation conditions for some special case struct members that are conditionally validated
        self.structMemberValidationConditions = [
            {
                'struct' : 'VkSubpassDependency2',
                'field' :  'VkPipelineStageFlagBits',
                'condition' : '!LvlFindInChain<VkMemoryBarrier2>(pCreateInfo->pDependencies[dependencyIndex].pNext)'
            },
            {
                'struct' : 'VkSubpassDependency2',
                'field' :  'VkAccessFlagBits',
                'condition' : '!LvlFindInChain<VkMemoryBarrier2>(pCreateInfo->pDependencies[dependencyIndex].pNext)'
            }
        ]

        # Map of structs type names to generated validation code for that struct type
        self.validatedStructs = dict()
        # Map of flags typenames
        self.flags = set()
        # Map of flag bits typename to list of values
        self.flagBits = dict()
        # Dictionary of required extension boolean expressions for each item in the current extension
        self.required_extension_expressions = dict()

    def generate(self):
        copyright = f'''{fileIsGeneratedWarning(os.path.basename(__file__))}
/***************************************************************************
*
* Copyright (c) 2015-2023 The Khronos Group Inc.
* Copyright (c) 2015-2023 Valve Corporation
* Copyright (c) 2015-2023 LunarG, Inc.
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
****************************************************************************/\n'''
        self.write(copyright)
        self.write('// NOLINTBEGIN') # Wrap for clang-tidy to ignore

        if self.filename == 'stateless_validation_helper.h':
            self.generateHeader()
        elif self.filename == 'stateless_validation_helper.cpp':
            self.generateSource()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateHeader(self):
        out = []
        out.append('#pragma once\n')
        for command in [x for x in self.vk.commands.values() if x.name not in self.blacklist]:
            out.extend([f'#ifdef {command.protect}\n'] if command.protect else [])
            prototype = command.cPrototype.split('VKAPI_CALL ')[1]
            prototype = f'bool PreCallValidate{prototype[2:]}'
            prototype = prototype.replace(');', ') const override;\n')
            if 'ValidationCache' in command.name:
                prototype = prototype.replace('const override', 'const')
            out.append(prototype)
            out.extend([f'#endif // {command.protect}\n'] if command.protect else [])
        self.write("".join(out))

    def generateSource(self):
        # Structure fields to ignore
        structMemberBlacklist = {
            'VkWriteDescriptorSet' : ['dstSet'],
            'VkAccelerationStructureGeometryKHR' :['geometry'],
            'VkDescriptorDataEXT' :['pSampler']
        }
        for struct in [x for x in self.vk.structs.values() if x.name in structMemberBlacklist]:
            for member in [x for x in struct.members if x.name in structMemberBlacklist[struct.name]]:
                member.noAutoValidity = True

        # TODO - We should not need this with VulkanObject, but the following are casuing issues
        # being "promoted"
        #  VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT
        #  VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_2_PLANE_444_FORMATS_FEATURES_EXT
        #  VK_STRUCTURE_TYPE_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_2_NV
        #  VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT
        #  VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT
        #  VK_STRUCTURE_TYPE_CHECKPOINT_DATA_2_NV
        #  VK_STRUCTURE_TYPE_MULTIVIEW_PER_VIEW_ATTRIBUTES_INFO_NVX
        #  VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR
        #  VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_4444_FORMATS_FEATURES_EXT
        #  VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_DENSITY_MAP_ATTACHMENT_INFO_EXT
        root = self.registry.reg
        stype_version_dict = dict()
        for extensions in root.findall('extensions'):
            for extension in extensions.findall('extension'):
                extensionName = extension.get('name')
                promotedTo = extension.get('promotedto')
                # TODO Issue 5103 - this is being used to remove false positive currently
                promotedToCore = promotedTo is not None and 'VK_VERSION' in promotedTo

                for entry in extension.iterfind('require/enum[@extends="VkStructureType"]'):
                    if (entry.get('comment') is None or 'typo' not in entry.get('comment')):
                        alias = entry.get('alias')
                        if (alias is not None and promotedToCore):
                            stype_version_dict[alias] = extensionName
        out = []

        out.append('''
#include "chassis.h"

#include "stateless/stateless_validation.h"
#include "enum_flag_bits.h"

bool StatelessValidation::ValidatePnextStructContents(const char *api_name, const ParameterName &parameter_name,
                                                      const VkBaseOutStructure* header, const char *pnext_vuid,
                                                      bool is_physdev_api, bool is_const_param) const {
    bool skip = false;
    switch(header->sType) {
''')

        # Generate the struct member checking code from the captured data
        for struct in self.vk.structs.values():
            # The string returned will be nested in an if check for a NULL pointer, so needs its indent incremented
            lines = self.genFuncBody(self.vk.structs[struct.name].members, '{funcName}', '{valuePrefix}', '{displayNamePrefix}', struct.name, False)
            if lines:
                self.validatedStructs[struct.name] = lines

        # Do some processing here to extract data from validatedstructs...
        for struct in [x for x in self.vk.structs.values() if x.extends]:
            out.extend([f'#ifdef {struct.protect}\n'] if struct.protect else [])

            postProcSpec = {}
            postProcSpec['ppp'] = '{postProcPrefix}'
            postProcSpec['pps'] = '{postProcSuffix}'
            postProcSpec['ppi'] = '{postProcInsert}'

            pnext_case = '\n'
            pnext_check = ''

            pnext_case += f'        // Validation code for {struct.name} structure members\n'
            pnext_case += f'        case {struct.sType}: {{ // Covers VUID-{struct.name}-sType-sType\n'

            if struct.sType and struct.version and all(not x.promotedTo for x in struct.extensions):
                pnext_check += '            if (api_version < %s) {\n' % struct.version.nameApi
                pnext_check += '                skip |= LogError(\n'
                pnext_check += '                           instance, pnext_vuid,\n'
                pnext_check += '                           "%%s: Includes a pNext pointer (%%s) to a VkStructureType (%s) which was added in %s but the "\n' % (struct.sType, struct.version.nameApi)
                pnext_check += '                           "current effective API version is %s.",\n'
                pnext_check += '                           api_name, parameter_name.get_name().c_str(), StringAPIVersion(api_version).c_str());\n'
                pnext_check += '            }\n'

            if struct.sType in stype_version_dict.keys():
                ext_name = stype_version_dict[struct.sType]
                # Dependent on enabled extension
                extension = self.vk.extensions[ext_name]
                pnext_check += '            if (is_const_param) {\n'
                if extension.device:
                    pnext_check += f'                if ((is_physdev_api && !SupportedByPdev(physical_device, {extension.nameString})) || (!is_physdev_api && !IsExtEnabled(device_extensions.{extension.name.lower()}))) {{\n'
                else:
                    pnext_check += '                if (!instance_extensions.%s) {\n' % (extension.name.lower())
                pnext_check += '                        skip |= LogError(\n'
                pnext_check += '                               instance, pnext_vuid,\n'
                pnext_check += '                               "%%s: Includes a pNext pointer (%%s) to a VkStructureType (%s), but its parent extension "\n' % struct.sType
                pnext_check += '                               "%s has not been enabled.",\n' % extension.name
                pnext_check += '                               api_name, parameter_name.get_name().c_str());\n'
                pnext_check += '                }\n'
                pnext_check += '            }\n'
                pnext_check += '\n'

            expr = self.expandStructCode(struct.name, struct.name, 'structure->', '', '                ', [], postProcSpec)
            struct_validation_source = self.ScrubStructCode(expr)
            if struct_validation_source != '':
                pnext_check += '            if (is_const_param) {\n'
                struct_validation_source = '                %s *structure = (%s *) header;\n' % (struct.name, struct.name) + struct_validation_source
                struct_validation_source += '            }\n'
            pnext_case += '%s%s' % (pnext_check, struct_validation_source)
            pnext_case += '        } break;\n'
            # Skip functions containing no validation
            if struct_validation_source or pnext_check != '':
                out.append(pnext_case)
            else:
                out.append('\n        // No Validation code for %s structure members  -- Covers VUID-%s-sType-sType\n' % (struct.name, struct.name))
            out.extend([f'#endif // {struct.protect}\n'] if struct.protect else [])
        out.append('''        default:
            skip = false;
    }
    return skip;
}

''')

        # Generate the command parameter checking code from the captured data
        indent = self.incIndent(None)
        for command in [x for x in self.vk.commands.values() if x.name not in self.blacklist]:
            out.extend([f'#ifdef {command.protect}\n'] if command.protect else [])
            # Skip first parameter if it is a dispatch handle (everything except vkCreateInstance)
            startIndex = 0 if command.name == 'vkCreateInstance' else 1
            if 'DestroySurfaceKH' in command.name:
                print("A")
            lines = self.genFuncBody(command.params[startIndex:], command.name, '', '', None, isPhysDevice = command.params[0].type == 'VkPhysicalDevice')
            # Cannot validate extension dependencies for device extension APIs having a physical device as their dispatchable object
            if (command.name in self.required_extension_expressions) and (not any(x.device for x in command.extensions) or command.params[0].type != 'VkPhysicalDevice'):
                for expr in self.required_extension_expressions[command.name]:
                    contains_or = len([c for v in expr for c in v if c == ',']) > 0
                    def ext_fn(x):
                        ex_name = x.lower()
                        instance = command.instance
                        if 'VK_VERSION_' in x:
                            ex_name = f'vk_feature_{ex_name[3:]}'
                            instance = True
                        if instance:
                            return f'instance_extensions.{ex_name}'
                        else:
                            return f'IsExtEnabled(device_extensions.{ex_name})'
                    if contains_or:
                        expr_str = exprToCpp(expr, ext_fn)
                        name_str = exprToCpp(expr)
                        lines.insert(0, f'if (!({expr_str})) skip |= OutputExtensionError("{command.name}", "{name_str}");\n')
                    else:
                        for ext in exprValues(expr):
                            if command.instance:
                                lines.insert(0, f'if (!{ext_fn(ext)}) skip |= OutputExtensionError("{command.name}", "{ext}");\n')
                            else:
                                lines.insert(0, f'if (!{ext_fn(ext)}) skip |= OutputExtensionError("{command.name}", "{ext}");\n')
            if lines:
                prototype = command.cPrototype[:-1].split('\n')
                prototype = '\n'.join(prototype)
                prototype += ' const {\n'
                prototype = prototype.split('VKAPI_CALL vk')[1]
                out.append('bool StatelessValidation::PreCallValidate' + prototype)
                out.append('%sbool skip = false;\n' % indent)
                if command.instance and command.version:
                    out.append('%s if (CheckPromotedApiAgainstVulkanVersion(%s, "%s", %s)) return true;\n' % (indent, command.params[0].name, command.name, command.version.nameApi))
                for line in lines:
                    if type(line) is list:
                        for sub in line:
                            out.append(indent + sub)
                    else:
                        out.append(indent + line)
                # Insert call to custom-written function if present
                if command.name in self.functions_with_manual_checks:
                    # Generate parameter list for manual fcn and down-chain calls
                    params_text = ''
                    for param in command.params:
                        params_text += '%s, ' % param.name
                    params_text = params_text[:-2] + ');\n'
                    out.append('    if (!skip) skip |= manual_PreCallValidate'+ command.name[2:] + '(' + params_text)
                out.append('%sreturn skip;\n' % indent)
                out.append('}\n')
                out.extend([f'#endif // {command.protect}\n'] if command.protect else [])
                out.append('\n')

        self.write("".join(out))

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

    # TODO - Improve this to not be so complex
    def beginFeature(self, interface, emit):
        # Start processing in superclass
        BaseGenerator.beginFeature(self, interface, emit)
        # Get base list of extension dependencies for all items in this extension
        base_required_extension_expressions = []
        base_req_exts = []
        if "VK_VERSION_1" not in self.featureName:
            # Save Name Define to get correct enable name later
            base_req_exts.append(self.featureName)
        # Add any defined extension dependencies to the base dependency list for this extension
        requires = interface.get('depends')
        if requires is not None:
            base_req_exts.append(f'({requires})')
        if len(base_req_exts) > 0:
            # This is a work around for https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5372
            expr_str = re.sub(r',VK_VERSION_1_\d+', '', '+'.join(base_req_exts))
            base_required_extension_expressions.append(parseExpr(expr_str))
        # Build dictionary of extension dependencies for each item in this extension
        for require_element in interface.findall('require'):
            # Copy base extension dependency list
            required_extension_expressions = list(base_required_extension_expressions)
            # Add any additional extension dependencies specified in this require block
            additional_extensions = require_element.get('depends')
            # require tags must split here by '+' as it is an AND operation according registry.adoc:
            #    == Attributes of tag:require tags
            #    attr:extension - optional, and only for tag:require tags.
            #    String containing one or more API extension names separated by , or +.
            #    Interfaces in the tag are only required if enabled extensions satisfy
            #    the logical expression in the string, where , is interpreted as a
            #    logical OR and '+' as a logical AND.
            if additional_extensions:
                required_extension_expressions.append(parseExpr(additional_extensions))
            # Save full extension list for all named items
            for element in require_element.findall('*[@name]'):
                self.required_extension_expressions[element.get('name')] = required_extension_expressions

    def genType(self, typeinfo, name, alias):
        BaseGenerator.genType(self, typeinfo, name, alias)
        if (typeinfo.elem.get('category') == 'bitmask' and not alias):
            self.flags.add(name)

    def genGroup(self, groupinfo, groupName, alias):
        BaseGenerator.genGroup(self, groupinfo, groupName, alias)
        if 'FlagBits' in groupName and groupName != 'VkStructureType':
            bits = []
            for elem in groupinfo.elem.findall('enum'):
                if elem.get('supported') != 'disabled' and elem.get('alias') is None:
                    bits.append(elem.get('name'))
            if bits:
                self.flagBits[groupName] = bits

    def isHandleOptional(self, member: Member, lengthMember: Member) -> bool :
        # Simple, if it's optional, return true
        if member.optional or member.optionalPointer:
            return True
        # If no validity is being generated, it usually means that validity is complex and not absolute, so let's say yes.
        if member.noAutoValidity:
            return True
        # If the parameter is an array and we haven't already returned, find out if any of the len parameters are optional
        if lengthMember and lengthMember.optional:
            return True
        return

    # Generate code to check for a specific condition before executing validation code
    def genConditionalCall(self, prefix, condition, exprs):
        checkedExpr = []
        localIndent = ''
        checkedExpr.append(localIndent + 'if ({})\n'.format(condition))
        checkedExpr.append(localIndent + '{\n')
        localIndent = self.incIndent(localIndent)
        for expr in exprs:
            checkedExpr.append(localIndent + expr)
        localIndent = self.decIndent(localIndent)
        checkedExpr.append(localIndent + '}\n')
        return [checkedExpr]

    # Get VUID identifier from implicit VUID tag
    def GetVuid(self, name, suffix):
        vuid_string = 'VUID-%s-%s' % (name, suffix)
        vuid = "kVUIDUndefined"
        if '->' in vuid_string:
           return vuid
        if vuid_string in self.valid_vuids:
            vuid = "\"%s\"" % vuid_string
        elif name in self.vk.commands:
            # Only commands have alias to worry about
            alias_string = 'VUID-%s-%s' % (self.vk.commands[name].alias, suffix)
            if alias_string in self.valid_vuids:
                vuid = "\"%s\"" % alias_string
        return vuid

    # Generate the sType check string
    def makeStructTypeCheck(self, prefix, member: Member, lengthMember: Member, valueRequired, lenValueRequired, lenPtrRequired, funcPrintName, lenPrintName, valuePrintName, postProcSpec, structTypeName):
        checkExpr = []
        struct = self.vk.structs[member.type]
        vuid_name = structTypeName if structTypeName is not None else funcPrintName
        stypeVUID = self.GetVuid(member.type, "sType-sType")
        paramVUID = self.GetVuid(vuid_name, "%s-parameter" % member.name)

        if lengthMember:
            count_required_vuid = self.GetVuid(vuid_name, "%s-arraylength" % member.length)

            # This is an array of struct pointers
            if member.cDeclaration.count('*') == 2:
                checkExpr.append('skip |= ValidateStructPointerTypeArray("{}", {ppp}"{ldn}"{pps}, {ppp}"{dn}"{pps}, "{sv}", {pf}{ln}, {pf}{vn}, {sv}, {}, {}, {}, {}, {});\n'.format(
                    funcPrintName, lenValueRequired, valueRequired, stypeVUID, paramVUID, count_required_vuid, ln=lengthMember.name, ldn=lenPrintName, dn=valuePrintName, vn=member.name, sv=struct.sType, pf=prefix, **postProcSpec))
            # This is an array with a pointer to a count value
            elif lengthMember.pointer:
                # When the length parameter is a pointer, there is an extra Boolean parameter in the function call to indicate if it is required
                checkExpr.append('skip |= ValidateStructTypeArray("{}", {ppp}"{ldn}"{pps}, {ppp}"{dn}"{pps}, "{sv}", {pf}{ln}, {pf}{vn}, {sv}, {}, {}, {}, {}, {}, {});\n'.format(
                    funcPrintName, lenPtrRequired, lenValueRequired, valueRequired, stypeVUID, paramVUID, count_required_vuid, ln=member.length, ldn=lenPrintName, dn=valuePrintName, vn=member.name, sv=struct.sType, pf=prefix, **postProcSpec))
            # This is an array with an integer count value
            else:
                checkExpr.append('skip |= ValidateStructTypeArray("{}", {ppp}"{ldn}"{pps}, {ppp}"{dn}"{pps}, "{sv}", {pf}{ln}, {pf}{vn}, {sv}, {}, {}, {}, {}, {});\n'.format(
                    funcPrintName, lenValueRequired, valueRequired, stypeVUID, paramVUID, count_required_vuid, ln=member.length, ldn=lenPrintName, dn=valuePrintName, vn=member.name, sv=struct.sType, pf=prefix, **postProcSpec))
        # This is an individual struct
        else:
            checkExpr.append('skip |= ValidateStructType("{}", {ppp}"{}"{pps}, "{sv}", {}{vn}, {sv}, {}, {}, {});\n'.format(
                funcPrintName, valuePrintName, prefix, valueRequired, paramVUID, stypeVUID, vn=member.name, sv=struct.sType, vt=member.type, **postProcSpec))
        return checkExpr

    # Generate the handle check string
    def makeHandleCheck(self, prefix, member: Member, lengthMember: Member, valueRequired, lenValueRequired, funcPrintName, lenPrintName, valuePrintName, postProcSpec):
        checkExpr = []
        if lengthMember:
            if lengthMember.pointer:
                # This is assumed to be an output array with a pointer to a count value
                raise Exception('Unsupported parameter validation case: Output handle array elements are not NULL checked')
            else:
                count_required_vuid = self.GetVuid(funcPrintName, "%s-arraylength" % (member.length))
                # This is an array with an integer count value
                checkExpr.append('skip |= ValidateHandleArray("{}", {ppp}"{ldn}"{pps}, {ppp}"{dn}"{pps}, {pf}{ln}, {pf}{vn}, {}, {}, {});\n'.format(
                    funcPrintName, lenValueRequired, valueRequired, count_required_vuid, ln=member.length, ldn=lenPrintName, dn=valuePrintName, vn=member.name, pf=prefix, **postProcSpec))
        else:
            # This is assumed to be an output handle pointer
            raise Exception('Unsupported parameter validation case: Output handles are not NULL checked')
        return checkExpr

    # Generate check string for an array of VkFlags values
    def makeFlagsArrayCheck(self, prefix, member: Member, lenValueRequired, callerName, lenPrintName, valuePrintName, postProcSpec):
        checkExpr = []
        flagBitsName = member.type.replace('Flags', 'FlagBits')
        if not flagBitsName in self.vk.bitmasks:
            raise Exception('Unsupported parameter validation case: array of reserved VkFlags')
        else:
            allFlags = 'All' + flagBitsName
            array_required_vuid = self.GetVuid(callerName, "%s-parameter" % (member.name))
            checkExpr.append('skip |= ValidateFlagsArray("{}", {ppp}"{}"{pps}, {ppp}"{}"{pps}, "{}", {}, {pf}{}, {pf}{}, {}, {});\n'.format(callerName, lenPrintName, valuePrintName, flagBitsName, allFlags, member.length, member.name, lenValueRequired, array_required_vuid, pf=prefix, **postProcSpec))
        return checkExpr

    # Generate pNext check string
    def makeStructNextCheck(self, prefix, member: Member, funcPrintName, valuePrintName, postProcSpec, structTypeName):
        checkExpr = []
        # Generate an array of acceptable VkStructureType values for pNext
        extStructCount = 0
        extStructVar = 'nullptr'
        extStructNames = 'nullptr'
        extStructData = 'nullptr'
        pNextVuid = self.GetVuid(structTypeName, "pNext-pNext")
        sTypeVuid = self.GetVuid(structTypeName, "sType-unique")
        if self.vk.structs[structTypeName].extendedBy:
            extStructVar = 'allowed_structs_{}'.format(structTypeName)
            extStructCount = '{}.size()'.format(extStructVar)
            extStructData = '{}.data()'.format(extStructVar)
            extraStype = ', VK_STRUCTURE_TYPE_INSTANCE_LAYER_SETTINGS_EXT' if structTypeName == 'VkInstanceCreateInfo' else ''
            extraStruct = ', VkInstanceLayerSettingsEXT' if structTypeName == 'VkInstanceCreateInfo' else ''
            extStructNames = '"' + ', '.join([x.name for x  in self.vk.structs[structTypeName].extendedBy]) + extraStruct + '"'
            checkExpr.append('constexpr std::array {} = {{ {}{} }};\n'.format(extStructVar, ', '.join([x.sType for x in self.vk.structs[structTypeName].extendedBy]), extraStype))
        checkExpr.append('skip |= ValidateStructPnext("{}", {ppp}"{}"{pps}, {}, {}{}, {}, {}, GeneratedVulkanHeaderVersion, {}, {});\n'.format(
            funcPrintName, valuePrintName, extStructNames, prefix, member.name, extStructCount, extStructData, pNextVuid, sTypeVuid, **postProcSpec))
        return checkExpr

    # Generate the pointer check string
    def makePointerCheck(self, prefix,  member: Member, lengthMember: Member, valueRequired, lenValueRequired, lenPtrRequired, funcPrintName, lenPrintName, valuePrintName, postProcSpec, structTypeName):
        checkExpr = []
        vuid_tag_name = structTypeName if structTypeName is not None else funcPrintName
        if lengthMember:
            length_deref = '->' in member.length
            count_required_vuid = self.GetVuid(vuid_tag_name, "%s-arraylength" % (member.length))
            array_required_vuid = self.GetVuid(vuid_tag_name, "%s-parameter" % (member.name))
            # TODO: Remove workaround for missing optional tag in vk.xml
            if array_required_vuid == '"VUID-VkFramebufferCreateInfo-pAttachments-parameter"':
                return []
            # This is an array with a pointer to a count value
            if lengthMember.pointer and not length_deref:
                # If count and array parameters are optional, there will be no validation
                if valueRequired == 'true' or lenPtrRequired == 'true' or lenValueRequired == 'true':
                    # When the length parameter is a pointer, there is an extra Boolean parameter in the function call to indicate if it is required
                    checkExpr.append('skip |= ValidateArray("{}", {ppp}"{ldn}"{pps}, {ppp}"{dn}"{pps}, {pf}{ln}, &{pf}{vn}, {}, {}, {}, {}, {});\n'.format(
                        funcPrintName, lenPtrRequired, lenValueRequired, valueRequired, count_required_vuid, array_required_vuid, ln=member.length, ldn=lenPrintName, dn=valuePrintName, vn=member.name, pf=prefix, **postProcSpec))
            # This is an array with an integer count value
            else:
                # If count and array parameters are optional, there will be no validation
                if valueRequired == 'true' or lenValueRequired == 'true':
                    if member.type != 'char':
                        # A valid VU can't use '->' in the middle so the generated VUID from the spec uses '::' instead
                        count_required_vuid = self.GetVuid(vuid_tag_name, "%s-arraylength" % (member.length.replace('->', '::')))
                        checkExpr.append('skip |= ValidateArray("{}", {ppp}"{ldn}"{pps}, {ppp}"{dn}"{pps}, {pf}{ln}, &{pf}{vn}, {}, {}, {}, {});\n'.format(
                            funcPrintName, lenValueRequired, valueRequired, count_required_vuid, array_required_vuid, ln=member.length, ldn=lenPrintName, dn=valuePrintName, vn=member.name, pf=prefix, **postProcSpec))
                    else:
                        # Arrays of strings receive special processing
                        checkExpr.append('skip |= ValidateStringArray("{}", {ppp}"{ldn}"{pps}, {ppp}"{dn}"{pps}, {pf}{ln}, {pf}{vn}, {}, {}, {}, {});\n'.format(
                            funcPrintName, lenValueRequired, valueRequired, count_required_vuid, array_required_vuid, ln=member.length, ldn=lenPrintName, dn=valuePrintName, vn=member.name, pf=prefix, **postProcSpec))
            if checkExpr and lengthMember and length_deref and member.length.count('->'):
                # Add checks to ensure the validation call does not dereference a NULL pointer to obtain the count
                count = member.length.count('->')
                checkedExpr = []
                localIndent = ''
                elements = member.length.split('->')
                # Open the if expression blocks
                for i in range(0, count):
                    checkedExpr.append(localIndent + 'if ({} != nullptr) {{\n'.format('->'.join(elements[0:i+1])))
                    localIndent = self.incIndent(localIndent)
                # Add the validation expression
                for expr in checkExpr:
                    checkedExpr.append(localIndent + expr)
                # Close the if blocks
                for i in range(0, count):
                    localIndent = self.decIndent(localIndent)
                    checkedExpr.append(localIndent + '}\n')
                checkExpr = [checkedExpr]
        # This is an individual struct that is not allowed to be NULL
        elif not (member.optional or member.optionalPointer or member.staticArray):
            # Function pointers need a reinterpret_cast to void*
            ptr_required_vuid = self.GetVuid(vuid_tag_name, "%s-parameter" % (member.name))
            if member.type.startswith('PFN_'):
                allocator_dict = {'pfnAllocation': '"VUID-VkAllocationCallbacks-pfnAllocation-00632"',
                                  'pfnReallocation': '"VUID-VkAllocationCallbacks-pfnReallocation-00633"',
                                  'pfnFree': '"VUID-VkAllocationCallbacks-pfnFree-00634"',
                                 }
                vuid = allocator_dict.get(member.name)
                if vuid is not None:
                    ptr_required_vuid = vuid
                checkExpr.append('skip |= ValidateRequiredPointer("{}", {ppp}"{}"{pps}, reinterpret_cast<const void*>({}{}), {});\n'.format(funcPrintName, valuePrintName, prefix, member.name, ptr_required_vuid, **postProcSpec))
            else:
                checkExpr.append('skip |= ValidateRequiredPointer("{}", {ppp}"{}"{pps}, {}{}, {});\n'.format(funcPrintName, valuePrintName, prefix, member.name, ptr_required_vuid, **postProcSpec))
        else:
            # Special case for optional internal allocation function pointers.
            if (member.type, member.name) == ('PFN_vkInternalAllocationNotification', 'pfnInternalAllocation'):
                checkExpr.extend(self.internalAllocationCheck(funcPrintName, prefix, member.name, 'pfnInternalFree', postProcSpec))
            elif (member.type, member.name) == ('PFN_vkInternalFreeNotification', 'pfnInternalFree'):
                checkExpr.extend(self.internalAllocationCheck(funcPrintName, prefix, member.name, 'pfnInternalAllocation', postProcSpec))
        return checkExpr

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

    # Process struct pointer/array validation code, performing name substitution if required
    def expandStructPointerCode(self, prefix, member: Member, lengthMember: Member, funcName, valueDisplayName, postProcSpec):
        expr = []
        expr.append('if ({}{} != nullptr)\n'.format(prefix, member.name))
        expr.append('{')
        indent = self.incIndent(None)
        if lengthMember:
            # Need to process all elements in the array
            length = member.length.split(',')[0]
            indexName = length.replace('Count', 'Index')
            expr[-1] += '\n'
            if lengthMember.pointer:
                # If the length value is a pointer, de-reference it for the count.
                expr.append(indent + 'for (uint32_t {iname} = 0; {iname} < *{}{}; ++{iname})\n'.format(prefix, length, iname=indexName))
            else:
                expr.append(indent + 'for (uint32_t {iname} = 0; {iname} < {}{}; ++{iname})\n'.format(prefix, length, iname=indexName))
            expr.append(indent + '{')
            indent = self.incIndent(indent)
            # Prefix for value name to display in error message
            if member.cDeclaration.count('*') == 2:
                memberNamePrefix = '{}{}[{}]->'.format(prefix, member.name, indexName)
                memberDisplayNamePrefix = ('{}[%i]->'.format(valueDisplayName), indexName)
            else:
                memberNamePrefix = '{}{}[{}].'.format(prefix, member.name, indexName)
                memberDisplayNamePrefix = ('{}[%i].'.format(valueDisplayName), indexName)
        else:
            memberNamePrefix = '{}{}->'.format(prefix, member.name)
            memberDisplayNamePrefix = '{}->'.format(valueDisplayName)
        # Expand the struct validation lines
        expr = self.expandStructCode(member.type, funcName, memberNamePrefix, memberDisplayNamePrefix, indent, expr, postProcSpec)
        if lengthMember:
            # Close if and for scopes
            indent = self.decIndent(indent)
            expr.append(indent + '}\n')
        expr.append('}\n')
        return expr

    # Generate the parameter checking code
    def genFuncBody(self, members: List[Member], funcName, valuePrefix, displayNamePrefix, structTypeName, isPhysDevice):
        struct = self.vk.structs[structTypeName] if structTypeName in self.vk.structs else None
        lines = []    # Generated lines of code
        duplicateCountVuid = [] # prevent duplicate VUs being generated

        # TODO Using a regex in this context is not ideal. Would be nicer if usedLines were a list of objects with "settings" (such as "isPhysDevice")
        validate_pnext_rx = re.compile(r'(.*ValidateStructPnext\(.*)(\).*\n*)', re.M)

        # Returnedonly structs should have most of their members ignored -- on entry, we only care about validating the sType and
        # pNext members. Everything else will be overwritten by the callee.
        for member in [x for x in members if not struct or not struct.returnedOnly or (x.name in ('sType', 'pNext'))]:
            usedLines = []
            lengthMember = None
            condition = None
            #
            # Prefix and suffix for post processing of parameter names for struct members.  Arrays of structures need special processing to include the array index in the full parameter name.
            postProcSpec = {}
            postProcSpec['ppp'] = '' if not structTypeName else '{postProcPrefix}'
            postProcSpec['pps'] = '' if not structTypeName else '{postProcSuffix}'
            postProcSpec['ppi'] = '' if not structTypeName else '{postProcInsert}'
            #
            # Generate the full name of the value, which will be printed in the error message, by adding the variable prefix to the value name
            valueDisplayName = '{}{}'.format(displayNamePrefix, member.name)
            #
            # Check for NULL pointers, ignore the in-out count parameters that
            # will be validated with their associated array
            if (member.pointer or member.staticArray) and not [x for x in members if x.length and member.name == x.length]:
                # Parameters for function argument generation
                req = 'true'    # Parameter cannot be NULL
                cpReq = 'true'  # Count pointer cannot be NULL
                cvReq = 'true'  # Count value cannot be 0
                lenDisplayName = None # Name of length parameter to print with validation messages; parameter name with prefix applied
                countRequiredVuid = None # If there is a count required VUID to check
                # Generate required/optional parameter strings for the pointer and count values
                if member.optional or member.optionalPointer:
                    req = 'false'
                if member.length:
                    # The parameter is an array with an explicit count parameter
                    # Find a named parameter in a parameter list
                    lengthMember = next((x for x in members if x.name == member.length), None)

                    # First check if any element of params matches length exactly
                    if not lengthMember:
                        # Otherwise, look for any elements of params that appear within length
                        candidates = [p for p in members if re.search(r'\b{}\b'.format(p.name), member.length)]
                        # 0 or 1 matches are expected, >1 would require a special case and/or explicit validation
                        if len(candidates) == 0:
                            lengthMember = None
                        elif len(candidates) == 1:
                            lengthMember = candidates[0]

                    if lengthMember:
                        lenDisplayName = member.length.split(',')[0].replace(lengthMember.name, displayNamePrefix + lengthMember.name)
                        if lengthMember.pointer:
                            cpReq = 'false' if lengthMember.optional else cpReq
                            cvReq = 'false' if lengthMember.optionalPointer else cvReq
                            # In case of count as field in another struct, look up field to see if count is optional.
                            len_deref = member.length.split('->')
                            if len(len_deref) == 2:
                                lenMembers = next((x.members for x in self.vk.structs.values() if x.name == lengthMember.type), None)
                                if lenMembers and next((x for x in lenMembers if x.name == len_deref[1] and x.optional), None):
                                    cvReq = 'false'
                        else:
                            vuidNameTag = structTypeName if structTypeName is not None else funcName
                            vuidName = self.GetVuid(vuidNameTag, "%s-arraylength" % (lengthMember.name))
                            # This VUID is considered special, as it is the only one whose names ends in "-arraylength" but has special conditions allowing bindingCount to be 0.
                            arrayVuidExceptions = ["\"VUID-vkCmdBindVertexBuffers2-bindingCount-arraylength\""]
                            if vuidName in arrayVuidExceptions:
                                continue
                            if lengthMember.optional:
                                cvReq = 'false'
                            elif member.noAutoValidity:
                                # Handle edge case where XML expresses a non-optional non-pointer value length with noautovalidity
                                # ex: <param noautovalidity="true"len="commandBufferCount">
                                vuidNameTag = structTypeName if structTypeName is not None else funcName
                                countRequiredVuid = self.GetVuid(vuidNameTag, "%s-arraylength" % (lengthMember.name))
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
                if member.noAutoValidity and member.type not in AllocatorFunctions and not countRequiredVuid:
                    # Log a diagnostic message when validation cannot be automatically generated and must be implemented manually
                    self.logMsg('diag', 'ParameterValidation: No validation for {} {}'.format(structTypeName if structTypeName else funcName, member.name))
                elif countRequiredVuid:
                    usedLines.append('skip |= ValidateArray("{}", {ppp}"{ldn}"{pps}, "", {pf}{ln}, &{pf}{vn}, true, false, {}, kVUIDUndefined);\n'.format(
                        funcName, countRequiredVuid, pf=valuePrefix, ldn=lenDisplayName, ln=member.length, vn=member.name, **postProcSpec))
                else:
                    if member.type in self.vk.structs and self.vk.structs[member.type].sType:
                        # If this is a pointer to a struct with an sType field, verify the type
                        usedLines += self.makeStructTypeCheck(valuePrefix, member, lengthMember, req, cvReq, cpReq, funcName, lenDisplayName, valueDisplayName, postProcSpec, structTypeName)
                    # If this is an input handle array that is not allowed to contain NULL handles, verify that none of the handles are VK_NULL_HANDLE
                    elif member.type in self.vk.handles and member.const and not self.isHandleOptional(member, lengthMember):
                        usedLines += self.makeHandleCheck(valuePrefix, member, lengthMember, req, cvReq, funcName, lenDisplayName, valueDisplayName, postProcSpec)
                    elif member.type in self.flags and member.const:
                        callerName = structTypeName if structTypeName else funcName
                        usedLines += self.makeFlagsArrayCheck(valuePrefix, member, cvReq, callerName, lenDisplayName, valueDisplayName, postProcSpec)
                    elif member.type == 'VkBool32' and member.const:
                        usedLines.append('skip |= ValidateBool32Array("{}", {ppp}"{}"{pps}, {ppp}"{}"{pps}, {pf}{}, {pf}{}, {}, {});\n'.format(funcName, lenDisplayName, valueDisplayName, member.length, member.name, cvReq, req, pf=valuePrefix, **postProcSpec))
                    elif member.type in self.vk.enums and member.const:
                        prefix = postProcSpec.get('ppp', '')
                        suffix = postProcSpec.get('pps', '')
                        usedLines.append(f'skip |= ValidateRangedEnumArray("{funcName}", {prefix}"{lenDisplayName}"{suffix}, {prefix}"{valueDisplayName}"{suffix}, "{member.type}", {valuePrefix}{member.length}, {valuePrefix}{member.name}, {cvReq}, {req});\n')
                    elif member.name == 'pNext':
                        usedLines += self.makeStructNextCheck(valuePrefix, member, funcName, valueDisplayName, postProcSpec, structTypeName)
                    else:
                        usedLines += self.makePointerCheck(valuePrefix, member, lengthMember, req, cvReq, cpReq, funcName, lenDisplayName, valueDisplayName, postProcSpec, structTypeName)
                    # If this is a pointer to a struct (input), see if it contains members that need to be checked
                    if member.type in self.validatedStructs:
                        if member.const:
                            usedLines.append(self.expandStructPointerCode(valuePrefix, member, lengthMember, funcName, valueDisplayName, postProcSpec))
                        elif self.vk.structs[member.type].returnedOnly:
                            usedLines.append(self.expandStructPointerCode(valuePrefix, member, lengthMember, funcName, valueDisplayName, postProcSpec))

                    is_const_str = 'true' if member.const else 'false'
                    isPhysDevice_str = 'true' if isPhysDevice else 'false'
                    for setter, _, elem in multi_string_iter(usedLines):
                        elem = re.sub(r', (true|false)', '', elem)
                        m = validate_pnext_rx.match(elem)
                        if m is not None:
                            setter(f'{m.group(1)}, {isPhysDevice_str}, {is_const_str}{m.group(2)}')

            # Non-pointer types
            else:
                # The parameter will not be processes when tagged as 'noautovalidity'
                # For the struct case, the struct type will not be validated, but any
                # members not tagged as 'noautovalidity' will be validated
                if member.noAutoValidity:
                    # Log a diagnostic message when validation cannot be automatically generated and must be implemented manually
                    self.logMsg('diag', 'ParameterValidation: No validation for {} {}'.format(structTypeName if structTypeName else funcName, member.name))
                else:
                    vuid_name_tag = structTypeName if structTypeName is not None else funcName
                    if member.type in self.vk.structs and self.vk.structs[member.type].sType:
                        vuid = self.GetVuid(member.type, "sType-sType")
                        usedLines.append('skip |= ValidateStructType("{}", {ppp}"{}"{pps}, "{sv}", &({}{vn}), {sv}, false, kVUIDUndefined, {});\n'.format(
                            funcName, valueDisplayName, valuePrefix, vuid, vn=member.name, sv=self.vk.structs[member.type].sType, vt=member.type, **postProcSpec))
                    elif member.type in self.vk.handles:
                        if not member.optional:
                            usedLines.append('skip |= ValidateRequiredHandle("{}", {ppp}"{}"{pps}, {}{});\n'.format(funcName, valueDisplayName, valuePrefix, member.name, **postProcSpec))
                    elif member.type in self.flags and member.type.replace('Flags', 'FlagBits') not in self.flagBits:
                        vuid = self.GetVuid(vuid_name_tag, "%s-zerobitmask" % (member.name))
                        usedLines.append('skip |= ValidateReservedFlags("{}", {ppp}"{}"{pps}, {pf}{}, {});\n'.format(funcName, valueDisplayName, member.name, vuid, pf=valuePrefix, **postProcSpec))
                    elif member.type in self.flags or member.type in self.flagBits:
                        if member.type in self.flags:
                            flagBitsName = member.type.replace('Flags', 'FlagBits')
                            flagsType = 'kOptionalFlags' if member.optional else 'kRequiredFlags'
                            invalidVuid = self.GetVuid(vuid_name_tag, "%s-parameter" % (member.name))
                            zeroVuid = self.GetVuid(vuid_name_tag, "%s-requiredbitmask" % (member.name))
                        elif member.type in self.flagBits:
                            flagBitsName = member.type
                            flagsType = 'kOptionalSingleBit' if member.optional else 'kRequiredSingleBit'
                            invalidVuid = self.GetVuid(vuid_name_tag, "%s-parameter" % (member.name))
                            zeroVuid = invalidVuid
                        # Bad workaround, but this whole file will be refactored soon
                        if flagBitsName == 'VkBuildAccelerationStructureFlagBitsNV':
                            flagBitsName = 'VkBuildAccelerationStructureFlagBitsKHR'
                        allFlagsName = 'All' + flagBitsName
                        zeroVuidArg = '' if member.optional else ', ' + zeroVuid
                        condition = [item for item in self.structMemberValidationConditions if (item['struct'] == structTypeName and item['field'] == flagBitsName)]
                        usedLines.append('skip |= ValidateFlags("{}", {ppp}"{}"{pps}, "{}", {}, {pf}{}, {}, {}{});\n'.format(funcName, valueDisplayName, flagBitsName, allFlagsName, member.name, flagsType, invalidVuid, zeroVuidArg, pf=valuePrefix, **postProcSpec))
                    elif member.type == 'VkBool32':
                        usedLines.append('skip |= ValidateBool32("{}", {ppp}"{}"{pps}, {}{});\n'.format(funcName, valueDisplayName, valuePrefix, member.name, **postProcSpec))
                    elif member.type in self.vk.enums and member.type != 'VkStructureType':
                        vuid = self.GetVuid(vuid_name_tag, "%s-parameter" % (member.name))
                        prefix = postProcSpec.get('ppp', '')
                        suffix = postProcSpec.get('pps', '')
                        usedLines.append(f'skip |= ValidateRangedEnum("{funcName}", {prefix}"{valueDisplayName}"{suffix}, "{member.type}", {valuePrefix}{member.name}, {vuid});\n')
                    # If this is a struct, see if it contains members that need to be checked
                    if member.type in self.validatedStructs:
                        memberNamePrefix = '{}{}.'.format(valuePrefix, member.name)
                        memberDisplayNamePrefix = '{}.'.format(valueDisplayName)
                        usedLines.append(self.expandStructCode(member.type, funcName, memberNamePrefix, memberDisplayNamePrefix, '', [], postProcSpec))
            # Append the parameter check to the function body for the current command
            if usedLines:
                # Apply special conditional checks
                if condition:
                    usedLines = self.genConditionalCall(valuePrefix, condition[0]['condition'], usedLines)
                lines += usedLines
        if not lines:
            lines.append('// No xml-driven validation\n')
        return lines


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