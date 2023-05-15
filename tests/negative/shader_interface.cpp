/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"

class NegativeShaderInterface : public VkLayerTest {};

TEST_F(NegativeShaderInterface, MaxVertexComponentsWithBuiltins) {
    TEST_DESCRIPTION("Test if the max componenets checks are being checked from OpMemberDecorate built-ins");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    props.limits.maxVertexOutputComponents = 128;
    props.limits.maxFragmentInputComponents = 128;
    fpvkSetPhysicalDeviceLimitsEXT(gpu(), &props.limits);

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // vec4 == 4 components
    // This gives 124 which is just below the set max limit
    const uint32_t numVec4 = 31;

    std::string vsSourceStr =
        "#version 450\n"
        "layout(location = 0) out block {\n";
    for (uint32_t i = 0; i < numVec4; i++) {
        vsSourceStr += "vec4 v" + std::to_string(i) + ";\n";
    }
    vsSourceStr +=
        "} outVs;\n"
        "\n"
        "void main() {\n"
        "    vec4 x = vec4(1.0);\n";
    for (uint32_t i = 0; i < numVec4; i++) {
        vsSourceStr += "outVs.v" + std::to_string(i) + " = x;\n";
    }

    // GLSL is defined to have a struct for the vertex shader built-in:
    //
    //    out gl_PerVertex {
    //        vec4 gl_Position;
    //        float gl_PointSize;
    //        float gl_ClipDistance[];
    //        float gl_CullDistance[];
    //    } gl_out[];
    //
    // by including gl_Position here 7 extra vertex input components are added pushing it over the 128
    // 124 + 7 > 128 limit
    vsSourceStr += "    gl_Position = x;\n";
    vsSourceStr += "}";

    std::string fsSourceStr =
        "#version 450\n"
        "layout(location = 0) in block {\n";
    for (uint32_t i = 0; i < numVec4; i++) {
        fsSourceStr += "vec4 v" + std::to_string(i) + ";\n";
    }
    fsSourceStr +=
        "} inPs;\n"
        "\n"
        "layout(location=0) out vec4 color;\n"
        "\n"
        "void main(){\n"
        "    color = vec4(1);\n"
        "}\n";

    VkShaderObj vs(this, vsSourceStr.c_str(), VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSourceStr.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };

    // maxFragmentInputComponents is not reached because GLSL should not be including any input fragment stage built-ins by default
    // only maxVertexOutputComponents is reached
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-Location-06272");
}

TEST_F(NegativeShaderInterface, MaxFragmentComponentsWithBuiltins) {
    TEST_DESCRIPTION("Test if the max componenets checks are being checked from OpDecorate built-ins");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    props.limits.maxVertexOutputComponents = 128;
    props.limits.maxFragmentInputComponents = 128;
    fpvkSetPhysicalDeviceLimitsEXT(gpu(), &props.limits);

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // vec4 == 4 components
    // This gives 128 which is the max limit
    const uint32_t numVec4 = 32;  // 32 * 4 == 128

    std::string vsSourceStr =
        "#version 450\n"
        "layout(location = 0) out block {\n";
    for (uint32_t i = 0; i < numVec4; i++) {
        vsSourceStr += "vec4 v" + std::to_string(i) + ";\n";
    }
    vsSourceStr +=
        "} outVs;\n"
        "\n"
        "void main() {\n"
        "    vec4 x = vec4(1.0);\n";
    for (uint32_t i = 0; i < numVec4; i++) {
        vsSourceStr += "outVs.v" + std::to_string(i) + " = x;\n";
    }
    vsSourceStr += "}";

    std::string fsSourceStr =
        "#version 450\n"
        "layout(location = 0) in block {\n";
    for (uint32_t i = 0; i < numVec4; i++) {
        fsSourceStr += "vec4 v" + std::to_string(i) + ";\n";
    }
    // By added gl_PointCoord it adds 2 more components to the fragment input stage
    fsSourceStr +=
        "} inPs;\n"
        "\n"
        "layout(location=0) out vec4 color;\n"
        "\n"
        "void main(){\n"
        "    color = vec4(1) * gl_PointCoord.x;\n"
        "}\n";

    VkShaderObj vs(this, vsSourceStr.c_str(), VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSourceStr.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };

    // maxVertexOutputComponents is not reached because GLSL should not be including any output vertex stage built-ins
    // only maxFragmentInputComponents is reached
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-Location-06272");
}

