#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2022 The Khronos Group Inc.
# Copyright (c) 2015-2022 Valve Corporation
# Copyright (c) 2015-2022 LunarG, Inc.
# Copyright (c) 2015-2022 Google Inc.
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
#
# Author: Mark Lobodzinski <mark@lunarg.com>

import os,re,sys
import xml.etree.ElementTree as etree
from generator import *
from collections import namedtuple
from common_codegen import *

#
# DispatchTableHelperOutputGeneratorOptions - subclass of GeneratorOptions.
class DispatchTableHelperOutputGeneratorOptions(GeneratorOptions):
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
                 genFuncPointers = True,
                 apicall = 'VKAPI_ATTR ',
                 apientry = 'VKAPI_CALL ',
                 apientryp = 'VKAPI_PTR *',
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
                sortProcedure = sortProcedure)
        self.genFuncPointers = genFuncPointers
        self.apicall         = apicall
        self.apientry        = apientry
        self.apientryp       = apientryp
        self.alignFuncParam  = alignFuncParam
#
# DispatchTableHelperOutputGenerator - subclass of OutputGenerator.
# Generates dispatch table helper header files for LVL
class DispatchTableHelperOutputGenerator(OutputGenerator):
    """Generate dispatch table helper header based on XML element attributes"""
    def __init__(self,
                 errFile = sys.stderr,
                 warnFile = sys.stderr,
                 diagFile = sys.stdout):
        OutputGenerator.__init__(self, errFile, warnFile, diagFile)
        # Internal state - accumulators for different inner block text
        self.instance_dispatch_list = []      # List of entries for instance dispatch list
        self.device_dispatch_list = []        # List of entries for device dispatch list
        self.dev_ext_stub_list = []           # List of stub functions for device extension functions
        self.stub_list = []                   # List of functions with stubs (promoted or extensions)
        self.extension_type = ''

    #
    # Called once at the beginning of each run
    def beginFile(self, genOpts):
        OutputGenerator.beginFile(self, genOpts)

        # Initialize members that require the tree
        self.handle_types = GetHandleTypes(self.registry.tree)

        write("#pragma once", file=self.outFile)
        # File Comment
        file_comment = '// *** THIS FILE IS GENERATED - DO NOT EDIT ***\n'
        file_comment += '// See dispatch_helper_generator.py for modifications\n'
        write(file_comment, file=self.outFile)
        # Copyright Notice
        copyright =  '/*\n'
        copyright += ' * Copyright (c) 2015-2022 The Khronos Group Inc.\n'
        copyright += ' * Copyright (c) 2015-2022 Valve Corporation\n'
        copyright += ' * Copyright (c) 2015-2022 LunarG, Inc.\n'
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
        copyright += ' * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>\n'
        copyright += ' * Author: Jon Ashburn <jon@lunarg.com>\n'
        copyright += ' * Author: Mark Lobodzinski <mark@lunarg.com>\n'
        copyright += ' */\n'

        preamble = ''
        preamble += '#include <vulkan/vulkan.h>\n'
        preamble += '#include <vulkan/vk_layer.h>\n'
        preamble += '#include <cstring>\n'
        preamble += '#include <string>\n'
        preamble += '#include "vk_layer_dispatch_table.h"\n'
        preamble += '#include "vk_extension_helper.h"\n'

        write(copyright, file=self.outFile)
        write(preamble, file=self.outFile)
    #
    # Write generate and write dispatch tables to output file
    def endFile(self):
        ext_enabled_fcn = ''
        device_table = ''
        instance_table = ''
        ext_enabled_fcn += self.OutputExtEnabledFunction()
        device_table += self.OutputDispatchTableHelper('device')
        instance_table += self.OutputDispatchTableHelper('instance')

        for stub in self.dev_ext_stub_list:
            write(stub, file=self.outFile)

        write("\n\n", file=self.outFile)
        write(ext_enabled_fcn, file=self.outFile)
        write("\n", file=self.outFile)
        write(device_table, file=self.outFile);
        write("\n", file=self.outFile)
        write(instance_table, file=self.outFile);

        # Finish processing in superclass
        OutputGenerator.endFile(self)
    #
    # Processing at beginning of each feature or extension
    def beginFeature(self, interface, emit):
        OutputGenerator.beginFeature(self, interface, emit)
        self.featureExtraProtect = GetFeatureProtect(interface)
        self.extension_type = interface.get('type')
    #
    # Process commands, adding to appropriate dispatch tables
    def genCmd(self, cmdinfo, name, alias):
        OutputGenerator.genCmd(self, cmdinfo, name, alias)

        avoid_entries = ['vkCreateInstance',
                         'vkCreateDevice']
        # Get first param type
        params = cmdinfo.elem.findall('param')
        info = self.getTypeNameTuple(params[0])

        if name not in avoid_entries:
            self.AddCommandToDispatchList(name, info[0], self.featureExtraProtect, cmdinfo)

    #
    # Determine if this API should be ignored or added to the instance or device dispatch table
    def AddCommandToDispatchList(self, name, handle_type, protect, cmdinfo):
        if handle_type not in self.handle_types:
            return
        extension = "VK_VERSION" not in self.featureName
        promoted = not extension and "VK_VERSION_1_0" != self.featureName
        if promoted or extension:
            # We want feature written for all promoted entrypoints in addition to extensions
            self.stub_list.append([name, self.featureName])
            # Build up stub function
            return_type = ''
            decl = self.makeCDecls(cmdinfo.elem)[1]
            if decl.startswith('typedef VkResult'):
                return_type = 'return VK_SUCCESS;'
            elif decl.startswith('typedef VkDeviceAddress'):
                return_type = 'return 0;'
            elif decl.startswith('typedef VkDeviceSize'):
                return_type = 'return 0;'
            elif decl.startswith('typedef uint32_t'):
                return_type = 'return 0;'
            elif decl.startswith('typedef uint64_t'):
                return_type = 'return 0;'
            elif decl.startswith('typedef VkBool32'):
                return_type = 'return VK_FALSE;'
            pre_decl, decl = decl.split('*PFN_vk')
            pre_decl = pre_decl.replace('typedef ', '')
            pre_decl = pre_decl.split(' (')[0]
            decl = decl.replace(')(', '(')
            decl = 'static VKAPI_ATTR ' + pre_decl + ' VKAPI_CALL Stub' + decl
            func_body = ' { ' + return_type + ' };'
            decl = decl.replace (';', func_body)
            if self.featureExtraProtect is not None:
                self.dev_ext_stub_list.append('#ifdef %s' % self.featureExtraProtect)
            self.dev_ext_stub_list.append(decl)
            if self.featureExtraProtect is not None:
                self.dev_ext_stub_list.append('#endif // %s' % self.featureExtraProtect)
        if handle_type != 'VkInstance' and handle_type != 'VkPhysicalDevice' and name != 'vkGetInstanceProcAddr':
            self.device_dispatch_list.append((name, self.featureExtraProtect))
        else:
            self.instance_dispatch_list.append((name, self.featureExtraProtect))
        return
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
    # Output a function that'll determine if an extension is in the enabled list
    def OutputExtEnabledFunction(self):
        ext_fcn = ''
        # First, write out our static data structure -- map of all APIs that are part of extensions to their extension.
        ext_fcn += 'const layer_data::unordered_map<std::string, std::string> api_extension_map {\n'
        api_ext = dict()
        handles = GetHandleTypes(self.registry.tree)
        features = self.registry.tree.findall('feature') + self.registry.tree.findall('extensions/extension')
        for feature in features:
            feature_name = feature.get('name')
            if 'VK_VERSION_1_0' == feature_name:
                continue
            feature_supported = feature.get('supported')
            # If feature is not yet supported, skip it
            if feature_supported == 'disabled':
                continue
            for require_element in feature.findall('require'):
                for command in require_element.findall('command'):
                    command_name = command.get('name')
                    if 'EnumerateInstanceVersion' in command_name:
                        continue
                    disp_obj = self.registry.tree.find("commands/command/[@name='%s']/param/type" % command_name)
                    if disp_obj is None:
                        cmd_info = self.registry.tree.find("commands/command/[@name='%s']" % command_name)
                        alias_name = cmd_info.get('alias')
                        if alias_name is not None:
                            disp_obj = self.registry.tree.find("commands/command/[@name='%s']/param/type" % alias_name)
                    if 'VkInstance' != disp_obj.text and 'VkPhysicalDevice' != disp_obj.text:
                        # Ensure APIs belonging to multiple extensions match the existing order
                        if command_name not in api_ext:
                            api_ext[command_name] = feature_name
        for api in sorted(api_ext):
            ext_fcn += '    {"%s", "%s"},\n' % (api, api_ext[api])
        ext_fcn += '};\n\n'
        ext_fcn += '// Using the above code-generated map of APINames-to-parent extension names, this function will:\n'
        ext_fcn += '//   o  Determine if the API has an associated extension\n'
        ext_fcn += '//   o  If it does, determine if that extension name is present in the passed-in set of enabled_ext_names \n'
        ext_fcn += '//   If the APIname has no parent extension, OR its parent extension name is IN the set, return TRUE, else FALSE\n'
        ext_fcn += 'static inline bool ApiParentExtensionEnabled(const std::string api_name, const DeviceExtensions *device_extension_info) {\n'
        ext_fcn += '    auto has_ext = api_extension_map.find(api_name);\n'
        ext_fcn += '    // Is this API part of an extension or feature group?\n'
        ext_fcn += '    if (has_ext != api_extension_map.end()) {\n'
        ext_fcn += '        // Was the extension for this API enabled in the CreateDevice call?\n'
        ext_fcn += '        auto info = device_extension_info->get_info(has_ext->second.c_str());\n'
        ext_fcn += '        if ((!info.state) || (device_extension_info->*(info.state) != kEnabledByCreateinfo)) {\n'
        ext_fcn += '            return false;\n'
        ext_fcn += '        }\n'
        ext_fcn += '    }\n'
        ext_fcn += '    return true;\n'
        ext_fcn += '}\n'
        return ext_fcn
    #
    # Create a dispatch table from the appropriate list and return it as a string
    def OutputDispatchTableHelper(self, table_type):
        entries = []
        table = ''
        if table_type == 'device':
            entries = self.device_dispatch_list
            table += 'static inline void layer_init_device_dispatch_table(VkDevice device, VkLayerDispatchTable *table, PFN_vkGetDeviceProcAddr gpa) {\n'
            table += '    memset(table, 0, sizeof(*table));\n'
            table += '    // Device function pointers\n'
        else:
            entries = self.instance_dispatch_list
            table += 'static inline void layer_init_instance_dispatch_table(VkInstance instance, VkLayerInstanceDispatchTable *table, PFN_vkGetInstanceProcAddr gpa) {\n'
            table += '    memset(table, 0, sizeof(*table));\n'
            table += '    // Instance function pointers\n'
            table += '    table->GetPhysicalDeviceProcAddr = (PFN_GetPhysicalDeviceProcAddr) gpa(instance, "vk_layerGetPhysicalDeviceProcAddr");\n'

        stubbed_functions = dict(self.stub_list)
        for item in entries:
            # Remove 'vk' from proto name
            base_name = item[0][2:]

            if item[1] is not None:
                table += '#ifdef %s\n' % item[1]

            # If we're looking for the proc we are passing in, just point the table to it.  This fixes the issue where
            # a layer overrides the function name for the loader.
            if ('device' in table_type and base_name == 'GetDeviceProcAddr'):
                table += '    table->GetDeviceProcAddr = gpa;\n'
            elif ('device' not in table_type and base_name == 'GetInstanceProcAddr'):
                table += '    table->GetInstanceProcAddr = gpa;\n'
            else:
                table += '    table->%s = (PFN_%s) gpa(%s, "%s");\n' % (base_name, item[0], table_type, item[0])
                if item[0] in stubbed_functions:
                    stub_check = '    if (table->%s == nullptr) { table->%s = (PFN_%s)Stub%s; }\n' % (base_name, base_name, item[0], base_name)
                    table += stub_check
            if item[1] is not None:
                table += '#endif // %s\n' % item[1]

        table += '}'
        return table
