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

import argparse, pdb, sys, time, os

# Simple timer functions
startTime = None

def startTimer(timeit):
    global startTime
    if timeit:
        startTime = time.process_time()

def endTimer(timeit, msg):
    global startTime
    if timeit:
        endTime = time.process_time()
        write(msg, endTime - startTime, file=sys.stderr)
        startTime = None

# Turn a list of strings into a regexp string matching exactly those strings
def makeREstring(list, default = None):
    if len(list) > 0 or default is None:
        return '^(' + '|'.join(list) + ')$'
    else:
        return default

# Returns a directory of [ generator function, generator options ] indexed
# by specified short names. The generator options incorporate the following
# parameters:
#
# args is an parsed argument object; see below for the fields that are used.
def makeGenOpts(args):
    global genOpts
    genOpts = {}

    # Allow downstream users to merge other (e.g. the main "vulkan") API into
    # the API for which code is generated
    mergeApiNames = None

    # Extensions to warn about, if enabled(list of extensions)
    warnExtensions = args.warnExtensions

    # Output target directory
    from generators.base_generator import SetOutputDirectory
    from generators.base_generator import SetTargetApiName
    SetOutputDirectory(args.directory)
    SetTargetApiName(args.api)

    # ValidationLayer Generators
    # Options for thread safety header code-generation
    
    genOpts['thread_safety_counter_definitions.h'] = [
          ThreadOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'thread_safety_counter_definitions.h',
            mergeApiNames     = mergeApiNames)
        ]
    
    genOpts['thread_safety_counter_instances.h'] = [
          ThreadOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'thread_safety_counter_instances.h',
            mergeApiNames     = mergeApiNames)
        ]
    
    genOpts['thread_safety_counter_bodies.h'] = [
          ThreadOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'thread_safety_counter_bodies.h',
            mergeApiNames     = mergeApiNames)
        ]
    
    genOpts['thread_safety_commands.h'] = [
          ThreadOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'thread_safety_commands.h',
            mergeApiNames     = mergeApiNames)
        ]

    # Options for thread safety source code-generation
    genOpts['thread_safety.cpp'] = [
          ThreadOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'thread_safety.cpp',
            mergeApiNames     = mergeApiNames)
        ]

    # Options for stateless validation source file
    genOpts['parameter_validation.cpp'] = [
          ParameterValidationOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'parameter_validation.cpp',
            valid_usage_path  = args.scripts)
          ]

    # Options for stateless validation header file
    genOpts['parameter_validation.h'] = [
          ParameterValidationOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'parameter_validation.h',
            valid_usage_path  = args.scripts)
          ]

    # Options for stateless validation enum helper file
    genOpts['enum_flag_bits.h'] = [
          ParameterValidationOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'enum_flag_bits.h',
            mergeApiNames     = mergeApiNames,
            valid_usage_path  = args.scripts)
          ]

    # Options for object_tracker code-generated validation routines
    genOpts['object_tracker.cpp'] = [
          ObjectTrackerOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'object_tracker.cpp',
            mergeApiNames     = mergeApiNames,
            valid_usage_path  = args.scripts)
        ]

    # Options for object_tracker code-generated prototypes
    genOpts['object_tracker.h'] = [
          ObjectTrackerOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'object_tracker.h',
            mergeApiNames     = mergeApiNames,
            valid_usage_path  = args.scripts)
        ]

    # Options for dispatch table helper generator
    genOpts['vk_dispatch_table_helper.h'] = [
          DispatchTableHelperOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_dispatch_table_helper.h',
            mergeApiNames     = mergeApiNames)
        ]

    # lvt_file generator options for lvt_function_pointers.h
    genOpts['lvt_function_pointers.h'] = [
          LvtFileOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'lvt_function_pointers.h',
            mergeApiNames     = mergeApiNames,
            lvt_file_type     = 'function_pointer_header')
        ]

    # lvt_file generator options for lvt_function_pointers.cpp
    genOpts['lvt_function_pointers.cpp'] = [
          LvtFileOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'lvt_function_pointers.cpp',
            lvt_file_type     = 'function_pointer_source')
        ]

    # Options for Layer dispatch table generator
    genOpts['vk_layer_dispatch_table.h'] = [
          LayerDispatchTableOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_layer_dispatch_table.h',
            mergeApiNames     = mergeApiNames)
        ]

    # Helper file generator options for vk_enum_string_helper.h
    genOpts['vk_enum_string_helper.h'] = [
          HelperFileOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_enum_string_helper.h',
            mergeApiNames     = mergeApiNames,
            helper_file_type  = 'enum_string_header')
        ]

    # Helper file generator options for vk_safe_struct.h
    genOpts['vk_safe_struct.h'] = [
          HelperFileOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_safe_struct.h',
            mergeApiNames     = mergeApiNames,
            helper_file_type  = 'safe_struct_header')
        ]

    # Helper file generator options for vk_safe_struct_utils.cpp
    genOpts['vk_safe_struct_utils.cpp'] = [
          HelperFileOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_safe_struct_utils.cpp',
            mergeApiNames     = mergeApiNames,
            helper_file_type  = 'safe_struct_source')
        ]

    # Helper file generator options for vk_safe_struct_core.cpp
    genOpts['vk_safe_struct_core.cpp'] = [
          HelperFileOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_safe_struct_core.cpp',
            mergeApiNames     = mergeApiNames,
            helper_file_type  = 'safe_struct_source')
        ]

    # Helper file generator options for vk_safe_struct_khr.cpp
    genOpts['vk_safe_struct_khr.cpp'] = [
          HelperFileOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_safe_struct_khr.cpp',
            mergeApiNames     = mergeApiNames,
            helper_file_type  = 'safe_struct_source')
        ]

    # Helper file generator options for vk_safe_struct_khr.cpp
    genOpts['vk_safe_struct_ext.cpp'] = [
          HelperFileOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_safe_struct_ext.cpp',
            mergeApiNames     = mergeApiNames,
            helper_file_type  = 'safe_struct_source')
        ]

    # Helper file generator options for vk_safe_struct_vendor.cpp
    genOpts['vk_safe_struct_vendor.cpp'] = [
          HelperFileOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_safe_struct_vendor.cpp',
            mergeApiNames     = mergeApiNames,
            helper_file_type  = 'safe_struct_source')
        ]

    # Helper file generator options for vk_object_types.h
    genOpts['vk_object_types.h'] = [
          HelperFileOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_object_types.h',
            mergeApiNames     = mergeApiNames,
            helper_file_type  = 'object_types_header')
        ]

    # Helper file generator options for extension_helper.h
    genOpts['vk_extension_helper.h'] = [
          HelperFileOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_extension_helper.h',
            mergeApiNames     = mergeApiNames,
            helper_file_type  = 'extension_helper_header')
        ]

    # Helper file generator options for typemap_helper.h
    genOpts['vk_typemap_helper.h'] = [
          HelperFileOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_typemap_helper.h',
            mergeApiNames     = mergeApiNames,
            helper_file_type  = 'typemap_helper_header')
        ]

    # Layer chassis related generation structs
    # Options for layer chassis header
    genOpts['chassis.h'] = [
          LayerChassisOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'chassis.h',
            mergeApiNames     = mergeApiNames,
            warnExtensions    = warnExtensions,
            helper_file_type  = 'layer_chassis_header')
        ]

    # Options for layer chassis source file
    genOpts['chassis.cpp'] = [
          LayerChassisOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'chassis.cpp',
            warnExtensions    = warnExtensions,
            helper_file_type  = 'layer_chassis_source')
        ]

    # Layer chassis related generation structs
    # Options for layer chassis header
    genOpts['chassis_dispatch_helper.h'] = [
          LayerChassisOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'chassis_dispatch_helper.h',
            mergeApiNames     = mergeApiNames,
            helper_file_type  = 'layer_chassis_helper_header')
        ]

    # Options for layer chassis dispatch source file
    genOpts['layer_chassis_dispatch.cpp'] = [
          LayerChassisDispatchOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'layer_chassis_dispatch.cpp',
            mergeApiNames     = mergeApiNames)
        ]

    # Options for layer chassis dispatch header file
    genOpts['layer_chassis_dispatch.h'] = [
          LayerChassisDispatchOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'layer_chassis_dispatch.h',
            mergeApiNames     = mergeApiNames)
        ]

    # Options for best practices code-generated source
    genOpts['best_practices.cpp'] = [
          BestPracticesOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'best_practices.cpp',
            mergeApiNames     = mergeApiNames)
        ]

    # Options for best_practices code-generated header
    genOpts['best_practices.h'] = [
          BestPracticesOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'best_practices.h',
            mergeApiNames     = mergeApiNames)
        ]

