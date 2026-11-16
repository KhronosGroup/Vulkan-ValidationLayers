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

#pragma once

#include <vulkan/vulkan_core.h>
#include "binding.h"

namespace vkt {

// Collects the tokens of a VkIndirectCommandsLayoutEXT and owns the data VkIndirectCommandsTokenDataEXT would point to.
// The offset of each token is not set here, vkt::IndirectCommandsLayout tightly packs them in the order added
class IndirectCommandsToken {
  public:
    void ExecutionSetPipeline(VkShaderStageFlags shader_stages);
    void ExecutionSetShaderObject(VkShaderStageFlags shader_stages);
    void PushConstant(const VkPushConstantRange& update_range);
    void SequenceIndex(const VkPushConstantRange& update_range);
    void PushData(const VkPushConstantRange& update_range);
    void PushDataSequenceIndex(const VkPushConstantRange& update_range);
    void IndexBuffer(VkIndirectCommandsInputModeFlagBitsEXT mode = VK_INDIRECT_COMMANDS_INPUT_MODE_VULKAN_INDEX_BUFFER_EXT);
    void VertexBuffer(uint32_t vertex_binding_unit);

    // For all the tokens that don't need any VkIndirectCommandsTokenDataEXT (draw, dispatch, trace rays, etc)
    void Action(VkIndirectCommandsTokenTypeEXT type);

    // Data pointers point into this object, so it must outlive the returned tokens
    std::vector<VkIndirectCommandsLayoutTokenEXT> Info(uint32_t& indirect_stride) const;

  private:
    struct Token {
        VkIndirectCommandsTokenTypeEXT type;
        VkIndirectCommandsExecutionSetTokenEXT execution_set = {};
        VkIndirectCommandsPushConstantTokenEXT push_constant = {};
        VkIndirectCommandsIndexBufferTokenEXT index_buffer = {};
        VkIndirectCommandsVertexBufferTokenEXT vertex_buffer = {};
    };
    std::vector<Token> tokens_;
};

class PreprocessBuffer {
  public:
    PreprocessBuffer() = default;
    PreprocessBuffer(const Device& dev, VkGeneratedCommandsInfoEXT& generated_commands_info,
                     const void* pipeline_or_shader_info = nullptr) {
        Init(dev, generated_commands_info, pipeline_or_shader_info);
    }
    void Init(const Device& dev, VkGeneratedCommandsInfoEXT& generated_commands_info,
              const void* pipeline_or_shader_info = nullptr);

    VkDeviceSize Size() const { return size_; }
    VkDeviceAddress Address() const { return address_; }

    vkt::Buffer buffer;
    vkt::DeviceMemory memory;

  private:
    VkDeviceSize size_ = 0;
    VkDeviceAddress address_ = 0;
};

}  // namespace vkt
