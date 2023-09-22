/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
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

#include "test_framework.h"
#include "render.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include "glslang/SPIRV/SPVRemapper.h"
#include <filesystem>
#include <cmath>
#include <cstdarg>

// Command-line options
enum TOptions {
    EOptionNone = 0x000,
    EOptionIntermediate = 0x001,
    EOptionSuppressInfolog = 0x002,
    EOptionMemoryLeakMode = 0x004,
    EOptionRelaxedErrors = 0x008,
    EOptionGiveWarnings = 0x010,
    EOptionLinkProgram = 0x020,
    EOptionMultiThreaded = 0x040,
    EOptionDumpConfig = 0x080,
    EOptionDumpReflection = 0x100,
    EOptionSuppressWarnings = 0x200,
    EOptionDumpVersions = 0x400,
    EOptionSpv = 0x800,
    EOptionDefaultDesktop = 0x1000,
};

struct SwapchainBuffers {
    VkImage image;
    VkCommandBuffer cmd;
    VkImageView view;
};

#ifndef _WIN32

#include <errno.h>

int fopen_s(FILE **pFile, const char *filename, const char *mode) {
    if (!pFile || !filename || !mode) {
        return EINVAL;
    }

    FILE *f = fopen(filename, mode);
    if (!f) {
        if (errno != 0) {
            return errno;
        } else {
            return ENOENT;
        }
    }
    *pFile = f;

    return 0;
}

#endif

#if !defined(VK_USE_PLATFORM_ANDROID_KHR)
// NOTE: Android doesn't use environment variables like desktop does!
//
// Certain VK_* environment variables accept lists.
// Return a vector of std::string containing each member in the list.
//
// EX input:
//  export VK_DRIVER_FILES=/intel.json:/amd.json
//  set VK_DRIVER_FILES=\nvidia.json;\mesa.json
static std::vector<std::string> GetVkEnvironmentVariable(const char *env_var) {
    const std::string str = GetEnvironment(env_var);
    if (str.empty()) {
        return {};
    }

    // Loader uses standard OS path separators per platform
    constexpr char delimiter =
#ifdef _WIN32
        ';';
#else
        ':';
#endif

    std::vector<std::string> items;
    std::string::size_type start = 0;

    std::string::size_type pos = str.find_first_of(delimiter, start);
    std::string::size_type length = pos;
    while (pos != std::string::npos) {
        // emplace uses std::substr which takes length from start
        items.emplace_back(str, start, length);

        start = pos + 1;

        pos = str.find_first_of(delimiter, start);

        length = pos - start;
    }
    items.emplace_back(str, start);

    return items;
}

static void CheckEnvironmentVariables() {
    for (const char *env_var : {"VK_DRIVER_FILES", "VK_ICD_FILENAMES"}) {
        const std::vector<std::string> driver_files = GetVkEnvironmentVariable(env_var);
        for (const std::string &driver_file : driver_files) {
            const std::filesystem::path icd_file(driver_file);

            // TODO: Error check relative paths (platform dependent)
            if (icd_file.is_relative()) {
                continue;
            }

            std::string user_provided;
            user_provided += "\n\n";
            user_provided += env_var;
            user_provided += " = ";
            user_provided += driver_file;

            if (!std::filesystem::exists(icd_file)) {
                std::cerr << "Invalid " << env_var << "! File doesn't exist!" << user_provided << std::endl;
                std::exit(EXIT_FAILURE);
            }

            if (icd_file.extension() != ".json") {
                std::cerr << "Invalid " << env_var << "! " << env_var << " must be a json file!\n" << user_provided << std::endl;
                std::exit(EXIT_FAILURE);
            }
        }
    }

    const std::vector<std::string> vk_layer_paths = GetVkEnvironmentVariable("VK_LAYER_PATH");
    bool found_json = false;
    for (const std::string &layer_path : vk_layer_paths) {
        const std::filesystem::path layer_dir(layer_path);

        // TODO: Error check relative paths (platform dependent)
        if (layer_dir.is_relative()) {
            continue;
        }
        const std::string user_provided = "\n\nVK_LAYER_PATH = " + layer_path;

        if (!std::filesystem::exists(layer_dir)) {
            std::cerr << "Invalid VK_LAYER_PATH! Directory " << layer_dir << " doesn't exist!" << user_provided << std::endl;
            std::exit(EXIT_FAILURE);
        }

        if (!std::filesystem::is_directory(layer_dir)) {
            std::cerr << "Invalid VK_LAYER_PATH! " << layer_dir << " must be a directory!" << user_provided << std::endl;
            std::exit(EXIT_FAILURE);
        }

        for (auto const &dir_entry : std::filesystem::directory_iterator{layer_dir}) {
            if (dir_entry.path().filename() == "VkLayer_khronos_validation.json") {
                found_json = true;
                break;
            }
        }
    }

    if (!found_json) {
        std::cerr << "Invalid VK_LAYER_PATH! VK_LAYER_PATH directory must contain VkLayer_khronos_validation.json!"
                  << GetEnvironment("VK_LAYER_PATH") << std::endl;
        std::exit(EXIT_FAILURE);
    }
}
#endif

