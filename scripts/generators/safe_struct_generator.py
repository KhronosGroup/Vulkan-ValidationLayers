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

import sys
import os
from common_codegen import *
from generators.generator_utils import *
from generators.base_generator import BaseGenerator

# Determine if a structure needs a safe_struct helper function
# That is, it has an sType or one of its members is a pointer
def needSafeStruct(struct: Struct) -> bool:
    if 'VkBase' in struct.name:
        return False #  Ingore structs like VkBaseOutStructure
    if struct.sType is not None:
        return True
    for member in struct.members:
        if member.pointer:
            return True
    return False

class SafeStructOutputGenerator(BaseGenerator):
    def __init__(self,
                 errFile = sys.stderr,
                 warnFile = sys.stderr,
                 diagFile = sys.stdout):
        BaseGenerator.__init__(self, errFile, warnFile, diagFile)

        self.custom_construct_params = {
            # safe_VkGraphicsPipelineCreateInfo needs to know if subpass has color and\or depth\stencil attachments to use its pointers
            'VkGraphicsPipelineCreateInfo' :
                ', const bool uses_color_attachment, const bool uses_depthstencil_attachment',
            # safe_VkPipelineViewportStateCreateInfo needs to know if viewport and scissor is dynamic to use its pointers
            'VkPipelineViewportStateCreateInfo' :
                ', const bool is_dynamic_viewports, const bool is_dynamic_scissors',
            # safe_VkAccelerationStructureBuildGeometryInfoKHR needs to know if we're doing a host or device build
            'VkAccelerationStructureBuildGeometryInfoKHR' :
                ', const bool is_host, const VkAccelerationStructureBuildRangeInfoKHR *build_range_infos',
            # safe_VkAccelerationStructureGeometryKHR needs to know if we're doing a host or device build
            'VkAccelerationStructureGeometryKHR' :
                ', const bool is_host, const VkAccelerationStructureBuildRangeInfoKHR *build_range_info',
            # safe_VkDescriptorDataEXT needs to know what field of union is intialized
            'VkDescriptorDataEXT' :
                ', const VkDescriptorType type',
            'VkPipelineRenderingCreateInfo' : ''
        }


    def containsObjectHandle(self, member: Member) -> bool:
        if member.type in self.vk.handles:
            return True
        if member.type in self.vk.structs:
            for subMember in self.vk.structs[member.type].members:
                if self.containsObjectHandle(subMember):
                    return True
        return False

    def generate(self):
        copyright = f'''{fileIsGeneratedWarning(os.path.basename(__file__))}
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
****************************************************************************/\n'''
        self.write(copyright)
        self.write('// NOLINTBEGIN') # Wrap for clang-tidy to ignore

        if self.filename == 'vk_safe_struct.h':
            self.generateHeader()
        elif self.filename == 'vk_safe_struct_utils.cpp':
            self.generateUtil()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateHeader(self):
        out = []
        out.append('''
#pragma once
#include <vulkan/vulkan.h>
#include <cstdlib>
#include <algorithm>
#include <functional>

// State that elements in a pNext chain may need to be aware of
struct PNextCopyState {
    // Custom initialization function. Returns true if the structure passed to init was initialized, false otherwise
    std::function<bool(VkBaseOutStructure* /* safe_sruct */, const VkBaseOutStructure* /* in_struct */)> init;
};

void *SafePnextCopy(const void *pNext, PNextCopyState* copy_state = {});
void FreePnextChain(const void *pNext);
char *SafeStringCopy(const char *in_string);
\n''')

        for struct in [x for x in self.vk.structs.values() if needSafeStruct(x)]:
            out.extend([f'#ifdef {struct.protect}\n'] if struct.protect else [])
            out.append(f'{"union" if struct.union else "struct"} safe_{struct.name} {{\n')
            # Can only initialize first member of an Union
            canInitialize = True
            for member in struct.members:
                if member.type in self.vk.structs:
                    if needSafeStruct(self.vk.structs[member.type]):
                        if member.pointer:
                            pointer = '*' * member.cDeclaration.count('*')
                            brackets = '' if struct.union else '{}'
                            out.append(f'    safe_{member.type}{pointer} {member.name}{brackets};\n')
                        else:
                            out.append(f'    safe_{member.type} {member.name};\n')
                        continue

                initialize = '{}' if member.pointer and canInitialize else ''
                if member.pointer and canInitialize:
                    canInitialize = not struct.union # Prevents union from initializing agian

                if (getFormatedLength(member.length) is not None) and self.containsObjectHandle(member):
                    out.append(f'    {member.type}* {member.name}{initialize};\n')
                else:
                    out.append(f'{member.cDeclaration}{initialize};\n')

            if (struct.name == 'VkDescriptorDataEXT'):
                out.append('    char type_at_end[sizeof(VkDescriptorDataEXT)+sizeof(VkDescriptorGetInfoEXT::type)];')

            constructParam = self.custom_construct_params.get(struct.name, '')
            out.append(f'''
    safe_{struct.name}(const {struct.name}* in_struct{constructParam}, PNextCopyState* copy_state = {{}});
    safe_{struct.name}(const safe_{struct.name}& copy_src);
    safe_{struct.name}& operator=(const safe_{struct.name}& copy_src);
    safe_{struct.name}();
    ~safe_{struct.name}();
    void initialize(const {struct.name}* in_struct{constructParam}, PNextCopyState* copy_state = {{}});
    void initialize(const safe_{struct.name}* copy_src, PNextCopyState* copy_state = {{}});
    {struct.name} *ptr() {{ return reinterpret_cast<{struct.name} *>(this); }}
    {struct.name} const *ptr() const {{ return reinterpret_cast<{struct.name} const *>(this); }}
''')

            if struct.name == 'VkShaderModuleCreateInfo':
                out.append('''
    // Primarily intended for use by GPUAV when replacing shader module code with instrumented code
    template<typename Container>
    void SetCode(const Container &code) {
        delete[] pCode;
        codeSize = static_cast<uint32_t>(code.size() * sizeof(uint32_t));
        pCode = new uint32_t[code.size()];
        std::copy(&code.front(), &code.back() + 1, const_cast<uint32_t*>(pCode));
    }
''')
            out.append('};\n')
            out.extend([f'#endif // {struct.protect}\n'] if struct.protect else [])
        self.write("".join(out))

    def generateUtil(self):
        out = []
        out.append('''
#include "vk_safe_struct.h"
#include "utils/vk_layer_utils.h"

#include <vector>

extern std::vector<std::pair<uint32_t, uint32_t>> custom_stype_info;

char *SafeStringCopy(const char *in_string) {
    if (nullptr == in_string) return nullptr;
    char* dest = new char[std::strlen(in_string) + 1];
    return std::strcpy(dest, in_string);
}

void *SafePnextCopy(const void *pNext, PNextCopyState* copy_state) {
    if (!pNext) return nullptr;

    void *safe_pNext{};
    const VkBaseOutStructure *header = reinterpret_cast<const VkBaseOutStructure *>(pNext);

    switch (header->sType) {
        // Add special-case code to copy beloved secret loader structs
        // Special-case Loader Instance Struct passed to/from layer in pNext chain
        case VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO: {
            VkLayerInstanceCreateInfo *struct_copy = new VkLayerInstanceCreateInfo;
            // TODO: Uses original VkLayerInstanceLink* chain, which should be okay for our uses
            memcpy(struct_copy, pNext, sizeof(VkLayerInstanceCreateInfo));
            struct_copy->pNext = SafePnextCopy(header->pNext, copy_state);
            safe_pNext = struct_copy;
            break;
        }
        // Special-case Loader Device Struct passed to/from layer in pNext chain
        case VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO: {
            VkLayerDeviceCreateInfo *struct_copy = new VkLayerDeviceCreateInfo;
            // TODO: Uses original VkLayerDeviceLink*, which should be okay for our uses
            memcpy(struct_copy, pNext, sizeof(VkLayerDeviceCreateInfo));
            struct_copy->pNext = SafePnextCopy(header->pNext, copy_state);
            safe_pNext = struct_copy;
            break;
        }''')

        for struct in [x for x in self.vk.structs.values() if x.structExtends is not None]:
            out.extend([f'\n#ifdef {struct.protect}'] if struct.protect else [])
            out.append(f'''
        case {struct.sType}:
            safe_pNext = new safe_{struct.name}(reinterpret_cast<const {struct.name} *>(pNext), copy_state);
            break;''')
            out.extend([f'\n#endif // {struct.protect}'] if struct.protect else [])

        out.append('''
        default: // Encountered an unknown sType -- skip (do not copy) this entry in the chain
            // If sType is in custom list, construct blind copy
            for (auto item : custom_stype_info) {
                if (item.first == header->sType) {
                    safe_pNext = malloc(item.second);
                    memcpy(safe_pNext, header, item.second);
                    // Deep copy the rest of the pNext chain
                    VkBaseOutStructure *custom_struct = reinterpret_cast<VkBaseOutStructure *>(safe_pNext);
                    if (custom_struct->pNext) {
                        custom_struct->pNext = reinterpret_cast<VkBaseOutStructure *>(SafePnextCopy(custom_struct->pNext, copy_state));
                    }
                }
            }
            if (!safe_pNext) {
                safe_pNext = SafePnextCopy(header->pNext, copy_state);
            }
            break;
    }

    return safe_pNext;
}

void FreePnextChain(const void *pNext) {
    if (!pNext) return;

    auto header = reinterpret_cast<const VkBaseOutStructure *>(pNext);

    switch (header->sType) {
        // Special-case Loader Instance Struct passed to/from layer in pNext chain
        case VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO:
            FreePnextChain(header->pNext);
            delete reinterpret_cast<const VkLayerInstanceCreateInfo *>(pNext);
            break;
        // Special-case Loader Device Struct passed to/from layer in pNext chain
        case VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO:
            FreePnextChain(header->pNext);
            delete reinterpret_cast<const VkLayerDeviceCreateInfo *>(pNext);
            break;
''')

        for struct in [x for x in self.vk.structs.values() if x.structExtends is not None]:
            out.extend([f'\n#ifdef {struct.protect}'] if struct.protect else [])
            out.append(f'''
        case {struct.sType}:
            delete reinterpret_cast<const safe_{struct.name} *>(header);
            break;''')
            out.extend([f'\n#endif // {struct.protect}'] if struct.protect else [])

        out.append('''
        default: // Encountered an unknown sType
            // If sType is in custom list, free custom struct memory and clean up
            for (auto item : custom_stype_info) {
                if (item.first == header->sType) {
                    if (header->pNext) {
                        FreePnextChain(header->pNext);
                    }
                    free(const_cast<void *>(pNext));
                    pNext = nullptr;
                    break;
                }
            }
            if (pNext) {
                FreePnextChain(header->pNext);
            }
            break;
    }
}''')
        self.write("".join(out))

    def generateSource(self):
        out = []
        self.write("".join(out))