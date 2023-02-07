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

import common_codegen

SPIRV_MAGIC = 0x07230203
COLUMNS = 4
INDENT = 4

def identifierize(s):
    # translate invalid chars
    s = re.sub("[^0-9a-zA-Z_]", "_", s)
    # translate leading digits
    return re.sub("^[^a-zA-Z_]+", "_", s)

def compile(filename, glslang_validator):
    tmpfile = os.path.basename(filename) + '.tmp'

    # invoke glslangValidator
    try:
        args = [glslang_validator, "-V", "-H", "-o", tmpfile, filename]
        output = subprocess.check_output(args, universal_newlines=True)
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

    # The disassembled SPIR-V first line has file path to the shader
    # Because this might be absolute on the system, remove it
    return (words, output.rstrip()[output.index('\n') + 1:])

def write(words, disassembled, filename, outfilename = None):
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

// disassembled SPIR-V
#if 0
%s
#endif

static const uint32_t %s[%d] = {
%s
};
""" % (disassembled, name, len(words), "\n".join(literals))

    if outfilename:
      out_file = outfilename
    else:
      out_file = os.path.join(common_codegen.repo_relative('layers/generated'), name + '.h')
    os.makedirs(os.path.dirname(out_file), exist_ok=True)
    with open(out_file, "w") as f:
        print(header, end="", file=f)

def main():
    parser = argparse.ArgumentParser(description='Generate spirv code for this repository, see layers/gpu_shaders/README.md for more deatils')
    parser.add_argument('--shader', action='store', type=str, help='Input Filename')
    parser.add_argument('--glslang', action='store', type=str, help='Path to glslangvalidator to use')
    parser.add_argument('--outfilename', action='store', type=str, help='Optional path to output file')
    args = parser.parse_args()

    generate_shaders = []
    if args.shader:
        if not os.path.isfile(args.shader):
            sys.exit("Cannot find infilename " + args.shader)
        generate_shaders.append(args.shader)
    else:
        # Get all shaders in gpu_shaders folder
        shader_type = ['vert', 'tesc', 'tese', 'geom', 'frag', 'comp', 'mesh', 'task', 'rgen', 'rint', 'rahit', 'rchit', 'rmiss', 'rcall']
        gpu_shaders = common_codegen.repo_relative('layers/gpu_shaders')
        for filename in os.listdir(gpu_shaders):
            if (filename.split(".")[-1] in shader_type):
                generate_shaders.append(os.path.join(gpu_shaders, filename))

    # default glslangValidator path
    glslang_validator =  common_codegen.repo_relative('external/glslang/build/install/bin/glslangValidator')
    if args.glslang:
        glslang_validator = args.glslang
    if not os.path.isfile(glslang_validator):
        sys.exit("Cannot find glslangValidator " + glslang_validator)

    for shader in generate_shaders:
        words, disassembled = compile(shader, glslang_validator)
        write(words, disassembled, shader, args.outfilename)

if __name__ == '__main__':
  main()
