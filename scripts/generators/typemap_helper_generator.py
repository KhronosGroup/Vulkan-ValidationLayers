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
from generators.base_generator import BaseGenerator

class TypemapHelperOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

    def generate(self):
        out = []
        out.append(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
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
        out.append('// NOLINTBEGIN') # Wrap for clang-tidy to ignore
        out.append('''
#pragma once
#include <vulkan/vulkan.h>

// These empty generic templates are specialized for each type with sType
// members and for each sType -- providing a two way map between structure
// types and sTypes

template <VkStructureType id> struct LvlSTypeMap {};
template <typename T> struct LvlTypeMap {};\n''')

        for struct in [x for x in self.vk.structs.values() if x.sType]:
            out.extend([f'#ifdef {struct.protect}'] if struct.protect else [])
            out.append(f'''
// Map type {struct.name} to id {struct.sType}
template <> struct LvlTypeMap<{struct.name}> {{
    static const VkStructureType kSType = {struct.sType};
}};

template <> struct LvlSTypeMap<{struct.sType}> {{
    typedef {struct.name} Type;
}};\n''')
            out.extend([f'#endif // {struct.protect}\n'] if struct.protect else [])

        out.append('''
// Find an entry of the given type in the const pNext chain
template <typename T> const T *LvlFindInChain(const void *next) {
    const VkBaseOutStructure *current = reinterpret_cast<const VkBaseOutStructure *>(next);
    const T *found = nullptr;
    while (current) {
        if (LvlTypeMap<T>::kSType == current->sType) {
            found = reinterpret_cast<const T*>(current);
            current = nullptr;
        } else {
            current = current->pNext;
        }
    }
    return found;
}

// Find an entry of the given type in the pNext chain
template <typename T> T *LvlFindModInChain(void *next) {
    VkBaseOutStructure *current = reinterpret_cast<VkBaseOutStructure *>(next);
    T *found = nullptr;
    while (current) {
        if (LvlTypeMap<T>::kSType == current->sType) {
            found = reinterpret_cast<T*>(current);
            current = nullptr;
        } else {
            current = current->pNext;
        }
    }
    return found;
}

// Init the header of an sType struct with pNext and optional fields
template <typename T, typename... StructFields>
T LvlInitStruct(void *p_next, StructFields... fields) {
    T out = {LvlTypeMap<T>::kSType, p_next, fields...};
    return out;
}

// Init the header of an sType struct
template <typename T>
T LvlInitStruct(void *p_next = nullptr) {
    T out = {};
    out.sType = LvlTypeMap<T>::kSType;
    out.pNext = p_next;
    return out;
}\n''')

        out.append('// NOLINTEND') # Wrap for clang-tidy to ignore
        self.write("".join(out))
