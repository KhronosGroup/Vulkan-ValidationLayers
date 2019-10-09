/* Copyright (c) 2018-2019 The Khronos Group Inc.
 * Copyright (c) 2018-2019 Valve Corporation
 * Copyright (c) 2018-2019 LunarG, Inc.
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

// Allow use of STL min and max functions in Windows
#define NOMINMAX
// This define indicates to build the VMA routines themselves
#define VMA_IMPLEMENTATION
// This define indicates that we will supply Vulkan function pointers at initialization
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#include "gpu_validation.h"
#include "shader_validation.h"
#include "spirv-tools/libspirv.h"
#include "spirv-tools/optimizer.hpp"
#include "spirv-tools/instrument.hpp"
#include <SPIRV/spirv.hpp>
#include <algorithm>
#include <regex>
#include "layer_chassis_dispatch.h"

// This is the number of bindings in the debug descriptor set.
static const uint32_t kNumBindingsInSet = 3;

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

// Implementation for Descriptor Set Manager class
GpuAssistedDescriptorSetManager::GpuAssistedDescriptorSetManager(GpuAssisted *dev_data) { dev_data_ = dev_data; }

GpuAssistedDescriptorSetManager::~GpuAssistedDescriptorSetManager() {
    for (auto &pool : desc_pool_map_) {
        DispatchDestroyDescriptorPool(dev_data_->device, pool.first, NULL);
    }
    desc_pool_map_.clear();
}

VkResult GpuAssistedDescriptorSetManager::GetDescriptorSet(VkDescriptorPool *desc_pool, VkDescriptorSet *desc_set) {
    std::vector<VkDescriptorSet> desc_sets;
    VkResult result = GetDescriptorSets(1, desc_pool, &desc_sets);
    if (result == VK_SUCCESS) {
        *desc_set = desc_sets[0];
    }
    return result;
}

VkResult GpuAssistedDescriptorSetManager::GetDescriptorSets(uint32_t count, VkDescriptorPool *pool,
                                                            std::vector<VkDescriptorSet> *desc_sets) {
    const uint32_t default_pool_size = kItemsPerChunk;
    VkResult result = VK_SUCCESS;
    VkDescriptorPool pool_to_use = VK_NULL_HANDLE;

    if (0 == count) {
        return result;
    }
    desc_sets->clear();
    desc_sets->resize(count);

    for (auto &pool : desc_pool_map_) {
        if (pool.second.used + count < pool.second.size) {
            pool_to_use = pool.first;
            break;
        }
    }
    if (VK_NULL_HANDLE == pool_to_use) {
        uint32_t pool_count = default_pool_size;
        if (count > default_pool_size) {
            pool_count = count;
        }
        const VkDescriptorPoolSize size_counts = {
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            pool_count * kNumBindingsInSet,
        };
        VkDescriptorPoolCreateInfo desc_pool_info = {};
        desc_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        desc_pool_info.pNext = NULL;
        desc_pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        desc_pool_info.maxSets = pool_count;
        desc_pool_info.poolSizeCount = 1;
        desc_pool_info.pPoolSizes = &size_counts;
        result = DispatchCreateDescriptorPool(dev_data_->device, &desc_pool_info, NULL, &pool_to_use);
        assert(result == VK_SUCCESS);
        if (result != VK_SUCCESS) {
            return result;
        }
        desc_pool_map_[pool_to_use].size = desc_pool_info.maxSets;
        desc_pool_map_[pool_to_use].used = 0;
    }
    std::vector<VkDescriptorSetLayout> desc_layouts(count, dev_data_->debug_desc_layout);

    VkDescriptorSetAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, NULL, pool_to_use, count,
                                              desc_layouts.data()};

    result = DispatchAllocateDescriptorSets(dev_data_->device, &alloc_info, desc_sets->data());
    assert(result == VK_SUCCESS);
    if (result != VK_SUCCESS) {
        return result;
    }
    *pool = pool_to_use;
    desc_pool_map_[pool_to_use].used += count;
    return result;
}

void GpuAssistedDescriptorSetManager::PutBackDescriptorSet(VkDescriptorPool desc_pool, VkDescriptorSet desc_set) {
    auto iter = desc_pool_map_.find(desc_pool);
    if (iter != desc_pool_map_.end()) {
        VkResult result = DispatchFreeDescriptorSets(dev_data_->device, desc_pool, 1, &desc_set);
        assert(result == VK_SUCCESS);
        if (result != VK_SUCCESS) {
            return;
        }
        desc_pool_map_[desc_pool].used--;
        if (0 == desc_pool_map_[desc_pool].used) {
            DispatchDestroyDescriptorPool(dev_data_->device, desc_pool, NULL);
            desc_pool_map_.erase(desc_pool);
        }
    }
    return;
}

// Trampolines to make VMA call Dispatch for Vulkan calls
static VKAPI_ATTR void VKAPI_CALL gpuVkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                                                   VkPhysicalDeviceProperties *pProperties) {
    DispatchGetPhysicalDeviceProperties(physicalDevice, pProperties);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                                                         VkPhysicalDeviceMemoryProperties *pMemoryProperties) {
    DispatchGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
}
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                                                          const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory) {
    return DispatchAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks *pAllocator) {
    DispatchFreeMemory(device, memory, pAllocator);
}
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                                                     VkMemoryMapFlags flags, void **ppData) {
    return DispatchMapMemory(device, memory, offset, size, flags, ppData);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkUnmapMemory(VkDevice device, VkDeviceMemory memory) { DispatchUnmapMemory(device, memory); }
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                                   const VkMappedMemoryRange *pMemoryRanges) {
    return DispatchFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
}
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                                        const VkMappedMemoryRange *pMemoryRanges) {
    return DispatchInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
}
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory,
                                                            VkDeviceSize memoryOffset) {
    return DispatchBindBufferMemory(device, buffer, memory, memoryOffset);
}
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory,
                                                           VkDeviceSize memoryOffset) {
    return DispatchBindImageMemory(device, image, memory, memoryOffset);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer,
                                                                   VkMemoryRequirements *pMemoryRequirements) {
    DispatchGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkGetImageMemoryRequirements(VkDevice device, VkImage image,
                                                                  VkMemoryRequirements *pMemoryRequirements) {
    DispatchGetImageMemoryRequirements(device, image, pMemoryRequirements);
}
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                                        const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer) {
    return DispatchCreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator) {
    return DispatchDestroyBuffer(device, buffer, pAllocator);
}
static VKAPI_ATTR VkResult VKAPI_CALL gpuVkCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkImage *pImage) {
    return DispatchCreateImage(device, pCreateInfo, pAllocator, pImage);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator) {
    DispatchDestroyImage(device, image, pAllocator);
}
static VKAPI_ATTR void VKAPI_CALL gpuVkCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                                     uint32_t regionCount, const VkBufferCopy *pRegions) {
    DispatchCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}

VkResult GpuAssisted::InitializeVma(VkPhysicalDevice physical_device, VkDevice device, VmaAllocator *pAllocator) {
    VmaVulkanFunctions functions;
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.device = device;
    allocatorInfo.physicalDevice = physical_device;

    functions.vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)gpuVkGetPhysicalDeviceProperties;
    functions.vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)gpuVkGetPhysicalDeviceMemoryProperties;
    functions.vkAllocateMemory = (PFN_vkAllocateMemory)gpuVkAllocateMemory;
    functions.vkFreeMemory = (PFN_vkFreeMemory)gpuVkFreeMemory;
    functions.vkMapMemory = (PFN_vkMapMemory)gpuVkMapMemory;
    functions.vkUnmapMemory = (PFN_vkUnmapMemory)gpuVkUnmapMemory;
    functions.vkFlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges)gpuVkFlushMappedMemoryRanges;
    functions.vkInvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges)gpuVkInvalidateMappedMemoryRanges;
    functions.vkBindBufferMemory = (PFN_vkBindBufferMemory)gpuVkBindBufferMemory;
    functions.vkBindImageMemory = (PFN_vkBindImageMemory)gpuVkBindImageMemory;
    functions.vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements)gpuVkGetBufferMemoryRequirements;
    functions.vkGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements)gpuVkGetImageMemoryRequirements;
    functions.vkCreateBuffer = (PFN_vkCreateBuffer)gpuVkCreateBuffer;
    functions.vkDestroyBuffer = (PFN_vkDestroyBuffer)gpuVkDestroyBuffer;
    functions.vkCreateImage = (PFN_vkCreateImage)gpuVkCreateImage;
    functions.vkDestroyImage = (PFN_vkDestroyImage)gpuVkDestroyImage;
    functions.vkCmdCopyBuffer = (PFN_vkCmdCopyBuffer)gpuVkCmdCopyBuffer;
    allocatorInfo.pVulkanFunctions = &functions;

    return vmaCreateAllocator(&allocatorInfo, pAllocator);
}

// Convenience function for reporting problems with setting up GPU Validation.
void GpuAssisted::ReportSetupProblem(VkDebugReportObjectTypeEXT object_type, uint64_t object_handle,
                                     const char *const specific_message) const {
    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, object_type, object_handle, "UNASSIGNED-GPU-Assisted Validation Error. ",
            "Detail: (%s)", specific_message);
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
    VkPhysicalDeviceFeatures supported_features;
    DispatchGetPhysicalDeviceFeatures(gpu, &supported_features);
    if (supported_features.fragmentStoresAndAtomics || supported_features.vertexPipelineStoresAndAtomics ||
        supported_features.shaderInt64) {
        VkPhysicalDeviceFeatures *features = nullptr;
        if (modified_create_info->pEnabledFeatures) {
            // If pEnabledFeatures, VkPhysicalDeviceFeatures2 in pNext chain is not allowed
            features = const_cast<VkPhysicalDeviceFeatures *>(modified_create_info->pEnabledFeatures);
        } else {
            VkPhysicalDeviceFeatures2 *features2 = nullptr;
            features2 =
                const_cast<VkPhysicalDeviceFeatures2 *>(lvl_find_in_chain<VkPhysicalDeviceFeatures2>(modified_create_info->pNext));
            if (features2) features = &features2->features;
        }
        if (features) {
            features->fragmentStoresAndAtomics = supported_features.fragmentStoresAndAtomics;
            features->vertexPipelineStoresAndAtomics = supported_features.vertexPipelineStoresAndAtomics;
            features->shaderInt64 = supported_features.shaderInt64;
        } else {
            VkPhysicalDeviceFeatures new_features = {};
            new_features.fragmentStoresAndAtomics = supported_features.fragmentStoresAndAtomics;
            new_features.vertexPipelineStoresAndAtomics = supported_features.vertexPipelineStoresAndAtomics;
            new_features.shaderInt64 = supported_features.shaderInt64;
            delete modified_create_info->pEnabledFeatures;
            modified_create_info->pEnabledFeatures = new VkPhysicalDeviceFeatures(new_features);
        }
    }
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
        ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                           "GPU-Assisted validation requires Vulkan 1.1 or later.  GPU-Assisted Validation disabled.");
        device_gpu_assisted->aborted = true;
        return;
    }

    if (!device_gpu_assisted->enabled_features.core.fragmentStoresAndAtomics ||
        !device_gpu_assisted->enabled_features.core.vertexPipelineStoresAndAtomics) {
        ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                           "GPU-Assisted validation requires fragmentStoresAndAtomics and vertexPipelineStoresAndAtomics.  "
                           "GPU-Assisted Validation disabled.");
        device_gpu_assisted->aborted = true;
        return;
    }

    if (device_extensions.vk_ext_buffer_device_address && !device_gpu_assisted->enabled_features.core.shaderInt64) {
        log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                "UNASSIGNED-GPU-Assisted Validation Warning",
                "shaderInt64 feature is not available.  No buffer device address checking will be attempted");
    }
    device_gpu_assisted->shaderInt64 = device_gpu_assisted->enabled_features.core.shaderInt64;

    // If api version 1.1 or later, SetDeviceLoaderData will be in the loader
    auto chain_info = get_chain_info(pCreateInfo, VK_LOADER_DATA_CALLBACK);
    assert(chain_info->u.pfnSetDeviceLoaderData);
    device_gpu_assisted->vkSetDeviceLoaderData = chain_info->u.pfnSetDeviceLoaderData;

    // Some devices have extremely high limits here, so set a reasonable max because we have to pad
    // the pipeline layout with dummy descriptor set layouts.
    device_gpu_assisted->adjusted_max_desc_sets = device_gpu_assisted->phys_dev_props.limits.maxBoundDescriptorSets;
    device_gpu_assisted->adjusted_max_desc_sets = std::min(33U, device_gpu_assisted->adjusted_max_desc_sets);

    // We can't do anything if there is only one.
    // Device probably not a legit Vulkan device, since there should be at least 4. Protect ourselves.
    if (device_gpu_assisted->adjusted_max_desc_sets == 1) {
        ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                           "Device can bind only a single descriptor set.  GPU-Assisted Validation disabled.");
        device_gpu_assisted->aborted = true;
        return;
    }
    device_gpu_assisted->desc_set_bind_index = device_gpu_assisted->adjusted_max_desc_sets - 1;
    log_msg(report_data, VK_DEBUG_REPORT_INFORMATION_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
            "UNASSIGNED-GPU-Assisted Validation. ", "Shaders using descriptor set at index %d. ",
            device_gpu_assisted->desc_set_bind_index);

    device_gpu_assisted->output_buffer_size = sizeof(uint32_t) * (spvtools::kInst2MaxOutCnt + 1);
    VkResult result1 = InitializeVma(physicalDevice, *pDevice, &device_gpu_assisted->vmaAllocator);
    assert(result1 == VK_SUCCESS);
    std::unique_ptr<GpuAssistedDescriptorSetManager> desc_set_manager(new GpuAssistedDescriptorSetManager(device_gpu_assisted));

    // The descriptor indexing checks require only the first "output" binding.
    const VkDescriptorSetLayoutBinding debug_desc_layout_bindings[kNumBindingsInSet] = {
        {
            0,  // output
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            1,
            VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT | kShaderStageAllRayTracing,
            NULL,
        },
        {
            1,  // input
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            1,
            VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT | kShaderStageAllRayTracing,
            NULL,
        },
        {
            2,  // input
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            1,
            VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT | kShaderStageAllRayTracing,
            NULL,
        },
    };
    const VkDescriptorSetLayoutCreateInfo debug_desc_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, NULL, 0,
                                                                    kNumBindingsInSet, debug_desc_layout_bindings};

    const VkDescriptorSetLayoutCreateInfo dummy_desc_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, NULL, 0, 0,
                                                                    NULL};

    result1 = DispatchCreateDescriptorSetLayout(*pDevice, &debug_desc_layout_info, NULL, &device_gpu_assisted->debug_desc_layout);

    // This is a layout used to "pad" a pipeline layout to fill in any gaps to the selected bind index.
    VkResult result2 =
        DispatchCreateDescriptorSetLayout(*pDevice, &dummy_desc_layout_info, NULL, &device_gpu_assisted->dummy_desc_layout);
    assert((result1 == VK_SUCCESS) && (result2 == VK_SUCCESS));
    if ((result1 != VK_SUCCESS) || (result2 != VK_SUCCESS)) {
        ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(*pDevice),
                           "Unable to create descriptor set layout.  GPU-Assisted Validation disabled.");
        if (result == VK_SUCCESS) {
            DispatchDestroyDescriptorSetLayout(*pDevice, device_gpu_assisted->debug_desc_layout, NULL);
        }
        if (result2 == VK_SUCCESS) {
            DispatchDestroyDescriptorSetLayout(*pDevice, device_gpu_assisted->dummy_desc_layout, NULL);
        }
        device_gpu_assisted->debug_desc_layout = VK_NULL_HANDLE;
        device_gpu_assisted->dummy_desc_layout = VK_NULL_HANDLE;
        device_gpu_assisted->aborted = true;
        return;
    }
    device_gpu_assisted->desc_set_manager = std::move(desc_set_manager);

    // Register callback to be called at any ResetCommandBuffer time
    device_gpu_assisted->SetCommandBufferResetCallback(
        [device_gpu_assisted](VkCommandBuffer command_buffer) -> void { device_gpu_assisted->ResetCommandBuffer(command_buffer); });

    CreateAccelerationStructureBuildValidationState(device_gpu_assisted);
}

void GpuAssisted::PostCallRecordGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfoEXT *pInfo,
                                                          VkDeviceAddress address) {
    BUFFER_STATE *buffer_state = GetBufferState(pInfo->buffer);
    // Validate against the size requested when the buffer was created
    if (buffer_state) {
        buffer_map[address] = buffer_state->createInfo.size;
        buffer_state->deviceAddress = address;
    }
}

void GpuAssisted::PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator) {
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    if (buffer_state) buffer_map.erase(buffer_state->deviceAddress);
}
// Clean up device-related resources
void GpuAssisted::PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator) {
    for (auto &queue_barrier_command_info_kv : queue_barrier_command_infos) {
        GpuAssistedQueueBarrierCommandInfo &queue_barrier_command_info = queue_barrier_command_info_kv.second;

        DispatchFreeCommandBuffers(device, queue_barrier_command_info.barrier_command_pool, 1,
                                   &queue_barrier_command_info.barrier_command_buffer);
        queue_barrier_command_info.barrier_command_buffer = VK_NULL_HANDLE;

        DispatchDestroyCommandPool(device, queue_barrier_command_info.barrier_command_pool, NULL);
        queue_barrier_command_info.barrier_command_pool = VK_NULL_HANDLE;
    }
    queue_barrier_command_infos.clear();
    if (debug_desc_layout) {
        DispatchDestroyDescriptorSetLayout(device, debug_desc_layout, NULL);
        debug_desc_layout = VK_NULL_HANDLE;
    }
    if (dummy_desc_layout) {
        DispatchDestroyDescriptorSetLayout(device, dummy_desc_layout, NULL);
        dummy_desc_layout = VK_NULL_HANDLE;
    }
    desc_set_manager.reset();

    DestroyAccelerationStructureBuildValidationState();

    if (vmaAllocator) {
        vmaDestroyAllocator(vmaAllocator);
    }
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
            ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                               "Failed to create vertex buffer for acceleration structure build validation.");
        }
    }

    if (result == VK_SUCCESS) {
        uint8_t *mapped_vbo_buffer = nullptr;
        result = vmaMapMemory(device_gpuav->vmaAllocator, vbo_allocation, (void **)&mapped_vbo_buffer);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                               "Failed to map vertex buffer for acceleration structure build validation.");
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
            ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                               "Failed to create index buffer for acceleration structure build validation.");
        }
    }

    if (result == VK_SUCCESS) {
        uint8_t *mapped_ibo_buffer = nullptr;
        result = vmaMapMemory(device_gpuav->vmaAllocator, ibo_allocation, (void **)&mapped_ibo_buffer);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                               "Failed to map index buffer for acceleration structure build validation.");
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
            ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device_gpuav->device),
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
            ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device_gpuav->device),
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
            ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device_gpuav->device),
                               "Failed to bind acceleration structure memory for acceleration structure build validation.");
        }
    }

    if (result == VK_SUCCESS) {
        result = DispatchGetAccelerationStructureHandleNV(device_gpuav->device, as_validation_state.replacement_as,
                                                          sizeof(uint64_t), &as_validation_state.replacement_as_handle);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device_gpuav->device),
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
    if (result == VK_SUCCESS) {
        VkBufferCreateInfo scratch_ci = {};
        scratch_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        scratch_ci.size = scratch_mem_requirements.memoryRequirements.size;
        scratch_ci.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;

        result = DispatchCreateBuffer(device_gpuav->device, &scratch_ci, nullptr, &scratch);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device_gpuav->device),
                               "Failed to create scratch buffer for acceleration structure build validation.");
        }
    }

    VmaAllocation scratch_allocation = VK_NULL_HANDLE;
    VmaAllocationInfo scratch_allocation_info = {};
    if (result == VK_SUCCESS) {
        VmaAllocationCreateInfo scratch_aci = {};
        scratch_aci.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        result = vmaAllocateMemory(device_gpuav->vmaAllocator, &scratch_mem_requirements.memoryRequirements, &scratch_aci,
                                   &scratch_allocation, &scratch_allocation_info);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device_gpuav->device),
                               "Failed to alloc scratch memory for acceleration structure build validation.");
        }
    }

    if (result == VK_SUCCESS) {
        result = DispatchBindBufferMemory(device_gpuav->device, scratch, scratch_allocation_info.deviceMemory,
                                          scratch_allocation_info.offset);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device_gpuav->device),
                               "Failed to bind scratch memory for acceleration structure build validation.");
        }
    }

    VkCommandPool command_pool = VK_NULL_HANDLE;
    if (result == VK_SUCCESS) {
        VkCommandPoolCreateInfo command_pool_ci = {};
        command_pool_ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_ci.queueFamilyIndex = 0;

        result = DispatchCreateCommandPool(device_gpuav->device, &command_pool_ci, nullptr, &command_pool);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device_gpuav->device),
                               "Failed to create command pool for acceleration structure build validation.");
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
            ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device_gpuav->device),
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
            ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device_gpuav->device),
                               "Failed to begin command buffer for acceleration structure build validation.");
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
            ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device_gpuav->device),
                               "Failed to submit command buffer for acceleration structure build validation.");
        }
    }

    if (result == VK_SUCCESS) {
        result = DispatchQueueWaitIdle(queue);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device_gpuav->device),
                               "Failed to wait for queue idle for acceleration structure build validation.");
        }
    }

    if (vbo != VK_NULL_HANDLE) {
        vmaDestroyBuffer(device_gpuav->vmaAllocator, vbo, vbo_allocation);
    }
    if (ibo != VK_NULL_HANDLE) {
        vmaDestroyBuffer(device_gpuav->vmaAllocator, ibo, ibo_allocation);
    }
    if (scratch != VK_NULL_HANDLE) {
        DispatchDestroyBuffer(device_gpuav->device, scratch, nullptr);
        vmaFreeMemory(device_gpuav->vmaAllocator, scratch_allocation);
    }
    if (command_pool != VK_NULL_HANDLE) {
        DispatchDestroyCommandPool(device_gpuav->device, command_pool, nullptr);
    }

    if (device_gpuav->debug_desc_layout == VK_NULL_HANDLE) {
        ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device_gpuav->device),
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
            ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device_gpuav->device),
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
            ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device_gpuav->device),
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
            ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device_gpuav->device),
                               "Failed to create compute pipeline for acceleration structure build validation.");
        }
    }

    if (shader_module != VK_NULL_HANDLE) {
        DispatchDestroyShaderModule(device_gpuav->device, shader_module, nullptr);
    }

    if (result == VK_SUCCESS) {
        as_validation_state.initialized = true;
        log_msg(report_data, VK_DEBUG_REPORT_INFORMATION_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                HandleToUint64(device_gpuav->device), "UNASSIGNED-GPU-Assisted Validation.",
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
        if (as_state.built && as_state.create_info.info.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV) {
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
        ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                           "Unable to allocate device memory.  Device could become unstable.");
        aborted = true;
        return;
    }

    GpuAccelerationStructureBuildValidationBuffer *mapped_validation_buffer = nullptr;
    result = vmaMapMemory(vmaAllocator, as_validation_buffer_info.validation_buffer_allocation, (void **)&mapped_validation_buffer);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                           "Unable to allocate device memory for acceleration structure build val buffer.");
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

    result =
        desc_set_manager->GetDescriptorSet(&as_validation_buffer_info.descriptor_pool, &as_validation_buffer_info.descriptor_set);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                           "Unable to get descriptor set for acceleration structure build.");
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

                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT,
                        HandleToUint64(as_validation_buffer_info.acceleration_structure), "UNASSIGNED-AccelerationStructure",
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
        ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device), strm.str().c_str());
    } else {
        // Modify the pipeline layout by:
        // 1. Copying the caller's descriptor set desc_layouts
        // 2. Fill in dummy descriptor layouts up to the max binding
        // 3. Fill in with the debug descriptor layout at the max binding slot
        cpl_state->new_layouts.reserve(adjusted_max_desc_sets);
        cpl_state->new_layouts.insert(cpl_state->new_layouts.end(), &pCreateInfo->pSetLayouts[0],
                                      &pCreateInfo->pSetLayouts[pCreateInfo->setLayoutCount]);
        for (uint32_t i = pCreateInfo->setLayoutCount; i < adjusted_max_desc_sets - 1; ++i) {
            cpl_state->new_layouts.push_back(dummy_desc_layout);
        }
        cpl_state->new_layouts.push_back(debug_desc_layout);
        cpl_state->modified_create_info.pSetLayouts = cpl_state->new_layouts.data();
        cpl_state->modified_create_info.setLayoutCount = adjusted_max_desc_sets;
    }
}

void GpuAssisted::PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout,
                                                     VkResult result) {
    ValidationStateTracker::PostCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, result);

    if (result != VK_SUCCESS) {
        ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                           "Unable to create pipeline layout.  Device could become unstable.");
        aborted = true;
    }
}

// Free the device memory and descriptor set associated with a command buffer.
void GpuAssisted::ResetCommandBuffer(VkCommandBuffer commandBuffer) {
    if (aborted) {
        return;
    }
    auto gpuav_buffer_list = GetGpuAssistedBufferInfo(commandBuffer);
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
        ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, HandleToUint64(commandBuffer),
                           "CmdWaitEvents recorded with VK_PIPELINE_STAGE_HOST_BIT set. "
                           "GPU_Assisted validation waits on queue completion. "
                           "This wait could block the host's signaling of this event, resulting in deadlock.");
    }
    return false;
}

void GpuAssisted::PostCallRecordGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                                            VkPhysicalDeviceProperties *pPhysicalDeviceProperties) {
    // There is an implicit layer that can cause this call to return 0 for maxBoundDescriptorSets - Ignore such calls
    if (enabled.gpu_validation_reserve_binding_slot && pPhysicalDeviceProperties->limits.maxBoundDescriptorSets > 0) {
        if (pPhysicalDeviceProperties->limits.maxBoundDescriptorSets > 1) {
            pPhysicalDeviceProperties->limits.maxBoundDescriptorSets -= 1;
        } else {
            log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT,
                    HandleToUint64(physicalDevice), "UNASSIGNED-GPU-Assisted Validation Setup Error.",
                    "Unable to reserve descriptor binding slot on a device with only one slot.");
        }
    }
}

void GpuAssisted::PostCallRecordGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                                             VkPhysicalDeviceProperties2 *pPhysicalDeviceProperties2) {
    // There is an implicit layer that can cause this call to return 0 for maxBoundDescriptorSets - Ignore such calls
    if (enabled.gpu_validation_reserve_binding_slot && pPhysicalDeviceProperties2->properties.limits.maxBoundDescriptorSets > 0) {
        if (pPhysicalDeviceProperties2->properties.limits.maxBoundDescriptorSets > 1) {
            pPhysicalDeviceProperties2->properties.limits.maxBoundDescriptorSets -= 1;
        } else {
            log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT,
                    HandleToUint64(physicalDevice), "UNASSIGNED-GPU-Assisted Validation Setup Error.",
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
    PreCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, cgpl_state->pipe_state, &new_pipeline_create_infos,
                                   VK_PIPELINE_BIND_POINT_GRAPHICS);
    cgpl_state->gpu_create_infos = new_pipeline_create_infos;
    cgpl_state->pCreateInfos = reinterpret_cast<VkGraphicsPipelineCreateInfo *>(cgpl_state->gpu_create_infos.data());
}

void GpuAssisted::PreCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                      const VkComputePipelineCreateInfo *pCreateInfos,
                                                      const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                      void *ccpl_state_data) {
    std::vector<safe_VkComputePipelineCreateInfo> new_pipeline_create_infos;
    auto *ccpl_state = reinterpret_cast<create_compute_pipeline_api_state *>(ccpl_state_data);
    PreCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, ccpl_state->pipe_state, &new_pipeline_create_infos,
                                   VK_PIPELINE_BIND_POINT_COMPUTE);
    ccpl_state->gpu_create_infos = new_pipeline_create_infos;
    ccpl_state->pCreateInfos = reinterpret_cast<VkComputePipelineCreateInfo *>(ccpl_state->gpu_create_infos.data());
}

void GpuAssisted::PreCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                           const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                           const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                           void *crtpl_state_data) {
    std::vector<safe_VkRayTracingPipelineCreateInfoNV> new_pipeline_create_infos;
    auto *crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_api_state *>(crtpl_state_data);
    PreCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, crtpl_state->pipe_state, &new_pipeline_create_infos,
                                   VK_PIPELINE_BIND_POINT_RAY_TRACING_NV);
    crtpl_state->gpu_create_infos = new_pipeline_create_infos;
    crtpl_state->pCreateInfos = reinterpret_cast<VkRayTracingPipelineCreateInfoNV *>(crtpl_state->gpu_create_infos.data());
}
template <typename CreateInfo>
struct CreatePipelineTraits {};
template <>
struct CreatePipelineTraits<VkGraphicsPipelineCreateInfo> {
    using SafeType = safe_VkGraphicsPipelineCreateInfo;
    static const SafeType &GetPipelineCI(const PIPELINE_STATE *pipeline_state) { return pipeline_state->graphicsPipelineCI; }
    static uint32_t GetStageCount(const VkGraphicsPipelineCreateInfo &createInfo) { return createInfo.stageCount; }
    static VkShaderModule GetShaderModule(const VkGraphicsPipelineCreateInfo &createInfo, uint32_t stage) {
        return createInfo.pStages[stage].module;
    }
    static void SetShaderModule(SafeType *createInfo, VkShaderModule shader_module, uint32_t stage) {
        createInfo->pStages[stage].module = shader_module;
    }
};

template <>
struct CreatePipelineTraits<VkComputePipelineCreateInfo> {
    using SafeType = safe_VkComputePipelineCreateInfo;
    static const SafeType &GetPipelineCI(const PIPELINE_STATE *pipeline_state) { return pipeline_state->computePipelineCI; }
    static uint32_t GetStageCount(const VkComputePipelineCreateInfo &createInfo) { return 1; }
    static VkShaderModule GetShaderModule(const VkComputePipelineCreateInfo &createInfo, uint32_t stage) {
        return createInfo.stage.module;
    }
    static void SetShaderModule(SafeType *createInfo, VkShaderModule shader_module, uint32_t stage) {
        assert(stage == 0);
        createInfo->stage.module = shader_module;
    }
};
template <>
struct CreatePipelineTraits<VkRayTracingPipelineCreateInfoNV> {
    using SafeType = safe_VkRayTracingPipelineCreateInfoNV;
    static const SafeType &GetPipelineCI(const PIPELINE_STATE *pipeline_state) { return pipeline_state->raytracingPipelineCI; }
    static uint32_t GetStageCount(const VkRayTracingPipelineCreateInfoNV &createInfo) { return createInfo.stageCount; }
    static VkShaderModule GetShaderModule(const VkRayTracingPipelineCreateInfoNV &createInfo, uint32_t stage) {
        return createInfo.pStages[stage].module;
    }
    static void SetShaderModule(SafeType *createInfo, VkShaderModule shader_module, uint32_t stage) {
        createInfo->pStages[stage].module = shader_module;
    }
};

// Examine the pipelines to see if they use the debug descriptor set binding index.
// If any do, create new non-instrumented shader modules and use them to replace the instrumented
// shaders in the pipeline.  Return the (possibly) modified create infos to the caller.
template <typename CreateInfo, typename SafeCreateInfo>
void GpuAssisted::PreCallRecordPipelineCreations(uint32_t count, const CreateInfo *pCreateInfos,
                                                 const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                 std::vector<std::shared_ptr<PIPELINE_STATE>> &pipe_state,
                                                 std::vector<SafeCreateInfo> *new_pipeline_create_infos,
                                                 const VkPipelineBindPoint bind_point) {
    using Accessor = CreatePipelineTraits<CreateInfo>;
    if (bind_point != VK_PIPELINE_BIND_POINT_GRAPHICS && bind_point != VK_PIPELINE_BIND_POINT_COMPUTE &&
        bind_point != VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
        return;
    }

    // Walk through all the pipelines, make a copy of each and flag each pipeline that contains a shader that uses the debug
    // descriptor set index.
    for (uint32_t pipeline = 0; pipeline < count; ++pipeline) {
        uint32_t stageCount = Accessor::GetStageCount(pCreateInfos[pipeline]);
        new_pipeline_create_infos->push_back(Accessor::GetPipelineCI(pipe_state[pipeline].get()));

        bool replace_shaders = false;
        if (pipe_state[pipeline]->active_slots.find(desc_set_bind_index) != pipe_state[pipeline]->active_slots.end()) {
            replace_shaders = true;
        }
        // If the app requests all available sets, the pipeline layout was not modified at pipeline layout creation and the already
        // instrumented shaders need to be replaced with uninstrumented shaders
        if (pipe_state[pipeline]->pipeline_layout->set_layouts.size() >= adjusted_max_desc_sets) {
            replace_shaders = true;
        }

        if (replace_shaders) {
            for (uint32_t stage = 0; stage < stageCount; ++stage) {
                const SHADER_MODULE_STATE *shader = GetShaderModuleState(Accessor::GetShaderModule(pCreateInfos[pipeline], stage));

                VkShaderModuleCreateInfo create_info = {};
                VkShaderModule shader_module;
                create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                create_info.pCode = shader->words.data();
                create_info.codeSize = shader->words.size() * sizeof(uint32_t);
                VkResult result = DispatchCreateShaderModule(device, &create_info, pAllocator, &shader_module);
                if (result == VK_SUCCESS) {
                    Accessor::SetShaderModule(&(*new_pipeline_create_infos)[pipeline], shader_module, stage);
                } else {
                    uint64_t moduleHandle = HandleToUint64(Accessor::GetShaderModule(pCreateInfos[pipeline], stage));
                    ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, moduleHandle,
                                       "Unable to replace instrumented shader with non-instrumented one.  "
                                       "Device could become unstable.");
                }
            }
        }
    }
}

void GpuAssisted::PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                        const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                        const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                        VkResult result, void *cgpl_state_data) {
    ValidationStateTracker::PostCallRecordCreateGraphicsPipelines(device, pipelineCache, count, pCreateInfos, pAllocator,
                                                                  pPipelines, result, cgpl_state_data);
    PostCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void GpuAssisted::PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                       const VkComputePipelineCreateInfo *pCreateInfos,
                                                       const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                       VkResult result, void *ccpl_state_data) {
    ValidationStateTracker::PostCallRecordCreateComputePipelines(device, pipelineCache, count, pCreateInfos, pAllocator, pPipelines,
                                                                 result, ccpl_state_data);
    PostCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, VK_PIPELINE_BIND_POINT_COMPUTE);
}

void GpuAssisted::PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                            const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                            const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                            VkResult result, void *crtpl_state_data) {
    ValidationStateTracker::PostCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, count, pCreateInfos, pAllocator,
                                                                      pPipelines, result, crtpl_state_data);
    PostCallRecordPipelineCreations(count, pCreateInfos, pAllocator, pPipelines, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV);
}

// For every pipeline:
// - For every shader in a pipeline:
//   - If the shader had to be replaced in PreCallRecord (because the pipeline is using the debug desc set index):
//     - Destroy it since it has been bound into the pipeline by now.  This is our only chance to delete it.
//   - Track the shader in the shader_map
//   - Save the shader binary if it contains debug code
template <typename CreateInfo>
void GpuAssisted::PostCallRecordPipelineCreations(const uint32_t count, const CreateInfo *pCreateInfos,
                                                  const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                  const VkPipelineBindPoint bind_point) {
    using Accessor = CreatePipelineTraits<CreateInfo>;
    if (bind_point != VK_PIPELINE_BIND_POINT_GRAPHICS && bind_point != VK_PIPELINE_BIND_POINT_COMPUTE &&
        bind_point != VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
        return;
    }
    for (uint32_t pipeline = 0; pipeline < count; ++pipeline) {
        auto pipeline_state = ValidationStateTracker::GetPipelineState(pPipelines[pipeline]);
        if (nullptr == pipeline_state) continue;

        uint32_t stageCount = 0;
        if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
            stageCount = pipeline_state->graphicsPipelineCI.stageCount;
        } else if (bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
            stageCount = 1;
        } else if (bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
            stageCount = pipeline_state->raytracingPipelineCI.stageCount;
        } else {
            assert(false);
        }

        for (uint32_t stage = 0; stage < stageCount; ++stage) {
            if (pipeline_state->active_slots.find(desc_set_bind_index) != pipeline_state->active_slots.end()) {
                DispatchDestroyShaderModule(device, Accessor::GetShaderModule(pCreateInfos[pipeline], stage), pAllocator);
            }

            const SHADER_MODULE_STATE *shader_state = nullptr;
            if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
                shader_state = GetShaderModuleState(pipeline_state->graphicsPipelineCI.pStages[stage].module);
            } else if (bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
                assert(stage == 0);
                shader_state = GetShaderModuleState(pipeline_state->computePipelineCI.stage.module);
            } else if (bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
                shader_state = GetShaderModuleState(pipeline_state->raytracingPipelineCI.pStages[stage].module);
            } else {
                assert(false);
            }

            std::vector<unsigned int> code;
            // Save the shader binary if debug info is present.
            // The core_validation ShaderModule tracker saves the binary too, but discards it when the ShaderModule
            // is destroyed.  Applications may destroy ShaderModules after they are placed in a pipeline and before
            // the pipeline is used, so we have to keep another copy.
            if (shader_state && shader_state->has_valid_spirv) {  // really checking for presense of SPIR-V code.
                for (auto insn : *shader_state) {
                    if (insn.opcode() == spv::OpLine) {
                        code = shader_state->words;
                        break;
                    }
                }
            }
            shader_map[shader_state->gpu_validation_shader_id].pipeline = pipeline_state->pipeline;
            // Be careful to use the originally bound (instrumented) shader here, even if PreCallRecord had to back it
            // out with a non-instrumented shader.  The non-instrumented shader (found in pCreateInfo) was destroyed above.
            VkShaderModule shader_module = VK_NULL_HANDLE;
            if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
                shader_module = pipeline_state->graphicsPipelineCI.pStages[stage].module;
            } else if (bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
                assert(stage == 0);
                shader_module = pipeline_state->computePipelineCI.stage.module;
            } else if (bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
                shader_module = pipeline_state->raytracingPipelineCI.pStages[stage].module;
            } else {
                assert(false);
            }
            shader_map[shader_state->gpu_validation_shader_id].shader_module = shader_module;
            shader_map[shader_state->gpu_validation_shader_id].pgm = std::move(code);
        }
    }
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

    // Load original shader SPIR-V
    uint32_t num_words = static_cast<uint32_t>(pCreateInfo->codeSize / 4);
    new_pgm.clear();
    new_pgm.reserve(num_words);
    new_pgm.insert(new_pgm.end(), &pCreateInfo->pCode[0], &pCreateInfo->pCode[num_words]);

    // Call the optimizer to instrument the shader.
    // Use the unique_shader_module_id as a shader ID so we can look up its handle later in the shader_map.
    // If descriptor indexing is enabled, enable length checks and updated descriptor checks
    const bool descriptor_indexing = device_extensions.vk_ext_descriptor_indexing;
    using namespace spvtools;
    spv_target_env target_env = SPV_ENV_VULKAN_1_1;
    Optimizer optimizer(target_env);
    optimizer.RegisterPass(
        CreateInstBindlessCheckPass(desc_set_bind_index, unique_shader_module_id, descriptor_indexing, descriptor_indexing, 2));
    optimizer.RegisterPass(CreateAggressiveDCEPass());
    if (device_extensions.vk_ext_buffer_device_address && shaderInt64)
        optimizer.RegisterPass(CreateInstBuffAddrCheckPass(desc_set_bind_index, unique_shader_module_id));
    bool pass = optimizer.Run(new_pgm.data(), new_pgm.size(), &new_pgm);
    if (!pass) {
        ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, VK_NULL_HANDLE,
                           "Failure to instrument shader.  Proceeding with non-instrumented shader.");
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

// Generate the stage-specific part of the message.
static void GenerateStageMessage(const uint32_t *debug_record, std::string &msg) {
    using namespace spvtools;
    std::ostringstream strm;
    switch (debug_record[kInstCommonOutStageIdx]) {
        case spv::ExecutionModelVertex: {
            strm << "Stage = Vertex. Vertex Index = " << debug_record[kInstVertOutVertexIndex]
                 << " Instance Index = " << debug_record[kInstVertOutInstanceIndex] << ". ";
        } break;
        case spv::ExecutionModelTessellationControl: {
            strm << "Stage = Tessellation Control.  Invocation ID = " << debug_record[kInstTessCtlOutInvocationId]
                 << ", Primitive ID = " << debug_record[kInstTessCtlOutPrimitiveId];
        } break;
        case spv::ExecutionModelTessellationEvaluation: {
            strm << "Stage = Tessellation Eval.  Primitive ID = " << debug_record[kInstTessEvalOutPrimitiveId]
                 << ", TessCoord (u, v) = (" << debug_record[kInstTessEvalOutTessCoordU] << ", "
                 << debug_record[kInstTessEvalOutTessCoordV] << "). ";
        } break;
        case spv::ExecutionModelGeometry: {
            strm << "Stage = Geometry.  Primitive ID = " << debug_record[kInstGeomOutPrimitiveId]
                 << " Invocation ID = " << debug_record[kInstGeomOutInvocationId] << ". ";
        } break;
        case spv::ExecutionModelFragment: {
            strm << "Stage = Fragment.  Fragment coord (x,y) = ("
                 << *reinterpret_cast<const float *>(&debug_record[kInstFragOutFragCoordX]) << ", "
                 << *reinterpret_cast<const float *>(&debug_record[kInstFragOutFragCoordY]) << "). ";
        } break;
        case spv::ExecutionModelGLCompute: {
            strm << "Stage = Compute.  Global invocation ID (x, y, z) = (" << debug_record[kInstCompOutGlobalInvocationIdX] << ", "
                 << debug_record[kInstCompOutGlobalInvocationIdY] << ", " << debug_record[kInstCompOutGlobalInvocationIdZ] << " )";
        } break;
        case spv::ExecutionModelRayGenerationNV: {
            strm << "Stage = Ray Generation.  Global Launch ID (x,y,z) = (" << debug_record[kInstRayTracingOutLaunchIdX] << ", "
                 << debug_record[kInstRayTracingOutLaunchIdY] << ", " << debug_record[kInstRayTracingOutLaunchIdZ] << "). ";
        } break;
        case spv::ExecutionModelIntersectionNV: {
            strm << "Stage = Intersection.  Global Launch ID (x,y,z) = (" << debug_record[kInstRayTracingOutLaunchIdX] << ", "
                 << debug_record[kInstRayTracingOutLaunchIdY] << ", " << debug_record[kInstRayTracingOutLaunchIdZ] << "). ";
        } break;
        case spv::ExecutionModelAnyHitNV: {
            strm << "Stage = Any Hit.  Global Launch ID (x,y,z) = (" << debug_record[kInstRayTracingOutLaunchIdX] << ", "
                 << debug_record[kInstRayTracingOutLaunchIdY] << ", " << debug_record[kInstRayTracingOutLaunchIdZ] << "). ";
        } break;
        case spv::ExecutionModelClosestHitNV: {
            strm << "Stage = Closest Hit.  Global Launch ID (x,y,z) = (" << debug_record[kInstRayTracingOutLaunchIdX] << ", "
                 << debug_record[kInstRayTracingOutLaunchIdY] << ", " << debug_record[kInstRayTracingOutLaunchIdZ] << "). ";
        } break;
        case spv::ExecutionModelMissNV: {
            strm << "Stage = Miss.  Global Launch ID (x,y,z) = (" << debug_record[kInstRayTracingOutLaunchIdX] << ", "
                 << debug_record[kInstRayTracingOutLaunchIdY] << ", " << debug_record[kInstRayTracingOutLaunchIdZ] << "). ";
        } break;
        case spv::ExecutionModelCallableNV: {
            strm << "Stage = Callable.  Global Launch ID (x,y,z) = (" << debug_record[kInstRayTracingOutLaunchIdX] << ", "
                 << debug_record[kInstRayTracingOutLaunchIdY] << ", " << debug_record[kInstRayTracingOutLaunchIdZ] << "). ";
        } break;
        default: {
            strm << "Internal Error (unexpected stage = " << debug_record[kInstCommonOutStageIdx] << "). ";
            assert(false);
        } break;
    }
    msg = strm.str();
}

// Generate the part of the message describing the violation.
static void GenerateValidationMessage(const uint32_t *debug_record, std::string &msg, std::string &vuid_msg) {
    using namespace spvtools;
    std::ostringstream strm;
    switch (debug_record[kInst2ValidationOutError]) {
        case kInstErrorBindlessBounds: {
            strm << "Index of " << debug_record[kInst2BindlessBoundsOutDescIndex] << " used to index descriptor array of length "
                 << debug_record[kInst2BindlessBoundsOutDescBound] << ". ";
            vuid_msg = "UNASSIGNED-Descriptor index out of bounds";
        } break;
        case kInstErrorBindlessUninit: {
            strm << "Descriptor index " << debug_record[kInst2BindlessUninitOutDescIndex] << " is uninitialized. ";
            vuid_msg = "UNASSIGNED-Descriptor uninitialized";
        } break;
        case kInstErrorBuffAddrUnallocRef: {
            uint64_t *ptr = (uint64_t *)&debug_record[kInst2BuffAddrUnallocOutDescPtrLo];
            strm << "Device address 0x" << std::hex << *ptr << " access out of bounds. ";
            vuid_msg = "UNASSIGNED-Device address out of bounds";
        } break;
        default: {
            strm << "Internal Error (unexpected error type = " << debug_record[kInst2ValidationOutError] << "). ";
            vuid_msg = "UNASSIGNED-Internal Error";
            assert(false);
        } break;
    }
    msg = strm.str();
}

static std::string LookupDebugUtilsName(const debug_report_data *report_data, const uint64_t object) {
    auto object_label = report_data->DebugReportGetUtilsObjectName(object);
    if (object_label != "") {
        object_label = "(" + object_label + ")";
    }
    return object_label;
}

// Generate message from the common portion of the debug report record.
static void GenerateCommonMessage(const debug_report_data *report_data, const CMD_BUFFER_STATE *cb_node,
                                  const uint32_t *debug_record, const VkShaderModule shader_module_handle,
                                  const VkPipeline pipeline_handle, const VkPipelineBindPoint pipeline_bind_point,
                                  const uint32_t operation_index, std::string &msg) {
    using namespace spvtools;
    std::ostringstream strm;
    if (shader_module_handle == VK_NULL_HANDLE) {
        strm << std::hex << std::showbase << "Internal Error: Unable to locate information for shader used in command buffer "
             << LookupDebugUtilsName(report_data, HandleToUint64(cb_node->commandBuffer)) << "("
             << HandleToUint64(cb_node->commandBuffer) << "). ";
        assert(true);
    } else {
        strm << std::hex << std::showbase << "Command buffer "
             << LookupDebugUtilsName(report_data, HandleToUint64(cb_node->commandBuffer)) << "("
             << HandleToUint64(cb_node->commandBuffer) << "). ";
        if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
            strm << "Draw ";
        } else if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
            strm << "Compute ";
        } else if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
            strm << "Ray Trace ";
        } else {
            assert(false);
            strm << "Unknown Pipeline Operation ";
        }
        strm << "Index " << operation_index << ". "
             << "Pipeline " << LookupDebugUtilsName(report_data, HandleToUint64(pipeline_handle)) << "("
             << HandleToUint64(pipeline_handle) << "). "
             << "Shader Module " << LookupDebugUtilsName(report_data, HandleToUint64(shader_module_handle)) << "("
             << HandleToUint64(shader_module_handle) << "). ";
    }
    strm << std::dec << std::noshowbase;
    strm << "Shader Instruction Index = " << debug_record[kInstCommonOutInstructionIdx] << ". ";
    msg = strm.str();
}

// Read the contents of the SPIR-V OpSource instruction and any following continuation instructions.
// Split the single string into a vector of strings, one for each line, for easier processing.
static void ReadOpSource(const SHADER_MODULE_STATE &shader, const uint32_t reported_file_id,
                         std::vector<std::string> &opsource_lines) {
    for (auto insn : shader) {
        if ((insn.opcode() == spv::OpSource) && (insn.len() >= 5) && (insn.word(3) == reported_file_id)) {
            std::istringstream in_stream;
            std::string cur_line;
            in_stream.str((char *)&insn.word(4));
            while (std::getline(in_stream, cur_line)) {
                opsource_lines.push_back(cur_line);
            }
            while ((++insn).opcode() == spv::OpSourceContinued) {
                in_stream.str((char *)&insn.word(1));
                while (std::getline(in_stream, cur_line)) {
                    opsource_lines.push_back(cur_line);
                }
            }
            break;
        }
    }
}

// The task here is to search the OpSource content to find the #line directive with the
// line number that is closest to, but still prior to the reported error line number and
// still within the reported filename.
// From this known position in the OpSource content we can add the difference between
// the #line line number and the reported error line number to determine the location
// in the OpSource content of the reported error line.
//
// Considerations:
// - Look only at #line directives that specify the reported_filename since
//   the reported error line number refers to its location in the reported filename.
// - If a #line directive does not have a filename, the file is the reported filename, or
//   the filename found in a prior #line directive.  (This is C-preprocessor behavior)
// - It is possible (e.g., inlining) for blocks of code to get shuffled out of their
//   original order and the #line directives are used to keep the numbering correct.  This
//   is why we need to examine the entire contents of the source, instead of leaving early
//   when finding a #line line number larger than the reported error line number.
//

// GCC 4.8 has a problem with std::regex that is fixed in GCC 4.9.  Provide fallback code for 4.8
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

#if defined(__GNUC__) && GCC_VERSION < 40900
static bool GetLineAndFilename(const std::string string, uint32_t *linenumber, std::string &filename) {
    // # line <linenumber> "<filename>" or
    // #line <linenumber> "<filename>"
    std::vector<std::string> tokens;
    std::stringstream stream(string);
    std::string temp;
    uint32_t line_index = 0;

    while (stream >> temp) tokens.push_back(temp);
    auto size = tokens.size();
    if (size > 1) {
        if (tokens[0] == "#" && tokens[1] == "line") {
            line_index = 2;
        } else if (tokens[0] == "#line") {
            line_index = 1;
        }
    }
    if (0 == line_index) return false;
    *linenumber = std::stoul(tokens[line_index]);
    uint32_t filename_index = line_index + 1;
    // Remove enclosing double quotes around filename
    if (size > filename_index) filename = tokens[filename_index].substr(1, tokens[filename_index].size() - 2);
    return true;
}
#else
static bool GetLineAndFilename(const std::string string, uint32_t *linenumber, std::string &filename) {
    static const std::regex line_regex(  // matches #line directives
        "^"                              // beginning of line
        "\\s*"                           // optional whitespace
        "#"                              // required text
        "\\s*"                           // optional whitespace
        "line"                           // required text
        "\\s+"                           // required whitespace
        "([0-9]+)"                       // required first capture - line number
        "(\\s+)?"                        // optional second capture - whitespace
        "(\".+\")?"                      // optional third capture - quoted filename with at least one char inside
        ".*");                           // rest of line (needed when using std::regex_match since the entire line is tested)

    std::smatch captures;

    bool found_line = std::regex_match(string, captures, line_regex);
    if (!found_line) return false;

    // filename is optional and considered found only if the whitespace and the filename are captured
    if (captures[2].matched && captures[3].matched) {
        // Remove enclosing double quotes.  The regex guarantees the quotes and at least one char.
        filename = captures[3].str().substr(1, captures[3].str().size() - 2);
    }
    *linenumber = std::stoul(captures[1]);
    return true;
}
#endif  // GCC_VERSION

// Extract the filename, line number, and column number from the correct OpLine and build a message string from it.
// Scan the source (from OpSource) to find the line of source at the reported line number and place it in another message string.
static void GenerateSourceMessages(const std::vector<unsigned int> &pgm, const uint32_t *debug_record, std::string &filename_msg,
                                   std::string &source_msg) {
    using namespace spvtools;
    std::ostringstream filename_stream;
    std::ostringstream source_stream;
    SHADER_MODULE_STATE shader;
    shader.words = pgm;
    // Find the OpLine just before the failing instruction indicated by the debug info.
    // SPIR-V can only be iterated in the forward direction due to its opcode/length encoding.
    uint32_t instruction_index = 0;
    uint32_t reported_file_id = 0;
    uint32_t reported_line_number = 0;
    uint32_t reported_column_number = 0;
    if (shader.words.size() > 0) {
        for (auto insn : shader) {
            if (insn.opcode() == spv::OpLine) {
                reported_file_id = insn.word(1);
                reported_line_number = insn.word(2);
                reported_column_number = insn.word(3);
            }
            if (instruction_index == debug_record[kInstCommonOutInstructionIdx]) {
                break;
            }
            instruction_index++;
        }
    }
    // Create message with file information obtained from the OpString pointed to by the discovered OpLine.
    std::string reported_filename;
    if (reported_file_id == 0) {
        filename_stream
            << "Unable to find SPIR-V OpLine for source information.  Build shader with debug info to get source information.";
    } else {
        bool found_opstring = false;
        for (auto insn : shader) {
            if ((insn.opcode() == spv::OpString) && (insn.len() >= 3) && (insn.word(1) == reported_file_id)) {
                found_opstring = true;
                reported_filename = (char *)&insn.word(2);
                if (reported_filename.empty()) {
                    filename_stream << "Shader validation error occurred at line " << reported_line_number;
                } else {
                    filename_stream << "Shader validation error occurred in file: " << reported_filename << " at line "
                                    << reported_line_number;
                }
                if (reported_column_number > 0) {
                    filename_stream << ", column " << reported_column_number;
                }
                filename_stream << ".";
                break;
            }
        }
        if (!found_opstring) {
            filename_stream << "Unable to find SPIR-V OpString for file id " << reported_file_id << " from OpLine instruction.";
        }
    }
    filename_msg = filename_stream.str();

    // Create message to display source code line containing error.
    if ((reported_file_id != 0)) {
        // Read the source code and split it up into separate lines.
        std::vector<std::string> opsource_lines;
        ReadOpSource(shader, reported_file_id, opsource_lines);
        // Find the line in the OpSource content that corresponds to the reported error file and line.
        if (!opsource_lines.empty()) {
            uint32_t saved_line_number = 0;
            std::string current_filename = reported_filename;  // current "preprocessor" filename state.
            std::vector<std::string>::size_type saved_opsource_offset = 0;
            bool found_best_line = false;
            for (auto it = opsource_lines.begin(); it != opsource_lines.end(); ++it) {
                uint32_t parsed_line_number;
                std::string parsed_filename;
                bool found_line = GetLineAndFilename(*it, &parsed_line_number, parsed_filename);
                if (!found_line) continue;

                bool found_filename = parsed_filename.size() > 0;
                if (found_filename) {
                    current_filename = parsed_filename;
                }
                if ((!found_filename) || (current_filename == reported_filename)) {
                    // Update the candidate best line directive, if the current one is prior and closer to the reported line
                    if (reported_line_number >= parsed_line_number) {
                        if (!found_best_line ||
                            (reported_line_number - parsed_line_number <= reported_line_number - saved_line_number)) {
                            saved_line_number = parsed_line_number;
                            saved_opsource_offset = std::distance(opsource_lines.begin(), it);
                            found_best_line = true;
                        }
                    }
                }
            }
            if (found_best_line) {
                assert(reported_line_number >= saved_line_number);
                std::vector<std::string>::size_type opsource_index =
                    (reported_line_number - saved_line_number) + 1 + saved_opsource_offset;
                if (opsource_index < opsource_lines.size()) {
                    source_stream << "\n" << reported_line_number << ": " << opsource_lines[opsource_index].c_str();
                } else {
                    source_stream << "Internal error: calculated source line of " << opsource_index << " for source size of "
                                  << opsource_lines.size() << " lines.";
                }
            } else {
                source_stream << "Unable to find suitable #line directive in SPIR-V OpSource.";
            }
        } else {
            source_stream << "Unable to find SPIR-V OpSource.";
        }
    }
    source_msg = source_stream.str();
}

// Pull together all the information from the debug record to build the error message strings,
// and then assemble them into a single message string.
// Retrieve the shader program referenced by the unique shader ID provided in the debug record.
// We had to keep a copy of the shader program with the same lifecycle as the pipeline to make
// sure it is available when the pipeline is submitted.  (The ShaderModule tracking object also
// keeps a copy, but it can be destroyed after the pipeline is created and before it is submitted.)
//
void GpuAssisted::AnalyzeAndReportError(CMD_BUFFER_STATE *cb_node, VkQueue queue, VkPipelineBindPoint pipeline_bind_point,
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
    GenerateStageMessage(debug_record, stage_message);
    GenerateCommonMessage(report_data, cb_node, debug_record, shader_module_handle, pipeline_handle, pipeline_bind_point,
                          operation_index, common_message);
    GenerateSourceMessages(pgm, debug_record, filename_message, source_message);
    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT, HandleToUint64(queue),
            vuid_msg.c_str(), "%s %s %s %s%s", validation_message.c_str(), common_message.c_str(), stage_message.c_str(),
            filename_message.c_str(), source_message.c_str());
    // The debug record at word kInstCommonOutSize is the number of words in the record
    // written by the shader.  Clear the entire record plus the total_words word at the start.
    const uint32_t words_to_clear = 1 + std::min(debug_record[kInstCommonOutSize], (uint32_t)kInst2MaxOutCnt);
    memset(debug_output_buffer, 0, sizeof(uint32_t) * words_to_clear);
}

// For the given command buffer, map its debug data buffers and read their contents for analysis.
void GpuAssisted::ProcessInstrumentationBuffer(VkQueue queue, CMD_BUFFER_STATE *cb_node) {
    if (cb_node && (cb_node->hasDrawCmd || cb_node->hasTraceRaysCmd || cb_node->hasDispatchCmd)) {
        auto gpu_buffer_list = GetGpuAssistedBufferInfo(cb_node->commandBuffer);
        uint32_t draw_index = 0;
        uint32_t compute_index = 0;
        uint32_t ray_trace_index = 0;

        for (auto &buffer_info : gpu_buffer_list) {
            char *pData;
            VkResult result = vmaMapMemory(vmaAllocator, buffer_info.output_mem_block.allocation, (void **)&pData);
            // Analyze debug output buffer
            if (result == VK_SUCCESS) {
                uint32_t operation_index = 0;
                if (buffer_info.pipeline_bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
                    operation_index = draw_index;
                } else if (buffer_info.pipeline_bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
                    operation_index = compute_index;
                } else if (buffer_info.pipeline_bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
                    operation_index = ray_trace_index;
                } else {
                    assert(false);
                }

                AnalyzeAndReportError(cb_node, queue, buffer_info.pipeline_bind_point, operation_index, (uint32_t *)pData);
                vmaUnmapMemory(vmaAllocator, buffer_info.output_mem_block.allocation);
            }

            if (buffer_info.pipeline_bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
                draw_index++;
            } else if (buffer_info.pipeline_bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
                compute_index++;
            } else if (buffer_info.pipeline_bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
                ray_trace_index++;
            } else {
                assert(false);
            }
        }
    }
}

// For the given command buffer, map its debug data buffers and update the status of any update after bind descriptors
void GpuAssisted::UpdateInstrumentationBuffer(CMD_BUFFER_STATE *cb_node) {
    auto gpu_buffer_list = GetGpuAssistedBufferInfo(cb_node->commandBuffer);
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

// Submit a memory barrier on graphics queues.
// Lazy-create and record the needed command buffer.
void GpuAssisted::SubmitBarrier(VkQueue queue) {
    auto queue_barrier_command_info_it = queue_barrier_command_infos.emplace(queue, GpuAssistedQueueBarrierCommandInfo{});
    if (queue_barrier_command_info_it.second) {
        GpuAssistedQueueBarrierCommandInfo &queue_barrier_command_info = queue_barrier_command_info_it.first->second;

        uint32_t queue_family_index = 0;

        auto queue_state_it = queueMap.find(queue);
        if (queue_state_it != queueMap.end()) {
            queue_family_index = queue_state_it->second.queueFamilyIndex;
        }

        VkResult result = VK_SUCCESS;

        VkCommandPoolCreateInfo pool_create_info = {};
        pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_create_info.queueFamilyIndex = queue_family_index;
        result = DispatchCreateCommandPool(device, &pool_create_info, nullptr, &queue_barrier_command_info.barrier_command_pool);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                               "Unable to create command pool for barrier CB.");
            queue_barrier_command_info.barrier_command_pool = VK_NULL_HANDLE;
            return;
        }

        VkCommandBufferAllocateInfo buffer_alloc_info = {};
        buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        buffer_alloc_info.commandPool = queue_barrier_command_info.barrier_command_pool;
        buffer_alloc_info.commandBufferCount = 1;
        buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        result = DispatchAllocateCommandBuffers(device, &buffer_alloc_info, &queue_barrier_command_info.barrier_command_buffer);
        if (result != VK_SUCCESS) {
            ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                               "Unable to create barrier command buffer.");
            DispatchDestroyCommandPool(device, queue_barrier_command_info.barrier_command_pool, nullptr);
            queue_barrier_command_info.barrier_command_pool = VK_NULL_HANDLE;
            queue_barrier_command_info.barrier_command_buffer = VK_NULL_HANDLE;
            return;
        }

        // Hook up command buffer dispatch
        vkSetDeviceLoaderData(device, queue_barrier_command_info.barrier_command_buffer);

        // Record a global memory barrier to force availability of device memory operations to the host domain.
        VkCommandBufferBeginInfo command_buffer_begin_info = {};
        command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        result = DispatchBeginCommandBuffer(queue_barrier_command_info.barrier_command_buffer, &command_buffer_begin_info);
        if (result == VK_SUCCESS) {
            VkMemoryBarrier memory_barrier = {};
            memory_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
            memory_barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
            memory_barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;

            DispatchCmdPipelineBarrier(queue_barrier_command_info.barrier_command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                       VK_PIPELINE_STAGE_HOST_BIT, 0, 1, &memory_barrier, 0, nullptr, 0, nullptr);
            DispatchEndCommandBuffer(queue_barrier_command_info.barrier_command_buffer);
        }
    }

    GpuAssistedQueueBarrierCommandInfo &queue_barrier_command_info = queue_barrier_command_info_it.first->second;
    if (queue_barrier_command_info.barrier_command_buffer != VK_NULL_HANDLE) {
        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &queue_barrier_command_info.barrier_command_buffer;
        DispatchQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
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

    if (aborted) return;
    bool buffers_present = false;
    // Don't QueueWaitIdle if there's nothing to process
    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        const VkSubmitInfo *submit = &pSubmits[submit_idx];
        for (uint32_t i = 0; i < submit->commandBufferCount; i++) {
            auto cb_node = GetCBState(submit->pCommandBuffers[i]);
            if (GetGpuAssistedBufferInfo(cb_node->commandBuffer).size() || cb_node->hasBuildAccelerationStructureCmd)
                buffers_present = true;
            for (auto secondaryCmdBuffer : cb_node->linkedCommandBuffers) {
                if (GetGpuAssistedBufferInfo(secondaryCmdBuffer->commandBuffer).size() || cb_node->hasBuildAccelerationStructureCmd)
                    buffers_present = true;
            }
        }
    }
    if (!buffers_present) return;

    SubmitBarrier(queue);

    DispatchQueueWaitIdle(queue);

    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        const VkSubmitInfo *submit = &pSubmits[submit_idx];
        for (uint32_t i = 0; i < submit->commandBufferCount; i++) {
            auto cb_node = GetCBState(submit->pCommandBuffers[i]);
            ProcessInstrumentationBuffer(queue, cb_node);
            ProcessAccelerationStructureBuildValidationBuffer(queue, cb_node);
            for (auto secondaryCmdBuffer : cb_node->linkedCommandBuffers) {
                ProcessInstrumentationBuffer(queue, secondaryCmdBuffer);
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

void GpuAssisted::PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) {
    AllocateValidationResources(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE);
}

void GpuAssisted::PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
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

void GpuAssisted::AllocateValidationResources(const VkCommandBuffer cmd_buffer, const VkPipelineBindPoint bind_point) {
    if (bind_point != VK_PIPELINE_BIND_POINT_GRAPHICS && bind_point != VK_PIPELINE_BIND_POINT_COMPUTE &&
        bind_point != VK_PIPELINE_BIND_POINT_RAY_TRACING_NV) {
        return;
    }
    VkResult result;

    if (aborted) return;

    std::vector<VkDescriptorSet> desc_sets;
    VkDescriptorPool desc_pool = VK_NULL_HANDLE;
    result = desc_set_manager->GetDescriptorSets(1, &desc_pool, &desc_sets);
    assert(result == VK_SUCCESS);
    if (result != VK_SUCCESS) {
        ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                           "Unable to allocate descriptor sets.  Device could become unstable.");
        aborted = true;
        return;
    }

    VkDescriptorBufferInfo output_desc_buffer_info = {};
    output_desc_buffer_info.range = output_buffer_size;

    auto cb_node = GetCBState(cmd_buffer);
    if (!cb_node) {
        ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device), "Unrecognized command buffer");
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
        ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                           "Unable to allocate device memory.  Device could become unstable.");
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
    if (number_of_sets > 0 && device_extensions.vk_ext_descriptor_indexing) {
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
                        log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT,
                                VK_NULL_HANDLE, "UNASSIGNED-GPU-Assisted Validation Warning",
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
            ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                               "Unable to allocate device memory.  Device could become unstable.");
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

    if (number_of_sets > 0 && device_extensions.vk_ext_buffer_device_address && buffer_map.size() && shaderInt64) {
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
            ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device),
                               "Unable to allocate device memory.  Device could become unstable.");
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
        if (pipeline_state && (pipeline_state->pipeline_layout->set_layouts.size() <= desc_set_bind_index)) {
            DispatchCmdBindDescriptorSets(cmd_buffer, bind_point, pipeline_state->pipeline_layout->layout, desc_set_bind_index, 1,
                                          desc_sets.data(), 0, nullptr);
        }
        // Record buffer and memory info in CB state tracking
        GetGpuAssistedBufferInfo(cmd_buffer)
            .emplace_back(output_block, di_input_block, bda_input_block, desc_sets[0], desc_pool, bind_point);
    } else {
        ReportSetupProblem(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, HandleToUint64(device), "Unable to find pipeline state");
        vmaDestroyBuffer(vmaAllocator, di_input_block.buffer, di_input_block.allocation);
        vmaDestroyBuffer(vmaAllocator, bda_input_block.buffer, bda_input_block.allocation);
        vmaDestroyBuffer(vmaAllocator, output_block.buffer, output_block.allocation);
        aborted = true;
        return;
    }
}
