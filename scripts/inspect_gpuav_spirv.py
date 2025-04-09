#!/usr/bin/env python3
# Copyright (c) 2025 The Khronos Group Inc.
# Copyright (c) 2025 Valve Corporation
# Copyright (c) 2025 LunarG, Inc.
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
import os
import re
import sys
import struct
import numpy as np
from collections import defaultdict

def PrintExecutionModels(execution_models: set):
    names = []
    for e in execution_models:
        if e == 0: # Maps to ExecutionModelVertex
            names.append('Vertex')
        elif e == 1:
            names.append('TessControl')
        elif e == 2:
            names.append('TessEval')
        elif e == 3:
            names.append('Geometry')
        elif e == 4:
            names.append('Fragment')
        elif e == 5:
            names.append('Compute')
        elif e == 6:
            names.append('Kernel') # not allowed
        elif e == 5267:
            names.append('TaskNV')
        elif e == 5268:
            names.append('MeshNV')
        elif e == 5364:
            names.append('TaskEXT')
        elif e == 5365:
            names.append('MeshEXT')
        elif e == 5313:
            names.append("RayGenerationKHR")
        elif e == 5314:
            names.append("IntersectionKHR")
        elif e == 5315:
            names.append("AnyHitKHR")
        elif e == 5316:
            names.append("ClosestHitKHR")
        elif e == 5317:
            names.append("MissKHR")
        elif e == 5318:
            names.append("CallableKHR")
        else:
            print("ERROR - Stage not found")
    return ",".join(names)

def ExtractShaderId(path: str):
    match = re.search(r'dump_(\d+)_after\.spv', path)
    return int(match.group(1)) if match else None

def ParseSpirv(file: str, stats_collector: dict):
    with open(file, "rb") as f:
        # Read the whole binary file as uint32 words
        words = list(struct.unpack(f"{len(f.read()) // 4}I", f.seek(0) or f.read()))

    name_dict = dict()
    function_call_count = defaultdict(int)
    execution_models = set()  # Use a set to store unique numbers

    offset = 5  # First 5 words are the header

    while offset < len(words):
        instruction = words[offset]
        length = instruction >> 16
        opcode = instruction & 0xFFFF

        if opcode == 5: # OpName
            target_id = words[offset + 1]
            raw_string = words[offset + 2 : offset + length]
            byte_data = b''.join(struct.pack("I", word) for word in raw_string)
            name = byte_data.split(b'\x00', 1)[0].decode("utf-8")
            name_dict[target_id] = name

        elif opcode == 15: # OpEntryPoint
            execution_model = words[offset + 1]
            execution_models.add(execution_model)

        elif opcode == 57: # OpFunctionCall
            function_id = words[offset + 3]
            function_call_count[function_id] += 1

        offset += length  # Move to the next instruction

    print(f'Shader Id {ExtractShaderId(file)} ({PrintExecutionModels(execution_models)})')
    for function_id, count in function_call_count.items():
        # Non-instrumented functions might not have a name
        if function_id in name_dict:
            function_name = name_dict[function_id]
            if function_name.startswith('inst_'):
                print(f"  {function_name}: {count}")
                # Store count per function to later do statistics
                if function_name not in stats_collector:
                    stats_collector[function_name] = []
                stats_collector[function_name].append(count)

def print_statistics(stats_collector: dict):
    print("\n=== Function Call Statistics ===")
    for function_name, counts in stats_collector.items():            
        counts_array = np.array(counts)
        print(f"\nFunction: {function_name}")
        print(f"  Count: {len(counts)}")
        print(f"  Min: {np.min(counts_array)}")
        print(f"  Max: {np.max(counts_array)}")
        print(f"  Mean: {np.mean(counts_array):.2f}")
        print(f"  Median: {np.median(counts_array):.2f}")
        print(f"  75th percentile: {np.percentile(counts_array, 75):.2f}")
        print(f"  95th percentile: {np.percentile(counts_array, 95):.2f}")

def main(argv):
    parser = argparse.ArgumentParser(description='Get info about a GPU-AV instrumented SPIR-V dump')
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument('--files', nargs='+', help='List of files to inspect')
    group.add_argument('--dir', help='Pass in a directory path that contains all the dumped files.', dest='directory')
    args = parser.parse_args(argv)

    spirv_list = []
    if args.directory:
        after_files = []
        pattern = re.compile(r'dump_(\d+)_after')
        for root, _, files in os.walk(args.directory):
            for file in files:
                if pattern.search(file):
                    full_path = os.path.join(root, file)
                    after_files.append(full_path)

        spirv_list = sorted(after_files, key=ExtractShaderId)
    else:
        spirv_list = args.files

    stats_collector = defaultdict(list)
    
    for file in spirv_list:
        ParseSpirv(file, stats_collector)
    
    print_statistics(stats_collector)

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
