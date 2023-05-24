#!/usr/bin/python3 -i
#
# Copyright (c) 2023 The Khronos Group Inc.
# Copyright (c) 2023 LunarG, Inc.
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

vvl_fake_extension = '_SYNCVAL'
present_stage = 'VK_PIPELINE_STAGE_2_PRESENT_ENGINE_BIT' + vvl_fake_extension
presenting_access = 'VK_ACCESS_2_PRESENT_ACQUIRE_READ_BIT' + vvl_fake_extension  # Treated as read
presented_access = 'VK_ACCESS_2_PRESENT_PRESENTED_BIT' + vvl_fake_extension  # Treated as write
present_engine_accesses = (presenting_access, presented_access)

syncEnumStageType = 'VkPipelineStageFlagBits2'
syncEnumAccessType = 'VkAccessFlagBits2'
syncEnumTypes = (syncEnumStageType, syncEnumAccessType)

separator = ' |\n        '

# Maps the 'queuetype' in XML to VkQueueFlagBits
queueTypeToQueueFlags = {
    'transfer' : 'VK_QUEUE_TRANSFER_BIT',
    'graphics' : 'VK_QUEUE_GRAPHICS_BIT',
    'compute' : 'VK_QUEUE_COMPUTE_BIT',
    'decode' : 'VK_QUEUE_VIDEO_DECODE_BIT_KHR',
    'encode'  : 'VK_QUEUE_VIDEO_ENCODE_BIT_KHR',
    'opticalflow' : 'VK_QUEUE_OPTICAL_FLOW_BIT_NV',
    'sparse_binding' : 'VK_QUEUE_SPARSE_BINDING_BIT',
}

transferExpansion = [
    ('transfer copy', 'VK_PIPELINE_STAGE_2_COPY_BIT'),
    ('transfer_resolve', 'VK_PIPELINE_STAGE_2_RESOLVE_BIT'),
    ('transfer_blit', 'VK_PIPELINE_STAGE_2_BLIT_BIT'),
    ('transfer_clear', 'VK_PIPELINE_STAGE_2_CLEAR_BIT'),
    ('transfer_acceleration_structure_copy', 'VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_COPY_BIT_KHR'),
]

multiStages = [
    'VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT',
    'VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT',
    'VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT',
    'VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT',
]

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

