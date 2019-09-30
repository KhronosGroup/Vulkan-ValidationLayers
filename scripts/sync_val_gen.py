#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2019 The Khronos Group Inc.
# Copyright (c) 2015-2019 Valve Corporation
# Copyright (c) 2015-2019 LunarG, Inc.
# Copyright (c) 2015-2019 Google Inc.
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
# Author: John Zulauf <jzulauf@lunarg.com>
debug_table_parse = False
debug_in_bit_order = False
debug_top_level = False

# Snipped from chapters/synchronization.txt -- tage v1.1.123
snippet_access_types_supported = '''
[[synchronization-access-types-supported]]
.Supported access types
[cols="50,50",options="header"]
|====
|Access flag                                                  | Supported pipeline stages
|ename:VK_ACCESS_INDIRECT_COMMAND_READ_BIT                    | ename:VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT
|ename:VK_ACCESS_INDEX_READ_BIT                               | ename:VK_PIPELINE_STAGE_VERTEX_INPUT_BIT
|ename:VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT                    | ename:VK_PIPELINE_STAGE_VERTEX_INPUT_BIT

|ename:VK_ACCESS_UNIFORM_READ_BIT                             |
ifdef::VK_NV_mesh_shader[]
                                                               ename:VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV, ename:VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV,
endif::VK_NV_mesh_shader[]
ifdef::VK_NV_ray_tracing[]
                                                               ename:VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV,
endif::VK_NV_ray_tracing[]
                                                               ename:VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, ename:VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT, ename:VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT, ename:VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT, ename:VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, or ename:VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT

|ename:VK_ACCESS_SHADER_READ_BIT                              |
ifdef::VK_NV_mesh_shader[]
                                                               ename:VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV, ename:VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV,
endif::VK_NV_mesh_shader[]
ifdef::VK_NV_ray_tracing[]
                                                               ename:VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV,
endif::VK_NV_ray_tracing[]
                                                               ename:VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, ename:VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT, ename:VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT, ename:VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT, ename:VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, or ename:VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT

|ename:VK_ACCESS_SHADER_WRITE_BIT                             |
ifdef::VK_NV_mesh_shader[]
                                                               ename:VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV, ename:VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV,
endif::VK_NV_mesh_shader[]
ifdef::VK_NV_ray_tracing[]
                                                               ename:VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV,
endif::VK_NV_ray_tracing[]
                                                               ename:VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, ename:VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT, ename:VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT, ename:VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT, ename:VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, or ename:VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT

|ename:VK_ACCESS_INPUT_ATTACHMENT_READ_BIT                    | ename:VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
|ename:VK_ACCESS_COLOR_ATTACHMENT_READ_BIT                    | ename:VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
|ename:VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT                   | ename:VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
|ename:VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT            | ename:VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, or ename:VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT
|ename:VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT           | ename:VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, or ename:VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT
|ename:VK_ACCESS_TRANSFER_READ_BIT                            | ename:VK_PIPELINE_STAGE_TRANSFER_BIT
|ename:VK_ACCESS_TRANSFER_WRITE_BIT                           | ename:VK_PIPELINE_STAGE_TRANSFER_BIT
|ename:VK_ACCESS_HOST_READ_BIT                                | ename:VK_PIPELINE_STAGE_HOST_BIT
|ename:VK_ACCESS_HOST_WRITE_BIT                               | ename:VK_PIPELINE_STAGE_HOST_BIT
|ename:VK_ACCESS_MEMORY_READ_BIT                              | N/A
|ename:VK_ACCESS_MEMORY_WRITE_BIT                             | N/A
ifdef::VK_EXT_blend_operation_advanced[]
|ename:VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT    | ename:VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
endif::VK_EXT_blend_operation_advanced[]
ifdef::VK_NVX_device_generated_commands[]
|ename:VK_ACCESS_COMMAND_PROCESS_READ_BIT_NVX                 | ename:VK_PIPELINE_STAGE_COMMAND_PROCESS_BIT_NVX
|ename:VK_ACCESS_COMMAND_PROCESS_WRITE_BIT_NVX                | ename:VK_PIPELINE_STAGE_COMMAND_PROCESS_BIT_NVX
endif::VK_NVX_device_generated_commands[]
ifdef::VK_EXT_conditional_rendering[]
|ename:VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT           | ename:VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT
endif::VK_EXT_conditional_rendering[]
ifdef::VK_NV_shading_rate_image[]
|ename:VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV               | ename:VK_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV
endif::VK_NV_shading_rate_image[]
ifdef::VK_EXT_transform_feedback[]
|ename:VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT             | ename:VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT
|ename:VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT     | ename:VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT
|ename:VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT      | ename:VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT
endif::VK_EXT_transform_feedback[]
ifdef::VK_NV_ray_tracing[]
|ename:VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV           | ename:VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV, or ename:VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV
|ename:VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV          | ename:VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV
endif::VK_NV_ray_tracing[]
ifdef::VK_EXT_fragment_density_map[]
|ename:VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT            | ename:VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT
endif::VK_EXT_fragment_density_map[]
|====
'''

