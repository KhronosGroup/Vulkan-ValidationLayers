/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
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
 */

#include <cassert>
#include <sstream>
#include <string>
#include <vector>
#include <set>

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>
#include "core_validation.h"
#include "generated/spirv_grammar_helper.h"
#include "state_tracker/image_state.h"
#include "state_tracker/shader_object_state.h"
#include "state_tracker/shader_stage_state.h"
#include "state_tracker/shader_module.h"
#include "state_tracker/render_pass_state.h"
#include "utils/vk_layer_utils.h"

bool CoreChecks::ValidateInterfaceVertexInput(const vvl::Pipeline &pipeline, const spirv::Module &module_state,
                                              const spirv::EntryPoint &entrypoint, const Location &create_info_loc) const {
    bool skip = false;
    const Location vi_loc = create_info_loc.dot(Field::pVertexInputState);

    struct AttribInputPair {
        const VkFormat *attribute_input = nullptr;
        const spirv::Instruction *shader_input = nullptr;
        uint32_t attribute_index = 0;
    };
    // For vertex input, we only need to care about Location.
    // You are not allowed to offset into the Component words
    // or have 2 variables in a location
    std::map<uint32_t, AttribInputPair> location_map;

    vku::safe_VkPipelineVertexInputStateCreateInfo const *input_state = pipeline.InputState();
    if (!input_state) {
        // if using vertex and mesh, will hit an error, but still might get here
        return skip;
    }

    for (uint32_t i = 0; i < input_state->vertexAttributeDescriptionCount; ++i) {
        const VkFormat format = input_state->pVertexAttributeDescriptions[i].format;
        const uint32_t format_size = GetVertexInputFormatSize(format);
        // Vulkan Spec: Location is made up of 16 bytes, never can have 0 Locations
        const uint32_t bytes_in_location = 16;
        const uint32_t num_locations = ((format_size - 1) / bytes_in_location) + 1;
        for (uint32_t j = 0; j < num_locations; ++j) {
            const uint32_t index = input_state->pVertexAttributeDescriptions[i].location + j;
            location_map[index].attribute_input = &(input_state->pVertexAttributeDescriptions[i].format);
            location_map[index].attribute_index = i;
        }
    }

    for (const auto *variable_ptr : entrypoint.user_defined_interface_variables) {
        const auto &variable = *variable_ptr;
        if ((variable.storage_class != spv::StorageClassInput)) {
            continue;  // not an input interface
        }
        // It is possible to have a struct block for the vertex input.
        // All members of struct must all have Locations or none of them will.
        // If the interface variable doesn't have the Locations, find them inside the struct members
        if (!variable.type_struct_info) {
            for (const auto &slot : variable.interface_slots) {
                location_map[slot.Location()].shader_input = &variable.base_type;
            }
        } else if (variable.decorations.location != spirv::kInvalidValue) {
            // Variable is decorated with Location
            uint32_t location = variable.decorations.location;
            for (uint32_t i = 0; i < variable.type_struct_info->members.size(); i++) {
                const auto &member = variable.type_struct_info->members[i];
                // can be 64-bit formats in the struct
                const uint32_t num_locations = module_state.GetLocationsConsumedByType(member.id);
                for (uint32_t j = 0; j < num_locations; ++j) {
                    location_map[location + j].shader_input = member.insn;
                }
                location += num_locations;
            }
        } else {
            // Can't be nested so only need to look at first level of members
            for (const auto &member : variable.type_struct_info->members) {
                location_map[member.decorations->location].shader_input = member.insn;
            }
        }
    }

    for (const auto &[location, attribute_info] : location_map) {
        const auto attribute_input = attribute_info.attribute_input;
        const auto shader_input = attribute_info.shader_input;

        if (attribute_input && !shader_input) {
            skip |= LogPerformanceWarning("WARNING-Shader-OutputNotConsumed", module_state.handle(), vi_loc,
                                          "Vertex attribute at location %" PRIu32 " not consumed by vertex shader.", location);
        } else if (!attribute_input && shader_input) {
            if (!enabled_features.vertexAttributeRobustness) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-Input-07904", module_state.handle(),
                                 vi_loc.dot(Field::pVertexAttributeDescriptions),
                                 "does not have a Location %" PRIu32
                                 " but vertex shader has an input variable at that Location. (This can be valid if "
                                 "vertexAttributeRobustness feature is enabled)",
                                 location);
            }
        } else if (attribute_input && shader_input) {
            const VkFormat attribute_format = *attribute_input;
            const uint32_t attribute_type = spirv::GetFormatType(attribute_format);
            const uint32_t var_base_type_id = shader_input->ResultId();
            const uint32_t var_numeric_type = module_state.GetNumericType(var_base_type_id);

            const bool attribute64 = vkuFormatIs64bit(attribute_format);
            const bool shader64 = module_state.GetBaseTypeInstruction(var_base_type_id)->GetBitWidth() == 64;

            // Type checking
            if ((attribute_type & var_numeric_type) == 0) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-Input-08733", module_state.handle(),
                                 vi_loc.dot(Field::pVertexAttributeDescriptions, attribute_info.attribute_index).dot(Field::format),
                                 "(%s) at Location %" PRIu32 " does not match vertex shader input type (%s).",
                                 string_VkFormat(attribute_format), location, module_state.DescribeType(var_base_type_id).c_str());
            } else if (attribute64 && !shader64) {
                skip |=
                    LogError("VUID-VkGraphicsPipelineCreateInfo-pVertexInputState-08929", module_state.handle(),
                             vi_loc.dot(Field::pVertexAttributeDescriptions, attribute_info.attribute_index).dot(Field::format),
                             "(%s) is a 64-bit format, but at Location %" PRIu32 " the vertex shader input is 32-bit type (%s).",
                             string_VkFormat(attribute_format), location, module_state.DescribeType(var_base_type_id).c_str());
            } else if (!attribute64 && shader64) {
                skip |=
                    LogError("VUID-VkGraphicsPipelineCreateInfo-pVertexInputState-08930", module_state.handle(),
                             vi_loc.dot(Field::pVertexAttributeDescriptions, attribute_info.attribute_index).dot(Field::format),
                             "(%s) is a 64-bit format, but at Location %" PRIu32 " the vertex shader input is 64-bit type (%s).",
                             string_VkFormat(attribute_format), location, module_state.DescribeType(var_base_type_id).c_str());
            } else if (attribute64 && shader64) {
                const uint32_t attribute_components = vkuFormatComponentCount(attribute_format);
                const uint32_t input_components = module_state.GetNumComponentsInBaseType(shader_input);
                if (attribute_components < input_components) {
                    skip |= LogError(
                        "VUID-VkGraphicsPipelineCreateInfo-pVertexInputState-09198", module_state.handle(),
                        vi_loc.dot(Field::pVertexAttributeDescriptions, attribute_info.attribute_index).dot(Field::format),
                        "(%s) is a %" PRIu32 "-wide 64-bit format, but at location %" PRIu32 " the vertex shader input is %" PRIu32
                        "-wide 64-bit type (%s). (64-bit vertex input don't have default values and require "
                        "components to match what is used in the shader)",
                        string_VkFormat(attribute_format), attribute_components, location, input_components,
                        module_state.DescribeType(var_base_type_id).c_str());
                }
            }
        } else {            // !attrib && !input
            assert(false);  // at least one exists in the map
        }
    }

    return skip;
}

