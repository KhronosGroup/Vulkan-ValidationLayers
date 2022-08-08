#!/usr/bin/env python3
# Copyright (c) 2021-2022 The Khronos Group Inc.
# Copyright (c) 2021-2022 Valve Corporation
# Copyright (c) 2021-2022 LunarG, Inc.
# Copyright (c) 2021-2022 Google Inc.
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
# Author: Mike Schuchardt <mikes@lunarg.com>

import argparse
import filecmp
import os
import shutil
import subprocess
import sys
import tempfile
import difflib

import common_codegen

# files to exclude from --verify check
verify_exclude = ['.clang-format']

def main(argv):
    parser = argparse.ArgumentParser(description='Generate source code for this repository')
    parser.add_argument('registry', metavar='REGISTRY_PATH', help='path to the Vulkan-Headers registry directory')
    parser.add_argument('grammar', metavar='GRAMMAR_PATH', help='path to the SPIRV-Headers grammar directory')
    group = parser.add_mutually_exclusive_group()
    group.add_argument('-i', '--incremental', action='store_true', help='only update repo files that change')
    group.add_argument('-v', '--verify', action='store_true', help='verify repo files match generator output')
    args = parser.parse_args(argv)

    gen_cmds = [*[[common_codegen.repo_relative('scripts/lvl_genvk.py'),
                   '-registry', os.path.abspath(os.path.join(args.registry,  'vk.xml')),
                   '-grammar', os.path.abspath(os.path.join(args.grammar,  'spirv.core.grammar.json')),
                   '-quiet',
                   filename] for filename in ["chassis.cpp",
                                              "chassis.h",
                                              "chassis_dispatch_helper.h",
                                              "layer_chassis_dispatch.cpp",
                                              "layer_chassis_dispatch.h",
                                              "object_tracker.cpp",
                                              "object_tracker.h",
                                              "parameter_validation.cpp",
                                              "parameter_validation.h",
                                              "synchronization_validation_types.cpp",
                                              "synchronization_validation_types.h",
                                              "thread_safety.cpp",
                                              "thread_safety.h",
                                              "vk_dispatch_table_helper.h",
                                              "vk_enum_string_helper.h",
                                              "vk_extension_helper.h",
                                              "vk_layer_dispatch_table.h",
                                              "vk_object_types.h",
                                              "vk_safe_struct.cpp",
                                              "vk_safe_struct.h",
                                              "lvt_function_pointers.cpp",
                                              "lvt_function_pointers.h",
                                              "vk_typemap_helper.h",
                                              "best_practices.h",
                                              "best_practices.cpp",
                                              "spirv_validation_helper.cpp",
                                              "spirv_grammar_helper.cpp",
                                              "spirv_grammar_helper.h",
                                              "command_validation.cpp",
                                              "command_validation.h",
                                              "vk_format_utils.cpp",
                                              "vk_format_utils.h",
                                              "corechecks_optick_instrumentation.cpp",
                                              "corechecks_optick_instrumentation.h"]],
                [common_codegen.repo_relative('scripts/vk_validation_stats.py'),
                 os.path.abspath(os.path.join(args.registry, 'validusage.json')),
                 '-export_header'],
                [common_codegen.repo_relative('scripts/external_revision_generator.py'),
                 '--json_file', common_codegen.repo_relative('scripts/known_good.json'),
                 '--json_keys', 'repos,3,commit',
                 '-s', 'SPIRV_TOOLS_COMMIT_ID',
                 '-o', 'spirv_tools_commit_id.h']]

    repo_dir = common_codegen.repo_relative('layers/generated')

    # get directory where generators will run
    if args.verify or args.incremental:
        # generate in temp directory so we can compare or copy later
        temp_obj = tempfile.TemporaryDirectory(prefix='VulkanVL_generated_source_')
        temp_dir = temp_obj.name
        gen_dir = temp_dir
    else:
        # generate directly in the repo
        gen_dir = repo_dir

    # run each code generator
    for cmd in gen_cmds:
        print(' '.join(cmd))
        try:
            subprocess.check_call([sys.executable] + cmd, cwd=gen_dir)
        except Exception as e:
            print('ERROR:', str(e))
            return 1

    # optional post-generation steps
    if args.verify:
        # compare contents of temp dir and repo
        temp_files = set(os.listdir(temp_dir))
        repo_files = set(os.listdir(repo_dir))
        files_match = True
        for filename in sorted((temp_files | repo_files) - set(verify_exclude)):
            temp_filename = os.path.join(temp_dir, filename)
            repo_filename = os.path.join(repo_dir, filename)
            if filename.startswith('gpu_'):
                # The shaders requires glslangvalidator,
                # so updated manually with generate_spirv when needed
                continue
            elif filename not in repo_files:
                print('ERROR: Missing repo file', filename)
                files_match = False
            elif filename not in temp_files:
                print('ERROR: Missing generator for', filename)
                files_match = False
            elif not filecmp.cmp(temp_filename, repo_filename, shallow=False):
                print('ERROR: Repo files do not match generator output for', filename)
                files_match = False
                # print line diff on file mismatch
                with open(temp_filename) as temp_file, open(repo_filename) as repo_file:
                    print(''.join(difflib.unified_diff(temp_file.readlines(),
                                                       repo_file.readlines(),
                                                       fromfile='temp/' + filename,
                                                       tofile=  'repo/' + filename)))

        # return code for test scripts
        if files_match:
            print('SUCCESS: Repo files match generator output')
            return 0
        return 1

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

