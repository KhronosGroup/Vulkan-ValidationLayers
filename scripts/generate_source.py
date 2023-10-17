#!/usr/bin/env python3
# Copyright (c) 2021-2023 The Khronos Group Inc.
# Copyright (c) 2021-2023 Valve Corporation
# Copyright (c) 2021-2023 LunarG, Inc.
# Copyright (c) 2021-2023 Google Inc.
# Copyright (c) 2023-2023 RasterGrid Kft.
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

import argparse
import filecmp
import os
import shutil
import subprocess
import sys
import tempfile
import difflib
import json
import common_ci
from xml.etree import ElementTree

def RunGenerators(api: str, registry: str, grammar: str, directory: str, styleFile: str, targetFilter: str):

    has_clang_format = shutil.which('clang-format') is not None
    if not has_clang_format:
        print("WARNING: Unable to find clang-format!")

    # These live in the Vulkan-Docs repo, but are pulled in via the
    # Vulkan-Headers/registry folder
    # At runtime we inject python path to find these helper scripts
    scripts = os.path.dirname(registry)
    scripts_directory_path = os.path.dirname(os.path.abspath(__file__))
    registry_headers_path = os.path.join(scripts_directory_path, scripts)
    sys.path.insert(0, registry_headers_path)
    from reg import Registry

    from generators.base_generator import BaseGeneratorOptions
    from generators.thread_safety_generator import ThreadSafetyOutputGenerator
    from generators.stateless_validation_helper_generator import StatelessValidationHelperOutputGenerator
    from generators.object_tracker_generator import  ObjectTrackerOutputGenerator
    from generators.dispatch_table_helper_generator import DispatchTableHelperOutputGenerator
    from generators.extension_helper_generator import ExtensionHelperOutputGenerator
    from generators.api_version_generator import ApiVersionOutputGenerator
    from generators.layer_dispatch_table_generator import LayerDispatchTableOutputGenerator
    from generators.layer_chassis_generator import LayerChassisOutputGenerator
    from generators.layer_chassis_dispatch_generator import LayerChassisDispatchOutputGenerator
    from generators.function_pointers_generator import FunctionPointersOutputGenerator
    from generators.best_practices_generator import BestPracticesOutputGenerator
    from generators.spirv_validation_generator import SpirvValidationHelperOutputGenerator
    from generators.spirv_grammar_generator import SpirvGrammarHelperOutputGenerator
    from generators.command_validation_generator import CommandValidationOutputGenerator
    from generators.dynamic_state_generator import DynamicStateOutputGenerator
    from generators.sync_validation_generator import SyncValidationOutputGenerator
    from generators.object_types_generator import ObjectTypesOutputGenerator
    from generators.safe_struct_generator import SafeStructOutputGenerator
    from generators.enum_flag_bits_generator import EnumFlagBitsOutputGenerator
    from generators.valid_enum_values_generator import ValidEnumValuesOutputGenerator
    from generators.spirv_tool_commit_id_generator import SpirvToolCommitIdOutputGenerator
    from generators.error_location_helper_generator import ErrorLocationHelperOutputGenerator
    from generators.pnext_chain_extraction_generator import PnextChainExtractionGenerator
    from generators.state_tracker_helper_generator import StateTrackerHelperOutputGenerator

    # These set fields that are needed by both OutputGenerator and BaseGenerator,
    # but are uniform and don't need to be set at a per-generated file level
    from generators.base_generator import SetOutputDirectory, SetTargetApiName, SetMergedApiNames
    SetOutputDirectory(directory)
    SetTargetApiName(api)

    valid_usage_file = os.path.join(scripts, 'validusage.json')

    # Build up a list of all generators
    # Note: Options variable names MUST match order of constructor variable in generator
    generators = {
        'thread_safety_counter_definitions.h' : {
            'generator' : ThreadSafetyOutputGenerator,
            'genCombined': True,
        },
        'thread_safety_counter_instances.h' : {
            'generator' : ThreadSafetyOutputGenerator,
            'genCombined': True,
        },
        'thread_safety_counter_bodies.h' : {
            'generator' : ThreadSafetyOutputGenerator,
            'genCombined': True,
        },
        'thread_safety_commands.h' : {
            'generator' : ThreadSafetyOutputGenerator,
            'genCombined': True,
        },
        'thread_safety.cpp' : {
            'generator' : ThreadSafetyOutputGenerator,
            'genCombined': True,
        },
        'stateless_validation_helper.h' : {
            'generator' : StatelessValidationHelperOutputGenerator,
            'genCombined': False,
            'options' : [valid_usage_file],
        },
        'stateless_validation_helper.cpp' : {
            'generator' : StatelessValidationHelperOutputGenerator,
            'genCombined': False,
            'options' : [valid_usage_file],
        },
        'enum_flag_bits.h' : {
            'generator' : EnumFlagBitsOutputGenerator,
            'genCombined': False,
        },
        'valid_enum_values.h' : {
            'generator' : ValidEnumValuesOutputGenerator,
            'genCombined': False,
        },
        'valid_enum_values.cpp' : {
            'generator' : ValidEnumValuesOutputGenerator,
            'genCombined': False,
        },
        'object_tracker.h' : {
            'generator' : ObjectTrackerOutputGenerator,
            'genCombined': True,
            'options' : [valid_usage_file],
        },
        'object_tracker.cpp' : {
            'generator' : ObjectTrackerOutputGenerator,
            'genCombined': True,
            'options' : [valid_usage_file],
        },
        'error_location_helper.h' : {
            'generator' : ErrorLocationHelperOutputGenerator,
            'genCombined': True,
        },
        'error_location_helper.cpp' : {
            'generator' : ErrorLocationHelperOutputGenerator,
            'genCombined': True,
        },
        'vk_dispatch_table_helper.h' : {
            'generator' : DispatchTableHelperOutputGenerator,
            'genCombined': True,
        },
        'vk_function_pointers.h' : {
            'generator' : FunctionPointersOutputGenerator,
            'genCombined': True,
        },
        'vk_function_pointers.cpp' : {
            'generator' : FunctionPointersOutputGenerator,
            'genCombined': True,
        },
        'vk_layer_dispatch_table.h' : {
            'generator' : LayerDispatchTableOutputGenerator,
            'genCombined': True,
        },
        'vk_safe_struct.h' : {
            'generator' : SafeStructOutputGenerator,
            'genCombined': True,
        },
        'vk_safe_struct_utils.cpp' : {
            'generator' : SafeStructOutputGenerator,
            'genCombined': True,
        },
        'vk_safe_struct_core.cpp' : {
            'generator' : SafeStructOutputGenerator,
            'genCombined': True,
        },
        'vk_safe_struct_khr.cpp' : {
            'generator' : SafeStructOutputGenerator,
            'genCombined': True,
        },
        'vk_safe_struct_ext.cpp' : {
            'generator' : SafeStructOutputGenerator,
            'genCombined': True,
        },
        'vk_safe_struct_vendor.cpp' : {
            'generator' : SafeStructOutputGenerator,
            'genCombined': True,
        },
        'vk_object_types.h' : {
            'generator' : ObjectTypesOutputGenerator,
            'genCombined': True,
        },
        'vk_extension_helper.h' : {
            'generator' : ExtensionHelperOutputGenerator,
            'genCombined': True,
        },
        'vk_api_version.h' : {
            'generator' : ApiVersionOutputGenerator,
            'genCombined': True,
        },

        'chassis.h' : {
            'generator' : LayerChassisOutputGenerator,
            'genCombined': True,
        },
        'chassis.cpp' : {
            'generator' : LayerChassisOutputGenerator,
            'genCombined': False,
        },
        'chassis_dispatch_helper.h' : {
            'generator' : LayerChassisOutputGenerator,
            'genCombined': True,
        },
        'layer_chassis_dispatch.h' : {
            'generator' : LayerChassisDispatchOutputGenerator,
            'genCombined': True,
        },
        'layer_chassis_dispatch.cpp' : {
            'generator' : LayerChassisDispatchOutputGenerator,
            'genCombined': True,
        },
        'best_practices.h' : {
            'generator' : BestPracticesOutputGenerator,
            'genCombined': True,
        },
        'best_practices.cpp' : {
            'generator' : BestPracticesOutputGenerator,
            'genCombined': True,
        },
        'sync_validation_types.h' : {
            'generator' : SyncValidationOutputGenerator,
            'genCombined': False,
        },
        'sync_validation_types.cpp' : {
            'generator' : SyncValidationOutputGenerator,
            'genCombined': False,
        },
        'spirv_validation_helper.cpp' : {
            'generator' : SpirvValidationHelperOutputGenerator,
            'genCombined': False,
        },
        'spirv_grammar_helper.h' : {
            'generator' : SpirvGrammarHelperOutputGenerator,
            'genCombined': False,
            'options' : [grammar],
        },
        'spirv_grammar_helper.cpp' : {
            'generator' : SpirvGrammarHelperOutputGenerator,
            'genCombined': False,
            'options' : [grammar],
        },
        'spirv_tools_commit_id.h' : {
            'genCombined': False,
            'generator' : SpirvToolCommitIdOutputGenerator,
        },
        'command_validation.cpp' : {
            'generator' : CommandValidationOutputGenerator,
            'genCombined': True,
            'options' : [valid_usage_file],
        },
        'dynamic_state_helper.h' : {
            'generator' : DynamicStateOutputGenerator,
            'genCombined': False,
        },
        'dynamic_state_helper.cpp' : {
            'generator' : DynamicStateOutputGenerator,
            'genCombined': False,
        },
        'pnext_chain_extraction.h' : {
            'generator' : PnextChainExtractionGenerator,
            'genCombined': True,
        },
        'pnext_chain_extraction.cpp' : {
            'generator' : PnextChainExtractionGenerator,
            'genCombined': True,
        },
        'state_tracker_helper.h' : {
            'generator' : StateTrackerHelperOutputGenerator,
            'genCombined': True,
        },
        'state_tracker_helper.cpp' : {
            'generator' : StateTrackerHelperOutputGenerator,
            'genCombined': True,
        },
    }

    unknownTargets = [x for x in (targetFilter if targetFilter else []) if x not in generators.keys()]
    if unknownTargets:
        print(f'ERROR: No generator options for unknown target(s): {", ".join(unknownTargets)}', file=sys.stderr)
        return 1

    # Filter if --target is passed in
    targets = [x for x in generators.keys() if not targetFilter or x in targetFilter]

    for index, target in enumerate(targets, start=1):
        print(f'[{index}|{len(targets)}] Generating {target}')

        # First grab a class contructor object and create an instance
        targetGenerator = generators[target]['generator']
        generatorOptions = generators[target]['options'] if 'options' in generators[target] else []
        generator = targetGenerator(*generatorOptions)

        # This code and the 'genCombined' generator metadata is used by downstream
        # users to generate code with all Vulkan APIs merged into the target API variant
        # (e.g. Vulkan SC) when needed. The constructed apiList is also used to filter
        # out non-applicable extensions later below.
        apiList = [api]
        if api != 'vulkan' and generators[target]['genCombined']:
            SetMergedApiNames('vulkan')
            apiList.append('vulkan')
        else:
            SetMergedApiNames(None)

        baseOptions = BaseGeneratorOptions(customFileName = target)

        # Create the registry object with the specified generator and generator
        # options. The options are set before XML loading as they may affect it.
        reg = Registry(generator, baseOptions)

        # Parse the specified registry XML into an ElementTree object
        tree = ElementTree.parse(registry)

        # Filter out extensions that are not on the API list
        [exts.remove(e) for exts in tree.findall('extensions') for e in exts.findall('extension') if (sup := e.get('supported')) is not None and all(api not in sup.split(',') for api in apiList)]

        # Load the XML tree into the registry object
        reg.loadElementTree(tree)

        # Finally, use the output generator to create the requested target
        reg.apiGen()

        # Run clang-format on the file
        if has_clang_format:
            common_ci.RunShellCmd(f'clang-format -i --style=file:{styleFile} {os.path.join(directory, target)}')

