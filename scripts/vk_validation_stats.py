#!/usr/bin/env python3
# Copyright (c) 2015-2023 The Khronos Group Inc.
# Copyright (c) 2015-2023 Valve Corporation
# Copyright (c) 2015-2023 LunarG, Inc.
# Copyright (c) 2015-2023 Google Inc.
# Copyright (c) 2023-2023 RasterGrid Kft.
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
import csv
import glob
import html
import json
import operator
import os
import re
import sys
import unicodedata
import subprocess
from collections import defaultdict
from collections import OrderedDict
from dataclasses import dataclass
from generate_spec_error_message import ValidationJSON

# helper to define paths relative to the repo root
def repo_relative(path):
    return os.path.abspath(os.path.join(os.path.dirname(__file__), '..', path))

verbose_mode = False
remove_duplicates = False
vuid_prefixes = ['VUID-', 'UNASSIGNED-', 'kVUID_']

# Hard-coded flags that could be command line args, if we decide that's useful
ignore_unassigned = True # These are not found in layer code unless they appear explicitly (most don't), so produce false positives

# These files should not change unless event there is a major refactoring in SPIR-V Tools
# Paths are relative from root of SPIR-V Tools repo
spirvtools_source_files = ["source/val/validation_state.cpp"]
spirvtools_test_files = ["test/val/*.cpp"]

def buildKvuidDict(unassigned_vuid_files):
    kvuid_dict = {}

    for uf in unassigned_vuid_files:
        line_num = 0
        with open(uf, encoding='utf-8') as f:
            for line in f:
                line_num = line_num + 1
                if True in [line.strip().startswith(comment) for comment in ['//', '/*']]:
                    continue

                if 'kVUID_' in line:
                    kvuid_pos = line.find('kVUID_'); assert(kvuid_pos >= 0)
                    eq_pos = line.find('=', kvuid_pos)
                    if eq_pos >= 0:
                        kvuid = line[kvuid_pos:eq_pos].strip(' \t\n;"')
                        unassigned_str = line[eq_pos+1:].strip(' \t\n;"')
                        # Handle the case when the value of the constant is on a new line due to formatting
                        if unassigned_str == '':
                            line = f.readline()
                            pos = line.find('"')
                            unassigned_str = line[pos+1:].strip(' \t\n;"')
                        kvuid_dict[kvuid] = unassigned_str
    return kvuid_dict

