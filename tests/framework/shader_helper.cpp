/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
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

#include "shader_helper.h"

#include "test_common.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include <glslang/Public/ShaderLang.h>

#ifdef VVL_USE_SLANG
#pragma push_macro("None")
#pragma push_macro("Bool")
#undef None
#undef Bool

#include "slang.h"
#include "slang-com-ptr.h"

#pragma pop_macro("None")
#pragma pop_macro("Bool")
#endif

static void ProcessConfigFile(const VkPhysicalDeviceLimits &device_limits, TBuiltInResource &out_resources) {
    // These are the default resources for TBuiltInResources.
    out_resources.maxLights = 32;
    out_resources.maxClipPlanes = 6;
    out_resources.maxTextureUnits = 32;
    out_resources.maxTextureCoords = 32;
    out_resources.maxVertexAttribs = 64;
    out_resources.maxVertexUniformComponents = 4096;
    out_resources.maxVaryingFloats = 64;
    out_resources.maxVertexTextureImageUnits = 32;
    out_resources.maxCombinedTextureImageUnits = 80;
    out_resources.maxTextureImageUnits = 32;
    out_resources.maxFragmentUniformComponents = 4096;
    out_resources.maxDrawBuffers = 32;
    out_resources.maxVertexUniformVectors = 128;
    out_resources.maxVaryingVectors = 8;
    out_resources.maxFragmentUniformVectors = 16;
    out_resources.maxVertexOutputVectors = 16;
    out_resources.maxFragmentInputVectors = 15;
    out_resources.minProgramTexelOffset = -8;
    out_resources.maxProgramTexelOffset = 7;
    out_resources.maxClipDistances = device_limits.maxClipDistances;
    out_resources.maxComputeWorkGroupCountX = device_limits.maxComputeWorkGroupCount[0];
    out_resources.maxComputeWorkGroupCountY = device_limits.maxComputeWorkGroupCount[1];
    out_resources.maxComputeWorkGroupCountZ = device_limits.maxComputeWorkGroupCount[2];
    out_resources.maxComputeWorkGroupSizeX = device_limits.maxComputeWorkGroupSize[0];
    out_resources.maxComputeWorkGroupSizeY = device_limits.maxComputeWorkGroupSize[1];
    out_resources.maxComputeWorkGroupSizeZ = device_limits.maxComputeWorkGroupSize[2];
    out_resources.maxComputeUniformComponents = 1024;
    out_resources.maxComputeTextureImageUnits = 16;
    out_resources.maxComputeImageUniforms = 8;
    out_resources.maxComputeAtomicCounters = 8;
    out_resources.maxComputeAtomicCounterBuffers = 1;
    out_resources.maxVaryingComponents = 60;
    out_resources.maxVertexOutputComponents = device_limits.maxVertexOutputComponents;
    out_resources.maxGeometryInputComponents = device_limits.maxGeometryInputComponents;
    out_resources.maxGeometryOutputComponents = device_limits.maxGeometryOutputComponents;
    out_resources.maxFragmentInputComponents = device_limits.maxFragmentInputComponents;
    out_resources.maxImageUnits = 8;
    out_resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
    out_resources.maxCombinedShaderOutputResources = 8;
    out_resources.maxImageSamples = 0;
    out_resources.maxVertexImageUniforms = 0;
    out_resources.maxTessControlImageUniforms = 0;
    out_resources.maxTessEvaluationImageUniforms = 0;
    out_resources.maxGeometryImageUniforms = 0;
    out_resources.maxFragmentImageUniforms = 8;
    out_resources.maxCombinedImageUniforms = 8;
    out_resources.maxGeometryTextureImageUnits = 16;
    out_resources.maxGeometryOutputVertices = device_limits.maxGeometryOutputVertices;
    out_resources.maxGeometryTotalOutputComponents = device_limits.maxGeometryTotalOutputComponents;
    out_resources.maxGeometryUniformComponents = 1024;
    out_resources.maxGeometryVaryingComponents = 64;
    out_resources.maxTessControlInputComponents = 128;
    out_resources.maxTessControlOutputComponents = 128;
    out_resources.maxTessControlTextureImageUnits = 16;
    out_resources.maxTessControlUniformComponents = 1024;
    out_resources.maxTessControlTotalOutputComponents = 4096;
    out_resources.maxTessEvaluationInputComponents = 128;
    out_resources.maxTessEvaluationOutputComponents = 128;
    out_resources.maxTessEvaluationTextureImageUnits = 16;
    out_resources.maxTessEvaluationUniformComponents = 1024;
    out_resources.maxTessPatchComponents = 120;
    out_resources.maxPatchVertices = 32;
    out_resources.maxTessGenLevel = 64;
    out_resources.maxViewports = device_limits.maxViewports;
    out_resources.maxVertexAtomicCounters = 0;
    out_resources.maxTessControlAtomicCounters = 0;
    out_resources.maxTessEvaluationAtomicCounters = 0;
    out_resources.maxGeometryAtomicCounters = 0;
    out_resources.maxFragmentAtomicCounters = 8;
    out_resources.maxCombinedAtomicCounters = 8;
    out_resources.maxAtomicCounterBindings = 1;
    out_resources.maxVertexAtomicCounterBuffers = 0;
    out_resources.maxTessControlAtomicCounterBuffers = 0;
    out_resources.maxTessEvaluationAtomicCounterBuffers = 0;
    out_resources.maxGeometryAtomicCounterBuffers = 0;
    out_resources.maxFragmentAtomicCounterBuffers = 1;
    out_resources.maxCombinedAtomicCounterBuffers = 1;
    out_resources.maxAtomicCounterBufferSize = 16384;
    out_resources.maxTransformFeedbackBuffers = 4;
    out_resources.maxTransformFeedbackInterleavedComponents = 64;
    out_resources.maxCullDistances = device_limits.maxCullDistances;
    out_resources.maxCombinedClipAndCullDistances = 8;
    out_resources.maxSamples = 4;
    out_resources.maxMeshOutputVerticesNV = 256;
    out_resources.maxMeshOutputPrimitivesNV = 512;
    out_resources.maxMeshWorkGroupSizeX_NV = 32;
    out_resources.maxMeshWorkGroupSizeY_NV = 1;
    out_resources.maxMeshWorkGroupSizeZ_NV = 1;
    out_resources.maxTaskWorkGroupSizeX_NV = 32;
    out_resources.maxTaskWorkGroupSizeY_NV = 1;
    out_resources.maxTaskWorkGroupSizeZ_NV = 1;
    out_resources.maxMeshViewCountNV = 4;
    out_resources.maxMeshOutputVerticesEXT = 256;
    out_resources.maxMeshOutputPrimitivesEXT = 512;
    out_resources.maxMeshWorkGroupSizeX_EXT = 32;
    out_resources.maxMeshWorkGroupSizeY_EXT = 1;
    out_resources.maxMeshWorkGroupSizeZ_EXT = 1;
    out_resources.maxTaskWorkGroupSizeX_EXT = 32;
    out_resources.maxTaskWorkGroupSizeY_EXT = 1;
    out_resources.maxTaskWorkGroupSizeZ_EXT = 1;
    out_resources.maxMeshViewCountEXT = 4;

    out_resources.limits.nonInductiveForLoops = 1;
    out_resources.limits.whileLoops = 1;
    out_resources.limits.doWhileLoops = 1;
    out_resources.limits.generalUniformIndexing = 1;
    out_resources.limits.generalAttributeMatrixVectorIndexing = 1;
    out_resources.limits.generalVaryingIndexing = 1;
    out_resources.limits.generalSamplerIndexing = 1;
    out_resources.limits.generalVariableIndexing = 1;
    out_resources.limits.generalConstantMatrixVectorIndexing = 1;
}

