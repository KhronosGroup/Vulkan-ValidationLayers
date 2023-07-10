#!/usr/bin/python3
#
# Copyright (c) 2013-2023 The Khronos Group Inc.
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
import sys
import os
from xml.etree import ElementTree

def RunGenerator(api: str, registry: str, grammar: str, directory: str, target: str):

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
    from generators.layer_dispatch_table_generator import LayerDispatchTableOutputGenerator
    from generators.layer_chassis_generator import LayerChassisOutputGenerator
    from generators.layer_chassis_dispatch_generator import LayerChassisDispatchOutputGenerator
    from generators.function_pointers_generator import FunctionPointersOutputGenerator
    from generators.best_practices_generator import BestPracticesOutputGenerator
    from generators.spirv_validation_generator import SpirvValidationHelperOutputGenerator
    from generators.spirv_grammar_generator import SpirvGrammarHelperOutputGenerator
    from generators.command_validation_generator import CommandValidationOutputGenerator
    from generators.format_utils_generator import FormatUtilsOutputGenerator
    from generators.dynamic_state_generator import DynamicStateOutputGenerator
    from generators.sync_validation_generator import SyncValidationOutputGenerator
    from generators.enum_string_helper_generator import EnumStringHelperOutputGenerator
    from generators.typemap_helper_generator import TypemapHelperOutputGenerator
    from generators.object_types_generator import ObjectTypesOutputGenerator
    from generators.safe_struct_generator import SafeStructOutputGenerator
    from generators.enum_flag_bits_generator import EnumFlagBitsOutputGenerator
    from generators.valid_enum_values_generator import ValidEnumValuesOutputGenerator
    from generators.spirv_tool_commit_id_generator import SpirvToolCommitIdOutputGenerator

    # Allow downstream users to merge other (e.g. the main "vulkan") API into
    # the API for which code is generated
    mergeApiNames = None

    # Output target directory
    from generators.base_generator import SetOutputDirectory
    from generators.base_generator import SetTargetApiName
    SetOutputDirectory(directory)
    SetTargetApiName(api)

    valid_usage_file = os.path.join(scripts, 'validusage.json')

    genOpts = {}

    # ValidationLayer Generators
    # Options for thread safety header code-generation
    genOpts['thread_safety_counter_definitions.h'] = [
          ThreadSafetyOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'thread_safety_counter_definitions.h',
            mergeApiNames     = mergeApiNames)
        ]

    genOpts['thread_safety_counter_instances.h'] = [
          ThreadSafetyOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'thread_safety_counter_instances.h',
            mergeApiNames     = mergeApiNames)
        ]

    genOpts['thread_safety_counter_bodies.h'] = [
          ThreadSafetyOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'thread_safety_counter_bodies.h',
            mergeApiNames     = mergeApiNames)
        ]

    genOpts['thread_safety_commands.h'] = [
          ThreadSafetyOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'thread_safety_commands.h',
            mergeApiNames     = mergeApiNames)
        ]

    genOpts['thread_safety.cpp'] = [
          ThreadSafetyOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'thread_safety.cpp',
            mergeApiNames     = mergeApiNames)
        ]

    genOpts['stateless_validation_helper.cpp'] = [
          StatelessValidationHelperOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'stateless_validation_helper.cpp',
            valid_usage_file  = valid_usage_file)
          ]

    genOpts['stateless_validation_helper.h'] = [
          StatelessValidationHelperOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'stateless_validation_helper.h',
            valid_usage_file  = valid_usage_file)
          ]

    genOpts['enum_flag_bits.h'] = [
          EnumFlagBitsOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'enum_flag_bits.h',
            mergeApiNames     = mergeApiNames)
          ]

    genOpts['valid_enum_values.h'] = [
          ValidEnumValuesOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'valid_enum_values.h',
            mergeApiNames     = mergeApiNames)
          ]

    genOpts['valid_enum_values.cpp'] = [
          ValidEnumValuesOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'valid_enum_values.cpp',
            mergeApiNames     = mergeApiNames)
          ]

    genOpts['object_tracker.cpp'] = [
          ObjectTrackerOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'object_tracker.cpp',
            mergeApiNames     = mergeApiNames,
            valid_usage_file  = valid_usage_file)
        ]

    genOpts['object_tracker.h'] = [
          ObjectTrackerOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'object_tracker.h',
            mergeApiNames     = mergeApiNames,
            valid_usage_file  = valid_usage_file)
        ]

    genOpts['vk_dispatch_table_helper.h'] = [
          DispatchTableHelperOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_dispatch_table_helper.h',
            mergeApiNames     = mergeApiNames)
        ]

    genOpts['vk_function_pointers.h'] = [
          FunctionPointersOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_function_pointers.h',
            mergeApiNames     = mergeApiNames)
        ]

    genOpts['vk_function_pointers.cpp'] = [
          FunctionPointersOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_function_pointers.cpp',
            mergeApiNames     = mergeApiNames)
        ]

    genOpts['vk_layer_dispatch_table.h'] = [
          LayerDispatchTableOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_layer_dispatch_table.h',
            mergeApiNames     = mergeApiNames)
        ]

    genOpts['vk_enum_string_helper.h'] = [
          EnumStringHelperOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_enum_string_helper.h',
            mergeApiNames     = mergeApiNames)
        ]

    genOpts['vk_safe_struct.h'] = [
          SafeStructOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_safe_struct.h',
            mergeApiNames     = mergeApiNames)
        ]

    genOpts['vk_safe_struct_utils.cpp'] = [
          SafeStructOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_safe_struct_utils.cpp',
            mergeApiNames     = mergeApiNames)
        ]

    genOpts['vk_safe_struct_core.cpp'] = [
          SafeStructOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_safe_struct_core.cpp',
            mergeApiNames     = mergeApiNames,
            helper_file_type  = 'safe_struct_source')
        ]

    genOpts['vk_safe_struct_khr.cpp'] = [
          SafeStructOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_safe_struct_khr.cpp',
            mergeApiNames     = mergeApiNames,
            helper_file_type  = 'safe_struct_source')
        ]

    genOpts['vk_safe_struct_ext.cpp'] = [
          SafeStructOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_safe_struct_ext.cpp',
            mergeApiNames     = mergeApiNames,
            helper_file_type  = 'safe_struct_source')
        ]

    genOpts['vk_safe_struct_vendor.cpp'] = [
          SafeStructOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_safe_struct_vendor.cpp',
            mergeApiNames     = mergeApiNames,
            helper_file_type  = 'safe_struct_source')
        ]

    genOpts['vk_object_types.h'] = [
          ObjectTypesOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_object_types.h',
            mergeApiNames     = mergeApiNames)
        ]

    genOpts['vk_extension_helper.h'] = [
          ExtensionHelperOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_extension_helper.h',
            mergeApiNames     = mergeApiNames)
        ]

    genOpts['vk_typemap_helper.h'] = [
          TypemapHelperOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_typemap_helper.h',
            mergeApiNames     = mergeApiNames)
        ]

    genOpts['chassis.h'] = [
          LayerChassisOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'chassis.h',
            mergeApiNames     = mergeApiNames,
            helper_file_type  = 'layer_chassis_header')
        ]

    genOpts['chassis.cpp'] = [
          LayerChassisOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'chassis.cpp',
            helper_file_type  = 'layer_chassis_source')
        ]

    genOpts['chassis_dispatch_helper.h'] = [
          LayerChassisOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'chassis_dispatch_helper.h',
            mergeApiNames     = mergeApiNames,
            helper_file_type  = 'layer_chassis_helper_header')
        ]

    genOpts['layer_chassis_dispatch.cpp'] = [
          LayerChassisDispatchOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'layer_chassis_dispatch.cpp',
            mergeApiNames     = mergeApiNames)
        ]

    genOpts['layer_chassis_dispatch.h'] = [
          LayerChassisDispatchOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'layer_chassis_dispatch.h',
            mergeApiNames     = mergeApiNames)
        ]

    genOpts['best_practices.cpp'] = [
          BestPracticesOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'best_practices.cpp',
            mergeApiNames     = mergeApiNames)
        ]

    genOpts['best_practices.h'] = [
          BestPracticesOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'best_practices.h',
            mergeApiNames     = mergeApiNames)
        ]

    genOpts['sync_validation_types.h'] = [
          SyncValidationOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'sync_validation_types.h',
            mergeApiNames     = mergeApiNames)
        ]

    genOpts['sync_validation_types.cpp'] = [
          SyncValidationOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'sync_validation_types.cpp',
            mergeApiNames     = mergeApiNames)
        ]

    genOpts['spirv_validation_helper.cpp'] = [
          SpirvValidationHelperOutputGenerator,
          BaseGeneratorOptions(filename = 'spirv_validation_helper.cpp')
        ]

    # Options for spirv_grammar_helper code-generated source
    # Only uses spirv grammar and not the vk.xml
    genOpts['spirv_grammar_helper.cpp'] = [
          SpirvGrammarHelperOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'spirv_grammar_helper.cpp',
            grammar           = grammar)
        ]

    # Options for spirv_grammar_helper code-generated header
    # Only uses spirv grammar and not the vk.xml
    genOpts['spirv_grammar_helper.h'] = [
          SpirvGrammarHelperOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'spirv_grammar_helper.h',
            grammar           = grammar)
        ]

    genOpts['spirv_tools_commit_id.h'] = [
          SpirvToolCommitIdOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'spirv_tools_commit_id.h')
        ]

    genOpts['command_validation.cpp'] = [
          CommandValidationOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'command_validation.cpp',
            mergeApiNames     = mergeApiNames,
            valid_usage_file  = valid_usage_file)
        ]

    genOpts['command_validation.h'] = [
          CommandValidationOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'command_validation.h',
            mergeApiNames     = mergeApiNames,
            valid_usage_file  = valid_usage_file)
        ]

    genOpts['dynamic_state_helper.cpp'] = [
          DynamicStateOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'dynamic_state_helper.cpp',
            mergeApiNames     = mergeApiNames)
        ]

    genOpts['dynamic_state_helper.h'] = [
          DynamicStateOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'dynamic_state_helper.h',
            mergeApiNames     = mergeApiNames)
        ]

    genOpts['vk_format_utils.cpp'] = [
          FormatUtilsOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_format_utils.cpp',
            mergeApiNames     = mergeApiNames)
        ]

    genOpts['vk_format_utils.h'] = [
          FormatUtilsOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_format_utils.h',
            mergeApiNames     = mergeApiNames)
        ]

    if (target not in genOpts.keys()):
        print(f'ERROR: No generator options for unknown target: {target}', file=sys.stderr)
        return

    createGenerator = genOpts[target][0]
    gen = createGenerator()

    options = genOpts[target][1]

    # Create the registry object with the specified generator and generator
    # options. The options are set before XML loading as they may affect it.
    reg = Registry(gen, options)

    # Parse the specified registry XML into an ElementTree object
    tree = ElementTree.parse(registry)

    # Filter out non-Vulkan extensions
    if api == 'vulkan':
        [exts.remove(e) for exts in tree.findall('extensions') for e in exts.findall('extension') if (sup := e.get('supported')) is not None and options.apiname not in sup.split(',')]

    # Load the XML tree into the registry object
    reg.loadElementTree(tree)

    # Finally, use the output generator to create the requested target
    reg.apiGen()

# -extension name
# For both, "name" may be a single name, or a space-separated list
# of names, or a regular expression.
if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    parser.add_argument('-api', action='store',
                        default='vulkan',
                        choices=['vulkan'],
                        help='Specify API name to generate')
    parser.add_argument('-registry', action='store',
                        default='vk.xml',
                        help='Use specified registry file instead of vk.xml')
    parser.add_argument('-grammar', action='store',
                        default='spirv.core.grammar.json',
                        help='Use specified grammar file instead of spirv.core.grammar.json')
    parser.add_argument('-o', action='store', dest='directory',
                        default='.',
                        help='Create target and related files in specified directory')
    parser.add_argument('target', metavar='target', nargs='?',
                        help='Specify target')

    args = parser.parse_args()

    RunGenerator(args.api, args.registry, args.grammar, args.directory, args.target)
