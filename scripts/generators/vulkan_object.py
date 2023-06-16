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
# Use the List and Dict types because support for default dict/list is
# not supported until Python 3.9+
from typing import List, Dict

@dataclass(frozen=True)
class Extension:
    """<extension>"""
    name: str
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
class CommandParam:
    """<command/param>"""
    name: str
    type: str
    alias: str
    externSync: bool
    optional: bool
    noAutoValidity: bool

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
    queues: List[str]         # ex. [ 'graphics', 'compute' ]
    successCodes: List[str]   # ex. [ 'VK_SUCCESS', 'VK_INCOMPLETE' ]
    errorCodes: List[str]     # ex. [ 'VK_ERROR_OUT_OF_HOST_MEMORY' ]

    # The command can be inside either a primary or secondary command buffer
    cmdBufferPrimary: bool
    cmdBufferSecondary: bool

    renderPass: str  # 'inside' | 'outside' | 'both'
    videoCoding: str # 'inside' | 'outside' | 'both'

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
class Enum:
    """<enum> of type enum"""
    name: str
    negative: bool # True if negative values are allowed (ex. VkResult)
    extension: str # None if part of 1.0 core

@dataclass(frozen=True)
class Enums:
    """<enums> of type enum"""
    name: str
    bitWidth: int # 32 or 64
    fields: List[Enum]

@dataclass(frozen=True)
class FlagBit:
    """<enum> of type bitmask"""
    name: str
    value: int # Value of flag
    multiBit: bool # if true, more than one bit is set (ex. VK_SHADER_STAGE_ALL_GRAPHICS)
    zero: bool # if true, the value is zero (ex. VK_PIPELINE_STAGE_NONE)
    extension: str # None if part of 1.0 core

@dataclass(frozen=True)
class Flags:
    """<enums> of type bitmask"""
    name: str
    bitWidth: int # 32 or 64
    fields: List[FlagBit]

@dataclass(frozen=True)
class Member:
    """<member>"""
    name: str
    type: str
    sType: str # if 'type' != 'VkStructureType' will be None
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
    returnedOnly: bool
    allowDuplicate: bool
    members: List[Member]

@dataclass(frozen=True)
class Union:
    """<type category="union">"""
    name: str
    structExtends: List[str]
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

    commands: Dict[str, Command]     = field(default_factory=dict, init=False)
    enums:    Dict[str, Enums]       = field(default_factory=dict, init=False)
    flags:    Dict[str, Flags]       = field(default_factory=dict, init=False)
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
    # # ex. [ 'VkAccessFlags' : 'VkAccessFlagBits' ]
    flagToBitsMap: Dict[str, str]    = field(default_factory=dict, init=False)
