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
from enum import IntFlag, Enum, auto
# Use the List and Dict types because support for default dict/list is
# not supported until Python 3.9+
from typing import List, Dict

@dataclass
class Extension:
    """<extension>"""
    name: str
    nameEnum: str # ex 'VK_KHR_SURFACE_EXTENSION_NAME'

    # Only one will be True, the other is False
    instance: bool
    device: bool

    depends: str
    vendorTag: str # 'EXT', 'KHR', etc
    platform: str # ex. 'android'
    protect: str # ex. 'VK_USE_PLATFORM_ANDROID_KHR'
    provisional: bool
    promotedTo: str
    deprecatedBy: str
    obsoletedBy: str
    specialUse: List[str]

    # These are here to allow for easy reverse lookups
    # Quotes allow us to forward declare the dataclass
    commands: List['Command'] = field(default_factory=list, init=False)
    enums:    List['Enum']    = field(default_factory=list, init=False)
    bitmask:  List['Bitmask'] = field(default_factory=list, init=False)
    # Use the Enum name to see what fields are extended
    enumFields: Dict[str, List['EnumField']] = field(default_factory=dict, init=False)
    # Use the Bitmaks name to see what flags are extended
    flags: Dict[str, List['Flag']] = field(default_factory=dict, init=False)

@dataclass
class Version:
    """
    <feature> which represents a version
    This will NEVER be Version 1.0, since having 'no version' is same as being 1.0
    """
    name: str # VK_VERSION_1_1
    apiName: str # VK_API_VERSION_1_1
    number: str # 1.1

@dataclass
class Handle:
    """<type> which represents a dispatch handle"""
    name: str
    type: str    # ex. 'VK_OBJECT_TYPE_BUFFER'
    parent: 'Handle'
    protect: str # ex. 'VK_USE_PLATFORM_ANDROID_KHR'
    # Only one will be True, the other is False
    instance: bool
    device: bool
    dispatchable: bool

@dataclass
class CommandParam:
    """<command/param>"""
    name: str
    type: str
    alias: str

    pointer: bool # type contains a pointer
    noAutoValidity: bool
    length: str # 'len' from XML showing what is used to set the length of an pointer

    optional: bool
    optionalPointer: bool # if type contains a pointer, is the pointer value optional

    externSync: bool
    externSyncPointer: List[str] # if type contains a pointer, might only specific members modified

class Queues(IntFlag):
    TRANSFER = auto()       # VK_QUEUE_TRANSFER_BIT
    GRAPHICS = auto()       # VK_QUEUE_GRAPHICS_BIT
    COMPUTE = auto()        # VK_QUEUE_COMPUTE_BIT
    PROTECTED = auto()      # VK_QUEUE_PROTECTED_BIT
    SPARSE_BINDING = auto() # VK_QUEUE_SPARSE_BINDING_BIT
    OPTICAL_FLOW = auto()   # VK_QUEUE_OPTICAL_FLOW_BIT_NV
    DECODE = auto()         # VK_QUEUE_VIDEO_DECODE_BIT_KHR
    ENCODE = auto()         # VK_QUEUE_VIDEO_ENCODE_BIT_KHR

class CommandScope(Enum):
    NONE = auto()
    INSIDE = auto()
    OUTSIDE = auto()
    BOTH = auto()

@dataclass
class Command:
    """<command>"""
    # Attributes of <command>
    name: str
    alias: str # Because commands are interfaces into layers/drivers, we need all command alias

    extensions: List[Extension] # All extensions that enable the struct
    version: Version # None if Version 1.0
    protect: str # ex. 'VK_ENABLE_BETA_EXTENSIONS'

    returnType: str # 'void', 'VkResult', etc

    params: List[CommandParam] # Each parameter of the command

    # Only one will be True, the other is False
    instance: bool
    device: bool

    tasks: List[str]          # ex. [ 'action', 'state', 'synchronization' ]
    queues: Queues            # zero == No Queues found
    successCodes: List[str]   # ex. [ 'VK_SUCCESS', 'VK_INCOMPLETE' ]
    errorCodes: List[str]     # ex. [ 'VK_ERROR_OUT_OF_HOST_MEMORY' ]

    # Shows support if command can be in a primary and/or secondary command buffer
    primary: bool
    secondary: bool

    renderPass: CommandScope
    videoCoding: CommandScope

    implicitExternSyncParams: List[str]

    # C prototype string - ex:
    # VKAPI_ATTR VkResult VKAPI_CALL vkCreateInstance(
    #   const VkInstanceCreateInfo* pCreateInfo,
    #   const VkAllocationCallbacks* pAllocator,
    #   VkInstance* pInstance);'
    cPrototype: str

    # function pointer typedef  - ex:
    # typedef VkResult (VKAPI_PTR *PFN_vkCreateInstance)
    #   (const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance);'
    cFunctionPointer: str

@dataclass
class EnumField:
    """<enum> of type enum"""
    name: str
    negative: bool # True if negative values are allowed (ex. VkResult)
    protect: str # ex. 'VK_ENABLE_BETA_EXTENSIONS'

    # some fields are enabled from 2 extensions (ex. VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR)
    extensions: List[Extension] # None if part of 1.0 core

@dataclass
class Enum:
    """<enums> of type enum"""
    name: str
    bitWidth: int # 32 or 64
    protect: str  # ex. 'VK_ENABLE_BETA_EXTENSIONS'
    returnedOnly: bool
    fields: List[EnumField]

    extensions: List[Extension] # None if part of 1.0 core
    # Unique list of all extension that are involved in 'fields' (superset of 'extensions')
    fieldExtensions: List[Extension]

