#!/usr/bin/python3 -i
#
# Copyright (c) 2023 Valve Corporation
# Copyright (c) 2023 LunarG, Inc.
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

from dataclasses import dataclass, field
from typing import *

# Helpers to keep things cleaner
def splitIfGet(elem, name):
    return elem.get(name).split(',') if elem.get(name) is not None else None

def textIfFind(elem, name):
    return elem.find(name).text if elem.find(name) is not None else None

# TODO - Just grab the <platfroms> on XML parsing to get this
platform_dict = {
    'android' : 'VK_USE_PLATFORM_ANDROID_KHR',
    'fuchsia' : 'VK_USE_PLATFORM_FUCHSIA',
    'ggp': 'VK_USE_PLATFORM_GGP',
    'ios' : 'VK_USE_PLATFORM_IOS_MVK',
    'macos' : 'VK_USE_PLATFORM_MACOS_MVK',
    'metal' : 'VK_USE_PLATFORM_METAL_EXT',
    'vi' : 'VK_USE_PLATFORM_VI_NN',
    'wayland' : 'VK_USE_PLATFORM_WAYLAND_KHR',
    'win32' : 'VK_USE_PLATFORM_WIN32_KHR',
    'xcb' : 'VK_USE_PLATFORM_XCB_KHR',
    'xlib' : 'VK_USE_PLATFORM_XLIB_KHR',
    'xlib_xrandr' : 'VK_USE_PLATFORM_XLIB_XRANDR_EXT',
    'provisional' : 'VK_ENABLE_BETA_EXTENSIONS',
    'directfb' : 'VK_USE_PLATFORM_DIRECTFB_EXT',
    'screen' : 'VK_USE_PLATFORM_SCREEN_QNX',
}

@dataclass
class Feature:
    """
    Describes the Version or Extensions
    Each other object (Command, Enum, Group, etc) can have a feature tied to it.
    This is used to describe how every is enabled

    If things are promoted it will be two seperate features - ex:
        vkCmdDrawIndirectCount == VK_VERSION_1_2
        vkCmdDrawIndirectCountKHR == VK_KHR_draw_indirect_count
    """
    name: str = None
    # Only one will be True, the other is Valse
    version: bool = False
    extension: bool = False

    specialUse: str = None
    promotedTo: str = None
    obsoletedBy: str = None
    deprecatedBy: str = None

    # name: str # Version (ex 'VK_VERSION_1_1') or Extension (ex. 'VK_KHR_surface')
    platform: str = None # ex. 'android'
    protect: str = None # ex. 'VK_USE_PLATFORM_ANDROID_KHR'

    extension_type: str = None # 'instance' or 'device'

    def __init__(self, generator, interface):
        self.name = interface.get('name')
        if self.name.startswith('VK_VERSION'):
            self.version = True
        else:
            self.extension = True

        self.specialUse = interface.get('specialuse')
        self.promotedTo = interface.get('promotedto')
        self.obsoletedBy = interface.get('obsoletedby')
        self.deprecatedBy = interface.get('deprecatedby')

        self.platform = interface.get('platform')
        if self.platform is not None:
            self.protect = platform_dict[self.platform]

        self.extension_type = interface.get('type')

    def IsPromotedCore(self):
        return self.version and self.name != 'VK_VERSION_1_0'

@dataclass
class CommandParam:
    """<command/param>"""
    name: str
    alias: str
    externsync: str
    optional: str
    selector: str
    noautovalidity: str
    type: str
    def __init__(self, param):
        self.name = param.find('name').text
        self.type = textIfFind(param, 'type')

        self.alias = param.get('alias')
        self.externsync = param.get('externsync')
        self.optional = param.get('optional')
        self.selector = param.get('selector')
        self.noautovalidity = param.get('noautovalidity')

@dataclass
class CommandProto:
    """<command/proto>"""
    name: str
    type: str
    def __init__(self, proto):
        self.name = proto.find('name').text
        self.type = textIfFind(proto, 'type')

