#!/usr/bin/env python3
# Copyright (c) 2015-2026 The Khronos Group Inc.
# Copyright (c) 2015-2026 Valve Corporation
# Copyright (c) 2015-2026 LunarG, Inc.
# Copyright (c) 2015-2026 Google Inc.
# Copyright (c) 2023-2026 RasterGrid Kft.
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
import re
import sys
import subprocess
from collections import defaultdict
from generate_spec_error_message import ValidationJSON

_VENDOR_SUFFIXES = ['IMG', 'AMD', 'AMDX', 'ARM', 'FSL', 'BRCM', 'NXP', 'NV', 'NVX',
                    'VIV', 'VSI', 'KDAB', 'ANDROID', 'CHROMIUM', 'FUCHSIA', 'GGP',
                    'GOOGLE', 'QCOM', 'LUNARG', 'NZXT', 'SAMSUNG', 'SEC', 'TIZEN',
                    'RENDERDOC', 'NN', 'MVK', 'MESA', 'INTEL', 'HUAWEI', 'VALVE',
                    'QNX', 'JUICE', 'FB', 'RASTERGRID', 'MSFT']

# helper to define paths relative to the repo root
def repo_relative(path):
    return os.path.abspath(os.path.join(os.path.dirname(__file__), '..', path))

def IsVendor(vuid : str = None):
    vkObject = vuid.split('-')[1]

    for vendor in _VENDOR_SUFFIXES:
        if vkObject.endswith(vendor):
            return vendor

    return None

remove_duplicates = False
vuid_prefixes = ['VUID-']

# These files should not change unless event there is a major refactoring in SPIR-V Tools
# Paths are relative from root of SPIR-V Tools repo
spirvtools_source_files = ["source/val/validation_state.cpp"]
spirvtools_test_files = ["test/val/*.cpp"]

class ValidationSource:
    def __init__(self, source_file_list):
        self.source_files = source_file_list
        self.vuid_count_dict = {} # dict of vuid values to the count of how much they're used, and location of where they're used
        self.duplicated_checks = 0
        self.explicit_vuids = set()
        self.implicit_vuids = set()
        self.all_vuids = set()
        self.action_set = set()

    def dedup(self):
        unique_explicit_vuids = {}
        for item in sorted(self.explicit_vuids):
            key = item[-5:]
            unique_explicit_vuids[key] = item

        self.explicit_vuids = set(list(unique_explicit_vuids.values()))
        self.all_vuids = self.explicit_vuids | self.implicit_vuids

    # https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/11781
    def action_dedup(self, spec_set):
        target_numbers = {int(num) for num in self.action_set}
        for vuid in spec_set:
            if int(vuid[-5:]) in target_numbers:
                self.explicit_vuids.add(vuid)
        self.all_vuids = self.explicit_vuids | self.implicit_vuids

    def parse(self, spirv_val):

        if spirv_val and spirv_val.enabled:
            self.source_files.extend(spirv_val.source_files)

        # build self.vuid_count_dict
        prepend = None
        for sf in self.source_files:
            spirv_file = True if spirv_val.enabled and sf.startswith(spirv_val.repo_path) else False
            line_num = 0
            with open(sf, encoding='utf-8') as f:
                for line in f:
                    line_num = line_num + 1
                    if True in [line.strip().startswith(comment) for comment in ['//', '/*']]:
                        if 'VUID-' not in line or 'TODO:' in line or 'Note:' in line:
                            continue
                    # Find vuid strings
                    if prepend is not None:
                        line = prepend[:-2] + line.lstrip().lstrip('"') # join lines skipping CR, whitespace and trailing/leading quote char
                        prepend = None

                    # Our hacked way to detect "common" draw/dispatch/traceRay VUIDs
                    if sf.endswith('drawdispatch_vuids.cpp') and '###' in line:
                        self.action_set.add(line.strip()[-5:])

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
                            if vuid not in self.vuid_count_dict:
                                self.vuid_count_dict[vuid] = {}
                                self.vuid_count_dict[vuid]['count'] = 1
                                self.vuid_count_dict[vuid]['file_line'] = []
                                self.vuid_count_dict[vuid]['spirv'] = False # default
                            else:
                                if self.vuid_count_dict[vuid]['count'] == 1:    # only count first time duplicated
                                    self.duplicated_checks = self.duplicated_checks + 1
                                self.vuid_count_dict[vuid]['count'] = self.vuid_count_dict[vuid]['count'] + 1
                            self.vuid_count_dict[vuid]['file_line'].append(f'{sf},{line_num}')
                            if spirv_file:
                                self.vuid_count_dict[vuid]['spirv'] = True
        # Sort vuids by type
        for vuid in self.vuid_count_dict.keys():
            if (vuid.startswith('VUID-')):
                vuid_number = vuid[-5:]
                if (vuid_number.isdecimal()):
                    self.explicit_vuids.add(vuid)    # explicit end in 5 numeric chars
                    if self.vuid_count_dict[vuid]['spirv']:
                        spirv_val.source_explicit_vuids.add(vuid)
                else:
                    self.implicit_vuids.add(vuid)
                    if self.vuid_count_dict[vuid]['spirv']:
                        spirv_val.source_implicit_vuids.add(vuid)
            else:
                print(f'Unable to categorize VUID: {vuid}')
                print("Confused while parsing VUIDs in layer source code - cannot proceed. (FIXME)")
                exit(-1)
        self.all_vuids = self.explicit_vuids | self.implicit_vuids
        if spirv_file:
            spirv_val.source_all_vuids = spirv_val.source_explicit_vuids | spirv_val.source_implicit_vuids

