/* Copyright (c) 2015-2018 The Khronos Group Inc.
 * Copyright (c) 2015-2018 Valve Corporation
 * Copyright (c) 2015-2018 LunarG, Inc.
 * Copyright (C) 2015-2018 Google Inc.
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

#include <cinttypes>
#include <cassert>
#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>
#include <SPIRV/spirv.hpp>
#include "vk_loader_platform.h"
#include "vk_enum_string_helper.h"
#include "vk_layer_data.h"
#include "vk_layer_extension_utils.h"
#include "vk_layer_utils.h"
#include "core_validation.h"
#include "core_validation_types.h"
#include "shader_validation.h"
#include "spirv-tools/libspirv.h"
#include "xxhash.h"

enum FORMAT_TYPE {
    FORMAT_TYPE_FLOAT = 1,  // UNORM, SNORM, FLOAT, USCALED, SSCALED, SRGB -- anything we consider float in the shader
    FORMAT_TYPE_SINT = 2,
    FORMAT_TYPE_UINT = 4,
};

typedef std::pair<unsigned, unsigned> location_t;

struct interface_var {
    uint32_t id;
    uint32_t type_id;
    uint32_t offset;
    bool is_patch;
    bool is_block_member;
    bool is_relaxed_precision;
    // TODO: collect the name, too? Isn't required to be present.
};

struct shader_stage_attributes {
    char const *const name;
    bool arrayed_input;
    bool arrayed_output;
};

static shader_stage_attributes shader_stage_attribs[] = {
    {"vertex shader", false, false},  {"tessellation control shader", true, true}, {"tessellation evaluation shader", true, false},
    {"geometry shader", true, false}, {"fragment shader", false, false},
};

// SPIRV utility functions
void shader_module::BuildDefIndex() {
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
            case spv::OpTypeAccelerationStructureNVX:
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
        case spv::ExecutionModelRayGenerationNVX:
            return VK_SHADER_STAGE_RAYGEN_BIT_NVX;
        case spv::ExecutionModelAnyHitNVX:
            return VK_SHADER_STAGE_ANY_HIT_BIT_NVX;
        case spv::ExecutionModelClosestHitNVX:
            return VK_SHADER_STAGE_CLOSEST_HIT_BIT_NVX;
        case spv::ExecutionModelMissNVX:
            return VK_SHADER_STAGE_MISS_BIT_NVX;
        case spv::ExecutionModelIntersectionNVX:
            return VK_SHADER_STAGE_INTERSECTION_BIT_NVX;
        case spv::ExecutionModelCallableNVX:
            return VK_SHADER_STAGE_CALLABLE_BIT_NVX;
        case spv::ExecutionModelTaskNV:
            return VK_SHADER_STAGE_TASK_BIT_NV;
        case spv::ExecutionModelMeshNV:
            return VK_SHADER_STAGE_MESH_BIT_NV;
        default:
            return 0;
    }
}

static spirv_inst_iter FindEntrypoint(shader_module const *src, char const *name, VkShaderStageFlagBits stageBits) {
    for (auto insn : *src) {
        if (insn.opcode() == spv::OpEntryPoint) {
            auto entrypointName = (char const *)&insn.word(3);
            auto executionModel = insn.word(1);
            auto entrypointStageBits = ExecutionModelToShaderStageFlagBits(executionModel);

            if (!strcmp(entrypointName, name) && (entrypointStageBits & stageBits)) {
                return insn;
            }
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
unsigned GetConstantValue(shader_module const *src, unsigned id) {
    auto value = src->get_def(id);
    assert(value != src->end());

    if (value.opcode() != spv::OpConstant) {
        // TODO: Either ensure that the specialization transform is already performed on a module we're
        //       considering here, OR -- specialize on the fly now.
        return 1;
    }

    return value.word(3);
}

static void DescribeTypeInner(std::ostringstream &ss, shader_module const *src, unsigned type) {
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
        case spv::OpTypeAccelerationStructureNVX:
            ss << "accelerationStruture";
            break;
        default:
            ss << "oddtype";
            break;
    }
}

static std::string DescribeType(shader_module const *src, unsigned type) {
    std::ostringstream ss;
    DescribeTypeInner(ss, src, type);
    return ss.str();
}

static bool IsNarrowNumericType(spirv_inst_iter type) {
    if (type.opcode() != spv::OpTypeInt && type.opcode() != spv::OpTypeFloat) return false;
    return type.word(2) < 64;
}

static bool TypesMatch(shader_module const *a, shader_module const *b, unsigned a_type, unsigned b_type, bool a_arrayed,
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

static unsigned ValueOrDefault(std::unordered_map<unsigned, unsigned> const &map, unsigned id, unsigned def) {
    auto it = map.find(id);
    if (it == map.end())
        return def;
    else
        return it->second;
}

static unsigned GetLocationsConsumedByType(shader_module const *src, unsigned type, bool strip_array_level) {
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
static unsigned GetFundamentalType(shader_module const *src, unsigned type) {
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

static spirv_inst_iter GetStructType(shader_module const *src, spirv_inst_iter def, bool is_array_of_verts) {
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

static bool CollectInterfaceBlockMembers(shader_module const *src, std::map<location_t, interface_var> *out,
                                         std::unordered_map<unsigned, unsigned> const &blocks, bool is_array_of_verts, uint32_t id,
                                         uint32_t type_id, bool is_patch, int /*first_location*/) {
    // Walk down the type_id presented, trying to determine whether it's actually an interface block.
    auto type = GetStructType(src, src->get_def(type_id), is_array_of_verts && !is_patch);
    if (type == src->end() || blocks.find(type.word(1)) == blocks.end()) {
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

static std::map<location_t, interface_var> CollectInterfaceByLocation(shader_module const *src, spirv_inst_iter entrypoint,
                                                                      spv::StorageClass sinterface, bool is_array_of_verts) {
    std::unordered_map<unsigned, unsigned> var_locations;
    std::unordered_map<unsigned, unsigned> var_builtins;
    std::unordered_map<unsigned, unsigned> var_components;
    std::unordered_map<unsigned, unsigned> blocks;
    std::unordered_map<unsigned, unsigned> var_patch;
    std::unordered_map<unsigned, unsigned> var_relaxed_precision;

    for (auto insn : *src) {
        // We consider two interface models: SSO rendezvous-by-location, and builtins. Complain about anything that
        // fits neither model.
        if (insn.opcode() == spv::OpDecorate) {
            if (insn.word(2) == spv::DecorationLocation) {
                var_locations[insn.word(1)] = insn.word(3);
            }

            if (insn.word(2) == spv::DecorationBuiltIn) {
                var_builtins[insn.word(1)] = insn.word(3);
            }

            if (insn.word(2) == spv::DecorationComponent) {
                var_components[insn.word(1)] = insn.word(3);
            }

            if (insn.word(2) == spv::DecorationBlock) {
                blocks[insn.word(1)] = 1;
            }

            if (insn.word(2) == spv::DecorationPatch) {
                var_patch[insn.word(1)] = 1;
            }

            if (insn.word(2) == spv::DecorationRelaxedPrecision) {
                var_relaxed_precision[insn.word(1)] = 1;
            }
        }
    }

    // TODO: handle grouped decorations
    // TODO: handle index=1 dual source outputs from FS -- two vars will have the same location, and we DON'T want to clobber.

    // Find the end of the entrypoint's name string. additional zero bytes follow the actual null terminator, to fill out the
    // rest of the word - so we only need to look at the last byte in the word to determine which word contains the terminator.
    uint32_t word = 3;
    while (entrypoint.word(word) & 0xff000000u) {
        ++word;
    }
    ++word;

    std::map<location_t, interface_var> out;

    for (; word < entrypoint.len(); word++) {
        auto insn = src->get_def(entrypoint.word(word));
        assert(insn != src->end());
        assert(insn.opcode() == spv::OpVariable);

        if (insn.word(3) == static_cast<uint32_t>(sinterface)) {
            unsigned id = insn.word(2);
            unsigned type = insn.word(1);

            int location = ValueOrDefault(var_locations, id, static_cast<unsigned>(-1));
            int builtin = ValueOrDefault(var_builtins, id, static_cast<unsigned>(-1));
            unsigned component = ValueOrDefault(var_components, id, 0);  // Unspecified is OK, is 0
            bool is_patch = var_patch.find(id) != var_patch.end();
            bool is_relaxed_precision = var_relaxed_precision.find(id) != var_relaxed_precision.end();

            if (builtin != -1)
                continue;
            else if (!CollectInterfaceBlockMembers(src, &out, blocks, is_array_of_verts, id, type, is_patch, location)) {
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

static std::vector<std::pair<uint32_t, interface_var>> CollectInterfaceByInputAttachmentIndex(
    shader_module const *src, std::unordered_set<uint32_t> const &accessible_ids) {
    std::vector<std::pair<uint32_t, interface_var>> out;

    for (auto insn : *src) {
        if (insn.opcode() == spv::OpDecorate) {
            if (insn.word(2) == spv::DecorationInputAttachmentIndex) {
                auto attachment_index = insn.word(3);
                auto id = insn.word(1);

                if (accessible_ids.count(id)) {
                    auto def = src->get_def(id);
                    assert(def != src->end());

                    if (def.opcode() == spv::OpVariable && insn.word(3) == spv::StorageClassUniformConstant) {
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

static bool IsWritableDescriptorType(shader_module const *module, uint32_t type_id, bool is_storage_buffer) {
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
            return sampled == 2 && dim != spv::DimSubpassData;
        }

        case spv::OpTypeStruct: {
            std::unordered_set<unsigned> nonwritable_members;
            for (auto insn : *module) {
                if (insn.opcode() == spv::OpDecorate && insn.word(1) == type.word(1)) {
                    if (insn.word(2) == spv::DecorationBufferBlock) {
                        // Legacy storage block in the Uniform storage class
                        // has its struct type decorated with BufferBlock.
                        is_storage_buffer = true;
                    }
                } else if (insn.opcode() == spv::OpMemberDecorate && insn.word(1) == type.word(1) &&
                           insn.word(3) == spv::DecorationNonWritable) {
                    nonwritable_members.insert(insn.word(2));
                }
            }

            // A buffer is writable if it's either flavor of storage buffer, and has any member not decorated
            // as nonwritable.
            return is_storage_buffer && nonwritable_members.size() != type.len() - 2;
        }
    }

    return false;
}

static std::vector<std::pair<descriptor_slot_t, interface_var>> CollectInterfaceByDescriptorSlot(
    debug_report_data const *report_data, shader_module const *src, std::unordered_set<uint32_t> const &accessible_ids,
    bool *has_writable_descriptor) {
    std::unordered_map<unsigned, unsigned> var_sets;
    std::unordered_map<unsigned, unsigned> var_bindings;
    std::unordered_map<unsigned, unsigned> var_nonwritable;

    for (auto insn : *src) {
        // All variables in the Uniform or UniformConstant storage classes are required to be decorated with both
        // DecorationDescriptorSet and DecorationBinding.
        if (insn.opcode() == spv::OpDecorate) {
            if (insn.word(2) == spv::DecorationDescriptorSet) {
                var_sets[insn.word(1)] = insn.word(3);
            }

            if (insn.word(2) == spv::DecorationBinding) {
                var_bindings[insn.word(1)] = insn.word(3);
            }

            // Note: do toplevel DecorationNonWritable out here; it applies to
            // the OpVariable rather than the type.
            if (insn.word(2) == spv::DecorationNonWritable) {
                var_nonwritable[insn.word(1)] = 1;
            }
        }
    }

    std::vector<std::pair<descriptor_slot_t, interface_var>> out;

    for (auto id : accessible_ids) {
        auto insn = src->get_def(id);
        assert(insn != src->end());

        if (insn.opcode() == spv::OpVariable &&
            (insn.word(3) == spv::StorageClassUniform || insn.word(3) == spv::StorageClassUniformConstant ||
             insn.word(3) == spv::StorageClassStorageBuffer)) {
            unsigned set = ValueOrDefault(var_sets, insn.word(2), 0);
            unsigned binding = ValueOrDefault(var_bindings, insn.word(2), 0);

            interface_var v = {};
            v.id = insn.word(2);
            v.type_id = insn.word(1);
            out.emplace_back(std::make_pair(set, binding), v);

            if (var_nonwritable.find(id) == var_nonwritable.end() &&
                IsWritableDescriptorType(src, insn.word(1), insn.word(3) == spv::StorageClassStorageBuffer)) {
                *has_writable_descriptor = true;
            }
        }
    }

    return out;
}

static bool ValidateViConsistency(debug_report_data const *report_data, VkPipelineVertexInputStateCreateInfo const *vi) {
    // Walk the binding descriptions, which describe the step rate and stride of each vertex buffer.  Each binding should
    // be specified only once.
    std::unordered_map<uint32_t, VkVertexInputBindingDescription const *> bindings;
    bool skip = false;

    for (unsigned i = 0; i < vi->vertexBindingDescriptionCount; i++) {
        auto desc = &vi->pVertexBindingDescriptions[i];
        auto &binding = bindings[desc->binding];
        if (binding) {
            // TODO: "VUID-VkGraphicsPipelineCreateInfo-pStages-00742" perhaps?
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            kVUID_Core_Shader_InconsistentVi, "Duplicate vertex input binding descriptions for binding %d",
                            desc->binding);
        } else {
            binding = desc;
        }
    }

    return skip;
}

static bool ValidateViAgainstVsInputs(debug_report_data const *report_data, VkPipelineVertexInputStateCreateInfo const *vi,
                                      shader_module const *vs, spirv_inst_iter entrypoint) {
    bool skip = false;

    auto inputs = CollectInterfaceByLocation(vs, entrypoint, spv::StorageClassInput, false);

    // Build index by location
    std::map<uint32_t, VkVertexInputAttributeDescription const *> attribs;
    if (vi) {
        for (unsigned i = 0; i < vi->vertexAttributeDescriptionCount; i++) {
            auto num_locations = GetLocationsConsumedByFormat(vi->pVertexAttributeDescriptions[i].format);
            for (auto j = 0u; j < num_locations; j++) {
                attribs[vi->pVertexAttributeDescriptions[i].location + j] = &vi->pVertexAttributeDescriptions[i];
            }
        }
    }

    auto it_a = attribs.begin();
    auto it_b = inputs.begin();
    bool used = false;

    while ((attribs.size() > 0 && it_a != attribs.end()) || (inputs.size() > 0 && it_b != inputs.end())) {
        bool a_at_end = attribs.size() == 0 || it_a == attribs.end();
        bool b_at_end = inputs.size() == 0 || it_b == inputs.end();
        auto a_first = a_at_end ? 0 : it_a->first;
        auto b_first = b_at_end ? 0 : it_b->first.first;

        if (!a_at_end && (b_at_end || a_first < b_first)) {
            if (!used &&
                log_msg(report_data, VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT,
                        HandleToUint64(vs->vk_shader_module), kVUID_Core_Shader_OutputNotConsumed,
                        "Vertex attribute at location %d not consumed by vertex shader", a_first)) {
                skip = true;
            }
            used = false;
            it_a++;
        } else if (!b_at_end && (a_at_end || b_first < a_first)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT,
                            HandleToUint64(vs->vk_shader_module), kVUID_Core_Shader_InputNotProduced,
                            "Vertex shader consumes input at location %d but not provided", b_first);
            it_b++;
        } else {
            unsigned attrib_type = GetFormatType(it_a->second->format);
            unsigned input_type = GetFundamentalType(vs, it_b->second.type_id);

            // Type checking
            if (!(attrib_type & input_type)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT,
                                HandleToUint64(vs->vk_shader_module), kVUID_Core_Shader_InterfaceTypeMismatch,
                                "Attribute type of `%s` at location %d does not match vertex shader input type of `%s`",
                                string_VkFormat(it_a->second->format), a_first, DescribeType(vs, it_b->second.type_id).c_str());
            }

            // OK!
            used = true;
            it_b++;
        }
    }

    return skip;
}

static bool ValidateFsOutputsAgainstRenderPass(debug_report_data const *report_data, shader_module const *fs,
                                               spirv_inst_iter entrypoint, PIPELINE_STATE const *pipeline, uint32_t subpass_index) {
    auto rpci = pipeline->rp_state->createInfo.ptr();

    std::map<uint32_t, VkFormat> color_attachments;
    auto subpass = rpci->pSubpasses[subpass_index];
    for (auto i = 0u; i < subpass.colorAttachmentCount; ++i) {
        uint32_t attachment = subpass.pColorAttachments[i].attachment;
        if (attachment == VK_ATTACHMENT_UNUSED) continue;
        if (rpci->pAttachments[attachment].format != VK_FORMAT_UNDEFINED) {
            color_attachments[i] = rpci->pAttachments[attachment].format;
        }
    }

    bool skip = false;

    // TODO: dual source blend index (spv::DecIndex, zero if not provided)

    auto outputs = CollectInterfaceByLocation(fs, entrypoint, spv::StorageClassOutput, false);

    auto it_a = outputs.begin();
    auto it_b = color_attachments.begin();
    bool used = false;

    // Walk attachment list and outputs together

    while ((outputs.size() > 0 && it_a != outputs.end()) || (color_attachments.size() > 0 && it_b != color_attachments.end())) {
        bool a_at_end = outputs.size() == 0 || it_a == outputs.end();
        bool b_at_end = color_attachments.size() == 0 || it_b == color_attachments.end();

        if (!a_at_end && (b_at_end || it_a->first.first < it_b->first)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT,
                            HandleToUint64(fs->vk_shader_module), kVUID_Core_Shader_OutputNotConsumed,
                            "fragment shader writes to output location %d with no matching attachment", it_a->first.first);
            it_a++;
        } else if (!b_at_end && (a_at_end || it_a->first.first > it_b->first)) {
            // Only complain if there are unmasked channels for this attachment. If the writemask is 0, it's acceptable for the
            // shader to not produce a matching output.
            if (!used) {
                if (pipeline->attachments[it_b->first].colorWriteMask != 0) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT,
                                    HandleToUint64(fs->vk_shader_module), kVUID_Core_Shader_InputNotProduced,
                                    "Attachment %d not written by fragment shader; undefined values will be written to attachment",
                                    it_b->first);
                }
            }
            used = false;
            it_b++;
        } else {
            unsigned output_type = GetFundamentalType(fs, it_a->second.type_id);
            unsigned att_type = GetFormatType(it_b->second);

            // Type checking
            if (!(output_type & att_type)) {
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT,
                    HandleToUint64(fs->vk_shader_module), kVUID_Core_Shader_InterfaceTypeMismatch,
                    "Attachment %d of type `%s` does not match fragment shader output type of `%s`; resulting values are undefined",
                    it_b->first, string_VkFormat(it_b->second), DescribeType(fs, it_a->second.type_id).c_str());
            }

            // OK!
            it_a++;
            used = true;
        }
    }

    return skip;
}

// For PointSize analysis we need to know if the variable decorated with the PointSize built-in was actually written to.
// This function examines instructions in the static call tree for a write to this variable.
static bool IsPointSizeWritten(shader_module const *src, spirv_inst_iter builtin_instr, spirv_inst_iter entrypoint) {
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
static std::unordered_set<uint32_t> MarkAccessibleIds(shader_module const *src, spirv_inst_iter entrypoint) {
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
                        case spv::OpAtomicLoad:
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
                            worklist.insert(insn.word(3));  // ptr
                            break;
                        case spv::OpStore:
                        case spv::OpAtomicStore:
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
                    }
                }
                break;
        }
    }

    return ids;
}

