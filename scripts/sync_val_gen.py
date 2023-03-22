#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2023 The Khronos Group Inc.
# Copyright (c) 2015-2023 Valve Corporation
# Copyright (c) 2015-2023 LunarG, Inc.
# Copyright (c) 2015-2023 Google Inc.
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
import json
import re

# Script operation controls
debug_json_parse = False
debug_table_parse = False
debug_in_bit_order = False
debug_top_level = False
debug_queue_caps = False
debug_stage_order_parse = False
debug_bubble_insert = False
experimental_ordering = False

# Some DRY constants
vvl_fake_extension = '_SYNCVAL'
present_stage = 'VK_PIPELINE_STAGE_2_PRESENT_ENGINE_BIT' + vvl_fake_extension
presenting_access = 'VK_ACCESS_2_PRESENT_ACQUIRE_READ_BIT' + vvl_fake_extension  # Treated as read
presented_access = 'VK_ACCESS_2_PRESENT_PRESENTED_BIT' + vvl_fake_extension  # Treated as write
present_engine_accesses = (presenting_access, presented_access)

host_stage = 'VK_PIPELINE_STAGE_2_HOST_BIT'
no_top_bottom_stages = { host_stage, present_stage }
top_stage ='VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT'
bot_stage ='VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT'

sync_enum_stage_type = 'VkPipelineStageFlagBits2'
sync_enum_access_type = 'VkAccessFlagBits2'
sync_enum_types = (sync_enum_stage_type, sync_enum_access_type)

def FauxStageAccess() :
    return { presented_access: [present_stage], presenting_access: [present_stage] }

def DeclareFauxConst(enums_in_bit_order) :
    lines = ["// Fake stages and accesses for acquire present support"]
    for enum_type in sync_enum_types:
        enum_list =enums_in_bit_order[enum_type]
        format_string = "static const " + enum_type + " {name} = 0x{mask:016X}ULL;"
        for enum_info in enum_list:
            if (vvl_fake_extension not in enum_info['name']) :
                continue
            lines.append(format_string.format(**enum_info))
    return lines

def ParseAccessMasks(valid_usage_path, all_stages):
    vu_json_filename = os.path.join(valid_usage_path + os.sep, 'validusage.json')
    access_stage_table = {}
    if os.path.isfile(vu_json_filename):
        json_file = open(vu_json_filename, 'r', encoding='utf-8')
        vuid_dict = json.load(json_file)
        json_file.close()
    if len(vuid_dict) == 0:
        print('Error: Could not find, or error loading %s/validusage.json\n', vu_json_filename)
        sys.exit(1)
    # every synchronization2 access mask bit has a VUID enumerating all pipeline stages it is compatible with. It looks like this:
    # {
    #   "vuid": "VUID-VkMemoryBarrier2KHR-srcAccessMask-03900",
    #   "text": " If pname:srcAccessMask includes <code>access_mask</code>,
    #             pname:srcStageMask <strong class=\"purple\">must</strong> include <code>stage_1</code>,
    #             <code>stage_2</code>, <code>VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT_KHR</code>,
    #             or <code>VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR</code>"
    # },
    vuid_prefix = 'VUID-VkMemoryBarrier2-srcAccessMask'
    text_prefix = ' If pname:srcAccessMask includes '
    # remove catch-all pipeline stages that we don't need in our tables.
    strip_stages = [
        'VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT',
        'VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT',
        'VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT',
        # expands to INDEX_INPUT and VERTEX_ATTRIBUTE_INPUT, which are included explicitly in the VUIDs
        'VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT',
        # temporarily disable VK_HUAWEI_invocation_mask pending more detail in the sync chapter
        'VK_PIPELINE_STAGE_2_INVOCATION_MASK_BIT_HUAWEI',
    ]
    expand_stages = {
        # these have the same value
        'VK_PIPELINE_STAGE_2_SHADING_RATE_IMAGE_BIT_NV': ['VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR'],
        # some VUIDs include the text "or one of the etext:VK_PIPELINE_STAGE_*_SHADER_BIT stages"
        'VK_PIPELINE_STAGE_*_SHADER_BIT': [stage for stage in all_stages if '_SHADER_BIT' in stage]
    }
    expand_access_bits = {
        'VK_ACCESS_2_SHADING_RATE_IMAGE_READ_BIT_NV': ['VK_ACCESS_2_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR'],
        'VK_ACCESS_2_SHADER_WRITE_BIT': ['VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT'],
        'VK_ACCESS_2_SHADER_READ_BIT': ['VK_ACCESS_2_UNIFORM_READ_BIT',
                                        'VK_ACCESS_2_SHADER_SAMPLED_READ_BIT',
                                        'VK_ACCESS_2_SHADER_STORAGE_READ_BIT',
                                        'VK_ACCESS_2_SHADER_BINDING_TABLE_READ_BIT_KHR'],
    }
    for extension_combo, vuid_list in vuid_dict['validation']['VkMemoryBarrier2'].items():
        for vuid in vuid_list:
            if debug_json_parse:
                print('extension_combo: ', extension_combo)
            if 'VK_KHR_synchronization2' not in extension_combo:
                continue
            # skip ray-query off VUIDs so we get the maximum set of pipeline stages allowed.
            if '!(VK_KHR_ray_query)' in extension_combo:
                continue
            if vuid['vuid'].startswith(vuid_prefix) and vuid['text'].startswith(text_prefix):
                matches = re.findall('<code>(.*?)</code>', vuid['text'], re.DOTALL)
                access_bits = expand_access_bits.get(matches[0], [matches[0]])
                stages = []
                for m in matches[1:]:
                    if m not in strip_stages:
                        stages += expand_stages.get(m, [m])
                for bit in access_bits:
                    if debug_json_parse:
                        print('access_mask: ', bit, ' stages: ', stages)
                    access_stage_table[bit] = stages
    return access_stage_table

