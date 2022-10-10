/* Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (C) 2015-2022 Google Inc.
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
#include "spirv_grammar_helper.h"
#include "xxhash.h"

static shader_stage_attributes shader_stage_attribs[] = {
    {"vertex shader", false, false, VK_SHADER_STAGE_VERTEX_BIT},
    {"tessellation control shader", true, true, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT},
    {"tessellation evaluation shader", true, false, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT},
    {"geometry shader", true, false, VK_SHADER_STAGE_GEOMETRY_BIT},
    {"fragment shader", false, false, VK_SHADER_STAGE_FRAGMENT_BIT},
};

static const spirv_inst_iter GetBaseTypeIter(const SHADER_MODULE_STATE &module_state, uint32_t type) {
    const auto &insn = module_state.get_def(type);
    const uint32_t base_insn_id = module_state.GetBaseType(insn);
    // Will return end() if an invalid/unknown base_insn_id is returned
    return module_state.get_def(base_insn_id);
}

static bool BaseTypesMatch(const SHADER_MODULE_STATE &a, const SHADER_MODULE_STATE &b, const spirv_inst_iter &a_base_insn,
                           const spirv_inst_iter &b_base_insn) {
    if (a_base_insn == a.end() || b_base_insn == b.end()) {
        return false;
    }
    const uint32_t a_opcode = a_base_insn.opcode();
    const uint32_t b_opcode = b_base_insn.opcode();
    if (a_opcode == b_opcode) {
        if (a_opcode == spv::OpTypeInt) {
            // Match width and signedness
            return a_base_insn.word(2) == b_base_insn.word(2) && a_base_insn.word(3) == b_base_insn.word(3);
        } else if (a_opcode == spv::OpTypeFloat) {
            // Match width
            return a_base_insn.word(2) == b_base_insn.word(2);
        } else if (a_opcode == spv::OpTypeBool) {
            return true;
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

static bool TypesMatch(const SHADER_MODULE_STATE &a, const SHADER_MODULE_STATE &b, uint32_t a_type, uint32_t b_type) {
    const auto &a_base_insn = GetBaseTypeIter(a, a_type);
    const auto &b_base_insn = GetBaseTypeIter(b, b_type);

    return BaseTypesMatch(a, b, a_base_insn, b_base_insn);
}

static uint32_t GetLocationsConsumedByFormat(VkFormat format) {
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

static uint32_t GetFormatType(VkFormat fmt) {
    if (FormatIsSINT(fmt)) return FORMAT_TYPE_SINT;
    if (FormatIsUINT(fmt)) return FORMAT_TYPE_UINT;
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

bool CoreChecks::ValidateViConsistency(safe_VkPipelineVertexInputStateCreateInfo const *vi) const {
    // Walk the binding descriptions, which describe the step rate and stride of each vertex buffer.  Each binding should
    // be specified only once.
    layer_data::unordered_map<uint32_t, VkVertexInputBindingDescription const *> bindings;
    bool skip = false;

    for (uint32_t i = 0; i < vi->vertexBindingDescriptionCount; i++) {
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

bool CoreChecks::ValidateViAgainstVsInputs(safe_VkPipelineVertexInputStateCreateInfo const *vi,
                                           const SHADER_MODULE_STATE &module_state, spirv_inst_iter entrypoint) const {
    bool skip = false;

    const auto inputs = module_state.CollectInterfaceByLocation(entrypoint, spv::StorageClassInput, false);

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
            skip |= LogPerformanceWarning(module_state.vk_shader_module(), kVUID_Core_Shader_OutputNotConsumed,
                                          "Vertex attribute at location %" PRIu32 " not consumed by vertex shader", location);
        } else if (!attrib && input) {
            skip |= LogError(module_state.vk_shader_module(), kVUID_Core_Shader_InputNotProduced,
                             "Vertex shader consumes input at location %" PRIu32 " but not provided", location);
        } else if (attrib && input) {
            const auto attrib_type = GetFormatType(attrib->format);
            const auto input_type = module_state.GetFundamentalType(input->type_id);

            // Type checking
            if (!(attrib_type & input_type)) {
                skip |= LogError(module_state.vk_shader_module(), kVUID_Core_Shader_InterfaceTypeMismatch,
                                 "Attribute type of `%s` at location %" PRIu32 " does not match vertex shader input type of `%s`",
                                 string_VkFormat(attrib->format), location, module_state.DescribeType(input->type_id).c_str());
            }
        } else {            // !attrib && !input
            assert(false);  // at least one exists in the map
        }
    }

    return skip;
}

bool CoreChecks::ValidateFsOutputsAgainstDynamicRenderingRenderPass(const SHADER_MODULE_STATE &module_state,
                                                                    spirv_inst_iter entrypoint,
                                                                    PIPELINE_STATE const *pipeline) const {
    bool skip = false;

    struct Attachment {
        const interface_var* output = nullptr;
    };
    std::map<uint32_t, Attachment> location_map;

    // TODO: dual source blend index (spv::DecIndex, zero if not provided)
    const auto outputs = module_state.CollectInterfaceByLocation(entrypoint, spv::StorageClassOutput, false);
    for (const auto& output_it : outputs) {
        auto const location = output_it.first.first;
        location_map[location].output = &output_it.second;
    }

    const auto ms_state = pipeline->MultisampleState();
    const bool alpha_to_coverage_enabled = ms_state && (ms_state->alphaToCoverageEnable == VK_TRUE);

    for (uint32_t location = 0; location < location_map.size(); ++location) {
         const auto output = location_map[location].output;

         const auto &rp_state = pipeline->RenderPassState();
         const auto &attachments = pipeline->Attachments();
         if (!output && location < attachments.size() && attachments[location].colorWriteMask != 0) {
             skip |= LogWarning(
                 module_state.vk_shader_module(), kVUID_Core_Shader_InputNotProduced,
                 "Attachment %" PRIu32 " not written by fragment shader; undefined values will be written to attachment", location);
         } else if (output && (location < rp_state->dynamic_rendering_pipeline_create_info.colorAttachmentCount)) {
             auto format = rp_state->dynamic_rendering_pipeline_create_info.pColorAttachmentFormats[location];
             const auto attachment_type = GetFormatType(format);
             const auto output_type = module_state.GetFundamentalType(output->type_id);

             // Type checking
             if (!(output_type & attachment_type)) {
                 skip |=
                     LogWarning(module_state.vk_shader_module(), kVUID_Core_Shader_InterfaceTypeMismatch,
                                "Attachment %" PRIu32
                                " of type `%s` does not match fragment shader output type of `%s`; resulting values are undefined",
                                location, string_VkFormat(format), module_state.DescribeType(output->type_id).c_str());
             }
         }
    }

    const auto output_zero = location_map.count(0) ? location_map[0].output : nullptr;
    bool location_zero_has_alpha = output_zero && module_state.get_def(output_zero->type_id) != module_state.end() &&
                                   module_state.GetComponentsConsumedByType(output_zero->type_id, false) == 4;
    if (alpha_to_coverage_enabled && !location_zero_has_alpha) {
        skip |= LogError(module_state.vk_shader_module(), kVUID_Core_Shader_NoAlphaAtLocation0WithAlphaToCoverage,
                         "fragment shader doesn't declare alpha output at location 0 even though alpha to coverage is enabled.");
    }

    return skip;
}

bool CoreChecks::ValidateFsOutputsAgainstRenderPass(const SHADER_MODULE_STATE &module_state, spirv_inst_iter entrypoint,
                                                    PIPELINE_STATE const *pipeline, uint32_t subpass_index) const {
    bool skip = false;

    struct Attachment {
        const VkAttachmentReference2 *reference = nullptr;
        const VkAttachmentDescription2 *attachment = nullptr;
        const interface_var *output = nullptr;
    };
    std::map<uint32_t, Attachment> location_map;

    const auto &rp_state = pipeline->RenderPassState();
    if (rp_state && !rp_state->UsesDynamicRendering()) {
        const auto rpci = rp_state->createInfo.ptr();
        const auto subpass = rpci->pSubpasses[subpass_index];
        for (uint32_t i = 0; i < subpass.colorAttachmentCount; ++i) {
            auto const &reference = subpass.pColorAttachments[i];
            location_map[i].reference = &reference;
            if (reference.attachment != VK_ATTACHMENT_UNUSED &&
                rpci->pAttachments[reference.attachment].format != VK_FORMAT_UNDEFINED) {
                location_map[i].attachment = &rpci->pAttachments[reference.attachment];
            }
        }
    }

    // TODO: dual source blend index (spv::DecIndex, zero if not provided)

    const auto outputs = module_state.CollectInterfaceByLocation(entrypoint, spv::StorageClassOutput, false);
    for (const auto &output_it : outputs) {
        auto const location = output_it.first.first;
        location_map[location].output = &output_it.second;
    }

    const auto *ms_state = pipeline->MultisampleState();
    const bool alpha_to_coverage_enabled = ms_state && (ms_state->alphaToCoverageEnable == VK_TRUE);

    // Don't check any color attachments if rasterization is disabled
    const auto raster_state = pipeline->RasterizationState();
    if (raster_state && !raster_state->rasterizerDiscardEnable) {
        for (const auto &location_it : location_map) {
            const auto reference = location_it.second.reference;
            if (reference != nullptr && reference->attachment == VK_ATTACHMENT_UNUSED) {
                continue;
            }

            const auto location = location_it.first;
            const auto attachment = location_it.second.attachment;
            const auto output = location_it.second.output;
            if (attachment && !output) {
                const auto &attachments = pipeline->Attachments();
                if (location < attachments.size() && attachments[location].colorWriteMask != 0) {
                    skip |= LogWarning(module_state.vk_shader_module(), kVUID_Core_Shader_InputNotProduced,
                                       "Attachment %" PRIu32
                                       " not written by fragment shader; undefined values will be written to attachment",
                                       location);
                }
            } else if (!attachment && output) {
                if (!(alpha_to_coverage_enabled && location == 0)) {
                    skip |=
                        LogWarning(module_state.vk_shader_module(), kVUID_Core_Shader_OutputNotConsumed,
                                   "fragment shader writes to output location %" PRIu32 " with no matching attachment", location);
                }
            } else if (attachment && output) {
                const auto attachment_type = GetFormatType(attachment->format);
                const auto output_type = module_state.GetFundamentalType(output->type_id);

                // Type checking
                if (!(output_type & attachment_type)) {
                    skip |= LogWarning(
                        module_state.vk_shader_module(), kVUID_Core_Shader_InterfaceTypeMismatch,
                        "Attachment %" PRIu32
                        " of type `%s` does not match fragment shader output type of `%s`; resulting values are undefined",
                        location, string_VkFormat(attachment->format), module_state.DescribeType(output->type_id).c_str());
                }
            } else {            // !attachment && !output
                assert(false);  // at least one exists in the map
            }
        }
    }

    const auto output_zero = location_map.count(0) ? location_map[0].output : nullptr;
    bool location_zero_has_alpha = output_zero && module_state.get_def(output_zero->type_id) != module_state.end() &&
                                   module_state.GetComponentsConsumedByType(output_zero->type_id, false) == 4;
    if (alpha_to_coverage_enabled && !location_zero_has_alpha) {
        skip |= LogError(module_state.vk_shader_module(), kVUID_Core_Shader_NoAlphaAtLocation0WithAlphaToCoverage,
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

bool CoreChecks::ValidatePushConstantUsage(const PIPELINE_STATE &pipeline, const SHADER_MODULE_STATE &module_state,
                                           safe_VkPipelineShaderStageCreateInfo const *pStage, const std::string &vuid) const {
    bool skip = false;
    // Temp workaround to prevent false positive errors
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/2450
    if (module_state.HasMultipleEntryPoints()) {
        return skip;
    }

    // Validate directly off the offsets. this isn't quite correct for arrays and matrices, but is a good first step.
    const auto *entrypoint = module_state.FindEntrypointStruct(pStage->pName, pStage->stage);
    if (!entrypoint || !entrypoint->push_constant_used_in_shader.IsUsed()) {
        return skip;
    }
    const auto &pipeline_layout = pipeline.PipelineLayoutState();
    std::vector<VkPushConstantRange> const *push_constant_ranges = pipeline_layout->push_constant_ranges.get();

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
                LogObjectList objlist(module_state.vk_shader_module());
                objlist.add(pipeline_layout->layout());
                skip |= LogError(objlist, vuid, "Push constant buffer:%s in %s is out of range in %s.", loc_descr.c_str(),
                                 string_VkShaderStageFlags(pStage->stage).c_str(),
                                 report_data->FormatHandle(pipeline_layout->layout()).c_str());
                break;
            }
        }
    }

    if (!found_stage) {
        LogObjectList objlist(module_state.vk_shader_module());
        objlist.add(pipeline_layout->layout());
        skip |= LogError(
            objlist, vuid, "Push constant is used in %s of %s. But %s doesn't set %s.",
            string_VkShaderStageFlags(pStage->stage).c_str(), report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
            report_data->FormatHandle(pipeline_layout->layout()).c_str(), string_VkShaderStageFlags(pStage->stage).c_str());
    }
    return skip;
}

bool CoreChecks::ValidateBuiltinLimits(const SHADER_MODULE_STATE &module_state, spirv_inst_iter entrypoint) const {
    bool skip = false;

    // Currently all builtin tested are only found in fragment shaders
    if (entrypoint.word(1) != spv::ExecutionModelFragment) {
        return skip;
    }

    // Find all builtin from just the interface variables
    for (uint32_t id : FindEntrypointInterfaces(entrypoint)) {
        auto insn = module_state.get_def(id);
        assert(insn.opcode() == spv::OpVariable);
        const decoration_set decorations = module_state.get_decorations(insn.word(2));

        // Currently don't need to search in structs
        if (((decorations.flags & decoration_set::builtin_bit) != 0) && (decorations.builtin == spv::BuiltInSampleMask)) {
            auto type_pointer = module_state.get_def(insn.word(1));
            assert(type_pointer.opcode() == spv::OpTypePointer);

            auto type = module_state.get_def(type_pointer.word(3));
            if (type.opcode() == spv::OpTypeArray) {
                uint32_t length = static_cast<uint32_t>(module_state.GetConstantValueById(type.word(3)));
                // Handles both the input and output sampleMask
                if (length > phys_dev_props.limits.maxSampleMaskWords) {
                    skip |=
                        LogError(module_state.vk_shader_module(), "VUID-VkPipelineShaderStageCreateInfo-maxSampleMaskWords-00711",
                                 "vkCreateGraphicsPipelines(): The BuiltIns SampleMask array sizes is %u which exceeds "
                                 "maxSampleMaskWords of %u in %s.",
                                 length, phys_dev_props.limits.maxSampleMaskWords,
                                 report_data->FormatHandle(module_state.vk_shader_module()).c_str());
                }
                break;
            }
        }
    }

    return skip;
}

// Validate that data for each specialization entry is fully contained within the buffer.
bool CoreChecks::ValidateSpecializations(const SHADER_MODULE_STATE &module_state,
                                         safe_VkPipelineShaderStageCreateInfo const *info) const {
    bool skip = false;

    const auto *spec = info->pSpecializationInfo;

    if (spec) {
        for (auto i = 0u; i < spec->mapEntryCount; i++) {
            if (spec->pMapEntries[i].offset >= spec->dataSize) {
                skip |= LogError(module_state.vk_shader_module(), "VUID-VkSpecializationInfo-offset-00773",
                                 "Specialization entry %u (for constant id %u) references memory outside provided specialization "
                                 "data (bytes %u..%zu; %zu bytes provided).",
                                 i, spec->pMapEntries[i].constantID, spec->pMapEntries[i].offset,
                                 spec->pMapEntries[i].offset + spec->dataSize - 1, spec->dataSize);

                continue;
            }
            if (spec->pMapEntries[i].offset + spec->pMapEntries[i].size > spec->dataSize) {
                skip |= LogError(module_state.vk_shader_module(), "VUID-VkSpecializationInfo-pMapEntries-00774",
                                 "Specialization entry %u (for constant id %u) references memory outside provided specialization "
                                 "data (bytes %u..%zu; %zu bytes provided).",
                                 i, spec->pMapEntries[i].constantID, spec->pMapEntries[i].offset,
                                 spec->pMapEntries[i].offset + spec->pMapEntries[i].size - 1, spec->dataSize);
            }
            for (uint32_t j = i + 1; j < spec->mapEntryCount; ++j) {
                if (spec->pMapEntries[i].constantID == spec->pMapEntries[j].constantID) {
                    skip |= LogError(module_state.vk_shader_module(), "VUID-VkSpecializationInfo-constantID-04911",
                                     "Specialization entry %" PRIu32 " and %" PRIu32 " have the same constantID (%" PRIu32 ").", i,
                                     j, spec->pMapEntries[i].constantID);
                }
            }
        }
    }

    return skip;
}

// TODO (jbolz): Can this return a const reference?
static std::set<uint32_t> TypeToDescriptorTypeSet(const SHADER_MODULE_STATE &module_state, uint32_t type_id,
                                                  uint32_t &descriptor_count, bool is_khr) {
    auto type = module_state.get_def(type_id);
    bool is_storage_buffer = false;
    descriptor_count = 1;
    std::set<uint32_t> ret;

    // Strip off any array or ptrs. Where we remove array levels, adjust the  descriptor count for each dimension.
    while (type.opcode() == spv::OpTypeArray || type.opcode() == spv::OpTypePointer || type.opcode() == spv::OpTypeRuntimeArray) {
        if (type.opcode() == spv::OpTypeRuntimeArray) {
            descriptor_count = 0;
            type = module_state.get_def(type.word(2));
        } else if (type.opcode() == spv::OpTypeArray) {
            descriptor_count *= module_state.GetConstantValueById(type.word(3));
            type = module_state.get_def(type.word(2));
        } else {
            if (type.word(2) == spv::StorageClassStorageBuffer) {
                is_storage_buffer = true;
            }
            type = module_state.get_def(type.word(3));
        }
    }

    switch (type.opcode()) {
        case spv::OpTypeStruct: {
            for (const auto insn : module_state.GetDecorationInstructions()) {
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
            auto image_type = module_state.get_def(type.word(2));
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

bool CoreChecks::RequirePropertyFlag(const SHADER_MODULE_STATE &module_state, VkBool32 check, char const *flag,
                                     char const *structure, const char *vuid) const {
    if (!check) {
        if (LogError(module_state.vk_shader_module(), vuid, "Shader requires flag %s set in %s but it is not set on the device",
                     flag, structure)) {
            return true;
        }
    }

    return false;
}

bool CoreChecks::RequireFeature(const SHADER_MODULE_STATE &module_state, VkBool32 feature, char const *feature_name,
                                const char *vuid) const {
    if (!feature) {
        if (LogError(module_state.vk_shader_module(), vuid, "Shader requires %s but is not enabled on the device", feature_name)) {
            return true;
        }
    }

    return false;
}

bool CoreChecks::ValidateShaderStageWritableOrAtomicDescriptor(const SHADER_MODULE_STATE &module_state, VkShaderStageFlagBits stage,
                                                               bool has_writable_descriptor, bool has_atomic_descriptor) const {
    bool skip = false;

    if (has_writable_descriptor || has_atomic_descriptor) {
        switch (stage) {
            case VK_SHADER_STAGE_FRAGMENT_BIT:
                skip |= RequireFeature(module_state, enabled_features.core.fragmentStoresAndAtomics, "fragmentStoresAndAtomics",
                                       "VUID-RuntimeSpirv-NonWritable-06340");
                break;
            case VK_SHADER_STAGE_VERTEX_BIT:
            case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
            case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            case VK_SHADER_STAGE_GEOMETRY_BIT:
                skip |= RequireFeature(module_state, enabled_features.core.vertexPipelineStoresAndAtomics,
                                       "vertexPipelineStoresAndAtomics", "VUID-RuntimeSpirv-NonWritable-06341");
                break;
            default:
                // No feature requirements for writes and atomics for other stages
                break;
        }
    }

    return skip;
}

bool CoreChecks::ValidateShaderStageGroupNonUniform(const SHADER_MODULE_STATE &module_state, VkShaderStageFlagBits stage,
                                                    spirv_inst_iter &insn) const {
    bool skip = false;

    // Check anything using a group operation (which currently is only OpGroupNonUnifrom* operations)
    if (GroupOperation(insn.opcode()) == true) {
        // Check the quad operations.
        if ((insn.opcode() == spv::OpGroupNonUniformQuadBroadcast) || (insn.opcode() == spv::OpGroupNonUniformQuadSwap)) {
            if ((stage != VK_SHADER_STAGE_FRAGMENT_BIT) && (stage != VK_SHADER_STAGE_COMPUTE_BIT)) {
                skip |=
                    RequireFeature(module_state, phys_dev_props_core11.subgroupQuadOperationsInAllStages,
                                   "VkPhysicalDeviceSubgroupProperties::quadOperationsInAllStages", "VUID-RuntimeSpirv-None-06342");
            }
        }

        uint32_t scope_type = spv::ScopeMax;
        if (insn.opcode() == spv::OpGroupNonUniformPartitionNV) {
            // OpGroupNonUniformPartitionNV always assumed subgroup as missing operand
            scope_type = spv::ScopeSubgroup;
        } else {
            // "All <id> used for Scope <id> must be of an OpConstant"
            auto scope_id = module_state.get_def(insn.word(3));
            scope_type = scope_id.word(3);
        }

        if (scope_type == spv::ScopeSubgroup) {
            // "Group operations with subgroup scope" must have stage support
            const VkSubgroupFeatureFlags supported_stages = phys_dev_props_core11.subgroupSupportedStages;
            skip |= RequirePropertyFlag(module_state, supported_stages & stage, string_VkShaderStageFlagBits(stage),
                                        "VkPhysicalDeviceSubgroupProperties::supportedStages", "VUID-RuntimeSpirv-None-06343");
        }

        if (!enabled_features.core12.shaderSubgroupExtendedTypes) {
            auto type = module_state.get_def(insn.word(1));

            if (type.opcode() == spv::OpTypeVector) {
                // Get the element type
                type = module_state.get_def(type.word(2));
            }

            if (type.opcode() != spv::OpTypeBool) {
                // Both OpTypeInt and OpTypeFloat the width is in the 2nd word.
                const uint32_t width = type.word(2);

                if ((type.opcode() == spv::OpTypeFloat && width == 16) ||
                    (type.opcode() == spv::OpTypeInt && (width == 8 || width == 16 || width == 64))) {
                    skip |= RequireFeature(module_state, enabled_features.core12.shaderSubgroupExtendedTypes,
                                           "VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures::shaderSubgroupExtendedTypes",
                                           "VUID-RuntimeSpirv-None-06275");
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateMemoryScope(const SHADER_MODULE_STATE &module_state, const spirv_inst_iter &insn) const {
    bool skip = false;

    const auto &entry = OpcodeMemoryScopePosition(insn.opcode());
    if (entry > 0) {
        const uint32_t scope_id = insn.word(entry);
        const auto &scope_def = module_state.GetConstantDef(scope_id);
        if (scope_def != module_state.end()) {
            const auto scope_type = module_state.GetConstantValue(scope_def);
            if (enabled_features.core12.vulkanMemoryModel && !enabled_features.core12.vulkanMemoryModelDeviceScope &&
                scope_type == spv::Scope::ScopeDevice) {
                skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-vulkanMemoryModel-06265",
                                 "VkPhysicalDeviceVulkan12Features::vulkanMemoryModel is enabled and "
                                 "VkPhysicalDeviceVulkan12Features::vulkanMemoryModelDeviceScope is disabled, but\n%s\nuses "
                                 "Device memory scope.",
                                 module_state.DescribeInstruction(insn).c_str());
            } else if (!enabled_features.core12.vulkanMemoryModel && scope_type == spv::Scope::ScopeQueueFamily) {
                skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-vulkanMemoryModel-06266",
                                 "VkPhysicalDeviceVulkan12Features::vulkanMemoryModel is not enabled, but\n%s\nuses "
                                 "QueueFamily memory scope.",
                                 module_state.DescribeInstruction(insn).c_str());
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateShaderStageInputOutputLimits(const SHADER_MODULE_STATE &module_state,
                                                      safe_VkPipelineShaderStageCreateInfo const *pStage,
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

    for (auto insn : module_state) {
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
            case spv::OpExecutionModeId:
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

    auto inputs = module_state.CollectInterfaceByLocation(entrypoint, spv::StorageClassInput, strip_input_array_level);
    auto outputs = module_state.CollectInterfaceByLocation(entrypoint, spv::StorageClassOutput, strip_output_array_level);

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

        int num_components = module_state.GetComponentsConsumedByType(iv.type_id, strip_input_array_level);
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

        int num_components = module_state.GetComponentsConsumedByType(iv.type_id, strip_output_array_level);
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
            num_comp_in += module_state.GetComponentsConsumedByType(var.baseTypePtrID, strip_input_array_level && !is_patch);
        } else {  // var.storageClass == spv::StorageClassOutput
            num_comp_out += module_state.GetComponentsConsumedByType(var.baseTypePtrID, strip_output_array_level && !is_patch);
        }
    }

    switch (pStage->stage) {
        case VK_SHADER_STAGE_VERTEX_BIT:
            if (num_comp_out > limits.maxVertexOutputComponents) {
                skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-Location-06272",
                                 "Invalid Pipeline CreateInfo State: Vertex shader exceeds "
                                 "VkPhysicalDeviceLimits::maxVertexOutputComponents of %u "
                                 "components by %u components",
                                 limits.maxVertexOutputComponents, num_comp_out - limits.maxVertexOutputComponents);
            }
            if (max_comp_out > static_cast<int>(limits.maxVertexOutputComponents)) {
                skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-Location-06272",
                                 "Invalid Pipeline CreateInfo State: Vertex shader output variable uses location that "
                                 "exceeds component limit VkPhysicalDeviceLimits::maxVertexOutputComponents (%u)",
                                 limits.maxVertexOutputComponents);
            }
            break;

        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
            if (num_comp_in > limits.maxTessellationControlPerVertexInputComponents) {
                skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-Location-06272",
                                 "Invalid Pipeline CreateInfo State: Tessellation control shader exceeds "
                                 "VkPhysicalDeviceLimits::maxTessellationControlPerVertexInputComponents of %u "
                                 "components by %u components",
                                 limits.maxTessellationControlPerVertexInputComponents,
                                 num_comp_in - limits.maxTessellationControlPerVertexInputComponents);
            }
            if (max_comp_in > static_cast<int>(limits.maxTessellationControlPerVertexInputComponents)) {
                skip |=
                    LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-Location-06272",
                             "Invalid Pipeline CreateInfo State: Tessellation control shader input variable uses location that "
                             "exceeds component limit VkPhysicalDeviceLimits::maxTessellationControlPerVertexInputComponents (%u)",
                             limits.maxTessellationControlPerVertexInputComponents);
            }
            if (num_comp_out > limits.maxTessellationControlPerVertexOutputComponents) {
                skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-Location-06272",
                                 "Invalid Pipeline CreateInfo State: Tessellation control shader exceeds "
                                 "VkPhysicalDeviceLimits::maxTessellationControlPerVertexOutputComponents of %u "
                                 "components by %u components",
                                 limits.maxTessellationControlPerVertexOutputComponents,
                                 num_comp_out - limits.maxTessellationControlPerVertexOutputComponents);
            }
            if (max_comp_out > static_cast<int>(limits.maxTessellationControlPerVertexOutputComponents)) {
                skip |=
                    LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-Location-06272",
                             "Invalid Pipeline CreateInfo State: Tessellation control shader output variable uses location that "
                             "exceeds component limit VkPhysicalDeviceLimits::maxTessellationControlPerVertexOutputComponents (%u)",
                             limits.maxTessellationControlPerVertexOutputComponents);
            }
            break;

        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            if (num_comp_in > limits.maxTessellationEvaluationInputComponents) {
                skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-Location-06272",
                                 "Invalid Pipeline CreateInfo State: Tessellation evaluation shader exceeds "
                                 "VkPhysicalDeviceLimits::maxTessellationEvaluationInputComponents of %u "
                                 "components by %u components",
                                 limits.maxTessellationEvaluationInputComponents,
                                 num_comp_in - limits.maxTessellationEvaluationInputComponents);
            }
            if (max_comp_in > static_cast<int>(limits.maxTessellationEvaluationInputComponents)) {
                skip |=
                    LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-Location-06272",
                             "Invalid Pipeline CreateInfo State: Tessellation evaluation shader input variable uses location that "
                             "exceeds component limit VkPhysicalDeviceLimits::maxTessellationEvaluationInputComponents (%u)",
                             limits.maxTessellationEvaluationInputComponents);
            }
            if (num_comp_out > limits.maxTessellationEvaluationOutputComponents) {
                skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-Location-06272",
                                 "Invalid Pipeline CreateInfo State: Tessellation evaluation shader exceeds "
                                 "VkPhysicalDeviceLimits::maxTessellationEvaluationOutputComponents of %u "
                                 "components by %u components",
                                 limits.maxTessellationEvaluationOutputComponents,
                                 num_comp_out - limits.maxTessellationEvaluationOutputComponents);
            }
            if (max_comp_out > static_cast<int>(limits.maxTessellationEvaluationOutputComponents)) {
                skip |=
                    LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-Location-06272",
                             "Invalid Pipeline CreateInfo State: Tessellation evaluation shader output variable uses location that "
                             "exceeds component limit VkPhysicalDeviceLimits::maxTessellationEvaluationOutputComponents (%u)",
                             limits.maxTessellationEvaluationOutputComponents);
            }
            // Portability validation
            if (IsExtEnabled(device_extensions.vk_khr_portability_subset)) {
                if (is_iso_lines && (VK_FALSE == enabled_features.portability_subset_features.tessellationIsolines)) {
                    skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-tessellationShader-06326",
                                     "Invalid Pipeline CreateInfo state (portability error): Tessellation evaluation shader"
                                     " is using abstract patch type IsoLines, but this is not supported on this platform");
                }
                if (is_point_mode && (VK_FALSE == enabled_features.portability_subset_features.tessellationPointMode)) {
                    skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-tessellationShader-06327",
                                     "Invalid Pipeline CreateInfo state (portability error): Tessellation evaluation shader"
                                     " is using abstract patch type PointMode, but this is not supported on this platform");
                }
            }
            break;

        case VK_SHADER_STAGE_GEOMETRY_BIT:
            if (num_comp_in > limits.maxGeometryInputComponents) {
                skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-Location-06272",
                                 "Invalid Pipeline CreateInfo State: Geometry shader exceeds "
                                 "VkPhysicalDeviceLimits::maxGeometryInputComponents of %u "
                                 "components by %u components",
                                 limits.maxGeometryInputComponents, num_comp_in - limits.maxGeometryInputComponents);
            }
            if (max_comp_in > static_cast<int>(limits.maxGeometryInputComponents)) {
                skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-Location-06272",
                                 "Invalid Pipeline CreateInfo State: Geometry shader input variable uses location that "
                                 "exceeds component limit VkPhysicalDeviceLimits::maxGeometryInputComponents (%u)",
                                 limits.maxGeometryInputComponents);
            }
            if (num_comp_out > limits.maxGeometryOutputComponents) {
                skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-Location-06272",
                                 "Invalid Pipeline CreateInfo State: Geometry shader exceeds "
                                 "VkPhysicalDeviceLimits::maxGeometryOutputComponents of %u "
                                 "components by %u components",
                                 limits.maxGeometryOutputComponents, num_comp_out - limits.maxGeometryOutputComponents);
            }
            if (max_comp_out > static_cast<int>(limits.maxGeometryOutputComponents)) {
                skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-Location-06272",
                                 "Invalid Pipeline CreateInfo State: Geometry shader output variable uses location that "
                                 "exceeds component limit VkPhysicalDeviceLimits::maxGeometryOutputComponents (%u)",
                                 limits.maxGeometryOutputComponents);
            }
            if (num_comp_out * num_vertices > limits.maxGeometryTotalOutputComponents) {
                skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-Location-06272",
                                 "Invalid Pipeline CreateInfo State: Geometry shader exceeds "
                                 "VkPhysicalDeviceLimits::maxGeometryTotalOutputComponents of %u "
                                 "components by %u components",
                                 limits.maxGeometryTotalOutputComponents,
                                 num_comp_out * num_vertices - limits.maxGeometryTotalOutputComponents);
            }
            break;

        case VK_SHADER_STAGE_FRAGMENT_BIT:
            if (num_comp_in > limits.maxFragmentInputComponents) {
                skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-Location-06272",
                                 "Invalid Pipeline CreateInfo State: Fragment shader exceeds "
                                 "VkPhysicalDeviceLimits::maxFragmentInputComponents of %u "
                                 "components by %u components",
                                 limits.maxFragmentInputComponents, num_comp_in - limits.maxFragmentInputComponents);
            }
            if (max_comp_in > static_cast<int>(limits.maxFragmentInputComponents)) {
                skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-Location-06272",
                                 "Invalid Pipeline CreateInfo State: Fragment shader input variable uses location that "
                                 "exceeds component limit VkPhysicalDeviceLimits::maxFragmentInputComponents (%u)",
                                 limits.maxFragmentInputComponents);
            }
            break;

        case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
        case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
        case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
        case VK_SHADER_STAGE_MISS_BIT_KHR:
        case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
        case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
        case VK_SHADER_STAGE_TASK_BIT_NV:
        case VK_SHADER_STAGE_MESH_BIT_NV:
            break;

        default:
            assert(false);  // This should never happen
    }
    return skip;
}

bool CoreChecks::ValidateShaderStorageImageFormatsVariables(const SHADER_MODULE_STATE &module_state,
                                                            const spirv_inst_iter &insn) const {
    bool skip = false;
    // Go through all variables for images and check decorations
    assert(insn.opcode() == spv::OpVariable);
    // spirv-val validates this is an OpTypePointer
    const spirv_inst_iter pointer_def = module_state.get_def(insn.word(1));
    if (pointer_def.word(2) != spv::StorageClassUniformConstant) {
        return skip;  // Vulkan Spec says storage image must be UniformConstant
    }
    spirv_inst_iter type_def = module_state.get_def(pointer_def.word(3));

    // Unpack an optional level of arraying
    if (type_def.opcode() == spv::OpTypeArray || type_def.opcode() == spv::OpTypeRuntimeArray) {
        type_def = module_state.get_def(type_def.word(2));
    }

    if (type_def != module_state.end() && type_def.opcode() == spv::OpTypeImage) {
        // Only check if the Image Dim operand is not SubpassData
        const uint32_t dim = type_def.word(3);
        // Only check storage images
        const uint32_t sampled = type_def.word(7);
        const uint32_t image_format = type_def.word(8);
        if ((dim == spv::DimSubpassData) || (sampled != 2) || (image_format != spv::ImageFormatUnknown)) {
            return skip;
        }

        const uint32_t var_id = insn.word(2);
        decoration_set img_decorations = module_state.get_decorations(var_id);

        if (!enabled_features.core.shaderStorageImageReadWithoutFormat &&
            !(img_decorations.flags & decoration_set::nonreadable_bit)) {
            skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-OpTypeImage-06270",
                             "shaderStorageImageReadWithoutFormat is not supported but\n%s\nhas an Image\n%s\nwith Unknown "
                             "format and is not decorated with NonReadable",
                             module_state.DescribeInstruction(module_state.get_def(var_id)).c_str(),
                             module_state.DescribeInstruction(type_def).c_str());
        }

        if (!enabled_features.core.shaderStorageImageWriteWithoutFormat &&
            !(img_decorations.flags & decoration_set::nonwritable_bit)) {
            skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-OpTypeImage-06269",
                             "shaderStorageImageWriteWithoutFormat is not supported but\n%s\nhas an Image\n%s\nwith "
                             "Unknown format and is not decorated with NonWritable",
                             module_state.DescribeInstruction(module_state.get_def(var_id)).c_str(),
                             module_state.DescribeInstruction(type_def).c_str());
        }
    }

    return skip;
}

bool CoreChecks::ValidateShaderStageMaxResources(const SHADER_MODULE_STATE &module_state, VkShaderStageFlagBits stage,
                                                 const PIPELINE_STATE *pipeline) const {
    bool skip = false;
    uint32_t total_resources = 0;

    const auto &rp_state = pipeline->RenderPassState();
    if ((stage == VK_SHADER_STAGE_FRAGMENT_BIT) && rp_state) {
        if (rp_state->UsesDynamicRendering()) {
            total_resources += rp_state->dynamic_rendering_pipeline_create_info.colorAttachmentCount;
        } else {
            // "For the fragment shader stage the framebuffer color attachments also count against this limit"
            total_resources += rp_state->createInfo.pSubpasses[pipeline->Subpass()].colorAttachmentCount;
        }
    }

    // TODO: This reuses a lot of GetDescriptorCountMaxPerStage but currently would need to make it agnostic in a way to handle
    // input from CreatePipeline and CreatePipelineLayout level
    const auto &layout_state = pipeline->PipelineLayoutState();
    if (layout_state) {
        for (auto set_layout : layout_state->set_layouts) {
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
    }

    if (total_resources > phys_dev_props.limits.maxPerStageResources) {
        const char *vuid = nullptr;
        if (stage == VK_SHADER_STAGE_COMPUTE_BIT) {
            vuid = "VUID-VkComputePipelineCreateInfo-layout-01687";
        } else if ((stage & VK_SHADER_STAGE_ALL_GRAPHICS) == 0) {
            vuid = "VUID-VkRayTracingPipelineCreateInfoKHR-layout-03428";
        } else {
            vuid = "VUID-VkGraphicsPipelineCreateInfo-layout-01688";
        }
        skip |= LogError(module_state.vk_shader_module(), vuid,
                         "Invalid Pipeline CreateInfo State: Shader Stage %s exceeds component limit "
                         "VkPhysicalDeviceLimits::maxPerStageResources (%u)",
                         string_VkShaderStageFlagBits(stage), phys_dev_props.limits.maxPerStageResources);
    }

    return skip;
}

// copy the specialization constant value into buf, if it is present
template <typename StageCreateInfo>
void GetSpecConstantValue(StageCreateInfo const *pStage, uint32_t spec_id, void *buf) {
    const auto *spec = pStage->pSpecializationInfo;

    if (spec && spec_id < spec->mapEntryCount) {
        memcpy(buf, (uint8_t *)spec->pData + spec->pMapEntries[spec_id].offset, spec->pMapEntries[spec_id].size);
    }
}

// Fill in value with the constant or specialization constant value, if available.
// Returns true if the value has been accurately filled out.
static bool GetIntConstantValue(spirv_inst_iter insn, const SHADER_MODULE_STATE &module_state,
                                safe_VkPipelineShaderStageCreateInfo const *pStage,
                                const layer_data::unordered_map<uint32_t, uint32_t> &id_to_spec_id, uint32_t *value) {
    auto type_id = module_state.get_def(insn.word(1));
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
VkComponentTypeNV GetComponentType(spirv_inst_iter insn) {
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
bool CoreChecks::ValidateCooperativeMatrix(const SHADER_MODULE_STATE &module_state,
                                           safe_VkPipelineShaderStageCreateInfo const *pStage,
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

        void Init(uint32_t id, const SHADER_MODULE_STATE &module_state, safe_VkPipelineShaderStageCreateInfo const *pStage,
                  const layer_data::unordered_map<uint32_t, uint32_t> &id_to_spec_id) {
            spirv_inst_iter insn = module_state.get_def(id);
            uint32_t component_type_id = insn.word(2);
            uint32_t scope_id = insn.word(3);
            uint32_t rows_id = insn.word(4);
            uint32_t cols_id = insn.word(5);
            auto component_type_iter = module_state.get_def(component_type_id);
            auto scope_iter = module_state.get_def(scope_id);
            auto rows_iter = module_state.get_def(rows_id);
            auto cols_iter = module_state.get_def(cols_id);

            all_constant = true;
            if (!GetIntConstantValue(scope_iter, module_state, pStage, id_to_spec_id, &scope)) {
                all_constant = false;
            }
            if (!GetIntConstantValue(rows_iter, module_state, pStage, id_to_spec_id, &rows)) {
                all_constant = false;
            }
            if (!GetIntConstantValue(cols_iter, module_state, pStage, id_to_spec_id, &cols)) {
                all_constant = false;
            }
            component_type = GetComponentType(component_type_iter);
        }
    };

    bool seen_coopmat_capability = false;

    for (auto insn : module_state) {
        if (OpcodeHasType(insn.opcode()) && OpcodeHasResult(insn.opcode())) {
            id_to_type_id[insn.word(2)] = insn.word(1);
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
                            module_state.vk_shader_module(), "VUID-RuntimeSpirv-OpTypeCooperativeMatrixNV-06322",
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
                m.Init(insn.word(1), module_state, pStage, id_to_spec_id);

                if (m.all_constant) {
                    // Validate that the type parameters are all supported for one of the
                    // operands of a cooperative matrix property.
                    bool valid = false;
                    for (uint32_t i = 0; i < cooperative_matrix_properties.size(); ++i) {
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
                        skip |= LogError(module_state.vk_shader_module(), kVUID_Core_Shader_CooperativeMatrixType,
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
                d.Init(id_to_type_id[insn.word(2)], module_state, pStage, id_to_spec_id);
                a.Init(id_to_type_id[insn.word(3)], module_state, pStage, id_to_spec_id);
                b.Init(id_to_type_id[insn.word(4)], module_state, pStage, id_to_spec_id);
                c.Init(id_to_type_id[insn.word(5)], module_state, pStage, id_to_spec_id);

                if (a.all_constant && b.all_constant && c.all_constant && d.all_constant) {
                    // Validate that the type parameters are all supported for the same
                    // cooperative matrix property.
                    bool valid = false;
                    for (uint32_t i = 0; i < cooperative_matrix_properties.size(); ++i) {
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
                        skip |= LogError(module_state.vk_shader_module(), kVUID_Core_Shader_CooperativeMatrixMulAdd,
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

bool CoreChecks::ValidateShaderResolveQCOM(const SHADER_MODULE_STATE &module_state,
                                           safe_VkPipelineShaderStageCreateInfo const *pStage,
                                           const PIPELINE_STATE *pipeline) const {
    bool skip = false;

    // If the pipeline's subpass description contains flag VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_QCOM,
    // then the fragment shader must not enable the SPIRV SampleRateShading capability.
    if (pStage->stage == VK_SHADER_STAGE_FRAGMENT_BIT) {
        for (auto insn : module_state) {
            switch (insn.opcode()) {
                case spv::OpCapability:
                    if (insn.word(1) == spv::CapabilitySampleRateShading) {
                        const auto &rp_state = pipeline->RenderPassState();
                        auto subpass_flags = (!rp_state) ? 0 : rp_state->createInfo.pSubpasses[pipeline->Subpass()].flags;
                        if ((subpass_flags & VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_QCOM) != 0) {
                            LogObjectList objlist(module_state.vk_shader_module());
                            objlist.add(rp_state->renderPass());
                            skip |=
                                LogError(objlist, "VUID-RuntimeSpirv-SampleRateShading-06378",
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

bool CoreChecks::ValidateShaderSubgroupSizeControl(const SHADER_MODULE_STATE &module_state,
                                                   safe_VkPipelineShaderStageCreateInfo const *pStage) const {
    bool skip = false;

    if ((pStage->flags & VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT) != 0 &&
        !enabled_features.core13.subgroupSizeControl) {
        skip |= LogError(
            module_state.vk_shader_module(), "VUID-VkPipelineShaderStageCreateInfo-flags-02784",
            "VkPipelineShaderStageCreateInfo flags contain VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT, "
            "but the VkPhysicalDeviceSubgroupSizeControlFeaturesEXT::subgroupSizeControl feature is not enabled.");
    }

    if ((pStage->flags & VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT) != 0 &&
        !enabled_features.core13.computeFullSubgroups) {
        skip |= LogError(
            module_state.vk_shader_module(), "VUID-VkPipelineShaderStageCreateInfo-flags-02785",
            "VkPipelineShaderStageCreateInfo flags contain VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT, but the "
            "VkPhysicalDeviceSubgroupSizeControlFeaturesEXT::computeFullSubgroups feature is not enabled");
    }

    return skip;
}

bool CoreChecks::ValidateAtomicsTypes(const SHADER_MODULE_STATE &module_state) const {
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

    for (const auto &atomic_inst : module_state.GetAtomicInstructions()) {
        const atomic_instruction &atomic = atomic_inst.second;
        const spirv_inst_iter atomic_def = module_state.at(atomic_inst.first);
        const uint32_t opcode = atomic_def.opcode();

        if ((atomic.bit_width == 64) && (atomic.type == spv::OpTypeInt)) {
            // Validate 64-bit image atomics
            if (((atomic.storage_class == spv::StorageClassStorageBuffer) || (atomic.storage_class == spv::StorageClassUniform)) &&
                (enabled_features.core12.shaderBufferInt64Atomics == VK_FALSE)) {
                skip |=
                    LogError(device, "VUID-RuntimeSpirv-None-06278",
                             "%s: Can't use 64-bit int atomics operations\n%s\nwith %s storage class without "
                             "shaderBufferInt64Atomics enabled.",
                             report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                             module_state.DescribeInstruction(atomic_def).c_str(), string_SpvStorageClass(atomic.storage_class));
            } else if ((atomic.storage_class == spv::StorageClassWorkgroup) &&
                       (enabled_features.core12.shaderSharedInt64Atomics == VK_FALSE)) {
                skip |= LogError(device, "VUID-RuntimeSpirv-None-06279",
                                 "%s: Can't use 64-bit int atomics operations\n%s\nwith Workgroup storage class without "
                                 "shaderSharedInt64Atomics enabled.",
                                 report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                                 module_state.DescribeInstruction(atomic_def).c_str());
            } else if ((atomic.storage_class == spv::StorageClassImage) && (valid_image_64_int == false)) {
                skip |= LogError(device, "VUID-RuntimeSpirv-None-06288",
                                 "%s: Can't use 64-bit int atomics operations\n%s\nwith Image storage class without "
                                 "shaderImageInt64Atomics enabled.",
                                 report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                                 module_state.DescribeInstruction(atomic_def).c_str());
            }
        } else if (atomic.type == spv::OpTypeFloat) {
            // Validate Floats
            if (atomic.storage_class == spv::StorageClassStorageBuffer) {
                if (valid_storage_buffer_float == false) {
                    const char *vuid = IsExtEnabled(device_extensions.vk_ext_shader_atomic_float2) ? "VUID-RuntimeSpirv-None-06284"
                                                                                                   : "VUID-RuntimeSpirv-None-06280";
                    skip |= LogError(device, vuid,
                                     "%s: Can't use float atomics operations\n%s\nwith StorageBuffer storage class without "
                                     "shaderBufferFloat32Atomics or shaderBufferFloat32AtomicAdd or shaderBufferFloat64Atomics or "
                                     "shaderBufferFloat64AtomicAdd or shaderBufferFloat16Atomics or shaderBufferFloat16AtomicAdd "
                                     "or shaderBufferFloat16AtomicMinMax or shaderBufferFloat32AtomicMinMax or "
                                     "shaderBufferFloat64AtomicMinMax enabled.",
                                     report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                                     module_state.DescribeInstruction(atomic_def).c_str());
                } else if (opcode == spv::OpAtomicFAddEXT) {
                    if ((atomic.bit_width == 16) && (float2_features.shaderBufferFloat16AtomicAdd == VK_FALSE)) {
                        skip |= LogError(device, kVUID_Core_Shader_AtomicFeature,
                                         "%s: Can't use 16-bit float atomics for add operations\n%s\nwith "
                                         "StorageBuffer storage class without shaderBufferFloat16AtomicAdd enabled.",
                                         report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 32) && (float_features.shaderBufferFloat32AtomicAdd == VK_FALSE)) {
                        skip |= LogError(device, kVUID_Core_Shader_AtomicFeature,
                                         "%s: Can't use 32-bit float atomics for add operations\n%s\nwith "
                                         "StorageBuffer storage class without shaderBufferFloat32AtomicAdd enabled.",
                                         report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 64) && (float_features.shaderBufferFloat64AtomicAdd == VK_FALSE)) {
                        skip |= LogError(device, kVUID_Core_Shader_AtomicFeature,
                                         "%s: Can't use 64-bit float atomics for add operations\n%s\nwith "
                                         "StorageBuffer storage class without shaderBufferFloat64AtomicAdd enabled.",
                                         report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    }
                } else if (opcode == spv::OpAtomicFMinEXT || opcode == spv::OpAtomicFMaxEXT) {
                    if ((atomic.bit_width == 16) && (float2_features.shaderBufferFloat16AtomicMinMax == VK_FALSE)) {
                        skip |= LogError(device, kVUID_Core_Shader_AtomicFeature,
                                         "%s: Can't use 16-bit float atomics for min/max operations\n%s\nwith "
                                         "StorageBuffer storage class without shaderBufferFloat16AtomicMinMax enabled.",
                                         report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 32) && (float2_features.shaderBufferFloat32AtomicMinMax == VK_FALSE)) {
                        skip |= LogError(device, kVUID_Core_Shader_AtomicFeature,
                                         "%s: Can't use 32-bit float atomics for min/max operations\n%s\nwith "
                                         "StorageBuffer storage class without shaderBufferFloat32AtomicMinMax enabled.",
                                         report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 64) && (float2_features.shaderBufferFloat64AtomicMinMax == VK_FALSE)) {
                        skip |= LogError(device, kVUID_Core_Shader_AtomicFeature,
                                         "%s: Can't use 64-bit float atomics for min/max operations\n%s\nwith "
                                         "StorageBuffer storage class without shaderBufferFloat64AtomicMinMax enabled.",
                                         report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    }
                } else {
                    // Assume is valid load/store/exchange (rest of supported atomic operations) or else spirv-val will catch
                    if ((atomic.bit_width == 16) && (float2_features.shaderBufferFloat16Atomics == VK_FALSE)) {
                        skip |= LogError(device, kVUID_Core_Shader_AtomicFeature,
                                         "%s: Can't use 16-bit float atomics for load/store/exhange operations\n%s\nwith "
                                         "StorageBuffer storage class without shaderBufferFloat16Atomics enabled.",
                                         report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 32) && (float_features.shaderBufferFloat32Atomics == VK_FALSE)) {
                        skip |= LogError(device, kVUID_Core_Shader_AtomicFeature,
                                         "%s: Can't use 32-bit float atomics for load/store/exhange operations\n%s\nwith "
                                         "StorageBuffer storage class without shaderBufferFloat32Atomics enabled.",
                                         report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 64) && (float_features.shaderBufferFloat64Atomics == VK_FALSE)) {
                        skip |= LogError(device, kVUID_Core_Shader_AtomicFeature,
                                         "%s: Can't use 64-bit float atomics for load/store/exhange operations\n%s\nwith "
                                         "StorageBuffer storage class without shaderBufferFloat64Atomics enabled.",
                                         report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    }
                }
            } else if (atomic.storage_class == spv::StorageClassWorkgroup) {
                if (valid_workgroup_float == false) {
                    const char *vuid = IsExtEnabled(device_extensions.vk_ext_shader_atomic_float2) ? "VUID-RuntimeSpirv-None-06285"
                                                                                                   : "VUID-RuntimeSpirv-None-06281";
                    skip |=
                        LogError(device, vuid,
                                 "%s: Can't use float atomics operations\n%s\nwith Workgroup storage class without "
                                 "shaderSharedFloat32Atomics or "
                                 "shaderSharedFloat32AtomicAdd or shaderSharedFloat64Atomics or shaderSharedFloat64AtomicAdd or "
                                 "shaderSharedFloat16Atomics or shaderSharedFloat16AtomicAdd or shaderSharedFloat16AtomicMinMax or "
                                 "shaderSharedFloat32AtomicMinMax or shaderSharedFloat64AtomicMinMax enabled.",
                                 report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                                 module_state.DescribeInstruction(atomic_def).c_str());
                } else if (opcode == spv::OpAtomicFAddEXT) {
                    if ((atomic.bit_width == 16) && (float2_features.shaderSharedFloat16AtomicAdd == VK_FALSE)) {
                        skip |= LogError(device, kVUID_Core_Shader_AtomicFeature,
                                         "%s: Can't use 16-bit float atomics for add operations\n%s\nwith Workgroup "
                                         "storage class without shaderSharedFloat16AtomicAdd enabled.",
                                         report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 32) && (float_features.shaderSharedFloat32AtomicAdd == VK_FALSE)) {
                        skip |= LogError(device, kVUID_Core_Shader_AtomicFeature,
                                         "%s: Can't use 32-bit float atomics for add operations\n%s\nwith Workgroup "
                                         "storage class without shaderSharedFloat32AtomicAdd enabled.",
                                         report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 64) && (float_features.shaderSharedFloat64AtomicAdd == VK_FALSE)) {
                        skip |= LogError(device, kVUID_Core_Shader_AtomicFeature,
                                         "%s: Can't use 64-bit float atomics for add operations\n%s\nwith Workgroup "
                                         "storage class without shaderSharedFloat64AtomicAdd enabled.",
                                         report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    }
                } else if (opcode == spv::OpAtomicFMinEXT || opcode == spv::OpAtomicFMaxEXT) {
                    if ((atomic.bit_width == 16) && (float2_features.shaderSharedFloat16AtomicMinMax == VK_FALSE)) {
                        skip |= LogError(device, kVUID_Core_Shader_AtomicFeature,
                                         "%s: Can't use 16-bit float atomics for min/max operations\n%s\nwith "
                                         "Workgroup storage class without shaderSharedFloat16AtomicMinMax enabled.",
                                         report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 32) && (float2_features.shaderSharedFloat32AtomicMinMax == VK_FALSE)) {
                        skip |= LogError(device, kVUID_Core_Shader_AtomicFeature,
                                         "%s: Can't use 32-bit float atomics for min/max operations\n%s\nwith "
                                         "Workgroup storage class without shaderSharedFloat32AtomicMinMax enabled.",
                                         report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 64) && (float2_features.shaderSharedFloat64AtomicMinMax == VK_FALSE)) {
                        skip |= LogError(device, kVUID_Core_Shader_AtomicFeature,
                                         "%s: Can't use 64-bit float atomics for min/max operations\n%s\nwith "
                                         "Workgroup storage class without shaderSharedFloat64AtomicMinMax enabled.",
                                         report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    }
                } else {
                    // Assume is valid load/store/exchange (rest of supported atomic operations) or else spirv-val will catch
                    if ((atomic.bit_width == 16) && (float2_features.shaderSharedFloat16Atomics == VK_FALSE)) {
                        skip |= LogError(device, kVUID_Core_Shader_AtomicFeature,
                                         "%s: Can't use 16-bit float atomics for load/store/exhange operations\n%s\nwith Workgroup "
                                         "storage class without shaderSharedFloat16Atomics enabled.",
                                         report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 32) && (float_features.shaderSharedFloat32Atomics == VK_FALSE)) {
                        skip |= LogError(device, kVUID_Core_Shader_AtomicFeature,
                                         "%s: Can't use 32-bit float atomics for load/store/exhange operations\n%s\nwith Workgroup "
                                         "storage class without shaderSharedFloat32Atomics enabled.",
                                         report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    } else if ((atomic.bit_width == 64) && (float_features.shaderSharedFloat64Atomics == VK_FALSE)) {
                        skip |= LogError(device, kVUID_Core_Shader_AtomicFeature,
                                         "%s: Can't use 64-bit float atomics for load/store/exhange operations\n%s\nwith Workgroup "
                                         "storage class without shaderSharedFloat64Atomics enabled.",
                                         report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                                         module_state.DescribeInstruction(atomic_def).c_str());
                    }
                }
            } else if ((atomic.storage_class == spv::StorageClassImage) && (valid_image_float == false)) {
                const char *vuid = IsExtEnabled(device_extensions.vk_ext_shader_atomic_float2) ? "VUID-RuntimeSpirv-None-06286"
                                                                                               : "VUID-RuntimeSpirv-None-06282";
                skip |= LogError(
                    device, vuid,
                    "%s: Can't use float atomics operations\n%s\nwith Image storage class without shaderImageFloat32Atomics or "
                    "shaderImageFloat32AtomicAdd or shaderImageFloat32AtomicMinMax enabled.",
                    report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                    module_state.DescribeInstruction(atomic_def).c_str());
            } else if ((atomic.bit_width == 16) && (valid_16_float == false)) {
                skip |= LogError(device, "VUID-RuntimeSpirv-None-06337",
                                 "%s: Can't use 16-bit float atomics operations\n%s\nwithout shaderBufferFloat16Atomics, "
                                 "shaderBufferFloat16AtomicAdd, shaderBufferFloat16AtomicMinMax, shaderSharedFloat16Atomics, "
                                 "shaderSharedFloat16AtomicAdd or shaderSharedFloat16AtomicMinMax enabled.",
                                 report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                                 module_state.DescribeInstruction(atomic_def).c_str());
            } else if ((atomic.bit_width == 32) && (valid_32_float == false)) {
                const char *vuid = IsExtEnabled(device_extensions.vk_ext_shader_atomic_float2) ? "VUID-RuntimeSpirv-None-06338"
                                                                                               : "VUID-RuntimeSpirv-None-06335";
                skip |= LogError(device, vuid,
                                 "%s: Can't use 32-bit float atomics operations\n%s\nwithout shaderBufferFloat32AtomicMinMax, "
                                 "shaderSharedFloat32AtomicMinMax, shaderImageFloat32AtomicMinMax, sparseImageFloat32AtomicMinMax, "
                                 "shaderBufferFloat32Atomics, shaderBufferFloat32AtomicAdd, shaderSharedFloat32Atomics, "
                                 "shaderSharedFloat32AtomicAdd, shaderImageFloat32Atomics, shaderImageFloat32AtomicAdd, "
                                 "sparseImageFloat32Atomics or sparseImageFloat32AtomicAdd enabled.",
                                 report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                                 module_state.DescribeInstruction(atomic_def).c_str());
            } else if ((atomic.bit_width == 64) && (valid_64_float == false)) {
                const char *vuid = IsExtEnabled(device_extensions.vk_ext_shader_atomic_float2) ? "VUID-RuntimeSpirv-None-06339"
                                                                                               : "VUID-RuntimeSpirv-None-06336";
                skip |= LogError(device, vuid,
                                 "%s: Can't use 64-bit float atomics operations\n%s\nwithout shaderBufferFloat64AtomicMinMax, "
                                 "shaderSharedFloat64AtomicMinMax, shaderBufferFloat64Atomics, shaderBufferFloat64AtomicAdd, "
                                 "shaderSharedFloat64Atomics or shaderSharedFloat64AtomicAdd enabled.",
                                 report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                                 module_state.DescribeInstruction(atomic_def).c_str());
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateExecutionModes(const SHADER_MODULE_STATE &module_state, spirv_inst_iter entrypoint,
                                        VkShaderStageFlagBits stage, const PIPELINE_STATE *pipeline) const {
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

    const auto &execution_mode_inst = module_state.GetExecutionModeInstructions();
    auto it = execution_mode_inst.find(entrypoint_id);
    if (it != execution_mode_inst.end()) {
        for (auto insn : it->second) {
            auto mode = insn.word(2);
            switch (mode) {
                case spv::ExecutionModeSignedZeroInfNanPreserve: {
                    auto bit_width = insn.word(3);
                    if (bit_width == 16 && !phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat16) {
                        skip |= LogError(
                            module_state.vk_shader_module(), "VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat16-06293",
                            "Shader requires SignedZeroInfNanPreserve for bit width 16 but it is not enabled on the device\n%s",
                            module_state.DescribeInstruction(insn).c_str());
                    } else if (bit_width == 32 && !phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat32) {
                        skip |= LogError(
                            module_state.vk_shader_module(), "VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat32-06294",
                            "Shader requires SignedZeroInfNanPreserve for bit width 32 but it is not enabled on the device\n%s",
                            module_state.DescribeInstruction(insn).c_str());
                    } else if (bit_width == 64 && !phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat64) {
                        skip |= LogError(
                            module_state.vk_shader_module(), "VUID-RuntimeSpirv-shaderSignedZeroInfNanPreserveFloat64-06295",
                            "Shader requires SignedZeroInfNanPreserve for bit width 64 but it is not enabled on the device\n%s",
                            module_state.DescribeInstruction(insn).c_str());
                    }
                    break;
                }

                case spv::ExecutionModeDenormPreserve: {
                    auto bit_width = insn.word(3);
                    if (bit_width == 16 && !phys_dev_props_core12.shaderDenormPreserveFloat16) {
                        skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-shaderDenormPreserveFloat16-06296",
                                         "Shader requires DenormPreserve for bit width 16 but it is not enabled on the device\n%s",
                                         module_state.DescribeInstruction(insn).c_str());
                        ;
                    } else if (bit_width == 32 && !phys_dev_props_core12.shaderDenormPreserveFloat32) {
                        skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-shaderDenormPreserveFloat32-06297",
                                         "Shader requires DenormPreserve for bit width 32 but it is not enabled on the device\n%s",
                                         module_state.DescribeInstruction(insn).c_str());
                    } else if (bit_width == 64 && !phys_dev_props_core12.shaderDenormPreserveFloat64) {
                        skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-shaderDenormPreserveFloat64-06298",
                                         "Shader requires DenormPreserve for bit width 64 but it is not enabled on the device\n%s",
                                         module_state.DescribeInstruction(insn).c_str());
                    }

                    if (first_denorm_execution_mode.first == spv::ExecutionModeMax) {
                        // Register the first denorm execution mode found
                        first_denorm_execution_mode = std::make_pair(static_cast<spv::ExecutionMode>(mode), bit_width);
                    } else if (first_denorm_execution_mode.first != mode && first_denorm_execution_mode.second != bit_width) {
                        switch (phys_dev_props_core12.denormBehaviorIndependence) {
                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY:
                                if (first_rounding_mode.second != 32 && bit_width != 32) {
                                    skip |= LogError(module_state.vk_shader_module(),
                                                     "VUID-RuntimeSpirv-denormBehaviorIndependence-06289",
                                                     "Shader uses different denorm execution modes for 16 and 64-bit but "
                                                     "denormBehaviorIndependence is "
                                                     "VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY on the device\n%s",
                                                     module_state.DescribeInstruction(insn).c_str());
                                }
                                break;

                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL:
                                break;

                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE: {
                                skip |=
                                    LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-denormBehaviorIndependence-06290",
                                             "Shader uses different denorm execution modes for different bit widths but "
                                             "denormBehaviorIndependence is "
                                             "VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE on the device\n%s",
                                             module_state.DescribeInstruction(insn).c_str());
                                break;
                            }

                            default:
                                break;
                        }
                    }
                    break;
                }

                case spv::ExecutionModeDenormFlushToZero: {
                    auto bit_width = insn.word(3);
                    if (bit_width == 16 && !phys_dev_props_core12.shaderDenormFlushToZeroFloat16) {
                        skip |=
                            LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-shaderDenormFlushToZeroFloat16-06299",
                                     "Shader requires DenormFlushToZero for bit width 16 but it is not enabled on the device\n%s",
                                     module_state.DescribeInstruction(insn).c_str());
                    } else if (bit_width == 32 && !phys_dev_props_core12.shaderDenormFlushToZeroFloat32) {
                        skip |=
                            LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-shaderDenormFlushToZeroFloat32-06300",
                                     "Shader requires DenormFlushToZero for bit width 32 but it is not enabled on the device\n%s",
                                     module_state.DescribeInstruction(insn).c_str());
                    } else if (bit_width == 64 && !phys_dev_props_core12.shaderDenormFlushToZeroFloat64) {
                        skip |=
                            LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-shaderDenormFlushToZeroFloat64-06301",
                                     "Shader requires DenormFlushToZero for bit width 64 but it is not enabled on the device\n%s",
                                     module_state.DescribeInstruction(insn).c_str());
                    }

                    if (first_denorm_execution_mode.first == spv::ExecutionModeMax) {
                        // Register the first denorm execution mode found
                        first_denorm_execution_mode = std::make_pair(static_cast<spv::ExecutionMode>(mode), bit_width);
                    } else if (first_denorm_execution_mode.first != mode && first_denorm_execution_mode.second != bit_width) {
                        switch (phys_dev_props_core12.denormBehaviorIndependence) {
                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY:
                                if (first_rounding_mode.second != 32 && bit_width != 32) {
                                    skip |= LogError(module_state.vk_shader_module(),
                                                     "VUID-RuntimeSpirv-denormBehaviorIndependence-06289",
                                                     "Shader uses different denorm execution modes for 16 and 64-bit but "
                                                     "denormBehaviorIndependence is "
                                                     "VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY on the device\n%s",
                                                     module_state.DescribeInstruction(insn).c_str());
                                }
                                break;

                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL:
                                break;

                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE: {
                                skip |=
                                    LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-denormBehaviorIndependence-06290",
                                             "Shader uses different denorm execution modes for different bit widths but "
                                             "denormBehaviorIndependence is "
                                             "VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE on the device\n%s",
                                             module_state.DescribeInstruction(insn).c_str());
                                break;
                            }

                            default:
                                break;
                        }
                    }
                    break;
                }

                case spv::ExecutionModeRoundingModeRTE: {
                    auto bit_width = insn.word(3);
                    if (bit_width == 16 && !phys_dev_props_core12.shaderRoundingModeRTEFloat16) {
                        skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-shaderRoundingModeRTEFloat16-06302",
                                         "Shader requires RoundingModeRTE for bit width 16 but it is not enabled on the device\n%s",
                                         module_state.DescribeInstruction(insn).c_str());
                    } else if (bit_width == 32 && !phys_dev_props_core12.shaderRoundingModeRTEFloat32) {
                        skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-shaderRoundingModeRTEFloat32-06303",
                                         "Shader requires RoundingModeRTE for bit width 32 but it is not enabled on the device\n%s",
                                         module_state.DescribeInstruction(insn).c_str());
                    } else if (bit_width == 64 && !phys_dev_props_core12.shaderRoundingModeRTEFloat64) {
                        skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-shaderRoundingModeRTEFloat64-06304",
                                         "Shader requires RoundingModeRTE for bit width 64 but it is not enabled on the device\n%s",
                                         module_state.DescribeInstruction(insn).c_str());
                    }

                    if (first_rounding_mode.first == spv::ExecutionModeMax) {
                        // Register the first rounding mode found
                        first_rounding_mode = std::make_pair(static_cast<spv::ExecutionMode>(mode), bit_width);
                    } else if (first_rounding_mode.first != mode && first_rounding_mode.second != bit_width) {
                        switch (phys_dev_props_core12.roundingModeIndependence) {
                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY:
                                if (first_rounding_mode.second != 32 && bit_width != 32) {
                                    skip |= LogError(module_state.vk_shader_module(),
                                                     "VUID-RuntimeSpirv-roundingModeIndependence-06291",
                                                     "Shader uses different rounding modes for 16 and 64-bit but "
                                                     "roundingModeIndependence is "
                                                     "VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY on the device\n%s",
                                                     module_state.DescribeInstruction(insn).c_str());
                                }
                                break;

                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL:
                                break;

                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE: {
                                skip |=
                                    LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-roundingModeIndependence-06292",
                                             "Shader uses different rounding modes for different bit widths but "
                                             "roundingModeIndependence is "
                                             "VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE on the device\n%s",
                                             module_state.DescribeInstruction(insn).c_str());
                                break;
                            }

                            default:
                                break;
                        }
                    }
                    break;
                }

                case spv::ExecutionModeRoundingModeRTZ: {
                    auto bit_width = insn.word(3);
                    if (bit_width == 16 && !phys_dev_props_core12.shaderRoundingModeRTZFloat16) {
                        skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-shaderRoundingModeRTZFloat16-06305",
                                         "Shader requires RoundingModeRTZ for bit width 16 but it is not enabled on the device\n%s",
                                         module_state.DescribeInstruction(insn).c_str());
                    } else if (bit_width == 32 && !phys_dev_props_core12.shaderRoundingModeRTZFloat32) {
                        skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-shaderRoundingModeRTZFloat32-06306",
                                         "Shader requires RoundingModeRTZ for bit width 32 but it is not enabled on the device\n%s",
                                         module_state.DescribeInstruction(insn).c_str());
                    } else if (bit_width == 64 && !phys_dev_props_core12.shaderRoundingModeRTZFloat64) {
                        skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-shaderRoundingModeRTZFloat64-06307",
                                         "Shader requires RoundingModeRTZ for bit width 64 but it is not enabled on the device\n%s",
                                         module_state.DescribeInstruction(insn).c_str());
                    }

                    if (first_rounding_mode.first == spv::ExecutionModeMax) {
                        // Register the first rounding mode found
                        first_rounding_mode = std::make_pair(static_cast<spv::ExecutionMode>(mode), bit_width);
                    } else if (first_rounding_mode.first != mode && first_rounding_mode.second != bit_width) {
                        switch (phys_dev_props_core12.roundingModeIndependence) {
                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY:
                                if (first_rounding_mode.second != 32 && bit_width != 32) {
                                    skip |= LogError(module_state.vk_shader_module(),
                                                     "VUID-RuntimeSpirv-roundingModeIndependence-06291",
                                                     "Shader uses different rounding modes for 16 and 64-bit but "
                                                     "roundingModeIndependence is "
                                                     "VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_32_BIT_ONLY on the device\n%s",
                                                     module_state.DescribeInstruction(insn).c_str());
                                }
                                break;

                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL:
                                break;

                            case VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE: {
                                skip |=
                                    LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-roundingModeIndependence-06292",
                                             "Shader uses different rounding modes for different bit widths but "
                                             "roundingModeIndependence is "
                                             "VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE on the device\n%s",
                                             module_state.DescribeInstruction(insn).c_str());
                                break;
                            }

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

                case spv::ExecutionModeLocalSizeId: {
                    if (!enabled_features.core13.maintenance4) {
                        skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-LocalSizeId-06434",
                                         "LocalSizeId execution mode used but maintenance4 feature not enabled");
                    }
                    if (!IsExtEnabled(device_extensions.vk_khr_maintenance4)) {
                        skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-LocalSizeId-06433",
                                         "LocalSizeId execution mode used but maintenance4 extension is not enabled and used "
                                         "Vulkan api version is 1.2 or less");
                    }
                    break;
                }

                case spv::ExecutionModeEarlyFragmentTests: {
                    const auto *ds_state = (pipeline) ? pipeline->DepthStencilState() : nullptr;
                    if ((stage == VK_SHADER_STAGE_FRAGMENT_BIT) &&
                        (ds_state &&
                         (ds_state->flags &
                          (VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_ARM |
                           VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_ARM)) != 0)) {
                        skip |= LogError(
                            module_state.vk_shader_module(), "VUID-VkGraphicsPipelineCreateInfo-flags-06591",
                            "The fragment shader enables early fragment tests, but VkPipelineDepthStencilStateCreateInfo::flags == "
                            "%s",
                            string_VkPipelineDepthStencilStateCreateFlags(ds_state->flags).c_str());
                    }
                    break;
                }
                case spv::ExecutionModeSubgroupUniformControlFlowKHR: {
                    if (!enabled_features.shader_subgroup_uniform_control_flow_features.shaderSubgroupUniformControlFlow ||
                        (phys_dev_ext_props.subgroup_properties.supportedStages & stage) == 0 ||
                        module_state.static_data_.has_invocation_repack_instruction) {
                        std::stringstream msg;
                        if (!enabled_features.shader_subgroup_uniform_control_flow_features.shaderSubgroupUniformControlFlow) {
                            msg << "shaderSubgroupUniformControlFlow feature must be enabled";
                        } else if ((phys_dev_ext_props.subgroup_properties.supportedStages & stage) == 0) {
                            msg << "stage" << string_VkShaderStageFlagBits(stage)
                                << " must be in VkPhysicalDeviceSubgroupProperties::supportedStages("
                                << string_VkShaderStageFlags(phys_dev_ext_props.subgroup_properties.supportedStages) << ")";
                        } else {
                            msg << "the shader must not use any invocation repack instructions";
                        }
                        skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-SubgroupUniformControlFlowKHR-06379",
                                         "If ExecutionModeSubgroupUniformControlFlowKHR is used %s.", msg.str().c_str());
                    }
                } break;
            }
        }
    }

    if (entrypoint.word(1) == spv::ExecutionModelGeometry) {
        if (vertices_out == 0 || vertices_out > phys_dev_props.limits.maxGeometryOutputVertices) {
            skip |= LogError(module_state.vk_shader_module(), "VUID-VkPipelineShaderStageCreateInfo-stage-00714",
                             "Geometry shader entry point must have an OpExecutionMode instruction that "
                             "specifies a maximum output vertex count that is greater than 0 and less "
                             "than or equal to maxGeometryOutputVertices. "
                             "OutputVertices=%d, maxGeometryOutputVertices=%d",
                             vertices_out, phys_dev_props.limits.maxGeometryOutputVertices);
        }

        if (invocations == 0 || invocations > phys_dev_props.limits.maxGeometryShaderInvocations) {
            skip |= LogError(module_state.vk_shader_module(), "VUID-VkPipelineShaderStageCreateInfo-stage-00715",
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
bool CoreChecks::ValidatePointListShaderState(const PIPELINE_STATE *pipeline, const SHADER_MODULE_STATE &module_state,
                                              spirv_inst_iter entrypoint, VkShaderStageFlagBits stage) const {
    if (pipeline->topology_at_rasterizer != VK_PRIMITIVE_TOPOLOGY_POINT_LIST) {
        return false;
    }

    bool pointsize_written = false;
    bool skip = false;

    // Search for PointSize built-in decorations
    for (const auto &set : module_state.GetBuiltinDecorationList()) {
        auto insn = module_state.at(set.offset);
        if (set.builtin == spv::BuiltInPointSize) {
            pointsize_written = module_state.IsBuiltInWritten(insn, entrypoint);
            if (pointsize_written) {
                break;
            }
        }
    }

    if ((stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT || stage == VK_SHADER_STAGE_GEOMETRY_BIT) &&
        !enabled_features.core.shaderTessellationAndGeometryPointSize) {
        if (pointsize_written) {
            skip |= LogError(module_state.vk_shader_module(), kVUID_Core_Shader_PointSizeBuiltInOverSpecified,
                             "Pipeline topology is set to POINT_LIST and geometry or tessellation shaders write PointSize which "
                             "is prohibited when the shaderTessellationAndGeometryPointSize feature is not enabled.");
        }
    } else if (!pointsize_written) {
        skip |=
            LogError(module_state.vk_shader_module(), kVUID_Core_Shader_MissingPointSizeBuiltIn,
                     "Pipeline topology is set to POINT_LIST, but PointSize is not written to in the shader corresponding to %s.",
                     string_VkShaderStageFlagBits(stage));
    }
    return skip;
}

bool CoreChecks::ValidatePrimitiveRateShaderState(const PIPELINE_STATE *pipeline, const SHADER_MODULE_STATE &module_state,
                                                  spirv_inst_iter entrypoint, VkShaderStageFlagBits stage) const {
    bool primitiverate_written = false;
    bool viewportindex_written = false;
    bool viewportmask_written = false;
    bool skip = false;

    // Check if the primitive shading rate is written
    for (const auto &set : module_state.GetBuiltinDecorationList()) {
        auto insn = module_state.at(set.offset);
        if (set.builtin == spv::BuiltInPrimitiveShadingRateKHR) {
            primitiverate_written = module_state.IsBuiltInWritten(insn, entrypoint);
        } else if (set.builtin == spv::BuiltInViewportIndex) {
            viewportindex_written = module_state.IsBuiltInWritten(insn, entrypoint);
        } else if (set.builtin == spv::BuiltInViewportMaskNV) {
            viewportmask_written = module_state.IsBuiltInWritten(insn, entrypoint);
        }
        if (primitiverate_written && viewportindex_written && viewportmask_written) {
            break;
        }
    }

    const auto viewport_state = pipeline->ViewportState();
    if (!phys_dev_ext_props.fragment_shading_rate_props.primitiveFragmentShadingRateWithMultipleViewports &&
        (pipeline->GetPipelineType() == VK_PIPELINE_BIND_POINT_GRAPHICS) && viewport_state) {
        if (!IsDynamic(pipeline, VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT) && viewport_state->viewportCount > 1 &&
            primitiverate_written) {
            skip |= LogError(module_state.vk_shader_module(),
                             "VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04503",
                             "vkCreateGraphicsPipelines: %s shader statically writes to PrimitiveShadingRateKHR built-in, but "
                             "multiple viewports "
                             "are used and the primitiveFragmentShadingRateWithMultipleViewports limit is not supported.",
                             string_VkShaderStageFlagBits(stage));
        }

        if (primitiverate_written && viewportindex_written) {
            skip |= LogError(module_state.vk_shader_module(),
                             "VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04504",
                             "vkCreateGraphicsPipelines: %s shader statically writes to both PrimitiveShadingRateKHR and "
                             "ViewportIndex built-ins,"
                             "but the primitiveFragmentShadingRateWithMultipleViewports limit is not supported.",
                             string_VkShaderStageFlagBits(stage));
        }

        if (primitiverate_written && viewportmask_written) {
            skip |= LogError(module_state.vk_shader_module(),
                             "VUID-VkGraphicsPipelineCreateInfo-primitiveFragmentShadingRateWithMultipleViewports-04505",
                             "vkCreateGraphicsPipelines: %s shader statically writes to both PrimitiveShadingRateKHR and "
                             "ViewportMaskNV built-ins,"
                             "but the primitiveFragmentShadingRateWithMultipleViewports limit is not supported.",
                             string_VkShaderStageFlagBits(stage));
        }
    }
    return skip;
}

bool CoreChecks::ValidateDecorations(const SHADER_MODULE_STATE &module_state) const {
    bool skip = false;

    std::vector<spirv_inst_iter> xfb_streams;
    std::vector<spirv_inst_iter> xfb_buffers;
    std::vector<spirv_inst_iter> xfb_offsets;

    for (const auto &op_decorate : module_state.GetDecorationInstructions()) {
        uint32_t decoration = op_decorate.word(2);
        if (decoration == spv::DecorationXfbStride) {
            uint32_t stride = op_decorate.word(3);
            if (stride > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferDataStride) {
                skip |= LogError(
                    module_state.vk_shader_module(), "VUID-RuntimeSpirv-XfbStride-06313",
                    "vkCreateGraphicsPipelines(): shader uses transform feedback with xfb_stride (%" PRIu32
                    ") greater than VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackBufferDataStride (%" PRIu32
                    ").",
                    stride, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferDataStride);
            }
        }
        if (decoration == spv::DecorationStream) {
            xfb_streams.push_back(op_decorate);
            uint32_t stream = op_decorate.word(3);
            if (stream >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams) {
                skip |= LogError(
                    module_state.vk_shader_module(), "VUID-RuntimeSpirv-Stream-06312",
                    "vkCreateGraphicsPipelines(): shader uses transform feedback with stream (%" PRIu32
                    ") not less than VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackStreams (%" PRIu32 ").",
                    stream, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams);
            }
        }
        if (decoration == spv::DecorationXfbBuffer) {
            xfb_buffers.push_back(op_decorate);
        }
        if (decoration == spv::DecorationOffset) {
            xfb_offsets.push_back(op_decorate);
        }
    }

    // XfbBuffer, buffer data size
    std::vector<std::pair<uint32_t, uint32_t>> buffer_data_sizes;
    for (const auto &op_decorate : xfb_offsets) {
        for (const auto xfb_buffer : xfb_buffers) {
            if (xfb_buffer.word(1) == op_decorate.word(1)) {
                const auto offset = op_decorate.word(3);
                const auto def = module_state.get_def(xfb_buffer.word(1));
                const auto size = module_state.GetTypeBytesSize(def);
                const uint32_t buffer_data_size = offset + size;
                if (buffer_data_size > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferDataSize) {
                    skip |= LogError(
                        module_state.vk_shader_module(), "VUID-RuntimeSpirv-Offset-06308",
                        "vkCreateGraphicsPipelines(): shader uses transform feedback with xfb_offset (%" PRIu32
                        ") + size of variable (%" PRIu32
                        ") greater than VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackBufferDataSize "
                        "(%" PRIu32 ").",
                        offset, size, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferDataSize);
                }

                bool found = false;
                for (auto &bds : buffer_data_sizes) {
                    if (bds.first == xfb_buffer.word(1)) {
                        bds.second = std::max(bds.second, buffer_data_size);
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    buffer_data_sizes.emplace_back(xfb_buffer.word(1), buffer_data_size);
                }

                break;
            }
        }
    }

    std::unordered_map<uint32_t, uint32_t> stream_data_size;
    for (const auto &xfb_stream : xfb_streams) {
        for (const auto& bds : buffer_data_sizes) {
            if (xfb_stream.word(1) == bds.first) {
                uint32_t stream = xfb_stream.word(3);
                const auto itr = stream_data_size.find(stream);
                if (itr != stream_data_size.end()) {
                    itr->second += bds.second;
                } else {
                    stream_data_size.insert({stream, bds.second});
                }
            }
        }
    }

    for (const auto& stream : stream_data_size) {
        if (stream.second > phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreamDataSize) {
            skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-XfbBuffer-06309",
                             "vkCreateGraphicsPipelines(): shader uses transform feedback with stream (%" PRIu32
                             ") having the sum of buffer data sizes (%" PRIu32
                             ") not less than VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackBufferDataSize "
                             "(%" PRIu32 ").",
                             stream.first, stream.second,
                             phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackBufferDataSize);
        }
    }

    return skip;
}

bool CoreChecks::ValidateComputeSharedMemory(const SHADER_MODULE_STATE &module_state, uint32_t total_shared_size) const {
    bool skip = false;

    // If not found before with spec constants, find here
    if (total_shared_size == 0) {
        // when using WorkgroupMemoryExplicitLayoutKHR
        // either all or none the structs are decorated with Block,
        // if using block, all must decorated with Aliased.
        // In this case we want to find the MAX not ADD the block sizes
        bool find_max_block = false;

        for (auto insn : module_state.static_data_.variable_inst) {
            // StorageClass Workgroup is shared memory
            if (insn.word(3) == spv::StorageClassWorkgroup) {
                if (module_state.get_decorations(insn.word(2)).flags & decoration_set::aliased_bit) {
                    find_max_block = true;
                }

                const uint32_t result_type_id = insn.word(1);
                const auto result_type = module_state.get_def(result_type_id);
                const auto type = module_state.get_def(result_type.word(3));
                const uint32_t variable_shared_size = module_state.GetTypeBytesSize(type);

                if (find_max_block) {
                    total_shared_size = std::max(total_shared_size, variable_shared_size);
                } else {
                    total_shared_size += variable_shared_size;
                }
            }
        }
    }

    if (total_shared_size > phys_dev_props.limits.maxComputeSharedMemorySize) {
        skip |=
            LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-Workgroup-06530",
                     "Shader uses %" PRIu32
                     " bytes of shared memory, more than allowed by physicalDeviceLimits::maxComputeSharedMemorySize (%" PRIu32 ")",
                     total_shared_size, phys_dev_props.limits.maxComputeSharedMemorySize);
    }
    return skip;
}

bool CoreChecks::ValidateShaderModuleId(const SHADER_MODULE_STATE &module_state, const PipelineStageState &stage_state,
                                        const safe_VkPipelineShaderStageCreateInfo *pStage, const VkPipelineCreateFlags flags) const {
    bool skip = false;
    const auto module_identifier = LvlFindInChain<VkPipelineShaderStageModuleIdentifierCreateInfoEXT>(pStage->pNext);
    const auto module_create_info = LvlFindInChain<VkShaderModuleCreateInfo>(pStage->pNext);
    if (module_identifier && (module_identifier->identifierSize > 0)) {
        if (!(enabled_features.shader_module_identifier_features.shaderModuleIdentifier)) {
            skip |= LogError(
                device, "VUID-VkPipelineShaderStageModuleIdentifierCreateInfoEXT-pNext-06850",
                "%s module (stage %s) VkPipelineShaderStageCreateInfo has a VkPipelineShaderStageModuleIdentifierCreateInfoEXT "
                "struct in the pNext chain but the shaderModuleIdentifier feature is not enabled",
                report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                string_VkShaderStageFlagBits(stage_state.stage_flag));
        }
        if (!(flags & VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT)) {
            skip |= LogError(
                device, "VUID-VkPipelineShaderStageModuleIdentifierCreateInfoEXT-pNext-06851",
                "%s module (stage %s) VkPipelineShaderStageCreateInfo has a VkPipelineShaderStageModuleIdentifierCreateInfoEXT "
                "struct in the pNext chain whose identifierSize is > 0 (%" PRIu32
                "), but the "
                "VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT bit is not set in the pipeline create flags",
                report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                string_VkShaderStageFlagBits(stage_state.stage_flag), module_identifier->identifierSize);
        }
        if (module_identifier->identifierSize > VK_MAX_SHADER_MODULE_IDENTIFIER_SIZE_EXT) {
            skip |= LogError(
                device, "VUID-VkPipelineShaderStageModuleIdentifierCreateInfoEXT-identifierSize-06852",
                "%s module (stage %s) VkPipelineShaderStageCreateInfo has a VkPipelineShaderStageModuleIdentifierCreateInfoEXT "
                "struct in the pNext chain whose identifierSize (%" PRIu32
                ") is > VK_MAX_SHADER_MODULE_IDENTIFIER_SIZE_EXT (%" PRIu32 ")",
                report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                string_VkShaderStageFlagBits(stage_state.stage_flag), module_identifier->identifierSize,
                VK_MAX_SHADER_MODULE_IDENTIFIER_SIZE_EXT);
        }
    }
    if (module_identifier && module_create_info) {
        skip |= LogError(
            device, "VUID-VkPipelineShaderStageCreateInfo-stage-06844",
            "%s module (stage %s) VkPipelineShaderStageCreateInfo has both a VkPipelineShaderStageModuleIdentifierCreateInfoEXT "
            "struct and a VkShaderModuleCreateInfo struct in the pNext chain",
            report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
            string_VkShaderStageFlagBits(stage_state.stage_flag));
    }
    if (enabled_features.graphics_pipeline_library_features.graphicsPipelineLibrary) {
        if (!module_identifier && pStage->module == VK_NULL_HANDLE && !module_create_info) {
            skip |= LogError(
                device, "VUID-VkPipelineShaderStageCreateInfo-stage-06845",
                "%s module (stage %s) VkPipelineShaderStageCreateInfo has no VkPipelineShaderStageModuleIdentifierCreateInfoEXT "
                "struct and no VkShaderModuleCreateInfo struct in the pNext chain, and module is not a valid VkShaderModule",
                report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                string_VkShaderStageFlagBits(stage_state.stage_flag));
        }
    } else {
        if (!module_identifier && pStage->module == VK_NULL_HANDLE) {
            const char *vuid = IsExtEnabled(device_extensions.vk_khr_pipeline_library)
                                   ? "VUID-VkPipelineShaderStageCreateInfo-stage-06846"
                                   : "VUID-VkPipelineShaderStageCreateInfo-stage-06847";
            skip |= LogError(
                device, vuid,
                "%s module (stage %s) VkPipelineShaderStageCreateInfo has no VkPipelineShaderStageModuleIdentifierCreateInfoEXT "
                "struct in the pNext chain, the graphicsPipelineLibrary feature is not enabled, and module is not a valid "
                "VkShaderModule",
                report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                string_VkShaderStageFlagBits(stage_state.stage_flag));
        }
    }
    if (module_identifier && pStage->module != VK_NULL_HANDLE) {
        skip |= LogError(
            device, "VUID-VkPipelineShaderStageCreateInfo-stage-06848",
            "%s module (stage %s) VkPipelineShaderStageCreateInfo has a VkPipelineShaderStageModuleIdentifierCreateInfoEXT "
            "struct in the pNext chain, and module is not VK_NULL_HANDLE",
            report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
            string_VkShaderStageFlagBits(stage_state.stage_flag));
    }
    return skip;
}

// Temporary data of a OpVariable when validating it.
// If found useful in another location, can move out to the header
struct VariableInstInfo {
    bool has_8bit = false;
    bool has_16bit = false;
};

// easier to use recursion to traverse the OpTypeStruct
static void GetVariableInfo(const SHADER_MODULE_STATE &module_state, const spirv_inst_iter &insn, VariableInstInfo &info) {
    if (insn == module_state.end()) {
        return;
    } else if (insn.opcode() == spv::OpTypeFloat || insn.opcode() == spv::OpTypeInt) {
        const uint32_t bit_width = insn.word(2);
        info.has_8bit |= (bit_width == 8);
        info.has_16bit |= (bit_width == 16);
    } else if (insn.opcode() == spv::OpTypeStruct) {
        for (uint32_t i = 2; i < insn.len(); i++) {
            const auto &base_insn = GetBaseTypeIter(module_state, insn.word(i));
            GetVariableInfo(module_state, base_insn, info);
        }
    }
}

bool CoreChecks::ValidateVariables(const SHADER_MODULE_STATE &module_state) const {
    bool skip = false;

    for (auto insn : module_state.static_data_.variable_inst) {
        const uint32_t storage_class = insn.word(3);

        if (storage_class == spv::StorageClassWorkgroup) {
            // If Workgroup variable is initalized, make sure the feature is enabled
            if (insn.len() > 4 &&
                !enabled_features.core13.shaderZeroInitializeWorkgroupMemory) {
                const char *vuid = IsExtEnabled(device_extensions.vk_khr_zero_initialize_workgroup_memory)
                                       ? "VUID-RuntimeSpirv-shaderZeroInitializeWorkgroupMemory-06372"
                                       : "VUID-RuntimeSpirv-OpVariable-06373";
                skip |= LogError(
                    module_state.vk_shader_module(), vuid,
                    "vkCreateShaderModule(): "
                    "VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeaturesKHR::shaderZeroInitializeWorkgroupMemory is not enabled, "
                    "but shader contains an OpVariable with Workgroup Storage Class with an Initializer operand.\n%s",
                    module_state.DescribeInstruction(insn).c_str());
            }
        }

        const auto type_pointer = module_state.get_def(insn.word(1));
        const auto type = module_state.get_def(type_pointer.word(3));
        // type will either be a float, int, or struct and if struct need to traverse it
        VariableInstInfo info;
        GetVariableInfo(module_state, type, info);

        if (info.has_8bit) {
           if (!enabled_features.core12.storageBuffer8BitAccess &&
             (storage_class == spv::StorageClassStorageBuffer ||  storage_class == spv::StorageClassShaderRecordBufferKHR || storage_class == spv::StorageClassPhysicalStorageBuffer)) {
               skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-storageBuffer8BitAccess-06328",
                                "vkCreateShaderModule(): storageBuffer8BitAccess is not enabled, but shader contains an 8-bit "
                                "OpVariable with %s Storage Class.\n%s",
                                string_SpvStorageClass(storage_class), module_state.DescribeInstruction(insn).c_str());
           }
           if (!enabled_features.core12.uniformAndStorageBuffer8BitAccess && storage_class == spv::StorageClassUniform) {
               skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-uniformAndStorageBuffer8BitAccess-06329",
                                "vkCreateShaderModule(): uniformAndStorageBuffer8BitAccess is not enabled, but shader contains an "
                                "8-bit OpVariable with Uniform Storage Class.\n%s",
                                module_state.DescribeInstruction(insn).c_str());
           }
           if (!enabled_features.core12.storagePushConstant8 && storage_class == spv::StorageClassPushConstant) {
               skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-storagePushConstant8-06330",
                                "vkCreateShaderModule(): storagePushConstant8 is not enabled, but shader contains an 8-bit "
                                "OpVariable with PushConstant Storage Class.\n%s",
                                module_state.DescribeInstruction(insn).c_str());
           }
        }

        if (info.has_16bit) {
          if (!enabled_features.core11.storageBuffer16BitAccess &&
             (storage_class == spv::StorageClassStorageBuffer ||  storage_class == spv::StorageClassShaderRecordBufferKHR || storage_class == spv::StorageClassPhysicalStorageBuffer)) {
              skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-storageBuffer16BitAccess-06331",
                               "vkCreateShaderModule(): storageBuffer16BitAccess is not enabled, but shader contains an 16-bit "
                               "OpVariable with %s Storage Class.\n%s",
                               string_SpvStorageClass(storage_class), module_state.DescribeInstruction(insn).c_str());
           }
           if (!enabled_features.core11.uniformAndStorageBuffer16BitAccess && storage_class == spv::StorageClassUniform) {
               skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-uniformAndStorageBuffer16BitAccess-06332",
                                "vkCreateShaderModule(): uniformAndStorageBuffer16BitAccess is not enabled, but shader contains an "
                                "16-bit OpVariable with Uniform Storage Class.\n%s",
                                module_state.DescribeInstruction(insn).c_str());
           }
           if (!enabled_features.core11.storagePushConstant16 && storage_class == spv::StorageClassPushConstant) {
               skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-storagePushConstant16-06333",
                                "vkCreateShaderModule(): storagePushConstant16 is not enabled, but shader contains an 16-bit "
                                "OpVariable with PushConstant Storage Class.\n%s",
                                module_state.DescribeInstruction(insn).c_str());
           }
           if (!enabled_features.core11.storageInputOutput16 &&
             (storage_class == spv::StorageClassInput || storage_class == spv::StorageClassOutput)) {
               skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-storageInputOutput16-06334",
                                "vkCreateShaderModule(): storageInputOutput16 is not enabled, but shader contains an 16-bit "
                                "OpVariable with %s Storage Class.\n%s",
                                string_SpvStorageClass(storage_class), module_state.DescribeInstruction(insn).c_str());
           }
        }

        // Checks based off shaderStorageImage(Read|Write)WithoutFormat are
        // disabled if VK_KHR_format_feature_flags2 is supported.
        //
        //   https://github.com/KhronosGroup/Vulkan-Docs/blob/6177645341afc/appendices/spirvenv.txt#L553
        //
        // The other checks need to take into account the format features and so
        // we apply that in the descriptor set matching validation code (see
        // descriptor_sets.cpp).
        if (!has_format_feature2) {
            skip |= ValidateShaderStorageImageFormatsVariables(module_state, insn);
        }
    }

    return skip;
}

bool CoreChecks::ValidateTransformFeedback(const SHADER_MODULE_STATE &module_state) const {
    bool skip = false;

    // Temp workaround to prevent false positive errors
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/2450
    if (module_state.HasMultipleEntryPoints()) {
        return skip;
    }

    layer_data::unordered_set<uint32_t> emitted_streams;
    bool output_points = false;
    for (const auto &insn : module_state) {
        const uint32_t opcode = insn.opcode();
        if (opcode == spv::OpEmitStreamVertex) {
            emitted_streams.emplace(static_cast<uint32_t>(module_state.GetConstantValueById(insn.word(1))));
        }
        if (opcode == spv::OpEmitStreamVertex || opcode == spv::OpEndStreamPrimitive) {
            uint32_t stream = static_cast<uint32_t>(module_state.GetConstantValueById(insn.word(1)));
            if (stream >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams) {
                skip |= LogError(
                    module_state.vk_shader_module(), "VUID-RuntimeSpirv-OpEmitStreamVertex-06310",
                    "vkCreateGraphicsPipelines(): shader uses transform feedback stream\n%s\nwith index %" PRIu32
                    ", which is not less than VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackStreams (%" PRIu32
                    ").",
                    module_state.DescribeInstruction(insn).c_str(), stream,
                    phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams);
            }
        }
        if ((opcode == spv::OpExecutionMode || opcode == spv::OpExecutionModeId) &&
            insn.word(2) == spv::ExecutionModeOutputPoints) {
            output_points = true;
        }
    }

    const uint32_t emitted_streams_size = static_cast<uint32_t>(emitted_streams.size());
    if (emitted_streams_size > 1 && !output_points &&
        phys_dev_ext_props.transform_feedback_props.transformFeedbackStreamsLinesTriangles == VK_FALSE) {
        skip |=
            LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-transformFeedbackStreamsLinesTriangles-06311",
                     "vkCreateGraphicsPipelines(): shader emits to %" PRIu32
                     " vertex streams and VkPhysicalDeviceTransformFeedbackPropertiesEXT::transformFeedbackStreamsLinesTriangles "
                     "is VK_FALSE, but execution mode is not OutputPoints.",
                     emitted_streams_size);
    }

    return skip;
}

// Checks for both TexelOffset and TexelGatherOffset limits
bool CoreChecks::ValidateTexelOffsetLimits(const SHADER_MODULE_STATE &module_state, spirv_inst_iter &insn) const {
    bool skip = false;

    const uint32_t opcode = insn.opcode();
    if (ImageGatherOperation(opcode) || ImageSampleOperation(opcode) || ImageFetchOperation(opcode)) {
        uint32_t image_operand_position = OpcodeImageOperandsPosition(opcode);
        // Image operands can be optional
        if (image_operand_position != 0 && insn.len() > image_operand_position) {
            auto image_operand = insn.word(image_operand_position);
            // Bits we are validating (sample/fetch only check ConstOffset)
            uint32_t offset_bits =
                ImageGatherOperation(opcode)
                    ? (spv::ImageOperandsOffsetMask | spv::ImageOperandsConstOffsetMask | spv::ImageOperandsConstOffsetsMask)
                    : (spv::ImageOperandsConstOffsetMask);
            if (image_operand & (offset_bits)) {
                // Operand values follow
                uint32_t index = image_operand_position + 1;
                // Each bit has it's own operand, starts with the smallest set bit and loop to the highest bit among
                // ImageOperandsOffsetMask, ImageOperandsConstOffsetMask and ImageOperandsConstOffsetsMask
                for (uint32_t i = 1; i < spv::ImageOperandsConstOffsetsMask; i <<= 1) {
                    if (image_operand & i) {  // If the bit is set, consume operand
                        if (insn.len() > index && (i & offset_bits)) {
                            uint32_t constant_id = insn.word(index);
                            const auto &constant = module_state.get_def(constant_id);
                            const bool is_dynamic_offset = constant == module_state.end();
                            if (!is_dynamic_offset && constant.opcode() == spv::OpConstantComposite) {
                                for (uint32_t j = 3; j < constant.len(); ++j) {
                                    uint32_t comp_id = constant.word(j);
                                    const auto &comp = module_state.get_def(comp_id);
                                    const auto &comp_type = module_state.get_def(comp.word(1));
                                    // Get operand value
                                    const uint32_t offset = comp.word(3);
                                    // spec requires minTexelGatherOffset/minTexelOffset to be -8 or less so never can compare if
                                    // unsigned spec requires maxTexelGatherOffset/maxTexelOffset to be 7 or greater so never can
                                    // compare if signed is less then zero
                                    const int32_t signed_offset = static_cast<int32_t>(offset);
                                    const bool use_signed = (comp_type.opcode() == spv::OpTypeInt && comp_type.word(3) != 0);

                                    // There are 2 sets of VU being covered where the only main difference is the opcode
                                    if (ImageGatherOperation(opcode)) {
                                        // min/maxTexelGatherOffset
                                        if (use_signed && (signed_offset < phys_dev_props.limits.minTexelGatherOffset)) {
                                            skip |=
                                                LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-OpImage-06376",
                                                         "vkCreateShaderModule(): Shader uses\n%s\nwith offset (%" PRIi32
                                                         ") less than VkPhysicalDeviceLimits::minTexelGatherOffset (%" PRIi32 ").",
                                                         module_state.DescribeInstruction(insn).c_str(), signed_offset,
                                                         phys_dev_props.limits.minTexelGatherOffset);
                                        } else if ((offset > phys_dev_props.limits.maxTexelGatherOffset) &&
                                                   (!use_signed || (use_signed && signed_offset > 0))) {
                                            skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-OpImage-06377",
                                                             "vkCreateShaderModule(): Shader uses\n%s\nwith offset (%" PRIu32
                                                             ") greater than VkPhysicalDeviceLimits::maxTexelGatherOffset (%" PRIu32
                                                             ").",
                                                             module_state.DescribeInstruction(insn).c_str(), offset,
                                                             phys_dev_props.limits.maxTexelGatherOffset);
                                        }
                                    } else {
                                        // min/maxTexelOffset
                                        if (use_signed && (signed_offset < phys_dev_props.limits.minTexelOffset)) {
                                            skip |=
                                                LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-OpImageSample-06435",
                                                         "vkCreateShaderModule(): Shader uses\n%s\nwith offset (%" PRIi32
                                                         ") less than VkPhysicalDeviceLimits::minTexelOffset (%" PRIi32 ").",
                                                         module_state.DescribeInstruction(insn).c_str(), signed_offset,
                                                         phys_dev_props.limits.minTexelOffset);
                                        } else if ((offset > phys_dev_props.limits.maxTexelOffset) &&
                                                   (!use_signed || (use_signed && signed_offset > 0))) {
                                            skip |=
                                                LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-OpImageSample-06436",
                                                         "vkCreateShaderModule(): Shader uses\n%s\nwith offset (%" PRIu32
                                                         ") greater than VkPhysicalDeviceLimits::maxTexelOffset (%" PRIu32 ").",
                                                         module_state.DescribeInstruction(insn).c_str(), offset,
                                                         phys_dev_props.limits.maxTexelOffset);
                                        }
                                    }
                                }
                            }
                        }
                        index += ImageOperandsParamCount(i);
                    }
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateShaderClock(const SHADER_MODULE_STATE &module_state, spirv_inst_iter &insn) const {
    bool skip = false;

    switch (insn.opcode()) {
        case spv::OpReadClockKHR: {
            auto scope_id = module_state.get_def(insn.word(3));
            auto scope_type = scope_id.word(3);
            // if scope isn't Subgroup or Device, spirv-val will catch
            if ((scope_type == spv::ScopeSubgroup) && (enabled_features.shader_clock_features.shaderSubgroupClock == VK_FALSE)) {
                skip |= LogError(device, "VUID-RuntimeSpirv-shaderSubgroupClock-06267",
                                 "%s: OpReadClockKHR is used with a Subgroup scope but shaderSubgroupClock was not enabled.\n%s",
                                 report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                                 module_state.DescribeInstruction(insn).c_str());
            } else if ((scope_type == spv::ScopeDevice) && (enabled_features.shader_clock_features.shaderDeviceClock == VK_FALSE)) {
                skip |= LogError(device, "VUID-RuntimeSpirv-shaderDeviceClock-06268",
                                 "%s: OpReadClockKHR is used with a Device scope but shaderDeviceClock was not enabled.\n%s",
                                 report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                                 module_state.DescribeInstruction(insn).c_str());
            }
            break;
        }
    }
    return skip;
}

bool CoreChecks::ValidateImageWrite(const SHADER_MODULE_STATE &module_state, spirv_inst_iter &insn) const {
    bool skip = false;

    if (insn.opcode() == spv::OpImageWrite) {
        // guaranteed by spirv-val to be an OpTypeImage
        const uint32_t image = module_state.GetTypeId(insn.word(1));
        const spirv_inst_iter image_def = module_state.get_def(image);
        const uint32_t image_format = image_def.word(8);
        // If format is 'Unknown' then need to wait until a descriptor is bound to it
        if (image_format != spv::ImageFormatUnknown) {
            const VkFormat compatible_format = CompatibleSpirvImageFormat(image_format);
            if (compatible_format != VK_FORMAT_UNDEFINED) {
                const uint32_t format_component_count = FormatComponentCount(compatible_format);
                const spirv_inst_iter texel_def = module_state.get_def(insn.word(3));
                const spirv_inst_iter texel_type = module_state.get_def(texel_def.word(1));
                const uint32_t texel_component_count = (texel_type.opcode() == spv::OpTypeVector) ? texel_type.word(3) : 1;
                if (texel_component_count < format_component_count) {
                    skip |= LogError(device, " VUID-RuntimeSpirv-OpImageWrite-07112",
                                     "%s: OpImageWrite Texel operand only contains %" PRIu32
                                     " components, but the OpImage format mapping to %s has %" PRIu32 " components.\n%s\n%s",
                                     report_data->FormatHandle(module_state.vk_shader_module()).c_str(), texel_component_count,
                                     string_VkFormat(compatible_format), format_component_count,
                                     module_state.DescribeInstruction(insn).c_str(),
                                     module_state.DescribeInstruction(image_def).c_str());
                }
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidatePipelineShaderStage(const PIPELINE_STATE *pipeline, const PipelineStageState &stage_state,
                                             bool check_point_size) const {
    bool skip = false;
    const auto *pStage = stage_state.create_info;
    const SHADER_MODULE_STATE &module_state = *stage_state.module_state.get();
    const auto &entrypoint = stage_state.entrypoint;

    skip |= ValidateShaderModuleId(module_state, stage_state, pStage, pipeline->GetPipelineCreateFlags());

    if (module_state.vk_shader_module() == VK_NULL_HANDLE) return skip;  // No real shader for further validation

    // to prevent const_cast on pipeline object, just store here as not needed outside function anyway
    uint32_t local_size_x = 0;
    uint32_t local_size_y = 0;
    uint32_t local_size_z = 0;
    uint32_t total_shared_size = 0;

    // Check the module
    if (!module_state.has_valid_spirv) {
        skip |= LogError(device, "VUID-VkPipelineShaderStageCreateInfo-module-parameter",
                         "%s does not contain valid spirv for stage %s.",
                         report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                         string_VkShaderStageFlagBits(stage_state.stage_flag));
    }

    // If specialization-constant instructions are present in the shader, the specializations should be applied.
    if (module_state.HasSpecConstants()) {
        // both spirv-opt and spirv-val will use the same flags
        spvtools::ValidatorOptions options;
        AdjustValidatorOptions(device_extensions, enabled_features, options);

        // setup the call back if the optimizer fails
        spv_target_env spirv_environment = PickSpirvEnv(api_version, IsExtEnabled(device_extensions.vk_khr_spirv_1_4));
        spvtools::Optimizer optimizer(spirv_environment);
        spvtools::MessageConsumer consumer = [&skip, &module_state, &stage_state, this](
                                                 spv_message_level_t level, const char *source, const spv_position_t &position,
                                                 const char *message) {
            skip |= LogError(device, "VUID-VkPipelineShaderStageCreateInfo-module-parameter",
                             "%s does not contain valid spirv for stage %s. %s",
                             report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                             string_VkShaderStageFlagBits(stage_state.stage_flag), message);
        };
        optimizer.SetMessageConsumer(consumer);

        // The app might be using the default spec constant values, but if they pass values at runtime to the pipeline then need to
        // use those values to apply to the spec constants
        if (pStage->pSpecializationInfo != nullptr && pStage->pSpecializationInfo->mapEntryCount > 0 &&
            pStage->pSpecializationInfo->pMapEntries != nullptr) {
            // Gather the specialization-constant values.
            auto const &specialization_info = pStage->pSpecializationInfo;
            auto const &specialization_data = reinterpret_cast<uint8_t const *>(specialization_info->pData);
            std::unordered_map<uint32_t, std::vector<uint32_t>> id_value_map;  // note: this must be std:: to work with spvtools
            id_value_map.reserve(specialization_info->mapEntryCount);
            for (auto i = 0u; i < specialization_info->mapEntryCount; ++i) {
                auto const &map_entry = specialization_info->pMapEntries[i];
                const auto itr = module_state.GetSpecConstMap().find(map_entry.constantID);
                // "If a constantID value is not a specialization constant ID used in the shader, that map entry does not affect the
                // behavior of the pipeline."
                if (itr != module_state.GetSpecConstMap().cend()) {
                    // Make sure map_entry.size matches the spec constant's size
                    uint32_t spec_const_size = decoration_set::kInvalidValue;
                    const auto def_ins = module_state.get_def(itr->second);
                    const auto type_ins = module_state.get_def(def_ins.word(1));
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
                            // spirv-val should catch if SpecId is not used on a
                            // OpSpecConstantTrue/OpSpecConstantFalse/OpSpecConstant and OpSpecConstant is validated to be a
                            // OpTypeInt or OpTypeFloat
                            break;
                    }

                    if (map_entry.size != spec_const_size) {
                        skip |= LogError(device, "VUID-VkSpecializationMapEntry-constantID-00776",
                                         "Specialization constant (ID = %" PRIu32 ", entry = %" PRIu32
                                         ") has invalid size %zu in shader module %s. Expected size is %" PRIu32
                                         " from shader definition.",
                                         map_entry.constantID, i, map_entry.size,
                                         report_data->FormatHandle(module_state.vk_shader_module()).c_str(), spec_const_size);
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

            // This pass takes the runtime spec const values and applies it into the SPIR-V
            // will turn a spec constant like
            //     OpSpecConstant %uint 1
            // to a use the value passed in instead (for example if the value is 32) so now it looks like
            //     OpSpecConstant %uint 32
            optimizer.RegisterPass(spvtools::CreateSetSpecConstantDefaultValuePass(id_value_map));
        }

        // This pass will turn OpSpecConstant into a OpConstant (also OpSpecConstantTrue/OpSpecConstantFalse)
        optimizer.RegisterPass(spvtools::CreateFreezeSpecConstantValuePass());
        // Using the new frozen OpConstant all OpSpecConstantComposite can be resolved turning them into OpConstantComposite
        // This is need incase a shdaer looks like:
        //
        //     layout(constant_id = 0) const uint x = 64;
        //     shared uint arr[x > 64 ? 64 : x];
        //
        // this will generate branch/switch statements that we want to leverage spirv-opt to apply to make parsing easier
        optimizer.RegisterPass(spvtools::CreateFoldSpecConstantOpAndCompositePass());
        // Currently need to re-run the pass as spirv-opt has a bug and not folding everything sometimes
        // https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/4399#issuecomment-1216203563
        optimizer.RegisterPass(spvtools::CreateFoldSpecConstantOpAndCompositePass());

        // Apply the specialization-constant values and revalidate the shader module is valid.
        const char *pSpecializationInfo_vuid = IsExtEnabled(device_extensions.vk_ext_shader_module_identifier)
                               ? "VUID-VkPipelineShaderStageCreateInfo-pSpecializationInfo-06849"
                               : "VUID-VkPipelineShaderStageCreateInfo-pSpecializationInfo-06719";
        std::vector<uint32_t> specialized_spirv;
        auto const optimized =
            optimizer.Run(module_state.words.data(), module_state.words.size(), &specialized_spirv, options, true);
        if (optimized) {
            spv_context ctx = spvContextCreate(spirv_environment);
            spv_const_binary_t binary{specialized_spirv.data(), specialized_spirv.size()};
            spv_diagnostic diag = nullptr;
            auto const spv_valid = spvValidateWithOptions(ctx, options, &binary, &diag);
            if (spv_valid != SPV_SUCCESS) {
                skip |= LogError(device, pSpecializationInfo_vuid,
                                 "After specialization was applied, %s does not contain valid spirv for stage %s.",
                                 report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                                 string_VkShaderStageFlagBits(stage_state.stage_flag));
            }

            // The new optimized SPIR-V will NOT match the original SHADER_MODULE_STATE object parsing, so a new SHADER_MODULE_STATE
            // object is needed. This an issue due to each pipeline being able to reuse the same shader module but with different
            // spec constant values.
            SHADER_MODULE_STATE spec_mod(specialized_spirv);

            // According to https://github.com/KhronosGroup/Vulkan-Docs/issues/1671 anything labeled as "static use" (such as if an
            // input is used or not) don't have to be checked post spec constants freezing since the device compiler is not
            // guaranteed to run things such as dead-code elimination. The following checks are things that don't follow under
            // "static use" rules and need to be validated still.
            auto specialized_it = spec_mod.begin();

            // see ValidateComputeSharedMemory() for details why we might track max block size
            layer_data::unordered_set<uint32_t> aliased_id;
            bool find_max_block = false;

            uint32_t workgroup_size_id = 0;  // result id can't be zero
            uint32_t local_size_id_x = 0;
            uint32_t local_size_id_y = 0;
            uint32_t local_size_id_z = 0;

            // make single interation through new shader
            while (specialized_it != spec_mod.end()) {
                const uint32_t opcode = specialized_it.opcode();

                if (opcode == spv::OpExecutionModeId && specialized_it.word(2) == spv::ExecutionModeLocalSizeId) {
                    local_size_id_x = specialized_it.word(3);
                    local_size_id_y = specialized_it.word(4);
                    local_size_id_z = specialized_it.word(5);
                }

                if (opcode == spv::OpDecorate) {
                    // Validate applied WorkgroupSize is still below maxComputeWorkGroupSize limit
                    if (specialized_it.word(2) == spv::DecorationBuiltIn && specialized_it.word(3) == spv::BuiltInWorkgroupSize) {
                        // Will be a OpConstantComposite and always have the OpDecorate section
                        workgroup_size_id = specialized_it.word(1);
                    }
                    if (specialized_it.word(2) == spv::DecorationAliased) {
                        aliased_id.emplace(specialized_it.word(1));
                    }
                }

                if (opcode == spv::OpConstantComposite && workgroup_size_id == specialized_it.word(2)) {
                    // VUID-WorkgroupSize-WorkgroupSize-04427 makes sure this is a OpTypeVector of int32 so this can be assuemd
                    local_size_x = spec_mod.get_def(specialized_it.word(3)).word(3);
                    local_size_y = spec_mod.get_def(specialized_it.word(4)).word(3);
                    local_size_z = spec_mod.get_def(specialized_it.word(5)).word(3);
                }

                if (opcode == spv::OpVariable && specialized_it.word(3) == spv::StorageClassWorkgroup) {
                    if (aliased_id.find(specialized_it.word(2)) != aliased_id.end()) {
                        find_max_block = true;
                    }

                    const uint32_t result_type_id = specialized_it.word(1);
                    const auto result_type = spec_mod.get_def(result_type_id);
                    const auto type = spec_mod.get_def(result_type.word(3));
                    const uint32_t variable_shared_size = spec_mod.GetTypeBitsSize(type) / 8;

                    if (find_max_block) {
                        total_shared_size = std::max(total_shared_size, variable_shared_size);
                    } else {
                        total_shared_size += variable_shared_size;
                    }
                }

                ++specialized_it;
            }

            // if after no WorkgroupSize is found, then can apply any possible LocalSizeId due to precedence order
            if (local_size_x == 0 && local_size_id_x != 0) {
                local_size_x = spec_mod.get_def(local_size_id_x).word(3);
                local_size_y = spec_mod.get_def(local_size_id_y).word(3);
                local_size_z = spec_mod.get_def(local_size_id_z).word(3);
            }

            spvDiagnosticDestroy(diag);
            spvContextDestroy(ctx);
        } else {
            // Should never get here, but better then asserting
            skip |= LogError(device, pSpecializationInfo_vuid,
                             "%s module (stage %s) attempted to apply specialization constants with spirv-opt but failed.",
                             report_data->FormatHandle(module_state.vk_shader_module()).c_str(),
                             string_VkShaderStageFlagBits(stage_state.stage_flag));
        }
    }

    // Check the entrypoint
    if (entrypoint == module_state.end()) {
        skip |= LogError(device, "VUID-VkPipelineShaderStageCreateInfo-pName-00707", "No entrypoint found named `%s` for stage %s.",
                         pStage->pName, string_VkShaderStageFlagBits(stage_state.stage_flag));
    }
    if (skip) return true;  // no point continuing beyond here, any analysis is just going to be garbage.

    // Mark accessible ids
    auto &accessible_ids = stage_state.accessible_ids;

    // Validate descriptor set layout against what the entrypoint actually uses

    // The following tries to limit the number of passes through the shader module. The validation passes in here are "stateless"
    // and mainly only checking the instruction in detail for a single operation
    for (auto insn : module_state) {
        skip |= ValidateTexelOffsetLimits(module_state, insn);
        skip |= ValidateShaderCapabilitiesAndExtensions(insn);
        skip |= ValidateShaderClock(module_state, insn);
        skip |= ValidateShaderStageGroupNonUniform(module_state, pStage->stage, insn);
        skip |= ValidateMemoryScope(module_state, insn);
        skip |= ValidateImageWrite(module_state, insn);
    }

    skip |= ValidateTransformFeedback(module_state);
    skip |= ValidateShaderStageWritableOrAtomicDescriptor(module_state, pStage->stage, stage_state.has_writable_descriptor,
                                                          stage_state.has_atomic_descriptor);
    skip |= ValidateShaderStageInputOutputLimits(module_state, pStage, pipeline, entrypoint);
    skip |= ValidateShaderStageMaxResources(module_state, pStage->stage, pipeline);
    skip |= ValidateAtomicsTypes(module_state);
    skip |= ValidateExecutionModes(module_state, entrypoint, pStage->stage, pipeline);
    skip |= ValidateSpecializations(module_state, pStage);
    skip |= ValidateDecorations(module_state);
    skip |= ValidateVariables(module_state);
    const auto *raster_state = pipeline->RasterizationState();
    if (check_point_size && raster_state && !raster_state->rasterizerDiscardEnable) {
        skip |= ValidatePointListShaderState(pipeline, module_state, entrypoint, pStage->stage);
    }
    skip |= ValidateBuiltinLimits(module_state, entrypoint);
    if (enabled_features.cooperative_matrix_features.cooperativeMatrix) {
        skip |= ValidateCooperativeMatrix(module_state, pStage, pipeline);
    }
    if (enabled_features.fragment_shading_rate_features.primitiveFragmentShadingRate) {
        skip |= ValidatePrimitiveRateShaderState(pipeline, module_state, entrypoint, pStage->stage);
    }
    if (IsExtEnabled(device_extensions.vk_qcom_render_pass_shader_resolve)) {
        skip |= ValidateShaderResolveQCOM(module_state, pStage, pipeline);
    }
    if (IsExtEnabled(device_extensions.vk_ext_subgroup_size_control)) {
        skip |= ValidateShaderSubgroupSizeControl(module_state, pStage);
    }

    // "layout must be consistent with the layout of the * shader"
    // 'consistent' -> #descriptorsets-pipelinelayout-consistency
    std::string vuid_layout_mismatch;
    switch (pipeline->GetCreateInfoSType()) {
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
    skip |= ValidatePushConstantUsage(*pipeline, module_state, pStage, vuid_layout_mismatch);

    // Validate descriptor use
    for (auto use : stage_state.descriptor_uses) {
        // Verify given pipelineLayout has requested setLayout with requested binding
        // const auto& layout_state = (stage_state.stage_flag == VK_SHADER_STAGE_VERTEX_BIT) ?
        // pipeline->PreRasterPipelineLayoutState() : pipeline->FragmentShaderPipelineLayoutState();
        const auto &binding = GetDescriptorBinding(pipeline->PipelineLayoutState().get(), use.first);
        unsigned required_descriptor_count;
        bool is_khr = binding && binding->descriptorType == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        std::set<uint32_t> descriptor_types =
            TypeToDescriptorTypeSet(module_state, use.second.type_id, required_descriptor_count, is_khr);

        if (!binding) {
            LogObjectList objlist(module_state.vk_shader_module());
            objlist.add(pipeline->PipelineLayoutState()->layout());
            skip |= LogError(objlist, vuid_layout_mismatch,
                             "Shader uses descriptor slot %u.%u (expected `%s`) but not declared in pipeline layout", use.first.set,
                             use.first.binding, string_descriptorTypes(descriptor_types).c_str());
        } else if (~binding->stageFlags & pStage->stage) {
            LogObjectList objlist(module_state.vk_shader_module());
            objlist.add(pipeline->PipelineLayoutState()->layout());
            skip |= LogError(objlist, vuid_layout_mismatch,
                             "Shader uses descriptor slot %u.%u but descriptor not accessible from stage %s", use.first.set,
                             use.first.binding, string_VkShaderStageFlagBits(pStage->stage));
        } else if ((binding->descriptorType != VK_DESCRIPTOR_TYPE_MUTABLE_EXT) &&
                   (descriptor_types.find(binding->descriptorType) == descriptor_types.end())) {
            LogObjectList objlist(module_state.vk_shader_module());
            objlist.add(pipeline->PipelineLayoutState()->layout());
            skip |= LogError(objlist, vuid_layout_mismatch,
                             "Type mismatch on descriptor slot %u.%u (expected `%s`) but descriptor of type %s", use.first.set,
                             use.first.binding, string_descriptorTypes(descriptor_types).c_str(),
                             string_VkDescriptorType(binding->descriptorType));
        } else if (binding->descriptorCount < required_descriptor_count) {
            LogObjectList objlist(module_state.vk_shader_module());
            objlist.add(pipeline->PipelineLayoutState()->layout());
            skip |= LogError(objlist, vuid_layout_mismatch,
                             "Shader expects at least %u descriptors for binding %u.%u but only %u provided",
                             required_descriptor_count, use.first.set, use.first.binding, binding->descriptorCount);
        }
    }

    // Validate use of input attachments against subpass structure
    if (pStage->stage == VK_SHADER_STAGE_FRAGMENT_BIT) {
        auto input_attachment_uses = module_state.CollectInterfaceByInputAttachmentIndex(accessible_ids);

        const auto &rp_state = pipeline->RenderPassState();
        if (rp_state && !rp_state->UsesDynamicRendering()) {
            auto rpci = rp_state->createInfo.ptr();
            auto subpass = pipeline->Subpass();
            for (auto use : input_attachment_uses) {
                auto input_attachments = rpci->pSubpasses[subpass].pInputAttachments;
                auto index = (input_attachments && use.first < rpci->pSubpasses[subpass].inputAttachmentCount)
                    ? input_attachments[use.first].attachment
                    : VK_ATTACHMENT_UNUSED;

                if (index == VK_ATTACHMENT_UNUSED) {
                    LogObjectList objlist(module_state.vk_shader_module());
                    objlist.add(pipeline->PipelineLayoutState()->layout());
                    skip |= LogError(objlist, kVUID_Core_Shader_MissingInputAttachment,
                                     "Shader consumes input attachment index %d but not provided in subpass", use.first);
                } else if (!(GetFormatType(rpci->pAttachments[index].format) &
                             module_state.GetFundamentalType(use.second.type_id))) {
                    LogObjectList objlist(module_state.vk_shader_module());
                    objlist.add(pipeline->PipelineLayoutState()->layout());
                    skip |= LogError(objlist, kVUID_Core_Shader_InputAttachmentTypeMismatch,
                                     "Subpass input attachment %u format of %s does not match type used in shader `%s`", use.first,
                                     string_VkFormat(rpci->pAttachments[index].format),
                                     module_state.DescribeType(use.second.type_id).c_str());
                }
            }
        }
    }
    if (pStage->stage == VK_SHADER_STAGE_COMPUTE_BIT) {
        skip |= ValidateComputeWorkGroupSizes(module_state, entrypoint, stage_state, local_size_x, local_size_y, local_size_z);
        skip |= ValidateComputeSharedMemory(module_state, total_shared_size);
    }

    return skip;
}

bool CoreChecks::ValidateInterfaceBetweenStages(const SHADER_MODULE_STATE &producer, spirv_inst_iter producer_entrypoint,
                                                shader_stage_attributes const *producer_stage, const SHADER_MODULE_STATE &consumer,
                                                spirv_inst_iter consumer_entrypoint,
                                                shader_stage_attributes const *consumer_stage) const {
    bool skip = false;

    auto outputs =
        producer.CollectInterfaceByLocation(producer_entrypoint, spv::StorageClassOutput, producer_stage->arrayed_output);
    auto inputs = consumer.CollectInterfaceByLocation(consumer_entrypoint, spv::StorageClassInput, consumer_stage->arrayed_input);

    auto output_it = outputs.begin();
    auto input_it = inputs.begin();

    uint32_t output_component = 0;
    uint32_t input_component = 0;

    // Maps sorted by key (location); walk them together to find mismatches
    while ((outputs.size() > 0 && output_it != outputs.end()) || (inputs.size() && input_it != inputs.end())) {
        bool output_at_end = outputs.size() == 0 || output_it == outputs.end();
        bool input_at_end = inputs.size() == 0 || input_it == inputs.end();
        auto output_first = output_at_end ? std::make_pair(0u, 0u) : output_it->first;
        auto input_first = input_at_end ? std::make_pair(0u, 0u) : input_it->first;

        output_first.second += output_component;
        input_first.second += input_component;

        const auto output_length =
            output_at_end ? 0 : producer.GetNumComponentsInBaseType(producer.get_def(output_it->second.type_id));
        const auto input_length =
            input_at_end ? 0 : consumer.GetNumComponentsInBaseType(consumer.get_def(input_it->second.type_id));
        assert(output_at_end || output_component < output_length);
        assert(input_at_end || input_component < input_length);

        if (input_at_end || ((!output_at_end) && (output_first < input_first))) {
            if (!enabled_features.core13.maintenance4) {
                const std::string msg = std::string{producer_stage->name} + " writes to output location " +
                                        std::to_string(output_first.first) + "." + std::to_string(output_first.second) +
                                        " which is not consumed by " + consumer_stage->name +
                                        ". "
                                        "Enable VK_KHR_maintenance4 device extension to allow relaxed interface matching between "
                                        "input and output vectors.";
                // It is not an error if a stage does not consume all outputs from the previous stage
                skip |= LogPerformanceWarning(producer.vk_shader_module(), kVUID_Core_Shader_OutputNotConsumed, "%s", msg.c_str());
            }
            if ((input_first.first > output_first.first) || input_at_end || (output_component + 1 == output_length)) {
                output_it++;
                output_component = 0;
            } else {
                output_component++;
            }
        } else if (output_at_end || output_first > input_first) {
            skip |= LogError(consumer.vk_shader_module(), kVUID_Core_Shader_InputNotProduced,
                             "%s consumes input location %" PRIu32 ".%" PRIu32 " which is not written by %s", consumer_stage->name,
                             input_first.first, input_first.second, producer_stage->name);
            if ((output_first.first > input_first.first) || output_at_end || (input_component + 1 == input_length)) {
                input_it++;
                input_component = 0;
            } else {
                input_component++;
            }
        } else {
            // subtleties of arrayed interfaces:
            // - if is_patch, then the member is not arrayed, even though the interface may be.
            // - if is_block_member, then the extra array level of an arrayed interface is not
            //   expressed in the member type -- it's expressed in the block type.
            if (!TypesMatch(producer, consumer, output_it->second.type_id, input_it->second.type_id)) {
                skip |= LogError(producer.vk_shader_module(), kVUID_Core_Shader_InterfaceTypeMismatch,
                                 "Type mismatch on location %" PRIu32 ".%" PRIu32 ", between %s and %s: '%s' vs '%s'",
                                 output_first.first, output_first.second, producer_stage->name, consumer_stage->name,
                                 producer.DescribeType(output_it->second.type_id).c_str(),
                                 consumer.DescribeType(input_it->second.type_id).c_str());
                output_it++;
                input_it++;
                continue;
            }
            if (output_it->second.is_patch != input_it->second.is_patch) {
                skip |= LogError(producer.vk_shader_module(), kVUID_Core_Shader_InterfaceTypeMismatch,
                                 "Decoration mismatch on location %" PRIu32 ".%" PRIu32
                                 ": is per-%s in %s stage but per-%s in %s stage",
                                 output_first.first, output_first.second, output_it->second.is_patch ? "patch" : "vertex",
                                 producer_stage->name, input_it->second.is_patch ? "patch" : "vertex", consumer_stage->name);
            }
            uint32_t output_remaining = output_length - output_component;
            uint32_t input_remaining = input_length - input_component;
            if (output_remaining == input_remaining) {  // Sizes match so we can advance both output_it and input_it
                output_it++;
                input_it++;
                output_component = 0;
                input_component = 0;
            } else if (output_remaining > input_remaining) {  // a has more components remaining
                output_component += input_remaining;
                input_component = 0;
                input_it++;
            } else if (input_remaining > output_remaining) {  // b has more components remaining
                input_component += output_remaining;
                output_component = 0;
                output_it++;
            }
            if (output_component == 4) {
                output_component = 0;
                output_it++;
            }
            if (input_component == 4) {
                input_component = 0;
                input_it++;
            }
        }
    }

    if (consumer_stage->stage != VK_SHADER_STAGE_FRAGMENT_BIT) {
        auto builtins_producer = producer.CollectBuiltinBlockMembers(producer_entrypoint, spv::StorageClassOutput);
        auto builtins_consumer = consumer.CollectBuiltinBlockMembers(consumer_entrypoint, spv::StorageClassInput);

        if (!builtins_producer.empty() && !builtins_consumer.empty()) {
            if (builtins_producer.size() != builtins_consumer.size()) {
                skip |= LogError(producer.vk_shader_module(), kVUID_Core_Shader_InterfaceTypeMismatch,
                                 "Number of elements inside builtin block differ between stages (%s %d vs %s %d).",
                                 producer_stage->name, static_cast<int>(builtins_producer.size()), consumer_stage->name,
                                 static_cast<int>(builtins_consumer.size()));
            } else {
                auto it_producer = builtins_producer.begin();
                auto it_consumer = builtins_consumer.begin();
                while (it_producer != builtins_producer.end() && it_consumer != builtins_consumer.end()) {
                    if (*it_producer != *it_consumer) {
                        skip |= LogError(producer.vk_shader_module(), kVUID_Core_Shader_InterfaceTypeMismatch,
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

static inline uint32_t DetermineFinalGeomStage(const PIPELINE_STATE &pipeline) {
    uint32_t stage_mask = pipeline.active_shaders;
    if (pipeline.topology_at_rasterizer == VK_PRIMITIVE_TOPOLOGY_POINT_LIST) {
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
    bool skip = false;

    if (pipeline->IsGraphicsLibrary()) {
        // Only validate stages in an executable pipeline, not a graphics library
        // TODO This currently makes executing executable pipeline more expensive than they need to be since we could be validating
        // more per library.
        return skip;
    }

    uint32_t pointlist_stage_mask = DetermineFinalGeomStage(*pipeline);

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

    auto vi_state = pipeline->InputState();

    if (vi_state) {
        skip |= ValidateViConsistency(vi_state);
    }

    if (vertex_stage && vertex_stage->module_state->has_valid_spirv && !IsDynamic(pipeline, VK_DYNAMIC_STATE_VERTEX_INPUT_EXT)) {
        skip |= ValidateViAgainstVsInputs(vi_state, *vertex_stage->module_state.get(), vertex_stage->entrypoint);
    }

    for (size_t i = 1; i < pipeline->stage_state.size(); i++) {
        const auto &producer = pipeline->stage_state[i - 1];
        const auto &consumer = pipeline->stage_state[i];
        assert(producer.module_state);
        if (&producer == fragment_stage) {
            break;
        }
        if (consumer.module_state) {
            if (consumer.module_state->has_valid_spirv && producer.module_state->has_valid_spirv) {
                auto producer_id = GetShaderStageId(producer.stage_flag);
                auto consumer_id = GetShaderStageId(consumer.stage_flag);
                skip |= ValidateInterfaceBetweenStages(*producer.module_state.get(), producer.entrypoint,
                                                       &shader_stage_attribs[producer_id], *consumer.module_state.get(),
                                                       consumer.entrypoint, &shader_stage_attribs[consumer_id]);
            }
        }
    }

    if (fragment_stage && fragment_stage->module_state->has_valid_spirv) {
        const auto &rp_state = pipeline->RenderPassState();
        if (rp_state && rp_state->UsesDynamicRendering()) {
            skip |= ValidateFsOutputsAgainstDynamicRenderingRenderPass(*fragment_stage->module_state.get(),
                                                                       fragment_stage->entrypoint, pipeline);
        } else {
            skip |= ValidateFsOutputsAgainstRenderPass(*fragment_stage->module_state.get(), fragment_stage->entrypoint, pipeline,
                                                       pipeline->Subpass());
        }
    }

    return skip;
}

bool CoreChecks::ValidateGraphicsPipelineShaderDynamicState(const PIPELINE_STATE *pipeline, const CMD_BUFFER_STATE *pCB,
                                                            const char *caller, const DrawDispatchVuid &vuid) const {
    bool skip = false;

    for (auto &stage : pipeline->stage_state) {
        if (stage.stage_flag == VK_SHADER_STAGE_VERTEX_BIT || stage.stage_flag == VK_SHADER_STAGE_GEOMETRY_BIT ||
            stage.stage_flag == VK_SHADER_STAGE_MESH_BIT_NV) {
            if (!phys_dev_ext_props.fragment_shading_rate_props.primitiveFragmentShadingRateWithMultipleViewports &&
                IsDynamic(pipeline, VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT) && pCB->viewportWithCountCount != 1) {
                if (stage.wrote_primitive_shading_rate) {
                    skip |=
                        LogError(stage.module_state.get()->vk_shader_module(), vuid.viewport_count_primitive_shading_rate,
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

uint32_t CoreChecks::CalcShaderStageCount(const PIPELINE_STATE &pipeline, VkShaderStageFlagBits stageBit) const {
    uint32_t total = 0;
    const auto stages = pipeline.GetShaderStages();
    for (const auto &stage : stages) {
        if (stage.stage == stageBit) {
            total++;
        }
    }

    const auto rt_lib_info = pipeline.GetRayTracingLibraryCreateInfo();
    if (rt_lib_info) {
        for (uint32_t i = 0; i < rt_lib_info->libraryCount; ++i) {
            auto library_pipeline = Get<PIPELINE_STATE>(rt_lib_info->pLibraries[i]);
            total += CalcShaderStageCount(*library_pipeline, stageBit);
        }
    }

    return total;
}

bool CoreChecks::GroupHasValidIndex(const PIPELINE_STATE &pipeline, uint32_t group, uint32_t stage) const {
    if (group == VK_SHADER_UNUSED_NV) {
        return true;
    }

    const auto stages = pipeline.GetShaderStages();

    const auto num_stages = static_cast<uint32_t>(stages.size());
    if (group < num_stages) {
        return (stages[group].stage & stage) != 0;
    }
    group -= num_stages;

    // Search libraries
    const auto rt_lib_info = pipeline.GetRayTracingLibraryCreateInfo();
    if (rt_lib_info) {
        for (uint32_t i = 0; i < rt_lib_info->libraryCount; ++i) {
            auto library_pipeline = Get<PIPELINE_STATE>(rt_lib_info->pLibraries[i]);
            const auto lib_stages = library_pipeline->GetShaderStages();
            const uint32_t stage_count = static_cast<uint32_t>(lib_stages.size());
            if (group < stage_count) {
                return (stages[group].stage & stage) != 0;
            }
            group -= stage_count;
        }
    }

    // group index too large
    return false;
}

bool CoreChecks::ValidateRayTracingPipeline(PIPELINE_STATE *pipeline, const safe_VkRayTracingPipelineCreateInfoCommon &create_info,
                                            VkPipelineCreateFlags flags, bool isKHR) const {
    bool skip = false;

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
                const auto library_pipelinestate = Get<PIPELINE_STATE>(create_info.pLibraryInfo->pLibraries[i]);
                const auto &library_create_info = library_pipelinestate->GetCreateInfo<VkRayTracingPipelineCreateInfoKHR>();
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
        const uint32_t raygen_stages_count = CalcShaderStageCount(*pipeline, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
        if (raygen_stages_count == 0) {
            skip |= LogError(
                device,
                isKHR ? "VUID-VkRayTracingPipelineCreateInfoKHR-stage-03425" : "VUID-VkRayTracingPipelineCreateInfoNV-stage-06232",
                " : The stage member of at least one element of pStages must be VK_SHADER_STAGE_RAYGEN_BIT_KHR.");
        }
    }
    if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR) != 0 &&
        (flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR) != 0) {
        skip |= LogError(
            device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-06546",
            "vkCreateRayTracingPipelinesKHR: flags (%s) contains both VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR and "
            "VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR bits.",
            string_VkPipelineCreateFlags(flags).c_str());
    }

    for (uint32_t group_index = 0; group_index < create_info.groupCount; group_index++) {
        const auto &group = groups[group_index];

        if (group.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV) {
            if (!GroupHasValidIndex(
                    *pipeline, group.generalShader,
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
            if (!GroupHasValidIndex(*pipeline, group.intersectionShader, VK_SHADER_STAGE_INTERSECTION_BIT_NV)) {
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
            if (!GroupHasValidIndex(*pipeline, group.anyHitShader, VK_SHADER_STAGE_ANY_HIT_BIT_KHR)) {
                skip |= LogError(device,
                                 isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-anyHitShader-03479"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-anyHitShader-02418",
                                 ": pGroups[%d]", group_index);
            }
            if (!GroupHasValidIndex(*pipeline, group.closestHitShader, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR)) {
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
                         "SPIR-V module not valid: Codesize must be a multiple of 4 but is %zu", pCreateInfo->codeSize);
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

bool CoreChecks::PreCallValidateGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule,
                                                             VkShaderModuleIdentifierEXT *pIdentifier) const {
    bool skip = false;
    if (!(enabled_features.shader_module_identifier_features.shaderModuleIdentifier)) {
        skip |= LogError(shaderModule, "VUID-vkGetShaderModuleIdentifierEXT-shaderModuleIdentifier-06884",
                         "vkGetShaderModuleIdentifierEXT() was called when the shaderModuleIdentifier feature was not enabled");
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetShaderModuleCreateInfoIdentifierEXT(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                                                       VkShaderModuleIdentifierEXT *pIdentifier) const {
    bool skip = false;
    if (!(enabled_features.shader_module_identifier_features.shaderModuleIdentifier)) {
        skip |= LogError(
            device, "VUID-vkGetShaderModuleCreateInfoIdentifierEXT-shaderModuleIdentifier-06885",
            "vkGetShaderModuleCreateInfoIdentifierEXT() was called when the shaderModuleIdentifier feature was not enabled");
    }
    return skip;
}

bool CoreChecks::ValidateComputeWorkGroupSizes(const SHADER_MODULE_STATE &module_state, const spirv_inst_iter &entrypoint,
                                               const PipelineStageState &stage_state, uint32_t local_size_x, uint32_t local_size_y,
                                               uint32_t local_size_z) const {
    bool skip = false;
    // If spec constants were used then the local size are already found if possible
    if (local_size_x == 0) {
        if (!module_state.FindLocalSize(entrypoint, local_size_x, local_size_y, local_size_z)) {
            return skip;  // no local size found
        }
    }

    if (local_size_x > phys_dev_props.limits.maxComputeWorkGroupSize[0]) {
        skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-x-06429",
                         "%s local_size_x (%" PRIu32 ") exceeds device limit maxComputeWorkGroupSize[0] (%" PRIu32 ").",
                         report_data->FormatHandle(module_state.vk_shader_module()).c_str(), local_size_x,
                         phys_dev_props.limits.maxComputeWorkGroupSize[0]);
    }
    if (local_size_y > phys_dev_props.limits.maxComputeWorkGroupSize[1]) {
        skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-y-06430",
                         "%s local_size_y (%" PRIu32 ") exceeds device limit maxComputeWorkGroupSize[1] (%" PRIu32 ").",
                         report_data->FormatHandle(module_state.vk_shader_module()).c_str(), local_size_x,
                         phys_dev_props.limits.maxComputeWorkGroupSize[1]);
    }
    if (local_size_z > phys_dev_props.limits.maxComputeWorkGroupSize[2]) {
        skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-z-06431",
                         "%s local_size_z (%" PRIu32 ") exceeds device limit maxComputeWorkGroupSize[2] (%" PRIu32 ").",
                         report_data->FormatHandle(module_state.vk_shader_module()).c_str(), local_size_x,
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
        skip |= LogError(module_state.vk_shader_module(), "VUID-RuntimeSpirv-x-06432",
                         "%s local_size (%" PRIu32 ", %" PRIu32 ", %" PRIu32
                         ") exceeds device limit maxComputeWorkGroupInvocations (%" PRIu32 ").",
                         report_data->FormatHandle(module_state.vk_shader_module()).c_str(), local_size_x, local_size_y,
                         local_size_z, limit);
    }

    const auto subgroup_flags = VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT |
                                VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT;
    const auto *required_subgroup_size_features =
        LvlFindInChain<VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT>(stage_state.create_info->pNext);
    if (required_subgroup_size_features) {
        const uint32_t requiredSubgroupSize = required_subgroup_size_features->requiredSubgroupSize;
        skip |= RequireFeature(module_state, enabled_features.core13.subgroupSizeControl, "subgroupSizeControl",
                               "VUID-VkPipelineShaderStageCreateInfo-pNext-02755");
        if ((phys_dev_ext_props.subgroup_size_control_props.requiredSubgroupSizeStages & stage_state.stage_flag) == 0) {
            skip |= LogError(
                module_state.vk_shader_module(), "VUID-VkPipelineShaderStageCreateInfo-pNext-02755",
                "Stage %s is not in VkPhysicalDeviceSubgroupSizeControlPropertiesEXT::requiredSubgroupSizeStages (%s).",
                string_VkShaderStageFlagBits(stage_state.stage_flag),
                string_VkShaderStageFlags(phys_dev_ext_props.subgroup_size_control_props.requiredSubgroupSizeStages).c_str());
        }
        if ((invocations > requiredSubgroupSize * phys_dev_ext_props.subgroup_size_control_props.maxComputeWorkgroupSubgroups)) {
            skip |=
                LogError(module_state.vk_shader_module(), "VUID-VkPipelineShaderStageCreateInfo-pNext-02756",
                         "Local workgroup size (%" PRIu32 ", %" PRIu32 ", %" PRIu32
                         ") is greater than VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT::requiredSubgroupSize (%" PRIu32
                         ") * maxComputeWorkgroupSubgroups (%" PRIu32 ").",
                         local_size_x, local_size_y, local_size_z, requiredSubgroupSize,
                         phys_dev_ext_props.subgroup_size_control_props.maxComputeWorkgroupSubgroups);
        }
        if ((stage_state.create_info->flags & VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT) > 0) {
            if (SafeModulo(local_size_x, requiredSubgroupSize) != 0) {
                skip |= LogError(
                    module_state.vk_shader_module(), "VUID-VkPipelineShaderStageCreateInfo-pNext-02757",
                    "Local workgroup size x (%" PRIu32
                    ") is not a multiple of VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT::requiredSubgroupSize (%" PRIu32
                    ").",
                    local_size_x, requiredSubgroupSize);
            }
        }
        if (!IsPowerOfTwo(requiredSubgroupSize)) {
            skip |= LogError(module_state.vk_shader_module(),
                             "VUID-VkPipelineShaderStageRequiredSubgroupSizeCreateInfo-requiredSubgroupSize-02760",
                             "VkPhysicalDeviceSubgroupSizeControlPropertiesEXT::requiredSubgroupSizeStages (%" PRIu32
                             ") is not a power of 2.",
                             requiredSubgroupSize);
        }
        if (requiredSubgroupSize < phys_dev_ext_props.subgroup_size_control_props.minSubgroupSize) {
            skip |= LogError(module_state.vk_shader_module(),
                             "VUID-VkPipelineShaderStageRequiredSubgroupSizeCreateInfo-requiredSubgroupSize-02761",
                             "VkPhysicalDeviceSubgroupSizeControlPropertiesEXT::requiredSubgroupSizeStages (%" PRIu32
                             ") is less than minSubgroupSize (%" PRIu32 ").",
                             requiredSubgroupSize, phys_dev_ext_props.subgroup_size_control_props.minSubgroupSize);
        }
        if (requiredSubgroupSize > phys_dev_ext_props.subgroup_size_control_props.maxSubgroupSize) {
            skip |= LogError(module_state.vk_shader_module(),
                             "VUID-VkPipelineShaderStageRequiredSubgroupSizeCreateInfo-requiredSubgroupSize-02762",
                             "VkPhysicalDeviceSubgroupSizeControlPropertiesEXT::requiredSubgroupSizeStages (%" PRIu32
                             ") is greater than maxSubgroupSize (%" PRIu32 ").",
                             requiredSubgroupSize, phys_dev_ext_props.subgroup_size_control_props.maxSubgroupSize);
        }
    }
    if ((stage_state.create_info->flags & subgroup_flags) == subgroup_flags) {
        if (SafeModulo(local_size_x, phys_dev_ext_props.subgroup_size_control_props.maxSubgroupSize) != 0) {
            skip |= LogError(
                module_state.vk_shader_module(), "VUID-VkPipelineShaderStageCreateInfo-flags-02758",
                "%s flags contain VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT and "
                "VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT bits, but local workgroup size in the X "
                "dimension (%" PRIu32
                ") is not a multiple of VkPhysicalDeviceSubgroupSizeControlPropertiesEXT::maxSubgroupSize (%" PRIu32 ").",
                report_data->FormatHandle(module_state.vk_shader_module()).c_str(), local_size_x,
                phys_dev_ext_props.subgroup_size_control_props.maxSubgroupSize);
        }
    } else if ((stage_state.create_info->flags & VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT) &&
               (stage_state.create_info->flags & VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT) == 0) {
        if (!required_subgroup_size_features) {
            if (SafeModulo(local_size_x, phys_dev_props_core11.subgroupSize) != 0) {
                skip |= LogError(
                    module_state.vk_shader_module(), "VUID-VkPipelineShaderStageCreateInfo-flags-02759",
                    "%s flags contain VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT bit, and not the"
                    "VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT bit, but local workgroup size in the "
                    "X dimension (%" PRIu32 ") is not a multiple of VkPhysicalDeviceVulkan11Properties::subgroupSize (%" PRIu32
                    ").",
                    report_data->FormatHandle(module_state.vk_shader_module()).c_str(), local_size_x,
                    phys_dev_props_core11.subgroupSize);
            }
        }
    }
    return skip;
}

spv_target_env PickSpirvEnv(uint32_t api_version, bool spirv_1_4) {
    if (api_version >= VK_API_VERSION_1_3) {
        return SPV_ENV_VULKAN_1_3;
    } else if (api_version >= VK_API_VERSION_1_2) {
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
    if (enabled_features.core13.maintenance4) {
        // --allow-localsizeid
        options.SetAllowLocalSizeId(true);
    }

    // Faster validation without friendly names.
    options.SetFriendlyNames(false);
}