//
// Convert VK shader type to compiler's
//
static EShLanguage FindLanguage(const VkShaderStageFlagBits shader_type) {
    switch (shader_type) {
        case VK_SHADER_STAGE_VERTEX_BIT:
            return EShLangVertex;

        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
            return EShLangTessControl;

        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            return EShLangTessEvaluation;

        case VK_SHADER_STAGE_GEOMETRY_BIT:
            return EShLangGeometry;

        case VK_SHADER_STAGE_FRAGMENT_BIT:
            return EShLangFragment;

        case VK_SHADER_STAGE_COMPUTE_BIT:
            return EShLangCompute;

        case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
            return EShLangRayGen;

        case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
            return EShLangAnyHit;

        case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
            return EShLangClosestHit;

        case VK_SHADER_STAGE_MISS_BIT_KHR:
            return EShLangMiss;

        case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
            return EShLangIntersect;

        case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
            return EShLangCallable;

        case VK_SHADER_STAGE_TASK_BIT_EXT:
            return EShLangTask;

        case VK_SHADER_STAGE_MESH_BIT_EXT:
            return EShLangMesh;

        default:
            assert(false);
            return EShLangVertex;
    }
}

struct GlslangTargetEnv {
    GlslangTargetEnv(const spv_target_env env) {
        switch (env) {
            case SPV_ENV_UNIVERSAL_1_0:
                language_version = glslang::EShTargetSpv_1_0;
                break;
            case SPV_ENV_UNIVERSAL_1_1:
                language_version = glslang::EShTargetSpv_1_1;
                break;
            case SPV_ENV_UNIVERSAL_1_2:
                language_version = glslang::EShTargetSpv_1_2;
                break;
            case SPV_ENV_UNIVERSAL_1_3:
                language_version = glslang::EShTargetSpv_1_3;
                break;
            case SPV_ENV_UNIVERSAL_1_4:
                language_version = glslang::EShTargetSpv_1_4;
                break;
            case SPV_ENV_UNIVERSAL_1_5:
                language_version = glslang::EShTargetSpv_1_5;
                break;
            case SPV_ENV_UNIVERSAL_1_6:
                language_version = glslang::EShTargetSpv_1_6;
                break;
            case SPV_ENV_VULKAN_1_0:
                client_version = glslang::EShTargetVulkan_1_0;
                break;
            case SPV_ENV_VULKAN_1_1:
                client_version = glslang::EShTargetVulkan_1_1;
                language_version = glslang::EShTargetSpv_1_3;
                break;
            case SPV_ENV_VULKAN_1_2:
                client_version = glslang::EShTargetVulkan_1_2;
                language_version = glslang::EShTargetSpv_1_5;
                break;
            case SPV_ENV_VULKAN_1_3:
                client_version = glslang::EShTargetVulkan_1_3;
                language_version = glslang::EShTargetSpv_1_6;
                break;
            default:
                assert(false && "Invalid SPIR-V environment");
                break;
        }
    }

