#!/usr/bin/python3 -i
#
# Copyright (c) 2023 The Khronos Group Inc.
# Copyright (c) 2023 LunarG, Inc.
# Copyright (c) 2023 Google Inc.
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

from vuAST import isCodifiedVU
from vu_allowlist import VU_ALLOWLIST
from vu_codegen import generateValidation, APIS_WITH_ADDITIONAL_STATE_ARG, ADDITIONAL_STATE_ARG

# ExplicitValidationGeneratorOptions - subclass of GeneratorOptions.
#
# Adds options used by ExplicitValidationOutputGenerator object during Explicit validation layer generation.
#
# Additional members
#   protectFile - True if multiple inclusion protection should be
#     generated (based on the filename) around the entire header.
#   protectFeature - True if #ifndef..#endif protection should be
#     generated around a feature interface in the header file.
#   genFuncPointers - True if function pointer typedefs should be
#     generated
#   protectProto - If conditional protection should be generated
#     around prototype declarations, set to either '#ifdef'
#     to require opt-in (#ifdef protectProtoStr) or '#ifndef'
#     to require opt-out (#ifndef protectProtoStr). Otherwise
#     set to None.
#   protectProtoStr - #ifdef/#ifndef symbol to use around prototype
#     declarations, if protectProto is set
#   apicall - string to use for the function declaration prefix,
#     such as APICALL on Windows.
#   apientry - string to use for the calling convention macro,
#     in typedefs, such as APIENTRY.
#   apientryp - string to use for the calling convention macro
#     in function pointer typedefs, such as APIENTRYP.
#   indentFuncProto - True if prototype declarations should put each
#     parameter on a separate line
#   indentFuncPointer - True if typedefed function pointers should put each
#     parameter on a separate line
#   alignFuncParam - if nonzero and parameters are being put on a
#     separate line, align parameter names at the specified column
class ExplicitValidationGeneratorOptions(GeneratorOptions):
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
                 apicall = 'VKAPI_ATTR ',
                 apientry = 'VKAPI_CALL ',
                 apientryp = 'VKAPI_PTR *',
                 indentFuncProto = True,
                 indentFuncPointer = False,
                 alignFuncParam = 48,
                 expandEnumerants = False,
                 valid_usage_path = '',
                 quiet = True):
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
        self.apicall           = apicall
        self.apientry          = apientry
        self.apientryp         = apientryp
        self.indentFuncProto   = indentFuncProto
        self.indentFuncPointer = indentFuncPointer
        self.alignFuncParam    = alignFuncParam
        self.expandEnumerants  = expandEnumerants
        self.valid_usage_path  = valid_usage_path
        self.quiet             = quiet

