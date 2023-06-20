
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

#include "spirv_hopper.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include <iostream>

static constexpr bool IsFloatFormat(SpvReflectFormat format) {
    switch (format) {
        case SPV_REFLECT_FORMAT_R32_SFLOAT:
        case SPV_REFLECT_FORMAT_R32G32_SFLOAT:
        case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:
        case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:
        case SPV_REFLECT_FORMAT_R64_SFLOAT:
        case SPV_REFLECT_FORMAT_R64G64_SFLOAT:
        case SPV_REFLECT_FORMAT_R64G64B64_SFLOAT:
        case SPV_REFLECT_FORMAT_R64G64B64A64_SFLOAT:
            return true;
        default:
            return false;
    }
}

// Can't pass the variable as members in structs can't get information
// But need the variable format to know which Numeric Type it is (float vs uint vs int)
std::string Hopper::GetTypeDescription(SpvReflectTypeDescription& description, SpvReflectFormat format) {
    std::string type;
    if (description.op == SpvOp::SpvOpTypeArray) {
        // An array input has a <type> output from the shader
        // SPIRV Reflect does not store the type of the array so the type
        // must be determined by the description
        if ((description.traits.numeric.matrix.column_count > 0) && (description.traits.numeric.matrix.row_count > 0)) {
            description.op = SpvOp::SpvOpTypeMatrix;
        } else if (description.traits.numeric.vector.component_count > 0) {
            description.op = SpvOp::SpvOpTypeVector;
        } else {
            // either a float, bool, or int. Must be inferred from format
            if (description.type_flags & SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_FLOAT) {
                description.op = SpvOp::SpvOpTypeFloat;
            } else if (description.type_flags & SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_INT) {
                description.op = SpvOp::SpvOpTypeInt;
            }
        }
    }

    switch (description.op) {
        case SpvOp::SpvOpTypeBool: {
            type += "bool";
            break;
        }
        case SpvOp::SpvOpTypeFloat: {
            type += "float";
            break;
        }
        case SpvOp::SpvOpTypeInt: {
            if (description.traits.numeric.scalar.signedness == 0) {
                type += "u";
            }
            type += "int";
            break;
        }
        case SpvOp::SpvOpTypeVector: {
            if (!IsFloatFormat(format)) {
                type += (description.traits.numeric.scalar.signedness == 0) ? "u" : "i";
            }
            type += "vec";
            const uint32_t component_count = description.traits.numeric.vector.component_count;
            type += std::to_string(component_count);
            break;
        }
        case SpvOp::SpvOpTypeMatrix: {
            type += "mat";
            const uint32_t column_count = description.traits.numeric.matrix.column_count;
            const uint32_t row_count = description.traits.numeric.matrix.row_count;
            if (column_count == row_count) {
                type += std::to_string(column_count);
            } else {
                type += std::to_string(column_count) + "x" + std::to_string(row_count);
            }
            break;
        }
        default:
            // Has a custom type (ex. struct)
            if (description.type_name != nullptr) {
                return description.type_name;
            }
            type += "UNKNOWN_STRUCT_" + std::to_string(description.id);  // Unsupported type
            break;
    }
    return type;
}

std::string Hopper::DefineCustomStruct(SpvReflectInterfaceVariable& variable) {
    SpvReflectTypeDescription& description = *variable.type_description;
    std::string shader = "struct ";
    shader += (description.type_name) ? description.type_name : "UNKNOWN_STRUCT_" + std::to_string(description.id);
    shader += " {\n";
    for (uint32_t i = 0; i < description.member_count; i++) {
        shader += "\t";
        shader += GetTypeDescription(description.members[i], variable.members[i].format);
        shader += " ";
        shader += (description.members[i].struct_member_name) ? description.members[i].struct_member_name
                                                              : "UNKNOWN_MEMBER_" + std::to_string(i);
        for (uint32_t j = 0; j < description.members[i].traits.array.dims_count; j++) {
            shader += "[" + std::to_string(description.members[i].traits.array.dims[j]) + "]";
        }
        shader += ";\n";
    }
    shader += "};\n";
    return shader;
}