    operator glslang::EShTargetLanguageVersion() const { return language_version; }

    operator glslang::EShTargetClientVersion() const { return client_version; }

  private:
    glslang::EShTargetLanguageVersion language_version = glslang::EShTargetSpv_1_0;
    glslang::EShTargetClientVersion client_version = glslang::EShTargetVulkan_1_0;
};

//
// Compile a given string containing GLSL into SPV for use by VK
// Return value of false means an error was encountered.
//
bool GLSLtoSPV(const VkPhysicalDeviceLimits &device_limits, const VkShaderStageFlagBits shader_type, const char *p_shader,
               std::vector<uint32_t> &spirv, const spv_target_env spv_env) {
    TBuiltInResource resources;
    ProcessConfigFile(device_limits, resources);

    EShMessages messages = static_cast<EShMessages>(EShMsgDefault | EShMsgSpvRules | EShMsgVulkanRules);
    EShLanguage stage = FindLanguage(shader_type);
    glslang::TShader shader(stage);
    GlslangTargetEnv glslang_env(spv_env);
    shader.setEnvTarget(glslang::EshTargetSpv, glslang_env);
    shader.setEnvClient(glslang::EShClientVulkan, glslang_env);

    const char *shader_strings[1];
    shader_strings[0] = p_shader;
    shader.setStrings(shader_strings, 1);

    if (!shader.parse(&resources, 100, false, messages)) {
        puts(shader.getInfoLog());
        puts(shader.getInfoDebugLog());
        return false;  // something didn't work
    }

    glslang::TProgram program;
    program.addShader(&shader);

    if (!program.link(messages)) {
        puts(shader.getInfoLog());
        puts(shader.getInfoDebugLog());
        return false;
    }

    glslang::SpvOptions spv_options;
    glslang::GlslangToSpv(*program.getIntermediate(stage), spirv, &spv_options);

    return true;
}

