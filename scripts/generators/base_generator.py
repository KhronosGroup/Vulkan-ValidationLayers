#!/usr/bin/python3 -i
#
# Copyright (c) 2023 Valve Corporation
# Copyright (c) 2023 LunarG, Inc.
# Copyright (c) 2023 RasterGrid Kft.
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

import os
import sys
import json
from generator import *
from common_codegen import *

from vkconventions import VulkanConventions
from generators.vulkan_object import *

# An API style convention object
vulkanConventions = VulkanConventions()

outputDirectory = '.'
def SetOutputDirectory(directory):
    global outputDirectory
    outputDirectory = directory

def SetTargetApiName(apiname):
    global targetApiName
    targetApiName = apiname


# Helpers to keep things cleaner
def splitIfGet(elem, name):
    return elem.get(name).split(',') if elem.get(name) is not None else None

def textIfFind(elem, name):
    return elem.find(name).text if elem.find(name) is not None else None

def intIfGet(elem, name):
    return None if elem.get(name) is None else int(elem.get(name), 0)

def boolGet(elem, name):
    return elem.get(name) is not None and elem.get(name) == "true"

#
# Walk the JSON-derived dict and find all "vuid" key values
def ExtractVUIDs(vuid_dict):
    if hasattr(vuid_dict, 'items'):
        for key, value in vuid_dict.items():
            if key == "vuid":
                yield value
            elif isinstance(value, dict):
                for vuid in ExtractVUIDs(value):
                    yield vuid
            elif isinstance (value, list):
                for listValue in value:
                    for vuid in ExtractVUIDs(listValue):
                        yield vuid

# This Generator Option is used across all Validation Layer generators
# After years of use, it has shown that all the options are unified across each generator (file)
# as it is easier to modifiy things per-file that need the difference
class BaseGeneratorOptions(GeneratorOptions):
    def __init__(self,
                 filename: str = None,
                 helper_file_type: str = None,
                 valid_usage_path: str = None,
                 lvt_file_type: str = None,
                 mergeApiNames: str = None,
                 warnExtensions: list = [],
                 grammar: str = None):
        GeneratorOptions.__init__(self,
                conventions = vulkanConventions,
                filename = filename,
                directory = outputDirectory,
                apiname = targetApiName,
                mergeApiNames = mergeApiNames,
                defaultExtensions = targetApiName,
                emitExtensions = '.*',
                emitSpirv = '.*',
                emitFormats = '.*')
        # These are used by the generator.py script
        self.apicall         = 'VKAPI_ATTR '
        self.apientry        = 'VKAPI_CALL '
        self.apientryp       = 'VKAPI_PTR *'
        self.alignFuncParam   = 48

        # These are custom fields for VVL
        # This allows passing data from lvl_genvk.py into each Generator (file)
        self.filename = filename
        self.helper_file_type = helper_file_type
        self.valid_usage_path = valid_usage_path
        self.lvt_file_type = lvt_file_type
        self.warnExtensions = warnExtensions
        self.grammar = grammar

