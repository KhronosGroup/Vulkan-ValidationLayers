#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2025 The Khronos Group Inc.
# Copyright (c) 2015-2025 Valve Corporation
# Copyright (c) 2015-2025 LunarG, Inc.
# Copyright (c) 2015-2025 Google Inc.
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
from vulkan_object import Member
from base_generator import BaseGenerator
from generators.generator_utils import PlatformGuardHelper

# This class is a container for any source code, data, or other behavior that is necessary to
# customize the generator script for a specific target API variant (e.g. Vulkan SC). As such,
# all of these API-specific interfaces and their use in the generator script are part of the
# contract between this repository and its downstream users. Changing or removing any of these
# interfaces or their use in the generator script will have downstream effects and thus
# should be avoided unless absolutely necessary.
class APISpecific:
    # Returns the list of validation layers for the target API
    @staticmethod
    def getValidationLayerList(targetApiName: str) -> list[dict[str, str]]:
        match targetApiName:

            # Vulkan specific validation layer list
            case 'vulkan':
                return [
                    {
                        'include': 'thread_tracker/thread_safety_validation.h',
                        'device': 'threadsafety::Device',
                        'instance': 'threadsafety::Instance',
                        'type': 'LayerObjectTypeThreading',
                        'enabled': '!settings.disabled[thread_safety]'
                    },
                    {
                        'include': 'stateless/stateless_validation.h',
                        'device': 'stateless::Device',
                        'instance': 'stateless::Instance',
                        'type': 'LayerObjectTypeParameterValidation',
                        'enabled': '!settings.disabled[stateless_checks]'
                    },
                    {
                        'include': 'object_tracker/object_lifetime_validation.h',
                        'device': 'object_lifetimes::Device',
                        'instance': 'object_lifetimes::Instance',
                        'type': 'LayerObjectTypeObjectTracker',
                        'enabled': '!settings.disabled[object_tracking]'
                    },
                    {
                        'include': 'state_tracker/state_tracker.h',
                        'device': 'vvl::DeviceState',
                        'instance': 'vvl::InstanceState',
                        'type': 'LayerObjectTypeStateTracker',
                        'enabled': '''
                            !settings.disabled[core_checks] ||
                            settings.enabled[best_practices] ||
                            settings.enabled[gpu_validation] ||
                            settings.enabled[debug_printf_validation] ||
                            settings.enabled[sync_validation]
                        '''
                    },
                    {
                        'include': 'core_checks/core_validation.h',
                        'device': 'CoreChecks',
                        'instance': 'core::Instance',
                        'type': 'LayerObjectTypeCoreValidation',
                        'enabled': '!settings.disabled[core_checks]'
                    },
                    {
                        'include': 'best_practices/best_practices_validation.h',
                        'device': 'BestPractices',
                        'instance': 'bp_state::Instance',
                        'type': 'LayerObjectTypeBestPractices',
                        'enabled': 'settings.enabled[best_practices]'
                    },
                    {
                        'include': 'gpuav/core/gpuav.h',
                        'device': 'gpuav::Validator',
                        'instance': 'gpuav::Instance',
                        'type': 'LayerObjectTypeGpuAssisted',
                        'enabled': 'settings.enabled[gpu_validation] || settings.enabled[debug_printf_validation]'
                    },
                    {
                        'include': 'sync/sync_validation.h',
                        'device': 'SyncValidator',
                        'instance': 'syncval::Instance',
                        'type': 'LayerObjectTypeSyncValidation',
                        'enabled': 'settings.enabled[sync_validation]'
                    }
                ]


class DispatchObjectGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

        # Commands which are not autogenerated but still intercepted
        self.no_autogen_list = (
            'vkCreateInstance',
            'vkDestroyInstance',
            'vkCreateDevice',
            'vkDestroyDevice',
            # Need to handle Acquired swapchain image handles
            'vkGetSwapchainImagesKHR',
            'vkDestroySwapchainKHR',
            # Have issues with generating logic to work correctly with Safe Struct
            'vkQueuePresentKHR',
            'vkCreateGraphicsPipelines',
            'vkCreateComputePipelines',
            'vkCreateRayTracingPipelinesNV',
            'vkCreateRayTracingPipelinesKHR',
            # Need handle which pool descriptors were allocated from
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
            'vkCmdPushDescriptorSetWithTemplate',
            'vkCmdPushDescriptorSetWithTemplateKHR',
            'vkCmdPushDescriptorSetWithTemplate2',
            'vkCmdPushDescriptorSetWithTemplate2KHR',
            # Tracking renderpass state for the pipeline safe struct
            'vkCreateRenderPass',
            'vkCreateRenderPass2KHR',
            'vkCreateRenderPass2',
            'vkDestroyRenderPass',
            # Accesses to the map itself are internally synchronized.
            'vkDebugMarkerSetObjectTagEXT',
            'vkDebugMarkerSetObjectNameEXT',
            'vkSetDebugUtilsObjectNameEXT',
            'vkSetDebugUtilsObjectTagEXT',
            # TODO - These have no manual source, but we still produce the headers
            'vkEnumerateInstanceExtensionProperties',
            'vkEnumerateInstanceLayerProperties',
            'vkEnumerateDeviceLayerProperties',
            'vkEnumerateInstanceVersion',
            # Manually track pToolCount
            'vkGetPhysicalDeviceToolProperties',
            'vkGetPhysicalDeviceToolPropertiesEXT',
            # Track deferred operations
            'vkDeferredOperationJoinKHR',
            'vkGetDeferredOperationResultKHR',
            # Need to deal with VkAccelerationStructureGeometryKHR
            'vkBuildAccelerationStructuresKHR',
            'vkGetAccelerationStructureBuildSizesKHR',
            'vkCmdBuildAccelerationStructuresKHR',
            # Depends on the VkDescriptorType to pick the union pointer
            'vkGetDescriptorEXT',
            # Depends on the VkIndirectExecutionSetInfoTypeEXT to pick the union pointer
            'vkCreateIndirectExecutionSetEXT',
            # Special destroy call from the Acquire
            'vkReleasePerformanceConfigurationINTEL',
            # need to call CopyExportMetalObjects
            'vkExportMetalObjectsEXT',
            # These are for special-casing the pInheritanceInfo issue (must be ignored for primary CBs)
            'vkAllocateCommandBuffers',
            'vkFreeCommandBuffers',
            'vkDestroyCommandPool',
            'vkBeginCommandBuffer',
            # Currently we don't properly generate a Wrap and will accidently unwrap the VkDisplayKHR handle
            'vkGetPhysicalDeviceDisplayPropertiesKHR',
            'vkGetPhysicalDeviceDisplayProperties2KHR',
            'vkGetPhysicalDeviceDisplayPlanePropertiesKHR',
            'vkGetPhysicalDeviceDisplayPlaneProperties2KHR',
            'vkGetDisplayModePropertiesKHR',
            'vkGetDisplayModeProperties2KHR',
            'vkGetDisplayPlaneSupportedDisplaysKHR',
            # Need to handle binaries in VkPipelineBinaryHandlesInfoKHR as output, not input.
            'vkCreatePipelineBinariesKHR',
            'vkGetPipelineKeyKHR',
            # Need to handle VkBindMemoryStatus
            'vkBindBufferMemory2',
            'vkBindBufferMemory2KHR',
            'vkBindImageMemory2',
            'vkBindImageMemory2KHR',
        )

        # List of all extension structs strings containing handles
        self.ndo_extension_structs = [
            # These are added manually because vkGetPipelineKeyKHR needs to unwrap these (see VUID 09604)
            "VkComputePipelineCreateInfo",
            "VkGraphicsPipelineCreateInfo",
            "VkRayTracingPipelineCreateInfoKHR",
            "VkExecutionGraphPipelineCreateInfoAMDX",
        ]

        self.extended_query_exts = (
            'VK_KHR_get_physical_device_properties2',
            'VK_KHR_external_semaphore_capabilities',
            'VK_KHR_external_fence_capabilities',
            'VK_KHR_external_memory_capabilities',
            'VK_KHR_get_memory_requirements2',
        )

        # Dispatch functions that need special state tracking variables passed in
        self.custom_definition = {}


    def isNonDispatchable(self, name: str) -> bool:
        return name in self.vk.handles and not self.vk.handles[name].dispatchable

    def containsNonDispatchableObject(self, structName: str) -> bool:
        struct = self.vk.structs[structName]
        for member in struct.members:
            if self.isNonDispatchable(member.type):
                return True
            # recurse for member structs, guard against infinite recursion
            elif member.type in self.vk.structs and member.type != struct.name:
                if self.containsNonDispatchableObject(member.type):
                    return True
        return False

    # Now that the data is all collected and complete, generate and output the wrapping/unwrapping routines
    def generate(self):
        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2015-2025 The Khronos Group Inc.
            * Copyright (c) 2015-2025 Valve Corporation
            * Copyright (c) 2015-2025 LunarG, Inc.
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

        if self.filename == 'dispatch_object_instance_methods.h':
            self.generateInstanceMethods()
        elif self.filename == 'dispatch_object_device_methods.h':
            self.generateDeviceMethods()
        elif self.filename == 'dispatch_functions.h':
            self.generateFunctions()
        elif self.filename == 'dispatch_object.cpp':
            self.generateSource()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateMethods(self, want_instance):
        out = []
        guard_helper = PlatformGuardHelper()
        for command in [x for x in self.vk.commands.values() if x.instance == want_instance]:
            prototype = command.cPrototype
            prototype = prototype.replace('VKAPI_ATTR ', '')
            prototype = prototype.replace('VKAPI_CALL vk', '')
            if command.name in self.custom_definition:
                prototype = prototype.replace(');', f'{self.custom_definition[command.name]});')
            out.extend(guard_helper.add_guard(command.protect))
            out.append(f'{prototype}\n')
        out.extend(guard_helper.add_guard(None))
        self.write("".join(out))

    def generateDeviceMethods(self):
        out = []
        out.append('''
            // This file contains methods for class vvl::dispatch::Device and it is designed to ONLY be
            // included into dispatch_object.h.

            #pragma once

            ''')
        self.write("".join(out))
        self.generateMethods(False)

    def generateInstanceMethods(self):
        out = []
        out.append('''
            // This file contains methods for class vvl::dispatch::Instance  and it is designed to ONLY be
            // included into dispatch_object.h.

            #pragma once
            ''')
        self.write("".join(out))
        self.generateMethods(True)

    def generateFunctions(self):
        out = []
        out.append('''
            // This file contains contains convience functions for non-chassis code that needs to
            // make vulkan calls.

            #pragma once

            #include "chassis/dispatch_object.h"

            ''')
        dispatchable_handles = [handle.name for handle in self.vk.handles.values() if handle.dispatchable]
        guard_helper = PlatformGuardHelper()
        for command in self.vk.commands.values():
            # Weed out weird commands like vkCreateInstance()
            if command.params[0].type not in dispatchable_handles:
                continue

            prototype = command.cPrototype
            prototype = prototype.replace('VKAPI_ATTR ', 'static inline ')
            prototype = prototype.replace('VKAPI_CALL vk', 'Dispatch')
            proto_extra = ''
            call_extra = ''
            if command.name in self.custom_definition:
                proto_extra = self.custom_definition[command.name]
                call_extra = ', ' + proto_extra.split(' ')[-1]
            prototype = prototype.replace(');', f'{proto_extra}) {{')
            out.extend(guard_helper.add_guard(command.protect))
            out.append(f'\n{prototype}\n')
            out.append(f'auto dispatch = vvl::dispatch::GetData({command.params[0].name});\n')
            returnResult = f'return ' if (command.returnType != 'void') else ''
            paramsList = ', '.join([param.name for param in command.params])
            out.append(f'{returnResult}{command.name.replace("vk", "dispatch->")}({paramsList}{call_extra});\n')
            out.append('}\n')
        out.extend(guard_helper.add_guard(None))
        out.append('// We make many internal dispatch calls to extended query functions which can depend on the API version\n')
        for extended_query_ext in self.extended_query_exts:
            for command in self.vk.extensions[extended_query_ext].commands:
                parameters = (command.cPrototype.split('(')[1])[:-2] # leaves just the parameters
                arguments = ','.join([x.name for x in command.params])
                out.append(f'''
                static inline {command.returnType} Dispatch{command.alias[2:]}Helper(APIVersion api_version, {parameters}) {{
                    if (api_version >= VK_API_VERSION_1_1) {{
                        return Dispatch{command.alias[2:]}({arguments});
                    }} else {{
                        return Dispatch{command.name[2:]}({arguments});
                    }}
                }}
                ''')
        self.write("".join(out))

    def generateSource(self):
        # Construct list of extension structs containing handles
        # Generate the list of APIs that might need to handle wrapped extension structs
        for struct in [x for x in self.vk.structs.values() if x.sType and x.extendedBy]:
            for extendedStruct in struct.extendedBy:
                if self.containsNonDispatchableObject(extendedStruct) and extendedStruct not in self.ndo_extension_structs:
                    self.ndo_extension_structs.append(extendedStruct)

        out = []
        out.append('''
            #include "chassis/dispatch_object.h"
            #include "utils/cast_utils.h"
            #include <vulkan/utility/vk_safe_struct.hpp>
            #include "state_tracker/pipeline_state.h"
            #include "containers/custom_containers.h"

             ''')
        for layer in APISpecific.getValidationLayerList(self.targetApiName):
             include_file = layer['include']
             out.append(f'#include "{include_file}"\n')
        out.append('\n')

        out.append('''
            #define DISPATCH_MAX_STACK_ALLOCATIONS 32

            namespace vvl {
            namespace dispatch {

            void Instance::InitValidationObjects() {
                 // Note that this DEFINES THE ORDER IN WHICH THE LAYER VALIDATION OBJECTS ARE CALLED
             ''')
        for layer in APISpecific.getValidationLayerList(self.targetApiName):
             classname = layer['instance']
             out.append(f'''
                 if ({layer["enabled"]}) {{
                     object_dispatch.emplace_back(new {classname}(this));
                 }}''')

        out.append('\n')
        out.append('}\n')
        out.append('''
            void Device::InitValidationObjects() {
                 // Note that this DEFINES THE ORDER IN WHICH THE LAYER VALIDATION OBJECTS ARE CALLED
             ''')
        for layer in APISpecific.getValidationLayerList(self.targetApiName):
             classname = layer['device']
             layer_type = layer['type']
             instance = layer['instance']
             instance_arg = f'static_cast<{instance}*>(dispatch_instance->GetValidationObject({layer_type}))'
             out.append(f'''
                 if ({layer["enabled"]}) {{
                     object_dispatch.emplace_back(new {classname}(this, {instance_arg}));
                 }}''')
        out.append('\n')
        out.append('}\n')

        out.append('''
            // Unique Objects pNext extension handling function
            void HandleWrapper::UnwrapPnextChainHandles(const void *pNext) {
                void *cur_pnext = const_cast<void *>(pNext);
                while (cur_pnext != nullptr) {
                    VkBaseOutStructure *header = reinterpret_cast<VkBaseOutStructure *>(cur_pnext);

                    switch (header->sType) {
            ''')
        guard_helper = PlatformGuardHelper()
        for struct in [self.vk.structs[x] for x in self.ndo_extension_structs]:
            (api_decls, api_pre, api_post) = self.uniquifyMembers(struct.members, 'safe_struct->', 0, False, False, False)
            # Only process extension structs containing handles
            if not api_pre:
                continue
            safe_name = 'vku::safe_' + struct.name
            out.extend(guard_helper.add_guard(struct.protect))
            out.append(f'case {struct.sType}: {{\n')
            out.append(f'    auto *safe_struct = reinterpret_cast<{safe_name} *>(cur_pnext);\n')
            out.append(api_pre)
            out.append('} break;\n')
        out.extend(guard_helper.add_guard(None))
        out.append('''
                    default:
                        break;
                }

                // Process the next structure in the chain
                cur_pnext = header->pNext;
            }
            }
            ''')

        out.append('''
            [[maybe_unused]] static bool NotDispatchableHandle(VkObjectType object_type) {
                switch(object_type) {
        ''')
        out.extend([f'case {handle.type}:\n' for handle in self.vk.handles.values() if handle.dispatchable])
        out.append('''return false;
                  default:
                    return true;
                }
            }
        ''')

        for command in [x for x in self.vk.commands.values() if x.name not in self.no_autogen_list]:
            out.extend(guard_helper.add_guard(command.protect))

            # Generate NDO wrapping/unwrapping code for all parameters
            isCreate = any(x in command.name for x in [
                'vkCreate',
                'vkAllocate',
                # Create a VkFence object
                'vkRegisterDeviceEvent',
                'vkRegisterDisplayEvent',
                # Create VkPerformanceConfigurationINTEL object
                'vkAcquirePerformanceConfigurationINTEL',
                # Special calls that we wrap because they are created statically on the driver, but users query for them
                'vkGetWinrtDisplayNV',
                'vkGetRandROutputDisplayEXT',
                'vkGetDrmDisplayEXT',
            ])
            isDestroy = any(x in command.name for x in ['vkDestroy', 'vkFree'])

            # Handle ndo create/allocate operations
            create_ndo_code = ''
            if isCreate:
                lastParam = command.params[-1]
                handle_type = lastParam.type
                if self.isNonDispatchable(handle_type):
                    # Check for special case where multiple handles are returned
                    wrap_call = 'WrapNew' if handle_type != 'VkDisplayKHR' else 'MaybeWrapDisplay'
                    ndo_array = lastParam.length is not None
                    create_ndo_code += 'if (VK_SUCCESS == result) {\n'
                    ndo_dest = f'*{lastParam.name}'
                    if ndo_array:
                        create_ndo_code += f'for (uint32_t index0 = 0; index0 < {lastParam.length}; index0++) {{\n'
                        ndo_dest = f'{lastParam.name}[index0]'
                    create_ndo_code += f'{ndo_dest} = {wrap_call}({ndo_dest});\n'
                    if ndo_array:
                        create_ndo_code += '}\n'
                    create_ndo_code += '}\n'

            # Handle ndo destroy/free operations
            destroy_ndo_code = ''
            if isDestroy:
                param = command.params[-2] # Last param is always VkAllocationCallbacks
                if self.isNonDispatchable(param.type):
                    # Remove a single handle from the map
                    destroy_ndo_code += f'{param.name} = Erase({param.name});'

            (api_decls, api_pre, api_post) = self.uniquifyMembers(command.params, '', 0, isCreate, isDestroy, True)
            api_post += create_ndo_code
            if isDestroy:
                api_pre += destroy_ndo_code
            elif api_pre:
                api_pre = f'{{\n{api_pre}}}\n'

            # If API doesn't contain NDO's, we still need to make a down-chain call
            down_chain_call_only = False
            if not api_decls and not api_pre and not api_post:
                down_chain_call_only = True

            prototype = command.cPrototype[:-1]
            prototype = prototype.replace('VKAPI_ATTR ', '')
            prototype = prototype.replace('VKAPI_CALL vk', 'Instance::' if command.instance else 'Device::')
            out.append(f'\n{prototype} {{\n')\

            # Pull out the text for each of the parameters, separate them by commas in a list
            paramstext = ', '.join([param.name for param in command.params])
            wrapped_paramstext = paramstext
            # If any of these paramters has been replaced by a local var, fix up the list
            for param in command.params:
                struct = self.vk.structs[param.type] if param.type in self.vk.structs else None
                isLocal = (self.isNonDispatchable(param.type) and param.length and param.const) or (struct and self.containsNonDispatchableObject(struct.name))
                isExtended = struct and struct.extendedBy and any(x in self.ndo_extension_structs for x in struct.extendedBy)
                if isLocal or isExtended:
                    if param.pointer:
                        if param.const:
                          wrapped_paramstext = wrapped_paramstext.replace(param.name, f'(const {param.type}*)local_{param.name}')
                        else:
                          wrapped_paramstext = wrapped_paramstext.replace(param.name, f'({param.type}*)local_{param.name}')
                    else:
                        wrapped_paramstext = wrapped_paramstext.replace(param.name, f'(const {param.type})local_{param.name}')

            # First, add check and down-chain call. Use correct dispatch table
            dispatch_table = 'instance_dispatch_table' if command.instance else 'device_dispatch_table'

            # Put all this together for the final down-chain call
            if not down_chain_call_only:
                out.append(f'if (!wrap_handles) return {dispatch_table}.{command.name[2:]}({paramstext});\n')

            # Handle return values, if any
            assignResult = f'{command.returnType} result = ' if command.returnType != 'void' else ''

            # Pre-pend declarations and pre-api-call codegen
            if api_decls:
                out.append("\n".join(str(api_decls).rstrip().split("\n")))
            if api_pre:
                out.append("\n".join(str(api_pre).rstrip().split("\n")))
            out.append('\n')
            # Generate the wrapped dispatch call
            out.append(f'{assignResult}{dispatch_table}.{command.name[2:]}({wrapped_paramstext});\n')

            out.append("\n".join(str(api_post).rstrip().split("\n")))
            out.append('\n')
            # Handle the return result variable, if any
            if assignResult != '':
                out.append('return result;\n')
            out.append('}\n')
        out.extend(guard_helper.add_guard(None))
        out.append('} // namespace dispatch\n')
        out.append('} // namespace vvl\n')

        self.write("".join(out))

    #
    # Clean up local declarations
    def cleanUpLocalDeclarations(self, prefix, name, len, deferred_name):
        cleanup = ''
        if deferred_name is not None:
            delete_var = f'local_{prefix}{name}'
            if len is None:
                delete_code = f'delete {delete_var}'
            else:
                delete_code = f'delete[] {delete_var}'
            cleanup = f'if ({delete_var}) {{\n'
            cleanup += f'''
                    // Fix check for deferred ray tracing pipeline creation
                    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5817
                    const bool is_operation_deferred = ({deferred_name} != VK_NULL_HANDLE) && (result == VK_OPERATION_DEFERRED_KHR);
                    if (is_operation_deferred) {{
                        std::vector<std::function<void()>> cleanup{{[{delete_var}](){{ {delete_code}; }}}};
                        deferred_operation_post_completion.insert({deferred_name}, cleanup);
                    }} else {{
                        {delete_code};
                    }}'''
            cleanup += '}\n'
        return cleanup

    #
    # topLevel indicates if elements are passed directly into the function else they're below a ptr/struct
    # isCreate means that this is API creates or allocates NDOs
    # isDestroy indicates that this API destroys or frees NDOs
    def uniquifyMembers(self, members: list[Member], prefix: str, arrayIndex: int, isCreate: bool, isDestroy: bool, topLevel: bool):
        decls = ''
        pre_code = ''
        post_code = ''
        index = f'index{str(arrayIndex)}'
        arrayIndex += 1
        # Process any NDOs in this structure and recurse for any sub-structs in this struct
        for member in members:
            # Handle NDOs
            if self.isNonDispatchable(member.type):
                count_name = member.length
                if (count_name is not None) and not topLevel:
                    count_name = f'{prefix}{member.length}'

                if (not topLevel) or (not isCreate) or (not member.pointer):
                    if count_name is not None:
                        if topLevel:
                            decls += f'small_vector<{member.type}, DISPATCH_MAX_STACK_ALLOCATIONS> var_local_{prefix}{member.name};\n'
                            decls += f'{member.type} *local_{prefix}{member.name} = nullptr;\n'
                        pre_code += f' if ({prefix}{member.name}) {{\n'
                        if topLevel:
                            pre_code += f'''
                                var_local_{prefix}{member.name}.resize({count_name});
                                local_{prefix}{member.name} = var_local_{prefix}{member.name}.data();
                                for (uint32_t {index} = 0; {index} < {count_name}; ++{index}) {{
                                    local_{prefix}{member.name}[{index}] = Unwrap({member.name}[{index}]);'''
                        else:
                            pre_code += f'''
                                for (uint32_t {index} = 0; {index} < {count_name}; ++{index}) {{
                                    {prefix}{member.name}[{index}] = Unwrap({prefix}{member.name}[{index}]);'''
                        pre_code += '}\n'
                        pre_code += '}\n'
                    else:
                        if topLevel:
                            if not isDestroy:
                                pre_code += f'{member.name} = Unwrap({member.name});\n'
                        else:
                            # Make temp copy of this var with the 'local' removed. It may be better to not pass in 'local_'
                            # as part of the string and explicitly print it
                            fix = str(prefix).strip('local_')
                            pre_code += f'''
                                if ({fix}{member.name}) {{
                                    {prefix}{member.name} = Unwrap({fix}{member.name});
                                }}'''
            # Handle Structs that contain NDOs at some level
            elif member.type in self.vk.structs:
                struct = self.vk.structs[member.type]
                process_pnext = struct.extendedBy and any(x in self.ndo_extension_structs for x in struct.extendedBy)
                # Structs at first level will have an NDO, OR, we need a safe_struct for the pnext chain
                if self.containsNonDispatchableObject(member.type) or process_pnext:
                    safe_type = 'vku::safe_' + member.type if any(x.pointer for x in struct.members) else member.type
                    # Struct Array
                    if member.length is not None:
                        # Check if this function can be deferred.
                        deferred_name = next((x.name for x in members if x.type == 'VkDeferredOperationKHR'), None)
                        # Update struct prefix
                        if topLevel:
                            new_prefix = f'local_{member.name}'
                            # Declare vku::safe_VkVarType for struct
                            if not deferred_name:
                                decls += f'small_vector<{safe_type}, DISPATCH_MAX_STACK_ALLOCATIONS> var_{new_prefix};\n'
                            decls += f'{safe_type} *{new_prefix} = nullptr;\n'

                        else:
                            new_prefix = f'{prefix}{member.name}'
                        pre_code += f'if ({prefix}{member.name}) {{\n'
                        if topLevel:
                            if deferred_name:
                                pre_code += f'{new_prefix} = new {safe_type}[{member.length}];\n'
                            else:
                                pre_code += f'var_{new_prefix}.resize({member.length});\n'
                                pre_code += f'{new_prefix} = var_{new_prefix}.data();\n'
                        pre_code += f'for (uint32_t {index} = 0; {index} < {prefix}{member.length}; ++{index}) {{\n'
                        if topLevel:
                            if safe_type.startswith('vku::safe'):
                                # Handle special initialize function for VkAccelerationStructureBuildGeometryInfoKHR
                                if member.type == "VkAccelerationStructureBuildGeometryInfoKHR":
                                    pre_code += f'{new_prefix}[{index}].initialize(&{member.name}[{index}], false, nullptr);\n'
                                else:
                                    pre_code += f'{new_prefix}[{index}].initialize(&{member.name}[{index}]);\n'
                            else:
                                pre_code += f'{new_prefix}[{index}] = {member.name}[{index}];\n'
                        if process_pnext:
                            pre_code += f'UnwrapPnextChainHandles({new_prefix}[{index}].pNext);\n'
                        local_prefix = f'{new_prefix}[{index}].'
                        # Process sub-structs in this struct
                        (tmp_decl, tmp_pre, tmp_post) = self.uniquifyMembers(struct.members, local_prefix, arrayIndex, isCreate, isDestroy, False)
                        decls += tmp_decl
                        pre_code += tmp_pre
                        post_code += tmp_post
                        pre_code += '}\n'
                        pre_code += '}\n'
                        if topLevel:
                            post_code += self.cleanUpLocalDeclarations(prefix, member.name, member.length, deferred_name)
                    # Single Struct
                    elif member.pointer:
                        # Check if this function can be deferred.
                        deferred_name = next((x.name for x in members if x.type == 'VkDeferredOperationKHR'), None)
                        # Update struct prefix
                        if topLevel:
                            new_prefix = f'local_{member.name}->'
                            if deferred_name is None:
                                decls += f'{safe_type} var_local_{prefix}{member.name};\n'
                            decls +=  f'{safe_type} *local_{prefix}{member.name} = nullptr;\n'
                        else:
                            new_prefix = f'{prefix}{member.name}->'
                        # Declare safe_VarType for struct
                        pre_code += f'if ({prefix}{member.name}) {{\n'
                        if topLevel:
                            if deferred_name is None:
                                pre_code += f'local_{prefix}{member.name} = &var_local_{prefix}{member.name};\n'
                            else:
                                pre_code += f'local_{member.name} = new {safe_type};\n'
                            if safe_type.startswith('vku::safe'):
                                # Handle special initialize function for VkAccelerationStructureBuildGeometryInfoKHR
                                if member.type == "VkAccelerationStructureBuildGeometryInfoKHR":
                                    pre_code += f'local_{prefix}{member.name}->initialize({member.name}, false, nullptr);\n'
                                else:
                                    pre_code += f'local_{prefix}{member.name}->initialize({member.name});\n'
                            else:
                                pre_code += f'*local_{prefix}{member.name} = *{member.name};\n'
                        # Process sub-structs in this struct
                        (tmp_decl, tmp_pre, tmp_post) = self.uniquifyMembers(struct.members, new_prefix, arrayIndex, isCreate, isDestroy, False)
                        decls += tmp_decl
                        pre_code += tmp_pre
                        post_code += tmp_post
                        if process_pnext:
                            pre_code += f'UnwrapPnextChainHandles({new_prefix}pNext);\n'
                        pre_code += '}\n'
                        if topLevel:
                            post_code += self.cleanUpLocalDeclarations(prefix, member.name, member.length, deferred_name)
                    else:
                        # Update struct prefix
                        if topLevel:
                            sys.exit(1)
                        else:
                            new_prefix = f'{prefix}{member.name}.'
                        # Process sub-structs in this struct
                        (tmp_decl, tmp_pre, tmp_post) = self.uniquifyMembers(struct.members, new_prefix, arrayIndex, isCreate, isDestroy, False)
                        decls += tmp_decl
                        pre_code += tmp_pre
                        post_code += tmp_post
                        if process_pnext:
                            pre_code += f'UnwrapPnextChainHandles({prefix}{member.name}.pNext);\n'
            elif member.type == 'VkObjectType' and member.name == 'objectType' and any(m.name == 'objectHandle' for m in members):
                pre_code += '''
                    if (NotDispatchableHandle(objectType)) {
                        objectHandle = Unwrap(objectHandle);
                    }
                '''
        return decls, pre_code, post_code