//
// Compile a given string containing SPIR-V assembly into SPV for use by VK
// Return value of false means an error was encountered.
//
bool ASMtoSPV(const spv_target_env target_env, const uint32_t options, const char *p_asm, std::vector<uint32_t> &spv) {
    spv_binary binary;
    spv_diagnostic diagnostic = nullptr;
    spv_context context = spvContextCreate(target_env);
    spv_result_t error = spvTextToBinaryWithOptions(context, p_asm, strlen(p_asm), options, &binary, &diagnostic);
    spvContextDestroy(context);
    if (error) {
        spvDiagnosticPrint(diagnostic);
        spvDiagnosticDestroy(diagnostic);
        return false;
    }
    spv.insert(spv.end(), binary->code, binary->code + binary->wordCount);
    spvBinaryDestroy(binary);

    return true;
}

void CheckSlangSupport() {
#ifndef VVL_USE_SLANG
    GTEST_SKIP() << "Slang not supported on this platform";
#endif
}

bool SlangToSPV(const char *slang_shader, const char *entry_point_name, std::vector<uint8_t> &out_bytes) {
#ifndef VVL_USE_SLANG
    (void)slang_shader;
    (void)entry_point_name;
    (void)out_bytes;
    return false;
#else
    // Function adapted from
    // https://github.com/shader-slang/slang/blob/master/examples/hello-world/main.cpp#L114
    // Used https://github.com/shader-slang/slang/issues/6678 to figure out how to use
    // `slang::IModule::loadModuleFromSourceString`

    using namespace Slang;

    // First we need to create slang global session with work with the Slang API.
    ComPtr<slang::IGlobalSession> slang_session;
    {
        SlangGlobalSessionDesc slang_session_desc = {};
        // slang_session_desc.enableGLSL = true; // Could be needed in the future
        slang::createGlobalSession(&slang_session_desc, slang_session.writeRef());
    }

    // Next we create a compilation session to generate SPIRV code from Slang source.
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = slang_session->findProfile("glsl_460");  // todo what spirv profile ?
    targetDesc.flags = 0;
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;

    std::vector<slang::CompilerOptionEntry> options;
    options.push_back(
        {slang::CompilerOptionName::EmitSpirvDirectly, {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}});
    // https://github.com/shader-slang/slang/issues/6248
    options.push_back(
        {slang::CompilerOptionName::VulkanUseEntryPointName, {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}});
    sessionDesc.compilerOptionEntries = options.data();
    sessionDesc.compilerOptionEntryCount = options.size();

    ComPtr<slang::ISession> session;
    SlangResult result = slang_session->createSession(sessionDesc, session.writeRef());
    if (result != 0) {
        ADD_FAILURE() << "Slang failure: slang_session->createSession()";
    }

    // Handle errors
    Slang::ComPtr<ISlangBlob> diagnostics;

    // Compile the source code
    // Once the session has been obtained, we can start loading code into it.
    // Here we use `loadModuleFromSourceString` and not `loadModule`.
    // moduleName and path are just placeholders
    Slang::ComPtr<slang::IModule> slang_module;
    slang_module = session->loadModuleFromSourceString("my_shader", "my_shader.slang", slang_shader, diagnostics.writeRef());
    if (slang_module == NULL) {
        ADD_FAILURE() << "Slang failure: loadModuleFromSourceString()\n" << ((const char *)diagnostics->getBufferPointer());
        return false;
    }

    // Loading the module will compile and check all the shader code in it,
    // including the shader entry points we want to use. Now that the module is loaded
    // we can look up those entry points by name.
    //
    // Note: If you are using this `loadModule` approach to load your shader code it is
    // important to tag your entry point functions with the `[shader("...")]` attribute
    // (e.g., `[shader("compute")] void computeMain(...)`). Without that information there
    // is no unambiguous way for the compiler to know which functions represent entry
    // points when it parses your code via `loadModule()`.
    //
    ComPtr<slang::IEntryPoint> entry_point;
    result = slang_module->findEntryPointByName(entry_point_name, entry_point.writeRef());
    if (result != 0) {
        ADD_FAILURE() << "Slang failure: loadModuleFromSourceString()\nCould not find entry point \"" << entry_point_name << "\"";
        return false;
    }

    // At this point we have a few different Slang API objects that represent
    // pieces of our code: `module`, `vertexEntryPoint`, and `fragmentEntryPoint`.
    //
    // A single Slang module could contain many different entry points (e.g.,
    // four vertex entry points, three fragment entry points, and two compute
    // shaders), and before we try to generate output code for our target API
    // we need to identify which entry points we plan to use together.
    //
    // Modules and entry points are both examples of *component types* in the
    // Slang API. The API also provides a way to build a *composite* out of
    // other pieces, and that is what we are going to do with our module
    // and entry points.
    //
    std::vector<slang::IComponentType *> componentTypes;
    componentTypes.emplace_back(slang_module);
    componentTypes.emplace_back(entry_point);

    // Actually creating the composite component type is a single operation
    // on the Slang session, but the operation could potentially fail if
    // something about the composite was invalid (e.g., you are trying to
    // combine multiple copies of the same module), so we need to deal
    // with the possibility of diagnostic output.
    //
    ComPtr<slang::IComponentType> composedProgram;
    {
        result = session->createCompositeComponentType(componentTypes.data(), (SlangInt)componentTypes.size(),
                                                       composedProgram.writeRef(), diagnostics.writeRef());
        if (result != 0) {
            ADD_FAILURE() << "Slang failure: createCompositeComponentType()\n" << ((const char *)diagnostics->getBufferPointer());
            return false;
        }
    }

    // Now we can call `composedProgram->getEntryPointCode()` to retrieve the
    // compiled SPIRV code that we will use to create a vulkan compute pipeline.
    // This will trigger the final Slang compilation and spirv code generation.
    ComPtr<slang::IBlob> spirvCode;
    {
        result = composedProgram->getEntryPointCode(0, 0, spirvCode.writeRef(), diagnostics.writeRef());
        if (result != 0) {
            ADD_FAILURE() << "Slang failure: createCompositeComponentType()\n" << ((const char *)diagnostics->getBufferPointer());
            return false;
        }
    }

    out_bytes.resize(spirvCode->getBufferSize());
    std::memcpy(out_bytes.data(), spirvCode->getBufferPointer(), spirvCode->getBufferSize());

    return true;
#endif
}

