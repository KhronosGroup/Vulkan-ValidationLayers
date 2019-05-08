/* Copyright (c) 2015-2019 The Khronos Group Inc.
 * Copyright (c) 2015-2019 Valve Corporation
 * Copyright (c) 2015-2019 LunarG, Inc.
 * Copyright (C) 2015-2019 Google Inc.
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
 * Author: Courtney Goeltzenleuchter <courtneygo@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 */
#ifndef CORE_VALIDATION_TYPES_H_
#define CORE_VALIDATION_TYPES_H_

#include "hash_vk_types.h"
#include "sparse_containers.h"
#include "vk_safe_struct.h"
#include "vulkan/vulkan.h"
#include "vk_layer_logging.h"
#include "vk_object_types.h"
#include "vk_extension_helper.h"
#include "vk_typemap_helper.h"
#include "layer_chassis_dispatch.h"

#include <array>
#include <atomic>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <list>

#ifdef VK_USE_PLATFORM_ANDROID_KHR
#include "android_ndk_types.h"
#endif  // VK_USE_PLATFORM_ANDROID_KHR

// Fwd declarations -- including descriptor_set.h creates an ugly include loop
namespace cvdescriptorset {
class DescriptorSetLayoutDef;
class DescriptorSetLayout;
class DescriptorSet;
}  // namespace cvdescriptorset

struct CMD_BUFFER_STATE;
class CoreChecks;

class BASE_NODE {
   public:
    // Track command buffers that this object is bound to
    //  binding initialized when cmd referencing object is bound to command buffer
    //  binding removed when command buffer is reset or destroyed
    // When an object is destroyed, any bound cbs are set to INVALID
    std::unordered_set<CMD_BUFFER_STATE *> cb_bindings;

    BASE_NODE(){};
};

// Track command pools and their command buffers
struct COMMAND_POOL_STATE : public BASE_NODE {
    uint32_t queueFamilyIndex;
    std::unordered_set<VkCommandBuffer> commandBuffers;
};

// Generic wrapper for vulkan objects
struct VK_OBJECT {
    uint64_t handle;
    VulkanObjectType type;
};

// Flags describing requirements imposed by the pipeline on a descriptor
enum descriptor_req {
    DESCRIPTOR_REQ_VIEW_TYPE_1D = 1 << VK_IMAGE_VIEW_TYPE_1D,
    DESCRIPTOR_REQ_VIEW_TYPE_1D_ARRAY = 1 << VK_IMAGE_VIEW_TYPE_1D_ARRAY,
    DESCRIPTOR_REQ_VIEW_TYPE_2D = 1 << VK_IMAGE_VIEW_TYPE_2D,
    DESCRIPTOR_REQ_VIEW_TYPE_2D_ARRAY = 1 << VK_IMAGE_VIEW_TYPE_2D_ARRAY,
    DESCRIPTOR_REQ_VIEW_TYPE_3D = 1 << VK_IMAGE_VIEW_TYPE_3D,
    DESCRIPTOR_REQ_VIEW_TYPE_CUBE = 1 << VK_IMAGE_VIEW_TYPE_CUBE,
    DESCRIPTOR_REQ_VIEW_TYPE_CUBE_ARRAY = 1 << VK_IMAGE_VIEW_TYPE_CUBE_ARRAY,
    DESCRIPTOR_REQ_ALL_VIEW_TYPE_BITS = (1 << (VK_IMAGE_VIEW_TYPE_END_RANGE + 1)) - 1,
    DESCRIPTOR_REQ_SINGLE_SAMPLE = 2 << VK_IMAGE_VIEW_TYPE_END_RANGE,
    DESCRIPTOR_REQ_MULTI_SAMPLE = DESCRIPTOR_REQ_SINGLE_SAMPLE << 1,
    DESCRIPTOR_REQ_COMPONENT_TYPE_FLOAT = DESCRIPTOR_REQ_MULTI_SAMPLE << 1,
    DESCRIPTOR_REQ_COMPONENT_TYPE_SINT = DESCRIPTOR_REQ_COMPONENT_TYPE_FLOAT << 1,
    DESCRIPTOR_REQ_COMPONENT_TYPE_UINT = DESCRIPTOR_REQ_COMPONENT_TYPE_SINT << 1,
};

struct DESCRIPTOR_POOL_STATE : BASE_NODE {
    VkDescriptorPool pool;
    std::unordered_set<cvdescriptorset::DescriptorSet *> sets;
    DESCRIPTOR_POOL_STATE(const VkDescriptorPool pool) : pool(pool) {}
};

