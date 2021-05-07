#!/usr/bin/env python3
#
# Copyright (c) 2016-2021 Valve Corporation
# Copyright (c) 2016-2021 LunarG, Inc.
# Copyright (c) 2016-2021 Google Inc.
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

"""Compile GLSL to SPIR-V.

Depends on glslangValidator
"""

import os
import sys
import subprocess
import struct
import re
import argparse

SPIRV_MAGIC = 0x07230203
COLUMNS = 4
INDENT = 4

parser = argparse.ArgumentParser(description='Generate spirv code for this repository')
parser.add_argument('--outfilename', action='store', help='Output Filename')
parser.add_argument('infilename', action='store', type=str, help='Input Filename')
parser.add_argument('glslangvalidator', action='store', help='glslangvalidator')
args_in = parser.parse_args()

if not os.path.isfile(args_in.infilename):
    sys.exit("Cannot find infilename " + args_in.infilename)

if not os.path.isfile(args_in.glslangvalidator):
    sys.exit("Cannot find glslangvalidator " + args_in.glslangvalidator)

def identifierize(s):
    # translate invalid chars
    s = re.sub("[^0-9a-zA-Z_]", "_", s)
    # translate leading digits
    return re.sub("^[^a-zA-Z_]+", "_", s)

def compile(filename, tmpfile):
    # invoke glslangValidator
    try:
        args = [args_in.glslangvalidator, "-V", "-H", "-o", tmpfile, filename]
        output = subprocess.check_output(args, universal_newlines=True)
    except subprocess.CalledProcessError as e:
        raise(e.output)

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

    return (words, output.rstrip())

base = os.path.basename(args_in.infilename)
words, comments = compile(args_in.infilename, base + ".tmp")

literals = []
for i in range(0, len(words), COLUMNS):
    columns = ["0x%08x" % word for word in words[i:(i + COLUMNS)]]
    literals.append(" " * INDENT + ", ".join(columns) + ",")

header = """#include <stdint.h>
#pragma once

// This file is ***GENERATED***.  Do Not Edit.
/* Copyright (c) 2021 The Khronos Group Inc.
 * Copyright (c) 2021 Valve Corporation
 * Copyright (c) 2021 LunarG, Inc.
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
 * Author: Tony Barbour <tony@lunarg.com>
 */

#if 0
%s
#endif

static const uint32_t %s[%d] = {
%s
};
""" % (comments, identifierize(base), len(words), "\n".join(literals))

if args_in.outfilename:
    with open(args_in.outfilename, "w") as f:
        print(header, end="", file=f)
else:
        print(header, end="")