bool CoreChecks::ValidateInterfaceFragmentOutput(const vvl::Pipeline &pipeline, const spirv::Module &module_state,
                                                 const spirv::EntryPoint &entrypoint, const Location &create_info_loc) const {
    bool skip = false;
    const auto *ms_state = pipeline.MultisampleState();
    if (!pipeline.IsDynamic(CB_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT) && ms_state && ms_state->alphaToCoverageEnable) {
        if (!entrypoint.has_alpha_to_coverage_variable) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-alphaToCoverageEnable-08891", module_state.handle(),
                             create_info_loc.dot(Field::pMultisampleState).dot(Field::alphaToCoverageEnable),
                             "is VK_TRUE, but the fragment shader doesn't declare a variable that covers "
                             "Location 0, Component 3 (alpha channel).");
        }
    }
    return skip;
}

bool CoreChecks::ValidateBuiltinLimits(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                       const vvl::Pipeline *pipeline, const Location &loc) const {
    bool skip = false;

    // Currently all builtin tested are only found in fragment shaders
    if (entrypoint.execution_model != spv::ExecutionModelFragment) {
        return skip;
    }

    for (const auto *variable : entrypoint.built_in_variables) {
        // Currently don't need to search in structs
        // Handles both the input and output sampleMask
        if (variable->decorations.builtin == spv::BuiltInSampleMask &&
            variable->array_size > phys_dev_props.limits.maxSampleMaskWords) {
            const char *vuid = pipeline ? "VUID-VkPipelineShaderStageCreateInfo-maxSampleMaskWords-00711"
                                        : "VUID-VkShaderCreateInfoEXT-pCode-08451";
            skip |= LogError(vuid, module_state.handle(), loc,
                             "The BuiltIns SampleMask array sizes is %" PRIu32
                             " which exceeds "
                             "maxSampleMaskWords of %" PRIu32 ".",
                             variable->array_size, phys_dev_props.limits.maxSampleMaskWords);
            break;
        }
    }

    return skip;
}