static bool ValidatePushConstantBlockAgainstPipeline(debug_report_data const *report_data,
                                                     std::vector<VkPushConstantRange> const *push_constant_ranges,
                                                     shader_module const *src, spirv_inst_iter type, VkShaderStageFlagBits stage) {
    bool skip = false;

    // Strip off ptrs etc
    type = GetStructType(src, type, false);
    assert(type != src->end());

    // Validate directly off the offsets. this isn't quite correct for arrays and matrices, but is a good first step.
    // TODO: arrays, matrices, weird sizes
    for (auto insn : *src) {
        if (insn.opcode() == spv::OpMemberDecorate && insn.word(1) == type.word(1)) {
            if (insn.word(3) == spv::DecorationOffset) {
                unsigned offset = insn.word(4);
                auto size = 4;  // Bytes; TODO: calculate this based on the type

                bool found_range = false;
                for (auto const &range : *push_constant_ranges) {
                    if (range.offset <= offset && range.offset + range.size >= offset + size) {
                        found_range = true;

                        if ((range.stageFlags & stage) == 0) {
                            skip |=
                                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        kVUID_Core_Shader_PushConstantNotAccessibleFromStage,
                                        "Push constant range covering variable starting at offset %u not accessible from stage %s",
                                        offset, string_VkShaderStageFlagBits(stage));
                        }

                        break;
                    }
                }

                if (!found_range) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    kVUID_Core_Shader_PushConstantOutOfRange,
                                    "Push constant range covering variable starting at offset %u not declared in layout", offset);
                }
            }
        }
    }

    return skip;
}

