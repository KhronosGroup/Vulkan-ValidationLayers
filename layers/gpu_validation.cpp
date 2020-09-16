/* Copyright (c) 2018-2020 The Khronos Group Inc.
 * Copyright (c) 2018-2020 Valve Corporation
 * Copyright (c) 2018-2020 LunarG, Inc.
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
 * Author: Karl Schultz <karl@lunarg.com>
 * Author: Tony Barbour <tony@lunarg.com>
 */

#include "gpu_validation.h"
#include "spirv-tools/optimizer.hpp"
#include "spirv-tools/instrument.hpp"
#include "layer_chassis_dispatch.h"

static const VkShaderStageFlags kShaderStageAllRayTracing =
    VK_SHADER_STAGE_ANY_HIT_BIT_NV | VK_SHADER_STAGE_CALLABLE_BIT_NV | VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV |
    VK_SHADER_STAGE_INTERSECTION_BIT_NV | VK_SHADER_STAGE_MISS_BIT_NV | VK_SHADER_STAGE_RAYGEN_BIT_NV;

// Keep in sync with the GLSL shader below.
struct GpuAccelerationStructureBuildValidationBuffer {
    uint32_t instances_to_validate;
    uint32_t replacement_handle_bits_0;
    uint32_t replacement_handle_bits_1;
    uint32_t invalid_handle_found;
    uint32_t invalid_handle_bits_0;
    uint32_t invalid_handle_bits_1;
    uint32_t valid_handles_count;
};

// This is the GLSL source for the compute shader that is used during ray tracing acceleration structure
// building validation which inspects instance buffers for top level acceleration structure builds and
// reports and replaces invalid bottom level acceleration structure handles with good bottom level
// acceleration structure handle so that applications can continue without undefined behavior long enough
// to report errors.
//
// #version 450
// layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
// struct VkGeometryInstanceNV {
//     uint unused[14];
//     uint handle_bits_0;
//     uint handle_bits_1;
// };
// layout(set=0, binding=0, std430) buffer InstanceBuffer {
//     VkGeometryInstanceNV instances[];
// };
// layout(set=0, binding=1, std430) buffer ValidationBuffer {
//     uint instances_to_validate;
//     uint replacement_handle_bits_0;
//     uint replacement_handle_bits_1;
//     uint invalid_handle_found;
//     uint invalid_handle_bits_0;
//     uint invalid_handle_bits_1;
//     uint valid_handles_count;
//     uint valid_handles[];
// };
// void main() {
//     for (uint instance_index = 0; instance_index < instances_to_validate; instance_index++) {
//         uint instance_handle_bits_0 = instances[instance_index].handle_bits_0;
//         uint instance_handle_bits_1 = instances[instance_index].handle_bits_1;
//         bool valid = false;
//         for (uint valid_handle_index = 0; valid_handle_index < valid_handles_count; valid_handle_index++) {
//             if (instance_handle_bits_0 == valid_handles[2*valid_handle_index+0] &&
//                 instance_handle_bits_1 == valid_handles[2*valid_handle_index+1]) {
//                 valid = true;
//                 break;
//             }
//         }
//         if (!valid) {
//             invalid_handle_found += 1;
//             invalid_handle_bits_0 = instance_handle_bits_0;
//             invalid_handle_bits_1 = instance_handle_bits_1;
//             instances[instance_index].handle_bits_0 = replacement_handle_bits_0;
//             instances[instance_index].handle_bits_1 = replacement_handle_bits_1;
//         }
//     }
// }
//
// To regenerate the spirv below:
//   1. Save the above GLSL source to a file called validation_shader.comp.
//   2. Run in terminal
//
//      glslangValidator.exe -x -V validation_shader.comp -o validation_shader.comp.spv
//
//   4. Copy-paste the contents of validation_shader.comp.spv here (clang-format will fix up the alignment).
static const uint32_t kComputeShaderSpirv[] = {
    0x07230203, 0x00010000, 0x00080007, 0x0000006d, 0x00000000, 0x00020011, 0x00000001, 0x0006000b, 0x00000001, 0x4c534c47,
    0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0005000f, 0x00000005, 0x00000004, 0x6e69616d,
    0x00000000, 0x00060010, 0x00000004, 0x00000011, 0x00000001, 0x00000001, 0x00000001, 0x00030003, 0x00000002, 0x000001c2,
    0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00060005, 0x00000008, 0x74736e69, 0x65636e61, 0x646e695f, 0x00007865,
    0x00070005, 0x00000011, 0x696c6156, 0x69746164, 0x75426e6f, 0x72656666, 0x00000000, 0x00090006, 0x00000011, 0x00000000,
    0x74736e69, 0x65636e61, 0x6f745f73, 0x6c61765f, 0x74616469, 0x00000065, 0x000a0006, 0x00000011, 0x00000001, 0x6c706572,
    0x6d656361, 0x5f746e65, 0x646e6168, 0x625f656c, 0x5f737469, 0x00000030, 0x000a0006, 0x00000011, 0x00000002, 0x6c706572,
    0x6d656361, 0x5f746e65, 0x646e6168, 0x625f656c, 0x5f737469, 0x00000031, 0x00090006, 0x00000011, 0x00000003, 0x61766e69,
    0x5f64696c, 0x646e6168, 0x665f656c, 0x646e756f, 0x00000000, 0x00090006, 0x00000011, 0x00000004, 0x61766e69, 0x5f64696c,
    0x646e6168, 0x625f656c, 0x5f737469, 0x00000030, 0x00090006, 0x00000011, 0x00000005, 0x61766e69, 0x5f64696c, 0x646e6168,
    0x625f656c, 0x5f737469, 0x00000031, 0x00080006, 0x00000011, 0x00000006, 0x696c6176, 0x61685f64, 0x656c646e, 0x6f635f73,
    0x00746e75, 0x00070006, 0x00000011, 0x00000007, 0x696c6176, 0x61685f64, 0x656c646e, 0x00000073, 0x00030005, 0x00000013,
    0x00000000, 0x00080005, 0x0000001b, 0x74736e69, 0x65636e61, 0x6e61685f, 0x5f656c64, 0x73746962, 0x0000305f, 0x00080005,
    0x0000001e, 0x65476b56, 0x74656d6f, 0x6e497972, 0x6e617473, 0x564e6563, 0x00000000, 0x00050006, 0x0000001e, 0x00000000,
    0x73756e75, 0x00006465, 0x00070006, 0x0000001e, 0x00000001, 0x646e6168, 0x625f656c, 0x5f737469, 0x00000030, 0x00070006,
    0x0000001e, 0x00000002, 0x646e6168, 0x625f656c, 0x5f737469, 0x00000031, 0x00060005, 0x00000020, 0x74736e49, 0x65636e61,
    0x66667542, 0x00007265, 0x00060006, 0x00000020, 0x00000000, 0x74736e69, 0x65636e61, 0x00000073, 0x00030005, 0x00000022,
    0x00000000, 0x00080005, 0x00000027, 0x74736e69, 0x65636e61, 0x6e61685f, 0x5f656c64, 0x73746962, 0x0000315f, 0x00040005,
    0x0000002d, 0x696c6176, 0x00000064, 0x00070005, 0x0000002f, 0x696c6176, 0x61685f64, 0x656c646e, 0x646e695f, 0x00007865,
    0x00040047, 0x00000010, 0x00000006, 0x00000004, 0x00050048, 0x00000011, 0x00000000, 0x00000023, 0x00000000, 0x00050048,
    0x00000011, 0x00000001, 0x00000023, 0x00000004, 0x00050048, 0x00000011, 0x00000002, 0x00000023, 0x00000008, 0x00050048,
    0x00000011, 0x00000003, 0x00000023, 0x0000000c, 0x00050048, 0x00000011, 0x00000004, 0x00000023, 0x00000010, 0x00050048,
    0x00000011, 0x00000005, 0x00000023, 0x00000014, 0x00050048, 0x00000011, 0x00000006, 0x00000023, 0x00000018, 0x00050048,
    0x00000011, 0x00000007, 0x00000023, 0x0000001c, 0x00030047, 0x00000011, 0x00000003, 0x00040047, 0x00000013, 0x00000022,
    0x00000000, 0x00040047, 0x00000013, 0x00000021, 0x00000001, 0x00040047, 0x0000001d, 0x00000006, 0x00000004, 0x00050048,
    0x0000001e, 0x00000000, 0x00000023, 0x00000000, 0x00050048, 0x0000001e, 0x00000001, 0x00000023, 0x00000038, 0x00050048,
    0x0000001e, 0x00000002, 0x00000023, 0x0000003c, 0x00040047, 0x0000001f, 0x00000006, 0x00000040, 0x00050048, 0x00000020,
    0x00000000, 0x00000023, 0x00000000, 0x00030047, 0x00000020, 0x00000003, 0x00040047, 0x00000022, 0x00000022, 0x00000000,
    0x00040047, 0x00000022, 0x00000021, 0x00000000, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00040015,
    0x00000006, 0x00000020, 0x00000000, 0x00040020, 0x00000007, 0x00000007, 0x00000006, 0x0004002b, 0x00000006, 0x00000009,
    0x00000000, 0x0003001d, 0x00000010, 0x00000006, 0x000a001e, 0x00000011, 0x00000006, 0x00000006, 0x00000006, 0x00000006,
    0x00000006, 0x00000006, 0x00000006, 0x00000010, 0x00040020, 0x00000012, 0x00000002, 0x00000011, 0x0004003b, 0x00000012,
    0x00000013, 0x00000002, 0x00040015, 0x00000014, 0x00000020, 0x00000001, 0x0004002b, 0x00000014, 0x00000015, 0x00000000,
    0x00040020, 0x00000016, 0x00000002, 0x00000006, 0x00020014, 0x00000019, 0x0004002b, 0x00000006, 0x0000001c, 0x0000000e,
    0x0004001c, 0x0000001d, 0x00000006, 0x0000001c, 0x0005001e, 0x0000001e, 0x0000001d, 0x00000006, 0x00000006, 0x0003001d,
    0x0000001f, 0x0000001e, 0x0003001e, 0x00000020, 0x0000001f, 0x00040020, 0x00000021, 0x00000002, 0x00000020, 0x0004003b,
    0x00000021, 0x00000022, 0x00000002, 0x0004002b, 0x00000014, 0x00000024, 0x00000001, 0x0004002b, 0x00000014, 0x00000029,
    0x00000002, 0x00040020, 0x0000002c, 0x00000007, 0x00000019, 0x0003002a, 0x00000019, 0x0000002e, 0x0004002b, 0x00000014,
    0x00000036, 0x00000006, 0x0004002b, 0x00000014, 0x0000003b, 0x00000007, 0x0004002b, 0x00000006, 0x0000003c, 0x00000002,
    0x0004002b, 0x00000006, 0x00000048, 0x00000001, 0x00030029, 0x00000019, 0x00000050, 0x0004002b, 0x00000014, 0x00000058,
    0x00000003, 0x0004002b, 0x00000014, 0x0000005d, 0x00000004, 0x0004002b, 0x00000014, 0x00000060, 0x00000005, 0x00050036,
    0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003b, 0x00000007, 0x00000008, 0x00000007,
    0x0004003b, 0x00000007, 0x0000001b, 0x00000007, 0x0004003b, 0x00000007, 0x00000027, 0x00000007, 0x0004003b, 0x0000002c,
    0x0000002d, 0x00000007, 0x0004003b, 0x00000007, 0x0000002f, 0x00000007, 0x0003003e, 0x00000008, 0x00000009, 0x000200f9,
    0x0000000a, 0x000200f8, 0x0000000a, 0x000400f6, 0x0000000c, 0x0000000d, 0x00000000, 0x000200f9, 0x0000000e, 0x000200f8,
    0x0000000e, 0x0004003d, 0x00000006, 0x0000000f, 0x00000008, 0x00050041, 0x00000016, 0x00000017, 0x00000013, 0x00000015,
    0x0004003d, 0x00000006, 0x00000018, 0x00000017, 0x000500b0, 0x00000019, 0x0000001a, 0x0000000f, 0x00000018, 0x000400fa,
    0x0000001a, 0x0000000b, 0x0000000c, 0x000200f8, 0x0000000b, 0x0004003d, 0x00000006, 0x00000023, 0x00000008, 0x00070041,
    0x00000016, 0x00000025, 0x00000022, 0x00000015, 0x00000023, 0x00000024, 0x0004003d, 0x00000006, 0x00000026, 0x00000025,
    0x0003003e, 0x0000001b, 0x00000026, 0x0004003d, 0x00000006, 0x00000028, 0x00000008, 0x00070041, 0x00000016, 0x0000002a,
    0x00000022, 0x00000015, 0x00000028, 0x00000029, 0x0004003d, 0x00000006, 0x0000002b, 0x0000002a, 0x0003003e, 0x00000027,
    0x0000002b, 0x0003003e, 0x0000002d, 0x0000002e, 0x0003003e, 0x0000002f, 0x00000009, 0x000200f9, 0x00000030, 0x000200f8,
    0x00000030, 0x000400f6, 0x00000032, 0x00000033, 0x00000000, 0x000200f9, 0x00000034, 0x000200f8, 0x00000034, 0x0004003d,
    0x00000006, 0x00000035, 0x0000002f, 0x00050041, 0x00000016, 0x00000037, 0x00000013, 0x00000036, 0x0004003d, 0x00000006,
    0x00000038, 0x00000037, 0x000500b0, 0x00000019, 0x00000039, 0x00000035, 0x00000038, 0x000400fa, 0x00000039, 0x00000031,
    0x00000032, 0x000200f8, 0x00000031, 0x0004003d, 0x00000006, 0x0000003a, 0x0000001b, 0x0004003d, 0x00000006, 0x0000003d,
    0x0000002f, 0x00050084, 0x00000006, 0x0000003e, 0x0000003c, 0x0000003d, 0x00050080, 0x00000006, 0x0000003f, 0x0000003e,
    0x00000009, 0x00060041, 0x00000016, 0x00000040, 0x00000013, 0x0000003b, 0x0000003f, 0x0004003d, 0x00000006, 0x00000041,
    0x00000040, 0x000500aa, 0x00000019, 0x00000042, 0x0000003a, 0x00000041, 0x000300f7, 0x00000044, 0x00000000, 0x000400fa,
    0x00000042, 0x00000043, 0x00000044, 0x000200f8, 0x00000043, 0x0004003d, 0x00000006, 0x00000045, 0x00000027, 0x0004003d,
    0x00000006, 0x00000046, 0x0000002f, 0x00050084, 0x00000006, 0x00000047, 0x0000003c, 0x00000046, 0x00050080, 0x00000006,
    0x00000049, 0x00000047, 0x00000048, 0x00060041, 0x00000016, 0x0000004a, 0x00000013, 0x0000003b, 0x00000049, 0x0004003d,
    0x00000006, 0x0000004b, 0x0000004a, 0x000500aa, 0x00000019, 0x0000004c, 0x00000045, 0x0000004b, 0x000200f9, 0x00000044,
    0x000200f8, 0x00000044, 0x000700f5, 0x00000019, 0x0000004d, 0x00000042, 0x00000031, 0x0000004c, 0x00000043, 0x000300f7,
    0x0000004f, 0x00000000, 0x000400fa, 0x0000004d, 0x0000004e, 0x0000004f, 0x000200f8, 0x0000004e, 0x0003003e, 0x0000002d,
    0x00000050, 0x000200f9, 0x00000032, 0x000200f8, 0x0000004f, 0x000200f9, 0x00000033, 0x000200f8, 0x00000033, 0x0004003d,
    0x00000006, 0x00000052, 0x0000002f, 0x00050080, 0x00000006, 0x00000053, 0x00000052, 0x00000024, 0x0003003e, 0x0000002f,
    0x00000053, 0x000200f9, 0x00000030, 0x000200f8, 0x00000032, 0x0004003d, 0x00000019, 0x00000054, 0x0000002d, 0x000400a8,
    0x00000019, 0x00000055, 0x00000054, 0x000300f7, 0x00000057, 0x00000000, 0x000400fa, 0x00000055, 0x00000056, 0x00000057,
    0x000200f8, 0x00000056, 0x00050041, 0x00000016, 0x00000059, 0x00000013, 0x00000058, 0x0004003d, 0x00000006, 0x0000005a,
    0x00000059, 0x00050080, 0x00000006, 0x0000005b, 0x0000005a, 0x00000048, 0x00050041, 0x00000016, 0x0000005c, 0x00000013,
    0x00000058, 0x0003003e, 0x0000005c, 0x0000005b, 0x0004003d, 0x00000006, 0x0000005e, 0x0000001b, 0x00050041, 0x00000016,
    0x0000005f, 0x00000013, 0x0000005d, 0x0003003e, 0x0000005f, 0x0000005e, 0x0004003d, 0x00000006, 0x00000061, 0x00000027,
    0x00050041, 0x00000016, 0x00000062, 0x00000013, 0x00000060, 0x0003003e, 0x00000062, 0x00000061, 0x0004003d, 0x00000006,
    0x00000063, 0x00000008, 0x00050041, 0x00000016, 0x00000064, 0x00000013, 0x00000024, 0x0004003d, 0x00000006, 0x00000065,
    0x00000064, 0x00070041, 0x00000016, 0x00000066, 0x00000022, 0x00000015, 0x00000063, 0x00000024, 0x0003003e, 0x00000066,
    0x00000065, 0x0004003d, 0x00000006, 0x00000067, 0x00000008, 0x00050041, 0x00000016, 0x00000068, 0x00000013, 0x00000029,
    0x0004003d, 0x00000006, 0x00000069, 0x00000068, 0x00070041, 0x00000016, 0x0000006a, 0x00000022, 0x00000015, 0x00000067,
    0x00000029, 0x0003003e, 0x0000006a, 0x00000069, 0x000200f9, 0x00000057, 0x000200f8, 0x00000057, 0x000200f9, 0x0000000d,
    0x000200f8, 0x0000000d, 0x0004003d, 0x00000006, 0x0000006b, 0x00000008, 0x00050080, 0x00000006, 0x0000006c, 0x0000006b,
    0x00000024, 0x0003003e, 0x00000008, 0x0000006c, 0x000200f9, 0x0000000a, 0x000200f8, 0x0000000c, 0x000100fd, 0x00010038};