class ValidationSource:
    def __init__(self, source_file_list, unassigned_vuid_files):
        self.source_files = source_file_list
        self.unassigned_vuid_files = unassigned_vuid_files
        self.vuid_count_dict = {} # dict of vuid values to the count of how much they're used, and location of where they're used
        self.duplicated_checks = 0
        self.explicit_vuids = set()
        self.implicit_vuids = set()
        self.unassigned_vuids = set()
        self.all_vuids = set()

    def parse(self, spirv_val):
        kvuid_dict = buildKvuidDict(self.unassigned_vuid_files)

        if spirv_val and spirv_val.enabled:
            self.source_files.extend(spirv_val.source_files)

        vuid_duplicate = set()
        # build self.vuid_count_dict
        prepend = None
        for sf in self.source_files:
            spirv_file = True if spirv_val.enabled and sf.startswith(spirv_val.repo_path) else False
            line_num = 0
            with open(sf, encoding='utf-8') as f:
                for line in f:
                    line_num = line_num + 1
                    if True in [line.strip().startswith(comment) for comment in ['//', '/*']]:
                        if 'VUID-' not in line or 'TODO:' in line:
                            continue
                    # Find vuid strings
                    if prepend is not None:
                        line = prepend[:-2] + line.lstrip().lstrip('"') # join lines skipping CR, whitespace and trailing/leading quote char
                        prepend = None
                    if any(prefix in line for prefix in vuid_prefixes):
                        # Replace the '(' of lines containing validation helper functions with ' ' to make them easier to parse
                        line = line.replace("(", " ")
                        line_list = line.split()

                        # A VUID string that has been broken by clang will start with a vuid prefix and end with -, and will be last in the list
                        broken_vuid = line_list[-1].strip('"')
                        if any(broken_vuid.startswith(prefix) for prefix in vuid_prefixes) and broken_vuid.endswith('-'):
                            prepend = line
                            continue

                        vuid_list = []
                        for str in line_list:
                            if any(prefix in str for prefix in vuid_prefixes):
                                vuid_list.append(str.strip(',);{}"*'))
                        for vuid in vuid_list:
                            if vuid.startswith('kVUID_'): vuid = kvuid_dict[vuid]
                            if vuid not in self.vuid_count_dict:
                                self.vuid_count_dict[vuid] = {}
                                self.vuid_count_dict[vuid]['count'] = 1
                                self.vuid_count_dict[vuid]['file_line'] = []
                                self.vuid_count_dict[vuid]['spirv'] = False # default
                            else:
                                if self.vuid_count_dict[vuid]['count'] == 1:    # only count first time duplicated
                                    self.duplicated_checks = self.duplicated_checks + 1
                                self.vuid_count_dict[vuid]['count'] = self.vuid_count_dict[vuid]['count'] + 1
                            self.vuid_count_dict[vuid]['file_line'].append('%s,%d' % (sf, line_num))
                            if spirv_file:
                                self.vuid_count_dict[vuid]['spirv'] = True
        # Sort vuids by type
        for vuid in self.vuid_count_dict.keys():
            if (vuid.startswith('VUID-')):
                vuid_number = vuid[-5:]
                if (vuid_number.isdecimal()):
                    if remove_duplicates:
                        if vuid_number in vuid_duplicate:
                            continue
                        vuid_duplicate.add(vuid_number)
                    self.explicit_vuids.add(vuid)    # explicit end in 5 numeric chars
                    if self.vuid_count_dict[vuid]['spirv']:
                        spirv_val.source_explicit_vuids.add(vuid)
                else:
                    self.implicit_vuids.add(vuid)
                    if self.vuid_count_dict[vuid]['spirv']:
                        spirv_val.source_implicit_vuids.add(vuid)
            elif (vuid.startswith('UNASSIGNED-')):
                self.unassigned_vuids.add(vuid)
            else:
                print("Unable to categorize VUID: %s" % vuid)
                print("Confused while parsing VUIDs in layer source code - cannot proceed. (FIXME)")
                exit(-1)
        self.all_vuids = self.explicit_vuids | self.implicit_vuids | self.unassigned_vuids
        if spirv_file:
            spirv_val.source_all_vuids = spirv_val.source_explicit_vuids | spirv_val.source_implicit_vuids

