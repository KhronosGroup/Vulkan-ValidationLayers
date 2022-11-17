#!/usr/bin/env python3
#
# Copyright (c) 2022 Google Inc.
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
import glob
import os
import sys

parser = argparse.ArgumentParser()
parser.add_argument("directory")
parser.add_argument("build")
parser.add_argument("stamp")
parser.add_argument("depfile")

args = parser.parse_args()

extensions = ["cpp", "h"]
subdirs = ["", "positive"]

cwd = os.getcwd()
os.chdir(args.directory)
files = set()
for ext in extensions:
    for subdir in subdirs:
        if subdir != "":
            subdir += "/"
        files = files.union(set(glob.glob(subdir + "*." + ext)))
os.chdir(cwd)

with open(args.build, "r") as build_file:
    success = True
    contents = build_file.read()
    for file in files:
        if file not in contents:
            print("File", file, "not included in ", args.build)
            success = False
    if not success:
        sys.exit(1)

with open(args.depfile, "w") as depfile:
    depfile_contents = args.stamp + ": " + " ".join(
        os.path.join(args.directory, x) for x in files)
    depfile.write(depfile_contents)

with open(args.stamp, "w") as stamp:
    stamp.write("1")
