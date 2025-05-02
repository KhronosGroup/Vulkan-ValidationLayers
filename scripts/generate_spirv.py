#!/usr/bin/env python3
#
# Copyright (c) 2016-2025 Valve Corporation
# Copyright (c) 2016-2025 LunarG, Inc.
# Copyright (c) 2016-2025 Google Inc.
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
# Compile GLSL to SPIR-V. Depends on glslangValidator

import os
import sys
import shutil
import subprocess
import struct
import re
import argparse
import common_ci
from concurrent.futures import ThreadPoolExecutor
from collections import namedtuple

SPIRV_MAGIC = 0x07230203
COLUMNS = 10
INDENT = 4

# Structure to hold processed shader data
ShaderData = namedtuple("ShaderData", ["name", "words", "function_offsets", "filename"])

def identifierize(s):
    # translate invalid chars
    s = re.sub("[^0-9a-zA-Z_]", "_", s)
    # translate leading digits
    return re.sub("^[^a-zA-Z_]+", "_", s)

def compile_shader(gpu_shaders_dir, filename, glslang_validator, spirv_opt, target_env):
    tmpfile = os.path.basename(filename) + '.tmp'

    # invoke glslangValidator
    try:
        args = [glslang_validator]

        if not target_env:
            requires_vulkan_1_2 = ['rgen', 'rint', 'rahit', 'rchit', 'rmiss', 'rcall']
            if filename.split(".")[-1] in requires_vulkan_1_2:
                target_env = "vulkan1.2"
            elif filename.find('instrumentation') != -1:
                target_env = "vulkan1.1" # Otherwise glslang might create BufferBlocks
            else:
                target_env = "vulkan1.0"
        if target_env:
            args += ["--target-env", target_env]
        # functions called by the SPIRV-Tools instrumentation require special options
        if filename.find('instrumentation') != -1:
            args += ["--no-link"]
        else:
            args += ["-V"]
        include_dir = "-I" + gpu_shaders_dir
        args += [include_dir, "-o", tmpfile, filename]
        subprocess.check_output(args, universal_newlines=True)
    except subprocess.CalledProcessError as e:
       print(f"Error compiling shader:")
       print(e.output, file=sys.stderr)
       if os.path.exists(tmpfile):
            os.remove(tmpfile)
       raise Exception(e.output) # Re-raise to propagate the error message

    # invoke spirv-opt
    try:
        args = [spirv_opt, tmpfile, '-o', tmpfile]

        # gpuav_shaders_constants.h adds many constants not needed and it slows down linking time
        args += ['--eliminate-dead-const']
        # Runs some basic optimizations that don't touch CFG for goal of making linking functions smaller (and faster)
        args += ['--eliminate-local-single-block']
        args += ['--eliminate-local-single-store']
        args += ['--vector-dce']
        args += ['--simplify-instructions']
        args += ['--eliminate-dead-code-aggressive']

        subprocess.check_output(args, universal_newlines=True)
    except subprocess.CalledProcessError as e:
        print(f"Error optimizing {filename} with spirv-opt:", file=sys.stderr)
        print(e.output, file=sys.stderr)
        if os.path.exists(tmpfile):
            os.remove(tmpfile)
        raise Exception(e.output) # Re-raise to propagate the error message

    # read the temp file into a list of SPIR-V words
    words = []
    try:
        with open(tmpfile, "rb") as f:
            data = f.read()
            assert(len(data) and len(data) % 4 == 0)

            # determine endianness
            fmt = ("<" if data[0] == (SPIRV_MAGIC & 0xff) else ">") + "I"
            for i in range(0, len(data), 4):
                words.append(struct.unpack(fmt, data[i:(i + 4)])[0])

            assert(words[0] == SPIRV_MAGIC)
    finally:
        if os.path.exists(tmpfile):
            os.remove(tmpfile)

    return words

