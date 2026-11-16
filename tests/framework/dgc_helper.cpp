/* Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
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

#include "dgc_helper.h"
#include "utils/device_generated_commands_utils.h"
#include <vector>

namespace vkt {

void IndirectCommandsToken::ExecutionSetPipeline(VkShaderStageFlags shader_stages) {
    Token& token = tokens_.emplace_back();
    token.type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT;
    token.execution_set = {VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT, shader_stages};
}

void IndirectCommandsToken::ExecutionSetShaderObject(VkShaderStageFlags shader_stages) {
    Token& token = tokens_.emplace_back();
    token.type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT;
    token.execution_set = {VK_INDIRECT_EXECUTION_SET_INFO_TYPE_SHADER_OBJECTS_EXT, shader_stages};
}

void IndirectCommandsToken::PushConstant(const VkPushConstantRange& update_range) {
    Token& token = tokens_.emplace_back();
    token.type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_EXT;
    token.push_constant = {update_range};
}

void IndirectCommandsToken::SequenceIndex(const VkPushConstantRange& update_range) {
    Token& token = tokens_.emplace_back();
    token.type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_SEQUENCE_INDEX_EXT;
    token.push_constant = {update_range};
}

void IndirectCommandsToken::PushData(const VkPushConstantRange& update_range) {
    Token& token = tokens_.emplace_back();
    token.type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_DATA_EXT;
    token.push_constant = {update_range};
}

void IndirectCommandsToken::PushDataSequenceIndex(const VkPushConstantRange& update_range) {
    Token& token = tokens_.emplace_back();
    token.type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_DATA_SEQUENCE_INDEX_EXT;
    token.push_constant = {update_range};
}

void IndirectCommandsToken::IndexBuffer(VkIndirectCommandsInputModeFlagBitsEXT mode) {
    Token& token = tokens_.emplace_back();
    token.type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_EXT;
    token.index_buffer = {mode};
}

void IndirectCommandsToken::VertexBuffer(uint32_t vertex_binding_unit) {
    Token& token = tokens_.emplace_back();
    token.type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_EXT;
    token.vertex_buffer = {vertex_binding_unit};
}

void IndirectCommandsToken::Action(VkIndirectCommandsTokenTypeEXT type) { tokens_.emplace_back().type = type; }

std::vector<VkIndirectCommandsLayoutTokenEXT> IndirectCommandsToken::Info(uint32_t& indirect_stride) const {
    std::vector<VkIndirectCommandsLayoutTokenEXT> infos;
    indirect_stride = 0;
    for (const Token& token : tokens_) {
        VkIndirectCommandsLayoutTokenEXT& info = infos.emplace_back(vku::InitStructHelper());
        info.type = token.type;
        info.offset = indirect_stride;
        switch (token.type) {
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT:
                info.data.pExecutionSet = &token.execution_set;
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_EXT:
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_SEQUENCE_INDEX_EXT:
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_DATA_EXT:
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_DATA_SEQUENCE_INDEX_EXT:
                info.data.pPushConstant = &token.push_constant;
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_EXT:
                info.data.pIndexBuffer = &token.index_buffer;
                break;
            case VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_EXT:
                info.data.pVertexBuffer = &token.vertex_buffer;
                break;
            default:
                break;
        }
        indirect_stride += vvl::IndirectCommandsTokenDataSize(info);
    }
    return infos;
}

IndirectCommandsLayout::IndirectCommandsLayout(const Device& dev, const IndirectCommandsToken& tokens,
                                               VkShaderStageFlags shader_stages, VkPipelineLayout pipeline_layout,
                                               VkIndirectCommandsLayoutUsageFlagsEXT flags) {
    VkIndirectCommandsLayoutCreateInfoEXT create_info = vku::InitStructHelper();
    create_info.flags = flags;
    create_info.shaderStages = shader_stages;
    create_info.pipelineLayout = pipeline_layout;
    const std::vector<VkIndirectCommandsLayoutTokenEXT> token_infos = tokens.Info(create_info.indirectStride);
    create_info.tokenCount = static_cast<uint32_t>(token_infos.size());
    create_info.pTokens = token_infos.data();
    Init(dev, create_info);
}

void PreprocessBuffer::Init(const vkt::Device& device, VkGeneratedCommandsInfoEXT& generated_commands_info,
                            const void* pipeline_or_shader_info) {
    VkGeneratedCommandsMemoryRequirementsInfoEXT dgc_mem_reqs = vku::InitStructHelper();
    dgc_mem_reqs.pNext = pipeline_or_shader_info ? pipeline_or_shader_info : generated_commands_info.pNext;
    dgc_mem_reqs.indirectCommandsLayout = generated_commands_info.indirectCommandsLayout;
    dgc_mem_reqs.indirectExecutionSet = generated_commands_info.indirectExecutionSet;
    dgc_mem_reqs.maxSequenceCount = generated_commands_info.maxSequenceCount;
    dgc_mem_reqs.maxDrawCount = generated_commands_info.maxDrawCount;

    VkMemoryRequirements2 mem_reqs2 = vku::InitStructHelper();
    vk::GetGeneratedCommandsMemoryRequirementsEXT(device, &dgc_mem_reqs, &mem_reqs2);
    size_ = mem_reqs2.memoryRequirements.size;
    address_ = 0;

    if (size_ > 0) {
        VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
        allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

        VkBufferUsageFlags2CreateInfo buffer_usage_flags = vku::InitStructHelper();
        buffer_usage_flags.usage = VK_BUFFER_USAGE_2_PREPROCESS_BUFFER_BIT_EXT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
        VkBufferCreateInfo buffer_ci = vku::InitStructHelper(&buffer_usage_flags);
        buffer_ci.size = size_;
        buffer.InitNoMemory(device, buffer_ci);
        // The memory must also be one of the memory types allowed for preprocess memory
        VkMemoryRequirements buffer_mem_reqs = buffer.MemoryRequirements();
        buffer_mem_reqs.memoryTypeBits &= mem_reqs2.memoryRequirements.memoryTypeBits;
        memory.Init(device, DeviceMemory::GetResourceAllocInfo(device, buffer_mem_reqs, 0, &allocate_flag_info));
        buffer.BindMemory(memory, 0);
        address_ = buffer.Address();
    }

    generated_commands_info.preprocessSize = size_;
    generated_commands_info.preprocessAddress = address_;
}

}  // namespace vkt
