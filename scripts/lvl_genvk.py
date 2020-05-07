#!/usr/bin/python3
#
# Copyright (c) 2013-2019 The Khronos Group Inc.
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

import argparse, cProfile, pdb, string, sys, time, os

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

    # Default class of extensions to include, or None
    defaultExtensions = args.defaultExtensions

    # Additional extensions to include (list of extensions)
    extensions = args.extension

    # Extensions to remove (list of extensions)
    removeExtensions = args.removeExtensions

    # Extensions to emit (list of extensions)
    emitExtensions = args.emitExtensions

    # Features to include (list of features)
    features = args.feature

    # Whether to disable inclusion protect in headers
    protect = args.protect

    # Output target directory
    directory = args.directory

    # Descriptive names for various regexp patterns used to select
    # versions and extensions
    allFeatures     = allExtensions = '.*'
    noFeatures      = noExtensions = None

    # Turn lists of names/patterns into matching regular expressions
    addExtensionsPat     = makeREstring(extensions, None)
    removeExtensionsPat  = makeREstring(removeExtensions, None)
    emitExtensionsPat    = makeREstring(emitExtensions, allExtensions)
    featuresPat          = makeREstring(features, allFeatures)

    # Copyright text prefixing all headers (list of strings).
    prefixStrings = [
        '/*',
        '** Copyright (c) 2015-2019 The Khronos Group Inc.',
        '**',
        '** Licensed under the Apache License, Version 2.0 (the "License");',
        '** you may not use this file except in compliance with the License.',
        '** You may obtain a copy of the License at',
        '**',
        '**     http://www.apache.org/licenses/LICENSE-2.0',
        '**',
        '** Unless required by applicable law or agreed to in writing, software',
        '** distributed under the License is distributed on an "AS IS" BASIS,',
        '** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.',
        '** See the License for the specific language governing permissions and',
        '** limitations under the License.',
        '*/',
        ''
    ]

    # Text specific to Vulkan headers
    vkPrefixStrings = [
        '/*',
        '** This header is generated from the Khronos Vulkan XML API Registry.',
        '**',
        '*/',
        ''
    ]

    # Defaults for generating re-inclusion protection wrappers (or not)
    protectFeature = protect

    # An API style convention object
    conventions = VulkanConventions()

    # ValidationLayer Generators
    # Options for thread safety header code-generation
    genOpts['thread_safety.h'] = [
          ThreadOutputGenerator,
          ThreadGeneratorOptions(
            conventions       = conventions,
            filename          = 'thread_safety.h',
            directory         = directory,
            apiname           = 'vulkan',
            profile           = None,
            versions          = featuresPat,
            emitversions      = featuresPat,
            defaultExtensions = 'vulkan',
            addExtensions     = addExtensionsPat,
            removeExtensions  = removeExtensionsPat,
            emitExtensions    = emitExtensionsPat,
            prefixText        = prefixStrings + vkPrefixStrings,
            protectFeature    = False,
            apicall           = 'VKAPI_ATTR ',
            apientry          = 'VKAPI_CALL ',
            apientryp         = 'VKAPI_PTR *',
            alignFuncParam    = 48,
            expandEnumerants = False)
        ]

    # Options for thread safety source code-generation
    genOpts['thread_safety.cpp'] = [
          ThreadOutputGenerator,
          ThreadGeneratorOptions(
            conventions       = conventions,
            filename          = 'thread_safety.cpp',
            directory         = directory,
            apiname           = 'vulkan',
            profile           = None,
            versions          = featuresPat,
            emitversions      = featuresPat,
            defaultExtensions = 'vulkan',
            addExtensions     = addExtensionsPat,
            removeExtensions  = removeExtensionsPat,
            emitExtensions    = emitExtensionsPat,
            prefixText        = prefixStrings + vkPrefixStrings,
            protectFeature    = False,
            apicall           = 'VKAPI_ATTR ',
            apientry          = 'VKAPI_CALL ',
            apientryp         = 'VKAPI_PTR *',
            alignFuncParam    = 48,
            expandEnumerants = False)
        ]

    # Options for stateless validation source file
    genOpts['parameter_validation.cpp'] = [
          ParameterValidationOutputGenerator,
          ParameterValidationGeneratorOptions(
            conventions       = conventions,
            filename          = 'parameter_validation.cpp',
            directory         = directory,
            apiname           = 'vulkan',
            profile           = None,
            versions          = featuresPat,
            emitversions      = featuresPat,
            defaultExtensions = 'vulkan',
            addExtensions     = addExtensionsPat,
            removeExtensions  = removeExtensionsPat,
            emitExtensions    = emitExtensionsPat,
            prefixText        = prefixStrings + vkPrefixStrings,
            apicall           = 'VKAPI_ATTR ',
            apientry          = 'VKAPI_CALL ',
            apientryp         = 'VKAPI_PTR *',
            alignFuncParam    = 48,
            expandEnumerants  = False,
            valid_usage_path  = args.scripts)
          ]

    # Options for stateless validation source file
    genOpts['parameter_validation.h'] = [
          ParameterValidationOutputGenerator,
          ParameterValidationGeneratorOptions(
            conventions       = conventions,
            filename          = 'parameter_validation.h',
            directory         = directory,
            apiname           = 'vulkan',
            profile           = None,
            versions          = featuresPat,
            emitversions      = featuresPat,
            defaultExtensions = 'vulkan',
            addExtensions     = addExtensionsPat,
            removeExtensions  = removeExtensionsPat,
            emitExtensions    = emitExtensionsPat,
            prefixText        = prefixStrings + vkPrefixStrings,
            apicall           = 'VKAPI_ATTR ',
            apientry          = 'VKAPI_CALL ',
            apientryp         = 'VKAPI_PTR *',
            alignFuncParam    = 48,
            expandEnumerants  = False,
            valid_usage_path  = args.scripts)
          ]

    # Options for object_tracker code-generated validation routines
    genOpts['object_tracker.cpp'] = [
          ObjectTrackerOutputGenerator,
          ObjectTrackerGeneratorOptions(
            conventions       = conventions,
            filename          = 'object_tracker.cpp',
            directory         = directory,
            apiname           = 'vulkan',
            profile           = None,
            versions          = featuresPat,
            emitversions      = featuresPat,
            defaultExtensions = 'vulkan',
            addExtensions     = addExtensionsPat,
            removeExtensions  = removeExtensionsPat,
            emitExtensions    = emitExtensionsPat,
            prefixText        = prefixStrings + vkPrefixStrings,
            protectFeature    = False,
            apicall           = 'VKAPI_ATTR ',
            apientry          = 'VKAPI_CALL ',
            apientryp         = 'VKAPI_PTR *',
            alignFuncParam    = 48,
            expandEnumerants  = False,
            valid_usage_path  = args.scripts)
        ]

    # Options for object_tracker code-generated prototypes
    genOpts['object_tracker.h'] = [
          ObjectTrackerOutputGenerator,
          ObjectTrackerGeneratorOptions(
            conventions       = conventions,
            filename          = 'object_tracker.h',
            directory         = directory,
            apiname           = 'vulkan',
            profile           = None,
            versions          = featuresPat,
            emitversions      = featuresPat,
            defaultExtensions = 'vulkan',
            addExtensions     = addExtensionsPat,
            removeExtensions  = removeExtensionsPat,
            emitExtensions    = emitExtensionsPat,
            prefixText        = prefixStrings + vkPrefixStrings,
            protectFeature    = False,
            apicall           = 'VKAPI_ATTR ',
            apientry          = 'VKAPI_CALL ',
            apientryp         = 'VKAPI_PTR *',
            alignFuncParam    = 48,
            expandEnumerants  = False,
            valid_usage_path  = args.scripts)
        ]

    # Options for dispatch table helper generator
    genOpts['vk_dispatch_table_helper.h'] = [
          DispatchTableHelperOutputGenerator,
          DispatchTableHelperOutputGeneratorOptions(
            conventions       = conventions,
            filename          = 'vk_dispatch_table_helper.h',
            directory         = directory,
            apiname           = 'vulkan',
            profile           = None,
            versions          = featuresPat,
            emitversions      = featuresPat,
            defaultExtensions = 'vulkan',
            addExtensions     = addExtensionsPat,
            removeExtensions  = removeExtensionsPat,
            emitExtensions    = emitExtensionsPat,
            prefixText        = prefixStrings + vkPrefixStrings,
            apicall           = 'VKAPI_ATTR ',
            apientry          = 'VKAPI_CALL ',
            apientryp         = 'VKAPI_PTR *',
            alignFuncParam    = 48,
            expandEnumerants = False)
        ]

    # lvt_file generator options for lvt_function_pointers.h
    genOpts['lvt_function_pointers.h'] = [
          LvtFileOutputGenerator,
          LvtFileOutputGeneratorOptions(
            conventions       = conventions,
            filename          = 'lvt_function_pointers.h',
            directory         = directory,
            apiname           = 'vulkan',
            profile           = None,
            versions          = featuresPat,
            emitversions      = featuresPat,
            defaultExtensions = 'vulkan',
            addExtensions     = addExtensionsPat,
            removeExtensions  = removeExtensionsPat,
            emitExtensions    = emitExtensionsPat,
            prefixText        = prefixStrings + vkPrefixStrings,
            apicall           = 'VKAPI_ATTR ',
            apientry          = 'VKAPI_CALL ',
            apientryp         = 'VKAPI_PTR *',
            alignFuncParam    = 48,
            expandEnumerants = False,
            lvt_file_type  = 'function_pointer_header')
        ]

    # lvt_file generator options for lvt_function_pointers.cpp
    genOpts['lvt_function_pointers.cpp'] = [
          LvtFileOutputGenerator,
          LvtFileOutputGeneratorOptions(
            conventions       = conventions,
            filename          = 'lvt_function_pointers.cpp',
            directory         = directory,
            apiname           = 'vulkan',
            profile           = None,
            versions          = featuresPat,
            emitversions      = featuresPat,
            defaultExtensions = 'vulkan',
            addExtensions     = addExtensionsPat,
            removeExtensions  = removeExtensionsPat,
            emitExtensions    = emitExtensionsPat,
            prefixText        = prefixStrings + vkPrefixStrings,
            apicall           = 'VKAPI_ATTR ',
            apientry          = 'VKAPI_CALL ',
            apientryp         = 'VKAPI_PTR *',
            alignFuncParam    = 48,
            expandEnumerants = False,
            lvt_file_type  = 'function_pointer_source')
        ]

    # Options for Layer dispatch table generator
    genOpts['vk_layer_dispatch_table.h'] = [
          LayerDispatchTableOutputGenerator,
          LayerDispatchTableGeneratorOptions(
            conventions       = conventions,
            filename          = 'vk_layer_dispatch_table.h',
            directory         = directory,
            apiname           = 'vulkan',
            profile           = None,
            versions          = featuresPat,
            emitversions      = featuresPat,
            defaultExtensions = 'vulkan',
            addExtensions     = addExtensionsPat,
            removeExtensions  = removeExtensionsPat,
            emitExtensions    = emitExtensionsPat,
            prefixText        = prefixStrings + vkPrefixStrings,
            apicall           = 'VKAPI_ATTR ',
            apientry          = 'VKAPI_CALL ',
            apientryp         = 'VKAPI_PTR *',
            alignFuncParam    = 48,
            expandEnumerants = False)
        ]

    # Helper file generator options for vk_enum_string_helper.h
    genOpts['vk_enum_string_helper.h'] = [
          HelperFileOutputGenerator,
          HelperFileOutputGeneratorOptions(
            conventions       = conventions,
            filename          = 'vk_enum_string_helper.h',
            directory         = directory,
            apiname           = 'vulkan',
            profile           = None,
            versions          = featuresPat,
            emitversions      = featuresPat,
            defaultExtensions = 'vulkan',
            addExtensions     = addExtensionsPat,
            removeExtensions  = removeExtensionsPat,
            emitExtensions    = emitExtensionsPat,
            prefixText        = prefixStrings + vkPrefixStrings,
            apicall           = 'VKAPI_ATTR ',
            apientry          = 'VKAPI_CALL ',
            apientryp         = 'VKAPI_PTR *',
            alignFuncParam    = 48,
            expandEnumerants  = False,
            helper_file_type  = 'enum_string_header')
        ]

    # Helper file generator options for vk_safe_struct.h
    genOpts['vk_safe_struct.h'] = [
          HelperFileOutputGenerator,
          HelperFileOutputGeneratorOptions(
            conventions       = conventions,
            filename          = 'vk_safe_struct.h',
            directory         = directory,
            apiname           = 'vulkan',
            profile           = None,
            versions          = featuresPat,
            emitversions      = featuresPat,
            defaultExtensions = 'vulkan',
            addExtensions     = addExtensionsPat,
            removeExtensions  = removeExtensionsPat,
            emitExtensions    = emitExtensionsPat,
            prefixText        = prefixStrings + vkPrefixStrings,
            apicall           = 'VKAPI_ATTR ',
            apientry          = 'VKAPI_CALL ',
            apientryp         = 'VKAPI_PTR *',
            alignFuncParam    = 48,
            expandEnumerants  = False,
            helper_file_type  = 'safe_struct_header')
        ]

    # Helper file generator options for vk_safe_struct.cpp
    genOpts['vk_safe_struct.cpp'] = [
          HelperFileOutputGenerator,
          HelperFileOutputGeneratorOptions(
            conventions       = conventions,
            filename          = 'vk_safe_struct.cpp',
            directory         = directory,
            apiname           = 'vulkan',
            profile           = None,
            versions          = featuresPat,
            emitversions      = featuresPat,
            defaultExtensions = 'vulkan',
            addExtensions     = addExtensionsPat,
            removeExtensions  = removeExtensionsPat,
            emitExtensions    = emitExtensionsPat,
            prefixText        = prefixStrings + vkPrefixStrings,
            apicall           = 'VKAPI_ATTR ',
            apientry          = 'VKAPI_CALL ',
            apientryp         = 'VKAPI_PTR *',
            alignFuncParam    = 48,
            expandEnumerants  = False,
            helper_file_type  = 'safe_struct_source')
        ]

    # Helper file generator options for vk_object_types.h
    genOpts['vk_object_types.h'] = [
          HelperFileOutputGenerator,
          HelperFileOutputGeneratorOptions(
            conventions       = conventions,
            filename          = 'vk_object_types.h',
            directory         = directory,
            apiname           = 'vulkan',
            profile           = None,
            versions          = featuresPat,
            emitversions      = featuresPat,
            defaultExtensions = 'vulkan',
            addExtensions     = addExtensionsPat,
            removeExtensions  = removeExtensionsPat,
            emitExtensions    = emitExtensionsPat,
            prefixText        = prefixStrings + vkPrefixStrings,
            apicall           = 'VKAPI_ATTR ',
            apientry          = 'VKAPI_CALL ',
            apientryp         = 'VKAPI_PTR *',
            alignFuncParam    = 48,
            expandEnumerants  = False,
            helper_file_type  = 'object_types_header')
        ]

    # Helper file generator options for extension_helper.h
    genOpts['vk_extension_helper.h'] = [
          HelperFileOutputGenerator,
          HelperFileOutputGeneratorOptions(
            conventions       = conventions,
            filename          = 'vk_extension_helper.h',
            directory         = directory,
            apiname           = 'vulkan',
            profile           = None,
            versions          = featuresPat,
            emitversions      = featuresPat,
            defaultExtensions = 'vulkan',
            addExtensions     = addExtensionsPat,
            removeExtensions  = removeExtensionsPat,
            emitExtensions    = emitExtensionsPat,
            prefixText        = prefixStrings + vkPrefixStrings,
            apicall           = 'VKAPI_ATTR ',
            apientry          = 'VKAPI_CALL ',
            apientryp         = 'VKAPI_PTR *',
            alignFuncParam    = 48,
            expandEnumerants  = False,
            helper_file_type  = 'extension_helper_header')
        ]

    # Helper file generator options for typemap_helper.h
    genOpts['vk_typemap_helper.h'] = [
          HelperFileOutputGenerator,
          HelperFileOutputGeneratorOptions(
            conventions       = conventions,
            filename          = 'vk_typemap_helper.h',
            directory         = directory,
            apiname           = 'vulkan',
            profile           = None,
            versions          = featuresPat,
            emitversions      = featuresPat,
            defaultExtensions = 'vulkan',
            addExtensions     = addExtensionsPat,
            removeExtensions  = removeExtensionsPat,
            emitExtensions    = emitExtensionsPat,
            prefixText        = prefixStrings + vkPrefixStrings,
            protectFeature    = False,
            apicall           = 'VKAPI_ATTR ',
            apientry          = 'VKAPI_CALL ',
            apientryp         = 'VKAPI_PTR *',
            alignFuncParam    = 48,
            expandEnumerants  = False,
            helper_file_type  = 'typemap_helper_header')
        ]

    # Layer chassis related generation structs
    # Options for layer chassis header
    genOpts['chassis.h'] = [
          LayerChassisOutputGenerator,
          LayerChassisGeneratorOptions(
            conventions       = conventions,
            filename          = 'chassis.h',
            directory         = directory,
            apiname           = 'vulkan',
            profile           = None,
            versions          = featuresPat,
            emitversions      = featuresPat,
            defaultExtensions = 'vulkan',
            addExtensions     = addExtensionsPat,
            removeExtensions  = removeExtensionsPat,
            emitExtensions    = emitExtensionsPat,
            prefixText        = prefixStrings + vkPrefixStrings,
            apicall           = 'VKAPI_ATTR ',
            apientry          = 'VKAPI_CALL ',
            apientryp         = 'VKAPI_PTR *',
            alignFuncParam    = 48,
            helper_file_type  = 'layer_chassis_header',
            expandEnumerants = False)
        ]

    # Options for layer chassis source file
    genOpts['chassis.cpp'] = [
          LayerChassisOutputGenerator,
          LayerChassisGeneratorOptions(
            conventions       = conventions,
            filename          = 'chassis.cpp',
            directory         = directory,
            apiname           = 'vulkan',
            profile           = None,
            versions          = featuresPat,
            emitversions      = featuresPat,
            defaultExtensions = 'vulkan',
            addExtensions     = addExtensionsPat,
            removeExtensions  = removeExtensionsPat,
            emitExtensions    = emitExtensionsPat,
            prefixText        = prefixStrings + vkPrefixStrings,
            apicall           = 'VKAPI_ATTR ',
            apientry          = 'VKAPI_CALL ',
            apientryp         = 'VKAPI_PTR *',
            alignFuncParam    = 48,
            helper_file_type  = 'layer_chassis_source',
            expandEnumerants = False)
        ]

    # Options for layer chassis dispatch source file
    genOpts['layer_chassis_dispatch.cpp'] = [
          LayerChassisDispatchOutputGenerator,
          LayerChassisDispatchGeneratorOptions(
            conventions       = conventions,
            filename          = 'layer_chassis_dispatch.cpp',
            directory         = directory,
            apiname           = 'vulkan',
            profile           = None,
            versions          = featuresPat,
            emitversions      = featuresPat,
            defaultExtensions = 'vulkan',
            addExtensions     = addExtensionsPat,
            removeExtensions  = removeExtensionsPat,
            emitExtensions    = emitExtensionsPat,
            prefixText        = prefixStrings + vkPrefixStrings,
            protectFeature    = False,
            apicall           = 'VKAPI_ATTR ',
            apientry          = 'VKAPI_CALL ',
            apientryp         = 'VKAPI_PTR *',
            alignFuncParam    = 48,
            expandEnumerants = False)
        ]

    # Options for layer chassis dispatch header file
    genOpts['layer_chassis_dispatch.h'] = [
          LayerChassisDispatchOutputGenerator,
          LayerChassisDispatchGeneratorOptions(
            conventions       = conventions,
            filename          = 'layer_chassis_dispatch.h',
            directory         = directory,
            apiname           = 'vulkan',
            profile           = None,
            versions          = featuresPat,
            emitversions      = featuresPat,
            defaultExtensions = 'vulkan',
            addExtensions     = addExtensionsPat,
            removeExtensions  = removeExtensionsPat,
            emitExtensions    = emitExtensionsPat,
            prefixText        = prefixStrings + vkPrefixStrings,
            protectFeature    = False,
            apicall           = 'VKAPI_ATTR ',
            apientry          = 'VKAPI_CALL ',
            apientryp         = 'VKAPI_PTR *',
            alignFuncParam    = 48,
            expandEnumerants = False)
        ]

    # generator for command_counter.h
    genOpts['command_counter_helper.h'] = [
          CommandCounterOutputGenerator,
          CommandCounterOutputGeneratorOptions(
            conventions       = conventions,
            filename          = 'command_counter_helper.h',
            directory         = directory,
            apiname           = 'vulkan',
            profile           = None,
            versions          = featuresPat,
            emitversions      = featuresPat,
            defaultExtensions = 'vulkan',
            addExtensions     = addExtensionsPat,
            removeExtensions  = removeExtensionsPat,
            emitExtensions    = emitExtensionsPat,
            prefixText        = prefixStrings + vkPrefixStrings,
            apicall           = 'VKAPI_ATTR ',
            apientry          = 'VKAPI_CALL ',
            apientryp         = 'VKAPI_PTR *',
            alignFuncParam    = 48,
            expandEnumerants = False,
            lvt_file_type  = 'function_pointer_header')
        ]

    # generator for command_counter.cpp
    genOpts['command_counter_helper.cpp'] = [
          CommandCounterOutputGenerator,
          CommandCounterOutputGeneratorOptions(
            conventions       = conventions,
            filename          = 'command_counter_helper.cpp',
            directory         = directory,
            apiname           = 'vulkan',
            profile           = None,
            versions          = featuresPat,
            emitversions      = featuresPat,
            defaultExtensions = 'vulkan',
            addExtensions     = addExtensionsPat,
            removeExtensions  = removeExtensionsPat,
            emitExtensions    = emitExtensionsPat,
            prefixText        = prefixStrings + vkPrefixStrings,
            apicall           = 'VKAPI_ATTR ',
            apientry          = 'VKAPI_CALL ',
            apientryp         = 'VKAPI_PTR *',
            alignFuncParam    = 48,
            expandEnumerants = False,
            lvt_file_type  = 'function_pointer_source')
        ]

    # Options for best practices code-generated source
    genOpts['best_practices.cpp'] = [
          BestPracticesOutputGenerator,
          BestPracticesOutputGeneratorOptions(
            conventions       = conventions,
            filename          = 'best_practices.cpp',
            directory         = directory,
            apiname           = 'vulkan',
            profile           = None,
            versions          = featuresPat,
            emitversions      = featuresPat,
            defaultExtensions = 'vulkan',
            addExtensions     = addExtensionsPat,
            removeExtensions  = removeExtensionsPat,
            emitExtensions    = emitExtensionsPat,
            prefixText        = prefixStrings + vkPrefixStrings,
            protectFeature    = False,
            apicall           = 'VKAPI_ATTR ',
            apientry          = 'VKAPI_CALL ',
            apientryp         = 'VKAPI_PTR *',
            alignFuncParam    = 48,
            expandEnumerants  = False)
        ]

    # Options for best_practices code-generated header
    genOpts['best_practices.h'] = [
          BestPracticesOutputGenerator,
          BestPracticesOutputGeneratorOptions(
            conventions       = conventions,
            filename          = 'best_practices.h',
            directory         = directory,
            apiname           = 'vulkan',
            profile           = None,
            versions          = featuresPat,
            emitversions      = featuresPat,
            defaultExtensions = 'vulkan',
            addExtensions     = addExtensionsPat,
            removeExtensions  = removeExtensionsPat,
            emitExtensions    = emitExtensionsPat,
            prefixText        = prefixStrings + vkPrefixStrings,
            protectFeature    = False,
            apicall           = 'VKAPI_ATTR ',
            apientry          = 'VKAPI_CALL ',
            apientryp         = 'VKAPI_PTR *',
            alignFuncParam    = 48,
            expandEnumerants  = False)
        ]

