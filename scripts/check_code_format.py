#!/usr/bin/env python
# Copyright (c) 2020-2022 Valve Corporation
# Copyright (c) 2020-2022 LunarG, Inc.

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
# Author: Mike Schuchardt <mikes@lunarg.com>
# Author: Nathaniel Cesario <nathaniel@lunarg.com>
# Author: Karl Schultz <karl@lunarg.com>
# Author: Tony Barbour <tony@lunarg.com>

# Script to determine if source code in Pull Request is properly formatted.
#
# This script checks for:
#   -- clang-format errors in the PR source code
#   -- out-of-date copyrights in PR source files
#   -- improperly formatted commit messages (using the function above)
#   -- assigning stype instead of using LvlInitStruct
#
# Notes:
#    Exits with non 0 exit code if formatting is needed.
#    Requires python3 to run correctly
#    In standalone mode (outside of CI), changes must be rebased on master
#        to get meaningful and complete results

import os
import argparse
import difflib
import re
import subprocess
import sys
from subprocess import check_output
from datetime import date
from argparse import RawDescriptionHelpFormatter

os.system("")
#
#
# Color print routine, takes a string matching a txtcolor above and the output string, resets color upon exit
def CPrint(msg_type, msg_string):
    color = '\033[0m'
    txtcolors = {'HELP_MSG':    '\033[0;36m',
                 'SUCCESS_MSG': '\033[1;32m',
                 'CONTENT':     '\033[1;39m',
                 'ERR_MSG':     '\033[1;31m',
                 'NO_COLOR':    '\033[0m'}
    print(txtcolors.get(msg_type, txtcolors['NO_COLOR']) + msg_string + txtcolors['NO_COLOR'])
#
#
# Check clang-formatting of source code diff
def VerifyClangFormatSource(target_refspec, target_files):
    CPrint('', "\nChecking PR source code for clang-format errors:")
    retval = 0
    good_file_pattern = re.compile('.*\\.(cpp|cc|c\+\+|cxx|c|h|hpp)$')
    diff_files_list = [item for item in target_files if good_file_pattern.search(item)]
    diff_files = ' '.join([str(elem) for elem in diff_files_list])
    retval = 0
    if diff_files != '':
        git_diff = subprocess.Popen(('git', 'diff', '-U0', target_refspec, '--', diff_files), stdout=subprocess.PIPE)
        diff_files_data = subprocess.check_output(('python3', './scripts/clang-format-diff.py', '-p1', '-style=file'), stdin=git_diff.stdout)
        diff_files_data = diff_files_data.decode('utf-8')
        if diff_files_data != '':
            CPrint('ERR_MSG', "\nFound formatting errors!")
            CPrint('CONTENT', "\n" + diff_files_data)
            retval = 1
    else:
        CPrint('SUCCESS_MSG', "\nThe modified source code in PR has been properly clang-formatted.\n\n")
    return retval
#
#
# Check copyright dates for modified files
def VerifyCopyrights(target_refspec, target_files):
    CPrint('', "\nChecking PR source files for correct copyright information:")
    retval = 0
    current_year = str(date.today().year)
    for file in target_files:
        if file is None or not os.path.isfile(file):
            continue
        copyright_match = re.search('Copyright (.)*LunarG', open(file, encoding="utf-8", errors='ignore').read(1024))
        if copyright_match and current_year not in copyright_match.group(0):
            CPrint('ERR_MSG', '\n' + file + " has an out-of-date copyright notice.")
            retval = 1;
    if retval == 0:
        CPrint('SUCCESS_MSG', "\nThe modified source files have correct copyright dates.\n\n")
    return retval