def process_shader(shader_filename, gpu_shaders_dir, glslang, spirv_opt, targetenv):
    words = compile_shader(gpu_shaders_dir, shader_filename, glslang, spirv_opt, targetenv)

    head_tail = os.path.split(shader_filename)
    name = os.path.basename(head_tail[0]) + "_" + head_tail[1]
    name = identifierize(name)

    function_offsets = []
    if "instrumentation" in shader_filename:
        offset = 5  # First 5 words are the header
        while offset < len(words):
            instruction = words[offset]
            length = instruction >> 16
            opcode = instruction & 0xFFFF
            if opcode == 54:  # OpFunction
                function_offsets.append(offset)
            offset += length  # Move to the next instruction

    return ShaderData(name=name, words=words, function_offsets=function_offsets, filename=shader_filename)


def write_aggregate_files(shader_data_list, apiname, outdir):
    output_basename = "gpuav_offline_spirv"
    header_content = []
    source_content = []

    file_header = f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See {os.path.basename(__file__)} for modifications

/***************************************************************************
 *
 * Copyright (c) 2021-2025 The Khronos Group Inc.
 * Copyright (c) 2021-2025 Valve Corporation
 * Copyright (c) 2021-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ****************************************************************************/
'''

    header_content.append(file_header)
    header_content.append("#pragma once\n")
    header_content.append("#include <cstdint>\n")
    header_content.append("// We have found having spirv code defined the header can lead to MSVC not recognizing changes\n")

    # Sort for consistent output order
    shader_data_list.sort(key=lambda x: x.name)

    for data in shader_data_list:
        header_content.append(f"extern const uint32_t {data.name}_size;")
        header_content.append(f"extern const uint32_t {data.name}[];")
        if data.function_offsets:
            header_content.append("// These offset match the function in the order they are declared in the GLSL source")
            for index, offset in enumerate(data.function_offsets):
                header_content.append(f'extern const uint32_t {data.name}_function_{index}_offset;')
        header_content.append("") # Add a newline for readability

    source_content.append(file_header)
    source_content.append(f'#include "{output_basename}.h"\n')
    source_content.append("// To view SPIR-V, copy contents of an array and paste in https://www.khronos.org/spir/visualizer/\n")

    for data in shader_data_list:
        literals = []
        for i in range(0, len(data.words), COLUMNS):
            columns = ["0x%08x" % word for word in data.words[i:(i + COLUMNS)]]
            literals.append(" " * INDENT + ", ".join(columns) + ",")
        literals_str = "\n".join(literals)
        if literals_str.endswith(','):
            literals_str = literals_str[:-1]

        source_content.append(f"[[maybe_unused]] const uint32_t {data.name}_size = {len(data.words)};")
        source_content.append(f"[[maybe_unused]] const uint32_t {data.name}[{len(data.words)}] = {{\n{literals_str}}};")

        if data.function_offsets:
            for index, offset in enumerate(data.function_offsets):
                source_content.append(f'[[maybe_unused]] const uint32_t {data.name}_function_{index}_offset = {offset};')
        source_content.append("")

    if outdir:
      out_file_dir = os.path.join(outdir, f'layers/{apiname}/generated')
    else:
      out_file_dir = common_ci.RepoRelative(f'layers/{apiname}/generated')

    os.makedirs(out_file_dir, exist_ok=True)

    out_file_header = os.path.join(out_file_dir, output_basename + '.h')
    out_file_source = os.path.join(out_file_dir, output_basename + '.cpp')

    with open(out_file_header, "w") as f:
        f.write("\n".join(header_content))

    with open(out_file_source, "w") as f:
        f.write("\n".join(source_content))


def main():
    parser = argparse.ArgumentParser(description='Generate single aggregated spirv header/source for GPU-AV shaders.')
    parser.add_argument('--api',
                        default='vulkan',
                        choices=['vulkan'],
                        help='Specify API name (used for output directory structure)')
    parser.add_argument('--glslang', action='store', type=str, help='Path to glslangValidator to use')
    parser.add_argument('--spirv-opt', action='store', dest='spirv_opt', type=str, help='Path to spirv-opt to use')
    parser.add_argument('--outdir', action='store', type=str, help='Optional path to output directory root')
    parser.add_argument('--targetenv', action='store', type=str, help='Optional --target-env argument passed down to glslangValidator')
    parser.add_argument('--single-thread', action='store_true', help='Run compilation on a single thread (for debugging)')
    args = parser.parse_args()

    shaders_to_compile = []
    shader_type = ['vert', 'tesc', 'tese', 'geom', 'frag', 'comp', 'mesh', 'task', 'rgen', 'rint', 'rahit', 'rchit', 'rmiss', 'rcall']
    try:
        gpu_shaders_dir = common_ci.RepoRelative('layers/gpuav/shaders')
        diagnostic_shaders_dir = common_ci.RepoRelative('layers/gpuav/shaders/validation_cmd')
        instrumentation_shaders_dir = common_ci.RepoRelative('layers/gpuav/shaders/instrumentation')

        for dir_path in [diagnostic_shaders_dir, instrumentation_shaders_dir]:
             if not os.path.isdir(dir_path):
                 print(f"Warning: Shader directory not found: {dir_path}", file=sys.stderr)
                 continue
             for filename in os.listdir(dir_path):
                if filename.split(".")[-1].lower() in shader_type: # Use lower() for case-insensitivity
                    shaders_to_compile.append(os.path.join(dir_path, filename))

    except Exception as e:
         sys.exit(f"Error finding shader directories: {e}")

    if not shaders_to_compile:
        sys.exit("No shader files found to compile.")

    external_dir = None
    for path_suffix in ['external/Debug/64', 'external/Release/64', 'external']:
        try:
            potential_dir = common_ci.RepoRelative(path_suffix)
            if os.path.isdir(potential_dir):
                external_dir = potential_dir
                break
        except Exception:
             continue

    if not external_dir:
         print("Warning: Could not automatically determine external tools directory.", file=sys.stderr)

    # default glslangValidator path
    glslang = common_ci.RepoRelative(os.path.join(external_dir, 'glslang/build/install/bin/glslang'))
    if args.glslang:
        glslang = args.glslang
    if not shutil.which(glslang):
        sys.exit("Cannot find glslangValidator " + glslang)


    # default spirv-opt path
    spirv_opt =  common_ci.RepoRelative(os.path.join(external_dir, 'SPIRV-Tools/build/install/bin/spirv-opt'))
    if args.spirv_opt:
        spirv_opt = args.spirv_opt
    if not shutil.which(spirv_opt):
        sys.exit("Cannot find spirv-opt " + spirv_opt)
    
    # compile shaders
    compiled_shader_data = []
    compilation_failed = False

    if len(shaders_to_compile) == 1 or args.single_thread:
        for shader in shaders_to_compile:
            try:
                result = process_shader(shader, gpu_shaders_dir, glslang, spirv_opt, args.targetenv)
                compiled_shader_data.append(result)
            except Exception:
                compilation_failed = True
    else:
        # glslang will take almost 1 second per shader on Windows (likely due to File I/O issues)
        # We multi-thread the list of |shaders_to_compile| to help speed things up
        with ThreadPoolExecutor() as executor:
            futures = {executor.submit(process_shader, shader, gpu_shaders_dir, glslang, spirv_opt, args.targetenv): shader for shader in shaders_to_compile}

            for future in futures:
                shader = futures[future]
                try:
                    result = future.result() # This will raise exceptions if any occurred inside process_shader
                    compiled_shader_data.append(result)
                except Exception:
                    compilation_failed = True

    if compilation_failed:
         sys.exit("One or more shaders failed to compile. Aggregated files will not be generated.")

    write_aggregate_files(compiled_shader_data, args.api, args.outdir)


if __name__ == '__main__':
  main()