// Convenience function for reporting problems with setting up GPU Validation.
template <typename T>
void GpuAssisted::ReportSetupProblem(T object, const char *const specific_message) const {
    LogError(object, "UNASSIGNED-GPU-Assisted Validation Error. ", "Detail: (%s)", specific_message);
}

bool GpuAssisted::CheckForDescriptorIndexing(DeviceFeatures enabled_features) const {
    bool result =
        (IsExtEnabled(device_extensions.vk_ext_descriptor_indexing) &&
         (enabled_features.core12.descriptorIndexing || enabled_features.core12.shaderInputAttachmentArrayDynamicIndexing ||
          enabled_features.core12.shaderUniformTexelBufferArrayDynamicIndexing ||
          enabled_features.core12.shaderStorageTexelBufferArrayDynamicIndexing ||
          enabled_features.core12.shaderUniformBufferArrayNonUniformIndexing ||
          enabled_features.core12.shaderSampledImageArrayNonUniformIndexing ||
          enabled_features.core12.shaderStorageBufferArrayNonUniformIndexing ||
          enabled_features.core12.shaderStorageImageArrayNonUniformIndexing ||
          enabled_features.core12.shaderInputAttachmentArrayNonUniformIndexing ||
          enabled_features.core12.shaderUniformTexelBufferArrayNonUniformIndexing ||
          enabled_features.core12.shaderStorageTexelBufferArrayNonUniformIndexing ||
          enabled_features.core12.descriptorBindingUniformBufferUpdateAfterBind ||
          enabled_features.core12.descriptorBindingSampledImageUpdateAfterBind ||
          enabled_features.core12.descriptorBindingStorageImageUpdateAfterBind ||
          enabled_features.core12.descriptorBindingStorageBufferUpdateAfterBind ||
          enabled_features.core12.descriptorBindingUniformTexelBufferUpdateAfterBind ||
          enabled_features.core12.descriptorBindingStorageTexelBufferUpdateAfterBind ||
          enabled_features.core12.descriptorBindingUpdateUnusedWhilePending ||
          enabled_features.core12.descriptorBindingPartiallyBound ||
          enabled_features.core12.descriptorBindingVariableDescriptorCount || enabled_features.core12.runtimeDescriptorArray));
    return result;
}

void GpuAssisted::PreCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer, void *cb_state_data) {
    // Ray tracing acceleration structure instance buffers also need the storage buffer usage as
    // acceleration structure build validation will find and replace invalid acceleration structure
    // handles inside of a compute shader.
    create_buffer_api_state *cb_state = reinterpret_cast<create_buffer_api_state *>(cb_state_data);
    if (cb_state && cb_state->modified_create_info.usage & VK_BUFFER_USAGE_RAY_TRACING_BIT_NV) {
        cb_state->modified_create_info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }
}

// Turn on necessary device features.
void GpuAssisted::PreCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo *create_info,
                                            const VkAllocationCallbacks *pAllocator, VkDevice *pDevice,
                                            safe_VkDeviceCreateInfo *modified_create_info) {
    DispatchGetPhysicalDeviceFeatures(gpu, &supported_features);
    VkPhysicalDeviceFeatures features = {};
    features.vertexPipelineStoresAndAtomics = true;
    features.fragmentStoresAndAtomics = true;
    features.shaderInt64 = true;
    UtilPreCallRecordCreateDevice(gpu, modified_create_info, supported_features, features);
}
// Perform initializations that can be done at Create Device time.
void GpuAssisted::PostCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator, VkDevice *pDevice, VkResult result) {
    // The state tracker sets up the device state
    ValidationStateTracker::PostCallRecordCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice, result);

    ValidationObject *device_object = GetLayerDataPtr(get_dispatch_key(*pDevice), layer_data_map);
    ValidationObject *validation_data = GetValidationObject(device_object->object_dispatch, this->container_type);
    GpuAssisted *device_gpu_assisted = static_cast<GpuAssisted *>(validation_data);

    if (device_gpu_assisted->phys_dev_props.apiVersion < VK_API_VERSION_1_1) {
        ReportSetupProblem(device, "GPU-Assisted validation requires Vulkan 1.1 or later.  GPU-Assisted Validation disabled.");
        device_gpu_assisted->aborted = true;
        return;
    }

    if (!supported_features.fragmentStoresAndAtomics || !supported_features.vertexPipelineStoresAndAtomics) {
        ReportSetupProblem(device,
                           "GPU-Assisted validation requires fragmentStoresAndAtomics and vertexPipelineStoresAndAtomics.  "
                           "GPU-Assisted Validation disabled.");
        device_gpu_assisted->aborted = true;
        return;
    }

    if ((device_extensions.vk_ext_buffer_device_address || device_extensions.vk_khr_buffer_device_address) &&
        !supported_features.shaderInt64) {
        LogWarning(device, "UNASSIGNED-GPU-Assisted Validation Warning",
                   "shaderInt64 feature is not available.  No buffer device address checking will be attempted");
    }
    device_gpu_assisted->shaderInt64 = supported_features.shaderInt64;
    device_gpu_assisted->physicalDevice = physicalDevice;
    device_gpu_assisted->device = *pDevice;
    device_gpu_assisted->output_buffer_size = sizeof(uint32_t) * (spvtools::kInstMaxOutCnt + 1);
    device_gpu_assisted->descriptor_indexing = CheckForDescriptorIndexing(device_gpu_assisted->enabled_features);
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
                                            VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT |
                                                VK_SHADER_STAGE_MESH_BIT_NV | VK_SHADER_STAGE_TASK_BIT_NV |
                                                kShaderStageAllRayTracing,
                                            NULL};
    bindings.push_back(binding);
    for (auto i = 1; i < 3; i++) {
        binding.binding = i;
        bindings.push_back(binding);
    }
    UtilPostCallRecordCreateDevice(pCreateInfo, bindings, device_gpu_assisted, device_gpu_assisted->phys_dev_props);
    CreateAccelerationStructureBuildValidationState(device_gpu_assisted);
}

