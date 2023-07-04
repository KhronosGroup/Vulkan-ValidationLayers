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

import sys
import os
from generators.generator_utils import (fileIsGeneratedWarning)
from generators.vulkan_object import (Struct, Command, Member)
from generators.base_generator import BaseGenerator
from typing import List

class LayerChassisDispatchOutputGenerator(BaseGenerator):
    def __init__(self,
                 errFile = sys.stderr,
                 warnFile = sys.stderr,
                 diagFile = sys.stdout):
        BaseGenerator.__init__(self, errFile, warnFile, diagFile)

        # Structs that are extending other structs and have NonDispatchable objects in them
        self.ndo_extension_structs = [] # List[Struct]

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

        if self.filename == 'layer_chassis_dispatch.h':
            self.generateHeader()
        elif self.filename == 'layer_chassis_dispatch.cpp':
            self.generateSource()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateHeader(self):
        out = []
        out.append('''
#pragma once

extern bool wrap_handles;

class ValidationObject;
void WrapPnextChainHandles(ValidationObject *layer_data, const void *pNext);

''')
        for command in self.vk.commands.values():
            prototype = command.cPrototype
            prototype = prototype.replace("VKAPI_ATTR ", "")
            prototype = prototype.replace("VKAPI_CALL vk", "Dispatch")
            out.extend([f'#ifdef {command.protect}\n'] if command.protect else [])
            out.append(f'{prototype}\n')
            out.extend([f'#endif // {command.protect}\n'] if command.protect else [])

        self.write("".join(out))

    def generateSource(self):
        out = []
        out.append('''
#include "utils/cast_utils.h"
#include "chassis.h"
#include "layer_chassis_dispatch.h"
#include "vk_safe_struct.h"
#include "state_tracker/pipeline_state.h"

#define DISPATCH_MAX_STACK_ALLOCATIONS 32
''')

        out.append('''

// Unique Objects pNext extension handling function
void WrapPnextChainHandles(ValidationObject *layer_data, const void *pNext) {
    void *cur_pnext = const_cast<void *>(pNext);
    while (cur_pnext != nullptr) {
        VkBaseOutStructure *header = reinterpret_cast<VkBaseOutStructure *>(cur_pnext);

        switch (header->sType) {
''')

        # Construct list of extension structs containing handles
        for struct in [x for x in self.vk.structs.values() if x.sType and x.extendedBy]:
            for extendedStruct in struct.extendedBy:
                if self.containsNonDispatchableObject(extendedStruct) and extendedStruct not in self.ndo_extension_structs:
                    self.ndo_extension_structs.append(extendedStruct)

        for struct in self.ndo_extension_structs:
            out.extend([f'#ifdef {struct.protect}\n'] if struct.protect else [])
            out.append(f'            case {struct.sType}: {{\n')
            out.append(f'                    safe_{struct.name} *safe_struct = reinterpret_cast<safe_{struct.name} *>(cur_pnext);\n')
            if struct.name == 'VkGraphicsPipelineShaderGroupsCreateInfoNV':
                # TODO - Figure out the rule to find this double loop
                out.append('''
                    if (safe_struct->pGroups) {
                        for (uint32_t index0 = 0; index0 < safe_struct->groupCount; ++index0) {
                            if (safe_struct->pGroups[index0].pStages) {
                                for (uint32_t index1 = 0; index1 < safe_struct->pGroups[index0].stageCount; ++index1) {
                                    if (safe_struct->pGroups[index0].pStages[index1].module) {
                                        safe_struct->pGroups[index0].pStages[index1].module = layer_data->Unwrap(safe_struct->pGroups[index0].pStages[index1].module);
                                    }
                                }
                            }
                        }
                    }\n''')

            for member in [x for x in struct.members if x.type in self.vk.handles and not self.vk.handles[x.type].dispatchable]:
                if member.pointer:
                    out.append(f'''                    if (safe_struct->{member.name}) {{
                        for (uint32_t index0 = 0; index0 < safe_struct->{member.length}; ++index0) {{
                            safe_struct->{member.name}[index0] = layer_data->Unwrap(safe_struct->{member.name}[index0]);
                        }}
                    }}\n''')
                else:
                    out.append(f'''                    if (safe_struct->{member.name}) {{
                        safe_struct->{member.name} = layer_data->Unwrap(safe_struct->{member.name});
                    }}\n''')
            out.append('                } break;\n\n')
            out.extend([f'#endif // {struct.protect}\n'] if struct.protect else [])
        out.append('''            default:
                break;
        }

        // Process the next structure in the chain
        cur_pnext = header->pNext;
    }
}
''')

        # Has own functions in layer_chassis_dispatch_manual.cpp
        manualOverride = [
            'vkCreateInstance',
            'vkDestroyInstance',
            'vkCreateDevice',
            'vkDestroyDevice',
            'vkCreateSwapchainKHR',
            'vkCreateSharedSwapchainsKHR',
            'vkGetSwapchainImagesKHR',
            'vkDestroySwapchainKHR',
            'vkQueuePresentKHR',
            'vkCreateGraphicsPipelines',
            'vkCreateComputePipelines',
            'vkCreateRayTracingPipelinesNV',
            'vkCreateRayTracingPipelinesKHR',
            'vkResetDescriptorPool',
            'vkDestroyDescriptorPool',
            'vkAllocateDescriptorSets',
            'vkFreeDescriptorSets',
            'vkCreateDescriptorUpdateTemplate',
            'vkCreateDescriptorUpdateTemplateKHR',
            'vkDestroyDescriptorUpdateTemplate',
            'vkDestroyDescriptorUpdateTemplateKHR',
            'vkUpdateDescriptorSetWithTemplate',
            'vkUpdateDescriptorSetWithTemplateKHR',
            'vkCmdPushDescriptorSetWithTemplateKHR',
            'vkDebugMarkerSetObjectTagEXT',
            'vkDebugMarkerSetObjectNameEXT',
            'vkCreateRenderPass',
            'vkCreateRenderPass2KHR',
            'vkCreateRenderPass2',
            'vkDestroyRenderPass',
            'vkSetDebugUtilsObjectNameEXT',
            'vkSetDebugUtilsObjectTagEXT',
            'vkGetPhysicalDeviceDisplayPropertiesKHR',
            'vkGetPhysicalDeviceDisplayProperties2KHR',
            'vkGetPhysicalDeviceDisplayPlanePropertiesKHR',
            'vkGetPhysicalDeviceDisplayPlaneProperties2KHR',
            'vkGetDisplayPlaneSupportedDisplaysKHR',
            'vkGetDisplayModePropertiesKHR',
            'vkGetDisplayModeProperties2KHR',
            'vkEnumerateInstanceExtensionProperties',
            'vkEnumerateInstanceLayerProperties',
            'vkEnumerateDeviceExtensionProperties',
            'vkEnumerateDeviceLayerProperties',
            'vkEnumerateInstanceVersion',
            'vkGetPhysicalDeviceToolPropertiesEXT',
            'vkSetPrivateDataEXT',
            'vkGetPrivateDataEXT',
            'vkDeferredOperationJoinKHR',
            'vkGetDeferredOperationResultKHR',
            'vkSetPrivateData',
            'vkGetPrivateData',
            'vkBuildAccelerationStructuresKHR',
            'vkGetDescriptorEXT',
            'vkReleasePerformanceConfigurationINTEL',
            # These are for special-casing the pInheritanceInfo issue (must be ignored for primary CBs)
            'vkAllocateCommandBuffers',
            'vkFreeCommandBuffers',
            'vkDestroyCommandPool',
            'vkBeginCommandBuffer',
            'vkGetAccelerationStructureBuildSizesKHR'
        ]

        for command in [x for x in self.vk.commands.values() if x.name not in manualOverride]:
            (api_decls, api_pre, api_post) = self.generateWrappingCode(command)

        self.write("".join(out))


    (tmp_decl, tmp_pre, tmp_post) = self.outputNDOs(member.type, member.name, count_name, prefix, index, indent, isDestroy, first_level_param)

    def outputNDOs(self, ndo_type, ndo_name, ndo_count, prefix, index, indent, destroy_func, destroy_array, top_level):
           if ndo_count is not None:
            if top_level == True:
                decls += '%s%s var_local_%s%s[DISPATCH_MAX_STACK_ALLOCATIONS];\n' % (indent, ndo_type, prefix, ndo_name)
                decls += '%s%s *local_%s%s = nullptr;\n' % (indent, ndo_type, prefix, ndo_name)
            pre_code += '%s    if (%s%s) {\n' % (indent, prefix, ndo_name)
            indent = self.incIndent(indent)
            if top_level == True:
                pre_code += '%s    local_%s%s = %s > DISPATCH_MAX_STACK_ALLOCATIONS ? new %s[%s] : var_local_%s%s;\n' % (indent, prefix, ndo_name, ndo_count, ndo_type, ndo_count, prefix, ndo_name)
                pre_code += '%s    for (uint32_t %s = 0; %s < %s; ++%s) {\n' % (indent, index, index, ndo_count, index)
                indent = self.incIndent(indent)
                pre_code += '%s    local_%s%s[%s] = layer_data->Unwrap(%s[%s]);\n' % (indent, prefix, ndo_name, index, ndo_name, index)
            else:
                pre_code += '%s    for (uint32_t %s = 0; %s < %s; ++%s) {\n' % (indent, index, index, ndo_count, index)
                indent = self.incIndent(indent)
                pre_code += '%s    %s%s[%s] = layer_data->Unwrap(%s%s[%s]);\n' % (indent, prefix, ndo_name, index, prefix, ndo_name, index)
            indent = self.decIndent(indent)
            pre_code += '%s    }\n' % indent
            indent = self.decIndent(indent)
            pre_code += '%s    }\n' % indent
            if top_level == True:
                post_code += '%sif (local_%s%s != var_local_%s%s)\n' % (indent, prefix, ndo_name, prefix, ndo_name)
                indent = self.incIndent(indent)
                post_code += '%sdelete[] local_%s;\n' % (indent, ndo_name)
        else:
            if top_level == True:
                if (destroy_func == False) or (destroy_array == True):
                    pre_code += '%s    %s = layer_data->Unwrap(%s);\n' % (indent, ndo_name, ndo_name)
            else:
                # Make temp copy of this var with the 'local' removed. It may be better to not pass in 'local_'
                # as part of the string and explicitly print it
                fix = str(prefix).strip('local_');
                pre_code += '%s    if (%s%s) {\n' % (indent, fix, ndo_name)
                indent = self.incIndent(indent)
                pre_code += '%s    %s%s = layer_data->Unwrap(%s%s);\n' % (indent, prefix, ndo_name, fix, ndo_name)
                indent = self.decIndent(indent)
                pre_code += '%s    }\n' % indent
        return decls, pre_code, post_code

    #
    # first_level_param indicates if elements are passed directly into the function else they're below a ptr/struct (really CommandParam)
    def uniquifyMembers(self, members: List[Member], indent: str, prefix: str, array_index: int, isCreate: bool, isDestroy: bool, first_level_param: bool):
        decls = ''
        pre_code = ''
        post_code = ''
        index = f'index{str(array_index)}'
        array_index += 1
        # Process any NDOs in this structure and recurse for any sub-structs in this struct
        for member in [x for x in members if x.type in self.vk.handles and not self.vk.handles[member.type].dispatchable]:
            # Handle NDOs
            count_name = member.length
            if count_name is not None:
                if not first_level_param:
                    count_name = '%s%s' % (prefix, member.length)

            if (not first_level_param) or (not isCreate) or (not member.pointer):
                # Output UO code for a single NDO (ndo_count is NULL) or a counted list of NDOs
                (tmp_decl, tmp_pre, tmp_post) = self.outputNDOs(member.type, member.name, count_name, prefix, index, indent, isDestroy, first_level_param)

                decls += tmp_decl
                pre_code += tmp_pre
                post_code += tmp_post

        # Handle Structs that contain NDOs at some level
        for member in [x for x in members if x.type in self.vk.structs]:
            struct = self.vk.structs[member.type]
            process_pnext = struct.extendedBy and any(x in self.ndo_extension_structs for x in struct.extendedBy)
            # Structs at first level will have an NDO, OR, we need a safe_struct for the pnext chain
            if self.struct_contains_ndo(member.type) == True or process_pnext:
                struct_info = self.struct_member_dict[member.type]
                if any(member.ispointer for member in struct_info):
                    safe_type = 'safe_' + member.type
                else:
                    safe_type = member.type
                # Struct Array
                if member.len is not None:
                    # Check if this function can be deferred.
                    deferred_name = next((member.name for member in members if member.type == 'VkDeferredOperationKHR'), None)
                    # Update struct prefix
                    if first_level_param == True:
                        new_prefix = 'local_%s' % member.name
                        # Declare safe_VarType for struct
                        decls += '%s%s *%s = nullptr;\n' % (indent, safe_type, new_prefix)
                    else:
                        new_prefix = '%s%s' % (prefix, member.name)
                    pre_code += '%s    if (%s%s) {\n' % (indent, prefix, member.name)
                    indent = self.incIndent(indent)
                    if first_level_param == True:
                        pre_code += '%s    %s = new %s[%s];\n' % (indent, new_prefix, safe_type, member.len)
                    pre_code += '%s    for (uint32_t %s = 0; %s < %s%s; ++%s) {\n' % (indent, index, index, prefix, member.len, index)
                    indent = self.incIndent(indent)
                    if first_level_param == True:
                        if 'safe_' in safe_type:
                            # Handle special initialize function for VkAccelerationStructureBuildGeometryInfoKHR
                            if member.type == "VkAccelerationStructureBuildGeometryInfoKHR":
                                pre_code += '%s    %s[%s].initialize(&%s[%s], false, nullptr);\n' % (indent, new_prefix, index, member.name, index)
                            else:
                                pre_code += '%s    %s[%s].initialize(&%s[%s]);\n' % (indent, new_prefix, index, member.name, index)
                        else:
                            pre_code += '%s    %s[%s] = %s[%s];\n' % (indent, new_prefix, index, member.name, index)
                        if process_pnext:
                            pre_code += '%s    WrapPnextChainHandles(layer_data, %s[%s].pNext);\n' % (indent, new_prefix, index)
                    local_prefix = '%s[%s].' % (new_prefix, index)
                    # Process sub-structs in this struct
                    (tmp_decl, tmp_pre, tmp_post) = self.uniquifyMembers(struct_info, indent, local_prefix, array_index, isCreate, isDestroy, False)
                    decls += tmp_decl
                    pre_code += tmp_pre
                    post_code += tmp_post
                    indent = self.decIndent(indent)
                    pre_code += '%s    }\n' % indent
                    indent = self.decIndent(indent)
                    pre_code += '%s    }\n' % indent
                    if first_level_param == True:
                        post_code += self.cleanUpLocalDeclarations(indent, prefix, member.name, member.len, deferred_name, index)
                # Single Struct
                elif member.ispointer:
                    # Check if this function can be deferred.
                    deferred_name = next((member.name for member in members if member.type == 'VkDeferredOperationKHR'), None)
                    # Update struct prefix
                    if first_level_param == True:
                        new_prefix = 'local_%s->' % member.name
                        if deferred_name is None:
                            decls += '%s%s var_local_%s%s;\n' % (indent, safe_type, prefix, member.name)
                        decls += '%s%s *local_%s%s = nullptr;\n' % (indent, safe_type, prefix, member.name)
                    else:
                        new_prefix = '%s%s->' % (prefix, member.name)
                    # Declare safe_VarType for struct
                    pre_code += '%s    if (%s%s) {\n' % (indent, prefix, member.name)
                    indent = self.incIndent(indent)
                    if first_level_param == True:
                        if deferred_name is None:
                            pre_code += '%s    local_%s%s = &var_local_%s%s;\n' % (indent, prefix, member.name, prefix, member.name)
                        else:
                            pre_code += '%s    local_%s = new %s;\n' % (indent, member.name, safe_type)
                        if 'safe_' in safe_type:
                            # Handle special initialize function for VkAccelerationStructureBuildGeometryInfoKHR
                            if member.type == "VkAccelerationStructureBuildGeometryInfoKHR":
                                pre_code += '%s    local_%s%s->initialize(%s, false, nullptr);\n' % (indent, prefix, member.name, member.name)
                            else:
                                pre_code += '%s    local_%s%s->initialize(%s);\n' % (indent, prefix, member.name, member.name)
                        else:
                            pre_code += '%s    *local_%s%s = *%s;\n' % (indent, prefix, member.name, member.name)
                    # Process sub-structs in this struct
                    (tmp_decl, tmp_pre, tmp_post) = self.uniquifyMembers(struct_info, indent, new_prefix, array_index, isCreate, isDestroy, False)
                    decls += tmp_decl
                    pre_code += tmp_pre
                    post_code += tmp_post
                    if process_pnext:
                        pre_code += '%s    WrapPnextChainHandles(layer_data, %spNext);\n' % (indent, new_prefix)
                    indent = self.decIndent(indent)
                    pre_code += '%s    }\n' % indent
                    if first_level_param == True:
                        post_code += self.cleanUpLocalDeclarations(indent, prefix, member.name, member.len, deferred_name, index)
                else:
                    # Update struct prefix
                    if first_level_param == True:
                        sys.exit(1)
                    else:
                        new_prefix = '%s%s.' % (prefix, member.name)
                    # Process sub-structs in this struct
                    (tmp_decl, tmp_pre, tmp_post) = self.uniquifyMembers(struct_info, indent, new_prefix, array_index, isCreate, isDestroy, False)
                    decls += tmp_decl
                    pre_code += tmp_pre
                    post_code += tmp_post
                    if process_pnext:
                        pre_code += '%s    WrapPnextChainHandles(layer_data, %s%s.pNext);\n' % (indent, prefix, member.name)
        return decls, pre_code, post_code

    #
    # Generate source for creating a non-dispatchable object
    def generateCreateFunc(self, indent, command: Command) -> str:
        create_ndo_code = ''
        lastParam = command.params[-1]
        handle_type = lastParam.type
        if handle_type in self.vk.handles and not self.vk.handles[handle_type].dispatchable:
            # Check for special case where multiple handles are returned
            ndo_array = lastParam.length is not None
            create_ndo_code += '%sif (VK_SUCCESS == result) {\n' % (indent)
            indent = self.incIndent(indent)
            ndo_dest = f'*{lastParam.name}'
            if ndo_array == True:
                create_ndo_code += '%sfor (uint32_t index0 = 0; index0 < %s; index0++) {\n' % (indent, lastParam.length)
                indent = self.incIndent(indent)
                ndo_dest = '%s[index0]' % lastParam.name
            create_ndo_code += '%s%s = layer_data->WrapNew(%s);\n' % (indent, ndo_dest, ndo_dest)
            if ndo_array == True:
                indent = self.decIndent(indent)
                create_ndo_code += '%s}\n' % indent
            indent = self.decIndent(indent)
            create_ndo_code += '%s}\n' % (indent)
        return create_ndo_code
    #
    # Generate source for destroying a non-dispatchable object
    def generateDestroyFunc(self, indent, command: Command):
        destroy_ndo_code = ''
        # Last param in a destroy is always the VkAllocationCallbacks
        handleParam = command.params[-2]
        if handleParam.type in self.vk.handles and not self.vk.handles[handleParam.type].dispatchable:
            # Remove a single handle from the map
            destroy_ndo_code += '%suint64_t %s_id = CastToUint64(%s);\n' % (indent, handleParam.name, handleParam.name)
            destroy_ndo_code += '%sauto iter = unique_id_mapping.pop(%s_id);\n' % (indent, handleParam.name)
            destroy_ndo_code += '%sif (iter != unique_id_mapping.end()) {\n' % (indent)
            indent = self.incIndent(indent)
            destroy_ndo_code += '%s%s = (%s)iter->second;\n' % (indent, handleParam.name, handleParam.type)
            indent = self.decIndent(indent);
            destroy_ndo_code += '%s} else {\n' % (indent)
            indent = self.incIndent(indent)
            destroy_ndo_code += '%s%s = (%s)0;\n' % (indent, handleParam.name, handleParam.type)
            indent = self.decIndent(indent);
            destroy_ndo_code += '%s}\n' % (indent)

        return destroy_ndo_code

    #
    # For a particular API, generate the non-dispatchable-object wrapping/unwrapping code
    def generateWrappingCode(self, command: Command):
        indent = '    '
        isCreate = any(x in command.name for x in ['Create', 'Allocate', 'GetRandROutputDisplayEXT', 'GetDrmDisplayEXT', 'RegisterDeviceEvent', 'RegisterDisplayEvent', 'AcquirePerformanceConfigurationINTEL'])
        isDestroy = any(x in command.name for x in ['Destroy', 'Free'])

        # Handle ndo create/allocate operations
        create_func = self.generateCreateFunc(indent, command) if isCreate else ''

        # Handle ndo destroy/free operations
        destroy_func = self.generateDestroyFunc(indent, command) if isDestroy else (False, '')

        api_decls = ''
        api_pre = ''
        api_post = ''
        # TODO - luckily the Member and CommandParam objects share enough same classes, but this function expects a Member only
        (api_decls, api_pre, api_post) = self.uniquifyMembers(command.params, indent, '', 0, isCreate, isDestroy, False, True)
        api_post += create_func
        if isDestroy:
            api_pre += destroy_func
        if api_pre:
            if not isDestroy:
                api_pre = f'    {{\n{api_pre}{indent}}}\n')
        return api_decls, api_pre, api_post

    def containsNonDispatchableObject(self, struct: Struct) -> bool:
        for member in struct.members:
            if member.type in self.vk.handles and not self.vk.handles[member.type].dispatchable:
                return True
            # recurse for member structs, guard against infinite recursion
            elif member.type in self.vk.structs and member.type != struct.name:
                if self.ContainsNonDispatchableObject(self.vk.structs[member.type]):
                    return True
        return False