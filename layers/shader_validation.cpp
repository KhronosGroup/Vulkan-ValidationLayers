/* Copyright (c) 2015-2020 The Khronos Group Inc.
 * Copyright (c) 2015-2020 Valve Corporation
 * Copyright (c) 2015-2020 LunarG, Inc.
 * Copyright (C) 2015-2020 Google Inc.
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
 *
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Dave Houlton <daveh@lunarg.com>
 */

#include "shader_validation.h"

#include <cassert>
#include <chrono>
#include <cinttypes>
#include <cmath>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <spirv/unified1/spirv.hpp>
#include "vk_loader_platform.h"
#include "vk_enum_string_helper.h"
#include "vk_layer_data.h"
#include "vk_layer_extension_utils.h"
#include "vk_layer_utils.h"
#include "chassis.h"
#include "core_validation.h"

#include "spirv-tools/libspirv.h"
#include "xxhash.h"

void decoration_set::add(uint32_t decoration, uint32_t value) {
    switch (decoration) {
        case spv::DecorationLocation:
            flags |= location_bit;
            location = value;
            break;
        case spv::DecorationPatch:
            flags |= patch_bit;
            break;
        case spv::DecorationRelaxedPrecision:
            flags |= relaxed_precision_bit;
            break;
        case spv::DecorationBlock:
            flags |= block_bit;
            break;
        case spv::DecorationBufferBlock:
            flags |= buffer_block_bit;
            break;
        case spv::DecorationComponent:
            flags |= component_bit;
            component = value;
            break;
        case spv::DecorationInputAttachmentIndex:
            flags |= input_attachment_index_bit;
            input_attachment_index = value;
            break;
        case spv::DecorationDescriptorSet:
            flags |= descriptor_set_bit;
            descriptor_set = value;
            break;
        case spv::DecorationBinding:
            flags |= binding_bit;
            binding = value;
            break;
        case spv::DecorationNonWritable:
            flags |= nonwritable_bit;
            break;
        case spv::DecorationBuiltIn:
            flags |= builtin_bit;
            builtin = value;
            break;
    }
}

enum FORMAT_TYPE {
    FORMAT_TYPE_FLOAT = 1,  // UNORM, SNORM, FLOAT, USCALED, SSCALED, SRGB -- anything we consider float in the shader
    FORMAT_TYPE_SINT = 2,
    FORMAT_TYPE_UINT = 4,
};

typedef std::pair<unsigned, unsigned> location_t;

static shader_stage_attributes shader_stage_attribs[] = {
    {"vertex shader", false, false, VK_SHADER_STAGE_VERTEX_BIT},
    {"tessellation control shader", true, true, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT},
    {"tessellation evaluation shader", true, false, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT},
    {"geometry shader", true, false, VK_SHADER_STAGE_GEOMETRY_BIT},
    {"fragment shader", false, false, VK_SHADER_STAGE_FRAGMENT_BIT},
};

unsigned ExecutionModelToShaderStageFlagBits(unsigned mode);

// SPIRV utility functions
void SHADER_MODULE_STATE::BuildDefIndex() {
    for (auto insn : *this) {
        switch (insn.opcode()) {
            // Types
            case spv::OpTypeVoid:
            case spv::OpTypeBool:
            case spv::OpTypeInt:
            case spv::OpTypeFloat:
            case spv::OpTypeVector:
            case spv::OpTypeMatrix:
            case spv::OpTypeImage:
            case spv::OpTypeSampler:
            case spv::OpTypeSampledImage:
            case spv::OpTypeArray:
            case spv::OpTypeRuntimeArray:
            case spv::OpTypeStruct:
            case spv::OpTypeOpaque:
            case spv::OpTypePointer:
            case spv::OpTypeFunction:
            case spv::OpTypeEvent:
            case spv::OpTypeDeviceEvent:
            case spv::OpTypeReserveId:
            case spv::OpTypeQueue:
            case spv::OpTypePipe:
            case spv::OpTypeAccelerationStructureNV:
            case spv::OpTypeCooperativeMatrixNV:
                def_index[insn.word(1)] = insn.offset();
                break;

                // Fixed constants
            case spv::OpConstantTrue:
            case spv::OpConstantFalse:
            case spv::OpConstant:
            case spv::OpConstantComposite:
            case spv::OpConstantSampler:
            case spv::OpConstantNull:
                def_index[insn.word(2)] = insn.offset();
                break;

                // Specialization constants
            case spv::OpSpecConstantTrue:
            case spv::OpSpecConstantFalse:
            case spv::OpSpecConstant:
            case spv::OpSpecConstantComposite:
            case spv::OpSpecConstantOp:
                def_index[insn.word(2)] = insn.offset();
                break;

                // Variables
            case spv::OpVariable:
                def_index[insn.word(2)] = insn.offset();
                break;

                // Functions
            case spv::OpFunction:
                def_index[insn.word(2)] = insn.offset();
                break;

                // Decorations
            case spv::OpDecorate: {
                auto targetId = insn.word(1);
                decorations[targetId].add(insn.word(2), insn.len() > 3u ? insn.word(3) : 0u);
            } break;
            case spv::OpGroupDecorate: {
                auto const &src = decorations[insn.word(1)];
                for (auto i = 2u; i < insn.len(); i++) decorations[insn.word(i)].merge(src);
            } break;

                // Entry points ... add to the entrypoint table
            case spv::OpEntryPoint: {
                // Entry points do not have an id (the id is the function id) and thus need their own table
                auto entrypoint_name = (char const *)&insn.word(3);
                auto execution_model = insn.word(1);
                auto entrypoint_stage = ExecutionModelToShaderStageFlagBits(execution_model);
                entry_points.emplace(entrypoint_name, EntryPoint{insn.offset(), entrypoint_stage});
                break;
            }

            default:
                // We don't care about any other defs for now.
                break;
        }
    }
}

unsigned ExecutionModelToShaderStageFlagBits(unsigned mode) {
    switch (mode) {
        case spv::ExecutionModelVertex:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case spv::ExecutionModelTessellationControl:
            return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case spv::ExecutionModelTessellationEvaluation:
            return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case spv::ExecutionModelGeometry:
            return VK_SHADER_STAGE_GEOMETRY_BIT;
        case spv::ExecutionModelFragment:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        case spv::ExecutionModelGLCompute:
            return VK_SHADER_STAGE_COMPUTE_BIT;
        case spv::ExecutionModelRayGenerationNV:
            return VK_SHADER_STAGE_RAYGEN_BIT_NV;
        case spv::ExecutionModelAnyHitNV:
            return VK_SHADER_STAGE_ANY_HIT_BIT_NV;
        case spv::ExecutionModelClosestHitNV:
            return VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
        case spv::ExecutionModelMissNV:
            return VK_SHADER_STAGE_MISS_BIT_NV;
        case spv::ExecutionModelIntersectionNV:
            return VK_SHADER_STAGE_INTERSECTION_BIT_NV;
        case spv::ExecutionModelCallableNV:
            return VK_SHADER_STAGE_CALLABLE_BIT_NV;
        case spv::ExecutionModelTaskNV:
            return VK_SHADER_STAGE_TASK_BIT_NV;
        case spv::ExecutionModelMeshNV:
            return VK_SHADER_STAGE_MESH_BIT_NV;
        default:
            return 0;
    }
}

spirv_inst_iter FindEntrypoint(SHADER_MODULE_STATE const *src, char const *name, VkShaderStageFlagBits stageBits) {
    auto range = src->entry_points.equal_range(name);
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second.stage == stageBits) {
            return src->at(it->second.offset);
        }
    }
    return src->end();
}

static char const *StorageClassName(unsigned sc) {
    switch (sc) {
        case spv::StorageClassInput:
            return "input";
        case spv::StorageClassOutput:
            return "output";
        case spv::StorageClassUniformConstant:
            return "const uniform";
        case spv::StorageClassUniform:
            return "uniform";
        case spv::StorageClassWorkgroup:
            return "workgroup local";
        case spv::StorageClassCrossWorkgroup:
            return "workgroup global";
        case spv::StorageClassPrivate:
            return "private global";
        case spv::StorageClassFunction:
            return "function";
        case spv::StorageClassGeneric:
            return "generic";
        case spv::StorageClassAtomicCounter:
            return "atomic counter";
        case spv::StorageClassImage:
            return "image";
        case spv::StorageClassPushConstant:
            return "push constant";
        case spv::StorageClassStorageBuffer:
            return "storage buffer";
        default:
            return "unknown";
    }
}

// Get the value of an integral constant
unsigned GetConstantValue(SHADER_MODULE_STATE const *src, unsigned id) {
    auto value = src->get_def(id);
    assert(value != src->end());

    if (value.opcode() != spv::OpConstant) {
        // TODO: Either ensure that the specialization transform is already performed on a module we're
        //       considering here, OR -- specialize on the fly now.
        return 1;
    }

    return value.word(3);
}

static void DescribeTypeInner(std::ostringstream &ss, SHADER_MODULE_STATE const *src, unsigned type) {
    auto insn = src->get_def(type);
    assert(insn != src->end());

    switch (insn.opcode()) {
        case spv::OpTypeBool:
            ss << "bool";
            break;
        case spv::OpTypeInt:
            ss << (insn.word(3) ? 's' : 'u') << "int" << insn.word(2);
            break;
        case spv::OpTypeFloat:
            ss << "float" << insn.word(2);
            break;
        case spv::OpTypeVector:
            ss << "vec" << insn.word(3) << " of ";
            DescribeTypeInner(ss, src, insn.word(2));
            break;
        case spv::OpTypeMatrix:
            ss << "mat" << insn.word(3) << " of ";
            DescribeTypeInner(ss, src, insn.word(2));
            break;
        case spv::OpTypeArray:
            ss << "arr[" << GetConstantValue(src, insn.word(3)) << "] of ";
            DescribeTypeInner(ss, src, insn.word(2));
            break;
        case spv::OpTypeRuntimeArray:
            ss << "runtime arr[] of ";
            DescribeTypeInner(ss, src, insn.word(2));
            break;
        case spv::OpTypePointer:
            ss << "ptr to " << StorageClassName(insn.word(2)) << " ";
            DescribeTypeInner(ss, src, insn.word(3));
            break;
        case spv::OpTypeStruct: {
            ss << "struct of (";
            for (unsigned i = 2; i < insn.len(); i++) {
                DescribeTypeInner(ss, src, insn.word(i));
                if (i == insn.len() - 1) {
                    ss << ")";
                } else {
                    ss << ", ";
                }
            }
            break;
        }
        case spv::OpTypeSampler:
            ss << "sampler";
            break;
        case spv::OpTypeSampledImage:
            ss << "sampler+";
            DescribeTypeInner(ss, src, insn.word(2));
            break;
        case spv::OpTypeImage:
            ss << "image(dim=" << insn.word(3) << ", sampled=" << insn.word(7) << ")";
            break;
        case spv::OpTypeAccelerationStructureNV:
            ss << "accelerationStruture";
            break;
        default:
            ss << "oddtype";
            break;
    }
}

static std::string DescribeType(SHADER_MODULE_STATE const *src, unsigned type) {
    std::ostringstream ss;
    DescribeTypeInner(ss, src, type);
    return ss.str();
}

static bool IsNarrowNumericType(spirv_inst_iter type) {
    if (type.opcode() != spv::OpTypeInt && type.opcode() != spv::OpTypeFloat) return false;
    return type.word(2) < 64;
}

static bool TypesMatch(SHADER_MODULE_STATE const *a, SHADER_MODULE_STATE const *b, unsigned a_type, unsigned b_type, bool a_arrayed,
                       bool b_arrayed, bool relaxed) {
    // Walk two type trees together, and complain about differences
    auto a_insn = a->get_def(a_type);
    auto b_insn = b->get_def(b_type);
    assert(a_insn != a->end());
    assert(b_insn != b->end());

    // Ignore runtime-sized arrays-- they cannot appear in these interfaces.

    if (a_arrayed && a_insn.opcode() == spv::OpTypeArray) {
        return TypesMatch(a, b, a_insn.word(2), b_type, false, b_arrayed, relaxed);
    }

    if (b_arrayed && b_insn.opcode() == spv::OpTypeArray) {
        // We probably just found the extra level of arrayness in b_type: compare the type inside it to a_type
        return TypesMatch(a, b, a_type, b_insn.word(2), a_arrayed, false, relaxed);
    }

    if (a_insn.opcode() == spv::OpTypeVector && relaxed && IsNarrowNumericType(b_insn)) {
        return TypesMatch(a, b, a_insn.word(2), b_type, a_arrayed, b_arrayed, false);
    }

    if (a_insn.opcode() != b_insn.opcode()) {
        return false;
    }

    if (a_insn.opcode() == spv::OpTypePointer) {
        // Match on pointee type. storage class is expected to differ
        return TypesMatch(a, b, a_insn.word(3), b_insn.word(3), a_arrayed, b_arrayed, relaxed);
    }

    if (a_arrayed || b_arrayed) {
        // If we havent resolved array-of-verts by here, we're not going to.
        return false;
    }

    switch (a_insn.opcode()) {
        case spv::OpTypeBool:
            return true;
        case spv::OpTypeInt:
            // Match on width, signedness
            return a_insn.word(2) == b_insn.word(2) && a_insn.word(3) == b_insn.word(3);
        case spv::OpTypeFloat:
            // Match on width
            return a_insn.word(2) == b_insn.word(2);
        case spv::OpTypeVector:
            // Match on element type, count.
            if (!TypesMatch(a, b, a_insn.word(2), b_insn.word(2), a_arrayed, b_arrayed, false)) return false;
            if (relaxed && IsNarrowNumericType(a->get_def(a_insn.word(2)))) {
                return a_insn.word(3) >= b_insn.word(3);
            } else {
                return a_insn.word(3) == b_insn.word(3);
            }
        case spv::OpTypeMatrix:
            // Match on element type, count.
            return TypesMatch(a, b, a_insn.word(2), b_insn.word(2), a_arrayed, b_arrayed, false) &&
                   a_insn.word(3) == b_insn.word(3);
        case spv::OpTypeArray:
            // Match on element type, count. these all have the same layout. we don't get here if b_arrayed. This differs from
            // vector & matrix types in that the array size is the id of a constant instruction, * not a literal within OpTypeArray
            return TypesMatch(a, b, a_insn.word(2), b_insn.word(2), a_arrayed, b_arrayed, false) &&
                   GetConstantValue(a, a_insn.word(3)) == GetConstantValue(b, b_insn.word(3));
        case spv::OpTypeStruct:
            // Match on all element types
            {
                if (a_insn.len() != b_insn.len()) {
                    return false;  // Structs cannot match if member counts differ
                }

                for (unsigned i = 2; i < a_insn.len(); i++) {
                    if (!TypesMatch(a, b, a_insn.word(i), b_insn.word(i), a_arrayed, b_arrayed, false)) {
                        return false;
                    }
                }

                return true;
            }
        default:
            // Remaining types are CLisms, or may not appear in the interfaces we are interested in. Just claim no match.
            return false;
    }
}

static unsigned GetLocationsConsumedByType(SHADER_MODULE_STATE const *src, unsigned type, bool strip_array_level) {
    auto insn = src->get_def(type);
    assert(insn != src->end());

    switch (insn.opcode()) {
        case spv::OpTypePointer:
            // See through the ptr -- this is only ever at the toplevel for graphics shaders we're never actually passing
            // pointers around.
            return GetLocationsConsumedByType(src, insn.word(3), strip_array_level);
        case spv::OpTypeArray:
            if (strip_array_level) {
                return GetLocationsConsumedByType(src, insn.word(2), false);
            } else {
                return GetConstantValue(src, insn.word(3)) * GetLocationsConsumedByType(src, insn.word(2), false);
            }
        case spv::OpTypeMatrix:
            // Num locations is the dimension * element size
            return insn.word(3) * GetLocationsConsumedByType(src, insn.word(2), false);
        case spv::OpTypeVector: {
            auto scalar_type = src->get_def(insn.word(2));
            auto bit_width =
                (scalar_type.opcode() == spv::OpTypeInt || scalar_type.opcode() == spv::OpTypeFloat) ? scalar_type.word(2) : 32;

            // Locations are 128-bit wide; 3- and 4-component vectors of 64 bit types require two.
            return (bit_width * insn.word(3) + 127) / 128;
        }
        default:
            // Everything else is just 1.
            return 1;

            // TODO: extend to handle 64bit scalar types, whose vectors may need multiple locations.
    }
}

static unsigned GetComponentsConsumedByType(SHADER_MODULE_STATE const *src, unsigned type, bool strip_array_level) {
    auto insn = src->get_def(type);
    assert(insn != src->end());

    switch (insn.opcode()) {
        case spv::OpTypePointer:
            // See through the ptr -- this is only ever at the toplevel for graphics shaders we're never actually passing
            // pointers around.
            return GetComponentsConsumedByType(src, insn.word(3), strip_array_level);
        case spv::OpTypeStruct: {
            uint32_t sum = 0;
            for (uint32_t i = 2; i < insn.len(); i++) {  // i=2 to skip word(0) and word(1)=ID of struct
                sum += GetComponentsConsumedByType(src, insn.word(i), false);
            }
            return sum;
        }
        case spv::OpTypeArray:
            if (strip_array_level) {
                return GetComponentsConsumedByType(src, insn.word(2), false);
            } else {
                return GetConstantValue(src, insn.word(3)) * GetComponentsConsumedByType(src, insn.word(2), false);
            }
        case spv::OpTypeMatrix:
            // Num locations is the dimension * element size
            return insn.word(3) * GetComponentsConsumedByType(src, insn.word(2), false);
        case spv::OpTypeVector: {
            auto scalar_type = src->get_def(insn.word(2));
            auto bit_width =
                (scalar_type.opcode() == spv::OpTypeInt || scalar_type.opcode() == spv::OpTypeFloat) ? scalar_type.word(2) : 32;
            // One component is 32-bit
            return (bit_width * insn.word(3) + 31) / 32;
        }
        case spv::OpTypeFloat: {
            auto bit_width = insn.word(2);
            return (bit_width + 31) / 32;
        }
        case spv::OpTypeInt: {
            auto bit_width = insn.word(2);
            return (bit_width + 31) / 32;
        }
        case spv::OpConstant:
            return GetComponentsConsumedByType(src, insn.word(1), false);
        default:
            return 0;
    }
}

static unsigned GetLocationsConsumedByFormat(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R64G64B64A64_SFLOAT:
        case VK_FORMAT_R64G64B64A64_SINT:
        case VK_FORMAT_R64G64B64A64_UINT:
        case VK_FORMAT_R64G64B64_SFLOAT:
        case VK_FORMAT_R64G64B64_SINT:
        case VK_FORMAT_R64G64B64_UINT:
            return 2;
        default:
            return 1;
    }
}

static unsigned GetFormatType(VkFormat fmt) {
    if (FormatIsSInt(fmt)) return FORMAT_TYPE_SINT;
    if (FormatIsUInt(fmt)) return FORMAT_TYPE_UINT;
    if (FormatIsDepthAndStencil(fmt)) return FORMAT_TYPE_FLOAT | FORMAT_TYPE_UINT;
    if (fmt == VK_FORMAT_UNDEFINED) return 0;
    // everything else -- UNORM/SNORM/FLOAT/USCALED/SSCALED is all float in the shader.
    return FORMAT_TYPE_FLOAT;
}

// characterizes a SPIR-V type appearing in an interface to a FF stage, for comparison to a VkFormat's characterization above.
// also used for input attachments, as we statically know their format.
static unsigned GetFundamentalType(SHADER_MODULE_STATE const *src, unsigned type) {
    auto insn = src->get_def(type);
    assert(insn != src->end());

    switch (insn.opcode()) {
        case spv::OpTypeInt:
            return insn.word(3) ? FORMAT_TYPE_SINT : FORMAT_TYPE_UINT;
        case spv::OpTypeFloat:
            return FORMAT_TYPE_FLOAT;
        case spv::OpTypeVector:
        case spv::OpTypeMatrix:
        case spv::OpTypeArray:
        case spv::OpTypeRuntimeArray:
        case spv::OpTypeImage:
            return GetFundamentalType(src, insn.word(2));
        case spv::OpTypePointer:
            return GetFundamentalType(src, insn.word(3));

        default:
            return 0;
    }
}

static uint32_t GetShaderStageId(VkShaderStageFlagBits stage) {
    uint32_t bit_pos = uint32_t(u_ffs(stage));
    return bit_pos - 1;
}

static spirv_inst_iter GetStructType(SHADER_MODULE_STATE const *src, spirv_inst_iter def, bool is_array_of_verts) {
    while (true) {
        if (def.opcode() == spv::OpTypePointer) {
            def = src->get_def(def.word(3));
        } else if (def.opcode() == spv::OpTypeArray && is_array_of_verts) {
            def = src->get_def(def.word(2));
            is_array_of_verts = false;
        } else if (def.opcode() == spv::OpTypeStruct) {
            return def;
        } else {
            return src->end();
        }
    }
}

