/* Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (C) 2015-2021 Google Inc.
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
 *
 * Author: Courtney Goeltzenleuchter <courtneygo@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 * Author: Tobias Hector <tobias.hector@amd.com>
 * Author: Jeremy Gebben <jeremyg@lunarg.com>
 */
#pragma once
#include "base_node.h"

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

    safe_VkRenderPassCreateInfo2 createInfo;
    std::vector<std::vector<uint32_t>> self_dependencies;
    std::vector<DAGNode> subpassToNode;
    layer_data::unordered_map<uint32_t, bool> attachment_first_read;
    std::vector<uint32_t> attachment_first_subpass;
    std::vector<uint32_t> attachment_last_subpass;
    std::vector<bool> attachment_first_is_transition;
    std::vector<SubpassDependencyGraphNode> subpass_dependencies;
    std::vector<std::vector<AttachmentTransition>> subpass_transitions;

    RENDER_PASS_STATE(VkRenderPass rp, VkRenderPassCreateInfo2 const *pCreateInfo);
    RENDER_PASS_STATE(VkRenderPass rp, VkRenderPassCreateInfo const *pCreateInfo);

    VkRenderPass renderPass() const { return handle_.Cast<VkRenderPass>(); }
};

class FRAMEBUFFER_STATE : public BASE_NODE {
  public:
    safe_VkFramebufferCreateInfo createInfo;
    std::shared_ptr<const RENDER_PASS_STATE> rp_state;
    std::vector<std::shared_ptr<IMAGE_VIEW_STATE>> attachments_view_state;

    FRAMEBUFFER_STATE(VkFramebuffer fb, const VkFramebufferCreateInfo *pCreateInfo, std::shared_ptr<RENDER_PASS_STATE> &&rpstate,
                      std::vector<std::shared_ptr<IMAGE_VIEW_STATE>> &&attachments);

    VkFramebuffer framebuffer() const { return handle_.Cast<VkFramebuffer>(); }

    virtual ~FRAMEBUFFER_STATE() { Destroy(); }

    void Destroy() override;
};