# Create an API generator and corresponding generator options based on
# the requested target and command line options.
    # Helper file generator options for sync_validation_types.h
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

    # Options for spirv_validation_helper code-generated header
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
            grammar           = args.grammar)
        ]

    # Options for spirv_grammar_helper code-generated header
    # Only uses spirv grammar and not the vk.xml
    genOpts['spirv_grammar_helper.h'] = [
          SpirvGrammarHelperOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'spirv_grammar_helper.h',
            grammar           = args.grammar)
        ]

    # Options for command_validation code-generated header
    genOpts['command_validation.cpp'] = [
          CommandValidationOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'command_validation.cpp',
            mergeApiNames     = mergeApiNames,
            valid_usage_path  = args.scripts)
        ]

    # generator for command_validation.h
    genOpts['command_validation.h'] = [
          CommandValidationOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'command_validation.h',
            mergeApiNames     = mergeApiNames,
            valid_usage_path  = args.scripts)
        ]

    genOpts['dynamic_state_helper.cpp'] = [
          DynamicStateOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'dynamic_state_helper.cpp',
            mergeApiNames     = mergeApiNames)
        ]

    # generator for command_validation.h
    genOpts['dynamic_state_helper.h'] = [
          DynamicStateOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'dynamic_state_helper.h',
            mergeApiNames     = mergeApiNames)
        ]

    # Options for format_utils code-generated header
    genOpts['vk_format_utils.cpp'] = [
          FormatUtilsOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_format_utils.cpp',
            mergeApiNames     = mergeApiNames)
        ]

    # generator for format_utils.h
    genOpts['vk_format_utils.h'] = [
          FormatUtilsOutputGenerator,
          BaseGeneratorOptions(
            filename          = 'vk_format_utils.h',
            mergeApiNames     = mergeApiNames)
        ]

