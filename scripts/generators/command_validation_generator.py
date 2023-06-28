#!/usr/bin/python3 -i
#
# Copyright (c) 2021-2023 The Khronos Group Inc.
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
from common_codegen import *
from generators.generator_utils import *
from generators.base_generator import BaseGenerator
#
# CommandValidationOutputGenerator - Generate implicit vkCmd validation for CoreChecks
class CommandValidationOutputGenerator(BaseGenerator):
    def __init__(self,
                 errFile = sys.stderr,
                 warnFile = sys.stderr,
                 diagFile = sys.stdout):
        BaseGenerator.__init__(self, errFile, warnFile, diagFile)
        self.headerFile = False # Header file generation flag
        self.sourceFile = False # Source file generation flag

    #
    # Called at beginning of processing as file is opened
    def generate(self):
        self.headerFile = (self.filename == 'command_validation.h')
        self.sourceFile = (self.filename == 'command_validation.cpp')

        copyright = f'''{fileIsGeneratedWarning(os.path.basename(__file__))}
/***************************************************************************
*
* Copyright (c) 2021-2023 Valve Corporation
* Copyright (c) 2021-2023 LunarG, Inc.
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

        if self.headerFile:
            self.generateHeader()
        else:
            self.generateSource()

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateHeader(self):
        self.write('#pragma once')
        self.write('#include <array>')
        #
        # List the enum for the commands
        out = []
        out.append('''