static bool ValidatePushConstantUsage(debug_report_data const *report_data,
                                      std::vector<VkPushConstantRange> const *push_constant_ranges, shader_module const *src,
                                      std::unordered_set<uint32_t> accessible_ids, VkShaderStageFlagBits stage) {
    bool skip = false;

    for (auto id : accessible_ids) {
        auto def_insn = src->get_def(id);
        if (def_insn.opcode() == spv::OpVariable && def_insn.word(3) == spv::StorageClassPushConstant) {
            skip |= ValidatePushConstantBlockAgainstPipeline(report_data, push_constant_ranges, src, src->get_def(def_insn.word(1)),
                                                             stage);
        }
    }

    return skip;
}

// Validate that data for each specialization entry is fully contained within the buffer.
static bool ValidateSpecializationOffsets(debug_report_data const *report_data, VkPipelineShaderStageCreateInfo const *info) {
    bool skip = false;

    VkSpecializationInfo const *spec = info->pSpecializationInfo;

    if (spec) {
        for (auto i = 0u; i < spec->mapEntryCount; i++) {
            // TODO: This is a good place for "VUID-VkSpecializationInfo-offset-00773".
            if (spec->pMapEntries[i].offset + spec->pMapEntries[i].size > spec->dataSize) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, 0,
                                "VUID-VkSpecializationInfo-pMapEntries-00774",
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
static std::set<uint32_t> TypeToDescriptorTypeSet(shader_module const *module, uint32_t type_id, unsigned &descriptor_count) {
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
        case spv::OpTypeAccelerationStructureNVX:
            ret.insert(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NVX);
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

static bool RequireFeature(debug_report_data const *report_data, VkBool32 feature, char const *feature_name) {
    if (!feature) {
        if (log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                    kVUID_Core_Shader_FeatureNotEnabled, "Shader requires %s but is not enabled on the device", feature_name)) {
            return true;
        }
    }

    return false;
}

static bool RequireExtension(debug_report_data const *report_data, bool extension, char const *extension_name) {
    if (!extension) {
        if (log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                    kVUID_Core_Shader_FeatureNotEnabled, "Shader requires extension %s but is not enabled on the device",
                    extension_name)) {
            return true;
        }
    }

    return false;
}

static bool ValidateShaderCapabilities(layer_data *dev_data, shader_module const *src, VkShaderStageFlagBits stage,
                                       bool has_writable_descriptor) {
    bool skip = false;

    auto report_data = GetReportData(dev_data);
    auto const &features = GetEnabledFeatures(dev_data);
    auto const &extensions = GetDeviceExtensions(dev_data);

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
        FeaturePointer(VkBool32 VkPhysicalDeviceDescriptorIndexingFeaturesEXT::*ptr)
            : IsEnabled([=](const DeviceFeatures &features) { return features.descriptor_indexing.*ptr; }) {}
        FeaturePointer(VkBool32 VkPhysicalDevice8BitStorageFeaturesKHR::*ptr)
            : IsEnabled([=](const DeviceFeatures &features) { return features.eight_bit_storage.*ptr; }) {}
    };

    struct CapabilityInfo {
        char const *name;
        FeaturePointer feature;
        bool DeviceExtensions::*extension;
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
        {spv::CapabilityStorageImageExtendedFormats, {"VkPhysicalDeviceFeatures::shaderStorageImageExtendedFormats", &VkPhysicalDeviceFeatures::shaderStorageImageExtendedFormats}},
        {spv::CapabilityInterpolationFunction, {"VkPhysicalDeviceFeatures::sampleRateShading", &VkPhysicalDeviceFeatures::sampleRateShading}},
        {spv::CapabilityStorageImageReadWithoutFormat, {"VkPhysicalDeviceFeatures::shaderStorageImageReadWithoutFormat", &VkPhysicalDeviceFeatures::shaderStorageImageReadWithoutFormat}},
        {spv::CapabilityStorageImageWriteWithoutFormat, {"VkPhysicalDeviceFeatures::shaderStorageImageWriteWithoutFormat", &VkPhysicalDeviceFeatures::shaderStorageImageWriteWithoutFormat}},
        {spv::CapabilityMultiViewport, {"VkPhysicalDeviceFeatures::multiViewport", &VkPhysicalDeviceFeatures::multiViewport}},

        {spv::CapabilityShaderNonUniformEXT, {VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME, nullptr, &DeviceExtensions::vk_ext_descriptor_indexing}},
        {spv::CapabilityRuntimeDescriptorArrayEXT, {"VkPhysicalDeviceDescriptorIndexingFeaturesEXT::runtimeDescriptorArray", &VkPhysicalDeviceDescriptorIndexingFeaturesEXT::runtimeDescriptorArray}},
        {spv::CapabilityInputAttachmentArrayDynamicIndexingEXT, {"VkPhysicalDeviceDescriptorIndexingFeaturesEXT::shaderInputAttachmentArrayDynamicIndexing", &VkPhysicalDeviceDescriptorIndexingFeaturesEXT::shaderInputAttachmentArrayDynamicIndexing}},
        {spv::CapabilityUniformTexelBufferArrayDynamicIndexingEXT, {"VkPhysicalDeviceDescriptorIndexingFeaturesEXT::shaderUniformTexelBufferArrayDynamicIndexing", &VkPhysicalDeviceDescriptorIndexingFeaturesEXT::shaderUniformTexelBufferArrayDynamicIndexing}},
        {spv::CapabilityStorageTexelBufferArrayDynamicIndexingEXT, {"VkPhysicalDeviceDescriptorIndexingFeaturesEXT::shaderStorageTexelBufferArrayDynamicIndexing", &VkPhysicalDeviceDescriptorIndexingFeaturesEXT::shaderStorageTexelBufferArrayDynamicIndexing}},
        {spv::CapabilityUniformBufferArrayNonUniformIndexingEXT, {"VkPhysicalDeviceDescriptorIndexingFeaturesEXT::shaderUniformBufferArrayNonUniformIndexing", &VkPhysicalDeviceDescriptorIndexingFeaturesEXT::shaderUniformBufferArrayNonUniformIndexing}},
        {spv::CapabilitySampledImageArrayNonUniformIndexingEXT, {"VkPhysicalDeviceDescriptorIndexingFeaturesEXT::shaderSampledImageArrayNonUniformIndexing", &VkPhysicalDeviceDescriptorIndexingFeaturesEXT::shaderSampledImageArrayNonUniformIndexing}},
        {spv::CapabilityStorageBufferArrayNonUniformIndexingEXT, {"VkPhysicalDeviceDescriptorIndexingFeaturesEXT::shaderStorageBufferArrayNonUniformIndexing", &VkPhysicalDeviceDescriptorIndexingFeaturesEXT::shaderStorageBufferArrayNonUniformIndexing}},
        {spv::CapabilityStorageImageArrayNonUniformIndexingEXT, {"VkPhysicalDeviceDescriptorIndexingFeaturesEXT::shaderStorageImageArrayNonUniformIndexing", &VkPhysicalDeviceDescriptorIndexingFeaturesEXT::shaderStorageImageArrayNonUniformIndexing}},
        {spv::CapabilityInputAttachmentArrayNonUniformIndexingEXT, {"VkPhysicalDeviceDescriptorIndexingFeaturesEXT::shaderInputAttachmentArrayNonUniformIndexing", &VkPhysicalDeviceDescriptorIndexingFeaturesEXT::shaderInputAttachmentArrayNonUniformIndexing}},
        {spv::CapabilityUniformTexelBufferArrayNonUniformIndexingEXT, {"VkPhysicalDeviceDescriptorIndexingFeaturesEXT::shaderUniformTexelBufferArrayNonUniformIndexing", &VkPhysicalDeviceDescriptorIndexingFeaturesEXT::shaderUniformTexelBufferArrayNonUniformIndexing}},
        {spv::CapabilityStorageTexelBufferArrayNonUniformIndexingEXT , {"VkPhysicalDeviceDescriptorIndexingFeaturesEXT::shaderStorageTexelBufferArrayNonUniformIndexing", &VkPhysicalDeviceDescriptorIndexingFeaturesEXT::shaderStorageTexelBufferArrayNonUniformIndexing}},

        // Capabilities that require an extension
        {spv::CapabilityDrawParameters, {VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME, nullptr, &DeviceExtensions::vk_khr_shader_draw_parameters}},
        {spv::CapabilityGeometryShaderPassthroughNV, {VK_NV_GEOMETRY_SHADER_PASSTHROUGH_EXTENSION_NAME, nullptr, &DeviceExtensions::vk_nv_geometry_shader_passthrough}},
        {spv::CapabilitySampleMaskOverrideCoverageNV, {VK_NV_SAMPLE_MASK_OVERRIDE_COVERAGE_EXTENSION_NAME, nullptr, &DeviceExtensions::vk_nv_sample_mask_override_coverage}},
        {spv::CapabilityShaderViewportIndexLayerEXT, {VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME, nullptr, &DeviceExtensions::vk_ext_shader_viewport_index_layer}},
        {spv::CapabilityShaderViewportIndexLayerNV, {VK_NV_VIEWPORT_ARRAY2_EXTENSION_NAME, nullptr, &DeviceExtensions::vk_nv_viewport_array2}},
        {spv::CapabilityShaderViewportMaskNV, {VK_NV_VIEWPORT_ARRAY2_EXTENSION_NAME, nullptr, &DeviceExtensions::vk_nv_viewport_array2}},
        {spv::CapabilitySubgroupBallotKHR, {VK_EXT_SHADER_SUBGROUP_BALLOT_EXTENSION_NAME, nullptr, &DeviceExtensions::vk_ext_shader_subgroup_ballot }},
        {spv::CapabilitySubgroupVoteKHR, {VK_EXT_SHADER_SUBGROUP_VOTE_EXTENSION_NAME, nullptr, &DeviceExtensions::vk_ext_shader_subgroup_vote }},
        {spv::CapabilityInt64Atomics, {VK_KHR_SHADER_ATOMIC_INT64_EXTENSION_NAME, nullptr, &DeviceExtensions::vk_khr_shader_atomic_int64 }},

        {spv::CapabilityStorageBuffer8BitAccess , {"VkPhysicalDevice8BitStorageFeaturesKHR::storageBuffer8BitAccess", &VkPhysicalDevice8BitStorageFeaturesKHR::storageBuffer8BitAccess, &DeviceExtensions::vk_khr_8bit_storage}},
        {spv::CapabilityUniformAndStorageBuffer8BitAccess , {"VkPhysicalDevice8BitStorageFeaturesKHR::uniformAndStorageBuffer8BitAccess", &VkPhysicalDevice8BitStorageFeaturesKHR::uniformAndStorageBuffer8BitAccess, &DeviceExtensions::vk_khr_8bit_storage}},
        {spv::CapabilityStoragePushConstant8 , {"VkPhysicalDevice8BitStorageFeaturesKHR::storagePushConstant8", &VkPhysicalDevice8BitStorageFeaturesKHR::storagePushConstant8, &DeviceExtensions::vk_khr_8bit_storage}},
    };
    // clang-format on

    for (auto insn : *src) {
        if (insn.opcode() == spv::OpCapability) {
            size_t n = capabilities.count(insn.word(1));
            if (1 == n) {  // key occurs exactly once
                auto it = capabilities.find(insn.word(1));
                if (it != capabilities.end()) {
                    if (it->second.feature) {
                        skip |= RequireFeature(report_data, it->second.feature.IsEnabled(*features), it->second.name);
                    }
                    if (it->second.extension) {
                        skip |= RequireExtension(report_data, extensions->*(it->second.extension), it->second.name);
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
                        has_feature = has_feature || it->second.feature.IsEnabled(*features);
                        feature_names += it->second.name;
                        feature_names += " ";
                    }
                    if (it->second.extension) {
                        needs_ext = true;
                        has_ext = has_ext || extensions->*(it->second.extension);
                        extension_names += it->second.name;
                        extension_names += " ";
                    }
                }
                if (needs_feature) {
                    feature_names += "]";
                    skip |= RequireFeature(report_data, has_feature, feature_names.c_str());
                }
                if (needs_ext) {
                    extension_names += "]";
                    skip |= RequireExtension(report_data, has_ext, extension_names.c_str());
                }
            }
        }
    }

    if (has_writable_descriptor) {
        switch (stage) {
            case VK_SHADER_STAGE_COMPUTE_BIT:
                /* No feature requirements for writes and atomics from compute
                 * stage */
                break;
            case VK_SHADER_STAGE_FRAGMENT_BIT:
                skip |= RequireFeature(report_data, features->core.fragmentStoresAndAtomics, "fragmentStoresAndAtomics");
                break;
            default:
                skip |=
                    RequireFeature(report_data, features->core.vertexPipelineStoresAndAtomics, "vertexPipelineStoresAndAtomics");
                break;
        }
    }

    return skip;
}

static uint32_t DescriptorTypeToReqs(shader_module const *module, uint32_t type_id) {
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
static VkDescriptorSetLayoutBinding const *GetDescriptorBinding(PIPELINE_LAYOUT_NODE const *pipelineLayout,
                                                                descriptor_slot_t slot) {
    if (!pipelineLayout) return nullptr;

    if (slot.first >= pipelineLayout->set_layouts.size()) return nullptr;

    return pipelineLayout->set_layouts[slot.first]->GetDescriptorSetLayoutBindingPtrFromBinding(slot.second);
}

static void ProcessExecutionModes(shader_module const *src, spirv_inst_iter entrypoint, PIPELINE_STATE *pipeline) {
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
bool ValidatePointListShaderState(const layer_data *dev_data, const PIPELINE_STATE *pipeline, shader_module const *src,
                                  spirv_inst_iter entrypoint, VkShaderStageFlagBits stage) {
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
        !GetEnabledFeatures(dev_data)->core.shaderTessellationAndGeometryPointSize) {
        if (pointsize_written) {
            skip |= log_msg(GetReportData(dev_data), VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                            HandleToUint64(pipeline->pipeline), kVUID_Core_Shader_PointSizeBuiltInOverSpecified,
                            "Pipeline topology is set to POINT_LIST and geometry or tessellation shaders write PointSize which "
                            "is prohibited when the shaderTessellationAndGeometryPointSize feature is not enabled.");
        }
    } else if (!pointsize_written) {
        skip |=
            log_msg(GetReportData(dev_data), VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
                    HandleToUint64(pipeline->pipeline), kVUID_Core_Shader_MissingPointSizeBuiltIn,
                    "Pipeline topology is set to POINT_LIST, but PointSize is not written to in the shader corresponding to %s.",
                    string_VkShaderStageFlagBits(stage));
    }
    return skip;
}

static bool ValidatePipelineShaderStage(layer_data *dev_data, VkPipelineShaderStageCreateInfo const *pStage,
                                        PIPELINE_STATE *pipeline, shader_module const **out_module, spirv_inst_iter *out_entrypoint,
                                        bool check_point_size) {
    bool skip = false;
    auto module = *out_module = GetShaderModuleState(dev_data, pStage->module);
    auto report_data = GetReportData(dev_data);

    if (!module->has_valid_spirv) return false;

    // Find the entrypoint
    auto entrypoint = *out_entrypoint = FindEntrypoint(module, pStage->pName, pStage->stage);
    if (entrypoint == module->end()) {
        if (log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                    "VUID-VkPipelineShaderStageCreateInfo-pName-00707", "No entrypoint found named `%s` for stage %s..",
                    pStage->pName, string_VkShaderStageFlagBits(pStage->stage))) {
            return true;  // no point continuing beyond here, any analysis is just going to be garbage.
        }
    }

    // Mark accessible ids
    auto accessible_ids = MarkAccessibleIds(module, entrypoint);
    ProcessExecutionModes(module, entrypoint, pipeline);

    // Validate descriptor set layout against what the entrypoint actually uses
    bool has_writable_descriptor = false;
    auto descriptor_uses = CollectInterfaceByDescriptorSlot(report_data, module, accessible_ids, &has_writable_descriptor);

    // Validate shader capabilities against enabled device features
    skip |= ValidateShaderCapabilities(dev_data, module, pStage->stage, has_writable_descriptor);

    skip |= ValidateSpecializationOffsets(report_data, pStage);
    skip |= ValidatePushConstantUsage(report_data, pipeline->pipeline_layout.push_constant_ranges.get(), module, accessible_ids,
                                      pStage->stage);
    if (check_point_size && !pipeline->graphicsPipelineCI.pRasterizationState->rasterizerDiscardEnable) {
        skip |= ValidatePointListShaderState(dev_data, pipeline, module, entrypoint, pStage->stage);
    }

    // Validate descriptor use
    for (auto use : descriptor_uses) {
        // While validating shaders capture which slots are used by the pipeline
        auto &reqs = pipeline->active_slots[use.first.first][use.first.second];
        reqs = descriptor_req(reqs | DescriptorTypeToReqs(module, use.second.type_id));

        // Verify given pipelineLayout has requested setLayout with requested binding
        const auto &binding = GetDescriptorBinding(&pipeline->pipeline_layout, use.first);
        unsigned required_descriptor_count;
        std::set<uint32_t> descriptor_types = TypeToDescriptorTypeSet(module, use.second.type_id, required_descriptor_count);

        if (!binding) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            kVUID_Core_Shader_MissingDescriptor,
                            "Shader uses descriptor slot %u.%u (expected `%s`) but not declared in pipeline layout",
                            use.first.first, use.first.second, string_descriptorTypes(descriptor_types).c_str());
        } else if (~binding->stageFlags & pStage->stage) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, 0,
                            kVUID_Core_Shader_DescriptorNotAccessibleFromStage,
                            "Shader uses descriptor slot %u.%u but descriptor not accessible from stage %s", use.first.first,
                            use.first.second, string_VkShaderStageFlagBits(pStage->stage));
        } else if (descriptor_types.find(binding->descriptorType) == descriptor_types.end()) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            kVUID_Core_Shader_DescriptorTypeMismatch,
                            "Type mismatch on descriptor slot %u.%u (expected `%s`) but descriptor of type %s", use.first.first,
                            use.first.second, string_descriptorTypes(descriptor_types).c_str(),
                            string_VkDescriptorType(binding->descriptorType));
        } else if (binding->descriptorCount < required_descriptor_count) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            kVUID_Core_Shader_DescriptorTypeMismatch,
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
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                kVUID_Core_Shader_MissingInputAttachment,
                                "Shader consumes input attachment index %d but not provided in subpass", use.first);
            } else if (!(GetFormatType(rpci->pAttachments[index].format) & GetFundamentalType(module, use.second.type_id))) {
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            kVUID_Core_Shader_InputAttachmentTypeMismatch,
                            "Subpass input attachment %u format of %s does not match type used in shader `%s`", use.first,
                            string_VkFormat(rpci->pAttachments[index].format), DescribeType(module, use.second.type_id).c_str());
            }
        }
    }

    return skip;
}

