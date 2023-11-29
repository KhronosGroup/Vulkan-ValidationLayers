#!/usr/bin/env python3
#
# Copyright (c) 2016-2023 Valve Corporation
# Copyright (c) 2016-2023 LunarG, Inc.
# Copyright (c) 2016-2022 Google Inc.
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
import subprocess
import struct
import re
import argparse
import common_ci

SPIRV_MAGIC = 0x07230203
COLUMNS = 10
INDENT = 4

def identifierize(s):
    # translate invalid chars
    s = re.sub("[^0-9a-zA-Z_]", "_", s)
    # translate leading digits
    return re.sub("^[^a-zA-Z_]+", "_", s)

def compile(filename, glslang_validator, spirv_opt, target_env):
    tmpfile = os.path.basename(filename) + '.tmp'

    # invoke glslangValidator
    try:
        args = [glslang_validator]

        if not target_env:
            requires_vulkan_1_2 = ['rgen']
            if filename.split(".")[-1] in requires_vulkan_1_2:
                target_env = "vulkan1.2"
        if target_env:
            args += ["--target-env", target_env]
        # functions called by the SPIRV-Tools instrumentation require special options
        if tmpfile.startswith("inst_"):
            args += ["--no-link", "--target-env"]
            if filename.endswith(".rgen"):
                args += ["vulkan1.2"]
            else:
                args += ["vulkan1.0"]
        else:
            args += ["-V"]
        args += ["-o", tmpfile, filename]
        subprocess.check_output(args, universal_newlines=True)
    except subprocess.CalledProcessError as e:
        raise Exception(e.output)

    # invoke spirv-opt
    try:
        args = [spirv_opt, tmpfile, '-o', tmpfile]

        # gpu_shaders_constants.h adds many constants not needed and it slows down linking time
        args += ['--eliminate-dead-const']
        # Runs some basic optimizations that don't touch CFG for goal of making linking functions smaller (and faster)
        args += ['--eliminate-local-single-block']
        args += ['--eliminate-local-single-store']
        args += ['--vector-dce']
        args += ['--simplify-instructions']
        args += ['--eliminate-dead-code-aggressive']

        subprocess.check_output(args, universal_newlines=True)
    except subprocess.CalledProcessError as e:
        raise Exception(e.output)

    # read the temp file into a list of SPIR-V words
    words = []
    with open(tmpfile, "rb") as f:
        data = f.read()
        assert(len(data) and len(data) % 4 == 0)

        # determine endianness
        fmt = ("<" if data[0] == (SPIRV_MAGIC & 0xff) else ">") + "I"
        for i in range(0, len(data), 4):
            words.append(struct.unpack(fmt, data[i:(i + 4)])[0])

        assert(words[0] == SPIRV_MAGIC)

    # remove temp file
    os.remove(tmpfile)

    return words

def write(words, filename, apiname, outdir = None):
    name = identifierize(os.path.basename(filename))

    literals = []
    for i in range(0, len(words), COLUMNS):
        columns = ["0x%08x" % word for word in words[i:(i + COLUMNS)]]
        literals.append(" " * INDENT + ", ".join(columns) + ",")

    header = """#include <stdint.h>
#pragma once

// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See generate_spirv.py for modifications

/***************************************************************************
*
* Copyright (c) 2021-2022 The Khronos Group Inc.
* Copyright (c) 2021-2023 Valve Corporation
* Copyright (c) 2021-2023 LunarG, Inc.
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

// To view SPIR-V, copy contents of array and paste in https://www.khronos.org/spir/visualizer/
static const uint32_t %s[%d] = {
%s
};
""" % (name, len(words), "\n".join(literals))

    if outdir:
      out_file = os.path.join(outdir, f'layers/{apiname}/generated')
    else:
      out_file = common_ci.RepoRelative(f'layers/{apiname}/generated')
    out_file = os.path.join(out_file, name + '.h')
    os.makedirs(os.path.dirname(out_file), exist_ok=True)
    with open(out_file, "w") as f:
        print(header, end="", file=f)

def write_inst_hash(outdir=None):
    shader_file = common_ci.RepoRelative('layers/gpu_shaders/inst_functions.comp')
    result = subprocess.run(["git", "hash-object", shader_file], capture_output=True, text=True)
    git_hash = result.stdout.rstrip('\n')

    try:
        int(git_hash, 16)
    except ValueError:
        raise ValueError(f'value for INST_SHADER_GIT_HASH ({git_hash}) must be a SHA1 hash.')
    if len(git_hash) != 40:
        raise ValueError(f'value for INST_SHADER_GIT_HASH ({git_hash}) must be a SHA1 hash.')

    out = []
    out.append(f'''
// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See {os.path.basename(__file__)} for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google Inc.
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
 ****************************************************************************/

#pragma once

''')

    out.append(f'#define INST_SHADER_GIT_HASH "{git_hash}"\n')

    if outdir:
      out_file = os.path.join(outdir, 'layers/vulkan/generated')
    else:
      out_file = common_ci.RepoRelative('layers/vulkan/generated')
    os.makedirs(out_file, exist_ok=True)
    out_file = os.path.join(out_file, "gpu_inst_shader_hash.h")
    with open(out_file, 'w') as outfile:
        outfile.write("".join(out))

def main():
    parser = argparse.ArgumentParser(description='Generate spirv code for this repository, see layers/gpu_shaders/README.md for more deatils')
    parser.add_argument('--api',
                        default='vulkan',
                        choices=['vulkan'],
                        help='Specify API name to generate')
    parser.add_argument('--shader', action='store', type=str, help='Input Filename')
    parser.add_argument('--glslang', action='store', type=str, help='Path to glslangValidator to use')
    parser.add_argument('--spirv-opt', action='store', dest='spirv_opt', type=str, help='Path to glslangValidator to use')
    parser.add_argument('--outdir', action='store', type=str, help='Optional path to output directory')
    parser.add_argument('--targetenv', action='store', type=str, help='Optional --target-env argument passed down to glslangValidator')
    args = parser.parse_args()

    generate_shaders = []
    if args.shader:
        if not os.path.isfile(args.shader):
            sys.exit("Cannot find infilename " + args.shader)
        generate_shaders.append(args.shader)
    else:
        # Get all shaders in gpu_shaders folder
        shader_type = ['vert', 'tesc', 'tese', 'geom', 'frag', 'comp', 'mesh', 'task', 'rgen', 'rint', 'rahit', 'rchit', 'rmiss', 'rcall']
        gpu_shaders = common_ci.RepoRelative('layers/gpu_shaders')
        for filename in os.listdir(gpu_shaders):
            if (filename.split(".")[-1] in shader_type):
                generate_shaders.append(os.path.join(gpu_shaders, filename))

    # default glslangValidator path
    glslang_validator =  common_ci.RepoRelative('external/glslang/build/install/bin/glslangValidator')
    if args.glslang:
        glslang_validator = args.glslang
    if not os.path.isfile(glslang_validator):
        sys.exit("Cannot find glslangValidator " + glslang_validator)

    # default spirv-opt path
    spirv_opt =  common_ci.RepoRelative('external/SPIRV-Tools/build/install/bin/spirv-opt')
    if args.spirv_opt:
        spirv_opt = args.spirv_opt
    if not os.path.isfile(spirv_opt):
        sys.exit("Cannot find spirv-opt " + spirv_opt)

    for shader in generate_shaders:
        words = compile(shader, glslang_validator, spirv_opt, args.targetenv)
        write(words, shader, args.api, args.outdir)
    write_inst_hash(args.outdir)

if __name__ == '__main__':
  main()