bool CoreChecks::ValidatePrimitiveTopology(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                           const vvl::Pipeline &pipeline, const Location &loc) const {
    bool skip = false;

    if (!pipeline.pre_raster_state || !pipeline.InputAssemblyState() || entrypoint.stage != VK_SHADER_STAGE_GEOMETRY_BIT ||
        pipeline.IsDynamic(CB_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY)) {
        return skip;
    }

    bool has_tess = false;
    VkPrimitiveTopology topology = pipeline.InputAssemblyState()->topology;
    for (uint32_t i = 0; i < pipeline.stage_states.size(); i++) {
        auto &stage_state = pipeline.stage_states[i];
        const VkShaderStageFlagBits stage = stage_state.GetStage();
        if (stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT || stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) {
            has_tess = true;
            if (stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) {
                if (stage_state.entrypoint->execution_mode.Has(spirv::ExecutionModeSet::point_mode_bit)) {
                    topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
                } else {
                    topology = stage_state.entrypoint->execution_mode.primitive_topology;
                }
            }
        }
    }

    VkPrimitiveTopology geom_topology = entrypoint.execution_mode.input_primitive_topology;
    bool mismatch = false;
    mismatch |= (topology == VK_PRIMITIVE_TOPOLOGY_POINT_LIST && geom_topology != VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
    mismatch |=
        IsValueIn(topology, {VK_PRIMITIVE_TOPOLOGY_LINE_LIST, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
                             VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY}) &&
        !IsValueIn(geom_topology,
                   {VK_PRIMITIVE_TOPOLOGY_LINE_LIST, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
                    VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY});
    mismatch |= IsValueIn(topology, {VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
                                     VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,
                                     VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY}) &&
                !IsValueIn(geom_topology, {VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
                                           VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,
                                           VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY});
    if (mismatch) {
        if (has_tess) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pStages-00739", module_state.handle(), loc,
                             "SPIR-V (Geometry stage) expects input topology %s, but tessellation evaluation shader output topology is %s.",
                             string_VkPrimitiveTopology(geom_topology), string_VkPrimitiveTopology(topology));
        } else {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pStages-00738", module_state.handle(), loc,
                             "SPIR-V (Geometry stage) expects input topology %s, but pipeline was created with primitive topology %s.",
                             string_VkPrimitiveTopology(geom_topology), string_VkPrimitiveTopology(topology));
        }
    }

    return skip;
}