// Set up environment for GLSL compiler
// Must be done once per process
void TestEnvironment::SetUp() {
#if !defined(VK_USE_PLATFORM_ANDROID_KHR)
    // Helps ensure common developer environment variables are set correctly
    CheckEnvironmentVariables();
#endif

    // Initialize GLSL to SPV compiler utility
    glslang::InitializeProcess();

    vk::InitCore("vulkan");
}

void TestEnvironment::TearDown() { glslang::FinalizeProcess(); }

void VkTestFramework::InitArgs(int *argc, char *argv[]) {
    for (int i = 1; i < *argc; ++i) {
        const std::string_view current_argument = argv[i];
        if (current_argument == "--strip-SPV") {
            m_strip_spv = true;
        } else if (current_argument == "--canonicalize-SPV") {
            m_canonicalize_spv = true;
        } else if (current_argument == "--print-vu") {
            m_print_vu = true;
        } else if (current_argument == "--device-index" && ((i + 1) < *argc)) {
            m_phys_device_index = std::atoi(argv[++i]);
        } else if ((current_argument == "--help") || (current_argument == "-h")) {
            printf("\nOther options:\n");
            printf(
                "\t--print-vu\n"
                "\t\tPrints all VUs - help see what new VU will look like.\n");
            printf(
                "\t--strip-SPV\n"
                "\t\tStrip SPIR-V debug information (line numbers, names, etc).\n");
            printf(
                "\t--canonicalize-SPV\n"
                "\t\tRemap SPIR-V ids before submission to aid compression.\n");
            printf(
                "\t--device-index <physical device index>\n"
                "\t\tIndex into VkPhysicalDevice array returned from vkEnumeratePhysicalDevices.\n"
                "\t\tThe default behavior is to automatically choose \"the most reasonable device.\"\n"
                "\t\tAn invalid index (i.e., outside the range [0, *pPhysicalDeviceCount)) will result in the default behavior\n");
            exit(0);
        } else {
            printf("\nUnrecognized option: %s\n", argv[i]);
            printf("\nUse --help or -h for option list.\n");
            exit(0);
        }
    }
}

void VkTestFramework::Finish() {}

