#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2023 The Khronos Group Inc.
# Copyright (c) 2015-2023 Valve Corporation
# Copyright (c) 2015-2023 LunarG, Inc.
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

import sys,os
from generator import *
from collections import namedtuple
from common_codegen import *

funcptr_source_preamble = '''
#include "lvt_function_pointers.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <string>
#include "containers/custom_containers.h"

#ifdef _WIN32
// Dynamic Loading:
typedef HMODULE dl_handle;
static dl_handle open_library(const char *lib_path) {
    // Try loading the library the original way first.
    dl_handle lib_handle = LoadLibrary(lib_path);
    if (lib_handle == NULL && GetLastError() == ERROR_MOD_NOT_FOUND) {
        // If that failed, then try loading it with broader search folders.
        lib_handle = LoadLibraryEx(lib_path, NULL, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR);
    }
    return lib_handle;
}
static char *open_library_error(const char *libPath) {
    static char errorMsg[164];
    (void)snprintf(errorMsg, 163, "Failed to open dynamic library \\\"%s\\\" with error %lu", libPath, GetLastError());
    return errorMsg;
}
static void *get_proc_address(dl_handle library, const char *name) {
    assert(library);
    assert(name);
    return (void *)GetProcAddress(library, name);
}
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__QNX__)

#include <dlfcn.h>

typedef void *dl_handle;
static inline dl_handle open_library(const char *libPath) {
    // When loading the library, we use RTLD_LAZY so that not all symbols have to be
    // resolved at this time (which improves performance). Note that if not all symbols
    // can be resolved, this could cause crashes later. Use the LD_BIND_NOW environment
    // variable to force all symbols to be resolved here.
    return dlopen(libPath, RTLD_LAZY | RTLD_LOCAL);
}
static inline const char *open_library_error(const char * /*libPath*/) { return dlerror(); }
static inline void *get_proc_address(dl_handle library, const char *name) {
    assert(library);
    assert(name);
    return dlsym(library, name);
}
#else
#error Dynamic library functions must be defined for this OS.
#endif


namespace vk {

'''

funcptr_header_preamble = '''
#include <vulkan/vulkan.h>

#ifdef _WIN32
/* Windows-specific common code: */
// WinBase.h defines CreateSemaphore and synchapi.h defines CreateEvent
//  undefine them to avoid conflicts with VkLayerDispatchTable struct members.
#ifdef CreateSemaphore
#undef CreateSemaphore
#endif
#ifdef CreateEvent
#undef CreateEvent
#endif
#endif

namespace vk {

'''