void GpuAssisted::PostCallRecordGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfoEXT *pInfo,
                                                       VkDeviceAddress address) {
    BUFFER_STATE *buffer_state = GetBufferState(pInfo->buffer);
    // Validate against the size requested when the buffer was created
    if (buffer_state) {
        buffer_map[address] = buffer_state->createInfo.size;
        buffer_state->deviceAddress = address;
    }
}

void GpuAssisted::PostCallRecordGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfoEXT *pInfo,
                                                          VkDeviceAddress address) {
    PostCallRecordGetBufferDeviceAddress(device, pInfo, address);
}

void GpuAssisted::PostCallRecordGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfoEXT *pInfo,
                                                          VkDeviceAddress address) {
    PostCallRecordGetBufferDeviceAddress(device, pInfo, address);
}

void GpuAssisted::PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator) {
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    if (buffer_state) buffer_map.erase(buffer_state->deviceAddress);
    ValidationStateTracker::PreCallRecordDestroyBuffer(device, buffer, pAllocator);
}

// Clean up device-related resources
void GpuAssisted::PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator) {
    DestroyAccelerationStructureBuildValidationState();
    UtilPreCallRecordDestroyDevice(this);
    ValidationStateTracker::PreCallRecordDestroyDevice(device, pAllocator);
    // State Tracker can end up making vma calls through callbacks - don't destroy allocator until ST is done
    if (vmaAllocator) {
        vmaDestroyAllocator(vmaAllocator);
    }
    desc_set_manager.reset();
}

void GpuAssisted::CreateAccelerationStructureBuildValidationState(GpuAssisted *device_gpuav) {
    if (device_gpuav->aborted) {
        return;
    }

    auto &as_validation_state = device_gpuav->acceleration_structure_validation_state;
    if (as_validation_state.initialized) {
        return;
    }

    if (!device_extensions.vk_nv_ray_tracing) {
        return;
    }

    // Outline:
    //   - Create valid bottom level acceleration structure which acts as replacement
    //      - Create and load vertex buffer
    //      - Create and load index buffer
    //      - Create, allocate memory for, and bind memory for acceleration structure
    //      - Query acceleration structure handle
    //      - Create command pool and command buffer
    //      - Record build acceleration structure command
    //      - Submit command buffer and wait for completion
    //      - Cleanup
    //  - Create compute pipeline for validating instance buffers
    //      - Create descriptor set layout
    //      - Create pipeline layout
    //      - Create pipeline
    //      - Cleanup

    VkResult result = VK_SUCCESS;

    VkBuffer vbo = VK_NULL_HANDLE;
    VmaAllocation vbo_allocation = VK_NULL_HANDLE;
    if (result == VK_SUCCESS) {
        VkBufferCreateInfo vbo_ci = {};
        vbo_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vbo_ci.size = sizeof(float) * 9;
        vbo_ci.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;

        VmaAllocationCreateInfo vbo_ai = {};
        vbo_ai.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        vbo_ai.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        result = vmaCreateBuffer(device_gpuav->vmaAllocator, &vbo_ci, &vbo_ai, &vbo, &vbo_allocation, nullptr);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Failed to create vertex buffer for acceleration structure build validation.");
        }
    }

    if (result == VK_SUCCESS) {
        uint8_t *mapped_vbo_buffer = nullptr;
        result = vmaMapMemory(device_gpuav->vmaAllocator, vbo_allocation, (void **)&mapped_vbo_buffer);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Failed to map vertex buffer for acceleration structure build validation.");
        } else {
            const std::vector<float> vertices = {1.0f, 0.0f, 0.0f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f};
            std::memcpy(mapped_vbo_buffer, (uint8_t *)vertices.data(), sizeof(float) * vertices.size());
            vmaUnmapMemory(device_gpuav->vmaAllocator, vbo_allocation);
        }
    }

    VkBuffer ibo = VK_NULL_HANDLE;
    VmaAllocation ibo_allocation = VK_NULL_HANDLE;
    if (result == VK_SUCCESS) {
        VkBufferCreateInfo ibo_ci = {};
        ibo_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        ibo_ci.size = sizeof(uint32_t) * 3;
        ibo_ci.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;

        VmaAllocationCreateInfo ibo_ai = {};
        ibo_ai.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        ibo_ai.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        result = vmaCreateBuffer(device_gpuav->vmaAllocator, &ibo_ci, &ibo_ai, &ibo, &ibo_allocation, nullptr);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Failed to create index buffer for acceleration structure build validation.");
        }
    }

    if (result == VK_SUCCESS) {
        uint8_t *mapped_ibo_buffer = nullptr;
        result = vmaMapMemory(device_gpuav->vmaAllocator, ibo_allocation, (void **)&mapped_ibo_buffer);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Failed to map index buffer for acceleration structure build validation.");
        } else {
            const std::vector<uint32_t> indicies = {0, 1, 2};
            std::memcpy(mapped_ibo_buffer, (uint8_t *)indicies.data(), sizeof(uint32_t) * indicies.size());
            vmaUnmapMemory(device_gpuav->vmaAllocator, ibo_allocation);
        }
    }

    VkGeometryNV geometry = {};
    geometry.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
    geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
    geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
    geometry.geometry.triangles.vertexData = vbo;
    geometry.geometry.triangles.vertexOffset = 0;
    geometry.geometry.triangles.vertexCount = 3;
    geometry.geometry.triangles.vertexStride = 12;
    geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    geometry.geometry.triangles.indexData = ibo;
    geometry.geometry.triangles.indexOffset = 0;
    geometry.geometry.triangles.indexCount = 3;
    geometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
    geometry.geometry.triangles.transformData = VK_NULL_HANDLE;
    geometry.geometry.triangles.transformOffset = 0;
    geometry.geometry.aabbs = {};
    geometry.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;

    VkAccelerationStructureCreateInfoNV as_ci = {};
    as_ci.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
    as_ci.info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
    as_ci.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    as_ci.info.instanceCount = 0;
    as_ci.info.geometryCount = 1;
    as_ci.info.pGeometries = &geometry;
    if (result == VK_SUCCESS) {
        result = DispatchCreateAccelerationStructureNV(device_gpuav->device, &as_ci, nullptr, &as_validation_state.replacement_as);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device_gpuav->device,
                               "Failed to create acceleration structure for acceleration structure build validation.");
        }
    }

    VkMemoryRequirements2 as_mem_requirements = {};
    if (result == VK_SUCCESS) {
        VkAccelerationStructureMemoryRequirementsInfoNV as_mem_requirements_info = {};
        as_mem_requirements_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
        as_mem_requirements_info.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
        as_mem_requirements_info.accelerationStructure = as_validation_state.replacement_as;

        DispatchGetAccelerationStructureMemoryRequirementsNV(device_gpuav->device, &as_mem_requirements_info, &as_mem_requirements);
    }

    VmaAllocationInfo as_memory_ai = {};
    if (result == VK_SUCCESS) {
        VmaAllocationCreateInfo as_memory_aci = {};
        as_memory_aci.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        result = vmaAllocateMemory(device_gpuav->vmaAllocator, &as_mem_requirements.memoryRequirements, &as_memory_aci,
                                   &as_validation_state.replacement_as_allocation, &as_memory_ai);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device_gpuav->device,
                               "Failed to alloc acceleration structure memory for acceleration structure build validation.");
        }
    }

    if (result == VK_SUCCESS) {
        VkBindAccelerationStructureMemoryInfoNV as_bind_info = {};
        as_bind_info.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
        as_bind_info.accelerationStructure = as_validation_state.replacement_as;
        as_bind_info.memory = as_memory_ai.deviceMemory;
        as_bind_info.memoryOffset = as_memory_ai.offset;

        result = DispatchBindAccelerationStructureMemoryNV(device_gpuav->device, 1, &as_bind_info);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device_gpuav->device,
                               "Failed to bind acceleration structure memory for acceleration structure build validation.");
        }
    }

    if (result == VK_SUCCESS) {
        result = DispatchGetAccelerationStructureHandleNV(device_gpuav->device, as_validation_state.replacement_as,
                                                          sizeof(uint64_t), &as_validation_state.replacement_as_handle);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device_gpuav->device,
                               "Failed to get acceleration structure handle for acceleration structure build validation.");
        }
    }

    VkMemoryRequirements2 scratch_mem_requirements = {};
    if (result == VK_SUCCESS) {
        VkAccelerationStructureMemoryRequirementsInfoNV scratch_mem_requirements_info = {};
        scratch_mem_requirements_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
        scratch_mem_requirements_info.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
        scratch_mem_requirements_info.accelerationStructure = as_validation_state.replacement_as;

        DispatchGetAccelerationStructureMemoryRequirementsNV(device_gpuav->device, &scratch_mem_requirements_info,
                                                             &scratch_mem_requirements);
    }

    VkBuffer scratch = VK_NULL_HANDLE;
    VmaAllocation scratch_allocation = {};
    if (result == VK_SUCCESS) {
        VkBufferCreateInfo scratch_ci = {};
        scratch_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        scratch_ci.size = scratch_mem_requirements.memoryRequirements.size;
        scratch_ci.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
        VmaAllocationCreateInfo scratch_aci = {};
        scratch_aci.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        result = vmaCreateBuffer(device_gpuav->vmaAllocator, &scratch_ci, &scratch_aci, &scratch, &scratch_allocation, nullptr);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device_gpuav->device,
                               "Failed to create scratch buffer for acceleration structure build validation.");
        }
    }

    VkCommandPool command_pool = VK_NULL_HANDLE;
    if (result == VK_SUCCESS) {
        VkCommandPoolCreateInfo command_pool_ci = {};
        command_pool_ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_ci.queueFamilyIndex = 0;

        result = DispatchCreateCommandPool(device_gpuav->device, &command_pool_ci, nullptr, &command_pool);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device_gpuav->device, "Failed to create command pool for acceleration structure build validation.");
        }
    }

    VkCommandBuffer command_buffer = VK_NULL_HANDLE;

    if (result == VK_SUCCESS) {
        VkCommandBufferAllocateInfo command_buffer_ai = {};
        command_buffer_ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_ai.commandPool = command_pool;
        command_buffer_ai.commandBufferCount = 1;
        command_buffer_ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        result = DispatchAllocateCommandBuffers(device_gpuav->device, &command_buffer_ai, &command_buffer);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device_gpuav->device,
                               "Failed to create command buffer for acceleration structure build validation.");
        }

        // Hook up command buffer dispatch
        device_gpuav->vkSetDeviceLoaderData(device_gpuav->device, command_buffer);
    }

    if (result == VK_SUCCESS) {
        VkCommandBufferBeginInfo command_buffer_bi = {};
        command_buffer_bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        result = DispatchBeginCommandBuffer(command_buffer, &command_buffer_bi);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device_gpuav->device, "Failed to begin command buffer for acceleration structure build validation.");
        }
    }

    if (result == VK_SUCCESS) {
        DispatchCmdBuildAccelerationStructureNV(command_buffer, &as_ci.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                                as_validation_state.replacement_as, VK_NULL_HANDLE, scratch, 0);
        DispatchEndCommandBuffer(command_buffer);
    }

    VkQueue queue = VK_NULL_HANDLE;
    if (result == VK_SUCCESS) {
        DispatchGetDeviceQueue(device_gpuav->device, 0, 0, &queue);

        // Hook up queue dispatch
        device_gpuav->vkSetDeviceLoaderData(device_gpuav->device, queue);

        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;
        result = DispatchQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device_gpuav->device,
                               "Failed to submit command buffer for acceleration structure build validation.");
        }
    }

    if (result == VK_SUCCESS) {
        result = DispatchQueueWaitIdle(queue);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device_gpuav->device, "Failed to wait for queue idle for acceleration structure build validation.");
        }
    }

    if (vbo != VK_NULL_HANDLE) {
        vmaDestroyBuffer(device_gpuav->vmaAllocator, vbo, vbo_allocation);
    }
    if (ibo != VK_NULL_HANDLE) {
        vmaDestroyBuffer(device_gpuav->vmaAllocator, ibo, ibo_allocation);
    }
    if (scratch != VK_NULL_HANDLE) {
        vmaDestroyBuffer(device_gpuav->vmaAllocator, scratch, scratch_allocation);
    }
    if (command_pool != VK_NULL_HANDLE) {
        DispatchDestroyCommandPool(device_gpuav->device, command_pool, nullptr);
    }

    if (device_gpuav->debug_desc_layout == VK_NULL_HANDLE) {
        ReportSetupProblem(device_gpuav->device,
                           "Failed to find descriptor set layout for acceleration structure build validation.");
        result = VK_INCOMPLETE;
    }

    if (result == VK_SUCCESS) {
        VkPipelineLayoutCreateInfo pipeline_layout_ci = {};
        pipeline_layout_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_ci.setLayoutCount = 1;
        pipeline_layout_ci.pSetLayouts = &device_gpuav->debug_desc_layout;
        result = DispatchCreatePipelineLayout(device_gpuav->device, &pipeline_layout_ci, 0, &as_validation_state.pipeline_layout);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device_gpuav->device,
                               "Failed to create pipeline layout for acceleration structure build validation.");
        }
    }

    VkShaderModule shader_module = VK_NULL_HANDLE;
    if (result == VK_SUCCESS) {
        VkShaderModuleCreateInfo shader_module_ci = {};
        shader_module_ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shader_module_ci.codeSize = sizeof(kComputeShaderSpirv);
        shader_module_ci.pCode = (uint32_t *)kComputeShaderSpirv;

        result = DispatchCreateShaderModule(device_gpuav->device, &shader_module_ci, nullptr, &shader_module);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device_gpuav->device,
                               "Failed to create compute shader module for acceleration structure build validation.");
        }
    }

    if (result == VK_SUCCESS) {
        VkPipelineShaderStageCreateInfo pipeline_stage_ci = {};
        pipeline_stage_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipeline_stage_ci.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        pipeline_stage_ci.module = shader_module;
        pipeline_stage_ci.pName = "main";

        VkComputePipelineCreateInfo pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipeline_ci.stage = pipeline_stage_ci;
        pipeline_ci.layout = as_validation_state.pipeline_layout;

        result = DispatchCreateComputePipelines(device_gpuav->device, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr,
                                                &as_validation_state.pipeline);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device_gpuav->device,
                               "Failed to create compute pipeline for acceleration structure build validation.");
        }
    }

    if (shader_module != VK_NULL_HANDLE) {
        DispatchDestroyShaderModule(device_gpuav->device, shader_module, nullptr);
    }

    if (result == VK_SUCCESS) {
        as_validation_state.initialized = true;
        LogInfo(device_gpuav->device, "UNASSIGNED-GPU-Assisted Validation.",
                "Acceleration Structure Building GPU Validation Enabled.");
    } else {
        device_gpuav->aborted = true;
    }
}