VkPipelineShaderStageCreateInfo const &VkShaderObj::GetStageCreateInfo() const { return m_stage_info; }

VkShaderObj::VkShaderObj(vkt::Device &device, const char *source, VkShaderStageFlagBits stage, const spv_target_env env,
                         SpvSourceType source_type, const VkSpecializationInfo *spec_info, char const *entry_point,
                         const void *pNext)
    : m_device(&device), m_source(source), m_spv_env(env) {
    m_stage_info = vku::InitStructHelper();
    m_stage_info.flags = 0;
    m_stage_info.stage = stage;
    m_stage_info.module = VK_NULL_HANDLE;
    m_stage_info.pName = entry_point;
    m_stage_info.pSpecializationInfo = spec_info;
    if (source_type == SPV_SOURCE_GLSL) {
        InitFromGLSL(pNext);
    } else if (source_type == SPV_SOURCE_ASM) {
        InitFromASM();
    } else if (source_type == SPV_SOURCE_SLANG) {
        InitFromSlang();
    }
}

VkShaderObj::VkShaderObj(VkRenderFramework *framework, const char *source, VkShaderStageFlagBits stage, const spv_target_env env,
                         SpvSourceType source_type, const VkSpecializationInfo *spec_info, char const *entry_point,
                         const void *pNext)
    : VkShaderObj(*framework->DeviceObj(), source, stage, env, source_type, spec_info, entry_point, pNext) {}

