/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
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
#include "state_tracker/state_object.h"
#include <vulkan/utility/vk_safe_struct.hpp>

namespace vvl {
class ImageView;
}  // namespace vvl

static inline u32 GetSubpassDepthStencilAttachmentIndex(const vku::safe_VkPipelineDepthStencilStateCreateInfo *pipe_ds_ci,
                                                        const vku::safe_VkAttachmentReference2 *depth_stencil_ref) {
    u32 depth_stencil_attachment = VK_ATTACHMENT_UNUSED;
    if (pipe_ds_ci && depth_stencil_ref) {
        depth_stencil_attachment = depth_stencil_ref->attachment;
    }
    return depth_stencil_attachment;
}

// Store the DAG.
struct DAGNode {
    u32 pass;
    std::vector<u32> prev;
    std::vector<u32> next;
};

struct SubpassDependencyGraphNode {
    u32 pass;

    std::map<const SubpassDependencyGraphNode *, std::vector<const VkSubpassDependency2 *>> prev;
    std::map<const SubpassDependencyGraphNode *, std::vector<const VkSubpassDependency2 *>> next;
    std::vector<u32> async;  // asynchronous subpasses with a lower subpass index

    std::vector<const VkSubpassDependency2 *> barrier_from_external;
    std::vector<const VkSubpassDependency2 *> barrier_to_external;
    std::unique_ptr<VkSubpassDependency2> implicit_barrier_from_external;
    std::unique_ptr<VkSubpassDependency2> implicit_barrier_to_external;
};

struct SubpassLayout {
    u32 index;
    VkImageLayout layout;
};

namespace vvl {

class RenderPass : public StateObject {
  public:
    struct AttachmentTransition {
        u32 prev_pass;
        u32 attachment;
        VkImageLayout old_layout;
        VkImageLayout new_layout;
        AttachmentTransition(u32 prev_pass_, u32 attachment_, VkImageLayout old_layout_, VkImageLayout new_layout_)
            : prev_pass(prev_pass_), attachment(attachment_), old_layout(old_layout_), new_layout(new_layout_) {}
    };
    const bool use_dynamic_rendering;
    const bool use_dynamic_rendering_inherited;
    const bool has_multiview_enabled;
    const bool rasterization_enabled{true};
    const vku::safe_VkRenderingInfo dynamic_rendering_begin_rendering_info;
    const vku::safe_VkPipelineRenderingCreateInfo dynamic_pipeline_rendering_create_info;
    const vku::safe_VkCommandBufferInheritanceRenderingInfo inheritance_rendering_info;
    const vku::safe_VkRenderPassCreateInfo2 create_info;
    using SubpassVec = std::vector<u32>;
    using SelfDepVec = std::vector<SubpassVec>;
    const std::vector<SubpassVec> self_dependencies;
    using DAGNodeVec = std::vector<DAGNode>;
    const DAGNodeVec subpass_to_node;
    using FirstReadMap = vvl::unordered_map<u32, bool>;
    const FirstReadMap attachment_first_read;
    const SubpassVec attachment_first_subpass;
    const SubpassVec attachment_last_subpass;
    using FirstIsTransitionVec = std::vector<bool>;
    const FirstIsTransitionVec attachment_first_is_transition;
    using SubpassGraphVec = std::vector<SubpassDependencyGraphNode>;
    const SubpassGraphVec subpass_dependencies;
    using TransitionVec = std::vector<std::vector<AttachmentTransition>>;
    const TransitionVec subpass_transitions;

    RenderPass(VkRenderPass handle, VkRenderPassCreateInfo2 const *pCreateInfo);
    RenderPass(VkRenderPass handle, VkRenderPassCreateInfo const *pCreateInfo);

    RenderPass(VkPipelineRenderingCreateInfo const *pPipelineRenderingCreateInfo, bool rasterization_enabled);
    RenderPass(VkRenderingInfo const *pRenderingInfo, bool rasterization_enabled);
    RenderPass(VkCommandBufferInheritanceRenderingInfo const *pInheritanceRenderingInfo);

    VkRenderPass VkHandle() const { return handle_.Cast<VkRenderPass>(); }

    bool UsesColorAttachment(u32 subpass) const;
    bool UsesDepthStencilAttachment(u32 subpass) const;
    bool UsesNoAttachment(u32 subpass) const;
    // prefer this to checking the individual flags unless you REALLY need to check one or the other
    // Same as checking if the handle != VK_NULL_HANDLE
    bool UsesDynamicRendering() const { return use_dynamic_rendering || use_dynamic_rendering_inherited; }
    u32 GetDynamicRenderingColorAttachmentCount() const;
    u32 GetDynamicRenderingViewMask() const;
    u32 GetViewMaskBits(u32 subpass) const;
    const VkMultisampledRenderToSingleSampledInfoEXT *GetMSRTSSInfo(u32 subpass) const;
};

class Framebuffer : public StateObject {
  public:
    const vku::safe_VkFramebufferCreateInfo safe_create_info;
    const VkFramebufferCreateInfo &create_info;
    std::shared_ptr<const RenderPass> rp_state;
    std::vector<std::shared_ptr<vvl::ImageView>> attachments_view_state;

    Framebuffer(VkFramebuffer handle, const VkFramebufferCreateInfo *pCreateInfo, std::shared_ptr<RenderPass> &&rpstate,
                std::vector<std::shared_ptr<vvl::ImageView>> &&attachments);
    void LinkChildNodes() override;

    VkFramebuffer VkHandle() const { return handle_.Cast<VkFramebuffer>(); }

    virtual ~Framebuffer() { Destroy(); }

    void Destroy() override;
};

}  // namespace vvl