void GpuAssisted::DestroyAccelerationStructureBuildValidationState() {
    auto &as_validation_state = acceleration_structure_validation_state;
    if (as_validation_state.pipeline != VK_NULL_HANDLE) {
        DispatchDestroyPipeline(device, as_validation_state.pipeline, nullptr);
    }
    if (as_validation_state.pipeline_layout != VK_NULL_HANDLE) {
        DispatchDestroyPipelineLayout(device, as_validation_state.pipeline_layout, nullptr);
    }
    if (as_validation_state.replacement_as != VK_NULL_HANDLE) {
        DispatchDestroyAccelerationStructureNV(device, as_validation_state.replacement_as, nullptr);
    }
    if (as_validation_state.replacement_as_allocation != VK_NULL_HANDLE) {
        vmaFreeMemory(vmaAllocator, as_validation_state.replacement_as_allocation);
    }
}

struct GPUAV_RESTORABLE_PIPELINE_STATE {
    VkPipelineBindPoint pipeline_bind_point = VK_PIPELINE_BIND_POINT_MAX_ENUM;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptor_sets;
    std::vector<std::vector<uint32_t>> dynamic_offsets;
    uint32_t push_descriptor_set_index = 0;
    std::vector<safe_VkWriteDescriptorSet> push_descriptor_set_writes;
    std::vector<uint8_t> push_constants_data;
    PushConstantRangesId push_constants_ranges;

    void Create(CMD_BUFFER_STATE *cb_state, VkPipelineBindPoint bind_point) {
        pipeline_bind_point = bind_point;

        LAST_BOUND_STATE &last_bound = cb_state->lastBound[bind_point];
        if (last_bound.pipeline_state) {
            pipeline = last_bound.pipeline_state->pipeline;
            pipeline_layout = last_bound.pipeline_layout;
            descriptor_sets.reserve(last_bound.per_set.size());
            for (std::size_t i = 0; i < last_bound.per_set.size(); i++) {
                const auto *bound_descriptor_set = last_bound.per_set[i].bound_descriptor_set;

                descriptor_sets.push_back(bound_descriptor_set->GetSet());
                if (bound_descriptor_set->IsPushDescriptor()) {
                    push_descriptor_set_index = static_cast<uint32_t>(i);
                }
                dynamic_offsets.push_back(last_bound.per_set[i].dynamicOffsets);
            }

            if (last_bound.push_descriptor_set) {
                push_descriptor_set_writes = last_bound.push_descriptor_set->GetWrites();
            }
            if (last_bound.pipeline_state->pipeline_layout->push_constant_ranges == cb_state->push_constant_data_ranges) {
                push_constants_data = cb_state->push_constant_data;
                push_constants_ranges = last_bound.pipeline_state->pipeline_layout->push_constant_ranges;
            }
        }
    }

    void Restore(VkCommandBuffer command_buffer) const {
        if (pipeline != VK_NULL_HANDLE) {
            DispatchCmdBindPipeline(command_buffer, pipeline_bind_point, pipeline);
            if (!descriptor_sets.empty()) {
                for (std::size_t i = 0; i < descriptor_sets.size(); i++) {
                    VkDescriptorSet descriptor_set = descriptor_sets[i];
                    if (descriptor_set != VK_NULL_HANDLE) {
                        DispatchCmdBindDescriptorSets(command_buffer, pipeline_bind_point, pipeline_layout,
                                                      static_cast<uint32_t>(i), 1, &descriptor_set,
                                                      static_cast<uint32_t>(dynamic_offsets[i].size()), dynamic_offsets[i].data());
                    }
                }
            }
            if (!push_descriptor_set_writes.empty()) {
                DispatchCmdPushDescriptorSetKHR(command_buffer, pipeline_bind_point, pipeline_layout, push_descriptor_set_index,
                                                static_cast<uint32_t>(push_descriptor_set_writes.size()),
                                                reinterpret_cast<const VkWriteDescriptorSet *>(push_descriptor_set_writes.data()));
            }
            for (const auto &push_constant_range : *push_constants_ranges) {
                if (push_constant_range.size == 0) continue;
                DispatchCmdPushConstants(command_buffer, pipeline_layout, push_constant_range.stageFlags,
                                         push_constant_range.offset, push_constant_range.size, push_constants_data.data());
            }
        }
    }
};

