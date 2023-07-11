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

    # These set fields that are needed by both OutputGenerator and BaseGenerator,
    # but are uniform and don't need to be set at a per-generated file level
    from generators.base_generator import (SetOutputDirectory, SetOutputFileName, SetTargetApiName)
    SetOutputDirectory(directory)
    SetOutputFileName(target)
    SetTargetApiName(api)

    valid_usage_file = os.path.join(scripts, 'validusage.json')

    # Build up a list of all generators
    # Note: Options variable names MUST match name of constructor variable names
    genOpts = {
        'thread_safety_counter_definitions.h' : {
            'generator' : ThreadSafetyOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'thread_safety_counter_instances.h' : {
            'generator' : ThreadSafetyOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'thread_safety_counter_bodies.h' : {
            'generator' : ThreadSafetyOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'thread_safety_commands.h' : {
            'generator' : ThreadSafetyOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'thread_safety.cpp' : {
            'generator' : ThreadSafetyOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'stateless_validation_helper.h' : {
            'generator' : StatelessValidationHelperOutputGenerator,
            'options' : [valid_usage_file],
            'baseGenOptions' : []
        },
        'stateless_validation_helper.cpp' : {
            'generator' : StatelessValidationHelperOutputGenerator,
            'options' : [valid_usage_file],
            'baseGenOptions' : []
        },
        'enum_flag_bits.h' : {
            'generator' : EnumFlagBitsOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'valid_enum_values.h' : {
            'generator' : ValidEnumValuesOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'valid_enum_values.cpp' : {
            'generator' : ValidEnumValuesOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'object_tracker.h' : {
            'generator' : ObjectTrackerOutputGenerator,
            'options' : [valid_usage_file],
            'baseGenOptions' : []
        },
        'object_tracker.cpp' : {
            'generator' : ObjectTrackerOutputGenerator,
            'options' : [valid_usage_file],
            'baseGenOptions' : []
        },
        'vk_dispatch_table_helper.h' : {
            'generator' : DispatchTableHelperOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'vk_function_pointers.h' : {
            'generator' : FunctionPointersOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'vk_function_pointers.cpp' : {
            'generator' : FunctionPointersOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'vk_layer_dispatch_table.h' : {
            'generator' : LayerDispatchTableOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'vk_enum_string_helper.h' : {
            'generator' : EnumStringHelperOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'vk_safe_struct.cpp' : {
            'generator' : SafeStructOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'vk_safe_struct.h' : {
            'generator' : SafeStructOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'vk_safe_struct_utils.cpp' : {
            'generator' : SafeStructOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'vk_safe_struct_core.cpp' : {
            'generator' : SafeStructOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'vk_safe_struct_khr.cpp' : {
            'generator' : SafeStructOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'vk_safe_struct_ext.cpp' : {
            'generator' : SafeStructOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'vk_safe_struct_vendor.cpp' : {
            'generator' : SafeStructOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'vk_object_types.h' : {
            'generator' : ObjectTypesOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'vk_extension_helper.h' : {
            'generator' : ExtensionHelperOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'vk_typemap_helper.h' : {
            'generator' : TypemapHelperOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'chassis.h' : {
            'generator' : LayerChassisOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'chassis.cpp' : {
            'generator' : LayerChassisOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'chassis_dispatch_helper.h' : {
            'generator' : LayerChassisOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'layer_chassis_dispatch.h' : {
            'generator' : LayerChassisDispatchOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'layer_chassis_dispatch.cpp' : {
            'generator' : LayerChassisDispatchOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'best_practices.h' : {
            'generator' : BestPracticesOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'best_practices.cpp' : {
            'generator' : BestPracticesOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'sync_validation_types.h' : {
            'generator' : SyncValidationOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'sync_validation_types.cpp' : {
            'generator' : SyncValidationOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'spirv_validation_helper.cpp' : {
            'generator' : SpirvValidationHelperOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'spirv_grammar_helper.h' : {
            'generator' : SpirvGrammarHelperOutputGenerator,
            'options' : [grammar],
            'baseGenOptions' : []
        },
        'spirv_grammar_helper.cpp' : {
            'generator' : SpirvGrammarHelperOutputGenerator,
            'options' : [grammar],
            'baseGenOptions' : []
        },
        'spirv_tools_commit_id.h' : {
            'generator' : SpirvToolCommitIdOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'command_validation.h' : {
            'generator' : CommandValidationOutputGenerator,
            'options' : [valid_usage_file],
            'baseGenOptions' : []
        },
        'command_validation.cpp' : {
            'generator' : CommandValidationOutputGenerator,
            'options' : [valid_usage_file],
            'baseGenOptions' : []
        },
        'dynamic_state_helper.h' : {
            'generator' : DynamicStateOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'dynamic_state_helper.cpp' : {
            'generator' : DynamicStateOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'vk_format_utils.h' : {
            'generator' : FormatUtilsOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
        'vk_format_utils.cpp' : {
            'generator' : FormatUtilsOutputGenerator,
            'options' : [],
            'baseGenOptions' : []
        },
    }

    if (target not in genOpts.keys()):
        print(f'ERROR: No generator options for unknown target: {target}', file=sys.stderr)
        return

    # First grab a class contructor object
    generator = genOpts[target]['generator']
    # Get array of argument for the class contructor
    options = genOpts[target]['options']
    # This is same as going
    #   gen = CommandValidationOutputGenerator(*[valid_usage_file = valid_usage_file])
    gen = generator(*options)

    baseGenOptions = genOpts[target]['baseGenOptions']
    options = BaseGeneratorOptions(*baseGenOptions)

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
