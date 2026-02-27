/*
 * Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
 * Copyright (c) 2026 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

class NegativeGpuAVSharedMemoryDataRace : public GpuAVSharedMemoryDataRaceTest {
  protected:
    void TestHelper(const char *source, uint32_t count, const char *error = "SharedMemoryDataRace");
};

void NegativeGpuAVSharedMemoryDataRace::TestHelper(const char* shader_source, uint32_t count, const char* error) {
    TEST_DESCRIPTION("Shared memory, data race");
    RETURN_IF_SKIP(InitSharedMemoryDataRace(count));

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError(error, count);
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, SingleScalar) {
    const char *shader_source = R"glsl(
        #version 450

        layout(local_size_x = 2) in;
        shared uint temp;
        void main() {
            temp = 0;
        }
    )glsl";

    TestHelper(shader_source, 1);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, SingleElementArray) {
    const char *shader_source = R"glsl(
        #version 450

        layout(local_size_x = 3) in;
        shared uint temp[1];
        void main() {
            temp[0] = 0;
        }
    )glsl";

    TestHelper(shader_source, 2);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, TwoThreadsShareValuesThroughArray) {
    const char *shader_source = R"glsl(
        #version 450

        layout(local_size_x = 2) in;
        shared uint temp[2];
        void main() {
            temp[gl_LocalInvocationIndex] = 0;
            uint x = temp[gl_LocalInvocationIndex ^ 1];
        }
    )glsl";

    TestHelper(shader_source, 2);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, TwoDimensionalArray) {
    const char *shader_source = R"glsl(
        #version 450

        layout(local_size_x = 3) in;
        shared uint temp[3][2];
        void main() {
            temp[gl_LocalInvocationIndex][1] = 0;
            uint x = temp[(gl_LocalInvocationIndex + 1) % 3][1];
        }
    )glsl";

    TestHelper(shader_source, 3);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, BasicStructRace) {
    const char *shader_source = R"glsl(
        #version 450

        layout(local_size_x = 2) in;
        struct S { uint a, b; };
        shared S temp;
        void main() {
            if (gl_LocalInvocationIndex == 0) {
                temp.a = 0;
            } else {
                temp.b = 0;
            }
            if (gl_LocalInvocationIndex == 0) {
                temp.b = 0;
            } else {
                temp.a = 0;
            }
        }
    )glsl";

    TestHelper(shader_source, 1);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, StructVsScalarRace) {
    const char *shader_source = R"glsl(
        #version 450

        layout(local_size_x = 2) in;
        struct S { uint a, b; };
        shared S temp;
        void main() {
            if (gl_LocalInvocationIndex == 0) {
                S t2;
                temp = t2;
            }
            if (gl_LocalInvocationIndex == 1) {
                uint b = temp.b;
            }
        }
    )glsl";

    TestHelper(shader_source, 1);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, VectorVsScalarRace) {
    const char *shader_source = R"glsl(
        #version 450

        layout(local_size_x = 2) in;
        shared uvec4 temp;
        void main() {
            if (gl_LocalInvocationIndex == 0) {
                temp = uvec4(0);
            }
            if (gl_LocalInvocationIndex == 1) {
                uint b = temp.z;
            }
        }
    )glsl";

    TestHelper(shader_source, 1);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, TwoVariablesRace) {
    const char *shader_source = R"glsl(
        #version 450

        layout(local_size_x = 2) in;
        shared uint a, b;
        void main() {
            if (gl_LocalInvocationIndex == 0) {
                a = 0;
            } else {
                b = 0;
            }
            if (gl_LocalInvocationIndex == 1) {
                a = 0;
            } else {
                b = 0;
            }
        }
    )glsl";

    TestHelper(shader_source, 1);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, TwoVectorsRace) {
    const char *shader_source = R"glsl(
        #version 450

        layout(local_size_x = 2) in;
        shared uvec4 a, b;
        void main() {
            if (gl_LocalInvocationIndex == 0) {
                a = uvec4(0);
            } else {
                b = uvec4(0);
            }
            if (gl_LocalInvocationIndex == 1) {
                a = uvec4(0);
            } else {
                b = uvec4(0);
            }
        }
    )glsl";

    TestHelper(shader_source, 1);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, VectorArrayRace) {
    const char *shader_source = R"glsl(
        #version 450

        layout(local_size_x = 4) in;
        shared uvec4 arr[4];
        void main() {
            arr[gl_LocalInvocationIndex] = uvec4(gl_LocalInvocationIndex);
            uvec4 sum;
            for (uint i = 0; i < 4; ++i) {
                sum += arr[i];
            }
        }
    )glsl";

    TestHelper(shader_source, 1);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, ShortIndex) {
    AddRequiredFeature(vkt::Feature::shaderInt16);

    const char *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_int16: enable

        layout(local_size_x = 4) in;
        shared uvec4 arr[4];
        void main() {
            arr[gl_LocalInvocationIndex] = uvec4(gl_LocalInvocationIndex);
            uvec4 sum;
            for (int16_t i = int16_t(0); i < 4; ++i) {
                sum += arr[i];
            }
        }
    )glsl";

    TestHelper(shader_source, 1);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, SpecConstantArrayRace) {
    const char *shader_source = R"glsl(
        #version 450

        layout(local_size_x = 2) in;
        layout(constant_id = 0) const uint N = 2;
        shared uint temp[N];
        void main() {
            temp[gl_LocalInvocationIndex] = 0;
            uint x = temp[gl_LocalInvocationIndex ^ 1];
        }
    )glsl";

    TestHelper(shader_source, 2);
}

TEST_F(NegativeGpuAVSharedMemoryDataRace, VariableName) {
    const char *shader_source = R"glsl(
        #version 450

        layout(local_size_x = 2) in;
        layout(constant_id = 0) const uint N = 2;
        uint other0 = 0;
        shared uint temp0[N];
        shared uint temp1[N];
        shared uint temp2[N];
        shared uint temp3[N];
        shared uint temp4[N];
        shared uint temp5[N];
        shared uint temp6[N];
        shared uint temp7[N];
        shared uint temp8[N];
        shared uint temp9[N];
        shared uint temp10[N];
        uint other1 = 0;
        void main() {
            uint other2 = 0;
            temp0[gl_LocalInvocationIndex] = 0;
            temp1[gl_LocalInvocationIndex] = 0;
            temp2[gl_LocalInvocationIndex] = 0;
            temp3[gl_LocalInvocationIndex] = 0;
            temp4[gl_LocalInvocationIndex] = 0;
            temp5[gl_LocalInvocationIndex] = 0;
            temp6[gl_LocalInvocationIndex] = 0;
            temp7[gl_LocalInvocationIndex] = 0;
            temp8[gl_LocalInvocationIndex] = 0;
            temp9[gl_LocalInvocationIndex] = 0;
            temp10[gl_LocalInvocationIndex] = 0;
            uint x = temp6[gl_LocalInvocationIndex ^ 1];
            uint other3 = 0;
        }
    )glsl";

    TestHelper(shader_source, 2, "temp6");
}