//
// These are the default resources for TBuiltInResources, used for both
//  - parsing this string for the case where the user didn't supply one
//  - dumping out a template for user construction of a config file
//
static const char *DefaultConfig =
    "MaxLights 32\n"
    "MaxClipPlanes 6\n"
    "MaxTextureUnits 32\n"
    "MaxTextureCoords 32\n"
    "MaxVertexAttribs 64\n"
    "MaxVertexUniformComponents 4096\n"
    "MaxVaryingFloats 64\n"
    "MaxVertexTextureImageUnits 32\n"
    "MaxCombinedTextureImageUnits 80\n"
    "MaxTextureImageUnits 32\n"
    "MaxFragmentUniformComponents 4096\n"
    "MaxDrawBuffers 32\n"
    "MaxVertexUniformVectors 128\n"
    "MaxVaryingVectors 8\n"
    "MaxFragmentUniformVectors 16\n"
    "MaxVertexOutputVectors 16\n"
    "MaxFragmentInputVectors 15\n"
    "MinProgramTexelOffset -8\n"
    "MaxProgramTexelOffset 7\n"
    "MaxClipDistances 8\n"
    "MaxComputeWorkGroupCountX 65535\n"
    "MaxComputeWorkGroupCountY 65535\n"
    "MaxComputeWorkGroupCountZ 65535\n"
    "MaxComputeWorkGroupSizeX 1024\n"
    "MaxComputeWorkGroupSizeY 1024\n"
    "MaxComputeWorkGroupSizeZ 64\n"
    "MaxComputeUniformComponents 1024\n"
    "MaxComputeTextureImageUnits 16\n"
    "MaxComputeImageUniforms 8\n"
    "MaxComputeAtomicCounters 8\n"
    "MaxComputeAtomicCounterBuffers 1\n"
    "MaxVaryingComponents 60\n"
    "MaxVertexOutputComponents 64\n"
    "MaxGeometryInputComponents 64\n"
    "MaxGeometryOutputComponents 128\n"
    "MaxFragmentInputComponents 128\n"
    "MaxImageUnits 8\n"
    "MaxCombinedImageUnitsAndFragmentOutputs 8\n"
    "MaxCombinedShaderOutputResources 8\n"
    "MaxImageSamples 0\n"
    "MaxVertexImageUniforms 0\n"
    "MaxTessControlImageUniforms 0\n"
    "MaxTessEvaluationImageUniforms 0\n"
    "MaxGeometryImageUniforms 0\n"
    "MaxFragmentImageUniforms 8\n"
    "MaxCombinedImageUniforms 8\n"
    "MaxGeometryTextureImageUnits 16\n"
    "MaxGeometryOutputVertices 256\n"
    "MaxGeometryTotalOutputComponents 1024\n"
    "MaxGeometryUniformComponents 1024\n"
    "MaxGeometryVaryingComponents 64\n"
    "MaxTessControlInputComponents 128\n"
    "MaxTessControlOutputComponents 128\n"
    "MaxTessControlTextureImageUnits 16\n"
    "MaxTessControlUniformComponents 1024\n"
    "MaxTessControlTotalOutputComponents 4096\n"
    "MaxTessEvaluationInputComponents 128\n"
    "MaxTessEvaluationOutputComponents 128\n"
    "MaxTessEvaluationTextureImageUnits 16\n"
    "MaxTessEvaluationUniformComponents 1024\n"
    "MaxTessPatchComponents 120\n"
    "MaxPatchVertices 32\n"
    "MaxTessGenLevel 64\n"
    "MaxViewports 16\n"
    "MaxVertexAtomicCounters 0\n"
    "MaxTessControlAtomicCounters 0\n"
    "MaxTessEvaluationAtomicCounters 0\n"
    "MaxGeometryAtomicCounters 0\n"
    "MaxFragmentAtomicCounters 8\n"
    "MaxCombinedAtomicCounters 8\n"
    "MaxAtomicCounterBindings 1\n"
    "MaxVertexAtomicCounterBuffers 0\n"
    "MaxTessControlAtomicCounterBuffers 0\n"
    "MaxTessEvaluationAtomicCounterBuffers 0\n"
    "MaxGeometryAtomicCounterBuffers 0\n"
    "MaxFragmentAtomicCounterBuffers 1\n"
    "MaxCombinedAtomicCounterBuffers 1\n"
    "MaxAtomicCounterBufferSize 16384\n"
    "MaxTransformFeedbackBuffers 4\n"
    "MaxTransformFeedbackInterleavedComponents 64\n"
    "MaxCullDistances 8\n"
    "MaxCombinedClipAndCullDistances 8\n"
    "MaxSamples 4\n"
    "MaxMeshOutputVerticesNV 256\n"
    "MaxMeshOutputPrimitivesNV 512\n"
    "MaxMeshWorkGroupSizeX_NV 32\n"
    "MaxMeshWorkGroupSizeY_NV 1\n"
    "MaxMeshWorkGroupSizeZ_NV 1\n"
    "MaxTaskWorkGroupSizeX_NV 32\n"
    "MaxTaskWorkGroupSizeY_NV 1\n"
    "MaxTaskWorkGroupSizeZ_NV 1\n"
    "MaxMeshViewCountNV 4\n"
    "MaxMeshOutputVerticesEXT 256\n"
    "MaxMeshOutputPrimitivesEXT 512\n"
    "MaxMeshWorkGroupSizeX_EXT 32\n"
    "MaxMeshWorkGroupSizeY_EXT 1\n"
    "MaxMeshWorkGroupSizeZ_EXT 1\n"
    "MaxTaskWorkGroupSizeX_EXT 32\n"
    "MaxTaskWorkGroupSizeY_EXT 1\n"
    "MaxTaskWorkGroupSizeZ_EXT 1\n"
    "MaxMeshViewCountEXT 4\n"

    "nonInductiveForLoops 1\n"
    "whileLoops 1\n"
    "doWhileLoops 1\n"
    "generalUniformIndexing 1\n"
    "generalAttributeMatrixVectorIndexing 1\n"
    "generalVaryingIndexing 1\n"
    "generalSamplerIndexing 1\n"
    "generalVariableIndexing 1\n"
    "generalConstantMatrixVectorIndexing 1\n";

//
// *.conf => this is a config file that can set limits/resources
//
bool VkTestFramework::SetConfigFile(const std::string &name) {
    if (name.size() < 5) return false;

    if (name.compare(name.size() - 5, 5, ".conf") == 0) {
        ConfigFile = name;
        return true;
    }

    return false;
}

