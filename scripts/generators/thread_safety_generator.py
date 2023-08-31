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
from generators.vulkan_object import Command, Param
from generators.base_generator import BaseGenerator

def GetParentInstance(param: Param) -> str:
    instanceParent = ['VkSurfaceKHR',
                      'VkDebugReportCallbackEXT',
                      'VkDebugUtilsMessengerEXT',
                      'VkDisplayKHR',
                      'VkDevice',
                      'VkInstance',
                      ]
    return 'ParentInstance' if param.type in instanceParent else ''

class ThreadSafetyOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

        # Commands shadowed by interface functions and are not implemented
        self.manual_commands = [
            'vkAllocateCommandBuffers',
            'vkFreeCommandBuffers',
            'vkCreateCommandPool',
            'vkResetCommandPool',
            'vkDestroyCommandPool',
            'vkAllocateDescriptorSets',
            'vkFreeDescriptorSets',
            'vkResetDescriptorPool',
            'vkDestroyDescriptorPool',
            'vkGetSwapchainImagesKHR',
            'vkDestroySwapchainKHR',
            'vkDestroyDevice',
            'vkGetDeviceQueue',
            'vkGetDeviceQueue2',
            'vkCreateDescriptorSetLayout',
            'vkUpdateDescriptorSets',
            'vkUpdateDescriptorSetWithTemplate',
            'vkUpdateDescriptorSetWithTemplateKHR',
            'vkGetDisplayPlaneSupportedDisplaysKHR',
            'vkGetDisplayModePropertiesKHR',
            'vkGetDisplayModeProperties2KHR',
            'vkGetDisplayPlaneCapabilities2KHR',
            'vkGetRandROutputDisplayEXT',
            'vkGetDrmDisplayEXT',
            'vkDeviceWaitIdle',
            'vkRegisterDisplayEventEXT',
            'vkCreateRayTracingPipelinesKHR',
            'vkQueuePresentKHR',
        ]

        self.blacklist = [
            # Currently not wrapping debug helper functions
            'vkSetDebugUtilsObjectNameEXT',
            'vkSetDebugUtilsObjectTagEXT',
            'vkDebugMarkerSetObjectTagEXT',
            'vkDebugMarkerSetObjectNameEXT',
            'vkCmdDebugMarkerBeginEXT',
            'vkCmdDebugMarkerEndEXT',
            'vkCmdDebugMarkerInsertEXT',
        ]

    def generate(self):
        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
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
        self.write('// NOLINTBEGIN') # Wrap for clang-tidy to ignore

        if self.filename == 'thread_safety.cpp':
            self.generateSource()
        elif self.filename == 'thread_safety_commands.h':
            self.generateCommands()
        elif self.filename == 'thread_safety_counter_definitions.h':
            self.generateCounterDefinitions()
        elif self.filename == 'thread_safety_counter_instances.h':
            self.generateCounterInstance()
        elif self.filename == 'thread_safety_counter_bodies.h':
            self.generateCounterBodies()

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore


    def makeThreadUseBlock(self, command: Command, start: bool = False, finish: bool = False) -> str:
        prefix = 'Start' if start else 'Finish'
        out = []
        # Find and add any parameters that are thread unsafe
        for param in command.params:
            if param.externSyncPointer:
                if param.length:
                    # Externsync can list pointers to arrays of members to synchronize
                    out.append(f'if ({param.name}) {{\n')
                    out.append(f'    for (uint32_t index = 0; index < {param.length}; index++) {{\n')
                    for member in param.externSyncPointer:
                        # Replace first empty [] in member name with index
                        element = member.replace('[]','[index]',1)

                        # XXX TODO: Can we do better to lookup types of externsync members?
                        suffix = ''
                        if 'surface' in member:
                            suffix = 'ParentInstance'

                        if '[]' in element:
                            # TODO: These null checks can be removed if threading ends up behind parameter
                            #       validation in layer order
                            element_ptr = element.split('[]')[0]
                            out.append(f'if ({element_ptr}) {{\n')
                            # Replace any second empty [] in element name with inner array index based on mapping array
                            # names like "pSomeThings[]" to "someThingCount" array size. This could be more robust by
                            # mapping a param member name to a struct type and "len" attribute.
                            limit = element[0:element.find('s[]')] + 'Count'
                            dotp = limit.rfind('.p')
                            limit = limit[0:dotp+1] + limit[dotp+2:dotp+3].lower() + limit[dotp+3:]
                            out.append(f'for (uint32_t index2=0; index2 < {limit}; index2++) {{\n')
                            element = element.replace('[]','[index2]')
                            out.append(f'    {prefix}WriteObject{suffix}({element}, vvl::Func::{command.name});')
                            out.append('}\n')
                            out.append('}\n')
                        else:
                            out.append(f'{prefix}WriteObject{suffix}({element}, vvl::Func::{command.name});\n')
                    out.append('}\n')
                    out.append('}\n')
                else:
                    # externsync can list members to synchronize
                    for member in param.externSyncPointer:
                        member = str(member).replace("::", "->")
                        member = str(member).replace(".", "->")
                        suffix = 'ParentInstance' if 'surface' in member else ''
                        out.append(f'    {prefix}WriteObject{suffix}({member}, vvl::Func::{command.name});\n')
            elif param.externSync:
                if param.length:
                    out.append(f'''
                        if ({param.name}) {{
                            for (uint32_t index = 0; index < {param.length}; index++) {{
                                {prefix}WriteObject{GetParentInstance(param)}({param.name}[index], vvl::Func::{command.name});
                            }}
                        }}\n''')
                else:
                    out.append(f'{prefix}WriteObject{GetParentInstance(param)}({param.name}, vvl::Func::{command.name});\n')
                    if ('Destroy' in command.name or 'Free' in command.name or 'ReleasePerformanceConfigurationINTEL' in command.name) and prefix == 'Finish':
                        out.append(f'DestroyObject{GetParentInstance(param)}({param.name});\n')
            elif param.pointer and ('Create' in command.name or 'Allocate' in command.name or 'AcquirePerformanceConfigurationINTEL' in command.name) and prefix == 'Finish':
                if param.type in self.vk.handles:
                    create_pipelines_call = True
                    create_shaders_call = True
                    # The CreateXxxPipelines/CreateShaders APIs can return a list of partly created pipelines/shaders upon failure
                    if not ('Create' in command.name and 'Pipelines' in command.name):
                        create_pipelines_call = False
                    if not ('Create' in command.name and 'Shaders' in command.name):
                        create_shaders_call = False
                    if not create_pipelines_call and not create_shaders_call:
                        out.append('if (record_obj.result == VK_SUCCESS) {\n')
                        create_pipelines_call = False
                    if param.length:
                        # Add pointer dereference for array counts that are pointer values
                        dereference = ''
                        for candidate in command.params:
                            if param.length == candidate.name:
                                if candidate.pointer:
                                    dereference = '*'
                        param_len = param.length.replace("::", "->")
                        out.append(f'if ({param.name}) {{\n')
                        out.append(f'    for (uint32_t index = 0; index < {dereference}{param_len}; index++) {{\n')
                        if create_pipelines_call:
                            out.append('if (!pPipelines[index]) continue;\n')
                        if create_shaders_call:
                            out.append('if (!pShaders[index]) continue;\n')
                        out.append(f'CreateObject{GetParentInstance(param)}({param.name}[index]);\n')
                        out.append('}\n')
                        out.append('}\n')

                    else:
                        out.append(f'CreateObject{GetParentInstance(param)}(*{param.name});\n')
                    if not create_pipelines_call and not create_shaders_call:
                        out.append('}\n')
            else:
                if param.type in self.vk.handles and param.type != 'VkPhysicalDevice':
                    if param.length and ('pPipelines' != param.name) and ('pShaders' != param.name or 'Create' not in command.name):
                        # Add pointer dereference for array counts that are pointer values
                        dereference = ''
                        for candidate in command.params:
                            if param.length == candidate.name:
                                if candidate.pointer:
                                    dereference = '*'
                        param_len = param.length.replace("::", "->")
                        out.append(f'''
                            if ({param.name}) {{
                                for (uint32_t index = 0; index < {dereference}{param.length}; index++) {{
                                    {prefix}ReadObject{GetParentInstance(param)}({param.name}[index], vvl::Func::{command.name});
                                }}
                            }}\n''')
                    elif not param.pointer:
                        # Pointer params are often being created.
                        # They are not being read from.
                        out.append(f'{prefix}ReadObject{GetParentInstance(param)}({param.name}, vvl::Func::{command.name});\n')


        for param in [x for x in command.params if x.externSync]:
            out.append('// Host access to ')
            if param.externSyncPointer:
                out.append(",".join(param.externSyncPointer))
            else:
                if param.length:
                    out.append(f'each member of {param.name}')
                elif param.pointer:
                    out.append(f'the object referenced by {param.name}')
                else:
                    out.append(param.name)
            out.append(' must be externally synchronized\n')

        # Find and add any "implicit" parameters that are thread unsafe
        out.extend([f'// {x} must be externally synchronized between host accesses\n' for x in command.implicitExternSyncParams])

        return "".join(out) if len(out) > 0 else None

    def generateSource(self):
        out = []
        out.append('''
            #include "chassis.h"
            #include "thread_tracker/thread_safety_validation.h"
            ''')
        for command in [x for x in self.vk.commands.values() if x.name not in self.blacklist and x.name not in self.manual_commands]:
            # Determine first if this function needs to be intercepted
            startThreadSafety = self.makeThreadUseBlock(command, start=True)
            finishThreadSafety = self.makeThreadUseBlock(command, finish=True)
            if startThreadSafety is None and finishThreadSafety is None:
                continue

            out.extend([f'#ifdef {command.protect}\n'] if command.protect else [])
            prototype = command.cPrototype.split('VKAPI_CALL ')[1]
            prototype = f'void ThreadSafety::PreCallRecord{prototype[2:]}'
            prototype = prototype.replace(');', ') {\n')
            out.append(prototype)
            out.extend([startThreadSafety] if startThreadSafety is not None else [])
            out.append('}\n\n')

            prototype = prototype.replace('PreCallRecord', 'PostCallRecord')
            prototype = prototype.replace(')', ', const RecordObject& record_obj)')
            out.append(prototype)
            out.extend([finishThreadSafety] if finishThreadSafety is not None else [])
            out.append('}\n\n')

            out.extend([f'#endif // {command.protect}\n'] if command.protect else [])
        self.write("".join(out))

    def generateCommands(self):
        out = []
        for command in [x for x in self.vk.commands.values() if x.name not in self.blacklist]:
            # Determine first if this function needs to be intercepted
            startThreadSafety = self.makeThreadUseBlock(command, start=True)
            finishThreadSafety = self.makeThreadUseBlock(command, finish=True)
            if startThreadSafety is None and finishThreadSafety is None:
                continue

            out.extend([f'#ifdef {command.protect}\n'] if command.protect else [])

            prototype = command.cPrototype.split('VKAPI_CALL ')[1]
            prototype = f'void PreCallRecord{prototype[2:]}'
            prototype = prototype.replace(');', ') override;\n\n')
            if 'ValidationCache' in command.name:
                prototype = prototype.replace(' override;', ';')
            out.append(prototype)

            prototype = prototype.replace('PreCallRecord', 'PostCallRecord')
            prototype = prototype.replace(')', ', const RecordObject& record_obj)')
            out.append(prototype)

            out.extend([f'#endif // {command.protect}\n'] if command.protect else [])
        self.write("".join(out))

    def generateCounterDefinitions(self):
        out = []
        out.append('// clang-format off\n')
        for handle in [x for x in self.vk.handles.values() if not x.dispatchable]:
            out.extend([f'#ifdef {handle.protect}\n'] if handle.protect else [])
            out.append(f'counter<{handle.name}> c_{handle.name};\n')
            out.extend([f'#endif // {handle.protect}\n'] if handle.protect else [])
        out.append('// clang-format on\n')
        self.write("".join(out))

    def generateCounterInstance(self):
        out = []
        out.append('// clang-format off\n')
        for handle in [x for x in self.vk.handles.values() if not x.dispatchable]:
            out.extend([f'#ifdef {handle.protect}\n'] if handle.protect else [])
            out.append(f'c_{handle.name}(kVulkanObjectType{handle.name[2:]}, this),\n')
            out.extend([f'#endif // {handle.protect}\n'] if handle.protect else [])
        out.append('// clang-format on\n')
        self.write("".join(out))

    def generateCounterBodies(self):
        out = []
        out.append('// clang-format off\n')
        instanceParent = ['VkSurfaceKHR', 'VkDebugReportCallbackEXT', 'VkDebugUtilsMessengerEXT', 'VkDisplayKHR']
        for handle in [x for x in self.vk.handles.values() if not x.dispatchable]:
            out.extend([f'#ifdef {handle.protect}\n'] if handle.protect else [])
            out.extend([f'WRAPPER_PARENT_INSTANCE({handle.name})\n'] if handle.name in instanceParent else [f'WRAPPER({handle.name})\n'])
            out.extend([f'#endif // {handle.protect}\n'] if handle.protect else [])
        out.append('// clang-format on\n')
        self.write("".join(out))
