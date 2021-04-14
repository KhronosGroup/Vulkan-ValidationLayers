/* Copyright (c) 2018-2021 The Khronos Group Inc.
 * Copyright (c) 2018-2021 Valve Corporation
 * Copyright (c) 2018-2021 LunarG, Inc.
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

#pragma once

#include "chassis.h"
#include "state_tracker.h"
#include "vk_mem_alloc.h"
#include "gpu_utils.h"
class GpuAssisted;

struct GpuAssistedDeviceMemoryBlock {
    VkBuffer buffer;
    VmaAllocation allocation;
    layer_data::unordered_map<uint32_t, const cvdescriptorset::Descriptor*> update_at_submit;
};

struct GpuAssistedPreDrawResources {
    GpuAssistedDeviceMemoryBlock new_count_buffer;
    VkDescriptorPool desc_pool;
    VkDescriptorSet desc_set;
    VkBuffer buffer;
    VkDeviceSize offset;
    uint32_t stride;
    VkDeviceSize buf_size;
};

struct GpuAssistedBufferInfo {
    GpuAssistedDeviceMemoryBlock output_mem_block;
    GpuAssistedDeviceMemoryBlock di_input_mem_block;   // Descriptor Indexing input
    GpuAssistedDeviceMemoryBlock bda_input_mem_block;  // Buffer Device Address input
    GpuAssistedPreDrawResources pre_draw_resources;
    VkDescriptorSet desc_set;
    VkDescriptorPool desc_pool;
    VkPipelineBindPoint pipeline_bind_point;
    CMD_TYPE cmd_type;
    GpuAssistedBufferInfo(GpuAssistedDeviceMemoryBlock output_mem_block, GpuAssistedDeviceMemoryBlock di_input_mem_block,
                          GpuAssistedDeviceMemoryBlock bda_input_mem_block, GpuAssistedPreDrawResources pre_draw_resources,
                          VkDescriptorSet desc_set, VkDescriptorPool desc_pool, VkPipelineBindPoint pipeline_bind_point,
                          CMD_TYPE cmd_type)
        : output_mem_block(output_mem_block),
          di_input_mem_block(di_input_mem_block),
          bda_input_mem_block(bda_input_mem_block),
          pre_draw_resources(pre_draw_resources),
          desc_set(desc_set),
          desc_pool(desc_pool),
          pipeline_bind_point(pipeline_bind_point),
          cmd_type(cmd_type){};
};

struct GpuAssistedShaderTracker {
    VkPipeline pipeline;
    VkShaderModule shader_module;
    std::vector<unsigned int> pgm;
};

struct GpuVuid {
    const char* uniform_access_oob = kVUIDUndefined;
    const char* storage_access_oob = kVUIDUndefined;
    const char* count_exceeds_bufsize_1 = kVUIDUndefined;
    const char* count_exceeds_bufsize = kVUIDUndefined;
};

struct GpuAssistedAccelerationStructureBuildValidationBufferInfo {
    // The acceleration structure that is being built.
    VkAccelerationStructureNV acceleration_structure = VK_NULL_HANDLE;

    // The descriptor pool and descriptor set being used to validate a given build.
    VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
    VkDescriptorSet descriptor_set = VK_NULL_HANDLE;

    // The storage buffer used by the validating compute shader whichcontains info about
    // the valid handles and which is written to communicate found invalid handles.
    VkBuffer validation_buffer = VK_NULL_HANDLE;
    VmaAllocation validation_buffer_allocation = VK_NULL_HANDLE;
};

struct GpuAssistedAccelerationStructureBuildValidationState {
    bool initialized = false;

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;

    VkAccelerationStructureNV replacement_as = VK_NULL_HANDLE;
    VmaAllocation replacement_as_allocation = VK_NULL_HANDLE;
    uint64_t replacement_as_handle = 0;

    layer_data::unordered_map<VkCommandBuffer, std::vector<GpuAssistedAccelerationStructureBuildValidationBufferInfo>>
        validation_buffers;
};

struct GpuAssistedPreDrawValidationState {
    bool globals_created = false;
    VkShaderModule validation_shader_module = VK_NULL_HANDLE;
    VkDescriptorSetLayout validation_ds_layout = VK_NULL_HANDLE;
    VkPipelineLayout validation_pipeline_layout = VK_NULL_HANDLE;
    layer_data::unordered_map <VkRenderPass, VkPipeline> renderpass_to_pipeline;
};

// To regenerate the spirv below:
//   1. Save the GLSL source below to a file called predraw_validation.vert.
//   2. Run in terminal
//
//      glslangValidator.exe -x -V predraw_validation.vert -o predraw_validation.vert.spv
//
//   4. Copy-paste the contents of predraw_validation.vert.spv here (clang-format will fix up the alignment).

// TODO - Find a way to share constants with layer code
//#version 450
//#define VAL_OUT_RECORD_SZ 10
//#define kInstValidationOutError 7
//#define kInstErrorPreDrawValidate 8
//#define pre_draw_count_too_big_error 1

//layout(set = 0, binding = 0) buffer OutputBuffer { 
//    uint count;
//    uint data[];
//} Output;
//layout(set = 0, binding = 1) buffer CountBuffer { uint data[]; } Count;
//layout(set = 0, binding = 2) buffer NewCountBuffer { uint data[]; } NewCount;
//layout(push_constant) uniform ufoo {
//    uint maxWrites;
//    uint countOffset;
//} u_info;
//void valErrorOut(uint error, uint count) {
//    uint vo_idx = atomicAdd(Output.count, VAL_OUT_RECORD_SZ);
//    if (vo_idx + VAL_OUT_RECORD_SZ > Output.data.length())
//        return;
//    Output.data[vo_idx + kInstValidationOutError] = kInstErrorPreDrawValidate;
//    Output.data[vo_idx + kInstValidationOutError + 1] = pre_draw_count_too_big_error;
//    Output.data[vo_idx + kInstValidationOutError + 2] = count;
//}
//void main() {
//    if (gl_VertexIndex == 0) {
//        uint countIn = Count.data[u_info.countOffset];
//        if (countIn > u_info.maxWrites) {
//            valErrorOut(pre_draw_count_too_big_error, countIn);
//            NewCount.data[0] = 0;
//        }
//        else {
//            NewCount.data[0] = countIn;
//        }
//    }
//}

static const uint32_t kPreDrawValidaitonShaderSpirv[] = {
    0x07230203, 0x00010000, 0x0008000a, 0x00000059, 0x00000000, 0x00020011, 0x00000001, 0x0006000b, 0x00000001, 0x4c534c47,
    0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0006000f, 0x00000000, 0x00000004, 0x6e69616d,
    0x00000000, 0x00000035, 0x00030003, 0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00070005,
    0x0000000b, 0x456c6176, 0x726f7272, 0x2874754f, 0x753b3175, 0x00003b31, 0x00040005, 0x00000009, 0x6f727265, 0x00000072,
    0x00040005, 0x0000000a, 0x6e756f63, 0x00000074, 0x00040005, 0x0000000d, 0x695f6f76, 0x00007864, 0x00060005, 0x0000000f,
    0x7074754f, 0x75427475, 0x72656666, 0x00000000, 0x00050006, 0x0000000f, 0x00000000, 0x6e756f63, 0x00000074, 0x00050006,
    0x0000000f, 0x00000001, 0x61746164, 0x00000000, 0x00040005, 0x00000011, 0x7074754f, 0x00007475, 0x00060005, 0x00000035,
    0x565f6c67, 0x65747265, 0x646e4978, 0x00007865, 0x00040005, 0x0000003a, 0x6e756f63, 0x006e4974, 0x00050005, 0x0000003c,
    0x6e756f43, 0x66754274, 0x00726566, 0x00050006, 0x0000003c, 0x00000000, 0x61746164, 0x00000000, 0x00040005, 0x0000003e,
    0x6e756f43, 0x00000074, 0x00040005, 0x0000003f, 0x6f6f6675, 0x00000000, 0x00060006, 0x0000003f, 0x00000000, 0x5778616d,
    0x65746972, 0x00000073, 0x00060006, 0x0000003f, 0x00000001, 0x6e756f63, 0x66664f74, 0x00746573, 0x00040005, 0x00000041,
    0x6e695f75, 0x00006f66, 0x00040005, 0x0000004d, 0x61726170, 0x0000006d, 0x00040005, 0x0000004e, 0x61726170, 0x0000006d,
    0x00060005, 0x00000052, 0x4377654e, 0x746e756f, 0x66667542, 0x00007265, 0x00050006, 0x00000052, 0x00000000, 0x61746164,
    0x00000000, 0x00050005, 0x00000054, 0x4377654e, 0x746e756f, 0x00000000, 0x00040047, 0x0000000e, 0x00000006, 0x00000004,
    0x00050048, 0x0000000f, 0x00000000, 0x00000023, 0x00000000, 0x00050048, 0x0000000f, 0x00000001, 0x00000023, 0x00000004,
    0x00030047, 0x0000000f, 0x00000003, 0x00040047, 0x00000011, 0x00000022, 0x00000000, 0x00040047, 0x00000011, 0x00000021,
    0x00000000, 0x00040047, 0x00000035, 0x0000000b, 0x0000002a, 0x00040047, 0x0000003b, 0x00000006, 0x00000004, 0x00050048,
    0x0000003c, 0x00000000, 0x00000023, 0x00000000, 0x00030047, 0x0000003c, 0x00000003, 0x00040047, 0x0000003e, 0x00000022,
    0x00000000, 0x00040047, 0x0000003e, 0x00000021, 0x00000001, 0x00050048, 0x0000003f, 0x00000000, 0x00000023, 0x00000000,
    0x00050048, 0x0000003f, 0x00000001, 0x00000023, 0x00000004, 0x00030047, 0x0000003f, 0x00000002, 0x00040047, 0x00000051,
    0x00000006, 0x00000004, 0x00050048, 0x00000052, 0x00000000, 0x00000023, 0x00000000, 0x00030047, 0x00000052, 0x00000003,
    0x00040047, 0x00000054, 0x00000022, 0x00000000, 0x00040047, 0x00000054, 0x00000021, 0x00000002, 0x00020013, 0x00000002,
    0x00030021, 0x00000003, 0x00000002, 0x00040015, 0x00000006, 0x00000020, 0x00000000, 0x00040020, 0x00000007, 0x00000007,
    0x00000006, 0x00050021, 0x00000008, 0x00000002, 0x00000007, 0x00000007, 0x0003001d, 0x0000000e, 0x00000006, 0x0004001e,
    0x0000000f, 0x00000006, 0x0000000e, 0x00040020, 0x00000010, 0x00000002, 0x0000000f, 0x0004003b, 0x00000010, 0x00000011,
    0x00000002, 0x00040015, 0x00000012, 0x00000020, 0x00000001, 0x0004002b, 0x00000012, 0x00000013, 0x00000000, 0x00040020,
    0x00000014, 0x00000002, 0x00000006, 0x0004002b, 0x00000006, 0x00000016, 0x0000000a, 0x0004002b, 0x00000006, 0x00000017,
    0x00000001, 0x0004002b, 0x00000006, 0x00000018, 0x00000000, 0x00020014, 0x0000001f, 0x0004002b, 0x00000012, 0x00000024,
    0x00000001, 0x0004002b, 0x00000006, 0x00000026, 0x00000007, 0x0004002b, 0x00000006, 0x00000028, 0x00000008, 0x0004002b,
    0x00000006, 0x00000030, 0x00000002, 0x00040020, 0x00000034, 0x00000001, 0x00000012, 0x0004003b, 0x00000034, 0x00000035,
    0x00000001, 0x0003001d, 0x0000003b, 0x00000006, 0x0003001e, 0x0000003c, 0x0000003b, 0x00040020, 0x0000003d, 0x00000002,
    0x0000003c, 0x0004003b, 0x0000003d, 0x0000003e, 0x00000002, 0x0004001e, 0x0000003f, 0x00000006, 0x00000006, 0x00040020,
    0x00000040, 0x00000009, 0x0000003f, 0x0004003b, 0x00000040, 0x00000041, 0x00000009, 0x00040020, 0x00000042, 0x00000009,
    0x00000006, 0x0003001d, 0x00000051, 0x00000006, 0x0003001e, 0x00000052, 0x00000051, 0x00040020, 0x00000053, 0x00000002,
    0x00000052, 0x0004003b, 0x00000053, 0x00000054, 0x00000002, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003,
    0x000200f8, 0x00000005, 0x0004003b, 0x00000007, 0x0000003a, 0x00000007, 0x0004003b, 0x00000007, 0x0000004d, 0x00000007,
    0x0004003b, 0x00000007, 0x0000004e, 0x00000007, 0x0004003d, 0x00000012, 0x00000036, 0x00000035, 0x000500aa, 0x0000001f,
    0x00000037, 0x00000036, 0x00000013, 0x000300f7, 0x00000039, 0x00000000, 0x000400fa, 0x00000037, 0x00000038, 0x00000039,
    0x000200f8, 0x00000038, 0x00050041, 0x00000042, 0x00000043, 0x00000041, 0x00000024, 0x0004003d, 0x00000006, 0x00000044,
    0x00000043, 0x00060041, 0x00000014, 0x00000045, 0x0000003e, 0x00000013, 0x00000044, 0x0004003d, 0x00000006, 0x00000046,
    0x00000045, 0x0003003e, 0x0000003a, 0x00000046, 0x0004003d, 0x00000006, 0x00000047, 0x0000003a, 0x00050041, 0x00000042,
    0x00000048, 0x00000041, 0x00000013, 0x0004003d, 0x00000006, 0x00000049, 0x00000048, 0x000500ac, 0x0000001f, 0x0000004a,
    0x00000047, 0x00000049, 0x000300f7, 0x0000004c, 0x00000000, 0x000400fa, 0x0000004a, 0x0000004b, 0x00000056, 0x000200f8,
    0x0000004b, 0x0003003e, 0x0000004d, 0x00000017, 0x0004003d, 0x00000006, 0x0000004f, 0x0000003a, 0x0003003e, 0x0000004e,
    0x0000004f, 0x00060039, 0x00000002, 0x00000050, 0x0000000b, 0x0000004d, 0x0000004e, 0x00060041, 0x00000014, 0x00000055,
    0x00000054, 0x00000013, 0x00000013, 0x0003003e, 0x00000055, 0x00000018, 0x000200f9, 0x0000004c, 0x000200f8, 0x00000056,
    0x0004003d, 0x00000006, 0x00000057, 0x0000003a, 0x00060041, 0x00000014, 0x00000058, 0x00000054, 0x00000013, 0x00000013,
    0x0003003e, 0x00000058, 0x00000057, 0x000200f9, 0x0000004c, 0x000200f8, 0x0000004c, 0x000200f9, 0x00000039, 0x000200f8,
    0x00000039, 0x000100fd, 0x00010038, 0x00050036, 0x00000002, 0x0000000b, 0x00000000, 0x00000008, 0x00030037, 0x00000007,
    0x00000009, 0x00030037, 0x00000007, 0x0000000a, 0x000200f8, 0x0000000c, 0x0004003b, 0x00000007, 0x0000000d, 0x00000007,
    0x00050041, 0x00000014, 0x00000015, 0x00000011, 0x00000013, 0x000700ea, 0x00000006, 0x00000019, 0x00000015, 0x00000017,
    0x00000018, 0x00000016, 0x0003003e, 0x0000000d, 0x00000019, 0x0004003d, 0x00000006, 0x0000001a, 0x0000000d, 0x00050080,
    0x00000006, 0x0000001b, 0x0000001a, 0x00000016, 0x00050044, 0x00000006, 0x0000001c, 0x00000011, 0x00000001, 0x0004007c,
    0x00000012, 0x0000001d, 0x0000001c, 0x0004007c, 0x00000006, 0x0000001e, 0x0000001d, 0x000500ac, 0x0000001f, 0x00000020,
    0x0000001b, 0x0000001e, 0x000300f7, 0x00000022, 0x00000000, 0x000400fa, 0x00000020, 0x00000021, 0x00000022, 0x000200f8,
    0x00000021, 0x000100fd, 0x000200f8, 0x00000022, 0x0004003d, 0x00000006, 0x00000025, 0x0000000d, 0x00050080, 0x00000006,
    0x00000027, 0x00000025, 0x00000026, 0x00060041, 0x00000014, 0x00000029, 0x00000011, 0x00000024, 0x00000027, 0x0003003e,
    0x00000029, 0x00000028, 0x0004003d, 0x00000006, 0x0000002a, 0x0000000d, 0x00050080, 0x00000006, 0x0000002b, 0x0000002a,
    0x00000026, 0x00050080, 0x00000006, 0x0000002c, 0x0000002b, 0x00000017, 0x00060041, 0x00000014, 0x0000002d, 0x00000011,
    0x00000024, 0x0000002c, 0x0003003e, 0x0000002d, 0x00000017, 0x0004003d, 0x00000006, 0x0000002e, 0x0000000d, 0x00050080,
    0x00000006, 0x0000002f, 0x0000002e, 0x00000026, 0x00050080, 0x00000006, 0x00000031, 0x0000002f, 0x00000030, 0x0004003d,
    0x00000006, 0x00000032, 0x0000000a, 0x00060041, 0x00000014, 0x00000033, 0x00000011, 0x00000024, 0x00000031, 0x0003003e,
    0x00000033, 0x00000032, 0x000100fd, 0x00010038};

class GpuAssisted : public ValidationStateTracker {
    VkPhysicalDeviceFeatures supported_features;
    VkBool32 shaderInt64;
    uint32_t unique_shader_module_id = 0;
    layer_data::unordered_map<VkCommandBuffer, std::vector<GpuAssistedBufferInfo>> command_buffer_map;  // gpu_buffer_list;
    uint32_t output_buffer_size;
    bool buffer_oob_enabled;
    bool validate_draw_indirect_count;
    std::map<VkDeviceAddress, VkDeviceSize> buffer_map;
    GpuAssistedAccelerationStructureBuildValidationState acceleration_structure_validation_state;
    GpuAssistedPreDrawValidationState pre_draw_validation_state;

    void PreRecordCommandBuffer(VkCommandBuffer command_buffer);
    bool CommandBufferNeedsProcessing(VkCommandBuffer command_buffer);
    void ProcessCommandBuffer(VkQueue queue, VkCommandBuffer command_buffer);

  public:
    GpuAssisted() { container_type = LayerObjectTypeGpuAssisted; }

    bool aborted = false;
    bool descriptor_indexing = false;
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    uint32_t adjusted_max_desc_sets;
    uint32_t desc_set_bind_index;
    VkDescriptorSetLayout debug_desc_layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout dummy_desc_layout = VK_NULL_HANDLE;
    std::unique_ptr<UtilDescriptorSetManager> desc_set_manager;
    layer_data::unordered_map<uint32_t, GpuAssistedShaderTracker> shader_map;
    PFN_vkSetDeviceLoaderData vkSetDeviceLoaderData;
    VmaAllocator vmaAllocator = {};
    std::map<VkQueue, UtilQueueBarrierCommandInfo> queue_barrier_command_infos;
    std::vector<GpuAssistedBufferInfo>& GetBufferInfo(const VkCommandBuffer command_buffer) {
        auto buffer_list = command_buffer_map.find(command_buffer);
        if (buffer_list == command_buffer_map.end()) {
            std::vector<GpuAssistedBufferInfo> new_list{};
            command_buffer_map[command_buffer] = new_list;
            return command_buffer_map[command_buffer];
        }
        return buffer_list->second;
    }

  public:
    template <typename T>
    void ReportSetupProblem(T object, const char* const specific_message) const;
    bool CheckForDescriptorIndexing(DeviceFeatures enabled_features) const;
    void PreCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, void* modified_create_info) override;
    void PostCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, VkResult result) override;
    void PostCallRecordGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                              VkDeviceAddress address) override;
    void PostCallRecordGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                 VkDeviceAddress address) override;
    void PostCallRecordGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                 VkDeviceAddress address) override;
    void PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) override;
    void PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) override;
    void PostCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                         const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                         VkResult result) override;
    void PreCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                           void* cpl_state_data) override;
    void PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                            VkResult result) override;
    void ResetCommandBuffer(VkCommandBuffer commandBuffer);
    bool PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                      VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                      uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                      uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                      uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) const override;
    bool PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                          const VkDependencyInfoKHR* pDependencyInfos) const override;
    void PreCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkBuffer* pBuffer, void* cb_state_data) override;
    void CreateAccelerationStructureBuildValidationState(GpuAssisted* device_GpuAssisted);
    void DestroyAccelerationStructureBuildValidationState();
    void PreCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo,
                                                      VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update,
                                                      VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                      VkBuffer scratch, VkDeviceSize scratchOffset) override;
    void ProcessAccelerationStructureBuildValidationBuffer(VkQueue queue, CMD_BUFFER_STATE* cb_node);
    void PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                              const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                              const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                              void* cgpl_state_data) override;
    void PreCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                             const VkComputePipelineCreateInfo* pCreateInfos,
                                             const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                             void* ccpl_state_data) override;
    void PreCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                  const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                  const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                  void* crtpl_state_data) override;
    void PreCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                   VkPipelineCache pipelineCache, uint32_t count,
                                                   const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                   const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                   void* crtpl_state_data) override;
    void PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                               const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result,
                                               void* cgpl_state_data) override;
    void PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                              const VkComputePipelineCreateInfo* pCreateInfos,
                                              const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result,
                                              void* ccpl_state_data) override;
    void PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                   const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                   const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result,
                                                   void* crtpl_state_data) override;
    void PostCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                    VkPipelineCache pipelineCache, uint32_t count,
                                                    const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                    const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                    VkResult result, void* crtpl_state_data) override;
    void PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) override;
    void PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks *pAllocator) override;
    bool InstrumentShader(const VkShaderModuleCreateInfo* pCreateInfo, std::vector<unsigned int>& new_pgm,
                          uint32_t* unique_shader_id);
    void PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                         void* csm_state_data) override;
    void AnalyzeAndGenerateMessages(VkCommandBuffer command_buffer, VkQueue queue, GpuAssistedBufferInfo &buffer_info,
        uint32_t operation_index, uint32_t* const debug_output_buffer);
 
    void SetDescriptorInitialized(uint32_t* pData, uint32_t index, const cvdescriptorset::Descriptor* descriptor);
    void UpdateInstrumentationBuffer(CMD_BUFFER_STATE* cb_node);
    const GpuVuid& GetGpuVuid(CMD_TYPE cmd_type) const;
    void PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) override;
    void PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                   VkResult result) override;
    void PreCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits,
                                      VkFence fence) override;
    void PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits, VkFence fence,
                                       VkResult result) override;
    void PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                              uint32_t firstInstance) override;
    void PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                     uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) override;
    void PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                      uint32_t stride) override;
    void PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                             uint32_t stride) override;
    void PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                              uint32_t stride, void* cdic_state) override;
    void PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                           uint32_t stride, void* cdic_state) override;
    void PreCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance,
                                                  VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                  uint32_t vertexStride) override;
    void PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride) override;
    void PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                  uint32_t stride) override;
    void PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) override;
    void PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 uint32_t drawCount, uint32_t stride) override;
    void PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride) override;
    void PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) override;
    void PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) override;
    void PreCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                                      uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;
    void PreCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                         uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                         uint32_t groupCountZ) override;
    void PreCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                     VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                     VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                     VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                     VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                     VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                     uint32_t width, uint32_t height, uint32_t depth) override;
    void PostCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                      VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                      VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                      VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                      VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                      VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                      uint32_t width, uint32_t height, uint32_t depth) override;
    void PreCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                      const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                      const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                      const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                      const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width,
                                      uint32_t height, uint32_t depth) override;
    void PostCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                       const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                       const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                       const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                       const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width,
                                       uint32_t height, uint32_t depth) override;
    void PreCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                              const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                              const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                              const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                              const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                              VkDeviceAddress indirectDeviceAddress) override;
    void PostCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                               const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                               VkDeviceAddress indirectDeviceAddress) override;
    void AllocateValidationResources(const VkCommandBuffer cmd_buffer, const VkPipelineBindPoint bind_point, CMD_TYPE cmd, void *cdic_state = nullptr);
    void AllocatePreDrawValidationResources(GpuAssistedDeviceMemoryBlock output_block, GpuAssistedPreDrawResources& resources,
                                            const LAST_BOUND_STATE& state, VkPipeline *pPipeline, cmd_draw_indirect_count_api_state *cdic_state);
    void PostCallRecordGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                                   VkPhysicalDeviceProperties* pPhysicalDeviceProperties) override;
    void PostCallRecordGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                                    VkPhysicalDeviceProperties2* pPhysicalDeviceProperties2) override;
};