//
// Parse either a .conf file provided by the user or the default string above.
//
void VkTestFramework::ProcessConfigFile(VkPhysicalDeviceLimits const *const device_limits) {
    char **configStrings = 0;
    char *config = 0;
    bool config_file_specified = false;
    if (ConfigFile.size() > 0) {
        configStrings = ReadFileData(ConfigFile.c_str());
        if (configStrings) {
            config = *configStrings;
            config_file_specified = true;
        } else {
            printf("Error opening configuration file; will instead use the default configuration\n");
        }
    }

    if (config == 0) {
        config = (char *)alloca(strlen(DefaultConfig) + 1);
        strcpy(config, DefaultConfig);
    }

    const char *delims = " \t\n\r";
    const char *token = strtok(config, delims);
    while (token) {
        const char *valueStr = strtok(0, delims);
        if (valueStr == 0 || !(valueStr[0] == '-' || (valueStr[0] >= '0' && valueStr[0] <= '9'))) {
            printf("Error: '%s' bad .conf file.  Each name must be followed by one number.\n", valueStr ? valueStr : "");
            return;
        }
        int value = atoi(valueStr);

        if (strcmp(token, "MaxLights") == 0)
            Resources.maxLights = value;
        else if (strcmp(token, "MaxClipPlanes") == 0)
            Resources.maxClipPlanes = value;
        else if (strcmp(token, "MaxTextureUnits") == 0)
            Resources.maxTextureUnits = value;
        else if (strcmp(token, "MaxTextureCoords") == 0)
            Resources.maxTextureCoords = value;
        else if (strcmp(token, "MaxVertexAttribs") == 0)
            Resources.maxVertexAttribs = value;
        else if (strcmp(token, "MaxVertexUniformComponents") == 0)
            Resources.maxVertexUniformComponents = value;
        else if (strcmp(token, "MaxVaryingFloats") == 0)
            Resources.maxVaryingFloats = value;
        else if (strcmp(token, "MaxVertexTextureImageUnits") == 0)
            Resources.maxVertexTextureImageUnits = value;
        else if (strcmp(token, "MaxCombinedTextureImageUnits") == 0)
            Resources.maxCombinedTextureImageUnits = value;
        else if (strcmp(token, "MaxTextureImageUnits") == 0)
            Resources.maxTextureImageUnits = value;
        else if (strcmp(token, "MaxFragmentUniformComponents") == 0)
            Resources.maxFragmentUniformComponents = value;
        else if (strcmp(token, "MaxDrawBuffers") == 0)
            Resources.maxDrawBuffers = value;
        else if (strcmp(token, "MaxVertexUniformVectors") == 0)
            Resources.maxVertexUniformVectors = value;
        else if (strcmp(token, "MaxVaryingVectors") == 0)
            Resources.maxVaryingVectors = value;
        else if (strcmp(token, "MaxFragmentUniformVectors") == 0)
            Resources.maxFragmentUniformVectors = value;
        else if (strcmp(token, "MaxVertexOutputVectors") == 0)
            Resources.maxVertexOutputVectors = value;
        else if (strcmp(token, "MaxFragmentInputVectors") == 0)
            Resources.maxFragmentInputVectors = value;
        else if (strcmp(token, "MinProgramTexelOffset") == 0)
            Resources.minProgramTexelOffset = value;
        else if (strcmp(token, "MaxProgramTexelOffset") == 0)
            Resources.maxProgramTexelOffset = value;
        else if (strcmp(token, "MaxClipDistances") == 0)
            Resources.maxClipDistances = (config_file_specified ? value : device_limits->maxClipDistances);
        else if (strcmp(token, "MaxComputeWorkGroupCountX") == 0)
            Resources.maxComputeWorkGroupCountX = (config_file_specified ? value : device_limits->maxComputeWorkGroupCount[0]);
        else if (strcmp(token, "MaxComputeWorkGroupCountY") == 0)
            Resources.maxComputeWorkGroupCountY = (config_file_specified ? value : device_limits->maxComputeWorkGroupCount[1]);
        else if (strcmp(token, "MaxComputeWorkGroupCountZ") == 0)
            Resources.maxComputeWorkGroupCountZ = (config_file_specified ? value : device_limits->maxComputeWorkGroupCount[2]);
        else if (strcmp(token, "MaxComputeWorkGroupSizeX") == 0)
            Resources.maxComputeWorkGroupSizeX = (config_file_specified ? value : device_limits->maxComputeWorkGroupSize[0]);
        else if (strcmp(token, "MaxComputeWorkGroupSizeY") == 0)
            Resources.maxComputeWorkGroupSizeY = (config_file_specified ? value : device_limits->maxComputeWorkGroupSize[1]);
        else if (strcmp(token, "MaxComputeWorkGroupSizeZ") == 0)
            Resources.maxComputeWorkGroupSizeZ = (config_file_specified ? value : device_limits->maxComputeWorkGroupSize[2]);
        else if (strcmp(token, "MaxComputeUniformComponents") == 0)
            Resources.maxComputeUniformComponents = value;
        else if (strcmp(token, "MaxComputeTextureImageUnits") == 0)
            Resources.maxComputeTextureImageUnits = value;
        else if (strcmp(token, "MaxComputeImageUniforms") == 0)
            Resources.maxComputeImageUniforms = value;
        else if (strcmp(token, "MaxComputeAtomicCounters") == 0)
            Resources.maxComputeAtomicCounters = value;
        else if (strcmp(token, "MaxComputeAtomicCounterBuffers") == 0)
            Resources.maxComputeAtomicCounterBuffers = value;
        else if (strcmp(token, "MaxVaryingComponents") == 0)
            Resources.maxVaryingComponents = value;
        else if (strcmp(token, "MaxVertexOutputComponents") == 0)
            Resources.maxVertexOutputComponents = (config_file_specified ? value : device_limits->maxVertexOutputComponents);
        else if (strcmp(token, "MaxGeometryInputComponents") == 0)
            Resources.maxGeometryInputComponents = (config_file_specified ? value : device_limits->maxGeometryInputComponents);
        else if (strcmp(token, "MaxGeometryOutputComponents") == 0)
            Resources.maxGeometryOutputComponents = (config_file_specified ? value : device_limits->maxGeometryOutputComponents);
        else if (strcmp(token, "MaxFragmentInputComponents") == 0)
            Resources.maxFragmentInputComponents = (config_file_specified ? value : device_limits->maxFragmentInputComponents);
        else if (strcmp(token, "MaxImageUnits") == 0)
            Resources.maxImageUnits = value;
        else if (strcmp(token, "MaxCombinedImageUnitsAndFragmentOutputs") == 0)
            Resources.maxCombinedImageUnitsAndFragmentOutputs = value;
        else if (strcmp(token, "MaxCombinedShaderOutputResources") == 0)
            Resources.maxCombinedShaderOutputResources = value;
        else if (strcmp(token, "MaxImageSamples") == 0)
            Resources.maxImageSamples = value;
        else if (strcmp(token, "MaxVertexImageUniforms") == 0)
            Resources.maxVertexImageUniforms = value;
        else if (strcmp(token, "MaxTessControlImageUniforms") == 0)
            Resources.maxTessControlImageUniforms = value;
        else if (strcmp(token, "MaxTessEvaluationImageUniforms") == 0)
            Resources.maxTessEvaluationImageUniforms = value;
        else if (strcmp(token, "MaxGeometryImageUniforms") == 0)
            Resources.maxGeometryImageUniforms = value;
        else if (strcmp(token, "MaxFragmentImageUniforms") == 0)
            Resources.maxFragmentImageUniforms = value;
        else if (strcmp(token, "MaxCombinedImageUniforms") == 0)
            Resources.maxCombinedImageUniforms = value;
        else if (strcmp(token, "MaxGeometryTextureImageUnits") == 0)
            Resources.maxGeometryTextureImageUnits = value;
        else if (strcmp(token, "MaxGeometryOutputVertices") == 0)
            Resources.maxGeometryOutputVertices = (config_file_specified ? value : device_limits->maxGeometryOutputVertices);
        else if (strcmp(token, "MaxGeometryTotalOutputComponents") == 0)
            Resources.maxGeometryTotalOutputComponents =
                (config_file_specified ? value : device_limits->maxGeometryTotalOutputComponents);
        else if (strcmp(token, "MaxGeometryUniformComponents") == 0)
            Resources.maxGeometryUniformComponents = value;
        else if (strcmp(token, "MaxGeometryVaryingComponents") == 0)
            Resources.maxGeometryVaryingComponents = value;
        else if (strcmp(token, "MaxTessControlInputComponents") == 0)
            Resources.maxTessControlInputComponents = value;
        else if (strcmp(token, "MaxTessControlOutputComponents") == 0)
            Resources.maxTessControlOutputComponents = value;
        else if (strcmp(token, "MaxTessControlTextureImageUnits") == 0)
            Resources.maxTessControlTextureImageUnits = value;
        else if (strcmp(token, "MaxTessControlUniformComponents") == 0)
            Resources.maxTessControlUniformComponents = value;
        else if (strcmp(token, "MaxTessControlTotalOutputComponents") == 0)
            Resources.maxTessControlTotalOutputComponents = value;
        else if (strcmp(token, "MaxTessEvaluationInputComponents") == 0)
            Resources.maxTessEvaluationInputComponents = value;
        else if (strcmp(token, "MaxTessEvaluationOutputComponents") == 0)
            Resources.maxTessEvaluationOutputComponents = value;
        else if (strcmp(token, "MaxTessEvaluationTextureImageUnits") == 0)
            Resources.maxTessEvaluationTextureImageUnits = value;
        else if (strcmp(token, "MaxTessEvaluationUniformComponents") == 0)
            Resources.maxTessEvaluationUniformComponents = value;
        else if (strcmp(token, "MaxTessPatchComponents") == 0)
            Resources.maxTessPatchComponents = value;
        else if (strcmp(token, "MaxPatchVertices") == 0)
            Resources.maxPatchVertices = value;
        else if (strcmp(token, "MaxTessGenLevel") == 0)
            Resources.maxTessGenLevel = value;
        else if (strcmp(token, "MaxViewports") == 0)
            Resources.maxViewports = (config_file_specified ? value : device_limits->maxViewports);
        else if (strcmp(token, "MaxVertexAtomicCounters") == 0)
            Resources.maxVertexAtomicCounters = value;
        else if (strcmp(token, "MaxTessControlAtomicCounters") == 0)
            Resources.maxTessControlAtomicCounters = value;
        else if (strcmp(token, "MaxTessEvaluationAtomicCounters") == 0)
            Resources.maxTessEvaluationAtomicCounters = value;
        else if (strcmp(token, "MaxGeometryAtomicCounters") == 0)
            Resources.maxGeometryAtomicCounters = value;
        else if (strcmp(token, "MaxFragmentAtomicCounters") == 0)
            Resources.maxFragmentAtomicCounters = value;
        else if (strcmp(token, "MaxCombinedAtomicCounters") == 0)
            Resources.maxCombinedAtomicCounters = value;
        else if (strcmp(token, "MaxAtomicCounterBindings") == 0)
            Resources.maxAtomicCounterBindings = value;
        else if (strcmp(token, "MaxVertexAtomicCounterBuffers") == 0)
            Resources.maxVertexAtomicCounterBuffers = value;
        else if (strcmp(token, "MaxTessControlAtomicCounterBuffers") == 0)
            Resources.maxTessControlAtomicCounterBuffers = value;
        else if (strcmp(token, "MaxTessEvaluationAtomicCounterBuffers") == 0)
            Resources.maxTessEvaluationAtomicCounterBuffers = value;
        else if (strcmp(token, "MaxGeometryAtomicCounterBuffers") == 0)
            Resources.maxGeometryAtomicCounterBuffers = value;
        else if (strcmp(token, "MaxFragmentAtomicCounterBuffers") == 0)
            Resources.maxFragmentAtomicCounterBuffers = value;
        else if (strcmp(token, "MaxCombinedAtomicCounterBuffers") == 0)
            Resources.maxCombinedAtomicCounterBuffers = value;
        else if (strcmp(token, "MaxAtomicCounterBufferSize") == 0)
            Resources.maxAtomicCounterBufferSize = value;
        else if (strcmp(token, "MaxTransformFeedbackBuffers") == 0)
            Resources.maxTransformFeedbackBuffers = value;
        else if (strcmp(token, "MaxTransformFeedbackInterleavedComponents") == 0)
            Resources.maxTransformFeedbackInterleavedComponents = value;
        else if (strcmp(token, "MaxCullDistances") == 0)
            Resources.maxCullDistances = (config_file_specified ? value : device_limits->maxCullDistances);
        else if (strcmp(token, "MaxCombinedClipAndCullDistances") == 0)
            Resources.maxCombinedClipAndCullDistances = value;
        else if (strcmp(token, "MaxSamples") == 0)
            Resources.maxSamples = value;
        else if (strcmp(token, "MaxMeshOutputVerticesNV") == 0)
            Resources.maxMeshOutputVerticesNV = value;
        else if (strcmp(token, "MaxMeshOutputPrimitivesNV") == 0)
            Resources.maxMeshOutputPrimitivesNV = value;
        else if (strcmp(token, "MaxMeshWorkGroupSizeX_NV") == 0)
            Resources.maxMeshWorkGroupSizeX_NV = value;
        else if (strcmp(token, "MaxMeshWorkGroupSizeY_NV") == 0)
            Resources.maxMeshWorkGroupSizeY_NV = value;
        else if (strcmp(token, "MaxMeshWorkGroupSizeZ_NV") == 0)
            Resources.maxMeshWorkGroupSizeZ_NV = value;
        else if (strcmp(token, "MaxTaskWorkGroupSizeX_NV") == 0)
            Resources.maxTaskWorkGroupSizeX_NV = value;
        else if (strcmp(token, "MaxTaskWorkGroupSizeY_NV") == 0)
            Resources.maxTaskWorkGroupSizeY_NV = value;
        else if (strcmp(token, "MaxTaskWorkGroupSizeZ_NV") == 0)
            Resources.maxTaskWorkGroupSizeZ_NV = value;
        else if (strcmp(token, "MaxMeshViewCountNV") == 0)
            Resources.maxMeshViewCountNV = value;
        else if (strcmp(token, "MaxMeshOutputVerticesEXT") == 0)
            Resources.maxMeshOutputVerticesEXT = value;
        else if (strcmp(token, "MaxMeshOutputPrimitivesEXT") == 0)
            Resources.maxMeshOutputPrimitivesEXT = value;
        else if (strcmp(token, "MaxMeshWorkGroupSizeX_EXT") == 0)
            Resources.maxMeshWorkGroupSizeX_EXT = value;
        else if (strcmp(token, "MaxMeshWorkGroupSizeY_EXT") == 0)
            Resources.maxMeshWorkGroupSizeY_EXT = value;
        else if (strcmp(token, "MaxMeshWorkGroupSizeZ_EXT") == 0)
            Resources.maxMeshWorkGroupSizeZ_EXT = value;
        else if (strcmp(token, "MaxTaskWorkGroupSizeX_EXT") == 0)
            Resources.maxTaskWorkGroupSizeX_EXT = value;
        else if (strcmp(token, "MaxTaskWorkGroupSizeY_EXT") == 0)
            Resources.maxTaskWorkGroupSizeY_EXT = value;
        else if (strcmp(token, "MaxTaskWorkGroupSizeZ_EXT") == 0)
            Resources.maxTaskWorkGroupSizeZ_EXT = value;
        else if (strcmp(token, "MaxMeshViewCountEXT") == 0)
            Resources.maxMeshViewCountEXT = value;

        else if (strcmp(token, "nonInductiveForLoops") == 0)
            Resources.limits.nonInductiveForLoops = (value != 0);
        else if (strcmp(token, "whileLoops") == 0)
            Resources.limits.whileLoops = (value != 0);
        else if (strcmp(token, "doWhileLoops") == 0)
            Resources.limits.doWhileLoops = (value != 0);
        else if (strcmp(token, "generalUniformIndexing") == 0)
            Resources.limits.generalUniformIndexing = (value != 0);
        else if (strcmp(token, "generalAttributeMatrixVectorIndexing") == 0)
            Resources.limits.generalAttributeMatrixVectorIndexing = (value != 0);
        else if (strcmp(token, "generalVaryingIndexing") == 0)
            Resources.limits.generalVaryingIndexing = (value != 0);
        else if (strcmp(token, "generalSamplerIndexing") == 0)
            Resources.limits.generalSamplerIndexing = (value != 0);
        else if (strcmp(token, "generalVariableIndexing") == 0)
            Resources.limits.generalVariableIndexing = (value != 0);
        else if (strcmp(token, "generalConstantMatrixVectorIndexing") == 0)
            Resources.limits.generalConstantMatrixVectorIndexing = (value != 0);
        else
            printf("Warning: unrecognized limit (%s) in configuration file.\n", token);

        token = strtok(0, delims);
    }
    if (configStrings) FreeFileData(configStrings);
}