# Generate a target based on the options in the matching genOpts{} object.
# This is encapsulated in a function so it can be profiled and/or timed.
# The args parameter is an parsed argument object containing the following
# fields that are used:
#   target - target to generate
#   directory - directory to generate it in
#   interfaces
def genTarget(args):
    global genOpts

    # Create generator options with parameters specified on command line
    makeGenOpts(args)

    if (args.target in genOpts.keys()):
        createGenerator = genOpts[args.target][0]
        options = genOpts[args.target][1]

        if not args.quiet:
            write('* Building', options.filename, file=sys.stderr)

        gen = createGenerator(errFile=errWarn,
                              warnFile=errWarn,
                              diagFile=diag)
        if not args.quiet:
            write('* Generated', options.filename, file=sys.stderr)
        return (gen, options)
    else:
        write('No generator options for unknown target:',
              args.target, file=sys.stderr)
        return none

# -extension name
# For both, "name" may be a single name, or a space-separated list
# of names, or a regular expression.
if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    parser.add_argument('-api', action='store',
                        default='vulkan',
                        choices=['vulkan'],
                        help='Specify API name to generate')
    parser.add_argument('-warnExtensions', action='append',
                        default=[],
                        help='Specify an extension with partial support. Warning will be log if it is enabled')
    parser.add_argument('-debug', action='store_true',
                        help='Enable debugging')
    parser.add_argument('-dump', action='store_true',
                        help='Enable dump to stderr')
    parser.add_argument('-diagfile', action='store',
                        default=None,
                        help='Write diagnostics to specified file')
    parser.add_argument('-errfile', action='store',
                        default=None,
                        help='Write errors and warnings to specified file instead of stderr')
    parser.add_argument('-registry', action='store',
                        default='vk.xml',
                        help='Use specified registry file instead of vk.xml')
    parser.add_argument('-grammar', action='store',
                        default='spirv.core.grammar.json',
                        help='Use specified grammar file instead of spirv.core.grammar.json')
    parser.add_argument('-time', action='store_true',
                        help='Enable timing')
    parser.add_argument('-validate', action='store_true',
                        help='Enable XML group validation')
    parser.add_argument('-o', action='store', dest='directory',
                        default='.',
                        help='Create target and related files in specified directory')
    parser.add_argument('target', metavar='target', nargs='?',
                        help='Specify target')
    parser.add_argument('-quiet', action='store_true', default=True,
                        help='Suppress script output during normal execution.')
    parser.add_argument('-verbose', action='store_false', dest='quiet', default=True,
                        help='Enable script output during normal execution.')

    # This argument tells us where to load the script from the Vulkan-Headers registry
    parser.add_argument('-scripts', action='store',
                        help='Find additional scripts in this directory')

    args = parser.parse_args()

    # default scripts path to be same as registry
    if not args.scripts:
        args.scripts = os.path.dirname(args.registry)

    scripts_directory_path = os.path.dirname(os.path.abspath(__file__))
    registry_headers_path = os.path.join(scripts_directory_path, args.scripts)
    sys.path.insert(0, registry_headers_path)

    from reg import *
    from generator import write
    from cgenerator import CGeneratorOptions, COutputGenerator

    # ValidationLayer Generator Modifications
    from generators.base_generator import BaseGeneratorOptions

    from generators.thread_safety_generator import ThreadOutputGenerator
    from generators.parameter_validation_generator import ParameterValidationOutputGenerator
    from generators.object_tracker_generator import  ObjectTrackerOutputGenerator
    from generators.dispatch_table_helper_generator import DispatchTableHelperOutputGenerator
    from generators.helper_file_generator import HelperFileOutputGenerator
    from generators.layer_dispatch_table_generator import LayerDispatchTableOutputGenerator
    from generators.layer_chassis_generator import LayerChassisOutputGenerator
    from generators.layer_chassis_dispatch_generator import LayerChassisDispatchOutputGenerator
    from generators.lvt_file_generator import LvtFileOutputGenerator
    from generators.best_practices_generator import BestPracticesOutputGenerator
    from generators.spirv_validation_generator import SpirvValidationHelperOutputGenerator
    from generators.spirv_grammar_generator import SpirvGrammarHelperOutputGenerator
    from generators.command_validation_generator import CommandValidationOutputGenerator
    from generators.format_utils_generator import FormatUtilsOutputGenerator
    from generators.dynamic_state_generator import DynamicStateOutputGenerator
    from generators.sync_validation_generator import SyncValidationOutputGenerator

    # create error/warning & diagnostic files
    if (args.errfile):
        errWarn = open(args.errfile, 'w', encoding='utf-8')
    else:
        errWarn = sys.stderr

    if (args.diagfile):
        diag = open(args.diagfile, 'w', encoding='utf-8')
    else:
        diag = None

    # Create the API generator & generator options
    (gen, options) = genTarget(args)

    # Create the registry object with the specified generator and generator
    # options. The options are set before XML loading as they may affect it.
    reg = Registry(gen, options)

    # Parse the specified registry XML into an ElementTree object
    startTimer(args.time)
    tree = etree.parse(args.registry)
    endTimer(args.time, '* Time to make ElementTree =')

    # Filter out non-Vulkan extensions
    if args.api == 'vulkan':
        [exts.remove(e) for exts in tree.findall('extensions') for e in exts.findall('extension') if (sup := e.get('supported')) is not None and options.apiname not in sup.split(',')]

    # Load the XML tree into the registry object
    startTimer(args.time)
    reg.loadElementTree(tree)
    endTimer(args.time, '* Time to parse ElementTree =')

    if (args.validate):
        reg.validateGroups()

    if (args.dump):
        write('* Dumping registry to regdump.txt', file=sys.stderr)
        reg.dumpReg(filehandle = open('regdump.txt', 'w', encoding='utf-8'))

    # Finally, use the output generator to create the requested target
    if (args.debug):
        pdb.run('reg.apiGen()')
    else:
        startTimer(args.time)
        reg.apiGen()
        endTimer(args.time, '* Time to generate ' + options.filename + ' =')
        genTarget(args)