static bool ValidateInterfaceBetweenStages(debug_report_data const *report_data, shader_module const *producer,
                                           spirv_inst_iter producer_entrypoint, shader_stage_attributes const *producer_stage,
                                           shader_module const *consumer, spirv_inst_iter consumer_entrypoint,
                                           shader_stage_attributes const *consumer_stage) {
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
            skip |= log_msg(report_data, VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT,
                            HandleToUint64(producer->vk_shader_module), kVUID_Core_Shader_OutputNotConsumed,
                            "%s writes to output location %u.%u which is not consumed by %s", producer_stage->name, a_first.first,
                            a_first.second, consumer_stage->name);
            a_it++;
        } else if (a_at_end || a_first > b_first) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT,
                            HandleToUint64(consumer->vk_shader_module), kVUID_Core_Shader_InputNotProduced,
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
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT,
                                HandleToUint64(producer->vk_shader_module), kVUID_Core_Shader_InterfaceTypeMismatch,
                                "Type mismatch on location %u.%u: '%s' vs '%s'", a_first.first, a_first.second,
                                DescribeType(producer, a_it->second.type_id).c_str(),
                                DescribeType(consumer, b_it->second.type_id).c_str());
            }
            if (a_it->second.is_patch != b_it->second.is_patch) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT,
                                HandleToUint64(producer->vk_shader_module), kVUID_Core_Shader_InterfaceTypeMismatch,
                                "Decoration mismatch on location %u.%u: is per-%s in %s stage but per-%s in %s stage",
                                a_first.first, a_first.second, a_it->second.is_patch ? "patch" : "vertex", producer_stage->name,
                                b_it->second.is_patch ? "patch" : "vertex", consumer_stage->name);
            }
            if (a_it->second.is_relaxed_precision != b_it->second.is_relaxed_precision) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT,
                                HandleToUint64(producer->vk_shader_module), kVUID_Core_Shader_InterfaceTypeMismatch,
                                "Decoration mismatch on location %u.%u: %s and %s stages differ in precision", a_first.first,
                                a_first.second, producer_stage->name, consumer_stage->name);
            }
            a_it++;
            b_it++;
        }
    }

    return skip;
}

