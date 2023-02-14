#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2023 The Khronos Group Inc.
# Copyright (c) 2015-2023 Valve Corporation
# Copyright (c) 2015-2023 LunarG, Inc.
# Copyright (c) 2015-2023 Google Inc.
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

import os,re,sys
import xml.etree.ElementTree as etree
from generator import *
from collections import namedtuple
from common_codegen import *
import sync_val_gen

#
# HelperFileOutputGeneratorOptions - subclass of GeneratorOptions.
class HelperFileOutputGeneratorOptions(GeneratorOptions):
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
                 alignFuncParam = 48,
                 library_name = '',
                 expandEnumerants = False,
                 helper_file_type = '',
                 valid_usage_path = ''):
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
        self.genFuncPointers  = genFuncPointers
        self.protectFile      = protectFile
        self.protectFeature   = protectFeature
        self.apicall          = apicall
        self.apientry         = apientry
        self.apientryp        = apientryp
        self.alignFuncParam   = alignFuncParam
        self.library_name     = library_name
        self.helper_file_type = helper_file_type
        self.valid_usage_path = valid_usage_path
#
# HelperFileOutputGenerator - subclass of OutputGenerator. Outputs Vulkan helper files
class HelperFileOutputGenerator(OutputGenerator):
    """Generate helper file based on XML element attributes"""
    def __init__(self,
                 errFile = sys.stderr,
                 warnFile = sys.stderr,
                 diagFile = sys.stdout):
        OutputGenerator.__init__(self, errFile, warnFile, diagFile)
        # Internal state - accumulators for different inner block text
        self.enum_output = ''                             # string built up of enum string routines
        # Internal state - accumulators for different inner block text
        self.structNames = []                             # List of Vulkan struct typenames
        self.structTypes = dict()                         # Map of Vulkan struct typename to required VkStructureType
        self.structMembers = []                           # List of StructMemberData records for all Vulkan structs
        self.object_types = []                            # List of all handle types
        self.object_guards = {}                           # Ifdef guards for object types
        self.object_type_aliases = []                     # Aliases to handles types (for handles that were extensions)
        self.debug_report_object_types = []               # Handy copy of debug_report_object_type enum data
        self.core_object_types = []                       # Handy copy of core_object_type enum data
        self.sync_enum = dict()                           # Handy copy of synchronization enum data
        self.device_extension_info = dict()               # Dict of device extension name defines and ifdef values
        self.instance_extension_info = dict()             # Dict of instance extension name defines and ifdef values
        self.structextends_list = []                      # List of structs which extend another struct via pNext
        self.structOrUnion = dict()                       # Map of Vulkan typename to 'struct' or 'union'

        # Named tuples to store struct and command data
        self.StructType = namedtuple('StructType', ['name', 'value'])
        self.CommandParam = namedtuple('CommandParam', ['type', 'name', 'ispointer', 'isstaticarray', 'isconst', 'iscount', 'len', 'extstructs', 'cdecl'])
        self.StructMemberData = namedtuple('StructMemberData', ['name', 'members', 'ifdef_protect', 'allowduplicate'])

        self.custom_construct_params = {
            # safe_VkGraphicsPipelineCreateInfo needs to know if subpass has color and\or depth\stencil attachments to use its pointers
            'VkGraphicsPipelineCreateInfo' :
                ', const bool uses_color_attachment, const bool uses_depthstencil_attachment',
            # safe_VkPipelineViewportStateCreateInfo needs to know if viewport and scissor is dynamic to use its pointers
            'VkPipelineViewportStateCreateInfo' :
                ', const bool is_dynamic_viewports, const bool is_dynamic_scissors',
            # safe_VkAccelerationStructureBuildGeometryInfoKHR needs to know if we're doing a host or device build
            'VkAccelerationStructureBuildGeometryInfoKHR' :
                ', const bool is_host, const VkAccelerationStructureBuildRangeInfoKHR *build_range_infos',
            # safe_VkAccelerationStructureGeometryKHR needs to know if we're doing a host or device build
            'VkAccelerationStructureGeometryKHR' :
                ', const bool is_host, const VkAccelerationStructureBuildRangeInfoKHR *build_range_info',
            # safe_VkDescriptorDataEXT needs to know what field of union is intialized
            'VkDescriptorDataEXT' :
                ', const VkDescriptorType type',
        }

        # Some bits are helper that include multiple bits, but it is more useful to use the flag name instead
        self.custom_bit_flag_print = {
            'VkShaderStageFlags' : ['VK_SHADER_STAGE_ALL', 'VK_SHADER_STAGE_ALL_GRAPHICS']
        }

    #
    # Called once at the beginning of each run
    def beginFile(self, genOpts):
        OutputGenerator.beginFile(self, genOpts)
        # Initialize members that require the tree
        self.handle_types = GetHandleTypes(self.registry.tree)
        # User-supplied prefix text, if any (list of strings)
        self.helper_file_type = genOpts.helper_file_type
        self.library_name = genOpts.library_name
        # File Comment
        file_comment = '// *** THIS FILE IS GENERATED - DO NOT EDIT ***\n'
        file_comment += '// See helper_file_generator.py for modifications\n'
        write(file_comment, file=self.outFile)
        # Copyright Notice
        copyright = ''
        copyright += '\n'
        copyright += '/***************************************************************************\n'
        copyright += ' *\n'
        copyright += ' * Copyright (c) 2015-2023 The Khronos Group Inc.\n'
        copyright += ' * Copyright (c) 2015-2023 Valve Corporation\n'
        copyright += ' * Copyright (c) 2015-2023 LunarG, Inc.\n'
        copyright += ' * Copyright (c) 2015-2023 Google Inc.\n'
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
        copyright += ' ****************************************************************************/\n'
        write(copyright, file=self.outFile)
    #
    # Write generated file content to output file
    def endFile(self):
        dest_file = ''
        dest_file += self.OutputDestFile()
        # Remove blank lines at EOF
        if dest_file.endswith('\n'):
            dest_file = dest_file[:-1]
        write(dest_file, file=self.outFile)
        # Finish processing in superclass
        OutputGenerator.endFile(self)
    #
    # Override parent class to be notified of the beginning of an extension
    def beginFeature(self, interface, emit):
        # Start processing in superclass
        OutputGenerator.beginFeature(self, interface, emit)
        self.featureExtraProtect = GetFeatureProtect(interface)

        if interface.tag != 'extension':
            return
        name = self.featureName
        index = 0
        while interface[0][index].tag == 'comment':
            index += 1
        nameElem = interface[0][index + 1]
        name_define = nameElem.get('name')
        if 'EXTENSION_NAME' not in name_define:
            print("Error in vk.xml file -- extension name is not available")
        requires = interface.get('requires')
        if requires is not None:
            required_extensions = requires.split(',')
        else:
            required_extensions = list()
        requiresCore = interface.get('requiresCore')
        if requiresCore is not None:
            required_extensions.append('VK_VERSION_%s' % ('_'.join(requiresCore.split('.'))))
        info = { 'define': GetNameDefine(interface), 'ifdef':self.featureExtraProtect, 'reqs':required_extensions }
        if interface.get('type') == 'instance':
            self.instance_extension_info[name] = info
        else:
            self.device_extension_info[name] = info

    #
    # Override parent class to be notified of the end of an extension
    def endFeature(self):
        # Finish processing in superclass
        OutputGenerator.endFeature(self)
    #
    # Grab group (e.g. C "enum" type) info to output for enum-string conversion helper
    def genGroup(self, groupinfo, groupName, alias):
        OutputGenerator.genGroup(self, groupinfo, groupName, alias)
        groupElem = groupinfo.elem
        bitwidth = int(groupElem.get('bitwidth','32'))
        # For enum_string_header
        if self.helper_file_type == 'enum_string_header':
            value_set = set()
            protect_dict = dict()
            for elem in groupElem.findall('enum'):
                if elem.get('supported') != 'disabled' and elem.get('alias') is None:
                    value_set.add(elem.get('name'))
                    if elem.get('protect') is not None:
                        protect_dict[elem.get('name')] = elem.get('protect')
            if value_set != set():
                self.enum_output += self.GenerateEnumStringConversion(groupName, value_set, bitwidth, protect_dict)
        elif self.helper_file_type == 'object_types_header':
            if groupName == 'VkDebugReportObjectTypeEXT':
                for elem in groupElem.findall('enum'):
                    if elem.get('supported') != 'disabled':
                        if elem.get('alias') is None: # TODO: Strangely the "alias" fn parameter does not work
                            item_name = elem.get('name')
                            if self.debug_report_object_types.count(item_name) == 0: # TODO: Strangely there are duplicates
                                self.debug_report_object_types.append(item_name)
            elif groupName == 'VkObjectType':
                for elem in groupElem.findall('enum'):
                    if elem.get('supported') != 'disabled':
                        if elem.get('alias') is None: # TODO: Strangely the "alias" fn parameter does not work
                            item_name = elem.get('name')
                            self.core_object_types.append(item_name)
        elif self.helper_file_type == 'sync_helper_header' or self.helper_file_type == 'sync_helper_source':
            if groupName in sync_val_gen.sync_enum_types:
                self.sync_enum[groupName] = []
                for elem in groupElem.findall('enum'):
                    if elem.get('supported') != 'disabled':
                        self.sync_enum[groupName].append(elem)

    #
    # Called for each type -- if the type is a struct/union, grab the metadata
    def genType(self, typeinfo, name, alias):
        OutputGenerator.genType(self, typeinfo, name, alias)
        typeElem = typeinfo.elem
        # If the type is a struct type, traverse the imbedded <member> tags generating a structure.
        # Otherwise, emit the tag text.
        category = typeElem.get('category')
        if category == 'handle':
            if alias:
                self.object_type_aliases.append((name,alias))
            else:
                self.object_types.append(name)
                self.object_guards[name] = self.featureExtraProtect

        elif (category == 'struct' or category == 'union'):
            self.structNames.append(name)
            self.genStruct(typeinfo, name, alias)
            if (category == 'union'):
                self.structOrUnion[name] = 'union'
            else:
                self.structOrUnion[name] = 'struct'

    #
    # Check if the parameter passed in is a pointer
    def paramIsPointer(self, param):
        ispointer = False
        for elem in param:
            if elem.tag == 'type' and elem.tail is not None and '*' in elem.tail:
                ispointer = True
        return ispointer
    #
    # Check if the parameter passed in is a static array
    def paramIsStaticArray(self, param):
        isstaticarray = 0
        paramname = param.find('name')
        if (paramname.tail is not None) and ('[' in paramname.tail):
            isstaticarray = paramname.tail.count('[')
        return isstaticarray
    #
    # Retrieve the type and name for a parameter
    def getTypeNameTuple(self, param):
        type = ''
        name = ''
        for elem in param:
            if elem.tag == 'type':
                type = noneStr(elem.text)
            elif elem.tag == 'name':
                name = noneStr(elem.text)
        return (type, name)
    #
    # Retrieve the value of the len tag
    def getLen(self, param):
        result = None
        len = param.attrib.get('len')
        if len and len != 'null-terminated':
            # For string arrays, 'len' can look like 'count,null-terminated', indicating that we
            # have a null terminated array of strings.  We strip the null-terminated from the
            # 'len' field and only return the parameter specifying the string count
            if 'null-terminated' in len:
                result = len.split(',')[0]
            else:
                result = len
            if 'altlen' in param.attrib:
                # Elements with latexmath 'len' also contain a C equivalent 'altlen' attribute
                # Use indexing operator instead of get() so we fail if the attribute is missing
                result = param.attrib['altlen']
            # Spec has now notation for len attributes, using :: instead of platform specific pointer symbol
            result = str(result).replace('::', '->')
        return result
    #
    # Check if a structure is or contains a dispatchable (dispatchable = True) or
    # non-dispatchable (dispatchable = False) handle
    def TypeContainsObjectHandle(self, handle_type, dispatchable):
        if dispatchable:
            type_check = self.handle_types.IsDispatchable
        else:
            type_check = self.handle_types.IsNonDispatchable
        if type_check(handle_type):
            return True
        # if handle_type is a struct, search its members
        if handle_type in self.structNames:
            member_index = next((i for i, v in enumerate(self.structMembers) if v[0] == handle_type), None)
            if member_index is not None:
                for item in self.structMembers[member_index].members:
                    if type_check(item.type):
                        return True
        return False
    #
    # Generate local ready-access data describing Vulkan structures and unions from the XML metadata
    def genStruct(self, typeinfo, typeName, alias):
        OutputGenerator.genStruct(self, typeinfo, typeName, alias)
        members = typeinfo.elem.findall('.//member')
        # Iterate over members once to get length parameters for arrays
        lens = set()
        for member in members:
            len = self.getLen(member)
            if len:
                lens.add(len)
        # Generate member info
        membersInfo = []
        for member in members:
            # Get the member's type and name
            info = self.getTypeNameTuple(member)
            type = info[0]
            name = info[1]
            cdecl = self.makeCParamDecl(member, 1)
            # Process VkStructureType
            if type == 'VkStructureType':
                # Extract the required struct type value from the comments
                # embedded in the original text defining the 'typeinfo' element
                rawXml = etree.tostring(typeinfo.elem).decode('ascii')
                result = re.search(r'VK_STRUCTURE_TYPE_\w+', rawXml)
                if result:
                    value = result.group(0)
                    # Store the required type value
                    self.structTypes[typeName] = self.StructType(name=name, value=value)
            # Store pointer/array/string info
            isstaticarray = self.paramIsStaticArray(member)
            membersInfo.append(self.CommandParam(type=type,
                                                 name=name,
                                                 ispointer=self.paramIsPointer(member),
                                                 isstaticarray=isstaticarray,
                                                 isconst=True if 'const' in cdecl else False,
                                                 iscount=True if name in lens else False,
                                                 len=self.getLen(member),
                                                 extstructs=self.registry.validextensionstructs[typeName] if name == 'pNext' else None,
                                                 cdecl=cdecl))
        # If true, this structure type can appear multiple times within a pNext chain
        allowduplicate = self.getBoolAttribute(typeinfo.elem, 'allowduplicate')
        # If this struct extends another, keep its name in list for further processing
        if typeinfo.elem.attrib.get('structextends') is not None:
            self.structextends_list.append(typeName)
        self.structMembers.append(self.StructMemberData(name=typeName, members=membersInfo, ifdef_protect=self.featureExtraProtect, allowduplicate=allowduplicate))
    #
    # Enum_string_header: Create a routine to convert an enumerated value into a string
    def GenerateEnumStringConversion(self, groupName, value_list, bitwidth, protect_dict):
        outstring = '\n'
        if self.featureExtraProtect is not None:
            outstring += '\n#ifdef %s\n\n' % self.featureExtraProtect
        groupType = 'uint64_t' if bitwidth == 64 else groupName

        outstring += 'static inline const char* string_%s(%s input_value)\n' % (groupName, groupType)
        outstring += '{\n'
        outstring += '    switch (input_value)\n'
        outstring += '    {\n'
        # Emit these in a repeatable order so file is generated with the same contents each time.
        # This helps compiler caching systems like ccache.
        for item in sorted(value_list):
            if item in protect_dict:
                outstring += '#ifdef %s\n' % protect_dict[item]
            outstring += '        case %s:\n' % item
            outstring += '            return "%s";\n' % item
            if item in protect_dict:
                outstring += '#endif // %s\n' % protect_dict[item]
        outstring += '        default:\n'
        outstring += '            return "Unhandled %s";\n' % groupName
        outstring += '    }\n'
        outstring += '}\n'

        bitsIndex = groupName.find('Bits')
        if (bitsIndex != -1):
            outstring += '\n'
            flagsName = groupName[0:bitsIndex] + "s" +  groupName[bitsIndex+4:]
            intsuffix = 'ULL' if bitwidth == 64 else 'U'
            outstring += 'static inline std::string string_%s(%s input_value)\n' % (flagsName, flagsName)
            outstring += '{\n'
            if flagsName in self.custom_bit_flag_print:
                for custom in self.custom_bit_flag_print[flagsName]:
                    outstring += '    if (input_value == %s) { return "%s"; }\n' % (custom, custom)
            outstring += '    std::string ret;\n'
            outstring += '    int index = 0;\n'
            outstring += '    while(input_value) {\n'
            outstring += '        if (input_value & 1) {\n'
            outstring += '            if( !ret.empty()) ret.append("|");\n'
            outstring += '            ret.append(string_%s(static_cast<%s>(1%s << index)));\n' % (groupName, groupType, intsuffix)
            outstring += '        }\n'
            outstring += '        ++index;\n'
            outstring += '        input_value >>= 1;\n'
            outstring += '    }\n'
            outstring += '    if (ret.empty()) ret.append("%s(0)");\n' % flagsName
            outstring += '    return ret;\n'
            outstring += '}\n'

        if self.featureExtraProtect is not None:
            outstring += '#endif // %s\n' % self.featureExtraProtect
        return outstring
    #
    # Enum_string_header: Create a routine to determine whether or not a structure type can appear multiple times in a pNext chain
    def GenerateDuplicatePnextInfo(self, value_list):
        outstring = '\nstatic inline bool IsDuplicatePnext(VkStructureType input_value)\n'
        outstring += '{\n'
        outstring += '    switch (input_value)\n'
        outstring += '    {\n'
        # Emit these in a repeatable order so file is generated with the same contents each time.
        # This helps compiler caching systems like ccache.
        for item in sorted(value_list):
            outstring += '        case %s:\n' % item
        outstring += '            return true;\n'
        outstring += '        default:\n'
        outstring += '            return false;\n'
        outstring += '    }\n'
        outstring += '}\n'
        return outstring
    def DuplicatePnextInfo(self):
        return self.GenerateDuplicatePnextInfo([self.structTypes[struct.name].value for struct in self.structMembers if struct.allowduplicate])

    #
    # Tack on a helper which, given an index into a VkPhysicalDeviceFeatures structure, will print the corresponding feature name
    def DeIndexPhysDevFeatures(self):
        pdev_members = None
        for name, members, ifdef, allowduplicate in self.structMembers:
            if name == 'VkPhysicalDeviceFeatures':
                pdev_members = members
                break
        deindex = '\n'
        deindex += 'static inline const char * GetPhysDevFeatureString(uint32_t index) {\n'
        deindex += '    const char * IndexToPhysDevFeatureString[] = {\n'
        for feature in pdev_members:
            deindex += '        "%s",\n' % feature.name
        deindex += '    };\n\n'
        deindex += '    return IndexToPhysDevFeatureString[index];\n'
        deindex += '}\n'
        return deindex
    #
    # Combine enum string helper header file preamble with body text and return
    def GenerateEnumStringHelperHeader(self):
            enum_string_helper_header = '\n'
            enum_string_helper_header += '#pragma once\n'
            enum_string_helper_header += '#ifdef _MSC_VER\n'
            enum_string_helper_header += '#pragma warning( disable : 4065 )\n'
            enum_string_helper_header += '#endif\n'
            enum_string_helper_header += '\n'
            enum_string_helper_header += '#include <string>\n'
            enum_string_helper_header += '#include <vulkan/vulkan.h>\n'
            enum_string_helper_header += '\n'
            enum_string_helper_header += self.enum_output
            enum_string_helper_header += self.DeIndexPhysDevFeatures()
            enum_string_helper_header += self.DuplicatePnextInfo()
            return enum_string_helper_header
    #
    # Helper function for declaring a counter variable only once
    def DeclareCounter(self, string_var, declare_flag):
        if declare_flag == False:
            string_var += '        uint32_t i = 0;\n'
            declare_flag = True
        return string_var, declare_flag
    #
    # Combine safe struct helper header file preamble with body text and return
    def GenerateSafeStructHelperHeader(self):
        safe_struct_helper_header = '\n'
        safe_struct_helper_header += '#pragma once\n'
        safe_struct_helper_header += '#include <vulkan/vulkan.h>\n'
        safe_struct_helper_header += '#include <stdlib.h>\n'
        safe_struct_helper_header += '#include <algorithm>\n'
        safe_struct_helper_header += '\n'
        safe_struct_helper_header += 'void *SafePnextCopy(const void *pNext);\n'
        safe_struct_helper_header += 'void FreePnextChain(const void *pNext);\n'
        safe_struct_helper_header += 'char *SafeStringCopy(const char *in_string);\n'
        safe_struct_helper_header += '\n'
        safe_struct_helper_header += self.GenerateSafeStructHeader()
        return safe_struct_helper_header
    #
    # safe_struct header: build function prototypes for header file
    def GenerateSafeStructHeader(self):
        safe_struct_header = ''
        for item in self.structMembers:
            if self.NeedSafeStruct(item) == True:
                safe_struct_header += '\n'
                if item.ifdef_protect is not None:
                    safe_struct_header += '#ifdef %s\n' % item.ifdef_protect
                safe_struct_header += self.structOrUnion[item.name] + ' safe_%s {\n' % (item.name)
                firstMemberInUnion = True
                isUnion = self.structOrUnion[item.name] == 'union'
                for member in item.members:
                    if member.type in self.structNames:
                        member_index = next((i for i, v in enumerate(self.structMembers) if v[0] == member.type), None)
                        if member_index is not None and self.NeedSafeStruct(self.structMembers[member_index]) == True:
                            if member.ispointer:
                                num_indirections = member.cdecl.count('*')
                                initString = '{}' if ((not isUnion) or (isUnion and firstMemberInUnion)) else ''
                                safe_struct_header += '    safe_%s%s %s%s;\n' % (member.type, '*' * num_indirections, member.name, initString)
                                if isUnion and firstMemberInUnion:
                                    firstMemberInUnion = False
                            else:
                                safe_struct_header += '    safe_%s %s;\n' % (member.type, member.name)
                            continue
                    if member.len is not None and (self.TypeContainsObjectHandle(member.type, True) or self.TypeContainsObjectHandle(member.type, False)):
                            safe_struct_header += '    %s* %s{};\n' % (member.type, member.name)
                    elif member.ispointer and firstMemberInUnion:
                        safe_struct_header += '%s{};\n' % member.cdecl
                        if isUnion and firstMemberInUnion:
                            firstMemberInUnion = False
                    else:
                        safe_struct_header += '%s;\n' % member.cdecl
                if (isUnion and item.name == 'VkDescriptorDataEXT'):
                    safe_struct_header += '    char type_at_end[sizeof(%s)+sizeof(%s)];\n' % (item.name, 'VkDescriptorGetInfoEXT::type')
                safe_struct_header += '    safe_%s(const %s* in_struct%s);\n' % (item.name, item.name, self.custom_construct_params.get(item.name, ''))
                safe_struct_header += '    safe_%s(const safe_%s& copy_src);\n' % (item.name, item.name)
                safe_struct_header += '    safe_%s& operator=(const safe_%s& copy_src);\n' % (item.name, item.name)
                safe_struct_header += '    safe_%s();\n' % item.name
                safe_struct_header += '    ~safe_%s();\n' % item.name
                safe_struct_header += '    void initialize(const %s* in_struct%s);\n' % (item.name, self.custom_construct_params.get(item.name, ''))
                safe_struct_header += '    void initialize(const safe_%s* copy_src);\n' % (item.name)
                safe_struct_header += '    %s *ptr() { return reinterpret_cast<%s *>(this); }\n' % (item.name, item.name)
                safe_struct_header += '    %s const *ptr() const { return reinterpret_cast<%s const *>(this); }\n' % (item.name, item.name)
                if item.name == 'VkShaderModuleCreateInfo':
                    safe_struct_header += '''
    // Primarily intended for use by GPUAV when replacing shader module code with instrumented code
    template<typename Container>
    void SetCode(const Container &code) {
        delete[] pCode;
        codeSize = static_cast<uint32_t>(code.size() * sizeof(uint32_t));
        pCode = new uint32_t[code.size()];
        std::copy(&code.front(), &code.back() + 1, const_cast<uint32_t*>(pCode));
    }
'''
                safe_struct_header += '};\n'
                if item.ifdef_protect is not None:
                    safe_struct_header += '#endif // %s\n' % item.ifdef_protect
        return safe_struct_header
    #
    # Generate extension helper header file
    def GenerateExtensionHelperHeader(self):

        promoted_1_1_exts = self.registry.tree.findall('*/extension[@promotedto="VK_VERSION_1_1"]')
        V_1_0_instance_extensions_promoted_to_V_1_1_core = sorted([e.get('name') for e in promoted_1_1_exts if e.get('type') == 'instance'])
        V_1_0_device_extensions_promoted_to_V_1_1_core = sorted([e.get('name') for e in promoted_1_1_exts if e.get('type') == 'device'])

        promoted_1_2_exts = self.registry.tree.findall('*/extension[@promotedto="VK_VERSION_1_2"]')
        V_1_1_instance_extensions_promoted_to_V_1_2_core = sorted([e.get('name') for e in promoted_1_2_exts if e.get('type') == 'instance'])
        V_1_1_device_extensions_promoted_to_V_1_2_core = sorted([e.get('name') for e in promoted_1_2_exts if e.get('type') == 'device'])

        promoted_1_3_exts = self.registry.tree.findall('*/extension[@promotedto="VK_VERSION_1_3"]')
        V_1_2_instance_extensions_promoted_to_V_1_3_core = sorted([e.get('name') for e in promoted_1_3_exts if e.get('type') == 'instance'])
        V_1_2_device_extensions_promoted_to_V_1_3_core = sorted([e.get('name') for e in promoted_1_3_exts if e.get('type') == 'device'])

        output = [
            '',
            '#ifndef VK_EXTENSION_HELPER_H_',
            '#define VK_EXTENSION_HELPER_H_',
            '#include <string>',
            '#include <utility>',
            '#include <set>',
            '#include <array>',
            '#include <vector>',
            '#include <cassert>',
            '',
            '#include <vulkan/vulkan.h>',
            '#include "vk_layer_data.h"',
            ''
            '#define VK_VERSION_1_1_NAME "VK_VERSION_1_1"',
            '',
            'enum ExtEnabled : unsigned char {',
            '    kNotEnabled,',
            '    kEnabledByCreateinfo,',
            '    kEnabledByApiLevel,',
            '};',
            '',
            '[[maybe_unused]] static bool IsExtEnabled(ExtEnabled extension) {',
            '    return (extension != kNotEnabled);',
            '};',
            '',
            '[[maybe_unused]] static bool IsExtEnabledByCreateinfo(ExtEnabled extension) {',
            '    return (extension == kEnabledByCreateinfo);',
            '};',
            '#define VK_VERSION_1_2_NAME "VK_VERSION_1_2"',
            '#define VK_VERSION_1_3_NAME "VK_VERSION_1_3"',
            '']

        for type in ['Instance', 'Device']:
            struct_type = '%sExtensions' % type
            if type == 'Instance':
                extension_dict = self.instance_extension_info
                promoted_1_1_ext_list = V_1_0_instance_extensions_promoted_to_V_1_1_core
                promoted_1_2_ext_list = V_1_1_instance_extensions_promoted_to_V_1_2_core
                promoted_1_3_ext_list = V_1_2_instance_extensions_promoted_to_V_1_3_core
                struct_decl = 'struct %s {' % struct_type
                instance_struct_type = struct_type
            else:
                extension_dict = self.device_extension_info
                promoted_1_1_ext_list = V_1_0_device_extensions_promoted_to_V_1_1_core
                promoted_1_2_ext_list = V_1_1_device_extensions_promoted_to_V_1_2_core
                promoted_1_3_ext_list = V_1_2_device_extensions_promoted_to_V_1_3_core
                struct_decl = 'struct %s : public %s {' % (struct_type, instance_struct_type)

            extension_items = sorted(extension_dict.items())

            field_name = { ext_name: ext_name.lower() for ext_name, info in extension_items }

            # Add in pseudo-extensions for core API versions so real extensions can depend on them
            extension_dict['VK_VERSION_1_3'] = {'define':"VK_VERSION_1_3_NAME", 'ifdef':None, 'reqs':[]}
            field_name['VK_VERSION_1_3'] = "vk_feature_version_1_3"
            extension_dict['VK_VERSION_1_2'] = {'define':"VK_VERSION_1_2_NAME", 'ifdef':None, 'reqs':[]}
            field_name['VK_VERSION_1_2'] = "vk_feature_version_1_2"
            extension_dict['VK_VERSION_1_1'] = {'define':"VK_VERSION_1_1_NAME", 'ifdef':None, 'reqs':[]}
            field_name['VK_VERSION_1_1'] = "vk_feature_version_1_1"

            if type == 'Instance':
                instance_field_name = field_name
                instance_extension_dict = extension_dict
            else:
                # Get complete field name and extension data for both Instance and Device extensions
                field_name.update(instance_field_name)
                extension_dict = extension_dict.copy()  # Don't modify the self.<dict> we're pointing to
                extension_dict.update(instance_extension_dict)

            # Output the data member list
            struct  = [struct_decl]
            struct.extend([ '    ExtEnabled vk_feature_version_1_1{kNotEnabled};'])
            struct.extend([ '    ExtEnabled vk_feature_version_1_2{kNotEnabled};'])
            struct.extend([ '    ExtEnabled vk_feature_version_1_3{kNotEnabled};'])
            struct.extend([ '    ExtEnabled %s{kNotEnabled};' % field_name[ext_name] for ext_name, info in extension_items])
            # TODO Issue 4841 -  It looks like framework is not ready for two properties structs per extension (like VK_EXT_descriptor_buffer have). Workarounding.
            struct.extend([ '    ExtEnabled vk_ext_descriptor_buffer_density{kNotEnabled};'])

            # Construct the extension information map -- mapping name to data member (field), and required extensions
            # The map is contained within a static function member for portability reasons.
            info_type = '%sInfo' % type
            info_map_type = '%sMap' % info_type
            req_type = '%sReq' % type
            req_vec_type = '%sVec' % req_type
            struct.extend([
                '',
                '    struct %s {' % req_type,
                '        const ExtEnabled %s::* enabled;' % struct_type,
                '        const char *name;',
                '    };',
                '    typedef std::vector<%s> %s;' % (req_type, req_vec_type),
                '    struct %s {' % info_type,
                '       %s(ExtEnabled %s::* state_, const %s requirements_): state(state_), requirements(requirements_) {}' % ( info_type, struct_type, req_vec_type),
                '       ExtEnabled %s::* state;' % struct_type,
                '       %s requirements;' % req_vec_type,
                '    };',
                '',
                '    typedef vvl::unordered_map<std::string,%s> %s;' % (info_type, info_map_type),
                '    static const %s &get_info_map() {' %info_map_type,
                '        static const %s info_map = {' % info_map_type ])
            struct.extend([
                '            {"VK_VERSION_1_1", %sInfo(&%sExtensions::vk_feature_version_1_1, {})},' % (type, type)])
            struct.extend([
                '            {"VK_VERSION_1_2", %sInfo(&%sExtensions::vk_feature_version_1_2, {})},' % (type, type)])
            struct.extend([
                '            {"VK_VERSION_1_3", %sInfo(&%sExtensions::vk_feature_version_1_3, {})},' % (type, type)])

            field_format = '&' + struct_type + '::%s'
            req_format = '{' + field_format+ ', %s}'
            req_indent = '\n                           '
            req_join = ',' + req_indent
            info_format = ('            {%s, ' + info_type + '(' + field_format + ', {%s})},')
            def format_info(ext_name, info):
                reqs = req_join.join([req_format % (field_name[req], extension_dict[req]['define']) for req in info['reqs']])
                return info_format % (info['define'], field_name[ext_name], '{%s}' % (req_indent + reqs) if reqs else '')

            struct.extend([Guarded(info['ifdef'], format_info(ext_name, info)) for ext_name, info in extension_items])
            struct.extend([
                '        };',
                '',
                '        return info_map;',
                '    }',
                '',
                '    static const %s &get_info(const char *name) {' % info_type,
                '        static const %s empty_info {nullptr, %s()};' % (info_type, req_vec_type),
                '        const auto &ext_map = %s::get_info_map();' % struct_type,
                '        const auto info = ext_map.find(name);',
                '        if ( info != ext_map.cend()) {',
                '            return info->second;',
                '        }',
                '        return empty_info;',
                '    }',
                ''])

            if type == 'Instance':
                struct.extend([
                    '    uint32_t NormalizeApiVersion(uint32_t specified_version) {',
                    '        if (specified_version < VK_API_VERSION_1_1)',
                    '            return VK_API_VERSION_1_0;',
                    '        else if (specified_version < VK_API_VERSION_1_2)',
                    '            return VK_API_VERSION_1_1;',
                    '        else if (specified_version < VK_API_VERSION_1_3)',
                    '            return VK_API_VERSION_1_2;',
                    '        else',
                    '            return VK_API_VERSION_1_3;',
                    '    }',
                    '',
                    '    uint32_t InitFromInstanceCreateInfo(uint32_t requested_api_version, const VkInstanceCreateInfo *pCreateInfo) {'])
            else:
                struct.extend([
                    '    %s() = default;' % struct_type,
                    '    %s(const %s& instance_ext) : %s(instance_ext) {}' % (struct_type, instance_struct_type, instance_struct_type),
                    '',
                    '    uint32_t InitFromDeviceCreateInfo(const %s *instance_extensions, uint32_t requested_api_version,' % instance_struct_type,
                    '                                      const VkDeviceCreateInfo *pCreateInfo = nullptr) {',
                    '        // Initialize: this to defaults,  base class fields to input.',
                    '        assert(instance_extensions);',
                    '        *this = %s(*instance_extensions);' % struct_type,
                    '']),
            struct.extend([
                '',
                f'        constexpr std::array<const char*, {len(promoted_1_1_ext_list)}> V_1_1_promoted_{type.lower()}_apis = {{' ])
            struct.extend(['            %s,' % extension_dict[ext_name]['define'] for ext_name in promoted_1_1_ext_list])
            struct.extend([
                '        };',
                f'        constexpr std::array<const char*, {len(promoted_1_2_ext_list)}> V_1_2_promoted_{type.lower()}_apis = {{' ])
            struct.extend(['            %s,' % extension_dict[ext_name]['define'] for ext_name in promoted_1_2_ext_list])
            struct.extend([
                '        };',
                f'        constexpr std::array<const char*, {len(promoted_1_3_ext_list)}> V_1_3_promoted_{type.lower()}_apis = {{' ])
            struct.extend(['            %s,' % extension_dict[ext_name]['define'] for ext_name in promoted_1_3_ext_list])
            struct.extend([
                '        };',
                '',
                '        // Initialize struct data, robust to invalid pCreateInfo',
                '        uint32_t api_version = NormalizeApiVersion(requested_api_version);',
                '        if (api_version >= VK_API_VERSION_1_1) {',
                '            auto info = get_info("VK_VERSION_1_1");',
                '            if (info.state) this->*(info.state) = kEnabledByCreateinfo;',
                '            for (auto promoted_ext : V_1_1_promoted_%s_apis) {' % type.lower(),
                '                info = get_info(promoted_ext);',
                '                assert(info.state);',
                '                if (info.state) this->*(info.state) = kEnabledByApiLevel;',
                '            }',
                '        }',
                '        if (api_version >= VK_API_VERSION_1_2) {',
                '            auto info = get_info("VK_VERSION_1_2");',
                '            if (info.state) this->*(info.state) = kEnabledByCreateinfo;',
                '            for (auto promoted_ext : V_1_2_promoted_%s_apis) {' % type.lower(),
                '                info = get_info(promoted_ext);',
                '                assert(info.state);',
                '                if (info.state) this->*(info.state) = kEnabledByApiLevel;',
                '            }',
                '        }',
                '        if (api_version >= VK_API_VERSION_1_3) {',
                '            auto info = get_info("VK_VERSION_1_3");',
                '            if (info.state) this->*(info.state) = kEnabledByCreateinfo;',
                '            for (auto promoted_ext : V_1_3_promoted_%s_apis) {' % type.lower(),
                '                info = get_info(promoted_ext);',
                '                assert(info.state);',
                '                if (info.state) this->*(info.state) = kEnabledByApiLevel;',
                '            }',
                '        }',
                '        // CreateInfo takes precedence over promoted',
                '        if (pCreateInfo && pCreateInfo->ppEnabledExtensionNames) {',
                '            for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {',
                '                if (!pCreateInfo->ppEnabledExtensionNames[i]) continue;',
                '                auto info = get_info(pCreateInfo->ppEnabledExtensionNames[i]);',
                '                if (info.state) this->*(info.state) = kEnabledByCreateinfo;',
                '            }',
                '        }',
                '        return api_version;',
                '    }',
                '};'])

            # Output reference lists of instance/device extension names
            struct.extend(['', 'static const std::set<std::string> k%sExtensionNames = {' % type])
            struct.extend([Guarded(info['ifdef'], '    %s,' % info['define']) for ext_name, info in extension_items])
            struct.extend(['};', ''])
            output.extend(struct)

        output.extend(['', '#endif // VK_EXTENSION_HELPER_H_'])
        return '\n'.join(output)
    #
    # Combine object types helper header file preamble with body text and return
    def GenerateObjectTypesHelperHeader(self):
        object_types_helper_header = '\n'
        object_types_helper_header += '#pragma once\n'
        object_types_helper_header += '\n'
        object_types_helper_header += self.GenerateObjectTypesHeader()
        return object_types_helper_header
    #
    # Object types header: create object enum type header file
    def GenerateObjectTypesHeader(self):
        object_types_header = '#include "cast_utils.h"\n'
        object_types_header += '\n'
        object_types_header += '// Object Type enum for validation layer internal object handling\n'
        object_types_header += 'typedef enum VulkanObjectType {\n'
        object_types_header += '    kVulkanObjectTypeUnknown = 0,\n'
        enum_num = 1
        type_list = []
        enum_entry_map = {}
        non_dispatchable = {}
        dispatchable = {}
        object_type_info = {}

        # Output enum definition as each handle is processed, saving the names to use for the conversion routine
        for item in self.object_types:
            fixup_name = item[2:]
            enum_entry = 'kVulkanObjectType%s' % fixup_name
            enum_entry_map[item] = enum_entry
            object_types_header += '    ' + enum_entry
            object_types_header += ' = %d,\n' % enum_num
            enum_num += 1
            type_list.append(enum_entry)
            object_type_info[enum_entry] = { 'VkType': item , 'Guard': self.object_guards[item]}
            # We'll want lists of the dispatchable and non dispatchable handles below with access to the same info
            if self.handle_types.IsNonDispatchable(item):
                non_dispatchable[item] = enum_entry
            else:
                dispatchable[item] = enum_entry

        object_types_header += '    kVulkanObjectTypeMax = %d,\n' % enum_num
        object_types_header += '    // Aliases for backwards compatibilty of "promoted" types\n'
        for (name, alias) in self.object_type_aliases:
            fixup_name = name[2:]
            object_types_header += '    kVulkanObjectType{} = {},\n'.format(fixup_name, enum_entry_map[alias])
        object_types_header += '} VulkanObjectType;\n\n'

        # Output name string helper
        object_types_header += '// Array of object name strings for OBJECT_TYPE enum conversion\n'
        object_types_header += 'static const char * const object_string[kVulkanObjectTypeMax] = {\n'
        object_types_header += '    "VkNonDispatchableHandle",\n'
        for item in self.object_types:
            object_types_header += '    "%s",\n' % item
        object_types_header += '};\n'

        # Helpers to create unified dict key from k<Name>, VK_OBJECT_TYPE_<Name>, and VK_DEBUG_REPORT_OBJECT_TYPE_<Name>
        def dro_to_key(raw_key): return re.search('^VK_DEBUG_REPORT_OBJECT_TYPE_(.*)_EXT$', raw_key).group(1).lower().replace("_","")
        def vko_to_key(raw_key): return re.search('^VK_OBJECT_TYPE_(.*)', raw_key).group(1).lower().replace("_","")
        def kenum_to_key(raw_key): return re.search('^kVulkanObjectType(.*)', raw_key).group(1).lower()

        dro_dict = {dro_to_key(dro) : dro for dro in self.debug_report_object_types}
        vko_dict = {vko_to_key(vko) : vko for vko in self.core_object_types}

        # Output a conversion routine from the layer object definitions to the debug report definitions
        object_types_header += '\n'
        object_types_header += '// Helper array to get Vulkan VK_EXT_debug_report object type enum from the internal layers version\n'
        object_types_header += 'const VkDebugReportObjectTypeEXT get_debug_report_enum[] = {\n'
        object_types_header += '    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, // kVulkanObjectTypeUnknown\n' # no unknown handle, so this must be here explicitly

        for object_type in type_list:
            # VK_DEBUG_REPORT is not updated anymore; there might be missing object types
            kenum_type = dro_dict.get(kenum_to_key(object_type), 'VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT')
            if object_type_info[object_type]['Guard']:
                object_types_header += '#ifdef %s\n' % object_type_info[object_type]['Guard']
            object_types_header += '    %s,   // %s\n' % (kenum_type, object_type)
            if object_type_info[object_type]['Guard']:
                object_types_header += '#else\n'
                object_types_header += '    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,   // %s\n' % object_type
                object_types_header += '#endif\n'
            object_type_info[object_type]['DbgType'] = kenum_type
        object_types_header += '};\n'

        # Output a conversion routine from the layer object definitions to the core object type definitions
        # This will intentionally *fail* for unmatched types as the VK_OBJECT_TYPE list should match the kVulkanObjectType list
        object_types_header += '\n'
        object_types_header += '// Helper function to get Official Vulkan VkObjectType enum from the internal layers version\n'
        object_types_header += 'static inline VkObjectType ConvertVulkanObjectToCoreObject(VulkanObjectType internal_type) {\n'
        object_types_header += '    switch (internal_type) {\n'

        for object_type in type_list:
            kenum_type = vko_dict[kenum_to_key(object_type)]
            if object_type_info[object_type]['Guard']:
                object_types_header += '#ifdef %s\n' % object_type_info[object_type]['Guard']
            object_types_header += '        case %s: return %s;\n' % (object_type, kenum_type)
            if object_type_info[object_type]['Guard']:
                object_types_header += '#endif\n'
            object_type_info[object_type]['VkoType'] = kenum_type
        object_types_header += '        default: return VK_OBJECT_TYPE_UNKNOWN;\n'
        object_types_header += '    }\n'
        object_types_header += '};\n'

        # Output a function converting from core object type definitions to the Vulkan object type enums
        object_types_header += '\n'
        object_types_header += '// Helper function to get internal layers object ids from the official Vulkan VkObjectType enum\n'
        object_types_header += 'static inline VulkanObjectType ConvertCoreObjectToVulkanObject(VkObjectType vulkan_object_type) {\n'
        object_types_header += '    switch (vulkan_object_type) {\n'

        for object_type in type_list:
            kenum_type = vko_dict[kenum_to_key(object_type)]
            if object_type_info[object_type]['Guard']:
                object_types_header += '#ifdef %s\n' % object_type_info[object_type]['Guard']
            object_types_header += '        case %s: return %s;\n' % (kenum_type, object_type)
            if object_type_info[object_type]['Guard']:
                object_types_header += '#endif\n'
        object_types_header += '        default: return kVulkanObjectTypeUnknown;\n'
        object_types_header += '    }\n'
        object_types_header += '};\n'

        # Create a functions to convert between VkDebugReportObjectTypeEXT and VkObjectType
        object_types_header +=     '\n'
        object_types_header +=     'static inline VkObjectType convertDebugReportObjectToCoreObject(VkDebugReportObjectTypeEXT debug_report_obj) {\n'
        object_types_header +=     '    switch (debug_report_obj) {\n'
        for dr_object_type in self.debug_report_object_types:
            object_types_header += '        case %s: return %s;\n' % (dr_object_type, vko_dict[dro_to_key(dr_object_type)])
        object_types_header +=     '        default: return VK_OBJECT_TYPE_UNKNOWN;\n'
        object_types_header +=     '    }\n'
        object_types_header +=     '}\n'

        object_types_header +=         '\n'
        object_types_header +=         'static inline VkDebugReportObjectTypeEXT convertCoreObjectToDebugReportObject(VkObjectType core_report_obj) {\n'
        object_types_header +=         '    switch (core_report_obj) {\n'
        for core_object_type in self.core_object_types:
            # VK_DEBUG_REPORT is not updated anymore; there might be missing object types
            dr_object_type = dro_dict.get(vko_to_key(core_object_type))
            if dr_object_type is not None:
                object_types_header += '        case %s: return %s;\n' % (core_object_type, dr_object_type)
        object_types_header +=         '        default: return VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;\n'
        object_types_header +=         '    }\n'
        object_types_header +=         '}\n'

        #
        object_types_header += '\n'
        traits_format = Outdent('''
            template <> struct VkHandleInfo<{vk_type}> {{
                static const VulkanObjectType kVulkanObjectType = {obj_type};
                static const VkDebugReportObjectTypeEXT kDebugReportObjectType = {dbg_type};
                static const VkObjectType kVkObjectType = {vko_type};
                static const char* Typename() {{
                    return "{vk_type}";
                }}
            }};
            template <> struct VulkanObjectTypeInfo<{obj_type}> {{
                typedef {vk_type} Type;
            }};
            ''')

        object_types_header += Outdent('''
            // Traits objects from each type statically map from Vk<handleType> to the various enums
            template <typename VkType> struct VkHandleInfo {};
            template <VulkanObjectType id> struct VulkanObjectTypeInfo {};

            // The following line must match the vulkan_core.h condition guarding VK_DEFINE_NON_DISPATCHABLE_HANDLE
            #if defined(__LP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__)) || defined(_M_X64) || defined(__ia64) || \
                defined(_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
            #define TYPESAFE_NONDISPATCHABLE_HANDLES
            #else
            VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkNonDispatchableHandle)
            ''')  +'\n'
        object_types_header += traits_format.format(vk_type='VkNonDispatchableHandle', obj_type='kVulkanObjectTypeUnknown',
                                                  dbg_type='VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT',
                                                  vko_type='VK_OBJECT_TYPE_UNKNOWN') + '\n'
        object_types_header += '#endif //  VK_DEFINE_HANDLE logic duplication\n'

        for vk_type, object_type in sorted(dispatchable.items()):
            info = object_type_info[object_type]
            object_types_header += traits_format.format(vk_type=vk_type, obj_type=object_type, dbg_type=info['DbgType'],
                                                      vko_type=info['VkoType'])
        object_types_header += '#ifdef TYPESAFE_NONDISPATCHABLE_HANDLES\n'
        for vk_type, object_type in sorted(non_dispatchable.items()):
            info = object_type_info[object_type]
            if info['Guard']:
                object_types_header += '#ifdef {}\n'.format(info['Guard'])
            object_types_header += traits_format.format(vk_type=vk_type, obj_type=object_type, dbg_type=info['DbgType'],
                                                      vko_type=info['VkoType'])
            if info['Guard']:
                object_types_header += '#endif\n'
        object_types_header += '#endif // TYPESAFE_NONDISPATCHABLE_HANDLES\n'

        object_types_header += Outdent('''
            struct VulkanTypedHandle {
                uint64_t handle;
                VulkanObjectType type;
                template <typename Handle>
                VulkanTypedHandle(Handle handle_, VulkanObjectType type_) :
                    handle(CastToUint64(handle_)),
                    type(type_) {
            #ifdef TYPESAFE_NONDISPATCHABLE_HANDLES
                    // For 32 bit it's not always safe to check for traits <-> type
                    // as all non-dispatchable handles have the same type-id and thus traits,
                    // but on 64 bit we can validate the passed type matches the passed handle
                    assert(type == VkHandleInfo<Handle>::kVulkanObjectType);
            #endif // TYPESAFE_NONDISPATCHABLE_HANDLES
                }
                template <typename Handle>
                Handle Cast() const {
            #ifdef TYPESAFE_NONDISPATCHABLE_HANDLES
                    assert(type == VkHandleInfo<Handle>::kVulkanObjectType);
            #endif // TYPESAFE_NONDISPATCHABLE_HANDLES
                    return CastFromUint64<Handle>(handle);
                }
                VulkanTypedHandle() :
                    handle(CastToUint64(VK_NULL_HANDLE)),
                    type(kVulkanObjectTypeUnknown) {}
                operator bool() const { return handle != 0; }
            }; ''')  +'\n'

        return object_types_header
    #
    # Generate pNext handling function
    def build_safe_struct_utility_funcs(self):
        # Construct Safe-struct helper functions

        string_copy_proc = '\n\n'
        string_copy_proc += 'char *SafeStringCopy(const char *in_string) {\n'
        string_copy_proc += '    if (nullptr == in_string) return nullptr;\n'
        string_copy_proc += '    char* dest = new char[std::strlen(in_string) + 1];\n'
        string_copy_proc += '    return std::strcpy(dest, in_string);\n'
        string_copy_proc += '}\n'

        build_pnext_proc = '\n'
        build_pnext_proc += 'void *SafePnextCopy(const void *pNext) {\n'
        build_pnext_proc += '    if (!pNext) return nullptr;\n'
        build_pnext_proc += '\n'
        build_pnext_proc += '    void *safe_pNext{};\n'
        build_pnext_proc += '    const VkBaseOutStructure *header = reinterpret_cast<const VkBaseOutStructure *>(pNext);\n'
        build_pnext_proc += '\n'
        build_pnext_proc += '    switch (header->sType) {\n'
        # Add special-case code to copy beloved secret loader structs
        build_pnext_proc += '        // Special-case Loader Instance Struct passed to/from layer in pNext chain\n'
        build_pnext_proc += '        case VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO: {\n'
        build_pnext_proc += '            VkLayerInstanceCreateInfo *struct_copy = new VkLayerInstanceCreateInfo;\n'
        build_pnext_proc += '            // TODO: Uses original VkLayerInstanceLink* chain, which should be okay for our uses\n'
        build_pnext_proc += '            memcpy(struct_copy, pNext, sizeof(VkLayerInstanceCreateInfo));\n'
        build_pnext_proc += '            struct_copy->pNext = SafePnextCopy(header->pNext);\n'
        build_pnext_proc += '            safe_pNext = struct_copy;\n'
        build_pnext_proc += '            break;\n'
        build_pnext_proc += '        }\n'
        build_pnext_proc += '        // Special-case Loader Device Struct passed to/from layer in pNext chain\n'
        build_pnext_proc += '        case VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO: {\n'
        build_pnext_proc += '            VkLayerDeviceCreateInfo *struct_copy = new VkLayerDeviceCreateInfo;\n'
        build_pnext_proc += '            // TODO: Uses original VkLayerDeviceLink*, which should be okay for our uses\n'
        build_pnext_proc += '            memcpy(struct_copy, pNext, sizeof(VkLayerDeviceCreateInfo));\n'
        build_pnext_proc += '            struct_copy->pNext = SafePnextCopy(header->pNext);\n'
        build_pnext_proc += '            safe_pNext = struct_copy;\n'
        build_pnext_proc += '            break;\n'
        build_pnext_proc += '        }\n'

        free_pnext_proc = '\n'
        free_pnext_proc += 'void FreePnextChain(const void *pNext) {\n'
        free_pnext_proc += '    if (!pNext) return;\n'
        free_pnext_proc += '\n'
        free_pnext_proc += '    auto header = reinterpret_cast<const VkBaseOutStructure *>(pNext);\n'
        free_pnext_proc += '\n'
        free_pnext_proc += '    switch (header->sType) {\n'
        free_pnext_proc += '        // Special-case Loader Instance Struct passed to/from layer in pNext chain\n'
        free_pnext_proc += '        case VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO:\n'
        free_pnext_proc += '            FreePnextChain(header->pNext);\n'
        free_pnext_proc += '            delete reinterpret_cast<const VkLayerInstanceCreateInfo *>(pNext);\n'
        free_pnext_proc += '            break;\n'
        free_pnext_proc += '        // Special-case Loader Device Struct passed to/from layer in pNext chain\n'
        free_pnext_proc += '        case VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO:\n'
        free_pnext_proc += '            FreePnextChain(header->pNext);\n'
        free_pnext_proc += '            delete reinterpret_cast<const VkLayerDeviceCreateInfo *>(pNext);\n'
        free_pnext_proc += '            break;\n'

        chain_structs = tuple(s for s in self.structMembers if s.name in self.structextends_list)
        ifdefs = sorted({cs.ifdef_protect for cs in chain_structs}, key = lambda i : i if i is not None else '')
        for ifdef in ifdefs:
            if ifdef is not None:
                build_pnext_proc += '#ifdef %s\n' % ifdef
                free_pnext_proc += '#ifdef %s\n' % ifdef

            assorted_chain_structs = tuple(s for s in chain_structs if s.ifdef_protect == ifdef)
            for struct in assorted_chain_structs:
                build_pnext_proc += '        case %s:\n' % self.structTypes[struct.name].value
                build_pnext_proc += '            safe_pNext = new safe_%s(reinterpret_cast<const %s *>(pNext));\n' % (struct.name, struct.name)
                build_pnext_proc += '            break;\n'

                free_pnext_proc += '        case %s:\n' % self.structTypes[struct.name].value
                free_pnext_proc += '            delete reinterpret_cast<const safe_%s *>(header);\n' % struct.name
                free_pnext_proc += '            break;\n'

            if ifdef is not None:
                build_pnext_proc += '#endif // %s\n' % ifdef
                free_pnext_proc += '#endif // %s\n' % ifdef

        build_pnext_proc += '        default: // Encountered an unknown sType -- skip (do not copy) this entry in the chain\n'
        build_pnext_proc += '            // If sType is in custom list, construct blind copy\n'
        build_pnext_proc += '            for (auto item : custom_stype_info) {\n'
        build_pnext_proc += '                if (item.first == header->sType) {\n'
        build_pnext_proc += '                    safe_pNext = malloc(item.second);\n'
        build_pnext_proc += '                    memcpy(safe_pNext, header, item.second);\n'
        build_pnext_proc += '                    // Deep copy the rest of the pNext chain\n'
        build_pnext_proc += '                    VkBaseOutStructure *custom_struct = reinterpret_cast<VkBaseOutStructure *>(safe_pNext);\n'
        build_pnext_proc += '                    if (custom_struct->pNext) {\n'
        build_pnext_proc += '                        custom_struct->pNext = reinterpret_cast<VkBaseOutStructure *>(SafePnextCopy(custom_struct->pNext));\n'
        build_pnext_proc += '                    }\n'
        build_pnext_proc += '                }\n'
        build_pnext_proc += '            }\n'
        build_pnext_proc += '            if (!safe_pNext) {\n'
        build_pnext_proc += '                safe_pNext = SafePnextCopy(header->pNext);\n'
        build_pnext_proc += '            }\n'
        build_pnext_proc += '            break;\n'
        build_pnext_proc += '    }\n'
        build_pnext_proc += '\n'
        build_pnext_proc += '    return safe_pNext;\n'
        build_pnext_proc += '}\n'

        free_pnext_proc += '        default: // Encountered an unknown sType\n'
        free_pnext_proc += '            // If sType is in custom list, free custom struct memory and clean up\n'
        free_pnext_proc += '            for (auto item : custom_stype_info) {\n'
        free_pnext_proc += '                if (item.first == header->sType) {\n'
        free_pnext_proc += '                    if (header->pNext) {\n'
        free_pnext_proc += '                        FreePnextChain(header->pNext);\n'
        free_pnext_proc += '                    }\n'
        free_pnext_proc += '                    free(const_cast<void *>(pNext));\n'
        free_pnext_proc += '                    pNext = nullptr;\n'
        free_pnext_proc += '                    break;\n'
        free_pnext_proc += '                }\n'
        free_pnext_proc += '            }\n'
        free_pnext_proc += '            if (pNext) {\n'
        free_pnext_proc += '                FreePnextChain(header->pNext);\n'
        free_pnext_proc += '            }\n'
        free_pnext_proc += '            break;\n'
        free_pnext_proc += '    }\n'
        free_pnext_proc += '}\n'

        pnext_procs = string_copy_proc + build_pnext_proc + free_pnext_proc
        return pnext_procs
    #
    # Determine if a structure needs a safe_struct helper function
    # That is, it has an sType or one of its members is a pointer
    def NeedSafeStruct(self, structure):
        if 'VkBase' in structure.name:
            return False
        if 'sType' == structure.name:
            return True
        for member in structure.members:
            if member.ispointer == True:
                return True
        return False
    #
    # Combine safe struct helper source file preamble with body text and return
    def GenerateSafeStructHelperSource(self):
        safe_struct_helper_source = '\n'
        safe_struct_helper_source += '#include "vk_safe_struct.h"\n'
        safe_struct_helper_source += '#include "vk_typemap_helper.h"\n'
        safe_struct_helper_source += '#include "vk_layer_utils.h"\n'
        safe_struct_helper_source += '\n'
        safe_struct_helper_source += '#include <string.h>\n'
        safe_struct_helper_source += '#include <cassert>\n'
        safe_struct_helper_source += '#include <cstring>\n'
        safe_struct_helper_source += '#include <vector>\n'
        safe_struct_helper_source += '\n'
        safe_struct_helper_source += '#include <vulkan/vk_layer.h>\n'
        safe_struct_helper_source += '\n'
        safe_struct_helper_source += 'extern std::vector<std::pair<uint32_t, uint32_t>> custom_stype_info;\n'
        safe_struct_helper_source += 'struct ASGeomKHRExtraData {\n'
        safe_struct_helper_source += '    ASGeomKHRExtraData(uint8_t *alloc, uint32_t primOffset, uint32_t primCount) :\n'
        safe_struct_helper_source += '        ptr(alloc),\n'
        safe_struct_helper_source += '        primitiveOffset(primOffset),\n'
        safe_struct_helper_source += '        primitiveCount(primCount)\n'
        safe_struct_helper_source += '    {}\n'
        safe_struct_helper_source += '    ~ASGeomKHRExtraData() {\n'
        safe_struct_helper_source += '        if (ptr)\n'
        safe_struct_helper_source += '            delete[] ptr;\n'
        safe_struct_helper_source += '    }\n'
        safe_struct_helper_source += '    uint8_t *ptr;\n'
        safe_struct_helper_source += '    uint32_t primitiveOffset;\n'
        safe_struct_helper_source += '    uint32_t primitiveCount;\n'
        safe_struct_helper_source += '};\n'
        safe_struct_helper_source += 'vl_concurrent_unordered_map<const safe_VkAccelerationStructureGeometryKHR*, ASGeomKHRExtraData*, 4> as_geom_khr_host_alloc;\n'
        safe_struct_helper_source += '\n'
        safe_struct_helper_source += self.GenerateSafeStructSource()
        safe_struct_helper_source += self.build_safe_struct_utility_funcs()

        return safe_struct_helper_source
    #
    # safe_struct source -- create bodies of safe struct helper functions
    def GenerateSafeStructSource(self):
        safe_struct_body = []
        wsi_structs = ['VkXlibSurfaceCreateInfoKHR',
                       'VkXcbSurfaceCreateInfoKHR',
                       'VkWaylandSurfaceCreateInfoKHR',
                       'VkAndroidSurfaceCreateInfoKHR',
                       'VkWin32SurfaceCreateInfoKHR',
                       'VkIOSSurfaceCreateInfoMVK',
                       'VkMacOSSurfaceCreateInfoMVK',
                       'VkMetalSurfaceCreateInfoEXT'
                       ]

        member_init_transforms = {
            'queueFamilyIndexCount': lambda m: f'{m.name}(0)'
        }

        def qfi_construct(item, member):
            true_index_setter = lambda i: f'{i}queueFamilyIndexCount = in_struct->queueFamilyIndexCount;\n'
            false_index_setter = lambda i: f'{i}queueFamilyIndexCount = 0;\n'
            if item.name == 'VkSwapchainCreateInfoKHR':
                return (f'(in_struct->imageSharingMode == VK_SHARING_MODE_CONCURRENT) && in_struct->{member.name}', true_index_setter, false_index_setter)
            else:
                return (f'(in_struct->sharingMode == VK_SHARING_MODE_CONCURRENT) && in_struct->{member.name}', true_index_setter, false_index_setter)

        # map of:
        #  <member name>: function(item, member) -> (condition, true statement, false statement)
        member_construct_conditions = {
            'pQueueFamilyIndices': qfi_construct
        }

        # For abstract types just want to save the pointer away
        # since we cannot make a copy.
        abstract_types = ['AHardwareBuffer',
                          'ANativeWindow',
                          'CAMetalLayer'
                         ]
        for item in self.structMembers:
            if self.NeedSafeStruct(item) == False:
                continue
            if item.name in wsi_structs:
                continue
            if item.ifdef_protect is not None:
                safe_struct_body.append("#ifdef %s\n" % item.ifdef_protect)
            ss_name = "safe_%s" % item.name
            init_list = ''          # list of members in struct constructor initializer
            default_init_list = ''  # Default constructor just inits ptrs to nullptr in initializer
            init_func_txt = ''      # Txt for initialize() function that takes struct ptr and inits members
            construct_txt = ''      # Body of constuctor as well as body of initialize() func following init_func_txt
            destruct_txt = ''

            custom_defeault_construct_txt = {
                'VkDescriptorDataEXT' :
                    '    VkDescriptorType* pType = (VkDescriptorType*)&type_at_end[sizeof(VkDescriptorDataEXT)];\n'
                    '    *pType = VK_DESCRIPTOR_TYPE_MAX_ENUM;\n'
            }
            custom_construct_txt = {
                # VkWriteDescriptorSet is special case because pointers may be non-null but ignored
                'VkWriteDescriptorSet' :
                    '    switch (descriptorType) {\n'
                    '        case VK_DESCRIPTOR_TYPE_SAMPLER:\n'
                    '        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:\n'
                    '        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:\n'
                    '        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:\n'
                    '        if (descriptorCount && in_struct->pImageInfo) {\n'
                    '            pImageInfo = new VkDescriptorImageInfo[descriptorCount];\n'
                    '            for (uint32_t i = 0; i < descriptorCount; ++i) {\n'
                    '                pImageInfo[i] = in_struct->pImageInfo[i];\n'
                    '            }\n'
                    '        }\n'
                    '        break;\n'
                    '        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:\n'
                    '        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:\n'
                    '        if (descriptorCount && in_struct->pBufferInfo) {\n'
                    '            pBufferInfo = new VkDescriptorBufferInfo[descriptorCount];\n'
                    '            for (uint32_t i = 0; i < descriptorCount; ++i) {\n'
                    '                pBufferInfo[i] = in_struct->pBufferInfo[i];\n'
                    '            }\n'
                    '        }\n'
                    '        break;\n'
                    '        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:\n'
                    '        if (descriptorCount && in_struct->pTexelBufferView) {\n'
                    '            pTexelBufferView = new VkBufferView[descriptorCount];\n'
                    '            for (uint32_t i = 0; i < descriptorCount; ++i) {\n'
                    '                pTexelBufferView[i] = in_struct->pTexelBufferView[i];\n'
                    '            }\n'
                    '        }\n'
                    '        break;\n'
                    '        default:\n'
                    '        break;\n'
                    '    }\n',
                'VkShaderModuleCreateInfo' :
                    '    if (in_struct->pCode) {\n'
                    '        pCode = reinterpret_cast<uint32_t *>(new uint8_t[codeSize]);\n'
                    '        memcpy((void *)pCode, (void *)in_struct->pCode, codeSize);\n'
                    '    }\n',
                # VkGraphicsPipelineCreateInfo is special case because its pointers may be non-null but ignored
                'VkGraphicsPipelineCreateInfo' :
                    '    const bool is_graphics_library = LvlFindInChain<VkGraphicsPipelineLibraryCreateInfoEXT>(in_struct->pNext) != nullptr;\n'
                    '    if (stageCount && in_struct->pStages) {\n'
                    '        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];\n'
                    '        for (uint32_t i = 0; i < stageCount; ++i) {\n'
                    '            pStages[i].initialize(&in_struct->pStages[i]);\n'
                    '        }\n'
                    '    }\n'
                    '    if (in_struct->pVertexInputState)\n'
                    '        pVertexInputState = new safe_VkPipelineVertexInputStateCreateInfo(in_struct->pVertexInputState);\n'
                    '    else\n'
                    '        pVertexInputState = nullptr;\n'
                    '    if (in_struct->pInputAssemblyState)\n'
                    '        pInputAssemblyState = new safe_VkPipelineInputAssemblyStateCreateInfo(in_struct->pInputAssemblyState);\n'
                    '    else\n'
                    '        pInputAssemblyState = nullptr;\n'
                    '    bool has_tessellation_stage = false;\n'
                    '    if (stageCount && pStages)\n'
                    '        for (uint32_t i = 0; i < stageCount && !has_tessellation_stage; ++i)\n'
                    '            if (pStages[i].stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT || pStages[i].stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)\n'
                    '                has_tessellation_stage = true;\n'
                    '    if (in_struct->pTessellationState && has_tessellation_stage)\n'
                    '        pTessellationState = new safe_VkPipelineTessellationStateCreateInfo(in_struct->pTessellationState);\n'
                    '    else\n'
                    '        pTessellationState = nullptr; // original pTessellationState pointer ignored\n'
                    '    bool is_dynamic_has_rasterization = false;\n'
                    '    if (in_struct->pDynamicState && in_struct->pDynamicState->pDynamicStates) {\n'
                    '        for (uint32_t i = 0; i < in_struct->pDynamicState->dynamicStateCount && !is_dynamic_has_rasterization; ++i)\n'
                    '            if (in_struct->pDynamicState->pDynamicStates[i] == VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE_EXT)\n'
                    '                is_dynamic_has_rasterization = true;\n'
                    '    }\n'
                    '    const bool has_rasterization = in_struct->pRasterizationState ? (is_dynamic_has_rasterization || !in_struct->pRasterizationState->rasterizerDiscardEnable) : false;\n'
                    '    if (in_struct->pViewportState && (has_rasterization || is_graphics_library)) {\n'
                    '        bool is_dynamic_viewports = false;\n'
                    '        bool is_dynamic_scissors = false;\n'
                    '        if (in_struct->pDynamicState && in_struct->pDynamicState->pDynamicStates) {\n'
                    '            for (uint32_t i = 0; i < in_struct->pDynamicState->dynamicStateCount && !is_dynamic_viewports; ++i)\n'
                    '                if (in_struct->pDynamicState->pDynamicStates[i] == VK_DYNAMIC_STATE_VIEWPORT)\n'
                    '                    is_dynamic_viewports = true;\n'
                    '            for (uint32_t i = 0; i < in_struct->pDynamicState->dynamicStateCount && !is_dynamic_scissors; ++i)\n'
                    '                if (in_struct->pDynamicState->pDynamicStates[i] == VK_DYNAMIC_STATE_SCISSOR)\n'
                    '                    is_dynamic_scissors = true;\n'
                    '        }\n'
                    '        pViewportState = new safe_VkPipelineViewportStateCreateInfo(in_struct->pViewportState, is_dynamic_viewports, is_dynamic_scissors);\n'
                    '    } else\n'
                    '        pViewportState = nullptr; // original pViewportState pointer ignored\n'
                    '    if (in_struct->pRasterizationState)\n'
                    '        pRasterizationState = new safe_VkPipelineRasterizationStateCreateInfo(in_struct->pRasterizationState);\n'
                    '    else\n'
                    '        pRasterizationState = nullptr;\n'
                    '    if (in_struct->pMultisampleState && (renderPass != VK_NULL_HANDLE || has_rasterization || is_graphics_library))\n'
                    '        pMultisampleState = new safe_VkPipelineMultisampleStateCreateInfo(in_struct->pMultisampleState);\n'
                    '    else\n'
                    '        pMultisampleState = nullptr; // original pMultisampleState pointer ignored\n'
                    '    // needs a tracked subpass state uses_depthstencil_attachment\n'
                    '    if (in_struct->pDepthStencilState && ((has_rasterization && uses_depthstencil_attachment) || is_graphics_library))\n'
                    '        pDepthStencilState = new safe_VkPipelineDepthStencilStateCreateInfo(in_struct->pDepthStencilState);\n'
                    '    else\n'
                    '        pDepthStencilState = nullptr; // original pDepthStencilState pointer ignored\n'
                    '    // needs a tracked subpass state usesColorAttachment\n'
                    '    if (in_struct->pColorBlendState && ((has_rasterization && uses_color_attachment) || is_graphics_library))\n'
                    '        pColorBlendState = new safe_VkPipelineColorBlendStateCreateInfo(in_struct->pColorBlendState);\n'
                    '    else\n'
                    '        pColorBlendState = nullptr; // original pColorBlendState pointer ignored\n'
                    '    if (in_struct->pDynamicState)\n'
                    '        pDynamicState = new safe_VkPipelineDynamicStateCreateInfo(in_struct->pDynamicState);\n'
                    '    else\n'
                    '        pDynamicState = nullptr;\n',
                 # VkPipelineViewportStateCreateInfo is special case because its pointers may be non-null but ignored
                'VkPipelineViewportStateCreateInfo' :
                    '    if (in_struct->pViewports && !is_dynamic_viewports) {\n'
                    '        pViewports = new VkViewport[in_struct->viewportCount];\n'
                    '        memcpy ((void *)pViewports, (void *)in_struct->pViewports, sizeof(VkViewport)*in_struct->viewportCount);\n'
                    '    }\n'
                    '    else\n'
                    '        pViewports = nullptr;\n'
                    '    if (in_struct->pScissors && !is_dynamic_scissors) {\n'
                    '        pScissors = new VkRect2D[in_struct->scissorCount];\n'
                    '        memcpy ((void *)pScissors, (void *)in_struct->pScissors, sizeof(VkRect2D)*in_struct->scissorCount);\n'
                    '    }\n'
                    '    else\n'
                    '        pScissors = nullptr;\n',
                # VkFrameBufferCreateInfo is special case because its pAttachments pointer may be non-null but ignored
                'VkFramebufferCreateInfo' :
                    '    if (attachmentCount && in_struct->pAttachments && !(flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT)) {\n'
                    '        pAttachments = new VkImageView[attachmentCount];\n'
                    '        for (uint32_t i = 0; i < attachmentCount; ++i) {\n'
                    '            pAttachments[i] = in_struct->pAttachments[i];\n'
                    '        }\n'
                    '    }\n',
                # VkDescriptorSetLayoutBinding is special case because its pImmutableSamplers pointer may be non-null but ignored
                'VkDescriptorSetLayoutBinding' :
                    '    const bool sampler_type = in_struct->descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER || in_struct->descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;\n'
                    '    if (descriptorCount && in_struct->pImmutableSamplers && sampler_type) {\n'
                    '        pImmutableSamplers = new VkSampler[descriptorCount];\n'
                    '        for (uint32_t i = 0; i < descriptorCount; ++i) {\n'
                    '            pImmutableSamplers[i] = in_struct->pImmutableSamplers[i];\n'
                    '        }\n'
                    '    }\n',
                'VkAccelerationStructureBuildGeometryInfoKHR':
                    '    if (geometryCount) {\n'
                    '        if ( in_struct->ppGeometries) {\n'
                    '            ppGeometries = new safe_VkAccelerationStructureGeometryKHR *[geometryCount];\n'
                    '            for (uint32_t i = 0; i < geometryCount; ++i) {\n'
                    '                ppGeometries[i] = new safe_VkAccelerationStructureGeometryKHR(in_struct->ppGeometries[i], is_host, &build_range_infos[i]);\n'
                    '            }\n'
                    '        } else {\n'
                    '            pGeometries = new safe_VkAccelerationStructureGeometryKHR[geometryCount];\n'
                    '            for (uint32_t i = 0; i < geometryCount; ++i) {\n'
                    '                (pGeometries)[i] = safe_VkAccelerationStructureGeometryKHR(&(in_struct->pGeometries)[i], is_host, &build_range_infos[i]);\n'
                    '            }\n'
                    '        }\n'
                    '    }\n',
                'VkAccelerationStructureTrianglesOpacityMicromapEXT':
                    '    if (usageCountsCount) {\n'
                    '        if ( in_struct->ppUsageCounts) {\n'
                    '            ppUsageCounts = new VkMicromapUsageEXT *[usageCountsCount];\n'
                    '            for (uint32_t i = 0; i < usageCountsCount; ++i) {\n'
                    '                memcpy ((void *)ppUsageCounts[i], (void *)in_struct->ppUsageCounts[i], sizeof(VkMicromapUsageEXT));'
                    '            }\n'
                    '        } else {\n'
                    '            pUsageCounts = new VkMicromapUsageEXT[usageCountsCount];\n'
                    '            memcpy ((void *)pUsageCounts, (void *)in_struct->pUsageCounts, sizeof(VkMicromapUsageEXT)*usageCountsCount);'
                    '        }\n'
                    '    }\n',
                'VkMicromapBuildInfoEXT':
                    '    if (usageCountsCount) {\n'
                    '        if ( in_struct->ppUsageCounts) {\n'
                    '            ppUsageCounts = new VkMicromapUsageEXT *[usageCountsCount];\n'
                    '            for (uint32_t i = 0; i < usageCountsCount; ++i) {\n'
                    '                memcpy ((void *)ppUsageCounts[i], (void *)in_struct->ppUsageCounts[i], sizeof(VkMicromapUsageEXT));'
                    '            }\n'
                    '        } else {\n'
                    '            pUsageCounts = new VkMicromapUsageEXT[usageCountsCount];\n'
                    '            memcpy ((void *)pUsageCounts, (void *)in_struct->pUsageCounts, sizeof(VkMicromapUsageEXT)*usageCountsCount);'
                    '        }\n'
                    '    }\n',
                'VkAccelerationStructureGeometryKHR':
                    '    if (is_host && geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {\n'
                    '        if (geometry.instances.arrayOfPointers) {\n'
                    '            size_t pp_array_size = build_range_info->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR*);\n'
                    '            size_t p_array_size = build_range_info->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR);\n'
                    '            size_t array_size = build_range_info->primitiveOffset + pp_array_size + p_array_size;\n'
                    '            uint8_t *allocation = new uint8_t[array_size];\n'
                    '            VkAccelerationStructureInstanceKHR **ppInstances = reinterpret_cast<VkAccelerationStructureInstanceKHR **>(allocation + build_range_info->primitiveOffset);\n'
                    '            VkAccelerationStructureInstanceKHR *pInstances = reinterpret_cast<VkAccelerationStructureInstanceKHR *>(allocation + build_range_info->primitiveOffset + pp_array_size);\n'
                    '            for (uint32_t i = 0; i < build_range_info->primitiveCount; ++i) {\n'
                    '                const uint8_t *byte_ptr = reinterpret_cast<const uint8_t *>(in_struct->geometry.instances.data.hostAddress);\n'
                    '                pInstances[i] = *(reinterpret_cast<VkAccelerationStructureInstanceKHR * const*>(byte_ptr + build_range_info->primitiveOffset)[i]);\n'
                    '                ppInstances[i] = &pInstances[i];\n'
                    '            }\n'
                    '            geometry.instances.data.hostAddress = allocation;\n'
                    '            as_geom_khr_host_alloc.insert(this, new ASGeomKHRExtraData(allocation, build_range_info->primitiveOffset, build_range_info->primitiveCount));\n'
                    '        } else {\n'
                    '            size_t array_size = build_range_info->primitiveOffset + build_range_info->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR);\n'
                    '            uint8_t *allocation = new uint8_t[array_size];\n'
                    '            memcpy(allocation, in_struct->geometry.instances.data.hostAddress, array_size);\n'
                    '            geometry.instances.data.hostAddress = allocation;\n'
                    '            as_geom_khr_host_alloc.insert(this, new ASGeomKHRExtraData(allocation, build_range_info->primitiveOffset, build_range_info->primitiveCount));\n'
                    '        }\n'
                    '    }\n',
                'VkMicromapBuildInfoEXT':
                    '   pNext = SafePnextCopy(in_struct->pNext);\n'
                    '   if (in_struct->pUsageCounts) {\n'
                    '       pUsageCounts = new VkMicromapUsageEXT[in_struct->usageCountsCount];\n'
                    '       memcpy ((void *)pUsageCounts, (void *)in_struct->pUsageCounts, sizeof(VkMicromapUsageEXT)*in_struct->usageCountsCount);\n'
                    '   }\n'
                    '   if (in_struct->ppUsageCounts) {\n'
                    '       VkMicromapUsageEXT** pointer_array  = new VkMicromapUsageEXT*[in_struct->usageCountsCount];\n'
                    '       for (uint32_t i = 0; i < in_struct->usageCountsCount; ++i) {\n'
                    '           pointer_array[i] = new VkMicromapUsageEXT(*in_struct->ppUsageCounts[i]);\n'
                    '       }\n'
                    '       ppUsageCounts = pointer_array;\n'
                    '   }\n',
                'VkAccelerationStructureTrianglesOpacityMicromapEXT':
                    '   pNext = SafePnextCopy(in_struct->pNext);\n'
                    '   if (in_struct->pUsageCounts) {\n'
                    '       pUsageCounts = new VkMicromapUsageEXT[in_struct->usageCountsCount];\n'
                    '       memcpy ((void *)pUsageCounts, (void *)in_struct->pUsageCounts, sizeof(VkMicromapUsageEXT)*in_struct->usageCountsCount);\n'
                    '   }\n'
                    '   if (in_struct->ppUsageCounts) {\n'
                    '       VkMicromapUsageEXT** pointer_array = new VkMicromapUsageEXT*[in_struct->usageCountsCount];\n'
                    '       for (uint32_t i = 0; i < in_struct->usageCountsCount; ++i) {\n'
                    '           pointer_array[i] = new VkMicromapUsageEXT(*in_struct->ppUsageCounts[i]);\n'
                    '       }\n'
                    '       ppUsageCounts = pointer_array;\n'
                    '   }\n',
                'VkDescriptorDataEXT' :
                    '    VkDescriptorType* pType = (VkDescriptorType*)&type_at_end[sizeof(VkDescriptorDataEXT)];\n'
                    '\n'
                    '    switch (type)\n'
                    '    {\n'
                    '        case VK_DESCRIPTOR_TYPE_MAX_ENUM:                   break;\n'
                    '        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:     break;\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:     break;\n'
                    '        case VK_DESCRIPTOR_TYPE_SAMPLER:                    pSampler              = new VkSampler(*in_struct->pSampler); break;\n'
                    '        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:     pCombinedImageSampler = new VkDescriptorImageInfo(*in_struct->pCombinedImageSampler); break;\n'
                    '        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:              pSampledImage         = in_struct->pSampledImage ? new VkDescriptorImageInfo(*in_struct->pSampledImage) : nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:              pStorageImage         = in_struct->pStorageImage ? new VkDescriptorImageInfo(*in_struct->pStorageImage) : nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:           pInputAttachmentImage = new VkDescriptorImageInfo(*in_struct->pInputAttachmentImage); break;\n'
                    '        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:       pUniformTexelBuffer   = in_struct->pUniformTexelBuffer ? new safe_VkDescriptorAddressInfoEXT(in_struct->pUniformTexelBuffer) : nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:       pStorageTexelBuffer   = in_struct->pStorageTexelBuffer ? new safe_VkDescriptorAddressInfoEXT(in_struct->pStorageTexelBuffer) : nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:             pUniformBuffer        = in_struct->pUniformBuffer ? new safe_VkDescriptorAddressInfoEXT(in_struct->pUniformBuffer) : nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:             pStorageBuffer        = in_struct->pStorageBuffer ? new safe_VkDescriptorAddressInfoEXT(in_struct->pStorageBuffer) : nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: accelerationStructure = in_struct->accelerationStructure; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:  accelerationStructure = in_struct->accelerationStructure; break;\n'
                    '        default:                                            break;\n'
                    '    }\n'
                    '\n'
                    '    *pType = type;\n',
            }

            custom_copy_txt = {
                # VkGraphicsPipelineCreateInfo is special case because it has custom construct parameters
                'VkGraphicsPipelineCreateInfo' :
                    '    pNext = SafePnextCopy(copy_src.pNext);\n'
                    '    const bool is_graphics_library = LvlFindInChain<VkGraphicsPipelineLibraryCreateInfoEXT>(copy_src.pNext);\n'
                    '    if (stageCount && copy_src.pStages) {\n'
                    '        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];\n'
                    '        for (uint32_t i = 0; i < stageCount; ++i) {\n'
                    '            pStages[i].initialize(&copy_src.pStages[i]);\n'
                    '        }\n'
                    '    }\n'
                    '    if (copy_src.pVertexInputState)\n'
                    '        pVertexInputState = new safe_VkPipelineVertexInputStateCreateInfo(*copy_src.pVertexInputState);\n'
                    '    else\n'
                    '        pVertexInputState = nullptr;\n'
                    '    if (copy_src.pInputAssemblyState)\n'
                    '        pInputAssemblyState = new safe_VkPipelineInputAssemblyStateCreateInfo(*copy_src.pInputAssemblyState);\n'
                    '    else\n'
                    '        pInputAssemblyState = nullptr;\n'
                    '    bool has_tessellation_stage = false;\n'
                    '    if (stageCount && pStages)\n'
                    '        for (uint32_t i = 0; i < stageCount && !has_tessellation_stage; ++i)\n'
                    '            if (pStages[i].stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT || pStages[i].stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)\n'
                    '                has_tessellation_stage = true;\n'
                    '    if (copy_src.pTessellationState && has_tessellation_stage)\n'
                    '        pTessellationState = new safe_VkPipelineTessellationStateCreateInfo(*copy_src.pTessellationState);\n'
                    '    else\n'
                    '        pTessellationState = nullptr; // original pTessellationState pointer ignored\n'
                    '    bool is_dynamic_has_rasterization = false;\n'
                    '    if (copy_src.pDynamicState && copy_src.pDynamicState->pDynamicStates) {\n'
                    '        for (uint32_t i = 0; i < copy_src.pDynamicState->dynamicStateCount && !is_dynamic_has_rasterization; ++i)\n'
                    '            if (copy_src.pDynamicState->pDynamicStates[i] == VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE_EXT)\n'
                    '                is_dynamic_has_rasterization = true;\n'
                    '    }\n'
                    '    const bool has_rasterization = copy_src.pRasterizationState ? (is_dynamic_has_rasterization || !copy_src.pRasterizationState->rasterizerDiscardEnable) : false;\n'
                    '    if (copy_src.pViewportState && (has_rasterization || is_graphics_library)) {\n'
                    '        pViewportState = new safe_VkPipelineViewportStateCreateInfo(*copy_src.pViewportState);\n'
                    '    } else\n'
                    '        pViewportState = nullptr; // original pViewportState pointer ignored\n'
                    '    if (copy_src.pRasterizationState)\n'
                    '        pRasterizationState = new safe_VkPipelineRasterizationStateCreateInfo(*copy_src.pRasterizationState);\n'
                    '    else\n'
                    '        pRasterizationState = nullptr;\n'
                    '    if (copy_src.pMultisampleState && (has_rasterization || is_graphics_library))\n'
                    '        pMultisampleState = new safe_VkPipelineMultisampleStateCreateInfo(*copy_src.pMultisampleState);\n'
                    '    else\n'
                    '        pMultisampleState = nullptr; // original pMultisampleState pointer ignored\n'
                    '    if (copy_src.pDepthStencilState && (has_rasterization || is_graphics_library))\n'
                    '        pDepthStencilState = new safe_VkPipelineDepthStencilStateCreateInfo(*copy_src.pDepthStencilState);\n'
                    '    else\n'
                    '        pDepthStencilState = nullptr; // original pDepthStencilState pointer ignored\n'
                    '    if (copy_src.pColorBlendState && (has_rasterization || is_graphics_library))\n'
                    '        pColorBlendState = new safe_VkPipelineColorBlendStateCreateInfo(*copy_src.pColorBlendState);\n'
                    '    else\n'
                    '        pColorBlendState = nullptr; // original pColorBlendState pointer ignored\n'
                    '    if (copy_src.pDynamicState)\n'
                    '        pDynamicState = new safe_VkPipelineDynamicStateCreateInfo(*copy_src.pDynamicState);\n'
                    '    else\n'
                    '        pDynamicState = nullptr;\n',
                 # VkPipelineViewportStateCreateInfo is special case because it has custom construct parameters
                'VkPipelineViewportStateCreateInfo' :
                    '    pNext = SafePnextCopy(copy_src.pNext);\n'
                    '    if (copy_src.pViewports) {\n'
                    '        pViewports = new VkViewport[copy_src.viewportCount];\n'
                    '        memcpy ((void *)pViewports, (void *)copy_src.pViewports, sizeof(VkViewport)*copy_src.viewportCount);\n'
                    '    }\n'
                    '    else\n'
                    '        pViewports = nullptr;\n'
                    '    if (copy_src.pScissors) {\n'
                    '        pScissors = new VkRect2D[copy_src.scissorCount];\n'
                    '        memcpy ((void *)pScissors, (void *)copy_src.pScissors, sizeof(VkRect2D)*copy_src.scissorCount);\n'
                    '    }\n'
                    '    else\n'
                    '        pScissors = nullptr;\n',
                'VkFramebufferCreateInfo' :
                    '    pNext = SafePnextCopy(copy_src.pNext);\n'
                    '    if (attachmentCount && copy_src.pAttachments && !(flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT)) {\n'
                    '        pAttachments = new VkImageView[attachmentCount];\n'
                    '        for (uint32_t i = 0; i < attachmentCount; ++i) {\n'
                    '            pAttachments[i] = copy_src.pAttachments[i];\n'
                    '        }\n'
                    '    }\n',
                'VkAccelerationStructureBuildGeometryInfoKHR':
                    '    if (geometryCount) {\n'
                    '        if ( copy_src.ppGeometries) {\n'
                    '            ppGeometries = new safe_VkAccelerationStructureGeometryKHR *[geometryCount];\n'
                    '            for (uint32_t i = 0; i < geometryCount; ++i) {\n'
                    '                ppGeometries[i] = new safe_VkAccelerationStructureGeometryKHR(*copy_src.ppGeometries[i]);\n'
                    '            }\n'
                    '        } else {\n'
                    '            pGeometries = new safe_VkAccelerationStructureGeometryKHR[geometryCount];\n'
                    '            for (uint32_t i = 0; i < geometryCount; ++i) {\n'
                    '                pGeometries[i] = safe_VkAccelerationStructureGeometryKHR(copy_src.pGeometries[i]);\n'
                    '            }\n'
                    '        }\n'
                    '    }\n',
                'VkAccelerationStructureGeometryKHR':
                    '    pNext = SafePnextCopy(copy_src.pNext);\n'
                    '    auto src_iter = as_geom_khr_host_alloc.find(&copy_src);\n'
                    '    if (src_iter != as_geom_khr_host_alloc.end()) {\n'
                    '        auto &src_alloc = src_iter->second;\n'
                    '        if (geometry.instances.arrayOfPointers) {\n'
                    '            size_t pp_array_size = src_alloc->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR*);\n'
                    '            size_t p_array_size = src_alloc->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR);\n'
                    '            size_t array_size = src_alloc->primitiveOffset + pp_array_size + p_array_size;\n'
                    '            uint8_t *allocation = new uint8_t[array_size];\n'
                    '            VkAccelerationStructureInstanceKHR **ppInstances = reinterpret_cast<VkAccelerationStructureInstanceKHR **>(allocation + src_alloc->primitiveOffset);\n'
                    '            VkAccelerationStructureInstanceKHR *pInstances = reinterpret_cast<VkAccelerationStructureInstanceKHR *>(allocation + src_alloc->primitiveOffset + pp_array_size);\n'
                    '            for (uint32_t i = 0; i < src_alloc->primitiveCount; ++i) {\n'
                    '                pInstances[i] = *(reinterpret_cast<VkAccelerationStructureInstanceKHR * const*>(src_alloc->ptr + src_alloc->primitiveOffset)[i]);\n'
                    '                ppInstances[i] = &pInstances[i];\n'
                    '            }\n'
                    '            geometry.instances.data.hostAddress = allocation;\n'
                    '            as_geom_khr_host_alloc.insert(this, new ASGeomKHRExtraData(allocation, src_alloc->primitiveOffset, src_alloc->primitiveCount));\n'
                    '        } else {\n'
                    '            size_t array_size = src_alloc->primitiveOffset + src_alloc->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR);\n'
                    '            uint8_t *allocation = new uint8_t[array_size];\n'
                    '            memcpy(allocation, src_alloc->ptr, array_size);\n'
                    '            geometry.instances.data.hostAddress = allocation;\n'
                    '            as_geom_khr_host_alloc.insert(this, new ASGeomKHRExtraData(allocation, src_alloc->primitiveOffset, src_alloc->primitiveCount));\n'
                    '        }\n'
                    '    }\n',
                'VkDescriptorDataEXT' :
                    '    VkDescriptorType* pType = (VkDescriptorType*)&type_at_end[sizeof(VkDescriptorDataEXT)];\n'
                    '    VkDescriptorType type = *(VkDescriptorType*)&copy_src.type_at_end[sizeof(VkDescriptorDataEXT)];\n'
                    '\n'
                    '    switch (type)\n'
                    '    {\n'
                    '        case VK_DESCRIPTOR_TYPE_MAX_ENUM:                   break;\n'
                    '        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:     break;\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:     break;\n'
                    '        case VK_DESCRIPTOR_TYPE_SAMPLER:                    pSampler              = new VkSampler(*copy_src.pSampler); break;\n'
                    '        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:     pCombinedImageSampler = new VkDescriptorImageInfo(*copy_src.pCombinedImageSampler); break;\n'
                    '        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:              pSampledImage         = new VkDescriptorImageInfo(*copy_src.pSampledImage); break;\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:              pStorageImage         = new VkDescriptorImageInfo(*copy_src.pStorageImage); break;\n'
                    '        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:           pInputAttachmentImage = new VkDescriptorImageInfo(*copy_src.pInputAttachmentImage); break;\n'
                    '        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:       pUniformTexelBuffer   = new safe_VkDescriptorAddressInfoEXT(*copy_src.pUniformTexelBuffer); break;\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:       pStorageTexelBuffer   = new safe_VkDescriptorAddressInfoEXT(*copy_src.pStorageTexelBuffer); break;\n'
                    '        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:             pUniformBuffer        = new safe_VkDescriptorAddressInfoEXT(*copy_src.pUniformBuffer); break;\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:             pStorageBuffer        = new safe_VkDescriptorAddressInfoEXT(*copy_src.pStorageBuffer); break;\n'
                    '        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: accelerationStructure = copy_src.accelerationStructure; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:  accelerationStructure = copy_src.accelerationStructure; break;\n'
                    '        default:                                            break;\n'
                    '    }\n'
                    '\n'
                    '    *pType = type;\n',
            }

            custom_destruct_txt = {
                'VkShaderModuleCreateInfo' :
                    '    if (pCode)\n'
                    '        delete[] reinterpret_cast<const uint8_t *>(pCode);\n',
                'VkAccelerationStructureBuildGeometryInfoKHR' :
                    '    if (ppGeometries) {\n'
                    '        for (uint32_t i = 0; i < geometryCount; ++i) {\n'
                    '             delete ppGeometries[i];\n'
                    '        }\n'
                    '        delete[] ppGeometries;\n'
                    '    } else if(pGeometries) {\n'
                    '        delete[] pGeometries;\n'
                    '    }\n',
                'VkAccelerationStructureGeometryKHR':
                    '    auto iter = as_geom_khr_host_alloc.pop(this);\n'
                    '    if (iter != as_geom_khr_host_alloc.end()) {\n'
                    '        delete iter->second;\n'
                    '    }\n',
                'VkMicromapBuildInfoEXT':
                    '    if (pUsageCounts)\n'
                    '        delete[] pUsageCounts;\n'
                    '    if (ppUsageCounts) {\n'
                    '        for (uint32_t i = 0; i < usageCountsCount; ++i) {\n'
                    '             delete ppUsageCounts[i];\n'
                    '        }\n'
                    '        delete[] ppUsageCounts;\n'
                    '    }\n'
                    '    if (pNext)\n'
                    '        FreePnextChain(pNext);\n',
                'VkAccelerationStructureTrianglesOpacityMicromapEXT':
                    '    if (pUsageCounts)\n'
                    '        delete[] pUsageCounts;\n'
                    '    if (ppUsageCounts) {\n'
                    '        for (uint32_t i = 0; i < usageCountsCount; ++i) {\n'
                    '             delete ppUsageCounts[i];\n'
                    '        }\n'
                    '        delete[] ppUsageCounts;\n'
                    '    }\n'
                    '    if (pNext)\n'
                    '        FreePnextChain(pNext);\n',
                'VkDescriptorDataEXT' :
                    '\n'
                    '    VkDescriptorType& thisType = *(VkDescriptorType*)&type_at_end[sizeof(VkDescriptorDataEXT)];\n'
                    '\n'
                    '    switch (thisType)\n'
                    '    {\n'
                    '        case VK_DESCRIPTOR_TYPE_MAX_ENUM:                   break;\n'
                    '        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:     break;\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:     break;\n'
                    '        case VK_DESCRIPTOR_TYPE_SAMPLER:                    delete pSampler;              pSampler              = nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:     delete pCombinedImageSampler; pCombinedImageSampler = nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:              delete pSampledImage;         pSampledImage         = nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:              delete pStorageImage;         pStorageImage         = nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:           delete pInputAttachmentImage; pInputAttachmentImage = nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:       delete pUniformTexelBuffer;   pUniformTexelBuffer   = nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:       delete pStorageTexelBuffer;   pStorageTexelBuffer   = nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:             delete pUniformBuffer;        pUniformBuffer        = nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:             delete pStorageBuffer;        pStorageBuffer        = nullptr; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: accelerationStructure = 0ull; break;\n'
                    '        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:  accelerationStructure = 0ull; break;\n'
                    '        default:                                            break;\n'
                    '    }\n'
                    '\n'
                    '    thisType = VK_DESCRIPTOR_TYPE_MAX_ENUM;\n',
            }
            copy_pnext = ''
            copy_strings = ''
            for member in item.members:
                m_type = member.type
                if member.name == 'pNext':
                    copy_pnext = '    pNext = SafePnextCopy(in_struct->pNext);\n'
                if member.type in self.structNames:
                    member_index = next((i for i, v in enumerate(self.structMembers) if v[0] == member.type), None)
                    if member_index is not None and self.NeedSafeStruct(self.structMembers[member_index]) == True:
                        m_type = 'safe_%s' % member.type
                if member.ispointer and 'safe_' not in m_type and self.TypeContainsObjectHandle(member.type, False) == False:
                    # Ptr types w/o a safe_struct, for non-null case need to allocate new ptr and copy data in
                    if m_type in ['void', 'char']:
                        if member.name != 'pNext':
                            if m_type == 'char':
                                # Create deep copies of strings
                                if member.len:
                                    copy_strings += '    char **tmp_%s = new char *[in_struct->%s];\n' % (member.name, member.len)
                                    copy_strings += '    for (uint32_t i = 0; i < %s; ++i) {\n' % member.len
                                    copy_strings += '        tmp_%s[i] = SafeStringCopy(in_struct->%s[i]);\n' % (member.name, member.name)
                                    copy_strings += '    }\n'
                                    copy_strings += '    %s = tmp_%s;\n' % (member.name, member.name)

                                    destruct_txt += '    if (%s) {\n' % member.name
                                    destruct_txt += '        for (uint32_t i = 0; i < %s; ++i) {\n' % member.len
                                    destruct_txt += '            delete [] %s[i];\n' % member.name
                                    destruct_txt += '        }\n'
                                    destruct_txt += '        delete [] %s;\n' % member.name
                                    destruct_txt += '    }\n'
                                else:
                                    copy_strings += '    %s = SafeStringCopy(in_struct->%s);\n' % (member.name, member.name)
                                    destruct_txt += '    if (%s) delete [] %s;\n' % (member.name, member.name)
                            else:
                                # For these exceptions just copy initial value over for now
                                init_list += '\n    %s(in_struct->%s),' % (member.name, member.name)
                                init_func_txt += '    %s = in_struct->%s;\n' % (member.name, member.name)
                        default_init_list += '\n    %s(nullptr),' % (member.name)
                    else:
                        default_init_list += '\n    %s(nullptr),' % (member.name)
                        init_list += '\n    %s(nullptr),' % (member.name)
                        if m_type in abstract_types:
                            construct_txt += '    %s = in_struct->%s;\n' % (member.name, member.name)
                        else:
                            init_func_txt += '    %s = nullptr;\n' % (member.name)
                            if not member.isstaticarray and (member.len is None or '/' in member.len):
                                construct_txt += '    if (in_struct->%s) {\n' % member.name
                                construct_txt += '        %s = new %s(*in_struct->%s);\n' % (member.name, m_type, member.name)
                                construct_txt += '    }\n'
                                destruct_txt += '    if (%s)\n' % member.name
                                destruct_txt += '        delete %s;\n' % member.name
                            else:
                                # Prepend struct members with struct name
                                decorated_length = member.len
                                for other_member in item.members:
                                    decorated_length = re.sub(r'\b({})\b'.format(other_member.name), r'in_struct->\1', decorated_length)
                                try:
                                    concurrent_clause = member_construct_conditions[member.name](item, member)
                                except:
                                    concurrent_clause = (f'in_struct->{member.name}', lambda x: '')
                                construct_txt += f'    if ({concurrent_clause[0]}) {{' + '\n'
                                construct_txt += '        %s = new %s[%s];\n' % (member.name, m_type, decorated_length)
                                construct_txt += '        memcpy ((void *)%s, (void *)in_struct->%s, sizeof(%s)*%s);\n' % (member.name, member.name, m_type, decorated_length)
                                construct_txt += concurrent_clause[1]('        ')
                                if len(concurrent_clause) > 2:
                                    construct_txt += '    } else {\n'
                                    construct_txt += concurrent_clause[2]('        ')
                                construct_txt += '    }\n'
                                destruct_txt += '    if (%s)\n' % member.name
                                destruct_txt += '        delete[] %s;\n' % member.name
                elif member.isstaticarray or member.len is not None:
                    if member.len is None:
                        # Extract length of static array by grabbing val between []
                        static_array_size = re.match(r"[^[]*\[([^]]*)\]", member.cdecl)
                        construct_txt += '    for (uint32_t i = 0; i < %s; ++i) {\n' % static_array_size.group(1)
                        construct_txt += '        %s[i] = in_struct->%s[i];\n' % (member.name, member.name)
                        construct_txt += '    }\n'
                    else:
                        # Init array ptr to NULL
                        default_init_list += '\n    %s(nullptr),' % member.name
                        init_list += '\n    %s(nullptr),' % member.name
                        init_func_txt += '    %s = nullptr;\n' % member.name
                        array_element = 'in_struct->%s[i]' % member.name
                        if member.type in self.structNames:
                            member_index = next((i for i, v in enumerate(self.structMembers) if v[0] == member.type), None)
                            if member_index is not None and self.NeedSafeStruct(self.structMembers[member_index]) == True:
                                array_element = '%s(&in_struct->safe_%s[i])' % (member.type, member.name)
                        construct_txt += '    if (%s && in_struct->%s) {\n' % (member.len, member.name)
                        construct_txt += '        %s = new %s[%s];\n' % (member.name, m_type, member.len)
                        destruct_txt += '    if (%s)\n' % member.name
                        destruct_txt += '        delete[] %s;\n' % member.name
                        construct_txt += '        for (uint32_t i = 0; i < %s; ++i) {\n' % (member.len)
                        if 'safe_' in m_type:
                            construct_txt += '            %s[i].initialize(&in_struct->%s[i]);\n' % (member.name, member.name)
                        else:
                            construct_txt += '            %s[i] = %s;\n' % (member.name, array_element)
                        construct_txt += '        }\n'
                        construct_txt += '    }\n'
                elif member.ispointer == True:
                    default_init_list += '\n    %s(nullptr),' % (member.name)
                    init_list += '\n    %s(nullptr),' % (member.name)
                    init_func_txt += '    %s = nullptr;\n' % (member.name)
                    construct_txt += '    if (in_struct->%s)\n' % member.name
                    construct_txt += '        %s = new %s(in_struct->%s);\n' % (member.name, m_type, member.name)
                    destruct_txt += '    if (%s)\n' % member.name
                    destruct_txt += '        delete %s;\n' % member.name
                elif 'safe_' in m_type and member.type == 'VkDescriptorDataEXT':
                    init_list += '\n    %s(&in_struct->%s, in_struct->type),' % (member.name, member.name)
                    init_func_txt += '    %s.initialize(&in_struct->%s, in_struct->type);\n' % (member.name, member.name)
                elif 'safe_' in m_type:
                    init_list += '\n    %s(&in_struct->%s),' % (member.name, member.name)
                    init_func_txt += '    %s.initialize(&in_struct->%s);\n' % (member.name, member.name)
                else:
                    try:
                        init_list += f'\n    {member_init_transforms[member.name](member)},'
                    except:
                        init_list += '\n    %s(in_struct->%s),' % (member.name, member.name)
                        init_func_txt += '    %s = in_struct->%s;\n' % (member.name, member.name)
                    if (self.structOrUnion[item.name] != 'union'):
                        if member.name == 'sType' and item.name in self.structTypes:
                            default_init_list += f'\n    {member.name}({self.structTypes[item.name].value}),'
                        else:
                            default_init_list += f'\n    {member.name}(),'
            if '' != init_list:
                init_list = init_list[:-1] # hack off final comma

            if item.name in custom_construct_txt:
                construct_txt = custom_construct_txt[item.name]

            construct_txt = copy_pnext + copy_strings + construct_txt

            if item.name in custom_destruct_txt:
                destruct_txt = custom_destruct_txt[item.name]

            if copy_pnext:
                destruct_txt += '    if (pNext)\n'
                destruct_txt += '        FreePnextChain(pNext);\n'

            if (self.structOrUnion[item.name] == 'union'):
                # Unions don't allow multiple members in the initialization list, so just call initialize
                safe_struct_body.append("\n%s::%s(const %s* in_struct%s)\n{\n%s}" % (ss_name, ss_name, item.name, self.custom_construct_params.get(item.name, ''), construct_txt))
                if (item.name == 'VkDescriptorDataEXT'):
                    default_init_list = ' type_at_end {0},'
            else:
                safe_struct_body.append("\n%s::%s(const %s* in_struct%s) :%s\n{\n%s}" % (ss_name, ss_name, item.name, self.custom_construct_params.get(item.name, ''), init_list, construct_txt))
            if '' != default_init_list:
                default_init_list = " :%s" % (default_init_list[:-1])
            default_init_body = '\n' + custom_defeault_construct_txt[item.name] if item.name in custom_defeault_construct_txt else ''
            safe_struct_body.append("\n%s::%s()%s\n{%s}" % (ss_name, ss_name, default_init_list, default_init_body))
            # Create slight variation of init and construct txt for copy constructor that takes a copy_src object reference vs. struct ptr
            copy_construct_init = init_func_txt.replace('in_struct->', 'copy_src.')
            if item.name == 'VkDescriptorGetInfoEXT':
                copy_construct_init = copy_construct_init.replace(', copy_src.type', '')
            copy_construct_txt = re.sub('(new \\w+)\\(in_struct->', '\\1(*copy_src.', construct_txt) # Pass object to copy constructors
            copy_construct_txt = copy_construct_txt.replace('in_struct->', 'copy_src.')              # Modify remaining struct refs for copy_src object
            if item.name in custom_copy_txt:
                copy_construct_txt = custom_copy_txt[item.name]
            copy_assign_txt = '    if (&copy_src == this) return *this;\n\n' + destruct_txt + '\n' + copy_construct_init + copy_construct_txt + '\n    return *this;'
            safe_struct_body.append("\n%s::%s(const %s& copy_src)\n{\n%s%s}" % (ss_name, ss_name, ss_name, copy_construct_init, copy_construct_txt)) # Copy constructor
            safe_struct_body.append("\n%s& %s::operator=(const %s& copy_src)\n{\n%s\n}" % (ss_name, ss_name, ss_name, copy_assign_txt)) # Copy assignment operator
            safe_struct_body.append("\n%s::~%s()\n{\n%s}" % (ss_name, ss_name, destruct_txt))
            safe_struct_body.append("\nvoid %s::initialize(const %s* in_struct%s)\n{\n%s%s%s}" % (ss_name, item.name, self.custom_construct_params.get(item.name, ''),
                                    destruct_txt, init_func_txt, construct_txt))
            # Copy initializer uses same txt as copy constructor but has a ptr and not a reference
            init_copy = copy_construct_init.replace('copy_src.', 'copy_src->')
            init_copy = re.sub(r'&copy_src(?!->)', 'copy_src', init_copy)           # Replace '&copy_src' with 'copy_src' unless it's followed by a dereference
            init_construct = copy_construct_txt.replace('copy_src.', 'copy_src->')
            init_construct = re.sub(r'&copy_src(?!->)', 'copy_src', init_construct) # Replace '&copy_src' with 'copy_src' unless it's followed by a dereference
            safe_struct_body.append("\nvoid %s::initialize(const %s* copy_src)\n{\n%s%s}" % (ss_name, ss_name, init_copy, init_construct))
            if item.ifdef_protect is not None:
                safe_struct_body.append("#endif // %s\n" % item.ifdef_protect)
        return "\n".join(safe_struct_body)
    #
    # Generate the type map
    def GenerateTypeMapHelperHeader(self):
        prefix = 'Lvl'
        typemap = prefix + 'TypeMap'
        idmap = prefix + 'STypeMap'
        type_member = 'Type'
        id_member = 'kSType'
        id_decl = 'static const VkStructureType '
        generic_header = 'VkBaseOutStructure'

        explanatory_comment = '\n'.join((
                '// These empty generic templates are specialized for each type with sType',
                '// members and for each sType -- providing a two way map between structure',
                '// types and sTypes'))

        empty_typemap = 'template <typename T> struct ' + typemap + ' {};'
        typemap_format  = 'template <> struct {template}<{typename}> {{\n'
        typemap_format += '    {id_decl}{id_member} = {id_value};\n'
        typemap_format += '}};\n'

        empty_idmap = 'template <VkStructureType id> struct ' + idmap + ' {};'
        idmap_format = ''.join((
            'template <> struct {template}<{id_value}> {{\n',
            '    typedef {typename} {typedef};\n',
            '}};\n'))

        # Define the utilities (here so any renaming stays consistent), if this grows large, refactor to a fixed .h file
        utilities_format = '\n'.join((
            '// Find an entry of the given type in the const pNext chain',
            'template <typename T> const T *{find_func}(const void *next) {{',
            '    const {header} *current = reinterpret_cast<const {header} *>(next);',
            '    const T *found = nullptr;',
            '    while (current) {{',
            '        if ({type_map}<T>::{id_member} == current->sType) {{',
            '            found = reinterpret_cast<const T*>(current);',
            '            current = nullptr;',
            '        }} else {{',
            '            current = current->pNext;',
            '        }}',
            '    }}',
            '    return found;',
            '}}',
            '',
            '// Find an entry of the given type in the pNext chain',
            'template <typename T> T *{find_mod_func}(void *next) {{',
            '    {header} *current = reinterpret_cast<{header} *>(next);',
            '    T *found = nullptr;',
            '    while (current) {{',
            '        if ({type_map}<T>::{id_member} == current->sType) {{',
            '            found = reinterpret_cast<T*>(current);',
            '            current = nullptr;',
            '        }} else {{',
            '            current = current->pNext;',
            '        }}',
            '    }}',
            '    return found;',
            '}}',
            '',
            '// Init the header of an sType struct with pNext and optional fields',
            'template <typename T, typename... StructFields>',
            'T {init_func}(void *p_next, StructFields... fields) {{',
            '    T out = {{{type_map}<T>::kSType, p_next, fields...}};',
            '    return out;',
            '}}',
            '// Init the header of an sType struct',
            'template <typename T>',
            'T {init_func}(void *p_next = nullptr) {{',
            '    T out = {{{type_map}<T>::kSType, p_next}};',
            '    return out;',
            '}}',
            ''))

        code = []

        # Generate header
        code.append('\n'.join((
            '#pragma once',
            '#include <vulkan/vulkan.h>\n',
            explanatory_comment, '',
            empty_idmap,
            empty_typemap, '')))

        # Generate the specializations for each type and stype
        for item in self.structMembers:
            typename = item.name
            info = self.structTypes.get(typename)
            if not info:
                continue

            if item.ifdef_protect is not None:
                code.append('#ifdef %s' % item.ifdef_protect)

            code.append('// Map type {} to id {}'.format(typename, info.value))
            code.append(typemap_format.format(template=typemap, typename=typename, id_value=info.value,
                id_decl=id_decl, id_member=id_member))
            code.append(idmap_format.format(template=idmap, typename=typename, id_value=info.value, typedef=type_member))

            if item.ifdef_protect is not None:
                code.append('#endif // %s' % item.ifdef_protect)

        # Generate Generate utilities for all types
        find_func = 'LvlFindInChain'
        find_mod_func = 'LvlFindModInChain'
        init_func = 'LvlInitStruct'
        code.append('\n'.join((
            utilities_format.format(id_member=id_member, type_map=typemap,
                header=generic_header, find_func=find_func,
                find_mod_func=find_mod_func, init_func=init_func), ''
            )))

        return "\n".join(code)

    #
    # Generate the type map
    def GenerateSyncHelperHeader(self):
        return sync_val_gen.GenSyncTypeHelper(self, False)

    def GenerateSyncHelperSource(self):
        return sync_val_gen.GenSyncTypeHelper(self, True)

    #
    # Create a helper file and return it as a string
    def OutputDestFile(self):
        if self.helper_file_type == 'enum_string_header':
            return self.GenerateEnumStringHelperHeader()
        elif self.helper_file_type == 'safe_struct_header':
            return self.GenerateSafeStructHelperHeader()
        elif self.helper_file_type == 'safe_struct_source':
            return self.GenerateSafeStructHelperSource()
        elif self.helper_file_type == 'object_types_header':
            return self.GenerateObjectTypesHelperHeader()
        elif self.helper_file_type == 'extension_helper_header':
            return self.GenerateExtensionHelperHeader()
        elif self.helper_file_type == 'typemap_helper_header':
            return self.GenerateTypeMapHelperHeader()
        elif self.helper_file_type == 'sync_helper_header':
            return self.GenerateSyncHelperHeader()
        elif self.helper_file_type == 'sync_helper_source':
            return self.GenerateSyncHelperSource()
        else:
            return 'Bad Helper File Generator Option %s' % self.helper_file_type

    # Check if attribute is "true"
    def getBoolAttribute(self, member, name):
        try: return member.attrib[name].lower() == 'true'
        except: return False
