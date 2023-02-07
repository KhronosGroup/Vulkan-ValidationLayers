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

import os,re,sys,string,json
import xml.etree.ElementTree as etree
from generator import *
from collections import namedtuple
from common_codegen import *

# This is a workaround to use a Python 2.7 and 3.x compatible syntax
from io import open

class SpirvGrammarHelperOutputGeneratorOptions(GeneratorOptions):
    def __init__(self,
                 conventions = None,
                 filename = None,
                 directory = '.',
                 genpath = None,
                 apiname = 'vulkan',
                 profile = None,
                 versions = '.*',
                 emitversions = '.*',
                 defaultExtensions = 'vulkan',
                 addExtensions = None,
                 removeExtensions = None,
                 emitExtensions = None,
                 emitSpirv = None,
                 sortProcedure = regSortFeatures,
                 genFuncPointers = True,
                 protectFile = True,
                 protectFeature = True,
                 apicall = 'VKAPI_ATTR ',
                 apientry = 'VKAPI_CALL ',
                 apientryp = 'VKAPI_PTR *',
                 indentFuncProto = True,
                 indentFuncPointer = False,
                 alignFuncParam = 48,
                 expandEnumerants = False,
                 grammar = None):
        GeneratorOptions.__init__(self,
                conventions = conventions,
                filename = filename,
                directory = directory,
                genpath = genpath,
                apiname = apiname,
                profile = profile,
                versions = versions,
                emitversions = emitversions,
                defaultExtensions = defaultExtensions,
                addExtensions = addExtensions,
                removeExtensions = removeExtensions,
                emitExtensions = emitExtensions,
                emitSpirv = emitSpirv,
                sortProcedure = sortProcedure)
        self.genFuncPointers = genFuncPointers
        self.protectFile     = protectFile
        self.protectFeature  = protectFeature
        self.apicall         = apicall
        self.apientry        = apientry
        self.apientryp       = apientryp
        self.indentFuncProto = indentFuncProto
        self.indentFuncPointer = indentFuncPointer
        self.alignFuncParam  = alignFuncParam
        self.expandEnumerants = expandEnumerants
        self.grammar = grammar
