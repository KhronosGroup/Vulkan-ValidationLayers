/* Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (C) 2015-2021 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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
 * Author: Tobias Hector <tobias.hector@amd.com>
 */

#include "shader_validation.h"

#include <cassert>
#include <cinttypes>
#include <cmath>
#include <sstream>
#include <string>
#include <vector>

#include <spirv/unified1/spirv.hpp>
#include "vk_enum_string_helper.h"
#include "vk_layer_data.h"
#include "vk_layer_utils.h"
#include "chassis.h"
#include "core_validation.h"

#include "xxhash.h"

static shader_stage_attributes shader_stage_attribs[] = {
    {"vertex shader", false, false, VK_SHADER_STAGE_VERTEX_BIT},
    {"tessellation control shader", true, true, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT},
    {"tessellation evaluation shader", true, false, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT},
    {"geometry shader", true, false, VK_SHADER_STAGE_GEOMETRY_BIT},
    {"fragment shader", false, false, VK_SHADER_STAGE_FRAGMENT_BIT},
};

static const spirv_inst_iter GetBaseTypeIter(SHADER_MODULE_STATE const *src, uint32_t type) {
    const auto &insn = src->get_def(type);
    const uint32_t base_insn_id = src->GetBaseType(insn);
    return src->get_def(base_insn_id);
}

static bool BaseTypesMatch(SHADER_MODULE_STATE const *a, SHADER_MODULE_STATE const *b, const spirv_inst_iter &a_base_insn,
                           const spirv_inst_iter &b_base_insn) {
    const uint32_t a_opcode = a_base_insn.opcode();
    const uint32_t b_opcode = b_base_insn.opcode();
    if (a_opcode == b_opcode) {
        if (a_opcode == spv::OpTypeInt) {
            // Match width and signedness
            return a_base_insn.word(2) == b_base_insn.word(2) && a_base_insn.word(3) == b_base_insn.word(3);
        } else if (a_opcode == spv::OpTypeFloat) {
            // Match width
            return a_base_insn.word(2) == b_base_insn.word(2);
        } else if (a_opcode == spv::OpTypeStruct) {
            // Match on all element types
            if (a_base_insn.len() != b_base_insn.len()) {
                return false;  // Structs cannot match if member counts differ
            }

            for (uint32_t i = 2; i < a_base_insn.len(); i++) {
                const auto &c_base_insn = GetBaseTypeIter(a, a_base_insn.word(i));
                const auto &d_base_insn = GetBaseTypeIter(b, b_base_insn.word(i));
                if (!BaseTypesMatch(a, b, c_base_insn, d_base_insn)) {
                    return false;
                }
            }

            return true;
        }
    }
    return false;
}

static bool TypesMatch(SHADER_MODULE_STATE const *a, SHADER_MODULE_STATE const *b, uint32_t a_type, uint32_t b_type) {
    const auto &a_base_insn = GetBaseTypeIter(a, a_type);
    const auto &b_base_insn = GetBaseTypeIter(b, b_type);

    return BaseTypesMatch(a, b, a_base_insn, b_base_insn);
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
    // Formats such as VK_FORMAT_D16_UNORM_S8_UINT are both
    if (FormatIsDepthAndStencil(fmt)) return FORMAT_TYPE_FLOAT | FORMAT_TYPE_UINT;
    if (fmt == VK_FORMAT_UNDEFINED) return 0;
    // everything else -- UNORM/SNORM/FLOAT/USCALED/SSCALED is all float in the shader.
    return FORMAT_TYPE_FLOAT;
}

static uint32_t GetShaderStageId(VkShaderStageFlagBits stage) {
    uint32_t bit_pos = uint32_t(u_ffs(stage));
    return bit_pos - 1;
}

bool CoreChecks::ValidateViConsistency(VkPipelineVertexInputStateCreateInfo const *vi) const {
    // Walk the binding descriptions, which describe the step rate and stride of each vertex buffer.  Each binding should
    // be specified only once.
    layer_data::unordered_map<uint32_t, VkVertexInputBindingDescription const *> bindings;
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

    const auto inputs = vs->CollectInterfaceByLocation(entrypoint, spv::StorageClassInput, false);

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
            skip |= LogPerformanceWarning(vs->vk_shader_module(), kVUID_Core_Shader_OutputNotConsumed,
                                          "Vertex attribute at location %" PRIu32 " not consumed by vertex shader", location);
        } else if (!attrib && input) {
            skip |= LogError(vs->vk_shader_module(), kVUID_Core_Shader_InputNotProduced,
                             "Vertex shader consumes input at location %" PRIu32 " but not provided", location);
        } else if (attrib && input) {
            const auto attrib_type = GetFormatType(attrib->format);
            const auto input_type = vs->GetFundamentalType(input->type_id);

            // Type checking
            if (!(attrib_type & input_type)) {
                skip |= LogError(vs->vk_shader_module(), kVUID_Core_Shader_InterfaceTypeMismatch,
                                 "Attribute type of `%s` at location %" PRIu32 " does not match vertex shader input type of `%s`",
                                 string_VkFormat(attrib->format), location, vs->DescribeType(input->type_id).c_str());
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
        const VkAttachmentReference2 *reference = nullptr;
        const VkAttachmentDescription2 *attachment = nullptr;
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

    const auto outputs = fs->CollectInterfaceByLocation(entrypoint, spv::StorageClassOutput, false);
    for (const auto &output_it : outputs) {
        auto const location = output_it.first.first;
        location_map[location].output = &output_it.second;
    }

    const bool alpha_to_coverage_enabled = pipeline->create_info.graphics.pMultisampleState != NULL &&
                                           pipeline->create_info.graphics.pMultisampleState->alphaToCoverageEnable == VK_TRUE;

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
                skip |= LogWarning(fs->vk_shader_module(), kVUID_Core_Shader_InputNotProduced,
                                   "Attachment %" PRIu32
                                   " not written by fragment shader; undefined values will be written to attachment",
                                   location);
            }
        } else if (!attachment && output) {
            if (!(alpha_to_coverage_enabled && location == 0)) {
                skip |= LogWarning(fs->vk_shader_module(), kVUID_Core_Shader_OutputNotConsumed,
                                   "fragment shader writes to output location %" PRIu32 " with no matching attachment", location);
            }
        } else if (attachment && output) {
            const auto attachment_type = GetFormatType(attachment->format);
            const auto output_type = fs->GetFundamentalType(output->type_id);

            // Type checking
            if (!(output_type & attachment_type)) {
                skip |=
                    LogWarning(fs->vk_shader_module(), kVUID_Core_Shader_InterfaceTypeMismatch,
                               "Attachment %" PRIu32
                               " of type `%s` does not match fragment shader output type of `%s`; resulting values are undefined",
                               location, string_VkFormat(attachment->format), fs->DescribeType(output->type_id).c_str());
            }
        } else {            // !attachment && !output
            assert(false);  // at least one exists in the map
        }
    }

    const auto output_zero = location_map.count(0) ? location_map[0].output : nullptr;
    bool location_zero_has_alpha = output_zero && fs->get_def(output_zero->type_id) != fs->end() &&
                                   fs->GetComponentsConsumedByType(output_zero->type_id, false) == 4;
    if (alpha_to_coverage_enabled && !location_zero_has_alpha) {
        skip |= LogError(fs->vk_shader_module(), kVUID_Core_Shader_NoAlphaAtLocation0WithAlphaToCoverage,
                         "fragment shader doesn't declare alpha output at location 0 even though alpha to coverage is enabled.");
    }

    return skip;
}

PushConstantByteState CoreChecks::ValidatePushConstantSetUpdate(const std::vector<uint8_t> &push_constant_data_update,
                                                                const shader_struct_member &push_constant_used_in_shader,
                                                                uint32_t &out_issue_index) const {
    const auto *used_bytes = push_constant_used_in_shader.GetUsedbytes();
    const auto used_bytes_size = used_bytes->size();
    if (used_bytes_size == 0) return PC_Byte_Updated;

    const auto push_constant_data_update_size = push_constant_data_update.size();
    const auto *data = push_constant_data_update.data();
    if ((*data == PC_Byte_Updated) && std::memcmp(data, data + 1, push_constant_data_update_size - 1) == 0) {
        if (used_bytes_size <= push_constant_data_update_size) {
            return PC_Byte_Updated;
        }
        const auto used_bytes_size1 = used_bytes_size - push_constant_data_update_size;

        const auto *used_bytes_data1 = used_bytes->data() + push_constant_data_update_size;
        if ((*used_bytes_data1 == 0) && std::memcmp(used_bytes_data1, used_bytes_data1 + 1, used_bytes_size1 - 1) == 0) {
            return PC_Byte_Updated;
        }
    }

    uint32_t i = 0;
    for (const auto used : *used_bytes) {
        if (used) {
            if (i >= push_constant_data_update.size() || push_constant_data_update[i] == PC_Byte_Not_Set) {
                out_issue_index = i;
                return PC_Byte_Not_Set;
            } else if (push_constant_data_update[i] == PC_Byte_Not_Updated) {
                out_issue_index = i;
                return PC_Byte_Not_Updated;
            }
        }
        ++i;
    }
    return PC_Byte_Updated;
}

bool CoreChecks::ValidatePushConstantUsage(const PIPELINE_STATE &pipeline, SHADER_MODULE_STATE const *src,
                                           VkPipelineShaderStageCreateInfo const *pStage, const std::string &vuid) const {
    bool skip = false;
    // Temp workaround to prevent false positive errors
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/2450
    if (src->multiple_entry_points) {
        return skip;
    }

    // Validate directly off the offsets. this isn't quite correct for arrays and matrices, but is a good first step.
    const auto *entrypoint = src->FindEntrypointStruct(pStage->pName, pStage->stage);
    if (!entrypoint || !entrypoint->push_constant_used_in_shader.IsUsed()) {
        return skip;
    }
    std::vector<VkPushConstantRange> const *push_constant_ranges = pipeline.pipeline_layout->push_constant_ranges.get();

    bool found_stage = false;
    for (auto const &range : *push_constant_ranges) {
        if (range.stageFlags & pStage->stage) {
            found_stage = true;
            std::string location_desc;
            std::vector<uint8_t> push_constant_bytes_set;
            if (range.offset > 0) {
                push_constant_bytes_set.resize(range.offset, PC_Byte_Not_Set);
            }
            push_constant_bytes_set.resize(range.offset + range.size, PC_Byte_Updated);
            uint32_t issue_index = 0;
            const auto ret =
                ValidatePushConstantSetUpdate(push_constant_bytes_set, entrypoint->push_constant_used_in_shader, issue_index);

            if (ret == PC_Byte_Not_Set) {
                const auto loc_descr = entrypoint->push_constant_used_in_shader.GetLocationDesc(issue_index);
                LogObjectList objlist(src->vk_shader_module());
                objlist.add(pipeline.pipeline_layout->layout());
                skip |= LogError(objlist, vuid, "Push constant buffer:%s in %s is out of range in %s.", loc_descr.c_str(),
                                 string_VkShaderStageFlags(pStage->stage).c_str(),
                                 report_data->FormatHandle(pipeline.pipeline_layout->layout()).c_str());
                break;
            }
        }
    }

    if (!found_stage) {
        LogObjectList objlist(src->vk_shader_module());
        objlist.add(pipeline.pipeline_layout->layout());
        skip |= LogError(objlist, vuid, "Push constant is used in %s of %s. But %s doesn't set %s.",
                         string_VkShaderStageFlags(pStage->stage).c_str(), report_data->FormatHandle(src->vk_shader_module()).c_str(),
                         report_data->FormatHandle(pipeline.pipeline_layout->layout()).c_str(),
                         string_VkShaderStageFlags(pStage->stage).c_str());
    }
    return skip;
}

bool CoreChecks::ValidateBuiltinLimits(SHADER_MODULE_STATE const *src, spirv_inst_iter entrypoint) const {
    bool skip = false;

    // Currently all builtin tested are only found in fragment shaders
    if (entrypoint.word(1) != spv::ExecutionModelFragment) {
        return skip;
    }

    // Find all builtin from just the interface variables
    for (uint32_t id : FindEntrypointInterfaces(entrypoint)) {
        auto insn = src->get_def(id);
        assert(insn.opcode() == spv::OpVariable);
        const decoration_set decorations = src->get_decorations(insn.word(2));

        // Currently don't need to search in structs
        if (((decorations.flags & decoration_set::builtin_bit) != 0) && (decorations.builtin == spv::BuiltInSampleMask)) {
            auto type_pointer = src->get_def(insn.word(1));
            assert(type_pointer.opcode() == spv::OpTypePointer);

            auto type = src->get_def(type_pointer.word(3));
            if (type.opcode() == spv::OpTypeArray) {
                uint32_t length = static_cast<uint32_t>(src->GetConstantValueById(type.word(3)));
                // Handles both the input and output sampleMask
                if (length > phys_dev_props.limits.maxSampleMaskWords) {
                    skip |= LogError(device, "VUID-VkPipelineShaderStageCreateInfo-maxSampleMaskWords-00711",
                                     "vkCreateGraphicsPipelines(): The BuiltIns SampleMask array sizes is %u which exceeds "
                                     "maxSampleMaskWords of %u in %s.",
                                     length, phys_dev_props.limits.maxSampleMaskWords,
                                     report_data->FormatHandle(src->vk_shader_module()).c_str());
                }
                break;
            }
        }
    }

    return skip;
}

// Validate that data for each specialization entry is fully contained within the buffer.
bool CoreChecks::ValidateSpecializations(VkPipelineShaderStageCreateInfo const *info) const {
    bool skip = false;

    VkSpecializationInfo const *spec = info->pSpecializationInfo;

    if (spec) {
        for (auto i = 0u; i < spec->mapEntryCount; i++) {
            if (spec->pMapEntries[i].offset >= spec->dataSize) {
                skip |= LogError(device, "VUID-VkSpecializationInfo-offset-00773",
                                 "Specialization entry %u (for constant id %u) references memory outside provided specialization "
                                 "data (bytes %u.." PRINTF_SIZE_T_SPECIFIER "; " PRINTF_SIZE_T_SPECIFIER " bytes provided).",
                                 i, spec->pMapEntries[i].constantID, spec->pMapEntries[i].offset,
                                 spec->pMapEntries[i].offset + spec->dataSize - 1, spec->dataSize);

                continue;
            }
            if (spec->pMapEntries[i].offset + spec->pMapEntries[i].size > spec->dataSize) {
                skip |= LogError(device, "VUID-VkSpecializationInfo-pMapEntries-00774",
                                 "Specialization entry %u (for constant id %u) references memory outside provided specialization "
                                 "data (bytes %u.." PRINTF_SIZE_T_SPECIFIER "; " PRINTF_SIZE_T_SPECIFIER " bytes provided).",
                                 i, spec->pMapEntries[i].constantID, spec->pMapEntries[i].offset,
                                 spec->pMapEntries[i].offset + spec->pMapEntries[i].size - 1, spec->dataSize);
            }
            for (uint32_t j = i + 1; j < spec->mapEntryCount; ++j) {
                if (spec->pMapEntries[i].constantID == spec->pMapEntries[j].constantID) {
                    skip |= LogError(device, "VUID-VkSpecializationInfo-constantID-04911",
                                     "Specialization entry %" PRIu32 " and %" PRIu32 " have the same constantID (%" PRIu32 ").", i,
                                     j, spec->pMapEntries[i].constantID);
                }
            }
        }
    }

    return skip;
}