# use simplest filtering to assure completest list
def ParseQueueCaps(table_text):
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
                print('// line {}'.format(line))
            cols = line.split('|')
            if debug_table_parse:
                print('len(cols)', len(cols), cols)
            if len(cols) == 3:
                stage_column = cols[2]
                access_enum = cols[1].split(':')[1].strip()
                access_stage_table[access_enum]=[]
            else:
                stage_column = cols[0].strip()
            if debug_table_parse:
                print('stage_column:', stage_column)
            if stage_column.startswith(' Any'):
                continue
            if stage_column.startswith(' None required'):
                continue
            stage_column = stage_column.replace(', or ',', ') # Oxford comma
            stage_column = stage_column.replace(' or ',', ') # !Oxford comma
            stage_column = stage_column.replace(', ',',')
            stage_column = stage_column.rstrip(',')
            stages = stage_column.split(',')
            if debug_table_parse:
                print('stages', len(stages), stages)
            if len(stages) < 1:
                continue
            elif not stages[0]:
                continue
            stages_lens = [len(s.split(':')) for s in stages]
            stage_enums = [ s.split(':')[1].strip('or ') for s in stages]

            access_stage_table[access_enum] += stage_enums
            if(debug_table_parse):
                print('// access_stage_table[{}]: {}'.format(access_enum, '|'.join(access_stage_table[access_enum])))


    # Add in the fake stage
    access_stage_table[present_stage] = []

    return access_stage_table

def CreateStageAccessTable(stage_order, access_stage_table):
    stage_access_table = { stage: list() for stage in stage_order}
    for access, stages in access_stage_table.items():
        for stage in stages:
            stage_access_table[stage].append(access)

    return stage_access_table

# Snipped from chapters/synchronization.adoc -- tag v1.3.230
# manual fixups:
# - add back TOP_OF_PIPE and BOTTOM_OF_PIPE stages to everything
# - sync2-ify stage names
# - remove ifndef::VK_KHR_synchronization2[] content
# - remove ifdefs
# - make sure each pipeline section starts with "For"
# - move single stage 'pipeline' descriptions out of the snippet
#   and into a python list.
snippet_pipeline_stages_order = '''
[[synchronization-pipeline-stages-types]][[synchronization-pipeline-graphics]]
<<pipelines-graphics, Graphics pipelines>> are executable on queues
supporting ename:VK_QUEUE_GRAPHICS_BIT.
Stages executed by graphics pipelines can: only be specified in commands
recorded for queues supporting ename:VK_QUEUE_GRAPHICS_BIT.

For the graphics primitive pipeline executes the following stages, with the logical ordering of the
stages matching the order specified here:

  * ename:VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT
  * ename:VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT
  * ename:VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT
  * ename:VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT
  * ename:VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT
  * ename:VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT
  * ename:VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT
  * ename:VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT
  * ename:VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT
  * ename:VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR
  * ename:VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT
  * ename:VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT
  * ename:VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT
  * ename:VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
  * ename:VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT

For the graphics mesh pipeline executes the following stages, with the logical
ordering of the stages matching the order specified here:

  * ename:VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT
  * ename:VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT
  * ename:VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT
  * ename:VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT
  * ename:VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR
  * ename:VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT
  * ename:VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT
  * ename:VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT
  * ename:VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
  * ename:VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT

For the compute pipeline, the following stages occur in this order:

  * ename:VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT
  * ename:VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT
  * ename:VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT
  * ename:VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT

For graphics pipeline commands executing in a render pass with a fragment
density map attachment, the following pipeline stage where the fragment
density map read happens has no particular order relative to the other
stages, except that it is logically earlier than
ename:VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT:

  * ename:VK_PIPELINE_STAGE_2_FRAGMENT_DENSITY_PROCESS_BIT_EXT
  * ename:VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT

For the ray tracing pipeline, the following stages occur in this order:

  * ename:VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT
  * ename:VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT
  * ename:VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR
  * ename:VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT

'''

#For the X pipeline, only one pipeline stage occurs, so no order is guaranteed:
unordered_stages =  [
  'VK_PIPELINE_STAGE_2_COPY_BIT',
  'VK_PIPELINE_STAGE_2_RESOLVE_BIT',
  'VK_PIPELINE_STAGE_2_BLIT_BIT',
  'VK_PIPELINE_STAGE_2_CLEAR_BIT',
  'VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_COPY_BIT_KHR',
  'VK_PIPELINE_STAGE_2_HOST_BIT',
  'VK_PIPELINE_STAGE_2_COMMAND_PREPROCESS_BIT_NV',
  'VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR',
  'VK_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT',
  'VK_PIPELINE_STAGE_2_VIDEO_DECODE_BIT_KHR',
  'VK_PIPELINE_STAGE_2_VIDEO_ENCODE_BIT_KHR',
  'VK_PIPELINE_STAGE_2_SUBPASS_SHADING_BIT_HUAWEI',
  'VK_PIPELINE_STAGE_2_OPTICAL_FLOW_BIT_NV',
  'VK_PIPELINE_STAGE_2_MICROMAP_BUILD_BIT_EXT',
  present_stage,
]

pipeline_name_labels = {
    'GRAPHICS': 'For the graphics primitive pipeline',
    'MESH': 'For the graphics mesh pipeline',
    'COMPUTE': 'For the compute pipeline',
    'RAY_TRACING_SHADE': 'For the ray tracing pipeline',
}

def StageOrderListFromSet(stage_order, stage_set):
    return [ stage for stage in stage_order if stage in stage_set]

