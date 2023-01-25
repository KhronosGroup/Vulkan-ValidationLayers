/* Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 LunarG, Inc.
 * Copyright (C) 2023 Google Inc.
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

#include "chassis.h"

#include "explicit_validation.h"
#include "layer_chassis_dispatch.h"

bool ExplicitValidation::shouldIgnore_VkGraphicsPipelineCreateInfo_pTessellationState(const VkStructureType sType,
                const void* pNext,
                const VkPipelineCreateFlags flags,
                const uint32_t stageCount,
                const VkPipelineShaderStageCreateInfo* pStages,
                const VkPipelineVertexInputStateCreateInfo* pVertexInputState,
                const VkPipelineInputAssemblyStateCreateInfo* pInputAssemblyState,
                const VkPipelineTessellationStateCreateInfo* pTessellationState,
                const VkPipelineViewportStateCreateInfo* pViewportState,
                const VkPipelineRasterizationStateCreateInfo* pRasterizationState,
                const VkPipelineMultisampleStateCreateInfo* pMultisampleState,
                const VkPipelineDepthStencilStateCreateInfo* pDepthStencilState,
                const VkPipelineColorBlendStateCreateInfo* pColorBlendState,
                const VkPipelineDynamicStateCreateInfo* pDynamicState,
                const VkPipelineLayout layout,
                const VkRenderPass renderPass,
                const uint32_t subpass,
                const VkPipeline basePipelineHandle,
                const int32_t basePipelineIndex) const
{
    // TODO
    return false;
}
