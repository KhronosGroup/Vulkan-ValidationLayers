#!/usr/bin/python3 -i
#
# Copyright (c) 2021 The Khronos Group Inc.
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
# Author: Spencer Fricke <s.fricke@samsung.com>

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
        # Need range to be large as largest possible operand index
        self.memoryScopeParam = [[] for i in range(5)]
        self.executionScopeParam = [[] for i in range(5)]
        self.imageOperandsParam = [[] for i in range(8)]

        # Lots of switch statements share same ending
        self.commonBoolSwitch = '''            found = true;
            break;
        default:
            break;
    }
    return found;
}\n
'''
        self.commonParamSwitch = '''            break;
        default:
            break;
    }
    return position;
}\n
'''

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
        copyright += ' * Copyright (c) 2021 The Khronos Group Inc.\n'
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
        copyright += ' * Author: Spencer Fricke <s.fricke@samsung.com>\n'
        copyright += ' *\n'
        copyright += ' * This file is related to anything that is found in the SPIR-V grammar\n'
        copyright += ' * file found in the SPIRV-Headers. Mainly used for SPIR-V util functions.\n'
        copyright += ' *\n'
        copyright += ' ****************************************************************************/\n'
        write(copyright, file=self.outFile)

        if self.sourceFile:
            write('#include "spirv_grammar_helper.h"', file=self.outFile)
        elif self.headerFile:
            write('#pragma once', file=self.outFile)
            write('#include <cstdint>', file=self.outFile)
            write('#include <spirv/unified1/spirv.hpp>', file=self.outFile)
        write('', file=self.outFile)
    #
    # Write generated file content to output file
    def endFile(self):
        write(self.atomicOperation(), file=self.outFile)
        write(self.groupOperation(), file=self.outFile)
        write(self.imageOperation(), file=self.outFile)
        write(self.scopeHelper(), file=self.outFile)
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

            for instruction in instructions:
                opname = instruction['opname']
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
                self.opcodes[instruction['opcode']] = opname

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
                        # some instructions have both types of IdScope
                        # OpReadClockKHR has the wrong 'name' as 'Scope'
                        if operand['kind'] == 'IdScope':
                            if operand['name'] == '\'Execution\'' or operand['name'] == '\'Scope\'':
                                self.executionScopeParam[index + 1].append(opname)
                            elif operand['name'] == '\'Memory\'':
                                self.memoryScopeParam[index + 1].append(opname)
                            else:
                                print("Error: unknown operand {} not handled correctly\n".format(opname))
                                sys.exit(1)
                        if operand['kind'] == 'ImageOperands':
                            self.imageOperandsParam[index + 1].append(opname)

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
    # Generate functions for scope id
    def scopeHelper(self):
        output = ''
        if self.headerFile:
            output += 'uint32_t MemoryScopeParam(uint32_t opcode);\n'
            output += 'uint32_t ExecutionScopeParam(uint32_t opcode);\n'
            output += 'uint32_t ImageOperandsParam(uint32_t opcode);\n'
        elif self.sourceFile:
            output += '// Return paramater position of memory scope ID or zero if there is none\n'
            output += 'uint32_t MemoryScopeParam(uint32_t opcode) {\n'
            output += '    uint32_t position = 0;\n'
            output += '    switch (opcode) {\n'
            for index, operands in enumerate(self.memoryScopeParam):
                for operand in operands:
                    output += '        case spv::{}:\n'.format(operand)
                if len(operands) != 0:
                    output += '            return {};\n'.format(index)
            output += self.commonParamSwitch

            output += '// Return paramater position of execution scope ID or zero if there is none\n'
            output += 'uint32_t ExecutionScopeParam(uint32_t opcode) {\n'
            output += '    uint32_t position = 0;\n'
            output += '    switch (opcode) {\n'
            for index, operands in enumerate(self.executionScopeParam):
                for operand in operands:
                    output += '        case spv::{}:\n'.format(operand)
                if len(operands) != 0:
                    output += '            return {};\n'.format(index)
            output += self.commonParamSwitch


            output += '// Return paramater position of Image Operands or zero if there is none\n'
            output += 'uint32_t ImageOperandsParam(uint32_t opcode) {\n'
            output += '    uint32_t position = 0;\n'
            output += '    switch (opcode) {\n'
            for index, operands in enumerate(self.imageOperandsParam):
                for operand in operands:
                    output += '        case spv::{}:\n'.format(operand)
                if len(operands) != 0:
                    output += '            return {};\n'.format(index)
            output += self.commonParamSwitch

        return output;
    #
    # Generate functions for getting strings to give better error messages
    def stringHelper(self):
        output = ''
        if self.headerFile:
            output =  'static inline const char* string_SpvOpcode(uint32_t opcode) {\n'
            output += '    switch ((spv::Op)opcode) {\n'
            for opcode, name in sorted(self.opcodes.items()):
                    output += '         case spv::' + name + ':\n'
                    output += '            return \"' + name + '\";\n'
            output += '        default:\n'
            output += '            return \"Unhandled Opcode\";\n'
            output += '    };\n'
            output += '};'
        return output