bool CoreChecks::ValidateInterfaceBetweenStages(const spirv::Module &producer, const spirv::EntryPoint &producer_entrypoint,
                                                const spirv::Module &consumer, const spirv::EntryPoint &consumer_entrypoint,
                                                const Location &create_info_loc) const {
    bool skip = false;

    if (producer_entrypoint.has_passthrough) {
        return skip;  // PassthroughNV doesn't have to do Location matching
    }

    const VkShaderStageFlagBits producer_stage = producer_entrypoint.stage;
    const VkShaderStageFlagBits consumer_stage = consumer_entrypoint.stage;

    // build up a mapping of which slots are used and then go through it and look for gaps
    struct ComponentInfo {
        const spirv::StageInterfaceVariable *output = nullptr;
        uint32_t output_type = 0;
        uint32_t output_width = 0;
        const spirv::StageInterfaceVariable *input = nullptr;
        uint32_t input_type = 0;
        uint32_t input_width = 0;
    };
    // <Location, Components[4]> (only 4 components in a Location)
    vvl::unordered_map<uint32_t, std::array<ComponentInfo, 4>> slot_map;

    for (const auto &[interface_slot, stage_variable] : producer_entrypoint.output_interface_slots) {
        auto &slot = slot_map[interface_slot.Location()][interface_slot.Component()];
        if (stage_variable->nested_struct) {
            return skip;  // TODO workaround
        }
        slot.output = stage_variable;
        slot.output_type = interface_slot.type;
        slot.output_width = interface_slot.bit_width;
    }

    for (const auto &[interface_slot, stage_variable] : consumer_entrypoint.input_interface_slots) {
        auto &slot = slot_map[interface_slot.Location()][interface_slot.Component()];
        if (stage_variable->nested_struct) {
            return skip;  // TODO workaround
        }
        slot.input = stage_variable;
        slot.input_type = interface_slot.type;
        slot.input_width = interface_slot.bit_width;
    }

    for (const auto &[location, components] : slot_map) {
        // Found that sometimes there is a big mismatch and printing out EVERY slot adds a lot of noise
        if (skip) break;

        for (uint32_t component = 0; component < 4; component++) {
            const auto &component_info = components[component];
            const auto *input_var = component_info.input;
            const auto *output_var = component_info.output;

            if ((input_var == nullptr) && (output_var == nullptr)) {
                continue;  // both empty
            } else if ((input_var != nullptr) && (output_var != nullptr)) {
                // if matched, need to check type
                // Only the OpType has to match, signed vs unsigned in not important
                if ((component_info.output_type != component_info.input_type) ||
                    (component_info.output_width != component_info.input_width)) {
                    const LogObjectList objlist(producer.handle(), consumer.handle());
                    skip |= LogError("VUID-RuntimeSpirv-OpEntryPoint-07754", objlist, create_info_loc,
                                     "(SPIR-V Interface) Type mismatch on Location %" PRIu32 " Component %" PRIu32
                                     ", between\n\n%s stage:\n%s%s\n\n%s stage:\n%s%s\n\n",
                                     location, component, string_VkShaderStageFlagBits(producer_stage),
                                     producer.DescribeVariable(output_var->id).c_str(),
                                     producer.DescribeType(output_var->type_id).c_str(),
                                     string_VkShaderStageFlagBits(consumer_stage), consumer.DescribeVariable(input_var->id).c_str(),
                                     consumer.DescribeType(input_var->type_id).c_str());
                    break;  // Only need to report for the first component found
                }

                // TODO - Being discussed if Patch have own Location slots or not
                // https://gitlab.khronos.org/vulkan/vulkan/-/issues/3858
                // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8014
                //
                // Tessellation needs to match Patch vs Vertex
                // if ((producer_stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) &&
                //     (consumer_stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) &&
                //     (input_var->is_patch != output_var->is_patch)) {
                //     const LogObjectList objlist(producer.handle(), consumer.handle());
                //     skip |= LogError("VUID-RuntimeSpirv-OpVariable-08746", objlist, create_info_loc,
                //                      "(SPIR-V Interface) at Location %" PRIu32 " Component %" PRIu32
                //                      " Tessellation Control is %s while Tessellation Evaluation is %s",
                //                      location, component, input_var->is_patch ? "patch" : "vertex",
                //                      output_var->is_patch ? "patch" : "vertex");
                //     break;  // Only need to report for the first component found
                // }

                // If using maintenance4 need to check Vectors incase different sizes
                if (!enabled_features.maintenance4 && (output_var->base_type.Opcode() == spv::OpTypeVector) &&
                    (input_var->base_type.Opcode() == spv::OpTypeVector)) {
                    // Note the "Component Count" in the VU refers to OpTypeVector's operand and NOT the "Component slot"
                    const uint32_t output_vec_size = output_var->base_type.Word(3);
                    const uint32_t input_vec_size = input_var->base_type.Word(3);
                    if (output_vec_size > input_vec_size) {
                        const LogObjectList objlist(producer.handle(), consumer.handle());
                        skip |= LogError("VUID-RuntimeSpirv-maintenance4-06817", objlist, create_info_loc,
                                         "(SPIR-V Interface) starting at Location %" PRIu32 " Component %" PRIu32
                                         " the Output (%s) has a Vec%" PRIu32 " while Input (%s) as a Vec%" PRIu32
                                         ". Enable VK_KHR_maintenance4 device extension to allow relaxed interface matching "
                                         "between input and output vectors.",
                                         location, component, string_VkShaderStageFlagBits(producer_stage), output_vec_size,
                                         string_VkShaderStageFlagBits(consumer_stage), input_vec_size);
                        break;  // Only need to report for the first component found
                    }
                }
            } else if ((input_var == nullptr) && (output_var != nullptr)) {
                // Missing input slot
                // It is not an error if a stage does not consume all outputs from the previous stage
                // The values will be undefined, but still legal
                // Don't give any warning if maintenance4 with vectors
                if (!enabled_features.maintenance4 && (output_var->base_type.Opcode() != spv::OpTypeVector)) {
                    const LogObjectList objlist(producer.handle(), consumer.handle());
                    skip |= LogPerformanceWarning("WARNING-Shader-OutputNotConsumed", objlist, create_info_loc,
                                                  "(SPIR-V Interface) %s declared to output location %" PRIu32 " Component %" PRIu32
                                                  " but is not an Input declared by %s.",
                                                  string_VkShaderStageFlagBits(producer_stage), location, component,
                                                  string_VkShaderStageFlagBits(consumer_stage));
                }
            } else if ((input_var != nullptr) && (output_var == nullptr)) {
                // Missing output slot
                if ((consumer_stage & (VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                                       VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)) &&
                    (input_var->base_type.Opcode() == spv::OpTypeArray)) {
                    break;  // When going inbetween Tessellation or Geometry, array size can be different
                }
                const LogObjectList objlist(producer.handle(), consumer.handle());
                skip |= LogError("VUID-RuntimeSpirv-OpEntryPoint-08743", objlist, create_info_loc,
                                 "(SPIR-V Interface) %s declared input at Location %" PRIu32 " Component %" PRIu32
                                 " %sbut it is not an Output declared in %s",
                                 string_VkShaderStageFlagBits(consumer_stage), location, component,
                                 input_var->is_patch ? "(Tessellation Patch) " : "", string_VkShaderStageFlagBits(producer_stage));
                break;  // Only need to report for the first component found
            }
        }
    }

    // Need to check the BuiltIn interface (if not going into Fragment)
    if (consumer_stage == VK_SHADER_STAGE_FRAGMENT_BIT) {
        return skip;
    }

    std::vector<uint32_t> input_builtins_block;
    std::vector<uint32_t> output_builtins_block;
    for (const auto *variable : producer_entrypoint.built_in_variables) {
        if (variable->storage_class == spv::StorageClassOutput && !variable->builtin_block.empty()) {
            output_builtins_block = variable->builtin_block;
            break;
        }
    }
    for (const auto *variable : consumer_entrypoint.built_in_variables) {
        if (variable->storage_class == spv::StorageClassInput && !variable->builtin_block.empty()) {
            input_builtins_block = variable->builtin_block;
            break;
        }
    }

    bool mismatch = false;
    if (input_builtins_block.empty() || output_builtins_block.empty()) {
        // TODO - Nothing about this in spec, need to add language to confirm this is correct
        return skip;
    } else if (input_builtins_block.size() != output_builtins_block.size()) {
        mismatch = true;
    } else {
        for (size_t i = 0; i < input_builtins_block.size(); i++) {
            const uint32_t input_builtin = input_builtins_block[i];
            const uint32_t output_builtin = output_builtins_block[i];
            if (input_builtin == spirv::kInvalidValue || output_builtin == spirv::kInvalidValue) {
                continue;  // some stages (TessControl -> TessEval) can have legal block vs non-block mismatch
            } else if (input_builtin != output_builtin) {
                mismatch = true;
            }
        }
    }

    if (mismatch) {
        std::stringstream msg;
        msg << string_VkShaderStageFlagBits(producer_stage) << " Output Block {\n";
        for (size_t i = 0; i < output_builtins_block.size(); i++) {
            msg << '\t' << i << ": " << string_SpvBuiltIn(output_builtins_block[i]) << '\n';
        }
        msg << "}\n";
        msg << string_VkShaderStageFlagBits(consumer_stage) << " Input Block {\n";
        for (size_t i = 0; i < input_builtins_block.size(); i++) {
            msg << '\t' << i << ": " << string_SpvBuiltIn(input_builtins_block[i]) << '\n';
        }
        msg << "}\n";
        const LogObjectList objlist(producer.handle(), consumer.handle());
        skip |= LogError("VUID-RuntimeSpirv-OpVariable-08746", objlist, create_info_loc,
                         "(SPIR-V Interface) Mismatch in BuiltIn blocks:\n %s", msg.str().c_str());
    }
    return skip;
}

