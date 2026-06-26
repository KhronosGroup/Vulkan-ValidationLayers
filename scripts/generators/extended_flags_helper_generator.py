#!/usr/bin/python3 -i
#
# Copyright (c) 2026 The Khronos Group Inc.
# Copyright (c) 2026 Valve Corporation
# Copyright (c) 2026 LunarG, Inc.
# Copyright (c) 2026 Google Inc.
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

from base_generator import BaseGenerator
from dataclasses import dataclass

class ExtendedFlagsHelperOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

    def generate(self):
        out = []
        out.append(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2015-2026 The Khronos Group Inc.
            * Copyright (c) 2015-2026 Valve Corporation
            * Copyright (c) 2015-2026 LunarG, Inc.
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
        out.append('// NOLINTBEGIN\n\n') # Wrap for clang-tidy to ignore

        if self.filename == 'extended_flags_helper_generator.h':
            out.append(self.generateHeader())
        elif self.filename == 'extended_flags_helper_generator.cpp':
            out.append(self.generateSource())
        else:
            out.append(f'\nFile name {self.filename} has no code to generate\n')

        out.append('// NOLINTEND') # Wrap for clang-tidy to ignore
        self.write("".join(out))

    def generateHeader(self):
        out = []

        out.append('#include "error_message/error_location.h"\n\n')

        for struct in [x for x in self.vk.structs.values()]:
            for member in [x for x in struct.members if x.extendedFlag]:
                extend_struct = self.vk.structs[member.extendedFlag.struct]
                member_type_name = member.type[2:]
                assert(len(extend_struct.members) == 3)
                extend_member = extend_struct.members[2]
                location_name = member.name[0].upper() + member.name[1:]
                out.append(f'{extend_member.type} Get{member_type_name}(const {struct.name}& create_info);\n')
                out.append(f'Location Get{location_name}Location(const {struct.name}& create_info, const Location& loc);\n\n')

        return "".join(out)

    def generateSource(self):
        out = []

        out.append('#include "extended_flags_helper_generator.h"\n\n')
        out.append('#include <vulkan/utility/vk_struct_helper.hpp>\n')
        out.append('#include "error_message/error_location.h"\n\n')

        for struct in [x for x in self.vk.structs.values()]:
            for member in [x for x in struct.members if x.extendedFlag]:
                extend_struct = self.vk.structs[member.extendedFlag.struct]
                member_type_name = member.type[2:]
                assert(len(extend_struct.members) == 3)
                extend_member = extend_struct.members[2]
                location_name  = member.name[0].upper() + member.name[1:]
                out.append(f'{extend_member.type} Get{member_type_name}(const {struct.name}& create_info) {{')
                out.append(f'const auto extended = vku::FindStructInPNextChain<{extend_struct.name}>(create_info.pNext);')
                out.append(f'if (extended) {{')
                out.append(f'    return extended->{extend_member.name};')
                out.append(f'}}')
                out.append(f'    return static_cast<{extend_member.type}>(create_info.{member.name});')
                out.append(f'}}\n\n')
                out.append(f'Location Get{location_name}Location(const {struct.name}& create_info, const Location& loc) {{')
                out.append(f'const auto extended = vku::FindStructInPNextChain<{extend_struct.name}>(create_info.pNext);')
                out.append(f'if (extended) {{')
                out.append(f'    return loc.pNext(vvl::Struct::{extend_struct.name}).dot(vvl::Field::{extend_member.name});')
                out.append(f'}}')
                out.append(f'    return loc.dot(vvl::Field::{member.name});')
                out.append(f'}}\n\n')

        return "".join(out)