@dataclass
class Command:
    """
    Class tracking all Vulkan Functions/Commands
    Everything from the <command> element in the XML
    """
    # Attributes of <command>
    alias: str
    api: str
    tasks: str
    queues: str
    successcodes: str
    errorcodes: str
    renderpass: str
    videocoding: str
    cmdbufferlevel: str
    # <command/proto>
    proto: CommandProto
    # <command/param>
    params: List[CommandParam] = field(default_factory=list)

    # C prototype string - ex:
    # VKAPI_ATTR VkResult VKAPI_CALL vkCreateInstance(
    #   const VkInstanceCreateInfo* pCreateInfo,
    #   const VkAllocationCallbacks* pAllocator,
    #   VkInstance* pInstance);'
    cPrototype: str = ""

    # function pointer typedef  - ex:
    # typedef VkResult (VKAPI_PTR *PFN_vkCreateInstance)
    #   (const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance);'
    cFunctionPointer: str = ""

    feature: Feature = None

    def __init__(self, generator, cmdinfo, params):
        attrib = cmdinfo.elem.attrib
        self.alias = attrib.get('alias')
        self.api = splitIfGet(attrib, 'api')
        self.tasks = splitIfGet(attrib, 'tasks')
        self.queues = splitIfGet(attrib, 'queues')
        self.successcodes = splitIfGet(attrib, 'successcodes')
        self.errorcodes = splitIfGet(attrib, 'errorcodes')
        self.renderpass = attrib.get('renderpass')
        self.videocoding = attrib.get('videocoding')
        self.cmdbufferlevel = splitIfGet(attrib, 'cmdbufferlevel')

        self.proto = CommandProto(cmdinfo.elem.find('proto'))

        self.params = params

        decls = generator.makeCDecls(cmdinfo.elem)
        self.cPrototype = decls[0]
        self.cFunctionPointer = decls[1]

        self.feature = generator.currentFeature

@dataclass
class Component:
    """
    Everything from the <component> element in the XML for Formats
    """
    type: str = None
    bits: str = None
    numericFormat: str = None
    planeIndex: int = 0

    def __init__(self, elem):
        self.type = elem.get('name')
        self.bits = elem.get('bits')
        self.numericFormat = elem.get('numericFormat')
        self.planeIndex = elem.get('planeIndex')


@dataclass
class Plane:
    """
    Everything from the <plane> element in the XML for Formats
    """
    index: int
    widthDivisor: int
    heightDivisor: int
    compatible: str = None

    def __init__(self, elem):
        self.index = int(elem.get('index'))
        self.widthDivisor = int(elem.get('widthDivisor'))
        self.heightDivisor = int(elem.get('heightDivisor'))
        self.compatible = elem.get('compatible')

@dataclass
class Format:
    """
    Class tracking all Vulkan Format
    Everything from the <format> element in the XML
    """
    className: str = None
    blockSize: int = 0
    texelsPerBlock: int = 0
    blockExtent: str = '1,1,1'
    packed: int = 0 # zero == not-packed
    chroma: str = None
    compressed: str = None

    # <format/component>
    components: List[Component] = field(default_factory=list)

    # <format/plane>
    planes: List[Plane] = field(default_factory=list)

    def __init__(self, elem, components, planes):
        # Make C++ name friendly class name
        self.className = elem.get('class')
        self.blockSize = int(elem.get('blockSize'))
        self.texelsPerBlock = int(elem.get('texelsPerBlock'))
        if elem.get('blockExtent'):
            self.blockExtent = elem.get('blockExtent')
        if elem.get('packed'):
            self.packed = int(elem.get('packed'))
        self.packed = elem.get('chroma')
        self.compressed = elem.get('compressed')
        self.components = components
        self.planes = planes

# This is the global Vulkan Object that holds all the information from parsing the XML
# This class is designed so all generator scripts can use this to obtain data
class VulkanObject():
    # < function name, Command object > hashamp
    commands = dict()
    # < format name, Format object > hashamp
    formats = dict()
    # Subset of 'commands' used for recording a command buffer (vkCmd*)
    recording_commands = dict()

    features = []

    dynamic_states = [] # VkDynamicState enum values