TEST_F(NegativeShaderInterface, MaxVertexOutputComponents) {
    TEST_DESCRIPTION(
        "Test that an error is produced when the number of output components from the vertex stage exceeds the device limit");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // overflow == 0: no overflow, 1: too many components, 2: location number too large
    for (uint32_t overflow = 0; overflow < 3; ++overflow) {
        m_errorMonitor->Reset();

        const uint32_t maxVsOutComp = m_device->props.limits.maxVertexOutputComponents + overflow;
        std::string vsSourceStr = "#version 450\n\n";
        const uint32_t numVec4 = maxVsOutComp / 4;
        uint32_t location = 0;
        if (overflow == 2) {
            vsSourceStr += "layout(location=" + std::to_string(numVec4 + 1) + ") out vec4 vn;\n";
        } else if (overflow == 1) {
            for (uint32_t i = 0; i < numVec4; i++) {
                vsSourceStr += "layout(location=" + std::to_string(location) + ") out vec4 v" + std::to_string(i) + ";\n";
                location += 1;
            }
            const uint32_t remainder = maxVsOutComp % 4;
            if (remainder != 0) {
                if (remainder == 1) {
                    vsSourceStr += "layout(location=" + std::to_string(location) + ") out float" + " vn;\n";
                } else {
                    vsSourceStr +=
                        "layout(location=" + std::to_string(location) + ") out vec" + std::to_string(remainder) + " vn;\n";
                }
                location += 1;
            }
        }
        vsSourceStr +=
            "void main(){\n"
            "}\n";

        std::string fsSourceStr = R"glsl(
            #version 450
            layout(location=0) out vec4 color;
            void main(){
                color = vec4(1);
            }
        )glsl";

        VkShaderObj vs(this, vsSourceStr.c_str(), VK_SHADER_STAGE_VERTEX_BIT);
        VkShaderObj fs(this, fsSourceStr.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };

        switch (overflow) {
            case 0: {
                CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
                break;
            }
            case 1: {
                // component and location limit (maxVertexOutputComponents)
                CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-Location-06272");
                break;
            }
            case 2: {
                // just component limit (maxVertexOutputComponents)
                CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-Location-06272");
                break;
            }
            default: {
                assert(0);
            }
        }
    }
}

TEST_F(NegativeShaderInterface, MaxComponentsBlocks) {
    TEST_DESCRIPTION("Test if the max componenets checks are done properly when in a single block");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // To make the test simple, just make sure max is 128 or less (most HW is 64 or 128)
    if (m_device->props.limits.maxVertexOutputComponents > 128 || m_device->props.limits.maxFragmentInputComponents > 128) {
        GTEST_SKIP() << "maxVertexOutputComponents or maxFragmentInputComponents too high for test";
    }
    // vec4 == 4 components
    // so this put the test over 128
    const uint32_t numVec4 = 33;

    std::string vsSourceStr =
        "#version 450\n"
        "layout(location = 0) out block {\n";
    for (uint32_t i = 0; i < numVec4; i++) {
        vsSourceStr += "vec4 v" + std::to_string(i) + ";\n";
    }
    vsSourceStr +=
        "} outVs;\n"
        "\n"
        "void main() {\n"
        "    vec4 x = vec4(1.0);\n";
    for (uint32_t i = 0; i < numVec4; i++) {
        vsSourceStr += "outVs.v" + std::to_string(i) + " = x;\n";
    }
    vsSourceStr += "}";

    std::string fsSourceStr =
        "#version 450\n"
        "layout(location = 0) in block {\n";
    for (uint32_t i = 0; i < numVec4; i++) {
        fsSourceStr += "vec4 v" + std::to_string(i) + ";\n";
    }
    fsSourceStr +=
        "} inPs;\n"
        "\n"
        "layout(location=0) out vec4 color;\n"
        "\n"
        "void main(){\n"
        "    color = vec4(1);\n"
        "}\n";

    VkShaderObj vs(this, vsSourceStr.c_str(), VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSourceStr.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };

    // 1 for maxVertexOutputComponents and 1 for maxFragmentInputComponents
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                      vector<string>{"VUID-RuntimeSpirv-Location-06272", "VUID-RuntimeSpirv-Location-06272"});
}

