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

import sys
from generator import *
from common_codegen import *

# This is a workaround to use a Python 2.7 and 3.x compatible syntax
from io import open

# Some DRY constants
vvl_fake_extension = '_SYNCVAL'
present_stage = 'VK_PIPELINE_STAGE_2_PRESENT_ENGINE_BIT' + vvl_fake_extension
presenting_access = 'VK_ACCESS_2_PRESENT_ACQUIRE_READ_BIT' + vvl_fake_extension  # Treated as read
presented_access = 'VK_ACCESS_2_PRESENT_PRESENTED_BIT' + vvl_fake_extension  # Treated as write
present_engine_accesses = (presenting_access, presented_access)

syncEnumStageType = 'VkPipelineStageFlagBits2'
syncEnumAccessType = 'VkAccessFlagBits2'
syncEnumTypes = (syncEnumStageType, syncEnumAccessType)

# Maps the 'queuetype' in XML to VkQueueFlagBits
queueTypeToQueueFlags = {
    'transfer' : 'VK_QUEUE_TRANSFER_BIT',
    'graphics' : 'VK_QUEUE_GRAPHICS_BIT',
    'compute' : 'VK_QUEUE_COMPUTE_BIT',
    'decode' : 'VK_QUEUE_VIDEO_DECODE_BIT_KHR',
    'encode'  : 'VK_QUEUE_VIDEO_ENCODE_BIT_KHR',
    'opticalflow' : 'VK_QUEUE_OPTICAL_FLOW_BIT_NV',
}

def BitSuffixed(name):
    alt_bit = ('_ANDROID', '_EXT', '_IMG', '_KHR', '_NV', '_NVX', vvl_fake_extension)
    bit_suf = name + '_BIT'
    # Since almost every bit ends with _KHR, just ignore it.
    # Otherwise some generated names end up with every other word being KHR.
    if name.endswith('_KHR') :
            bit_suf = name.replace('_KHR', '_BIT')
    else:
        for alt in alt_bit:
            if name.endswith(alt) :
                bit_suf = name.replace(alt, '_BIT' + alt)
                break
    return bit_suf