void GpuAssisted::PreCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                               const VkAccelerationStructureInfoNV *pInfo, VkBuffer instanceData,
                                                               VkDeviceSize instanceOffset, VkBool32 update,
                                                               VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                               VkBuffer scratch, VkDeviceSize scratchOffset) {
    if (pInfo == nullptr || pInfo->type != VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV) {
        return;
    }

    auto &as_validation_state = acceleration_structure_validation_state;
    if (!as_validation_state.initialized) {
        return;
    }

    // Empty acceleration structure is valid according to the spec.
    if (pInfo->instanceCount == 0 || instanceData == VK_NULL_HANDLE) {
        return;
    }

    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    assert(cb_state != nullptr);

    std::vector<uint64_t> current_valid_handles;
    for (const auto &as_state_kv : accelerationStructureMap) {
        const ACCELERATION_STRUCTURE_STATE &as_state = *as_state_kv.second;
        if (as_state.built && as_state.create_infoNV.info.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV) {
            current_valid_handles.push_back(as_state.opaque_handle);
        }
    }

    GpuAssistedAccelerationStructureBuildValidationBufferInfo as_validation_buffer_info = {};
    as_validation_buffer_info.acceleration_structure = dst;

    const VkDeviceSize validation_buffer_size =
        // One uint for number of instances to validate
        4 +
        // Two uint for the replacement acceleration structure handle
        8 +
        // One uint for number of invalid handles found
        4 +
        // Two uint for the first invalid handle found
        8 +
        // One uint for the number of current valid handles
        4 +
        // Two uint for each current valid handle
        (8 * current_valid_handles.size());

    VkBufferCreateInfo validation_buffer_create_info = {};
    validation_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    validation_buffer_create_info.size = validation_buffer_size;
    validation_buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    VmaAllocationCreateInfo validation_buffer_alloc_info = {};
    validation_buffer_alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

    VkResult result = vmaCreateBuffer(vmaAllocator, &validation_buffer_create_info, &validation_buffer_alloc_info,
                                      &as_validation_buffer_info.validation_buffer,
                                      &as_validation_buffer_info.validation_buffer_allocation, nullptr);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, "Unable to allocate device memory.  Device could become unstable.");
        aborted = true;
        return;
    }

    GpuAccelerationStructureBuildValidationBuffer *mapped_validation_buffer = nullptr;
    result = vmaMapMemory(vmaAllocator, as_validation_buffer_info.validation_buffer_allocation, (void **)&mapped_validation_buffer);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, "Unable to allocate device memory for acceleration structure build val buffer.");
        aborted = true;
        return;
    }

    mapped_validation_buffer->instances_to_validate = pInfo->instanceCount;
    mapped_validation_buffer->replacement_handle_bits_0 =
        reinterpret_cast<const uint32_t *>(&as_validation_state.replacement_as_handle)[0];
    mapped_validation_buffer->replacement_handle_bits_1 =
        reinterpret_cast<const uint32_t *>(&as_validation_state.replacement_as_handle)[1];
    mapped_validation_buffer->invalid_handle_found = 0;
    mapped_validation_buffer->invalid_handle_bits_0 = 0;
    mapped_validation_buffer->invalid_handle_bits_1 = 0;
    mapped_validation_buffer->valid_handles_count = static_cast<uint32_t>(current_valid_handles.size());

    uint32_t *mapped_valid_handles = reinterpret_cast<uint32_t *>(&mapped_validation_buffer[1]);
    for (std::size_t i = 0; i < current_valid_handles.size(); i++) {
        const uint64_t current_valid_handle = current_valid_handles[i];

        *mapped_valid_handles = reinterpret_cast<const uint32_t *>(&current_valid_handle)[0];
        ++mapped_valid_handles;
        *mapped_valid_handles = reinterpret_cast<const uint32_t *>(&current_valid_handle)[1];
        ++mapped_valid_handles;
    }

    vmaUnmapMemory(vmaAllocator, as_validation_buffer_info.validation_buffer_allocation);

    static constexpr const VkDeviceSize kInstanceSize = 64;
    const VkDeviceSize instance_buffer_size = kInstanceSize * pInfo->instanceCount;

    result = desc_set_manager->GetDescriptorSet(&as_validation_buffer_info.descriptor_pool, debug_desc_layout,
                                                &as_validation_buffer_info.descriptor_set);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, "Unable to get descriptor set for acceleration structure build.");
        aborted = true;
        return;
    }

    VkDescriptorBufferInfo descriptor_buffer_infos[2] = {};
    descriptor_buffer_infos[0].buffer = instanceData;
    descriptor_buffer_infos[0].offset = instanceOffset;
    descriptor_buffer_infos[0].range = instance_buffer_size;
    descriptor_buffer_infos[1].buffer = as_validation_buffer_info.validation_buffer;
    descriptor_buffer_infos[1].offset = 0;
    descriptor_buffer_infos[1].range = validation_buffer_size;

    VkWriteDescriptorSet descriptor_set_writes[2] = {};
    descriptor_set_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_set_writes[0].dstSet = as_validation_buffer_info.descriptor_set;
    descriptor_set_writes[0].dstBinding = 0;
    descriptor_set_writes[0].descriptorCount = 1;
    descriptor_set_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_set_writes[0].pBufferInfo = &descriptor_buffer_infos[0];
    descriptor_set_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_set_writes[1].dstSet = as_validation_buffer_info.descriptor_set;
    descriptor_set_writes[1].dstBinding = 1;
    descriptor_set_writes[1].descriptorCount = 1;
    descriptor_set_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_set_writes[1].pBufferInfo = &descriptor_buffer_infos[1];

    DispatchUpdateDescriptorSets(device, 2, descriptor_set_writes, 0, nullptr);

    // Issue a memory barrier to make sure anything writing to the instance buffer has finished.
    VkMemoryBarrier memory_barrier = {};
    memory_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memory_barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
    memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    DispatchCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1,
                               &memory_barrier, 0, nullptr, 0, nullptr);

    // Save a copy of the compute pipeline state that needs to be restored.
    GPUAV_RESTORABLE_PIPELINE_STATE restorable_state;
    restorable_state.Create(cb_state, VK_PIPELINE_BIND_POINT_COMPUTE);

    // Switch to and launch the validation compute shader to find, replace, and report invalid acceleration structure handles.
    DispatchCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, as_validation_state.pipeline);
    DispatchCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, as_validation_state.pipeline_layout, 0, 1,
                                  &as_validation_buffer_info.descriptor_set, 0, nullptr);
    DispatchCmdDispatch(commandBuffer, 1, 1, 1);

    // Issue a buffer memory barrier to make sure that any invalid bottom level acceleration structure handles
    // have been replaced by the validation compute shader before any builds take place.
    VkBufferMemoryBarrier instance_buffer_barrier = {};
    instance_buffer_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    instance_buffer_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    instance_buffer_barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
    instance_buffer_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    instance_buffer_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    instance_buffer_barrier.buffer = instanceData;
    instance_buffer_barrier.offset = instanceOffset;
    instance_buffer_barrier.size = instance_buffer_size;
    DispatchCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                               VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 0, nullptr, 1, &instance_buffer_barrier, 0,
                               nullptr);

    // Restore the previous compute pipeline state.
    restorable_state.Restore(commandBuffer);

    as_validation_state.validation_buffers[commandBuffer].push_back(std::move(as_validation_buffer_info));
}

void GpuAssisted::ProcessAccelerationStructureBuildValidationBuffer(VkQueue queue, CMD_BUFFER_STATE *cb_node) {
    if (cb_node == nullptr || !cb_node->hasBuildAccelerationStructureCmd) {
        return;
    }

    auto &as_validation_info = acceleration_structure_validation_state;
    auto &as_validation_buffer_infos = as_validation_info.validation_buffers[cb_node->commandBuffer];
    for (const auto &as_validation_buffer_info : as_validation_buffer_infos) {
        GpuAccelerationStructureBuildValidationBuffer *mapped_validation_buffer = nullptr;

        VkResult result =
            vmaMapMemory(vmaAllocator, as_validation_buffer_info.validation_buffer_allocation, (void **)&mapped_validation_buffer);
        if (result == VK_SUCCESS) {
            if (mapped_validation_buffer->invalid_handle_found > 0) {
                uint64_t invalid_handle = 0;
                reinterpret_cast<uint32_t *>(&invalid_handle)[0] = mapped_validation_buffer->invalid_handle_bits_0;
                reinterpret_cast<uint32_t *>(&invalid_handle)[1] = mapped_validation_buffer->invalid_handle_bits_1;

                LogError(as_validation_buffer_info.acceleration_structure, "UNASSIGNED-AccelerationStructure",
                         "Attempted to build top level acceleration structure using invalid bottom level acceleration structure "
                         "handle (%" PRIu64 ")",
                         invalid_handle);
            }
            vmaUnmapMemory(vmaAllocator, as_validation_buffer_info.validation_buffer_allocation);
        }
    }
}

void GpuAssisted::PostCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                                  const VkBindAccelerationStructureMemoryInfoNV *pBindInfos,
                                                                  VkResult result) {
    if (VK_SUCCESS != result) return;
    ValidationStateTracker::PostCallRecordBindAccelerationStructureMemoryNV(device, bindInfoCount, pBindInfos, result);
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        const VkBindAccelerationStructureMemoryInfoNV &info = pBindInfos[i];
        ACCELERATION_STRUCTURE_STATE *as_state = GetAccelerationStructureState(info.accelerationStructure);
        if (as_state) {
            DispatchGetAccelerationStructureHandleNV(device, info.accelerationStructure, 8, &as_state->opaque_handle);
        }
    }
}

// Modify the pipeline layout to include our debug descriptor set and any needed padding with the dummy descriptor set.
void GpuAssisted::PreCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout,
                                                    void *cpl_state_data) {
    if (aborted) {
        return;
    }

    create_pipeline_layout_api_state *cpl_state = reinterpret_cast<create_pipeline_layout_api_state *>(cpl_state_data);

    if (cpl_state->modified_create_info.setLayoutCount >= adjusted_max_desc_sets) {
        std::ostringstream strm;
        strm << "Pipeline Layout conflict with validation's descriptor set at slot " << desc_set_bind_index << ". "
             << "Application has too many descriptor sets in the pipeline layout to continue with gpu validation. "
             << "Validation is not modifying the pipeline layout. "
             << "Instrumented shaders are replaced with non-instrumented shaders.";
        ReportSetupProblem(device, strm.str().c_str());
    } else {
        UtilPreCallRecordCreatePipelineLayout(cpl_state, this, pCreateInfo);
    }
}

void GpuAssisted::PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout,
                                                     VkResult result) {
    ValidationStateTracker::PostCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, result);

    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, "Unable to create pipeline layout.  Device could become unstable.");
        aborted = true;
    }
}

// Free the device memory and descriptor set associated with a command buffer.
void GpuAssisted::ResetCommandBuffer(VkCommandBuffer commandBuffer) {
    if (aborted) {
        return;
    }
    auto gpuav_buffer_list = GetBufferInfo(commandBuffer);
    for (auto buffer_info : gpuav_buffer_list) {
        vmaDestroyBuffer(vmaAllocator, buffer_info.output_mem_block.buffer, buffer_info.output_mem_block.allocation);
        if (buffer_info.di_input_mem_block.buffer) {
            vmaDestroyBuffer(vmaAllocator, buffer_info.di_input_mem_block.buffer, buffer_info.di_input_mem_block.allocation);
        }
        if (buffer_info.bda_input_mem_block.buffer) {
            vmaDestroyBuffer(vmaAllocator, buffer_info.bda_input_mem_block.buffer, buffer_info.bda_input_mem_block.allocation);
        }
        if (buffer_info.desc_set != VK_NULL_HANDLE) {
            desc_set_manager->PutBackDescriptorSet(buffer_info.desc_pool, buffer_info.desc_set);
        }
    }
    command_buffer_map.erase(commandBuffer);

    auto &as_validation_info = acceleration_structure_validation_state;
    auto &as_validation_buffer_infos = as_validation_info.validation_buffers[commandBuffer];
    for (auto &as_validation_buffer_info : as_validation_buffer_infos) {
        vmaDestroyBuffer(vmaAllocator, as_validation_buffer_info.validation_buffer,
                         as_validation_buffer_info.validation_buffer_allocation);

        if (as_validation_buffer_info.descriptor_set != VK_NULL_HANDLE) {
            desc_set_manager->PutBackDescriptorSet(as_validation_buffer_info.descriptor_pool,
                                                   as_validation_buffer_info.descriptor_set);
        }
    }
    as_validation_info.validation_buffers.erase(commandBuffer);
}
// Just gives a warning about a possible deadlock.
bool GpuAssisted::PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                               VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                               uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                               uint32_t bufferMemoryBarrierCount,
                                               const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                                               const VkImageMemoryBarrier *pImageMemoryBarriers) const {
    if (srcStageMask & VK_PIPELINE_STAGE_HOST_BIT) {
        ReportSetupProblem(commandBuffer,
                           "CmdWaitEvents recorded with VK_PIPELINE_STAGE_HOST_BIT set. "
                           "GPU_Assisted validation waits on queue completion. "
                           "This wait could block the host's signaling of this event, resulting in deadlock.");
    }
    return false;
}

void GpuAssisted::PostCallRecordGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                                            VkPhysicalDeviceProperties *pPhysicalDeviceProperties) {
    // There is an implicit layer that can cause this call to return 0 for maxBoundDescriptorSets - Ignore such calls
    if (enabled[gpu_validation_reserve_binding_slot] && pPhysicalDeviceProperties->limits.maxBoundDescriptorSets > 0) {
        if (pPhysicalDeviceProperties->limits.maxBoundDescriptorSets > 1) {
            pPhysicalDeviceProperties->limits.maxBoundDescriptorSets -= 1;
        } else {
            LogWarning(physicalDevice, "UNASSIGNED-GPU-Assisted Validation Setup Error.",
                       "Unable to reserve descriptor binding slot on a device with only one slot.");
        }
    }
}

