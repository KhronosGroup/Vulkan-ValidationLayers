#!/usr/bin/python3 -i
#
# Copyright (c) 2021-2023 The Khronos Group Inc.
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

import sys
import os
import re
import json
from generators.base_generator import BaseGenerator

#
# Generate SPIR-V grammar helper for SPIR-V opcodes, enums, etc
# Has zero relationship to the Vulkan API (doesn't use vk.xml)
class SpirvGrammarHelperOutputGenerator(BaseGenerator):
    def __init__(self,
                 grammar):
        BaseGenerator.__init__(self)

        self.opcodes = dict()
        self.atomicsOps = []
        self.groupOps = []
        self.imageAcesssOps = []
        self.sampledImageAccessOps = []
        self.imageGatherOps = []
        self.imageSampleOps = []
        self.imageFetchOps = []
        self.storageClassList = [] # list of storage classes
        self.executionModelList = []
        self.executionModeList = []
        self.decorationList = []
        self.builtInList = []
        self.dimList = []
        self.cooperativeMatrixList = []
        # Need range to be large as largest possible operand index
        self.imageOperandsParamCount = [[] for i in range(3)]

        self.parseGrammar(grammar)

    def addToStringList(self, operandKind, kind, list, ignoreList = []):
        if operandKind['kind'] == kind:
            values = [] # prevent alias from being duplicatd
            for enum in operandKind['enumerants']:
                if enum['value'] not in values and enum['enumerant'] not in ignoreList:
                    list.append(enum['enumerant'])
                    values.append(enum['value'])

    #
    # Takes the SPIR-V Grammar JSON and parses it
    # Emulates the gen*() functions the vk.xml calls
    #
    # In the future, IF more then this generator wants to use the grammar
    # it would be better to move the file opening to run_generators.py
    def parseGrammar(self, grammar):
        with open(grammar, 'r') as jsonFile:
            data = json.load(jsonFile)
            instructions = data['instructions']
            operandKinds = data['operand_kinds']

            # Build list from json of all capabilities that are only for kernel
            # This needs to be done before loop instructions
            kernelCapability = ['Kernel']
            # some SPV_INTEL_* are not allowed in Vulkan and are just adding unused opcodes
            # TODO bring in vk.xml to cross check valid extensions/capabilities instead of starting another hardcoded list
            kernelCapability.append('ArbitraryPrecisionIntegersINTEL')
            kernelCapability.append('ArbitraryPrecisionFixedPointINTEL')
            kernelCapability.append('ArbitraryPrecisionFloatingPointINTEL')
            kernelCapability.append('SubgroupAvcMotionEstimationINTEL')
            kernelCapability.append('SubgroupAvcMotionEstimationIntraINTEL')
            kernelCapability.append('SubgroupAvcMotionEstimationChromaINTEL')

            for operandKind in operandKinds:
                if operandKind['kind'] == 'Capability':
                    for enum in operandKind['enumerants']:
                        if 'capabilities' in enum and len(enum['capabilities']) == 1 and enum['capabilities'][0] == 'Kernel':
                            kernelCapability.append(enum['enumerant'])
                if operandKind['kind'] == 'ImageOperands':
                    values = [] # prevent alias from being duplicatd
                    for enum in operandKind['enumerants']:
                        count = 0  if 'parameters' not in enum else len(enum['parameters'])
                        if enum['value'] not in values:
                            self.imageOperandsParamCount[count].append(enum['enumerant'])
                            values.append(enum['value'])
                self.addToStringList(operandKind, 'StorageClass', self.storageClassList)
                self.addToStringList(operandKind, 'ExecutionModel', self.executionModelList)
                self.addToStringList(operandKind, 'ExecutionMode', self.executionModeList)
                self.addToStringList(operandKind, 'Decoration', self.decorationList)
                self.addToStringList(operandKind, 'BuiltIn', self.builtInList)
                self.addToStringList(operandKind, 'Dim', self.dimList)
                self.addToStringList(operandKind, 'CooperativeMatrixOperands', self.cooperativeMatrixList, ['NoneKHR'])

            for instruction in instructions:
                opname = instruction['opname']
                opcode = instruction['opcode']
                if 'capabilities' in instruction:
                    notSupported = True
                    for capability in instruction['capabilities']:
                        if capability not in kernelCapability:
                            notSupported = False
                            break
                    if notSupported:
                        continue # If just 'Kernel' capabilites then it's ment for OpenCL and skip instruction

                # Nice side effect of using a dict here is alias opcodes will be last in the grammar file
                # ex: OpTypeAccelerationStructureNV will be replaced by OpTypeAccelerationStructureKHR
                self.opcodes[opcode] = {
                    'name' : opname,
                    'hasType' : "false",
                    'hasResult' : "false",

                    'memoryScopePosition' : 0,
                    'executionScopePosition' : 0,
                    'imageOperandsPosition' : 0,
                    'storageClassPosition' : 0,

                    'imageRefPosition' : 0,
                    'sampledImageRefPosition' : 0,
                }

                if instruction['class'] == 'Atomic':
                    self.atomicsOps.append(opname)
                if instruction['class'] == 'Non-Uniform':
                    self.groupOps.append(opname)
                if re.search("OpImage.*Gather", opname) is not None:
                    self.imageGatherOps.append(opname)
                if re.search("OpImageFetch.*", opname) is not None:
                    self.imageFetchOps.append(opname)
                if re.search("OpImageSample.*", opname) is not None:
                    self.imageSampleOps.append(opname)
                if 'operands' in instruction:
                    for index, operand in enumerate(instruction['operands']):
                        if operand['kind'] == 'IdResultType':
                            self.opcodes[opcode]['hasType'] = "true"
                        if operand['kind'] == 'IdResult':
                            self.opcodes[opcode]['hasResult'] = "true"
                        # some instructions have both types of IdScope
                        # OpReadClockKHR has the wrong 'name' as 'Scope'
                        if operand['kind'] == 'IdScope':
                            if operand['name'] == '\'Execution\'' or operand['name'] == '\'Scope\'':
                                self.opcodes[opcode]['executionScopePosition'] = index + 1
                            elif operand['name'] == '\'Memory\'':
                                self.opcodes[opcode]['memoryScopePosition'] = index + 1
                            elif operand['name'] == '\'Visibility\'':
                                continue # ignore
                            else:
                                print(f'Error: unknown operand {opname} with IdScope {operand["name"]} not handled correctly\n')
                                sys.exit(1)
                        if operand['kind'] == 'ImageOperands':
                            self.opcodes[opcode]['imageOperandsPosition'] = index + 1
                        if operand['kind'] == 'StorageClass':
                            self.opcodes[opcode]['storageClassPosition'] = index + 1
                        if operand['kind'] == 'IdRef':
                            if operand['name'] == '\'Image\'':
                                self.opcodes[opcode]['imageRefPosition'] = index + 1
                            elif operand['name'] == '\'Sampled Image\'':
                                self.opcodes[opcode]['sampledImageRefPosition'] = index + 1

                if re.search("OpImage*", opname) is not None:
                    info = self.opcodes[opcode]
                    imageRef = info['imageRefPosition']
                    sampledImageRef = info['sampledImageRefPosition']
                    if imageRef == 0 and sampledImageRef == 0:
                        # things like OpImageSparseTexelsResident don't do an actual image operation
                        continue
                    elif imageRef != 0 and sampledImageRef != 0:
                        print("Error: unknown opcode {} not handled correctly\n".format(opname))
                        sys.exit(1)
                    elif imageRef != 0:
                        self.imageAcesssOps.append(opname)
                    elif sampledImageRef != 0:
                        self.sampledImageAccessOps.append(opname)

    def generate(self):
        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2021-2023 The Khronos Group Inc.
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
            * This file is related to anything that is found in the SPIR-V grammar
            * file found in the SPIRV-Headers. Mainly used for SPIR-V util functions.
            *
            ****************************************************************************/\n''')
        self.write('// NOLINTBEGIN') # Wrap for clang-tidy to ignore

        if self.filename == 'spirv_grammar_helper.h':
            self.generateHeader()
        elif self.filename == 'spirv_grammar_helper.cpp':
            self.generateSource()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateHeader(self):
        out = []
        out.append('''
            #pragma once
            #include <cstdint>
            #include <string>
            #include <spirv/unified1/spirv.hpp>

            bool AtomicOperation(uint32_t opcode);
            bool GroupOperation(uint32_t opcode);

            bool ImageGatherOperation(uint32_t opcode);
            bool ImageFetchOperation(uint32_t opcode);
            bool ImageSampleOperation(uint32_t opcode);
            uint32_t ImageAccessOperandsPosition(uint32_t opcode);
            uint32_t SampledImageAccessOperandsPosition(uint32_t opcode);

            bool OpcodeHasType(uint32_t opcode);
            bool OpcodeHasResult(uint32_t opcode);

            uint32_t OpcodeMemoryScopePosition(uint32_t opcode);
            uint32_t OpcodeExecutionScopePosition(uint32_t opcode);
            uint32_t OpcodeImageOperandsPosition(uint32_t opcode);

            uint32_t ImageOperandsParamCount(uint32_t opcode);

            const char* string_SpvOpcode(uint32_t opcode);
            const char* string_SpvStorageClass(uint32_t storage_class);
            const char* string_SpvExecutionModel(uint32_t execution_model);
            const char* string_SpvExecutionMode(uint32_t execution_mode);
            const char* string_SpvDecoration(uint32_t decoration);
            const char* string_SpvBuiltIn(uint32_t built_in);
            const char* string_SpvDim(uint32_t dim);
            std::string string_SpvCooperativeMatrixOperands(uint32_t mask);
            ''')
        self.write("".join(out))

    def generateSource(self):
        out = []
        out.append('''
            #include "containers/custom_containers.h"
            #include "spirv_grammar_helper.h"
            #include "state_tracker/shader_instruction.h"

            // All information related to each SPIR-V opcode instruction
            struct InstructionInfo {
                const char* name;
                bool has_type;    // always operand 0 if present
                bool has_result;  // always operand 1 if present

                uint32_t memory_scope_position;     // operand ID position or zero if not present
                uint32_t execution_scope_position;  // operand ID position or zero if not present
                uint32_t image_operands_position;   // operand ID position or zero if not present

                uint32_t image_access_operands_position;          // operand ID position or zero if not present
                uint32_t sampled_image_access_operands_position;  // operand ID position or zero if not present
            };
            ''')

        out.append('''