TEST_F(NegativeShaderInterface, MaxFragmentInputComponents) {
    TEST_DESCRIPTION(
        "Test that an error is produced when the number of input components from the fragment stage exceeds the device limit");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // overflow == 0: no overflow, 1: too many components, 2: location number too large
    for (uint32_t overflow = 0; overflow < 3; ++overflow) {
        m_errorMonitor->Reset();

        const uint32_t maxFsInComp = m_device->props.limits.maxFragmentInputComponents + overflow;
        std::string fsSourceStr = "#version 450\n\n";
        const uint32_t numVec4 = maxFsInComp / 4;
        uint32_t location = 0;
        if (overflow == 2) {
            fsSourceStr += "layout(location=" + std::to_string(numVec4 + 1) + ") in float" + " vn;\n";
        } else {
            for (uint32_t i = 0; i < numVec4; i++) {
                fsSourceStr += "layout(location=" + std::to_string(location) + ") in vec4 v" + std::to_string(i) + ";\n";
                location += 1;
            }
            const uint32_t remainder = maxFsInComp % 4;
            if (remainder != 0) {
                if (remainder == 1) {
                    fsSourceStr += "layout(location=" + std::to_string(location) + ") in float" + " vn;\n";
                } else {
                    fsSourceStr +=
                        "layout(location=" + std::to_string(location) + ") in vec" + std::to_string(remainder) + " vn;\n";
                }
                location += 1;
            }
        }
        fsSourceStr +=
            "layout(location=0) out vec4 color;"
            "\n"
            "void main(){\n"
            "    color = vec4(1);\n"
            "}\n";
        VkShaderObj fs(this, fsSourceStr.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT);

        m_errorMonitor->SetUnexpectedError("VUID-RuntimeSpirv-OpEntryPoint-08743");
        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };
        switch (overflow) {
            case 0: {
                CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
                break;
            }
            case 1: {
                // (maxFragmentInputComponents)
                CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-Location-06272");
                break;
            }
            case 2:
                // (maxFragmentInputComponents)
                CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-Location-06272");
                break;
            default: {
                assert(0);
            }
        }
    }
}

TEST_F(NegativeShaderInterface, FragmentInputNotProvided) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a fragment shader input which is not present in the outputs of the previous stage");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) in float x;
        layout(location=0) out vec4 color;
        void main(){
           color = vec4(x);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
}

TEST_F(NegativeShaderInterface, FragmentInputNotProvidedInBlock) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a fragment shader input within an interace block, which is not present in the outputs "
        "of the previous stage.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *fsSource = R"glsl(
        #version 450
        in block { layout(location=0) float x; } ins;
        layout(location=0) out vec4 color;
        void main(){
           color = vec4(ins.x);
        }
    )glsl";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
}

TEST_F(NegativeShaderInterface, VsFsTypeMismatch) {
    TEST_DESCRIPTION("Test that an error is produced for mismatched types across the vertex->fragment shader interface");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) out int x;
        void main(){
           x = 0;
           gl_Position = vec4(1);
        }
    )glsl";
    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) in float x; /* VS writes int */
        layout(location=0) out vec4 color;
        void main(){
           color = vec4(x);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-07754");
}