# Class to parse the validation layer test source and store testnames
class ValidationTests:
    def __init__(self, test_file_list, unassigned_vuid_files):
        self.test_files = test_file_list
        self.unassigned_vuid_files = unassigned_vuid_files
        self.test_trigger_txt_list = ['TEST_F(']
        self.explicit_vuids = set()
        self.implicit_vuids = set()
        self.unassigned_vuids = set()
        self.all_vuids = set()
        #self.test_to_vuids = {} # Map test name to VUIDs tested
        self.vuid_to_tests = defaultdict(set) # Map VUIDs to set of test names where implemented

    # Parse test files into internal data struct
    def parse(self, spirv_val):
        kvuid_dict = buildKvuidDict(self.unassigned_vuid_files)

        if spirv_val and spirv_val.enabled:
            self.test_files.extend(spirv_val.test_files)

        vuid_duplicate = set()
        # For each test file, parse test names into set
        grab_next_line = False # handle testname on separate line than wildcard
        testname = ''
        prepend = None
        for test_file in self.test_files:
            spirv_file = True if spirv_val.enabled and test_file.startswith(spirv_val.repo_path) else False
            with open(test_file, encoding='utf-8') as tf:
                for line in tf:
                    if True in [line.strip().startswith(comment) for comment in ['//', '/*']]:
                        continue
                    elif True in [x in line for x in ['TEST_DESCRIPTION', 'vvl_vuid_hash']]:
                        continue # Tests have extra place it might not want to report VUIDs

                    # if line ends in a broken VUID string, fix that before proceeding
                    if prepend is not None:
                        line = prepend[:-2] + line.lstrip().lstrip('"') # join lines skipping CR, whitespace and trailing/leading quote char
                        prepend = None
                    if any(prefix in line for prefix in vuid_prefixes):
                        line_list = line.split()

                        # A VUID string that has been broken by clang will start with a vuid prefix and end with -, and will be last in the list
                        broken_vuid = line_list[-1].strip('"')
                        if any(broken_vuid.startswith(prefix) for prefix in vuid_prefixes) and broken_vuid.endswith('-'):
                            prepend = line
                            continue

                    if any(ttt in line for ttt in self.test_trigger_txt_list):
                        testname = line.split(',')[-1]
                        testname = testname.strip().strip(' {)')
                        if ('' == testname):
                            grab_next_line = True
                            continue
                        testgroup = line.split(',')[0][line.index('(') + 1:]
                        testname = testgroup + '.' + testname
                    if grab_next_line: # test name on its own line
                        grab_next_line = False
                        testname = testname.strip().strip(' {)')
                    # Don't count anything in disabled tests
                    if 'DISABLED_' in testname:
                        continue
                    if any(prefix in line for prefix in vuid_prefixes):
                        line_list = re.split('[\s{}[\]()"]+',line)
                        for sub_str in line_list:
                            if any(prefix in sub_str for prefix in vuid_prefixes):
                                vuid_str = sub_str.strip(',);:"*')
                                if vuid_str.startswith('kVUID_'): vuid_str = kvuid_dict[vuid_str]
                                self.vuid_to_tests[vuid_str].add(testname)
                                if (vuid_str.startswith('VUID-')):
                                    vuid_number = vuid_str[-5:]
                                    if (vuid_number.isdecimal()):
                                        if remove_duplicates:
                                            if vuid_number in vuid_duplicate:
                                                continue
                                            vuid_duplicate.add(vuid_number)
                                        self.explicit_vuids.add(vuid_str)    # explicit end in 5 numeric chars
                                        if spirv_file:
                                            spirv_val.test_explicit_vuids.add(vuid_str)
                                    else:
                                        self.implicit_vuids.add(vuid_str)
                                        if spirv_file:
                                            spirv_val.test_implicit_vuids.add(vuid_str)
                                elif (vuid_str.startswith('UNASSIGNED-')):
                                    self.unassigned_vuids.add(vuid_str)
                                else:
                                    print("Unable to categorize VUID: %s" % vuid_str)
                                    print("Confused while parsing VUIDs in test code - cannot proceed. (FIXME)")
                                    exit(-1)
        self.all_vuids = self.explicit_vuids | self.implicit_vuids | self.unassigned_vuids

# Class to do consistency checking
#
class Consistency:
    def __init__(self, all_json, all_checks, all_tests):
        self.valid = all_json
        self.checks = all_checks
        self.tests = all_tests

    # Report undefined VUIDs in source code
    def undef_vuids_in_layer_code(self):
        undef_set = self.checks - self.valid
        undef_set.discard('VUID-Undefined') # don't report Undefined
        if ignore_unassigned:
            unassigned = set({uv for uv in undef_set if uv.startswith('UNASSIGNED-')})
            undef_set = undef_set - unassigned
        if (len(undef_set) > 0):
            print("\nFollowing VUIDs found in layer code are not defined in validusage.json (%d):" % len(undef_set))
            undef = list(undef_set)
            undef.sort()
            for vuid in undef:
                print("    %s" % vuid)
            return False
        return True

    # Report undefined VUIDs in tests
    def undef_vuids_in_tests(self):
        undef_set = self.tests - self.valid
        undef_set.discard('VUID-Undefined') # don't report Undefined
        if ignore_unassigned:
            unassigned = set({uv for uv in undef_set if uv.startswith('UNASSIGNED-')})
            undef_set = undef_set - unassigned
        if (len(undef_set) > 0):
            print("\nFollowing VUIDs found in layer tests are not defined in validusage.json (%d):" % len(undef_set))
            undef = list(undef_set)
            undef.sort()
            for vuid in undef:
                print("    %s" % vuid)
            return False
        return True

    # Report vuids in tests that are not in source
    def vuids_tested_not_checked(self):
        undef_set = self.tests - self.checks
        undef_set.discard('VUID-Undefined') # don't report Undefined
        if ignore_unassigned:
            unassigned = set()
            for vuid in undef_set:
                if vuid.startswith('UNASSIGNED-'):
                    unassigned.add(vuid)
            undef_set = undef_set - unassigned
        if (len(undef_set) > 0):
            print("\nFollowing VUIDs found in tests but are not checked in layer code (%d):" % len(undef_set))
            undef = list(undef_set)
            undef.sort()
            for vuid in undef:
                print("    %s" % vuid)
            return False
        return True

    # TODO: Explicit checked VUIDs which have no test
    # def explicit_vuids_checked_not_tested(self):