# use simplest filtering to assure completest list
def ParseAccessType(table_text):
    preproc = ''
    access_stage_table = {}
    skip = False
    outside_table = True
    for line in table_text.split('\n'):
        if not line.strip():
            continue
        elif outside_table:
            if line.startswith('|===='):
                outside_table = False
            continue
        elif line.startswith('ifndef'):
            # Negative preproc filter
            preproc = line
            skip = True # for now just ignore subset filtering
        elif line.startswith('ifdef'):
            # Positive preproc filter
            preproc = line
            skip = False # No support for nested directives...
        elif line.startswith('endif'):
            # Positive preproc filter
            skip = False
        elif (line.startswith('|ename:') or line.startswith('        ')) and not skip:
            if debug_table_parse:
                print("// line {}".format(line))
            cols = line.split('|')
            if debug_table_parse:
                print("len(cols)", len(cols), cols)
            if len(cols) == 3:
                stage_column = cols[2]
                access_enum = cols[1].split(':')[1].strip()
                access_stage_table[access_enum]=[]
            else:
                stage_column = cols[0].strip()
            if debug_table_parse:
                print("stage_column:", stage_column)
            if stage_column.startswith(' N/A'):
                continue
            stage_column = stage_column.replace(', ',',')
            stage_column = stage_column.rstrip(',')
            stages = stage_column.split(',')
            if debug_table_parse:
                print("stages", len(stages), stages)
            if len(stages) < 1:
                continue
            elif not stages[0]:
                continue
            stages_lens = [len(s.split(':')) for s in stages]
            stage_enums = [ s.split(':')[1].strip('or ') for s in stages]

            access_stage_table[access_enum] += stage_enums
            if(debug_table_parse):
                print("// access_stage_table[{}]: {}".format(access_enum, "|".join(access_stage_table[access_enum])))

    return access_stage_table

def CreateStageAccessTable(stage_order, access_stage_table):
    stage_access_table = { stage: list() for stage in stage_order}
    for access, stages in access_stage_table.items():
        for stage in stages:
            stage_access_table[stage].append(access)
    
    return stage_access_table


# Note: there should be a machine readable merged order, but there isn't... so we'll need some built-in consistency checks
# Pipeline stages in rough order, merge sorted.  When
# Legal stages are masked in, remaining stages are ordered
pipeline_order = '''
VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT
VK_PIPELINE_STAGE_VERTEX_INPUT_BIT
VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT
VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT
VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT
VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT
VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV
VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV
VK_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV
VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT
VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
VK_PIPELINE_STAGE_TRANSFER_BIT
VK_PIPELINE_STAGE_COMMAND_PROCESS_BIT_NVX
VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
VK_PIPELINE_STAGE_HOST_BIT
VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT
VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV
VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV
'''

sync_enum_types = ('VkPipelineStageFlagBits', 'VkAccessFlagBits' )