bool Hopper::CreatePassThroughVertex() {
    std::string shader = "#version 450\n";
    for (auto variable : input_variables) {
        if (IsBuiltinType(variable) == true) {
            continue;
        } else if ((shader_stage == VK_SHADER_STAGE_GEOMETRY_BIT && variable->format == SPV_REFLECT_FORMAT_UNDEFINED)) {
            // TODO - Figure out why some Geometry shaders can these bogus variables
            continue;
        }

        // Need to define struct to match
        if (variable->type_description->op == SpvOp::SpvOpTypeStruct) {
            shader += DefineCustomStruct(*variable);
        }

        // over 3 is invalid, means not set, zero is default implicit value
        const uint32_t component = (variable->component > 3) ? 0 : variable->component;

        shader += "layout(location = ";
        shader += std::to_string(variable->location);
        if (component > 0) {
            shader += ", component = " + std::to_string(component);
        }
        shader += ") out ";
        shader += GetTypeDescription(*variable->type_description, variable->format);
        shader += " ";
        // Names might not be valid GLSL names, so just give unique name
        shader += "var_" + std::to_string(variable->location) + "_" + std::to_string(component);

        // Vertex output into Gemometry are not actually arrays
        if (shader_stage != VK_SHADER_STAGE_GEOMETRY_BIT) {
            for (uint32_t i = 0; i < variable->array.dims_count; i++) {
                shader += "[" + std::to_string(variable->array.dims[i]) + "]";
            }
        }
        shader += ";\n";
    }

    shader += "void main() { gl_Position = vec4(1.0); }";
    return BuildPassThroughShader(shader, VK_SHADER_STAGE_VERTEX_BIT);
}

bool Hopper::CreatePassThroughVertexNoInterface() {
    std::string shader = "#version 450\nvoid main() { }";
    return BuildPassThroughShader(shader, VK_SHADER_STAGE_VERTEX_BIT);
}