static inline uint32_t DetermineFinalGeomStage(PIPELINE_STATE *pipeline, VkGraphicsPipelineCreateInfo *pCreateInfo) {
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
bool ValidateAndCapturePipelineShaderState(layer_data *dev_data, PIPELINE_STATE *pipeline) {
    auto pCreateInfo = pipeline->graphicsPipelineCI.ptr();
    int vertex_stage = GetShaderStageId(VK_SHADER_STAGE_VERTEX_BIT);
    int fragment_stage = GetShaderStageId(VK_SHADER_STAGE_FRAGMENT_BIT);
    auto report_data = GetReportData(dev_data);

    shader_module const *shaders[32];
    memset(shaders, 0, sizeof(shaders));
    spirv_inst_iter entrypoints[32];
    memset(entrypoints, 0, sizeof(entrypoints));
    bool skip = false;

    uint32_t pointlist_stage_mask = DetermineFinalGeomStage(pipeline, pCreateInfo);

    for (uint32_t i = 0; i < pCreateInfo->stageCount; i++) {
        auto pStage = &pCreateInfo->pStages[i];
        auto stage_id = GetShaderStageId(pStage->stage);
        skip |= ValidatePipelineShaderStage(dev_data, pStage, pipeline, &shaders[stage_id], &entrypoints[stage_id],
                                            (pointlist_stage_mask == pStage->stage));
    }

    // if the shader stages are no good individually, cross-stage validation is pointless.
    if (skip) return true;

    auto vi = pCreateInfo->pVertexInputState;

    if (vi) {
        skip |= ValidateViConsistency(report_data, vi);
    }

    if (shaders[vertex_stage] && shaders[vertex_stage]->has_valid_spirv) {
        skip |= ValidateViAgainstVsInputs(report_data, vi, shaders[vertex_stage], entrypoints[vertex_stage]);
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
                skip |= ValidateInterfaceBetweenStages(report_data, shaders[producer], entrypoints[producer],
                                                       &shader_stage_attribs[producer], shaders[consumer], entrypoints[consumer],
                                                       &shader_stage_attribs[consumer]);
            }

            producer = consumer;
        }
    }

    if (shaders[fragment_stage] && shaders[fragment_stage]->has_valid_spirv) {
        skip |= ValidateFsOutputsAgainstRenderPass(report_data, shaders[fragment_stage], entrypoints[fragment_stage], pipeline,
                                                   pCreateInfo->subpass);
    }

    return skip;
}

