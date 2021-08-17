#!/usr/bin/python3 -i
#
# Copyright (c) 2021 The Khronos Group Inc.
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
#
# Author: Spencer Fricke <s.fricke@samsung.com>

import os,re,sys,string,json
import xml.etree.ElementTree as etree
from generator import *
from collections import namedtuple
from common_codegen import *

# This is a workaround to use a Python 2.7 and 3.x compatible syntax
from io import open

class CommandValidationOutputGeneratorOptions(GeneratorOptions):
    def __init__(self,
                 conventions = None,
                 filename = None,
                 directory = '.',
                 genpath = None,
                 apiname = None,
                 profile = None,
                 versions = '.*',
                 emitversions = '.*',
                 defaultExtensions = None,
                 addExtensions = None,
                 removeExtensions = None,
                 emitExtensions = None,
                 emitSpirv = None,
                 sortProcedure = regSortFeatures,
                 prefixText = "",
                 genFuncPointers = True,
                 protectFile = True,
                 protectFeature = True,
                 apicall = '',
                 apientry = '',
                 apientryp = '',
                 indentFuncProto = True,
                 indentFuncPointer = False,
                 alignFuncParam = 0,
                 expandEnumerants = True,
                 valid_usage_path = ''):
        GeneratorOptions.__init__(self,
                conventions = conventions,
                filename = filename,
                directory = directory,
                genpath = genpath,
                apiname = apiname,
                profile = profile,
                versions = versions,
                emitversions = emitversions,
                defaultExtensions = defaultExtensions,
                addExtensions = addExtensions,
                removeExtensions = removeExtensions,
                emitExtensions = emitExtensions,
                emitSpirv = emitSpirv,
                sortProcedure = sortProcedure)
        self.prefixText      = prefixText
        self.genFuncPointers = genFuncPointers
        self.protectFile     = protectFile
        self.protectFeature  = protectFeature
        self.apicall         = apicall
        self.apientry        = apientry
        self.apientryp       = apientryp
        self.indentFuncProto = indentFuncProto
        self.indentFuncPointer = indentFuncPointer
        self.alignFuncParam  = alignFuncParam
        self.expandEnumerants = expandEnumerants
        self.valid_usage_path = valid_usage_path
