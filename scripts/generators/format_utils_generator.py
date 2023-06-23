#!/usr/bin/python3 -i
#
# Copyright (c) 2022-2023 The Khronos Group Inc.
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

import sys,os
from generator import *
from common_codegen import *

#
# FormatUtilsOutputGenerator - Generate SPIR-V validation
# for SPIR-V extensions and capabilities
class FormatUtilsOutputGenerator(OutputGenerator):
    def __init__(self,
                 errFile = sys.stderr,
                 warnFile = sys.stderr,
                 diagFile = sys.stdout):
        OutputGenerator.__init__(self, errFile, warnFile, diagFile)
        self.headerFile = False # Header file generation flag
        self.sourceFile = False # Source file generation flag

        self.allFormats = dict()
        self.classes = dict()
        self.maxPlaneCount = 1
        self.maxComponentCount = 1
        self.numericFormats = dict()
        self.compressedFormats = dict()
        self.depthFormats = dict()
        self.stencilFormats = dict()
        self.packedFormats = []
        self.ycbcrFormats = dict()
        self.planarFormats = dict()
        self.formats64bit = []

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
        self.headerFile = (genOpts.filename == 'vk_format_utils.h')
        self.sourceFile = (genOpts.filename == 'vk_format_utils.cpp')

        # File Comment
        file_comment = '// *** THIS FILE IS GENERATED - DO NOT EDIT ***\n'
        file_comment += '// See {} for modifications\n'.format(os.path.basename(__file__))
        write(file_comment, file=self.outFile)
        # Copyright Statement
        copyright = ''
        copyright += '\n'
        copyright += '/***************************************************************************\n'
        copyright += ' *\n'
        copyright += ' * Copyright (c) 2015-2023 The Khronos Group Inc.\n'
        copyright += ' * Copyright (c) 2015-2023 Valve Corporation\n'
        copyright += ' * Copyright (c) 2015-2023 LunarG, Inc.\n'
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
        if self.sourceFile:
            write('#include "vk_format_utils.h"', file=self.outFile)
            write('#include "utils/vk_layer_utils.h"', file=self.outFile)
            write('#include <vector>', file=self.outFile)
        elif self.headerFile:
            write('#pragma once', file=self.outFile)
            write('#include <vulkan/vk_layer.h>', file=self.outFile)
            export = '''
#ifdef __cplusplus
extern "C" {
#endif'''
            write(export, file=self.outFile)

    #
    # Write generated file content to output file
    def endFile(self):
        self.defines()
        self.numericFunctions()
        self.compressedFunctions()
        self.depthStencilFunctions()
        self.packedFunctions()
        self.ycbcrFunctions()
        self.multiplaneFunctions()
        self.sizeFunctions()
        self.utilFunctions()
        if self.headerFile:
            export = '''
#ifdef __cplusplus
}
#endif'''
            write(export, file=self.outFile)
        # Finish processing in superclass
        OutputGenerator.endFile(self)
    #
    # Capture all Format elements from registry
    def genFormat(self, format, formatinfo, alias):
        OutputGenerator.genFormat(self, format, formatinfo, alias)
        elem = format.elem
        formatName = elem.get('name')

        # Make C++ name friendly class name
        classBit = elem.get('class')
        classBit = classBit.replace('-', '')
        classBit = classBit.replace(' ', '_')
        classBit = classBit.upper()
        if classBit[0].isdigit():
            classBit = '_' + classBit
        self.classes[elem.get('class')] = classBit

        self.allFormats[formatName] = {
            'class' : classBit,
            'blockSize' : int(elem.get('blockSize')),
            'texelsPerBlock' : int(elem.get('texelsPerBlock')),
            'blockExtent' : '1,1,1', # default
            'components' : []
        }

        if elem.get('blockExtent'):
            self.allFormats[formatName]['blockExtent'] = elem.get('blockExtent')

        if elem.get('packed'):
            self.packedFormats.append(formatName)

        if elem.get('chroma'):
            self.ycbcrFormats[formatName] = elem.get('chroma')

        if elem.get('compressed'):
            compressed = elem.get('compressed').replace(' ', '_')
            if compressed not in self.compressedFormats:
                # create list if first time
                self.compressedFormats[compressed] = []
            self.compressedFormats[compressed].append(formatName)

        self.maxComponentCount = max(self.maxComponentCount, sum(1 for _ in elem.iter('component')))
        # some formats (VK_FORMAT_D16_UNORM_S8_UINT) are not same numeric
        baseNumeric = elem.find('component').get('numericFormat')
        sameNumeric = True
        has64Bit = False
        for component in elem.iterfind('component'):
            componentBits = component.get('bits')
            if componentBits == 'compressed':
                componentBits = 'COMPRESSED_COMPONENT'
            elif componentBits == '64':
                has64Bit = True

            self.allFormats[formatName]['components'].append({
                'type' : component.get('name'),
                'bits' : componentBits,
                'numericFormat' : component.get('numericFormat')
            })

            if (baseNumeric != component.get('numericFormat')):
                sameNumeric = False

            # Some duplication with allFormats list,
            # but much easier then re-loop where the D/S component is later
            # Note: Make assumption only ever one D or S component in a fomrat
            if component.get('name') == 'D':
                self.depthFormats[formatName] = {
                    'bits' : componentBits,
                    'numericFormat' : component.get('numericFormat')
                }
            elif component.get('name') == 'S':
                self.stencilFormats[formatName] = {
                    'bits' : componentBits,
                    'numericFormat' : component.get('numericFormat')
                }

        if has64Bit:
            self.formats64bit.append(formatName)

        for plane in elem.iterfind('plane'):
            if formatName not in self.planarFormats:
                # create list if first time
                self.planarFormats[formatName] = []
            self.planarFormats[formatName].append({
                'index' : int(plane.get('index')),
                'widthDivisor' : int(plane.get('widthDivisor')),
                'heightDivisor' : int(plane.get('heightDivisor')),
                'compatible' : plane.get('compatible'),
            })

            index = int(plane.get('index'))
            self.maxPlaneCount = max(self.maxPlaneCount, (index + 1))

        if sameNumeric:
            if baseNumeric not in self.numericFormats:
                # create list if first time
                self.numericFormats[baseNumeric] = []
            self.numericFormats[baseNumeric].append(formatName)

    #
    # Create defines that are used either by other files (headerFile) or just internally (sourceFile)
    def defines(self):
        out = ['\n']
        if self.sourceFile:
            out.append('''
enum class COMPONENT_TYPE {
    NONE,
    R,
    G,
    B,
    A,
    D,
    S
};

// Compressed formats don't have a defined component size
const uint32_t COMPRESSED_COMPONENT = 0xFFFFFFFF;

struct COMPONENT_INFO {
    COMPONENT_TYPE type;
    uint32_t size; // bits

    COMPONENT_INFO() : type(COMPONENT_TYPE::NONE), size(0) {};
    COMPONENT_INFO(COMPONENT_TYPE type, uint32_t size) : type(type), size(size) {};
};

// Generic information for all formats
struct FORMAT_INFO {
    FORMAT_COMPATIBILITY_CLASS compatibility;
    uint32_t block_size; // bytes
    uint32_t texel_per_block;
    VkExtent3D block_extent;
    uint32_t component_count;
    COMPONENT_INFO components[FORMAT_MAX_COMPONENTS];
};

namespace std {
template <>
struct hash<VkFormat> {
    size_t operator()(VkFormat fmt) const noexcept {
        return hash<uint32_t>()(static_cast<uint32_t>(fmt));
    }
};
}

// clang-format off
static const vvl::unordered_map<VkFormat, FORMAT_INFO> kVkFormatTable = {
''')
            for f, info in sorted(self.allFormats.items()):
                out.append(f'    {{{f},\n')
                out.append('        {{FORMAT_COMPATIBILITY_CLASS::{}, {}, {}, {{{}}}, {},\n        {{'.format(
                    info['class'], info['blockSize'], info['texelsPerBlock'], info['blockExtent'].replace(',', ', '), len(info['components'])))
                for index, component in enumerate(info['components']):
                    out.append('{{COMPONENT_TYPE::{}, {}}}'.format(component['type'], component['bits']))
                    out.append(', ' if (index + 1 != len(info['components'])) else '')
                out.append('} }},\n')

            out.append('    {VK_FORMAT_UNDEFINED, {FORMAT_COMPATIBILITY_CLASS::NONE, 0, 0, {0, 0, 0}, 0, {}}}')
            out.append('};\n')
            out.append('// clang-format on\n')

            out.append('''
struct PER_PLANE_COMPATIBILITY {
    uint32_t width_divisor;
    uint32_t height_divisor;
    VkFormat compatible_format;

    // Need default otherwise if app tries to grab a plane that doesn't exist it will crash
    // if returned the value of 0 in IMAGE_STATE::GetEffectiveSubresourceExtent()
    // This is ok, because there are VUs later that will catch the bad app behaviour
    PER_PLANE_COMPATIBILITY() : width_divisor(1), height_divisor(1), compatible_format(VK_FORMAT_UNDEFINED) {}
    PER_PLANE_COMPATIBILITY(uint32_t width_divisor, uint32_t height_divisor, VkFormat compatible_format) :
        width_divisor(width_divisor), height_divisor(height_divisor), compatible_format(compatible_format) {}
};

// Information for multiplanar formats
struct MULTIPLANE_COMPATIBILITY {
    PER_PLANE_COMPATIBILITY per_plane[FORMAT_MAX_PLANES];
};

// Source: Vulkan spec Table 47. Plane Format Compatibility Table
// clang-format off
static const vvl::unordered_map<VkFormat, MULTIPLANE_COMPATIBILITY> kVkMultiplaneCompatibilityMap {
''')

            for f in sorted(self.planarFormats.keys()):
                out.append(f'    {{ {f}, {{{{\n')
                for index, plane in enumerate(self.planarFormats[f]):
                    if (index != plane['index']):
                        self.logMsg('error', 'index of planes were not added in order')
                    out.append('        {{ {}, {}, {} }}'.format(plane['widthDivisor'], plane['heightDivisor'], plane['compatible']))
                    out.append(',\n' if (index + 1 != len(self.planarFormats[f])) else '\n    }}},\n')
            out.append('};\n')
            out.append('// clang-format on\n')

        elif self.headerFile:
            out.append(f'static constexpr uint32_t FORMAT_MAX_PLANES = {self.maxPlaneCount};\n')
            out.append(f'static constexpr uint32_t FORMAT_MAX_COMPONENTS = {self.maxComponentCount};\n')
            out.append('\n')
            out.append('enum class FORMAT_NUMERICAL_TYPE {\n')
            out.append('    NONE = 0,\n')
            for index, numericFormat in enumerate(sorted(self.numericFormats.keys()), start=1):
                out.append(f'    {numericFormat}')
                out.append(',\n' if (index != len(self.numericFormats.keys())) else '\n')
            out.append('};\n')
            out.append('\n')
            count = 0
            out.append('enum class FORMAT_COMPATIBILITY_CLASS {\n')
            out.append('    NONE = 0,\n')
            for name, classBit in sorted(self.classes.items()):
                count += 1
                out.append(f'    {classBit}')
                out.append(',\n' if (count != len(self.classes)) else '\n')
            out.append('};\n')
        write("".join(out), file=self.outFile)
    #
    # Generate functions for numeric based functions
    def numericFunctions(self):
        out = []
        if self.headerFile:
            out.append('// Numeric\n')
            out.append('// Formats with more then one numeric type (VK_FORMAT_D16_UNORM_S8_UINT) will return false\n')
            for key in self.numericFormats.keys():
                out.append(f'bool FormatIs{key}(VkFormat format);\n')
            out.append('''
// Types from "Interpretation of Numeric Format" table (OpTypeFloat vs OpTypeInt)
static inline bool FormatIsSampledInt(VkFormat format) { return (FormatIsSINT(format) || FormatIsUINT(format)); }
static inline bool FormatIsSampledFloat(VkFormat format) {
    return (FormatIsUNORM(format)   || FormatIsSNORM(format)   ||
            FormatIsUSCALED(format) || FormatIsSSCALED(format) ||
            FormatIsUFLOAT(format)  || FormatIsSFLOAT(format)  ||
            FormatIsSRGB(format));
}
''')
        elif self.sourceFile:
            for key in self.numericFormats.keys():
                out.append(f'\n// Return true if all components in the format are an {key}\n')
                out.append(f'bool FormatIs{key}(VkFormat format) {{\n')
                out.append('    bool found = false;\n')
                out.append('    switch (format) {\n')
                for f in self.numericFormats[key]:
                    out.append(f'        case {f}:\n')
                out.append(self.commonBoolSwitch)

        write("".join(out), file=self.outFile)
    #
    # Generate functions for compressed based functions
    def compressedFunctions(self):
        out = []
        if self.headerFile:
            out.append('// Compressed\n')
            for key in sorted(self.compressedFormats.keys()):
                out.append(f'bool FormatIsCompressed_{key}(VkFormat format);\n')
            out.append('bool FormatIsCompressed(VkFormat format);\n')

        elif self.sourceFile:
            for key in sorted(self.compressedFormats.keys()):
                out.append(f'\n// Return true if the format is a {key} compressed image format\n')
                out.append(f'bool FormatIsCompressed_{key}(VkFormat format) {{\n')
                out.append('    bool found = false;\n')
                out.append('    switch (format) {\n')
                for f in sorted(self.compressedFormats[key]):
                    out.append(f'        case {f}:\n')
                out.append(self.commonBoolSwitch)

            out.append('\n// Return true if the format is any compressed image format\n')
            out.append('bool FormatIsCompressed(VkFormat format) {\n')
            out.append('    return (\n')
            for index, key in enumerate(sorted(self.compressedFormats.keys()), start=1):
                out.append(f'      FormatIsCompressed_{key}(format)')
                out.append(' ||\n' if (index != len(self.compressedFormats.keys())) else ');\n')
            out.append('}\n')

        write("".join(out), file=self.outFile)
    #
    # Generate functions for depth/stencil based functions
    def depthStencilFunctions(self):
        out = []
        if self.headerFile:
            out.append('''// Depth/Stencil
bool FormatIsDepthOrStencil(VkFormat format);
bool FormatIsDepthAndStencil(VkFormat format);
bool FormatIsDepthOnly(VkFormat format);
bool FormatIsStencilOnly(VkFormat format);
static inline bool FormatHasDepth(VkFormat format) { return (FormatIsDepthOnly(format) || FormatIsDepthAndStencil(format)); }
static inline bool FormatHasStencil(VkFormat format) { return (FormatIsStencilOnly(format) || FormatIsDepthAndStencil(format)); }
uint32_t FormatDepthSize(VkFormat format);
uint32_t FormatStencilSize(VkFormat format);
FORMAT_NUMERICAL_TYPE FormatDepthNumericalType(VkFormat format);
FORMAT_NUMERICAL_TYPE FormatStencilNumericalType(VkFormat format);
''')
        elif self.sourceFile:
            out.append('\n// Return true if format is a depth OR stencil format\n')
            out.append('bool FormatIsDepthOrStencil(VkFormat format) {\n')
            out.append('    bool found = false;\n')
            out.append('    switch (format) {\n')
            for f in sorted(self.allFormats.keys()):
                if f in self.depthFormats.keys() or f in self.stencilFormats.keys():
                    out.append(f'        case {f}:\n')
            out.append(self.commonBoolSwitch)

            out.append('\n// Return true if format is a depth AND stencil format\n')
            out.append('bool FormatIsDepthAndStencil(VkFormat format) {\n')
            out.append('    bool found = false;\n')
            out.append('    switch (format) {\n')
            for f in sorted(self.depthFormats.keys()):
                if f in self.stencilFormats.keys():
                    out.append(f'        case {f}:\n')
            out.append(self.commonBoolSwitch)

            out.append('\n// Return true if format is a depth ONLY format\n')
            out.append('bool FormatIsDepthOnly(VkFormat format) {\n')
            out.append('    bool found = false;\n')
            out.append('    switch (format) {\n')
            for f in sorted(self.depthFormats.keys()):
                if f not in self.stencilFormats.keys():
                    out.append(f'        case {f}:\n')
            out.append(self.commonBoolSwitch)

            out.append('\n// Return true if format is a stencil ONLY format\n')
            out.append('bool FormatIsStencilOnly(VkFormat format) {\n')
            out.append('    bool found = false;\n')
            out.append('    switch (format) {\n')
            for f in sorted(self.stencilFormats.keys()):
                if f not in self.depthFormats.keys():
                    out.append(f'        case {f}:\n')
            out.append(self.commonBoolSwitch)

            out.append('\n// Returns size of depth component in bits')
            out.append('\n// Returns zero if no depth component\n')
            out.append('uint32_t FormatDepthSize(VkFormat format) {\n')
            out.append('    switch (format) {\n')
            # sorts case statments together with same return value
            used = []
            for key, value in sorted(self.depthFormats.items()):
                if key not in used:
                    for key_dup, value_dup in sorted(self.depthFormats.items()):
                        if value_dup['bits'] == value['bits']:
                            used.append(key_dup)
                            out.append(f'           case {key_dup}:\n')
                    out.append('            return {};\n'.format(value['bits']))
            out.append('        default:\n')
            out.append('            return 0;\n')
            out.append('     }\n')
            out.append('}\n')

            out.append('\n// Returns size of stencil component in bits')
            out.append('\n// Returns zero if no stencil component\n')
            out.append('uint32_t FormatStencilSize(VkFormat format) {\n')
            out.append('    switch (format) {\n')
            # sorts case statments together with same return value
            used = []
            for key, value in sorted(self.stencilFormats.items()):
                if key not in used:
                    for key_dup, value_dup in sorted(self.stencilFormats.items()):
                        if value_dup['bits'] == value['bits']:
                            used.append(key_dup)
                            out.append(f'           case {key_dup}:\n')
                    out.append('            return {};\n'.format(value['bits']))
            out.append('        default:\n')
            out.append('            return 0;\n')
            out.append('     }\n')
            out.append('}\n')

            out.append('\n// Returns NONE if no depth component\n')
            out.append('FORMAT_NUMERICAL_TYPE FormatDepthNumericalType(VkFormat format) {\n')
            out.append('    switch (format) {\n')
            # sorts case statments together with same return value
            used = []
            for key, value in sorted(self.depthFormats.items()):
                if key not in used:
                    for key_dup, value_dup in sorted(self.depthFormats.items()):
                        if value_dup['numericFormat'] == value['numericFormat']:
                            used.append(key_dup)
                            out.append(f'           case {key_dup}:\n')
                    out.append('            return FORMAT_NUMERICAL_TYPE::{};\n'.format(value['numericFormat']))
            out.append('        default:\n')
            out.append('            return FORMAT_NUMERICAL_TYPE::NONE;\n')
            out.append('     }\n')
            out.append('}\n')

            out.append('\n// Returns NONE if no stencil component\n')
            out.append('FORMAT_NUMERICAL_TYPE FormatStencilNumericalType(VkFormat format) {\n')
            out.append('    switch (format) {\n')
            # sorts case statments together with same return value
            used = []
            for key, value in sorted(self.stencilFormats.items()):
                if key not in used:
                    for key_dup, value_dup in sorted(self.stencilFormats.items()):
                        if value_dup['numericFormat'] == value['numericFormat']:
                            used.append(key_dup)
                            out.append(f'           case {key_dup}:\n')
                    out.append('            return FORMAT_NUMERICAL_TYPE::{};\n'.format(value['numericFormat']))
            out.append('        default:\n')
            out.append('            return FORMAT_NUMERICAL_TYPE::NONE;\n')
            out.append('     }\n')
            out.append('}\n')

        write("".join(out), file=self.outFile)
    #
    # Generate functions for packed based functions
    def packedFunctions(self):
        out = []
        if self.headerFile:
            out.append('''// Packed
bool FormatIsPacked(VkFormat format);
''')
        elif self.sourceFile:
            out.append('\n// Return true if format is a packed format\n')
            out.append('bool FormatIsPacked(VkFormat format) {\n')
            out.append('    bool found = false;\n')
            out.append('    switch (format) {\n')
            for f in self.packedFormats:
                out.append(f'        case {f}:\n')
            out.append(self.commonBoolSwitch)

        write("".join(out), file=self.outFile)
    #
    # Generate functions for YCbCr based functions
    def ycbcrFunctions(self):
        out = []
        if self.headerFile:
            out.append('''// YCbCr
bool FormatRequiresYcbcrConversion(VkFormat format);
bool FormatIsXChromaSubsampled(VkFormat format);
bool FormatIsYChromaSubsampled(VkFormat format);
''')
        elif self.sourceFile:
            out.append('\n// Return true if format requires sampler YCBCR conversion\n')
            out.append('// for VK_IMAGE_ASPECT_COLOR_BIT image views\n')
            out.append('// Table found in spec\n')
            out.append('bool FormatRequiresYcbcrConversion(VkFormat format) {\n')
            out.append('    bool found = false;\n')
            out.append('    switch (format) {\n')
            for f in sorted(self.ycbcrFormats.keys()):
                out.append(f'        case {f}:\n')
            out.append(self.commonBoolSwitch)

            out.append('\nbool FormatIsXChromaSubsampled(VkFormat format) {\n')
            out.append('    bool found = false;\n')
            out.append('    switch (format) {\n')
            for f in sorted(self.ycbcrFormats.keys()):
                if self.ycbcrFormats[f] == '420' or self.ycbcrFormats[f] == '422':
                    out.append(f'        case {f}:\n')
            out.append(self.commonBoolSwitch)

            out.append('\nbool FormatIsYChromaSubsampled(VkFormat format) {\n')
            out.append('    bool found = false;\n')
            out.append('    switch (format) {\n')
            for f in sorted(self.ycbcrFormats.keys()):
                if self.ycbcrFormats[f] == '420':
                    out.append(f'        case {f}:\n')
            out.append(self.commonBoolSwitch)

        write("".join(out), file=self.outFile)
    #
    # Generate functions for Multiplane based functions
    def multiplaneFunctions(self):
        out = []
        if self.headerFile:
            out.append('// Multiplane\n')
            out.append('// Single-plane "_422" formats are treated as 2x1 compressed (for copies)\n')
            out.append('\nconstexpr bool FormatIsSinglePlane_422(VkFormat format) {\n')
            out.append('    bool found = false;\n')
            out.append('    switch (format) {\n')
            for f in sorted(self.ycbcrFormats.keys()):
                if self.ycbcrFormats[f] == '422' and f not in self.planarFormats.keys():
                    out.append(f'        case {f}:\n')
            out.append(self.commonBoolSwitch)

            out.append('\n// Returns number of planes in format (which is 1 by default)\n')
            out.append('constexpr uint32_t FormatPlaneCount(VkFormat format) {\n')
            out.append('    switch (format) {\n')
            for i in range(2, self.maxPlaneCount + 1):
                for f in sorted(self.planarFormats.keys()):
                    if len(self.planarFormats[f]) == i:
                        out.append(f'        case {f}:\n')
                out.append(f'            return {i};\n')
            out.append('        default:\n')
            out.append('            return 1;\n')
            out.append('     }\n')
            out.append('}\n')
            out.append('''
constexpr bool FormatIsMultiplane(VkFormat format) { return ((FormatPlaneCount(format)) > 1u); }
VkFormat FindMultiplaneCompatibleFormat(VkFormat mp_fmt, VkImageAspectFlags plane_aspect);
VkExtent2D FindMultiplaneExtentDivisors(VkFormat mp_fmt, VkImageAspectFlags plane_aspect);
''')

        elif self.sourceFile:
            out.append('''
// Will return VK_FORMAT_UNDEFINED if given a plane aspect that doesn't exist for the format
VkFormat FindMultiplaneCompatibleFormat(VkFormat mp_fmt, VkImageAspectFlags plane_aspect) {
    const uint32_t plane_idx = GetPlaneIndex(plane_aspect);
    auto it = kVkMultiplaneCompatibilityMap.find(mp_fmt);
    if ((it == kVkMultiplaneCompatibilityMap.end()) || (plane_idx >= FORMAT_MAX_PLANES)) {
        return VK_FORMAT_UNDEFINED;
    }

    return it->second.per_plane[plane_idx].compatible_format;
}

// Will return {1, 1} if given a plane aspect that doesn't exist for the format
VkExtent2D FindMultiplaneExtentDivisors(VkFormat mp_fmt, VkImageAspectFlags plane_aspect) {
    VkExtent2D divisors = {1, 1};
    const uint32_t plane_idx = GetPlaneIndex(plane_aspect);
    auto it = kVkMultiplaneCompatibilityMap.find(mp_fmt);
    if ((it == kVkMultiplaneCompatibilityMap.end()) || (plane_idx >= FORMAT_MAX_PLANES)) {
        return divisors;
    }

    divisors.width = it->second.per_plane[plane_idx].width_divisor;
    divisors.height = it->second.per_plane[plane_idx].height_divisor;
    return divisors;
}
''')
        write("".join(out), file=self.outFile)
    #
    # Generate functions for size based functions
    def sizeFunctions(self):
        out = []
        if self.headerFile:
            out.append('''// Size
uint32_t FormatComponentCount(VkFormat format);
VkExtent3D FormatTexelBlockExtent(VkFormat format);
FORMAT_COMPATIBILITY_CLASS FormatCompatibilityClass(VkFormat format);
bool FormatElementIsTexel(VkFormat format);
uint32_t FormatElementSize(VkFormat format, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);
double FormatTexelSize(VkFormat format, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);
''')
             # Could loop the components, but faster to just list these
            out.append('''// True if Format contains a 64-bit component\n
constexpr bool FormatIs64bit(VkFormat format) {
    bool found = false;
    switch (format) {
''')
            for f in sorted(self.formats64bit):
                out.append(f'        case {f}:\n')
            out.append(self.commonBoolSwitch)
            out.append('''
// Components
bool FormatHasComponentSize(VkFormat format, uint32_t size);
bool FormatHasRed(VkFormat format);
bool FormatHasGreen(VkFormat format);
bool FormatHasBlue(VkFormat format);
bool FormatHasAlpha(VkFormat format);
bool FormatsSameComponentBits(VkFormat format_a, VkFormat format_b);
''')
        elif self.sourceFile:
            out.append('''
uint32_t FormatComponentCount(VkFormat format) {
    auto format_info = kVkFormatTable.find(format);
    if (format_info != kVkFormatTable.end()) {
        return format_info->second.component_count;
    }
    return 0;
}

VkExtent3D FormatTexelBlockExtent(VkFormat format) {
    auto format_info = kVkFormatTable.find(format);
    if (format_info != kVkFormatTable.end()) {
        return format_info->second.block_extent;
    }
    return {1, 1, 1};
}

FORMAT_COMPATIBILITY_CLASS FormatCompatibilityClass(VkFormat format) {
    auto format_info = kVkFormatTable.find(format);
    if (format_info != kVkFormatTable.end()) {
        return format_info->second.compatibility;
    }
    return FORMAT_COMPATIBILITY_CLASS::NONE;
}

// Return true if format is 'normal', with one texel per format element
bool FormatElementIsTexel(VkFormat format) {
    if (FormatIsPacked(format) || FormatIsCompressed(format) || FormatIsSinglePlane_422(format) || FormatIsMultiplane(format)) {
        return false;
    } else {
        return true;
    }
}

// Return size, in bytes, of one element of the specified format
// For uncompressed this is one texel, for compressed it is one block
uint32_t FormatElementSize(VkFormat format, VkImageAspectFlags aspectMask) {
    // Depth/Stencil aspect have separate helper functions
    if (aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) {
        return FormatStencilSize(format) / 8;
    } else if (aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) {
        return FormatDepthSize(format) / 8;
    } else if (FormatIsMultiplane(format)) {
        // Element of entire multiplane format is not useful,
        // Want to get just a single plane as the lookup format
        format = FindMultiplaneCompatibleFormat(format, aspectMask);
    }

    auto item = kVkFormatTable.find(format);
    if (item != kVkFormatTable.end()) {
        return item->second.block_size;
    }
    return 0;
}

// Return the size in bytes of one texel of given foramt
// For compressed or multi-plane, this may be a fractional number
double FormatTexelSize(VkFormat format, VkImageAspectFlags aspectMask) {
    double texel_size = static_cast<double>(FormatElementSize(format, aspectMask));
    VkExtent3D block_extent = FormatTexelBlockExtent(format);
    uint32_t texels_per_block = block_extent.width * block_extent.height * block_extent.depth;
    if (1 < texels_per_block) {
        texel_size /= static_cast<double>(texels_per_block);
    }
    return texel_size;
}

bool FormatHasComponentSize(VkFormat format, uint32_t size) {
    auto item = kVkFormatTable.find(format);
    if (item == kVkFormatTable.end()) {
        return false;
    }
    const COMPONENT_INFO* begin = item->second.components;
    const COMPONENT_INFO* end = item->second.components + FORMAT_MAX_COMPONENTS;
    return std::find_if(begin, end, [size](const COMPONENT_INFO& info) { return info.size == size; }) != end;
}

static bool FormatHasComponentType(VkFormat format, COMPONENT_TYPE component) {
    auto item = kVkFormatTable.find(format);
    if (item == kVkFormatTable.end()) {
        return false;
    }
    const COMPONENT_INFO* begin = item->second.components;
    const COMPONENT_INFO* end = item->second.components + FORMAT_MAX_COMPONENTS;
    return std::find_if(begin, end, [component](const COMPONENT_INFO& info) { return info.type == component; }) != end;
}

bool FormatHasRed(VkFormat format) {
    return FormatHasComponentType(format, COMPONENT_TYPE::R);
}

bool FormatHasGreen(VkFormat format) {
    return FormatHasComponentType(format, COMPONENT_TYPE::G);
}

bool FormatHasBlue(VkFormat format) {
    return FormatHasComponentType(format, COMPONENT_TYPE::B);
}

bool FormatHasAlpha(VkFormat format) {
    return FormatHasComponentType(format, COMPONENT_TYPE::A);
}

bool FormatsSameComponentBits(VkFormat format_a, VkFormat format_b) {
    const auto item_a = kVkFormatTable.find(format_a);
    const auto item_b = kVkFormatTable.find(format_b);
    if (item_a == kVkFormatTable.end() || item_b == kVkFormatTable.end()) {
        return false;
    } else if (item_a->second.component_count != item_b->second.component_count) {
        return false;
    }
    // Need to loop match each component type is found in both formats
    // formats are maxed at 4 components, so the double loop is not going to scale
    for (uint32_t i = 0; i < item_a->second.component_count; i++) {
        const auto& component_a = item_a->second.components[i];
        bool component_match = false;
        for (uint32_t j = 0; j < item_b->second.component_count; j++) {
            const auto& component_b = item_b->second.components[j];
            if ((component_a.type == component_b.type) && (component_a.size == component_b.size)) {
                component_match = true;
                break;
            }
        }
        if (!component_match) {
            return false;
        }
    }
    return true;
}''')
        write("".join(out), file=self.outFile)

    #
    # Misc functions
    def utilFunctions(self):
        out = []
        if self.headerFile:
            out.append('''
// Utils/misc
static inline bool FormatIsUndef(VkFormat format) { return (format == VK_FORMAT_UNDEFINED); }
// "blocked image" are defined in the spec (vkspec.html#blocked-image)
static inline bool FormatIsBlockedImage(VkFormat format) {
    return (FormatIsCompressed(format) || FormatIsSinglePlane_422(format));
}
// No official spec definition of "color format"
// So anything that could NOT be a "color format" is a color format
static inline bool FormatIsColor(VkFormat format) {
    return !(FormatIsUndef(format) || FormatIsDepthOrStencil(format) || FormatIsMultiplane(format));
}''')
        write("".join(out), file=self.outFile)