#
#
# Check commit message formats for commits in this PR/Branch
def VerifyCommitMessageFormat(target_refspec, target_files):
    CPrint('', "\nChecking PR commit messages for consistency issues:")
    retval = 0

    # Construct correct commit list
    pr_commit_range_parms = ['git', 'log', '--no-merges', '--left-only', target_refspec, '--pretty=format:"XXXNEWLINEXXX"%n%B']

    commit_data = check_output(pr_commit_range_parms)
    commit_text = commit_data.decode('utf-8')
    if commit_text is None:
        CPrint('SUCCESS_MSG', "\nNo commit messages were found for format checks.\n")
        return retval

    msg_cur_line = 0
    msg_prev_line = ''
    for msg_line_text in commit_text.splitlines():
        msg_cur_line += 1
        if 'XXXNEWLINEXXX' in msg_line_text:
            msg_cur_line = 0
        line_length = len(msg_line_text)

        if msg_cur_line == 1:
            # Enforce subject line must be 64 chars or less
            if line_length > 64:
                CPrint('ERR_MSG', "The following subject line exceeds 64 characters in length.")
                CPrint('CONTENT', "     '" + msg_line_text + "'\n")
                retval = 1
            # Output error if last char of subject line is not alpha-numeric
            if msg_line_text[-1] in '.,':
                CPrint('ERR_MSG', "For the following commit, the last character of the subject line must not be a period or comma.")
                CPrint('CONTENT', "     '" + msg_line_text + "'\n")
                retval = 1
            # Output error if subject line doesn't start with 'module: '
            if 'Revert' not in msg_line_text:
                module_name = msg_line_text.split(' ')[0]
                if module_name[-1] != ':':
                    CPrint('ERR_MSG', "The following subject line must start with a single word specifying the functional area of the change, followed by a colon and space.")
                    CPrint('ERR_MSG', "e.g., 'layers: Subject line here' or 'corechecks: Fix off-by-one error in ValidateFences'.")
                    CPrint('ERR_MSG', "Other common module names include layers, build, cmake, tests, docs, scripts, stateless, gpu, syncval, practices, etc.")
                    CPrint('CONTENT', "     '" + msg_line_text + "'\n")
                    retval = 1
                else:
                    # Check if first character after the colon is lower-case
                    subject_body = msg_line_text.split(': ')[1]
                    if not subject_body[0].isupper():
                        CPrint('ERR_MSG', "The first word of the subject line after the ':' character must be capitalized.")
                        CPrint('CONTENT', "     '" + msg_line_text + "'\n")
                        retval = 1
            # Check that first character of subject line is not capitalized
            if msg_line_text[0].isupper():
                CPrint('ERR_MSG', "The first word of the subject line must be lower case.")
                CPrint('CONTENT', "     '" + msg_line_text + "'\n")
                retval = 1
        elif msg_cur_line == 2:
            # Commit message must have a blank line between subject and body
            if line_length != 0:
                CPrint('ERR_MSG', "The following subject line must be followed by a blank line.")
                CPrint('CONTENT', "     '" + msg_prev_line + "'\n")
                retval = 1
        else:
            # Lines in a commit message body must be less than 72 characters in length (but give some slack)
            if line_length > 76:
                CPrint('ERR_MSG', "The following commit message body line exceeds the 72 character limit.")
                CPrint('CONTENT', "     '" + msg_line_text + "'\n")
                retval = 1
        msg_prev_line = msg_line_text
    if retval == 0:
        CPrint('SUCCESS_MSG', "\nThe commit messages are properly formatted.\n\n")
    else:
        CPrint('HELP_MSG', "Commit Message Format Requirements:")
        CPrint('HELP_MSG', "-----------------------------------")
        CPrint('HELP_MSG', "o  Subject lines must be <= 64 characters in length")
        CPrint('HELP_MSG', "o  Subject lines must start with a module keyword which is lower-case and followed by a colon and a space")
        CPrint('HELP_MSG', "o  The first word following the colon must be capitalized and the subject line must not end in a '.'")
        CPrint('HELP_MSG', "o  The subject line must be followed by a blank line")
        CPrint('HELP_MSG', "o  The commit description must be <= 72 characters in width\n")
        CPrint('HELP_MSG', "Examples:")
        CPrint('HELP_MSG', "---------")
        CPrint('HELP_MSG', "     build: Fix Vulkan header/registry detection for SDK")
        CPrint('HELP_MSG', "     tests: Fix QueryPerformanceIncompletePasses stride usage")
        CPrint('HELP_MSG', "     corechecks: Fix validation of VU 03227")
        CPrint('HELP_MSG', "     state_tracker: Remove 'using std::*' statements")
        CPrint('HELP_MSG', "     stateless: Account for DynStateWithCount for multiViewport\n")
        CPrint('HELP_MSG', "Refer to this document for additional detail:")
        CPrint('HELP_MSG', "https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/master/CONTRIBUTING.md#coding-conventions-and-formatting")
    return retval