void VkTestFramework::SetMessageOptions(EShMessages &messages) {
    if (m_compile_options & EOptionRelaxedErrors) messages = (EShMessages)(messages | EShMsgRelaxedErrors);
    if (m_compile_options & EOptionIntermediate) messages = (EShMessages)(messages | EShMsgAST);
    if (m_compile_options & EOptionSuppressWarnings) messages = (EShMessages)(messages | EShMsgSuppressWarnings);
}

//
//   Malloc a string of sufficient size and read a string into it.
//
char **VkTestFramework::ReadFileData(const char *fileName) {
    FILE *in;
#if defined(_WIN32) && defined(__GNUC__)
    in = fopen(fileName, "r");
    int errorCode = in ? 0 : 1;
#else
    int errorCode = fopen_s(&in, fileName, "r");
#endif

    size_t count = 0;
    const int maxSourceStrings = 5;
    char **return_data = (char **)malloc(sizeof(char *) * (maxSourceStrings + 1));

    if (errorCode) {
        printf("Error: unable to open input file: %s\n", fileName);
        return nullptr;
    }

    while (fgetc(in) != EOF) count++;

    if (fseek(in, 0, SEEK_SET) != 0) {
        printf("Error fseek to start of file\n");
        return nullptr;
    }

    char *fdata = (char *)malloc(count + 2);
    if (fdata == nullptr) {
        printf("Error allocating memory\n");
        return nullptr;
    }
    if (fread(fdata, 1, count, in) != count) {
        printf("Error reading input file: %s\n", fileName);
        return nullptr;
    }
    fdata[count] = '\0';
    fclose(in);
    if (count == 0) {
        return_data[0] = (char *)malloc(count + 2);
        return_data[0][0] = '\0';
        m_num_shader_strings = 0;
        return return_data;
    } else
        m_num_shader_strings = 1;

    size_t len = (int)(ceil)((float)count / (float)m_num_shader_strings);
    size_t ptr_len = 0, i = 0;
    while (count > 0) {
        return_data[i] = (char *)malloc(len + 2);
        memcpy(return_data[i], fdata + ptr_len, len);
        return_data[i][len] = '\0';
        count -= (len);
        ptr_len += (len);
        if (count < len) {
            if (count == 0) {
                m_num_shader_strings = (static_cast<int>(i) + 1);
                break;
            }
            len = count;
        }
        ++i;
    }
    return return_data;
}