bool VkShaderObj::InitFromGLSL(const void *pNext) {
    std::vector<uint32_t> spv;
    GLSLtoSPV(m_device->Physical().limits_, m_stage_info.stage, m_source, spv, m_spv_env);

    VkShaderModuleCreateInfo moduleCreateInfo = vku::InitStructHelper();
    moduleCreateInfo.pNext = pNext;
    moduleCreateInfo.codeSize = spv.size() * sizeof(uint32_t);
    moduleCreateInfo.pCode = spv.data();

    Init(*m_device, moduleCreateInfo);
    m_stage_info.module = handle();
    return VK_NULL_HANDLE != handle();
}

// Because shaders are currently validated at pipeline creation time, there are test cases that might fail shader module
// creation due to supplying an invalid/unknown SPIR-V capability/operation. This is called after VkShaderObj creation when
// tests are found to crash on a CI device
VkResult VkShaderObj::InitFromGLSLTry(const vkt::Device *custom_device) {
    std::vector<uint32_t> spv;
    // 99% of tests just use the framework's VkDevice, but this allows for tests to use custom device object
    // Can't set at contructor time since all reference members need to be initialized then.
    VkPhysicalDeviceLimits limits = (custom_device) ? custom_device->Physical().limits_ : m_device->Physical().limits_;
    GLSLtoSPV(limits, m_stage_info.stage, m_source, spv, m_spv_env);

    VkShaderModuleCreateInfo moduleCreateInfo = vku::InitStructHelper();
    moduleCreateInfo.codeSize = spv.size() * sizeof(uint32_t);
    moduleCreateInfo.pCode = spv.data();

    const auto result = InitTry(((custom_device) ? *custom_device : *m_device), moduleCreateInfo);
    m_stage_info.module = handle();
    return result;
}

bool VkShaderObj::InitFromASM() {
    std::vector<uint32_t> spv;
    ASMtoSPV(m_spv_env, 0, m_source, spv);

    VkShaderModuleCreateInfo moduleCreateInfo = vku::InitStructHelper();
    moduleCreateInfo.codeSize = spv.size() * sizeof(uint32_t);
    moduleCreateInfo.pCode = spv.data();

    Init(*m_device, moduleCreateInfo);
    m_stage_info.module = handle();
    return VK_NULL_HANDLE != handle();
}

VkResult VkShaderObj::InitFromASMTry() {
    std::vector<uint32_t> spv;
    ASMtoSPV(m_spv_env, 0, m_source, spv);

    VkShaderModuleCreateInfo moduleCreateInfo = vku::InitStructHelper();
    moduleCreateInfo.codeSize = spv.size() * sizeof(uint32_t);
    moduleCreateInfo.pCode = spv.data();

    const auto result = InitTry(*m_device, moduleCreateInfo);
    m_stage_info.module = handle();
    return result;
}

bool VkShaderObj::InitFromSlang() {
#ifndef VVL_USE_SLANG
    return false;
#else
    std::vector<uint8_t> bytes;
    if (!SlangToSPV(m_source, m_stage_info.pName, bytes)) {
        return false;
    }
    VkShaderModuleCreateInfo module_ci = vku::InitStructHelper();
    module_ci.codeSize = bytes.size();
    module_ci.pCode = (uint32_t *)bytes.data();

    const auto result = InitTry(*m_device, module_ci);
    m_stage_info.module = handle();
    return result == VK_SUCCESS;
#endif
}

// static
VkShaderObj VkShaderObj::CreateFromGLSL(VkRenderFramework *framework, const char *source, VkShaderStageFlagBits stage,
                                        const spv_target_env spv_env, const VkSpecializationInfo *spec_info,
                                        const char *entry_point) {
    auto shader = VkShaderObj(framework, source, stage, spv_env, SPV_SOURCE_GLSL_TRY, spec_info, entry_point);
    if (VK_SUCCESS == shader.InitFromGLSLTry()) {
        return shader;
    }
    return {};
}

// static
VkShaderObj VkShaderObj::CreateFromASM(VkRenderFramework *framework, const char *source, VkShaderStageFlagBits stage,
                                       const spv_target_env spv_env, const VkSpecializationInfo *spec_info,
                                       const char *entry_point) {
    auto shader = VkShaderObj(framework, source, stage, spv_env, SPV_SOURCE_ASM_TRY, spec_info, entry_point);
    if (VK_SUCCESS == shader.InitFromASMTry()) {
        return shader;
    }
    return {};
}