# Class to output database in various flavors
#
class OutputDatabase:
    def __init__(self, val_json, val_source, val_tests, spirv_val):
        self.vj = val_json
        self.vs = val_source
        self.vt = val_tests
        self.sv = spirv_val

    def dump_txt(self, filename, only_unimplemented=False):
        print(f'\nDumping database to text file: {filename}')
        with open(filename, 'w', encoding='utf-8') as txt:
            txt.write("## VUID Database\n")
            txt.write("## Format: VUID_NAME | CHECKED | SPIRV-TOOL | TEST | TYPE | API/STRUCT | EXTENSION | VUID_TEXT\n##\n")
            vuid_list = list(self.vj.all_vuids)
            vuid_list.sort()
            for vuid in vuid_list:
                db_list = self.vj.vuid_db[vuid]
                db_list.sort(key=operator.itemgetter('ext')) # sort list to ease diffs of output file
                for db_entry in db_list:
                    checked = 'N'
                    spirv = 'N'
                    if vuid in self.vs.all_vuids:
                        if only_unimplemented:
                            continue
                        else:
                            checked = 'Y'
                            if vuid in self.sv.source_all_vuids:
                                spirv = 'Y'
                    test = 'None'
                    if vuid in self.vt.vuid_to_tests:
                        test_list = list(self.vt.vuid_to_tests[vuid])
                        test_list.sort()   # sort tests, for diff-ability
                        sep = ', '
                        test = sep.join(test_list)

                    txt.write("%s | %s | %s | %s | %s | %s | %s | %s\n" % (vuid, checked, test, spirv, db_entry['type'], db_entry['api'], db_entry['ext'], db_entry['text']))

    def dump_csv(self, filename, only_unimplemented=False):
        print(f'\nDumping database to csv file: {filename}')
        with open(filename, 'w', newline='', encoding='utf-8') as csvfile:
            cw = csv.writer(csvfile)
            cw.writerow(['VUID_NAME','CHECKED','SPIRV-TOOL', 'TEST','TYPE','API/STRUCT','EXTENSION','VUID_TEXT'])
            vuid_list = list(self.vj.all_vuids)
            vuid_list.sort()
            for vuid in vuid_list:
                for db_entry in self.vj.vuid_db[vuid]:
                    row = [vuid]
                    if vuid in self.vs.all_vuids:
                        if only_unimplemented:
                            continue
                        else:
                            row.append('Y') # checked
                            if vuid in self.sv.source_all_vuids:
                                row.append('Y') # spirv-tool
                            else:
                                row.append('N') # spirv-tool

                    else:
                        row.append('N') # checked
                        row.append('N') # spirv-tool
                    test = 'None'
                    if vuid in self.vt.vuid_to_tests:
                        sep = ', '
                        test = sep.join(sorted(self.vt.vuid_to_tests[vuid]))
                    row.append(test)
                    row.append(db_entry['type'])
                    row.append(db_entry['api'])
                    row.append(db_entry['ext'])
                    row.append(db_entry['text'])
                    cw.writerow(row)

    def dump_html(self, filename, only_unimplemented=False):
        print(f'\nDumping database to html file: {filename}')
        preamble = '<!DOCTYPE html>\n<html>\n<head>\n<style>\ntable, th, td {\n border: 1px solid black;\n border-collapse: collapse; \n}\n</style>\n<body>\n<h2>Valid Usage Database</h2>\n<font size="2" face="Arial">\n<table style="width:100%">\n'
        headers = '<tr><th>VUID NAME</th><th>CHECKED</th><th>SPIRV-TOOL</th><th>TEST</th><th>TYPE</th><th>API/STRUCT</th><th>EXTENSION</th><th>VUID TEXT</th></tr>\n'
        with open(filename, 'w', encoding='utf-8') as hfile:
            hfile.write(preamble)
            hfile.write(headers)
            vuid_list = list(self.vj.all_vuids)
            vuid_list.sort()
            for vuid in vuid_list:
                for db_entry in self.vj.vuid_db[vuid]:
                    checked = '<span style="color:red;">N</span>'
                    spirv = ''
                    if vuid in self.vs.all_vuids:
                        if only_unimplemented:
                            continue
                        else:
                            checked = '<span style="color:limegreen;">Y</span>'
                            if vuid in self.sv.source_all_vuids:
                                spirv = 'Y'
                    hfile.write('<tr><th>%s</th>' % vuid)
                    hfile.write('<th>%s</th>' % checked)
                    hfile.write('<th>%s</th>' % spirv)
                    test = 'None'
                    if vuid in self.vt.vuid_to_tests:
                        sep = ', '
                        test = sep.join(sorted(self.vt.vuid_to_tests[vuid]))
                    hfile.write('<th>%s</th>' % test)
                    hfile.write('<th>%s</th>' % db_entry['type'])
                    hfile.write('<th>%s</th>' % db_entry['api'])
                    hfile.write('<th>%s</th>' % db_entry['ext'])
                    hfile.write('<th>%s</th></tr>\n' % db_entry['text'])
            hfile.write('</table>\n</body>\n</html>\n')

    def dump_extension_coverage(self, filename):
        print(f'\nDumping extension coverage report to file: {filename}')
        @dataclass
        class ExtEntry:
            checked = 0
            total = 0
        ext_db = defaultdict(ExtEntry)
        vuid_list = list(self.vj.all_vuids)
        for vuid in vuid_list:
            for db_entry in self.vj.vuid_db[vuid]:
                for ext_name in set(re.findall(r'\w+', db_entry['ext'])):
                    ext_db[ext_name].total += 1
                    if vuid in self.vs.all_vuids:
                        ext_db[ext_name].checked += 1

        with open(filename, 'w', newline='', encoding='utf-8') as csvfile:
            cw = csv.writer(csvfile)
            cw.writerow(['EXTENSION','CHECKED','TOTAL','COVERAGE'])
            for ext_name in sorted(ext_db):
                ext_entry = ext_db[ext_name]
                cw.writerow([ext_name, ext_entry.checked, ext_entry.total, ext_entry.checked/ext_entry.total])