// Static table to replace having many large switch statement functions for looking up each part
// of a given SPIR-V opcode instruction
//
// clang-format off
static const vvl::unordered_map<uint32_t, InstructionInfo> kInstructionTable {
''')
        for info in self.opcodes.values():
            out.append('    {{spv::{}, {{"{}", {}, {}, {}, {}, {}, {}, {}}}}},\n'.format(
                info['name'],
                info['name'],
                info['hasType'],
                info['hasResult'],
                info['memoryScopePosition'],
                info['executionScopePosition'],
                info['imageOperandsPosition'],
                info['imageRefPosition'],
                info['sampledImageRefPosition'],
            ))
        out.append('};\n')
        out.append('// clang-format on\n')

        # \n is not allowed in f-string until 3.12
        atomicCase = "\n".join([f"        case spv::{f}:" for f in self.atomicsOps])
        groupCase = "\n".join([f"        case spv::{f}:" for f in self.groupOps])
        out.append(f'''
            // Any non supported operation will be covered with VUID 01090
            bool AtomicOperation(uint32_t opcode) {{
                bool found = false;
                switch (opcode) {{
            {atomicCase}
                        found = true;
                        break;
                    default:
                        break;
                }}
                return found;
            }}

            // Any non supported operation will be covered with VUID 01090
            bool GroupOperation(uint32_t opcode) {{
                bool found = false;
                switch (opcode) {{
            {groupCase}
                        found = true;
                        break;
                    default:
                        break;
                }}
                return found;
            }}
            ''')

        out.append('''
            spv::StorageClass Instruction::StorageClass() const {
                spv::StorageClass storage_class = spv::StorageClassMax;
                switch (Opcode()) {
            ''')
        for info in [x for x in self.opcodes.values() if x['storageClassPosition'] != 0]:
            out.append(f'        case spv::{info["name"]}:\n')
            out.append(f'            storage_class = static_cast<spv::StorageClass>(Word({info["storageClassPosition"]}));\n')
            out.append('            break;\n')
        out.append('''
                    default:
                        break;
                }
                return storage_class;
            }
            ''')

        imageGatherOpsCase = "\n".join([f"        case spv::{f}:" for f in self.imageGatherOps])
        imageFetchOpsCase = "\n".join([f"        case spv::{f}:" for f in self.imageFetchOps])
        imageSampleOpsCase = "\n".join([f"        case spv::{f}:" for f in self.imageSampleOps])
        out.append(f'''
            bool ImageGatherOperation(uint32_t opcode) {{
                bool found = false;
                switch (opcode) {{
            {imageGatherOpsCase}
                        found = true;
                        break;
                    default:
                        break;
                }}
                return found;
            }}

            bool ImageFetchOperation(uint32_t opcode) {{
                bool found = false;
                switch (opcode) {{
            {imageFetchOpsCase}
                        found = true;
                        break;
                    default:
                        break;
                }}
                return found;
            }}

            bool ImageSampleOperation(uint32_t opcode) {{
                bool found = false;
                switch (opcode) {{
            {imageSampleOpsCase}
                        found = true;
                        break;
                    default:
                        break;
                }}
                return found;
            }}
            ''')

        out.append('''
            // Return operand position of Image IdRef or zero if there is none
            uint32_t ImageAccessOperandsPosition(uint32_t opcode) {
                uint32_t position = 0;
                auto format_info = kInstructionTable.find(opcode);
                if (format_info != kInstructionTable.end()) {
                    position = format_info->second.image_access_operands_position;
                }
                return position;
            }

            // Return operand position of 'Sampled Image' IdRef or zero if there is none
            uint32_t SampledImageAccessOperandsPosition(uint32_t opcode) {
                uint32_t position = 0;
                auto format_info = kInstructionTable.find(opcode);
                if (format_info != kInstructionTable.end()) {
                    position = format_info->second.sampled_image_access_operands_position;
                }
                return position;
            }

            bool OpcodeHasType(uint32_t opcode) {
                bool has_type = false;
                auto format_info = kInstructionTable.find(opcode);
                if (format_info != kInstructionTable.end()) {
                    has_type = format_info->second.has_type;
                }
                return has_type;
            }

            bool OpcodeHasResult(uint32_t opcode) {
                bool has_result = false;
                auto format_info = kInstructionTable.find(opcode);
                if (format_info != kInstructionTable.end()) {
                    has_result = format_info->second.has_result;
                }
                return has_result;
            }

            // Return operand position of Memory Scope <ID> or zero if there is none
            uint32_t OpcodeMemoryScopePosition(uint32_t opcode) {
                uint32_t position = 0;
                auto format_info = kInstructionTable.find(opcode);
                if (format_info != kInstructionTable.end()) {
                    position = format_info->second.memory_scope_position;
                }
                return position;
            }

            // Return operand position of Execution Scope <ID> or zero if there is none
            uint32_t OpcodeExecutionScopePosition(uint32_t opcode) {
                uint32_t position = 0;
                auto format_info = kInstructionTable.find(opcode);
                if (format_info != kInstructionTable.end()) {
                    position = format_info->second.execution_scope_position;
                }
                return position;
            }

            // Return operand position of Image Operands <ID> or zero if there is none
            uint32_t OpcodeImageOperandsPosition(uint32_t opcode) {
                uint32_t position = 0;
                auto format_info = kInstructionTable.find(opcode);
                if (format_info != kInstructionTable.end()) {
                    position = format_info->second.image_operands_position;
                }
                return position;
            }

            // Return number of optional parameter from ImageOperands
            uint32_t ImageOperandsParamCount(uint32_t image_operand) {
                uint32_t count = 0;
                switch (image_operand) {
            ''')

        for index, operands in enumerate(self.imageOperandsParamCount):
            for operand in operands:
                if operand == 'None': # not sure why header is not consistent with this
                    out.append(f'        case spv::ImageOperandsMask{operand}:\n')
                else:
                    out.append(f'        case spv::ImageOperands{operand}Mask:\n')
            if len(operands) != 0:
                out.append(f'            return {index};\n')
        out.append('''
                    default:
                        break;
                }
                return count;
            }
            ''')

        out.append('''
            const char* string_SpvOpcode(uint32_t opcode) {
                auto format_info = kInstructionTable.find(opcode);
                if (format_info != kInstructionTable.end()) {
                    return format_info->second.name;
                } else {
                    return "Unknown Opcode";
                }
            }
            ''')

        out.append(f'''
            const char* string_SpvStorageClass(uint32_t storage_class) {{
                switch(storage_class) {{
            {"".join([f"""        case spv::StorageClass{x}:
                        return "{x}";
            """ for x in self.storageClassList])}
                    default:
                        return "Unknown Storage Class";
                }}
            }}

            const char* string_SpvExecutionModel(uint32_t execution_model) {{
                switch(execution_model) {{
            {"".join([f"""        case spv::ExecutionModel{x}:
                        return "{x}";
            """ for x in self.executionModelList])}
                    default:
                        return "Unknown Execution Model";
                }}
            }}

            const char* string_SpvExecutionMode(uint32_t execution_mode) {{
                switch(execution_mode) {{
            {"".join([f"""        case spv::ExecutionMode{x}:
                        return "{x}";
            """ for x in self.executionModeList])}
                    default:
                        return "Unknown Execution Mode";
                }}
            }}

            const char* string_SpvDecoration(uint32_t decoration) {{
                switch(decoration) {{
            {"".join([f"""        case spv::Decoration{x}:
                        return "{x}";
            """ for x in self.decorationList])}
                    default:
                        return "Unknown Decoration";
                }}
            }}

            const char* string_SpvBuiltIn(uint32_t built_in) {{
                switch(built_in) {{
            {"".join([f"""        case spv::BuiltIn{x}:
                        return "{x}";
            """ for x in self.builtInList])}
                    default:
                        return "Unknown BuiltIn";
                }}
            }}

            const char* string_SpvDim(uint32_t dim) {{
                switch(dim) {{
            {"".join([f"""        case spv::Dim{x}:
                        return "{x}";
            """ for x in self.dimList])}
                    default:
                        return "Unknown Dim";
                }}
            }}

            static const char* string_SpvCooperativeMatrixOperandsMask(spv::CooperativeMatrixOperandsMask mask) {{
                switch(mask) {{
                    case spv::CooperativeMatrixOperandsMaskNone:
                        return "None";
            {"".join([f"""        case spv::CooperativeMatrixOperands{x}Mask:
                        return "{x}";
            """ for x in self.cooperativeMatrixList])}
                    default:
                        return "Unknown CooperativeMatrixOperandsMask";
                }}
            }}

            std::string string_SpvCooperativeMatrixOperands(uint32_t mask) {{
                std::string ret;
                while(mask) {{
                    if (mask & 1) {{
                        if(!ret.empty()) ret.append("|");
                        ret.append(string_SpvCooperativeMatrixOperandsMask(static_cast<spv::CooperativeMatrixOperandsMask>(1U << mask)));
                    }}
                    mask >>= 1;
                }}
                if (ret.empty()) ret.append("CooperativeMatrixOperandsMask(0)");
                return ret;
            }}
            ''')
        self.write("".join(out))