void GpuAssisted::PostCallRecordGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                                             VkPhysicalDeviceProperties2 *pPhysicalDeviceProperties2) {
    // There is an implicit layer that can cause this call to return 0 for maxBoundDescriptorSets - Ignore such calls
    if (enabled[gpu_validation_reserve_binding_slot] && pPhysicalDeviceProperties2->properties.limits.maxBoundDescriptorSets > 0) {
        if (pPhysicalDeviceProperties2->properties.limits.maxBoundDescriptorSets > 1) {
            pPhysicalDeviceProperties2->properties.limits.maxBoundDescriptorSets -= 1;
        } else {
            LogWarning(physicalDevice, "UNASSIGNED-GPU-Assisted Validation Setup Error.",
                       "Unable to reserve descriptor binding slot on a device with only one slot.");
        }
    }
}

void GpuAssisted::PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                       const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                       const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                       void *cgpl_state_data) {
    std::vector<safe_VkGraphicsPipelineCreateInfo> new_pipeline_create_infos;
    create_graphics_pipeline_api_state *cgpl_state = reinterpret_cast<create_graphics_pipeline_api_state *>(cgpl_state_data);
    UtilPreCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, cgpl_state->pipe_state,
                                       &new_pipeline_create_infos, VK_PIPELINE_BIND_POINT_GRAPHICS, this);
    cgpl_state->gpu_create_infos = new_pipeline_create_infos;
    cgpl_state->pCreateInfos = reinterpret_cast<VkGraphicsPipelineCreateInfo *>(cgpl_state->gpu_create_infos.data());
}

void GpuAssisted::PreCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                      const VkComputePipelineCreateInfo *pCreateInfos,
                                                      const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                      void *ccpl_state_data) {
    std::vector<safe_VkComputePipelineCreateInfo> new_pipeline_create_infos;
    auto *ccpl_state = reinterpret_cast<create_compute_pipeline_api_state *>(ccpl_state_data);
    UtilPreCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, ccpl_state->pipe_state,
                                       &new_pipeline_create_infos, VK_PIPELINE_BIND_POINT_COMPUTE, this);
    ccpl_state->gpu_create_infos = new_pipeline_create_infos;
    ccpl_state->pCreateInfos = reinterpret_cast<VkComputePipelineCreateInfo *>(ccpl_state->gpu_create_infos.data());
}

void GpuAssisted::PreCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                           const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                           const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                           void *crtpl_state_data) {
    std::vector<safe_VkRayTracingPipelineCreateInfoCommon> new_pipeline_create_infos;
    auto *crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_api_state *>(crtpl_state_data);
    UtilPreCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, crtpl_state->pipe_state,
                                       &new_pipeline_create_infos, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, this);
    crtpl_state->gpu_create_infos = new_pipeline_create_infos;
    crtpl_state->pCreateInfos = reinterpret_cast<VkRayTracingPipelineCreateInfoNV *>(crtpl_state->gpu_create_infos.data());
}

void GpuAssisted::PreCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                            const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                            const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                            void *crtpl_state_data) {
    std::vector<safe_VkRayTracingPipelineCreateInfoCommon> new_pipeline_create_infos;
    auto *crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_khr_api_state *>(crtpl_state_data);
    UtilPreCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, crtpl_state->pipe_state,
                                       &new_pipeline_create_infos, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, this);
    crtpl_state->gpu_create_infos = new_pipeline_create_infos;
    crtpl_state->pCreateInfos = reinterpret_cast<VkRayTracingPipelineCreateInfoKHR *>(crtpl_state->gpu_create_infos.data());
}

void GpuAssisted::PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                        const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                        const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                        VkResult result, void *cgpl_state_data) {
    ValidationStateTracker::PostCallRecordCreateGraphicsPipelines(device, pipelineCache, count, pCreateInfos, pAllocator,
                                                                  pPipelines, result, cgpl_state_data);
    create_graphics_pipeline_api_state *cgpl_state = reinterpret_cast<create_graphics_pipeline_api_state *>(cgpl_state_data);
    UtilCopyCreatePipelineFeedbackData(count, pCreateInfos, cgpl_state->gpu_create_infos.data());
    UtilPostCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, VK_PIPELINE_BIND_POINT_GRAPHICS, this);
}

void GpuAssisted::PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                       const VkComputePipelineCreateInfo *pCreateInfos,
                                                       const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                       VkResult result, void *ccpl_state_data) {
    ValidationStateTracker::PostCallRecordCreateComputePipelines(device, pipelineCache, count, pCreateInfos, pAllocator, pPipelines,
                                                                 result, ccpl_state_data);
    create_compute_pipeline_api_state *ccpl_state = reinterpret_cast<create_compute_pipeline_api_state *>(ccpl_state_data);
    UtilCopyCreatePipelineFeedbackData(count, pCreateInfos, ccpl_state->gpu_create_infos.data());
    UtilPostCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, VK_PIPELINE_BIND_POINT_COMPUTE, this);
}

void GpuAssisted::PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                            const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                            const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                            VkResult result, void *crtpl_state_data) {
    auto *crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_khr_api_state *>(crtpl_state_data);
    ValidationStateTracker::PostCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, count, pCreateInfos, pAllocator,
                                                                      pPipelines, result, crtpl_state_data);
    UtilCopyCreatePipelineFeedbackData(count, pCreateInfos, crtpl_state->gpu_create_infos.data());
    UtilPostCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, this);
}

void GpuAssisted::PostCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                             const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                             const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                             VkResult result, void *crtpl_state_data) {
    auto *crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_khr_api_state *>(crtpl_state_data);
    ValidationStateTracker::PostCallRecordCreateRayTracingPipelinesKHR(device, pipelineCache, count, pCreateInfos, pAllocator,
                                                                       pPipelines, result, crtpl_state_data);
    UtilCopyCreatePipelineFeedbackData(count, pCreateInfos, crtpl_state->gpu_create_infos.data());
    UtilPostCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, this);
}

// Remove all the shader trackers associated with this destroyed pipeline.
void GpuAssisted::PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks *pAllocator) {
    for (auto it = shader_map.begin(); it != shader_map.end();) {
        if (it->second.pipeline == pipeline) {
            it = shader_map.erase(it);
        } else {
            ++it;
        }
    }
    ValidationStateTracker::PreCallRecordDestroyPipeline(device, pipeline, pAllocator);
}

// Call the SPIR-V Optimizer to run the instrumentation pass on the shader.
bool GpuAssisted::InstrumentShader(const VkShaderModuleCreateInfo *pCreateInfo, std::vector<unsigned int> &new_pgm,
                                   uint32_t *unique_shader_id) {
    if (aborted) return false;
    if (pCreateInfo->pCode[0] != spv::MagicNumber) return false;

    const spvtools::MessageConsumer GpuConsoleMessageConsumer =
        [this](spv_message_level_t level, const char *, const spv_position_t &position, const char *message) -> void {
        switch (level) {
            case SPV_MSG_FATAL:
            case SPV_MSG_INTERNAL_ERROR:
            case SPV_MSG_ERROR:
                this->LogError(this->device, "UNASSIGNED-GPU-Assisted", "Error during shader instrumentation: line %zu: %s",
                               position.index, message);
                break;
            default:
                break;
        }
    };

    // Load original shader SPIR-V
    uint32_t num_words = static_cast<uint32_t>(pCreateInfo->codeSize / 4);
    new_pgm.clear();
    new_pgm.reserve(num_words);
    new_pgm.insert(new_pgm.end(), &pCreateInfo->pCode[0], &pCreateInfo->pCode[num_words]);

    // Call the optimizer to instrument the shader.
    // Use the unique_shader_module_id as a shader ID so we can look up its handle later in the shader_map.
    // If descriptor indexing is enabled, enable length checks and updated descriptor checks
    using namespace spvtools;
    spv_target_env target_env = PickSpirvEnv(api_version, (device_extensions.vk_khr_spirv_1_4 != kNotEnabled));
    spvtools::ValidatorOptions options;
    AdjustValidatorOptions(device_extensions, enabled_features, options);
    Optimizer optimizer(target_env);
    optimizer.SetMessageConsumer(GpuConsoleMessageConsumer);
    optimizer.RegisterPass(
        CreateInstBindlessCheckPass(desc_set_bind_index, unique_shader_module_id, descriptor_indexing, descriptor_indexing));
    optimizer.RegisterPass(CreateAggressiveDCEPass());
    if ((device_extensions.vk_ext_buffer_device_address || device_extensions.vk_khr_buffer_device_address) && shaderInt64 &&
        enabled_features.core12.bufferDeviceAddress)
        optimizer.RegisterPass(CreateInstBuffAddrCheckPass(desc_set_bind_index, unique_shader_module_id));
    bool pass = optimizer.Run(new_pgm.data(), new_pgm.size(), &new_pgm, options, false);
    if (!pass) {
        ReportSetupProblem(device, "Failure to instrument shader.  Proceeding with non-instrumented shader.");
    }
    *unique_shader_id = unique_shader_module_id++;
    return pass;
}
// Create the instrumented shader data to provide to the driver.
void GpuAssisted::PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule,
                                                  void *csm_state_data) {
    create_shader_module_api_state *csm_state = reinterpret_cast<create_shader_module_api_state *>(csm_state_data);
    bool pass = InstrumentShader(pCreateInfo, csm_state->instrumented_pgm, &csm_state->unique_shader_id);
    if (pass) {
        csm_state->instrumented_create_info.pCode = csm_state->instrumented_pgm.data();
        csm_state->instrumented_create_info.codeSize = csm_state->instrumented_pgm.size() * sizeof(unsigned int);
    }
}

// Generate the part of the message describing the violation.
static void GenerateValidationMessage(const uint32_t *debug_record, std::string &msg, std::string &vuid_msg) {
    using namespace spvtools;
    std::ostringstream strm;
    switch (debug_record[kInstValidationOutError]) {
        case kInstErrorBindlessBounds: {
            strm << "Index of " << debug_record[kInstBindlessBoundsOutDescIndex] << " used to index descriptor array of length "
                 << debug_record[kInstBindlessBoundsOutDescBound] << ". ";
            vuid_msg = "UNASSIGNED-Descriptor index out of bounds";
        } break;
        case kInstErrorBindlessUninit: {
            strm << "Descriptor index " << debug_record[kInstBindlessUninitOutDescIndex] << " is uninitialized. ";
            vuid_msg = "UNASSIGNED-Descriptor uninitialized";
        } break;
        case kInstErrorBuffAddrUnallocRef: {
            uint64_t *ptr = (uint64_t *)&debug_record[kInstBuffAddrUnallocOutDescPtrLo];
            strm << "Device address 0x" << std::hex << *ptr << " access out of bounds. ";
            vuid_msg = "UNASSIGNED-Device address out of bounds";
        } break;
        default: {
            strm << "Internal Error (unexpected error type = " << debug_record[kInstValidationOutError] << "). ";
            vuid_msg = "UNASSIGNED-Internal Error";
            assert(false);
        } break;
    }
    msg = strm.str();
}

