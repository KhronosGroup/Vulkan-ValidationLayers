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

import sys,os
from generator import *
from common_codegen import *
from generators.base_generator import BaseGenerator


# DispatchTableHelperOutputGenerator - subclass of OutputGenerator.
# Generates dispatch table helper header files for LVL
class DispatchTableHelperOutputGenerator(BaseGenerator):
    """Generate dispatch table helper header based on XML element attributes"""
    def __init__(self,
                 errFile = sys.stderr,
                 warnFile = sys.stderr,
                 diagFile = sys.stdout):
        BaseGenerator.__init__(self, errFile, warnFile, diagFile)
        # Internal state - accumulators for different inner block text
        self.instance_dispatch_list = []      # List of entries for instance dispatch list
        self.device_dispatch_list = []        # List of entries for device dispatch list
        self.dev_ext_stub_list = []           # List of stub functions for device extension functions
        self.stub_list = []                   # List of functions with stubs (promoted or extensions)

    #
    # Write generate and write dispatch tables to output file
    def endFile(self):
        self.BuildDispatchCommands()
        ext_enabled_fcn = ''
        device_table = ''
        instance_table = ''
        ext_enabled_fcn += self.OutputExtEnabledFunction()
        device_table += self.OutputDispatchTableHelper('device')
        instance_table += self.OutputDispatchTableHelper('instance')

        write("#pragma once", file=self.outFile)
        # File Comment
        file_comment = '// *** THIS FILE IS GENERATED - DO NOT EDIT ***\n'
        file_comment += '// See {} for modifications\n'.format(os.path.basename(__file__))
        write(file_comment, file=self.outFile)
        # Copyright Notice
        copyright =  '/*\n'
        copyright += ' * Copyright (c) 2015-2023 The Khronos Group Inc.\n'
        copyright += ' * Copyright (c) 2015-2023 Valve Corporation\n'
        copyright += ' * Copyright (c) 2015-2023 LunarG, Inc.\n'
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

        for stub in self.dev_ext_stub_list:
            write(stub, file=self.outFile)

        write("\n\n", file=self.outFile)
        write(ext_enabled_fcn, file=self.outFile)
        write("\n", file=self.outFile)
        write(device_table, file=self.outFile);
        write("\n", file=self.outFile)
        write(instance_table, file=self.outFile);

        # Finish processing in superclass
        BaseGenerator.endFile(self)

    def BuildDispatchCommands(self):
        avoid_entries = ['vkCreateInstance', 'vkCreateDevice']
        for name, command in self.vk.commands.items():
            handle_type = command.params[0].type
            if name in avoid_entries:
                continue
            if handle_type not in self.handle_types:
                continue
            if command.feature.extension or command.feature.IsPromotedCore():
                # We want feature written for all promoted entrypoints in addition to extensions
                self.stub_list.append([name, command.feature.name])
                # Build up stub function
                return_type = ''
                decl = command.cFunctionPointer
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
                if command.feature.protect is not None:
                    self.dev_ext_stub_list.append('#ifdef %s' % command.feature.protect)
                self.dev_ext_stub_list.append(decl)
                if command.feature.protect is not None:
                    self.dev_ext_stub_list.append('#endif // %s' % command.feature.protect)
            if handle_type != 'VkInstance' and handle_type != 'VkPhysicalDevice' and name != 'vkGetInstanceProcAddr':
                self.device_dispatch_list.append((name, command.feature.protect))
            else:
                self.instance_dispatch_list.append((name, command.feature.protect))

    #
    # Output a function that'll determine if an extension is in the enabled list
    def OutputExtEnabledFunction(self):
        ext_fcn = ''
        # First, write out our static data structure -- map of all APIs that are part of extensions to their extension.
        api_ext = dict()
        max_ext_len = 1
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
                            api_ext[command_name] = [feature_name]
                        elif feature_name not in api_ext[command_name]:
                            api_ext[command_name].append(feature_name)
                        if (ml := len(api_ext[command_name])) > max_ext_len:
                            max_ext_len = ml
        ext_fcn += f'const vvl::unordered_map<std::string, small_vector<std::string, {max_ext_len}, size_t>> api_extension_map {{\n'
        for api in sorted(api_ext):
            api_exts_formatted = ', '.join([f'"{e}"' for e in api_ext[api]])
            ext_fcn += f'    {{ "{api}", {{ {api_exts_formatted} }} }},\n'
        ext_fcn += '''};


// Using the above code-generated map of APINames-to-parent extension names, this function will:
//   o  Determine if the API has an associated extension
//   o  If it does, determine if that extension name is present in the passed-in set of device or instance enabled_ext_names
//   If the APIname has no parent extension, OR its parent extension name is IN one of the sets, return TRUE, else FALSE
static inline bool ApiParentExtensionEnabled(const std::string api_name, const DeviceExtensions *device_extension_info) {
    auto has_ext = api_extension_map.find(api_name);
    // Is this API part of an extension or feature group?
    if (has_ext != api_extension_map.end()) {

        // Was the extension for this API enabled in the CreateDevice call?
        for (const auto& ext : has_ext->second) {
            auto info = device_extension_info->get_info(ext.c_str());
            if (info.state) {
                return device_extension_info->*(info.state) == kEnabledByCreateinfo || device_extension_info->*(info.state) == kEnabledByInteraction;
            }
        }

        // Was the extension for this API enabled in the CreateInstance call?
        auto instance_extension_info = static_cast<const InstanceExtensions*>(device_extension_info);
        for (const auto& ext : has_ext->second) {
            auto inst_info = instance_extension_info->get_info(ext.c_str());
            if (inst_info.state) {
                return instance_extension_info->*(inst_info.state) == kEnabledByCreateinfo || device_extension_info->*(inst_info.state) == kEnabledByInteraction;
            }
        }
        return false;
    }
    return true;
}
'''
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
