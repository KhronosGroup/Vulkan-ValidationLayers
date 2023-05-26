#!/usr/bin/python3 -i
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
from generator import *
from common_codegen import *

# ThreadOutputGenerator - Generate Thread checking framework
class ThreadOutputGenerator(OutputGenerator):
    inline_copyright_message = """
// This file is ***GENERATED***.  Do Not Edit.
// See thread_safety_generator.py for modifications.

/* Copyright (c) 2015-2023 The Khronos Group Inc.
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
 */"""

    # This is an ordered list of sections in the header file.
    ALL_SECTIONS = ['command']
    def __init__(self,
                 errFile = sys.stderr,
                 warnFile = sys.stderr,
                 diagFile = sys.stdout):
        OutputGenerator.__init__(self, errFile, warnFile, diagFile)
        # Internal state - accumulators for different inner block text
        self.sections = dict([(section, []) for section in self.ALL_SECTIONS])
        self.non_dispatchable_types = set()

    # Check if the parameter passed in is a pointer to an array
    def paramIsArray(self, param):
        return param.attrib.get('len') is not None

    # Check if the parameter passed in is a pointer
    def paramIsPointer(self, param):
        ispointer = False
        for elem in param:
            if elem.tag == 'type' and elem.tail is not None and '*' in elem.tail:
                ispointer = True
        return ispointer

    # Map paramtype to a thread safety suffix, either 'ParentInstance' or ''
    def paramSuffix(self, paramtype):
        if paramtype is not None:
            paramtype = paramtype.text
        else:
            paramtype = 'None'

        # Use 'in' to check the types, to handle suffixes and pointers, except for VkDevice
        # which can be confused with VkDeviceMemory
        suffix = ''
        if 'VkSurface' in paramtype or 'VkSwapchainKHR' in paramtype or 'VkDebugReportCallback' in paramtype or 'VkDebugUtilsMessenger' in paramtype or 'VkDevice' == paramtype or 'VkDevice*' == paramtype or 'VkInstance' in paramtype or 'VkDisplayKHR' in paramtype:
            suffix = 'ParentInstance'
        return suffix

    def makeThreadUseBlock(self, cmd, name, functionprefix):
        """Generate C function pointer typedef for <command> Element"""
        paramdecl = ''
        # Find and add any parameters that are thread unsafe
        params = cmd.findall('param')
        for param in params:
            paramname = param.find('name')
            if False: # self.paramIsPointer(param):
                paramdecl += '    // not watching use of pointer ' + paramname.text + '\n'
            else:
                externsync = param.attrib.get('externsync')
                if externsync == 'true':
                    if self.paramIsArray(param):
                        paramdecl += 'if (' + paramname.text + ') {\n'
                        paramdecl += '    for (uint32_t index=0; index < ' + param.attrib.get('len') + '; index++) {\n'
                        paramdecl += '        ' + functionprefix + 'WriteObject' + self.paramSuffix(param.find('type')) + '(' + paramname.text + '[index], "' + name + '");\n'
                        paramdecl += '    }\n'
                        paramdecl += '}\n'
                    else:
                        paramdecl += functionprefix + 'WriteObject' + self.paramSuffix(param.find('type')) + '(' + paramname.text + ', "' + name + '");\n'
                        if ('Destroy' in name or 'Free' in name or 'ReleasePerformanceConfigurationINTEL' in name) and functionprefix == 'Finish':
                            paramdecl += 'DestroyObject' + self.paramSuffix(param.find('type')) + '(' + paramname.text + ');\n'
                elif (param.attrib.get('externsync')):
                    if self.paramIsArray(param):
                        # Externsync can list pointers to arrays of members to synchronize
                        paramdecl += 'if (' + paramname.text + ') {\n'
                        paramdecl += '    for (uint32_t index=0; index < ' + param.attrib.get('len') + '; index++) {\n'
                        second_indent = '    '
                        for member in externsync.split(","):
                            # Replace first empty [] in member name with index
                            element = member.replace('[]','[index]',1)

                            # XXX TODO: Can we do better to lookup types of externsync members?
                            suffix = ''
                            if 'surface' in member or 'swapchain' in member.lower():
                                suffix = 'ParentInstance'

                            if '[]' in element:
                                # TODO: These null checks can be removed if threading ends up behind parameter
                                #       validation in layer order
                                element_ptr = element.split('[]')[0]
                                paramdecl += '        if (' + element_ptr + ') {\n'
                                # Replace any second empty [] in element name with inner array index based on mapping array
                                # names like "pSomeThings[]" to "someThingCount" array size. This could be more robust by
                                # mapping a param member name to a struct type and "len" attribute.
                                limit = element[0:element.find('s[]')] + 'Count'
                                dotp = limit.rfind('.p')
                                limit = limit[0:dotp+1] + limit[dotp+2:dotp+3].lower() + limit[dotp+3:]
                                paramdecl += '            for (uint32_t index2=0; index2 < '+limit+'; index2++) {\n'
                                element = element.replace('[]','[index2]')
                                second_indent = '        '
                                paramdecl += '        ' + second_indent + functionprefix + 'WriteObject' + suffix + '(' + element + ', "' + name + '");\n'
                                paramdecl += '            }\n'
                                paramdecl += '        }\n'
                            else:
                                paramdecl += '    ' + second_indent + functionprefix + 'WriteObject' + suffix + '(' + element + ', "' + name + '");\n'
                        paramdecl += '    }\n'
                        paramdecl += '}\n'
                    else:
                        # externsync can list members to synchronize
                        for member in externsync.split(","):
                            member = str(member).replace("::", "->")
                            member = str(member).replace(".", "->")
                            # XXX TODO: Can we do better to lookup types of externsync members?
                            suffix = ''
                            if 'surface' in member or 'swapchain' in member.lower():
                                suffix = 'ParentInstance'
                            paramdecl += '    ' + functionprefix + 'WriteObject' + suffix + '(' + member + ', "' + name + '");\n'
                elif self.paramIsPointer(param) and ('Create' in name or 'Allocate' in name or 'AcquirePerformanceConfigurationINTEL' in name) and functionprefix == 'Finish':
                    paramtype = param.find('type')
                    if paramtype is not None:
                        paramtype = paramtype.text
                    else:
                        paramtype = 'None'
                    if paramtype in self.handle_types:
                        indent = ''
                        create_pipelines_call = True
                        create_shaders_call = True
                        # The CreateXxxPipelines/CreateShaders APIs can return a list of partly created pipelines/shaders upon failure
                        if not ('Create' in name and 'Pipelines' in name):
                            create_pipelines_call = False
                        if not ('Create' in name and 'Shaders' in name):
                            create_shaders_call = False
                        if not (create_pipelines_call or create_shaders_call):
                            paramdecl += 'if (result == VK_SUCCESS) {\n'
                            create_pipelines_call = False
                            indent = '    '
                        if self.paramIsArray(param):
                            # Add pointer dereference for array counts that are pointer values
                            dereference = ''
                            for candidate in params:
                                if param.attrib.get('len') == candidate.find('name').text:
                                    if self.paramIsPointer(candidate):
                                        dereference = '*'
                            param_len = str(param.attrib.get('len')).replace("::", "->")
                            paramdecl += indent + 'if (' + paramname.text + ') {\n'
                            paramdecl += indent + '    for (uint32_t index = 0; index < ' + dereference + param_len + '; index++) {\n'
                            if create_pipelines_call:
                                paramdecl += indent + '        if (!pPipelines[index]) continue;\n'
                            if create_shaders_call:
                                paramdecl += indent + '        if (!pShaders[index]) continue;\n'
                            paramdecl += indent + '        CreateObject' + self.paramSuffix(param.find('type')) + '(' + paramname.text + '[index]);\n'
                            paramdecl += indent + '    }\n'
                            paramdecl += indent + '}\n'
                        else:
                            paramdecl += '    CreateObject' + self.paramSuffix(param.find('type')) + '(*' + paramname.text + ');\n'
                        if not create_pipelines_call and not create_shaders_call:
                            paramdecl += '}\n'
                else:
                    paramtype = param.find('type')
                    if paramtype is not None:
                        paramtype = paramtype.text
                    else:
                        paramtype = 'None'
                    if paramtype in self.handle_types and paramtype != 'VkPhysicalDevice':
                        if self.paramIsArray(param) and ('pPipelines' != paramname.text) and ('pShaders' != paramname.text or not 'Create' in name):
                            # Add pointer dereference for array counts that are pointer values
                            dereference = ''
                            for candidate in params:
                                if param.attrib.get('len') == candidate.find('name').text:
                                    if self.paramIsPointer(candidate):
                                        dereference = '*'
                            param_len = str(param.attrib.get('len')).replace("::", "->")
                            paramdecl += 'if (' + paramname.text + ') {\n'
                            paramdecl += '    for (uint32_t index = 0; index < ' + dereference + param_len + '; index++) {\n'
                            paramdecl += '        ' + functionprefix + 'ReadObject' + self.paramSuffix(param.find('type')) + '(' + paramname.text + '[index], "' + name + '");\n'
                            paramdecl += '    }\n'
                            paramdecl += '}\n'
                        elif not self.paramIsPointer(param):
                            # Pointer params are often being created.
                            # They are not being read from.
                            paramdecl += functionprefix + 'ReadObject' + self.paramSuffix(param.find('type')) + '(' + paramname.text + ', "' + name + '");\n'
        explicitexternsyncparams = cmd.findall("param[@externsync]")
        if (explicitexternsyncparams is not None):
            for param in explicitexternsyncparams:
                externsyncattrib = param.attrib.get('externsync')
                paramname = param.find('name')
                paramdecl += '// Host access to '
                if externsyncattrib == 'true':
                    if self.paramIsArray(param):
                        paramdecl += 'each member of ' + paramname.text
                    elif self.paramIsPointer(param):
                        paramdecl += 'the object referenced by ' + paramname.text
                    else:
                        paramdecl += paramname.text
                else:
                    paramdecl += externsyncattrib
                paramdecl += ' must be externally synchronized\n'

        # Find and add any "implicit" parameters that are thread unsafe
        implicitexternsyncparams = cmd.find('implicitexternsyncparams')
        if (implicitexternsyncparams is not None):
            for elem in implicitexternsyncparams:
                paramdecl += '// '
                paramdecl += elem.text
                paramdecl += ' must be externally synchronized between host accesses\n'

        if (paramdecl == ''):
            return None
        else:
            return paramdecl
    def beginFile(self, genOpts):
        OutputGenerator.beginFile(self, genOpts)

        # Initialize members that require the tree
        self.handle_types = GetHandleTypes(self.registry.tree)
        self.is_aliased_type = GetHandleAliased(self.registry.tree)
        self.type_guards = GetTypeGuards(self.registry.tree)

        write(self.inline_copyright_message, file=self.outFile)

        self.counter_definitions_header_file = (genOpts.filename == 'thread_safety_counter_definitions.h')
        self.counter_instances_header_file = (genOpts.filename == 'thread_safety_counter_instances.h')
        self.counter_bodies_header_file = (genOpts.filename == 'thread_safety_counter_bodies.h')
        self.commands_file = (genOpts.filename == 'thread_safety_commands.h')
        self.source_file = (genOpts.filename == 'thread_safety.cpp')

        if not self.source_file \
            and not self.counter_definitions_header_file \
            and not self.counter_instances_header_file \
            and not self.counter_bodies_header_file \
            and not self.commands_file:
                print("Error: Output Filenames have changed, update generator source.\n")
                sys.exit(1)

        if self.source_file:
            write('#include "chassis.h"', file=self.outFile)
            write('#include "thread_tracker/thread_safety_validation.h"', file=self.outFile)
            self.newline()


    def endFile(self):

        # Create class definitions
        counter_class_defs = ''
        counter_class_instances = ''
        counter_class_bodies = ''

        for obj in sorted(self.non_dispatchable_types):
            if (not self.is_aliased_type[obj]):
                obj_guard = self.type_guards.get(obj)
                counter_class_defs += Guarded(obj_guard, 'counter<%s> c_%s;\n' % (obj, obj))
                obj_type = 'kVulkanObjectType' + obj[2:]
                counter_class_instances += Guarded(obj_guard, 'c_%s("%s", %s, this),\n' % (obj, obj, obj_type))
                if 'VkSurface' in obj or 'VkSwapchainKHR' in obj or 'VkDebugReportCallback' in obj or 'VkDebugUtilsMessenger' in obj or 'VkDisplayKHR' in obj:
                    counter_class_bodies += 'WRAPPER_PARENT_INSTANCE(%s)\n' % obj
                else:
                    counter_class_bodies += Guarded(obj_guard, 'WRAPPER(%s)\n' % obj)

        if self.commands_file or self.source_file:
            write('\n'.join(self.sections['command']), file=self.outFile)
        if self.counter_definitions_header_file:
            write(counter_class_defs, file=self.outFile)
        if self.counter_instances_header_file:
            write(counter_class_instances, file=self.outFile)
        if self.counter_bodies_header_file:
            write(counter_class_bodies, file=self.outFile)

        # Finish processing in superclass
        OutputGenerator.endFile(self)

    def beginFeature(self, interface, emit):
        #write('// starting beginFeature', file=self.outFile)
        # Start processing in superclass
        OutputGenerator.beginFeature(self, interface, emit)
        # C-specific
        # Accumulate includes, defines, types, enums, function pointer typedefs,
        # end function prototypes separately for this feature. They're only
        # printed in endFeature().
        self.featureExtraProtect = GetFeatureProtect(interface)
        if (self.featureExtraProtect is not None):
            self.appendSection('command', '\n#ifdef %s' % self.featureExtraProtect)

        #write('// ending beginFeature', file=self.outFile)
    def endFeature(self):
        # C-specific
        if (self.emit):
            if (self.featureExtraProtect is not None):
                self.appendSection('command', '#endif // %s' % self.featureExtraProtect)
        # Finish processing in superclass
        OutputGenerator.endFeature(self)
    #
    # Append a definition to the specified section
    def appendSection(self, section, text):
        self.sections[section].append(text)
    #
    # Type generation
    def genType(self, typeinfo, name, alias):
        OutputGenerator.genType(self, typeinfo, name, alias)
        if self.handle_types.IsNonDispatchable(name):
            self.non_dispatchable_types.add(name)
    #
    # Struct (e.g. C "struct" type) generation.
    # This is a special case of the <type> tag where the contents are
    # interpreted as a set of <member> tags instead of freeform C
    # C type declarations. The <member> tags are just like <param>
    # tags - they are a declaration of a struct or union member.
    # Only simple member declarations are supported (no nested
    # structs etc.)
    def genStruct(self, typeinfo, typeName, alias):
        OutputGenerator.genStruct(self, typeinfo, typeName, alias)
        body = 'typedef ' + typeinfo.elem.get('category') + ' ' + typeName + ' {\n'
        # paramdecl = self.makeCParamDecl(typeinfo.elem, self.genOpts.alignFuncParam)
        for member in typeinfo.elem.findall('.//member'):
            body += self.makeCParamDecl(member, self.genOpts.alignFuncParam)
            body += ';\n'
        body += '} ' + typeName + ';\n'
        self.appendSection('struct', body)
    #
    # Group (e.g. C "enum" type) generation.
    # These are concatenated together with other types.
    def genGroup(self, groupinfo, groupName, alias):
        pass
    # Enumerant generation
    # <enum> tags may specify their values in several ways, but are usually
    # just integers.
    def genEnum(self, enuminfo, name, alias):
        pass
    #
    # Command generation
    def genCmd(self, cmdinfo, name, alias):
        # Commands shadowed by interface functions and are not implemented
        special_functions = [
            'vkAllocateCommandBuffers',
            'vkFreeCommandBuffers',
            'vkCreateCommandPool',
            'vkResetCommandPool',
            'vkDestroyCommandPool',
            'vkAllocateDescriptorSets',
            'vkFreeDescriptorSets',
            'vkResetDescriptorPool',
            'vkDestroyDescriptorPool',
            'vkQueuePresentKHR',
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
        ]
        if name == 'vkQueuePresentKHR' or (name in special_functions and self.source_file):
            return

        if (("DebugMarker" in name or "DebugUtilsObject" in name) and "EXT" in name):
            self.appendSection('command', '// TODO - not wrapping EXT function ' + name)
            return

        # Determine first if this function needs to be intercepted
        startthreadsafety = self.makeThreadUseBlock(cmdinfo.elem, name, 'Start')
        finishthreadsafety = self.makeThreadUseBlock(cmdinfo.elem, name, 'Finish')
        if startthreadsafety is None and finishthreadsafety is None:
            return

        if startthreadsafety is None:
            startthreadsafety = ''
        if finishthreadsafety is None:
            finishthreadsafety = ''

        OutputGenerator.genCmd(self, cmdinfo, name, alias)

        # setup common to call wrappers
        # first parameter is always dispatchable
        dispatchable_type = cmdinfo.elem.find('param/type').text
        dispatchable_name = cmdinfo.elem.find('param/name').text

        decls = self.makeCDecls(cmdinfo.elem)

        result_type = cmdinfo.elem.find('proto/type')

        if self.source_file:
            pre_decl = decls[0][:-1]
            pre_decl = pre_decl.split("VKAPI_CALL ")[1]
            pre_decl = 'void ThreadSafety::PreCallRecord' + pre_decl + ' {'

            # PreCallRecord
            self.appendSection('command', '')
            self.appendSection('command', pre_decl)
            self.appendSection('command', "    " + "\n    ".join(str(startthreadsafety).rstrip().split("\n")))
            self.appendSection('command', '}')

            # PostCallRecord
            post_decl = pre_decl.replace('PreCallRecord', 'PostCallRecord')
            if result_type.text == 'VkResult':
                post_decl = post_decl.replace(')', ',\n    VkResult                                    result)')
            elif result_type.text == 'VkDeviceAddress':
                post_decl = post_decl.replace(')', ',\n    VkDeviceAddress                             result)')
            self.appendSection('command', '')
            self.appendSection('command', post_decl)
            self.appendSection('command', "    " + "\n    ".join(str(finishthreadsafety).rstrip().split("\n")))
            self.appendSection('command', '}')

        if self.commands_file:
            pre_decl = decls[0][:-1]
            pre_decl = pre_decl.split("VKAPI_CALL ")[1]
            decl_terminator = ';'
            if 'ValidationCache' not in pre_decl:
                decl_terminator = ' override;'
            pre_decl = 'void PreCallRecord' + pre_decl + decl_terminator

            # PreCallRecord
            self.appendSection('command', '')
            self.appendSection('command', pre_decl)

            # PostCallRecord
            post_decl = pre_decl.replace('PreCallRecord', 'PostCallRecord')
            if result_type.text == 'VkResult':
                post_decl = post_decl.replace(')', ',\n    VkResult                                    result)')
            elif result_type.text == 'VkDeviceAddress':
                post_decl = post_decl.replace(')', ',\n    VkDeviceAddress                             result)')
            self.appendSection('command', '')
            self.appendSection('command', post_decl)

    #
    # override makeProtoName to drop the "vk" prefix
    def makeProtoName(self, name, tail):
        return self.genOpts.apientry + name[2:] + tail
