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
from generators.generator_utils import (incIndent, decIndent, addIndent)
from generators.vulkan_object import (Member)
from generators.base_generator import BaseGenerator

class PnextChainExtractionGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

        # List of Vulkan structures that a pnext chain extraction function will be generated for
        self.target_structs = [
            'VkPhysicalDeviceImageFormatInfo2'
            ]
    
    def generate(self):
        out = []
        out.append(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See {os.path.basename(__file__)} for modifications

/***************************************************************************
*
* Copyright (c) 2023 The Khronos Group Inc.
* Copyright (c) 2023 Valve Corporation
* Copyright (c) 2023 LunarG, Inc.
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
****************************************************************************/\n\n''')
        out.append('// NOLINTBEGIN\n\n') # Wrap for clang-tidy to ignore

        if self.filename == 'pnext_chain_extraction.h':
            out.append(self.generateHeader())
        elif self.filename == 'pnext_chain_extraction.cpp':
            out.append(self.generateSource())
        else:
            out.append(f'\nFile name {self.filename} has no code to generate\n')

        out.append('\n// NOLINTEND') # Wrap for clang-tidy to ignore
        self.write("".join(out))

    def generateHeader(self):
        out = []
        out.append(f'''#pragma once

#include <tuple>

#include "vulkan/vulkan.h"

namespace vvl {{

// Add element to end of pNext chain: chain_end must be a pointer to the last struct of the chain
void* PnextChainAdd(void *chain_end, void *new_struct);

// Remove last element from pNext chain 
void PnextChainRemoveLast(void *chain);

// Utility to make a selective copy of a pNext chain.
// Structs listed in the returned tuple are the one extending some Vulkan structs, like VkPhysicalDeviceImageFormatInfo2.
// Copied structs are the one mentioned in the returned tuple type and found in `in_pnext_chain`.
// If the returned tuple is stack allocated, each struct is NOT a deep copy of the corresponding struct in in_pnext_chain,
// so be mindful of pointers copies.
// Beginning of pNext chain is stored in `std::get<0>(returned_tuple)` as a `void *`
template <typename T>
T ExtractPnextChain(const void *in_pnext_chain) {{}}

''')
        # Declare functions
        for struct_name in self.target_structs:
            struct = self.vk.structs[struct_name]
            out.append(f'\nusing PnextChain{struct_name} = std::tuple<void *,\n\t')
            out.append(',\n\t'.join(struct.extendedBy))
            out.append('>;\n')
            out.append('template <>\n')
            out.append(f'PnextChain{struct_name} ExtractPnextChain(const void *in_pnext_chain);\n\n')

        out.append('}\n')
        return "".join(out)

    def generateSource(self):
        out = []
        out.append(f'''
#include "pnext_chain_extraction.h"

#include "vk_typemap_helper.h"

#include <cassert>

namespace vvl {{
       
void* PnextChainAdd(void *chain_end, void *new_struct) {{
    assert(new_struct);
    if (!chain_end) {{
    	return new_struct;
    }}
    auto *vk_base_struct = reinterpret_cast<VkBaseOutStructure*>(chain_end);
    assert(!vk_base_struct->pNext);
    vk_base_struct->pNext = reinterpret_cast<VkBaseOutStructure*>(new_struct);
    return new_struct;
}};

void PnextChainRemoveLast(void *chain) {{
    if (!chain) {{
        return;
    }}
    auto *current = reinterpret_cast<VkBaseOutStructure *>(chain);
    auto *prev = current;
    while (current) {{
        prev = current;
        current = reinterpret_cast<VkBaseOutStructure *>(current->pNext);
    }}
    prev->pNext = nullptr;
}}

''')

        # Define functions
        for struct_name in self.target_structs:
            struct = self.vk.structs[struct_name]
            out.append('\ntemplate <>\n')
            out.append(f'PnextChain{struct_name} ExtractPnextChain(const void *in_pnext_chain) {{\n\n')
            
            # Declare object to be returned
            out.append('\tstd::tuple<void *,\n\t')
            out.append(',\n\t'.join(struct.extendedBy))
            out.append('> out_structs;\n')

            out.append(f'''
    void * &chain_end = std::get<0>(out_structs);\n''')

            # Add extraction logic for each struct extending target struct
            for extending_struct in struct.extendedBy:
                out.append(f'''
    if (auto *chain_struct = LvlFindInChain<{extending_struct}>(in_pnext_chain)) {{
    	auto &out_chain_struct = std::get<{extending_struct}>(out_structs);
    	out_chain_struct = *chain_struct;
        out_chain_struct.pNext = nullptr;
    	chain_end = PnextChainAdd(chain_end, &out_chain_struct);
    }}\n''')
            out.append('\n\treturn out_structs;\n}\n')

        out.append('\n}\n')
        return "".join(out)
