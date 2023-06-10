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

import os,sys,json
from generator import *
from common_codegen import *
from generators.base_generator import BaseGenerator

# This is a workaround to use a Python 2.7 and 3.x compatible syntax
from io import open

#
# CommandValidationOutputGenerator - Generate implicit vkCmd validation for CoreChecks
class CommandValidationOutputGenerator(BaseGenerator):
    def __init__(self,
                 errFile = sys.stderr,
                 warnFile = sys.stderr,
                 diagFile = sys.stdout):
        BaseGenerator.__init__(self, errFile, warnFile, diagFile)

        self.valid_vuids = set()                          # Set of all valid VUIDs
        self.vuid_dict = dict()                           # VUID dictionary (from JSON)
        self.header_file = False                          # Header file generation flag
        self.source_file = False                          # Source file generation flag

    #
    # Walk the JSON-derived dict and find all "vuid" key values
    def ExtractVUIDs(self, d):
        if hasattr(d, 'items'):
            for k, v in d.items():
                if k == "vuid":
                    yield v
                elif isinstance(v, dict):
                    for s in self.ExtractVUIDs(v):
                        yield s
                elif isinstance (v, list):
                    for l in v:
                        for s in self.ExtractVUIDs(l):
                            yield s

    #
    # Write generated file content to output file
    def endFile(self):
        self.header_file = (self.filename == 'command_validation.h')
        self.source_file = (self.filename == 'command_validation.cpp')

        # Build a set of all vuid text strings found in validusage.json
        vu_json_filename = os.path.join(self.valid_usage_path + os.sep, 'validusage.json')
        if os.path.isfile(vu_json_filename):
            json_file = open(vu_json_filename, 'r', encoding='utf-8')
            self.vuid_dict = json.load(json_file)
            json_file.close()
        if len(self.vuid_dict) == 0:
            print("Error: Could not find, or error loading %s/validusage.json\n", vu_json_filename)
            sys.exit(1)
        for json_vuid_string in self.ExtractVUIDs(self.vuid_dict):
            self.valid_vuids.add(json_vuid_string)

        # File Comment
        file_comment = '// *** THIS FILE IS GENERATED - DO NOT EDIT ***\n'
        file_comment += '// See {} for modifications\n'.format(os.path.basename(__file__))
        write(file_comment, file=self.outFile)
        # Copyright Statement
        copyright = ''
        copyright += '\n'
        copyright += '/***************************************************************************\n'
        copyright += ' *\n'
        copyright += ' * Copyright (c) 2021-2023 The Khronos Group Inc.\n'
        copyright += ' *\n'
        copyright += ' * Licensed under the Apache License, Version 2.0 (the "License");\n'
        copyright += ' * you may not use this file except in compliance with the License.\n'
        copyright += ' * You may obtain a copy of the License at\n'
        copyright += ' *\n'
        copyright += ' *     http://www.apache.org/licenses/LICENSE-2.0\n'
        copyright += ' *\n'
        copyright += ' * Unless required by applicable law or agreed to in writing, software\n'
        copyright += ' * distributed under the License is distributed on an "AS IS" BASIS,\n'
        copyright += ' * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n'
        copyright += ' * See the License for the specific language governing permissions and\n'
        copyright += ' * limitations under the License.\n'
        copyright += ' ****************************************************************************/\n'
        write(copyright, file=self.outFile)

        if self.header_file:
            write('#pragma once', file=self.outFile)
            write('#include <array>', file=self.outFile)
            write(self.commandTypeEnum(), file=self.outFile)
            write(self.commandNameList(), file=self.outFile)
        elif self.source_file:
            write('#include "error_message/logging.h"', file=self.outFile)
            write('#include "core_checks/core_validation.h"', file=self.outFile)
            write(self.commandRecordingList(), file=self.outFile)
            write(self.commandQueueTypeList(), file=self.outFile)
            write(self.commandRenderPassList(), file=self.outFile)
            write(self.commandVideoCodingList(), file=self.outFile)
            write(self.commandBufferLevelList(), file=self.outFile)
            write(self.validateFunction(), file=self.outFile)
        # Finish processing in superclass
        BaseGenerator.endFile(self)

    #
    # List the enum for the commands
    def commandTypeEnum(self):
        output = '''
// Used as key for maps of all vkCmd* calls
// Does not include vkBeginCommandBuffer/vkEndCommandBuffer
typedef enum CMD_TYPE {
    CMD_NONE = 0,\n'''

        counter = 1
        for name, _ in sorted(self.vk.recording_commands.items()):
            output += '    CMD_' + name[5:].upper() + ' = ' + str(counter) + ',\n'
            counter += 1

        output += '    CMD_RANGE_SIZE = ' + str(counter)
        output += '''
} CMD_TYPE;'''
        return output

    #
    # For each CMD_TYPE give a string name
    def commandNameList(self):
        output = '''
static const std::array<const char *, CMD_RANGE_SIZE> kGeneratedCommandNameList = {{
    "Command_Undefined",\n'''
        for name, _ in sorted(self.vk.recording_commands.items()):
            output += '    \"' + name + '\",\n'
        output += '}};'
        return output

    #
    # For each CMD_TYPE give a string name add a *-recording VUID
    # Each vkCmd* will have one
    def commandRecordingList(self):
        output = '''
static const std::array<const char *, CMD_RANGE_SIZE> kGeneratedMustBeRecordingList = {{
    kVUIDUndefined,\n'''
        for name, command in sorted(self.vk.recording_commands.items()):
            if command.alias:
                name = command.alias
            vuid = 'VUID-' + name + '-commandBuffer-recording'
            if vuid not in self.valid_vuids:
                print("Warning: Could not find {} in validusage.json".format(vuid))
                vuid = vuid.replace('VUID-', 'UNASSIGNED-')
            output += '    \"' + vuid + '\",\n'
        output += '}};'
        return output

    #
    # For each CMD_TYPE give a queue type and string name add a *-commandBuffer-cmdpool VUID
    # Each vkCmd* will have one
    def commandQueueTypeList(self):
        output = '''
struct CommandSupportedQueueType {
    VkQueueFlags flags;
    const char* vuid;
};
static const std::array<CommandSupportedQueueType, CMD_RANGE_SIZE> kGeneratedQueueTypeList = {{
    {VK_QUEUE_FLAG_BITS_MAX_ENUM, kVUIDUndefined},\n'''
        for name, command in sorted(self.vk.recording_commands.items()):
            if command.alias:
                name = command.alias
            flags = []
            for queue in command.queues:
                if queue == 'graphics':
                    flags.append("VK_QUEUE_GRAPHICS_BIT")
                elif queue == 'compute':
                    flags.append("VK_QUEUE_COMPUTE_BIT")
                elif queue == 'transfer':
                    flags.append("VK_QUEUE_TRANSFER_BIT")
                elif queue == 'sparse_binding':
                    flags.append("VK_QUEUE_SPARSE_BINDING_BIT")
                elif queue == 'protected':
                    flags.append("VK_QUEUE_PROTECTED_BIT")
                elif queue == 'decode':
                    flags.append("VK_QUEUE_VIDEO_DECODE_BIT_KHR")
                elif queue == 'encode':
                    flags.append("VK_QUEUE_VIDEO_ENCODE_BIT_KHR")
                elif queue == 'opticalflow':
                    flags.append("VK_QUEUE_OPTICAL_FLOW_BIT_NV")
                else:
                    print(f'A new queue type {queue} was added to VkQueueFlagBits and need to update generation code')
                    sys.exit(1)
            vuid = 'VUID-' + name + '-commandBuffer-cmdpool'
            if vuid not in self.valid_vuids:
                print("Warning: Could not find {} in validusage.json".format(vuid))
                vuid = vuid.replace('VUID-', 'UNASSIGNED-')
            output += '    {' + ' | '.join(flags) + ', \"' + vuid + '\"},\n'
        output += '}};'
        return output

    #
    # For each CMD_TYPE give a the renderpass restriction and a *-renderpass VUID
    def commandRenderPassList(self):
        output = '''
enum CMD_RENDER_PASS_TYPE {
    CMD_RENDER_PASS_INSIDE,
    CMD_RENDER_PASS_OUTSIDE,
    CMD_RENDER_PASS_BOTH
};
struct CommandSupportedRenderPass {
    CMD_RENDER_PASS_TYPE renderPass;
    const char* vuid;
};
static const std::array<CommandSupportedRenderPass, CMD_RANGE_SIZE> kGeneratedRenderPassList = {{
    {CMD_RENDER_PASS_BOTH, kVUIDUndefined}, // CMD_NONE\n'''
        for name, command in sorted(self.vk.recording_commands.items()):
            if command.alias:
                name = command.alias
            render_pass_type = ''
            render_pass = command.renderpass
            if render_pass == 'inside':
                render_pass_type = 'CMD_RENDER_PASS_INSIDE'
            elif render_pass == 'outside':
                render_pass_type = 'CMD_RENDER_PASS_OUTSIDE'
            elif render_pass != 'both':
                print("renderpass attribute was %s and not known, need to update generation code", renderpass)
                sys.exit(1)

            # Only will be a VUID if not BOTH
            if render_pass == 'both':
                output += '    {CMD_RENDER_PASS_BOTH, kVUIDUndefined},\n'
            else:
                vuid = 'VUID-' + name + '-renderpass'
                if vuid not in self.valid_vuids:
                    print("Warning: Could not find {} in validusage.json".format(vuid))
                    vuid = vuid.replace('VUID-', 'UNASSIGNED-')
                output += '    {' + render_pass_type + ', \"' + vuid + '\"},\n'
        output += '}};'
        return output

    #
    # For each CMD_TYPE give a videocoding restriction and a *-videocoding VUID
    def commandVideoCodingList(self):
        output = '''
enum CMD_VIDEO_CODING_TYPE {
    CMD_VIDEO_CODING_INSIDE,
    CMD_VIDEO_CODING_OUTSIDE,
    CMD_VIDEO_CODING_BOTH
};
struct CommandSupportedVideoCoding {
    CMD_VIDEO_CODING_TYPE videoCoding;
    const char* vuid;
};
static const std::array<CommandSupportedVideoCoding, CMD_RANGE_SIZE> kGeneratedVideoCodingList = {{
    {CMD_VIDEO_CODING_BOTH, kVUIDUndefined}, // CMD_NONE\n'''
        for name, command in sorted(self.vk.recording_commands.items()):
            if command.alias:
                name = command.alias
            video_coding_type = ''
            video_coding = command.videocoding
            if video_coding is None:
                video_coding = 'outside'
            if video_coding == 'inside':
                video_coding_type = 'CMD_VIDEO_CODING_INSIDE'
            elif video_coding == 'outside':
                video_coding_type = 'CMD_VIDEO_CODING_OUTSIDE'
            elif video_coding != 'both':
                print("videocoding attribute was %s and not known, need to update generation code", video_coding)
                sys.exit(1)

            # Only will be a VUID if not BOTH
            if video_coding == 'both':
                output += '    {CMD_VIDEO_CODING_BOTH, kVUIDUndefined},\n'
            else:
                vuid = 'VUID-' + name + '-videocoding'
                if vuid not in self.valid_vuids:
                    print("Warning: Could not find {} in validusage.json".format(vuid))
                    vuid = vuid.replace('VUID-', 'UNASSIGNED-')
                output += '    {' + video_coding_type + ', \"' + vuid + '\"},\n'
        output += '}};'
        return output

    #
    # For each CMD_TYPE give a buffer level restriction and add a *-bufferlevel VUID
    def commandBufferLevelList(self):
        output = '''
static const std::array<const char *, CMD_RANGE_SIZE> kGeneratedBufferLevelList = {{
    kVUIDUndefined, // CMD_NONE\n'''
        for name, command in sorted(self.vk.recording_commands.items()):
            if command.alias:
                name = command.alias
            buffer_level = command.cmdbufferlevel
            # Currently there is only "primary" or "primary,secondary" in XML
            # Hard to predict what might change, so will error out instead if assumption breaks
            if len(buffer_level) > 1:
                output += '    nullptr,\n'
            elif 'primary' in buffer_level:
                vuid = 'VUID-' + name + '-bufferlevel'
                if vuid not in self.valid_vuids:
                    print("Warning: Could not find {} in validusage.json".format(vuid))
                    vuid = vuid.replace('VUID-', 'UNASSIGNED-')
                output += '    \"' + vuid + '\",\n'
            else:
                print("cmdbufferlevel attribute was %s and not known, need to update generation code", buffer_level)
                sys.exit(1)
        output += '}};'
        return output

    #
    # The main function to validate all the commands
    def validateFunction(self):
        output = '''
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
    if (supportedRenderPass.renderPass == CMD_RENDER_PASS_INSIDE) {
        skip |= OutsideRenderPass(cb_state, caller_name, supportedRenderPass.vuid);
    } else if (supportedRenderPass.renderPass == CMD_RENDER_PASS_OUTSIDE) {
        skip |= InsideRenderPass(cb_state, caller_name, supportedRenderPass.vuid);
    }

    // Validate if command is inside or outside a video coding scope if applicable
    const auto supportedVideoCoding = kGeneratedVideoCodingList[cmd];
    if (supportedVideoCoding.videoCoding == CMD_VIDEO_CODING_INSIDE) {
        skip |= OutsideVideoCodingScope(cb_state, caller_name, supportedVideoCoding.vuid);
    } else if (supportedVideoCoding.videoCoding == CMD_VIDEO_CODING_OUTSIDE) {
        skip |= InsideVideoCodingScope(cb_state, caller_name, supportedVideoCoding.vuid);
    }

    // Validate if command has to be recorded in a primary command buffer
    const auto supportedBufferLevel = kGeneratedBufferLevelList[cmd];
    if (supportedBufferLevel != nullptr) {
        skip |= ValidatePrimaryCommandBuffer(cb_state, caller_name, supportedBufferLevel);
    }

    return skip;
}'''
        return output

