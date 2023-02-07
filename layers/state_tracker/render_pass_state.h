/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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
#include "state_tracker/base_node.h"
#include "vk_safe_struct.h"

class IMAGE_VIEW_STATE;

static inline uint32_t GetSubpassDepthStencilAttachmentIndex(const safe_VkPipelineDepthStencilStateCreateInfo *pipe_ds_ci,
                                                             const safe_VkAttachmentReference2 *depth_stencil_ref) {
    uint32_t depth_stencil_attachment = VK_ATTACHMENT_UNUSED;
    if (pipe_ds_ci && depth_stencil_ref) {
        depth_stencil_attachment = depth_stencil_ref->attachment;
    }
    return depth_stencil_attachment;
}

struct SUBPASS_INFO {
    bool used;
    VkImageUsageFlagBits usage;
    VkImageLayout layout;

    SUBPASS_INFO() : used(false), usage(VkImageUsageFlagBits(0)), layout(VK_IMAGE_LAYOUT_UNDEFINED) {}
};

// Store the DAG.
struct DAGNode {
    uint32_t pass;
    std::vector<uint32_t> prev;
    std::vector<uint32_t> next;
};

struct SubpassDependencyGraphNode {
    uint32_t pass;
    struct Dependency {
        const VkSubpassDependency2 *dependency;
        const SubpassDependencyGraphNode *node;
        Dependency() = default;
        Dependency(const VkSubpassDependency2 *dependency_, const SubpassDependencyGraphNode *node_)
            : dependency(dependency_), node(node_) {}
    };
    std::map<const SubpassDependencyGraphNode *, std::vector<const VkSubpassDependency2 *>> prev;
    std::map<const SubpassDependencyGraphNode *, std::vector<const VkSubpassDependency2 *>> next;
    std::vector<uint32_t> async;  // asynchronous subpasses with a lower subpass index

    std::vector<const VkSubpassDependency2 *> barrier_from_external;
    std::vector<const VkSubpassDependency2 *> barrier_to_external;
    std::unique_ptr<VkSubpassDependency2> implicit_barrier_from_external;
    std::unique_ptr<VkSubpassDependency2> implicit_barrier_to_external;
};

struct SubpassLayout {
    uint32_t index;
    VkImageLayout layout;
};

class RENDER_PASS_STATE : public BASE_NODE {
  public:
    struct AttachmentTransition {
        uint32_t prev_pass;
        uint32_t attachment;
        VkImageLayout old_layout;
        VkImageLayout new_layout;
        AttachmentTransition(uint32_t prev_pass_, uint32_t attachment_, VkImageLayout old_layout_, VkImageLayout new_layout_)
            : prev_pass(prev_pass_), attachment(attachment_), old_layout(old_layout_), new_layout(new_layout_) {}
    };
    const bool use_dynamic_rendering;
    const bool use_dynamic_rendering_inherited;
    const bool has_multiview_enabled;
    const safe_VkRenderingInfo dynamic_rendering_begin_rendering_info;
    const safe_VkPipelineRenderingCreateInfo dynamic_rendering_pipeline_create_info;
    const safe_VkCommandBufferInheritanceRenderingInfo inheritance_rendering_info;
    const safe_VkRenderPassCreateInfo2 createInfo;
    using SubpassVec = std::vector<uint32_t>;
    using SelfDepVec = std::vector<SubpassVec>;
    const std::vector<SubpassVec> self_dependencies;
    using DAGNodeVec = std::vector<DAGNode>;
    const DAGNodeVec subpass_to_node;
    using FirstReadMap = vvl::unordered_map<uint32_t, bool>;
    const FirstReadMap attachment_first_read;
    const SubpassVec attachment_first_subpass;
    const SubpassVec attachment_last_subpass;
    using FirstIsTransitionVec = std::vector<bool>;
    const FirstIsTransitionVec attachment_first_is_transition;
    using SubpassGraphVec = std::vector<SubpassDependencyGraphNode>;
    const SubpassGraphVec subpass_dependencies;
    using TransitionVec = std::vector<std::vector<AttachmentTransition>>;
    const TransitionVec subpass_transitions;

    RENDER_PASS_STATE(VkRenderPass rp, VkRenderPassCreateInfo2 const *pCreateInfo);
    RENDER_PASS_STATE(VkRenderPass rp, VkRenderPassCreateInfo const *pCreateInfo);

    RENDER_PASS_STATE(VkPipelineRenderingCreateInfo const *pPipelineRenderingCreateInfo);
    RENDER_PASS_STATE(VkRenderingInfo const *pRenderingInfo);
    RENDER_PASS_STATE(VkCommandBufferInheritanceRenderingInfo const *pInheritanceRenderingInfo);

    VkRenderPass renderPass() const { return handle_.Cast<VkRenderPass>(); }

    bool UsesColorAttachment(uint32_t subpass) const;
    bool UsesDepthStencilAttachment(uint32_t subpass) const;
    // prefer this to checking the individual flags unless you REALLY need to check one or the other
    bool UsesDynamicRendering() const { return use_dynamic_rendering || use_dynamic_rendering_inherited; }
    uint32_t GetDynamicRenderingColorAttachmentCount() const;
    uint32_t GetDynamicRenderingViewMask() const;
    uint32_t GetViewMaskBits(uint32_t subpass) const;
};

class FRAMEBUFFER_STATE : public BASE_NODE {
  public:
    const safe_VkFramebufferCreateInfo createInfo;
    std::shared_ptr<const RENDER_PASS_STATE> rp_state;
    std::vector<std::shared_ptr<IMAGE_VIEW_STATE>> attachments_view_state;

    FRAMEBUFFER_STATE(VkFramebuffer fb, const VkFramebufferCreateInfo *pCreateInfo, std::shared_ptr<RENDER_PASS_STATE> &&rpstate,
                      std::vector<std::shared_ptr<IMAGE_VIEW_STATE>> &&attachments);
    void LinkChildNodes() override;

    VkFramebuffer framebuffer() const { return handle_.Cast<VkFramebuffer>(); }

    virtual ~FRAMEBUFFER_STATE() { Destroy(); }

    void Destroy() override;
};
