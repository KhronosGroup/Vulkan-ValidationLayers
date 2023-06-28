/*
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <iostream>
#include "spirv_hopper.h"
#include <unordered_map>
#include "vulkan_object.h"

#include "generated/vk_function_pointers.h"
#include "generated/vk_typemap_helper.h"

#define REFLECT_SUCCESS(err)                                          \
    {                                                                 \
        const SpvReflectResult result = err;                          \
        if (result != SpvReflectResult::SPV_REFLECT_RESULT_SUCCESS) { \
            return false;                                             \
        }                                                             \
    }

#define VK_SUCCESS(err)              \
    {                                \
        const VkResult result = err; \
        if (result != VK_SUCCESS) {  \
            return false;            \
        }                            \
    }

Hopper::Hopper(VulkanInstance& vk, size_t file_size, const void* spirv_data)
    : vk(vk), file_size(file_size), spirv_data(spirv_data) {}

Hopper::~Hopper() {
    for (const auto& shader_module : shader_modules) {
        if (shader_module != VK_NULL_HANDLE) {
            vk::DestroyShaderModule(vk.device, shader_module, nullptr);
        }
    }
    if (render_pass != VK_NULL_HANDLE) {
        vk::DestroyRenderPass(vk.device, render_pass, nullptr);
    }
    if (pipeline_layout != VK_NULL_HANDLE) {
        vk::DestroyPipelineLayout(vk.device, pipeline_layout, nullptr);
    }
    if (pipeline != VK_NULL_HANDLE) {
        vk::DestroyPipeline(vk.device, pipeline, nullptr);
    }

    spvReflectDestroyShaderModule(&module);
}

bool Hopper::Reflect() {
    REFLECT_SUCCESS(spvReflectCreateShaderModule2(SPV_REFLECT_MODULE_FLAG_NO_COPY, file_size, spirv_data, &module));

    if (module.entry_point_count == 0) {
        std::cout << "WARNING: No entrypoint, no way to test\n";
        return true;
    } else if (module.entry_point_count > 1) {
        uint32_t shader_stage_set = 0x0;
        for (uint32_t i = 0; i < module.entry_point_count; i++) {
            if (shader_stage_set & module.entry_points[i].shader_stage) {
                std::cout << "Warning: Can't handle duplicate Execution Modes in moudle, might have duplciated objects\n";
            }
            shader_stage_set |= module.entry_points[i].shader_stage;
        }
        // TODO - Just take first entry point for now
        std::cout << "WARNING: Only the first entry point modules is used\n";
    }
    entry_point = module.entry_points[0];
    shader_stage = static_cast<VkShaderStageFlagBits>(module.shader_stage);

    uint32_t count = 0;
    REFLECT_SUCCESS(spvReflectEnumerateEntryPointInputVariables(&module, entry_point.name, &count, nullptr));
    input_variables.resize(count);
    REFLECT_SUCCESS(spvReflectEnumerateEntryPointInputVariables(&module, entry_point.name, &count, input_variables.data()));

    REFLECT_SUCCESS(spvReflectEnumerateEntryPointOutputVariables(&module, entry_point.name, &count, nullptr));
    output_variables.resize(count);
    REFLECT_SUCCESS(spvReflectEnumerateEntryPointOutputVariables(&module, entry_point.name, &count, output_variables.data()));

    REFLECT_SUCCESS(spvReflectEnumerateEntryPointPushConstantBlocks(&module, entry_point.name, &count, nullptr));
    push_constants.resize(count);
    REFLECT_SUCCESS(spvReflectEnumerateEntryPointPushConstantBlocks(&module, entry_point.name, &count, push_constants.data()));

    REFLECT_SUCCESS(spvReflectEnumerateEntryPointDescriptorSets(&module, entry_point.name, &count, nullptr));
    descriptor_sets.resize(count);
    REFLECT_SUCCESS(spvReflectEnumerateEntryPointDescriptorSets(&module, entry_point.name, &count, descriptor_sets.data()));
    return true;
}

bool Hopper::CreatePipelineLayout() {
    // get the VkDescriptorSetLayouts
    std::vector<VkDescriptorSetLayout> layouts;
    struct DescriptorSetLayoutData {
        uint32_t set;
        VkDescriptorSetLayoutCreateInfo ds_create_info;
        std::vector<VkDescriptorSetLayoutBinding> bindings;
    };

    std::vector<DescriptorSetLayoutData> set_layouts(descriptor_sets.size(), DescriptorSetLayoutData{});
    for (uint32_t set_id = 0; set_id < descriptor_sets.size(); set_id++) {
        const SpvReflectDescriptorSet& reflect_set = *(descriptor_sets[set_id]);
        DescriptorSetLayoutData& layout = set_layouts[set_id];

        // There can be a template descriptor where 2 variables share the same slot, but will be duplicated here
        // <binding, index into bindings to grab again>
        std::unordered_map<uint32_t, uint32_t> used_bindings;

        for (uint32_t i = 0; i < reflect_set.binding_count; i++) {
            const SpvReflectDescriptorBinding& reflect = *(reflect_set.bindings[i]);
            VkDescriptorSetLayoutBinding new_binding;

            new_binding.descriptorType = static_cast<VkDescriptorType>(reflect.descriptor_type);
            new_binding.binding = reflect.binding;
            new_binding.descriptorCount = 1;
            for (uint32_t dim_id = 0; dim_id < reflect.array.dims_count; dim_id++) {
                const uint32_t dim = reflect.array.dims[dim_id];
                // an OpTypeRuntimeArray will have a zero dim
                // https://github.com/KhronosGroup/SPIRV-Reflect/issues/177
                if (dim != 0) {
                    new_binding.descriptorCount *= reflect.array.dims[dim_id];
                }
            }
            new_binding.stageFlags = shader_stage;
            new_binding.pImmutableSamplers = nullptr;

            if (auto used = used_bindings.find(new_binding.binding); used != used_bindings.end()) {
                VkDescriptorSetLayoutBinding& other_binding = layout.bindings[used->second];
                if ((other_binding.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER &&
                     new_binding.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) ||
                    (other_binding.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE &&
                     new_binding.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER)) {
                    // COMBINED images in HLSL can actually be seperated types sharing same slot
                    other_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                } else {
                    // templated descriptor - nothing to do for now
                }
                continue;  // prevent duplicated bindings index
            }

            if (new_binding.descriptorType == VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
                input_attachments.push_back(&reflect);
            }
            layout.bindings.push_back(new_binding);
            used_bindings[new_binding.binding] = static_cast<uint32_t>(layout.bindings.size() - 1);
        }
        layout.set = reflect_set.set;
        layout.ds_create_info = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
        layout.ds_create_info.bindingCount = static_cast<uint32_t>(layout.bindings.size());
        layout.ds_create_info.pBindings = layout.bindings.data();
    }

    uint32_t id = 0;
    for (const DescriptorSetLayoutData& set_layout : set_layouts) {
        while (set_layout.set > id) {
            id++;
            VkDescriptorSetLayout layout = VK_NULL_HANDLE;
            VK_SUCCESS(vk::CreateDescriptorSetLayout(vk.device, &set_layout.ds_create_info, nullptr, &layout));
            layouts.push_back(layout);
        }
        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        VK_SUCCESS(vk::CreateDescriptorSetLayout(vk.device, &set_layout.ds_create_info, nullptr, &layout));
        layouts.push_back(layout);
        id++;
    }

    // Get the push constants
    std::vector<VkPushConstantRange> push_constant_ranges;
    for (uint32_t i = 0; i < push_constants.size(); i++) {
        VkPushConstantRange range = {};
        range.stageFlags = static_cast<VkShaderStageFlagBits>(shader_stage);
        range.size = push_constants[i]->size;
        range.offset = push_constants[i]->offset;
        push_constant_ranges.push_back(range);
    }

    // Create the pipeline layout
    {
        auto pipeline_layout_info = LvlInitStruct<VkPipelineLayoutCreateInfo>();
        pipeline_layout_info.flags = 0;
        pipeline_layout_info.pSetLayouts = layouts.data();
        pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipeline_layout_info.pPushConstantRanges = push_constant_ranges.data();
        pipeline_layout_info.pushConstantRangeCount = static_cast<uint32_t>(push_constant_ranges.size());
        VK_SUCCESS(vk::CreatePipelineLayout(vk.device, &pipeline_layout_info, nullptr, &pipeline_layout));
    }

    // cleanup
    for (VkDescriptorSetLayout& layout : layouts) {
        vk::DestroyDescriptorSetLayout(vk.device, layout, nullptr);
    }
    return true;
}

// Some Builtins like 'gl_in' of tessellation shaders are structs and so the gl_* identifiers are reserved. Can't assume all
// structs are Builtins.
bool Hopper::IsBuiltinType(SpvReflectInterfaceVariable* variable) {
    return (variable->built_in >= 0 || (variable->name && std::string(variable->name).find("gl_") == 0));
}

bool Hopper::CreateShaderStage(size_t code_size, const void* code, VkShaderStageFlagBits stage, const char* name) {
    auto shader_module_info = LvlInitStruct<VkShaderModuleCreateInfo>();
    shader_module_info.flags = 0;
    shader_module_info.pCode = reinterpret_cast<const uint32_t*>(code);
    shader_module_info.codeSize = code_size;
    VkShaderModule shader_module;
    VK_SUCCESS(vk::CreateShaderModule(vk.device, &shader_module_info, nullptr, &shader_module));
    shader_modules.push_back(shader_module);

    auto shader_stage_info = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
    shader_stage_info.flags = 0;
    shader_stage_info.stage = stage;
    shader_stage_info.module = shader_module;
    shader_stage_info.pName = name;
    shader_stage_info.pSpecializationInfo = nullptr;
    shader_stages_info.push_back(shader_stage_info);
    return true;
}

static uint32_t DescriptionLocationsConsumed(SpvReflectTypeDescription& description) {
    const uint32_t scalar_bytes = description.traits.numeric.scalar.width / 8;
    const uint32_t bytes_in_location = 16;
    uint32_t vector_location_consumed = 1;
    uint32_t column_count = 1;

    // SpvOpTypeVector
    if (description.traits.numeric.vector.component_count > 0) {
        uint32_t vector_length = description.traits.numeric.vector.component_count;
        vector_location_consumed = (vector_length * scalar_bytes > bytes_in_location) ? 2 : 1;
    }
    // SpvOpTypeMatrix
    if ((description.traits.numeric.matrix.column_count > 0) && (description.traits.numeric.matrix.row_count > 0)) {
        column_count = description.traits.numeric.matrix.column_count;
    }

    return vector_location_consumed * column_count;
}

bool Hopper::CreateVertexAttributeDescriptions(SpvReflectInterfaceVariable& variable) {
    bool success = true;
    SpvReflectTypeDescription& description = *variable.type_description;
    if (variable.member_count > 0) {
        if (description.op == SpvOp::SpvOpTypeArray) {
            // SPIRV-Reflect can't handle array-of-structs currently
            return false;
        }

        // Walk down struct
        for (uint32_t i = 0; i < variable.member_count; i++) {
            success &= CreateVertexAttributeDescriptions(variable.members[i]);
        }
    } else {
        uint32_t array_elements = 1;
        for (uint32_t i = 0; i < description.traits.array.dims_count; i++) {
            array_elements *= description.traits.array.dims[i];
        }

        const uint32_t locations = DescriptionLocationsConsumed(description) * array_elements;

        const bool is_block_location = (variable.location == 0);
        const uint32_t start_location = is_block_location ? block_location : variable.location;

        if (is_block_location) {
            block_location += locations;
        }

        for (uint32_t i = 0; i < locations; i++) {
            // The only things we need are the location and format for each Vertex Input
            // binding and offset values don't matter
            vertex_input_attributes.push_back({start_location + i, 0, static_cast<VkFormat>(variable.format), 0});
        }
    }
    return success;
}

bool Hopper::CreateGraphicsPipeline() {
    // Create pass through shaders for stages at need one
    bool success = true;
    switch (shader_stage) {
        case VK_SHADER_STAGE_GEOMETRY_BIT:
        case VK_SHADER_STAGE_FRAGMENT_BIT:
            success &= CreatePassThroughVertex();
            break;
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            success &= CreatePassThroughVertexNoInterface();
            success &= CreatePassThroughTessellationControl();
            break;
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
            success &= CreatePassThroughVertexNoInterface();
            success &= CreatePassThroughTessellationEval();
            break;
        default:
            break;
    }
    if (!success) return false;

    uint32_t attachment_count = 0;
    // VkPipelineColorBlendStateCreateInfo
    {
        if (shader_stage == VK_SHADER_STAGE_FRAGMENT_BIT) {
            std::vector<uint32_t> fragment_attachments;
            for (uint32_t i = 0; i < output_variables.size(); i++) {
                attachment_count = std::max(attachment_count, output_variables[i]->location);
            }
            // locations are 0 indexed so increment by 1
            attachment_count += 1;
        }

        color_attachment_references.resize(attachment_count);
        color_blend_attachments.resize(attachment_count);
        for (uint32_t i = 0; i < attachment_count; i++) {
            color_attachment_references[i].attachment = 0;
            color_attachment_references[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            color_blend_attachments[i].blendEnable = VK_FALSE;
            color_blend_attachments[i].colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        }

        vk.color_blend_state.attachmentCount = static_cast<uint32_t>(color_blend_attachments.size());
        vk.color_blend_state.pAttachments = color_blend_attachments.data();
    }

    // VkPipelineVertexInputStateCreateInfo
    {
        vk.vertex_input_state.vertexAttributeDescriptionCount = 0;
        vk.vertex_input_state.pVertexAttributeDescriptions = nullptr;

        if (shader_stage == VK_SHADER_STAGE_VERTEX_BIT) {
            for (uint32_t i = 0; i < input_variables.size(); i++) {
                SpvReflectInterfaceVariable* input_variable = input_variables[i];
                // built in types (gl_VertexIndex, etc) are not part of the vertex input
                // Any negative value means it is not part of the SpvBuiltIn
                if (IsBuiltinType(input_variable) == true) {
                    continue;
                }

                // Locations can either be set on the Struct or the first level members.
                // It is not valid to have Locations decorations on nested struct blocks.
                // Since the value is "zero" for not being set, simpler to just know here where it is decorated.
                // No decoration == "zero" implicitly (which is the default)
                block_location = input_variable->location;

                success = CreateVertexAttributeDescriptions(*input_variable);
                if (!success) return false;
            }

            vk.vertex_input_state.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_input_attributes.size());
            vk.vertex_input_state.pVertexAttributeDescriptions = vertex_input_attributes.data();
        }
    }

    // VkPipelineInputAssemblyStateCreateInfo
    {
        vk.input_assembly_state.topology = ((shader_stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) ||
                                            (shader_stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT))
                                               ? VK_PRIMITIVE_TOPOLOGY_PATCH_LIST
                                               : VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }

    // Subpass and Renderpass
    {
        // Find all the attachments and create a Framebuffer and RenderPass
        VkSubpassDescription subpass_description = {};
        subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_description.colorAttachmentCount = attachment_count;
        subpass_description.pColorAttachments = color_attachment_references.data();

        // Create dummy input attachments if the shader requires input attachments.
        std::vector<VkAttachmentReference> input_attachment_reference;
        std::vector<VkAttachmentDescription> input_attachment_description;
        for (uint32_t i = 0; i < input_attachments.size(); i++) {
            // input_attachment_reference.push_back({input_attachments[i].input_attachment_index, VK_IMAGE_LAYOUT_GENERAL});
            input_attachment_reference.push_back({0, VK_IMAGE_LAYOUT_GENERAL});
        }
        subpass_description.inputAttachmentCount = static_cast<uint32_t>(input_attachment_reference.size());
        subpass_description.pInputAttachments = input_attachment_reference.data();

        auto render_pass_info = LvlInitStruct<VkRenderPassCreateInfo>();
        render_pass_info.flags = 0;
        render_pass_info.attachmentCount = 1;
        render_pass_info.pAttachments = &vk.basic_attachment_description;
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass_description;
        render_pass_info.dependencyCount = 0;
        render_pass_info.pDependencies = nullptr;
        VK_SUCCESS(vk::CreateRenderPass(vk.device, &render_pass_info, nullptr, &render_pass));
    }

    auto pipeline_info = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipeline_info.flags = 0;
    pipeline_info.stageCount = static_cast<uint32_t>(shader_stages_info.size());
    pipeline_info.pStages = shader_stages_info.data();
    pipeline_info.pVertexInputState = &vk.vertex_input_state;
    pipeline_info.pInputAssemblyState = &vk.input_assembly_state;
    pipeline_info.pTessellationState = &vk.tessellation_state;
    pipeline_info.pViewportState = &vk.viewport_input_state;
    pipeline_info.pRasterizationState = &vk.rasterization_state;
    pipeline_info.pMultisampleState = &vk.multisample_state;
    pipeline_info.pDepthStencilState = nullptr;
    pipeline_info.pColorBlendState = &vk.color_blend_state;
    pipeline_info.pDynamicState = nullptr;
    pipeline_info.layout = pipeline_layout;
    pipeline_info.renderPass = render_pass;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

    VK_SUCCESS(vk::CreateGraphicsPipelines(vk.device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline));
    return success;
}

bool Hopper::CreateGraphicsMeshPipeline() {
    bool success = true;
    // Only a mesh shader is required, if it is a task shader, create a mesh shader to match
    if (shader_stage == VK_SHADER_STAGE_TASK_BIT_EXT) {
        CreatePassThroughMesh();
    }

    // Renderpass
    {
        // Find all the attachments and create a Framebuffer and RenderPass
        VkSubpassDescription subpass_description = {};
        subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_description.colorAttachmentCount = 0;
        subpass_description.inputAttachmentCount = 0;  // inputAttachment only allowed in Frag
        subpass_description.pResolveAttachments = nullptr;
        subpass_description.pDepthStencilAttachment = nullptr;
        auto render_pass_info = LvlInitStruct<VkRenderPassCreateInfo>();
        render_pass_info.flags = 0;
        render_pass_info.attachmentCount = 1;
        render_pass_info.pAttachments = &vk.basic_attachment_description;
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass_description;
        render_pass_info.dependencyCount = 0;
        render_pass_info.pDependencies = nullptr;
        VK_SUCCESS(vk::CreateRenderPass(vk.device, &render_pass_info, nullptr, &render_pass));
    }

    auto pipeline_info = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipeline_info.flags = 0;
    pipeline_info.stageCount = static_cast<uint32_t>(shader_stages_info.size());
    pipeline_info.pStages = shader_stages_info.data();
    pipeline_info.pVertexInputState = nullptr;
    pipeline_info.pInputAssemblyState = nullptr;
    pipeline_info.pTessellationState = nullptr;
    pipeline_info.pViewportState = &vk.viewport_input_state;
    pipeline_info.pRasterizationState = &vk.rasterization_state;
    pipeline_info.pMultisampleState = &vk.multisample_state;
    pipeline_info.pDepthStencilState = nullptr;
    pipeline_info.pColorBlendState = nullptr;
    pipeline_info.pDynamicState = nullptr;
    pipeline_info.layout = pipeline_layout;
    pipeline_info.renderPass = render_pass;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

    VK_SUCCESS(vk::CreateGraphicsPipelines(vk.device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline));
    return success;
}

bool Hopper::CreateComputePipeline() {
    auto pipeline_info = LvlInitStruct<VkComputePipelineCreateInfo>();
    pipeline_info.flags = 0;
    pipeline_info.stage = shader_stages_info[0];
    pipeline_info.layout = pipeline_layout;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

    VK_SUCCESS(vk::CreateComputePipelines(vk.device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline));
    return true;
}

bool Hopper::Run() {
    if (!Reflect()) return false;
    if (!CreatePipelineLayout()) return false;
    // Create shader module for shader we are testing
    if (!CreateShaderStage(file_size, spirv_data, shader_stage, entry_point.name)) return false;

    bool success = true;
    switch (shader_stage) {
        case VK_SHADER_STAGE_VERTEX_BIT:
        case VK_SHADER_STAGE_GEOMETRY_BIT:
        case VK_SHADER_STAGE_FRAGMENT_BIT:
            success = CreateGraphicsPipeline();
            break;
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            break;
        case VK_SHADER_STAGE_COMPUTE_BIT:
            success = CreateComputePipeline();
            break;
        case VK_SHADER_STAGE_TASK_BIT_EXT:
        case VK_SHADER_STAGE_MESH_BIT_EXT:
            success = CreateGraphicsMeshPipeline();
            break;
        case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
        case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
        case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
        case VK_SHADER_STAGE_MISS_BIT_KHR:
        case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
        case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
            std::cout << "Currently Ray Tracing stages not supported\n";
            break;
        case VK_SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI:
        case VK_SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI:
            std::cout << "Currently Subpass Shading stages not supported\n";
            break;
        default:
            std::cout << "Shader stage not found\n";
            return false;
            break;
    }

    return success;
}