struct RENDER_PASS_STATE : public BASE_NODE {
    VkRenderPass renderPass;

    // ColorAttachment Index for each color attachment for each subpass
    std::vector<std::vector<uint32_t>> subpasses;

    RENDER_PASS_STATE(VkRenderPassCreateInfo2KHR const *pCreateInfo) {
        subpasses.resize(pCreateInfo->subpassCount);
        for (uint32_t subpass = 0; subpass < subpasses.size(); subpass++) {
            subpasses[subpass].resize(pCreateInfo->pSubpasses[subpass].colorAttachmentCount);
            for (uint32_t attachment = 0; attachment < subpasses[subpass].size(); attachment++) {
                subpasses[subpass][attachment] = pCreateInfo->pSubpasses[subpass].pColorAttachments[attachment].attachment;
            }
        }
    }

    RENDER_PASS_STATE(VkRenderPassCreateInfo const *pCreateInfo) {
        subpasses.resize(pCreateInfo->subpassCount);
        for (uint32_t subpass = 0; subpass < subpasses.size(); subpass++) {
            subpasses[subpass].resize(pCreateInfo->pSubpasses[subpass].colorAttachmentCount);
            for (uint32_t attachment = 0; attachment < subpasses[subpass].size(); attachment++) {
                subpasses[subpass][attachment] = pCreateInfo->pSubpasses[subpass].pColorAttachments[attachment].attachment;
            }
        }
    }
};

// Canonical dictionary for PushConstantRanges
using PushConstantRangesDict = hash_util::Dictionary<PushConstantRanges>;
using PushConstantRangesId = PushConstantRangesDict::Id;

// Canonical dictionary for the pipeline layout's layout of descriptorsetlayouts
using DescriptorSetLayoutDef = cvdescriptorset::DescriptorSetLayoutDef;
using DescriptorSetLayoutId = std::shared_ptr<const DescriptorSetLayoutDef>;
using PipelineLayoutSetLayoutsDef = std::vector<DescriptorSetLayoutId>;
using PipelineLayoutSetLayoutsDict =
    hash_util::Dictionary<PipelineLayoutSetLayoutsDef, hash_util::IsOrderedContainer<PipelineLayoutSetLayoutsDef>>;
using PipelineLayoutSetLayoutsId = PipelineLayoutSetLayoutsDict::Id;

// Store layouts and pushconstants for PipelineLayout
struct PIPELINE_LAYOUT_STATE {
    VkPipelineLayout layout;
    std::vector<std::shared_ptr<cvdescriptorset::DescriptorSetLayout const>> set_layouts;
    PushConstantRangesId push_constant_ranges;

    PIPELINE_LAYOUT_STATE() : layout(VK_NULL_HANDLE), set_layouts{}, push_constant_ranges{} {}

    void reset() {
        layout = VK_NULL_HANDLE;
        set_layouts.clear();
        push_constant_ranges.reset();
    }
};

class PIPELINE_STATE : public BASE_NODE {
   public:
    VkPipeline pipeline;
    safe_VkGraphicsPipelineCreateInfo graphicsPipelineCI;
    safe_VkComputePipelineCreateInfo computePipelineCI;
    safe_VkRayTracingPipelineCreateInfoNV raytracingPipelineCI;
    // Capture which slots (set#->bindings) are actually used by the shaders of this pipeline
    std::unordered_map<uint32_t, std::map<uint32_t, descriptor_req>> active_slots;
    // Vtx input info (if any)
    std::unordered_map<uint32_t, uint32_t> vertex_binding_to_index_map_;
    std::vector<VkPipelineColorBlendAttachmentState> attachments;
    bool blendConstantsEnabled;  // Blend constants enabled for any attachments
    PIPELINE_LAYOUT_STATE pipeline_layout;
    VkPrimitiveTopology topology_at_rasterizer;

    // Default constructor
    PIPELINE_STATE()
        : pipeline{},
          graphicsPipelineCI{},
          computePipelineCI{},
          raytracingPipelineCI{},
          active_slots(),
          vertex_binding_to_index_map_(),
          attachments(),
          blendConstantsEnabled(false),
          pipeline_layout(),
          topology_at_rasterizer{} {}

    void reset() {
        VkGraphicsPipelineCreateInfo emptyGraphicsCI = {};
        graphicsPipelineCI.initialize(&emptyGraphicsCI, false, false);
        VkComputePipelineCreateInfo emptyComputeCI = {};
        computePipelineCI.initialize(&emptyComputeCI);
        VkRayTracingPipelineCreateInfoNV emptyRayTracingCI = {};
        raytracingPipelineCI.initialize(&emptyRayTracingCI);
    }