void VkTestFramework::FreeFileData(char **data) {
    for (int i = 0; i < m_num_shader_strings; i++) free(data[i]);
}

//
//   Deduce the language from the filename.  Files must end in one of the
//   following extensions:
//
//   .vert = vertex
//   .tesc = tessellation control
//   .tese = tessellation evaluation
//   .geom = geometry
//   .frag = fragment
//   .comp = compute
//
EShLanguage VkTestFramework::FindLanguage(const std::string &name) {
    size_t ext = name.rfind('.');
    if (ext == std::string::npos) {
        return EShLangVertex;
    }

    std::string suffix = name.substr(ext + 1, std::string::npos);
    if (suffix == "vert")
        return EShLangVertex;
    else if (suffix == "tesc")
        return EShLangTessControl;
    else if (suffix == "tese")
        return EShLangTessEvaluation;
    else if (suffix == "geom")
        return EShLangGeometry;
    else if (suffix == "frag")
        return EShLangFragment;
    else if (suffix == "comp")
        return EShLangCompute;

    return EShLangVertex;
}

//
// Convert VK shader type to compiler's
//
EShLanguage VkTestFramework::FindLanguage(const VkShaderStageFlagBits shader_type) {
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
bool VkTestFramework::GLSLtoSPV(VkPhysicalDeviceLimits const *const device_limits, const VkShaderStageFlagBits shader_type,
                                const char *pshader, std::vector<uint32_t> &spirv, bool debug, const spv_target_env spv_env) {
    glslang::TProgram program;
    const char *shaderStrings[1];

    // TODO: Do we want to load a special config file depending on the
    // shader source? Optional name maybe?
    //    SetConfigFile(fileName);

    ProcessConfigFile(device_limits);

    EShMessages messages = EShMsgDefault;
    SetMessageOptions(messages);
    messages = static_cast<EShMessages>(messages | EShMsgSpvRules | EShMsgVulkanRules);
    if (debug) {
        messages = static_cast<EShMessages>(messages | EShMsgDebugInfo);
    }

    EShLanguage stage = FindLanguage(shader_type);
    glslang::TShader *shader = new glslang::TShader(stage);
    GlslangTargetEnv glslang_env(spv_env);
    shader->setEnvTarget(glslang::EshTargetSpv, glslang_env);
    shader->setEnvClient(glslang::EShClientVulkan, glslang_env);

    shaderStrings[0] = pshader;
    shader->setStrings(shaderStrings, 1);

    if (!shader->parse(&Resources, (m_compile_options & EOptionDefaultDesktop) ? 110 : 100, false, messages)) {
        if (!(m_compile_options & EOptionSuppressInfolog)) {
            puts(shader->getInfoLog());
            puts(shader->getInfoDebugLog());
        }

        return false;  // something didn't work
    }

    program.addShader(shader);

    //
    // Program-level processing...
    //

    if (!program.link(messages)) {
        if (!(m_compile_options & EOptionSuppressInfolog)) {
            puts(shader->getInfoLog());
            puts(shader->getInfoDebugLog());
        }

        return false;
    }

    if (m_compile_options & EOptionDumpReflection) {
        program.buildReflection();
        program.dumpReflection();
    }

    glslang::SpvOptions spv_options;
    if (debug) {
        spv_options.generateDebugInfo = true;
    }
    glslang::GlslangToSpv(*program.getIntermediate(stage), spirv, &spv_options);

    //
    // Test the different modes of SPIR-V modification
    //
    if (this->m_canonicalize_spv) {
        spv::spirvbin_t(0).remap(spirv, spv::spirvbin_t::ALL_BUT_STRIP);
    }

    if (this->m_strip_spv) {
        spv::spirvbin_t(0).remap(spirv, spv::spirvbin_t::STRIP);
    }

    if (this->m_do_everything_spv) {
        spv::spirvbin_t(0).remap(spirv, spv::spirvbin_t::DO_EVERYTHING);
    }

    delete shader;

    return true;
}

//
// Compile a given string containing SPIR-V assembly into SPV for use by VK
// Return value of false means an error was encountered.
//
bool VkTestFramework::ASMtoSPV(const spv_target_env target_env, const uint32_t options, const char *pasm,
                               std::vector<uint32_t> &spv) {
    spv_binary binary;
    spv_diagnostic diagnostic = nullptr;
    spv_context context = spvContextCreate(target_env);
    spv_result_t error = spvTextToBinaryWithOptions(context, pasm, strlen(pasm), options, &binary, &diagnostic);
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
