#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2023 The Khronos Group Inc.
# Copyright (c) 2015-2023 Valve Corporation
# Copyright (c) 2015-2023 LunarG, Inc.
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

import os,re,sys
import xml.etree.ElementTree as etree
from generator import *
from collections import namedtuple
from common_codegen import *

funcptr_source_preamble = '''
#include "lvt_function_pointers.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>

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
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__)

#include <dlfcn.h>

typedef void *dl_handle;
static inline dl_handle open_library(const char *libPath) {
    // When loading the library, we use RTLD_LAZY so that not all symbols have to be
    // resolved at this time (which improves performance). Note that if not all symbols
    // can be resolved, this could cause crashes later. Use the LD_BIND_NOW environment
    // variable to force all symbols to be resolved here.
    return dlopen(libPath, RTLD_LAZY | RTLD_LOCAL);
}
static inline const char *open_library_error(const char *libPath) { return dlerror(); }
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
# LvtFileOutputGeneratorOptions - subclass of GeneratorOptions.
class LvtFileOutputGeneratorOptions(GeneratorOptions):
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
                 expandEnumerants = False,
                 lvt_file_type = ''):
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
        self.lvt_file_type   = lvt_file_type
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
        self.dispatch_list = []               # List of entries for dispatch list
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
        file_comment += '// See lvt_file_generator.py for modifications\n'
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
        self.featureExtraProtect = GetFeatureProtect(interface)

    #
    # Process commands, adding to dispatch list
    def genCmd(self, cmdinfo, name, alias):
        OutputGenerator.genCmd(self, cmdinfo, name, alias)
        # Get first param type
        params = cmdinfo.elem.findall('param')
        info = self.getTypeNameTuple(params[0])
        self.AddCommandToDispatchList(name, info[0], self.featureExtraProtect, cmdinfo)

    #
    # Determine if this API should be ignored or added to the funcptr list
    def AddCommandToDispatchList(self, name, handle_type, protect, cmdinfo):
        WSI_mandatory_extensions = [
            'VK_KHR_win32_surface',
            'VK_KHR_xcb_surface',
            'VK_KHR_xlib_surface',
            'VK_KHR_wayland_surface',
            'VK_MVK_macos_surface',
            'VK_KHR_surface',
            'VK_KHR_swapchain',
            'VK_KHR_display',
            'VK_KHR_android_surface',
            ]
        if 'VK_VERSION' in self.featureName or self.featureName in WSI_mandatory_extensions:
            self.dispatch_list.append((name, self.featureExtraProtect))
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
        entries = []
        table = funcptr_source_preamble
        entries = self.dispatch_list

        for item in entries:
            # Remove 'vk' from proto name
            base_name = item[0][2:]
            if item[1] is not None:
                table += '#ifdef %s\n' % item[1]
            table += 'PFN_%s %s;\n' % (item[0], base_name)
            if item[1] is not None:
                table += '#endif // %s\n' % item[1]

        table += '''

void InitDispatchTable() {

#if(WIN32)
    const char filename[] = "vulkan-1.dll";
    auto lib_handle = open_library(filename);
#elif(__APPLE__)
    const char filename[] = "libvulkan.dylib";
    auto lib_handle = open_library(filename);
#else
    const char *filename = "libvulkan.so";
    auto lib_handle = open_library(filename);
    if (!lib_handle) {
        filename = "libvulkan.so.1";
        lib_handle = open_library(filename);
    }
#endif

    if (lib_handle == nullptr) {
        printf("%s\\n", open_library_error(filename));
        exit(1);
    }

'''

        for item in entries:
            # Remove 'vk' from proto name
            base_name = item[0][2:]

            if item[1] is not None:
                table += '#ifdef %s\n' % item[1]
            table += '    %s = reinterpret_cast<PFN_%s>(get_proc_address(lib_handle, "%s"));\n' % (base_name, item[0], item[0])
            if item[1] is not None:
                table += '#endif // %s\n' % item[1]
        table += '}\n\n'
        table += '} // namespace vk'
        return table
    #
    # Create the test function pointer source and return it as a string
    def GenerateFunctionPointerHeader(self):
        entries = []
        table = funcptr_header_preamble
        entries = self.dispatch_list

        for item in entries:
            # Remove 'vk' from proto name
            base_name = item[0][2:]
            if item[1] is not None:
                table += '#ifdef %s\n' % item[1]
            table += 'extern PFN_%s %s;\n' % (item[0], base_name)
            if item[1] is not None:
                table += '#endif // %s\n' % item[1]
        table += '\n'
        table += 'void InitDispatchTable();\n\n'
        table += '} // namespace vk'
        return table

    # Create a helper file and return it as a string
    def OutputDestFile(self):
        if self.lvt_file_type == 'function_pointer_header':
            return self.GenerateFunctionPointerHeader()
        elif self.lvt_file_type == 'function_pointer_source':
            return self.GenerateFunctionPointerSource()
        else:
            return 'Bad LVT File Generator Option %s' % self.lvt_file_type
