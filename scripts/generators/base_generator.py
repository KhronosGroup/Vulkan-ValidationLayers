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

import sys
from generator import *
from common_codegen import *
from generators.vulkan_object import *
from typing import *

# Temporary workaround for vkconventions python2 compatibility
import abc; abc.ABC = abc.ABCMeta('ABC', (object,), {})
from vkconventions import VulkanConventions

# An API style convention object
vulkanConventions = VulkanConventions()

outputDirectory = '.'
def SetOutputDirectory(directory):
    global outputDirectory
    outputDirectory = directory

def SetTargetApiName(apiname):
    global targetApiName
    targetApiName = apiname

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
        self.vk = {}

        # Data from the generator options
        self.filename = None
        self.helper_file_type = ''
        self.valid_usage_path = ''
        self.lvt_file_type =  ''
        self.warnExtensions = []
        self.grammar = None

        self.currentFeature = {}

    def beginFile(self, genOpts):
        OutputGenerator.beginFile(self, genOpts)
        self.vk = VulkanObject()

        self.filename = genOpts.filename
        self.helper_file_type = genOpts.helper_file_type
        self.valid_usage_path = genOpts.valid_usage_path
        self.lvt_file_type = genOpts.lvt_file_type
        self.warnExtensions = genOpts.warnExtensions
        self.grammar = genOpts.grammar

        # Initialize members that require the tree
        self.handle_types = GetHandleTypes(self.registry.tree)

    def endFile(self):
        # This should not have to do anything but call into OutputGenerator
        OutputGenerator.endFile(self)

    #
    # Processing point at beginning of each extension definition
    def beginFeature(self, interface, emit):
        OutputGenerator.beginFeature(self, interface, emit)
        self.currentFeature = Feature(self, interface)
        self.vk.features.append(self.currentFeature)

        self.featureExtraProtect = GetFeatureProtect(interface)

    #
    # List the enum for the commands
    def genGroup(self, groupinfo, name, alias):
        if (name == 'VkDynamicState'):
            for elem in groupinfo.elem.findall('enum'):
                if elem.get('alias') is None:
                    self.vk.dynamic_states.append(elem.get('name'))

    #
    # All <command> from XML
    def genCmd(self, cmdinfo, name, alias):
        OutputGenerator.genCmd(self, cmdinfo, name, alias)

        # DataClass with typing.List are not mutable, so need to pass in any Lists
        params = []
        for param in cmdinfo.elem.findall('param'):
            params.append(CommandParam(param))

        self.vk.commands[name] = Command(self, cmdinfo, params)

        if name.startswith('vkCmd') and params[0].type == 'VkCommandBuffer':
            self.vk.recording_commands[name] = self.vk.commands[name]

    #
    # All <format> from XML
    def genFormat(self, format, formatinfo, alias):
        OutputGenerator.genFormat(self, format, formatinfo, alias)

        components = []
        planes = []
        for component in format.elem.iterfind('component'):
            components.append(component)
        for plane in format.elem.iterfind('plane'):
            planes.append(plane)

        name = format.elem.get('name')
        self.vk.formats[name] = Format(format.elem, components, planes)