static bool CollectInterfaceBlockMembers(SHADER_MODULE_STATE const *src, std::map<location_t, interface_var> *out,
                                         bool is_array_of_verts, uint32_t id, uint32_t type_id, bool is_patch,
                                         int /*first_location*/) {
    // Walk down the type_id presented, trying to determine whether it's actually an interface block.
    auto type = GetStructType(src, src->get_def(type_id), is_array_of_verts && !is_patch);
    if (type == src->end() || !(src->get_decorations(type.word(1)).flags & decoration_set::block_bit)) {
        // This isn't an interface block.
        return false;
    }

    std::unordered_map<unsigned, unsigned> member_components;
    std::unordered_map<unsigned, unsigned> member_relaxed_precision;
    std::unordered_map<unsigned, unsigned> member_patch;

    // Walk all the OpMemberDecorate for type's result id -- first pass, collect components.
    for (auto insn : *src) {
        if (insn.opcode() == spv::OpMemberDecorate && insn.word(1) == type.word(1)) {
            unsigned member_index = insn.word(2);

            if (insn.word(3) == spv::DecorationComponent) {
                unsigned component = insn.word(4);
                member_components[member_index] = component;
            }

            if (insn.word(3) == spv::DecorationRelaxedPrecision) {
                member_relaxed_precision[member_index] = 1;
            }

            if (insn.word(3) == spv::DecorationPatch) {
                member_patch[member_index] = 1;
            }
        }
    }

    // TODO: correctly handle location assignment from outside

    // Second pass -- produce the output, from Location decorations
    for (auto insn : *src) {
        if (insn.opcode() == spv::OpMemberDecorate && insn.word(1) == type.word(1)) {
            unsigned member_index = insn.word(2);
            unsigned member_type_id = type.word(2 + member_index);

            if (insn.word(3) == spv::DecorationLocation) {
                unsigned location = insn.word(4);
                unsigned num_locations = GetLocationsConsumedByType(src, member_type_id, false);
                auto component_it = member_components.find(member_index);
                unsigned component = component_it == member_components.end() ? 0 : component_it->second;
                bool is_relaxed_precision = member_relaxed_precision.find(member_index) != member_relaxed_precision.end();
                bool member_is_patch = is_patch || member_patch.count(member_index) > 0;

                for (unsigned int offset = 0; offset < num_locations; offset++) {
                    interface_var v = {};
                    v.id = id;
                    // TODO: member index in interface_var too?
                    v.type_id = member_type_id;
                    v.offset = offset;
                    v.is_patch = member_is_patch;
                    v.is_block_member = true;
                    v.is_relaxed_precision = is_relaxed_precision;
                    (*out)[std::make_pair(location + offset, component)] = v;
                }
            }
        }
    }

    return true;
}

static std::vector<uint32_t> FindEntrypointInterfaces(spirv_inst_iter entrypoint) {
    assert(entrypoint.opcode() == spv::OpEntryPoint);

    std::vector<uint32_t> interfaces;
    // Find the end of the entrypoint's name string. additional zero bytes follow the actual null terminator, to fill out the
    // rest of the word - so we only need to look at the last byte in the word to determine which word contains the terminator.
    uint32_t word = 3;
    while (entrypoint.word(word) & 0xff000000u) {
        ++word;
    }
    ++word;

    for (; word < entrypoint.len(); word++) interfaces.push_back(entrypoint.word(word));

    return interfaces;
}

static std::map<location_t, interface_var> CollectInterfaceByLocation(SHADER_MODULE_STATE const *src, spirv_inst_iter entrypoint,
                                                                      spv::StorageClass sinterface, bool is_array_of_verts) {
    // TODO: handle index=1 dual source outputs from FS -- two vars will have the same location, and we DON'T want to clobber.

    std::map<location_t, interface_var> out;

    for (uint32_t iid : FindEntrypointInterfaces(entrypoint)) {
        auto insn = src->get_def(iid);
        assert(insn != src->end());
        assert(insn.opcode() == spv::OpVariable);

        if (insn.word(3) == static_cast<uint32_t>(sinterface)) {
            auto d = src->get_decorations(iid);
            unsigned id = insn.word(2);
            unsigned type = insn.word(1);

            int location = d.location;
            int builtin = d.builtin;
            unsigned component = d.component;
            bool is_patch = (d.flags & decoration_set::patch_bit) != 0;
            bool is_relaxed_precision = (d.flags & decoration_set::relaxed_precision_bit) != 0;

            if (builtin != -1)
                continue;
            else if (!CollectInterfaceBlockMembers(src, &out, is_array_of_verts, id, type, is_patch, location)) {
                // A user-defined interface variable, with a location. Where a variable occupied multiple locations, emit
                // one result for each.
                unsigned num_locations = GetLocationsConsumedByType(src, type, is_array_of_verts && !is_patch);
                for (unsigned int offset = 0; offset < num_locations; offset++) {
                    interface_var v = {};
                    v.id = id;
                    v.type_id = type;
                    v.offset = offset;
                    v.is_patch = is_patch;
                    v.is_relaxed_precision = is_relaxed_precision;
                    out[std::make_pair(location + offset, component)] = v;
                }
            }
        }
    }

    return out;
}