TEST_F(NegativeShaderInterface, VsFsTypeMismatchInBlock) {
    TEST_DESCRIPTION(
        "Test that an error is produced for mismatched types across the vertex->fragment shader interface, when the variable is "
        "contained within an interface block");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
        #version 450
        out block { layout(location=0) int x; } outs;
        void main(){
           outs.x = 0;
           gl_Position = vec4(1);
        }
    )glsl";
    char const *fsSource = R"glsl(
        #version 450
        in block { layout(location=0) float x; } ins; /* VS writes int */
        layout(location=0) out vec4 color;
        void main(){
           color = vec4(ins.x);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-07754");
}

TEST_F(NegativeShaderInterface, VsFsTypeMismatchVectorSize) {
    TEST_DESCRIPTION("OpTypeVector has larger output than input");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddOptionalExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 x;
        void main(){
           gl_Position = vec4(1.0);
        }
    )glsl";
    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) in vec3 x;
        layout(location=0) out vec4 color;
        void main(){
           color = vec4(1.0);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    const char *vuid = IsExtensionsEnabled(VK_KHR_MAINTENANCE_4_EXTENSION_NAME) ? "VUID-RuntimeSpirv-maintenance4-06817"
                                                                                : "VUID-RuntimeSpirv-OpTypeVector-06816";
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuid);
}

TEST_F(NegativeShaderInterface, VsFsMismatchByLocation) {
    TEST_DESCRIPTION(
        "Test that an error is produced for location mismatches across the vertex->fragment shader interface; This should manifest "
        "as a not-written/not-consumed pair, but flushes out broken walking of the interfaces");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
        #version 450
        out block { layout(location=1) float x; } outs;
        void main(){
           outs.x = 0;
           gl_Position = vec4(1);
        }
    )glsl";
    char const *fsSource = R"glsl(
        #version 450
        in block { layout(location=0) float x; } ins;
        layout(location=0) out vec4 color;
        void main(){
           color = vec4(ins.x);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
}