    void initGraphicsPipeline(const VkGraphicsPipelineCreateInfo *pCreateInfo, std::shared_ptr<RENDER_PASS_STATE> &&rpstate) {
        reset();
        bool uses_color_attachment = false;
        if (pCreateInfo->subpass < rpstate->subpasses.size()) {
            for (uint32_t i = 0; i < rpstate->subpasses[pCreateInfo->subpass].size(); ++i) {
                if (rpstate->subpasses[pCreateInfo->subpass][i] != VK_ATTACHMENT_UNUSED) {
                    uses_color_attachment = true;
                    break;
                }
            }
        }
        graphicsPipelineCI.initialize(pCreateInfo, uses_color_attachment, false);
        for (uint32_t i = 0; i < pCreateInfo->stageCount; i++) {
            const VkPipelineShaderStageCreateInfo *pPSSCI = &pCreateInfo->pStages[i];
        }
        if (graphicsPipelineCI.pColorBlendState) {
            const auto pCBCI = graphicsPipelineCI.pColorBlendState;
            if (pCBCI->attachmentCount) {
                this->attachments = std::vector<VkPipelineColorBlendAttachmentState>(pCBCI->pAttachments,
                                                                                     pCBCI->pAttachments + pCBCI->attachmentCount);
            }
        }
        if (graphicsPipelineCI.pInputAssemblyState) {
            topology_at_rasterizer = graphicsPipelineCI.pInputAssemblyState->topology;
        }
    }

    void initComputePipeline(const VkComputePipelineCreateInfo *pCreateInfo) {
        reset();
        computePipelineCI.initialize(pCreateInfo);
    }

    void initRayTracingPipelineNV(const VkRayTracingPipelineCreateInfoNV *pCreateInfo) {
        reset();
        raytracingPipelineCI.initialize(pCreateInfo);
    }
};

// Track last states that are bound per pipeline bind point (Gfx & Compute)
struct LAST_BOUND_STATE {
    LAST_BOUND_STATE() { reset(); }  // must define default constructor for portability reasons
    PIPELINE_STATE *pipeline_state;
    VkPipelineLayout pipeline_layout;
    // Track each set that has been bound
    std::vector<cvdescriptorset::DescriptorSet *> boundDescriptorSets;
    std::unique_ptr<cvdescriptorset::DescriptorSet> push_descriptor_set;
    // One dynamic offset per dynamic descriptor bound to this CB
    std::vector<std::vector<uint32_t>> dynamicOffsets;

    void reset() {
        pipeline_state = nullptr;
        pipeline_layout = VK_NULL_HANDLE;
        boundDescriptorSets.clear();
        push_descriptor_set = nullptr;
        dynamicOffsets.clear();
    }
};

// Cmd Buffer Wrapper Struct - TODO : This desperately needs its own class
struct CMD_BUFFER_STATE : public BASE_NODE {
    VkCommandBuffer commandBuffer;
    VkDevice device;
    bool hasDrawCmd;
    // Currently storing "lastBound" objects on per-CB basis
    //  long-term may want to create caches of "lastBound" states and could have
    //  each individual CMD_NODE referencing its own "lastBound" state
    // Store last bound state for Gfx & Compute pipeline bind points
    std::map<uint32_t, LAST_BOUND_STATE> lastBound;

    RENDER_PASS_STATE *activeRenderPass;
    VkCommandBuffer primaryCommandBuffer;
    // If primary, the secondary command buffers we will call.
    // If secondary, the primary command buffers we will be called by.
    std::unordered_set<CMD_BUFFER_STATE *> linkedCommandBuffers;

    // Cache of current insert label...
    LoggingLabel debug_label;
};

struct SHADER_MODULE_STATE;
struct DeviceExtensions;

enum RenderPassCreateVersion { RENDER_PASS_VERSION_1 = 0, RENDER_PASS_VERSION_2 = 1 };

// GPU Validation
struct ShaderTracker {
    VkPipeline pipeline;
    VkShaderModule shader_module;
    std::vector<unsigned int> pgm;
};

std::shared_ptr<cvdescriptorset::DescriptorSetLayout const> const GetDescriptorSetLayout(CoreChecks const *, VkDescriptorSetLayout);

#endif  // CORE_VALIDATION_TYPES_H_
