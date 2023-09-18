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

import os
from generators.base_generator import BaseGenerator

class ValidEnumValuesOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

    def generate(self):
        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2015-2023 The Khronos Group Inc.
            * Copyright (c) 2015-2023 Valve Corporation
            * Copyright (c) 2015-2023 LunarG, Inc.
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
        self.write('// NOLINTBEGIN') # Wrap for clang-tidy to ignore

        if self.filename == 'valid_enum_values.h':
            self.generateHeader()
        elif self.filename == 'valid_enum_values.cpp':
            self.generateSource()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateHeader(self):
        out = []
        out.append("// clang-format off\n")
        for enum in [x for x in self.vk.enums.values() if x.name != 'VkStructureType' and not x.returnedOnly]:
            out.extend([f'#ifdef {enum.protect}\n'] if enum.protect else [])
            out.append(f'template<> std::vector<{enum.name}> ValidationObject::ValidParamValues() const;\n')
            out.extend([f'#endif //{enum.protect}\n'] if enum.protect else [])
        out.append("// clang-format on\n")

        self.write("".join(out))

    def generateSource(self):
        out = []
        out.append('''
            #include "chassis.h"
            #include "utils/hash_vk_types.h"

            // TODO (ncesario) This is not ideal as we compute the enabled extensions every time this function is called.
            //      Ideally "values" would be something like a static variable that is built once and this function returns
            //      a span of the container. This does not work for applications which create and destroy many instances and
            //      devices over the lifespan of the project (e.g., VLT).

            ''')
        out.append("// clang-format off\n")
        for enum in [x for x in self.vk.enums.values() if x.name != 'VkStructureType' and not x.returnedOnly]:
            out.extend([f'#ifdef {enum.protect}\n'] if enum.protect else [])
            out.append(f'template<>\nstd::vector<{enum.name}> ValidationObject::ValidParamValues() const {{\n')

            # If the field has same/subset extensions as enum, we count it as "core" for the struct
            coreEnums = [x.name for x in enum.fields if not x.extensions or (x.extensions and all(e in enum.extensions for e in x.extensions))]
            out.extend([f'    constexpr std::array Core{enum.name}Enums = {{{", ".join(coreEnums)}}};\n'] if coreEnums else [])

            out.append(f'    static const vvl::unordered_map<const ExtEnabled DeviceExtensions::*, std::vector<{enum.name}>> Extended{enum.name}Enums = {{\n')

            for extension in [x for x in enum.fieldExtensions if x not in enum.extensions]:
                out.append(f'        {{ &DeviceExtensions::{extension.name.lower()}, {{ {", ".join([x.name for x in extension.enumFields[enum.name]])} }} }},\n')
            out.append('    };')

            startValue = f'values(Core{enum.name}Enums.cbegin(), Core{enum.name}Enums.cend())' if coreEnums else 'values'
            out.append(f'''
    std::vector<{enum.name}> {startValue};
    std::set<{enum.name}> unique_exts;
    for (const auto& [extension, enums]: Extended{enum.name}Enums) {{
        if (IsExtEnabled(device_extensions.*extension)) {{
            unique_exts.insert(enums.cbegin(), enums.cend());
        }}
    }}
    std::copy(unique_exts.cbegin(), unique_exts.cend(), std::back_inserter(values));
    return values;
}}\n''')
            out.extend([f'#endif //{enum.protect}\n'] if enum.protect else [])
            out.append('\n')

        out.append("// clang-format on\n")
        self.write(''.join(out))