def InBitOrder(tag, enum_elem):
    # The input may be unordered or sparse w.r.t. the mask field, sort and gap fill
    found = []
    for elem in enum_elem:
        bitpos = elem.get('bitpos')
        name = elem.get('name')
        if not bitpos:
            continue
    
        if name.endswith("MAX_ENUM"):
            break

        found.append({'name': name, 'bitpos': int(bitpos)})

    in_bit_order = []
    for entry in sorted(found, key=lambda record: record['bitpos']):
        if debug_in_bit_order:
            print ("adding ", {'name': entry['name'], 'mask': (1 << entry['bitpos'])})
        bitpos = entry['bitpos']
        in_bit_order.append({'name': entry['name'], 'mask': (1 << bitpos), 'bitpos': bitpos})

    return in_bit_order

def BitSuffixed(name):
    alt_bit = ('_ANDROID', '_EXT', '_IMG', '_KHR', '_NV', '_NVX')
    bit_suf = name + '_BIT'
    for alt in alt_bit:
        if name.endswith(alt) :
            bit_suf = name.replace(alt, '_BIT' + alt)
            break
    return bit_suf;

 # Create the stage/access combination from the legal uses of access with stages
def CreateStageAccessCombinations(config, stage_order, stage_access_types):
    index = 0;
    enum_prefix = config['enum_prefix']
    stage_accesses = []
    for stage in stage_order:
        mini_stage = stage.lstrip()
        if mini_stage.startswith(enum_prefix):
            mini_stage = mini_stage.replace(enum_prefix,"")
        else: 
            mini_stage = mini_stage.replace("VK_PIPELINE_STAGE_", "")
        mini_stage = mini_stage.replace("_BIT", "")

        for access in stage_access_types[stage]:
            mini_access = access.replace("VK_ACCESS_", "").replace("_BIT", "")
            stage_access = "_".join((mini_stage,mini_access))
            stage_access = enum_prefix + stage_access
            stage_access_bit = BitSuffixed(stage_access)
            is_read = stage_access.endswith('_READ') or ( '_READ_' in stage_access)
            stage_accesses.append({
                    'stage_access': stage_access,
                    'stage_access_string' : '"' + stage_access + '"',
                    'stage_access_bit': stage_access_bit,
                    'index': index,
                    'stage': stage,
                    'mini_stage': mini_stage,
                    'access': access,
                    'is_read': 'true' if is_read else 'false',
                    'mini_access': mini_access })
            index += 1

    return stage_accesses

def StageAccessEnums(stage_accesses, config):
    type_prefix = config['type_prefix']
    var_prefix = config['var_prefix']
    sync_mask_name = config['sync_mask_name']
    indent = config['indent']
    # The stage/access combinations in ordinal order
    output = []
    ordinal_name = config['ordinal_name']
    output.append('// Unique number for each  stage/access combination')
    output.append('enum {} {{'.format(ordinal_name))
    output.extend([ '{}{} = {},'.format(indent, e['stage_access'], e['index'])  for e in stage_accesses])
    output.append('};')
    output.append('')

    # The stage/access combinations as bit mask
    bits_name = config['bits_name']
    output.append('// Unique bit for each  stage/access combination')
    output.append('enum {} : {} {{'.format(bits_name, sync_mask_name))
    output.extend([ '{}{} = {}(1) << {},'.format(indent, e['stage_access_bit'], sync_mask_name, e['stage_access'])  for e in stage_accesses])
    output.append('};')
    output.append('')

    map_name = var_prefix + 'StageAccessIndexByStageAccessBit' 
    output.append('// Map of the StageAccessIndices from the StageAccess Bit')
    output.append('static std::map<{}, {}> {}  = {{'.format(sync_mask_name, ordinal_name, map_name))
    output.extend([ '{}{{ {}, {} }},'.format(indent, e['stage_access_bit'], e['stage_access'])  for e in stage_accesses])
    output.append('};')
    output.append('')

    # stage/access debug information based on ordinal enum
    sa_info_type = '{}StageAccessInfoType'.format(type_prefix)
    output.append('struct {} {{'.format(sa_info_type))
    output.append('{}const char *name;'.format(indent))
    output.append('{}VkPipelineStageFlagBits stage_mask;'.format(indent))
    output.append('{}VkAccessFlagBits access_mask;'.format(indent))
    output.append('{}{} stage_access_index;'.format(indent, ordinal_name))
    output.append('{}{} stage_access_bit;'.format(indent, bits_name))
    output.append('};\n')

    sa_info_var = '{}StageAccessInfoByStageAccessIndex'.format(config['var_prefix'])
    output.append('// Array of text names and component masks for each stage/access index')
    output.append('static std::array<{}, {}> {} = {{ {{'.format(sa_info_type, len(stage_accesses), sa_info_var))
    fields_format ='{tab}{tab}{}'
    fields = ['stage_access_string', 'stage', 'access', 'stage_access', 'stage_access_bit']
    for entry in stage_accesses:
        output.append(indent+'{')
        output.append (',\n'.join([fields_format.format(entry[field], tab=indent)  for field in fields]))
        output.append('\n' + indent +'},')
    output.append('} };')
    output.append('')

    return output