#
#
# Check for test code assigning sType instead of using LvlInitStruct in this PR/Branch
def VerifyTypeAssign(target_refspec, target_files):
    CPrint('', "\nChecking test code for sType assignment instead of using LvlInitStruct:")
    retval = 0

    test_files_list = [item for item in target_files if item.startswith('tests/')]
    test_files = ' '.join([str(elem) for elem in test_files_list])
    if not test_files:
        return 0
    test_diff = subprocess.Popen(('git', 'diff', '-U0', target_refspec, '--', test_files), stdout=subprocess.PIPE)
    stdout, stderr = test_diff.communicate()
    stdout = stdout.decode('utf-8')
    stype_regex = re.compile(r'\.sType\s*=')
    on_regex = re.compile(r'stype-check\s*on')
    off_regex = re.compile(r'stype-check\s*off')
    checking = True
    for line in stdout.split('\n'):
        if not line.startswith('-'):
            if checking:
                if off_regex.search(line, re.IGNORECASE):
                    checking = False
                elif stype_regex.search(line):
                    CPrint('ERR_MSG', "Test assigning sType instead of using LvlInitStruct")
                    CPrint('ERR_MSG', "If this is a case where LvlInitStruct cannot be used, //stype-check off can be used to turn off sType checking")
                    CPrint('CONTENT', "     '" + line + "'\n")
                    retval = 1
            else:
                if on_regex.search(line, re.IGNORECASE):
                    checking = True
    return retval
#
#
# Entrypoint
def main():
    DEFAULT_REFSPEC = 'origin/master'

    parser = argparse.ArgumentParser(description='''Usage: python3 ./scripts/check_code_format.py
    - Reqires python3 and clang-format 7.0+
    - Run script in repo root
    - May produce inaccurate clang-format results if local branch is not rebased on the TARGET_REFSPEC
    ''', formatter_class=RawDescriptionHelpFormatter)
    parser.add_argument('--target-refspec', metavar='TARGET_REFSPEC', type=str, dest='target_refspec', help = 'Refspec to '
        + 'diff against (default is origin/master)', default=DEFAULT_REFSPEC)
    parser.add_argument('--base-refspec', metavar='BASE_REFSPEC', type=str, dest='base_refspec', help = 'Base refspec to '
        + ' compare (default is HEAD)', default='HEAD')
    parser.add_argument('--fetch-main', dest='fetch_main', action='store_true', help='Fetch the master branch first.'
        + ' Useful with --target-refspec=FETCH_HEAD to compare against what is currently on master')
    args = parser.parse_args()

    if sys.version_info[0] != 3:
        print("This script requires Python 3. Run script with [-h] option for more details.")
        exit()

    target_refspec = args.target_refspec
    base_refspec = args.base_refspec

    if args.fetch_main:
        print('Fetching master branch...')
        subprocess.check_call(['git', 'fetch', 'https://github.com/KhronosGroup/Vulkan-ValidationLayers.git', 'master'])

    # Check if this is a merge commit
    commit_parents = check_output(['git', 'rev-list', '--parents', '-n', '1', 'HEAD'])
    if len(commit_parents.split(b' ')) > 2:
        # If this is a merge commit, this is a PR being built, and has been merged into master for testing.
        # The first parent (HEAD^) is going to be master, the second parent (HEAD^2) is going to be the PR commit.
        # TODO (ncesario) We should *ONLY* get here when on github CI, building a PR. Should probably print a
        #      warning if this happens locally.
        target_refspec = 'HEAD^'
        base_refspec = 'HEAD^2'

    diff_range = f'{target_refspec}...{base_refspec}'
    rdiff_range = f'{base_refspec}...{target_refspec}'

    #
    #
    # Get list of files involved in this branch
    target_files_data = subprocess.check_output(['git', 'diff', '--name-only', diff_range])
    target_files = target_files_data.decode('utf-8')
    target_files = target_files.split("\n")

    if os.path.isfile('check_code_format.py'):
        os.chdir('..')

    clang_format_failure = VerifyClangFormatSource(diff_range, target_files)
    copyright_failure = VerifyCopyrights(diff_range, target_files)
    commit_msg_failure = VerifyCommitMessageFormat(rdiff_range, target_files)
    stype_assign_failure = VerifyTypeAssign(diff_range, target_files)

    if clang_format_failure or copyright_failure or commit_msg_failure or stype_assign_failure:
        CPrint('ERR_MSG', "\nOne or more format checks failed.\n\n")
        exit(1)
    else:
        CPrint('SUCCESS_MSG', "\nAll format checks passed.\n\n")
        exit(0)

if __name__ == '__main__':
  main()
