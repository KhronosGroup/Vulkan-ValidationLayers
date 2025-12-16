#!/usr/bin/python3 -i
#
# Copyright (c) 2021-2025 The Khronos Group Inc.
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
import sys
from generators.generator_utils import buildListVUID, getVUID
from vulkan_object import CommandScope
from base_generator import BaseGenerator
#
# CommandValidationOutputGenerator - Generate implicit vkCmd validation for CoreChecks
class CommandValidationOutputGenerator(BaseGenerator):
    def __init__(self,
                 valid_usage_file):
        BaseGenerator.__init__(self)
        self.valid_vuids = buildListVUID(valid_usage_file)
    #
    # Called at beginning of processing as file is opened
    def generate(self):
        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2021-2025 Valve Corporation
            * Copyright (c) 2021-2025 LunarG, Inc.
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

        if self.filename == 'command_validation.h':
            self.generateHeader()
        elif self.filename == 'command_validation.cpp':
            self.generateSource()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateHeader(self):
        out = []
        out.append('''
            #pragma once

            #include "generated/error_location_helper.h"

            enum class CommandScope { Inside, Outside, Both };

            struct CommandValidationInfo {
                const char* recording_vuid;
                const char* buffer_level_vuid;

                VkQueueFlags queue_flags;
                const char* queue_vuid;

                CommandScope render_pass_scope;
                const char* render_pass_vuid;

                CommandScope video_coding_scope;
                const char* video_coding_vuid;

                const char* suspended_vuid;

                bool state;
                bool action;
                bool synchronization;
            };

            const CommandValidationInfo& GetCommandValidationInfo(vvl::Func command);
            ''')
        self.write("".join(out))

    def generateSource(self):
        out = []
        out.append('''
            #include "command_validation.h"
            #include "containers/custom_containers.h"
            ''')
        out.append('// clang-format off\n')
        out.append('extern const char *kVUIDUndefined;\n')
        out.append('using Func = vvl::Func;\n')
        out.append('static const auto &GetCommandValidationTable() {\n')
        out.append('static const vvl::unordered_map<Func, CommandValidationInfo> kCommandValidationTable {\n')
        for command in [x for x in self.vk.commands.values() if x.name.startswith('vkCmd')]:
            out.append(f'{{Func::{command.name}, {{\n')
            # recording_vuid
            alias_name = command.name if command.alias is None else command.alias
            vuid = getVUID(self.valid_vuids, f'VUID-{alias_name}-commandBuffer-recording')
            out.append(f'    {vuid},\n')

            # buffer_level_vuid
            if command.primary and command.secondary:
                out.append('    nullptr,\n')
            elif command.primary:
                vuid = getVUID(self.valid_vuids, f'VUID-{alias_name}-bufferlevel')
                out.append(f'    {vuid},\n')
            else:
                # Currently there is only "primary" or "primary,secondary" in XML
                # Hard to predict what might change, so will error out instead if assumption breaks
                print('cmdbufferlevel attribute was and not known, need to update generation code')
                sys.exit(1)

            queue_flags = ' | '.join(command.queues)
            vuid = getVUID(self.valid_vuids, f'VUID-{alias_name}-commandBuffer-cmdpool')
            out.append(f'    {queue_flags}, {vuid},\n')

            # render_pass / render_pass_vuid
            renderPassType = 'CommandScope::Both'
            vuid = 'kVUIDUndefined' # Only will be a VUID if not BOTH
            if command.renderPass is CommandScope.INSIDE:
                renderPassType = 'CommandScope::Inside'
                vuid = getVUID(self.valid_vuids, f'VUID-{alias_name}-renderpass')
            elif command.renderPass is CommandScope.OUTSIDE:
                renderPassType = 'CommandScope::Outside'
                vuid = getVUID(self.valid_vuids, f'VUID-{alias_name}-renderpass')
            out.append(f'    {renderPassType}, {vuid},\n')

            # video_coding / video_coding_vuid
            videoCodingType = 'CommandScope::Both'
            vuid = 'kVUIDUndefined' # Only will be a VUID if not BOTH
            if command.videoCoding is CommandScope.INSIDE:
                videoCodingType = 'CommandScope::Inside'
                vuid = getVUID(self.valid_vuids, f'VUID-{alias_name}-videocoding')
            elif command.videoCoding is CommandScope.OUTSIDE or command.videoCoding is CommandScope.NONE:
                videoCodingType = 'CommandScope::Outside'
                vuid = getVUID(self.valid_vuids, f'VUID-{alias_name}-videocoding')
            out.append(f'    {videoCodingType}, {vuid},\n')

            # command type
            is_state = 'true' if 'state' in command.tasks else 'false'
            is_action = 'true' if 'action' in command.tasks else 'false'
            is_synchronization = 'true' if 'synchronization' in command.tasks else 'false'

            vuid = 'kVUIDUndefined'
            if is_action == 'true' or is_synchronization == 'true':
                vuid = getVUID(self.valid_vuids, f'VUID-{alias_name}-suspended')
            out.append(f'    {vuid},\n')

            out.append(f'    {is_state}, {is_action}, {is_synchronization},\n')

            out.append('}},\n')
        out.append('};\n')
        out.append('return kCommandValidationTable;\n')
        out.append('}\n')

        out.append('''
const CommandValidationInfo& GetCommandValidationInfo(vvl::Func command) {
    auto info_it = GetCommandValidationTable().find(command);
    assert(info_it != GetCommandValidationTable().end());
    return info_it->second;
}
            ''')
        out.append('// clang-format on\n')
        self.write("".join(out))