static std::vector<uint32_t> CollectBuiltinBlockMembers(SHADER_MODULE_STATE const *src, spirv_inst_iter entrypoint,
                                                        uint32_t storageClass) {
    std::vector<uint32_t> variables;
    std::vector<uint32_t> builtinStructMembers;
    std::vector<uint32_t> builtinDecorations;

    for (auto insn : *src) {
        switch (insn.opcode()) {
            // Find all built-in member decorations
            case spv::OpMemberDecorate:
                if (insn.word(3) == spv::DecorationBuiltIn) {
                    builtinStructMembers.push_back(insn.word(1));
                }
                break;
            // Find all built-in decorations
            case spv::OpDecorate:
                switch (insn.word(2)) {
                    case spv::DecorationBlock: {
                        uint32_t blockID = insn.word(1);
                        for (auto builtInBlockID : builtinStructMembers) {
                            // Check if one of the members of the block are built-in -> the block is built-in
                            if (blockID == builtInBlockID) {
                                builtinDecorations.push_back(blockID);
                                break;
                            }
                        }
                        break;
                    }
                    case spv::DecorationBuiltIn:
                        builtinDecorations.push_back(insn.word(1));
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }

    // Find all interface variables belonging to the entrypoint and matching the storage class
    for (uint32_t id : FindEntrypointInterfaces(entrypoint)) {
        auto def = src->get_def(id);
        assert(def != src->end());
        assert(def.opcode() == spv::OpVariable);

        if (def.word(3) == storageClass) variables.push_back(def.word(1));
    }

    // Find all members belonging to the builtin block selected
    std::vector<uint32_t> builtinBlockMembers;
    for (auto &var : variables) {
        auto def = src->get_def(src->get_def(var).word(3));

        // It could be an array of IO blocks. The element type should be the struct defining the block contents
        if (def.opcode() == spv::OpTypeArray) def = src->get_def(def.word(2));

        // Now find all members belonging to the struct defining the IO block
        if (def.opcode() == spv::OpTypeStruct) {
            for (auto builtInID : builtinDecorations) {
                if (builtInID == def.word(1)) {
                    for (int i = 2; i < (int)def.len(); i++)
                        builtinBlockMembers.push_back(spv::BuiltInMax);  // Start with undefined builtin for each struct member.
                                                                         // These shouldn't be left after replacing.
                    for (auto insn : *src) {
                        if (insn.opcode() == spv::OpMemberDecorate && insn.word(1) == builtInID &&
                            insn.word(3) == spv::DecorationBuiltIn) {
                            auto structIndex = insn.word(2);
                            assert(structIndex < builtinBlockMembers.size());
                            builtinBlockMembers[structIndex] = insn.word(4);
                        }
                    }
                }
            }
        }
    }

    return builtinBlockMembers;
}

static std::vector<std::pair<uint32_t, interface_var>> CollectInterfaceByInputAttachmentIndex(
    SHADER_MODULE_STATE const *src, std::unordered_set<uint32_t> const &accessible_ids) {
    std::vector<std::pair<uint32_t, interface_var>> out;

    for (auto insn : *src) {
        if (insn.opcode() == spv::OpDecorate) {
            if (insn.word(2) == spv::DecorationInputAttachmentIndex) {
                auto attachment_index = insn.word(3);
                auto id = insn.word(1);

                if (accessible_ids.count(id)) {
                    auto def = src->get_def(id);
                    assert(def != src->end());
                    if (def.opcode() == spv::OpVariable && def.word(3) == spv::StorageClassUniformConstant) {
                        auto num_locations = GetLocationsConsumedByType(src, def.word(1), false);
                        for (unsigned int offset = 0; offset < num_locations; offset++) {
                            interface_var v = {};
                            v.id = id;
                            v.type_id = def.word(1);
                            v.offset = offset;
                            out.emplace_back(attachment_index + offset, v);
                        }
                    }
                }
            }
        }
    }

    return out;
}

static bool AtomicOperation(uint32_t opcode) {
    switch (opcode) {
        case spv::OpAtomicLoad:
        case spv::OpAtomicStore:
        case spv::OpAtomicExchange:
        case spv::OpAtomicCompareExchange:
        case spv::OpAtomicCompareExchangeWeak:
        case spv::OpAtomicIIncrement:
        case spv::OpAtomicIDecrement:
        case spv::OpAtomicIAdd:
        case spv::OpAtomicISub:
        case spv::OpAtomicSMin:
        case spv::OpAtomicUMin:
        case spv::OpAtomicSMax:
        case spv::OpAtomicUMax:
        case spv::OpAtomicAnd:
        case spv::OpAtomicOr:
        case spv::OpAtomicXor:
        case spv::OpAtomicFAddEXT:
            return true;
        default:
            return false;
    }
    return false;
}

// Check writable, image atomic operation
static void IsSpecificDescriptorType(SHADER_MODULE_STATE const *module, const spirv_inst_iter &id_it, bool is_storage_buffer,
                                     bool is_check_writable, interface_var &out_interface_var) {
    uint32_t type_id = id_it.word(1);
    auto type = module->get_def(type_id);

    // Strip off any array or ptrs. Where we remove array levels, adjust the  descriptor count for each dimension.
    while (type.opcode() == spv::OpTypeArray || type.opcode() == spv::OpTypePointer || type.opcode() == spv::OpTypeRuntimeArray) {
        if (type.opcode() == spv::OpTypeArray || type.opcode() == spv::OpTypeRuntimeArray) {
            type = module->get_def(type.word(2));  // Element type
        } else {
            type = module->get_def(type.word(3));  // Pointee type
        }
    }
    switch (type.opcode()) {
        case spv::OpTypeImage: {
            auto dim = type.word(3);
            auto sampled = type.word(7);
            if (sampled == 2 && dim != spv::DimSubpassData) {
                std::vector<unsigned> imagwrite_members;
                std::vector<unsigned> atomic_members;
                std::unordered_map<unsigned, unsigned> load_members;
                std::unordered_map<unsigned, unsigned> accesschain_members;
                std::unordered_map<unsigned, unsigned> image_texel_pointer_members;

                unsigned int id = id_it.word(2);

                for (auto insn : *module) {
                    switch (insn.opcode()) {
                        case spv::OpImageWrite: {
                            if (is_check_writable) imagwrite_members.emplace_back(insn.word(1));  // Load id
                            break;
                        }
                        case spv::OpLoad: {
                            // 2: Load id, 3: object id or AccessChain id
                            load_members.insert(std::make_pair(insn.word(2), insn.word(3)));
                            break;
                        }
                        case spv::OpAccessChain: {
                            // 2: AccessChain id, 3: object id
                            if (insn.word(3) == id) {
                                accesschain_members.insert(std::make_pair(insn.word(2), insn.word(3)));
                            }
                            break;
                        }
                        case spv::OpImageTexelPointer: {
                            // 2: ImageTexelPointer id, 3: object id
                            image_texel_pointer_members.insert(std::make_pair(insn.word(2), insn.word(3)));
                            break;
                        }
                        default: {
                            if (AtomicOperation(insn.opcode())) {
                                if (insn.opcode() == spv::OpAtomicStore) {
                                    atomic_members.emplace_back(insn.word(1));  // ImageTexelPointer id
                                } else {
                                    atomic_members.emplace_back(insn.word(3));  // ImageTexelPointer id
                                }
                            }
                            break;
                        }
                    }
                }
                out_interface_var.is_writable = false;
                out_interface_var.is_atomic_operation = false;

                for (auto load_id : imagwrite_members) {
                    auto load_it = load_members.find(load_id);
                    if (load_it == load_members.end()) {
                        continue;
                    }
                    if (load_it->second == id) {
                        out_interface_var.is_writable = true;
                        break;
                    }

                    auto accesschain_it = accesschain_members.find(load_it->second);
                    if (accesschain_it == accesschain_members.end()) {
                        continue;
                    }
                    out_interface_var.is_writable = true;
                    accesschain_members.erase(accesschain_it);
                    break;
                }

                for (auto itp_id : atomic_members) {
                    auto ltp_it = image_texel_pointer_members.find(itp_id);
                    if (ltp_it == image_texel_pointer_members.end()) {
                        continue;
                    }
                    if (ltp_it->second == id) {
                        out_interface_var.is_atomic_operation = true;
                        break;
                    }

                    auto accesschain_it = accesschain_members.find(ltp_it->second);
                    if (accesschain_it == accesschain_members.end()) {
                        continue;
                    }
                    out_interface_var.is_atomic_operation = true;
                    break;
                }
            }
            return;
        }

        case spv::OpTypeStruct: {
            std::unordered_set<unsigned> nonwritable_members;
            if (module->get_decorations(type.word(1)).flags & decoration_set::buffer_block_bit) is_storage_buffer = true;
            for (auto insn : *module) {
                if (insn.opcode() == spv::OpMemberDecorate && insn.word(1) == type.word(1) &&
                    insn.word(3) == spv::DecorationNonWritable) {
                    nonwritable_members.insert(insn.word(2));
                }
            }

            // A buffer is writable if it's either flavor of storage buffer, and has any member not decorated
            // as nonwritable.
            if (is_storage_buffer && nonwritable_members.size() != type.len() - 2) {
                std::vector<unsigned> store_members;
                std::unordered_map<unsigned, unsigned> accesschain_members;
                std::vector<unsigned> atomic_store_members;
                std::unordered_map<unsigned, unsigned> image_texel_pointer_members;
                unsigned int id = id_it.word(2);

                for (auto insn : *module) {
                    switch (insn.opcode()) {
                        case spv::OpStore: {
                            if (insn.word(1) == id) {
                                out_interface_var.is_writable = true;
                                return;
                            }
                            store_members.emplace_back(insn.word(1));  // object id or AccessChain id
                            break;
                        }
                        case spv::OpAtomicStore: {
                            atomic_store_members.emplace_back(insn.word(1));  // ImageTexelPointer id
                            break;
                        }
                        case spv::OpImageTexelPointer: {
                            // 2: ImageTexelPointer id, 3: object id
                            image_texel_pointer_members.insert(std::make_pair(insn.word(2), insn.word(3)));
                            break;
                        }
                        case spv::OpAccessChain: {
                            // 2: AccessChain id, 3: object id
                            if (insn.word(3) == id) {
                                accesschain_members.insert(std::make_pair(insn.word(2), insn.word(3)));
                            }
                            break;
                        }
                        default:
                            break;
                    }
                }
                out_interface_var.is_writable = false;

                for (auto oid : store_members) {
                    auto accesschain_it = accesschain_members.find(oid);
                    if (accesschain_it == accesschain_members.end()) {
                        continue;
                    }
                    out_interface_var.is_writable = true;
                    return;
                }

                for (auto itp_id : atomic_store_members) {
                    auto ltp_it = image_texel_pointer_members.find(itp_id);
                    if (ltp_it == image_texel_pointer_members.end()) {
                        continue;
                    }
                    if (ltp_it->second == id) {
                        out_interface_var.is_writable = true;
                        return;
                    }

                    auto accesschain_it = accesschain_members.find(ltp_it->second);
                    if (accesschain_it == accesschain_members.end()) {
                        continue;
                    }
                    out_interface_var.is_writable = true;
                    return;
                }
            }
        }
    }
}

std::vector<std::pair<descriptor_slot_t, interface_var>> CollectInterfaceByDescriptorSlot(
    SHADER_MODULE_STATE const *src, std::unordered_set<uint32_t> const &accessible_ids, bool *has_writable_descriptor,
    bool *has_atomic_descriptor) {
    std::vector<std::pair<descriptor_slot_t, interface_var>> out;

    for (auto id : accessible_ids) {
        auto insn = src->get_def(id);
        assert(insn != src->end());

        if (insn.opcode() == spv::OpVariable &&
            (insn.word(3) == spv::StorageClassUniform || insn.word(3) == spv::StorageClassUniformConstant ||
             insn.word(3) == spv::StorageClassStorageBuffer)) {
            auto d = src->get_decorations(insn.word(2));
            unsigned set = d.descriptor_set;
            unsigned binding = d.binding;

            interface_var v = {};
            v.id = insn.word(2);
            v.type_id = insn.word(1);

            IsSpecificDescriptorType(src, insn, insn.word(3) == spv::StorageClassStorageBuffer,
                                     !(d.flags & decoration_set::nonwritable_bit), v);
            if (v.is_writable) *has_writable_descriptor = true;
            if (v.is_atomic_operation) *has_atomic_descriptor = true;
            if (d.flags & decoration_set::input_attachment_index_bit) {
                v.input_index = d.input_attachment_index;
            }
            out.emplace_back(std::make_pair(set, binding), v);
        }
    }

    return out;
}

std::unordered_set<uint32_t> CollectWritableOutputLocationinFS(const SHADER_MODULE_STATE &module,
                                                               const VkPipelineShaderStageCreateInfo &stage_info) {
    std::unordered_set<uint32_t> location_list;
    if (stage_info.stage != VK_SHADER_STAGE_FRAGMENT_BIT) return location_list;
    const auto entrypoint = FindEntrypoint(&module, stage_info.pName, stage_info.stage);
    const auto outputs = CollectInterfaceByLocation(&module, entrypoint, spv::StorageClassOutput, false);
    std::unordered_set<unsigned> store_members;
    std::unordered_map<unsigned, unsigned> accesschain_members;

    for (auto insn : module) {
        switch (insn.opcode()) {
            case spv::OpStore:
            case spv::OpAtomicStore: {
                store_members.insert(insn.word(1));  // object id or AccessChain id
                break;
            }
            case spv::OpAccessChain: {
                // 2: AccessChain id, 3: object id
                if (insn.word(3)) accesschain_members.insert(std::make_pair(insn.word(2), insn.word(3)));
                break;
            }
            default:
                break;
        }
    }
    if (store_members.empty()) {
        return location_list;
    }
    for (auto output : outputs) {
        auto store_it = store_members.find(output.second.id);
        if (store_it != store_members.end()) {
            location_list.insert(output.first.first);
            store_members.erase(store_it);
            continue;
        }
        store_it = store_members.begin();
        while (store_it != store_members.end()) {
            auto accesschain_it = accesschain_members.find(*store_it);
            if (accesschain_it == accesschain_members.end()) {
                ++store_it;
                continue;
            }
            if (accesschain_it->second == output.second.id) {
                location_list.insert(output.first.first);
                store_members.erase(store_it);
                accesschain_members.erase(accesschain_it);
                break;
            }
            ++store_it;
        }
    }
    return location_list;
}

bool CoreChecks::ValidateViConsistency(VkPipelineVertexInputStateCreateInfo const *vi) const {
    // Walk the binding descriptions, which describe the step rate and stride of each vertex buffer.  Each binding should
    // be specified only once.
    std::unordered_map<uint32_t, VkVertexInputBindingDescription const *> bindings;
    bool skip = false;

    for (unsigned i = 0; i < vi->vertexBindingDescriptionCount; i++) {
        auto desc = &vi->pVertexBindingDescriptions[i];
        auto &binding = bindings[desc->binding];
        if (binding) {
            // TODO: "VUID-VkGraphicsPipelineCreateInfo-pStages-00742" perhaps?
            skip |= LogError(device, kVUID_Core_Shader_InconsistentVi, "Duplicate vertex input binding descriptions for binding %d",
                             desc->binding);
        } else {
            binding = desc;
        }
    }

    return skip;
}

bool CoreChecks::ValidateViAgainstVsInputs(VkPipelineVertexInputStateCreateInfo const *vi, SHADER_MODULE_STATE const *vs,
                                           spirv_inst_iter entrypoint) const {
    bool skip = false;

    const auto inputs = CollectInterfaceByLocation(vs, entrypoint, spv::StorageClassInput, false);

    // Build index by location
    std::map<uint32_t, const VkVertexInputAttributeDescription *> attribs;
    if (vi) {
        for (uint32_t i = 0; i < vi->vertexAttributeDescriptionCount; ++i) {
            const auto num_locations = GetLocationsConsumedByFormat(vi->pVertexAttributeDescriptions[i].format);
            for (uint32_t j = 0; j < num_locations; ++j) {
                attribs[vi->pVertexAttributeDescriptions[i].location + j] = &vi->pVertexAttributeDescriptions[i];
            }
        }
    }

    struct AttribInputPair {
        const VkVertexInputAttributeDescription *attrib = nullptr;
        const interface_var *input = nullptr;
    };
    std::map<uint32_t, AttribInputPair> location_map;
    for (const auto &attrib_it : attribs) location_map[attrib_it.first].attrib = attrib_it.second;
    for (const auto &input_it : inputs) location_map[input_it.first.first].input = &input_it.second;

    for (const auto &location_it : location_map) {
        const auto location = location_it.first;
        const auto attrib = location_it.second.attrib;
        const auto input = location_it.second.input;

        if (attrib && !input) {
            skip |= LogPerformanceWarning(vs->vk_shader_module, kVUID_Core_Shader_OutputNotConsumed,
                                          "Vertex attribute at location %" PRIu32 " not consumed by vertex shader", location);
        } else if (!attrib && input) {
            skip |= LogError(vs->vk_shader_module, kVUID_Core_Shader_InputNotProduced,
                             "Vertex shader consumes input at location %" PRIu32 " but not provided", location);
        } else if (attrib && input) {
            const auto attrib_type = GetFormatType(attrib->format);
            const auto input_type = GetFundamentalType(vs, input->type_id);

            // Type checking
            if (!(attrib_type & input_type)) {
                skip |= LogError(vs->vk_shader_module, kVUID_Core_Shader_InterfaceTypeMismatch,
                                 "Attribute type of `%s` at location %" PRIu32 " does not match vertex shader input type of `%s`",
                                 string_VkFormat(attrib->format), location, DescribeType(vs, input->type_id).c_str());
            }
        } else {            // !attrib && !input
            assert(false);  // at least one exists in the map
        }
    }

    return skip;
}

bool CoreChecks::ValidateFsOutputsAgainstRenderPass(SHADER_MODULE_STATE const *fs, spirv_inst_iter entrypoint,
                                                    PIPELINE_STATE const *pipeline, uint32_t subpass_index) const {
    bool skip = false;

    const auto rpci = pipeline->rp_state->createInfo.ptr();

    struct Attachment {
        const VkAttachmentReference2KHR *reference = nullptr;
        const VkAttachmentDescription2KHR *attachment = nullptr;
        const interface_var *output = nullptr;
    };
    std::map<uint32_t, Attachment> location_map;

    const auto subpass = rpci->pSubpasses[subpass_index];
    for (uint32_t i = 0; i < subpass.colorAttachmentCount; ++i) {
        auto const &reference = subpass.pColorAttachments[i];
        location_map[i].reference = &reference;
        if (reference.attachment != VK_ATTACHMENT_UNUSED &&
            rpci->pAttachments[reference.attachment].format != VK_FORMAT_UNDEFINED) {
            location_map[i].attachment = &rpci->pAttachments[reference.attachment];
        }
    }

    // TODO: dual source blend index (spv::DecIndex, zero if not provided)

    const auto outputs = CollectInterfaceByLocation(fs, entrypoint, spv::StorageClassOutput, false);
    for (const auto &output_it : outputs) {
        auto const location = output_it.first.first;
        location_map[location].output = &output_it.second;
    }

    const bool alphaToCoverageEnabled = pipeline->graphicsPipelineCI.pMultisampleState != NULL &&
                                        pipeline->graphicsPipelineCI.pMultisampleState->alphaToCoverageEnable == VK_TRUE;

    for (const auto &location_it : location_map) {
        const auto reference = location_it.second.reference;
        if (reference != nullptr && reference->attachment == VK_ATTACHMENT_UNUSED) {
            continue;
        }

        const auto location = location_it.first;
        const auto attachment = location_it.second.attachment;
        const auto output = location_it.second.output;
        if (attachment && !output) {
            if (pipeline->attachments[location].colorWriteMask != 0) {
                skip |= LogWarning(fs->vk_shader_module, kVUID_Core_Shader_InputNotProduced,
                                   "Attachment %" PRIu32
                                   " not written by fragment shader; undefined values will be written to attachment",
                                   location);
            }
        } else if (!attachment && output) {
            if (!(alphaToCoverageEnabled && location == 0)) {
                skip |= LogWarning(fs->vk_shader_module, kVUID_Core_Shader_OutputNotConsumed,
                                   "fragment shader writes to output location %" PRIu32 " with no matching attachment", location);
            }
        } else if (attachment && output) {
            const auto attachment_type = GetFormatType(attachment->format);
            const auto output_type = GetFundamentalType(fs, output->type_id);

            // Type checking
            if (!(output_type & attachment_type)) {
                skip |=
                    LogWarning(fs->vk_shader_module, kVUID_Core_Shader_InterfaceTypeMismatch,
                               "Attachment %" PRIu32
                               " of type `%s` does not match fragment shader output type of `%s`; resulting values are undefined",
                               location, string_VkFormat(attachment->format), DescribeType(fs, output->type_id).c_str());
            }
        } else {            // !attachment && !output
            assert(false);  // at least one exists in the map
        }
    }

    const auto output_zero = location_map.count(0) ? location_map[0].output : nullptr;
    bool locationZeroHasAlpha = output_zero && fs->get_def(output_zero->type_id) != fs->end() &&
                                GetComponentsConsumedByType(fs, output_zero->type_id, false) == 4;
    if (alphaToCoverageEnabled && !locationZeroHasAlpha) {
        skip |= LogError(fs->vk_shader_module, kVUID_Core_Shader_NoAlphaAtLocation0WithAlphaToCoverage,
                         "fragment shader doesn't declare alpha output at location 0 even though alpha to coverage is enabled.");
    }

    return skip;
}

// For PointSize analysis we need to know if the variable decorated with the PointSize built-in was actually written to.
// This function examines instructions in the static call tree for a write to this variable.
static bool IsPointSizeWritten(SHADER_MODULE_STATE const *src, spirv_inst_iter builtin_instr, spirv_inst_iter entrypoint) {
    auto type = builtin_instr.opcode();
    uint32_t target_id = builtin_instr.word(1);
    bool init_complete = false;

    if (type == spv::OpMemberDecorate) {
        // Built-in is part of a structure -- examine instructions up to first function body to get initial IDs
        auto insn = entrypoint;
        while (!init_complete && (insn.opcode() != spv::OpFunction)) {
            switch (insn.opcode()) {
                case spv::OpTypePointer:
                    if ((insn.word(3) == target_id) && (insn.word(2) == spv::StorageClassOutput)) {
                        target_id = insn.word(1);
                    }
                    break;
                case spv::OpVariable:
                    if (insn.word(1) == target_id) {
                        target_id = insn.word(2);
                        init_complete = true;
                    }
                    break;
            }
            insn++;
        }
    }

    if (!init_complete && (type == spv::OpMemberDecorate)) return false;

    bool found_write = false;
    std::unordered_set<uint32_t> worklist;
    worklist.insert(entrypoint.word(2));

    // Follow instructions in call graph looking for writes to target
    while (!worklist.empty() && !found_write) {
        auto id_iter = worklist.begin();
        auto id = *id_iter;
        worklist.erase(id_iter);

        auto insn = src->get_def(id);
        if (insn == src->end()) {
            continue;
        }

        if (insn.opcode() == spv::OpFunction) {
            // Scan body of function looking for other function calls or items in our ID chain
            while (++insn, insn.opcode() != spv::OpFunctionEnd) {
                switch (insn.opcode()) {
                    case spv::OpAccessChain:
                        if (insn.word(3) == target_id) {
                            if (type == spv::OpMemberDecorate) {
                                auto value = GetConstantValue(src, insn.word(4));
                                if (value == builtin_instr.word(2)) {
                                    target_id = insn.word(2);
                                }
                            } else {
                                target_id = insn.word(2);
                            }
                        }
                        break;
                    case spv::OpStore:
                        if (insn.word(1) == target_id) {
                            found_write = true;
                        }
                        break;
                    case spv::OpFunctionCall:
                        worklist.insert(insn.word(3));
                        break;
                }
            }
        }
    }
    return found_write;
}

// For some analyses, we need to know about all ids referenced by the static call tree of a particular entrypoint. This is
// important for identifying the set of shader resources actually used by an entrypoint, for example.
// Note: we only explore parts of the image which might actually contain ids we care about for the above analyses.
//  - NOT the shader input/output interfaces.
//
// TODO: The set of interesting opcodes here was determined by eyeballing the SPIRV spec. It might be worth
// converting parts of this to be generated from the machine-readable spec instead.
std::unordered_set<uint32_t> MarkAccessibleIds(SHADER_MODULE_STATE const *src, spirv_inst_iter entrypoint) {
    std::unordered_set<uint32_t> ids;
    std::unordered_set<uint32_t> worklist;
    worklist.insert(entrypoint.word(2));

    while (!worklist.empty()) {
        auto id_iter = worklist.begin();
        auto id = *id_iter;
        worklist.erase(id_iter);

        auto insn = src->get_def(id);
        if (insn == src->end()) {
            // ID is something we didn't collect in BuildDefIndex. that's OK -- we'll stumble across all kinds of things here
            // that we may not care about.
            continue;
        }

        // Try to add to the output set
        if (!ids.insert(id).second) {
            continue;  // If we already saw this id, we don't want to walk it again.
        }

        switch (insn.opcode()) {
            case spv::OpFunction:
                // Scan whole body of the function, enlisting anything interesting
                while (++insn, insn.opcode() != spv::OpFunctionEnd) {
                    switch (insn.opcode()) {
                        case spv::OpLoad:
                            worklist.insert(insn.word(3));  // ptr
                            break;
                        case spv::OpStore:
                            worklist.insert(insn.word(1));  // ptr
                            break;
                        case spv::OpAccessChain:
                        case spv::OpInBoundsAccessChain:
                            worklist.insert(insn.word(3));  // base ptr
                            break;
                        case spv::OpSampledImage:
                        case spv::OpImageSampleImplicitLod:
                        case spv::OpImageSampleExplicitLod:
                        case spv::OpImageSampleDrefImplicitLod:
                        case spv::OpImageSampleDrefExplicitLod:
                        case spv::OpImageSampleProjImplicitLod:
                        case spv::OpImageSampleProjExplicitLod:
                        case spv::OpImageSampleProjDrefImplicitLod:
                        case spv::OpImageSampleProjDrefExplicitLod:
                        case spv::OpImageFetch:
                        case spv::OpImageGather:
                        case spv::OpImageDrefGather:
                        case spv::OpImageRead:
                        case spv::OpImage:
                        case spv::OpImageQueryFormat:
                        case spv::OpImageQueryOrder:
                        case spv::OpImageQuerySizeLod:
                        case spv::OpImageQuerySize:
                        case spv::OpImageQueryLod:
                        case spv::OpImageQueryLevels:
                        case spv::OpImageQuerySamples:
                        case spv::OpImageSparseSampleImplicitLod:
                        case spv::OpImageSparseSampleExplicitLod:
                        case spv::OpImageSparseSampleDrefImplicitLod:
                        case spv::OpImageSparseSampleDrefExplicitLod:
                        case spv::OpImageSparseSampleProjImplicitLod:
                        case spv::OpImageSparseSampleProjExplicitLod:
                        case spv::OpImageSparseSampleProjDrefImplicitLod:
                        case spv::OpImageSparseSampleProjDrefExplicitLod:
                        case spv::OpImageSparseFetch:
                        case spv::OpImageSparseGather:
                        case spv::OpImageSparseDrefGather:
                        case spv::OpImageTexelPointer:
                            worklist.insert(insn.word(3));  // Image or sampled image
                            break;
                        case spv::OpImageWrite:
                            worklist.insert(insn.word(1));  // Image -- different operand order to above
                            break;
                        case spv::OpFunctionCall:
                            for (uint32_t i = 3; i < insn.len(); i++) {
                                worklist.insert(insn.word(i));  // fn itself, and all args
                            }
                            break;

                        case spv::OpExtInst:
                            for (uint32_t i = 5; i < insn.len(); i++) {
                                worklist.insert(insn.word(i));  // Operands to ext inst
                            }
                            break;

                        default: {
                            if (AtomicOperation(insn.opcode())) {
                                if (insn.opcode() == spv::OpAtomicStore) {
                                    worklist.insert(insn.word(1));  // ptr
                                } else {
                                    worklist.insert(insn.word(3));  // ptr
                                }
                            }
                            break;
                        }
                    }
                }
                break;
        }
    }

    return ids;
}

bool CoreChecks::ValidatePushConstantBlockAgainstPipeline(std::vector<VkPushConstantRange> const *push_constant_ranges,
                                                          SHADER_MODULE_STATE const *src, spirv_inst_iter type,
                                                          VkShaderStageFlagBits stage) const {
    bool skip = false;

    // Strip off ptrs etc
    type = GetStructType(src, type, false);
    assert(type != src->end());

    // Validate directly off the offsets. this isn't quite correct for arrays and matrices, but is a good first step.
    // TODO: arrays, matrices, weird sizes
    for (auto insn : *src) {
        if (insn.opcode() == spv::OpMemberDecorate && insn.word(1) == type.word(1)) {
            if (insn.word(3) == spv::DecorationOffset) {
                auto const member = insn.word(2);
                auto const offset = insn.word(4);
                auto const size = 4;  // Bytes; TODO: calculate this based on the type

                bool found_range = false;
                for (auto const &range : *push_constant_ranges) {
                    if ((range.offset <= offset) && ((range.offset + range.size) >= (offset + size)) &&
                        (range.stageFlags & stage)) {
                        found_range = true;

                        break;
                    }
                }

                if (!found_range) {
                    skip |= LogError(device, kVUID_Core_Shader_PushConstantOutOfRange,
                                     "Shader push-constant buffer member %u at offset %u is not declared in pipeline layout",
                                     member, offset);
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidatePushConstantUsage(std::vector<VkPushConstantRange> const *push_constant_ranges,
                                           SHADER_MODULE_STATE const *src, std::unordered_set<uint32_t> accessible_ids,
                                           VkShaderStageFlagBits stage) const {
    bool skip = false;

    for (auto id : accessible_ids) {
        auto def_insn = src->get_def(id);
        if (def_insn.opcode() == spv::OpVariable && def_insn.word(3) == spv::StorageClassPushConstant) {
            skip |= ValidatePushConstantBlockAgainstPipeline(push_constant_ranges, src, src->get_def(def_insn.word(1)), stage);
        }
    }

    return skip;
}

// Validate that data for each specialization entry is fully contained within the buffer.
bool CoreChecks::ValidateSpecializationOffsets(VkPipelineShaderStageCreateInfo const *info) const {
    bool skip = false;

    VkSpecializationInfo const *spec = info->pSpecializationInfo;

    if (spec) {
        for (auto i = 0u; i < spec->mapEntryCount; i++) {
            if (spec->pMapEntries[i].offset >= spec->dataSize) {
                skip |= LogError(device, "VUID-VkSpecializationInfo-offset-00773",
                                 "Specialization entry %u (for constant id %u) references memory outside provided specialization "
                                 "data (bytes %u.." PRINTF_SIZE_T_SPECIFIER "; " PRINTF_SIZE_T_SPECIFIER " bytes provided)..",
                                 i, spec->pMapEntries[i].constantID, spec->pMapEntries[i].offset,
                                 spec->pMapEntries[i].offset + spec->dataSize - 1, spec->dataSize);

                continue;
            }
            if (spec->pMapEntries[i].offset + spec->pMapEntries[i].size > spec->dataSize) {
                skip |= LogError(device, "VUID-VkSpecializationInfo-pMapEntries-00774",
                                 "Specialization entry %u (for constant id %u) references memory outside provided specialization "
                                 "data (bytes %u.." PRINTF_SIZE_T_SPECIFIER "; " PRINTF_SIZE_T_SPECIFIER " bytes provided)..",
                                 i, spec->pMapEntries[i].constantID, spec->pMapEntries[i].offset,
                                 spec->pMapEntries[i].offset + spec->pMapEntries[i].size - 1, spec->dataSize);
            }
        }
    }

    return skip;
}

// TODO (jbolz): Can this return a const reference?
static std::set<uint32_t> TypeToDescriptorTypeSet(SHADER_MODULE_STATE const *module, uint32_t type_id, unsigned &descriptor_count) {
    auto type = module->get_def(type_id);
    bool is_storage_buffer = false;
    descriptor_count = 1;
    std::set<uint32_t> ret;

    // Strip off any array or ptrs. Where we remove array levels, adjust the  descriptor count for each dimension.
    while (type.opcode() == spv::OpTypeArray || type.opcode() == spv::OpTypePointer || type.opcode() == spv::OpTypeRuntimeArray) {
        if (type.opcode() == spv::OpTypeRuntimeArray) {
            descriptor_count = 0;
            type = module->get_def(type.word(2));
        } else if (type.opcode() == spv::OpTypeArray) {
            descriptor_count *= GetConstantValue(module, type.word(3));
            type = module->get_def(type.word(2));
        } else {
            if (type.word(2) == spv::StorageClassStorageBuffer) {
                is_storage_buffer = true;
            }
            type = module->get_def(type.word(3));
        }
    }

    switch (type.opcode()) {
        case spv::OpTypeStruct: {
            for (auto insn : *module) {
                if (insn.opcode() == spv::OpDecorate && insn.word(1) == type.word(1)) {
                    if (insn.word(2) == spv::DecorationBlock) {
                        if (is_storage_buffer) {
                            ret.insert(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
                            ret.insert(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);
                            return ret;
                        } else {
                            ret.insert(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
                            ret.insert(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
                            ret.insert(VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT);
                            return ret;
                        }
                    } else if (insn.word(2) == spv::DecorationBufferBlock) {
                        ret.insert(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
                        ret.insert(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);
                        return ret;
                    }
                }
            }

            // Invalid
            return ret;
        }

        case spv::OpTypeSampler:
            ret.insert(VK_DESCRIPTOR_TYPE_SAMPLER);
            ret.insert(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            return ret;

        case spv::OpTypeSampledImage: {
            // Slight relaxation for some GLSL historical madness: samplerBuffer doesn't really have a sampler, and a texel
            // buffer descriptor doesn't really provide one. Allow this slight mismatch.
            auto image_type = module->get_def(type.word(2));
            auto dim = image_type.word(3);
            auto sampled = image_type.word(7);
            if (dim == spv::DimBuffer && sampled == 1) {
                ret.insert(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
                return ret;
            }
        }
            ret.insert(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            return ret;

        case spv::OpTypeImage: {
            // Many descriptor types backing image types-- depends on dimension and whether the image will be used with a sampler.
            // SPIRV for Vulkan requires that sampled be 1 or 2 -- leaving the decision to runtime is unacceptable.
            auto dim = type.word(3);
            auto sampled = type.word(7);

            if (dim == spv::DimSubpassData) {
                ret.insert(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
                return ret;
            } else if (dim == spv::DimBuffer) {
                if (sampled == 1) {
                    ret.insert(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
                    return ret;
                } else {
                    ret.insert(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
                    return ret;
                }
            } else if (sampled == 1) {
                ret.insert(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
                ret.insert(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
                return ret;
            } else {
                ret.insert(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
                return ret;
            }
        }
        case spv::OpTypeAccelerationStructureNV:
            ret.insert(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV);
            return ret;

            // We shouldn't really see any other junk types -- but if we do, they're a mismatch.
        default:
            return ret;  // Matches nothing
    }
}

static std::string string_descriptorTypes(const std::set<uint32_t> &descriptor_types) {
    std::stringstream ss;
    for (auto it = descriptor_types.begin(); it != descriptor_types.end(); ++it) {
        if (ss.tellp()) ss << ", ";
        ss << string_VkDescriptorType(VkDescriptorType(*it));
    }
    return ss.str();
}

bool CoreChecks::RequirePropertyFlag(VkBool32 check, char const *flag, char const *structure) const {
    if (!check) {
        if (LogError(device, kVUID_Core_Shader_ExceedDeviceLimit,
                     "Shader requires flag %s set in %s but it is not set on the device", flag, structure)) {
            return true;
        }
    }

    return false;
}

bool CoreChecks::RequireFeature(VkBool32 feature, char const *feature_name) const {
    if (!feature) {
        if (LogError(device, kVUID_Core_Shader_FeatureNotEnabled, "Shader requires %s but is not enabled on the device",
                     feature_name)) {
            return true;
        }
    }

    return false;
}

bool CoreChecks::RequireExtension(bool extension, char const *extension_name) const {
    if (!extension) {
        if (LogError(device, kVUID_Core_Shader_FeatureNotEnabled, "Shader requires extension %s but is not enabled on the device",
                     extension_name)) {
            return true;
        }
    }

    return false;
}

bool CoreChecks::ValidateShaderCapabilities(SHADER_MODULE_STATE const *src, VkShaderStageFlagBits stage) const {
    bool skip = false;

    struct FeaturePointer {
        // Callable object to test if this feature is enabled in the given aggregate feature struct
        const std::function<VkBool32(const DeviceFeatures &)> IsEnabled;

        // Test if feature pointer is populated
        explicit operator bool() const { return static_cast<bool>(IsEnabled); }

        // Default and nullptr constructor to create an empty FeaturePointer
        FeaturePointer() : IsEnabled(nullptr) {}
        FeaturePointer(std::nullptr_t ptr) : IsEnabled(nullptr) {}

        // Constructors to populate FeaturePointer based on given pointer to member
        FeaturePointer(VkBool32 VkPhysicalDeviceFeatures::*ptr)
            : IsEnabled([=](const DeviceFeatures &features) { return features.core.*ptr; }) {}
        FeaturePointer(VkBool32 VkPhysicalDeviceVulkan11Features::*ptr)
            : IsEnabled([=](const DeviceFeatures &features) { return features.core11.*ptr; }) {}
        FeaturePointer(VkBool32 VkPhysicalDeviceVulkan12Features::*ptr)
            : IsEnabled([=](const DeviceFeatures &features) { return features.core12.*ptr; }) {}
        FeaturePointer(VkBool32 VkPhysicalDeviceTransformFeedbackFeaturesEXT::*ptr)
            : IsEnabled([=](const DeviceFeatures &features) { return features.transform_feedback_features.*ptr; }) {}
        FeaturePointer(VkBool32 VkPhysicalDeviceCooperativeMatrixFeaturesNV::*ptr)
            : IsEnabled([=](const DeviceFeatures &features) { return features.cooperative_matrix_features.*ptr; }) {}
        FeaturePointer(VkBool32 VkPhysicalDeviceComputeShaderDerivativesFeaturesNV::*ptr)
            : IsEnabled([=](const DeviceFeatures &features) { return features.compute_shader_derivatives_features.*ptr; }) {}
        FeaturePointer(VkBool32 VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV::*ptr)
            : IsEnabled([=](const DeviceFeatures &features) { return features.fragment_shader_barycentric_features.*ptr; }) {}
        FeaturePointer(VkBool32 VkPhysicalDeviceShaderImageFootprintFeaturesNV::*ptr)
            : IsEnabled([=](const DeviceFeatures &features) { return features.shader_image_footprint_features.*ptr; }) {}
        FeaturePointer(VkBool32 VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::*ptr)
            : IsEnabled([=](const DeviceFeatures &features) { return features.fragment_shader_interlock_features.*ptr; }) {}
        FeaturePointer(VkBool32 VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT::*ptr)
            : IsEnabled([=](const DeviceFeatures &features) { return features.demote_to_helper_invocation_features.*ptr; }) {}
        FeaturePointer(VkBool32 VkPhysicalDeviceRayTracingFeaturesKHR::*ptr)
            : IsEnabled([=](const DeviceFeatures &features) { return features.ray_tracing_features.*ptr; }) {}
    };

    struct CapabilityInfo {
        char const *name;
        FeaturePointer feature;
        ExtEnabled DeviceExtensions::*extension;
    };

    // clang-format off
    static const std::unordered_multimap<uint32_t, CapabilityInfo> capabilities = {
        // Capabilities always supported by a Vulkan 1.0 implementation -- no
        // feature bits.
        {spv::CapabilityMatrix, {nullptr}},
        {spv::CapabilityShader, {nullptr}},
        {spv::CapabilityInputAttachment, {nullptr}},
        {spv::CapabilitySampled1D, {nullptr}},
        {spv::CapabilityImage1D, {nullptr}},
        {spv::CapabilitySampledBuffer, {nullptr}},
        {spv::CapabilityStorageImageExtendedFormats, {nullptr}},
        {spv::CapabilityImageQuery, {nullptr}},
        {spv::CapabilityDerivativeControl, {nullptr}},

        // Capabilities that are optionally supported, but require a feature to
        // be enabled on the device
        {spv::CapabilityGeometry, {"VkPhysicalDeviceFeatures::geometryShader", &VkPhysicalDeviceFeatures::geometryShader}},
        {spv::CapabilityTessellation, {"VkPhysicalDeviceFeatures::tessellationShader", &VkPhysicalDeviceFeatures::tessellationShader}},
        {spv::CapabilityFloat64, {"VkPhysicalDeviceFeatures::shaderFloat64", &VkPhysicalDeviceFeatures::shaderFloat64}},
        {spv::CapabilityInt64, {"VkPhysicalDeviceFeatures::shaderInt64", &VkPhysicalDeviceFeatures::shaderInt64}},
        {spv::CapabilityTessellationPointSize, {"VkPhysicalDeviceFeatures::shaderTessellationAndGeometryPointSize", &VkPhysicalDeviceFeatures::shaderTessellationAndGeometryPointSize}},
        {spv::CapabilityGeometryPointSize, {"VkPhysicalDeviceFeatures::shaderTessellationAndGeometryPointSize", &VkPhysicalDeviceFeatures::shaderTessellationAndGeometryPointSize}},
        {spv::CapabilityImageGatherExtended, {"VkPhysicalDeviceFeatures::shaderImageGatherExtended", &VkPhysicalDeviceFeatures::shaderImageGatherExtended}},
        {spv::CapabilityStorageImageMultisample, {"VkPhysicalDeviceFeatures::shaderStorageImageMultisample", &VkPhysicalDeviceFeatures::shaderStorageImageMultisample}},
        {spv::CapabilityUniformBufferArrayDynamicIndexing, {"VkPhysicalDeviceFeatures::shaderUniformBufferArrayDynamicIndexing", &VkPhysicalDeviceFeatures::shaderUniformBufferArrayDynamicIndexing}},
        {spv::CapabilitySampledImageArrayDynamicIndexing, {"VkPhysicalDeviceFeatures::shaderSampledImageArrayDynamicIndexing", &VkPhysicalDeviceFeatures::shaderSampledImageArrayDynamicIndexing}},
        {spv::CapabilityStorageBufferArrayDynamicIndexing, {"VkPhysicalDeviceFeatures::shaderStorageBufferArrayDynamicIndexing", &VkPhysicalDeviceFeatures::shaderStorageBufferArrayDynamicIndexing}},
        {spv::CapabilityStorageImageArrayDynamicIndexing, {"VkPhysicalDeviceFeatures::shaderStorageImageArrayDynamicIndexing", &VkPhysicalDeviceFeatures::shaderStorageBufferArrayDynamicIndexing}},
        {spv::CapabilityClipDistance, {"VkPhysicalDeviceFeatures::shaderClipDistance", &VkPhysicalDeviceFeatures::shaderClipDistance}},
        {spv::CapabilityCullDistance, {"VkPhysicalDeviceFeatures::shaderCullDistance", &VkPhysicalDeviceFeatures::shaderCullDistance}},
        {spv::CapabilityImageCubeArray, {"VkPhysicalDeviceFeatures::imageCubeArray", &VkPhysicalDeviceFeatures::imageCubeArray}},
        {spv::CapabilitySampleRateShading, {"VkPhysicalDeviceFeatures::sampleRateShading", &VkPhysicalDeviceFeatures::sampleRateShading}},
        {spv::CapabilitySparseResidency, {"VkPhysicalDeviceFeatures::shaderResourceResidency", &VkPhysicalDeviceFeatures::shaderResourceResidency}},
        {spv::CapabilityMinLod, {"VkPhysicalDeviceFeatures::shaderResourceMinLod", &VkPhysicalDeviceFeatures::shaderResourceMinLod}},
        {spv::CapabilitySampledCubeArray, {"VkPhysicalDeviceFeatures::imageCubeArray", &VkPhysicalDeviceFeatures::imageCubeArray}},
        {spv::CapabilityImageMSArray, {"VkPhysicalDeviceFeatures::shaderStorageImageMultisample", &VkPhysicalDeviceFeatures::shaderStorageImageMultisample}},
        {spv::CapabilityInterpolationFunction, {"VkPhysicalDeviceFeatures::sampleRateShading", &VkPhysicalDeviceFeatures::sampleRateShading}},
        {spv::CapabilityStorageImageReadWithoutFormat, {"VkPhysicalDeviceFeatures::shaderStorageImageReadWithoutFormat", &VkPhysicalDeviceFeatures::shaderStorageImageReadWithoutFormat}},
        {spv::CapabilityStorageImageWriteWithoutFormat, {"VkPhysicalDeviceFeatures::shaderStorageImageWriteWithoutFormat", &VkPhysicalDeviceFeatures::shaderStorageImageWriteWithoutFormat}},
        {spv::CapabilityMultiViewport, {"VkPhysicalDeviceFeatures::multiViewport", &VkPhysicalDeviceFeatures::multiViewport}},

        {spv::CapabilityShaderNonUniformEXT, {VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME, nullptr, &DeviceExtensions::vk_ext_descriptor_indexing}},
        {spv::CapabilityRuntimeDescriptorArrayEXT, {"VkPhysicalDeviceDescriptorIndexingFeatures::runtimeDescriptorArray", &VkPhysicalDeviceVulkan12Features::runtimeDescriptorArray}},
        {spv::CapabilityInputAttachmentArrayDynamicIndexingEXT, {"VkPhysicalDeviceDescriptorIndexingFeatures::shaderInputAttachmentArrayDynamicIndexing", &VkPhysicalDeviceVulkan12Features::shaderInputAttachmentArrayDynamicIndexing}},
        {spv::CapabilityUniformTexelBufferArrayDynamicIndexingEXT, {"VkPhysicalDeviceDescriptorIndexingFeatures::shaderUniformTexelBufferArrayDynamicIndexing", &VkPhysicalDeviceVulkan12Features::shaderUniformTexelBufferArrayDynamicIndexing}},
        {spv::CapabilityStorageTexelBufferArrayDynamicIndexingEXT, {"VkPhysicalDeviceDescriptorIndexingFeatures::shaderStorageTexelBufferArrayDynamicIndexing", &VkPhysicalDeviceVulkan12Features::shaderStorageTexelBufferArrayDynamicIndexing}},
        {spv::CapabilityUniformBufferArrayNonUniformIndexingEXT, {"VkPhysicalDeviceDescriptorIndexingFeatures::shaderUniformBufferArrayNonUniformIndexing", &VkPhysicalDeviceVulkan12Features::shaderUniformBufferArrayNonUniformIndexing}},
        {spv::CapabilitySampledImageArrayNonUniformIndexingEXT, {"VkPhysicalDeviceDescriptorIndexingFeatures::shaderSampledImageArrayNonUniformIndexing", &VkPhysicalDeviceVulkan12Features::shaderSampledImageArrayNonUniformIndexing}},
        {spv::CapabilityStorageBufferArrayNonUniformIndexingEXT, {"VkPhysicalDeviceDescriptorIndexingFeatures::shaderStorageBufferArrayNonUniformIndexing", &VkPhysicalDeviceVulkan12Features::shaderStorageBufferArrayNonUniformIndexing}},
        {spv::CapabilityStorageImageArrayNonUniformIndexingEXT, {"VkPhysicalDeviceDescriptorIndexingFeatures::shaderStorageImageArrayNonUniformIndexing", &VkPhysicalDeviceVulkan12Features::shaderStorageImageArrayNonUniformIndexing}},
        {spv::CapabilityInputAttachmentArrayNonUniformIndexingEXT, {"VkPhysicalDeviceDescriptorIndexingFeatures::shaderInputAttachmentArrayNonUniformIndexing", &VkPhysicalDeviceVulkan12Features::shaderInputAttachmentArrayNonUniformIndexing}},
        {spv::CapabilityUniformTexelBufferArrayNonUniformIndexingEXT, {"VkPhysicalDeviceDescriptorIndexingFeatures::shaderUniformTexelBufferArrayNonUniformIndexing", &VkPhysicalDeviceVulkan12Features::shaderUniformTexelBufferArrayNonUniformIndexing}},
        {spv::CapabilityStorageTexelBufferArrayNonUniformIndexingEXT, {"VkPhysicalDeviceDescriptorIndexingFeatures::shaderStorageTexelBufferArrayNonUniformIndexing", &VkPhysicalDeviceVulkan12Features::shaderStorageTexelBufferArrayNonUniformIndexing}},

        // Capabilities that require an extension
        {spv::CapabilityDrawParameters, {VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME, nullptr, &DeviceExtensions::vk_khr_shader_draw_parameters}},
        {spv::CapabilityGeometryShaderPassthroughNV, {VK_NV_GEOMETRY_SHADER_PASSTHROUGH_EXTENSION_NAME, nullptr, &DeviceExtensions::vk_nv_geometry_shader_passthrough}},
        {spv::CapabilitySampleMaskOverrideCoverageNV, {VK_NV_SAMPLE_MASK_OVERRIDE_COVERAGE_EXTENSION_NAME, nullptr, &DeviceExtensions::vk_nv_sample_mask_override_coverage}},
        {spv::CapabilityShaderViewportIndexLayerEXT, {VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME, nullptr, &DeviceExtensions::vk_ext_shader_viewport_index_layer}},
        {spv::CapabilityShaderViewportIndexLayerNV, {VK_NV_VIEWPORT_ARRAY2_EXTENSION_NAME, nullptr, &DeviceExtensions::vk_nv_viewport_array2}},
        {spv::CapabilityShaderViewportMaskNV, {VK_NV_VIEWPORT_ARRAY2_EXTENSION_NAME, nullptr, &DeviceExtensions::vk_nv_viewport_array2}},
        {spv::CapabilitySubgroupBallotKHR, {VK_EXT_SHADER_SUBGROUP_BALLOT_EXTENSION_NAME, nullptr, &DeviceExtensions::vk_ext_shader_subgroup_ballot }},
        {spv::CapabilitySubgroupVoteKHR, {VK_EXT_SHADER_SUBGROUP_VOTE_EXTENSION_NAME, nullptr, &DeviceExtensions::vk_ext_shader_subgroup_vote }},
        {spv::CapabilityGroupNonUniformPartitionedNV, {VK_NV_SHADER_SUBGROUP_PARTITIONED_EXTENSION_NAME, nullptr, &DeviceExtensions::vk_nv_shader_subgroup_partitioned}},
        {spv::CapabilityInt64Atomics, {VK_KHR_SHADER_ATOMIC_INT64_EXTENSION_NAME, nullptr, &DeviceExtensions::vk_khr_shader_atomic_int64 }},
        {spv::CapabilityShaderClockKHR, {VK_KHR_SHADER_CLOCK_EXTENSION_NAME, nullptr, &DeviceExtensions::vk_khr_shader_clock }},

        {spv::CapabilityComputeDerivativeGroupQuadsNV, {"VkPhysicalDeviceComputeShaderDerivativesFeaturesNV::computeDerivativeGroupQuads", &VkPhysicalDeviceComputeShaderDerivativesFeaturesNV::computeDerivativeGroupQuads, &DeviceExtensions::vk_nv_compute_shader_derivatives}},
        {spv::CapabilityComputeDerivativeGroupLinearNV, {"VkPhysicalDeviceComputeShaderDerivativesFeaturesNV::computeDerivativeGroupLinear", &VkPhysicalDeviceComputeShaderDerivativesFeaturesNV::computeDerivativeGroupLinear, &DeviceExtensions::vk_nv_compute_shader_derivatives}},
        {spv::CapabilityFragmentBarycentricNV, {"VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV::fragmentShaderBarycentric", &VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV::fragmentShaderBarycentric, &DeviceExtensions::vk_nv_fragment_shader_barycentric}},

        {spv::CapabilityStorageBuffer8BitAccess, {"VkPhysicalDevice8BitStorageFeaturesKHR::storageBuffer8BitAccess", &VkPhysicalDeviceVulkan12Features::storageBuffer8BitAccess, &DeviceExtensions::vk_khr_8bit_storage}},
        {spv::CapabilityUniformAndStorageBuffer8BitAccess, {"VkPhysicalDevice8BitStorageFeaturesKHR::uniformAndStorageBuffer8BitAccess", &VkPhysicalDeviceVulkan12Features::uniformAndStorageBuffer8BitAccess, &DeviceExtensions::vk_khr_8bit_storage}},
        {spv::CapabilityStoragePushConstant8, {"VkPhysicalDevice8BitStorageFeaturesKHR::storagePushConstant8", &VkPhysicalDeviceVulkan12Features::storagePushConstant8, &DeviceExtensions::vk_khr_8bit_storage}},

        {spv::CapabilityTransformFeedback, { "VkPhysicalDeviceTransformFeedbackFeaturesEXT::transformFeedback", &VkPhysicalDeviceTransformFeedbackFeaturesEXT::transformFeedback, &DeviceExtensions::vk_ext_transform_feedback}},
        {spv::CapabilityGeometryStreams, { "VkPhysicalDeviceTransformFeedbackFeaturesEXT::geometryStreams", &VkPhysicalDeviceTransformFeedbackFeaturesEXT::geometryStreams, &DeviceExtensions::vk_ext_transform_feedback}},

        {spv::CapabilityFloat16, {"VkPhysicalDeviceFloat16Int8FeaturesKHR::shaderFloat16", &VkPhysicalDeviceVulkan12Features::shaderFloat16, &DeviceExtensions::vk_khr_shader_float16_int8}},
        {spv::CapabilityInt8, {"VkPhysicalDeviceFloat16Int8FeaturesKHR::shaderInt8", &VkPhysicalDeviceVulkan12Features::shaderInt8, &DeviceExtensions::vk_khr_shader_float16_int8}},

        {spv::CapabilityImageFootprintNV, {"VkPhysicalDeviceShaderImageFootprintFeaturesNV::imageFootprint", &VkPhysicalDeviceShaderImageFootprintFeaturesNV::imageFootprint, &DeviceExtensions::vk_nv_shader_image_footprint}},

        {spv::CapabilityCooperativeMatrixNV, {"VkPhysicalDeviceCooperativeMatrixFeaturesNV::cooperativeMatrix", &VkPhysicalDeviceCooperativeMatrixFeaturesNV::cooperativeMatrix, &DeviceExtensions::vk_nv_cooperative_matrix}},

        {spv::CapabilitySignedZeroInfNanPreserve, {"VkPhysicalDeviceFloatControlsPropertiesKHR::shaderSignedZeroInfNanPreserve", nullptr, &DeviceExtensions::vk_khr_shader_float_controls}},
        {spv::CapabilityDenormPreserve, {"VkPhysicalDeviceFloatControlsPropertiesKHR::shaderDenormPreserve", nullptr, &DeviceExtensions::vk_khr_shader_float_controls}},
        {spv::CapabilityDenormFlushToZero, {"VkPhysicalDeviceFloatControlsPropertiesKHR::shaderDenormFlushToZero", nullptr, &DeviceExtensions::vk_khr_shader_float_controls}},
        {spv::CapabilityRoundingModeRTE, {"VkPhysicalDeviceFloatControlsPropertiesKHR::shaderRoundingModeRTE", nullptr, &DeviceExtensions::vk_khr_shader_float_controls}},
        {spv::CapabilityRoundingModeRTZ, {"VkPhysicalDeviceFloatControlsPropertiesKHR::shaderRoundingModeRTZ", nullptr, &DeviceExtensions::vk_khr_shader_float_controls}},

        {spv::CapabilityFragmentShaderSampleInterlockEXT,       {"VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::fragmentShaderSampleInterlock",       &VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::fragmentShaderSampleInterlock,         &DeviceExtensions::vk_ext_fragment_shader_interlock}},
        {spv::CapabilityFragmentShaderPixelInterlockEXT,        {"VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::fragmentShaderPixelInterlock",        &VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::fragmentShaderPixelInterlock,          &DeviceExtensions::vk_ext_fragment_shader_interlock}},
        {spv::CapabilityFragmentShaderShadingRateInterlockEXT,  {"VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::fragmentShaderShadingRateInterlock",  &VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::fragmentShaderShadingRateInterlock,    &DeviceExtensions::vk_ext_fragment_shader_interlock}},
        {spv::CapabilityDemoteToHelperInvocationEXT,       {"VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT::shaderDemoteToHelperInvocation",       &VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT::shaderDemoteToHelperInvocation,         &DeviceExtensions::vk_ext_shader_demote_to_helper_invocation}},

        {spv::CapabilityPhysicalStorageBufferAddresses, {"VkPhysicalDeviceBufferDeviceAddressFeatures::bufferDeviceAddress", &VkPhysicalDeviceVulkan12Features::bufferDeviceAddress, &DeviceExtensions::vk_ext_buffer_device_address}},
        // Should be non-EXT token, but Android SPIRV-Headers are out of date, and the token value is the same anyway
        {spv::CapabilityPhysicalStorageBufferAddressesEXT, {"VkPhysicalDeviceBufferDeviceAddressFeaturesEXT::bufferDeviceAddress", &VkPhysicalDeviceVulkan12Features::bufferDeviceAddress, &DeviceExtensions::vk_khr_buffer_device_address}},

        {spv::CapabilityRayTracingProvisionalKHR, {"VkPhysicalDeviceRayTracingFeaturesKHR::rayTracing", &VkPhysicalDeviceRayTracingFeaturesKHR::rayTracing, &DeviceExtensions::vk_khr_ray_tracing}},
        {spv::CapabilityRayQueryProvisionalKHR, {"VkPhysicalDeviceRayTracingFeaturesKHR::rayQuery", &VkPhysicalDeviceRayTracingFeaturesKHR::rayQuery, &DeviceExtensions::vk_khr_ray_tracing}},
        {spv::CapabilityRayTraversalPrimitiveCullingProvisionalKHR, {"VkPhysicalDeviceRayTracingFeaturesKHR::rayTracingPrimitiveCulling", &VkPhysicalDeviceRayTracingFeaturesKHR::rayTracingPrimitiveCulling, &DeviceExtensions::vk_khr_ray_tracing}},
    };
    // clang-format on

    for (auto insn : *src) {
        if (insn.opcode() == spv::OpCapability) {
            size_t n = capabilities.count(insn.word(1));
            if (1 == n) {  // key occurs exactly once
                auto it = capabilities.find(insn.word(1));
                if (it != capabilities.end()) {
                    if (it->second.feature) {
                        skip |= RequireFeature(it->second.feature.IsEnabled(enabled_features), it->second.name);
                    }
                    if (it->second.extension) {
                        skip |= RequireExtension(IsExtEnabled((device_extensions.*(it->second.extension))), it->second.name);
                    }
                }
            } else if (1 < n) {  // key occurs multiple times, at least one must be enabled
                bool needs_feature = false, has_feature = false;
                bool needs_ext = false, has_ext = false;
                std::string feature_names = "(one of) [ ";
                std::string extension_names = feature_names;
                auto caps = capabilities.equal_range(insn.word(1));
                for (auto it = caps.first; it != caps.second; ++it) {
                    if (it->second.feature) {
                        needs_feature = true;
                        has_feature = has_feature || it->second.feature.IsEnabled(enabled_features);
                        feature_names += it->second.name;
                        feature_names += " ";
                    }
                    if (it->second.extension) {
                        needs_ext = true;
                        has_ext = has_ext || device_extensions.*(it->second.extension);
                        extension_names += it->second.name;
                        extension_names += " ";
                    }
                }
                if (needs_feature) {
                    feature_names += "]";
                    skip |= RequireFeature(has_feature, feature_names.c_str());
                }
                if (needs_ext) {
                    extension_names += "]";
                    skip |= RequireExtension(has_ext, extension_names.c_str());
                }
            }

            {  // Do group non-uniform checks
                const VkSubgroupFeatureFlags supportedOperations = phys_dev_props_core11.subgroupSupportedOperations;
                const VkSubgroupFeatureFlags supportedStages = phys_dev_props_core11.subgroupSupportedStages;

                switch (insn.word(1)) {
                    default:
                        break;
                    case spv::CapabilityGroupNonUniform:
                    case spv::CapabilityGroupNonUniformVote:
                    case spv::CapabilityGroupNonUniformArithmetic:
                    case spv::CapabilityGroupNonUniformBallot:
                    case spv::CapabilityGroupNonUniformShuffle:
                    case spv::CapabilityGroupNonUniformShuffleRelative:
                    case spv::CapabilityGroupNonUniformClustered:
                    case spv::CapabilityGroupNonUniformQuad:
                    case spv::CapabilityGroupNonUniformPartitionedNV:
                        RequirePropertyFlag(supportedStages & stage, string_VkShaderStageFlagBits(stage),
                                            "VkPhysicalDeviceSubgroupProperties::supportedStages");
                        break;
                }

                switch (insn.word(1)) {
                    default:
                        break;
                    case spv::CapabilityGroupNonUniform:
                        RequirePropertyFlag(supportedOperations & VK_SUBGROUP_FEATURE_BASIC_BIT, "VK_SUBGROUP_FEATURE_BASIC_BIT",
                                            "VkPhysicalDeviceSubgroupProperties::supportedOperations");
                        break;
                    case spv::CapabilityGroupNonUniformVote:
                        RequirePropertyFlag(supportedOperations & VK_SUBGROUP_FEATURE_VOTE_BIT, "VK_SUBGROUP_FEATURE_VOTE_BIT",
                                            "VkPhysicalDeviceSubgroupProperties::supportedOperations");
                        break;
                    case spv::CapabilityGroupNonUniformArithmetic:
                        RequirePropertyFlag(supportedOperations & VK_SUBGROUP_FEATURE_ARITHMETIC_BIT,
                                            "VK_SUBGROUP_FEATURE_ARITHMETIC_BIT",
                                            "VkPhysicalDeviceSubgroupProperties::supportedOperations");
                        break;
                    case spv::CapabilityGroupNonUniformBallot:
                        RequirePropertyFlag(supportedOperations & VK_SUBGROUP_FEATURE_BALLOT_BIT, "VK_SUBGROUP_FEATURE_BALLOT_BIT",
                                            "VkPhysicalDeviceSubgroupProperties::supportedOperations");
                        break;
                    case spv::CapabilityGroupNonUniformShuffle:
                        RequirePropertyFlag(supportedOperations & VK_SUBGROUP_FEATURE_SHUFFLE_BIT,
                                            "VK_SUBGROUP_FEATURE_SHUFFLE_BIT",
                                            "VkPhysicalDeviceSubgroupProperties::supportedOperations");
                        break;
                    case spv::CapabilityGroupNonUniformShuffleRelative:
                        RequirePropertyFlag(supportedOperations & VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT,
                                            "VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT",
                                            "VkPhysicalDeviceSubgroupProperties::supportedOperations");
                        break;
                    case spv::CapabilityGroupNonUniformClustered:
                        RequirePropertyFlag(supportedOperations & VK_SUBGROUP_FEATURE_CLUSTERED_BIT,
                                            "VK_SUBGROUP_FEATURE_CLUSTERED_BIT",
                                            "VkPhysicalDeviceSubgroupProperties::supportedOperations");
                        break;
                    case spv::CapabilityGroupNonUniformQuad:
                        RequirePropertyFlag(supportedOperations & VK_SUBGROUP_FEATURE_QUAD_BIT, "VK_SUBGROUP_FEATURE_QUAD_BIT",
                                            "VkPhysicalDeviceSubgroupProperties::supportedOperations");
                        break;
                    case spv::CapabilityGroupNonUniformPartitionedNV:
                        RequirePropertyFlag(supportedOperations & VK_SUBGROUP_FEATURE_PARTITIONED_BIT_NV,
                                            "VK_SUBGROUP_FEATURE_PARTITIONED_BIT_NV",
                                            "VkPhysicalDeviceSubgroupProperties::supportedOperations");
                        break;
                }
            }
        } else if (insn.opcode() == spv::OpExtension) {
            std::string extension_name = (char const *)&insn.word(1);

            if (extension_name == "SPV_KHR_non_semantic_info") {
                skip |= RequireExtension(IsExtEnabled(device_extensions.vk_khr_shader_non_semantic_info),
                                         VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateShaderStageWritableOrAtomicDescriptor(VkShaderStageFlagBits stage, bool has_writable_descriptor,
                                                               bool has_atomic_descriptor) const {
    bool skip = false;

    if (has_writable_descriptor || has_atomic_descriptor) {
        switch (stage) {
            case VK_SHADER_STAGE_COMPUTE_BIT:
            case VK_SHADER_STAGE_RAYGEN_BIT_NV:
            case VK_SHADER_STAGE_ANY_HIT_BIT_NV:
            case VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV:
            case VK_SHADER_STAGE_MISS_BIT_NV:
            case VK_SHADER_STAGE_INTERSECTION_BIT_NV:
            case VK_SHADER_STAGE_CALLABLE_BIT_NV:
            case VK_SHADER_STAGE_TASK_BIT_NV:
            case VK_SHADER_STAGE_MESH_BIT_NV:
                /* No feature requirements for writes and atomics from compute
                 * raytracing, or mesh stages */
                break;
            case VK_SHADER_STAGE_FRAGMENT_BIT:
                skip |= RequireFeature(enabled_features.core.fragmentStoresAndAtomics, "fragmentStoresAndAtomics");
                break;
            default:
                skip |= RequireFeature(enabled_features.core.vertexPipelineStoresAndAtomics, "vertexPipelineStoresAndAtomics");
                break;
        }
    }

    return skip;
}

bool CoreChecks::ValidateShaderStageGroupNonUniform(SHADER_MODULE_STATE const *module, VkShaderStageFlagBits stage) const {
    bool skip = false;

    auto const subgroup_props = phys_dev_props_core11;

    for (auto inst : *module) {
        // Check the quad operations.
        switch (inst.opcode()) {
            default:
                break;
            case spv::OpGroupNonUniformQuadBroadcast:
            case spv::OpGroupNonUniformQuadSwap:
                if ((stage != VK_SHADER_STAGE_FRAGMENT_BIT) && (stage != VK_SHADER_STAGE_COMPUTE_BIT)) {
                    skip |= RequireFeature(subgroup_props.subgroupQuadOperationsInAllStages,
                                           "VkPhysicalDeviceSubgroupProperties::quadOperationsInAllStages");
                }
                break;
        }

        if (!enabled_features.core12.shaderSubgroupExtendedTypes) {
            switch (inst.opcode()) {
                default:
                    break;
                case spv::OpGroupNonUniformAllEqual:
                case spv::OpGroupNonUniformBroadcast:
                case spv::OpGroupNonUniformBroadcastFirst:
                case spv::OpGroupNonUniformShuffle:
                case spv::OpGroupNonUniformShuffleXor:
                case spv::OpGroupNonUniformShuffleUp:
                case spv::OpGroupNonUniformShuffleDown:
                case spv::OpGroupNonUniformIAdd:
                case spv::OpGroupNonUniformFAdd:
                case spv::OpGroupNonUniformIMul:
                case spv::OpGroupNonUniformFMul:
                case spv::OpGroupNonUniformSMin:
                case spv::OpGroupNonUniformUMin:
                case spv::OpGroupNonUniformFMin:
                case spv::OpGroupNonUniformSMax:
                case spv::OpGroupNonUniformUMax:
                case spv::OpGroupNonUniformFMax:
                case spv::OpGroupNonUniformBitwiseAnd:
                case spv::OpGroupNonUniformBitwiseOr:
                case spv::OpGroupNonUniformBitwiseXor:
                case spv::OpGroupNonUniformLogicalAnd:
                case spv::OpGroupNonUniformLogicalOr:
                case spv::OpGroupNonUniformLogicalXor:
                case spv::OpGroupNonUniformQuadBroadcast:
                case spv::OpGroupNonUniformQuadSwap: {
                    auto type = module->get_def(inst.word(1));

                    if (type.opcode() == spv::OpTypeVector) {
                        // Get the element type
                        type = module->get_def(type.word(2));
                    }

                    if (type.opcode() == spv::OpTypeBool) {
                        break;
                    }

                    // Both OpTypeInt and OpTypeFloat the width is in the 2nd word.
                    const uint32_t width = type.word(2);

                    if ((type.opcode() == spv::OpTypeFloat && width == 16) ||
                        (type.opcode() == spv::OpTypeInt && (width == 8 || width == 16 || width == 64))) {
                        skip |= RequireFeature(enabled_features.core12.shaderSubgroupExtendedTypes,
                                               "VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures::shaderSubgroupExtendedTypes");
                    }
                    break;
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateShaderStageInputOutputLimits(SHADER_MODULE_STATE const *src, VkPipelineShaderStageCreateInfo const *pStage,
                                                      const PIPELINE_STATE *pipeline, spirv_inst_iter entrypoint) const {
    if (pStage->stage == VK_SHADER_STAGE_COMPUTE_BIT || pStage->stage == VK_SHADER_STAGE_ALL_GRAPHICS ||
        pStage->stage == VK_SHADER_STAGE_ALL) {
        return false;
    }

    bool skip = false;
    auto const &limits = phys_dev_props.limits;

    std::set<uint32_t> patchIDs;
    struct Variable {
        uint32_t baseTypePtrID;
        uint32_t ID;
        uint32_t storageClass;
    };
    std::vector<Variable> variables;

    uint32_t numVertices = 0;

    auto entrypointVariables = FindEntrypointInterfaces(entrypoint);

    for (auto insn : *src) {
        switch (insn.opcode()) {
            // Find all Patch decorations
            case spv::OpDecorate:
                switch (insn.word(2)) {
                    case spv::DecorationPatch: {
                        patchIDs.insert(insn.word(1));
                        break;
                    }
                    default:
                        break;
                }
                break;
            // Find all input and output variables
            case spv::OpVariable: {
                Variable var = {};
                var.storageClass = insn.word(3);
                if ((var.storageClass == spv::StorageClassInput || var.storageClass == spv::StorageClassOutput) &&
                    // Only include variables in the entrypoint's interface
                    find(entrypointVariables.begin(), entrypointVariables.end(), insn.word(2)) != entrypointVariables.end()) {
                    var.baseTypePtrID = insn.word(1);
                    var.ID = insn.word(2);
                    variables.push_back(var);
                }
                break;
            }
            case spv::OpExecutionMode:
                if (insn.word(1) == entrypoint.word(2)) {
                    switch (insn.word(2)) {
                        default:
                            break;
                        case spv::ExecutionModeOutputVertices:
                            numVertices = insn.word(3);
                            break;
                    }
                }
                break;
            default:
                break;
        }
    }

    bool strip_output_array_level =
        (pStage->stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT || pStage->stage == VK_SHADER_STAGE_MESH_BIT_NV);
    bool strip_input_array_level =
        (pStage->stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT ||
         pStage->stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT || pStage->stage == VK_SHADER_STAGE_GEOMETRY_BIT);

    uint32_t numCompIn = 0, numCompOut = 0;
    int maxCompIn = 0, maxCompOut = 0;

    auto inputs = CollectInterfaceByLocation(src, entrypoint, spv::StorageClassInput, strip_input_array_level);
    auto outputs = CollectInterfaceByLocation(src, entrypoint, spv::StorageClassOutput, strip_output_array_level);

    // Find max component location used for input variables.
    for (auto &var : inputs) {
        int location = var.first.first;
        int component = var.first.second;
        interface_var &iv = var.second;

        // Only need to look at the first location, since we use the type's whole size
        if (iv.offset != 0) {
            continue;
        }

        if (iv.is_patch) {
            continue;
        }

        int numComponents = GetComponentsConsumedByType(src, iv.type_id, strip_input_array_level);
        maxCompIn = std::max(maxCompIn, location * 4 + component + numComponents);
    }

    // Find max component location used for output variables.
    for (auto &var : outputs) {
        int location = var.first.first;
        int component = var.first.second;
        interface_var &iv = var.second;

        // Only need to look at the first location, since we use the type's whole size
        if (iv.offset != 0) {
            continue;
        }

        if (iv.is_patch) {
            continue;
        }

        int numComponents = GetComponentsConsumedByType(src, iv.type_id, strip_output_array_level);
        maxCompOut = std::max(maxCompOut, location * 4 + component + numComponents);
    }

    // XXX TODO: Would be nice to rewrite this to use CollectInterfaceByLocation (or something similar),
    // but that doesn't include builtins.
    for (auto &var : variables) {
        // Check if the variable is a patch. Patches can also be members of blocks,
        // but if they are then the top-level arrayness has already been stripped
        // by the time GetComponentsConsumedByType gets to it.
        bool isPatch = patchIDs.find(var.ID) != patchIDs.end();

        if (var.storageClass == spv::StorageClassInput) {
            numCompIn += GetComponentsConsumedByType(src, var.baseTypePtrID, strip_input_array_level && !isPatch);
        } else {  // var.storageClass == spv::StorageClassOutput
            numCompOut += GetComponentsConsumedByType(src, var.baseTypePtrID, strip_output_array_level && !isPatch);
        }
    }

    switch (pStage->stage) {
        case VK_SHADER_STAGE_VERTEX_BIT:
            if (numCompOut > limits.maxVertexOutputComponents) {
                skip |= LogError(pipeline->pipeline, kVUID_Core_Shader_ExceedDeviceLimit,
                                 "Invalid Pipeline CreateInfo State: Vertex shader exceeds "
                                 "VkPhysicalDeviceLimits::maxVertexOutputComponents of %u "
                                 "components by %u components",
                                 limits.maxVertexOutputComponents, numCompOut - limits.maxVertexOutputComponents);
            }
            if (maxCompOut > (int)limits.maxVertexOutputComponents) {
                skip |= LogError(pipeline->pipeline, kVUID_Core_Shader_ExceedDeviceLimit,
                                 "Invalid Pipeline CreateInfo State: Vertex shader output variable uses location that "
                                 "exceeds component limit VkPhysicalDeviceLimits::maxVertexOutputComponents (%u)",
                                 limits.maxVertexOutputComponents);
            }
            break;

        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
            if (numCompIn > limits.maxTessellationControlPerVertexInputComponents) {
                skip |= LogError(pipeline->pipeline, kVUID_Core_Shader_ExceedDeviceLimit,
                                 "Invalid Pipeline CreateInfo State: Tessellation control shader exceeds "
                                 "VkPhysicalDeviceLimits::maxTessellationControlPerVertexInputComponents of %u "
                                 "components by %u components",
                                 limits.maxTessellationControlPerVertexInputComponents,
                                 numCompIn - limits.maxTessellationControlPerVertexInputComponents);
            }
            if (maxCompIn > (int)limits.maxTessellationControlPerVertexInputComponents) {
                skip |=
                    LogError(pipeline->pipeline, kVUID_Core_Shader_ExceedDeviceLimit,
                             "Invalid Pipeline CreateInfo State: Tessellation control shader input variable uses location that "
                             "exceeds component limit VkPhysicalDeviceLimits::maxTessellationControlPerVertexInputComponents (%u)",
                             limits.maxTessellationControlPerVertexInputComponents);
            }
            if (numCompOut > limits.maxTessellationControlPerVertexOutputComponents) {
                skip |= LogError(pipeline->pipeline, kVUID_Core_Shader_ExceedDeviceLimit,
                                 "Invalid Pipeline CreateInfo State: Tessellation control shader exceeds "
                                 "VkPhysicalDeviceLimits::maxTessellationControlPerVertexOutputComponents of %u "
                                 "components by %u components",
                                 limits.maxTessellationControlPerVertexOutputComponents,
                                 numCompOut - limits.maxTessellationControlPerVertexOutputComponents);
            }
            if (maxCompOut > (int)limits.maxTessellationControlPerVertexOutputComponents) {
                skip |=
                    LogError(pipeline->pipeline, kVUID_Core_Shader_ExceedDeviceLimit,
                             "Invalid Pipeline CreateInfo State: Tessellation control shader output variable uses location that "
                             "exceeds component limit VkPhysicalDeviceLimits::maxTessellationControlPerVertexOutputComponents (%u)",
                             limits.maxTessellationControlPerVertexOutputComponents);
            }
            break;

        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            if (numCompIn > limits.maxTessellationEvaluationInputComponents) {
                skip |= LogError(pipeline->pipeline, kVUID_Core_Shader_ExceedDeviceLimit,
                                 "Invalid Pipeline CreateInfo State: Tessellation evaluation shader exceeds "
                                 "VkPhysicalDeviceLimits::maxTessellationEvaluationInputComponents of %u "
                                 "components by %u components",
                                 limits.maxTessellationEvaluationInputComponents,
                                 numCompIn - limits.maxTessellationEvaluationInputComponents);
            }
            if (maxCompIn > (int)limits.maxTessellationEvaluationInputComponents) {
                skip |=
                    LogError(pipeline->pipeline, kVUID_Core_Shader_ExceedDeviceLimit,
                             "Invalid Pipeline CreateInfo State: Tessellation evaluation shader input variable uses location that "
                             "exceeds component limit VkPhysicalDeviceLimits::maxTessellationEvaluationInputComponents (%u)",
                             limits.maxTessellationEvaluationInputComponents);
            }
            if (numCompOut > limits.maxTessellationEvaluationOutputComponents) {
                skip |= LogError(pipeline->pipeline, kVUID_Core_Shader_ExceedDeviceLimit,
                                 "Invalid Pipeline CreateInfo State: Tessellation evaluation shader exceeds "
                                 "VkPhysicalDeviceLimits::maxTessellationEvaluationOutputComponents of %u "
                                 "components by %u components",
                                 limits.maxTessellationEvaluationOutputComponents,
                                 numCompOut - limits.maxTessellationEvaluationOutputComponents);
            }
            if (maxCompOut > (int)limits.maxTessellationEvaluationOutputComponents) {
                skip |=
                    LogError(pipeline->pipeline, kVUID_Core_Shader_ExceedDeviceLimit,
                             "Invalid Pipeline CreateInfo State: Tessellation evaluation shader output variable uses location that "
                             "exceeds component limit VkPhysicalDeviceLimits::maxTessellationEvaluationOutputComponents (%u)",
                             limits.maxTessellationEvaluationOutputComponents);
            }
            break;

        case VK_SHADER_STAGE_GEOMETRY_BIT:
            if (numCompIn > limits.maxGeometryInputComponents) {
                skip |= LogError(pipeline->pipeline, kVUID_Core_Shader_ExceedDeviceLimit,
                                 "Invalid Pipeline CreateInfo State: Geometry shader exceeds "
                                 "VkPhysicalDeviceLimits::maxGeometryInputComponents of %u "
                                 "components by %u components",
                                 limits.maxGeometryInputComponents, numCompIn - limits.maxGeometryInputComponents);
            }
            if (maxCompIn > (int)limits.maxGeometryInputComponents) {
                skip |= LogError(pipeline->pipeline, kVUID_Core_Shader_ExceedDeviceLimit,
                                 "Invalid Pipeline CreateInfo State: Geometry shader input variable uses location that "
                                 "exceeds component limit VkPhysicalDeviceLimits::maxGeometryInputComponents (%u)",
                                 limits.maxGeometryInputComponents);
            }
            if (numCompOut > limits.maxGeometryOutputComponents) {
                skip |= LogError(pipeline->pipeline, kVUID_Core_Shader_ExceedDeviceLimit,
                                 "Invalid Pipeline CreateInfo State: Geometry shader exceeds "
                                 "VkPhysicalDeviceLimits::maxGeometryOutputComponents of %u "
                                 "components by %u components",
                                 limits.maxGeometryOutputComponents, numCompOut - limits.maxGeometryOutputComponents);
            }
            if (maxCompOut > (int)limits.maxGeometryOutputComponents) {
                skip |= LogError(pipeline->pipeline, kVUID_Core_Shader_ExceedDeviceLimit,
                                 "Invalid Pipeline CreateInfo State: Geometry shader output variable uses location that "
                                 "exceeds component limit VkPhysicalDeviceLimits::maxGeometryOutputComponents (%u)",
                                 limits.maxGeometryOutputComponents);
            }
            if (numCompOut * numVertices > limits.maxGeometryTotalOutputComponents) {
                skip |= LogError(pipeline->pipeline, kVUID_Core_Shader_ExceedDeviceLimit,
                                 "Invalid Pipeline CreateInfo State: Geometry shader exceeds "
                                 "VkPhysicalDeviceLimits::maxGeometryTotalOutputComponents of %u "
                                 "components by %u components",
                                 limits.maxGeometryTotalOutputComponents,
                                 numCompOut * numVertices - limits.maxGeometryTotalOutputComponents);
            }
            break;

        case VK_SHADER_STAGE_FRAGMENT_BIT:
            if (numCompIn > limits.maxFragmentInputComponents) {
                skip |= LogError(pipeline->pipeline, kVUID_Core_Shader_ExceedDeviceLimit,
                                 "Invalid Pipeline CreateInfo State: Fragment shader exceeds "
                                 "VkPhysicalDeviceLimits::maxFragmentInputComponents of %u "
                                 "components by %u components",
                                 limits.maxFragmentInputComponents, numCompIn - limits.maxFragmentInputComponents);
            }
            if (maxCompIn > (int)limits.maxFragmentInputComponents) {
                skip |= LogError(pipeline->pipeline, kVUID_Core_Shader_ExceedDeviceLimit,
                                 "Invalid Pipeline CreateInfo State: Fragment shader input variable uses location that "
                                 "exceeds component limit VkPhysicalDeviceLimits::maxFragmentInputComponents (%u)",
                                 limits.maxFragmentInputComponents);
            }
            break;

        case VK_SHADER_STAGE_RAYGEN_BIT_NV:
        case VK_SHADER_STAGE_ANY_HIT_BIT_NV:
        case VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV:
        case VK_SHADER_STAGE_MISS_BIT_NV:
        case VK_SHADER_STAGE_INTERSECTION_BIT_NV:
        case VK_SHADER_STAGE_CALLABLE_BIT_NV:
        case VK_SHADER_STAGE_TASK_BIT_NV:
        case VK_SHADER_STAGE_MESH_BIT_NV:
            break;

        default:
            assert(false);  // This should never happen
    }
    return skip;
}

bool CoreChecks::ValidateShaderStageMaxResources(VkShaderStageFlagBits stage, const PIPELINE_STATE *pipeline) const {
    bool skip = false;
    uint32_t total_resources = 0;

    // Only currently testing for graphics and compute pipelines
    // TODO: Add check and support for Ray Tracing pipeline VUID 03428
    if ((stage & (VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT)) == 0) {
        return false;
    }

    if (stage == VK_SHADER_STAGE_FRAGMENT_BIT) {
        // "For the fragment shader stage the framebuffer color attachments also count against this limit"
        total_resources += pipeline->rp_state->createInfo.pSubpasses[pipeline->graphicsPipelineCI.subpass].colorAttachmentCount;
    }

    // TODO: This reuses a lot of GetDescriptorCountMaxPerStage but currently would need to make it agnostic in a way to handle
    // input from CreatePipeline and CreatePipelineLayout level
    for (auto set_layout : pipeline->pipeline_layout->set_layouts) {
        if ((set_layout->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT) != 0) {
            continue;
        }

        for (uint32_t binding_idx = 0; binding_idx < set_layout->GetBindingCount(); binding_idx++) {
            const VkDescriptorSetLayoutBinding *binding = set_layout->GetDescriptorSetLayoutBindingPtrFromIndex(binding_idx);
            // Bindings with a descriptorCount of 0 are "reserved" and should be skipped
            if (((stage & binding->stageFlags) != 0) && (binding->descriptorCount > 0)) {
                // Check only descriptor types listed in maxPerStageResources description in spec
                switch (binding->descriptorType) {
                    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                        total_resources += binding->descriptorCount;
                        break;
                    default:
                        break;
                }
            }
        }
    }

    if (total_resources > phys_dev_props.limits.maxPerStageResources) {
        const char *vuid = (stage == VK_SHADER_STAGE_COMPUTE_BIT) ? "VUID-VkComputePipelineCreateInfo-layout-01687"
                                                                  : "VUID-VkGraphicsPipelineCreateInfo-layout-01688";
        skip |= LogError(pipeline->pipeline, vuid,
                         "Invalid Pipeline CreateInfo State: Shader Stage %s exceeds component limit "
                         "VkPhysicalDeviceLimits::maxPerStageResources (%u)",
                         string_VkShaderStageFlagBits(stage), phys_dev_props.limits.maxPerStageResources);
    }

    return skip;
}

// copy the specialization constant value into buf, if it is present
void GetSpecConstantValue(VkPipelineShaderStageCreateInfo const *pStage, uint32_t spec_id, void *buf) {
    VkSpecializationInfo const *spec = pStage->pSpecializationInfo;

    if (spec && spec_id < spec->mapEntryCount) {
        memcpy(buf, (uint8_t *)spec->pData + spec->pMapEntries[spec_id].offset, spec->pMapEntries[spec_id].size);
    }
}

// Fill in value with the constant or specialization constant value, if available.
// Returns true if the value has been accurately filled out.
static bool GetIntConstantValue(spirv_inst_iter insn, SHADER_MODULE_STATE const *src, VkPipelineShaderStageCreateInfo const *pStage,
                                const std::unordered_map<uint32_t, uint32_t> &id_to_spec_id, uint32_t *value) {
    auto type_id = src->get_def(insn.word(1));
    if (type_id.opcode() != spv::OpTypeInt || type_id.word(2) != 32) {
        return false;
    }
    switch (insn.opcode()) {
        case spv::OpSpecConstant:
            *value = insn.word(3);
            GetSpecConstantValue(pStage, id_to_spec_id.at(insn.word(2)), value);
            return true;
        case spv::OpConstant:
            *value = insn.word(3);
            return true;
        default:
            return false;
    }
}

// Map SPIR-V type to VK_COMPONENT_TYPE enum
VkComponentTypeNV GetComponentType(spirv_inst_iter insn, SHADER_MODULE_STATE const *src) {
    switch (insn.opcode()) {
        case spv::OpTypeInt:
            switch (insn.word(2)) {
                case 8:
                    return insn.word(3) != 0 ? VK_COMPONENT_TYPE_SINT8_NV : VK_COMPONENT_TYPE_UINT8_NV;
                case 16:
                    return insn.word(3) != 0 ? VK_COMPONENT_TYPE_SINT16_NV : VK_COMPONENT_TYPE_UINT16_NV;
                case 32:
                    return insn.word(3) != 0 ? VK_COMPONENT_TYPE_SINT32_NV : VK_COMPONENT_TYPE_UINT32_NV;
                case 64:
                    return insn.word(3) != 0 ? VK_COMPONENT_TYPE_SINT64_NV : VK_COMPONENT_TYPE_UINT64_NV;
                default:
                    return VK_COMPONENT_TYPE_MAX_ENUM_NV;
            }
        case spv::OpTypeFloat:
            switch (insn.word(2)) {
                case 16:
                    return VK_COMPONENT_TYPE_FLOAT16_NV;
                case 32:
                    return VK_COMPONENT_TYPE_FLOAT32_NV;
                case 64:
                    return VK_COMPONENT_TYPE_FLOAT64_NV;
                default:
                    return VK_COMPONENT_TYPE_MAX_ENUM_NV;
            }
        default:
            return VK_COMPONENT_TYPE_MAX_ENUM_NV;
    }
}

// Validate SPV_NV_cooperative_matrix behavior that can't be statically validated
// in SPIRV-Tools (e.g. due to specialization constant usage).
bool CoreChecks::ValidateCooperativeMatrix(SHADER_MODULE_STATE const *src, VkPipelineShaderStageCreateInfo const *pStage,
                                           const PIPELINE_STATE *pipeline) const {
    bool skip = false;

    // Map SPIR-V result ID to specialization constant id (SpecId decoration value)
    std::unordered_map<uint32_t, uint32_t> id_to_spec_id;
    // Map SPIR-V result ID to the ID of its type.
    std::unordered_map<uint32_t, uint32_t> id_to_type_id;

    struct CoopMatType {
        uint32_t scope, rows, cols;
        VkComponentTypeNV component_type;
        bool all_constant;

        CoopMatType() : scope(0), rows(0), cols(0), component_type(VK_COMPONENT_TYPE_MAX_ENUM_NV), all_constant(false) {}

        void Init(uint32_t id, SHADER_MODULE_STATE const *src, VkPipelineShaderStageCreateInfo const *pStage,
                  const std::unordered_map<uint32_t, uint32_t> &id_to_spec_id) {
            spirv_inst_iter insn = src->get_def(id);
            uint32_t component_type_id = insn.word(2);
            uint32_t scope_id = insn.word(3);
            uint32_t rows_id = insn.word(4);
            uint32_t cols_id = insn.word(5);
            auto component_type_iter = src->get_def(component_type_id);
            auto scope_iter = src->get_def(scope_id);
            auto rows_iter = src->get_def(rows_id);
            auto cols_iter = src->get_def(cols_id);

            all_constant = true;
            if (!GetIntConstantValue(scope_iter, src, pStage, id_to_spec_id, &scope)) {
                all_constant = false;
            }
            if (!GetIntConstantValue(rows_iter, src, pStage, id_to_spec_id, &rows)) {
                all_constant = false;
            }
            if (!GetIntConstantValue(cols_iter, src, pStage, id_to_spec_id, &cols)) {
                all_constant = false;
            }
            component_type = GetComponentType(component_type_iter, src);
        }
    };

    bool seen_coopmat_capability = false;

    for (auto insn : *src) {
        // Whitelist instructions whose result can be a cooperative matrix type, and
        // keep track of their types. It would be nice if SPIRV-Headers generated code
        // to identify which instructions have a result type and result id. Lacking that,
        // this whitelist is based on the set of instructions that
        // SPV_NV_cooperative_matrix says can be used with cooperative matrix types.
        switch (insn.opcode()) {
            case spv::OpLoad:
            case spv::OpCooperativeMatrixLoadNV:
            case spv::OpCooperativeMatrixMulAddNV:
            case spv::OpSNegate:
            case spv::OpFNegate:
            case spv::OpIAdd:
            case spv::OpFAdd:
            case spv::OpISub:
            case spv::OpFSub:
            case spv::OpFDiv:
            case spv::OpSDiv:
            case spv::OpUDiv:
            case spv::OpMatrixTimesScalar:
            case spv::OpConstantComposite:
            case spv::OpCompositeConstruct:
            case spv::OpConvertFToU:
            case spv::OpConvertFToS:
            case spv::OpConvertSToF:
            case spv::OpConvertUToF:
            case spv::OpUConvert:
            case spv::OpSConvert:
            case spv::OpFConvert:
                id_to_type_id[insn.word(2)] = insn.word(1);
                break;
            default:
                break;
        }

        switch (insn.opcode()) {
            case spv::OpDecorate:
                if (insn.word(2) == spv::DecorationSpecId) {
                    id_to_spec_id[insn.word(1)] = insn.word(3);
                }
                break;
            case spv::OpCapability:
                if (insn.word(1) == spv::CapabilityCooperativeMatrixNV) {
                    seen_coopmat_capability = true;

                    if (!(pStage->stage & phys_dev_ext_props.cooperative_matrix_props.cooperativeMatrixSupportedStages)) {
                        skip |= LogError(
                            pipeline->pipeline, kVUID_Core_Shader_CooperativeMatrixSupportedStages,
                            "OpTypeCooperativeMatrixNV used in shader stage not in cooperativeMatrixSupportedStages (= %u)",
                            phys_dev_ext_props.cooperative_matrix_props.cooperativeMatrixSupportedStages);
                    }
                }
                break;
            case spv::OpMemoryModel:
                // If the capability isn't enabled, don't bother with the rest of this function.
                // OpMemoryModel is the first required instruction after all OpCapability instructions.
                if (!seen_coopmat_capability) {
                    return skip;
                }
                break;
            case spv::OpTypeCooperativeMatrixNV: {
                CoopMatType M;
                M.Init(insn.word(1), src, pStage, id_to_spec_id);

                if (M.all_constant) {
                    // Validate that the type parameters are all supported for one of the
                    // operands of a cooperative matrix property.
                    bool valid = false;
                    for (unsigned i = 0; i < cooperative_matrix_properties.size(); ++i) {
                        if (cooperative_matrix_properties[i].AType == M.component_type &&
                            cooperative_matrix_properties[i].MSize == M.rows && cooperative_matrix_properties[i].KSize == M.cols &&
                            cooperative_matrix_properties[i].scope == M.scope) {
                            valid = true;
                            break;
                        }
                        if (cooperative_matrix_properties[i].BType == M.component_type &&
                            cooperative_matrix_properties[i].KSize == M.rows && cooperative_matrix_properties[i].NSize == M.cols &&
                            cooperative_matrix_properties[i].scope == M.scope) {
                            valid = true;
                            break;
                        }
                        if (cooperative_matrix_properties[i].CType == M.component_type &&
                            cooperative_matrix_properties[i].MSize == M.rows && cooperative_matrix_properties[i].NSize == M.cols &&
                            cooperative_matrix_properties[i].scope == M.scope) {
                            valid = true;
                            break;
                        }
                        if (cooperative_matrix_properties[i].DType == M.component_type &&
                            cooperative_matrix_properties[i].MSize == M.rows && cooperative_matrix_properties[i].NSize == M.cols &&
                            cooperative_matrix_properties[i].scope == M.scope) {
                            valid = true;
                            break;
                        }
                    }
                    if (!valid) {
                        skip |= LogError(pipeline->pipeline, kVUID_Core_Shader_CooperativeMatrixType,
                                         "OpTypeCooperativeMatrixNV (result id = %u) operands don't match a supported matrix type",
                                         insn.word(1));
                    }
                }
                break;
            }
            case spv::OpCooperativeMatrixMulAddNV: {
                CoopMatType A, B, C, D;
                if (id_to_type_id.find(insn.word(2)) == id_to_type_id.end() ||
                    id_to_type_id.find(insn.word(3)) == id_to_type_id.end() ||
                    id_to_type_id.find(insn.word(4)) == id_to_type_id.end() ||
                    id_to_type_id.find(insn.word(5)) == id_to_type_id.end()) {
                    // Couldn't find type of matrix
                    assert(false);
                    break;
                }
                D.Init(id_to_type_id[insn.word(2)], src, pStage, id_to_spec_id);
                A.Init(id_to_type_id[insn.word(3)], src, pStage, id_to_spec_id);
                B.Init(id_to_type_id[insn.word(4)], src, pStage, id_to_spec_id);
                C.Init(id_to_type_id[insn.word(5)], src, pStage, id_to_spec_id);

                if (A.all_constant && B.all_constant && C.all_constant && D.all_constant) {
                    // Validate that the type parameters are all supported for the same
                    // cooperative matrix property.
                    bool valid = false;
                    for (unsigned i = 0; i < cooperative_matrix_properties.size(); ++i) {
                        if (cooperative_matrix_properties[i].AType == A.component_type &&
                            cooperative_matrix_properties[i].MSize == A.rows && cooperative_matrix_properties[i].KSize == A.cols &&
                            cooperative_matrix_properties[i].scope == A.scope &&

                            cooperative_matrix_properties[i].BType == B.component_type &&
                            cooperative_matrix_properties[i].KSize == B.rows && cooperative_matrix_properties[i].NSize == B.cols &&
                            cooperative_matrix_properties[i].scope == B.scope &&

                            cooperative_matrix_properties[i].CType == C.component_type &&
                            cooperative_matrix_properties[i].MSize == C.rows && cooperative_matrix_properties[i].NSize == C.cols &&
                            cooperative_matrix_properties[i].scope == C.scope &&

                            cooperative_matrix_properties[i].DType == D.component_type &&
                            cooperative_matrix_properties[i].MSize == D.rows && cooperative_matrix_properties[i].NSize == D.cols &&
                            cooperative_matrix_properties[i].scope == D.scope) {
                            valid = true;
                            break;
                        }
                    }
                    if (!valid) {
                        skip |= LogError(pipeline->pipeline, kVUID_Core_Shader_CooperativeMatrixMulAdd,
                                         "OpCooperativeMatrixMulAddNV (result id = %u) operands don't match a supported matrix "
                                         "VkCooperativeMatrixPropertiesNV",
                                         insn.word(2));
                    }
                }
                break;
            }
            default:
                break;
        }
    }

    return skip;
}

bool CoreChecks::ValidateExecutionModes(SHADER_MODULE_STATE const *src, spirv_inst_iter entrypoint) const {
    auto entrypoint_id = entrypoint.word(2);

    // The first denorm execution mode encountered, along with its bit width.
    // Used to check if SeparateDenormSettings is respected.
    std::pair<spv::ExecutionMode, uint32_t> first_denorm_execution_mode = std::make_pair(spv::ExecutionModeMax, 0);

    // The first rounding mode encountered, along with its bit width.
    // Used to check if SeparateRoundingModeSettings is respected.
    std::pair<spv::ExecutionMode, uint32_t> first_rounding_mode = std::make_pair(spv::ExecutionModeMax, 0);

    bool skip = false;

    uint32_t verticesOut = 0;
    uint32_t invocations = 0;

    for (auto insn : *src) {
        if (insn.opcode() == spv::OpExecutionMode && insn.word(1) == entrypoint_id) {
            auto mode = insn.word(2);
            switch (mode) {
                case spv::ExecutionModeSignedZeroInfNanPreserve: {
                    auto bit_width = insn.word(3);
                    if ((bit_width == 16 && !phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat16) ||
                        (bit_width == 32 && !phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat32) ||
                        (bit_width == 64 && !phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat64)) {
                        skip |= LogError(
                            device, kVUID_Core_Shader_FeatureNotEnabled,
                            "Shader requires SignedZeroInfNanPreserve for bit width %d but it is not enabled on the device",
                            bit_width);
                    }
                    break;
                }

                case spv::ExecutionModeDenormPreserve: {
                    auto bit_width = insn.word(3);
                    if ((bit_width == 16 && !phys_dev_props_core12.shaderDenormPreserveFloat16) ||
                        (bit_width == 32 && !phys_dev_props_core12.shaderDenormPreserveFloat32) ||
                        (bit_width == 64 && !phys_dev_props_core12.shaderDenormPreserveFloat64)) {
                        skip |= LogError(device, kVUID_Core_Shader_FeatureNotEnabled,
                                         "Shader requires DenormPreserve for bit width %d but it is not enabled on the device",
                                         bit_width);
                    }

                    if (first_denorm_execution_mode.first == spv::ExecutionModeMax) {
                        // Register the first denorm execution mode found
                        first_denorm_execution_mode = std::make_pair(static_cast<spv::ExecutionMode>(mode), bit_width);
                    } else if (first_denorm_execution_mode.first != mode && first_denorm_execution_mode.second != bit_width) {
                        switch (phys_dev_props_core12.denormBehaviorIndependence) {
                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY_KHR:
                                if (first_rounding_mode.second != 32 && bit_width != 32) {
                                    skip |= LogError(device, kVUID_Core_Shader_FeatureNotEnabled,
                                                     "Shader uses different denorm execution modes for 16 and 64-bit but "
                                                     "denormBehaviorIndependence is "
                                                     "VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY_KHR on the device");
                                }
                                break;

                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL_KHR:
                                break;

                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE_KHR:
                                skip |= LogError(device, kVUID_Core_Shader_FeatureNotEnabled,
                                                 "Shader uses different denorm execution modes for different bit widths but "
                                                 "denormBehaviorIndependence is "
                                                 "VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE_KHR on the device");
                                break;

                            default:
                                break;
                        }
                    }
                    break;
                }

                case spv::ExecutionModeDenormFlushToZero: {
                    auto bit_width = insn.word(3);
                    if ((bit_width == 16 && !phys_dev_props_core12.shaderDenormFlushToZeroFloat16) ||
                        (bit_width == 32 && !phys_dev_props_core12.shaderDenormFlushToZeroFloat32) ||
                        (bit_width == 64 && !phys_dev_props_core12.shaderDenormFlushToZeroFloat64)) {
                        skip |= LogError(device, kVUID_Core_Shader_FeatureNotEnabled,
                                         "Shader requires DenormFlushToZero for bit width %d but it is not enabled on the device",
                                         bit_width);
                    }

                    if (first_denorm_execution_mode.first == spv::ExecutionModeMax) {
                        // Register the first denorm execution mode found
                        first_denorm_execution_mode = std::make_pair(static_cast<spv::ExecutionMode>(mode), bit_width);
                    } else if (first_denorm_execution_mode.first != mode && first_denorm_execution_mode.second != bit_width) {
                        switch (phys_dev_props_core12.denormBehaviorIndependence) {
                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY_KHR:
                                if (first_rounding_mode.second != 32 && bit_width != 32) {
                                    skip |= LogError(device, kVUID_Core_Shader_FeatureNotEnabled,
                                                     "Shader uses different denorm execution modes for 16 and 64-bit but "
                                                     "denormBehaviorIndependence is "
                                                     "VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY_KHR on the device");
                                }
                                break;

                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL_KHR:
                                break;

                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE_KHR:
                                skip |= LogError(device, kVUID_Core_Shader_FeatureNotEnabled,
                                                 "Shader uses different denorm execution modes for different bit widths but "
                                                 "denormBehaviorIndependence is "
                                                 "VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE_KHR on the device");
                                break;

                            default:
                                break;
                        }
                    }
                    break;
                }

                case spv::ExecutionModeRoundingModeRTE: {
                    auto bit_width = insn.word(3);
                    if ((bit_width == 16 && !phys_dev_props_core12.shaderRoundingModeRTEFloat16) ||
                        (bit_width == 32 && !phys_dev_props_core12.shaderRoundingModeRTEFloat32) ||
                        (bit_width == 64 && !phys_dev_props_core12.shaderRoundingModeRTEFloat64)) {
                        skip |= LogError(device, kVUID_Core_Shader_FeatureNotEnabled,
                                         "Shader requires RoundingModeRTE for bit width %d but it is not enabled on the device",
                                         bit_width);
                    }

                    if (first_rounding_mode.first == spv::ExecutionModeMax) {
                        // Register the first rounding mode found
                        first_rounding_mode = std::make_pair(static_cast<spv::ExecutionMode>(mode), bit_width);
                    } else if (first_rounding_mode.first != mode && first_rounding_mode.second != bit_width) {
                        switch (phys_dev_props_core12.roundingModeIndependence) {
                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY_KHR:
                                if (first_rounding_mode.second != 32 && bit_width != 32) {
                                    skip |= LogError(device, kVUID_Core_Shader_FeatureNotEnabled,
                                                     "Shader uses different rounding modes for 16 and 64-bit but "
                                                     "roundingModeIndependence is "
                                                     "VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY_KHR on the device");
                                }
                                break;

                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL_KHR:
                                break;

                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE_KHR:
                                skip |= LogError(device, kVUID_Core_Shader_FeatureNotEnabled,
                                                 "Shader uses different rounding modes for different bit widths but "
                                                 "roundingModeIndependence is "
                                                 "VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE_KHR on the device");
                                break;

                            default:
                                break;
                        }
                    }
                    break;
                }

                case spv::ExecutionModeRoundingModeRTZ: {
                    auto bit_width = insn.word(3);
                    if ((bit_width == 16 && !phys_dev_props_core12.shaderRoundingModeRTZFloat16) ||
                        (bit_width == 32 && !phys_dev_props_core12.shaderRoundingModeRTZFloat32) ||
                        (bit_width == 64 && !phys_dev_props_core12.shaderRoundingModeRTZFloat64)) {
                        skip |= LogError(device, kVUID_Core_Shader_FeatureNotEnabled,
                                         "Shader requires RoundingModeRTZ for bit width %d but it is not enabled on the device",
                                         bit_width);
                    }

                    if (first_rounding_mode.first == spv::ExecutionModeMax) {
                        // Register the first rounding mode found
                        first_rounding_mode = std::make_pair(static_cast<spv::ExecutionMode>(mode), bit_width);
                    } else if (first_rounding_mode.first != mode && first_rounding_mode.second != bit_width) {
                        switch (phys_dev_props_core12.roundingModeIndependence) {
                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY_KHR:
                                if (first_rounding_mode.second != 32 && bit_width != 32) {
                                    skip |= LogError(device, kVUID_Core_Shader_FeatureNotEnabled,
                                                     "Shader uses different rounding modes for 16 and 64-bit but "
                                                     "roundingModeIndependence is "
                                                     "VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY_KHR on the device");
                                }
                                break;

                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL_KHR:
                                break;

                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE_KHR:
                                skip |= LogError(device, kVUID_Core_Shader_FeatureNotEnabled,
                                                 "Shader uses different rounding modes for different bit widths but "
                                                 "roundingModeIndependence is "
                                                 "VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE_KHR on the device");
                                break;

                            default:
                                break;
                        }
                    }
                    break;
                }

                case spv::ExecutionModeOutputVertices: {
                    verticesOut = insn.word(3);
                    break;
                }

                case spv::ExecutionModeInvocations: {
                    invocations = insn.word(3);
                    break;
                }
            }
        }
    }

    if (entrypoint.word(1) == spv::ExecutionModelGeometry) {
        if (verticesOut == 0 || verticesOut > phys_dev_props.limits.maxGeometryOutputVertices) {
            skip |= LogError(device, "VUID-VkPipelineShaderStageCreateInfo-stage-00714",
                             "Geometry shader entry point must have an OpExecutionMode instruction that "
                             "specifies a maximum output vertex count that is greater than 0 and less "
                             "than or equal to maxGeometryOutputVertices. "
                             "OutputVertices=%d, maxGeometryOutputVertices=%d",
                             verticesOut, phys_dev_props.limits.maxGeometryOutputVertices);
        }

        if (invocations == 0 || invocations > phys_dev_props.limits.maxGeometryShaderInvocations) {
            skip |= LogError(device, "VUID-VkPipelineShaderStageCreateInfo-stage-00715",
                             "Geometry shader entry point must have an OpExecutionMode instruction that "
                             "specifies an invocation count that is greater than 0 and less "
                             "than or equal to maxGeometryShaderInvocations. "
                             "Invocations=%d, maxGeometryShaderInvocations=%d",
                             invocations, phys_dev_props.limits.maxGeometryShaderInvocations);
        }
    }
    return skip;
}

uint32_t DescriptorTypeToReqs(SHADER_MODULE_STATE const *module, uint32_t type_id) {
    auto type = module->get_def(type_id);

    while (true) {
        switch (type.opcode()) {
            case spv::OpTypeArray:
            case spv::OpTypeRuntimeArray:
            case spv::OpTypeSampledImage:
                type = module->get_def(type.word(2));
                break;
            case spv::OpTypePointer:
                type = module->get_def(type.word(3));
                break;
            case spv::OpTypeImage: {
                auto dim = type.word(3);
                auto arrayed = type.word(5);
                auto msaa = type.word(6);

                uint32_t bits = 0;
                switch (GetFundamentalType(module, type.word(2))) {
                    case FORMAT_TYPE_FLOAT:
                        bits = DESCRIPTOR_REQ_COMPONENT_TYPE_FLOAT;
                        break;
                    case FORMAT_TYPE_UINT:
                        bits = DESCRIPTOR_REQ_COMPONENT_TYPE_UINT;
                        break;
                    case FORMAT_TYPE_SINT:
                        bits = DESCRIPTOR_REQ_COMPONENT_TYPE_SINT;
                        break;
                    default:
                        break;
                }

                switch (dim) {
                    case spv::Dim1D:
                        bits |= arrayed ? DESCRIPTOR_REQ_VIEW_TYPE_1D_ARRAY : DESCRIPTOR_REQ_VIEW_TYPE_1D;
                        return bits;
                    case spv::Dim2D:
                        bits |= msaa ? DESCRIPTOR_REQ_MULTI_SAMPLE : DESCRIPTOR_REQ_SINGLE_SAMPLE;
                        bits |= arrayed ? DESCRIPTOR_REQ_VIEW_TYPE_2D_ARRAY : DESCRIPTOR_REQ_VIEW_TYPE_2D;
                        return bits;
                    case spv::Dim3D:
                        bits |= DESCRIPTOR_REQ_VIEW_TYPE_3D;
                        return bits;
                    case spv::DimCube:
                        bits |= arrayed ? DESCRIPTOR_REQ_VIEW_TYPE_CUBE_ARRAY : DESCRIPTOR_REQ_VIEW_TYPE_CUBE;
                        return bits;
                    case spv::DimSubpassData:
                        bits |= msaa ? DESCRIPTOR_REQ_MULTI_SAMPLE : DESCRIPTOR_REQ_SINGLE_SAMPLE;
                        return bits;
                    default:  // buffer, etc.
                        return bits;
                }
            }
            default:
                return 0;
        }
    }
}

// For given pipelineLayout verify that the set_layout_node at slot.first
//  has the requested binding at slot.second and return ptr to that binding
static VkDescriptorSetLayoutBinding const *GetDescriptorBinding(PIPELINE_LAYOUT_STATE const *pipelineLayout,
                                                                descriptor_slot_t slot) {
    if (!pipelineLayout) return nullptr;

    if (slot.first >= pipelineLayout->set_layouts.size()) return nullptr;

    return pipelineLayout->set_layouts[slot.first]->GetDescriptorSetLayoutBindingPtrFromBinding(slot.second);
}

int32_t GetShaderResourceDimensionality(const SHADER_MODULE_STATE *module, const interface_var &resource) {
    if (module == nullptr) return -1;

    auto type = module->get_def(resource.type_id);
    while (true) {
        switch (type.opcode()) {
            case spv::OpTypeSampledImage:
                type = module->get_def(type.word(2));
                break;
            case spv::OpTypePointer:
                type = module->get_def(type.word(3));
                break;
            case spv::OpTypeImage:
                return type.word(3);
            default:
                return -1;
        }
    }
}

bool FindLocalSize(SHADER_MODULE_STATE const *src, uint32_t &local_size_x, uint32_t &local_size_y, uint32_t &local_size_z) {
    for (auto insn : *src) {
        if (insn.opcode() == spv::OpEntryPoint) {
            auto executionModel = insn.word(1);
            auto entrypointStageBits = ExecutionModelToShaderStageFlagBits(executionModel);
            if (entrypointStageBits == VK_SHADER_STAGE_COMPUTE_BIT) {
                auto entrypoint_id = insn.word(2);
                for (auto insn1 : *src) {
                    if (insn1.opcode() == spv::OpExecutionMode && insn1.word(1) == entrypoint_id &&
                        insn1.word(2) == spv::ExecutionModeLocalSize) {
                        local_size_x = insn1.word(3);
                        local_size_y = insn1.word(4);
                        local_size_z = insn1.word(5);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void ProcessExecutionModes(SHADER_MODULE_STATE const *src, const spirv_inst_iter &entrypoint, PIPELINE_STATE *pipeline) {
    auto entrypoint_id = entrypoint.word(2);
    bool is_point_mode = false;

    for (auto insn : *src) {
        if (insn.opcode() == spv::OpExecutionMode && insn.word(1) == entrypoint_id) {
            switch (insn.word(2)) {
                case spv::ExecutionModePointMode:
                    // In tessellation shaders, PointMode is separate and trumps the tessellation topology.
                    is_point_mode = true;
                    break;

                case spv::ExecutionModeOutputPoints:
                    pipeline->topology_at_rasterizer = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
                    break;

                case spv::ExecutionModeIsolines:
                case spv::ExecutionModeOutputLineStrip:
                    pipeline->topology_at_rasterizer = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
                    break;

                case spv::ExecutionModeTriangles:
                case spv::ExecutionModeQuads:
                case spv::ExecutionModeOutputTriangleStrip:
                    pipeline->topology_at_rasterizer = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
                    break;
            }
        }
    }

    if (is_point_mode) pipeline->topology_at_rasterizer = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
}

// If PointList topology is specified in the pipeline, verify that a shader geometry stage writes PointSize
//    o If there is only a vertex shader : gl_PointSize must be written when using points
//    o If there is a geometry or tessellation shader:
//        - If shaderTessellationAndGeometryPointSize feature is enabled:
//            * gl_PointSize must be written in the final geometry stage
//        - If shaderTessellationAndGeometryPointSize feature is disabled:
//            * gl_PointSize must NOT be written and a default of 1.0 is assumed
bool CoreChecks::ValidatePointListShaderState(const PIPELINE_STATE *pipeline, SHADER_MODULE_STATE const *src,
                                              spirv_inst_iter entrypoint, VkShaderStageFlagBits stage) const {
    if (pipeline->topology_at_rasterizer != VK_PRIMITIVE_TOPOLOGY_POINT_LIST) {
        return false;
    }

    bool pointsize_written = false;
    bool skip = false;

    // Search for PointSize built-in decorations
    std::vector<uint32_t> pointsize_builtin_offsets;
    spirv_inst_iter insn = entrypoint;
    while (!pointsize_written && (insn.opcode() != spv::OpFunction)) {
        if (insn.opcode() == spv::OpMemberDecorate) {
            if (insn.word(3) == spv::DecorationBuiltIn) {
                if (insn.word(4) == spv::BuiltInPointSize) {
                    pointsize_written = IsPointSizeWritten(src, insn, entrypoint);
                }
            }
        } else if (insn.opcode() == spv::OpDecorate) {
            if (insn.word(2) == spv::DecorationBuiltIn) {
                if (insn.word(3) == spv::BuiltInPointSize) {
                    pointsize_written = IsPointSizeWritten(src, insn, entrypoint);
                }
            }
        }

        insn++;
    }

    if ((stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT || stage == VK_SHADER_STAGE_GEOMETRY_BIT) &&
        !enabled_features.core.shaderTessellationAndGeometryPointSize) {
        if (pointsize_written) {
            skip |= LogError(pipeline->pipeline, kVUID_Core_Shader_PointSizeBuiltInOverSpecified,
                             "Pipeline topology is set to POINT_LIST and geometry or tessellation shaders write PointSize which "
                             "is prohibited when the shaderTessellationAndGeometryPointSize feature is not enabled.");
        }
    } else if (!pointsize_written) {
        skip |=
            LogError(pipeline->pipeline, kVUID_Core_Shader_MissingPointSizeBuiltIn,
                     "Pipeline topology is set to POINT_LIST, but PointSize is not written to in the shader corresponding to %s.",
                     string_VkShaderStageFlagBits(stage));
    }
    return skip;
}

bool CoreChecks::ValidatePipelineShaderStage(VkPipelineShaderStageCreateInfo const *pStage, const PIPELINE_STATE *pipeline,
                                             const PIPELINE_STATE::StageState &stage_state, const SHADER_MODULE_STATE *module,
                                             const spirv_inst_iter &entrypoint, bool check_point_size) const {
    bool skip = false;

    // Check the module
    if (!module->has_valid_spirv) {
        skip |= LogError(device, "VUID-VkPipelineShaderStageCreateInfo-module-parameter",
                         "%s does not contain valid spirv for stage %s.",
                         report_data->FormatHandle(module->vk_shader_module).c_str(), string_VkShaderStageFlagBits(pStage->stage));
    }

    // If specialization-constant values are given and specialization-constant instructions are present in the shader, the
    // specializations should be applied and validated.
    if (pStage->pSpecializationInfo != nullptr && pStage->pSpecializationInfo->mapEntryCount > 0 &&
        pStage->pSpecializationInfo->pMapEntries != nullptr && module->has_specialization_constants) {
        // Gather the specialization-constant values.
        auto const &specialization_info = pStage->pSpecializationInfo;
        auto const &specialization_data = reinterpret_cast<uint8_t const *>(specialization_info->pData);
        std::unordered_map<uint32_t, std::vector<uint32_t>> id_value_map;
        id_value_map.reserve(specialization_info->mapEntryCount);
        for (auto i = 0u; i < specialization_info->mapEntryCount; ++i) {
            auto const &map_entry = specialization_info->pMapEntries[i];

            // Expect only scalar types.
            assert(map_entry.size == 1 || map_entry.size == 2 || map_entry.size == 4 || map_entry.size == 8);
            auto entry = id_value_map.emplace(map_entry.constantID, std::vector<uint32_t>(map_entry.size > 4 ? 2 : 1));
            memcpy(entry.first->second.data(), specialization_data + map_entry.offset, map_entry.size);
        }

        // Apply the specialization-constant values and revalidate the shader module.
        spv_target_env spirv_environment = PickSpirvEnv(api_version, (device_extensions.vk_khr_spirv_1_4 != kNotEnabled));
        spvtools::Optimizer optimizer(spirv_environment);
        spvtools::MessageConsumer consumer = [&skip, &module, &pStage, this](spv_message_level_t level, const char *source,
                                                                             const spv_position_t &position, const char *message) {
            skip |= LogError(
                device, "VUID-VkPipelineShaderStageCreateInfo-module-parameter", "%s does not contain valid spirv for stage %s. %s",
                report_data->FormatHandle(module->vk_shader_module).c_str(), string_VkShaderStageFlagBits(pStage->stage), message);
        };
        optimizer.SetMessageConsumer(consumer);
        optimizer.RegisterPass(spvtools::CreateSetSpecConstantDefaultValuePass(id_value_map));
        optimizer.RegisterPass(spvtools::CreateFreezeSpecConstantValuePass());
        std::vector<uint32_t> specialized_spirv;
        auto const optimized =
            optimizer.Run(module->words.data(), module->words.size(), &specialized_spirv, spvtools::ValidatorOptions(), true);
        assert(optimized == true);

        if (optimized) {
            spv_context ctx = spvContextCreate(spirv_environment);
            spv_const_binary_t binary{specialized_spirv.data(), specialized_spirv.size()};
            spv_diagnostic diag = nullptr;
            spvtools::ValidatorOptions options;
            AdjustValidatorOptions(device_extensions, enabled_features, options);
            auto const spv_valid = spvValidateWithOptions(ctx, options, &binary, &diag);
            if (spv_valid != SPV_SUCCESS) {
                skip |= LogError(device, "VUID-VkPipelineShaderStageCreateInfo-module-04145",
                                 "After specialization was applied, %s does not contain valid spirv for stage %s.",
                                 report_data->FormatHandle(module->vk_shader_module).c_str(),
                                 string_VkShaderStageFlagBits(pStage->stage));
            }

            spvDiagnosticDestroy(diag);
            spvContextDestroy(ctx);
        }
    }

    // Check the entrypoint
    if (entrypoint == module->end()) {
        skip |=
            LogError(device, "VUID-VkPipelineShaderStageCreateInfo-pName-00707", "No entrypoint found named `%s` for stage %s..",
                     pStage->pName, string_VkShaderStageFlagBits(pStage->stage));
    }
    if (skip) return true;  // no point continuing beyond here, any analysis is just going to be garbage.

    // Mark accessible ids
    auto &accessible_ids = stage_state.accessible_ids;

    // Validate descriptor set layout against what the entrypoint actually uses
    bool has_writable_descriptor = stage_state.has_writable_descriptor;
    auto &descriptor_uses = stage_state.descriptor_uses;

    // Validate shader capabilities against enabled device features
    skip |= ValidateShaderCapabilities(module, pStage->stage);
    skip |=
        ValidateShaderStageWritableOrAtomicDescriptor(pStage->stage, has_writable_descriptor, stage_state.has_atomic_descriptor);
    skip |= ValidateShaderStageInputOutputLimits(module, pStage, pipeline, entrypoint);
    skip |= ValidateShaderStageMaxResources(pStage->stage, pipeline);
    skip |= ValidateShaderStageGroupNonUniform(module, pStage->stage);
    skip |= ValidateExecutionModes(module, entrypoint);
    skip |= ValidateSpecializationOffsets(pStage);
    skip |= ValidatePushConstantUsage(pipeline->pipeline_layout->push_constant_ranges.get(), module, accessible_ids, pStage->stage);
    if (check_point_size && !pipeline->graphicsPipelineCI.pRasterizationState->rasterizerDiscardEnable) {
        skip |= ValidatePointListShaderState(pipeline, module, entrypoint, pStage->stage);
    }
    skip |= ValidateCooperativeMatrix(module, pStage, pipeline);

    std::string vuid_layout_mismatch;
    if (pipeline->graphicsPipelineCI.sType == VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO) {
        vuid_layout_mismatch = "VUID-VkGraphicsPipelineCreateInfo-layout-00756";
    } else if (pipeline->computePipelineCI.sType == VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO) {
        vuid_layout_mismatch = "VUID-VkComputePipelineCreateInfo-layout-00703";
    } else if (pipeline->raytracingPipelineCI.sType == VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR) {
        vuid_layout_mismatch = "VUID-VkRayTracingPipelineCreateInfoKHR-layout-03427";
    } else if (pipeline->raytracingPipelineCI.sType == VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV) {
        vuid_layout_mismatch = "VUID-VkRayTracingPipelineCreateInfoNV-layout-03427";
    }

    // Validate descriptor use
    for (auto use : descriptor_uses) {
        // Verify given pipelineLayout has requested setLayout with requested binding
        const auto &binding = GetDescriptorBinding(pipeline->pipeline_layout.get(), use.first);
        unsigned required_descriptor_count;
        std::set<uint32_t> descriptor_types = TypeToDescriptorTypeSet(module, use.second.type_id, required_descriptor_count);

        if (!binding) {
            skip |= LogError(device, vuid_layout_mismatch,
                             "Shader uses descriptor slot %u.%u (expected `%s`) but not declared in pipeline layout",
                             use.first.first, use.first.second, string_descriptorTypes(descriptor_types).c_str());
        } else if (~binding->stageFlags & pStage->stage) {
            skip |= LogError(device, vuid_layout_mismatch,
                             "Shader uses descriptor slot %u.%u but descriptor not accessible from stage %s", use.first.first,
                             use.first.second, string_VkShaderStageFlagBits(pStage->stage));
        } else if (descriptor_types.find(binding->descriptorType) == descriptor_types.end()) {
            skip |= LogError(device, vuid_layout_mismatch,
                             "Type mismatch on descriptor slot %u.%u (expected `%s`) but descriptor of type %s", use.first.first,
                             use.first.second, string_descriptorTypes(descriptor_types).c_str(),
                             string_VkDescriptorType(binding->descriptorType));
        } else if (binding->descriptorCount < required_descriptor_count) {
            skip |= LogError(device, vuid_layout_mismatch,
                             "Shader expects at least %u descriptors for binding %u.%u but only %u provided",
                             required_descriptor_count, use.first.first, use.first.second, binding->descriptorCount);
        }
    }

    // Validate use of input attachments against subpass structure
    if (pStage->stage == VK_SHADER_STAGE_FRAGMENT_BIT) {
        auto input_attachment_uses = CollectInterfaceByInputAttachmentIndex(module, accessible_ids);

        auto rpci = pipeline->rp_state->createInfo.ptr();
        auto subpass = pipeline->graphicsPipelineCI.subpass;

        for (auto use : input_attachment_uses) {
            auto input_attachments = rpci->pSubpasses[subpass].pInputAttachments;
            auto index = (input_attachments && use.first < rpci->pSubpasses[subpass].inputAttachmentCount)
                             ? input_attachments[use.first].attachment
                             : VK_ATTACHMENT_UNUSED;

            if (index == VK_ATTACHMENT_UNUSED) {
                skip |= LogError(device, kVUID_Core_Shader_MissingInputAttachment,
                                 "Shader consumes input attachment index %d but not provided in subpass", use.first);
            } else if (!(GetFormatType(rpci->pAttachments[index].format) & GetFundamentalType(module, use.second.type_id))) {
                skip |=
                    LogError(device, kVUID_Core_Shader_InputAttachmentTypeMismatch,
                             "Subpass input attachment %u format of %s does not match type used in shader `%s`", use.first,
                             string_VkFormat(rpci->pAttachments[index].format), DescribeType(module, use.second.type_id).c_str());
            }
        }
    }
    if (pStage->stage == VK_SHADER_STAGE_COMPUTE_BIT) {
        skip |= ValidateComputeWorkGroupSizes(module);
    }
    return skip;
}

bool CoreChecks::ValidateInterfaceBetweenStages(SHADER_MODULE_STATE const *producer, spirv_inst_iter producer_entrypoint,
                                                shader_stage_attributes const *producer_stage, SHADER_MODULE_STATE const *consumer,
                                                spirv_inst_iter consumer_entrypoint,
                                                shader_stage_attributes const *consumer_stage) const {
    bool skip = false;

    auto outputs =
        CollectInterfaceByLocation(producer, producer_entrypoint, spv::StorageClassOutput, producer_stage->arrayed_output);
    auto inputs = CollectInterfaceByLocation(consumer, consumer_entrypoint, spv::StorageClassInput, consumer_stage->arrayed_input);

    auto a_it = outputs.begin();
    auto b_it = inputs.begin();

    // Maps sorted by key (location); walk them together to find mismatches
    while ((outputs.size() > 0 && a_it != outputs.end()) || (inputs.size() && b_it != inputs.end())) {
        bool a_at_end = outputs.size() == 0 || a_it == outputs.end();
        bool b_at_end = inputs.size() == 0 || b_it == inputs.end();
        auto a_first = a_at_end ? std::make_pair(0u, 0u) : a_it->first;
        auto b_first = b_at_end ? std::make_pair(0u, 0u) : b_it->first;

        if (b_at_end || ((!a_at_end) && (a_first < b_first))) {
            skip |= LogPerformanceWarning(producer->vk_shader_module, kVUID_Core_Shader_OutputNotConsumed,
                                          "%s writes to output location %u.%u which is not consumed by %s", producer_stage->name,
                                          a_first.first, a_first.second, consumer_stage->name);
            a_it++;
        } else if (a_at_end || a_first > b_first) {
            skip |= LogError(consumer->vk_shader_module, kVUID_Core_Shader_InputNotProduced,
                             "%s consumes input location %u.%u which is not written by %s", consumer_stage->name, b_first.first,
                             b_first.second, producer_stage->name);
            b_it++;
        } else {
            // subtleties of arrayed interfaces:
            // - if is_patch, then the member is not arrayed, even though the interface may be.
            // - if is_block_member, then the extra array level of an arrayed interface is not
            //   expressed in the member type -- it's expressed in the block type.
            if (!TypesMatch(producer, consumer, a_it->second.type_id, b_it->second.type_id,
                            producer_stage->arrayed_output && !a_it->second.is_patch && !a_it->second.is_block_member,
                            consumer_stage->arrayed_input && !b_it->second.is_patch && !b_it->second.is_block_member, true)) {
                skip |= LogError(producer->vk_shader_module, kVUID_Core_Shader_InterfaceTypeMismatch,
                                 "Type mismatch on location %u.%u: '%s' vs '%s'", a_first.first, a_first.second,
                                 DescribeType(producer, a_it->second.type_id).c_str(),
                                 DescribeType(consumer, b_it->second.type_id).c_str());
            }
            if (a_it->second.is_patch != b_it->second.is_patch) {
                skip |= LogError(producer->vk_shader_module, kVUID_Core_Shader_InterfaceTypeMismatch,
                                 "Decoration mismatch on location %u.%u: is per-%s in %s stage but per-%s in %s stage",
                                 a_first.first, a_first.second, a_it->second.is_patch ? "patch" : "vertex", producer_stage->name,
                                 b_it->second.is_patch ? "patch" : "vertex", consumer_stage->name);
            }
            if (a_it->second.is_relaxed_precision != b_it->second.is_relaxed_precision) {
                skip |= LogError(producer->vk_shader_module, kVUID_Core_Shader_InterfaceTypeMismatch,
                                 "Decoration mismatch on location %u.%u: %s and %s stages differ in precision", a_first.first,
                                 a_first.second, producer_stage->name, consumer_stage->name);
            }
            a_it++;
            b_it++;
        }
    }

    if (consumer_stage->stage != VK_SHADER_STAGE_FRAGMENT_BIT) {
        auto builtins_producer = CollectBuiltinBlockMembers(producer, producer_entrypoint, spv::StorageClassOutput);
        auto builtins_consumer = CollectBuiltinBlockMembers(consumer, consumer_entrypoint, spv::StorageClassInput);

        if (!builtins_producer.empty() && !builtins_consumer.empty()) {
            if (builtins_producer.size() != builtins_consumer.size()) {
                skip |= LogError(producer->vk_shader_module, kVUID_Core_Shader_InterfaceTypeMismatch,
                                 "Number of elements inside builtin block differ between stages (%s %d vs %s %d).",
                                 producer_stage->name, (int)builtins_producer.size(), consumer_stage->name,
                                 (int)builtins_consumer.size());
            } else {
                auto it_producer = builtins_producer.begin();
                auto it_consumer = builtins_consumer.begin();
                while (it_producer != builtins_producer.end() && it_consumer != builtins_consumer.end()) {
                    if (*it_producer != *it_consumer) {
                        skip |= LogError(producer->vk_shader_module, kVUID_Core_Shader_InterfaceTypeMismatch,
                                         "Builtin variable inside block doesn't match between %s and %s.", producer_stage->name,
                                         consumer_stage->name);
                        break;
                    }
                    it_producer++;
                    it_consumer++;
                }
            }
        }
    }

    return skip;
}

static inline uint32_t DetermineFinalGeomStage(const PIPELINE_STATE *pipeline, const VkGraphicsPipelineCreateInfo *pCreateInfo) {
    uint32_t stage_mask = 0;
    if (pipeline->topology_at_rasterizer == VK_PRIMITIVE_TOPOLOGY_POINT_LIST) {
        for (uint32_t i = 0; i < pCreateInfo->stageCount; i++) {
            stage_mask |= pCreateInfo->pStages[i].stage;
        }
        // Determine which shader in which PointSize should be written (the final geometry stage)
        if (stage_mask & VK_SHADER_STAGE_MESH_BIT_NV) {
            stage_mask = VK_SHADER_STAGE_MESH_BIT_NV;
        } else if (stage_mask & VK_SHADER_STAGE_GEOMETRY_BIT) {
            stage_mask = VK_SHADER_STAGE_GEOMETRY_BIT;
        } else if (stage_mask & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) {
            stage_mask = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        } else if (stage_mask & VK_SHADER_STAGE_VERTEX_BIT) {
            stage_mask = VK_SHADER_STAGE_VERTEX_BIT;
        }
    }
    return stage_mask;
}

// Validate that the shaders used by the given pipeline and store the active_slots
//  that are actually used by the pipeline into pPipeline->active_slots
bool CoreChecks::ValidateGraphicsPipelineShaderState(const PIPELINE_STATE *pipeline) const {
    auto pCreateInfo = pipeline->graphicsPipelineCI.ptr();
    int vertex_stage = GetShaderStageId(VK_SHADER_STAGE_VERTEX_BIT);
    int fragment_stage = GetShaderStageId(VK_SHADER_STAGE_FRAGMENT_BIT);

    const SHADER_MODULE_STATE *shaders[32];
    memset(shaders, 0, sizeof(shaders));
    spirv_inst_iter entrypoints[32];
    memset(entrypoints, 0, sizeof(entrypoints));
    bool skip = false;

    uint32_t pointlist_stage_mask = DetermineFinalGeomStage(pipeline, pCreateInfo);

    for (uint32_t i = 0; i < pCreateInfo->stageCount; i++) {
        auto pStage = &pCreateInfo->pStages[i];
        auto stage_id = GetShaderStageId(pStage->stage);
        shaders[stage_id] = GetShaderModuleState(pStage->module);
        entrypoints[stage_id] = FindEntrypoint(shaders[stage_id], pStage->pName, pStage->stage);
        skip |= ValidatePipelineShaderStage(pStage, pipeline, pipeline->stage_state[i], shaders[stage_id], entrypoints[stage_id],
                                            (pointlist_stage_mask == pStage->stage));
    }

    // if the shader stages are no good individually, cross-stage validation is pointless.
    if (skip) return true;

    auto vi = pCreateInfo->pVertexInputState;

    if (vi) {
        skip |= ValidateViConsistency(vi);
    }

    if (shaders[vertex_stage] && shaders[vertex_stage]->has_valid_spirv) {
        skip |= ValidateViAgainstVsInputs(vi, shaders[vertex_stage], entrypoints[vertex_stage]);
    }

    int producer = GetShaderStageId(VK_SHADER_STAGE_VERTEX_BIT);
    int consumer = GetShaderStageId(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);

    while (!shaders[producer] && producer != fragment_stage) {
        producer++;
        consumer++;
    }

    for (; producer != fragment_stage && consumer <= fragment_stage; consumer++) {
        assert(shaders[producer]);
        if (shaders[consumer]) {
            if (shaders[consumer]->has_valid_spirv && shaders[producer]->has_valid_spirv) {
                skip |= ValidateInterfaceBetweenStages(shaders[producer], entrypoints[producer], &shader_stage_attribs[producer],
                                                       shaders[consumer], entrypoints[consumer], &shader_stage_attribs[consumer]);
            }

            producer = consumer;
        }
    }

    if (shaders[fragment_stage] && shaders[fragment_stage]->has_valid_spirv) {
        skip |= ValidateFsOutputsAgainstRenderPass(shaders[fragment_stage], entrypoints[fragment_stage], pipeline,
                                                   pCreateInfo->subpass);
    }

    return skip;
}

bool CoreChecks::ValidateComputePipelineShaderState(PIPELINE_STATE *pipeline) const {
    const auto &stage = *pipeline->computePipelineCI.stage.ptr();

    const SHADER_MODULE_STATE *module = GetShaderModuleState(stage.module);
    const spirv_inst_iter entrypoint = FindEntrypoint(module, stage.pName, stage.stage);

    return ValidatePipelineShaderStage(&stage, pipeline, pipeline->stage_state[0], module, entrypoint, false);
}

bool CoreChecks::ValidateRayTracingPipeline(PIPELINE_STATE *pipeline, bool isKHR) const {
    bool skip = false;

    if (isKHR) {
        if (pipeline->raytracingPipelineCI.maxRecursionDepth > phys_dev_ext_props.ray_tracing_propsKHR.maxRecursionDepth) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-maxRecursionDepth-03464", ": %d > %d",
                             pipeline->raytracingPipelineCI.maxRecursionDepth,
                             phys_dev_ext_props.ray_tracing_propsKHR.maxRecursionDepth);
        }
        for (uint32_t i = 0; i < pipeline->raytracingPipelineCI.libraries.libraryCount; ++i) {
            const PIPELINE_STATE *pLibrary_pipelinestate = GetPipelineState(pipeline->raytracingPipelineCI.libraries.pLibraries[i]);
            if (pLibrary_pipelinestate->raytracingPipelineCI.maxRecursionDepth !=
                pipeline->raytracingPipelineCI.maxRecursionDepth) {
                skip |= LogError(
                    device, "VUID-VkRayTracingPipelineCreateInfoKHR-pLibraries-03467",
                    "vkCreateRayTracingPipelinesKHR: Each element  (%d) of the pLibraries member of libraries must have been"
                    "created with the value of maxRecursionDepth (%d) equal to that in this pipeline (%d) .",
                    i, pLibrary_pipelinestate->raytracingPipelineCI.maxRecursionDepth,
                    pipeline->raytracingPipelineCI.maxRecursionDepth);
            }
            if (pLibrary_pipelinestate->raytracingPipelineCI.pLibraryInterface->maxAttributeSize !=
                    pipeline->raytracingPipelineCI.pLibraryInterface->maxAttributeSize ||
                pLibrary_pipelinestate->raytracingPipelineCI.pLibraryInterface->maxPayloadSize !=
                    pipeline->raytracingPipelineCI.pLibraryInterface->maxPayloadSize ||
                pLibrary_pipelinestate->raytracingPipelineCI.pLibraryInterface->maxCallableSize !=
                    pipeline->raytracingPipelineCI.pLibraryInterface->maxCallableSize) {
                skip |=
                    LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-pLibraries-03469",
                             "vkCreateRayTracingPipelinesKHR: Each element of the pLibraries member of libraries must have been "
                             "created with values of the maxPayloadSize,"
                             "maxAttributeSize, and maxCallableSize members of pLibraryInterface equal to those in this pipeline.");
            }
        }
    } else {
        if (pipeline->raytracingPipelineCI.maxRecursionDepth > phys_dev_ext_props.ray_tracing_propsNV.maxRecursionDepth) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-maxRecursionDepth-03457", ": %d > %d",
                             pipeline->raytracingPipelineCI.maxRecursionDepth,
                             phys_dev_ext_props.ray_tracing_propsNV.maxRecursionDepth);
        }
    }
    const auto *stages = pipeline->raytracingPipelineCI.ptr()->pStages;
    const auto *groups = pipeline->raytracingPipelineCI.ptr()->pGroups;

    uint32_t raygen_stages_found = 0;
    for (uint32_t stage_index = 0; stage_index < pipeline->raytracingPipelineCI.stageCount; stage_index++) {
        const auto &stage = stages[stage_index];

        const SHADER_MODULE_STATE *module = GetShaderModuleState(stage.module);
        const spirv_inst_iter entrypoint = FindEntrypoint(module, stage.pName, stage.stage);

        skip |= ValidatePipelineShaderStage(&stage, pipeline, pipeline->stage_state[stage_index], module, entrypoint, false);

        if (stage.stage == VK_SHADER_STAGE_RAYGEN_BIT_NV) {
            raygen_stages_found++;
        }
    }
    if (raygen_stages_found == 0) {
        skip |= LogError(
            device,
            isKHR ? "VUID-VkRayTracingPipelineCreateInfoKHR-stage-03425" : "VUID-VkRayTracingPipelineCreateInfoNV-stage-03425",
            " : zero raygen stages specified");
    }

    for (uint32_t group_index = 0; group_index < pipeline->raytracingPipelineCI.groupCount; group_index++) {
        const auto &group = groups[group_index];

        if (group.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV) {
            if (group.generalShader >= pipeline->raytracingPipelineCI.stageCount ||
                (stages[group.generalShader].stage != VK_SHADER_STAGE_RAYGEN_BIT_NV &&
                 stages[group.generalShader].stage != VK_SHADER_STAGE_MISS_BIT_NV &&
                 stages[group.generalShader].stage != VK_SHADER_STAGE_CALLABLE_BIT_NV)) {
                skip |= LogError(device,
                                 isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03474"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02413",
                                 ": pGroups[%d]", group_index);
            }
            if (group.anyHitShader != VK_SHADER_UNUSED_NV || group.closestHitShader != VK_SHADER_UNUSED_NV ||
                group.intersectionShader != VK_SHADER_UNUSED_NV) {
                skip |= LogError(device,
                                 isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03475"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02414",
                                 ": pGroups[%d]", group_index);
            }
        } else if (group.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV) {
            if (group.intersectionShader >= pipeline->raytracingPipelineCI.stageCount ||
                stages[group.intersectionShader].stage != VK_SHADER_STAGE_INTERSECTION_BIT_NV) {
                skip |= LogError(device,
                                 isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03476"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02415",
                                 ": pGroups[%d]", group_index);
            }
        } else if (group.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV) {
            if (group.intersectionShader != VK_SHADER_UNUSED_NV) {
                skip |= LogError(device,
                                 isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03477"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02416",
                                 ": pGroups[%d]", group_index);
            }
        }

        if (group.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV ||
            group.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV) {
            if (group.anyHitShader != VK_SHADER_UNUSED_NV && (group.anyHitShader >= pipeline->raytracingPipelineCI.stageCount ||
                                                              stages[group.anyHitShader].stage != VK_SHADER_STAGE_ANY_HIT_BIT_NV)) {
                skip |= LogError(device,
                                 isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-anyHitShader-03479"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-anyHitShader-02418",
                                 ": pGroups[%d]", group_index);
            }
            if (group.closestHitShader != VK_SHADER_UNUSED_NV &&
                (group.closestHitShader >= pipeline->raytracingPipelineCI.stageCount ||
                 stages[group.closestHitShader].stage != VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV)) {
                skip |= LogError(device,
                                 isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-closestHitShader-03478"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-closestHitShader-02417",
                                 ": pGroups[%d]", group_index);
            }
        }
    }
    return skip;
}

uint32_t ValidationCache::MakeShaderHash(VkShaderModuleCreateInfo const *smci) { return XXH32(smci->pCode, smci->codeSize, 0); }

static ValidationCache *GetValidationCacheInfo(VkShaderModuleCreateInfo const *pCreateInfo) {
    const auto validation_cache_ci = lvl_find_in_chain<VkShaderModuleValidationCacheCreateInfoEXT>(pCreateInfo->pNext);
    if (validation_cache_ci) {
        return CastFromHandle<ValidationCache *>(validation_cache_ci->validationCache);
    }
    return nullptr;
}

bool CoreChecks::PreCallValidateCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                                   const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule) const {
    bool skip = false;
    spv_result_t spv_valid = SPV_SUCCESS;

    if (disabled[shader_validation]) {
        return false;
    }

    auto have_glsl_shader = device_extensions.vk_nv_glsl_shader;

    if (!have_glsl_shader && (pCreateInfo->codeSize % 4)) {
        skip |= LogError(device, "VUID-VkShaderModuleCreateInfo-pCode-01376",
                         "SPIR-V module not valid: Codesize must be a multiple of 4 but is " PRINTF_SIZE_T_SPECIFIER ".",
                         pCreateInfo->codeSize);
    } else {
        auto cache = GetValidationCacheInfo(pCreateInfo);
        uint32_t hash = 0;
        if (cache) {
            hash = ValidationCache::MakeShaderHash(pCreateInfo);
            if (cache->Contains(hash)) return false;
        }

        // Use SPIRV-Tools validator to try and catch any issues with the module itself. If specialization constants are present,
        // the default values will be used during validation.
        spv_target_env spirv_environment = PickSpirvEnv(api_version, (device_extensions.vk_khr_spirv_1_4 != kNotEnabled));
        spv_context ctx = spvContextCreate(spirv_environment);
        spv_const_binary_t binary{pCreateInfo->pCode, pCreateInfo->codeSize / sizeof(uint32_t)};
        spv_diagnostic diag = nullptr;
        spvtools::ValidatorOptions options;
        AdjustValidatorOptions(device_extensions, enabled_features, options);
        spv_valid = spvValidateWithOptions(ctx, options, &binary, &diag);
        if (spv_valid != SPV_SUCCESS) {
            if (!have_glsl_shader || (pCreateInfo->pCode[0] == spv::MagicNumber)) {
                if (spv_valid == SPV_WARNING) {
                    skip |= LogWarning(device, kVUID_Core_Shader_InconsistentSpirv, "SPIR-V module not valid: %s",
                                       diag && diag->error ? diag->error : "(no error text)");
                } else {
                    skip |= LogError(device, kVUID_Core_Shader_InconsistentSpirv, "SPIR-V module not valid: %s",
                                     diag && diag->error ? diag->error : "(no error text)");
                }
            }
        } else {
            if (cache) {
                cache->Insert(hash);
            }
        }

        spvDiagnosticDestroy(diag);
        spvContextDestroy(ctx);
    }

    return skip;
}

bool CoreChecks::ValidateComputeWorkGroupSizes(const SHADER_MODULE_STATE *shader) const {
    bool skip = false;
    uint32_t local_size_x = 0;
    uint32_t local_size_y = 0;
    uint32_t local_size_z = 0;
    if (FindLocalSize(shader, local_size_x, local_size_y, local_size_z)) {
        if (local_size_x > phys_dev_props.limits.maxComputeWorkGroupSize[0]) {
            skip |= LogError(shader->vk_shader_module, "UNASSIGNED-features-limits-maxComputeWorkGroupSize",
                             "%s local_size_x (%" PRIu32 ") exceeds device limit maxComputeWorkGroupSize[0] (%" PRIu32 ").",
                             report_data->FormatHandle(shader->vk_shader_module).c_str(), local_size_x,
                             phys_dev_props.limits.maxComputeWorkGroupSize[0]);
        }
        if (local_size_y > phys_dev_props.limits.maxComputeWorkGroupSize[1]) {
            skip |= LogError(shader->vk_shader_module, "UNASSIGNED-features-limits-maxComputeWorkGroupSize",
                             "%s local_size_y (%" PRIu32 ") exceeds device limit maxComputeWorkGroupSize[1] (%" PRIu32 ").",
                             report_data->FormatHandle(shader->vk_shader_module).c_str(), local_size_x,
                             phys_dev_props.limits.maxComputeWorkGroupSize[1]);
        }
        if (local_size_z > phys_dev_props.limits.maxComputeWorkGroupSize[2]) {
            skip |= LogError(shader->vk_shader_module, "UNASSIGNED-features-limits-maxComputeWorkGroupSize",
                             "%s local_size_z (%" PRIu32 ") exceeds device limit maxComputeWorkGroupSize[2] (%" PRIu32 ").",
                             report_data->FormatHandle(shader->vk_shader_module).c_str(), local_size_x,
                             phys_dev_props.limits.maxComputeWorkGroupSize[2]);
        }

        uint32_t limit = phys_dev_props.limits.maxComputeWorkGroupInvocations;
        uint64_t invocations = local_size_x * local_size_y;
        // Prevent overflow.
        bool fail = false;
        if (invocations > UINT32_MAX || invocations > limit) {
            fail = true;
        }
        if (!fail) {
            invocations *= local_size_z;
            if (invocations > UINT32_MAX || invocations > limit) {
                fail = true;
            }
        }
        if (fail) {
            skip |= LogError(shader->vk_shader_module, "UNASSIGNED-features-limits-maxComputeWorkGroupInvocations",
                             "%s local_size (%" PRIu32 ", %" PRIu32 ", %" PRIu32
                             ") exceeds device limit maxComputeWorkGroupInvocations (%" PRIu32 ").",
                             report_data->FormatHandle(shader->vk_shader_module).c_str(), local_size_x, local_size_y, local_size_z,
                             limit);
        }
    }
    return skip;
}

spv_target_env PickSpirvEnv(uint32_t api_version, bool spirv_1_4) {
    if (api_version >= VK_API_VERSION_1_2) {
        return SPV_ENV_VULKAN_1_2;
    } else if (api_version >= VK_API_VERSION_1_1) {
        if (spirv_1_4) {
            return SPV_ENV_VULKAN_1_1_SPIRV_1_4;
        } else {
            return SPV_ENV_VULKAN_1_1;
        }
    }
    return SPV_ENV_VULKAN_1_0;
}

void AdjustValidatorOptions(const DeviceExtensions device_extensions, const DeviceFeatures enabled_features,
                            spvtools::ValidatorOptions &options) {
    if (device_extensions.vk_khr_relaxed_block_layout) {
        options.SetRelaxBlockLayout(true);
    }
    if (device_extensions.vk_khr_uniform_buffer_standard_layout && enabled_features.core12.uniformBufferStandardLayout == VK_TRUE) {
        options.SetUniformBufferStandardLayout(true);
    }
    if (device_extensions.vk_ext_scalar_block_layout && enabled_features.core12.scalarBlockLayout == VK_TRUE) {
        options.SetScalarBlockLayout(true);
    }
}
