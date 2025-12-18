/*
 * Copyright (c) 2023-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

class PositiveTileMemoryHeap : public TileMemoryHeapTest {};

TEST_F(PositiveTileMemoryHeap, BasicBuffer) {
    TEST_DESCRIPTION("Create tile memory storage buffer and use it within a draw.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileMemoryHeap);

    RETURN_IF_SKIP(Init());

    // Create a Tile Memory Buffer
    vkt::Buffer buffer(*m_device,
                       vkt::Buffer::CreateInfo(4096, VK_BUFFER_USAGE_2_TILE_MEMORY_BIT_QCOM | VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT),
                       vkt::no_mem);

    // Query Tile Memory Buffer Requirements
    VkBufferMemoryRequirementsInfo2 buffer_info = vku::InitStructHelper();
    VkMemoryRequirements2 buffer_reqs = vku::InitStructHelper();
    VkTileMemoryRequirementsQCOM tile_mem_reqs = vku::InitStructHelper();
    buffer_info.buffer = buffer;
    buffer_reqs.pNext = &tile_mem_reqs;
    vk::GetBufferMemoryRequirements2(device(), &buffer_info, &buffer_reqs);

    if (tile_mem_reqs.size == 0) {
        GTEST_SKIP() << "Buffer not eligible to be bound with Tile Memory.";
    }

    // Find a memory configuration for the Tile Memory Buffer, otherwise exit
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.memoryTypeIndex = 0;
    alloc_info.allocationSize = tile_mem_reqs.size;
    bool pass = m_device->Physical().SetMemoryType(buffer_reqs.memoryRequirements.memoryTypeBits, &alloc_info,
                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM);

    if (!pass) {
        GTEST_SKIP() << "Could not find an eligible Tile Memory Type.";
    }

    // Bind Tile Memory to Buffer
    vkt::DeviceMemory buffer_memory(*m_device, alloc_info);
    vk::BindBufferMemory(device(), buffer, buffer_memory, 0);

    // Create Compute Shader to write to Tile Memory Buffer
    const char *cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer ssbo { float tileMemBuffer; };
        void main() {
           tileMemBuffer = 1.0f;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    // Requires SPIR-V 1.3 for SPV_KHR_storage_buffer_storage_class
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.CreateComputePipeline();
    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, buffer, 0, 4096, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    VkTileMemoryBindInfoQCOM tile_mem_bind_info = vku::InitStructHelper();
    tile_mem_bind_info.memory = buffer_memory;

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindTileMemoryQCOM(m_command_buffer, &tile_mem_bind_info);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
}