// TODO (jbolz): Can this return a const reference?
static std::set<uint32_t> TypeToDescriptorTypeSet(SHADER_MODULE_STATE const *module, uint32_t type_id, unsigned &descriptor_count,
                                                  bool is_khr) {
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
            descriptor_count *= module->GetConstantValueById(type.word(3));
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
            for (auto insn : module->decoration_inst) {
                if (insn.word(1) == type.word(1)) {
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
            is_khr ? ret.insert(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR)
                   : ret.insert(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV);
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

bool CoreChecks::RequirePropertyFlag(VkBool32 check, char const *flag, char const *structure, const char *vuid) const {
    if (!check) {
        if (LogError(device, vuid, "Shader requires flag %s set in %s but it is not set on the device", flag, structure)) {
            return true;
        }
    }

    return false;
}

bool CoreChecks::RequireFeature(VkBool32 feature, char const *feature_name, const char *vuid) const {
    if (!feature) {
        if (LogError(device, vuid, "Shader requires %s but is not enabled on the device", feature_name)) {
            return true;
        }
    }

    return false;
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
                skip |= RequireFeature(enabled_features.core.fragmentStoresAndAtomics, "fragmentStoresAndAtomics",
                                       "VUID-RuntimeSpirv-NonWritable-06340");
                break;
            default:
                skip |= RequireFeature(enabled_features.core.vertexPipelineStoresAndAtomics, "vertexPipelineStoresAndAtomics",
                                       "VUID-RuntimeSpirv-NonWritable-06341");
                break;
        }
    }

    return skip;
}

bool CoreChecks::ValidateShaderStageGroupNonUniform(SHADER_MODULE_STATE const *module, VkShaderStageFlagBits stage,
                                                    spirv_inst_iter &insn) const {
    bool skip = false;

    // Check anything using a group operation (which currently is only OpGroupNonUnifrom* operations)
    if (GroupOperation(insn.opcode()) == true) {
        // Check the quad operations.
        if ((insn.opcode() == spv::OpGroupNonUniformQuadBroadcast) || (insn.opcode() == spv::OpGroupNonUniformQuadSwap)) {
            if ((stage != VK_SHADER_STAGE_FRAGMENT_BIT) && (stage != VK_SHADER_STAGE_COMPUTE_BIT)) {
                skip |=
                    RequireFeature(phys_dev_props_core11.subgroupQuadOperationsInAllStages,
                                   "VkPhysicalDeviceSubgroupProperties::quadOperationsInAllStages", "VUID-RuntimeSpirv-None-06342");
            }
        }

        uint32_t scope_type = spv::ScopeMax;
        if (insn.opcode() == spv::OpGroupNonUniformPartitionNV) {
            // OpGroupNonUniformPartitionNV always assumed subgroup as missing operand
            scope_type = spv::ScopeSubgroup;
        } else {
            // "All <id> used for Scope <id> must be of an OpConstant"
            auto scope_id = module->get_def(insn.word(3));
            scope_type = scope_id.word(3);
        }

        if (scope_type == spv::ScopeSubgroup) {
            // "Group operations with subgroup scope" must have stage support
            const VkSubgroupFeatureFlags supported_stages = phys_dev_props_core11.subgroupSupportedStages;
            skip |= RequirePropertyFlag(supported_stages & stage, string_VkShaderStageFlagBits(stage),
                                        "VkPhysicalDeviceSubgroupProperties::supportedStages", "VUID-RuntimeSpirv-None-06343");
        }

        if (!enabled_features.core12.shaderSubgroupExtendedTypes) {
            auto type = module->get_def(insn.word(1));

            if (type.opcode() == spv::OpTypeVector) {
                // Get the element type
                type = module->get_def(type.word(2));
            }

            if (type.opcode() != spv::OpTypeBool) {
                // Both OpTypeInt and OpTypeFloat the width is in the 2nd word.
                const uint32_t width = type.word(2);

                if ((type.opcode() == spv::OpTypeFloat && width == 16) ||
                    (type.opcode() == spv::OpTypeInt && (width == 8 || width == 16 || width == 64))) {
                    skip |= RequireFeature(enabled_features.core12.shaderSubgroupExtendedTypes,
                                           "VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures::shaderSubgroupExtendedTypes",
                                           "VUID-RuntimeSpirv-None-06275");
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateWorkgroupSize(SHADER_MODULE_STATE const *src, VkPipelineShaderStageCreateInfo const *pStage,
                                       const std::unordered_map<uint32_t, std::vector<uint32_t>>& id_value_map) const {
    bool skip = false;

    std::array<uint32_t, 3> work_group_size = src->GetWorkgroupSize(pStage, id_value_map);

    for (uint32_t i = 0; i < 3; ++i) {
        if (work_group_size[i] > phys_dev_props.limits.maxComputeWorkGroupSize[i]) {
            const char member = 'x' + static_cast<int8_t>(i);
            skip |= LogError(device, kVUID_Core_Shader_MaxComputeWorkGroupSize,
                             "Specialization constant is being used to specialize WorkGroupSize.%c, but value (%" PRIu32
                             ") is greater than VkPhysicalDeviceLimits::maxComputeWorkGroupSize[%" PRIu32 "] = %" PRIu32 ".",
                             member, work_group_size[i], i, phys_dev_props.limits.maxComputeWorkGroupSize[i]);
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

    std::set<uint32_t> patch_i_ds;
    struct Variable {
        uint32_t baseTypePtrID;
        uint32_t ID;
        uint32_t storageClass;
    };
    std::vector<Variable> variables;

    uint32_t num_vertices = 0;
    bool is_iso_lines = false;
    bool is_point_mode = false;

    auto entrypoint_variables = FindEntrypointInterfaces(entrypoint);

    for (auto insn : *src) {
        switch (insn.opcode()) {
            // Find all Patch decorations
            case spv::OpDecorate:
                switch (insn.word(2)) {
                    case spv::DecorationPatch: {
                        patch_i_ds.insert(insn.word(1));
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
                    find(entrypoint_variables.begin(), entrypoint_variables.end(), insn.word(2)) != entrypoint_variables.end()) {
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
                            num_vertices = insn.word(3);
                            break;
                        case spv::ExecutionModeIsolines:
                            is_iso_lines = true;
                            break;
                        case spv::ExecutionModePointMode:
                            is_point_mode = true;
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

    uint32_t num_comp_in = 0, num_comp_out = 0;
    int max_comp_in = 0, max_comp_out = 0;

    auto inputs = src->CollectInterfaceByLocation(entrypoint, spv::StorageClassInput, strip_input_array_level);
    auto outputs = src->CollectInterfaceByLocation(entrypoint, spv::StorageClassOutput, strip_output_array_level);

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

        int num_components = src->GetComponentsConsumedByType(iv.type_id, strip_input_array_level);
        max_comp_in = std::max(max_comp_in, location * 4 + component + num_components);
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

        int num_components = src->GetComponentsConsumedByType(iv.type_id, strip_output_array_level);
        max_comp_out = std::max(max_comp_out, location * 4 + component + num_components);
    }

    // XXX TODO: Would be nice to rewrite this to use CollectInterfaceByLocation (or something similar),
    // but that doesn't include builtins.
    // When rewritten, using the CreatePipelineExceedVertexMaxComponentsWithBuiltins test it would be nice to also let the user know
    // how many components were from builtins as it might not be obvious
    for (auto &var : variables) {
        // Check if the variable is a patch. Patches can also be members of blocks,
        // but if they are then the top-level arrayness has already been stripped
        // by the time GetComponentsConsumedByType gets to it.
        bool is_patch = patch_i_ds.find(var.ID) != patch_i_ds.end();

        if (var.storageClass == spv::StorageClassInput) {
            num_comp_in += src->GetComponentsConsumedByType(var.baseTypePtrID, strip_input_array_level && !is_patch);
        } else {  // var.storageClass == spv::StorageClassOutput
            num_comp_out += src->GetComponentsConsumedByType(var.baseTypePtrID, strip_output_array_level && !is_patch);
        }
    }

    switch (pStage->stage) {
        case VK_SHADER_STAGE_VERTEX_BIT:
            if (num_comp_out > limits.maxVertexOutputComponents) {
                skip |= LogError(pipeline->pipeline(), "VUID-RuntimeSpirv-Location-06272",
                                 "Invalid Pipeline CreateInfo State: Vertex shader exceeds "
                                 "VkPhysicalDeviceLimits::maxVertexOutputComponents of %u "
                                 "components by %u components",
                                 limits.maxVertexOutputComponents, num_comp_out - limits.maxVertexOutputComponents);
            }
            if (max_comp_out > static_cast<int>(limits.maxVertexOutputComponents)) {
                skip |= LogError(pipeline->pipeline(), "VUID-RuntimeSpirv-Location-06272",
                                 "Invalid Pipeline CreateInfo State: Vertex shader output variable uses location that "
                                 "exceeds component limit VkPhysicalDeviceLimits::maxVertexOutputComponents (%u)",
                                 limits.maxVertexOutputComponents);
            }
            break;

        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
            if (num_comp_in > limits.maxTessellationControlPerVertexInputComponents) {
                skip |= LogError(pipeline->pipeline(), "VUID-RuntimeSpirv-Location-06272",
                                 "Invalid Pipeline CreateInfo State: Tessellation control shader exceeds "
                                 "VkPhysicalDeviceLimits::maxTessellationControlPerVertexInputComponents of %u "
                                 "components by %u components",
                                 limits.maxTessellationControlPerVertexInputComponents,
                                 num_comp_in - limits.maxTessellationControlPerVertexInputComponents);
            }
            if (max_comp_in > static_cast<int>(limits.maxTessellationControlPerVertexInputComponents)) {
                skip |=
                    LogError(pipeline->pipeline(), "VUID-RuntimeSpirv-Location-06272",
                             "Invalid Pipeline CreateInfo State: Tessellation control shader input variable uses location that "
                             "exceeds component limit VkPhysicalDeviceLimits::maxTessellationControlPerVertexInputComponents (%u)",
                             limits.maxTessellationControlPerVertexInputComponents);
            }
            if (num_comp_out > limits.maxTessellationControlPerVertexOutputComponents) {
                skip |= LogError(pipeline->pipeline(), "VUID-RuntimeSpirv-Location-06272",
                                 "Invalid Pipeline CreateInfo State: Tessellation control shader exceeds "
                                 "VkPhysicalDeviceLimits::maxTessellationControlPerVertexOutputComponents of %u "
                                 "components by %u components",
                                 limits.maxTessellationControlPerVertexOutputComponents,
                                 num_comp_out - limits.maxTessellationControlPerVertexOutputComponents);
            }
            if (max_comp_out > static_cast<int>(limits.maxTessellationControlPerVertexOutputComponents)) {
                skip |=
                    LogError(pipeline->pipeline(), "VUID-RuntimeSpirv-Location-06272",
                             "Invalid Pipeline CreateInfo State: Tessellation control shader output variable uses location that "
                             "exceeds component limit VkPhysicalDeviceLimits::maxTessellationControlPerVertexOutputComponents (%u)",
                             limits.maxTessellationControlPerVertexOutputComponents);
            }
            break;

        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            if (num_comp_in > limits.maxTessellationEvaluationInputComponents) {
                skip |= LogError(pipeline->pipeline(), "VUID-RuntimeSpirv-Location-06272",
                                 "Invalid Pipeline CreateInfo State: Tessellation evaluation shader exceeds "
                                 "VkPhysicalDeviceLimits::maxTessellationEvaluationInputComponents of %u "
                                 "components by %u components",
                                 limits.maxTessellationEvaluationInputComponents,
                                 num_comp_in - limits.maxTessellationEvaluationInputComponents);
            }
            if (max_comp_in > static_cast<int>(limits.maxTessellationEvaluationInputComponents)) {
                skip |=
                    LogError(pipeline->pipeline(), "VUID-RuntimeSpirv-Location-06272",
                             "Invalid Pipeline CreateInfo State: Tessellation evaluation shader input variable uses location that "
                             "exceeds component limit VkPhysicalDeviceLimits::maxTessellationEvaluationInputComponents (%u)",
                             limits.maxTessellationEvaluationInputComponents);
            }
            if (num_comp_out > limits.maxTessellationEvaluationOutputComponents) {
                skip |= LogError(pipeline->pipeline(), "VUID-RuntimeSpirv-Location-06272",
                                 "Invalid Pipeline CreateInfo State: Tessellation evaluation shader exceeds "
                                 "VkPhysicalDeviceLimits::maxTessellationEvaluationOutputComponents of %u "
                                 "components by %u components",
                                 limits.maxTessellationEvaluationOutputComponents,
                                 num_comp_out - limits.maxTessellationEvaluationOutputComponents);
            }
            if (max_comp_out > static_cast<int>(limits.maxTessellationEvaluationOutputComponents)) {
                skip |=
                    LogError(pipeline->pipeline(), "VUID-RuntimeSpirv-Location-06272",
                             "Invalid Pipeline CreateInfo State: Tessellation evaluation shader output variable uses location that "
                             "exceeds component limit VkPhysicalDeviceLimits::maxTessellationEvaluationOutputComponents (%u)",
                             limits.maxTessellationEvaluationOutputComponents);
            }
            // Portability validation
            if (IsExtEnabled(device_extensions.vk_khr_portability_subset)) {
                if (is_iso_lines && (VK_FALSE == enabled_features.portability_subset_features.tessellationIsolines)) {
                    skip |= LogError(pipeline->pipeline(), "VUID-RuntimeSpirv-tessellationShader-06326",
                                     "Invalid Pipeline CreateInfo state (portability error): Tessellation evaluation shader"
                                     " is using abstract patch type IsoLines, but this is not supported on this platform");
                }
                if (is_point_mode && (VK_FALSE == enabled_features.portability_subset_features.tessellationPointMode)) {
                    skip |= LogError(pipeline->pipeline(), "VUID-RuntimeSpirv-tessellationShader-06327",
                                     "Invalid Pipeline CreateInfo state (portability error): Tessellation evaluation shader"
                                     " is using abstract patch type PointMode, but this is not supported on this platform");
                }
            }
            break;

        case VK_SHADER_STAGE_GEOMETRY_BIT:
            if (num_comp_in > limits.maxGeometryInputComponents) {
                skip |= LogError(pipeline->pipeline(), "VUID-RuntimeSpirv-Location-06272",
                                 "Invalid Pipeline CreateInfo State: Geometry shader exceeds "
                                 "VkPhysicalDeviceLimits::maxGeometryInputComponents of %u "
                                 "components by %u components",
                                 limits.maxGeometryInputComponents, num_comp_in - limits.maxGeometryInputComponents);
            }
            if (max_comp_in > static_cast<int>(limits.maxGeometryInputComponents)) {
                skip |= LogError(pipeline->pipeline(), "VUID-RuntimeSpirv-Location-06272",
                                 "Invalid Pipeline CreateInfo State: Geometry shader input variable uses location that "
                                 "exceeds component limit VkPhysicalDeviceLimits::maxGeometryInputComponents (%u)",
                                 limits.maxGeometryInputComponents);
            }
            if (num_comp_out > limits.maxGeometryOutputComponents) {
                skip |= LogError(pipeline->pipeline(), "VUID-RuntimeSpirv-Location-06272",
                                 "Invalid Pipeline CreateInfo State: Geometry shader exceeds "
                                 "VkPhysicalDeviceLimits::maxGeometryOutputComponents of %u "
                                 "components by %u components",
                                 limits.maxGeometryOutputComponents, num_comp_out - limits.maxGeometryOutputComponents);
            }
            if (max_comp_out > static_cast<int>(limits.maxGeometryOutputComponents)) {
                skip |= LogError(pipeline->pipeline(), "VUID-RuntimeSpirv-Location-06272",
                                 "Invalid Pipeline CreateInfo State: Geometry shader output variable uses location that "
                                 "exceeds component limit VkPhysicalDeviceLimits::maxGeometryOutputComponents (%u)",
                                 limits.maxGeometryOutputComponents);
            }
            if (num_comp_out * num_vertices > limits.maxGeometryTotalOutputComponents) {
                skip |= LogError(pipeline->pipeline(), "VUID-RuntimeSpirv-Location-06272",
                                 "Invalid Pipeline CreateInfo State: Geometry shader exceeds "
                                 "VkPhysicalDeviceLimits::maxGeometryTotalOutputComponents of %u "
                                 "components by %u components",
                                 limits.maxGeometryTotalOutputComponents,
                                 num_comp_out * num_vertices - limits.maxGeometryTotalOutputComponents);
            }
            break;

        case VK_SHADER_STAGE_FRAGMENT_BIT:
            if (num_comp_in > limits.maxFragmentInputComponents) {
                skip |= LogError(pipeline->pipeline(), "VUID-RuntimeSpirv-Location-06272",
                                 "Invalid Pipeline CreateInfo State: Fragment shader exceeds "
                                 "VkPhysicalDeviceLimits::maxFragmentInputComponents of %u "
                                 "components by %u components",
                                 limits.maxFragmentInputComponents, num_comp_in - limits.maxFragmentInputComponents);
            }
            if (max_comp_in > static_cast<int>(limits.maxFragmentInputComponents)) {
                skip |= LogError(pipeline->pipeline(), "VUID-RuntimeSpirv-Location-06272",
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

bool CoreChecks::ValidateShaderStorageImageFormats(SHADER_MODULE_STATE const *src) const {
    bool skip = false;

    // Got through all ImageRead/Write instructions
    for (auto insn : *src) {
        switch (insn.opcode()) {
            case spv::OpImageSparseRead:
            case spv::OpImageRead: {
                spirv_inst_iter type_def = src->GetImageFormatInst(insn.word(3));
                if (type_def != src->end()) {
                    const auto dim = type_def.word(3);
                    // If the Image Dim operand is not SubpassData, the Image Format must not be Unknown, unless the
                    // StorageImageReadWithoutFormat Capability was declared.
                    if (dim != spv::DimSubpassData && type_def.word(8) == spv::ImageFormatUnknown) {
                        skip |= RequireFeature(enabled_features.core.shaderStorageImageReadWithoutFormat,
                                               "shaderStorageImageReadWithoutFormat",
                                               kVUID_Features_shaderStorageImageReadWithoutFormat);
                    }
                }
                break;
            }
            case spv::OpImageWrite: {
                spirv_inst_iter type_def = src->GetImageFormatInst(insn.word(1));
                if (type_def != src->end()) {
                    if (type_def.word(8) == spv::ImageFormatUnknown) {
                        skip |= RequireFeature(enabled_features.core.shaderStorageImageWriteWithoutFormat,
                                               "shaderStorageImageWriteWithoutFormat",
                                               kVUID_Features_shaderStorageImageWriteWithoutFormat);
                    }
                }
                break;
            }

        }
    }

    // Go through all variables for images and check decorations
    for (auto insn : *src) {
        if (insn.opcode() != spv::OpVariable)
            continue;

        uint32_t var = insn.word(2);
        spirv_inst_iter type_def = src->GetImageFormatInst(insn.word(1));
        if (type_def == src->end())
            continue;
        // Only check if the Image Dim operand is not SubpassData
        const auto dim = type_def.word(3);
        if (dim == spv::DimSubpassData) continue;
        // Only check storage images
        if (type_def.word(7) != 2) continue;
        if (type_def.word(8) != spv::ImageFormatUnknown) continue;

        decoration_set img_decorations = src->get_decorations(var);

        if (!enabled_features.core.shaderStorageImageReadWithoutFormat &&
            !(img_decorations.flags & decoration_set::nonreadable_bit)) {
            skip |= LogError(device, "VUID-RuntimeSpirv-OpTypeImage-06270",
                             "shaderStorageImageReadWithoutFormat not supported but variable %" PRIu32
                             " "
                             " without format not marked a NonReadable",
                             var);
        }

        if (!enabled_features.core.shaderStorageImageWriteWithoutFormat &&
            !(img_decorations.flags & decoration_set::nonwritable_bit)) {
            skip |= LogError(device, "VUID-RuntimeSpirv-OpTypeImage-06269",
                             "shaderStorageImageWriteWithoutFormat not supported but variable %" PRIu32
                             " "
                             "without format not marked a NonWritable",
                             var);
        }
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
        total_resources += pipeline->rp_state->createInfo.pSubpasses[pipeline->create_info.graphics.subpass].colorAttachmentCount;
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
        skip |= LogError(pipeline->pipeline(), vuid,
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
                                const layer_data::unordered_map<uint32_t, uint32_t> &id_to_spec_id, uint32_t *value) {
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
    layer_data::unordered_map<uint32_t, uint32_t> id_to_spec_id;
    // Map SPIR-V result ID to the ID of its type.
    layer_data::unordered_map<uint32_t, uint32_t> id_to_type_id;

    struct CoopMatType {
        uint32_t scope, rows, cols;
        VkComponentTypeNV component_type;
        bool all_constant;

        CoopMatType() : scope(0), rows(0), cols(0), component_type(VK_COMPONENT_TYPE_MAX_ENUM_NV), all_constant(false) {}

        void Init(uint32_t id, SHADER_MODULE_STATE const *src, VkPipelineShaderStageCreateInfo const *pStage,
                  const layer_data::unordered_map<uint32_t, uint32_t> &id_to_spec_id) {
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
                            pipeline->pipeline(), "VUID-RuntimeSpirv-OpTypeCooperativeMatrixNV-06322",
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
                CoopMatType m;
                m.Init(insn.word(1), src, pStage, id_to_spec_id);

                if (m.all_constant) {
                    // Validate that the type parameters are all supported for one of the
                    // operands of a cooperative matrix property.
                    bool valid = false;
                    for (unsigned i = 0; i < cooperative_matrix_properties.size(); ++i) {
                        if (cooperative_matrix_properties[i].AType == m.component_type &&
                            cooperative_matrix_properties[i].MSize == m.rows && cooperative_matrix_properties[i].KSize == m.cols &&
                            cooperative_matrix_properties[i].scope == m.scope) {
                            valid = true;
                            break;
                        }
                        if (cooperative_matrix_properties[i].BType == m.component_type &&
                            cooperative_matrix_properties[i].KSize == m.rows && cooperative_matrix_properties[i].NSize == m.cols &&
                            cooperative_matrix_properties[i].scope == m.scope) {
                            valid = true;
                            break;
                        }
                        if (cooperative_matrix_properties[i].CType == m.component_type &&
                            cooperative_matrix_properties[i].MSize == m.rows && cooperative_matrix_properties[i].NSize == m.cols &&
                            cooperative_matrix_properties[i].scope == m.scope) {
                            valid = true;
                            break;
                        }
                        if (cooperative_matrix_properties[i].DType == m.component_type &&
                            cooperative_matrix_properties[i].MSize == m.rows && cooperative_matrix_properties[i].NSize == m.cols &&
                            cooperative_matrix_properties[i].scope == m.scope) {
                            valid = true;
                            break;
                        }
                    }
                    if (!valid) {
                        skip |= LogError(pipeline->pipeline(), kVUID_Core_Shader_CooperativeMatrixType,
                                         "OpTypeCooperativeMatrixNV (result id = %u) operands don't match a supported matrix type",
                                         insn.word(1));
                    }
                }
                break;
            }
            case spv::OpCooperativeMatrixMulAddNV: {
                CoopMatType a, b, c, d;
                if (id_to_type_id.find(insn.word(2)) == id_to_type_id.end() ||
                    id_to_type_id.find(insn.word(3)) == id_to_type_id.end() ||
                    id_to_type_id.find(insn.word(4)) == id_to_type_id.end() ||
                    id_to_type_id.find(insn.word(5)) == id_to_type_id.end()) {
                    // Couldn't find type of matrix
                    assert(false);
                    break;
                }
                d.Init(id_to_type_id[insn.word(2)], src, pStage, id_to_spec_id);
                a.Init(id_to_type_id[insn.word(3)], src, pStage, id_to_spec_id);
                b.Init(id_to_type_id[insn.word(4)], src, pStage, id_to_spec_id);
                c.Init(id_to_type_id[insn.word(5)], src, pStage, id_to_spec_id);

                if (a.all_constant && b.all_constant && c.all_constant && d.all_constant) {
                    // Validate that the type parameters are all supported for the same
                    // cooperative matrix property.
                    bool valid = false;
                    for (unsigned i = 0; i < cooperative_matrix_properties.size(); ++i) {
                        if (cooperative_matrix_properties[i].AType == a.component_type &&
                            cooperative_matrix_properties[i].MSize == a.rows && cooperative_matrix_properties[i].KSize == a.cols &&
                            cooperative_matrix_properties[i].scope == a.scope &&

                            cooperative_matrix_properties[i].BType == b.component_type &&
                            cooperative_matrix_properties[i].KSize == b.rows && cooperative_matrix_properties[i].NSize == b.cols &&
                            cooperative_matrix_properties[i].scope == b.scope &&

                            cooperative_matrix_properties[i].CType == c.component_type &&
                            cooperative_matrix_properties[i].MSize == c.rows && cooperative_matrix_properties[i].NSize == c.cols &&
                            cooperative_matrix_properties[i].scope == c.scope &&

                            cooperative_matrix_properties[i].DType == d.component_type &&
                            cooperative_matrix_properties[i].MSize == d.rows && cooperative_matrix_properties[i].NSize == d.cols &&
                            cooperative_matrix_properties[i].scope == d.scope) {
                            valid = true;
                            break;
                        }
                    }
                    if (!valid) {
                        skip |= LogError(pipeline->pipeline(), kVUID_Core_Shader_CooperativeMatrixMulAdd,
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

bool CoreChecks::ValidateShaderResolveQCOM(SHADER_MODULE_STATE const *src, VkPipelineShaderStageCreateInfo const *pStage,
                                           const PIPELINE_STATE *pipeline) const {
    bool skip = false;

    // If the pipeline's subpass description contains flag VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_QCOM,
    // then the fragment shader must not enable the SPIRV SampleRateShading capability.
    if (pStage->stage == VK_SHADER_STAGE_FRAGMENT_BIT) {
        for (auto insn : *src) {
            switch (insn.opcode()) {
                case spv::OpCapability:
                    if (insn.word(1) == spv::CapabilitySampleRateShading) {
                        auto subpass_flags =
                            (pipeline->rp_state == nullptr)
                                ? 0
                                : pipeline->rp_state->createInfo.pSubpasses[pipeline->create_info.graphics.subpass].flags;
                        if ((subpass_flags & VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_QCOM) != 0) {
                            skip |=
                                LogError(pipeline->pipeline(), "VUID-RuntimeSpirv-SampleRateShading-06378",
                                         "Invalid Pipeline CreateInfo State: fragment shader enables SampleRateShading capability "
                                         "and the subpass flags includes VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_QCOM.");
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateShaderSubgroupSizeControl(VkPipelineShaderStageCreateInfo const *pStage) const {
    bool skip = false;

    if ((pStage->flags & VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT) != 0 &&
        !enabled_features.subgroup_size_control_features.subgroupSizeControl) {
        skip |= LogError(
            device, "VUID-VkPipelineShaderStageCreateInfo-flags-02784",
            "VkPipelineShaderStageCreateInfo flags contain VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT, "
            "but the VkPhysicalDeviceSubgroupSizeControlFeaturesEXT::subgroupSizeControl feature is not enabled.");
    }

    if ((pStage->flags & VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT) != 0 &&
        !enabled_features.subgroup_size_control_features.computeFullSubgroups) {
        skip |= LogError(
            device, "VUID-VkPipelineShaderStageCreateInfo-flags-02785",
            "VkPipelineShaderStageCreateInfo flags contain VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT, but the "
            "VkPhysicalDeviceSubgroupSizeControlFeaturesEXT::computeFullSubgroups feature is not enabled");
    }

    return skip;
}

bool CoreChecks::ValidateAtomicsTypes(SHADER_MODULE_STATE const *src) const {
    bool skip = false;

    // "If sparseImageInt64Atomics is enabled, shaderImageInt64Atomics must be enabled"
    const bool valid_image_64_int = enabled_features.shader_image_atomic_int64_features.shaderImageInt64Atomics == VK_TRUE;

    const VkPhysicalDeviceShaderAtomicFloatFeaturesEXT &float_features = enabled_features.shader_atomic_float_features;
    const VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT &float2_features = enabled_features.shader_atomic_float2_features;

    const bool valid_storage_buffer_float = (
        (float_features.shaderBufferFloat32Atomics == VK_TRUE) ||
        (float_features.shaderBufferFloat32AtomicAdd == VK_TRUE) ||
        (float_features.shaderBufferFloat64Atomics == VK_TRUE) ||
        (float_features.shaderBufferFloat64AtomicAdd == VK_TRUE) ||
        (float2_features.shaderBufferFloat16Atomics == VK_TRUE) ||
        (float2_features.shaderBufferFloat16AtomicAdd == VK_TRUE) ||
        (float2_features.shaderBufferFloat16AtomicMinMax == VK_TRUE) ||
        (float2_features.shaderBufferFloat32AtomicMinMax == VK_TRUE) ||
        (float2_features.shaderBufferFloat64AtomicMinMax == VK_TRUE));

    const bool valid_workgroup_float = (
        (float_features.shaderSharedFloat32Atomics == VK_TRUE) ||
        (float_features.shaderSharedFloat32AtomicAdd == VK_TRUE) ||
        (float_features.shaderSharedFloat64Atomics == VK_TRUE) ||
        (float_features.shaderSharedFloat64AtomicAdd == VK_TRUE) ||
        (float2_features.shaderSharedFloat16Atomics == VK_TRUE) ||
        (float2_features.shaderSharedFloat16AtomicAdd == VK_TRUE) ||
        (float2_features.shaderSharedFloat16AtomicMinMax == VK_TRUE) ||
        (float2_features.shaderSharedFloat32AtomicMinMax == VK_TRUE) ||
        (float2_features.shaderSharedFloat64AtomicMinMax == VK_TRUE));

    const bool valid_image_float = (
        (float_features.shaderImageFloat32Atomics == VK_TRUE) ||
        (float_features.shaderImageFloat32AtomicAdd == VK_TRUE) ||
        (float2_features.shaderImageFloat32AtomicMinMax == VK_TRUE));

    const bool valid_16_float = (
        (float2_features.shaderBufferFloat16Atomics == VK_TRUE) ||
        (float2_features.shaderBufferFloat16AtomicAdd == VK_TRUE) ||
        (float2_features.shaderBufferFloat16AtomicMinMax == VK_TRUE) ||
        (float2_features.shaderSharedFloat16Atomics == VK_TRUE) ||
        (float2_features.shaderSharedFloat16AtomicAdd == VK_TRUE) ||
        (float2_features.shaderSharedFloat16AtomicMinMax == VK_TRUE));

    const bool valid_32_float = (
        (float_features.shaderBufferFloat32Atomics == VK_TRUE) ||
        (float_features.shaderBufferFloat32AtomicAdd == VK_TRUE) ||
        (float_features.shaderSharedFloat32Atomics == VK_TRUE) ||
        (float_features.shaderSharedFloat32AtomicAdd == VK_TRUE) ||
        (float_features.shaderImageFloat32Atomics == VK_TRUE) ||
        (float_features.shaderImageFloat32AtomicAdd == VK_TRUE) ||
        (float2_features.shaderBufferFloat32AtomicMinMax == VK_TRUE) ||
        (float2_features.shaderSharedFloat32AtomicMinMax == VK_TRUE) ||
        (float2_features.shaderImageFloat32AtomicMinMax == VK_TRUE));

    const bool valid_64_float = (
        (float_features.shaderBufferFloat64Atomics == VK_TRUE) ||
        (float_features.shaderBufferFloat64AtomicAdd == VK_TRUE) ||
        (float_features.shaderSharedFloat64Atomics == VK_TRUE) ||
        (float_features.shaderSharedFloat64AtomicAdd == VK_TRUE) ||
        (float2_features.shaderBufferFloat64AtomicMinMax == VK_TRUE) ||
        (float2_features.shaderSharedFloat64AtomicMinMax == VK_TRUE));
    // clang-format on

    for (auto &atomic_inst : src->atomic_inst) {
        const atomic_instruction &atomic = atomic_inst.second;
        const uint32_t opcode = src->at(atomic_inst.first).opcode();

        if ((atomic.bit_width == 64) && (atomic.type == spv::OpTypeInt)) {
            // Validate 64-bit image atomics
            if (((atomic.storage_class == spv::StorageClassStorageBuffer) || (atomic.storage_class == spv::StorageClassUniform)) &&
                (enabled_features.core12.shaderBufferInt64Atomics == VK_FALSE)) {
                skip |= LogError(
                    device, "VUID-RuntimeSpirv-None-06278",
                    "%s: Can't use 64-bit int atomics operations with %s storage class without shaderBufferInt64Atomics enabled.",
                    report_data->FormatHandle(src->vk_shader_module()).c_str(), StorageClassName(atomic.storage_class));
            } else if ((atomic.storage_class == spv::StorageClassWorkgroup) &&
                       (enabled_features.core12.shaderSharedInt64Atomics == VK_FALSE)) {
                skip |= LogError(device, "VUID-RuntimeSpirv-None-06279",
                                 "%s: Can't use 64-bit int atomics operations with Workgroup storage class without "
                                 "shaderSharedInt64Atomics enabled.",
                                 report_data->FormatHandle(src->vk_shader_module()).c_str());
            } else if ((atomic.storage_class == spv::StorageClassImage) && (valid_image_64_int == false)) {
                skip |= LogError(device, "VUID-RuntimeSpirv-None-06288",
                                 "%s: Can't use 64-bit int atomics operations with Image storage class without "
                                 "shaderImageInt64Atomics enabled.",
                                 report_data->FormatHandle(src->vk_shader_module()).c_str());
            }
        } else if (atomic.type == spv::OpTypeFloat) {
            // Validate Floats
            if (atomic.storage_class == spv::StorageClassStorageBuffer) {
                if (valid_storage_buffer_float == false) {
                    const char *vuid = IsExtEnabled(device_extensions.vk_ext_shader_atomic_float2) ? "VUID-RuntimeSpirv-None-06284"
                                                                                                   : "VUID-RuntimeSpirv-None-06280";
                    skip |= LogError(device, vuid,
                                     "%s: Can't use float atomics operations with StorageBuffer storage class without "
                                     "shaderBufferFloat32Atomics or shaderBufferFloat32AtomicAdd or shaderBufferFloat64Atomics or "
                                     "shaderBufferFloat64AtomicAdd or shaderBufferFloat16Atomics or shaderBufferFloat16AtomicAdd "
                                     "or shaderBufferFloat16AtomicMinMax or shaderBufferFloat32AtomicMinMax or "
                                     "shaderBufferFloat64AtomicMinMax enabled.",
                                     report_data->FormatHandle(src->vk_shader_module()).c_str());
                } else if (opcode == spv::OpAtomicFAddEXT) {
                    if ((atomic.bit_width == 16) && (float2_features.shaderBufferFloat16AtomicAdd == VK_FALSE)) {
                        skip |= LogError(device, kVUID_Core_Shader_AtomicFeature,
                                         "%s: Can't use 16-bit float atomics for add operations (OpAtomicFAddEXT) with "
                                         "StorageBuffer storage class without shaderBufferFloat16AtomicAdd enabled.",
                                         report_data->FormatHandle(src->vk_shader_module()).c_str());
                    } else if ((atomic.bit_width == 32) && (float_features.shaderBufferFloat32AtomicAdd == VK_FALSE)) {
                        skip |= LogError(device, kVUID_Core_Shader_AtomicFeature,
                                         "%s: Can't use 32-bit float atomics for add operations (OpAtomicFAddEXT) with "
                                         "StorageBuffer storage class without shaderBufferFloat32AtomicAdd enabled.",
                                         report_data->FormatHandle(src->vk_shader_module()).c_str());
                    } else if ((atomic.bit_width == 64) && (float_features.shaderBufferFloat64AtomicAdd == VK_FALSE)) {
                        skip |= LogError(device, kVUID_Core_Shader_AtomicFeature,
                                         "%s: Can't use 64-bit float atomics for add operations (OpAtomicFAddEXT) with "
                                         "StorageBuffer storage class without shaderBufferFloat64AtomicAdd enabled.",
                                         report_data->FormatHandle(src->vk_shader_module()).c_str());
                    }
                } else if (opcode == spv::OpAtomicFMinEXT || opcode == spv::OpAtomicFMaxEXT) {
                    if ((atomic.bit_width == 16) && (float2_features.shaderBufferFloat16AtomicMinMax == VK_FALSE)) {
                        skip |= LogError(
                            device, kVUID_Core_Shader_AtomicFeature,
                            "%s: Can't use 16-bit float atomics for min/max operations (OpAtomicFMinEXT or OpAtomicFMaxEXT) with "
                            "StorageBuffer storage class without shaderBufferFloat16AtomicMinMax enabled.",
                            report_data->FormatHandle(src->vk_shader_module()).c_str());
                    } else if ((atomic.bit_width == 32) && (float2_features.shaderBufferFloat32AtomicMinMax == VK_FALSE)) {
                        skip |= LogError(
                            device, kVUID_Core_Shader_AtomicFeature,
                            "%s: Can't use 32-bit float atomics for min/max operations (OpAtomicFMinEXT or OpAtomicFMaxEXT) with "
                            "StorageBuffer storage class without shaderBufferFloat32AtomicMinMax enabled.",
                            report_data->FormatHandle(src->vk_shader_module()).c_str());
                    } else if ((atomic.bit_width == 64) && (float2_features.shaderBufferFloat64AtomicMinMax == VK_FALSE)) {
                        skip |= LogError(
                            device, kVUID_Core_Shader_AtomicFeature,
                            "%s: Can't use 64-bit float atomics for min/max operations (OpAtomicFMinEXT or OpAtomicFMaxEXT) with "
                            "StorageBuffer storage class without shaderBufferFloat64AtomicMinMax enabled.",
                            report_data->FormatHandle(src->vk_shader_module()).c_str());
                    }
                } else {
                    // Assume is valid load/store/exchange (rest of supported atomic operations) or else spirv-val will catch
                    if ((atomic.bit_width == 16) && (float2_features.shaderBufferFloat16Atomics == VK_FALSE)) {
                        skip |= LogError(
                            device, kVUID_Core_Shader_AtomicFeature,
                            "%s: Can't use 16-bit float atomics for load/store/exhange operations (OpAtomicLoad, OpAtomicStore, "
                            "OpAtomicExchange) with StorageBuffer storage class without shaderBufferFloat16Atomics enabled.",
                            report_data->FormatHandle(src->vk_shader_module()).c_str());
                    } else if ((atomic.bit_width == 32) && (float_features.shaderBufferFloat32Atomics == VK_FALSE)) {
                        skip |= LogError(
                            device, kVUID_Core_Shader_AtomicFeature,
                            "%s: Can't use 32-bit float atomics for load/store/exhange operations (OpAtomicLoad, OpAtomicStore, "
                            "OpAtomicExchange) with StorageBuffer storage class without shaderBufferFloat32Atomics enabled.",
                            report_data->FormatHandle(src->vk_shader_module()).c_str());
                    } else if ((atomic.bit_width == 64) && (float_features.shaderBufferFloat64Atomics == VK_FALSE)) {
                        skip |= LogError(
                            device, kVUID_Core_Shader_AtomicFeature,
                            "%s: Can't use 64-bit float atomics for load/store/exhange operations (OpAtomicLoad, OpAtomicStore, "
                            "OpAtomicExchange) with StorageBuffer storage class without shaderBufferFloat64Atomics enabled.",
                            report_data->FormatHandle(src->vk_shader_module()).c_str());
                    }
                }
            } else if (atomic.storage_class == spv::StorageClassWorkgroup) {
                if (valid_workgroup_float == false) {
                    const char *vuid = IsExtEnabled(device_extensions.vk_ext_shader_atomic_float2) ? "VUID-RuntimeSpirv-None-06285"
                                                                                                   : "VUID-RuntimeSpirv-None-06281";
                    skip |= LogError(
                        device, vuid,
                        "%s: Can't use float atomics operations with Workgroup storage class without shaderSharedFloat32Atomics or "
                        "shaderSharedFloat32AtomicAdd or shaderSharedFloat64Atomics or shaderSharedFloat64AtomicAdd or "
                        "shaderSharedFloat16Atomics or shaderSharedFloat16AtomicAdd or shaderSharedFloat16AtomicMinMax or "
                        "shaderSharedFloat32AtomicMinMax or shaderSharedFloat64AtomicMinMax enabled.",
                        report_data->FormatHandle(src->vk_shader_module()).c_str());
                } else if (opcode == spv::OpAtomicFAddEXT) {
                    if ((atomic.bit_width == 16) && (float2_features.shaderSharedFloat16AtomicAdd == VK_FALSE)) {
                        skip |= LogError(device, kVUID_Core_Shader_AtomicFeature,
                                         "%s: Can't use 16-bit float atomics for add operations (OpAtomicFAddEXT) with Workgroup "
                                         "storage class without shaderSharedFloat16AtomicAdd enabled.",
                                         report_data->FormatHandle(src->vk_shader_module()).c_str());
                    } else if ((atomic.bit_width == 32) && (float_features.shaderSharedFloat32AtomicAdd == VK_FALSE)) {
                        skip |= LogError(device, kVUID_Core_Shader_AtomicFeature,
                                         "%s: Can't use 32-bit float atomics for add operations (OpAtomicFAddEXT) with Workgroup "
                                         "storage class without shaderSharedFloat32AtomicAdd enabled.",
                                         report_data->FormatHandle(src->vk_shader_module()).c_str());
                    } else if ((atomic.bit_width == 64) && (float_features.shaderSharedFloat64AtomicAdd == VK_FALSE)) {
                        skip |= LogError(device, kVUID_Core_Shader_AtomicFeature,
                                         "%s: Can't use 64-bit float atomics for add operations (OpAtomicFAddEXT) with Workgroup "
                                         "storage class without shaderSharedFloat64AtomicAdd enabled.",
                                         report_data->FormatHandle(src->vk_shader_module()).c_str());
                    }
                } else if (opcode == spv::OpAtomicFMinEXT || opcode == spv::OpAtomicFMaxEXT) {
                    if ((atomic.bit_width == 16) && (float2_features.shaderSharedFloat16AtomicMinMax == VK_FALSE)) {
                        skip |= LogError(
                            device, kVUID_Core_Shader_AtomicFeature,
                            "%s: Can't use 16-bit float atomics for min/max operations (OpAtomicFMinEXT or OpAtomicFMaxEXT) with "
                            "Workgroup storage class without shaderSharedFloat16AtomicMinMax enabled.",
                            report_data->FormatHandle(src->vk_shader_module()).c_str());
                    } else if ((atomic.bit_width == 32) && (float2_features.shaderSharedFloat32AtomicMinMax == VK_FALSE)) {
                        skip |= LogError(
                            device, kVUID_Core_Shader_AtomicFeature,
                            "%s: Can't use 32-bit float atomics for min/max operations (OpAtomicFMinEXT or OpAtomicFMaxEXT) with "
                            "Workgroup storage class without shaderSharedFloat32AtomicMinMax enabled.",
                            report_data->FormatHandle(src->vk_shader_module()).c_str());
                    } else if ((atomic.bit_width == 64) && (float2_features.shaderSharedFloat64AtomicMinMax == VK_FALSE)) {
                        skip |= LogError(
                            device, kVUID_Core_Shader_AtomicFeature,
                            "%s: Can't use 64-bit float atomics for min/max operations (OpAtomicFMinEXT or OpAtomicFMaxEXT) with "
                            "Workgroup storage class without shaderSharedFloat64AtomicMinMax enabled.",
                            report_data->FormatHandle(src->vk_shader_module()).c_str());
                    }
                } else {
                    // Assume is valid load/store/exchange (rest of supported atomic operations) or else spirv-val will catch
                    if ((atomic.bit_width == 16) && (float2_features.shaderSharedFloat16Atomics == VK_FALSE)) {
                        skip |= LogError(
                            device, kVUID_Core_Shader_AtomicFeature,
                            "%s: Can't use 16-bit float atomics for load/store/exhange operations (OpAtomicLoad, OpAtomicStore, "
                            "OpAtomicExchange) with Workgroup storage class without shaderSharedFloat16Atomics enabled.",
                            report_data->FormatHandle(src->vk_shader_module()).c_str());
                    } else if ((atomic.bit_width == 32) && (float_features.shaderSharedFloat32Atomics == VK_FALSE)) {
                        skip |= LogError(
                            device, kVUID_Core_Shader_AtomicFeature,
                            "%s: Can't use 32-bit float atomics for load/store/exhange operations (OpAtomicLoad, OpAtomicStore, "
                            "OpAtomicExchange) with Workgroup storage class without shaderSharedFloat32Atomics enabled.",
                            report_data->FormatHandle(src->vk_shader_module()).c_str());
                    } else if ((atomic.bit_width == 64) && (float_features.shaderSharedFloat64Atomics == VK_FALSE)) {
                        skip |= LogError(
                            device, kVUID_Core_Shader_AtomicFeature,
                            "%s: Can't use 64-bit float atomics for load/store/exhange operations (OpAtomicLoad, OpAtomicStore, "
                            "OpAtomicExchange) with Workgroup storage class without shaderSharedFloat64Atomics enabled.",
                            report_data->FormatHandle(src->vk_shader_module()).c_str());
                    }
                }
            } else if ((atomic.storage_class == spv::StorageClassImage) && (valid_image_float == false)) {
                const char *vuid = IsExtEnabled(device_extensions.vk_ext_shader_atomic_float2) ? "VUID-RuntimeSpirv-None-06286"
                                                                                               : "VUID-RuntimeSpirv-None-06282";
                skip |=
                    LogError(device, vuid,
                             "%s: Can't use float atomics operations with Image storage class without shaderImageFloat32Atomics or "
                             "shaderImageFloat32AtomicAdd or shaderImageFloat32AtomicMinMax enabled.",
                             report_data->FormatHandle(src->vk_shader_module()).c_str());
            } else if ((atomic.bit_width == 16) && (valid_16_float == false)) {
                skip |= LogError(device, "VUID-RuntimeSpirv-None-06337",
                                 "%s: Can't use 16-bit float atomics operations without shaderBufferFloat16Atomics, "
                                 "shaderBufferFloat16AtomicAdd, shaderBufferFloat16AtomicMinMax, shaderSharedFloat16Atomics, "
                                 "shaderSharedFloat16AtomicAdd or shaderSharedFloat16AtomicMinMax enabled.",
                                 report_data->FormatHandle(src->vk_shader_module()).c_str());
            } else if ((atomic.bit_width == 32) && (valid_32_float == false)) {
                const char *vuid = IsExtEnabled(device_extensions.vk_ext_shader_atomic_float2) ? "VUID-RuntimeSpirv-None-06338"
                                                                                               : "VUID-RuntimeSpirv-None-06335";
                skip |= LogError(device, vuid,
                                 "%s: Can't use 32-bit float atomics operations without shaderBufferFloat32AtomicMinMax, "
                                 "shaderSharedFloat32AtomicMinMax, shaderImageFloat32AtomicMinMax, sparseImageFloat32AtomicMinMax, "
                                 "shaderBufferFloat32Atomics, shaderBufferFloat32AtomicAdd, shaderSharedFloat32Atomics, "
                                 "shaderSharedFloat32AtomicAdd, shaderImageFloat32Atomics, shaderImageFloat32AtomicAdd, "
                                 "sparseImageFloat32Atomics or sparseImageFloat32AtomicAdd enabled.",
                                 report_data->FormatHandle(src->vk_shader_module()).c_str());
            } else if ((atomic.bit_width == 64) && (valid_64_float == false)) {
                const char *vuid = IsExtEnabled(device_extensions.vk_ext_shader_atomic_float2) ? "VUID-RuntimeSpirv-None-06339"
                                                                                               : "VUID-RuntimeSpirv-None-06336";
                skip |= LogError(device, vuid,
                                 "%s: Can't use 64-bit float atomics operations without shaderBufferFloat64AtomicMinMax, "
                                 "shaderSharedFloat64AtomicMinMax, shaderBufferFloat64Atomics, shaderBufferFloat64AtomicAdd, "
                                 "shaderSharedFloat64Atomics or shaderSharedFloat64AtomicAdd enabled.",
                                 report_data->FormatHandle(src->vk_shader_module()).c_str());
            }
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

    uint32_t vertices_out = 0;
    uint32_t invocations = 0;

    auto it = src->execution_mode_inst.find(entrypoint_id);
    if (it != src->execution_mode_inst.end()) {
        for (auto insn : it->second) {
            auto mode = insn.word(2);
            switch (mode) {
                case spv::ExecutionModeSignedZeroInfNanPreserve: {
                    auto bit_width = insn.word(3);
                    if (bit_width == 16 && !phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat16) {
                        skip |= LogError(
                            device, "VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat16-06293",
                            "Shader requires SignedZeroInfNanPreserve for bit width 16 but it is not enabled on the device");
                    } else if (bit_width == 32 && !phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat32) {
                        skip |= LogError(
                            device, "VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat32-06294",
                            "Shader requires SignedZeroInfNanPreserve for bit width 32 but it is not enabled on the device");
                    } else if (bit_width == 64 && !phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat64) {
                        skip |= LogError(
                            device, "VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat64-06295",
                            "Shader requires SignedZeroInfNanPreserve for bit width 64 but it is not enabled on the device");
                    }
                    break;
                }

                case spv::ExecutionModeDenormPreserve: {
                    auto bit_width = insn.word(3);
                    if (bit_width == 16 && !phys_dev_props_core12.shaderDenormPreserveFloat16) {
                        skip |= LogError(device, "VUID-RuntimeSpirv-shaderDenormPreserveFloat16-06296",
                                         "Shader requires DenormPreserve for bit width 16 but it is not enabled on the device");
                    } else if (bit_width == 32 && !phys_dev_props_core12.shaderDenormPreserveFloat32) {
                        skip |= LogError(device, "VUID-RuntimeSpirv-shaderDenormPreserveFloat32-06297",
                                         "Shader requires DenormPreserve for bit width 32 but it is not enabled on the device");
                    } else if (bit_width == 64 && !phys_dev_props_core12.shaderDenormPreserveFloat64) {
                        skip |= LogError(device, "VUID-RuntimeSpirv-shaderDenormPreserveFloat64-06298",
                                         "Shader requires DenormPreserve for bit width 64 but it is not enabled on the device");
                    }

                    if (first_denorm_execution_mode.first == spv::ExecutionModeMax) {
                        // Register the first denorm execution mode found
                        first_denorm_execution_mode = std::make_pair(static_cast<spv::ExecutionMode>(mode), bit_width);
                    } else if (first_denorm_execution_mode.first != mode && first_denorm_execution_mode.second != bit_width) {
                        switch (phys_dev_props_core12.denormBehaviorIndependence) {
                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY:
                                if (first_rounding_mode.second != 32 && bit_width != 32) {
                                    skip |= LogError(device, "VUID-RuntimeSpirv-denormBehaviorIndependence-06289",
                                                     "Shader uses different denorm execution modes for 16 and 64-bit but "
                                                     "denormBehaviorIndependence is "
                                                     "VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY on the device");
                                }
                                break;

                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL:
                                break;

                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE:
                                skip |= LogError(device, "VUID-RuntimeSpirv-denormBehaviorIndependence-06290",
                                                 "Shader uses different denorm execution modes for different bit widths but "
                                                 "denormBehaviorIndependence is "
                                                 "VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE on the device");
                                break;

                            default:
                                break;
                        }
                    }
                    break;
                }

                case spv::ExecutionModeDenormFlushToZero: {
                    auto bit_width = insn.word(3);
                    if (bit_width == 16 && !phys_dev_props_core12.shaderDenormFlushToZeroFloat16) {
                        skip |= LogError(device, "VUID-RuntimeSpirv-shaderDenormFlushToZeroFloat16-06299",
                                         "Shader requires DenormFlushToZero for bit width 16 but it is not enabled on the device");
                    } else if (bit_width == 32 && !phys_dev_props_core12.shaderDenormFlushToZeroFloat32) {
                        skip |= LogError(device, "VUID-RuntimeSpirv-shaderDenormFlushToZeroFloat32-06300",
                                         "Shader requires DenormFlushToZero for bit width 32 but it is not enabled on the device");
                    } else if (bit_width == 64 && !phys_dev_props_core12.shaderDenormFlushToZeroFloat64) {
                        skip |= LogError(device, "VUID-RuntimeSpirv-shaderDenormFlushToZeroFloat64-06301",
                                         "Shader requires DenormFlushToZero for bit width 64 but it is not enabled on the device");
                    }

                    if (first_denorm_execution_mode.first == spv::ExecutionModeMax) {
                        // Register the first denorm execution mode found
                        first_denorm_execution_mode = std::make_pair(static_cast<spv::ExecutionMode>(mode), bit_width);
                    } else if (first_denorm_execution_mode.first != mode && first_denorm_execution_mode.second != bit_width) {
                        switch (phys_dev_props_core12.denormBehaviorIndependence) {
                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY:
                                if (first_rounding_mode.second != 32 && bit_width != 32) {
                                    skip |= LogError(device, "VUID-RuntimeSpirv-denormBehaviorIndependence-06289",
                                                     "Shader uses different denorm execution modes for 16 and 64-bit but "
                                                     "denormBehaviorIndependence is "
                                                     "VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY on the device");
                                }
                                break;

                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL:
                                break;

                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE:
                                skip |= LogError(device, "VUID-RuntimeSpirv-denormBehaviorIndependence-06290",
                                                 "Shader uses different denorm execution modes for different bit widths but "
                                                 "denormBehaviorIndependence is "
                                                 "VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE on the device");
                                break;

                            default:
                                break;
                        }
                    }
                    break;
                }

                case spv::ExecutionModeRoundingModeRTE: {
                    auto bit_width = insn.word(3);
                    if (bit_width == 16 && !phys_dev_props_core12.shaderRoundingModeRTEFloat16) {
                        skip |= LogError(device, "VUID-RuntimeSpirv-shaderRoundingModeRTEFloat16-06302",
                                         "Shader requires RoundingModeRTE for bit width 16 but it is not enabled on the device");
                    } else if (bit_width == 32 && !phys_dev_props_core12.shaderRoundingModeRTEFloat32) {
                        skip |= LogError(device, "VUID-RuntimeSpirv-shaderRoundingModeRTEFloat32-06303",
                                         "Shader requires RoundingModeRTE for bit width 32 but it is not enabled on the device");
                    } else if (bit_width == 64 && !phys_dev_props_core12.shaderRoundingModeRTEFloat64) {
                        skip |= LogError(device, "VUID-RuntimeSpirv-shaderRoundingModeRTEFloat64-06304",
                                         "Shader requires RoundingModeRTE for bit width 64 but it is not enabled on the device");
                    }

                    if (first_rounding_mode.first == spv::ExecutionModeMax) {
                        // Register the first rounding mode found
                        first_rounding_mode = std::make_pair(static_cast<spv::ExecutionMode>(mode), bit_width);
                    } else if (first_rounding_mode.first != mode && first_rounding_mode.second != bit_width) {
                        switch (phys_dev_props_core12.roundingModeIndependence) {
                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY:
                                if (first_rounding_mode.second != 32 && bit_width != 32) {
                                    skip |= LogError(device, "VUID-RuntimeSpirv-roundingModeIndependence-06291",
                                                     "Shader uses different rounding modes for 16 and 64-bit but "
                                                     "roundingModeIndependence is "
                                                     "VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY on the device");
                                }
                                break;

                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL:
                                break;

                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE:
                                skip |= LogError(device, "VUID-RuntimeSpirv-roundingModeIndependence-06292",
                                                 "Shader uses different rounding modes for different bit widths but "
                                                 "roundingModeIndependence is "
                                                 "VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE on the device");
                                break;

                            default:
                                break;
                        }
                    }
                    break;
                }

                case spv::ExecutionModeRoundingModeRTZ: {
                    auto bit_width = insn.word(3);
                    if (bit_width == 16 && !phys_dev_props_core12.shaderRoundingModeRTZFloat16) {
                        skip |= LogError(device, "VUID-RuntimeSpirv-shaderRoundingModeRTZFloat16-06305",
                                         "Shader requires RoundingModeRTZ for bit width 16 but it is not enabled on the device");
                    } else if (bit_width == 32 && !phys_dev_props_core12.shaderRoundingModeRTZFloat32) {
                        skip |= LogError(device, "VUID-RuntimeSpirv-shaderRoundingModeRTZFloat32-06306",
                                         "Shader requires RoundingModeRTZ for bit width 32 but it is not enabled on the device");
                    } else if (bit_width == 64 && !phys_dev_props_core12.shaderRoundingModeRTZFloat64) {
                        skip |= LogError(device, "VUID-RuntimeSpirv-shaderRoundingModeRTZFloat64-06307",
                                         "Shader requires RoundingModeRTZ for bit width 64 but it is not enabled on the device");
                    }

                    if (first_rounding_mode.first == spv::ExecutionModeMax) {
                        // Register the first rounding mode found
                        first_rounding_mode = std::make_pair(static_cast<spv::ExecutionMode>(mode), bit_width);
                    } else if (first_rounding_mode.first != mode && first_rounding_mode.second != bit_width) {
                        switch (phys_dev_props_core12.roundingModeIndependence) {
                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY:
                                if (first_rounding_mode.second != 32 && bit_width != 32) {
                                    skip |= LogError(device, "VUID-RuntimeSpirv-roundingModeIndependence-06291",
                                                     "Shader uses different rounding modes for 16 and 64-bit but "
                                                     "roundingModeIndependence is "
                                                     "VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY on the device");
                                }
                                break;

                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL:
                                break;

                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE:
                                skip |= LogError(device, "VUID-RuntimeSpirv-roundingModeIndependence-06292",
                                                 "Shader uses different rounding modes for different bit widths but "
                                                 "roundingModeIndependence is "
                                                 "VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE on the device");
                                break;

                            default:
                                break;
                        }
                    }
                    break;
                }

                case spv::ExecutionModeOutputVertices: {
                    vertices_out = insn.word(3);
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
        if (vertices_out == 0 || vertices_out > phys_dev_props.limits.maxGeometryOutputVertices) {
            skip |= LogError(device, "VUID-VkPipelineShaderStageCreateInfo-stage-00714",
                             "Geometry shader entry point must have an OpExecutionMode instruction that "
                             "specifies a maximum output vertex count that is greater than 0 and less "
                             "than or equal to maxGeometryOutputVertices. "
                             "OutputVertices=%d, maxGeometryOutputVertices=%d",
                             vertices_out, phys_dev_props.limits.maxGeometryOutputVertices);
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

// For given pipelineLayout verify that the set_layout_node at slot.first
//  has the requested binding at slot.second and return ptr to that binding
static VkDescriptorSetLayoutBinding const *GetDescriptorBinding(PIPELINE_LAYOUT_STATE const *pipelineLayout,
                                                                DescriptorSlot slot) {
    if (!pipelineLayout) return nullptr;

    if (slot.set >= pipelineLayout->set_layouts.size()) return nullptr;

    return pipelineLayout->set_layouts[slot.set]->GetDescriptorSetLayoutBindingPtrFromBinding(slot.binding);
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
    for (auto set : src->builtin_decoration_list) {
        auto insn = src->at(set.offset);
        if (set.builtin == spv::BuiltInPointSize) {
            pointsize_written = src->IsBuiltInWritten(insn, entrypoint);
            if (pointsize_written) {
                break;
            }
        }
    }

    if ((stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT || stage == VK_SHADER_STAGE_GEOMETRY_BIT) &&
        !enabled_features.core.shaderTessellationAndGeometryPointSize) {
        if (pointsize_written) {
            skip |= LogError(pipeline->pipeline(), kVUID_Core_Shader_PointSizeBuiltInOverSpecified,
                             "Pipeline topology is set to POINT_LIST and geometry or tessellation shaders write PointSize which "
                             "is prohibited when the shaderTessellationAndGeometryPointSize feature is not enabled.");
        }
    } else if (!pointsize_written) {
        skip |=
            LogError(pipeline->pipeline(), kVUID_Core_Shader_MissingPointSizeBuiltIn,
                     "Pipeline topology is set to POINT_LIST, but PointSize is not written to in the shader corresponding to %s.",
                     string_VkShaderStageFlagBits(stage));
    }
    return skip;
}

bool CoreChecks::ValidatePrimitiveRateShaderState(const PIPELINE_STATE *pipeline, SHADER_MODULE_STATE const *src,
                                                  spirv_inst_iter entrypoint, VkShaderStageFlagBits stage) const {
    bool primitiverate_written = false;
    bool viewportindex_written = false;
    bool viewportmask_written = false;
    bool skip = false;

    // Check if the primitive shading rate is written
    for (auto set : src->builtin_decoration_list) {
        auto insn = src->at(set.offset);
        if (set.builtin == spv::BuiltInPrimitiveShadingRateKHR) {
            primitiverate_written = src->IsBuiltInWritten(insn, entrypoint);
        } else if (set.builtin == spv::BuiltInViewportIndex) {
            viewportindex_written = src->IsBuiltInWritten(insn, entrypoint);
        } else if (set.builtin == spv::BuiltInViewportMaskNV) {
            viewportmask_written = src->IsBuiltInWritten(insn, entrypoint);
        }
        if (primitiverate_written && viewportindex_written && viewportmask_written) {
            break;
        }
    }

    if (!phys_dev_ext_props.fragment_shading_rate_props.primitiveFragmentShadingRateWithMultipleViewports &&
        (pipeline->GetPipelineType() == VK_PIPELINE_BIND_POINT_GRAPHICS) && pipeline->create_info.graphics.pViewportState) {
        if (!IsDynamic(pipeline, VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT) &&
            pipeline->create_info.graphics.pViewportState->viewportCount > 1 && primitiverate_written) {
            skip |= LogError(pipeline->pipeline(),
                             "VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04503",
                             "vkCreateGraphicsPipelines: %s shader statically writes to PrimitiveShadingRateKHR built-in, but "
                             "multiple viewports "
                             "are used and the primitiveFragmentShadingRateWithMultipleViewports limit is not supported.",
                             string_VkShaderStageFlagBits(stage));
        }

        if (primitiverate_written && viewportindex_written) {
            skip |= LogError(pipeline->pipeline(),
                             "VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04504",
                             "vkCreateGraphicsPipelines: %s shader statically writes to both PrimitiveShadingRateKHR and "
                             "ViewportIndex built-ins,"
                             "but the primitiveFragmentShadingRateWithMultipleViewports limit is not supported.",
                             string_VkShaderStageFlagBits(stage));
        }

        if (primitiverate_written && viewportmask_written) {
            skip |= LogError(pipeline->pipeline(),
                             "VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04505",
                             "vkCreateGraphicsPipelines: %s shader statically writes to both PrimitiveShadingRateKHR and "
                             "ViewportMaskNV built-ins,"
                             "but the primitiveFragmentShadingRateWithMultipleViewports limit is not supported.",
                             string_VkShaderStageFlagBits(stage));
        }
    }
    return skip;
}

bool CoreChecks::ValidateDecorations(SHADER_MODULE_STATE const* module) const {
    bool skip = false;

    for (const auto &op_decorate : module->decoration_inst) {
        uint32_t decoration = op_decorate.word(2);
        if (decoration == spv::DecorationXfbStride) {
            uint32_t stride = op_decorate.word(3);
            if (stride > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferDataStride) {
                skip |= LogError(
                    device, "VUID-RuntimeSpirv-XfbStride-06313",
                    "vkCreateGraphicsPipelines(): shader uses transform feedback with xfb_stride (%" PRIu32
                    ") greater than VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackBufferDataStride (%" PRIu32
                    ").",
                    stride, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferDataStride);
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateTransformFeedback(SHADER_MODULE_STATE const *module, spirv_inst_iter &insn) const {
    bool skip = false;

    uint32_t opcode = insn.opcode();
    if (opcode == spv::OpEmitStreamVertex || opcode == spv::OpEndStreamPrimitive) {
        uint32_t stream = static_cast<uint32_t>(module->GetConstantValueById(insn.word(1)));
        if (stream >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams) {
            skip |= LogError(
                device, "VUID-RuntimeSpirv-OpEmitStreamVertex-06310",
                "vkCreateGraphicsPipelines(): shader uses transform feedback stream with index %" PRIu32
                ", which is not less than VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackStreams (%" PRIu32
                ").",
                stream, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams);
        }
    }

    return skip;
}

bool CoreChecks::ValidateTexelGatherOffset(SHADER_MODULE_STATE const *src, spirv_inst_iter &insn) const {
    bool skip = false;

    const uint32_t opcode = insn.opcode();
    // If opcode is OpImage*Gather
    if (opcode == spv::OpImageGather || opcode == spv::OpImageDrefGather || opcode == spv::OpImageSparseGather ||
        opcode == spv::OpImageSparseDrefGather) {
        if (insn.len() > 6) { // Image operands are optional
            auto image_operand = insn.word(6);
            // Bits we are validating
            uint32_t offset_bits =
                spv::ImageOperandsOffsetMask | spv::ImageOperandsConstOffsetMask | spv::ImageOperandsConstOffsetsMask;
            if (image_operand & (offset_bits)) {
                // Operand values start at word 7
                uint32_t index = 7;
                // Each bit has it's own operand, starts with the smallest set bit and loop to the highest bit among
                // ImageOperandsOffsetMask, ImageOperandsConstOffsetMask and ImageOperandsConstOffsetsMask
                for (uint32_t i = 1; i < spv::ImageOperandsConstOffsetsMask; i <<= 1) {
                    if (image_operand & i) {  // If the bit is set, consume operand
                        if (insn.len() > index && (i & offset_bits)) {
                            uint32_t constant_id = insn.word(index);
                            const auto &constant = src->get_def(constant_id);
                            const bool is_dynamic_offset = constant == src->end();
                            if (!is_dynamic_offset && constant.opcode() == spv::OpConstantComposite) {
                                for (uint32_t j = 3; j < constant.len(); ++j) {
                                    uint32_t comp_id = constant.word(j);
                                    const auto &comp = src->get_def(comp_id);
                                    const auto &comp_type = src->get_def(comp.word(1));
                                    // Get operand value
                                    const uint32_t offset = comp.word(3);
                                    const int32_t signed_offset = static_cast<int32_t>(offset);
                                    const bool use_signed = (comp_type.opcode() == spv::OpTypeInt && comp_type.word(3) != 0);

                                    // spec requires minTexelGatherOffset to be -8 or less so never can compare if unsigned
                                    // spec requires maxTexelGatherOffset to be 7 or greater so never can compare if signed is less
                                    // then zero
                                    if (use_signed && (signed_offset < phys_dev_props.limits.minTexelGatherOffset)) {
                                        skip |= LogError(device, "VUID-RuntimeSpirv-OpImage-06376",
                                                         "vkCreateShaderModule(): Shader uses OpImageGather with offset (%" PRIi32
                                                         ") less than VkPhysicalDeviceLimits::minTexelGatherOffset (%" PRIi32 ").",
                                                         signed_offset, phys_dev_props.limits.minTexelGatherOffset);
                                    } else if ((offset > phys_dev_props.limits.maxTexelGatherOffset) &&
                                               (!use_signed || (use_signed && signed_offset > 0))) {
                                        skip |=
                                            LogError(device, "VUID-RuntimeSpirv-OpImage-06377",
                                                     "vkCreateShaderModule(): Shader uses OpImageGather with offset (%" PRIu32
                                                     ") greater than VkPhysicalDeviceLimits::maxTexelGatherOffset (%" PRIu32 ").",
                                                     offset, phys_dev_props.limits.maxTexelGatherOffset);
                                    }
                                }
                            }
                        }
                        index += src->ImageOperandsCount(i);
                    }
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateShaderClock(SHADER_MODULE_STATE const *module, spirv_inst_iter &insn) const {
    bool skip = false;

    switch (insn.opcode()) {
        case spv::OpReadClockKHR: {
            auto scope_id = module->get_def(insn.word(3));
            auto scope_type = scope_id.word(3);
            // if scope isn't Subgroup or Device, spirv-val will catch
            if ((scope_type == spv::ScopeSubgroup) && (enabled_features.shader_clock_features.shaderSubgroupClock == VK_FALSE)) {
                skip |= LogError(device, "VUID-RuntimeSpirv-shaderSubgroupClock-06267",
                                 "%s: OpReadClockKHR is used with a Subgroup scope but shaderSubgroupClock was not enabled.",
                                 report_data->FormatHandle(module->vk_shader_module()).c_str());
            } else if ((scope_type == spv::ScopeDevice) && (enabled_features.shader_clock_features.shaderDeviceClock == VK_FALSE)) {
                skip |= LogError(device, "VUID-RuntimeSpirv-shaderDeviceClock-06268",
                                 "%s: OpReadClockKHR is used with a Device scope but shaderDeviceClock was not enabled.",
                                 report_data->FormatHandle(module->vk_shader_module()).c_str());
            }
            break;
        }
    }
    return skip;
}

bool CoreChecks::ValidatePipelineShaderStage(const PIPELINE_STATE *pipeline, const PipelineStageState &stage_state,
                                             bool check_point_size) const {
    bool skip = false;
    const auto *pStage = stage_state.create_info;
    const auto *module = stage_state.module.get();
    const auto &entrypoint = stage_state.entrypoint;
    // Check the module
    if (!module->has_valid_spirv) {
        skip |= LogError(
            device, "VUID-VkPipelineShaderStageCreateInfo-module-parameter", "%s does not contain valid spirv for stage %s.",
            report_data->FormatHandle(module->vk_shader_module()).c_str(), string_VkShaderStageFlagBits(stage_state.stage_flag));
    }

    // If specialization-constant values are given and specialization-constant instructions are present in the shader, the
    // specializations should be applied and validated.
    if (pStage->pSpecializationInfo != nullptr && pStage->pSpecializationInfo->mapEntryCount > 0 &&
        pStage->pSpecializationInfo->pMapEntries != nullptr && module->has_specialization_constants) {
        // Gather the specialization-constant values.
        auto const &specialization_info = pStage->pSpecializationInfo;
        auto const &specialization_data = reinterpret_cast<uint8_t const *>(specialization_info->pData);
        std::unordered_map<uint32_t, std::vector<uint32_t>> id_value_map;  // note: this must be std:: to work with spvtools
        id_value_map.reserve(specialization_info->mapEntryCount);
        for (auto i = 0u; i < specialization_info->mapEntryCount; ++i) {
            auto const &map_entry = specialization_info->pMapEntries[i];
            auto itr = module->spec_const_map.find(map_entry.constantID);
            // "If a constantID value is not a specialization constant ID used in the shader, that map entry does not affect the
            // behavior of the pipeline."
            if (itr != module->spec_const_map.cend()) {
                // Make sure map_entry.size matches the spec constant's size
                uint32_t spec_const_size = decoration_set::kInvalidValue;
                const auto def_ins = module->get_def(itr->second);
                const auto type_ins = module->get_def(def_ins.word(1));
                // Specialization constants can only be of type bool, scalar integer, or scalar floating point
                switch (type_ins.opcode()) {
                    case spv::OpTypeBool:
                        // "If the specialization constant is of type boolean, size must be the byte size of VkBool32"
                        spec_const_size = sizeof(VkBool32);
                        break;
                    case spv::OpTypeInt:
                    case spv::OpTypeFloat:
                        spec_const_size = type_ins.word(2) / 8;
                        break;
                    default:
                        // spirv-val should catch if SpecId is not used on a OpSpecConstantTrue/OpSpecConstantFalse/OpSpecConstant
                        // and OpSpecConstant is validated to be a OpTypeInt or OpTypeFloat
                        break;
                }

                if (map_entry.size != spec_const_size) {
                    skip |=
                        LogError(device, "VUID-VkSpecializationMapEntry-constantID-00776",
                                 "Specialization constant (ID = %" PRIu32 ", entry = %" PRIu32
                                 ") has invalid size %zu in shader module %s. Expected size is %" PRIu32 " from shader definition.",
                                 map_entry.constantID, i, map_entry.size,
                                 report_data->FormatHandle(module->vk_shader_module()).c_str(), spec_const_size);
                }
            }

            if ((map_entry.offset + map_entry.size) <= specialization_info->dataSize) {
                // Allocate enough room for ceil(map_entry.size / 4) to store entries
                std::vector<uint32_t> entry_data((map_entry.size + 4 - 1) / 4, 0);
                uint8_t *out_p = reinterpret_cast<uint8_t *>(entry_data.data());
                const uint8_t *const start_in_p = specialization_data + map_entry.offset;
                const uint8_t *const end_in_p = start_in_p + map_entry.size;

                std::copy(start_in_p, end_in_p, out_p);
                id_value_map.emplace(map_entry.constantID, std::move(entry_data));
            }
        }

        // Apply the specialization-constant values and revalidate the shader module.
        spv_target_env spirv_environment = PickSpirvEnv(api_version, IsExtEnabled(device_extensions.vk_khr_spirv_1_4));
        spvtools::Optimizer optimizer(spirv_environment);
        spvtools::MessageConsumer consumer = [&skip, &module, &stage_state, this](spv_message_level_t level, const char *source,
                                                                                  const spv_position_t &position,
                                                                                  const char *message) {
            skip |= LogError(device, "VUID-VkPipelineShaderStageCreateInfo-module-parameter",
                             "%s does not contain valid spirv for stage %s. %s",
                             report_data->FormatHandle(module->vk_shader_module()).c_str(),
                             string_VkShaderStageFlagBits(stage_state.stage_flag), message);
        };
        optimizer.SetMessageConsumer(consumer);
        optimizer.RegisterPass(spvtools::CreateSetSpecConstantDefaultValuePass(id_value_map));
        optimizer.RegisterPass(spvtools::CreateFreezeSpecConstantValuePass());
        std::vector<uint32_t> specialized_spirv;
        auto const optimized = optimizer.Run(module->words.data(), module->words.size(), &specialized_spirv);
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
                                 report_data->FormatHandle(module->vk_shader_module()).c_str(),
                                 string_VkShaderStageFlagBits(stage_state.stage_flag));
            }

            spvDiagnosticDestroy(diag);
            spvContextDestroy(ctx);
        }

        skip |= ValidateWorkgroupSize(module, pStage, id_value_map);
    }

    // Check the entrypoint
    if (entrypoint == module->end()) {
        skip |= LogError(device, "VUID-VkPipelineShaderStageCreateInfo-pName-00707", "No entrypoint found named `%s` for stage %s.",
                         pStage->pName, string_VkShaderStageFlagBits(stage_state.stage_flag));
    }
    if (skip) return true;  // no point continuing beyond here, any analysis is just going to be garbage.

    // Mark accessible ids
    auto &accessible_ids = stage_state.accessible_ids;

    // Validate descriptor set layout against what the entrypoint actually uses

    // The following tries to limit the number of passes through the shader module. The validation passes in here are "stateless"
    // and mainly only checking the instruction in detail for a single operation
    uint32_t total_shared_size = 0;
    for (auto insn : *module) {
        skip |= ValidateTransformFeedback(module, insn);
        skip |= ValidateTexelGatherOffset(module, insn);
        skip |= ValidateShaderCapabilitiesAndExtensions(module, insn);
        skip |= ValidateShaderClock(module, insn);
        skip |= ValidateShaderStageGroupNonUniform(module, pStage->stage, insn);
        total_shared_size += module->CalcComputeSharedMemory(pStage->stage, insn);
    }

    if (total_shared_size > phys_dev_props.limits.maxComputeSharedMemorySize) {
        skip |= LogError(device, kVUID_Core_Shader_MaxComputeSharedMemorySize,
                         "Shader uses %" PRIu32 " bytes of shared memory, more than allowed by physicalDeviceLimits::maxComputeSharedMemorySize (%" PRIu32 ")",
                         total_shared_size, phys_dev_props.limits.maxComputeSharedMemorySize);
    }

    skip |= ValidateShaderStageWritableOrAtomicDescriptor(pStage->stage, stage_state.has_writable_descriptor,
                                                          stage_state.has_atomic_descriptor);
    skip |= ValidateShaderStageInputOutputLimits(module, pStage, pipeline, entrypoint);
    skip |= ValidateShaderStorageImageFormats(module);
    skip |= ValidateShaderStageMaxResources(pStage->stage, pipeline);
    skip |= ValidateAtomicsTypes(module);
    skip |= ValidateExecutionModes(module, entrypoint);
    skip |= ValidateSpecializations(pStage);
    skip |= ValidateDecorations(module);
    if (check_point_size && !pipeline->create_info.graphics.pRasterizationState->rasterizerDiscardEnable) {
        skip |= ValidatePointListShaderState(pipeline, module, entrypoint, pStage->stage);
    }
    skip |= ValidateBuiltinLimits(module, entrypoint);
    if (enabled_features.cooperative_matrix_features.cooperativeMatrix) {
        skip |= ValidateCooperativeMatrix(module, pStage, pipeline);
    }
    if (enabled_features.fragment_shading_rate_features.primitiveFragmentShadingRate) {
        skip |= ValidatePrimitiveRateShaderState(pipeline, module, entrypoint, pStage->stage);
    }
    if (IsExtEnabled(device_extensions.vk_qcom_render_pass_shader_resolve)) {
        skip |= ValidateShaderResolveQCOM(module, pStage, pipeline);
    }
    if (IsExtEnabled(device_extensions.vk_ext_subgroup_size_control)) {
        skip |= ValidateShaderSubgroupSizeControl(pStage);
    }

    // "layout must be consistent with the layout of the * shader"
    // 'consistent' -> #descriptorsets-pipelinelayout-consistency
    std::string vuid_layout_mismatch;
    switch (pipeline->create_info.graphics.sType) {
        case VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO:
            vuid_layout_mismatch = "VUID-VkGraphicsPipelineCreateInfo-layout-00756";
            break;
        case VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO:
            vuid_layout_mismatch = "VUID-VkComputePipelineCreateInfo-layout-00703";
            break;
        case VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR:
            vuid_layout_mismatch = "VUID-VkRayTracingPipelineCreateInfoKHR-layout-03427";
            break;
        case VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV:
            vuid_layout_mismatch = "VUID-VkRayTracingPipelineCreateInfoNV-layout-03427";
            break;
        default:
            assert(false);
            break;
    }

    // Validate Push Constants use
    skip |= ValidatePushConstantUsage(*pipeline, module, pStage, vuid_layout_mismatch);

    // Validate descriptor use
    for (auto use : stage_state.descriptor_uses) {
        // Verify given pipelineLayout has requested setLayout with requested binding
        const auto &binding = GetDescriptorBinding(pipeline->pipeline_layout.get(), use.first);
        unsigned required_descriptor_count;
        bool is_khr = binding && binding->descriptorType == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        std::set<uint32_t> descriptor_types =
            TypeToDescriptorTypeSet(module, use.second.type_id, required_descriptor_count, is_khr);

        if (!binding) {
            skip |= LogError(device, vuid_layout_mismatch,
                             "Shader uses descriptor slot %u.%u (expected `%s`) but not declared in pipeline layout",
                             use.first.set, use.first.binding, string_descriptorTypes(descriptor_types).c_str());
        } else if (~binding->stageFlags & pStage->stage) {
            skip |= LogError(device, vuid_layout_mismatch,
                             "Shader uses descriptor slot %u.%u but descriptor not accessible from stage %s", use.first.set,
                             use.first.binding, string_VkShaderStageFlagBits(pStage->stage));
        } else if ((binding->descriptorType != VK_DESCRIPTOR_TYPE_MUTABLE_VALVE) &&
                   (descriptor_types.find(binding->descriptorType) == descriptor_types.end())) {
            skip |= LogError(device, vuid_layout_mismatch,
                             "Type mismatch on descriptor slot %u.%u (expected `%s`) but descriptor of type %s", use.first.set,
                             use.first.binding, string_descriptorTypes(descriptor_types).c_str(),
                             string_VkDescriptorType(binding->descriptorType));
        } else if (binding->descriptorCount < required_descriptor_count) {
            skip |= LogError(device, vuid_layout_mismatch,
                             "Shader expects at least %u descriptors for binding %u.%u but only %u provided",
                             required_descriptor_count, use.first.set, use.first.binding, binding->descriptorCount);
        }
    }

    // Validate use of input attachments against subpass structure
    if (pStage->stage == VK_SHADER_STAGE_FRAGMENT_BIT) {
        auto input_attachment_uses = module->CollectInterfaceByInputAttachmentIndex(accessible_ids);

        auto rpci = pipeline->rp_state->createInfo.ptr();
        auto subpass = pipeline->create_info.graphics.subpass;

        for (auto use : input_attachment_uses) {
            auto input_attachments = rpci->pSubpasses[subpass].pInputAttachments;
            auto index = (input_attachments && use.first < rpci->pSubpasses[subpass].inputAttachmentCount)
                             ? input_attachments[use.first].attachment
                             : VK_ATTACHMENT_UNUSED;

            if (index == VK_ATTACHMENT_UNUSED) {
                skip |= LogError(device, kVUID_Core_Shader_MissingInputAttachment,
                                 "Shader consumes input attachment index %d but not provided in subpass", use.first);
            } else if (!(GetFormatType(rpci->pAttachments[index].format) & module->GetFundamentalType(use.second.type_id))) {
                skip |=
                    LogError(device, kVUID_Core_Shader_InputAttachmentTypeMismatch,
                             "Subpass input attachment %u format of %s does not match type used in shader `%s`", use.first,
                             string_VkFormat(rpci->pAttachments[index].format), module->DescribeType(use.second.type_id).c_str());
            }
        }
    }
    if (pStage->stage == VK_SHADER_STAGE_COMPUTE_BIT) {
        skip |= ValidateComputeWorkGroupSizes(module, entrypoint, stage_state);
    }

    return skip;
}

bool CoreChecks::ValidateInterfaceBetweenStages(SHADER_MODULE_STATE const *producer, spirv_inst_iter producer_entrypoint,
                                                shader_stage_attributes const *producer_stage, SHADER_MODULE_STATE const *consumer,
                                                spirv_inst_iter consumer_entrypoint,
                                                shader_stage_attributes const *consumer_stage) const {
    bool skip = false;

    auto outputs =
        producer->CollectInterfaceByLocation(producer_entrypoint, spv::StorageClassOutput, producer_stage->arrayed_output);
    auto inputs = consumer->CollectInterfaceByLocation(consumer_entrypoint, spv::StorageClassInput, consumer_stage->arrayed_input);

    auto a_it = outputs.begin();
    auto b_it = inputs.begin();

    uint32_t a_component = 0;
    uint32_t b_component = 0;

    // Maps sorted by key (location); walk them together to find mismatches
    while ((outputs.size() > 0 && a_it != outputs.end()) || (inputs.size() && b_it != inputs.end())) {
        bool a_at_end = outputs.size() == 0 || a_it == outputs.end();
        bool b_at_end = inputs.size() == 0 || b_it == inputs.end();
        auto a_first = a_at_end ? std::make_pair(0u, 0u) : a_it->first;
        auto b_first = b_at_end ? std::make_pair(0u, 0u) : b_it->first;

        a_first.second += a_component;
        b_first.second += b_component;

        const auto a_length = a_at_end ? 0 : producer->GetNumComponentsInBaseType(producer->get_def(a_it->second.type_id));
        const auto b_length = b_at_end ? 0 : consumer->GetNumComponentsInBaseType(consumer->get_def(b_it->second.type_id));
        assert(a_at_end || a_component < a_length);
        assert(b_at_end || b_component < b_length);

        if (b_at_end || ((!a_at_end) && (a_first < b_first))) {
            skip |= LogPerformanceWarning(producer->vk_shader_module(), kVUID_Core_Shader_OutputNotConsumed,
                                          "%s writes to output location %" PRIu32 ".%" PRIu32 " which is not consumed by %s",
                                          producer_stage->name, a_first.first, a_first.second, consumer_stage->name);
            if ((b_first.first > a_first.first) || b_at_end || (a_component + 1 == a_length)) {
                a_it++;
                a_component = 0;
            } else {
                a_component++;
            }
        } else if (a_at_end || a_first > b_first) {
            skip |= LogError(consumer->vk_shader_module(), kVUID_Core_Shader_InputNotProduced,
                             "%s consumes input location %" PRIu32 ".%" PRIu32 " which is not written by %s", consumer_stage->name,
                             b_first.first, b_first.second, producer_stage->name);
            if ((a_first.first > b_first.first) || a_at_end || (b_component + 1 == b_length)) {
                b_it++;
                b_component = 0;
            } else {
                b_component++;
            }
        } else {
            // subtleties of arrayed interfaces:
            // - if is_patch, then the member is not arrayed, even though the interface may be.
            // - if is_block_member, then the extra array level of an arrayed interface is not
            //   expressed in the member type -- it's expressed in the block type.
            if (!TypesMatch(producer, consumer, a_it->second.type_id, b_it->second.type_id)) {
                skip |= LogError(producer->vk_shader_module(), kVUID_Core_Shader_InterfaceTypeMismatch,
                                 "Type mismatch on location %" PRIu32 ".%" PRIu32 ": '%s' vs '%s'", a_first.first, a_first.second,
                                 producer->DescribeType(a_it->second.type_id).c_str(),
                                 consumer->DescribeType(b_it->second.type_id).c_str());
                a_it++;
                b_it++;
                continue;
            }
            if (a_it->second.is_patch != b_it->second.is_patch) {
                skip |= LogError(producer->vk_shader_module(), kVUID_Core_Shader_InterfaceTypeMismatch,
                                 "Decoration mismatch on location %u.%u: is per-%s in %s stage but per-%s in %s stage",
                                 a_first.first, a_first.second, a_it->second.is_patch ? "patch" : "vertex", producer_stage->name,
                                 b_it->second.is_patch ? "patch" : "vertex", consumer_stage->name);
            }
            if (a_it->second.is_relaxed_precision != b_it->second.is_relaxed_precision) {
                skip |= LogError(producer->vk_shader_module(), kVUID_Core_Shader_InterfaceTypeMismatch,
                                 "Decoration mismatch on location %" PRIu32 ".%" PRIu32 ": %s and %s stages differ in precision",
                                 a_first.first, a_first.second, producer_stage->name, consumer_stage->name);
            }
            uint32_t a_remaining = a_length - a_component;
            uint32_t b_remaining = b_length - b_component;
            if (a_remaining == b_remaining) {  // Sizes match so we can advance both a_it and b_it
                a_it++;
                b_it++;
                a_component = 0;
                b_component = 0;
            } else if (a_remaining > b_remaining) {  // a has more components remaining
                a_component += b_remaining;
                b_component = 0;
                b_it++;
            } else if (b_remaining > a_remaining) {  // b has more components remaining
                b_component += a_remaining;
                a_component = 0;
                a_it++;
            }
        }
    }

    if (consumer_stage->stage != VK_SHADER_STAGE_FRAGMENT_BIT) {
        auto builtins_producer = producer->CollectBuiltinBlockMembers(producer_entrypoint, spv::StorageClassOutput);
        auto builtins_consumer = consumer->CollectBuiltinBlockMembers(consumer_entrypoint, spv::StorageClassInput);

        if (!builtins_producer.empty() && !builtins_consumer.empty()) {
            if (builtins_producer.size() != builtins_consumer.size()) {
                skip |= LogError(producer->vk_shader_module(), kVUID_Core_Shader_InterfaceTypeMismatch,
                                 "Number of elements inside builtin block differ between stages (%s %d vs %s %d).",
                                 producer_stage->name, static_cast<int>(builtins_producer.size()), consumer_stage->name,
                                 static_cast<int>(builtins_consumer.size()));
            } else {
                auto it_producer = builtins_producer.begin();
                auto it_consumer = builtins_consumer.begin();
                while (it_producer != builtins_producer.end() && it_consumer != builtins_consumer.end()) {
                    if (*it_producer != *it_consumer) {
                        skip |= LogError(producer->vk_shader_module(), kVUID_Core_Shader_InterfaceTypeMismatch,
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
    const auto create_info = pipeline->create_info.graphics.ptr();

    bool skip = false;

    uint32_t pointlist_stage_mask = DetermineFinalGeomStage(pipeline, create_info);

    const PipelineStageState *vertex_stage = nullptr, *fragment_stage = nullptr;
    for (auto &stage : pipeline->stage_state) {
        skip |= ValidatePipelineShaderStage(pipeline, stage, (pointlist_stage_mask == stage.stage_flag));
        if (stage.stage_flag == VK_SHADER_STAGE_VERTEX_BIT) {
            vertex_stage = &stage;
        }
        if (stage.stage_flag == VK_SHADER_STAGE_FRAGMENT_BIT) {
            fragment_stage = &stage;
        }
    }

    // if the shader stages are no good individually, cross-stage validation is pointless.
    if (skip) return true;

    auto vi = create_info->pVertexInputState;

    if (vi) {
        skip |= ValidateViConsistency(vi);
    }

    if (vertex_stage && vertex_stage->module->has_valid_spirv && !IsDynamic(pipeline, VK_DYNAMIC_STATE_VERTEX_INPUT_EXT)) {
        skip |= ValidateViAgainstVsInputs(vi, vertex_stage->module.get(), vertex_stage->entrypoint);
    }

    for (size_t i = 1; i < pipeline->stage_state.size(); i++) {
        const auto &producer = pipeline->stage_state[i - 1];
        const auto &consumer = pipeline->stage_state[i];
        assert(producer.module);
        if (&producer == fragment_stage) {
            break;
        }
        if (consumer.module) {
            if (consumer.module->has_valid_spirv && producer.module->has_valid_spirv) {
                auto producer_id = GetShaderStageId(producer.stage_flag);
                auto consumer_id = GetShaderStageId(consumer.stage_flag);
                skip |=
                    ValidateInterfaceBetweenStages(producer.module.get(), producer.entrypoint, &shader_stage_attribs[producer_id],
                                                   consumer.module.get(), consumer.entrypoint, &shader_stage_attribs[consumer_id]);
            }

        }
    }

    if (fragment_stage && fragment_stage->module->has_valid_spirv) {
        skip |= ValidateFsOutputsAgainstRenderPass(fragment_stage->module.get(), fragment_stage->entrypoint, pipeline,
                                                   create_info->subpass);
    }

    return skip;
}

void CoreChecks::RecordGraphicsPipelineShaderDynamicState(PIPELINE_STATE *pipeline_state) {
    if (phys_dev_ext_props.fragment_shading_rate_props.primitiveFragmentShadingRateWithMultipleViewports ||
        !IsDynamic(pipeline_state, VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT)) {
        return;
    }

    for (auto &stage : pipeline_state->stage_state) {
        if (stage.stage_flag == VK_SHADER_STAGE_VERTEX_BIT || stage.stage_flag == VK_SHADER_STAGE_GEOMETRY_BIT ||
            stage.stage_flag == VK_SHADER_STAGE_MESH_BIT_NV) {
            bool primitiverate_written = false;

            for (auto set : stage.module->builtin_decoration_list) {
                auto insn = stage.module->at(set.offset);
                if (set.builtin == spv::BuiltInPrimitiveShadingRateKHR) {
                    primitiverate_written = stage.module->IsBuiltInWritten(insn, stage.entrypoint);
                }
                if (primitiverate_written) {
                    break;
                }
            }

            if (primitiverate_written) {
                pipeline_state->wrote_primitive_shading_rate.insert(stage.stage_flag);
            }
        }
    }
}

bool CoreChecks::ValidateGraphicsPipelineShaderDynamicState(const PIPELINE_STATE *pipeline, const CMD_BUFFER_STATE *pCB,
                                                            const char *caller, const DrawDispatchVuid &vuid) const {
    bool skip = false;

    for (auto &stage : pipeline->stage_state) {
        if (stage.stage_flag == VK_SHADER_STAGE_VERTEX_BIT || stage.stage_flag == VK_SHADER_STAGE_GEOMETRY_BIT ||
            stage.stage_flag == VK_SHADER_STAGE_MESH_BIT_NV) {
            if (!phys_dev_ext_props.fragment_shading_rate_props.primitiveFragmentShadingRateWithMultipleViewports &&
                IsDynamic(pipeline, VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT) && pCB->viewportWithCountCount != 1) {
                if (pipeline->wrote_primitive_shading_rate.find(stage.stage_flag) != pipeline->wrote_primitive_shading_rate.end()) {
                    skip |=
                        LogError(pipeline->pipeline(), vuid.viewport_count_primitive_shading_rate,
                                 "%s: %s shader of currently bound pipeline statically writes to PrimitiveShadingRateKHR built-in"
                                 "but multiple viewports are set by the last call to vkCmdSetViewportWithCountEXT,"
                                 "and the primitiveFragmentShadingRateWithMultipleViewports limit is not supported.",
                                 caller, string_VkShaderStageFlagBits(stage.stage_flag));
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateComputePipelineShaderState(PIPELINE_STATE *pipeline) const {
    return ValidatePipelineShaderStage(pipeline, pipeline->stage_state[0], false);
}

uint32_t CoreChecks::CalcShaderStageCount(const PIPELINE_STATE *pipeline, VkShaderStageFlagBits stageBit) const {
    uint32_t total = 0;
    const auto &create_info = pipeline->create_info.raytracing;
    const auto *stages = create_info.ptr()->pStages;
    for (uint32_t stage_index = 0; stage_index < create_info.stageCount; stage_index++) {
        if (stages[stage_index].stage == stageBit) {
            total++;
        }
    }

    if (create_info.pLibraryInfo) {
        for (uint32_t i = 0; i < create_info.pLibraryInfo->libraryCount; ++i) {
            const PIPELINE_STATE *library_pipeline = GetPipelineState(create_info.pLibraryInfo->pLibraries[i]);
            total += CalcShaderStageCount(library_pipeline, stageBit);
        }
    }

    return total;
}

bool CoreChecks::GroupHasValidIndex(const PIPELINE_STATE *pipeline, uint32_t group, uint32_t stage) const {
    if (group == VK_SHADER_UNUSED_NV) {
        return true;
    }

    const auto &create_info = pipeline->create_info.raytracing;
    const auto *stages = create_info.ptr()->pStages;

    if (group < create_info.stageCount) {
        return (stages[group].stage & stage) != 0;
    }
    group -= create_info.stageCount;

    // Search libraries
    if (create_info.pLibraryInfo) {
        for (uint32_t i = 0; i < create_info.pLibraryInfo->libraryCount; ++i) {
            const PIPELINE_STATE *library_pipeline = GetPipelineState(create_info.pLibraryInfo->pLibraries[i]);
            const uint32_t stage_count = library_pipeline->create_info.raytracing.ptr()->stageCount;
            if (group < stage_count) {
                return (library_pipeline->create_info.raytracing.ptr()->pStages[group].stage & stage) != 0;
            }
            group -= stage_count;
        }
    }

    // group index too large
    return false;
}

bool CoreChecks::ValidateRayTracingPipeline(PIPELINE_STATE *pipeline, VkPipelineCreateFlags flags, bool isKHR) const {
    bool skip = false;

    const auto &create_info = pipeline->create_info.raytracing;
    if (isKHR) {
        if (create_info.maxPipelineRayRecursionDepth > phys_dev_ext_props.ray_tracing_propsKHR.maxRayRecursionDepth) {
            skip |=
                LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-maxPipelineRayRecursionDepth-03589",
                         "vkCreateRayTracingPipelinesKHR: maxPipelineRayRecursionDepth (%d ) must be less than or equal to "
                         "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxRayRecursionDepth %d",
                         create_info.maxPipelineRayRecursionDepth, phys_dev_ext_props.ray_tracing_propsKHR.maxRayRecursionDepth);
        }
        if (create_info.pLibraryInfo) {
            for (uint32_t i = 0; i < create_info.pLibraryInfo->libraryCount; ++i) {
                const PIPELINE_STATE *library_pipelinestate = GetPipelineState(create_info.pLibraryInfo->pLibraries[i]);
                const auto &library_create_info = library_pipelinestate->create_info.raytracing;
                if (library_create_info.maxPipelineRayRecursionDepth != create_info.maxPipelineRayRecursionDepth) {
                    skip |= LogError(
                        device, "VUID-VkRayTracingPipelineCreateInfoKHR-pLibraries-03591",
                        "vkCreateRayTracingPipelinesKHR: Each element  (%d) of the pLibraries member of libraries must have been"
                        "created with the value of maxPipelineRayRecursionDepth (%d) equal to that in this pipeline (%d) .",
                        i, library_create_info.maxPipelineRayRecursionDepth, create_info.maxPipelineRayRecursionDepth);
                }
                if (library_create_info.pLibraryInfo && (library_create_info.pLibraryInterface->maxPipelineRayHitAttributeSize !=
                                                             create_info.pLibraryInterface->maxPipelineRayHitAttributeSize ||
                                                         library_create_info.pLibraryInterface->maxPipelineRayPayloadSize !=
                                                             create_info.pLibraryInterface->maxPipelineRayPayloadSize)) {
                    skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-03593",
                                     "vkCreateRayTracingPipelinesKHR: If pLibraryInfo is not NULL, each element of its pLibraries "
                                     "member must have been created with values of the maxPipelineRayPayloadSize and "
                                     "maxPipelineRayHitAttributeSize members of pLibraryInterface equal to those in this pipeline");
                }
                if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR) &&
                    !(library_create_info.flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR)) {
                    skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03594",
                                     "vkCreateRayTracingPipelinesKHR: If flags includes "
                                     "VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR, each element of "
                                     "the pLibraries member of libraries must have been created with the "
                                     "VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR bit set");
                }
            }
        }
    } else {
        if (create_info.maxRecursionDepth > phys_dev_ext_props.ray_tracing_propsNV.maxRecursionDepth) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-maxRecursionDepth-03457",
                             "vkCreateRayTracingPipelinesNV: maxRecursionDepth (%d) must be less than or equal to "
                             "VkPhysicalDeviceRayTracingPropertiesNV::maxRecursionDepth (%d)",
                             create_info.maxRecursionDepth, phys_dev_ext_props.ray_tracing_propsNV.maxRecursionDepth);
        }
    }
    const auto *groups = create_info.ptr()->pGroups;

    for (uint32_t stage_index = 0; stage_index < create_info.stageCount; stage_index++) {
        skip |= ValidatePipelineShaderStage(pipeline, pipeline->stage_state[stage_index], false);
    }

    if ((create_info.flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) == 0) {
        const uint32_t raygen_stages_count = CalcShaderStageCount(pipeline, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
        if (raygen_stages_count == 0) {
            skip |= LogError(
                device,
                isKHR ? "VUID-VkRayTracingPipelineCreateInfoKHR-stage-03425" : "VUID-VkRayTracingPipelineCreateInfoNV-stage-06232",
                " : The stage member of at least one element of pStages must be VK_SHADER_STAGE_RAYGEN_BIT_KHR.");
        }
    }

    for (uint32_t group_index = 0; group_index < create_info.groupCount; group_index++) {
        const auto &group = groups[group_index];

        if (group.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV) {
            if (!GroupHasValidIndex(
                    pipeline, group.generalShader,
                    VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_MISS_BIT_NV | VK_SHADER_STAGE_CALLABLE_BIT_NV)) {
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
            if (!GroupHasValidIndex(pipeline, group.intersectionShader, VK_SHADER_STAGE_INTERSECTION_BIT_NV)) {
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
            if (!GroupHasValidIndex(pipeline, group.anyHitShader, VK_SHADER_STAGE_ANY_HIT_BIT_NV)) {
                skip |= LogError(device,
                                 isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-anyHitShader-03479"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-anyHitShader-02418",
                                 ": pGroups[%d]", group_index);
            }
            if (!GroupHasValidIndex(pipeline, group.closestHitShader, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV)) {
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
    const auto validation_cache_ci = LvlFindInChain<VkShaderModuleValidationCacheCreateInfoEXT>(pCreateInfo->pNext);
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

    auto have_glsl_shader = IsExtEnabled(device_extensions.vk_nv_glsl_shader);

    if (!have_glsl_shader && (pCreateInfo->codeSize % 4)) {
        skip |= LogError(device, "VUID-VkShaderModuleCreateInfo-pCode-01376",
                         "SPIR-V module not valid: Codesize must be a multiple of 4 but is " PRINTF_SIZE_T_SPECIFIER ".",
                         pCreateInfo->codeSize);
    } else {
        auto cache = GetValidationCacheInfo(pCreateInfo);
        uint32_t hash = 0;
        // If app isn't using a shader validation cache, use the default one from CoreChecks
        if (!cache) cache = CastFromHandle<ValidationCache *>(core_validation_cache);
        if (cache) {
            hash = ValidationCache::MakeShaderHash(pCreateInfo);
            if (cache->Contains(hash)) return false;
        }

        // Use SPIRV-Tools validator to try and catch any issues with the module itself. If specialization constants are present,
        // the default values will be used during validation.
        spv_target_env spirv_environment = PickSpirvEnv(api_version, IsExtEnabled(device_extensions.vk_khr_spirv_1_4));
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

bool CoreChecks::ValidateComputeWorkGroupSizes(const SHADER_MODULE_STATE *shader, const spirv_inst_iter &entrypoint,
                                               const PipelineStageState &stage_state) const {
    bool skip = false;
    uint32_t local_size_x = 0;
    uint32_t local_size_y = 0;
    uint32_t local_size_z = 0;
    if (shader->FindLocalSize(entrypoint, local_size_x, local_size_y, local_size_z)) {
        if (local_size_x > phys_dev_props.limits.maxComputeWorkGroupSize[0]) {
            skip |= LogError(shader->vk_shader_module(), "VUID-RuntimeSpirv-x-06429",
                             "%s local_size_x (%" PRIu32 ") exceeds device limit maxComputeWorkGroupSize[0] (%" PRIu32 ").",
                             report_data->FormatHandle(shader->vk_shader_module()).c_str(), local_size_x,
                             phys_dev_props.limits.maxComputeWorkGroupSize[0]);
        }
        if (local_size_y > phys_dev_props.limits.maxComputeWorkGroupSize[1]) {
            skip |= LogError(shader->vk_shader_module(), "VUID-RuntimeSpirv-y-06430",
                             "%s local_size_y (%" PRIu32 ") exceeds device limit maxComputeWorkGroupSize[1] (%" PRIu32 ").",
                             report_data->FormatHandle(shader->vk_shader_module()).c_str(), local_size_x,
                             phys_dev_props.limits.maxComputeWorkGroupSize[1]);
        }
        if (local_size_z > phys_dev_props.limits.maxComputeWorkGroupSize[2]) {
            skip |= LogError(shader->vk_shader_module(), "VUID-RuntimeSpirv-z-06431",
                             "%s local_size_z (%" PRIu32 ") exceeds device limit maxComputeWorkGroupSize[2] (%" PRIu32 ").",
                             report_data->FormatHandle(shader->vk_shader_module()).c_str(), local_size_x,
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
            skip |= LogError(shader->vk_shader_module(), "VUID-RuntimeSpirv-x-06432",
                             "%s local_size (%" PRIu32 ", %" PRIu32 ", %" PRIu32
                             ") exceeds device limit maxComputeWorkGroupInvocations (%" PRIu32 ").",
                             report_data->FormatHandle(shader->vk_shader_module()).c_str(), local_size_x, local_size_y,
                             local_size_z, limit);
        }

        const auto subgroup_flags = VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT |
                                    VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT;
        if ((stage_state.create_info->flags & subgroup_flags) == subgroup_flags) {
            if (SafeModulo(local_size_x, phys_dev_ext_props.subgroup_size_control_props.maxSubgroupSize) != 0) {
                skip |= LogError(
                    shader->vk_shader_module(), "VUID-VkPipelineShaderStageCreateInfo-flags-02758",
                    "%s flags contain VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT and "
                    "VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT bits, but local workgroup size in the X "
                    "dimension (%" PRIu32
                    ") is not a multiple of VkPhysicalDeviceSubgroupSizeControlPropertiesEXT::maxSubgroupSize (%" PRIu32 ").",
                    report_data->FormatHandle(shader->vk_shader_module()).c_str(), local_size_x,
                    phys_dev_ext_props.subgroup_size_control_props.maxSubgroupSize);
            }
        } else if ((stage_state.create_info->flags & VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT) &&
            (stage_state.create_info->flags & VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT) == 0) {
            const auto *required_subgroup_size_features =
                LvlFindInChain<VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT>(stage_state.create_info->pNext);
            if (!required_subgroup_size_features) {
                if (SafeModulo(local_size_x, phys_dev_props_core11.subgroupSize) != 0) {
                    skip |= LogError(
                        shader->vk_shader_module(), "VUID-VkPipelineShaderStageCreateInfo-flags-02759",
                        "%s flags contain VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT bit, and not the"
                        "VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT bit, but local workgroup size in the "
                        "X dimension (%" PRIu32 ") is not a multiple of VkPhysicalDeviceVulkan11Properties::subgroupSize (%" PRIu32
                        ").",
                        report_data->FormatHandle(shader->vk_shader_module()).c_str(), local_size_x,
                        phys_dev_props_core11.subgroupSize);
                }
            }
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

// Some Vulkan extensions/features are just all done in spirv-val behind optional settings
void AdjustValidatorOptions(const DeviceExtensions &device_extensions, const DeviceFeatures &enabled_features,
                            spvtools::ValidatorOptions &options) {
    // VK_KHR_relaxed_block_layout never had a feature bit so just enabling the extension allows relaxed layout
    // Was promotoed in Vulkan 1.1 so anyone using Vulkan 1.1 also gets this for free
    if (IsExtEnabled(device_extensions.vk_khr_relaxed_block_layout)) {
        // --relax-block-layout
        options.SetRelaxBlockLayout(true);
    }

    // The rest of the settings are controlled from a feature bit, which are set correctly in the state tracking. Regardless of
    // Vulkan version used, the feature bit is needed (also described in the spec).

    if (enabled_features.core12.uniformBufferStandardLayout == VK_TRUE) {
        // --uniform-buffer-standard-layout
        options.SetUniformBufferStandardLayout(true);
    }
    if (enabled_features.core12.scalarBlockLayout == VK_TRUE) {
        // --scalar-block-layout
        options.SetScalarBlockLayout(true);
    }
    if (enabled_features.workgroup_memory_explicit_layout_features.workgroupMemoryExplicitLayoutScalarBlockLayout) {
        // --workgroup-scalar-block-layout
        options.SetWorkgroupScalarBlockLayout(true);
    }
}