class SpirvValidation:
    def __init__(self, repo_path):
        self.enabled = (repo_path is not None)
        self.repo_path = repo_path
        self.version = 'unknown'
        self.source_files = []
        self.test_files = []
        self.source_explicit_vuids = set()
        self.source_implicit_vuids = set()
        self.source_all_vuids = set()
        self.test_explicit_vuids = set()
        self.test_implicit_vuids = set()

    def load(self, verbose):
        if self.enabled is False:
            return
        # Get hash from git if available
        try:
            git_dir = os.path.join(self.repo_path, '.git')
            process = subprocess.Popen(['git', '--git-dir='+git_dir ,'rev-parse', 'HEAD'], shell=False, stdout=subprocess.PIPE)
            self.version = process.communicate()[0].strip().decode('utf-8')[:7]
            if process.poll() != 0:
                throw
            elif verbose:
                print('Found SPIR-V Tools version %s' % self.version)
        except:
            # leave as default
            if verbose:
                print('Could not find .git file for version of SPIR-V tools, marking as %s' % self.version)

        # Find and parse files with VUIDs in source
        for path in spirvtools_source_files:
            self.source_files.extend(glob.glob(os.path.join(self.repo_path, path)))
        for path in spirvtools_test_files:
            self.test_files.extend(glob.glob(os.path.join(self.repo_path, path)))