# ExplicitValidationOutputGenerator - subclass of OutputGenerator.
# Generates explicit validation layer code.
#
# ---- methods ----
# ExplicitValidationOutputGenerator(errFile, warnFile, diagFile) - args as for
#   OutputGenerator. Defines additional internal state.
# ---- methods overriding base class ----
# beginFile(genOpts)
# endFile()
# beginFeature(interface, emit)
# endFeature()
# genType(typeinfo,name)
# genStruct(typeinfo,name)
# genCmd(cmdinfo)
class ExplicitValidationOutputGenerator(OutputGenerator):
    """Generate Explicit Validation code based on XML element attributes"""
    # This is an ordered list of sections in the header file.
    ALL_SECTIONS = ['command']

    def __init__(self,
                 errFile = sys.stderr,
                 warnFile = sys.stderr,
                 diagFile = sys.stdout):
        OutputGenerator.__init__(self, errFile, warnFile, diagFile)

        self.INDENT_SPACES = 4

        self.vu_dict = dict()               # Dict of api -> [(vuid, vu)]
        self.structextends_list = []        # List of structs which extend another struct
        self.alias_dict = dict()            # Dict of cmd|struct aliases
        self.header_file = False            # Header file generation flag
        self.source_commands_file = False   # Source file generation flag (*_commands.cpp)
        self.source_structs_file = False    # Source file generation flag (*_structs.cpp)
        self.current_feature = None         # ifdef condition for current structs/commands
        self.feature_protect = dict()       # Whether generated code should be protected by ifdef

        self.generated = []                 # Generated code for the header or source

        self.unique_id = 0                  # An id to generate unique temp variables

        # Data required to generate validation for an API token.  This is deferred to make sure the
        # self.feature_protect is complete before code generation.  This makes the generator not
        # sensitive to the order of declarations in the xml.
        self.GenerateData = namedtuple('GenerateData', ['alias', 'members', 'prefix',
                                                        'isCommand'])
        self.generate_data = dict()

        # In some cases, validation for some structs need to be ignored.  For example, if
        # VkGraphicsPipelineCreateInfo doesn't include the tessellation states, pTessellationState
        # should be ignored, even if it's not nullptr.
        #
        # When these members are encountered, a call to `shouldIgnore_API_MEMBER()` is made, which
        # is manually implemented.
        #
        # TODO: currently a placeholder, needs to be completed.
        self.ignorables = {
            ('VkGraphicsPipelineCreateInfo', 'pTessellationState'),
        }

        # A list of objects to pass down from one API token to validation of its structs.  A map
        # from a function name or prefix to the parameters that should be carried over.
        #
        # TODO: currently a placeholder, needs to be completed.
        self.parent_object_list = [
            ('vkCmd', ['commandBuffer']),
        ]

        # The following are commands ignored by the chassis.  No code is generated for them.
        self.ignorelist = [
            # Explicitly skipped by chassis
            'vkEnumerateInstanceVersion',
            # ValidationCache functions do not get dispatched
            'vkCreateValidationCacheEXT',
            'vkDestroyValidationCacheEXT',
            'vkMergeValidationCachesEXT',
            'vkGetValidationCacheDataEXT',
        ]

    # Generate Copyright comment block for file
    def GenerateCopyright(self, start_year = '2023'):
        from datetime import datetime
        curr_year = datetime.now().year
        year = f'{start_year}-{curr_year}' if start_year is not None else curr_year
        return f'''/* *** THIS FILE IS GENERATED - DO NOT EDIT! ***
 * See parameter_validation_generator.py for modifications
 *
 * Copyright (c) {year} The Khronos Group Inc.
 * Copyright (c) {year} LunarG, Inc.
 * Copyright (C) {year} Google Inc.
 * Copyright (c) {year} Valve Corporation
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
 */\n\n'''

    def makeIndent(self, indent):
        return ' ' * (indent * self.INDENT_SPACES)

    def makeUniqueVar(self, name):
        self.unique_id += 1
        return '_' + name + str(self.unique_id)

    # Turn literal '\n' to \n in VU text
    def convertNewLine(self, vu):
        return vu.split('\\n')

    # Walk the JSON-derived dict and find all "vuid"/"text" pairs of interest
    def extractVUs(self, vus, api=''):
        if isinstance(vus, dict):
            # Get the vuid/text pair if any
            if 'vuid' in vus:
                assert('text' in vus)

                vuid = vus['vuid']
                vuText = self.convertNewLine(vus['text'])

                # Flag errors in the spec if {refpage} is not set correctly.
                assert(vuid.split('-')[1].startswith(api))

                if isCodifiedVU(vuText):
                    # TODO: remove `or True`; left there for debugging at the moment.
                    if vuid not in VU_ALLOWLIST and (not self.genOpts.quiet or True):
                        print('Info: skipping explicit validation codegen for ' + vuid)
                    else:
                        yield api, vuid, vuText

            # Recursively extract VUIDs from nested items
            for name, subvus in vus.items():
                for vu in self.extractVUs(subvus, name if api == '' else api):
                    yield vu
        elif isinstance (vus, list):
            for subvus in vus:
                for vu in self.extractVUs(subvus, api):
                    yield vu

    # Called at file creation time
    def beginFile(self, genOpts):
        OutputGenerator.beginFile(self, genOpts)
        self.header_file = (genOpts.filename == 'explicit_validation_decl.h')
        self.source_commands_file = (genOpts.filename == 'explicit_validation_commands.cpp')
        self.source_structs_file = (genOpts.filename == 'explicit_validation_structs.cpp')

        if not self.header_file and not self.source_commands_file and not self.source_structs_file:
            print("Error: Output Filenames have changed, update generator source.\n")
            sys.exit(1)

        # Output Copyright text
        s = self.GenerateCopyright()
        write(s, file=self.outFile)

        if self.header_file:
            write('#pragma once\n', file=self.outFile)
            return

        # Header includes
        write('#include "chassis.h"', file=self.outFile)
        self.newline()
        write('#include "explicit/explicit_validation.h"', file=self.outFile)
        self.newline()

        # Load validusage.json and extract VUs out of it.
        vu_json_filename = os.path.join(genOpts.valid_usage_path, 'validusage.json')
        vus = {}
        if os.path.isfile(vu_json_filename):
            json_file = open(vu_json_filename, 'r', encoding='utf-8')
            vus = json.load(json_file)
            json_file.close()
        if len(vus) == 0:
            print("Error: Could not find, or error loading %s/validusage.json\n", vu_json_filename)
            sys.exit(1)

        for api, vuid, vu in self.extractVUs(vus['validation']):
            if api not in self.vu_dict:
                self.vu_dict[api] = []
            self.vu_dict[api].append((vuid, vu))

    # Called at end-time for final content output
    def endFile(self):
        # Generate code for all API tokens
        for api, data in self.generate_data.items():
            self.genAPIToken(api, data.alias, data.members, data.prefix, data.isCommand)

        # Generate code for pNext validation
        if self.source_structs_file:
            self.generated += self.makeValidatePNextFunction()

        # Generate declaration of ValidatePNext
        if self.header_file:
            self.generated += self.makeValidatePNextProto()
            self.generated += [';\n']

        # Output generated code
        write(''.join(self.generated), file=self.outFile)

        # Finish processing in superclass
        OutputGenerator.endFile(self)

    # Processing at beginning of each feature or extension
    def beginFeature(self, interface, emit):
        # Start processing in superclass
        OutputGenerator.beginFeature(self, interface, emit)

        self.current_feature = GetFeatureProtect(interface)

    # Called at the end of each extension (feature)
    def endFeature(self):
        self.current_feature = None

        # Finish processing in superclass
        OutputGenerator.endFeature(self)

    def makeParamDecl(self, member, isCommand):
        decl = self.makeCParamDecl(member, 0).lstrip()
        # Remove `:bitcount` from decl, if any
        decl = decl.split(':')[0]

        # For structs, make all parameters const.
        if not isCommand and 'const' not in decl:
            decl = 'const ' + decl

        return decl

    def makePrototype(self, api, members, prefix, isCommand):
        """Make a prototype for validating the api token when passed its members.

        For commands, this `override`s the functions in ValidationObject.  For structs, these would
        be helpers called indirectly from commands.

        A few functions in VVL have an extra state parameter.
        """

        assert(api[:2] in ['vk', 'Vk'])
        result = ['bool ']
        if not self.header_file:
            result += ['ExplicitValidation::']
        result += [prefix, api[2:]]
        separator = '('
        comma_separator = ',\n' + self.makeIndent(1 if self.header_file else 4)

        # For structs, carry over objects of interest from the caller.  For example when validating
        # VkImageSubresourceRange through vkCmdClearColorImage, we want to return the commandBuffer
        # in the object list as well.
        if not isCommand:
            result += [separator, 'const LogObjectList &_parentObjects']
            separator = comma_separator

        for member in members:
            decl = self.makeParamDecl(member, isCommand)
            result += [separator, decl]
            separator = comma_separator

        if api in APIS_WITH_ADDITIONAL_STATE_ARG:
            result += [separator, 'void *', ADDITIONAL_STATE_ARG]

        result += [') const']
        if isCommand and self.header_file:
            result += [' override']
        return result

    def getName(self, elem):
        return elem.find('name').text

    def makeCallStructValidation(self, structName, expr, parentObjects, indent):
        """Given an expression that evaluates to a struct (e.g. *pDepthStencilAttachment), and the
        struct type (e.g. VkAttachmentReference), this function creates a call to
        PreCallValidateTYPE((expr).X, (expr).Y, ...)"""

        # Put the expression in a temporary variable for simplicity
        var = self.makeUniqueVar('s')
        indentStr = self.makeIndent(indent)
        result = [indentStr, 'const auto ', var, ' = ', expr, ';\n']

        # Make the call, or'ing the result with `skip`.
        assert(structName[:2] == 'Vk')
        result += [indentStr, 'skip |= Validate', structName[2:], '(', parentObjects]
        separator = ',\n' + self.makeIndent(indent + 1)

        info = self.registry.typedict[structName]
        assert(info is not None)
        assert(info.elem.get('category') in ['struct', 'union'])

        members = info.elem.findall('./member')
        for member in members:
            result += [separator, var, '->', self.getName(member)]
            separator = ',\n' + self.makeIndent(indent + 1)

        result += [');\n']
        return result

    # Check if the parameter passed in is a pointer
    def paramIsPointer(self, param):
        ispointer = 0
        paramtype = param.find('type')
        if (paramtype.tail is not None) and ('*' in paramtype.tail):
            ispointer += paramtype.tail.count('*')

        # For static arrays, consider them pointers too
        name = param.find('name')
        if name.tail is not None and '[' in name.tail:
            ispointer += name.tail.count('[')

        return ispointer

    # Retrieve the value of the len tag
    def getLen(self, param):
        result = None
        # Default to altlen when available to avoid LaTeX markup
        if 'altlen' in param.attrib:
            len = param.attrib.get('altlen')
        else:
            len = param.attrib.get('len')

        if len is None:
            # This could be a static array.  Extract the length from the constant.
            name = param.find('name')
            if name.tail is not None and '[' in name.tail:
                match = re.search(r'\[(\d+)\]', name.tail)
                if match:
                    len = match.group(1)
            return len

        # Currently, only first level is supported for multidimensional arrays.
        len = len.split(',')[0]

        return len

    def makeIgnoreCheckCall(self, api, name, members):
        # If it needs special ignore check, call that.
        if (api, name) in self.ignorables:
            args = ', '.join([self.getName(member) for member in members])
            return ''.join(['!shouldIgnore_', api, '_', name, '(', args, ')'])

        # Otherwise just do a null check
        return name + ' != nullptr'

    def makeGuardBegin(self, api):
        result = []

        assert(api in self.feature_protect)
        feature = self.feature_protect[api]
        if feature:
            result += ['#ifdef ', feature, '\n']

        return result

    def makeGuardEnd(self, api):
        result = []

        if self.feature_protect[api]:
            result += ['#endif\n']

        return result

    def getPointerMap(self, members):
        """For each member, returns a map of name->pointer level.  This is used in combination with
        len to know if the len is a value or a pointer."""
        pointerMap = {}
        for member in members:
            name = self.getName(member)
            isPointer = self.paramIsPointer(member)
            pointerMap[name] = isPointer
        return pointerMap

    def makeCallMemberValidation(self, api, members, parentObjects, indent):
        """Go over every struct member/param and call validate on it (with expanded members)."""

        result = []

        pointerMap = self.getPointerMap(members)

        for member in members:
            typename = member.find('type').text
            isStruct = (typename in self.registry.typedict and
                        self.registry.typedict[typename].elem.get('category') in ['struct', 'union'])

            if not isStruct:
                continue

            if self.alias_dict[typename]:
                typename = self.alias_dict[typename]

            name = self.getName(member)
            isPointer = pointerMap[name]
            arrayLen = self.getLen(member)

            # If the struct is return-only, don't validate it
            if typename not in self.generate_data:
                assert(self.registry.typedict[typename].elem.get('returnedonly') == 'true')
                continue

            # If arrayLen is a pointer, dereference it for looping
            if arrayLen in pointerMap:
                arrayLen = '*' * pointerMap[arrayLen] + arrayLen

            # If pointer, add a null or ignore check
            has_null_check = isPointer > 0

            if has_null_check:
                result += [self.makeIndent(indent), 'if (',
                           self.makeIgnoreCheckCall(api, name, members), ') {\n']
                indent += 1

            # If array, validate every element
            index = ''
            if arrayLen:
                arrayVar = self.makeUniqueVar('i')
                result += [self.makeIndent(indent), 'for (uint32_t ', arrayVar, ' = 0;',
                           arrayVar, ' < ', arrayLen, '; ++', arrayVar, ') {\n']
                index = '[' + arrayVar + ']'
                indent += 1

                # Remove one pointer level in this case.
                assert(isPointer > 0)
                isPointer -= 1

            # The expression that refers to the struct is `{prefix}name{suffix}`, where `{prefix}`
            # can be `&` if the member is not a pointer, and `{suffix}` can be `[index]` if it's an
            # array.
            expr = ''.join(['&' if isPointer == 0 else '', name, index])
            result += self.makeCallStructValidation(typename, expr, parentObjects, indent)

            if arrayLen:
                indent -= 1
                result += [self.makeIndent(indent), '}\n']

            if has_null_check:
                indent -= 1
                result += [self.makeIndent(indent), '}\n']

        # Validate the pNext chain, if any.
        if len(members) >= 2 and self.getName(members[1]) == 'pNext':
            result += self.makeValidatePNextCall(indent)

        return result

    def makeValidatePNextCall(self, indent):
        result = [self.makeIndent(indent), 'skip |= ValidatePNext(_parentObjects, pNext);\n']

        return result

    def makeValidatePNextProto(self):
        return ['bool ', '' if self.header_file else 'ExplicitValidation::',
                'ValidatePNext(const LogObjectList &_parentObjects, const void *pnext) const']

    def makeValidatePNextFunction(self):
        result = self.makeValidatePNextProto()
        result += [""" {
    bool skip = false;
    const VkBaseInStructure *header = reinterpret_cast<const VkBaseInStructure *>(pnext);
    while (header) {
        switch (header->sType) {
"""]

        indent = 2
        # Generate a case for each struct
        for struct in self.structextends_list:

            members = self.registry.typedict[struct].elem.findall('.//member')
            assert(self.getName(members[0]) == 'sType')

            # Get the VkStructureType value that identifies this struct
            stype = members[0].get('values')

            result += self.makeGuardBegin(struct)

            result += [self.makeIndent(indent), 'case ', stype, ': {\n']
            result += self.makeCallStructValidation(struct,
                                                    'reinterpret_cast<const ' + struct + ' *>(header)',
                                                    '_parentObjects',
                                                    indent + 1)
            result += [self.makeIndent(indent + 1), 'break;\n']
            result += [self.makeIndent(indent), '}\n']

            result += self.makeGuardEnd(struct)

        result += ["""        default:
            break;
        }
        header = header->pNext;
    }
    return skip;
}
"""]

        return result

    # Type generation
    def genType(self, typeinfo, name, alias):
        OutputGenerator.genType(self, typeinfo, name, alias)

        category = typeinfo.elem.get('category')
        if category in ['struct', 'union']:
            self.genStruct(typeinfo, name, alias)

    # Generate validation for struct types
    def genStruct(self, typeinfo, typeName, alias):
        OutputGenerator.genStruct(self, typeinfo, typeName, alias)

        self.alias_dict[typeName] = alias

        if alias:
            typeinfo = self.registry.typedict[alias]

        # Don't attempt to validate structs that are return-only
        if typeinfo.elem.attrib.get('returnedonly') is not None:
            return

        if alias is None and typeinfo.elem.attrib.get('structextends') is not None:
            # If this struct extends another, keep its name in list for further processing
            self.structextends_list.append(typeName)

        self.feature_protect[typeName] = self.current_feature

        members = typeinfo.elem.findall('./member')
        self.generate_data[typeName] = self.GenerateData(alias, members, 'Validate', False)

    # Generate validation for entry points
    def genCmd(self, cmdinfo, name, alias):
        OutputGenerator.genCmd(self, cmdinfo, name, alias)

        self.alias_dict[name] = alias

        if name in self.ignorelist:
            return

        if alias:
            cmdinfo = self.registry.cmddict[alias]

        self.feature_protect[name] = self.current_feature

        params = cmdinfo.elem.findall('./param')
        self.generate_data[name] = self.GenerateData(alias, params, 'PreCallValidate', True)

    def makeAliasCall(self, alias, members, prefix, isCommand, indent):
        assert(alias[:2] in ['vk', 'Vk'])
        result = [self.makeIndent(indent), 'return ', prefix, alias[2:]]
        separator = '('

        if not isCommand:
            result += [separator, '_parentObjects']
            separator = ', '

        for member in members:
            result += [separator, self.getName(member)]
            separator = ', '

        result += [');\n']
        return result

    def makeCarryOverObjects(self, api, isCommand, indent):
        # If this is a command, there are no objects already.  The carry over objects list starts
        # empty.  If it's a struct, `_parentObjects` may already contain objects.
        #
        # In either case, some objects may be added based on self.parent_object_list

        newObjects = None
        for prefix, toAdd in self.parent_object_list:
            if api.startswith(prefix):
                newObjects = toAdd
                break

        # Return list of objects already being carried over if there's nothing to add
        if newObjects is None:
            return [], '{}' if isCommand else '_parentObjects'

        # Otherwise, declare a new object list
        result = [self.makeIndent(indent), 'LogObjectList _carryOverObjects']
        if not isCommand:
            result += [' = _parentObjects']
        result += [';\n']

        for obj in newObjects:
            result += [self.makeIndent(indent), '_carryOverObjects.add(', obj, ');\n']

        return result, '_carryOverObjects'

    # Generate validation for entry points and struct types
    def genAPIToken(self, api, alias, members, prefix, isCommand):
        prototype = self.makePrototype(api, members, prefix, isCommand)

        # For the header, just add the prototype
        if self.header_file:
            self.generated += self.makeGuardBegin(api)
            self.generated += prototype
            self.generated += [';\n']
            self.generated += self.makeGuardEnd(api)
            return

        target_file = self.source_commands_file if isCommand else self.source_structs_file

        # For the source file, generate the validation, and make the call to validation of each
        # struct member + validation of pNext chain if applicable.
        if target_file:
            self.generated += self.makeGuardBegin(api)
            self.generated += prototype
            self.generated += [' {\n']

            if alias:
                # If this is an alias, call the validation for the alias instead.
                self.generated += self.makeAliasCall(alias, members, prefix, isCommand, 1)
            else:
                self.generated += [self.makeIndent(1), 'bool skip = false;\n']
                if api in self.vu_dict:
                    self.generated += generateValidation(self.registry, api,
                                                         self.vu_dict[api])

                carryOverInit, carryOverExpr = self.makeCarryOverObjects(api, isCommand, 1)
                self.generated += carryOverInit

                # Call validation for members that are of struct type too.
                self.generated += self.makeCallMemberValidation(api, members, carryOverExpr, 1)
                self.generated += [self.makeIndent(1), 'return skip;\n']

            self.generated += ['}\n']
            self.generated += self.makeGuardEnd(api)