class SyncValidationOutputGeneratorOptions(GeneratorOptions):
    def __init__(self,
                 conventions = None,
                 filename = None,
                 directory = '.',
                 genpath = None,
                 apiname = 'vulkan',
                 profile = None,
                 versions = '.*',
                 emitversions = '.*',
                 defaultExtensions = 'vulkan',
                 addExtensions = None,
                 removeExtensions = None,
                 emitExtensions = None,
                 emitSpirv = None,
                 emitFormats = None,
                 sortProcedure = regSortFeatures,
                 genFuncPointers = True,
                 protectFile = True,
                 protectFeature = False,
                 apicall = 'VKAPI_ATTR ',
                 apientry = 'VKAPI_CALL ',
                 apientryp = 'VKAPI_PTR *',
                 indentFuncProto = True,
                 indentFuncPointer = False,
                 alignFuncParam = 48,
                 expandEnumerants = False):
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
                emitFormats = emitFormats,
                sortProcedure = sortProcedure)
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
#
# SyncValidationOutputGenerator - Generate sync validation
class SyncValidationOutputGenerator(OutputGenerator):
    def __init__(self,
                 errFile = sys.stderr,
                 warnFile = sys.stderr,
                 diagFile = sys.stdout):
        OutputGenerator.__init__(self, errFile, warnFile, diagFile)
        self.headerFile = False # Header file generation flag
        self.sourceFile = False # Source file generation flag

        self.syncEnum = dict() # Handy copy of synchronization enum data

        self.shaderStages = [] # VK_PIPELINE_STAGE_*_SHADER_BIT
         # < pipeline_name, [pipeline stages in logical order] >
        self.pipelineStageList = dict()
        # List all pipeline stages, not depending on pipeline type
        self.allPipelineStageSet = set()
        # < pipeline_name, { stage : 'stage', after : [stages], before : [stages] }  >
        self.pipelineStageDependencies = dict()
        # < queue type, [stages] >
        self.queueToStages = dict()
        # < stage, [equivalent stages] >
        self.stageEquivalent = dict()
        # < access, [equivalent accesses] >
        self.accessEquivalent = dict()
        # < access, [stages] >
        self.accessToStageMap = dict()
        # < stage, [accesses] >
        self.stageToAccessMap = dict()

        self.enumsInBitOrder = dict()
        self.stageAccessCombo = dict()

    #
    # Called at beginning of processing as file is opened
    def beginFile(self, genOpts):
        OutputGenerator.beginFile(self, genOpts)
        self.headerFile = genOpts.filename.endswith('.h')
        self.sourceFile = not self.headerFile

        # File Comment
        file_comment = '// *** THIS FILE IS GENERATED - DO NOT EDIT ***\n'
        file_comment += '// See format_utils_generator.py for modifications\n'
        write(file_comment, file=self.outFile)
        # Copyright Statement
        copyright = ''
        copyright += '\n'
        copyright += '/***************************************************************************\n'
        copyright += ' *\n'
        copyright += ' * Copyright (c) 2015-2023 The Khronos Group Inc.\n'
        copyright += ' * Copyright (c) 2015-2023 Valve Corporation\n'
        copyright += ' * Copyright (c) 2015-2023 LunarG, Inc.\n'
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
        if self.sourceFile:
            write('#include "sync_validation_types.h"', file=self.outFile)
        elif self.headerFile:
            write('#pragma once', file=self.outFile)
            write('#include <array>', file=self.outFile)
            write('#include <bitset>', file=self.outFile)
            write('#include <map>', file=self.outFile)
            write('#include <stdint.h>', file=self.outFile)
            write('#include <vulkan/vulkan.h>', file=self.outFile)
            write('#include "containers/custom_containers.h"', file=self.outFile)

    # Write generated file content to output file
    def endFile(self):
        # Fake stages and accesses for acquire present support
        self.accessToStageMap[presented_access] = [present_stage]
        self.accessToStageMap[presenting_access] = [present_stage]

        # Need to be done after parsing, but before generating code
        self.enumsInBitOrder = self.getEnumsInBitOrder()
        self.stageAccessCombo = self.createStageAccessCombinations()

        write(self.defines(), file=self.outFile)
        write(self.accessIndex(), file=self.outFile)
        write(self.accessFlags(), file=self.outFile)
        write(self.accessReadWriteMask(), file=self.outFile)
        write(self.infoByStageAccessIndex(), file=self.outFile)
        write(self.accessMaskByStageBit(), file=self.outFile)
        write(self.accessMaskByShaderStage(), file=self.outFile)
        write(self.accessMaskByAccessBit(), file=self.outFile)
        write(self.directStageToAccessMask(), file=self.outFile)
        write(self.allCommandStagesByQueueFlags(), file=self.outFile)
        write(self.logicallyEarlierStages(), file=self.outFile)
        write(self.logicallyLaterStages(), file=self.outFile)

        # Finish processing in superclass
        OutputGenerator.endFile(self)

    #
    # Grab group (e.g. C "enum" type) info to output for enum-string conversion helper
    def genGroup(self, groupinfo, groupName, alias):
        OutputGenerator.genGroup(self, groupinfo, groupName, alias)
        if groupName in syncEnumTypes:
            self.syncEnum[groupName] = []
            groupElem = groupinfo.elem
            for elem in groupElem.findall('enum'):
                if elem.get('supported') != 'disabled':
                    self.syncEnum[groupName].append(elem)
                # While looping through everything, init the array in the dict
                name = elem.get('alias') if elem.get('alias') is not None else elem.get('name')
                if groupName == syncEnumStageType:
                    self.stageEquivalent[name] = []
                    self.stageToAccessMap[name] = []
                elif groupName == syncEnumAccessType:
                    self.accessEquivalent[name] = []
                    self.accessToStageMap[name] = []

    # Gets all <syncstage>
    def genSyncStage(self, sync):
        name = sync.elem.get('name')
        self.allPipelineStageSet.add(name)
        if name.endswith('_SHADER_BIT'):
            self.shaderStages.append(name)

        queueTypes = ['transfer', 'graphics', 'compute', 'decode', 'encode', 'sparse_binding', 'opticalflow'] # default
        syncSupport = sync.elem.find('syncsupport')
        # If not support tag, all are supported
        if syncSupport is not None:
           queueTypes = syncSupport.get('queues').split(',')
        for queueType in queueTypes:
            if queueType not in self.queueToStages:
                self.queueToStages[queueType] = []
            self.queueToStages[queueType].append(name)

        syncEquivalent = sync.elem.find('syncequivalent')
        if syncEquivalent is not None:
            self.stageEquivalent[name] = syncEquivalent.get('stage').split(',')

    # Gets all <syncaccess>
    def genSyncAccess(self, sync):
        name = sync.elem.get('name')
        syncSupport = sync.elem.find('syncsupport')
        if syncSupport is not None:
            supportStages = syncSupport.get('stage').split(',')
            self.accessToStageMap[name] = supportStages
            for stage in supportStages:
                self.stageToAccessMap[stage].append(name)

        syncEquivalent = sync.elem.find('syncequivalent')
        if syncEquivalent is not None:
            self.accessEquivalent[name] = syncEquivalent.get('access').split(',')

    # Gets all <syncpipeline>
    def genSyncPipeline(self, sync):
        name = sync.elem.get('name').replace(' ', '_')
        self.pipelineStageList[name] = []
        self.pipelineStageDependencies[name] = []
        for elem in sync.elem.findall('syncpipelinestage'):
            self.pipelineStageList[name].append(elem.text)
            order = elem.get('order')
            before = elem.get('before')
            after = elem.get('after')
            stageOrder = {'stage' : elem.text, 'before' : [], 'after' : []}
            if before is not None:
                stageOrder['before'].append(before.split(','))

            if after is not None:
                stageOrder['after'].append(after.split(','))

            self.pipelineStageDependencies[name].append(stageOrder)
        return

    def getInBitOrder(self, tag, enum_elem):
        # The input may be unordered or sparse w.r.t. the mask field, sort and gap fill
        found = []
        for elem in enum_elem:
            bitpos = elem.get('bitpos')
            name = elem.get('name')
            if not bitpos:
                continue

            if name.endswith('NONE_KHR'):
                break

            found.append({'name': name, 'bitpos': int(bitpos)})

        in_bit_order = []
        for entry in sorted(found, key=lambda record: record['bitpos']):
            bitpos = entry['bitpos']
            in_bit_order.append({'name': entry['name'], 'mask': (1 << bitpos), 'bitpos': bitpos})

        return in_bit_order


    def getEnumsInBitOrder(self):
        enum_in_bit_order = dict()
        for enum_type in syncEnumTypes:
            enum_in_bit_order[enum_type] = self.getInBitOrder(enum_type, self.syncEnum[enum_type])

        # add the fake present engine enums
        stage_bitpos = enum_in_bit_order[syncEnumStageType][-1]['bitpos'] + 1
        enum_in_bit_order[syncEnumStageType].append({'name': present_stage, 'mask': (1 << stage_bitpos), 'bitpos': stage_bitpos})

        access_bitpos = enum_in_bit_order[syncEnumAccessType][-1]['bitpos']
        for name in present_engine_accesses:
            access_bitpos = access_bitpos + 1
            enum_in_bit_order[syncEnumAccessType].append({'name': name, 'mask': (1 << access_bitpos), 'bitpos': access_bitpos})

        return enum_in_bit_order


    #
    # Create defines that are used either by other files (headerFile) or just internally (sourceFile)
    def defines(self):
        output = '\n'
        if self.headerFile:
            output += '// Fake stages and accesses for acquire present support\n'
            for enumType in syncEnumTypes:
                enum_list = self.enumsInBitOrder[enumType]
                format_string = 'static const ' + enumType + ' {name} = 0x{mask:016X}ULL;'
                for enum_info in enum_list:
                    if (vvl_fake_extension not in enum_info['name']) :
                        continue
                    output += '\n'
                    output += format_string.format(**enum_info)
        return output

    def accessIndex(self):
        output = '\n'
        if self.headerFile:
            output += '// Unique number for each  stage/access combination\n'
            output += 'enum SyncStageAccessIndex {\n'
            for access in self.stageAccessCombo:
                output += '\t{} = {},\n'.format( access['stage_access'], access['index'])
            output += '};'
        return output

    def accessFlags(self):
        output = '\n'
        if self.headerFile:
            output += 'using SyncStageAccessFlags = std::bitset<128>;\n'
            output += '// Unique bit for each stage/access combination\n'
            for access in self.stageAccessCombo:
                if access['stage_access_bit'] is not None:
                    output += 'static const SyncStageAccessFlags {} = (SyncStageAccessFlags(1) << {});\n'.format(access['stage_access_bit'], access['stage_access'])

        return output


    def accessReadWriteMask(self):
        output = ''
        if self.headerFile:
            read_list = []
            write_list = []
            for e in self.stageAccessCombo:
                if e['is_read'] is None:
                    continue
                if e['is_read'] == 'true':
                    read_list.append(e['stage_access_bit'])
                else:
                    write_list.append(e['stage_access_bit'])

            output += '// Constants defining the mask of all read and write stage_access states\n'
            output += 'static const SyncStageAccessFlags syncStageAccessReadMask = ( //  Mask of all read StageAccess bits\n'
            for bit in read_list:
                output += '\t{}{}\n'.format(bit, ' |' if bit != read_list[-1] else '')
            output += ');'
            output += '\n\n'

            output += 'static const SyncStageAccessFlags syncStageAccessWriteMask = ( //  Mask of all write StageAccess bits\n'
            for bit in write_list:
                output += '\t{}{}\n'.format(bit, ' |' if bit != write_list[-1] else '')
            output += ');'

        return output

    def infoByStageAccessIndex(self):
        output = ''
        if self.headerFile:
            output += '''
struct SyncStageAccessInfoType {
    const char *name;
    VkPipelineStageFlags2 stage_mask;
    VkAccessFlags2 access_mask;
    SyncStageAccessIndex stage_access_index;
    SyncStageAccessFlags stage_access_bit;
};\n'''

            output += '// Array of text names and component masks for each stage/access index\n'
            output += 'const std::array<SyncStageAccessInfoType, {}>& syncStageAccessInfoByStageAccessIndex();\n'.format(len(self.stageAccessCombo))
        elif self.sourceFile:
            output += '// Array of text names and component masks for each stage/access index\n'
            output += 'const std::array<SyncStageAccessInfoType, {}>& syncStageAccessInfoByStageAccessIndex() {{\n'.format(len(self.stageAccessCombo))
            output += 'static const std::array<SyncStageAccessInfoType, {}> variable = {{ {{\n'.format(len(self.stageAccessCombo))
            for stageAccess in self.stageAccessCombo:
                output += '\t{\n'
                output += '\t\t{},\n'.format(stageAccess['stage_access_string'])
                output += '\t\t{},\n'.format(stageAccess['stage'])
                output += '\t\t{},\n'.format(stageAccess['access'])
                output += '\t\t{},\n'.format(stageAccess['stage_access'])
                bit = stageAccess['stage_access_bit'] if stageAccess['stage_access_bit'] is not None else 'SyncStageAccessFlags(0)'
                output += '\t\t{}\n'.format(bit)
                output += '\t},\n'
            output += '}};\n'
            output += 'return variable;\n'
            output += '}\n'
        return output


    def accessMaskByStageBit(self):
        output = ''
        if self.headerFile:
            output += '// Bit order mask of stage_access bit for each stage\n'
            output += 'const std::map<VkPipelineStageFlags2, SyncStageAccessFlags>& syncStageAccessMaskByStageBit();\n'
        elif self.sourceFile:
            output += ''
        return output

    def accessMaskByShaderStage(self):
        output = ''
        if self.headerFile:
            output += '''
struct SyncShaderStageAccess {
    SyncStageAccessIndex sampled_read;
    SyncStageAccessIndex storage_read;
    SyncStageAccessIndex storage_write;
    SyncStageAccessIndex uniform_read;
};
'''
            output += 'const std::map<VkShaderStageFlagBits, SyncShaderStageAccess>& syncStageAccessMaskByShaderStage();\n'
        elif self.sourceFile:
            output += ''
        return output


    def accessMaskByAccessBit(self):
        output = ''
        if self.headerFile:
            output += '// Bit order mask of stage_access bit for each access\n'
            output += 'const std::map<VkAccessFlags2, SyncStageAccessFlags>& syncStageAccessMaskByAccessBit();\n'
        elif self.sourceFile:
            output += ''
        return output


    def directStageToAccessMask(self):
        output = ''
        if self.headerFile:
            output += '// Direct VkPipelineStageFlags to valid VkAccessFlags lookup table\n'
            output += 'const std::map<VkPipelineStageFlags2, VkAccessFlags2>& syncDirectStageToAccessMask();\n'
        elif self.sourceFile:
            output += ''
        return output


    def allCommandStagesByQueueFlags(self):
        output = ''
        if self.headerFile:
            output += '// Pipeline stages corresponding to VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT for each VkQueueFlagBits\n'
            output += 'const std::map<VkQueueFlagBits, VkPipelineStageFlags2>& syncAllCommandStagesByQueueFlags();\n'
        elif self.sourceFile:
            output += ''
        return output


    def logicallyEarlierStages(self):
        output = ''
        if self.headerFile:
            output += '// Masks of logically earlier stage flags for a given stage flag\n'
            output += '\n'
        elif self.sourceFile:
            output += ''
        return output


    def logicallyLaterStages(self):
        output = ''
        if self.headerFile:
            output += '// Masks of logically later stage flags for a given stage flag\n'
            output += '\n'
        elif self.sourceFile:
            output += ''
        return output


    # Create the stage/access combination from the legal uses of access with stages
    def createStageAccessCombinations(self):
        index = 1
        enum_prefix = 'SYNC_'
        stage_accesses = []
        none_stage_access = enum_prefix + 'ACCESS_INDEX_NONE'
        stage_accesses.append({
                        'stage_access': none_stage_access,
                        'stage_access_string' : '"' + none_stage_access + '"',
                        'stage_access_bit': None,
                        'index': 0,
                        'stage': 'VK_PIPELINE_STAGE_2_NONE_KHR',
                        'access': 'VK_ACCESS_2_NONE_KHR',
                        'is_read': None}) #tri-state logic hack...
        for stage in self.allPipelineStageSet:
            mini_stage = stage.lstrip()
            if mini_stage.startswith(enum_prefix):
                mini_stage = mini_stage.replace(enum_prefix,'')
            else:
                mini_stage = mini_stage.replace('VK_PIPELINE_STAGE_2_', '')
            mini_stage = mini_stage.replace('_BIT_KHR', '')
            mini_stage = mini_stage.replace('_BIT', '')

            # Because access_stage_table's elements order might be different sometimes.
            # It causes the generator creates different code. It needs to be sorted.
            self.stageToAccessMap[stage].sort();
            for access in self.stageToAccessMap[stage]:
                mini_access = access.replace('VK_ACCESS_2_', '').replace('_BIT_KHR', '')
                mini_access = mini_access.replace('_BIT', '')
                stage_access = '_'.join((mini_stage,mini_access))
                stage_access = enum_prefix + stage_access
                stage_access_bit = BitSuffixed(stage_access)
                is_read = stage_access.endswith('_READ') or ( '_READ_' in stage_access)
                stage_accesses.append({
                        'stage_access': stage_access,
                        'stage_access_string' : '"' + stage_access + '"',
                        'stage_access_bit': stage_access_bit,
                        'index': index,
                        'stage': stage,
                        'access': access,
                        'is_read': 'true' if is_read else 'false' })
                index += 1

        # Add synthetic stage/access
        synth_stage_access = [ 'IMAGE_LAYOUT_TRANSITION', 'QUEUE_FAMILY_OWNERSHIP_TRANSFER']
        stage = 'VK_PIPELINE_STAGE_2_NONE_KHR'
        access = 'VK_ACCESS_2_NONE_KHR'

        for synth in synth_stage_access :
            stage_access = enum_prefix + synth
            stage_access_bit = BitSuffixed(stage_access)
            is_read = False # both ILT and QFO are R/W operations
            stage_accesses.append({
                        'stage_access': stage_access,
                        'stage_access_string' : '"' + stage_access + '"',
                        'stage_access_bit': stage_access_bit,
                        'index': index,
                        'stage': stage,
                        'access': access,
                        'is_read': 'true' if is_read else 'false' })
            index += 1

        return stage_accesses