// Used as key for maps of all vkCmd* calls
// Does not include vkBeginCommandBuffer/vkEndCommandBuffer
typedef enum CMD_TYPE {
    CMD_NONE = 0,\n''')
        counter = 1
        for name in filter(lambda x: x.startswith('vkCmd'), self.vk.commands.keys()):
            out.append(f'    CMD_{name[5:].upper()} = {str(counter)},\n')
            counter += 1
        out.append(f'    CMD_RANGE_SIZE = {str(counter)}\n')
        out.append('} CMD_TYPE;\n')
        self.write("".join(out))

        #
        # For each CMD_TYPE give a string name
        out = []
        out.append('static const std::array<const char *, CMD_RANGE_SIZE> kGeneratedCommandNameList = {{\n')
        out.append('    "Command_Undefined",\n')
        for name in filter(lambda x: x.startswith('vkCmd'), self.vk.commands.keys()):
            out.append(f'    "{name}",\n')
        out.append('}};')
        self.write("".join(out))

    def generateSource(self):
        self.write('#include "error_message/logging.h"')
        self.write('#include "core_checks/core_validation.h"')

        #
        # For each CMD_TYPE give a string name add a *-recording VUID
        # Each vkCmd* will have one
        out = []
        out.append('static const std::array<const char *, CMD_RANGE_SIZE> kGeneratedMustBeRecordingList = {{\n')
        out.append('    kVUIDUndefined,\n')
        for command in filter(lambda x: x.name.startswith('vkCmd'), self.vk.commands.values()):
            name = command.name if command.alias is None else command.alias
            vuid = getVUID(self.valid_vuids, f'VUID-{name}-commandBuffer-recording')
            out.append(f'    {vuid},\n')
        out.append('}};')
        self.write("".join(out))

        #
        # For each CMD_TYPE give a queue type and string name add a *-commandBuffer-cmdpool VUID
        # Each vkCmd* will have one
        out = []
        out.append('''
struct CommandSupportedQueueType {
    VkQueueFlags flags;
    const char* vuid;
};
static const std::array<CommandSupportedQueueType, CMD_RANGE_SIZE> kGeneratedQueueTypeList = {{
    {VK_QUEUE_FLAG_BITS_MAX_ENUM, kVUIDUndefined},\n''')
        for command in filter(lambda x: x.name.startswith('vkCmd'), self.vk.commands.values()):
            name = command.name if command.alias is None else command.alias
            flags = []
            flags.extend(["VK_QUEUE_GRAPHICS_BIT"] if Queues.GRAPHICS & command.queues else [])
            flags.extend(["VK_QUEUE_COMPUTE_BIT"] if Queues.COMPUTE & command.queues else [])
            flags.extend(["VK_QUEUE_TRANSFER_BIT"] if Queues.TRANSFER & command.queues else [])
            flags.extend(["VK_QUEUE_SPARSE_BINDING_BIT"] if Queues.SPARSE_BINDING & command.queues else [])
            flags.extend(["VK_QUEUE_PROTECTED_BIT"] if Queues.PROTECTED & command.queues else [])
            flags.extend(["VK_QUEUE_VIDEO_DECODE_BIT_KHR"] if Queues.DECODE & command.queues else [])
            flags.extend(["VK_QUEUE_VIDEO_ENCODE_BIT_KHR"] if Queues.ENCODE & command.queues else [])
            flags.extend(["VK_QUEUE_OPTICAL_FLOW_BIT_NV"] if Queues.OPTICAL_FLOW & command.queues else [])
            flags = ' | '.join(flags)

            vuid = f'VUID-{name}-commandBuffer-cmdpool'
            if vuid not in self.valid_vuids:
                print(f'Warning: Could not find {vuid} in validusage.json')
                vuid = vuid.replace('VUID-', 'UNASSIGNED-')
            out.append(f'    {{{flags}, "{vuid}"}},\n')
        out.append('}};')
        self.write("".join(out))

        #
        # For each CMD_TYPE give a the renderpass restriction and a *-renderpass VUID
        out = []
        out.append('''
enum CMD_SCOPE_TYPE {
    CMD_SCOPE_INSIDE,
    CMD_SCOPE_OUTSIDE,
    CMD_SCOPE_BOTH
};

struct CommandSupportedRenderPass {
    CMD_SCOPE_TYPE renderPass;
    const char* vuid;
};
static const std::array<CommandSupportedRenderPass, CMD_RANGE_SIZE> kGeneratedRenderPassList = {{
    {CMD_SCOPE_BOTH, kVUIDUndefined}, // CMD_NONE\n''')
        for command in filter(lambda x: x.name.startswith('vkCmd'), self.vk.commands.values()):
            name = command.name if command.alias is None else command.alias
            vuid = f'"VUID-{name}-renderpass"'
            renderPassType = ''

            if command.renderPass is CommandScope.INSIDE:
                renderPassType = 'CMD_SCOPE_INSIDE'
            elif command.renderPass is CommandScope.OUTSIDE:
                renderPassType = 'CMD_SCOPE_OUTSIDE'
            elif command.renderPass is CommandScope.BOTH:
                renderPassType = 'CMD_SCOPE_BOTH'
                vuid = 'kVUIDUndefined' # Only will be a VUID if not BOTH

            # Remove string quotes from VUID
            if vuid[1:-1] not in self.valid_vuids and command.renderPass is not CommandScope.BOTH:
                print(f'Warning: Could not find {vuid} in validusage.json')
                vuid = vuid.replace('VUID-', 'UNASSIGNED-')
            out.append(f'    {{{renderPassType}, {vuid}}},\n')
        out.append('}};')
        self.write("".join(out))

        #
        # For each CMD_TYPE give a videocoding restriction and a *-videocoding VUID
        out = []
        out.append('''
struct CommandSupportedVideoCoding {
    CMD_SCOPE_TYPE videoCoding;
    const char* vuid;
};
static const std::array<CommandSupportedVideoCoding, CMD_RANGE_SIZE> kGeneratedVideoCodingList = {{
    {CMD_SCOPE_BOTH, kVUIDUndefined}, // CMD_NONE\n''')
        for command in filter(lambda x: x.name.startswith('vkCmd'), self.vk.commands.values()):
            name = command.name if command.alias is None else command.alias
            vuid = f'"VUID-{name}-videocoding"'
            VideoCodingType = ''

            if command.videoCoding is CommandScope.INSIDE:
                VideoCodingType = 'CMD_SCOPE_INSIDE'
            elif command.videoCoding is CommandScope.OUTSIDE or command.videoCoding is CommandScope.NONE:
                VideoCodingType = 'CMD_SCOPE_OUTSIDE'
            elif command.videoCoding is CommandScope.BOTH:
                VideoCodingType = 'CMD_SCOPE_BOTH'
                vuid = 'kVUIDUndefined' # Only will be a VUID if not BOTH

            # Remove string quotes from VUID
            if vuid[1:-1] not in self.valid_vuids and command.videoCoding is not CommandScope.BOTH:
                print(f'Warning: Could not find {vuid} in validusage.json')
                vuid = vuid.replace('VUID-', 'UNASSIGNED-')
            out.append(f'    {{{VideoCodingType}, {vuid}}},\n')
        out.append('}};')
        self.write("".join(out))

        #
        # For each CMD_TYPE give a buffer level restriction and add a *-bufferlevel VUID
        out = []
        out.append('static const std::array<const char *, CMD_RANGE_SIZE> kGeneratedBufferLevelList = {{\n')
        out.append('    kVUIDUndefined, // CMD_NONE\n')
        for command in filter(lambda x: x.name.startswith('vkCmd'), self.vk.commands.values()):
            name = command.name if command.alias is None else command.alias
            if command.primary and command.secondary:
                out.append('    nullptr,\n')
            elif command.primary:
                vuid = getVUID(self.valid_vuids, f'VUID-{name}-bufferlevel')
                out.append(f'    {vuid},\n')
            else:
                # Currently there is only "primary" or "primary,secondary" in XML
                # Hard to predict what might change, so will error out instead if assumption breaks
                print('cmdbufferlevel attribute was and not known, need to update generation code')
                sys.exit(1)
        out.append('}};')
        self.write("".join(out))

        #
        # The main function to validate all the commands
        # TODO - Remove C++ code from being a single python string
        self.write('''
// Used to handle all the implicit VUs that are autogenerated from the registry
bool CoreChecks::ValidateCmd(const CMD_BUFFER_STATE &cb_state, const CMD_TYPE cmd) const {
    bool skip = false;
    const char *caller_name = CommandTypeString(cmd);

    // Validate the given command being added to the specified cmd buffer,
    // flagging errors if CB is not in the recording state or if there's an issue with the Cmd ordering
    switch (cb_state.state) {
        case CB_RECORDING:
            skip |= ValidateCmdSubpassState(cb_state, cmd);
            break;

        case CB_INVALID_COMPLETE:
        case CB_INVALID_INCOMPLETE:
            skip |= ReportInvalidCommandBuffer(cb_state, caller_name);
            break;

        default:
            assert(cmd != CMD_NONE);
            const auto error = kGeneratedMustBeRecordingList[cmd];
            skip |= LogError(cb_state.commandBuffer(), error, "You must call vkBeginCommandBuffer() before this call to %s.",
                            caller_name);
    }

    // Validate the command pool from which the command buffer is from that the command is allowed for queue type
    const auto supportedQueueType = kGeneratedQueueTypeList[cmd];
    skip |= ValidateCmdQueueFlags(cb_state, caller_name, supportedQueueType.flags, supportedQueueType.vuid);

    // Validate if command is inside or outside a render pass if applicable
    const auto supportedRenderPass = kGeneratedRenderPassList[cmd];
    if (supportedRenderPass.renderPass == CMD_SCOPE_INSIDE) {
        skip |= OutsideRenderPass(cb_state, caller_name, supportedRenderPass.vuid);
    } else if (supportedRenderPass.renderPass == CMD_SCOPE_OUTSIDE) {
        skip |= InsideRenderPass(cb_state, caller_name, supportedRenderPass.vuid);
    }

    // Validate if command is inside or outside a video coding scope if applicable
    const auto supportedVideoCoding = kGeneratedVideoCodingList[cmd];
    if (supportedVideoCoding.videoCoding == CMD_SCOPE_INSIDE) {
        skip |= OutsideVideoCodingScope(cb_state, caller_name, supportedVideoCoding.vuid);
    } else if (supportedVideoCoding.videoCoding == CMD_SCOPE_OUTSIDE) {
        skip |= InsideVideoCodingScope(cb_state, caller_name, supportedVideoCoding.vuid);
    }

    // Validate if command has to be recorded in a primary command buffer
    const auto supportedBufferLevel = kGeneratedBufferLevelList[cmd];
    if (supportedBufferLevel != nullptr) {
        skip |= ValidatePrimaryCommandBuffer(cb_state, caller_name, supportedBufferLevel);
    }

    return skip;
}''')