def BubbleInsertStages(stage_order, prior, subseq):
    # Get a fixed list for the order in which we'll add... this is a 'seed' order and will guarantee repeatilbity as it fixes the
    # order items not sorted relative to each other.  Any seed would do, even alphabetical
    stage_set = set(prior.keys()).union(set(subseq.keys()))

    #stage_set_ordered = StageOrderListFromSet(stage_order, stage_set)
    # Come up with a consistent stage order seed that will leave stage types (extension, core, vendor) grouped
    # Reverse the strings -- a spoonerism of the whole stage string, to put the type first
    #     spoonerism -- https://en.wikipedia.org/wiki/Spoonerism
    stage_spooner = list()
    acme = '_AAAAAA'
    stage_spooner = sorted([ stage.replace('_BIT', acme)[::-1] for stage in stage_set if stage not in no_top_bottom_stages])
    print('ZZZ BUBBLE spooner\n', '\n '.join(stage_spooner))

    # De-spoonerize
    stage_set_ordered = [ stage[::-1].replace(acme, '_BIT') for stage in stage_spooner]
    print('ZZZ BUBBLE de-spooner\n', '\n '.join(stage_set_ordered))

    stage_set_ordered = stage_set_ordered | no_top_bottom_stages
    stages = [ stage_set_ordered.pop(0) ]

    for stage in stage_set_ordered:
        if debug_bubble_insert: print('BUBBLE adding stage:', stage)
        inserted = False;
        for i in range(len(stages)):
            if stages[i] in subseq[stage]:
                stages.insert(i, stage)
                inserted = True
                break
        if not inserted:
            stages.append(stage)
        if debug_bubble_insert: print('BUBBLE result:', stages)

    print('BUBBLE\n', '\n '.join(stages))
    return stages

def ParsePipelineStageOrder(stage_order, stage_order_snippet, config) :
    pipeline_name = ''
    stage_lists = {}
    touchup = set()
    stage_entry_prefix = '* ename:'
    all_stages = set()
    list_started = False
    # Parse the snippet
    for line in stage_order_snippet.split('\n'):
        line = line.strip()
        if debug_stage_order_parse: print ('STAGE_ORDER', line)
        if not line:
            if debug_stage_order_parse: print ('STAGE_ORDER', 'skip empty')
            if list_started:
                if debug_stage_order_parse: print ('STAGE_ORDER', 'EOL')
                pipeline_name = ''
            continue
        if line.startswith('For') :
            for name, label in pipeline_name_labels.items():
                if line.startswith(label):
                    pipeline_name = name
                    stage_lists[name] = list()
                    list_started = False
                    break
            if debug_stage_order_parse: print ('STAGE_ORDER', 'new pipeline', pipeline_name)
            continue
        if line.startswith(stage_entry_prefix):
            if debug_stage_order_parse: print ('STAGE_ORDER', 'new entry')
            list_started = True
            stage = line.lstrip(stage_entry_prefix)
            all_stages.add(stage)
            if pipeline_name:
                if debug_stage_order_parse: print ('STAGE_ORDER', 'normal entry')
                stage_lists[pipeline_name].append(stage)
            else:
                # See if we've seen this before.  Touchups must be novel
                novel_stage = all([not stage in stage_list for stage_list in stage_lists.values()])
                if (novel_stage):
                    if debug_stage_order_parse: print ('STAGE_ORDER', 'touchup entry')
                    touchup.add(stage)
                else:
                    if debug_stage_order_parse: print ('STAGE_ORDER', 'context entry')

    if debug_stage_order_parse:
        print('STAGE_ORDER', 'PARSED PIPELINES')
        for pipeline_name, stage_list in stage_lists.items():
            print(pipeline_name,'|'.join(stage_list))

        print('STAGE_ORDER', 'PARSED all_stages')
        print('all_stages','|'.join(all_stages))


    # Create earlier/later maps
    prior = { stage:set() for stage in all_stages }
    subseq = { stage:set() for stage in all_stages }

    for pipeline_name, stage_list in stage_lists.items():
        for i in range(len(stage_list)):
            prior[stage_list[i]].update(stage_list[:i])
            subseq[stage_list[i]].update(stage_list[i+1:])

    # Touch up the stages that don't quite parse right
    touchups_done = set()

    # FDP Stage is only constrained to be before VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT
    fdp_stage = 'VK_PIPELINE_STAGE_2_FRAGMENT_DENSITY_PROCESS_BIT_EXT'
    fdp_before = 'VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT'
    prior[fdp_stage] = set()
    subseq[fdp_stage] = set([fdp_before])
    subseq[fdp_stage].update(subseq[fdp_before])
    for stage in subseq[fdp_stage]:
        prior[stage].add(fdp_stage)

    touchups_done.add(fdp_stage)


    # Handle all stages that don't enforce an order
    for stage in unordered_stages:
        prior[stage] = set()
        subseq[stage] = set()

    # Make sure top and bottom got added to every prior and subseq (respectively) except for HOST
    # and the every stage is prior and susequent to bottom and top (respectively) also except for HOST
    for stage, prior_stages in prior.items():
        if stage in no_top_bottom_stages: continue
        prior_stages.add(top_stage)
        subseq[top_stage].add(stage)
    for stage, subseq_stages in subseq.items():
        if stage in no_top_bottom_stages: continue
        subseq_stages.add(bot_stage)
        prior[bot_stage].add(stage)

    if experimental_ordering:
        stage_order = BubbleInsertStages(stage_order, prior, subseq)

    # Convert sets to ordered vectors
    prior_sets = prior
    prior = { key:StageOrderListFromSet(stage_order, value) for key, value in prior_sets.items() }
    subseq_sets = subseq
    subseq = { key:StageOrderListFromSet(stage_order, value) for key, value in subseq_sets.items() }

    if debug_stage_order_parse:
        print('STAGE_ORDER PRIOR STAGES')
        for stage, stage_set in prior.items():
            print(stage,'|'.join(stage_set))

        print('STAGE_ORDER SUBSEQUENT STAGES')
        for stage, stage_set in subseq.items():
            print(stage,'|'.join(stage_set))

    if touchups_done != touchup:
        print('STAGE_ORDER Stage order touchups failed')
        print('STAGE_ORDER touchups_done', touchups_done)
        print('STAGE_ORDER touchups found', touchup)
        exit(-1)

    return { 'stages': all_stages, 'stage_lists':stage_lists, 'prior': prior, 'subseq':subseq, 'touchups':touchup }