@dataclass
class Flag:
    """<enum> of type bitmask"""
    name: str
    value: int     # Value of flag
    multiBit: bool # if true, more than one bit is set (ex. VK_SHADER_STAGE_ALL_GRAPHICS)
    zero: bool     # if true, the value is zero (ex. VK_PIPELINE_STAGE_NONE)
    protect: str   # ex. 'VK_ENABLE_BETA_EXTENSIONS'

    # some fields are enabled from 2 extensions (ex. VK_TOOL_PURPOSE_DEBUG_REPORTING_BIT_EXT)
    extensions: List[str] # None if part of 1.0 core

@dataclass
class Bitmask:
    """<enums> of type bitmask"""
    name: str     # ex. 'VkAccessFlagBits2'
    flagName: str # ex. 'VkAccessFlags2'
    bitWidth: int # 32 or 64
    protect: str  # ex. 'VK_ENABLE_BETA_EXTENSIONS'
    flags: List[Flag]

    extensions: List[Extension] # None if part of 1.0 core
    # Unique list of all extension that are involved in 'flag' (superset of 'extensions')
    flagExtensions: List[Extension]

@dataclass
class Member:
    """<member>"""
    name: str
    type: str
    externSync: bool
    optional: bool
    optionalPointer: bool # if type contains a pointer, is the pointer value optional
    noAutoValidity: bool
    length: str
    limitType: str
    pointer: bool # type contains a pointer

    # C string of member, example:
    #   - const void* pNext
    #   - VkFormat format
    #   - VkStructureType sType
    cDeclaration: str

@dataclass
class Struct:
    """<type category="struct"> or <type category="union">"""
    name: str
    extensions: List[Extension] # All extensions that enable the struct
    version: Version # None if Version 1.0

    union: bool # Unions are just a subset of a Structs
    structExtends: List[str]
    protect: str  # ex. 'VK_ENABLE_BETA_EXTENSIONS'
    sType: str # if 'members[0].type' != 'VkStructureType' will be None
    returnedOnly: bool
    allowDuplicate: bool
    members: List[Member]

@dataclass
class FormatComponent:
    """<format/component>"""
    type: str # 'R', 'G', 'B', 'A', 'D', 'S', etc
    bits: str # will be an INT or 'compressed'
    numericFormat: str # 'UNORM', 'SINT', etc
    planeIndex: int # None if no planeIndex in format

@dataclass
class FormatPlane:
    """<format/plane>"""
    index: int
    widthDivisor: int
    heightDivisor: int
    compatible: str

@dataclass
class Format:
    """<format>"""
    name: str
    className: str
    blockSize: int
    texelsPerBlock: int
    blockExtent: List[str]
    packed: int # None == not-packed
    chroma: str
    compressed: str
    components: List[FormatComponent] # <format/component>
    planes: List[FormatPlane]  # <format/plane>
    spirvImageFormat: str

@dataclass
class SyncSupport:
    """<syncsupport>"""
    queues: List[str]
    stage: List[str]

@dataclass
class SyncEquivalent:
    """<syncequivalent>"""
    stage: List[str]
    access: List[str]

@dataclass
class SyncStage:
    """<syncstage>"""
    name: str
    support: SyncSupport
    equivalent: SyncEquivalent

@dataclass
class SyncAccess:
    """<syncaccess>"""
    name: str
    support: SyncSupport
    equivalent: SyncEquivalent

@dataclass
class SyncPipelineStage:
    """<syncpipelinestage>"""
    order: str
    before: str
    after: str
    value: str

@dataclass
class SyncPipeline:
    """<syncpipeline>"""
    name: str
    depends: List[str]
    stages: List[SyncPipelineStage]

@dataclass
class SpirvEnables:
    """What is needed to enable the SPIR-V element"""
    version: str
    extension: str
    struct: str
    feature: str
    requires: str
    property: str
    member: str
    value: str

@dataclass
class Spirv:
    """<spirvextension> and <spirvcapability>"""
    name: str
    # Only one will be True, the other is False
    extension: bool
    capability: bool
    enable: List[SpirvEnables]

# This is the global Vulkan Object that holds all the information from parsing the XML
# This class is designed so all generator scripts can use this to obtain data
@dataclass
class VulkanObject():
    headerVersion: int = 0 # value of VK_HEADER_VERSION

    extensions: Dict[str, Extension] = field(default_factory=dict, init=False)
    versions:   Dict[str, Version]   = field(default_factory=dict, init=False)

    handles:  Dict[str, Handle]      = field(default_factory=dict, init=False)
    commands: Dict[str, Command]     = field(default_factory=dict, init=False)
    enums:    Dict[str, Enum]        = field(default_factory=dict, init=False)
    bitmasks: Dict[str, Bitmask]     = field(default_factory=dict, init=False)
    structs:  Dict[str, Struct]      = field(default_factory=dict, init=False)
    formats:  Dict[str, Format]      = field(default_factory=dict, init=False)

    syncStage:    List[SyncStage]    = field(default_factory=list, init=False)
    syncAccess:   List[SyncAccess]   = field(default_factory=list, init=False)
    syncPipeline: List[SyncPipeline] = field(default_factory=list, init=False)

    spirv: List[Spirv]               = field(default_factory=list, init=False)

    # # ex. [ 'xlib' : 'VK_USE_PLATFORM_XLIB_KHR' ]
    platforms: Dict[str, str]        = field(default_factory=dict, init=False)
    # # List of all vendor Sufix names (ex. 'KHR', 'EXT', etc. )
    vendorTags: List[str]            = field(default_factory=list, init=False)