# Class to parse the validation layer test source and store testnames
class ValidationTests:
    def __init__(self, test_file_list):
        self.test_files = test_file_list
        self.test_trigger_txt_list = ['TEST_F(']
        self.explicit_vuids = set()
        self.implicit_vuids = set()
        self.all_vuids = set()
        #self.test_to_vuids = {} # Map test name to VUIDs tested
        self.vuid_to_tests = defaultdict(set) # Map VUIDs to set of test names where implemented

    def dedup(self):
        unique_explicit_vuids = {}
        for item in sorted(self.explicit_vuids):
            key = item[-5:]
            unique_explicit_vuids[key] = item

        self.explicit_vuids = set(list(unique_explicit_vuids.values()))
        self.all_vuids = self.explicit_vuids | self.implicit_vuids

    # Parse test files into internal data struct
    def parse(self, spirv_val):
        if spirv_val and spirv_val.enabled:
            self.test_files.extend(spirv_val.test_files)

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
                        line_list = re.split(r'[\s{}[\]()"]+', line)
                        for sub_str in line_list:
                            if any(prefix in sub_str for prefix in vuid_prefixes):
                                vuid_str = sub_str.strip(',);:"*')
                                self.vuid_to_tests[vuid_str].add(testname)
                                if (vuid_str.startswith('VUID-')):
                                    vuid_number = vuid_str[-5:]
                                    if (vuid_number.isdecimal()):
                                        self.explicit_vuids.add(vuid_str)    # explicit end in 5 numeric chars
                                        if spirv_file:
                                            spirv_val.test_explicit_vuids.add(vuid_str)
                                    else:
                                        self.implicit_vuids.add(vuid_str)
                                        if spirv_file:
                                            spirv_val.test_implicit_vuids.add(vuid_str)
                                else:
                                    print(f'Unable to categorize VUID: {vuid_str}')
                                    print("Confused while parsing VUIDs in test code - cannot proceed. (FIXME)")
                                    exit(-1)
        self.all_vuids = self.explicit_vuids | self.implicit_vuids

# Class to do consistency checking
#
class Consistency:
    def __init__(self, all_json, all_checks, all_tests):
        self.valid = all_json
        self.checks = all_checks
        self.tests = all_tests
        # don't report
        self.discard = [
            # Currently a bug with clang-format in spirv-tools
            'VUID-',
            'VUID-PrimitiveTriangleIndicesEXT-',
            'VUID-HitTriangleVertexPositionsKHR-',
        ]

    # Report undefined VUIDs in source code
    def undef_vuids_in_layer_code(self):
        undef_set = self.checks - self.valid
        [undef_set.discard(item) for item in self.discard]
        if (len(undef_set) > 0):
            print(f'\nFollowing VUIDs found in layer code are not defined in validusage.json ({len(undef_set)}):')
            undef = list(undef_set)
            undef.sort()
            for vuid in undef:
                print(f'    {vuid}')
            return False
        return True

    # Report undefined VUIDs in tests
    def undef_vuids_in_tests(self):
        undef_set = self.tests - self.valid
        [undef_set.discard(item) for item in self.discard]
        if (len(undef_set) > 0):
            print(f'\nFollowing VUIDs found in layer tests are not defined in validusage.json ({len(undef_set)}):')
            undef = list(undef_set)
            undef.sort()
            for vuid in undef:
                print(f'    {vuid}')
            return False
        return True

    # Report vuids in tests that are not in source
    def vuids_tested_not_checked(self):
        undef_set = self.tests - self.checks
        [undef_set.discard(item) for item in self.discard]
        if (len(undef_set) > 0):
            print(f'\nFollowing VUIDs found in tests but are not checked in layer code ({len(undef_set)}):')
            undef = list(undef_set)
            undef.sort()
            for vuid in undef:
                print(f'    {vuid}')
            return False
        return True


