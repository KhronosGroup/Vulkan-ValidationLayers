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

from generator import *
from common_codegen import *

# Temporary workaround for vkconventions python2 compatibility
import abc; abc.ABC = abc.ABCMeta('ABC', (object,), {})
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
    return elem.get(name) is None or elem.get(name) != "true"

# This Generator Option is used across all Validation Layer generators
# After years of use, it has shown that all the options are unified across each generator (file)
# as it is easier to modifiy things per-file that need the difference
class BaseGeneratorOptions(GeneratorOptions):
    def __init__(self,
                 filename = None,
                 helper_file_type = '',
                 valid_usage_path = '',
                 lvt_file_type = '',
                 mergeApiNames = None,
                 warnExtensions = [],
                 grammar = None):
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
        self.lvt_file_type =  lvt_file_type
        self.warnExtensions    = warnExtensions
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

        # Data from the generator options
        self.filename = None
        self.helper_file_type = ''
        self.valid_usage_path = ''
        self.lvt_file_type =  ''
        self.warnExtensions = []
        self.grammar = None

        # Needed because beginFeature()/endFeatures() wraps all
        # the genCmd() calls that are created with a given
        # Version or Extension
        self.currentFeature = None

    def write(self, data):
        write(data, file=self.outFile)

    def beginFile(self, genOpts):
        OutputGenerator.beginFile(self, genOpts)

        self.filename = genOpts.filename
        self.helper_file_type = genOpts.helper_file_type
        self.valid_usage_path = genOpts.valid_usage_path
        self.lvt_file_type = genOpts.lvt_file_type
        self.warnExtensions = genOpts.warnExtensions
        self.grammar = genOpts.grammar

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
            self.currentFeature = Extension(name, instance, device, depends, vendorTag,
                                            platform, self.featureExtraProtect,
                                            provisional, promotedto, deprecatedby,
                                            obsoletedby, specialuse)
            self.vk.extensions[name] = self.currentFeature
        else: # version
            number = interface.get('number')
            self.currentFeature = Version(name, number)
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
        queues = splitIfGet(attrib, 'queues')
        successcodes = splitIfGet(attrib, 'successcodes')
        errorcodes = splitIfGet(attrib, 'errorcodes')
        cmdbufferlevel = attrib.get('cmdbufferlevel')
        cmdBufferPrimary = cmdbufferlevel is not None and 'primary' in cmdbufferlevel
        cmdBufferSecondary = cmdbufferlevel is not None and 'secondary' in cmdbufferlevel
        renderpass = attrib.get('renderpass')
        videocoding = attrib.get('videocoding')

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
            paramOptional = boolGet(param, 'optional')
            paramNoautovalidity = boolGet(param, 'noautovalidity')
            params.append(CommandParam(paramName, paramType, paramAlias, paramExternsync,
                                       paramOptional, paramNoautovalidity))

        self.vk.commands[name] = Command(name, alias, self.currentFeature, returnType,
                                         api, tasks, queues, successcodes, errorcodes,
                                         cmdBufferPrimary, cmdBufferSecondary,
                                        renderpass, videocoding, params, cPrototype,
                                        cFunctionPointer)

    #
    # List the enum for the commands
    def genGroup(self, groupinfo, name, alias):
        enumElem = groupinfo.elem
        bitwidth = 32 if enumElem.get('bitwidth') is None else enumElem.get('bitwidth')
        fields = []
        if enumElem.get('type') == "enum":
            for elem in enumElem.findall('enum'):
                if elem.get('alias') is not None:
                    continue
                fieldName = elem.get('name')
                negative = elem.get('dir') != None
                extension = elem.get('extname')
                fields.append(Enum(fieldName, negative, extension))

            self.vk.enums[name] = Enums(name, bitwidth, fields)

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
                extension = elem.get('extname')
                fields.append(FlagBit(fieldName, fieldValue, fieldMultiBit, fieldZero, extension))

            self.vk.flags[name] = Flags(name, bitwidth, fields)

    def genType(self, typeinfo, typeName, alias):
        OutputGenerator.genType(self, typeinfo, typeName, alias)
        typeElem = typeinfo.elem
        category = typeElem.get('category')
        if (category == 'bitmask'):
            flagBitsName = typeElem.get('requires')
            flagName = typeElem.get('name')
            if flagBitsName is None:
                return
            self.vk.flagToBitsMap[flagName] = flagBitsName

        elif (category == 'struct' or category == 'union'):
            returnedOnly = boolGet(typeElem, 'returnedonly')
            allowDuplicate = boolGet(typeElem, 'allowduplicate')
            structExtends = splitIfGet(typeElem, 'structextends')

            membersElem = typeinfo.elem.findall('.//member')
            members = []
            for member in membersElem:
                for comment in member.findall('comment'):
                    member.remove(comment)

                name = textIfFind(member, 'name')
                type = textIfFind(member, 'type')
                sType = member.get('values')
                externSync = boolGet(member, 'externsync')
                optional = boolGet(member, 'optional')
                noautovalidity = boolGet(member, 'noautovalidity')
                length = member.get('altlen') if member.get('altlen') is not None else member.get('length')
                limittype = member.get('limittype')
                cdecl = self.makeCParamDecl(member, 0)

                members.append(Member(name, type, sType, externSync, optional,
                                      noautovalidity, length, limittype, cdecl))

            if category == 'union':
                self.vk.unions[typeName] = Union(typeName, structExtends, members)
            else:
                self.vk.structs[typeName] = Struct(typeName, structExtends, returnedOnly,
                                                   allowDuplicate, members)

        else:
            # not all categories are used
            #   'group'/'enum' are routed to genGroup instead
            #   'basetype'/`define`/'handle'/'include' are only for headers
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

        self.vk.formats[name] = Format(name, className, blockSize, texelsPerBlock,
                                       blockExtent, packed, chroma, compressed,
                                       components, planes)

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