#
# This object handles all the parsing from reg.py generator scripts in the Vulkan-Headers
# It will grab all the data and form it into a single object the rest of the generators will use
class BaseGenerator(OutputGenerator):
    def __init__(self,
                 errFile = sys.stderr,
                 warnFile = sys.stderr,
                 diagFile = sys.stdout):
        OutputGenerator.__init__(self, errFile, warnFile, diagFile)
        self.vk = VulkanObject()

        # reg.py has a `self.featureName` but this is nicer because
        # it will be either the Version or Extension object
        self.currentFeature = None

        # These are custom fields for the Validation Layers
        self.valid_vuids = set() # Set of all valid VUIDs

    def write(self, data):
        # Prevents having to check before writting
        if data is not None and data != "":
            write(data, file=self.outFile)


    def beginFile(self, genOpts):
        OutputGenerator.beginFile(self, genOpts)

        self.filename = genOpts.filename
        self.helper_file_type = genOpts.helper_file_type
        self.valid_usage_path = genOpts.valid_usage_path
        self.lvt_file_type = genOpts.lvt_file_type
        self.warnExtensions = genOpts.warnExtensions
        self.grammar = genOpts.grammar

        # Build a set of all vuid text strings found in validusage.json
        if self.valid_usage_path is not None:
            vu_json_filename = os.path.join(self.valid_usage_path, 'validusage.json')
            if not os.path.isfile(vu_json_filename):
                print(f'Error: Could not find, or error loading {vu_json_filename}')
                sys.exit(1)
            json_file = open(vu_json_filename, 'r', encoding='utf-8')
            vuid_dict = json.load(json_file)
            json_file.close()
            if len(vuid_dict) == 0:
                print(f'Error: Failed to load {vu_json_filename}')
                sys.exit(1)
            for json_vuid_string in ExtractVUIDs(vuid_dict):
                self.valid_vuids.add(json_vuid_string)
        # List of VUs that should exists, but have a spec bug
        for vuid in [
            # https://gitlab.khronos.org/vulkan/vulkan/-/issues/3548
            "VUID-VkImageDrmFormatModifierExplicitCreateInfoEXT-drmFormatModifierPlaneCount-arraylength",
            "VUID-VkImportMemoryHostPointerInfoEXT-pHostPointer-parameter"
        ]:
            self.valid_vuids.add(vuid)


        # Initialize members that require the tree
        self.handle_types = GetHandleTypes(self.registry.tree)

        # Not gen*() command to get these, so do it manually
        platforms = self.registry.tree.findall('platforms/platform')
        for platform in platforms:
            self.vk.platforms[platform.get('name')] = platform.get('protect')

        tags = self.registry.tree.findall('tags')
        for tag in tags:
            self.vk.vendorTags.append(tag.get('name'))

    # This function should be overloaded
    def generate(self):
        print("WARNING: This should not be called from the child class")
        return

    def endFile(self):
        # This is the point were reg.py has ran, everything is collected
        # We do some post processing now

        # Use structs and commands to find which things are returnedOnly
        for struct in [x for x in self.vk.structs.values() if not x.returnedOnly]:
            for enum in [self.vk.enums[x.type] for x in struct.members if x.type in self.vk.enums]:
                enum.returnedOnly = False
        for command in self.vk.commands.values():
            for enum in [self.vk.enums[x.type] for x in command.params if x.type in self.vk.enums]:
                enum.returnedOnly = False

        # All inherited generators should run from here
        self.generate()

        # This should not have to do anything but call into OutputGenerator
        OutputGenerator.endFile(self)

    #
    # Processing point at beginning of each extension definition
    def beginFeature(self, interface, emit):
        OutputGenerator.beginFeature(self, interface, emit)
        self.featureExtraProtect = GetFeatureProtect(interface)
        name = interface.get('name')

        if interface.tag == 'extension':
            instance = interface.get('type') == 'instance'
            device = not instance
            depends = interface.get('depends')
            vendorTag = interface.get('author')
            platform = interface.get('platform')
            provisional = boolGet(interface, 'provisional')
            promotedto = interface.get('promotedto')
            deprecatedby = interface.get('deprecatedby')
            obsoletedby = interface.get('obsoletedby')
            specialuse = splitIfGet(interface, 'specialuse')
            # Not sure if better way to get this info
            nameEnum = self.featureDictionary[name]['enumconstant'][None][None][1]

            self.currentFeature = Extension(name, nameEnum, instance, device, depends, vendorTag,
                                            platform, self.featureExtraProtect,
                                            provisional, promotedto, deprecatedby,
                                            obsoletedby, specialuse)
            self.vk.extensions[name] = self.currentFeature
        else: # version
            number = interface.get('number')
            if number != '1.0':
                apiName = name.replace('VK_', 'VK_API_')
                self.currentFeature = Version(name, apiName, number)
                self.vk.versions[name] = self.currentFeature

    def endFeature(self):
        OutputGenerator.endFeature(self)
        self.currentFeature = None

    #
    # All <command> from XML
    def genCmd(self, cmdinfo, name, alias):
        OutputGenerator.genCmd(self, cmdinfo, name, alias)
        attrib = cmdinfo.elem.attrib
        alias = attrib.get('alias')
        api = splitIfGet(attrib, 'api')
        tasks = splitIfGet(attrib, 'tasks')

        queues = 0
        queues_list = splitIfGet(attrib, 'queues')
        if queues_list is not None:
            queues |= Queues.TRANSFER if 'transfer' in queues_list else 0
            queues |= Queues.GRAPHICS if 'graphics' in queues_list else 0
            queues |= Queues.COMPUTE if 'compute' in queues_list else 0
            queues |= Queues.PROTECTED if 'protected' in queues_list else 0
            queues |= Queues.SPARSE_BINDING if 'sparse_binding' in queues_list else 0
            queues |= Queues.OPTICAL_FLOW if 'opticalflow' in queues_list else 0
            queues |= Queues.DECODE if 'decode' in queues_list else 0
            queues |= Queues.ENCODE if 'encode' in queues_list else 0

        successcodes = splitIfGet(attrib, 'successcodes')
        errorcodes = splitIfGet(attrib, 'errorcodes')
        cmdbufferlevel = attrib.get('cmdbufferlevel')
        primary = cmdbufferlevel is not None and 'primary' in cmdbufferlevel
        secondary = cmdbufferlevel is not None and 'secondary' in cmdbufferlevel

        renderpass = attrib.get('renderpass')
        renderpass = CommandScope.NONE if renderpass is None else getattr(CommandScope, renderpass.upper())
        videocoding = attrib.get('videocoding')
        videocoding = CommandScope.NONE if videocoding is None else getattr(CommandScope, videocoding.upper())

        protoElem = cmdinfo.elem.find('proto')
        returnType = textIfFind(protoElem, 'type')

        decls = self.makeCDecls(cmdinfo.elem)
        cPrototype = decls[0]
        cFunctionPointer = decls[1]

        params = []
        for param in cmdinfo.elem.findall('param'):
            paramName = param.find('name').text
            paramType = textIfFind(param, 'type')
            paramAlias = param.get('alias')
            paramExternsync = boolGet(param, 'externsync')
            paramNoautovalidity = boolGet(param, 'noautovalidity')
            paramLength = param.get('altlen') if param.get('altlen') is not None else param.get('len')

            # See Member::optional code for details of this
            optionalValues = splitIfGet(param, 'optional')
            paramOptional = optionalValues is not None and optionalValues[0].lower() == "true"
            paramOptionalPointer = optionalValues is not None and len(optionalValues) > 1 and optionalValues[1].lower() == "true"

            params.append(CommandParam(paramName, paramType, paramAlias, paramExternsync,
                                       paramOptional, paramOptionalPointer,
                                       paramNoautovalidity, paramLength))

        self.vk.commands[name] = Command(name, alias, self.currentFeature, returnType,
                                         api, tasks, queues, successcodes, errorcodes,
                                         primary, secondary, renderpass, videocoding,
                                         params, cPrototype, cFunctionPointer)

    #
    # List the enum for the commands
    # TODO - Seems empty groups like `VkDeviceDeviceMemoryReportCreateInfoEXT` don't show up in here
    def genGroup(self, groupinfo, groupName, alias):
        if alias is not None:
            return
        # There can be case where the Enum/Bitmask is in a protect, but the individual
        # fields also have their own protect
        groupProtect = self.currentFeature.protect if hasattr(self.currentFeature, 'protect') and self.currentFeature.protect is not None else None
        enumElem = groupinfo.elem
        bitwidth = 32 if enumElem.get('bitwidth') is None else enumElem.get('bitwidth')
        fields = []
        if enumElem.get('type') == "enum":
            for elem in enumElem.findall('enum'):
                if elem.get('alias') is not None:
                    continue
                fieldName = elem.get('name')
                negative = elem.get('dir') != None
                extensions = None if elem.get('extname') is None else [elem.get('extname')]
                protect = elem.get('protect')

                # Some values have multiple extensions (ex VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR)
                # genGroup() lists them twice
                updateField = next((x for x in fields if x.name == fieldName), None)
                if updateField is not None:
                    # Watch out for edge case like VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE which is a version and ext
                    if updateField.extensions is None:
                        updateField = [extensions]
                    else:
                        updateField.extensions.append(extensions)
                else:
                    fields.append(EnumField(fieldName, negative, extensions, protect))

            self.vk.enums[groupName] = Enum(groupName, bitwidth, groupProtect, True, fields)

        else: # "bitmask"
            for elem in enumElem.findall('enum'):
                if elem.get('alias') is not None:
                    continue
                fieldMultiBit = False
                fieldZero = False
                fieldValue = intIfGet(elem, 'bitpos')
                if fieldValue is None:
                    fieldValue = intIfGet(elem, 'value')
                    fieldMultiBit = fieldValue != 0
                    fieldZero = fieldValue == 0
                fieldName = elem.get('name')
                extensions = None if elem.get('extname') is None else [elem.get('extname')]
                protect = elem.get('protect')

                # Some values have multiple extensions (ex VK_TOOL_PURPOSE_DEBUG_REPORTING_BIT_EXT)
                # genGroup() lists them twice
                updateField = next((x for x in fields if x.name == fieldName), None)
                if updateField is not None:
                    updateField.extensions.append(extensions)
                else:
                    fields.append(Flag(fieldName, fieldValue, fieldMultiBit, fieldZero,
                                        extensions, protect))

            flagName = groupName.replace('FlagBits', 'Flags')
            self.vk.bitmasks[groupName] = Bitmask(groupName, flagName, bitwidth, groupProtect, fields)

    def genType(self, typeInfo, typeName, alias):
        OutputGenerator.genType(self, typeInfo, typeName, alias)
        typeElem = typeInfo.elem
        protect = self.currentFeature.protect if hasattr(self.currentFeature, 'protect') and self.currentFeature.protect is not None else None
        category = typeElem.get('category')
        if (category == 'struct' or category == 'union'):
            version = self.currentFeature if isinstance(self.currentFeature, Version) else None
            extension = [self.currentFeature] if isinstance(self.currentFeature, Extension) else []
            if alias is not None:
                struct = self.vk.structs[alias]
                # Some structs (ex VkAttachmentSampleCountInfoAMD) can have multiple alias pointing to same extension
                struct.extensions += extension if extension and extension[0] not in struct.extensions else []
                struct.version = version if struct.version is None else struct.version
                return

            union = category == 'union'

            returnedOnly = boolGet(typeElem, 'returnedonly')
            allowDuplicate = boolGet(typeElem, 'allowduplicate')
            structExtends = splitIfGet(typeElem, 'structextends')

            membersElem = typeInfo.elem.findall('.//member')
            members = []
            sType = None
            for member in membersElem:
                for comment in member.findall('comment'):
                    member.remove(comment)

                name = textIfFind(member, 'name')
                type = textIfFind(member, 'type')
                sType = member.get('values') if member.get('values') is not None else sType
                externSync = boolGet(member, 'externsync')
                noautovalidity = boolGet(member, 'noautovalidity')
                length = member.get('altlen') if member.get('altlen') is not None else member.get('len')
                limittype = member.get('limittype')
                cdecl = self.makeCParamDecl(member, 0)
                pointer = '*' in cdecl

                # if a pointer, this can be a something like:
                #     optional="true,false" for ppGeometries
                #     optional="false,true" for pPhysicalDeviceCount
                # the first is if the variable itself is optional
                # the second is the value of the pointer is optiona;
                optionalValues = splitIfGet(member, 'optional')
                optional = optionalValues is not None and optionalValues[0].lower() == "true"
                optionalPointer = optionalValues is not None and len(optionalValues) > 1 and optionalValues[1].lower() == "true"

                members.append(Member(name, type, externSync, optional, optionalPointer,
                                      noautovalidity, length, limittype, pointer, cdecl))

            self.vk.structs[typeName] = Struct(typeName, extension, version, union,
                                               structExtends, protect, sType, returnedOnly,
                                               allowDuplicate, members)

        elif category == 'handle':
            if alias is not None:
                return
            type = typeElem.get('objtypeenum')
            instance = typeElem.get('parent') == 'VkInstance'
            device = not instance
            dispatchable = self.handle_types[typeName] == 'VK_DEFINE_HANDLE'
            self.vk.handles[typeName] = Handle(typeName, type, protect, instance, device, dispatchable)

        elif category == 'define':
            if typeName == 'VK_HEADER_VERSION':
                self.vk.headerVersion = typeElem.find('name').tail.strip()

        else:
            # not all categories are used
            #   'group'/'enum'/'bitmask' are routed to genGroup instead
            #   'basetype'/'include' are only for headers
            #   'funcpointer` ingore until needed
            return

    def genSpirv(self, spirvinfo, spirvName, alias):
        OutputGenerator.genSpirv(self, spirvinfo, spirvName, alias)
        spirvElem = spirvinfo.elem
        name = spirvElem.get('name')
        extension = True if spirvElem.tag == 'spirvextension' else False
        capability = not extension

        enables = []
        for elem in spirvElem:
            version = elem.attrib.get('version')
            extensionEnable = elem.attrib.get('extension')
            struct = elem.attrib.get('struct')
            feature = elem.attrib.get('feature')
            requires = elem.attrib.get('requires')
            propertyEnable = elem.attrib.get('property')
            member = elem.attrib.get('member')
            value = elem.attrib.get('value')
            enables.append(SpirvEnables(version, extensionEnable, struct, feature,
                                        requires, propertyEnable, member, value))

        self.vk.spirv.append(Spirv(name, extension, capability, enables))

    def genFormat(self, format, formatinfo, alias):
        OutputGenerator.genFormat(self, format, formatinfo, alias)
        formatElem = format.elem
        name = formatElem.get('name')

        components = []
        for component in formatElem.iterfind('component'):
            type = component.get('name')
            bits = component.get('bits')
            numericFormat = component.get('numericFormat')
            planeIndex = intIfGet(component, 'planeIndex')
            components.append(FormatComponent(type, bits, numericFormat, planeIndex))

        planes = []
        for plane in formatElem.iterfind('plane'):
            index = int(plane.get('index'))
            widthDivisor = int(plane.get('widthDivisor'))
            heightDivisor = int(plane.get('heightDivisor'))
            compatible = plane.get('compatible')
            planes.append(FormatPlane(index, widthDivisor, heightDivisor, compatible))

        className = formatElem.get('class')
        blockSize = int(formatElem.get('blockSize'))
        texelsPerBlock = int(formatElem.get('texelsPerBlock'))
        blockExtent = splitIfGet(formatElem, 'blockExtent')
        packed = intIfGet(formatElem, 'packed')
        chroma = formatElem.get('chroma')
        compressed = formatElem.get('compressed')
        spirvImageFormat = formatElem.find('spirvimageformat')
        if spirvImageFormat is not None:
            spirvImageFormat = spirvImageFormat.get('name')

        self.vk.formats[name] = Format(name, className, blockSize, texelsPerBlock,
                                       blockExtent, packed, chroma, compressed,
                                       components, planes, spirvImageFormat)

    def genSyncStage(self, sync):
        OutputGenerator.genSyncStage(self, sync)
        syncElem = sync.elem
        name = syncElem.get('name')

        support = None
        supportElem = syncElem.find('syncsupport')
        if supportElem is not None:
            queues = splitIfGet(supportElem, 'queues')
            stage = splitIfGet(supportElem, 'stage')
            support = SyncSupport(queues, stage)

        equivalent = None
        equivalentElem = syncElem.find('syncequivalent')
        if equivalentElem is not None:
            stage = splitIfGet(equivalentElem, 'stage')
            access = splitIfGet(equivalentElem, 'access')
            support = SyncEquivalent(stage, access)

        self.vk.syncStage.append(SyncStage(name, support, equivalent))

    def genSyncAccess(self, sync):
        OutputGenerator.genSyncAccess(self, sync)
        syncElem = sync.elem
        name = syncElem.get('name')

        support = None
        supportElem = syncElem.find('syncsupport')
        if supportElem is not None:
            queues = splitIfGet(supportElem, 'queues')
            stage = splitIfGet(supportElem, 'stage')
            support = SyncSupport(queues, stage)

        equivalent = None
        equivalentElem = syncElem.find('syncequivalent')
        if equivalentElem is not None:
            stage = splitIfGet(equivalentElem, 'stage')
            access = splitIfGet(equivalentElem, 'access')
            support = SyncEquivalent(stage, access)

        self.vk.syncAccess.append(SyncAccess(name, support, equivalent))

    def genSyncPipeline(self, sync):
        OutputGenerator.genSyncPipeline(self, sync)
        syncElem = sync.elem
        name = syncElem.get('name')
        depends = splitIfGet(syncElem, 'depends')
        stages = []
        for stageElem in syncElem.findall('syncpipelinestage'):
            order = stageElem.get('order')
            before = stageElem.get('before')
            after = stageElem.get('after')
            value = stageElem.text
            stages.append(SyncPipelineStage(order, before, after, value))

        self.vk.syncPipeline.append(SyncPipeline(name, depends, stages))