# Class to output database in various flavors
#
class OutputDatabase:
    def __init__(self, val_json, val_source, val_tests, spirv_val):
        self.vj = val_json
        self.vs = val_source
        self.vt = val_tests
        self.sv = spirv_val

        # < Github Issue number : [VUIDs]>
        self.issue_map = {
            "3289" : ["06355", "06356", "06357", "06358", "06353", "06552", "06553", "06554", "06892", "06893", "06998", "08750", "08751", "08761"],
            "3305" : ["06276", "06277"],
            "4047" : ["01104"],
            "5431" : ["00643", "00644", "00647", "00648", "00668", "01133", "01134", "01135", "01455", "01518", "01519", "01520", "01539", "01540", "01541", "01542", "01543", "01544", "01746", "01747", "01750", "01751", "01754", "01755", "03264", "12331", "12332"],
            "5724" : ["03511", "03512", "03513", "03514", "03638", "03639", "03640", "03641", "03680", "03681", "03682", "03683", "03684", "03685", "03686", "03687", "03688", "03689", "03690", "03691", "03692", "03693", "03694", "03696", "03697", "04029", "04035", "04041", "04735", "04736"],
            "5749" : ["06289", "06290", "06291", "06292"],
            "5796" : ["06172", "06173", "06174", "06175", "06176", "06177"],
            "5858" : ["03407", "03645", "03646", "03647", "03651", "03652", "03653", "03663", "03664", "03671", "03672", "03703", "03704", "03705", "03706", "03709", "03717", "03724", "03768", "03769", "03770", "03773", "03777", "03801", "03808", "10607", "11845", "12281"],
            "6656" : ["08756", "08757"],
            "6801" : ["00708", "00709", "00710", "00713", "00715", "08448", "08449", "08450", "08453", "08454", "08455"],
            "7141" : ["09366", "09367", "09373", "09374"],
            "7481" : ["03561"],
            "7580" : ["06352", "06359"],
            "7688" : ["09588", "09590", "09592"],
            "8095" : ["10795", "10796", "10797"],
            "8605" : ["02707", "09370", "09371", "10198", "10929", "11004", "11029", "11034", "11038", "11044", "11046", "11048", "11049", "11051", "11052", "11055", "11056", "11065", "11066", "11068", "11117", "11118", "11120", "11122", "11140", "11142", "11144", "11149", "11150"],
            "9065" : ["10389", "10390"],
            "9081" : ["06344", "06345", "06346", "06347"],
            "9102" : ["08727", "09595", "09596", "09597"],
            "9103" : ["01182"],
            "9104" : ["06632"],
            "9176" : ["02777", "02779", "06738"],
            "9250" : ["08899", "08900", "08903", "08904", "08906", "08907"],
            "9251" : ["04475"],
            "9447" : ["03429", "03511", "03636", "03679", "04735", "04736"],
            "10618" : ["03049", "03050", "10915", "10916"],
            "11117" : ["11527", "11529", "11530", "11532", "11533", "11534", "11535", "11536", "11537", "11538", "11539", "11540", "11856", "11857", "11858", "11859", "11871", "11872", "11873"],
            "11332" : ["12289", "12290", "12291", "12292"],
            "11376" : ["00657", "00659", "00660", "00661", "00663", "01125", "01127", "01129", "01130", "01439", "01440", "01441", "01447", "01449", "01451", "01460", "01461", "01467", "01468", "03261", "03262"],
            "11377" : ["01307", "02673"],
            "11386" : ["01181", "03839", "03840", "03841", "03847"],
            "11388" : ["03821", "03822"],
            "11413" : ["01246", "01253", "01258", "01259", "06740", "10284"],
            "11414" : ["02317", "02319", "02320", "11006"],
            "11415" : ["09308", "09317"],
            "11416" : ["02263", "02264", "02265", "06420"],
            "11418" : ["09601", "10749", "10750", "11800", "11801"],
            "11419" : ["00062", "01239", "01240", "01241", "01498", "01815", "01911", "02591"],
            "11420" : ["02873", "02876", "02903", "06993", "06995", "10202"],
            "11421" : ["01524", "01530", "06013", "06017", "06023"],
            "11422" : ["09586"],
            "11423" : ["07890"],
            "11425" : ["09578"],
            "11426" : ["03592"],
            "11429" : ["09589", "09591", "09593"],
            "11431" : ["08053", "08054", "08116", "08119", "08604", "08605"],
            "11436" : ["01094", "01095", "08744", "08745"],
            "11443" : ["02741", "02742"],
            "11444" : ["02808", "03855", "10910", "10911"],
            "11445" : ["01565", "07468", "07469"],
            "11446" : ["02596", "02597", "06685", "06686", "08456", "08457", "08459"],
            "11447" : ["01245", "01282", "01293", "10285"],
            "11448" : ["00065", "03867"],
            "11451" : ["07474"],
            "11452" : ["00740", "04331", "04332", "04335", "04487", "04488", "04489", "06264", "07045", "07051", "07057", "10595", "10596", "10597", "12333", "12335", "12336", "12337"],
            "11453" : ["06022", "06533", "06534", "06535", "06536", "09299", "09300"],
            "11481" : ["02532", "02533", "03049", "03050", "09044", "09045", "09046"],
            "11793" : ["12379"],
            "11813" : ["11387", "11388"],
            "11837" : ["11165"],
            "11839" : ["03667", "04964", "04963", "04959"],
            "11855" : ["07076"],
            "11861" : ["09582"],
            "11862" : ["03880"],
            "11863" : ["10834"],
            "11864" : ["12208"],
        }

        # < Github Issue number : [VUIDs]>
        self.spirv_issue_map = {
            "6586" : ["10127", "10128", "10129", "10130", "10131", "10132", "10133"],
            "6587" : ["04678", "04679"],
            "6588" : ["10584", "10585", "10586", "10587", "10588", "10604"],
            "6590" : ["04709"],
            "6593" : ["08724"],
            "6594" : ["04687", "04689", "04690", "04691", "04692", "04716", "04865", "04694", "04693", "04696", "04697"],
        }

        self.issue_topic_map = {
            '5431'  : 'external',
            '11376' : 'external',
            '9250'  : 'gpl',
            '3305'  : 'gpuav',
            '5796'  : 'gpuav',
            '8605'  : 'gpuav',
            '9081'  : 'gpuav',
            '9251'  : 'gpuav',
            '11332' : 'gpuav',
            '11431' : 'gpuav',
            '11446' : 'gpuav',
            '11452' : 'gpuav',
            '11793' : 'gpuav',
            '11837' : 'gpuav',
            '11863' : 'gpuav',
            '11864' : 'gpuav',
            '3289'  : 'rtx',
            '5724'  : 'rtx',
            '5858'  : 'rtx',
            '7481'  : 'rtx',
            '7580'  : 'rtx',
            '9447'  : 'rtx',
            '11839' : 'rtx',
            '4047'  : 'sparse',
            '11377' : 'wsi',
            '11413' : 'wsi',
            '11447' : 'wsi',
        }

        # < Vendor : [VUIDs]>
        self.vendor_map = {
            'AMD' : ['06767', '06768', '06769', '06770', '06771', '06772', '06773'],
            'AMDX' : ['09178', '09191', '09192', '09193', '09194', '09195', '09196', '09197', '10187', '10188', '10189', '10190', '10191', '10884', '10893', '10898', '10899', '10900', '10901', '10902', '10903', '10905', '10906'],
            'ARM' : ['09396', '09397', '09398', '09399', '09682', '09683', '09698', '09699', '09711', '09753', '09754', '09759', '09762', '09858', '09878', '09879', '09908', '09909', '09912', '09913', '09914', '09915', '09918', '09931', '09932', '09933', '09940', '09941', '09942', '09946', '09947', '09948', '09949', '09952', '09953', '09956', '12372', '12373'],
            'FUCHSIA' : ['04749', '04751', '04752', '06380', '06381', '06382', '06383', '06384', '06385', '06386', '06387', '06389', '06390', '06392', '06408', '07902', '07903'],
            'HUAWEI' : ['04949', '04950', '04992', '04993', '07813'],
            'IMG' : ['09583', '09584'],
            'NV' : ['01038', '01050', '02805', '02966', '04137', '04139', '04141', '04142', '04569', '04570', '04571', '04572', '04573', '04574', '04575', '04672', '04673', '04674', '04684', '04778', '04927', '04928', '04929', '04930', '04954', '04955', '06324', '06519', '06569', '06570', '06571', '06572', '06648', '06649', '07063', '07488', '07489', '07491', '07492', '07494', '07920', '08701', '09008', '09228', '09383', '09384', '09385', '09386', '09463', '09464', '09466', '10096', '10097', '10098', '10099', '10100', '10101', '11467', '11820', '11875', '11876'],
            'NVX' : ['04951', '06595', '06596', '06597', '06598', '06599', '10578'],
            'QCOM' : ['00207', '00208', '00209', '02627', '02864', '02865', '02866', '02867', '02868', '02869', '02870', '02871', '02872', '04554', '04555', '04557', '04558', '04560', '04561', '04565', '04566', '06203', '06204', '06205', '06206', '06207', '06208', '06971', '06972', '06973', '06974', '06975', '06976', '06977', '06978', '06979', '06980', '06981', '06982', '06983', '06984', '06985', '06986', '06987', '06988', '06989', '06990', '08723', '09204', '09207', '09208', '09209', '09210', '09212', '09213', '09214', '09215', '09216', '09217', '09219', '09220', '09221', '09222', '09223', '09224', '09225', '09226', '10051', '10052', '10053', '10054', '10055', '10056', '10057', '10058', '10614', '10615', '10616', '10617', '10618', '10619', '10620', '10621', '10622', '10623', '10624', '10625', '10626', '10627', '10628', '10629', '10630', '10631', '10632', '10633', '10634', '10635', '10639', '10640', '10641', '10642', '10643', '10644', '10645', '10646', '10647', '10648', '10649', '10650', '10651', '10652', '10653', '10654', '10655', '10656', '10657', '10658', '10659', '10660', '10661', '10662', '10663', '10664', '10665', '10666', '10667', '10670', '10671', '10673', '10674', '10675', '10676', '10677', '10678', '10679', '10681', '10682', '10683', '10686', '10687', '10688', '10689', '10690', '10691', '10692', '10693', '10694', '10695', '10696', '10697', '10698', '10699', '10700', '10701', '10702', '10703', '10704', '10705', '10706', '10707', '10708', '10709', '10710', '10711', '10712', '10713', '10714', '10755', '11830', '11831', '11832', '11833', '11834', '11835', '11836', '11837', '11838', '11842', '11843', '11844', '11877', '12352', '12353', '12354'],
            'QNX' : ['08941', '08942', '08943', '08944', '08945', '08946', '08951', '08952', '08953', '08954', '08955', '08957', '08958', '08959', '08960', '08961', '08962'],
            'VAVLE' : ['10918', '10919', '10920', '10921', '10922', '10923', '10924']
        }

        self.topic_vuid_map = {
        'VK_EXT_graphics_pipeline_library' : ['06611', '06616', '06617', '06618', '06619', '06623', '06624', '06628', '06684'],
            'VK_EXT_opacity_micromap' : ['03678', '07334', '07335', '07432', '07433', '07434', '07435', '07436', '07437', '07438', '07440', '07441', '07461', '07462', '07463', '07464', '07465', '07466', '07467', '07501', '07502', '07508', '07509', '07510', '07511', '07512', '07517', '07518', '07519', '07520', '07521', '07522', '07523', '07524', '07525', '07526', '07527', '07528', '07529', '07530', '07532', '07533', '07534', '07535', '07538', '07539', '07540', '07541', '07545', '07546', '07547', '07549', '07550', '07552', '07556', '07557', '07558', '07559', '07561', '07562', '07565', '07567', '07568', '07572', '07576', '07577', '07578', '07579', '08704', '08705', '08706', '08707', '08708', '08709', '09180', '10071', '10072', '10719', '10722', '10723', '10904', '11821'],
            'devicegroup' : ['00085', '00376', '00377', '00691', '00692', '00693', '00694', '01118', '01119', '01152', '01157', '01167', '01297', '01298', '01299', '01300', '01301', '01302', '01303', '01605', '01626', '01628', '01629', '01635', '01637', '01638', '01639', '01640', '01641', '03826', '03833', '03846', '03888', '03889'],
            'external' : ['08922'],
            'gpuav' : ['02713', '04745', '07117', '07118', '07288', '07699', '08601', '08731', '09003', '09190', '09218', '09565', '09645', '10134', '10135', '10136', '10137', '10138', '10139', '10140', '10141', '10142', '10143', '10144', '10145', '10146', '10147', '10148', '10149', '10934', '10945', '10950', '10961', '10962', '10966', '10967', '10968', '10969', '10970', '10971', '10975', '11166', '11297', '11298', '11299', '11300', '11301', '11302', '11304', '11305', '11306', '11309', '11319', '11340', '11341', '11342', '11343', '11345', '11348', '11349', '11350', '11372', '11373', '11374', '11379', '11380', '11382', '11383', '11384', '11397', '11398', '11437', '11438', '11439', '11440', '11441', '11442', '11443', '11450', '11455', '11456', '11481', '11769', '12211', '12212', '12214', '12282', '12283', '12284', '12285', '12286', '12287'],
            'rtx' : ['03414', '03746', '04960', '10418', '11485', '11855'],
            'sparse' : ['01103', '06287', '10938', '10939'],
            'wsi' : ['00081', '10409', '10411', '10412', '10416']
        }

    def dump_html(self, filename):
        print(f'\nDumping TODO list to html file: {filename}')
        preamble = '<!DOCTYPE html>\n<html>\n<head>\n<style>\ntable, th, td {\n border: 1px solid black;\n border-collapse: collapse; \n}\n</style>\n<body>\n<h2>List of current Valid Usages not being validated</h2>\n<font size="2" face="Arial">\n'
        table = []
        table.append('<table style="width:100%">\n<tr><th>VUID</th><th>TYPE</th><th>Comment</th></tr>\n')

        total_count = 0
        vendor_count = 0
        micro_map_count = 0
        gpl_count = 0
        rtx_count = 0
        wsi_count = 0
        gpuav_count = 0
        device_group_count = 0
        external_count = 0
        sparse_count = 0

        vuid_list = list(self.vj.all_vuids)
        vuid_list.sort()
        for vuid in vuid_list:
            if vuid in self.vs.all_vuids:
                continue
            total_count += 1
            table.append(f'<tr><th>{vuid}</th>')
            assert(len(self.vj.vuid_db[vuid]) == 1)
            table.append(f'<th>{self.vj.vuid_db[vuid][0]["type"]}</th>')

            vendor = IsVendor(vuid)
            if vendor is not None:
                vendor_count += 1
                table.append(f'<th>{vendor} vendor extension</th></tr>')
                continue

            vendor = None
            for key, list_of_vuids in self.vendor_map.items():
                if vuid[-5:] in list_of_vuids:
                    vendor = key
                    break
            if vendor:
                vendor_count += 1
                table.append(f'<th>{vendor} vendor extension</th></tr>')
                continue

            issue_number = None
            for key, list_of_vuids in self.issue_map.items():
                if vuid[-5:] in list_of_vuids:
                    issue_number = key
                    break
            if issue_number:
                table.append(f'<th><a href=https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/{issue_number}> Issue {issue_number}</th></tr>')
                if issue_number in self.issue_topic_map:
                    if self.issue_topic_map[issue_number] == 'gpuav':
                        gpuav_count += 1
                    elif self.issue_topic_map[issue_number] == 'external':
                        external_count += 1
                    elif self.issue_topic_map[issue_number] == 'rtx':
                        rtx_count += 1
                        gpuav_count += 1
                    elif self.issue_topic_map[issue_number] == 'wsi':
                        wsi_count += 1
                    elif self.issue_topic_map[issue_number] == 'gpl':
                        gpl_count += 1
                    elif self.issue_topic_map[issue_number] == 'sparse':
                        sparse_count += 1
                continue

            issue_number = None
            for key, list_of_vuids in self.spirv_issue_map.items():
                if vuid[-5:] in list_of_vuids:
                    issue_number = key
                    break
            if issue_number:
                table.append(f'<th><a href=https://github.com/KhronosGroup/SPIRV-Tools/issues/{issue_number}> SPIRV-Tools Issue {issue_number}</th></tr>')
                continue

            topic = None
            for key, list_of_vuids in self.topic_vuid_map.items():
                if vuid[-5:] in list_of_vuids:
                    topic = key
                    break
            if topic:
                if topic == 'VK_EXT_opacity_micromap':
                    micro_map_count += 1
                    table.append(f'<th>VK_EXT_opacity_micromap</th></tr>')
                elif topic == 'VK_EXT_graphics_pipeline_library':
                    gpl_count += 1
                    table.append(f'<th>VK_EXT_graphics_pipeline_library</th></tr>')
                elif topic == 'gpuav':
                    gpuav_count += 1
                    table.append(f'<th>Requires GPU-AV</th></tr>')
                elif topic == 'rtx':
                    rtx_count += 1
                    gpuav_count += 1
                    table.append(f'<th>RTX with GPU-AV</th></tr>')
                elif topic == 'devicegroup':
                    device_group_count += 1
                    table.append(f'<th>Device Groups</th></tr>')
                elif topic == 'external':
                    external_count += 1
                    table.append(f'<th>External Memory/Sync</th></tr>')
                elif topic == 'sparse':
                    sparse_count += 1
                    table.append(f'<th>Sparse Memory</th></tr>')
                elif topic == 'wsi':
                    wsi_count += 1
                    table.append(f'<th>WSI</th></tr>')
                continue

            table.append('<th>To be categorized</th></tr>')

        table.append('</table>\n</body>\n</html>\n')

        stats = []
        stats.append(f'<h1>Total Remaing VUs: {total_count}</h1>')
        stats.append('<ul>')
        stats.append(f'<li><b>Vendor extensions</b>: {vendor_count} ({(vendor_count / total_count):.1%})</li>')
        stats.append(f'<li><b>VK_EXT_opacity_micromap</b>: {micro_map_count} ({(micro_map_count / total_count):.1%})</li>')
        stats.append(f'<li><b>VK_EXT_graphics_pipeline_library</b>: {gpl_count} ({(gpl_count / total_count):.1%})</li>')
        stats.append(f'<li><b>External Memory/Sync</b>: {external_count} ({(external_count / total_count):.1%})</li>')
        stats.append(f'<li><b>WSI</b>: {wsi_count} ({(wsi_count / total_count):.1%})</li>')
        stats.append(f'<li><b>Device Groups</b>: {device_group_count} ({(device_group_count / total_count):.1%})</li>')
        stats.append(f'<li><b>Sparse Memory</b>: {sparse_count} ({(sparse_count / total_count):.1%})</li>')
        stats.append(f'<li><b>Require GPU-AV support</b>: {gpuav_count} ({(gpuav_count / total_count):.1%})</li>')
        stats.append(f'<ul><li><b>Ray Tracing (GPU-AV)</b>: {rtx_count} ({(rtx_count / total_count):.1%})</li></ul>')
        stats.append('</ul>')

        with open(filename, 'w', encoding='utf-8') as hfile:
            hfile.write(preamble)
            hfile.write(''.join(stats))
            hfile.write(''.join(table))

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

    def load(self):
        if self.enabled is False:
            return
        # Get hash from git if available
        try:
            git_dir = os.path.join(self.repo_path, '.git')
            process = subprocess.Popen(['git', '--git-dir='+git_dir ,'rev-parse', 'HEAD'], shell=False, stdout=subprocess.PIPE)
            self.version = process.communicate()[0].strip().decode('utf-8')[:7]
            if process.poll() != 0:
                throw
        except:
            # leave as default
            print(f'Could not find .git file for version of SPIR-V tools, marking as {self.version}')

        # Find and parse files with VUIDs in source
        for path in spirvtools_source_files:
            self.source_files.extend(glob.glob(os.path.join(self.repo_path, path)))
        for path in spirvtools_test_files:
            self.test_files.extend(glob.glob(os.path.join(self.repo_path, path)))