# Note: there should be a machine readable merged order, but there isn't... so we'll need some built-in consistency checks
# Pipeline stages in rough order, merge sorted.  When
# Legal stages are masked in, remaining stages are ordered
pipeline_order = '''
VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT
VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT
VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT
VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT
VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT
VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT
VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT
VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT
VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT
VK_PIPELINE_STAGE_2_FRAGMENT_DENSITY_PROCESS_BIT_EXT
VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT
VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT
VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR
VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT
VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT
VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT
VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT
VK_PIPELINE_STAGE_2_COPY_BIT
VK_PIPELINE_STAGE_2_RESOLVE_BIT
VK_PIPELINE_STAGE_2_BLIT_BIT
VK_PIPELINE_STAGE_2_CLEAR_BIT
VK_PIPELINE_STAGE_2_COMMAND_PREPROCESS_BIT_NV
VK_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT
VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR
VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR
VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_COPY_BIT_KHR
VK_PIPELINE_STAGE_2_VIDEO_DECODE_BIT_KHR
VK_PIPELINE_STAGE_2_VIDEO_ENCODE_BIT_KHR
VK_PIPELINE_STAGE_2_SUBPASS_SHADING_BIT_HUAWEI
VK_PIPELINE_STAGE_2_OPTICAL_FLOW_BIT_NV
VK_PIPELINE_STAGE_2_MICROMAP_BUILD_BIT_EXT
VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT
VK_PIPELINE_STAGE_2_HOST_BIT
'''

def InBitOrder(tag, enum_elem):
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
        if debug_in_bit_order:
            print ('adding ', {'name': entry['name'], 'mask': (1 << entry['bitpos'])})
        bitpos = entry['bitpos']
        in_bit_order.append({'name': entry['name'], 'mask': (1 << bitpos), 'bitpos': bitpos})

    return in_bit_order


def EnumsInBitOrder(sync_enum):
    enum_in_bit_order = dict()
    for enum_type in sync_enum_types:
        enum_in_bit_order[enum_type] = InBitOrder(enum_type, sync_enum[enum_type])

    # add the fake present engine enums
    stage_bitpos = enum_in_bit_order[sync_enum_stage_type][-1]['bitpos'] + 1
    enum_in_bit_order[sync_enum_stage_type].append({'name': present_stage, 'mask': (1 << stage_bitpos), 'bitpos': stage_bitpos})

    access_bitpos = enum_in_bit_order[sync_enum_access_type][-1]['bitpos']
    for name in present_engine_accesses:
        access_bitpos = access_bitpos + 1
        enum_in_bit_order[sync_enum_access_type].append({'name': name, 'mask': (1 << access_bitpos), 'bitpos': access_bitpos})

    return enum_in_bit_order