# Create an API generator and corresponding generator options based on
# the requested target and command line options.
# This is encapsulated in a function so it can be profiled and/or timed.
# The args parameter is an parsed argument object containing the following
# fields that are used:
#   target - target to generate
#   directory - directory to generate it in
#   protect - True if re-inclusion wrappers should be created
#   extensions - list of additional extensions to include in generated
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
            write('* options.versions          =', options.versions, file=sys.stderr)
            write('* options.emitversions      =', options.emitversions, file=sys.stderr)
            write('* options.defaultExtensions =', options.defaultExtensions, file=sys.stderr)
            write('* options.addExtensions     =', options.addExtensions, file=sys.stderr)
            write('* options.removeExtensions  =', options.removeExtensions, file=sys.stderr)
            write('* options.emitExtensions    =', options.emitExtensions, file=sys.stderr)

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

# -feature name
# -extension name
# For both, "name" may be a single name, or a space-separated list
# of names, or a regular expression.
if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    parser.add_argument('-defaultExtensions', action='store',
                        default='vulkan',
                        help='Specify a single class of extensions to add to targets')
    parser.add_argument('-extension', action='append',
                        default=[],
                        help='Specify an extension or extensions to add to targets')
    parser.add_argument('-removeExtensions', action='append',
                        default=[],
                        help='Specify an extension or extensions to remove from targets')
    parser.add_argument('-emitExtensions', action='append',
                        default=[],
                        help='Specify an extension or extensions to emit in targets')
    parser.add_argument('-feature', action='append',
                        default=[],
                        help='Specify a core API feature name or names to add to targets')
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
    parser.add_argument('-noprotect', dest='protect', action='store_false',
                        help='Disable inclusion protection in output headers')
    parser.add_argument('-profile', action='store_true',
                        help='Enable profiling')
    parser.add_argument('-registry', action='store',
                        default='vk.xml',
                        help='Use specified registry file instead of vk.xml')
    parser.add_argument('-time', action='store_true',
                        help='Enable timing')
    parser.add_argument('-validate', action='store_true',
                        help='Enable group validation')
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
    from thread_safety_generator import  ThreadGeneratorOptions, ThreadOutputGenerator
    from parameter_validation_generator import ParameterValidationGeneratorOptions, ParameterValidationOutputGenerator
    from object_tracker_generator import ObjectTrackerGeneratorOptions, ObjectTrackerOutputGenerator
    from dispatch_table_helper_generator import DispatchTableHelperOutputGenerator, DispatchTableHelperOutputGeneratorOptions
    from helper_file_generator import HelperFileOutputGenerator, HelperFileOutputGeneratorOptions
    from layer_dispatch_table_generator import LayerDispatchTableOutputGenerator, LayerDispatchTableGeneratorOptions
    from layer_chassis_generator import LayerChassisOutputGenerator, LayerChassisGeneratorOptions
    from layer_chassis_dispatch_generator import LayerChassisDispatchOutputGenerator, LayerChassisDispatchGeneratorOptions
    from lvt_file_generator import LvtFileOutputGenerator, LvtFileOutputGeneratorOptions
    from command_counter_generator import CommandCounterOutputGenerator, CommandCounterOutputGeneratorOptions
    from best_practices_generator import BestPracticesOutputGenerator, BestPracticesOutputGeneratorOptions

    # Temporary workaround for vkconventions python2 compatibility
    import abc; abc.ABC = abc.ABCMeta('ABC', (object,), {})
    from vkconventions import VulkanConventions

    # This splits arguments which are space-separated lists
    args.feature = [name for arg in args.feature for name in arg.split()]
    args.extension = [name for arg in args.extension for name in arg.split()]

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