def CrossReferenceTable(table_name, table_desc, key_type, key_vec_base, mask_map, config):
    mask_type = config['sync_mask_name']
    indent = config['indent']
    key_vec =  [ e['name'] for e in key_vec_base]

    table = ['// ' + table_desc]
    table.append('static std::map<{}, {}> {} = {{'.format(key_type, mask_type, config['var_prefix'] + table_name))

    for mask_key in key_vec:
        mask_vec = mask_map[mask_key]
        if len(mask_vec) == 0:
            continue

        if len(mask_vec) > 1:
            sep = ' |\n' + indent * 2
            table.append( '{tab}{{ {}, (\n{tab}{tab}{}\n{tab})}},'.format(mask_key, sep.join(mask_vec), tab=indent))
        else:
            table.append( '{}{{ {}, {}}},'.format(indent, mask_key, mask_vec[0]))
    table.append('};')
    table.append('')

    return table

def DoubleCrossReferenceTable(table_name, table_desc, stage_keys, access_keys, stage_access_stage_access_map, config):
    indent = config['indent']
    ordinal_name = config['ordinal_name']

    table = ['// ' + table_desc]
    table.append('static std::map<{vk_stage_flags}, std::map<{vk_access_flags}, {ordinal_name}>> {var_prefix}{} = {{'.format(table_name, **config))
    sep = ' },\n' + indent * 2 + '{ '

    for stage, access_map in stage_access_stage_access_map.items():
        if len(access_map.keys()) == 0: continue
        accesses = [ '{}, {}'.format(access, index) for access, index in access_map.items() ]
        entry_format = '{tab}{{ {key}, {{\n{tab}{tab}{{ {val} }}\n{tab}}} }},'
        table.append( entry_format.format(key=stage, val=sep.join(accesses), tab=indent))
    table.append('};')
    table.append('')

    return table