// Pull together all the information from the debug record to build the error message strings,
// and then assemble them into a single message string.
// Retrieve the shader program referenced by the unique shader ID provided in the debug record.
// We had to keep a copy of the shader program with the same lifecycle as the pipeline to make
// sure it is available when the pipeline is submitted.  (The ShaderModule tracking object also
// keeps a copy, but it can be destroyed after the pipeline is created and before it is submitted.)
//
void GpuAssisted::AnalyzeAndGenerateMessages(VkCommandBuffer command_buffer, VkQueue queue, VkPipelineBindPoint pipeline_bind_point,
                                             uint32_t operation_index, uint32_t *const debug_output_buffer) {
    using namespace spvtools;
    const uint32_t total_words = debug_output_buffer[0];
    // A zero here means that the shader instrumentation didn't write anything.
    // If you have nothing to say, don't say it here.
    if (0 == total_words) {
        return;
    }
    // The first word in the debug output buffer is the number of words that would have
    // been written by the shader instrumentation, if there was enough room in the buffer we provided.
    // The number of words actually written by the shaders is determined by the size of the buffer
    // we provide via the descriptor.  So, we process only the number of words that can fit in the
    // buffer.
    // Each "report" written by the shader instrumentation is considered a "record".  This function
    // is hard-coded to process only one record because it expects the buffer to be large enough to
    // hold only one record.  If there is a desire to process more than one record, this function needs
    // to be modified to loop over records and the buffer size increased.
    std::string validation_message;
    std::string stage_message;
    std::string common_message;
    std::string filename_message;
    std::string source_message;
    std::string vuid_msg;
    VkShaderModule shader_module_handle = VK_NULL_HANDLE;
    VkPipeline pipeline_handle = VK_NULL_HANDLE;
    std::vector<unsigned int> pgm;
    // The first record starts at this offset after the total_words.
    const uint32_t *debug_record = &debug_output_buffer[kDebugOutputDataOffset];
    // Lookup the VkShaderModule handle and SPIR-V code used to create the shader, using the unique shader ID value returned
    // by the instrumented shader.
    auto it = shader_map.find(debug_record[kInstCommonOutShaderId]);
    if (it != shader_map.end()) {
        shader_module_handle = it->second.shader_module;
        pipeline_handle = it->second.pipeline;
        pgm = it->second.pgm;
    }
    GenerateValidationMessage(debug_record, validation_message, vuid_msg);
    UtilGenerateStageMessage(debug_record, stage_message);
    UtilGenerateCommonMessage(report_data, command_buffer, debug_record, shader_module_handle, pipeline_handle, pipeline_bind_point,
                              operation_index, common_message);
    UtilGenerateSourceMessages(pgm, debug_record, false, filename_message, source_message);
    LogError(queue, vuid_msg.c_str(), "%s %s %s %s%s", validation_message.c_str(), common_message.c_str(), stage_message.c_str(),
             filename_message.c_str(), source_message.c_str());
    // The debug record at word kInstCommonOutSize is the number of words in the record
    // written by the shader.  Clear the entire record plus the total_words word at the start.
    const uint32_t words_to_clear = 1 + std::min(debug_record[kInstCommonOutSize], (uint32_t)kInstMaxOutCnt);
    memset(debug_output_buffer, 0, sizeof(uint32_t) * words_to_clear);
}

// For the given command buffer, map its debug data buffers and update the status of any update after bind descriptors
void GpuAssisted::UpdateInstrumentationBuffer(CMD_BUFFER_STATE *cb_node) {
    auto gpu_buffer_list = GetBufferInfo(cb_node->commandBuffer);
    uint32_t *pData;
    for (auto &buffer_info : gpu_buffer_list) {
        if (buffer_info.di_input_mem_block.update_at_submit.size() > 0) {
            VkResult result = vmaMapMemory(vmaAllocator, buffer_info.di_input_mem_block.allocation, (void **)&pData);
            if (result == VK_SUCCESS) {
                for (auto update : buffer_info.di_input_mem_block.update_at_submit) {
                    if (update.second->updated) pData[update.first] = 1;
                }
                vmaUnmapMemory(vmaAllocator, buffer_info.di_input_mem_block.allocation);
            }
        }
    }
}

void GpuAssisted::PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence) {
    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        const VkSubmitInfo *submit = &pSubmits[submit_idx];
        for (uint32_t i = 0; i < submit->commandBufferCount; i++) {
            auto cb_node = GetCBState(submit->pCommandBuffers[i]);
            UpdateInstrumentationBuffer(cb_node);
            for (auto secondaryCmdBuffer : cb_node->linkedCommandBuffers) {
                UpdateInstrumentationBuffer(secondaryCmdBuffer);
            }
        }
    }
}

// Issue a memory barrier to make GPU-written data available to host.
// Wait for the queue to complete execution.
// Check the debug buffers for all the command buffers that were submitted.
void GpuAssisted::PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence,
                                            VkResult result) {
    ValidationStateTracker::PostCallRecordQueueSubmit(queue, submitCount, pSubmits, fence, result);

    if (aborted || (result != VK_SUCCESS)) return;
    bool buffers_present = false;
    // Don't QueueWaitIdle if there's nothing to process
    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        const VkSubmitInfo *submit = &pSubmits[submit_idx];
        for (uint32_t i = 0; i < submit->commandBufferCount; i++) {
            auto cb_node = GetCBState(submit->pCommandBuffers[i]);
            if (GetBufferInfo(cb_node->commandBuffer).size() || cb_node->hasBuildAccelerationStructureCmd) buffers_present = true;
            for (auto secondaryCmdBuffer : cb_node->linkedCommandBuffers) {
                if (GetBufferInfo(secondaryCmdBuffer->commandBuffer).size() || cb_node->hasBuildAccelerationStructureCmd)
                    buffers_present = true;
            }
        }
    }
    if (!buffers_present) return;

    UtilSubmitBarrier(queue, this);

    DispatchQueueWaitIdle(queue);

    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        const VkSubmitInfo *submit = &pSubmits[submit_idx];
        for (uint32_t i = 0; i < submit->commandBufferCount; i++) {
            auto cb_node = GetCBState(submit->pCommandBuffers[i]);
            UtilProcessInstrumentationBuffer(queue, cb_node, this);
            ProcessAccelerationStructureBuildValidationBuffer(queue, cb_node);
            for (auto secondaryCmdBuffer : cb_node->linkedCommandBuffers) {
                UtilProcessInstrumentationBuffer(queue, secondaryCmdBuffer, this);
                ProcessAccelerationStructureBuildValidationBuffer(queue, cb_node);
            }
        }
    }
}

void GpuAssisted::PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                       uint32_t firstVertex, uint32_t firstInstance) {
    AllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void GpuAssisted::PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                              uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    AllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void GpuAssisted::PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                               uint32_t stride) {
    AllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void GpuAssisted::PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      uint32_t count, uint32_t stride) {
    AllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void GpuAssisted::PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride) {
    ValidationStateTracker::PreCallRecordCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                                 maxDrawCount, stride);
    AllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void GpuAssisted::PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                    uint32_t stride) {
    ValidationStateTracker::PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                              maxDrawCount, stride);
    AllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void GpuAssisted::PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                              uint32_t maxDrawCount, uint32_t stride) {
    ValidationStateTracker::PreCallRecordCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer,
                                                                        countBufferOffset, maxDrawCount, stride);
    AllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void GpuAssisted::PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride) {
    ValidationStateTracker::PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                                     maxDrawCount, stride);
    AllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void GpuAssisted::PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) {
    ValidationStateTracker::PreCallRecordCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
    AllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void GpuAssisted::PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          uint32_t drawCount, uint32_t stride) {
    ValidationStateTracker::PreCallRecordCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
    AllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void GpuAssisted::PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                               uint32_t maxDrawCount, uint32_t stride) {
    ValidationStateTracker::PreCallRecordCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer,
                                                                         countBufferOffset, maxDrawCount, stride);
    AllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void GpuAssisted::PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) {
    AllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE);
}

void GpuAssisted::PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    AllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE);
}

void GpuAssisted::PreCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                               uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                               uint32_t groupCountZ) {
    AllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE);
}

void GpuAssisted::PreCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                  uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                  uint32_t groupCountZ) {
    AllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE);
}

void GpuAssisted::PreCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                              VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                              VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                              VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                              VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                              VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                              uint32_t width, uint32_t height, uint32_t depth) {
    AllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV);
}

void GpuAssisted::PostCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                               VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                               VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                               VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                               VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                               VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                               uint32_t width, uint32_t height, uint32_t depth) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    cb_state->hasTraceRaysCmd = true;
}

void GpuAssisted::PreCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                               const VkStridedBufferRegionKHR *pRaygenShaderBindingTable,
                                               const VkStridedBufferRegionKHR *pMissShaderBindingTable,
                                               const VkStridedBufferRegionKHR *pHitShaderBindingTable,
                                               const VkStridedBufferRegionKHR *pCallableShaderBindingTable, uint32_t width,
                                               uint32_t height, uint32_t depth) {
    AllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
}

void GpuAssisted::PostCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                                const VkStridedBufferRegionKHR *pRaygenShaderBindingTable,
                                                const VkStridedBufferRegionKHR *pMissShaderBindingTable,
                                                const VkStridedBufferRegionKHR *pHitShaderBindingTable,
                                                const VkStridedBufferRegionKHR *pCallableShaderBindingTable, uint32_t width,
                                                uint32_t height, uint32_t depth) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    cb_state->hasTraceRaysCmd = true;
}

void GpuAssisted::PreCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                       const VkStridedBufferRegionKHR *pRaygenShaderBindingTable,
                                                       const VkStridedBufferRegionKHR *pMissShaderBindingTable,
                                                       const VkStridedBufferRegionKHR *pHitShaderBindingTable,
                                                       const VkStridedBufferRegionKHR *pCallableShaderBindingTable, VkBuffer buffer,
                                                       VkDeviceSize offset) {
    AllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
}

void GpuAssisted::PostCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                        const VkStridedBufferRegionKHR *pRaygenShaderBindingTable,
                                                        const VkStridedBufferRegionKHR *pMissShaderBindingTable,
                                                        const VkStridedBufferRegionKHR *pHitShaderBindingTable,
                                                        const VkStridedBufferRegionKHR *pCallableShaderBindingTable,
                                                        VkBuffer buffer, VkDeviceSize offset) {
    CMD_BUFFER_STATE *cb_state = GetCBState(commandBuffer);
    cb_state->hasTraceRaysCmd = true;
}