#
# SpirvGrammarHelperOutputGenerator - Generate SPIR-V grammar helper
# for SPIR-V opcodes, enums, etc
class SpirvGrammarHelperOutputGenerator(OutputGenerator):
    def __init__(self,
                 errFile = sys.stderr,
                 warnFile = sys.stderr,
                 diagFile = sys.stdout):
        OutputGenerator.__init__(self, errFile, warnFile, diagFile)
        self.headerFile = False # Header file generation flag
        self.sourceFile = False # Source file generation flag

        self.opcodes = dict()
        self.atomicsOps = []
        self.groupOps = []
        self.imageGatherOps = []
        self.imageSampleOps = []
        self.imageFetchOps = []
        self.storageClassList = [] # list of storage classes
        self.executionModel = []
        # Need range to be large as largest possible operand index
        self.imageOperandsParamCount = [[] for i in range(3)]

        # Lots of switch statements share same ending
        self.commonBoolSwitch = '''            found = true;
            break;
        default:
            break;
    }
    return found;
}\n
'''

    def commonParamSwitch(self, variableName):
        return '''        default:
            break;
    }}
    return {};
}}\n
'''.format(variableName)

    #
    # Called at beginning of processing as file is opened
    def beginFile(self, genOpts):
        OutputGenerator.beginFile(self, genOpts)
        self.parseGrammar(genOpts.grammar)

        self.headerFile = (genOpts.filename == 'spirv_grammar_helper.h')
        self.sourceFile = (genOpts.filename == 'spirv_grammar_helper.cpp')
        if not self.headerFile and not self.sourceFile:
            print("Error: Output Filenames have changed, update generator source.\n")
            sys.exit(1)

        # File Comment
        file_comment = '// *** THIS FILE IS GENERATED - DO NOT EDIT ***\n'
        file_comment += '// See spirv_gramar_generator.py for modifications\n'
        write(file_comment, file=self.outFile)
        # Copyright Statement
        copyright = ''
        copyright += '\n'
        copyright += '/***************************************************************************\n'
        copyright += ' *\n'
        copyright += ' * Copyright (c) 2021-2023 The Khronos Group Inc.\n'
        copyright += ' *\n'
        copyright += ' * Licensed under the Apache License, Version 2.0 (the "License");\n'
        copyright += ' * you may not use this file except in compliance with the License.\n'
        copyright += ' * You may obtain a copy of the License at\n'
        copyright += ' *\n'
        copyright += ' *     http://www.apache.org/licenses/LICENSE-2.0\n'
        copyright += ' *\n'
        copyright += ' * Unless required by applicable law or agreed to in writing, software\n'
        copyright += ' * distributed under the License is distributed on an "AS IS" BASIS,\n'
        copyright += ' * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n'
        copyright += ' * See the License for the specific language governing permissions and\n'
        copyright += ' * limitations under the License.\n'
        copyright += ' *\n'
        copyright += ' * This file is related to anything that is found in the SPIR-V grammar\n'
        copyright += ' * file found in the SPIRV-Headers. Mainly used for SPIR-V util functions.\n'
        copyright += ' *\n'
        copyright += ' ****************************************************************************/\n'
        write(copyright, file=self.outFile)

        if self.sourceFile:
            write('#include "vk_layer_data.h"', file=self.outFile)
            write('#include "spirv_grammar_helper.h"', file=self.outFile)
            write('#include "state_tracker/shader_instruction.h"', file=self.outFile)
        elif self.headerFile:
            write('#pragma once', file=self.outFile)
            write('#include <cstdint>', file=self.outFile)
            write('#include <spirv/unified1/spirv.hpp>', file=self.outFile)
        write('', file=self.outFile)
    #
    # Write generated file content to output file
    def endFile(self):
        write(self.instructionTable(), file=self.outFile)
        write(self.atomicOperation(), file=self.outFile)
        write(self.groupOperation(), file=self.outFile)
        write(self.storageClassHelper(), file=self.outFile)
        write(self.imageOperation(), file=self.outFile)
        write(self.parameterHelper(), file=self.outFile)
        write(self.stringHelper(), file=self.outFile)
        # Finish processing in superclass
        OutputGenerator.endFile(self)
    #
    # Takes the SPIR-V Grammar JSON and parses it
    # Emulates the gen*() functions the vk.xml calls
    #
    # In the future, IF more then this generator wants to use the grammar
    # it would be better to move the file opening to lvl_genvk.py
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
                if operandKind['kind'] == 'StorageClass':
                    values = [] # prevent alias from being duplicatd
                    for enum in operandKind['enumerants']:
                        if enum['value'] not in values:
                            self.storageClassList.append(enum['enumerant'])
                            values.append(enum['value'])
                if operandKind['kind'] == 'ExecutionModel':
                    values = [] # prevent alias from being duplicatd
                    for enum in operandKind['enumerants']:
                        if enum['value'] not in values:
                            self.executionModel.append(enum['enumerant'])
                            values.append(enum['value'])

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
                            else:
                                print("Error: unknown operand {} not handled correctly\n".format(opname))
                                sys.exit(1)
                        if operand['kind'] == 'ImageOperands':
                            self.opcodes[opcode]['imageOperandsPosition'] = index + 1
                        if operand['kind'] == 'StorageClass':
                            self.opcodes[opcode]['storageClassPosition'] = index + 1

    #
    # Generate table for each opcode instruction
    def instructionTable(self):
        output = ''
        if self.sourceFile:
            output += '// All information related to each SPIR-V opcode instruction\n'
            output += 'struct InstructionInfo {\n'
            output += '    const char* name;\n'
            output += '    bool has_type; // always operand 0 if present\n'
            output += '    bool has_result; // always operand 1 if present\n'
            output += '    uint32_t memory_scope_position; // operand ID position or zero if not present\n'
            output += '    uint32_t execution_scope_position; // operand ID position or zero if not present\n'
            output += '    uint32_t image_operands_position; // operand ID position or zero if not present\n'
            output += '};\n'
            output += '\n'
            output += '// Static table to replace having many large switch statement functions for looking up each part\n'
            output += '// of a given SPIR-V opcode instruction\n'
            output += '//\n'
            output += '// clang-format off\n'
            output += 'static const vvl::unordered_map<uint32_t, InstructionInfo> kInstructionTable {\n'
            for opcode, info in sorted(self.opcodes.items()):
                output += f'    {{spv::{info["name"]}, {{"{info["name"]}", {info["hasType"]}, {info["hasResult"]}, {info["memoryScopePosition"]}, {info["executionScopePosition"]}, {info["imageOperandsPosition"]}}}}},\n'
            output += '};\n'
            output += '// clang-format on\n'
        return output;
    #
    # Generate functions for numeric based functions
    def atomicOperation(self):
        output = ''
        if self.headerFile:
            output += 'bool AtomicOperation(uint32_t opcode);\n'
        elif self.sourceFile:
            output += '// Any non supported operation will be covered with VUID 01090\n'
            output += 'bool AtomicOperation(uint32_t opcode) {\n'
            output += '    bool found = false;\n'
            output += '    switch (opcode) {\n'
            for f in self.atomicsOps:
                output += '        case spv::{}:\n'.format(f)
            output += self.commonBoolSwitch

        return output;
    #
    # Generate functions for numeric based functions
    def groupOperation(self):
        output = ''
        if self.headerFile:
            output += 'bool GroupOperation(uint32_t opcode);\n'
        elif self.sourceFile:
            output += '// Any non supported operation will be covered with VUID 01090\n'
            output += 'bool GroupOperation(uint32_t opcode) {\n'
            output += '    bool found = false;\n'
            output += '    switch (opcode) {\n'
            for f in self.groupOps:
                output += '        case spv::{}:\n'.format(f)
            output += self.commonBoolSwitch

        return output;
    #
    # Get storage class from instruction
    def storageClassHelper(self):
        output = ''
        if self.sourceFile:
            output += 'spv::StorageClass Instruction::StorageClass() const {\n'
            output += '    spv::StorageClass storage_class = spv::StorageClassMax;\n'
            output += '    switch (Opcode()) {\n'
            for opcode, info in sorted(self.opcodes.items()):
                operand = info['storageClassPosition']
                if operand == 0:
                    continue
                output += '        case spv::{}:\n'.format(info['name'])
                output += '            storage_class = static_cast<spv::StorageClass>(Word({}));\n'.format(info['storageClassPosition'])
                output += '            break;\n'
            output += self.commonParamSwitch('storage_class')
        return output;
    #
    # Generate functions for image operations
    def imageOperation(self):
        output = ''
        if self.headerFile:
            output += 'bool ImageGatherOperation(uint32_t opcode);\n'
            output += 'bool ImageFetchOperation(uint32_t opcode);\n'
            output += 'bool ImageSampleOperation(uint32_t opcode);\n'
        elif self.sourceFile:
            output += 'bool ImageGatherOperation(uint32_t opcode) {\n'
            output += '    bool found = false;\n'
            output += '    switch (opcode) {\n'
            for f in self.imageGatherOps:
                output += '        case spv::{}:\n'.format(f)
            output += self.commonBoolSwitch
            output += '\n'

            output += 'bool ImageFetchOperation(uint32_t opcode) {\n'
            output += '    bool found = false;\n'
            output += '    switch (opcode) {\n'
            for f in self.imageFetchOps:
                output += '        case spv::{}:\n'.format(f)
            output += self.commonBoolSwitch
            output += '\n'

            output += 'bool ImageSampleOperation(uint32_t opcode) {\n'
            output += '    bool found = false;\n'
            output += '    switch (opcode) {\n'
            for f in self.imageSampleOps:
                output += '        case spv::{}:\n'.format(f)
            output += self.commonBoolSwitch

        return output;
    #
    # Generate functions for operand parameter switch cases
    def parameterHelper(self):
        output = ''
        if self.headerFile:
            output += 'bool OpcodeHasType(uint32_t opcode);\n'
            output += 'bool OpcodeHasResult(uint32_t opcode);\n'
            output += '\n'
            output += 'uint32_t OpcodeMemoryScopePosition(uint32_t opcode);\n'
            output += 'uint32_t OpcodeExecutionScopePosition(uint32_t opcode);\n'
            output += 'uint32_t OpcodeImageOperandsPosition(uint32_t opcode);\n'
            output += '\n'
            output += 'uint32_t ImageOperandsParamCount(uint32_t opcode);\n'
        elif self.sourceFile:
            output += 'bool OpcodeHasType(uint32_t opcode) {\n'
            output += '    bool has_type = false;\n'
            output += '    auto format_info = kInstructionTable.find(opcode);\n'
            output += '    if (format_info != kInstructionTable.end()) {\n'
            output += '        has_type = format_info->second.has_type;\n'
            output += '    }\n'
            output += '    return has_type;\n'
            output += '}\n\n'

            output += 'bool OpcodeHasResult(uint32_t opcode) {\n'
            output += '    bool has_result = false;\n'
            output += '    auto format_info = kInstructionTable.find(opcode);\n'
            output += '    if (format_info != kInstructionTable.end()) {\n'
            output += '        has_result = format_info->second.has_result;\n'
            output += '    }\n'
            output += '    return has_result;\n'
            output += '}\n\n'

            output += '// Return operand position of Memory Scope <ID> or zero if there is none\n'
            output += 'uint32_t OpcodeMemoryScopePosition(uint32_t opcode) {\n'
            output += '    uint32_t position = 0;\n'
            output += '    auto format_info = kInstructionTable.find(opcode);\n'
            output += '    if (format_info != kInstructionTable.end()) {\n'
            output += '        position = format_info->second.memory_scope_position;\n'
            output += '    }\n'
            output += '    return position;\n'
            output += '}\n\n'

            output += '// Return operand position of Execution Scope <ID> or zero if there is none\n'
            output += 'uint32_t OpcodeExecutionScopePosition(uint32_t opcode) {\n'
            output += '    uint32_t position = 0;\n'
            output += '    auto format_info = kInstructionTable.find(opcode);\n'
            output += '    if (format_info != kInstructionTable.end()) {\n'
            output += '        position = format_info->second.execution_scope_position;\n'
            output += '    }\n'
            output += '    return position;\n'
            output += '}\n\n'

            output += '// Return operand position of Image Operands <ID> or zero if there is none\n'
            output += 'uint32_t OpcodeImageOperandsPosition(uint32_t opcode) {\n'
            output += '    uint32_t position = 0;\n'
            output += '    auto format_info = kInstructionTable.find(opcode);\n'
            output += '    if (format_info != kInstructionTable.end()) {\n'
            output += '        position = format_info->second.image_operands_position;\n'
            output += '    }\n'
            output += '    return position;\n'
            output += '}\n\n'

            output += '// Return number of optional parameter from ImageOperands\n'
            output += 'uint32_t ImageOperandsParamCount(uint32_t image_operand) {\n'
            output += '    uint32_t count = 0;\n'
            output += '    switch (image_operand) {\n'
            for index, operands in enumerate(self.imageOperandsParamCount):
                for operand in operands:
                    if operand == 'None': # not sure why header is not consistent with this
                        output += '        case spv::ImageOperandsMask{}:\n'.format(operand)
                    else:
                        output += '        case spv::ImageOperands{}Mask:\n'.format(operand)
                if len(operands) != 0:
                    output += '            return {};\n'.format(index)
            output += self.commonParamSwitch('count')

        return output;
    #
    # Generate functions for getting strings to give better error messages
    def stringHelper(self):
        output = ''
        if self.headerFile:
            output =  'const char* string_SpvOpcode(uint32_t opcode);\n'
            output +=  'const char* string_SpvStorageClass(uint32_t storage_class);\n'
            output +=  'const char* string_SpvExecutionModel(uint32_t execution_model);\n'
        elif self.sourceFile:
            output =  'const char* string_SpvOpcode(uint32_t opcode) {\n'
            output += '    auto format_info = kInstructionTable.find(opcode);\n'
            output += '    if (format_info != kInstructionTable.end()) {\n'
            output += '        return format_info->second.name;\n'
            output += '    } else {\n'
            output += '        return \"Unhandled Opcode\";\n'
            output += '    }\n'
            output += '};\n'
            output += '\nconst char* string_SpvStorageClass(uint32_t storage_class) {\n'
            output += '    switch(storage_class) {\n'
            for storageClass in self.storageClassList:
                output += '        case spv::StorageClass{}:\n'.format(storageClass)
                output += '            return \"{}\";\n'.format(storageClass)
            output += '        default:\n'
            output += '            return \"unknown\";\n'
            output += '    }\n'
            output += '};\n'
            output += '\nconst char* string_SpvExecutionModel(uint32_t execution_model) {\n'
            output += '    switch(execution_model) {\n'
            for executionModel in self.executionModel:
                output += '        case spv::ExecutionModel{}:\n'.format(executionModel)
                output += '            return \"{}\";\n'.format(executionModel)
            output += '        default:\n'
            output += '            return \"unknown\";\n'
            output += '    }\n'
            output += '};\n'
        return output