def StageAccessCrossReference(sync_enum, stage_access_combinations, config):
    output = []
    # Setup the cross reference tables
    enum_in_bit_order = dict()
    for enum_type in sync_enum_types:
        enum_in_bit_order[enum_type] = InBitOrder(enum_type, sync_enum[enum_type])

    stages_in_bit_order =  enum_in_bit_order['VkPipelineStageFlagBits']
    access_in_bit_order =  enum_in_bit_order['VkAccessFlagBits']
    stage_access_mask_stage_map = { e['name']: [] for e in stages_in_bit_order }
    #stage_access_mask_stage_map[none_stage] = [] # Support for N/A
    stage_access_mask_access_map = { e['name']: [] for e in access_in_bit_order }
    stage_access_stage_access_map = {  e['name']: dict() for e in stages_in_bit_order }

    for stage_access_combo in stage_access_combinations:
        combo_bit = stage_access_combo['stage_access_bit']
        stage = stage_access_combo['stage']
        access = stage_access_combo['access']
        stage_access_mask_stage_map[stage].append(combo_bit)
        stage_access_mask_access_map[access].append(combo_bit)
        stage_access_stage_access_map[stage][access] = stage_access_combo['stage_access']

    # sas: stage_access masks by stage used to build up SyncMaskTypes from VkPipelineStageFlagBits
    sas_desc = 'Bit order mask of stage_access bit for each stage'
    sas_name = 'StageAccessMaskByStageBit'
    output.extend(CrossReferenceTable(sas_name, sas_desc, 'VkPipelineStageFlags', stages_in_bit_order, stage_access_mask_stage_map, config))

    # saa -- stage_access by access used to build up SyncMaskTypes from VkAccessFlagBits
    saa_name = 'StageAccessMaskByAccessBit'
    saa_desc = 'Bit order mask of stage_access bit for each access'
    output.extend(CrossReferenceTable(saa_name, saa_desc, 'VkAccessFlags', access_in_bit_order, stage_access_mask_access_map, config))

    #sasa -- stage access index by stage by access
    sasa_name = 'StageAccessIndexByStageAndAccess'
    sasa_desc = 'stage_access index for each stage and access'
    output.extend(DoubleCrossReferenceTable(sasa_name, sasa_desc,stages_in_bit_order, access_in_bit_order, stage_access_stage_access_map, config))

    return output

def GenerateStaticMask(name, desc, bits, config):
    sep = ' |\n' + config['indent']
    variable_format = 'static {sync_mask_name} {var_prefix}StageAccess{}Mask = ( //  {}'
    output = [variable_format.format(name, desc, **config)]
    output.append(config['indent'] + sep.join(bits))
    output.extend([');', ''])

    return output

def ReadWriteMasks(stage_access_combinations, config):
    read_list = [ e['stage_access_bit'] for e in stage_access_combinations if e['is_read'] is 'true']
    write_list = [ e['stage_access_bit'] for e in stage_access_combinations if not e['is_read'] is 'true']
    output = ['// Constants defining the mask of all read and write stage_access states']
    output.extend(GenerateStaticMask('Read',  'Mask of all read StageAccess bits', read_list, config))
    output.extend(GenerateStaticMask('Write',  'Mask of all write StageAccess bits', write_list, config))

    return output

def GenSyncTypeHelper(gen) :
    config = {
        'var_prefix': 'sync',
        'type_prefix': 'Sync',
        'enum_prefix': 'SYNC_',
        'indent': '    ',
        'sync_mask_base_type': 'uint64_t',
        'vk_stage_flags': 'VkPipelineStageFlags',
        'vk_access_flags': 'VkAccessFlags'}
    config['sync_mask_name'] = '{}StageAccessFlags'.format(config['type_prefix'])
    config['ordinal_name'] = '{}StageAccessIndex'.format(config['type_prefix'])
    config['bits_name'] = '{}StageAccessFlagBits'.format(config['type_prefix'])

    lines = ['#pragma once', '', '#include <array>', '#include <map>', '#include <stdint.h>', '#include <vulkan/vulkan.h>', '']
    lines.extend(['// clang-format off', ''])
    lines.extend(("using {} = {};".format(config['sync_mask_name'], config['sync_mask_base_type']), ''))

    stage_order = pipeline_order.split()
    access_types = {stage:list() for stage in stage_order}
    if debug_top_level:
        lines.append('// Access types \n//    ' + '\n//    '.join(access_types) +  '\n' * 2)

    access_stage_table = ParseAccessType(snippet_access_types_supported)
    stage_access_table = CreateStageAccessTable(stage_order, access_stage_table)
    stage_access_combinations = CreateStageAccessCombinations(config, stage_order, stage_access_table)
    lines.extend(StageAccessEnums(stage_access_combinations, config))

    lines.extend(StageAccessCrossReference(gen.sync_enum, stage_access_combinations, config))
    lines.extend(ReadWriteMasks(stage_access_combinations, config))

    return  '\n'.join(lines)