#
# LvtFileOutputGenerator - subclass of OutputGenerator.
# Generates files needed by the layer validation tests
class LvtFileOutputGenerator(OutputGenerator):
    """Generate LVT support files based on XML element attributes"""
    def __init__(self,
                 errFile = sys.stderr,
                 warnFile = sys.stderr,
                 diagFile = sys.stdout):
        OutputGenerator.__init__(self, errFile, warnFile, diagFile)
        # Internal state - accumulators for different inner block text
        self.coreInfo = []
        self.ExtensionInfo = namedtuple('ExtensionInfo', ['type', 'protection_macro', 'commands'])
        self.ExtensionCommand = namedtuple('ExtensionCommand', ['name', 'dispatch_param_type'])
        self.extensionInfo = dict() # extension name -> ExtensionInfo

    #
    # Called once at the beginning of each run
    def beginFile(self, genOpts):
        OutputGenerator.beginFile(self, genOpts)

        # Initialize members that require the tree
        self.handle_types = GetHandleTypes(self.registry.tree)
        self.lvt_file_type = genOpts.lvt_file_type

        if genOpts.lvt_file_type == 'function_pointer_header':
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
        write(copyright, file=self.outFile)
    #
    # Write completed source code to output file
    def endFile(self):
        dest_file = ''
        dest_file += self.OutputDestFile()
        # Remove blank lines at EOF
        if dest_file.endswith('\n'):
            dest_file = dest_file[:-1]
        write(dest_file, file=self.outFile);
        # Finish processing in superclass
        OutputGenerator.endFile(self)
    #
    # Processing at beginning of each feature or extension
    def beginFeature(self, interface, emit):
        OutputGenerator.beginFeature(self, interface, emit)
        self.featureName = interface.get('name')
        self.extensionType = interface.get('type')
        self.featureExtraProtect = GetFeatureProtect(interface)

    #
    # Process commands, adding to dispatch list
    def genCmd(self, cmdinfo, name, alias):
        OutputGenerator.genCmd(self, cmdinfo, name, alias)
        # Get first param type
        params = cmdinfo.elem.findall('param')
        dispatch_param_type = self.getTypeNameTuple(params[0])[0]
        self.AddCommandToDispatchList(name, dispatch_param_type)

    #
    # Determine if this API should be ignored or added to the funcptr list
    def AddCommandToDispatchList(self, name, dispatch_param_type):
        if 'VK_VERSION' in self.featureName:
            self.coreInfo.append((name, self.featureExtraProtect))
        else:
            if self.featureName not in self.extensionInfo:
                self.extensionInfo[self.featureName] = \
                    self.ExtensionInfo(type=self.extensionType, protection_macro=self.featureExtraProtect, commands=[])
            self.extensionInfo[self.featureName].commands.append(\
                self.ExtensionCommand(name=name, dispatch_param_type=dispatch_param_type))
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
    # Create the test function pointer source and return it as a string
    def GenerateFunctionPointerSource(self):
        table = funcptr_source_preamble

        for item in self.coreInfo:
            # Remove 'vk' from proto name
            base_name = item[0][2:]
            if item[1] is not None:
                table += f'#ifdef {item[1]}\n'
            table += f'PFN_{item[0]} {base_name};\n'
            if item[1] is not None:
                table += f'#endif // {item[1]}\n'

        table += '\n// Extension function pointers\n'
        for name in sorted(self.extensionInfo.keys()):
            table += f'// {name}\n'
            ext = self.extensionInfo[name]
            if ext.protection_macro is not None:
                table += f'#ifdef {ext.protection_macro}\n'
            for cmd in ext.commands:
                table += f'PFN_{cmd.name} {cmd.name[2:]};\n'
            if ext.protection_macro is not None:
                table += f'#endif // {ext.protection_macro}\n'

        table += '''

void InitCore(const char *api_name) {

#if defined(WIN32)
    std::string filename = std::string(api_name) + "-1.dll";
    auto lib_handle = open_library(filename.c_str());
#elif(__APPLE__)
    std::string filename = std::string("lib") + api_name + ".dylib";
    auto lib_handle = open_library(filename.c_str());
#else
    std::string filename = std::string("lib") + api_name + ".so";
    auto lib_handle = open_library(filename.c_str());
    if (!lib_handle) {
        filename = std::string("lib") + api_name + ".so.1";
        lib_handle = open_library(filename.c_str());
    }
#endif

    if (lib_handle == nullptr) {
        printf("%s\\n", open_library_error(filename.c_str()));
        exit(1);
    }

'''
        # Core functions
        for item in self.coreInfo:
            # Remove 'vk' from proto name
            base_name = item[0][2:]

            if item[1] is not None:
                table += f'#ifdef {item[1]}\n'
            table += f'    {base_name} = reinterpret_cast<PFN_{item[0]}>(get_proc_address(lib_handle, "{item[0]}"));\n'
            if item[1] is not None:
                table += f'#endif // {item[1]}\n'
        table += '}\n\n'

        # Instance extension functions
        table += 'void InitInstanceExtension(VkInstance instance, const char* extension_name) {\n'
        table += '    assert(instance);\n'
        table += '    static const vvl::unordered_map<std::string, std::function<void(VkInstance)>> initializers = {\n'
        for name in sorted(self.extensionInfo.keys()):
            ext = self.extensionInfo[name]
            if ext.type != 'instance':
                continue
            if ext.protection_macro is not None:
                table += f'#ifdef {ext.protection_macro}\n'
            table += ' ' * 8 + '{\n'
            table += ' ' * 12 +  f'"{name}", [](VkInstance instance) {{\n'
            for cmd in ext.commands:
                table += ' ' * 16 + f'{cmd.name[2:]} = reinterpret_cast<PFN_{cmd.name}>(GetInstanceProcAddr(instance, "{cmd.name}"));\n'
            table += ' ' * 12 +  '}\n'
            table += ' ' * 8 +  '},\n'
            if ext.protection_macro is not None:
                table += f'#endif // {ext.protection_macro}\n'
        table += '    };\n\n'
        table += '    if (auto it = initializers.find(extension_name); it != initializers.end())\n'
        table += '        (it->second)(instance);\n'
        table += '}\n\n'

        # Device extension functions
        table += 'void InitDeviceExtension(VkInstance instance, VkDevice device, const char* extension_name) {\n'
        table += '    static const vvl::unordered_map<std::string, std::function<void(VkInstance, VkDevice)>> initializers = {\n'
        for name in sorted(self.extensionInfo.keys()):
            ext = self.extensionInfo[name]
            if ext.type != 'device':
                continue
            if ext.protection_macro is not None:
                table += f'#ifdef {ext.protection_macro}\n'
            table += ' ' * 8 + '{\n'
            # Select proper signature to prevent unused parameter compiler warning
            has_instance_level = any(cmd.dispatch_param_type == 'VkPhysicalDevice' for cmd in ext.commands)
            has_device_level = any(cmd.dispatch_param_type != 'VkPhysicalDevice' for cmd in ext.commands)
            if has_instance_level and has_device_level:
                table += ' ' * 12 +  f'"{name}", [](VkInstance instance, VkDevice device) {{\n'
            elif has_instance_level:
                table += ' ' * 12 +  f'"{name}", [](VkInstance instance, VkDevice) {{\n'
            else:
                table += ' ' * 12 +  f'"{name}", [](VkInstance, VkDevice device) {{\n'
            for cmd in ext.commands:
                # NOTE: On Android GDPA does not work for physical-device-level functionality but GIPA works.
                # It's stated in the spec that GIPA _can_ be used to get physical-device-level functionality.
                # Use GIPA to get physical-device-level functionality on all platforms.
                physical_device_level = (cmd.dispatch_param_type == 'VkPhysicalDevice')
                if physical_device_level:
                    table += ' ' * 16 + f'{cmd.name[2:]} = reinterpret_cast<PFN_{cmd.name}>(GetInstanceProcAddr(instance, "{cmd.name}"));\n'
                else:
                    table += ' ' * 16 + f'{cmd.name[2:]} = reinterpret_cast<PFN_{cmd.name}>(GetDeviceProcAddr(device, "{cmd.name}"));\n'
            table += ' ' * 12 + '}\n'
            table += ' ' * 8 + '},\n'
            if ext.protection_macro is not None:
                table += f'#endif // {ext.protection_macro}\n'
        table += '    };\n\n'
        table += '    if (auto it = initializers.find(extension_name); it != initializers.end())\n'
        table += '        (it->second)(instance, device);\n'
        table += '}\n\n'

        # Zero all extension pointers
        table += 'void ResetAllExtensions() {\n'
        for name in sorted(self.extensionInfo.keys()):
            table += f'    // {name}\n'
            ext = self.extensionInfo[name]
            if ext.protection_macro is not None:
                table += f'#ifdef {ext.protection_macro}\n'
            for cmd in ext.commands:
                table += f'    {cmd.name[2:]} = nullptr;\n'
            if ext.protection_macro is not None:
                table += f'#endif // {ext.protection_macro}\n'
        table += '}\n\n'

        table += '} // namespace vk'
        return table
    #
    # Create the test function pointer source and return it as a string
    def GenerateFunctionPointerHeader(self):
        table = funcptr_header_preamble

        for item in self.coreInfo:
            # Remove 'vk' from proto name
            base_name = item[0][2:]
            if item[1] is not None:
                table += f'#ifdef {item[1]}\n'
            table += f'extern PFN_{item[0]} {base_name};\n'
            if item[1] is not None:
                table += f'#endif // {item[1]}\n'

        table += '\n// Extension function pointers\n'
        for name in sorted(self.extensionInfo.keys()):
            table += f'// {name}\n'
            ext = self.extensionInfo[name]
            if ext.protection_macro is not None:
                table += f'#ifdef {ext.protection_macro}\n'
            for cmd in ext.commands:
                table += f'extern PFN_{cmd.name} {cmd.name[2:]};\n'
            if ext.protection_macro is not None:
                table += f'#endif // {ext.protection_macro}\n'

        table += '\n'
        table += 'void InitCore(const char *api_name);\n'
        table += 'void InitInstanceExtension(VkInstance instance, const char* extension_name);\n'
        table += 'void InitDeviceExtension(VkInstance instance, VkDevice device, const char* extension_name);\n'
        table += 'void ResetAllExtensions();\n'
        table += '\n'
        table += '} // namespace vk'
        return table

    # Create a helper file and return it as a string
    def OutputDestFile(self):
        if self.lvt_file_type == 'function_pointer_header':
            return self.GenerateFunctionPointerHeader()
        elif self.lvt_file_type == 'function_pointer_source':
            return self.GenerateFunctionPointerSource()
        else:
            return f'Bad LVT File Generator Option {self.lvt_file_type}'