# Snipped from chapters/synchronization.adoc -- tag v1.3.230
# manual fixups:
# - sync2-ify stage names
# - remove ifdefs
# - expand VERTEX_INPUT and TRANSFER stages
# - manually add VIDEO_DECODE and VIDEO_ENCODE stages TODO: get them added to the spec
snippet_pipeline_stages_supported = '''
[[synchronization-pipeline-stages-supported]]
.Supported pipeline stage flags
[cols="60%,40%",options="header"]
|====
|Pipeline stage flag                                          | Required queue capability flag
|ename:VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT                      | None required
|ename:VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT                    | ename:VK_QUEUE_GRAPHICS_BIT or ename:VK_QUEUE_COMPUTE_BIT
|ename:VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT     | ename:VK_QUEUE_GRAPHICS_BIT
|ename:VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT                | ename:VK_QUEUE_GRAPHICS_BIT
|ename:VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT                    | ename:VK_QUEUE_GRAPHICS_BIT
|ename:VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT      | ename:VK_QUEUE_GRAPHICS_BIT
|ename:VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT   | ename:VK_QUEUE_GRAPHICS_BIT
|ename:VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT                  | ename:VK_QUEUE_GRAPHICS_BIT
|ename:VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT                  | ename:VK_QUEUE_GRAPHICS_BIT
|ename:VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT             | ename:VK_QUEUE_GRAPHICS_BIT
|ename:VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT              | ename:VK_QUEUE_GRAPHICS_BIT
|ename:VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT          | ename:VK_QUEUE_GRAPHICS_BIT
|ename:VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT                   | ename:VK_QUEUE_COMPUTE_BIT
|ename:VK_PIPELINE_STAGE_2_COPY_BIT                       | ename:VK_QUEUE_GRAPHICS_BIT, ename:VK_QUEUE_COMPUTE_BIT or ename:VK_QUEUE_TRANSFER_BIT
|ename:VK_PIPELINE_STAGE_2_CLEAR_BIT                      | ename:VK_QUEUE_GRAPHICS_BIT, ename:VK_QUEUE_COMPUTE_BIT or ename:VK_QUEUE_TRANSFER_BIT
|ename:VK_PIPELINE_STAGE_2_RESOLVE_BIT                    | ename:VK_QUEUE_GRAPHICS_BIT, ename:VK_QUEUE_COMPUTE_BIT or ename:VK_QUEUE_TRANSFER_BIT
|ename:VK_PIPELINE_STAGE_2_BLIT_BIT                       | ename:VK_QUEUE_GRAPHICS_BIT, ename:VK_QUEUE_COMPUTE_BIT or ename:VK_QUEUE_TRANSFER_BIT
|ename:VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_COPY_BIT_KHR  | ename:VK_QUEUE_GRAPHICS_BIT, ename:VK_QUEUE_COMPUTE_BIT or ename:VK_QUEUE_TRANSFER_BIT
|ename:VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT                   | None required
|ename:VK_PIPELINE_STAGE_2_HOST_BIT                             | None required
|ename:VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT                     | ename:VK_QUEUE_GRAPHICS_BIT
|ename:VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT                     | None required
|ename:VK_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT        | ename:VK_QUEUE_GRAPHICS_BIT or ename:VK_QUEUE_COMPUTE_BIT
|ename:VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT           | ename:VK_QUEUE_GRAPHICS_BIT
|ename:VK_PIPELINE_STAGE_2_COMMAND_PREPROCESS_BIT_NV            | ename:VK_QUEUE_GRAPHICS_BIT or ename:VK_QUEUE_COMPUTE_BIT
|ename:VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR            | ename:VK_QUEUE_GRAPHICS_BIT
|ename:VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT                   | ename:VK_QUEUE_GRAPHICS_BIT
|ename:VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT                   | ename:VK_QUEUE_GRAPHICS_BIT
|ename:VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR | ename:VK_QUEUE_COMPUTE_BIT
|ename:VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR           | ename:VK_QUEUE_COMPUTE_BIT
|ename:VK_PIPELINE_STAGE_2_FRAGMENT_DENSITY_PROCESS_BIT_EXT     | ename:VK_QUEUE_GRAPHICS_BIT
|ename:VK_PIPELINE_STAGE_2_SUBPASS_SHADING_BIT_HUAWEI         | ename:VK_QUEUE_GRAPHICS_BIT
|ename:VK_PIPELINE_STAGE_2_OPTICAL_FLOW_BIT_NV                | ename:VK_QUEUE_OPTICAL_FLOW_BIT_NV
|ename:VK_PIPELINE_STAGE_2_MICROMAP_BUILD_BIT_EXT             | ename:VK_QUEUE_COMPUTE_BIT
|ename:VK_PIPELINE_STAGE_2_VIDEO_DECODE_BIT_KHR                 | ename:VK_QUEUE_VIDEO_DECODE_BIT_KHR
|ename:VK_PIPELINE_STAGE_2_VIDEO_ENCODE_BIT_KHR                 | ename:VK_QUEUE_VIDEO_ENCODE_BIT_KHR
|====
'''

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
    return bit_suf;

 # Create the stage/access combination from the legal uses of access with stages
def CreateStageAccessCombinations(config, stage_order, stage_access_types):
    index = 1;
    enum_prefix = config['enum_prefix']
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
    for stage in stage_order:
        mini_stage = stage.lstrip()
        if mini_stage.startswith(enum_prefix):
            mini_stage = mini_stage.replace(enum_prefix,'')
        else:
            mini_stage = mini_stage.replace('VK_PIPELINE_STAGE_2_', '')
        mini_stage = mini_stage.replace('_BIT_KHR', '')
        mini_stage = mini_stage.replace('_BIT', '')

        # Because access_stage_table's elements order might be different sometimes.
        # It causes the generator creates different code. It needs to be sorted.
        stage_access_types[stage].sort();
        for access in stage_access_types[stage]:
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

def StageAccessEnums(stage_accesses, config):
    type_prefix = config['type_prefix']
    var_prefix = config['var_prefix']
    sync_mask_name = config['sync_mask_name']
    indent = config['indent']
    # The stage/access combinations in ordinal order
    output = []
    ordinal_name = config['ordinal_name']
    if not config['is_source']:
        output.append('// Unique number for each  stage/access combination')
        output.append('enum {} {{'.format(ordinal_name))
        output.extend([ '{}{} = {},'.format(indent, e['stage_access'], e['index'])  for e in stage_accesses])
        output.append('};')
        output.append('')

    # The stage/access combinations as bit mask
    output.append('// Unique bit for each  stage/access combination')
    if not config['is_source']:
        output.extend([ '{}static const {} {} = ({}(1) << {});'.format('', sync_mask_name, e['stage_access_bit'], sync_mask_name, e['stage_access'])  for e in stage_accesses if e['stage_access_bit'] is not None])
    output.append('')

    map_name = var_prefix + 'StageAccessIndexByStageAccessBit'
    output.append('// Map of the StageAccessIndices from the StageAccess Bit')
    typename = 'vvl::unordered_map<{}, {}>'.format(sync_mask_name, ordinal_name)
    if config['is_source']:
        output.append('const {}& {}() {{'.format(typename, map_name))
        output.append('{}static const {} variable = {{'.format(indent, typename))
        output.extend([ '{}{{ {}, {} }},'.format(indent * 2, e['stage_access_bit'], e['stage_access'])  for e in stage_accesses if e['stage_access_bit'] is not None])
        output.append('{}}};'.format(indent))
        output.append('{}return variable;'.format(indent))
        output.append('}')
    else:
        output.append('const {}& {}();'.format(typename, map_name))
    output.append('')

    # stage/access debug information based on ordinal enum
    sa_info_type = '{}StageAccessInfoType'.format(type_prefix)
    if not config['is_source']:
        output.append('struct {} {{'.format(sa_info_type))
        output.append('{}const char *name;'.format(indent))
        output.append('{}VkPipelineStageFlags2 stage_mask;'.format(indent))
        output.append('{}VkAccessFlags2 access_mask;'.format(indent))
        output.append('{}{} stage_access_index;'.format(indent, ordinal_name))
        output.append('{}{} stage_access_bit;'.format(indent, sync_mask_name))
        output.append('};\n')

    sa_info_var = '{}StageAccessInfoByStageAccessIndex'.format(config['var_prefix'])
    output.append('// Array of text names and component masks for each stage/access index')
    typename = 'std::array<{}, {}>'.format(sa_info_type, len(stage_accesses))
    if config['is_source']:
        output.append('const {}& {}() {{'.format(typename, sa_info_var))
        output.append('static const {} variable = {{ {{'.format(typename))
        fields_format ='{tab}{tab}{}'
        fields = ['stage_access_string', 'stage', 'access', 'stage_access']
        for entry in stage_accesses:
            output.append(indent+'{')
            for field in fields:
                output.append(fields_format.format(entry[field], tab=indent) + ',')
            bit = entry['stage_access_bit']
            output.append(fields_format.format(bit if bit is not None else 'SyncStageAccessFlags(0)', tab=indent))
            output.append(indent +'},')
        output.append('}};')
        output.append('return variable;')
        output.append('}')
    else:
        output.append('const {}& {}();'.format(typename, sa_info_var))

    output.append('')

    return output