// Note - This is missing a variation check for ShaderObjects, but there is one for Dynamic Rendering and likely people using
// ShaderObject are not using Render Passes
bool CoreChecks::ValidateFsOutputsAgainstRenderPass(const spirv::Module &module_state, const spirv::EntryPoint &entrypoint,
                                                    const vvl::Pipeline &pipeline, uint32_t subpass_index,
                                                    const Location &create_info_loc) const {
    bool skip = false;

    struct Attachment {
        const VkAttachmentReference2 *reference = nullptr;
        const VkAttachmentDescription2 *attachment = nullptr;
        const spirv::StageInterfaceVariable *output = nullptr;
    };
    std::map<uint32_t, Attachment> location_map;

    const auto &rp_state = pipeline.RenderPassState();
    ASSERT_AND_RETURN_SKIP(rp_state);
    const auto rpci = rp_state->create_info.ptr();
    if (subpass_index >= rpci->subpassCount) return skip;

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
    for (const auto *variable : entrypoint.user_defined_interface_variables) {
        if ((variable->storage_class != spv::StorageClassOutput) || variable->interface_slots.empty()) {
            continue;  // not an output interface
        }

        // TODO - https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7923
        // Need to redo logic to handle array of outputs
        if (variable->array_size > 1) {
            return false;
        }
        // It is not allowed to have Block Fragment or 64-bit vectors output in Frag shader
        // This means all Locations in slots will be the same
        location_map[variable->interface_slots[0].Location()].output = variable;
    }

    const auto *ms_state = pipeline.MultisampleState();
    const bool alpha_to_coverage_enabled = ms_state && (ms_state->alphaToCoverageEnable == VK_TRUE);

    for (const auto &[location, attachment_info] : location_map) {
        const auto reference = attachment_info.reference;
        if (reference != nullptr && reference->attachment == VK_ATTACHMENT_UNUSED) {
            continue;
        }

        const VkAttachmentDescription2 *attachment = attachment_info.attachment;
        const spirv::StageInterfaceVariable *output = attachment_info.output;
        if (attachment && !output) {
            // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9616
            // Need to understand when undefined or not
        } else if (!attachment && output) {
            // With alphaToCoverage, the write is not "discarded" as the alpha mask is still updated
            if (!alpha_to_coverage_enabled || location != 0) {
                skip |= LogUndefinedValue("Undefined-Value-ShaderOutputNotConsumed", module_state.handle(), create_info_loc,
                                          "Inside the fragment shader, it writes to output Location %" PRIu32
                                          " but there is no VkSubpassDescription::pColorAttachments[%" PRIu32
                                          "] and this write is unused.\nSpec information at "
                                          "https://docs.vulkan.org/spec/latest/chapters/interfaces.html#interfaces-fragmentoutput",
                                          location, location);
            }
        } else if (attachment && output) {
            const uint32_t attachment_type = spirv::GetFormatType(attachment->format);
            const uint32_t output_type = module_state.GetNumericType(output->type_id);

            // Type checking
            if ((output_type & attachment_type) == 0) {
                skip |= LogUndefinedValue("Undefined-Value-ShaderFragmentOutputMismatch", module_state.handle(), create_info_loc,
                                          "Inside the fragment shader, it writes to output Location %" PRIu32
                                          " with a numeric type of %s but VkSubpassDescription::pColorAttachments[%" PRIu32
                                          "] pointing at VkRenderPassCreateInfo::pAttachments[%" PRIu32
                                          "] is created with %s (numeric type of %s) which does not match and the resulting values "
                                          "written will be undefined.\nSpec information at "
                                          "https://docs.vulkan.org/spec/latest/chapters/interfaces.html#interfaces-fragmentoutput",
                                          location, spirv::string_NumericType(output_type), location, reference->attachment,
                                          string_VkFormat(attachment->format), spirv::string_NumericType(attachment_type));
            }
        } else {            // !attachment && !output
            assert(false);  // at least one exists in the map
        }
    }

    return skip;
}

