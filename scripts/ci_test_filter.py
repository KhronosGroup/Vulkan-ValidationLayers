#!/usr/bin/env python3
# Copyright (c) 2026 Valve Corporation
# Copyright (c) 2026 LunarG, Inc.

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

# Selects the tests CI runs, from the files a change touches.
# Shared by GitHub CI (tests.py) and internal CI (VulkanTests).
#
# Prints a gtest filter excluding the tests the change cannot affect, e.g.:
#   ci_test_filter.py --changes-since origin/main
#   -*        <- only docs/ changed, no test to run
#             <- (empty) run all tests

import argparse
import os
import subprocess
import common_ci

# Base commit of the GitHub PR being tested, None outside of GitHub PRs
def GetGitHubPRBase() -> str | None:
    if os.environ.get('GITHUB_EVENT_NAME') != 'pull_request':
        return None

    # PRs check out their merge commit into main: HEAD^1 is main.
    # Jobs clone with depth 1, fetch HEAD^1.
    subprocess.check_call(['git', 'fetch', '--depth=2', 'origin', os.environ['GITHUB_SHA']], cwd=common_ci.PROJECT_SRC_DIR)
    return 'HEAD^1'

# Files changed on HEAD since it forked from base
def GetChangedFiles(base: str) -> list[str]:
    output = subprocess.check_output(['git', 'diff', '--name-only', f'{base}...HEAD'], cwd=common_ci.PROJECT_SRC_DIR, text=True)
    return output.splitlines()

# gtest patterns of the tests changed_files cannot affect
def GetExcludedTests(changed_files: list[str]) -> list[str]:
    # Documentation affects no test.
    # No changed file (e.g. testing main itself) still runs everything.
    if changed_files and all(f.startswith('docs/') for f in changed_files):
        return ['*']

    return []

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--changes-since', dest='changes_since', metavar='COMMIT', required=True,
                        help='Base the filter on the files changed on HEAD since it forked from COMMIT')
    args = parser.parse_args()

    excluded_tests = GetExcludedTests(GetChangedFiles(args.changes_since))
    print(f'-{":".join(excluded_tests)}' if excluded_tests else '')