def UnpackField(map, field='name'):
    return [ e[field] for e in map ]

def CrossReferenceTable(table_name, table_desc, key_type, mapped_type, key_vec, mask_map, config):
    indent = config['indent']

    table = ['// ' + table_desc]
    typename = 'std::map<{}, {}>'.format(key_type, mapped_type)
    if config['is_source']:
        table.append('const {}& {}() {{'.format(typename, config['var_prefix'] + table_name))
        table.append('{}static const {} variable = {{'.format(indent, typename))
        for mask_key in key_vec:
            mask_vec = mask_map[mask_key]
            if len(mask_vec) == 0:
                continue

            if len(mask_vec) > 1:
                sep = ' |\n' + indent * 2
                table.append( '{tab}{{ {}, (\n{tab}{tab}{}\n{tab})}},'.format(mask_key, sep.join(mask_vec), tab=indent))
            else:
                table.append( '{}{{ {}, {}}},'.format(indent, mask_key, mask_vec[0]))
        if(table_name == 'StageAccessMaskByAccessBit'):
            table.append( '{}{{ {}, {}}},'.format(indent, 'VK_ACCESS_2_MEMORY_READ_BIT', 'syncStageAccessReadMask'))
            table.append( '{}{{ {}, {}}},'.format(indent, 'VK_ACCESS_2_MEMORY_WRITE_BIT', 'syncStageAccessWriteMask'))
        table.append('{}}};'.format(indent))
        table.append('{}return variable;'.format(indent))
        table.append('}')
    else:
        table.append('const {}& {}();'.format(typename, config['var_prefix'] + table_name))
    table.append('')

    return table

def DoubleCrossReferenceTable(table_name, table_desc, stage_keys, access_keys, stage_access_stage_access_map, config):
    indent = config['indent']
    ordinal_name = config['ordinal_name']

    table = ['// ' + table_desc]
    typename = 'std::map<{vk_stage_flags}, std::map<{vk_access_flags}, {ordinal_name}>>'.format(**config)
    if config['is_source']:
        table.append('const {}& {var_prefix}{}() {{'.format(typename, table_name, **config))
        table.append('static const {} variable = {{'.format(typename))
        sep = ' },\n' + indent * 2 + '{ '

        # Because stage_access_stage_access_map's elements order might be different sometimes.
        # It causes the generator creates different code. It needs to be sorted.
        for i in sorted(stage_access_stage_access_map):
            if len(stage_access_stage_access_map[i].keys()) == 0: continue
            accesses = [ '{}, {}'.format(access, index) for access, index in sorted(stage_access_stage_access_map[i].items()) ]
            entry_format = '{tab}{{ {key}, {{\n{tab}{tab}{{ {val} }}\n{tab}}} }},'
            table.append( entry_format.format(key=i, val=sep.join(accesses), tab=indent))
        table.append('{}}};'.format(indent))
        table.append('{}return variable;'.format(indent))
        table.append('}')
    else:
        table.append('const {}& {var_prefix}{}();'.format(typename, table_name, **config))
    table.append('')

    return table

