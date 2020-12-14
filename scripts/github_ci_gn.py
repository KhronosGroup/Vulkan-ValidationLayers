#!/usr/bin/env python3
# Copyright (c) 2020 Valve Corporation
# Copyright (c) 2020 LunarG, Inc.

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

# Script to determine if source code in Pull Request is properly formatted.

import os
import argparse
import shutil
import subprocess
import sys
import platform
import common_codegen
from argparse import RawDescriptionHelpFormatter

os.system("")

VERBOSE = False
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.split(os.path.abspath(__file__))[0], '..'))

# Split command lines into a list of args for python subprocess
def ListArgs(command_string):
    return command_string.split(" ")

# Runs a command in a directory and returns its standard output stream.
#    Captures the standard error stream and prints it if error.
#    Raises a RuntimeError if the command fails to launch or otherwise fails.
def command_output(cmd, directory):
    if VERBOSE:
        print('In {d}: {cmd}'.format(d=directory, cmd=cmd))
    p = subprocess.Popen(
        cmd, cwd=directory, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    (stdout, stderr) = p.communicate()
    #if p.returncode != 0:
        #print('*** Error ***\nstderr contents:\n{}'.format(stderr))
        #if not fail_ok:
        #    raise RuntimeError('Failed to run {} in {}'.format(cmd, directory))
    if VERBOSE:
        print(stdout)
    return p.returncode, stdout

#
# Fetch Android components, build Android VVL
def BuildGn():
    clone_cmd = 'git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git depot_tools'
    ret_code, out_text = command_output(ListArgs(clone_cmd), PROJECT_ROOT)
    os.environ['PATH'] = os.environ.get('PATH') + ":" + common_codegen.repo_relative("depot_tools")
    print("Cloning Ninja depot_tools\n", out_text.decode())

    if ret_code == 0:
        # Run GN build update dependencies script
        update_cmd = './build-gn/update_deps.sh'
        ret_code, out_text = command_output(ListArgs(update_cmd), PROJECT_ROOT)
        print("Updating Ninja build dependencies\n", out_text.decode())

    if ret_code == 0:
        # Gen GN files
        # TODO: Enable ccache for Ninja builds : --args="cc_wrapper=\"ccache\""
        #       There are issues passing this on the command line.
        #       Possible solution is to write/append this line to the out/Debug/args.gn file
        gn_gen_cmd = 'gn gen out/Debug'
        ret_code, out_text = command_output(ListArgs(gn_gen_cmd), PROJECT_ROOT)
        print("Generating Ninja Files\n", out_text.decode())

    if ret_code == 0:
        # GN Build
        ninja_build_cmd = 'ninja -C out/Debug'
        ret_code, out_text = command_output(ListArgs(ninja_build_cmd), PROJECT_ROOT)
        print("Running Ninja Build\n", out_text.decode())

    if ret_code != 0:
        sys.exit(ret_code)
    return 0

#
# Module Entrypoint
def main():
    parser = argparse.ArgumentParser(description='''Usage: python3 ./scripts/github_ci_gn.py
    - Reqires python3
    - Run script in repo root
    ''', formatter_class=RawDescriptionHelpFormatter)

    if sys.version_info[0] != 3:
        print("This script requires Python 3. Run script with [-h] option for more details.")
        exit()

    failed = BuildGn()

    if failed:
        exit(1)
    else:
        exit(0)

if __name__ == '__main__':
  main()