#
# CommandValidationOutputGenerator - Generate implicit vkCmd validation for CoreChecks
class CommandValidationOutputGenerator(OutputGenerator):
    def __init__(self,
                 errFile = sys.stderr,
                 warnFile = sys.stderr,
                 diagFile = sys.stdout):
        OutputGenerator.__init__(self, errFile, warnFile, diagFile)

        self.valid_vuids = set()                          # Set of all valid VUIDs
        self.vuid_dict = dict()                           # VUID dictionary (from JSON)
        self.commands = dict()                            # dictionary of all vkCmd* calls to cmdInfo
        self.alias_dict = dict()                          # Dict of cmd aliases
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
    # Called at beginning of processing as file is opened
    def beginFile(self, genOpts):
        OutputGenerator.beginFile(self, genOpts)
        self.header_file = (genOpts.filename == 'command_validation.h')
        self.source_file = (genOpts.filename == 'command_validation.cpp')

        if not self.header_file and not self.source_file:
            print("Error: Output Filenames have changed, update generator source.\n")
            sys.exit(1)

        # Build a set of all vuid text strings found in validusage.json
        self.valid_usage_path = genOpts.valid_usage_path
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
        file_comment += '// See command_validation_generator.py for modifications\n'
        write(file_comment, file=self.outFile)
        # Copyright Statement
        copyright = ''
        copyright += '\n'
        copyright += '/***************************************************************************\n'
        copyright += ' *\n'
        copyright += ' * Copyright (c) 2021 The Khronos Group Inc.\n'
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
        copyright += ' *\n'
        copyright += ' * Author: Spencer Fricke <s.fricke@samsung.com>\n'
        copyright += ' *\n'
        copyright += ' ****************************************************************************/\n'
        write(copyright, file=self.outFile)

        if self.source_file:
            write('#include "vk_layer_logging.h"', file=self.outFile)
            write('#include "core_validation.h"', file=self.outFile)
        elif self.header_file:
            write('#pragma once', file=self.outFile)
            write('#include <array>', file=self.outFile)

    #
    # Write generated file content to output file
    def endFile(self):
        if self.header_file:
            write(self.commandTypeEnum(), file=self.outFile)
            write(self.commandNameList(), file=self.outFile)
        elif self.source_file:
            write(self.commandRecordingList(), file=self.outFile)
            write(self.commandQueueTypeList(), file=self.outFile)
            write(self.commandRenderPassList(), file=self.outFile)
            write(self.commandBufferLevelList(), file=self.outFile)
            write(self.validateFunction(), file=self.outFile)
        # Finish processing in superclass
        OutputGenerator.endFile(self)

    #
    # Retrieve the type and name for a parameter
    def getTypeNameTuple(self, param):
        type = ''
        name = ''
        for elem in param:
            if elem.tag == 'type':
                type = noneStr(elem.text)
            elif elem.tag == 'name':
                name = noneStr(elem.text)
        return (type, name)

    #
    # Capture command parameter info to be used for param check code generation.
    def genCmd(self, cmdinfo, name, alias):
        OutputGenerator.genCmd(self, cmdinfo, name, alias)
        # Get first param type
        params = cmdinfo.elem.findall('param')
        info = self.getTypeNameTuple(params[0])
        if name.startswith('vkCmd') and info[0] == 'VkCommandBuffer':
            if alias is None:
                self.commands[name] = cmdinfo
            else:
                self.alias_dict[name] = alias

    #
    # List the enum for the commands
    def commandTypeEnum(self):
        output = '''
// Used as key for maps of all vkCmd* calls
// Does not include vkBeginCommandBuffer/vkEndCommandBuffer
typedef enum CMD_TYPE {
    CMD_NONE = 0,\n'''

        counter = 1
        for name, cmdinfo in sorted(self.commands.items()):
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
        for name, cmdinfo in sorted(self.commands.items()):
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
        for name, cmdinfo in sorted(self.commands.items()):
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
        for name, cmdinfo in sorted(self.commands.items()):
            flags = []
            queues = cmdinfo.elem.attrib.get('queues').split(',')
            for queue in queues:
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
                else:
                    print("A new queue type %s was added to VkQueueFlagBits and need to update generation code", queue)
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
        for name, cmdinfo in sorted(self.commands.items()):
            render_pass_type = ''
            render_pass = cmdinfo.elem.attrib.get('renderpass')
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
    # For each CMD_TYPE give a buffer level restriction and add a *-bufferlevel VUID
    def commandBufferLevelList(self):
        output = '''
static const std::array<const char *, CMD_RANGE_SIZE> kGeneratedBufferLevelList = {{
    kVUIDUndefined, // CMD_NONE\n'''
        for name, cmdinfo in sorted(self.commands.items()):
            buffer_level = cmdinfo.elem.attrib.get('cmdbufferlevel')
            # Currently there is only "primary" or "primary,secondary" in XML
            # Hard to predict what might change, so will error out instead if assumption breaks
            if buffer_level == "primary,secondary":
                output += '    nullptr,\n'
            elif buffer_level == "primary":
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
bool CoreChecks::ValidateCmd(const CMD_BUFFER_STATE *cb_state, const CMD_TYPE cmd, const char *caller_name) const {
    bool skip = false;

    // Validate the given command being added to the specified cmd buffer,
    // flagging errors if CB is not in the recording state or if there's an issue with the Cmd ordering
    switch (cb_state->state) {
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
            skip |= LogError(cb_state->commandBuffer(), error, "You must call vkBeginCommandBuffer() before this call to %s.",
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

    // Validate if command has to be recorded in a primary command buffer
    const auto supportedBufferLevel = kGeneratedBufferLevelList[cmd];
    if (supportedBufferLevel != nullptr) {
        skip |= ValidatePrimaryCommandBuffer(cb_state, caller_name, supportedBufferLevel);
    }

    return skip;
}'''
        return output