# helper to define paths relative to the repo root
def repo_relative(path):
    return os.path.abspath(os.path.join(os.path.dirname(__file__), '..', path))

def main(argv):
    # files to exclude from --verify check
    # The shaders requires glslangvalidator, so they are updated manually with generate_spirv when needed
    verify_exclude = [
        '.clang-format',
        'gpu_as_inspection_comp.h',
        'gpu_pre_dispatch_comp.h',
        'gpu_pre_draw_vert.h',
        'inst_functions_comp.h',
        'gpu_inst_shader_hash.h'
    ]

    parser = argparse.ArgumentParser(description='Generate source code for this repository')
    parser.add_argument('--api',
                        default='vulkan',
                        choices=['vulkan'],
                        help='Specify API name to generate')
    parser.add_argument('registry', metavar='REGISTRY_PATH', help='path to the Vulkan-Headers registry directory')
    parser.add_argument('grammar', metavar='GRAMMAR_PATH', help='path to the SPIRV-Headers grammar directory')
    parser.add_argument('--generated-version', help='sets the header version used to generate the repo')
    parser.add_argument('-o', help='Create target and related files in specified directory.', dest='output_directory')
    group = parser.add_mutually_exclusive_group()
    group.add_argument('--target', nargs='+', help='only generate file names passed in')
    group.add_argument('-i', '--incremental', action='store_true', help='only update repo files that change')
    group.add_argument('-v', '--verify', action='store_true', help='verify repo files match generator output')
    args = parser.parse_args(argv)

    repo_dir = repo_relative(f'layers/{args.api}/generated')

    # Need pass style file incase running with --verify and it can't find the file automatically in the temp directory
    styleFile = os.path.join(repo_dir, '.clang-format')
    if common_ci.IsGHA() and args.verify:
        # Have found that sometimes (~5%) the 20.04 Ubuntu machines have clang-format v11 but we need v14 to
        # use a dedicated styleFile location. For these case there we can survive just skipping the verify check
        stdout = subprocess.check_output(['clang-format', '--version']).decode("utf-8")
        version = stdout[stdout.index('version') + 8:][:2]
        if int(version) < 14:
            return 0 # Success

    # Update the api_version in the respective json files
    if args.generated_version:
        json_files = []
        json_files.append(repo_relative('layers/VkLayer_khronos_validation.json.in'))
        json_files.append(repo_relative('tests/layers/VkLayer_device_profile_api.json.in'))
        for json_file in json_files:
            with open(json_file) as f:
                data = json.load(f)

            data["layer"]["api_version"] = args.generated_version

            with open(json_file, mode='w', encoding='utf-8', newline='\n') as f:
                f.write(json.dumps(data, indent=4))

    # get directory where generators will run
    if args.verify or args.incremental:
        # generate in temp directory so we can compare or copy later
        temp_obj = tempfile.TemporaryDirectory(prefix='vvl_codegen_')
        temp_dir = temp_obj.name
        gen_dir = temp_dir
    else:
        # generate directly in the repo
        gen_dir = repo_dir

    if args.output_directory is not None:
      gen_dir = args.output_directory

    registry = os.path.abspath(os.path.join(args.registry,  'vk.xml'))
    grammar = os.path.abspath(os.path.join(args.grammar, 'spirv.core.grammar.json'))
    RunGenerators(args.api, registry, grammar, gen_dir, styleFile, args.target)

    # Generate vk_validation_error_messages.h
    try:
        cmd = [repo_relative("scripts/vk_validation_stats.py"),
               os.path.abspath(os.path.join(args.registry, "validusage.json")),
              '-export_header']
        print(' '.join(cmd))
        subprocess.check_call([sys.executable] + cmd, cwd=gen_dir)
    except Exception as e:
        print('ERROR:', str(e))
        return 1

    # optional post-generation steps
    if args.verify:
        # compare contents of temp dir and repo
        temp_files = set(os.listdir(temp_dir))
        repo_files = set(os.listdir(repo_dir))
        for filename in sorted((temp_files | repo_files) - set(verify_exclude)):
            temp_filename = os.path.join(temp_dir, filename)
            repo_filename = os.path.join(repo_dir, filename)
            if filename not in repo_files:
                print('ERROR: Missing repo file', filename)
                return 2
            elif filename not in temp_files:
                print('ERROR: Missing generator for', filename)
                return 3
            elif not filecmp.cmp(temp_filename, repo_filename, shallow=False):
                print('ERROR: Repo files do not match generator output for', filename)
                # print line diff on file mismatch
                with open(temp_filename) as temp_file, open(repo_filename) as repo_file:
                    print(''.join(difflib.unified_diff(temp_file.readlines(),
                                                       repo_file.readlines(),
                                                       fromfile='temp/' + filename,
                                                       tofile=  'repo/' + filename)))
                return 4

        # return code for test scripts
        print('SUCCESS: Repo files match generator output')

    elif args.incremental:
        # copy missing or differing files from temp directory to repo
        for filename in os.listdir(temp_dir):
            temp_filename = os.path.join(temp_dir, filename)
            repo_filename = os.path.join(repo_dir, filename)
            if not os.path.exists(repo_filename) or \
               not filecmp.cmp(temp_filename, repo_filename, shallow=False):
                print('update', repo_filename)
                shutil.copyfile(temp_filename, repo_filename)

    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))

