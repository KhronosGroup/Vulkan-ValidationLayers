#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2017, 2019-2020 The Khronos Group Inc.
# Copyright (c) 2015-2017, 2019-2020 Valve Corporation
# Copyright (c) 2015-2017, 2019-2020 LunarG, Inc.
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
# Author: Mark Lobodzinski <mark@lunarg.com>

import os,re,sys,string
import shutil
import subprocess
import platform
import xml.etree.ElementTree as etree
from collections import namedtuple, OrderedDict

# helper to define paths relative to the repo root
def repo_relative(path):
    return os.path.abspath(os.path.join(os.path.dirname(__file__), '..', path))

# Runs a command in a directory and returns its return code.
#    Captures the standard error stream and prints it if error.
def RunShellCmd(command, directory):
    print("Command: ", command)
    cmd_list = command.split(" ")
    p = subprocess.Popen(cmd_list, cwd=directory, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    (stdout, stderr) = p.communicate()  # Forces a wait
    if p.returncode != 0:
        print('Error -- stderr contents:\n', stderr.decode())
    else:
        print(stdout.decode())
    return p.returncode

