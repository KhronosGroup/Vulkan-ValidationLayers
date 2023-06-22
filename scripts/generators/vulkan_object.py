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

@dataclass(frozen=True)
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

@dataclass(frozen=True)
class Version:
    """<feature> which represents a version"""
    name: str # VK_VERSION_1_0
    number: str # 1.0

@dataclass(frozen=True)
class Handle:
    """<type> which represents a dispatch handle"""
    name: str
    type: str    # ex. 'VK_OBJECT_TYPE_BUFFER'
    protect: str # ex. 'VK_USE_PLATFORM_ANDROID_KHR'
    # Only one will be True, the other is False
    instance: bool
    device: bool
    dispatchable: bool

@dataclass(frozen=True)
class CommandParam:
    """<command/param>"""
    name: str
    type: str
    alias: str
    externSync: bool
    optional: bool
    noAutoValidity: bool

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

@dataclass(frozen=True)
class Command:
    """<command>"""
    # Attributes of <command>
    name: str
    alias: str # Because commands are interfaces into layers/drivers, we need all command alias

    # This is the union of (Extension | Version | None) - but requires Python 3.9
    feature: object

    returnType: str # 'void', 'VkResult', etc

    api: List[str]            # ex. [ 'vulkan' ]
    tasks: List[str]          # ex. [ 'action', 'state', 'synchronization' ]
    queues: Queues            # zero == No Queues found
    successCodes: List[str]   # ex. [ 'VK_SUCCESS', 'VK_INCOMPLETE' ]
    errorCodes: List[str]     # ex. [ 'VK_ERROR_OUT_OF_HOST_MEMORY' ]

    # Shows support if command can be in a primary and/or secondary command buffer
    primary: bool
    secondary: bool

    renderPass: CommandScope
    videoCoding: CommandScope

    params: List[CommandParam] # <command/param>

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

@dataclass(frozen=True)
class EnumField:
    """<enum> of type enum"""
    name: str
    negative: bool # True if negative values are allowed (ex. VkResult)
    # some fields are enabled from 2 extensions (ex. VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR)
    extensions: List[str] # None if part of 1.0 core
    protect: str # ex. 'VK_ENABLE_BETA_EXTENSIONS'

@dataclass(frozen=True)
class Enum:
    """<enums> of type enum"""
    name: str
    bitWidth: int # 32 or 64
    protect: str  # ex. 'VK_ENABLE_BETA_EXTENSIONS'
    fields: List[EnumField]

@dataclass(frozen=True)
class Flag:
    """<enum> of type bitmask"""
    name: str
    value: int     # Value of flag
    multiBit: bool # if true, more than one bit is set (ex. VK_SHADER_STAGE_ALL_GRAPHICS)
    zero: bool     # if true, the value is zero (ex. VK_PIPELINE_STAGE_NONE)
    # some fields are enabled from 2 extensions (ex. VK_TOOL_PURPOSE_DEBUG_REPORTING_BIT_EXT)
    extensions: List[str] # None if part of 1.0 core
    protect: str   # ex. 'VK_ENABLE_BETA_EXTENSIONS'

@dataclass(frozen=True)
class Bitmask:
    """<enums> of type bitmask"""
    name: str     # ex. 'VkAccessFlagBits2'
    flagName: str # ex. 'VkAccessFlags2'
    bitWidth: int # 32 or 64
    protect: str  # ex. 'VK_ENABLE_BETA_EXTENSIONS'
    flags: List[Flag]

@dataclass(frozen=True)
class Member:
    """<member>"""
    name: str
    type: str
    externSync: bool
    optional: bool
    noAutoValidity: bool
    length: str
    limitType: str

    # C string of member, example:
    #   - const void* pNext
    #   - VkFormat format
    #   - VkStructureType sType
    cDeclaration: str

@dataclass(frozen=True)
class Struct:
    """<type category="struct">"""
    name: str
    structExtends: List[str]
    protect: str  # ex. 'VK_ENABLE_BETA_EXTENSIONS'
    sType: str # if 'members[0].type' != 'VkStructureType' will be None
    returnedOnly: bool
    allowDuplicate: bool
    members: List[Member]

@dataclass(frozen=True)
class Union:
    """<type category="union">"""
    name: str
    structExtends: List[str]
    protect: str  # ex. 'VK_ENABLE_BETA_EXTENSIONS'
    members: List[Member]

@dataclass(frozen=True)
class FormatComponent:
    """<format/component>"""
    type: str
    bits: str
    numericFormat: str
    planeIndex: int # None if no planeIndex in format

@dataclass(frozen=True)
class FormatPlane:
    """<format/plane>"""
    index: int
    widthDivisor: int
    heightDivisor: int
    compatible: str

@dataclass(frozen=True)
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

@dataclass(frozen=True)
class SyncSupport:
    """<syncsupport>"""
    queues: List[str]
    stage: List[str]

@dataclass(frozen=True)
class SyncEquivalent:
    """<syncequivalent>"""
    stage: List[str]
    access: List[str]

@dataclass(frozen=True)
class SyncStage:
    """<syncstage>"""
    name: str
    support: SyncSupport
    equivalent: SyncEquivalent

@dataclass(frozen=True)
class SyncAccess:
    """<syncaccess>"""
    name: str
    support: SyncSupport
    equivalent: SyncEquivalent

@dataclass(frozen=True)
class SyncPipelineStage:
    """<syncpipelinestage>"""
    order: str
    before: str
    after: str
    value: str

@dataclass(frozen=True)
class SyncPipeline:
    """<syncpipeline>"""
    name: str
    depends: List[str]
    stages: List[SyncPipelineStage]

@dataclass(frozen=True)
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

@dataclass(frozen=True)
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
    extensions: Dict[str, Extension] = field(default_factory=dict, init=False)
    versions:   Dict[str, Version]   = field(default_factory=dict, init=False)

    handles:  Dict[str, Handle]      = field(default_factory=dict, init=False)
    commands: Dict[str, Command]     = field(default_factory=dict, init=False)
    enums:    Dict[str, Enum]        = field(default_factory=dict, init=False)
    bitmasks: Dict[str, Bitmask]     = field(default_factory=dict, init=False)
    structs:  Dict[str, Struct]      = field(default_factory=dict, init=False)
    unions:   Dict[str, Union]       = field(default_factory=dict, init=False)
    formats:  Dict[str, Format]      = field(default_factory=dict, init=False)

    syncStage:    List[SyncStage]    = field(default_factory=list, init=False)
    syncAccess:   List[SyncAccess]   = field(default_factory=list, init=False)
    syncPipeline: List[SyncPipeline] = field(default_factory=list, init=False)

    spirv: List[Spirv]               = field(default_factory=list, init=False)

    # # ex. [ 'xlib' : 'VK_USE_PLATFORM_XLIB_KHR' ]
    platforms: Dict[str, str]        = field(default_factory=dict, init=False)
    # # List of all vendor Sufix names (ex. 'KHR', 'EXT', etc. )
    vendorTags: List[str]            = field(default_factory=list, init=False)