def StageAccessCrossReference(enum_in_bit_order, stage_access_combinations, config):
    output = []
    # Setup the cross reference tables
    stages_in_bit_order =  enum_in_bit_order['VkPipelineStageFlagBits2']
    access_in_bit_order =  enum_in_bit_order['VkAccessFlagBits2']
    stage_access_mask_stage_map = { e['name']: [] for e in stages_in_bit_order }
    #stage_access_mask_stage_map[none_stage] = [] # Support for N/A
    stage_access_mask_access_map = { e['name']: [] for e in access_in_bit_order }
    stage_access_stage_access_map = {  e['name']: dict() for e in stages_in_bit_order }
    direct_stage_to_access_map = {  e['name']: [] for e in stages_in_bit_order }

    for stage_access_combo in stage_access_combinations:
        combo_bit = stage_access_combo['stage_access_bit']
        stage = stage_access_combo['stage']
        if stage == 'VK_PIPELINE_STAGE_2_NONE_KHR': continue
        access = stage_access_combo['access']
        if access == 'VK_ACCESS_2_FLAG_NONE_KHR' : continue
        stage_access_mask_stage_map[stage].append(combo_bit)
        stage_access_mask_access_map[access].append(combo_bit)
        stage_access_stage_access_map[stage][access] = stage_access_combo['stage_access']
        direct_stage_to_access_map[stage].append(stage_access_combo['access'])

    # sas: stage_access masks by stage used to build up SyncMaskTypes from VkPipelineStageFlagBits
    sas_desc = 'Bit order mask of stage_access bit for each stage'
    sas_name = 'StageAccessMaskByStageBit'
    output.extend(CrossReferenceTable(sas_name, sas_desc, 'VkPipelineStageFlags2', config['sync_mask_name'],
                                      UnpackField(stages_in_bit_order), stage_access_mask_stage_map, config))

    # saa -- stage_access by access used to build up SyncMaskTypes from VkAccessFlagBits
    saa_name = 'StageAccessMaskByAccessBit'
    saa_desc = 'Bit order mask of stage_access bit for each access'
    output.extend(CrossReferenceTable(saa_name, saa_desc, 'VkAccessFlags2',  config['sync_mask_name'],
                                      UnpackField(access_in_bit_order), stage_access_mask_access_map, config))

    #sasa -- stage access index by stage by access
    sasa_name = 'StageAccessIndexByStageAndAccess'
    sasa_desc = 'stage_access index for each stage and access'
    output.extend(DoubleCrossReferenceTable(sasa_name, sasa_desc,stages_in_bit_order, access_in_bit_order, stage_access_stage_access_map, config))


    # direct VkPipelineStageFlags to valid VkAccessFlags lookup table
    direct_name = 'DirectStageToAccessMask'
    direct_desc = 'Direct VkPipelineStageFlags to valid VkAccessFlags lookup table'
    output.extend(CrossReferenceTable(direct_name, direct_desc, 'VkPipelineStageFlags2', 'VkAccessFlags2',
                                      UnpackField(stages_in_bit_order), direct_stage_to_access_map, config))

    return output

def GenerateStaticMask(name, desc, bits, config):
    sep = ' |\n' + config['indent']
    if not config['is_source']:
        variable_format = 'static const {sync_mask_name} {var_prefix}StageAccess{}Mask = ( //  {}'
        output = [variable_format.format(name, desc, **config)]
        output.append(config['indent'] + sep.join([b for b in bits if b is not None]))
        output.extend([');', ''])
        return output
    return []

def ReadWriteMasks(stage_access_combinations, config):
    # tri-state logic hack to keep the NONE stage access index out of these masks.
    read_list = [ e['stage_access_bit'] for e in stage_access_combinations if e['is_read']  == 'true' and e['is_read'] is not None]
    write_list = [ e['stage_access_bit'] for e in stage_access_combinations if e['is_read'] != 'true' and e['is_read'] is not None]
    output = ['// Constants defining the mask of all read and write stage_access states']
    output.extend(GenerateStaticMask('Read',  'Mask of all read StageAccess bits', read_list, config))
    output.extend(GenerateStaticMask('Write',  'Mask of all write StageAccess bits', write_list, config))

    return output

def AllCommandsByQueueCapability(stage_order, stage_queue_table, config):
    queue_cap_set = set()
    for stage, queue_flag_list in stage_queue_table.items():
        for queue_flag in queue_flag_list:
            queue_cap_set.add(queue_flag)

    queue_caps = sorted(queue_cap_set)
    queue_flag_map = { queue_flag:list() for queue_flag in queue_caps }

    # bits that expand out to several different stages
    expanded = ('VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT', 'VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT',
                'VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT', 'VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT')
    for stage in stage_order:
        if (stage in expanded) or (vvl_fake_extension in stage) :
            continue

        queue_flag_list = stage_queue_table[stage]

        if len(queue_flag_list) == 0:
            queue_flag_list = queue_caps

        if debug_queue_caps: print(stage, queue_flag_list)
        for queue_flag in queue_flag_list:
             queue_flag_map[queue_flag].append(stage)

    name = 'AllCommandStagesByQueueFlags'
    desc = 'Pipeline stages corresponding to VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT for each VkQueueFlagBits'
    return CrossReferenceTable(name, desc, 'VkQueueFlagBits', 'VkPipelineStageFlags2', queue_caps, queue_flag_map, config)

def PipelineOrderMaskMap(stage_order, stage_order_map, config):
    output = list()
    prior_name = 'LogicallyEarlierStages'
    prior_desc = 'Masks of logically earlier stage flags for a given stage flag'
    output.extend(CrossReferenceTable(prior_name, prior_desc, config['vk_stage_bits'], config['vk_stage_flags'], stage_order,
                                     stage_order_map['prior'], config))

    subseq_name = 'LogicallyLaterStages'
    subseq_desc = 'Masks of logically later stage flags for a given stage flag'
    output.extend(CrossReferenceTable(subseq_name, subseq_desc, config['vk_stage_bits'], config['vk_stage_flags'], stage_order,
                                     stage_order_map['subseq'], config))

    order_name = 'StageOrder'
    order_desc = 'Lookup table of stage orderings'
    order_nums = {stage_order[i] : [i] for i in range(len(stage_order)) }

    output.extend(CrossReferenceTable(order_name, order_desc, config['vk_stage_bits'], 'int', stage_order,
                                      order_nums, config))
    return output

def ShaderStageToSyncStageAccess( shader_stage_key, sync_stage_key ):
    return '    {{VK_SHADER_STAGE_{}, {{\n        SYNC_{}_SHADER_SAMPLED_READ, SYNC_{}_SHADER_STORAGE_READ, SYNC_{}_SHADER_STORAGE_WRITE, SYNC_{}_UNIFORM_READ}}}},'.format(shader_stage_key, sync_stage_key, sync_stage_key, sync_stage_key, sync_stage_key)