void GpuAssisted::AllocateValidationResources(const VkCommandBuffer cmd_buffer, const VkPipelineBindPoint bind_point) {
    if (bind_point != VK_PIPELINE_BIND_POINT_GRAPHICS && bind_point != VK_PIPELINE_BIND_POINT_COMPUTE &&
        bind_point != VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
        return;
    }
    VkResult result;

    if (aborted) return;

    std::vector<VkDescriptorSet> desc_sets;
    VkDescriptorPool desc_pool = VK_NULL_HANDLE;
    result = desc_set_manager->GetDescriptorSets(1, &desc_pool, debug_desc_layout, &desc_sets);
    assert(result == VK_SUCCESS);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, "Unable to allocate descriptor sets.  Device could become unstable.");
        aborted = true;
        return;
    }

    VkDescriptorBufferInfo output_desc_buffer_info = {};
    output_desc_buffer_info.range = output_buffer_size;

    auto cb_node = GetCBState(cmd_buffer);
    if (!cb_node) {
        ReportSetupProblem(device, "Unrecognized command buffer");
        aborted = true;
        return;
    }

    // Allocate memory for the output block that the gpu will use to return any error information
    GpuAssistedDeviceMemoryBlock output_block = {};
    VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = output_buffer_size;
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
    result = vmaCreateBuffer(vmaAllocator, &bufferInfo, &allocInfo, &output_block.buffer, &output_block.allocation, nullptr);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(device, "Unable to allocate device memory.  Device could become unstable.");
        aborted = true;
        return;
    }

    // Clear the output block to zeros so that only error information from the gpu will be present
    uint32_t *pData;
    result = vmaMapMemory(vmaAllocator, output_block.allocation, (void **)&pData);
    if (result == VK_SUCCESS) {
        memset(pData, 0, output_buffer_size);
        vmaUnmapMemory(vmaAllocator, output_block.allocation);
    }

    GpuAssistedDeviceMemoryBlock di_input_block = {}, bda_input_block = {};
    VkDescriptorBufferInfo di_input_desc_buffer_info = {};
    VkDescriptorBufferInfo bda_input_desc_buffer_info = {};
    VkWriteDescriptorSet desc_writes[3] = {};
    uint32_t desc_count = 1;
    auto const &state = cb_node->lastBound[bind_point];
    uint32_t number_of_sets = (uint32_t)state.per_set.size();

    // Figure out how much memory we need for the input block based on how many sets and bindings there are
    // and how big each of the bindings is
    if (number_of_sets > 0 && descriptor_indexing) {
        uint32_t descriptor_count = 0;  // Number of descriptors, including all array elements
        uint32_t binding_count = 0;     // Number of bindings based on the max binding number used
        for (auto s : state.per_set) {
            auto desc = s.bound_descriptor_set;
            if (desc && (desc->GetBindingCount() > 0)) {
                auto bindings = desc->GetLayout()->GetSortedBindingSet();
                binding_count += desc->GetLayout()->GetMaxBinding() + 1;
                for (auto binding : bindings) {
                    // Shader instrumentation is tracking inline uniform blocks as scalers. Don't try to validate inline uniform
                    // blocks
                    if (VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT == desc->GetLayout()->GetTypeFromBinding(binding)) {
                        descriptor_count++;
                        LogWarning(device, "UNASSIGNED-GPU-Assisted Validation Warning",
                                   "VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT descriptors will not be validated by GPU assisted "
                                   "validation");
                    } else if (binding == desc->GetLayout()->GetMaxBinding() && desc->IsVariableDescriptorCount(binding)) {
                        descriptor_count += desc->GetVariableDescriptorCount();
                    } else {
                        descriptor_count += desc->GetDescriptorCountFromBinding(binding);
                    }
                }
            }
        }

        // Note that the size of the input buffer is dependent on the maximum binding number, which
        // can be very large.  This is because for (set = s, binding = b, index = i), the validation
        // code is going to dereference Input[ i + Input[ b + Input[ s + Input[ Input[0] ] ] ] ] to
        // see if descriptors have been written. In gpu_validation.md, we note this and advise
        // using densely packed bindings as a best practice when using gpu-av with descriptor indexing
        uint32_t words_needed = 1 + (number_of_sets * 2) + (binding_count * 2) + descriptor_count;
        allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        bufferInfo.size = words_needed * 4;
        result =
            vmaCreateBuffer(vmaAllocator, &bufferInfo, &allocInfo, &di_input_block.buffer, &di_input_block.allocation, nullptr);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Unable to allocate device memory.  Device could become unstable.");
            aborted = true;
            return;
        }

        // Populate input buffer first with the sizes of every descriptor in every set, then with whether
        // each element of each descriptor has been written or not.  See gpu_validation.md for a more thourough
        // outline of the input buffer format
        result = vmaMapMemory(vmaAllocator, di_input_block.allocation, (void **)&pData);
        memset(pData, 0, static_cast<size_t>(bufferInfo.size));
        // Pointer to a sets array that points into the sizes array
        uint32_t *sets_to_sizes = pData + 1;
        // Pointer to the sizes array that contains the array size of the descriptor at each binding
        uint32_t *sizes = sets_to_sizes + number_of_sets;
        // Pointer to another sets array that points into the bindings array that points into the written array
        uint32_t *sets_to_bindings = sizes + binding_count;
        // Pointer to the bindings array that points at the start of the writes in the writes array for each binding
        uint32_t *bindings_to_written = sets_to_bindings + number_of_sets;
        // Index of the next entry in the written array to be updated
        uint32_t written_index = 1 + (number_of_sets * 2) + (binding_count * 2);
        uint32_t bindCounter = number_of_sets + 1;
        // Index of the start of the sets_to_bindings array
        pData[0] = number_of_sets + binding_count + 1;

        for (auto s : state.per_set) {
            auto desc = s.bound_descriptor_set;
            if (desc && (desc->GetBindingCount() > 0)) {
                auto layout = desc->GetLayout();
                auto bindings = layout->GetSortedBindingSet();
                // For each set, fill in index of its bindings sizes in the sizes array
                *sets_to_sizes++ = bindCounter;
                // For each set, fill in the index of its bindings in the bindings_to_written array
                *sets_to_bindings++ = bindCounter + number_of_sets + binding_count;
                for (auto binding : bindings) {
                    // For each binding, fill in its size in the sizes array
                    // Shader instrumentation is tracking inline uniform blocks as scalers. Don't try to validate inline uniform
                    // blocks
                    if (VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT == desc->GetLayout()->GetTypeFromBinding(binding)) {
                        sizes[binding] = 1;
                    } else if (binding == layout->GetMaxBinding() && desc->IsVariableDescriptorCount(binding)) {
                        sizes[binding] = desc->GetVariableDescriptorCount();
                    } else {
                        sizes[binding] = desc->GetDescriptorCountFromBinding(binding);
                    }
                    // Fill in the starting index for this binding in the written array in the bindings_to_written array
                    bindings_to_written[binding] = written_index;

                    // Shader instrumentation is tracking inline uniform blocks as scalers. Don't try to validate inline uniform
                    // blocks
                    if (VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT == desc->GetLayout()->GetTypeFromBinding(binding)) {
                        pData[written_index++] = 1;
                        continue;
                    }

                    auto index_range = desc->GetGlobalIndexRangeFromBinding(binding, true);
                    // For each array element in the binding, update the written array with whether it has been written
                    for (uint32_t i = index_range.start; i < index_range.end; ++i) {
                        auto *descriptor = desc->GetDescriptorFromGlobalIndex(i);
                        if (descriptor->updated) {
                            pData[written_index] = 1;
                        } else if (desc->IsUpdateAfterBind(binding)) {
                            // If it hasn't been written now and it's update after bind, put it in a list to check at QueueSubmit
                            di_input_block.update_at_submit[written_index] = descriptor;
                        }
                        written_index++;
                    }
                }
                auto last = desc->GetLayout()->GetMaxBinding();
                bindings_to_written += last + 1;
                bindCounter += last + 1;
                sizes += last + 1;
            } else {
                *sets_to_sizes++ = 0;
                *sets_to_bindings++ = 0;
            }
        }
        vmaUnmapMemory(vmaAllocator, di_input_block.allocation);

        di_input_desc_buffer_info.range = (words_needed * 4);
        di_input_desc_buffer_info.buffer = di_input_block.buffer;
        di_input_desc_buffer_info.offset = 0;

        desc_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        desc_writes[1].dstBinding = 1;
        desc_writes[1].descriptorCount = 1;
        desc_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        desc_writes[1].pBufferInfo = &di_input_desc_buffer_info;
        desc_writes[1].dstSet = desc_sets[0];

        desc_count = 2;
    }

    if ((device_extensions.vk_ext_buffer_device_address || device_extensions.vk_khr_buffer_device_address) && buffer_map.size() &&
        shaderInt64 && enabled_features.core12.bufferDeviceAddress) {
        // Example BDA input buffer assuming 2 buffers using BDA:
        // Word 0 | Index of start of buffer sizes (in this case 5)
        // Word 1 | 0x0000000000000000
        // Word 2 | Device Address of first buffer  (Addresses sorted in ascending order)
        // Word 3 | Device Address of second buffer
        // Word 4 | 0xffffffffffffffff
        // Word 5 | 0 (size of pretend buffer at word 1)
        // Word 6 | Size in bytes of first buffer
        // Word 7 | Size in bytes of second buffer
        // Word 8 | 0 (size of pretend buffer in word 4)

        uint32_t num_buffers = static_cast<uint32_t>(buffer_map.size());
        uint32_t words_needed = (num_buffers + 3) + (num_buffers + 2);
        allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        bufferInfo.size = words_needed * 8;  // 64 bit words
        result =
            vmaCreateBuffer(vmaAllocator, &bufferInfo, &allocInfo, &bda_input_block.buffer, &bda_input_block.allocation, nullptr);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(device, "Unable to allocate device memory.  Device could become unstable.");
            aborted = true;
            return;
        }
        uint64_t *bda_data;
        result = vmaMapMemory(vmaAllocator, bda_input_block.allocation, (void **)&bda_data);
        uint32_t address_index = 1;
        uint32_t size_index = 3 + num_buffers;
        memset(bda_data, 0, static_cast<size_t>(bufferInfo.size));
        bda_data[0] = size_index;       // Start of buffer sizes
        bda_data[address_index++] = 0;  // NULL address
        bda_data[size_index++] = 0;

        for (auto const &value : buffer_map) {
            bda_data[address_index++] = value.first;
            bda_data[size_index++] = value.second;
        }
        bda_data[address_index] = UINTPTR_MAX;
        bda_data[size_index] = 0;
        vmaUnmapMemory(vmaAllocator, bda_input_block.allocation);

        bda_input_desc_buffer_info.range = (words_needed * 8);
        bda_input_desc_buffer_info.buffer = bda_input_block.buffer;
        bda_input_desc_buffer_info.offset = 0;

        desc_writes[desc_count].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        desc_writes[desc_count].dstBinding = 2;
        desc_writes[desc_count].descriptorCount = 1;
        desc_writes[desc_count].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        desc_writes[desc_count].pBufferInfo = &bda_input_desc_buffer_info;
        desc_writes[desc_count].dstSet = desc_sets[0];
        desc_count++;
    }

    // Write the descriptor
    output_desc_buffer_info.buffer = output_block.buffer;
    output_desc_buffer_info.offset = 0;

    desc_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_writes[0].descriptorCount = 1;
    desc_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    desc_writes[0].pBufferInfo = &output_desc_buffer_info;
    desc_writes[0].dstSet = desc_sets[0];
    DispatchUpdateDescriptorSets(device, desc_count, desc_writes, 0, NULL);

    auto iter = cb_node->lastBound.find(bind_point);  // find() allows read-only access to cb_state
    if (iter != cb_node->lastBound.end()) {
        auto pipeline_state = iter->second.pipeline_state;
        if (pipeline_state && (pipeline_state->pipeline_layout->set_layouts.size() <= desc_set_bind_index) &&
            !pipeline_state->pipeline_layout->destroyed) {
            DispatchCmdBindDescriptorSets(cmd_buffer, bind_point, pipeline_state->pipeline_layout->layout, desc_set_bind_index, 1,
                                          desc_sets.data(), 0, nullptr);
        }
        if (pipeline_state && pipeline_state->pipeline_layout->destroyed) {
            ReportSetupProblem(device, "Pipeline layout has been destroyed, aborting GPU-AV");
            aborted = true;
        } else {
            // Record buffer and memory info in CB state tracking
            GetBufferInfo(cmd_buffer)
                .emplace_back(output_block, di_input_block, bda_input_block, desc_sets[0], desc_pool, bind_point);
        }
    } else {
        ReportSetupProblem(device, "Unable to find pipeline state");
        aborted = true;
    }
    if (aborted) {
        vmaDestroyBuffer(vmaAllocator, di_input_block.buffer, di_input_block.allocation);
        vmaDestroyBuffer(vmaAllocator, bda_input_block.buffer, bda_input_block.allocation);
        vmaDestroyBuffer(vmaAllocator, output_block.buffer, output_block.allocation);
        return;
    }
}