bool ValidateComputePipeline(layer_data *dev_data, PIPELINE_STATE *pipeline) {
    auto pCreateInfo = pipeline->computePipelineCI.ptr();

    shader_module const *module;
    spirv_inst_iter entrypoint;

    return ValidatePipelineShaderStage(dev_data, &pCreateInfo->stage, pipeline, &module, &entrypoint, false);
}

bool ValidateRaytracingPipelineNVX(layer_data *dev_data, PIPELINE_STATE *pipeline) {
    auto pCreateInfo = pipeline->raytracingPipelineCI.ptr();

    shader_module const *module;
    spirv_inst_iter entrypoint;

    return ValidatePipelineShaderStage(dev_data, pCreateInfo->pStages, pipeline, &module, &entrypoint, false);
}

uint32_t ValidationCache::MakeShaderHash(VkShaderModuleCreateInfo const *smci) { return XXH32(smci->pCode, smci->codeSize, 0); }

static ValidationCache *GetValidationCacheInfo(VkShaderModuleCreateInfo const *pCreateInfo) {
    while ((pCreateInfo = (VkShaderModuleCreateInfo const *)pCreateInfo->pNext) != nullptr) {
        if (pCreateInfo->sType == VK_STRUCTURE_TYPE_SHADER_MODULE_VALIDATION_CACHE_CREATE_INFO_EXT)
            return (ValidationCache *)((VkShaderModuleValidationCacheCreateInfoEXT const *)pCreateInfo)->validationCache;
    }

    return nullptr;
}