def main(argv):
    TXT_FILENAME = "validation_error_database.txt"
    CSV_FILENAME = "validation_error_database.csv"
    HTML_FILENAME = "validation_error_database.html"
    EXTENSION_COVERAGE_FILENAME = "validation_extension_coverage.csv"

    parser = argparse.ArgumentParser()
    parser.add_argument('json_file', help="registry file 'validusage.json'")
    parser.add_argument('-api',
                        default='vulkan',
                        choices=['vulkan'],
                        help='Specify API name to use')
    parser.add_argument('-c', action='store_true',
                        help='report consistency warnings')
    parser.add_argument('-todo', action='store_true',
                        help='report unimplemented VUIDs')
    parser.add_argument('-unassigned', action='store_true',
                        help='report unassigned VUIDs')
    parser.add_argument('-vuid', metavar='VUID_NAME',
                        help='report status of individual VUID <VUID_NAME>')
    parser.add_argument('-spirvtools', metavar='PATH',
                        help='when pointed to root directory of SPIRV-Tools repo, will search the repo for VUs that are implemented there')
    parser.add_argument('-text', nargs='?', const=TXT_FILENAME, metavar='FILENAME',
                        help=f'export the error database in text format to <FILENAME>, defaults to {TXT_FILENAME}')
    parser.add_argument('-csv', nargs='?', const=CSV_FILENAME, metavar='FILENAME',
                        help=f'export the error database in csv format to <FILENAME>, defaults to {CSV_FILENAME}')
    parser.add_argument('-html', nargs='?', const=HTML_FILENAME, metavar='FILENAME',
                        help=f'export the error database in html format to <FILENAME>, defaults to {HTML_FILENAME}')
    parser.add_argument('-extension_coverage', nargs='?', const=EXTENSION_COVERAGE_FILENAME, metavar='FILENAME',
                        help=f'export an extension coverage report to <FILENAME>, defaults to {EXTENSION_COVERAGE_FILENAME}')
    parser.add_argument('-remove_duplicates', action='store_true',
                        help='remove duplicate VUID numbers')
    parser.add_argument('-summary', action='store_true',
                        help='output summary of VUID coverage')
    parser.add_argument('-verbose', action='store_true',
                        help='show your work (to stdout)')
    args = parser.parse_args()

    # We need python modules found in the registry directory. This assumes that the validusage.json file is in that directory,
    # and hasn't been copied elsewhere.
    registry_dir = os.path.dirname(args.json_file)
    sys.path.insert(0, registry_dir)

    layer_source_files = [repo_relative(path) for path in [
        'layers/error_message/unimplementable_validation.h',
        'layers/state_tracker/cmd_buffer_state.cpp', # some Video VUIDs are in here
        'layers/state_tracker/descriptor_sets.cpp',
        'layers/gpu_validation/gpu_vuids.h',
        'layers/stateless/stateless_validation.h',
        f'layers/{args.api}/generated/stateless_validation_helper.cpp',
        f'layers/{args.api}/generated/object_tracker.cpp',
        f'layers/{args.api}/generated/spirv_validation_helper.cpp',
        f'layers/{args.api}/generated/command_validation.cpp',
    ]]
    # Be careful not to add vk_validation_error_messages.h or it will show 100% test coverage
    layer_source_files.extend(glob.glob(os.path.join(repo_relative('layers/core_checks/'), '*.cpp')))
    layer_source_files.extend(glob.glob(os.path.join(repo_relative('layers/stateless/'), '*.cpp')))
    layer_source_files.extend(glob.glob(os.path.join(repo_relative('layers/sync/'), '*.cpp')))
    layer_source_files.extend(glob.glob(os.path.join(repo_relative('layers/object_tracker/'), '*.cpp')))
    layer_source_files.extend(glob.glob(os.path.join(repo_relative('layers/drawdispatch/'), '*.cpp')))
    layer_source_files.extend(glob.glob(os.path.join(repo_relative('layers/gpu_validation/'), '*.cpp')))

    test_source_files = glob.glob(os.path.join(repo_relative('tests/unit'), '*.cpp'))

    unassigned_vuid_files = [repo_relative(path) for path in [
        'layers/best_practices/best_practices_error_enums.h',
        'layers/stateless/stateless_validation.h',
        'layers/object_tracker/object_lifetime_validation.h'
    ]]

    global verbose_mode
    verbose_mode = args.verbose

    global remove_duplicates
    remove_duplicates = args.remove_duplicates

    # Load in SPIRV-Tools if passed in
    spirv_val = SpirvValidation(args.spirvtools)
    spirv_val.load(verbose_mode)

    # Parse validusage json
    val_json = ValidationJSON(args.json_file)
    val_json.parse()
    exp_json = len(val_json.explicit_vuids)
    imp_json = len(val_json.implicit_vuids)
    all_json = len(val_json.all_vuids)
    if verbose_mode:
        print("Found %d unique error vuids in validusage.json file." % all_json)
        print("  %d explicit" % exp_json)
        print("  %d implicit" % imp_json)
        if len(val_json.duplicate_vuids) > 0:
            print("%d VUIDs appear in validusage.json more than once." % len(val_json.duplicate_vuids))
            for vuid in val_json.duplicate_vuids:
                print("  %s" % vuid)
                for ext in val_json.vuid_db[vuid]:
                    print("    with extension: %s" % ext['ext'])

    # Parse layer source files
    val_source = ValidationSource(layer_source_files, unassigned_vuid_files)
    val_source.parse(spirv_val)
    exp_checks = len(val_source.explicit_vuids)
    imp_checks = len(val_source.implicit_vuids)
    all_checks = len(val_source.vuid_count_dict.keys())
    spirv_exp_checks = len(spirv_val.source_explicit_vuids) if spirv_val.enabled else 0
    spirv_imp_checks = len(spirv_val.source_implicit_vuids) if spirv_val.enabled else 0
    spirv_all_checks = (spirv_exp_checks + spirv_imp_checks) if spirv_val.enabled else 0
    if verbose_mode:
        print("Found %d unique vuid checks in layer source code." % all_checks)
        print("  %d explicit" % exp_checks)
        if spirv_val.enabled:
            print("    SPIR-V Tool make up %d" % spirv_exp_checks)
        print("  %d implicit" % imp_checks)
        if spirv_val.enabled:
            print("    SPIR-V Tool make up %d" % spirv_imp_checks)
        print("  %d unassigned" % len(val_source.unassigned_vuids))
        print("  %d checks are implemented more that once" % val_source.duplicated_checks)

    # Parse test files
    val_tests = ValidationTests(test_source_files, unassigned_vuid_files)
    val_tests.parse(spirv_val)
    exp_tests = len(val_tests.explicit_vuids)
    imp_tests = len(val_tests.implicit_vuids)
    all_tests = len(val_tests.all_vuids)
    spirv_exp_tests = len(spirv_val.test_explicit_vuids) if spirv_val.enabled else 0
    spirv_imp_tests = len(spirv_val.test_implicit_vuids) if spirv_val.enabled else 0
    spirv_all_tests = (spirv_exp_tests + spirv_imp_tests) if spirv_val.enabled else 0
    if verbose_mode:
        print("Found %d unique error vuids in test source code." % all_tests)
        print("  %d explicit" % exp_tests)
        if spirv_val.enabled:
            print("    From SPIRV-Tools: %d" % spirv_exp_tests)
        print("  %d implicit" % imp_tests)
        if spirv_val.enabled:
            print("    From SPIRV-Tools: %d" % spirv_imp_tests)
        print("  %d unassigned" % len(val_tests.unassigned_vuids))

    # Process stats
    if args.summary:
        if spirv_val.enabled:
            print("\nValidation Statistics (using validusage.json version %s and SPIRV-Tools version %s)" % (val_json.api_version, spirv_val.version))
        else:
            print("\nValidation Statistics (using validusage.json version %s)" % val_json.api_version)
        print("  VUIDs defined in JSON file:  %04d explicit, %04d implicit, %04d total." % (exp_json, imp_json, all_json))
        print("  VUIDs checked in layer code: %04d explicit, %04d implicit, %04d total." % (exp_checks, imp_checks, all_checks))
        if spirv_val.enabled:
            print("             From SPIRV-Tools: %04d explicit, %04d implicit, %04d total." % (spirv_exp_checks, spirv_imp_checks, spirv_all_checks))
        print("  VUIDs tested in layer tests: %04d explicit, %04d implicit, %04d total." % (exp_tests, imp_tests, all_tests))
        if spirv_val.enabled:
            print("             From SPIRV-Tools: %04d explicit, %04d implicit, %04d total." % (spirv_exp_tests, spirv_imp_tests, spirv_all_tests))

        print("\nVUID check coverage")
        print("  Explicit VUIDs checked: %.1f%% (%d checked vs %d defined)" % ((100.0 * exp_checks / exp_json), exp_checks, exp_json))
        print("  Implicit VUIDs checked: %.1f%% (%d checked vs %d defined)" % ((100.0 * imp_checks / imp_json), imp_checks, imp_json))
        print("  Overall VUIDs checked:  %.1f%% (%d checked vs %d defined)" % ((100.0 * all_checks / all_json), all_checks, all_json))

        print("\nVUID test coverage")
        print("  Explicit VUIDs tested: %.1f%% (%d tested vs %d checks)" % ((100.0 * exp_tests / exp_checks), exp_tests, exp_checks))
        print("  Implicit VUIDs tested: %.1f%% (%d tested vs %d checks)" % ((100.0 * imp_tests / imp_checks), imp_tests, imp_checks))
        print("  Overall VUIDs tested:  %.1f%% (%d tested vs %d checks)" % ((100.0 * all_tests / all_checks), all_tests, all_checks))

    # Report status of a single VUID
    if args.vuid:
        print("\n\nChecking status of <%s>" % args.vuid)
        if args.vuid not in val_json.all_vuids and not args.vuid.startswith('UNASSIGNED-'):
            print('  Not a valid VUID string.')
        else:
            if args.vuid in val_source.explicit_vuids:
                print('  Implemented!')
                line_list = val_source.vuid_count_dict[args.vuid]['file_line']
                for line in line_list:
                    print('    => %s' % line)
            elif args.vuid in val_source.implicit_vuids:
                print('  Implemented! (Implicit)')
                line_list = val_source.vuid_count_dict[args.vuid]['file_line']
                for line in line_list:
                    print('    => %s' % line)
            else:
                print('  Not implemented.')
            if args.vuid in val_tests.all_vuids:
                print('  Has a test!')
                test_list = val_tests.vuid_to_tests[args.vuid]
                for test in test_list:
                    print('    => %s' % test)
            else:
                print('  Not tested.')

    # Report unimplemented explicit VUIDs
    if args.todo:
        unim_explicit = val_json.explicit_vuids - val_source.explicit_vuids
        print("\n\n%d explicit VUID checks remain unimplemented:" % len(unim_explicit))
        ulist = list(unim_explicit)
        ulist.sort()
        for vuid in ulist:
            print("  => %s" % vuid)

    # Report unassigned VUIDs
    if args.unassigned:
        # TODO: I do not really want VUIDs created for warnings though here
        print("\n\n%d checks without a spec VUID:" % len(val_source.unassigned_vuids))
        ulist = list(val_source.unassigned_vuids)
        ulist.sort()
        for vuid in ulist:
            print("  => %s" % vuid)
            line_list = val_source.vuid_count_dict[vuid]['file_line']
            for line in line_list:
                print('    => %s' % line)
        print("\n%d tests without a spec VUID:" % len(val_source.unassigned_vuids))
        ulist = list(val_tests.unassigned_vuids)
        ulist.sort()
        for vuid in ulist:
            print("  => %s" % vuid)
            test_list = val_tests.vuid_to_tests[vuid]
            for test in test_list:
                print('    => %s' % test)

    # Consistency tests
    if args.c:
        print("\n\nRunning consistency tests...")
        con = Consistency(val_json.all_vuids, val_source.all_vuids, val_tests.all_vuids)
        ok = con.undef_vuids_in_layer_code()
        ok &= con.undef_vuids_in_tests()
        ok &= con.vuids_tested_not_checked()

        if ok:
            print("  OK! No inconsistencies found.")

    # Output database in requested format(s)
    db_out = OutputDatabase(val_json, val_source, val_tests, spirv_val)
    if args.text:
        db_out.dump_txt(args.text, args.todo)
    if args.csv:
        db_out.dump_csv(args.csv, args.todo)
    if args.html:
        db_out.dump_html(args.html, args.todo)
    if args.extension_coverage:
        db_out.dump_extension_coverage(args.extension_coverage)

if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