def SortSetBasedOnOrder(stage_set, stage_order):
    return [ stage for stage in stage_order if stage in stage_set]

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

        # List of all stages sorted according to enum numeric value.
        # This does not include stages similar to ALL_GRAPHICS that represent multiple stages
        self.stages = []

        # List of stages obtained from merge-sorting ordered stages from each pipeline type.
        # This defines how the stages are are ordered in the ealiest/latest stage bitmask
        self.logicallyOrderedStages = []

        # pipeline names from <syncpipeline>
        self.pipelineNames = []

        # < pipeline_name, [pipeline stages in logical order (exactly as defined in XML)] >
        self.pipelineStages = dict()

        # < pipeline_name, [{ stage : 'stage', ordered: True/False, after : [stages], before : [stages] }]  >
        # Each stage includes ordering info but also the stages itself are ordered according to 
        # order/before/after directives. So, if you need iterate over stages from specific pipeline type
        # according to all ordering constrains just iterate over the list as asual.
        self.pipelineStagesOrdered = dict()

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
        self.stageAccessCombo = []

    #
    # Called at beginning of processing as file is opened
    def beginFile(self, genOpts):
        OutputGenerator.beginFile(self, genOpts)
        self.headerFile = genOpts.filename.endswith('.h')
        self.sourceFile = not self.headerFile

        # File Comment
        file_comment = '// *** THIS FILE IS GENERATED - DO NOT EDIT ***\n'
        file_comment += '// See sync_validation_generator.py for modifications\n'
        write(file_comment, file=self.outFile)
        # Copyright Statement
        copyright = ''
        copyright += '\n'
        copyright += '/***************************************************************************\n'
        copyright += ' *\n'
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
            write('#pragma once\n', file=self.outFile)
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
        self.stageToAccessMap[present_stage] = [presenting_access, presented_access]

        self.enumsInBitOrder = self.getEnumsInBitOrder()
        self.logicallyOrderedStages = self.getStagesInLogicalOrder()

        self.stages.append(present_stage)
        # sort self.stages based on VkPipelineStageFlagBits2 bit order
        sort_order = {stage_info['name'] : index for index, stage_info in enumerate(self.enumsInBitOrder['VkPipelineStageFlagBits2'])}
        sort_order['VK_PIPELINE_STAGE_2_NONE'] = -1
        self.stages.sort(key=lambda stage: sort_order[stage])

        self.stageAccessCombo = self.createStageAccessCombinations()

        write(self.defines(), file=self.outFile)
        write(self.accessIndex(), file=self.outFile)
        write(self.accessFlags(), file=self.outFile)
        write(self.infoByStageAccessIndex(), file=self.outFile)
        write(self.accessReadWriteMask(), file=self.outFile)
        write(self.stageAccessMaskByStage(), file=self.outFile)
        write(self.stageAccessMaskByAccess(), file=self.outFile)
        write(self.accessMaskByStage(), file=self.outFile)
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

        if syncEquivalent is None and name != 'VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT':
            self.stages.append(name)

    # Gets all <syncaccess>
    def genSyncAccess(self, sync):
        name = sync.elem.get('name')

        syncEquivalent = sync.elem.find('syncequivalent')
        if syncEquivalent is not None:
            self.accessEquivalent[name] = syncEquivalent.get('access').split(',')
            # In general, syncval algorithms work only with 'base' accesses
            # and skip aliases/multi-accesses, or expands multi-accesses when necessary
            return

        syncSupport = sync.elem.find('syncsupport')
        if syncSupport is not None:
            supportStages = syncSupport.get('stage').split(',')
            self.accessToStageMap[name] = supportStages
            for stage in supportStages:
                self.stageToAccessMap[stage].append(name)

    # Gets all <syncpipeline>
    def genSyncPipeline(self, sync):
        name = sync.elem.get('name').replace(' ', '_')

        # special case for trasfer stage: expand it to primitive transfer operations
        if name == 'transfer':
            for transfer_pipeline_name, transfer_stage in transferExpansion:
                self.pipelineNames.append(transfer_pipeline_name)
                self.pipelineStages[transfer_pipeline_name] = []
                self.pipelineStagesOrdered[transfer_pipeline_name] = []
                for elem in sync.elem.findall('syncpipelinestage'):
                    stage = elem.text
                    if stage == 'VK_PIPELINE_STAGE_2_TRANSFER_BIT':
                        stage = transfer_stage
                    self.pipelineStages[transfer_pipeline_name].append(stage)
                    self.pipelineStagesOrdered[transfer_pipeline_name].append({'stage' : stage, 'ordered' : True, 'before' : None, 'after' : None })
        # regular case (non-expandable stages)
        else:
            self.pipelineNames.append(name)
            self.pipelineStages[name] = []
            self.pipelineStagesOrdered[name] = []
            before_list = []
            after_list = []
            unordered_list = []
            for elem in sync.elem.findall('syncpipelinestage'):
                stage = elem.text
                self.pipelineStages[name].append(stage)
                order = elem.get('order')
                before = elem.get('before')
                after = elem.get('after')
                stage_order = {'stage' : stage, 'ordered' : order != 'None', 'before' : None, 'after' : None }
                if before is not None:
                    stage_order['before'] = before
                    before_list.append(stage_order)
                elif after is not None:
                    stage_order['after'] = after
                    after_list.append(stage_order)
                elif order == 'None':
                    unordered_list.append(stage_order)
                else:
                    self.pipelineStagesOrdered[name].append(stage_order)

            # process ordering directives
            for stage_order in before_list:
                before_stage = stage_order['before']
                before_index = next((i for i, s in enumerate(self.pipelineStagesOrdered[name]) if s['stage'] == before_stage))
                self.pipelineStagesOrdered[name].insert(before_index, stage_order)

            for stage_order in after_list:
                after_stage = stage_order['after']
                after_index = next((i for i, s in enumerate(self.pipelineStagesOrdered[name]) if s['stage'] == after_stage))
                self.pipelineStagesOrdered[name].insert(after_index + 1, stage_order)

            for stage_order in unordered_list:
                self.pipelineStagesOrdered[name].append(stage_order)


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

    def getStagesInLogicalOrder(self):
        logical_order = ['VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT']
        for pipeline_name in self.pipelineNames:
            for index, stage_info in enumerate(self.pipelineStagesOrdered[pipeline_name]):
                stage = stage_info['stage']
                if stage not in logical_order:
                    later_stages =[s['stage'] for s in self.pipelineStagesOrdered[pipeline_name][index+1:]]
                    insert_loc = len(logical_order)
                    while insert_loc > 0:
                        if any(s in logical_order[:insert_loc] for s in later_stages):
                            insert_loc -= 1
                        else:
                            break
                    logical_order.insert(insert_loc, stage)
        logical_order.append('VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT')
        return logical_order

    #
    # Create defines that are used either by other files (headerFile) or just internally (sourceFile)
    def defines(self):
        output = ''
        if self.headerFile:
            output = '\n'
            output += '// Fake stages and accesses for acquire present support'
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
        output = ''
        if self.headerFile:
            output = '\n'
            output += '// Unique number for each  stage/access combination\n'
            output += 'enum SyncStageAccessIndex {\n'
            for access in self.stageAccessCombo:
                output += '    {} = {},\n'.format( access['stage_access'], access['index'])
            output += '};'
        return output

    def accessFlags(self):
        output = ''
        if self.headerFile:
            output = '\n'
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
                output += '    {}{}\n'.format(bit, ' |' if bit != read_list[-1] else '')
            output += ');'
            output += '\n\n'

            output += 'static const SyncStageAccessFlags syncStageAccessWriteMask = ( //  Mask of all write StageAccess bits\n'
            for bit in write_list:
                output += '    {}{}\n'.format(bit, ' |' if bit != write_list[-1] else '')
            output += ');\n'

        return output

    def infoByStageAccessIndex(self):
        output = ''
        if self.headerFile:
            output += 'struct SyncStageAccessInfoType {\n'
            output += '    const char *name;\n'
            output += '    VkPipelineStageFlags2 stage_mask;\n'
            output += '    VkAccessFlags2 access_mask;\n'
            output += '    SyncStageAccessIndex stage_access_index;\n'
            output += '    SyncStageAccessFlags stage_access_bit;\n'
            output += '};\n\n'
            output += '// Array of text names and component masks for each stage/access index\n'
            output += 'const std::array<SyncStageAccessInfoType, {}>& syncStageAccessInfoByStageAccessIndex();\n'.format(len(self.stageAccessCombo))
        elif self.sourceFile:
            output += 'const std::array<SyncStageAccessInfoType, {}>& syncStageAccessInfoByStageAccessIndex() {{\n'.format(len(self.stageAccessCombo))
            output += 'static const std::array<SyncStageAccessInfoType, {}> variable = {{ {{\n'.format(len(self.stageAccessCombo))
            for stageAccess in self.stageAccessCombo:
                output += '    {\n'
                output += '        {},\n'.format(stageAccess['stage_access_string'])
                output += '        {},\n'.format(stageAccess['stage'])
                output += '        {},\n'.format(stageAccess['access'])
                output += '        {},\n'.format(stageAccess['stage_access'])
                bit = stageAccess['stage_access_bit'] if stageAccess['stage_access_bit'] is not None else 'SyncStageAccessFlags(0)'
                output += '        {}\n'.format(bit)
                output += '    },\n'
            output += '}};\n'
            output += 'return variable;\n'
            output += '}\n'
        return output


    def stageAccessMaskByStage(self):
        map_type = 'const std::map<VkPipelineStageFlags2, SyncStageAccessFlags>'
        func_name = 'syncStageAccessMaskByStageBit'
        output = ''
        if self.headerFile:
            output += '// Bit order mask of stage_access bit for each stage\n'
            output += f'{map_type}& {func_name}();\n'
        elif self.sourceFile:
            output += f'{map_type}& {func_name}() {{\n'
            output += f'    static {map_type} variable = {{\n'
            stage_to_stageAccess = {}
            for stageAccess_info in self.stageAccessCombo:
                stage = stageAccess_info['stage']
                if stage == 'VK_PIPELINE_STAGE_2_NONE_KHR': continue
                stageAccess_bit = stageAccess_info['stage_access_bit']
                stage_to_stageAccess[stage] = stage_to_stageAccess.get(stage, []) + [stageAccess_bit]
            stages_in_bit_order = [e['name'] for e in self.enumsInBitOrder['VkPipelineStageFlagBits2']]
            for stage in stages_in_bit_order:
                if stage in stage_to_stageAccess:
                    output += f'    {{ {stage}, (\n        {separator.join(stage_to_stageAccess[stage])}\n    )}},\n'
            output += '    };\n'
            output += '    return variable;\n'
            output += '}\n\n'
        return output


    def stageAccessMaskByAccess(self):
        map_type = 'const std::map<VkAccessFlags2, SyncStageAccessFlags>'
        func_name = 'syncStageAccessMaskByAccessBit'
        output = ''
        if self.headerFile:
            output += '// Bit order mask of stage_access bit for each access\n'
            output += f'{map_type}& {func_name}();\n'
        elif self.sourceFile:
            output += f'{map_type}& {func_name}() {{\n'
            output += f'    static {map_type} variable = {{\n'
            access_to_stageAccess = {}
            for stageAccess_info in self.stageAccessCombo:
                access = stageAccess_info['access']
                if access == 'VK_ACCESS_2_FLAG_NONE_KHR': continue
                stageAccess_bit = stageAccess_info['stage_access_bit']
                access_to_stageAccess[access] = access_to_stageAccess.get(access, []) + [stageAccess_bit]
            accesses_in_bit_order = [e['name'] for e in self.enumsInBitOrder['VkAccessFlagBits2']]
            for access in accesses_in_bit_order:
                if access in access_to_stageAccess:
                    output += f'    {{ {access}, (\n        {separator.join(access_to_stageAccess[access])}\n    )}},\n'
            output += '    { VK_ACCESS_2_MEMORY_READ_BIT, (\n        syncStageAccessReadMask\n    )},\n'
            output += '    { VK_ACCESS_2_MEMORY_WRITE_BIT, (\n        syncStageAccessWriteMask\n    )},\n'
            output += '    };\n'
            output += '    return variable;\n'
            output += '}\n\n'
        return output


    def accessMaskByStage(self):
        map_type = 'const std::map<VkPipelineStageFlags2, VkAccessFlags2>'
        func_name = 'syncDirectStageToAccessMask'
        output = ''
        if self.headerFile:
            output += '// Direct VkPipelineStageFlags to valid VkAccessFlags lookup table\n'
            output += f'{map_type}& {func_name}();\n'
        elif self.sourceFile:
            output += f'{map_type}& {func_name}() {{\n'
            output += f'    static {map_type} variable = {{\n'
            stage_to_access = {}
            for stageAccess_info in self.stageAccessCombo:
                stage = stageAccess_info['stage']
                if stage == 'VK_PIPELINE_STAGE_2_NONE_KHR': continue
                stage_to_access[stage] = stage_to_access.get(stage, []) + [stageAccess_info['access']]
            stages_in_bit_order = [e['name'] for e in self.enumsInBitOrder['VkPipelineStageFlagBits2']]
            for stage in stages_in_bit_order:
                if stage in stage_to_access:
                    output += f'    {{ {stage}, (\n        {separator.join(stage_to_access[stage])}\n    )}},\n'
            output += '    };\n'
            output += '    return variable;\n'
            output += '}\n\n'
        return output


    def allCommandStagesByQueueFlags(self):
        map_type = 'const std::map<VkQueueFlagBits, VkPipelineStageFlags2>'
        func_name = 'syncAllCommandStagesByQueueFlags'
        output = ''
        if self.headerFile:
            output += '// Pipeline stages corresponding to VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT for each VkQueueFlagBits\n'
            output += f'{map_type}& {func_name}();\n'
        elif self.sourceFile:
            output += f'{map_type}& {func_name}() {{\n'
            output += f'    static {map_type} variable = {{\n'
            queue_caps = []
            queue_cap_to_stages = {}
            for queue_name, stages in self.queueToStages.items():
                if queue_name == 'sparse_binding': continue
                cap_flag = queueTypeToQueueFlags[queue_name]
                queue_caps.append(cap_flag)
                queue_cap_to_stages[cap_flag] = []
                for stage in self.stages:
                    if stage in stages and stage not in multiStages and stage != 'VK_PIPELINE_STAGE_2_NONE':
                        queue_cap_to_stages[cap_flag].append(stage)
            queue_caps.sort()
            for cap_flag in queue_caps:
                output += f'    {{ {cap_flag}, (\n        {separator.join(queue_cap_to_stages[cap_flag])}\n    )}},\n'
            output += '    };\n'
            output += '    return variable;\n'
            output += '}\n\n'
        return output


    def logicallyEarlierStages(self):
        map_type = 'const std::map<VkPipelineStageFlags2, VkPipelineStageFlags2>'
        func_name = 'syncLogicallyEarlierStages'
        output = ''
        if self.headerFile:
            output += '// Masks of logically earlier stage flags for a given stage flag\n'
            output += f'{map_type}& {func_name}();\n'
        elif self.sourceFile:
            output += f'{map_type}& {func_name}() {{\n'
            output += f'    static {map_type} variable = {{\n'

            earlier_stages = {}
            earlier_stages['VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT'] = set(['VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT'])
            earlier_stages['VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT'] = set(['VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT', 'VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT'])

            for stages in self.pipelineStagesOrdered.values():
                for i in range(len(stages)):
                    stage_order = stages[i]
                    stage = stage_order['stage']
                    if stage == 'VK_PIPELINE_STAGE_2_HOST_BIT':
                        continue
                    if stage not in earlier_stages:
                        earlier_stages[stage] = set(['VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT'])
                        earlier_stages['VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT'].update([stage])

                    if not stage_order['ordered']:
                        earlier_stages[stage] = set(['VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT'])
                    else:
                        earlier_stages[stage].update([s['stage'] for s in stages[:i]])

            earlier_stages = { key:SortSetBasedOnOrder(values, self.logicallyOrderedStages) for key, values in earlier_stages.items() }

            for stage in self.stages:
                if stage in earlier_stages and len(earlier_stages[stage]) > 0:
                    output += f'    {{ {stage}, (\n        {separator.join(earlier_stages[stage])}\n    )}},\n'

            output += '    };\n'
            output += '    return variable;\n'
            output += '}\n\n'
        return output


    def logicallyLaterStages(self):
        map_type = 'const std::map<VkPipelineStageFlags2, VkPipelineStageFlags2>'
        func_name = 'syncLogicallyLaterStages'
        output = ''
        if self.headerFile:
            output += '// Masks of logically later stage flags for a given stage flag\n'
            output += f'{map_type}& {func_name}();\n'
        elif self.sourceFile:
            output += f'{map_type}& {func_name}() {{\n'
            output += f'    static {map_type} variable = {{\n'

            later_stages = {}
            later_stages['VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT'] = set(['VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT'])
            later_stages['VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT'] = set(['VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT', 'VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT'])

            for stages in self.pipelineStagesOrdered.values():
                for i in range(len(stages)):
                    stage_order = stages[i]
                    stage = stage_order['stage']
                    if stage == 'VK_PIPELINE_STAGE_2_HOST_BIT':
                        continue
                    if stage not in later_stages:
                        later_stages[stage] = set(['VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT'])
                        later_stages['VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT'].update([stage])

                    if not stage_order['ordered']:
                        later_stages[stage] = set(['VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT'])
                    else:
                        later_stages[stage].update([s['stage'] for s in stages[i+1:] if s['ordered']])

            later_stages = { key:SortSetBasedOnOrder(values, self.logicallyOrderedStages) for key, values in later_stages.items() }

            for stage in self.stages:
                if stage in later_stages and len(later_stages[stage]) > 0:
                    output += f'    {{ {stage}, (\n        {separator.join(later_stages[stage])}\n    )}},\n'

            output += '    };\n'
            output += '    return variable;\n'
            output += '}\n\n'
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
        for stage in self.stages:
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
