/* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (C) 2015-2026 Google Inc.
 * Copyright (C) 2026 Qualcomm Technologies, Inc.
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
#include <vulkan/vulkan_core.h>
#include "state_tracker/state_object.h"
#include <vulkan/utility/vk_safe_struct.hpp>
#include <map>

namespace vvl {
class ImageView;
}  // namespace vvl

static inline uint32_t GetSubpassDepthStencilAttachmentIndex(const vku::safe_VkPipelineDepthStencilStateCreateInfo *pipe_ds_ci,
                                                             const vku::safe_VkAttachmentReference2 *depth_stencil_ref) {
    uint32_t depth_stencil_attachment = VK_ATTACHMENT_UNUSED;
    if (pipe_ds_ci && depth_stencil_ref) {
        depth_stencil_attachment = depth_stencil_ref->attachment;
    }
    return depth_stencil_attachment;
}

struct SubpassDependencyInfo {
    // For dependencies between subpasses this is a dstSubpass.
    // For external dependencies this can be either srcSubpass or dstSubpass
    uint32_t subpass;

    // Map's key is a srcSubpass in subpass dependency.
    // this->subpass is a dstSubpass in subpass dependency.
    // Map's value is a list of dependencies defined for a given (srcSubpass, dstSubpass) pair.
    std::map<uint32_t, std::vector<const VkSubpassDependency2 *>> dependencies;

    // Asynchronous subpasses with a lower subpass index
    std::vector<uint32_t> async;

    // Subpass dependencies with srcSubpass = VK_SUBPASS_EXTERNAL
    std::vector<const VkSubpassDependency2 *> barrier_from_external;

    // Subpass dependencies with dstSubpass = VK_SUBPASS_EXTERNAL
    std::vector<const VkSubpassDependency2 *> barrier_to_external;

    // Implicit external barriers are defined only if subpass dependencies do not specify them.
    // Given how SubpassDependencyInfo objects are stored, it is safe to keep references to
    // these barriers in the barrier_from_external and barrier_to_external vectors.
    VkSubpassDependency2 implicit_barrier_from_external;
    VkSubpassDependency2 implicit_barrier_to_external;
};

struct SubpassLayout {
    uint32_t index;
    VkImageLayout layout;
};

namespace vvl {

// Vulkan 1.0 has a VkRenderPass object, things like dynamic rendering moved the handle to be across various other structs/calls.
// We create a "RenderPass" object for dynamic rendering, so other code can just use it without caring how the contents were added
// inside.
class RenderPass : public StateObject {
  public:
    const vku::safe_VkRenderPassCreateInfo2 create_info;

    const bool use_dynamic_rendering;
    const bool use_dynamic_rendering_inherited;

    const vku::safe_VkRenderingInfo dynamic_rendering_begin_rendering_info;
    // Note, this is not the exact VkPipelineRenderingCreateInfo passed in, instead it will handle fields being ignored
    const vku::safe_VkPipelineRenderingCreateInfo dynamic_pipeline_rendering_create_info;
    // when a secondary command buffer is recorded with VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT
    const vku::safe_VkCommandBufferInheritanceRenderingInfo inheritance_rendering_info;
    // because colorAttachmentCount is needed to calculate everything, save it.
    const uint32_t dynamic_rendering_color_attachment_count;

    const bool has_multiview_enabled;
    const bool has_tile_shading_enabled;

    // For each subpass, indices into pDependencies for that subpass's self-dependencies
    const std::vector<std::vector<uint32_t>> self_dependencies;  // [subpassCount]

    // Dependency information for each subpass
    const std::vector<SubpassDependencyInfo> subpass_dependency_infos;  // [subpassCount]

    // Maps view index to subpass index: subpass = SubpassPerView[view_index].
    // If multiview is not used then this stores a single subpass index as the first element
    using SubpassPerView = small_vector<uint32_t, 2>;

    // For each attachment, the first subpass that uses it.
    // VK_SUBPASS_EXTERNAL if the attachment is unused.
    // If multiview is enabled, the subpass is tracked per view
    const std::vector<SubpassPerView> attachment_first_subpass;  // [attachmentCount]

    // For each attachment, the last subpass that uses it.
    // VK_SUBPASS_EXTERNAL if the attachment is unused.
    // If multiview is enabled the subpass is defined per view
    const std::vector<SubpassPerView> attachment_last_subpass;  // [attachmentCount]

    struct AttachmentTransition {
        // Subpass index or VK_SUBPASS_EXTERNAL for transitions from initialLayout.
        // For transitions into finalLayout this is the last subpass that used the attachment.
        uint32_t src_subpass;

        uint32_t attachment;
        VkImageLayout old_layout;
        VkImageLayout new_layout;
    };
    // The list of transitions for each subpass.
    // The last element (subpass_transitions[subpassCount]) are the transitions into finalLayout.
    // NOTE: this first element should not be interpreted as all transitions from initialLayout.
    // The initiaLayout transitions are defined by the first subpass that used the attachment,
    // so they may be registered for any subpass [0..subpassCount-1]
    const std::vector<std::vector<AttachmentTransition>> subpass_transitions;  // [subpassCount + 1]

    // vkCreateRenderPass
    RenderPass(VkRenderPass handle, VkRenderPassCreateInfo const *pCreateInfo);
    // vkCreateRenderPass2
    RenderPass(VkRenderPass handle, VkRenderPassCreateInfo2 const *pCreateInfo);

    // vkCmdBeginRendering
    explicit RenderPass(const VkRenderingInfo& rendering_info);
    // vkBeginCommandBuffer (dynamic rendering in secondary commadn buffer)
    explicit RenderPass(VkCommandBufferInheritanceRenderingInfo const *pInheritanceRenderingInfo);

    // vkCreateGraphicsPipelines (dynamic rendering state tied to pipeline state)
    explicit RenderPass(const VkPipelineRenderingCreateInfo& rendering_ci);

    VkRenderPass VkHandle() const { return handle_.Cast<VkRenderPass>(); }

    bool UsesColorAttachment(uint32_t subpass) const;
    bool UsesDepthStencilAttachment(uint32_t subpass) const;
    bool UsesNoAttachment(uint32_t subpass) const;
    // prefer this to checking the individual flags unless you REALLY need to check one or the other
    // Same as checking if the handle != VK_NULL_HANDLE
    bool UsesDynamicRendering() const { return use_dynamic_rendering || use_dynamic_rendering_inherited; }
    // These helpers are because at draw time, we won't know if the values are from VkRenderingInfo or
    // VkCommandBufferInheritanceRenderingInfo
    uint32_t GetDynamicRenderingViewMask() const;
    VkRenderingFlags GetRenderingFlags() const;
    const VkMultisampledRenderToSingleSampledInfoEXT *GetMSRTSSInfo(uint32_t subpass) const;
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