bool PreCallValidateCreateShaderModule(layer_data *dev_data, VkShaderModuleCreateInfo const *pCreateInfo, bool *spirv_valid) {
    bool skip = false;
    spv_result_t spv_valid = SPV_SUCCESS;
    auto report_data = GetReportData(dev_data);

    if (GetDisables(dev_data)->shader_validation) {
        return false;
    }

    auto have_glsl_shader = GetDeviceExtensions(dev_data)->vk_nv_glsl_shader;

    if (!have_glsl_shader && (pCreateInfo->codeSize % 4)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkShaderModuleCreateInfo-pCode-01376",
                        "SPIR-V module not valid: Codesize must be a multiple of 4 but is " PRINTF_SIZE_T_SPECIFIER ".",
                        pCreateInfo->codeSize);
    } else {
        auto cache = GetValidationCacheInfo(pCreateInfo);
        uint32_t hash = 0;
        if (cache) {
            hash = ValidationCache::MakeShaderHash(pCreateInfo);
            if (cache->Contains(hash)) return false;
        }

        // Use SPIRV-Tools validator to try and catch any issues with the module itself
        spv_target_env spirv_environment = SPV_ENV_VULKAN_1_0;
        if (GetApiVersion(dev_data) >= VK_API_VERSION_1_1) {
            spirv_environment = SPV_ENV_VULKAN_1_1;
        }
        spv_context ctx = spvContextCreate(spirv_environment);
        spv_const_binary_t binary{pCreateInfo->pCode, pCreateInfo->codeSize / sizeof(uint32_t)};
        spv_diagnostic diag = nullptr;
        spv_validator_options options = spvValidatorOptionsCreate();
        if (GetDeviceExtensions(dev_data)->vk_khr_relaxed_block_layout) {
            spvValidatorOptionsSetRelaxBlockLayout(options, true);
        }
        spv_valid = spvValidateWithOptions(ctx, options, &binary, &diag);
        if (spv_valid != SPV_SUCCESS) {
            if (!have_glsl_shader || (pCreateInfo->pCode[0] == spv::MagicNumber)) {
                skip |=
                    log_msg(report_data, spv_valid == SPV_WARNING ? VK_DEBUG_REPORT_WARNING_BIT_EXT : VK_DEBUG_REPORT_ERROR_BIT_EXT,
                            VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, kVUID_Core_Shader_InconsistentSpirv,
                            "SPIR-V module not valid: %s", diag && diag->error ? diag->error : "(no error text)");
            }
        } else {
            if (cache) {
                cache->Insert(hash);
            }
        }

        spvValidatorOptionsDestroy(options);
        spvDiagnosticDestroy(diag);
        spvContextDestroy(ctx);
    }

    *spirv_valid = (spv_valid == SPV_SUCCESS);
    return skip;
}
