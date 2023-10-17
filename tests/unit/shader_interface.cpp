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
#include "../framework/pipeline_helper.h"

TEST_F(NegativeShaderInterface, MaxVertexComponentsWithBuiltins) {
    TEST_DESCRIPTION("Test if the max componenets checks are being checked from OpMemberDecorate built-ins");

    RETURN_IF_SKIP(InitFramework())
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

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

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

    RETURN_IF_SKIP(InitFramework())
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

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

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

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    // overflow == 0: no overflow, 1: too many components, 2: location number too large
    for (uint32_t overflow = 0; overflow < 3; ++overflow) {
        m_errorMonitor->Reset();

        const uint32_t maxVsOutComp = m_device->phy().limits_.maxVertexOutputComponents + overflow;
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

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    // To make the test simple, just make sure max is 128 or less (most HW is 64 or 128)
    if (m_device->phy().limits_.maxVertexOutputComponents > 128 || m_device->phy().limits_.maxFragmentInputComponents > 128) {
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

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    // overflow == 0: no overflow, 1: too many components, 2: location number too large
    for (uint32_t overflow = 0; overflow < 3; ++overflow) {
        m_errorMonitor->Reset();

        const uint32_t maxFsInComp = m_device->phy().limits_.maxFragmentInputComponents + overflow;
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

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

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

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

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

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

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

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

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
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

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
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-maintenance4-06817");
}

TEST_F(NegativeShaderInterface, VsFsTypeMismatchBlockStruct) {
    TEST_DESCRIPTION("Have a struct inside a block between shaders");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        struct S {
            vec4 a[2];
            float b;
            int c; // difference
        };

        out block {
            layout(location=0) float x;
            layout(location=6) S y;
            layout(location=10) int[4] z;
        } outBlock;

        void main() {
            outBlock.y.a[1] = vec4(1);
            gl_Position = vec4(1);
        }
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        struct S {
            vec4 a[2];
            float b;
            float c; // difference
        };

        in block {
            layout(location=0) float x;
            layout(location=6) S y;
            layout(location=10) int[4] z;
        } inBlock;

        layout(location=0) out vec4 color;
        void main(){
            color = inBlock.y.a[1];
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-07754");
}

TEST_F(NegativeShaderInterface, VsFsTypeMismatchBlockStruct64bit) {
    TEST_DESCRIPTION("Have a struct inside a block between shaders");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();
    if (!m_device->phy().features().shaderFloat64) {
        GTEST_SKIP() << "Device does not support 64bit floats";
    }

    char const *vsSource = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable

        struct S {
            vec4 a[2];
            float b;
            f64vec3 c; // difference (takes 2 locations)
        };

        out block {
            layout(location=0) S x[2];
            layout(location=10) int y;
        } outBlock;

        void main() {}
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        struct S {
            vec4 a[2];
            float b;
            vec4 c; // difference
            vec2 d;
        };

        in block {
            layout(location=0) S x[2];
            layout(location=10) int y;
        } inBlock;

        layout(location=0) out vec4 color;
        void main(){}
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    // One for component 0 and 1
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-07754");
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-07754");
}

TEST_F(NegativeShaderInterface, VsFsTypeMismatchBlockArrayOfStruct) {
    TEST_DESCRIPTION("Have an array of struct inside a block between shaders");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        struct S {
            vec4 a[2];
            float b;
            int c; // difference
        };

        out block {
            layout(location=0) float x;
            layout(location=6) S[2] y; // each array is 4 locations slots
            layout(location=14) int[4] z;
        } outBlock;

        void main() {
            outBlock.y[1].a[1] = vec4(1);
            gl_Position = vec4(1);
        }
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        struct S {
            vec4 a[2];
            float b;
            float c; // difference
        };

        in block {
            layout(location=0) float x;
            layout(location=6) S[2] y;
            layout(location=14) int[4] z;
        } inBlock;

        layout(location=0) out vec4 color;
        void main(){
            color = inBlock.y[1].a[1];
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-07754");
}

TEST_F(NegativeShaderInterface, VsFsTypeMismatchBlockStructInnerArraySize) {
    TEST_DESCRIPTION("Have an struct inside a block between shaders, array size is difference");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();
    if (m_device->phy().limits_.maxVertexOutputComponents <= 64) {
        GTEST_SKIP() << "maxVertexOutputComponents is too low";
    }

    char const *vsSource = R"glsl(
        #version 450
        struct S {
            vec4 a[2];
            float b;
            int[2] c; // difference
        };

        out block {
            layout(location=0) float x;
            layout(location=6) S[2] y;
            layout(location=18) int[4] z;
        } outBlock;

        void main() {
            outBlock.y[1].a[1] = vec4(1);
            gl_Position = vec4(1);
        }
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        struct S {
            vec4 a[2];
            float b;
            int[3] c; // difference
        };

        in block {
            layout(location=0) float x;
            layout(location=6) S[2] y;
            layout(location=18) int[4] z;
        } inBlock;

        layout(location=0) out vec4 color;
        void main(){
            color = inBlock.y[1].a[1];
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    // Both are errors, depending on compiler, the order of variables listed will hit one before the other
    m_errorMonitor->SetUnexpectedError("VUID-RuntimeSpirv-OpEntryPoint-07754");
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
}

TEST_F(NegativeShaderInterface, VsFsTypeMismatchBlockStructOuterArraySize) {
    TEST_DESCRIPTION("Have an struct inside a block between shaders, array size is difference");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();
    if (m_device->phy().limits_.maxVertexOutputComponents <= 64) {
        GTEST_SKIP() << "maxVertexOutputComponents is too low";
    }

    char const *vsSource = R"glsl(
        #version 450
        struct S {
            vec4 a[2];
            float b;
            float c;
        };

        out block {
            layout(location=0) float x;
            layout(location=6) S[2] y; // difference
            layout(location=20) int[4] z;
        } outBlock;

        void main() {
            outBlock.y[1].a[1] = vec4(1);
            gl_Position = vec4(1);
        }
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        struct S {
            vec4 a[2];
            float b;
            float c;
        };

        in block {
            layout(location=0) float x;
            layout(location=6) S[3] y; // difference
            layout(location=20) int[4] z;
        } inBlock;

        layout(location=0) out vec4 color;
        void main(){
            color = inBlock.y[1].a[1];
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
}

TEST_F(NegativeShaderInterface, VsFsTypeMismatchBlockStructArraySizeVertex) {
    TEST_DESCRIPTION(
        "Have an struct inside a block between shaders, array size is difference, but from the vertex shader being too large");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();
    if (m_device->phy().limits_.maxVertexOutputComponents <= 64) {
        GTEST_SKIP() << "maxVertexOutputComponents is too low";
    }

    char const *vsSource = R"glsl(
        #version 450
        struct S {
            vec4 a[2];
            float b;
            int[3] c; // difference
        };

        out block {
            layout(location=0) float x;
            layout(location=6) S[2] y;
            layout(location=18) int[4] z;
        } outBlock;

        void main() {
            outBlock.y[1].a[1] = vec4(1);
            gl_Position = vec4(1);
        }
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        struct S {
            vec4 a[2];
            float b;
            int[2] c; // difference
        };

        in block {
            layout(location=0) float x;
            layout(location=6) S[2] y;
            layout(location=18) int[4] z;
        } inBlock;

        layout(location=0) out vec4 color;
        void main(){
            color = inBlock.y[1].a[1];
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    // Both are errors, depending on compiler, the order of variables listed will hit one before the other
    m_errorMonitor->SetUnexpectedError("VUID-RuntimeSpirv-OpEntryPoint-08743");
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-07754");
}

TEST_F(NegativeShaderInterface, VsFsTypeMismatchBlockStructOuter2DArraySize) {
    TEST_DESCRIPTION("Have an struct inside a block between shaders, array size is difference");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();
    if (m_device->phy().limits_.maxVertexOutputComponents <= 64) {
        GTEST_SKIP() << "maxVertexOutputComponents is too low";
    }

    char const *vsSource = R"glsl(
        #version 450
        struct S {
            vec4 a[2];
            float b;
        };

        out block {
            layout(location=0) float x;
            layout(location=2) S[2][2] y; // difference
            layout(location=21) int[2] z;
        } outBlock;

        void main() {}
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        struct S {
            vec4 a[2];
            float b;
        };

        in block {
            layout(location=0) float x;
            layout(location=2) S[2][3] y; // difference
            layout(location=21) int[2] z;
        } inBlock;

        layout(location=0) out vec4 color;
        void main(){}
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
}

TEST_F(NegativeShaderInterface, VsFsTypeMismatchBlockNestedStructType64bit) {
    TEST_DESCRIPTION("Have nested struct inside a block between shaders");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();
    if (!m_device->phy().features().shaderFloat64) {
        GTEST_SKIP() << "Device does not support 64bit floats";
    }

    char const *vsSource = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable

        struct A {
            float a0_;
        };
        struct B {
            f64vec3 b0_;
            A b1_;
        };
        struct C {
            A c1_;
            B c2_;
        };

        out block {
            layout(location=0) float x;
            layout(location=1) C y;
        } outBlock;

        void main() {}
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        struct A {
            float a0_;
        };
        struct B {
            vec3 b0_;
            A b1_;
        };
        struct C {
            A c1_;
            B c2_;
        };

        in block {
            layout(location=0) float x;
            layout(location=1) C y;
        } inBlock;

        layout(location=0) out vec4 color;
        void main(){}
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    // Depending on compiler sorting order, might report multiple 07754 VUs
    m_errorMonitor->SetAllowedFailureMsg("VUID-RuntimeSpirv-OpEntryPoint-07754");
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-07754");
}

TEST_F(NegativeShaderInterface, VsFsTypeMismatchBlockNestedStructArray) {
    TEST_DESCRIPTION("Have nested struct inside a block between shaders");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        struct A {
            float a0_;
        };
        struct B {
            int b0_;
            A b1_; // difference
        };
        struct C {
            vec4 c0_[2];
            A c1_;
            B c2_;
        };

        out block {
            layout(location=0) float x;
            layout(location=1) C y;
        } outBlock;

        void main() {}
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        struct A {
            float a0_;
        };
        struct B {
            int b0_;
            A b1_[2];  // difference
        };
        struct C {
            vec4 c0_[2];
            A c1_;
            B c2_;
        };

        in block {
            layout(location=0) float x;
            layout(location=1) C y;
        } inBlock;

        layout(location=0) out vec4 color;
        void main(){}
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
}

TEST_F(NegativeShaderInterface, VsFsMismatchByLocation) {
    TEST_DESCRIPTION(
        "Test that an error is produced for location mismatches across the vertex->fragment shader interface; This should manifest "
        "as a not-written/not-consumed pair, but flushes out broken walking of the interfaces");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

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

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

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

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

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
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-07754");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderInterface, VertexOutputNotConsumed) {
    TEST_DESCRIPTION("Test that a warning is produced for a vertex output that is not consumed by the fragment stage");

    SetTargetApiVersion(VK_API_VERSION_1_0);

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

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

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

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

TEST_F(NegativeShaderInterface, AlphaToCoverageOutputLocation0) {
    TEST_DESCRIPTION("Test that an error is produced when alpha to coverage is enabled but no output at location 0 is declared.");

    RETURN_IF_SKIP(Init())
    InitRenderTarget(0u);

    VkShaderObj fs(this, kMinimalShaderGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineMultisampleStateCreateInfo ms_state_ci = vku::InitStructHelper();
    ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms_state_ci.alphaToCoverageEnable = VK_TRUE;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.pipe_ms_state_ci_ = ms_state_ci;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-alphaToCoverageEnable-08891");
}

TEST_F(NegativeShaderInterface, AlphaToCoverageOutputNoAlpha) {
    TEST_DESCRIPTION(
        "Test that an error is produced when alpha to coverage is enabled but output at location 0 doesn't have alpha component.");

    RETURN_IF_SKIP(Init())
    InitRenderTarget(0u);

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec3 x;
        void main(){
           x = vec3(1);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineMultisampleStateCreateInfo ms_state_ci = vku::InitStructHelper();
    ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms_state_ci.alphaToCoverageEnable = VK_TRUE;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.pipe_ms_state_ci_ = ms_state_ci;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-alphaToCoverageEnable-08891");
}

TEST_F(NegativeShaderInterface, AlphaToCoverageArrayIndex) {
    TEST_DESCRIPTION("Have array out outputs, but start at index 1");

    RETURN_IF_SKIP(Init())
    InitRenderTarget(0u);

    char const *fsSource = R"glsl(
        #version 450
        layout(location=1) out vec4 fragData[4];
        void main() {
            fragData[0] = vec4(1.0);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineMultisampleStateCreateInfo ms_state_ci = vku::InitStructHelper();
    ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms_state_ci.alphaToCoverageEnable = VK_TRUE;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.pipe_ms_state_ci_ = ms_state_ci;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-alphaToCoverageEnable-08891");
}

TEST_F(NegativeShaderInterface, AlphaToCoverageArrayVec3) {
    TEST_DESCRIPTION("Have array out outputs, but not contain the alpha component");

    RETURN_IF_SKIP(Init())
    InitRenderTarget(0u);

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec3 fragData[4];
        void main() {
            fragData[0] = vec3(1.0);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineMultisampleStateCreateInfo ms_state_ci = vku::InitStructHelper();
    ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms_state_ci.alphaToCoverageEnable = VK_TRUE;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.pipe_ms_state_ci_ = ms_state_ci;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-alphaToCoverageEnable-08891");
}

TEST_F(NegativeShaderInterface, MultidimensionalArray) {
    TEST_DESCRIPTION("Make sure multidimensional arrays are handled");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();
    if (m_device->phy().limits_.maxVertexOutputComponents <= 64) {
        GTEST_SKIP() << "maxVertexOutputComponents is too low";
    }

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) out float[4][2][2] x;
        void main() {}
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) in float[4][3][2] x; // 2 extra Locations
        layout(location=0) out float color;
        void main(){}
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
}

TEST_F(NegativeShaderInterface, MultidimensionalArrayDim) {
    TEST_DESCRIPTION("Make sure multidimensional arrays are handled");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();
    if (m_device->phy().limits_.maxVertexOutputComponents <= 64) {
        GTEST_SKIP() << "maxVertexOutputComponents is too low";
    }

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) out float[4][2][2] x;
        void main() {}
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) in float[17] x; // 1 extra Locations
        layout(location=0) out float color;
        void main(){}
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
}

TEST_F(NegativeShaderInterface, MultidimensionalArray64bit) {
    TEST_DESCRIPTION("Make sure multidimensional arrays are handled for 64bits");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();
    if (!m_device->phy().features().shaderFloat64) {
        GTEST_SKIP() << "Device does not support 64bit floats";
    }

    char const *vsSource = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
        layout(location=0) out f64vec3[2][2][2] x; // take 2 locations each (total 16)
        layout(location=24) out float y;
        void main() {}
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
        layout(location=0) flat in f64vec3[2][3][2] x;
        layout(location=24) out float color;
        void main(){}
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
}

TEST_F(NegativeShaderInterface, PackingInsideArray) {
    TEST_DESCRIPTION("From https://gitlab.khronos.org/vulkan/vulkan/-/issues/3558");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    char const *vsSource = R"glsl(
        #version 450
        layout(location = 0, component = 1) out float[2] x;
        void main() {}
    )glsl";

    char const *fsSource = R"glsl(
        #version 450
        layout(location = 0, component = 1) in float x;
        layout(location = 1, component = 0) in float y;
        layout(location=0) out float color;
        void main(){}
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEntryPoint-08743");
}

TEST_F(NegativeShaderInterface, CreatePipelineFragmentOutputNotWritten) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a fragment shader which does not provide an output for one of the pipeline's color "
        "attachments");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    VkShaderObj fs(this, kMinimalShaderGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.cb_attachments_[0].colorWriteMask = 1;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kWarningBit, "Undefined-Value-ShaderInputNotProduced");
}

TEST_F(NegativeShaderInterface, CreatePipelineFragmentOutputTypeMismatch) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a mismatch between the fundamental type of an fragment shader output variable, and the "
        "format of the corresponding attachment");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out ivec4 x; /* not UNORM */
        void main(){
           x = ivec4(1);
        }
    )glsl";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kWarningBit, "Undefined-Value-ShaderFragmentOutputMismatch");
}

TEST_F(NegativeShaderInterface, CreatePipelineFragmentOutputNotConsumed) {
    TEST_DESCRIPTION(
        "Test that a warning is produced for a fragment shader which provides a spurious output with no matching attachment");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 x;
        layout(location=1) out vec4 y; /* no matching attachment for this */
        void main(){
           x = vec4(1);
           y = vec4(1);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kWarningBit, "Undefined-Value-ShaderOutputNotConsumed");
}