// This is validated at draw time unlike the VkRenderPass version
bool CoreChecks::ValidateDrawDynamicRenderingFsOutputs(const LastBound &last_bound_state, const vvl::Pipeline *pipeline,
                                                       const vvl::RenderPass &rp_state, const Location &loc) const {
    bool skip = false;

    const spirv::EntryPoint *entrypoint = last_bound_state.GetFragmentEntryPoint();
    if (!entrypoint) return skip;

    if (rp_state.use_dynamic_rendering_inherited) return skip;

    struct Attachment {
        const VkRenderingAttachmentInfo *rendering_attachment_info = nullptr;
        const spirv::StageInterfaceVariable *output = nullptr;
    };
    std::map<uint32_t, Attachment> location_map;

    const uint32_t color_attachment_count = rp_state.dynamic_rendering_begin_rendering_info.colorAttachmentCount;
    for (uint32_t i = 0; i < color_attachment_count; ++i) {
        location_map[i].rendering_attachment_info = rp_state.dynamic_rendering_begin_rendering_info.pColorAttachments[i].ptr();
    }

    // TODO: dual source blend index (spv::DecIndex, zero if not provided)
    for (const auto *variable : entrypoint->user_defined_interface_variables) {
        if ((variable->storage_class != spv::StorageClassOutput) || variable->interface_slots.empty()) {
            continue;  // not an output interface
        }

        // TODO - https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7923
        // Need to redo logic to handle array of outputs
        if (variable->array_size > 1) {
            return false;
        }
        // It is not allowed to have Block Fragment or 64-bit vectors output in Frag shader
        // This means all Locations in slots will be the same
        location_map[variable->interface_slots[0].Location()].output = variable;
    }

    for (const auto &[location, attachment_info] : location_map) {
        const bool has_attachment =
            attachment_info.rendering_attachment_info && attachment_info.rendering_attachment_info->imageView != VK_NULL_HANDLE;
        const spirv::StageInterfaceVariable *output = attachment_info.output;

        if (has_attachment && !output) {
            // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9616
            // Need to understand when undefined or not
        } else if (!has_attachment && output) {
            // With alphaToCoverage, the write is not "discarded" as the alpha mask is still updated
            if (!last_bound_state.IsAlphaToCoverage() || location != 0) {
                const bool null_image_view = attachment_info.rendering_attachment_info &&
                                             attachment_info.rendering_attachment_info->imageView == VK_NULL_HANDLE;
                const LogObjectList objlist = last_bound_state.cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
                skip |= LogUndefinedValue("Undefined-Value-ShaderOutputNotConsumed-DynamicRendering", objlist, loc,
                                          "Inside the fragment shader, it writes to output Location %" PRIu32
                                          " but there is no VkRenderingInfo::pColorAttachments[%" PRIu32
                                          "]%s and this write is unused.\nSpec information at "
                                          "https://docs.vulkan.org/spec/latest/chapters/interfaces.html#interfaces-fragmentoutput",
                                          location, location, null_image_view ? " (imageView is VK_NULL_HANDLE)" : "");
            }
        } else if (has_attachment && output) {
            if (last_bound_state.cb_state.rendering_attachments.set_color_indexes ||
                last_bound_state.cb_state.rendering_attachments.set_color_locations) {
                // TODO - Handle VK_KHR_dynamic_rendering_local_read
                // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8887
                continue;
            }

            const auto image_view_state = Get<vvl::ImageView>(attachment_info.rendering_attachment_info->imageView);
            const uint32_t attachment_type = spirv::GetFormatType(image_view_state->create_info.format);

            // TODO - This create helper to do this via LastBound (and find other places doing similar thing)
            const spirv::Module *module_state = nullptr;
            if (pipeline && pipeline->fragment_shader_state && pipeline->fragment_shader_state->fragment_shader &&
                pipeline->fragment_shader_state->fragment_shader->spirv) {
                module_state = pipeline->fragment_shader_state->fragment_shader->spirv.get();
            } else if (!pipeline) {
                const vvl::ShaderObject *shader_object = last_bound_state.GetShaderStateIfValid(ShaderObjectStage::FRAGMENT);
                if (shader_object && shader_object->spirv) {
                    module_state = shader_object->spirv.get();
                }
            }
            ASSERT_AND_CONTINUE(module_state);
            const uint32_t output_type = module_state->GetNumericType(output->type_id);

            // Type checking
            if ((output_type & attachment_type) == 0) {
                const LogObjectList objlist = last_bound_state.cb_state.GetObjectList(VK_PIPELINE_BIND_POINT_GRAPHICS);
                skip |= LogUndefinedValue("Undefined-Value-ShaderFragmentOutputMismatch-DynamicRendering", objlist, loc,
                                          "Inside the fragment shader, it writes to output Location %" PRIu32
                                          " with a numeric type of %s but VkRenderingInfo::pColorAttachments[%" PRIu32
                                          "].imageView is created with %s (numeric type of %s) which does not match and the "
                                          "resulting values written will be undefined.\nSpec information at "
                                          "https://docs.vulkan.org/spec/latest/chapters/interfaces.html#interfaces-fragmentoutput",
                                          location, spirv::string_NumericType(output_type), location,
                                          string_VkFormat(image_view_state->create_info.format),
                                          spirv::string_NumericType(attachment_type));
            }
        } else {  // !attachment && !output
            // Means empty fragment shader and no color attachments
            // going to hit other VUs like VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08912
        }
    }

    return skip;
}