def ShaderStageAndSyncStageAccessMap(config):
    output = []
    if not config['is_source']:
        output.append('struct SyncShaderStageAccess {')
        output.append('    SyncStageAccessIndex sampled_read;')
        output.append('    SyncStageAccessIndex storage_read;')
        output.append('    SyncStageAccessIndex storage_write;')
        output.append('    SyncStageAccessIndex uniform_read;')
        output.append('};\n')
        output.append('const std::map<VkShaderStageFlagBits, SyncShaderStageAccess>& syncStageAccessMaskByShaderStage();')
    else:
        output.append('const std::map<VkShaderStageFlagBits, SyncShaderStageAccess>& syncStageAccessMaskByShaderStage() {')
        output.append('    static const std::map<VkShaderStageFlagBits, SyncShaderStageAccess> variable = {')
        output.append(ShaderStageToSyncStageAccess('VERTEX_BIT', 'VERTEX_SHADER'))
        output.append(ShaderStageToSyncStageAccess('TESSELLATION_CONTROL_BIT', 'TESSELLATION_CONTROL_SHADER'))
        output.append(ShaderStageToSyncStageAccess('TESSELLATION_EVALUATION_BIT', 'TESSELLATION_EVALUATION_SHADER'))
        output.append(ShaderStageToSyncStageAccess('GEOMETRY_BIT', 'GEOMETRY_SHADER'))
        output.append(ShaderStageToSyncStageAccess('FRAGMENT_BIT', 'FRAGMENT_SHADER'))
        output.append(ShaderStageToSyncStageAccess('COMPUTE_BIT', 'COMPUTE_SHADER'))
        output.append(ShaderStageToSyncStageAccess('RAYGEN_BIT_KHR', 'RAY_TRACING_SHADER'))
        output.append(ShaderStageToSyncStageAccess('ANY_HIT_BIT_KHR', 'RAY_TRACING_SHADER'))
        output.append(ShaderStageToSyncStageAccess('CLOSEST_HIT_BIT_KHR', 'RAY_TRACING_SHADER'))
        output.append(ShaderStageToSyncStageAccess('MISS_BIT_KHR', 'RAY_TRACING_SHADER'))
        output.append(ShaderStageToSyncStageAccess('INTERSECTION_BIT_KHR', 'RAY_TRACING_SHADER'))
        output.append(ShaderStageToSyncStageAccess('CALLABLE_BIT_KHR', 'RAY_TRACING_SHADER'))
        output.append(ShaderStageToSyncStageAccess('TASK_BIT_EXT', 'TASK_SHADER_EXT'))
        output.append(ShaderStageToSyncStageAccess('MESH_BIT_EXT', 'MESH_SHADER_EXT'))
        output.append('};\n')
        output.append('return variable;\n')
        output.append('}\n')
    return output

def GenSyncTypeHelper(gen, is_source) :
    lines = []

    config = {
        'var_prefix': 'sync',
        'type_prefix': 'Sync',
        'enum_prefix': 'SYNC_',
        'indent': '    ',
        'sync_mask_base_type': 'std::bitset<128>',
        'vk_stage_flags': 'VkPipelineStageFlags2',
        'vk_stage_bits': 'VkPipelineStageFlags2',
        'vk_access_flags': 'VkAccessFlags2',
        'vk_access_bits': 'VkAccessFlags2',
        'is_source': is_source}
    config['sync_mask_name'] = '{}StageAccessFlags'.format(config['type_prefix'])
    config['ordinal_name'] = '{}StageAccessIndex'.format(config['type_prefix'])
    config['bits_name'] = '{}StageAccessFlags'.format(config['type_prefix'])

    enums_in_bit_order = EnumsInBitOrder(gen.sync_enum)

    if config['is_source']:
        lines.extend(('#include "sync_validation_types.h"', ''))
    else:
        lines.extend(('#pragma once', '', '#include <array>', '#include <bitset>', '#include <map>', '#include <stdint.h>', '#include <vulkan/vulkan.h>',
                 '#include "vk_layer_data.h"'))
        lines.extend(('using {} = {};'.format(config['sync_mask_name'], config['sync_mask_base_type']), ''))
    lines.extend(['// clang-format off', ''])

    if not config['is_source']:
        lines.extend(DeclareFauxConst(enums_in_bit_order))
        lines.append('')


    stage_order = pipeline_order.split()
    stage_order.append(present_stage)
    access_types = {stage:list() for stage in stage_order}
    if debug_top_level:
        lines.append('// Access types \n//    ' + '\n//    '.join(access_types) +  '\n' * 2)

    stage_order_map = ParsePipelineStageOrder(stage_order, snippet_pipeline_stages_order, config)
    access_stage_table = ParseAccessMasks(gen.genOpts.valid_usage_path, stage_order)
    access_stage_table.update(FauxStageAccess());
    stage_queue_cap_table = ParseQueueCaps(snippet_pipeline_stages_supported)
    stage_access_table = CreateStageAccessTable(stage_order, access_stage_table)
    stage_access_combinations = CreateStageAccessCombinations(config, stage_order, stage_access_table)
    lines.extend(StageAccessEnums(stage_access_combinations, config))

    lines.extend(ReadWriteMasks(stage_access_combinations, config))

    lines.extend(StageAccessCrossReference(enums_in_bit_order, stage_access_combinations, config))
    lines.extend(AllCommandsByQueueCapability(stage_order, stage_queue_cap_table, config))
    lines.extend(PipelineOrderMaskMap(stage_order, stage_order_map, config))
    lines.extend(ShaderStageAndSyncStageAccessMap(config))
    return  '\n'.join(lines)