// TODO - The CreatePassThrough*() function share a lot, could make a common
bool Hopper::CreatePassThroughTessellationEval() {
    std::string shader = "#version 450\n";
    const size_t patchIndex = shader.size();
    shader += "layout(triangles, equal_spacing, cw) in;\n";

    for (auto variable : output_variables) {
        if (IsBuiltinType(variable) == true) {
            continue;
        }

        // Need to define struct to match
        if (variable->type_description->op == SpvOp::SpvOpTypeStruct) {
            shader += DefineCustomStruct(*variable);
        }

        // over 3 is invalid, means not set, zero is default implicit value
        const uint32_t component = (variable->component > 3) ? 0 : variable->component;

        shader += "layout(location = ";
        shader += std::to_string(variable->location);
        if (component > 0) {
            shader += ", component = " + std::to_string(component);
        }
        shader += ") in ";
        if (variable->type_description->type_name != nullptr) {
            std::string patch = DefineCustomStruct(*variable);
            shader.insert(patchIndex, patch);
        }
        shader += GetTypeDescription(*variable->type_description, variable->format);
        shader += " ";
        shader += "var_" + std::to_string(variable->location) + "_" + std::to_string(component);
        shader += "[]";
        shader += ";\n";
    }
    shader += "void main() { gl_Position = vec4(1.0); }";
    return BuildPassThroughShader(shader, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
}

bool Hopper::CreatePassThroughTessellationControl() {
    std::string shader = "#version 450\n";
    const size_t patchIndex = shader.size();
    shader += "layout(vertices = 3) out;\n";

    for (auto variable : input_variables) {
        if (IsBuiltinType(variable) == true) {
            continue;
        }

        // Need to define struct to match
        if (variable->type_description->op == SpvOp::SpvOpTypeStruct) {
            shader += DefineCustomStruct(*variable);
        }

        // over 3 is invalid, means not set, zero is default implicit value
        const uint32_t component = (variable->component > 3) ? 0 : variable->component;

        shader += "layout(location = ";
        shader += std::to_string(variable->location);
        if (component > 0) {
            shader += ", component = " + std::to_string(component);
        }
        shader += ") out ";
        if (variable->type_description->type_name != nullptr) {
            std::string patch = DefineCustomStruct(*variable);
            shader.insert(patchIndex, patch);
        }
        shader += GetTypeDescription(*variable->type_description, variable->format);
        shader += " ";
        shader += "var_" + std::to_string(variable->location) + "_" + std::to_string(component);
        shader += "[]";
        shader += ";\n";
    }
    shader += "void main() { }";
    return BuildPassThroughShader(shader, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
}

bool Hopper::CreatePassThroughMesh() {
    static const uint32_t simple_mesh_spv[52] = {
        0x07230203, 0x00010600, 0x00070000, 0x00000005, 0x00000000, 0x00020011, 0x000014A3, 0x0006000A, 0x5F565053,
        0x5F545845, 0x6873656D, 0x6168735F, 0x00726564, 0x0003000E, 0x00000000, 0x00000001, 0x0005000F, 0x000014F5,
        0x00000001, 0x6E69616D, 0x00000000, 0x00060010, 0x00000001, 0x00000011, 0x00000001, 0x00000001, 0x00000001,
        0x00040010, 0x00000001, 0x0000001A, 0x00000003, 0x00040010, 0x00000001, 0x00001496, 0x00000001, 0x00030010,
        0x00000001, 0x000014B2, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00050036, 0x00000002,
        0x00000001, 0x00000000, 0x00000003, 0x000200F8, 0x00000004, 0x000100FD, 0x00010038};
    return CreateShaderStage(sizeof(simple_mesh_spv), simple_mesh_spv, VK_SHADER_STAGE_MESH_BIT_EXT);
}

static TBuiltInResource InitResources() {
    TBuiltInResource resources;
    resources.maxLights = 32;
    resources.maxClipPlanes = 6;
    resources.maxTextureUnits = 32;
    resources.maxTextureCoords = 32;
    resources.maxVertexAttribs = 64;
    resources.maxVertexUniformComponents = 4096;
    resources.maxVaryingFloats = 64;
    resources.maxVertexTextureImageUnits = 32;
    resources.maxCombinedTextureImageUnits = 80;
    resources.maxTextureImageUnits = 32;
    resources.maxFragmentUniformComponents = 4096;
    resources.maxDrawBuffers = 32;
    resources.maxVertexUniformVectors = 128;
    resources.maxVaryingVectors = 8;
    resources.maxFragmentUniformVectors = 16;
    resources.maxVertexOutputVectors = 16;
    resources.maxFragmentInputVectors = 15;
    resources.minProgramTexelOffset = -8;
    resources.maxProgramTexelOffset = 7;
    resources.maxClipDistances = 8;
    resources.maxComputeWorkGroupCountX = 65535;
    resources.maxComputeWorkGroupCountY = 65535;
    resources.maxComputeWorkGroupCountZ = 65535;
    resources.maxComputeWorkGroupSizeX = 1024;
    resources.maxComputeWorkGroupSizeY = 1024;
    resources.maxComputeWorkGroupSizeZ = 64;
    resources.maxComputeUniformComponents = 1024;
    resources.maxComputeTextureImageUnits = 16;
    resources.maxComputeImageUniforms = 8;
    resources.maxComputeAtomicCounters = 8;
    resources.maxComputeAtomicCounterBuffers = 1;
    resources.maxVaryingComponents = 60;
    resources.maxVertexOutputComponents = 64;
    resources.maxGeometryInputComponents = 64;
    resources.maxGeometryOutputComponents = 128;
    resources.maxFragmentInputComponents = 128;
    resources.maxImageUnits = 8;
    resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
    resources.maxCombinedShaderOutputResources = 8;
    resources.maxImageSamples = 0;
    resources.maxVertexImageUniforms = 0;
    resources.maxTessControlImageUniforms = 0;
    resources.maxTessEvaluationImageUniforms = 0;
    resources.maxGeometryImageUniforms = 0;
    resources.maxFragmentImageUniforms = 8;
    resources.maxCombinedImageUniforms = 8;
    resources.maxGeometryTextureImageUnits = 16;
    resources.maxGeometryOutputVertices = 256;
    resources.maxGeometryTotalOutputComponents = 1024;
    resources.maxGeometryUniformComponents = 1024;
    resources.maxGeometryVaryingComponents = 64;
    resources.maxTessControlInputComponents = 128;
    resources.maxTessControlOutputComponents = 128;
    resources.maxTessControlTextureImageUnits = 16;
    resources.maxTessControlUniformComponents = 1024;
    resources.maxTessControlTotalOutputComponents = 4096;
    resources.maxTessEvaluationInputComponents = 128;
    resources.maxTessEvaluationOutputComponents = 128;
    resources.maxTessEvaluationTextureImageUnits = 16;
    resources.maxTessEvaluationUniformComponents = 1024;
    resources.maxTessPatchComponents = 120;
    resources.maxPatchVertices = 32;
    resources.maxTessGenLevel = 64;
    resources.maxViewports = 16;
    resources.maxVertexAtomicCounters = 0;
    resources.maxTessControlAtomicCounters = 0;
    resources.maxTessEvaluationAtomicCounters = 0;
    resources.maxGeometryAtomicCounters = 0;
    resources.maxFragmentAtomicCounters = 8;
    resources.maxCombinedAtomicCounters = 8;
    resources.maxAtomicCounterBindings = 1;
    resources.maxVertexAtomicCounterBuffers = 0;
    resources.maxTessControlAtomicCounterBuffers = 0;
    resources.maxTessEvaluationAtomicCounterBuffers = 0;
    resources.maxGeometryAtomicCounterBuffers = 0;
    resources.maxFragmentAtomicCounterBuffers = 1;
    resources.maxCombinedAtomicCounterBuffers = 1;
    resources.maxAtomicCounterBufferSize = 16384;
    resources.maxTransformFeedbackBuffers = 4;
    resources.maxTransformFeedbackInterleavedComponents = 64;
    resources.maxCullDistances = 8;
    resources.maxCombinedClipAndCullDistances = 8;
    resources.maxSamples = 4;
    resources.maxMeshOutputVerticesNV = 256;
    resources.maxMeshOutputPrimitivesNV = 512;
    resources.maxMeshWorkGroupSizeX_NV = 32;
    resources.maxMeshWorkGroupSizeY_NV = 1;
    resources.maxMeshWorkGroupSizeZ_NV = 1;
    resources.maxTaskWorkGroupSizeX_NV = 32;
    resources.maxTaskWorkGroupSizeY_NV = 1;
    resources.maxTaskWorkGroupSizeZ_NV = 1;
    resources.maxMeshViewCountNV = 4;
    resources.maxMeshOutputVerticesEXT = 256;
    resources.maxMeshOutputPrimitivesEXT = 512;
    resources.maxMeshWorkGroupSizeX_EXT = 32;
    resources.maxMeshWorkGroupSizeY_EXT = 1;
    resources.maxMeshWorkGroupSizeZ_EXT = 1;
    resources.maxTaskWorkGroupSizeX_EXT = 32;
    resources.maxTaskWorkGroupSizeY_EXT = 1;
    resources.maxTaskWorkGroupSizeZ_EXT = 1;
    resources.maxMeshViewCountEXT = 4;
    resources.maxDualSourceDrawBuffersEXT = 1;
    resources.limits.nonInductiveForLoops = 1;
    resources.limits.whileLoops = 1;
    resources.limits.doWhileLoops = 1;
    resources.limits.generalUniformIndexing = 1;
    resources.limits.generalAttributeMatrixVectorIndexing = 1;
    resources.limits.generalVaryingIndexing = 1;
    resources.limits.generalSamplerIndexing = 1;
    resources.limits.generalVariableIndexing = 1;
    resources.limits.generalConstantMatrixVectorIndexing = 1;

    return resources;
}
static const TBuiltInResource DefaultTBuiltInResource = InitResources();

bool Hopper::BuildPassThroughShader(std::string& source, VkShaderStageFlagBits stage) {
    glslang::TProgram program;

    EShLanguage glsl_stage = EShLangVertex;
    switch (stage) {
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
            glsl_stage = EShLangTessControl;
            break;
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            glsl_stage = EShLangTessEvaluation;
            break;
        default:
            break;
    }

    // TODO - smart pointer, also in VkTestFramework::GLSLtoSPV
    glslang::TShader* shader = new glslang::TShader(glsl_stage);
    shader->setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_6);
    shader->setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);

    const char* shader_strings = source.c_str();
    const int shader_length = static_cast<int>(source.length());
    shader->setStringsWithLengths(&shader_strings, &shader_length, 1);
    EShMessages messages = EShMsgDefault;
    if (!shader->parse(&DefaultTBuiltInResource, 110, false, messages)) {
        std::cout << shader->getInfoLog() << "\n";
        std::cout << source << "\n";
        return false;
    }

    program.addShader(shader);
    if (!program.link(messages)) {
        std::cout << shader->getInfoLog() << "\n";
        std::cout << source << "\n";
        return false;
    }

    glslang::SpvOptions spv_options;
    std::vector<uint32_t> spirv;
    glslang::GlslangToSpv(*program.getIntermediate(glsl_stage), spirv, &spv_options);

    delete shader;

    return CreateShaderStage(spirv.size() * sizeof(uint32_t), spirv.data(), stage);
}