bool CoreChecks::ValidatePipelineTessellationStages(const spirv::Module &tesc_module_state,
                                                    const spirv::EntryPoint &tesc_entrypoint,
                                                    const spirv::Module &tese_module_state,
                                                    const spirv::EntryPoint &tese_entrypoint,
                                                    const Location &create_info_loc) const {
    bool skip = false;

    const auto tesc_subdivision = tesc_entrypoint.execution_mode.tessellation_subdivision;
    const auto tese_subdivision = tese_entrypoint.execution_mode.tessellation_subdivision;
    const auto tesc_patch_size = tesc_entrypoint.execution_mode.output_vertices;
    const auto tese_patch_size = tese_entrypoint.execution_mode.output_vertices;
    if (tesc_subdivision == 0 && tese_subdivision == 0) {
        const LogObjectList objlist(tesc_module_state.handle(), tese_module_state.handle());
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pStages-00732", objlist, create_info_loc,
                         "Subdivision type is not specified in either of tessellation stages");
    } else if (tesc_subdivision != 0 && tese_subdivision != 0 && tesc_subdivision != tese_subdivision) {
        const LogObjectList objlist(tesc_module_state.handle(), tese_module_state.handle());
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pStages-00733", objlist, create_info_loc,
                         "Subdivision type specified in tessellation control shader is %s, but subdivison type specified in "
                         "tessellation evaluation shader is %s",
                         string_SpvExecutionMode(tesc_subdivision), string_SpvExecutionMode(tese_subdivision));
    }
    if (tesc_patch_size == vvl::kU32Max && tese_patch_size == vvl::kU32Max) {
        const LogObjectList objlist(tesc_module_state.handle(), tese_module_state.handle());
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pStages-00734", objlist, create_info_loc,
                         "Output patch size is not specified in either of tessellation stages");
    } else if (tesc_patch_size != vvl::kU32Max && tese_patch_size != vvl::kU32Max && tesc_patch_size != tese_patch_size) {
        const LogObjectList objlist(tesc_module_state.handle(), tese_module_state.handle());
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pStages-00735", objlist, create_info_loc,
                         "Output patch size specified in tessellation control shader is %" PRIu32
                         ", but subdivison type specified in tessellation evaluation shader is %" PRIu32,
                         tesc_patch_size, tese_patch_size);
    }
    return skip;
}

