#!/usr/bin/python3 -i
#
# Copyright (c) 2023 The Khronos Group Inc.
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

#
# DynamicStateOutputGenerator - Generate SPIR-V validation
# for SPIR-V extensions and capabilities
class DynamicStateOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

    def generate(self):
        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
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
            ****************************************************************************/\n''')

        self.write('// NOLINTBEGIN') # Wrap for clang-tidy to ignore

        if self.filename == 'dynamic_state_helper.h':
            self.generateHeader()
        elif self.filename == 'dynamic_state_helper.cpp':
            self.generateSource()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateHeader(self):
        out = []
        out.append('''
            #pragma once
            #include <bitset>

            // Reorders VkDynamicState so it can be a bitset
            typedef enum CBDynamicState {
            ''')
        for index, field in enumerate(self.vk.enums['VkDynamicState'].fields, start=1):
            # VK_DYNAMIC_STATE_LINE_WIDTH -> STATE_LINE_WIDTH
            out.append(f'CB_DYNAMIC_{field.name[11:]} = {index},\n')

        out.append(f'CB_DYNAMIC_STATE_STATUS_NUM = {len(self.vk.enums["VkDynamicState"].fields) + 1}')
        out.append('''
            } CBDynamicState;

            using CBDynamicFlags = std::bitset<CB_DYNAMIC_STATE_STATUS_NUM>;
            CBDynamicState ConvertToCBDynamicState(VkDynamicState dynamic_state);
            const char* DynamicStateToString(CBDynamicState dynamic_state);
            std::string DynamicStatesToString(CBDynamicFlags const &dynamic_states);
            ''')
        self.write("".join(out))

    def generateSource(self):
        out = []
        out.append('''
            #include "core_checks/core_validation.h"

            static VkDynamicState ConvertToDynamicState(CBDynamicState dynamic_state) {
                switch (dynamic_state) {
            ''')
        for field in self.vk.enums['VkDynamicState'].fields:
            # VK_DYNAMIC_STATE_LINE_WIDTH -> STATE_LINE_WIDTH
            out.append(f'case CB_DYNAMIC_{field.name[11:]}:\n')
            out.append(f'    return {field.name};\n')

        out.append('''
                    default:
                        return VK_DYNAMIC_STATE_MAX_ENUM;
                }
            }
            ''')

        out.append('''
            CBDynamicState ConvertToCBDynamicState(VkDynamicState dynamic_state) {
                switch (dynamic_state) {
            ''')

        for field in self.vk.enums['VkDynamicState'].fields:
            # VK_DYNAMIC_STATE_LINE_WIDTH -> STATE_LINE_WIDTH
            out.append(f'case {field.name}:\n')
            out.append(f'    return CB_DYNAMIC_{field.name[11:]};\n')
        out.append('''
                    default:
                        return CB_DYNAMIC_STATE_STATUS_NUM;
                }
            }
            ''')

        out.append('''
            const char* DynamicStateToString(CBDynamicState dynamic_state) {
                return string_VkDynamicState(ConvertToDynamicState(dynamic_state));
            }

            std::string DynamicStatesToString(CBDynamicFlags const& dynamic_states) {
                std::string ret;
                // enum is not zero based
                for (int index = 1; index < CB_DYNAMIC_STATE_STATUS_NUM; ++index) {
                    CBDynamicState status = static_cast<CBDynamicState>(index);
                    if (dynamic_states[status]) {
                        if (!ret.empty()) ret.append("|");
                        ret.append(string_VkDynamicState(ConvertToDynamicState(status)));
                    }
                }
                if (ret.empty()) ret.append(string_VkDynamicState(ConvertToDynamicState(CB_DYNAMIC_STATE_STATUS_NUM)));
                return ret;
            }
            ''')
        self.write("".join(out))