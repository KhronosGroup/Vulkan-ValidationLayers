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

class EnumStringHelperOutputGenerator(BaseGenerator):
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
****************************************************************************/
''')
        out.append('// NOLINTBEGIN') # Wrap for clang-tidy to ignore

        out.append('''
#pragma once
#include <string>
#include <vulkan/vulkan.h>
''')

        # TODO - this should be moved into different generated util file
        out.append('\nstatic inline bool IsDuplicatePnext(VkStructureType input_value) {\n')
        out.append('    switch (input_value) {\n')

        for struct in [x for x in self.vk.structs.values() if x.allowDuplicate and x.sType is not None]:
            # The sType will always be first member of struct
            out.append(f'        case {struct.sType}:\n')
        out.append('            return true;\n')
        out.append('        default:\n')
        out.append('            return false;\n')
        out.append('    }\n')
        out.append('}\n')
        out.append('\n')

        # If there are no fields (empty enum) ignore
        for enum in [x for x in self.vk.enums.values() if len(x.fields) > 0]:
            groupType = enum.name if enum.bitWidth == 32 else 'uint64_t'
            out.extend([f'#ifdef {enum.protect}\n'] if enum.protect else [])
            out.append(f'static inline const char* string_{enum.name}({groupType} input_value) {{\n')
            out.append('    switch (input_value) {\n')
            for field in enum.fields:
                out.extend([f'#ifdef {field.protect}\n'] if field.protect else [])
                out.append(f'        case {field.name}:\n')
                out.append(f'            return "{field.name}";\n')
                out.extend([f'#endif //{field.protect}\n'] if field.protect else [])
            out.append('        default:\n')
            out.append(f'            return "Unhandled {enum.name}";\n')
            out.append('    }\n')
            out.append('}\n')
            out.extend([f'#endif //{enum.protect}\n'] if enum.protect else [])
        out.append('\n')

        # For bitmask, first create a string for FlagBits, then a Flags version that calls into it
        # If there are no flags (empty bitmask) ignore
        for bitmask in [x for x in self.vk.bitmasks.values() if len(x.flags) > 0]:
            groupType = bitmask.name if bitmask.bitWidth == 32 else 'uint64_t'
            out.extend([f'#ifdef {bitmask.protect}\n'] if bitmask.protect else [])
            out.append(f'static inline const char* string_{bitmask.name}({groupType} input_value) {{\n')
            out.append('    switch (input_value) {\n')
            for flag in [x for x in bitmask.flags if not x.multiBit]:
                out.extend([f'#ifdef {flag.protect}\n'] if flag.protect else [])
                out.append(f'        case {flag.name}:\n')
                out.append(f'            return "{flag.name}";\n')
                out.extend([f'#endif //{flag.protect}\n'] if flag.protect else [])
            out.append('        default:\n')
            out.append(f'            return "Unhandled {bitmask.name}";\n')
            out.append('    }\n')
            out.append('}\n')

            mulitBitChecks = ''
            for flag in [x for x in bitmask.flags if x.multiBit]:
                mulitBitChecks += f'    if (input_value == {flag.name}) {{ return "{flag.name}"; }}\n'
            intSuffix = 'U' if bitmask.bitWidth == 32 else 'ULL'

            out.append(f'''
static inline std::string string_{bitmask.flagName}({bitmask.flagName} input_value) {{
{mulitBitChecks}    std::string ret;
    int index = 0;
    while(input_value) {{
        if (input_value & 1) {{
            if( !ret.empty()) ret.append("|");
            ret.append(string_{bitmask.name}(static_cast<{groupType}>(1{intSuffix} << index)));
        }}
        ++index;
        input_value >>= 1;
    }}
    if (ret.empty()) ret.append("{bitmask.flagName}(0)");
    return ret;
}}\n''')
            out.extend([f'#endif //{bitmask.protect}\n'] if bitmask.protect else [])

        out.append('// NOLINTEND') # Wrap for clang-tidy to ignore
        self.write("".join(out))