// Validate that the shaders used by the given pipeline and store the active_slots
//  that are actually used by the pipeline into pPipeline->active_slots
bool CoreChecks::ValidateGraphicsPipelineShaderState(const vvl::Pipeline &pipeline, const Location &create_info_loc) const {
    bool skip = false;

    if (!(pipeline.pre_raster_state || pipeline.fragment_shader_state)) {
        // Only validate pipelines that contain shader stages
        return skip;
    }

    const ShaderStageState *vertex_stage = nullptr, *tesc_stage = nullptr, *tese_stage = nullptr, *fragment_stage = nullptr;
    for (uint32_t i = 0; i < pipeline.stage_states.size(); i++) {
        auto &stage_state = pipeline.stage_states[i];
        const VkShaderStageFlagBits stage = stage_state.GetStage();
        // Only validate the shader state once when added, not again when linked
        if ((stage & pipeline.linking_shaders) == 0) {
            skip |= ValidateShaderStage(stage_state, &pipeline, create_info_loc.dot(Field::pStages, i));
        }
        if (stage == VK_SHADER_STAGE_VERTEX_BIT) {
            vertex_stage = &stage_state;
        } else if (stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) {
            tesc_stage = &stage_state;
        } else if (stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) {
            tese_stage = &stage_state;
        } else if (stage == VK_SHADER_STAGE_FRAGMENT_BIT) {
            fragment_stage = &stage_state;
        }
    }

    // if the shader stages are no good individually, cross-stage validation is pointless.
    if (skip) return true;

    if (pipeline.vertex_input_state && vertex_stage && vertex_stage->entrypoint && vertex_stage->spirv_state &&
        !pipeline.IsDynamic(CB_DYNAMIC_STATE_VERTEX_INPUT_EXT)) {
        skip |=
            ValidateInterfaceVertexInput(pipeline, *vertex_stage->spirv_state.get(), *vertex_stage->entrypoint, create_info_loc);
    }

    if (pipeline.fragment_shader_state && fragment_stage && fragment_stage->entrypoint && fragment_stage->spirv_state) {
        skip |= ValidateInterfaceFragmentOutput(pipeline, *fragment_stage->spirv_state.get(), *fragment_stage->entrypoint,
                                                create_info_loc);
    }

    // We need to order the stages not how they are supplied in VkGraphicsPipelineCreateInfo::pStages but rather how they are
    // chained together in the pipeline. Note, we could be in the PreRaster GPL path, so there may not be a Fragment Shader see
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8443
    if (pipeline.stage_states.size() > 1) {
        const size_t not_found = vvl::kU32Max;
        auto get_stage = [&pipeline, not_found = not_found](VkShaderStageFlagBits stage) {
            for (size_t i = 0; i < pipeline.stage_states.size(); i++) {
                if (pipeline.stage_states[i].GetStage() == stage) {
                    return i;
                }
            }
            return not_found;
        };
        // Two graphic pipeline paths will be
        // Vert -> (Tess) -> (Geom) -> [Fragment]
        // (Task) -> Mesh -> [Fragment]
        // Pack both paths in, works because fragment are the last stage always
        const std::array ordered_stages = {VK_SHADER_STAGE_VERTEX_BIT,
                                           VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                           VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                                           VK_SHADER_STAGE_GEOMETRY_BIT,
                                           VK_SHADER_STAGE_FRAGMENT_BIT,
                                           VK_SHADER_STAGE_TASK_BIT_EXT,
                                           VK_SHADER_STAGE_MESH_BIT_EXT,
                                           VK_SHADER_STAGE_FRAGMENT_BIT};

        // Use active_shaders because with GPL, you will need to check PreRaster and Fragment together at linking
        const bool has_vertex_shader = pipeline.active_shaders & VK_SHADER_STAGE_VERTEX_BIT;
        const bool has_task_shader = pipeline.active_shaders & VK_SHADER_STAGE_TASK_BIT_EXT;

        size_t ordered_stages_index = has_vertex_shader ? 0 : has_task_shader ? 5 : 6;
        size_t producer_index = get_stage(ordered_stages[ordered_stages_index++]);
        assert(producer_index != not_found);

        size_t consumer_index = not_found;
        // start at 1 as we are always searching for the next consumer
        for (size_t i = 1; i < pipeline.stage_states.size(); i++) {
            // Find current producer's consumer
            while (ordered_stages_index < ordered_stages.size()) {
                consumer_index = get_stage(ordered_stages[ordered_stages_index++]);
                if (consumer_index != not_found) break;
            }

            const auto &producer = pipeline.stage_states[producer_index];
            const auto &consumer = pipeline.stage_states[consumer_index];

            const std::shared_ptr<const spirv::Module> &producer_spirv =
                producer.spirv_state ? producer.spirv_state : producer.module_state->spirv;
            const std::shared_ptr<const spirv::Module> &consumer_spirv =
                consumer.spirv_state ? consumer.spirv_state : consumer.module_state->spirv;

            if (consumer_spirv && producer_spirv && consumer.entrypoint && producer.entrypoint) {
                skip |= ValidateInterfaceBetweenStages(*producer_spirv.get(), *producer.entrypoint, *consumer_spirv.get(),
                                                       *consumer.entrypoint, create_info_loc);
            }

            producer_index = consumer_index;
        }
    }

    // Don't check any color attachments if rasterization is disabled
    if (fragment_stage && fragment_stage->entrypoint && fragment_stage->spirv_state && !pipeline.RasterizationDisabled()) {
        const auto &rp_state = pipeline.RenderPassState();
        // Dynamic Rendering is done at draw time incase the user has VK_EXT_dynamic_rendering_unused_attachments we can't do all
        // the checks at this time
        if (rp_state && !rp_state->UsesDynamicRendering()) {
            skip |= ValidateFsOutputsAgainstRenderPass(*fragment_stage->spirv_state.get(), *fragment_stage->entrypoint, pipeline,
                                                       pipeline.Subpass(), create_info_loc);
        }
    }

    if (tesc_stage && tesc_stage->spirv_state && tesc_stage->entrypoint && tese_stage && tese_stage->spirv_state &&
        tese_stage->entrypoint) {
        skip |= ValidatePipelineTessellationStages(*tesc_stage->spirv_state, *tesc_stage->entrypoint, *tese_stage->spirv_state,
                                                   *tese_stage->entrypoint, create_info_loc);
    }

    return skip;
}
