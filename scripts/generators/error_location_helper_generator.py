#!/usr/bin/python3 -i
#
# Copyright (c) 2023 The Khronos Group Inc.
# Copyright (c) 2023 Valve Corporation
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

class ErrorLocationHelperOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

        self.fields = set()
        self.pointer_fields = set()

    def generate(self):
        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2023 The Khronos Group Inc.
            * Copyright (c) 2023 Valve Corporation
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
        self.write('// NOLINTBEGIN') # Wrap for clang-tidy to ignore

        # Build set of all field names found in all structs and commands
        for command in [x for x in self.vk.commands.values() if not x.alias]:
            for param in command.params:
                self.fields.add(param.name)
                if param.pointer:
                    self.pointer_fields.add(param.name)
        for struct in self.vk.structs.values():
            for member in struct.members:
                self.fields.add(member.name)
                if member.pointer:
                    self.pointer_fields.add(member.name)

        # Pointers in spec start with 'p' except a few cases when dealing with external items (etc WSI).
        # These are names that are also not pointers and removing them in effort to keep the code
        # simpler and just have a few cases where we use a 'dot' instead of an 'arrow' for the error messages.
        self.pointer_fields.remove('buffer') # VkImportAndroidHardwareBufferInfoANDROID
        self.pointer_fields.remove('display') # VkWaylandSurfaceCreateInfoKHR
        self.pointer_fields.remove('window') # VkAndroidSurfaceCreateInfoKHR (and few others)
        self.pointer_fields.remove('surface') # VkDirectFBSurfaceCreateInfoEXT

        # Sort alphabetically
        self.fields = sorted(self.fields)
        self.pointer_fields = sorted(self.pointer_fields)

        if self.filename == 'error_location_helper.h':
            self.generateHeader()
        elif self.filename == 'error_location_helper.cpp':
            self.generateSource()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateHeader(self):
        out = []
        out.append('''
            #pragma once
            #include <string_view>
            #include <vulkan/vulkan.h>

            namespace vvl {
            enum class Func {
                Empty = 0,
            ''')
        # Want alpha-sort for ease of look at list while debugging
        for index, command in enumerate(sorted(self.vk.commands.values()), start=1):
            out.append(f'    {command.name} = {index},\n')
        out.append('};\n')

        out.append('\n')
        out.append('enum class Struct {\n')
        out.append('    Empty = 0,\n')
        # Want alpha-sort for ease of look at list while debugging
        for struct in sorted(self.vk.structs.values()):
            out.append(f'    {struct.name},\n')
        out.append('};\n')

        out.append('\n')
        out.append('enum class Field {\n')
        out.append('    Empty = 0,\n')
        # Already alpha-sorted
        for field in self.fields:
            out.append(f'    {field},\n')
        out.append('};\n')

        out.append('''
            const char* String(Func func);
            const char* String(Struct structure);
            const char* String(Field field);

            bool IsFieldPointer(Field field);
            }  // namespace vvl
            ''')
        self.write("".join(out))

    def generateSource(self):
        out = []
        out.append('''
            #include "error_location_helper.h"
            #include "containers/custom_containers.h"
            #include <assert.h>
            ''')

        out.append('''
// clang-format off
namespace vvl {
''')
        out.append('''
const char* String(Func func) {
    static const std::string_view table[] = {
    {"INVALID_EMPTY", 15}, // Func::Empty
''')
        # Need to be alpha-sort also to match array indexing
        for command in sorted(self.vk.commands.values()):
            out.append(f'    {{"{command.name}", {len(command.name) + 1}}},\n')
        out.append('''    };
    return table[(int)func].data();
}

const char* String(Struct structure) {
    static const std::string_view table[] = {
    {"INVALID_EMPTY", 15}, // Struct::Empty
''')
        # Need to be alpha-sort also to match array indexing
        for struct in sorted(self.vk.structs.values()):
            out.append(f'    {{"{struct.name}", {len(struct.name) + 1}}},\n')
        out.append('''    };
    return table[(int)structure].data();
}

const char* String(Field field) {
    static const std::string_view table[] = {
    {"INVALID_EMPTY", 15}, // Field::Empty
''')
        for field in self.fields:
            out.append(f'    {{"{field}", {len(field) + 1}}},\n')
        out.append('''    };
    return table[(int)field].data();
}

bool IsFieldPointer(Field field) {
    switch (field) {
''')
        for field in self.pointer_fields:
            out.append(f'    case Field::{field}:\n')
        out.append('''        return true;
    default:
        return false;
    }
}
}  // namespace vvl
// clang-format on
''')
        self.write("".join(out))