TEST_F(NegativeShaderInterface, VsFsMismatchByComponent) {
    TEST_DESCRIPTION(
        "Test that an error is produced for component mismatches across the vertex->fragment shader interface. It's not enough to "
        "have the same set of locations in use; matching is defined in terms of spirv variables.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
        #version 450
        out block { layout(location=0, component=0) float x; } outs;
        void main(){
           outs.x = 0;
           gl_Position = vec4(1);
        }
    )glsl";
    char const *fsSource = R"glsl(
        #version 450
        in block { layout(location=0, component=1) float x; } ins;
        layout(location=0) out vec4 color;
        void main(){
           color = vec4(ins.x);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
}

TEST_F(NegativeShaderInterface, InputOutputMismatch) {
    TEST_DESCRIPTION("Test mismatch between vertex shader output and fragment shader input.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const char vsSource[] = R"glsl(
        #version 450
        layout(location = 1) out int v;
        void main() {
            v = 1;
        }
    )glsl";

    const char fsSource[] = R"glsl(
        #version 450
        layout(location = 0) out vec4 color;
        layout(location = 1) in float v;
        void main() {
           color = vec4(v);
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-07754");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderInterface, VertexOutputNotConsumed) {
    TEST_DESCRIPTION("Test that a warning is produced for a vertex output that is not consumed by the fragment stage");

    SetTargetApiVersion(VK_API_VERSION_1_0);

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) out float x;
        void main(){
           gl_Position = vec4(1);
           x = 0;
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kPerformanceWarningBit,
                                      "UNASSIGNED-CoreValidation-Shader-OutputNotConsumed");
}

// Spec doesn't clarify if this is valid or not
// https://gitlab.khronos.org/vulkan/vulkan/-/issues/3293
TEST_F(NegativeShaderInterface, DISABLED_InputAndOutputComponents) {
    TEST_DESCRIPTION("Test invalid shader layout in and out with different components.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    {
        char const *vsSource = R"glsl(
                #version 450

                layout(location = 0, component = 0) out float r;
                layout(location = 0, component = 2) out float b;

                void main() {
                    r = 0.25f;
                    b = 0.75f;
                }
            )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

        char const *fsSource = R"glsl(
                #version 450

                layout(location = 0) in vec3 rgb;

                layout (location = 0) out vec4 color;

                void main() {
                    color = vec4(rgb, 1.0f);
                }
            )glsl";
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
    }

    {
        char const *vsSource = R"glsl(
                #version 450

                layout(location = 0) out vec3 v;

                void main() {
                }
            )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

        char const *fsSource = R"glsl(
                #version 450

                layout(location = 0, component = 0) in float a;
                layout(location = 0, component = 2) in float b;

                layout (location = 0) out vec4 color;

                void main() {
                    color = vec4(1.0f);
                }
            )glsl";
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kPerformanceWarningBit,
                                          "UNASSIGNED-CoreValidation-Shader-OutputNotConsumed");
    }

    {
        char const *vsSource = R"glsl(
                #version 450

                layout(location = 0) out vec3 v;

                void main() {
                    v = vec3(1.0);
                }
            )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

        char const *fsSource = R"glsl(
                #version 450

                layout(location = 0) in vec4 v;

                layout (location = 0) out vec4 color;

                void main() {
                    color = v;
                }
            )glsl";
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
    }

    {
        char const *vsSource = R"glsl(
                #version 450

                layout(location = 0) out vec3 v;

                void main() {
                    v = vec3(1.0);
                }
            )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

        char const *fsSource = R"glsl(
                #version 450

                layout (location = 0) out vec4 color;

                void main() {
                    color = vec4(1.0);
                }
            )glsl";
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }

    {
        char const *vsSource = R"glsl(
                #version 450

                layout(location = 0) out vec3 v1;
                layout(location = 1) out vec3 v2;
                layout(location = 2) out vec3 v3;

                void main() {
                    v1 = vec3(1.0);
                    v2 = vec3(2.0);
                    v3 = vec3(3.0);
                }
            )glsl";
        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

        char const *fsSource = R"glsl(
                #version 450

                layout (location = 0) in vec3 v1;
                layout (location = 2) in vec3 v3;

                layout (location = 0) out vec4 color;

                void main() {
                    color = vec4(v1 * v3, 1.0);
                }
            )glsl";
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
    }
}

// Currently need to clarify the VU - https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5520
TEST_F(NegativeShaderInterface, DISABLED_NoOutputLocation0ButAlphaToCoverageEnabled) {
    TEST_DESCRIPTION("Test that an error is produced when alpha to coverage is enabled but no output at location 0 is declared.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(0u));

    VkShaderObj fs(this, bindStateMinimalShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineMultisampleStateCreateInfo ms_state_ci = LvlInitStruct<VkPipelineMultisampleStateCreateInfo>();
    ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms_state_ci.alphaToCoverageEnable = VK_TRUE;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.pipe_ms_state_ci_ = ms_state_ci;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                      "UNASSIGNED-CoreValidation-Shader-NoAlphaAtLocation0WithAlphaToCoverage");
}

// Currently need to clarify the VU - https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5520
TEST_F(NegativeShaderInterface, DISABLED_NoAlphaLocation0ButAlphaToCoverageEnabled) {
    TEST_DESCRIPTION(
        "Test that an error is produced when alpha to coverage is enabled but output at location 0 doesn't have alpha component.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(0u));

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec3 x;
        void main(){
           x = vec3(1);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineMultisampleStateCreateInfo ms_state_ci = LvlInitStruct<VkPipelineMultisampleStateCreateInfo>();
    ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms_state_ci.alphaToCoverageEnable = VK_TRUE;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.pipe_ms_state_ci_ = ms_state_ci;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                      "UNASSIGNED-CoreValidation-Shader-NoAlphaAtLocation0WithAlphaToCoverage");
}
