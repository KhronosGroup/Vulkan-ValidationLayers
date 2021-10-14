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

        self.atomicsOps = []
        self.groupOps = []

        # Lots of switch statements share same ending
        self.commonBoolSwitch  = '''            found = true;
            break;
        default:
            break;
    }
    return found;
}
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
            write('#include <spirv/unified1/spirv.hpp>', file=self.outFile)
        elif self.headerFile:
            write('#pragma once', file=self.outFile)
            write('#include <cstdint>', file=self.outFile)
        write('', file=self.outFile)
    #
    # Write generated file content to output file
    def endFile(self):
        write(self.atomicOperation(), file=self.outFile)
        write(self.groupOperation(), file=self.outFile)
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
            for instruction in instructions:
                if 'capabilities' in instruction and len(instruction['capabilities']) == 1 and instruction['capabilities'][0] == 'Kernel':
                    continue # If just 'Kernel' then op is ment for OpenCL
                if instruction['class'] == 'Atomic':
                    self.atomicsOps.append(instruction['opname'])
                if instruction['class'] == 'Non-Uniform':
                    self.groupOps.append(instruction['opname'])
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