def main(argv):
    HTML_FILENAME = "validation_error_database.html"

    parser = argparse.ArgumentParser()
    parser.add_argument('json_file', help="registry file 'validusage.json'")
    parser.add_argument('-api',
                        default='vulkan',
                        choices=['vulkan'],
                        help='Specify API name to use')
    parser.add_argument('-c', action='store_true',
                        help='report consistency warnings')
    parser.add_argument('-spirvtools', metavar='PATH',
                        help='when pointed to root directory of SPIRV-Tools repo, will search the repo for VUs that are implemented there')
    parser.add_argument('-html', nargs='?', const=HTML_FILENAME, metavar='FILENAME',
                        help=f'export the TODO error database in html format to <FILENAME>, defaults to {HTML_FILENAME}')
    parser.add_argument('-remove_duplicates', action='store_true',
                        help='remove duplicate VUID numbers')
    parser.add_argument('-summary', action='store_true',
                        help='output summary of VUID coverage')
    args = parser.parse_args()

    # We need python modules found in the registry directory. This assumes that the validusage.json file is in that directory,
    # and hasn't been copied elsewhere.
    registry_dir = os.path.dirname(args.json_file)
    sys.path.insert(0, registry_dir)

    layer_source_files = [repo_relative(path) for path in [
        'layers/error_message/unimplementable_validation.h',
        'layers/state_tracker/video_session_state.cpp',
        'layers/state_tracker/cmd_buffer_state.cpp',
        'layers/layer_options.cpp',
        'layers/core_checks/cc_buffer_address.h',
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
    layer_source_files.extend(glob.glob(os.path.join(repo_relative('layers/gpuav/'), '*.cpp')))
    layer_source_files.extend(glob.glob(os.path.join(repo_relative('layers/gpuav/validation_cmd/'), '*.cpp')))
    layer_source_files.extend(glob.glob(os.path.join(repo_relative('layers/gpuav/core/'), '*.cpp')))
    layer_source_files.extend(glob.glob(os.path.join(repo_relative('layers/gpuav/descriptor_validation/'), '*.cpp')))
    layer_source_files.extend(glob.glob(os.path.join(repo_relative('layers/gpuav/error_message/'), '*.cpp')))
    layer_source_files.extend(glob.glob(os.path.join(repo_relative('layers/gpuav/instrumentation/'), '*.cpp')))

    test_source_files = glob.glob(os.path.join(repo_relative('tests/unit'), '*.cpp'))

    global remove_duplicates
    remove_duplicates = args.remove_duplicates

    # Load in SPIRV-Tools if passed in
    spirv_val = SpirvValidation(args.spirvtools)
    spirv_val.load()

    # Parse validusage json
    val_json = ValidationJSON(args.json_file)
    val_json.parse()
    if remove_duplicates:
        val_json.dedup()
    exp_json = len(val_json.explicit_vuids)
    imp_json = len(val_json.implicit_vuids)
    all_json = len(val_json.all_vuids)

    # Parse layer source files
    val_source = ValidationSource(layer_source_files)
    val_source.parse(spirv_val)
    if remove_duplicates:
        val_source.dedup()
    val_source.action_dedup(val_json.explicit_vuids)
    exp_checks = len(val_source.explicit_vuids)
    imp_checks = len(val_source.implicit_vuids)
    all_checks = exp_checks + imp_checks
    spirv_exp_checks = len(spirv_val.source_explicit_vuids) if spirv_val.enabled else 0
    spirv_imp_checks = len(spirv_val.source_implicit_vuids) if spirv_val.enabled else 0
    spirv_all_checks = (spirv_exp_checks + spirv_imp_checks) if spirv_val.enabled else 0

    # Parse test files
    val_tests = ValidationTests(test_source_files)
    val_tests.parse(spirv_val)
    if remove_duplicates:
        val_tests.dedup()
    exp_tests = len(val_tests.explicit_vuids)
    imp_tests = len(val_tests.implicit_vuids)
    all_tests = len(val_tests.all_vuids)
    spirv_exp_tests = len(spirv_val.test_explicit_vuids) if spirv_val.enabled else 0
    spirv_imp_tests = len(spirv_val.test_implicit_vuids) if spirv_val.enabled else 0
    spirv_all_tests = (spirv_exp_tests + spirv_imp_tests) if spirv_val.enabled else 0

    # Process stats
    if args.summary:
        if spirv_val.enabled:
            print(f'\nValidation Statistics (using validusage.json version {val_json.api_version} and SPIRV-Tools version {spirv_val.version})')
        else:
            print(f'\nValidation Statistics (using validusage.json version {val_json.api_version})')
        print(f"  VUIDs defined in JSON file:  {exp_json:04d} explicit, {imp_json:04d} implicit, {all_json:04d} total.")
        print(f"  VUIDs checked in layer code: {exp_checks:04d} explicit, {imp_checks:04d} implicit, {all_checks:04d} total.")
        if spirv_val.enabled:
            print(f"             From SPIRV-Tools: {spirv_exp_checks:04d} explicit, {spirv_imp_checks:04d} implicit, {spirv_all_checks:04d} total.")
        print(f"  VUIDs tested in layer tests: {exp_tests:04d} explicit, {imp_tests:04d} implicit, {all_tests:04d} total.")
        if spirv_val.enabled:
            print(f"             From SPIRV-Tools: {spirv_exp_tests:04d} explicit, {spirv_imp_tests:04d} implicit, {spirv_all_tests:04d} total.")

        print("\nVUID check coverage")
        print(f"  Explicit VUIDs checked: {(100.0 * exp_checks / exp_json):.1f}% ({exp_checks} checked vs {exp_json} defined)")
        print(f"  Implicit VUIDs checked: {(100.0 * imp_checks / imp_json):.1f}% ({imp_checks} checked vs {imp_json} defined)")
        print(f"  Overall VUIDs checked:  {(100.0 * all_checks / all_json):.1f}% ({all_checks} checked vs {all_json} defined)")

        unimplemented_explicit = val_json.all_vuids - val_source.all_vuids
        vendor_count = sum(1 for vuid in unimplemented_explicit if IsVendor(vuid) is not None)
        print(f'    {len(unimplemented_explicit)} VUID checks remain unimplemented ({vendor_count} are from Vendor objects)')

        print("\nVUID test coverage")
        print(f"  Explicit VUIDs tested: {(100.0 * exp_tests / exp_checks):.1f}% ({exp_tests} tested vs {exp_checks} checks)")
        print(f"  Implicit VUIDs tested: {(100.0 * imp_tests / imp_checks):.1f}% ({imp_tests} tested vs {imp_checks} checks)")
        print(f"  Overall VUIDs tested:  {(100.0 * all_tests / all_checks):.1f}% ({all_tests} tested vs {all_checks} checks)")

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
    # (We use to support CSV and Text, but neither were used and not worth maintaining)
    if args.html:
        db_out = OutputDatabase(val_json, val_source, val_tests, spirv_val)
        db_out.dump_html(args.html)

if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
