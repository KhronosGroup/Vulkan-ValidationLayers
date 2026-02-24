/* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (C) 2015-2026 Google Inc.
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

#include "state_tracker/render_pass_state.h"
#include "utils/convert_utils.h"
#include "state_tracker/image_state.h"
#include "containers/span.h"

// Defined according to spec (check VkSubpassDependency documentation)
static VkSubpassDependency2 ImplicitDependencyFromExternal(uint32_t subpass) {
    VkSubpassDependency2 from_external = vku::InitStructHelper();
    from_external.srcSubpass = VK_SUBPASS_EXTERNAL;
    from_external.dstSubpass = subpass;
    from_external.srcStageMask = VK_PIPELINE_STAGE_NONE;
    from_external.dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    from_external.srcAccessMask = 0;
    from_external.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    return from_external;
}

// Defined according to spec (check VkSubpassDependency documentation)
static VkSubpassDependency2 ImplicitDependencyToExternal(uint32_t subpass) {
    VkSubpassDependency2 to_external = vku::InitStructHelper();
    to_external.srcSubpass = subpass;
    to_external.dstSubpass = VK_SUBPASS_EXTERNAL;
    to_external.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    to_external.dstStageMask = VK_PIPELINE_STAGE_NONE;
    to_external.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    to_external.dstAccessMask = 0;
    return to_external;
}

// NOTE: The functions below are only called from the vvl::RenderPass constructor, and use const_cast<> to set up
// members that never change after construction is finished.
static void RecordRenderPassDAG(const VkRenderPassCreateInfo2 *pCreateInfo, vvl::RenderPass &render_pass) {
    auto &self_dependencies = const_cast<std::vector<std::vector<uint32_t>> &>(render_pass.self_dependencies);
    self_dependencies.resize(pCreateInfo->subpassCount);
    auto &subpass_dependency_infos = const_cast<std::vector<SubpassDependencyInfo> &>(render_pass.subpass_dependency_infos);
    subpass_dependency_infos.resize(pCreateInfo->subpassCount);

    for (uint32_t i = 0; i < pCreateInfo->subpassCount; ++i) {
        self_dependencies[i].clear();
        subpass_dependency_infos[i].subpass = i;
    }
    for (uint32_t i = 0; i < pCreateInfo->dependencyCount; ++i) {
        const VkSubpassDependency2 &dependency = pCreateInfo->pDependencies[i];
        const uint32_t src_subpass = dependency.srcSubpass;
        const uint32_t dst_subpass = dependency.dstSubpass;

        if (src_subpass == VK_SUBPASS_EXTERNAL && dst_subpass == VK_SUBPASS_EXTERNAL) {
            // Invalid per VUID-VkSubpassDependency-srcSubpass-00865
            continue;
        }
        if (src_subpass == dst_subpass) {
            self_dependencies[dependency.srcSubpass].push_back(i);
        } else if (src_subpass == VK_SUBPASS_EXTERNAL) {
            subpass_dependency_infos[dst_subpass].barrier_from_external.emplace_back(&dependency);
        } else if (dst_subpass == VK_SUBPASS_EXTERNAL) {
            subpass_dependency_infos[src_subpass].barrier_to_external.emplace_back(&dependency);
        } else {
            subpass_dependency_infos[dst_subpass].dependencies[src_subpass].emplace_back(&dependency);
        }
    }

    // If no barriers to external are provided for a given subpass, add them.
    // This is used for initialLayout/finalLayout transitions when corresponding
    // subpass is the first/last subpass that uses the attachment.
    for (SubpassDependencyInfo &info : subpass_dependency_infos) {
        if (info.barrier_from_external.empty()) {
            info.implicit_barrier_from_external = ImplicitDependencyFromExternal(info.subpass);
            info.barrier_from_external.emplace_back(&info.implicit_barrier_from_external);
        }
        if (info.barrier_to_external.empty()) {
            info.implicit_barrier_to_external = ImplicitDependencyToExternal(info.subpass);
            info.barrier_to_external.emplace_back(&info.implicit_barrier_to_external);
        }
    }

    //
    // Determine "asynchrononous" subpassess
    // syncronization is only interested in asyncronous stages *earlier* that the current one... so we'll only look towards those.
    // NOTE: This is O(N^3), which we could shrink to O(N^2logN) using sets instead of arrays, but given that N is likely to be
    // small and the K for |= from the prev is must less than for set, we'll accept the brute force.
    std::vector<std::vector<bool>> pass_depends(pCreateInfo->subpassCount);
    for (uint32_t i = 1; i < pCreateInfo->subpassCount; ++i) {
        auto &depends = pass_depends[i];
        depends.resize(i);
        SubpassDependencyInfo &info = subpass_dependency_infos[i];
        for (const auto &[src_subpass, _] : info.dependencies) {
            const auto &src_depends = pass_depends[src_subpass];
            for (uint32_t j = 0; j < src_subpass; j++) {
                depends[j] = depends[j] || src_depends[j];
            }
            depends[src_subpass] = true;
        }
        for (uint32_t subpass = 0; subpass < info.subpass; subpass++) {
            if (!depends[subpass]) {
                info.async.push_back(subpass);
            }
        }
    }
}

struct AttachmentTracker {  // This is really only of local interest, but a bit big for a lambda
    vvl::RenderPass &rp;
    std::vector<uint32_t> &first;
    std::vector<uint32_t> &last;
    std::vector<std::vector<vvl::RenderPass::AttachmentTransition>> &subpass_transitions;
    const uint32_t attachment_count;
    std::vector<VkImageLayout> attachment_layout;
    std::vector<std::vector<VkImageLayout>> subpass_attachment_layout;
    explicit AttachmentTracker(vvl::RenderPass &render_pass)
        : rp(render_pass),
          first(const_cast<std::vector<uint32_t> &>(rp.attachment_first_subpass)),
          last(const_cast<std::vector<uint32_t> &>(rp.attachment_last_subpass)),
          subpass_transitions(
              const_cast<std::vector<std::vector<vvl::RenderPass::AttachmentTransition>> &>(rp.subpass_transitions)),
          attachment_count(rp.create_info.attachmentCount),
          attachment_layout(),
          subpass_attachment_layout() {
        first.resize(attachment_count, VK_SUBPASS_EXTERNAL);
        last.resize(attachment_count, VK_SUBPASS_EXTERNAL);
        subpass_transitions.resize(rp.create_info.subpassCount + 1);  // Add an extra for EndRenderPass
        attachment_layout.reserve(attachment_count);
        subpass_attachment_layout.resize(rp.create_info.subpassCount);
        for (auto &subpass_layouts : subpass_attachment_layout) {
            subpass_layouts.resize(attachment_count, kInvalidLayout);
        }

        for (uint32_t j = 0; j < attachment_count; j++) {
            attachment_layout.push_back(rp.create_info.pAttachments[j].initialLayout);
        }
    }

    void Update(uint32_t subpass, const uint32_t *preserved, uint32_t count) {
        // for preserved attachment, preserve the layout from the most recent (max subpass) dependency
        // or initial, if none

        // max_prev is invariant across attachments
        uint32_t max_prev = VK_SUBPASS_EXTERNAL;
        for (const auto &[src_subpass, _] : rp.subpass_dependency_infos[subpass].dependencies) {
            max_prev = (max_prev == VK_SUBPASS_EXTERNAL) ? src_subpass : std::max(src_subpass, max_prev);
        }

        for (const auto attachment : vvl::make_span(preserved, count)) {
            if (max_prev == VK_SUBPASS_EXTERNAL) {
                subpass_attachment_layout[subpass][attachment] = rp.create_info.pAttachments[attachment].initialLayout;
            } else {
                subpass_attachment_layout[subpass][attachment] = subpass_attachment_layout[max_prev][attachment];
            }
        }
    }

    void Update(uint32_t subpass, const VkAttachmentReference2 *attach_ref, uint32_t count, bool is_read) {
        if (nullptr == attach_ref) return;
        for (uint32_t j = 0; j < count; ++j) {
            const auto attachment = attach_ref[j].attachment;
            if (attachment != VK_ATTACHMENT_UNUSED) {
                const auto layout = attach_ref[j].layout;
                const auto initial_layout = rp.create_info.pAttachments[attachment].initialLayout;
                bool no_external_transition = true;
                if (first[attachment] == VK_SUBPASS_EXTERNAL) {
                    first[attachment] = subpass;
                    if (initial_layout != layout) {
                        subpass_transitions[subpass].emplace_back(
                            vvl::RenderPass::AttachmentTransition{VK_SUBPASS_EXTERNAL, attachment, initial_layout, layout});
                        no_external_transition = false;
                    }
                }
                last[attachment] = subpass;

                for (const auto &[src_subpass, _] : rp.subpass_dependency_infos[subpass].dependencies) {
                    const auto prev_layout = subpass_attachment_layout[src_subpass][attachment];
                    if ((prev_layout != kInvalidLayout) && (prev_layout != layout)) {
                        subpass_transitions[subpass].emplace_back(
                            vvl::RenderPass::AttachmentTransition{src_subpass, attachment, prev_layout, layout});
                    }
                }

                if (no_external_transition && (rp.subpass_dependency_infos[subpass].dependencies.empty())) {
                    // This will insert a layout transition when dependencies are missing between first and subsequent use
                    // but is consistent with the idea of an implicit external dependency
                    if (initial_layout != layout) {
                        subpass_transitions[subpass].emplace_back(
                            vvl::RenderPass::AttachmentTransition{VK_SUBPASS_EXTERNAL, attachment, initial_layout, layout});
                    }
                }

                attachment_layout[attachment] = layout;
                subpass_attachment_layout[subpass][attachment] = layout;
            }
        }
    }
    void FinalTransitions() {
        auto &final_transitions = subpass_transitions[rp.create_info.subpassCount];

        for (uint32_t attachment = 0; attachment < attachment_count; ++attachment) {
            const auto final_layout = rp.create_info.pAttachments[attachment].finalLayout;
            // Add final transitions for attachments that were used and change layout.
            if ((last[attachment] != VK_SUBPASS_EXTERNAL) && final_layout != attachment_layout[attachment]) {
                final_transitions.emplace_back(vvl::RenderPass::AttachmentTransition{last[attachment], attachment,
                                                                                     attachment_layout[attachment], final_layout});
            }
        }
    }
};

static bool IsRenderPassMultiViewEnabled(const VkRenderPassCreateInfo2 &renderpass_ci) {
    // From the spec:
    // If the VkSubpassDescription2::viewMask member of any element of pSubpasses is not zero,
    // multiview functionality is considered to be enabled for this render pass.
    bool is_multiview_enabled = false;
    for (uint32_t subpass_index = 0; subpass_index < renderpass_ci.subpassCount; subpass_index++) {
        const VkSubpassDescription2 &subpass = renderpass_ci.pSubpasses[subpass_index];
        is_multiview_enabled |= (subpass.viewMask != 0);
    }
    return is_multiview_enabled;
}

static void InitRenderPassState(vvl::RenderPass &render_pass) {
    auto create_info = render_pass.create_info.ptr();

    RecordRenderPassDAG(create_info, render_pass);

    AttachmentTracker attachment_tracker(render_pass);

    for (uint32_t subpass_index = 0; subpass_index < create_info->subpassCount; ++subpass_index) {
        const VkSubpassDescription2 &subpass = create_info->pSubpasses[subpass_index];
        attachment_tracker.Update(subpass_index, subpass.pColorAttachments, subpass.colorAttachmentCount, false);
        attachment_tracker.Update(subpass_index, subpass.pResolveAttachments, subpass.colorAttachmentCount, false);
        attachment_tracker.Update(subpass_index, subpass.pDepthStencilAttachment, 1, false);
        attachment_tracker.Update(subpass_index, subpass.pInputAttachments, subpass.inputAttachmentCount, true);
        attachment_tracker.Update(subpass_index, subpass.pPreserveAttachments, subpass.preserveAttachmentCount);
    }
    attachment_tracker.FinalTransitions();
}

namespace vvl {

// vkCreateRenderPass2
RenderPass::RenderPass(VkRenderPass handle, VkRenderPassCreateInfo2 const* pCreateInfo)
    : StateObject(handle, kVulkanObjectTypeRenderPass),
      create_info(pCreateInfo),
      use_dynamic_rendering(false),
      use_dynamic_rendering_inherited(false),
      dynamic_rendering_color_attachment_count(0),
      has_multiview_enabled(IsRenderPassMultiViewEnabled(*create_info.ptr())) {
    InitRenderPassState(*this);
}

static vku::safe_VkRenderPassCreateInfo2 ConvertCreateInfo(const VkRenderPassCreateInfo &create_info) {
    vku::safe_VkRenderPassCreateInfo2 create_info_2 = ConvertVkRenderPassCreateInfoToV2KHR(create_info);
    return create_info_2;
}

// vkCreateRenderPass
RenderPass::RenderPass(VkRenderPass handle, VkRenderPassCreateInfo const* pCreateInfo)
    : StateObject(handle, kVulkanObjectTypeRenderPass),
      create_info(ConvertCreateInfo(*pCreateInfo)),
      use_dynamic_rendering(false),
      use_dynamic_rendering_inherited(false),
      dynamic_rendering_color_attachment_count(0),
      has_multiview_enabled(IsRenderPassMultiViewEnabled(*create_info.ptr())) {
    InitRenderPassState(*this);
}

// vkCreateGraphicsPipelines (dynamic rendering state tied to pipeline state)
// (created in DeviceState::PreCallValidateCreateGraphicsPipelines)
RenderPass::RenderPass(const VkPipelineRenderingCreateInfo& rendering_ci)
    : StateObject(static_cast<VkRenderPass>(VK_NULL_HANDLE), kVulkanObjectTypeRenderPass),
      use_dynamic_rendering(true),
      use_dynamic_rendering_inherited(false),
      dynamic_pipeline_rendering_create_info(&rendering_ci),
      dynamic_rendering_color_attachment_count(rendering_ci.colorAttachmentCount),
      has_multiview_enabled(rendering_ci.viewMask != 0) {}

bool RenderPass::UsesColorAttachment(uint32_t subpass_num) const {
    bool result = false;

    if (subpass_num < create_info.subpassCount) {
        const auto &subpass = create_info.pSubpasses[subpass_num];

        for (uint32_t i = 0; i < subpass.colorAttachmentCount; ++i) {
            if (subpass.pColorAttachments[i].attachment != VK_ATTACHMENT_UNUSED) {
                result = true;
                break;
            }
        }

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
        // VK_ANDROID_external_format_resolve allows for the only color attachment to be VK_ATTACHMENT_UNUSED
        // but in this case, it will use the resolve attachment as color attachment. Which means that we do
        // actually use color attachments
        if (subpass.pResolveAttachments != nullptr) {
            for (uint32_t i = 0; i < subpass.colorAttachmentCount && !result; ++i) {
                uint32_t resolveAttachmentIndex = subpass.pResolveAttachments[i].attachment;
                const void *resolveAtatchmentPNextChain = create_info.pAttachments[resolveAttachmentIndex].pNext;
                if (vku::FindStructInPNextChain<VkExternalFormatANDROID>(resolveAtatchmentPNextChain)) result = true;
            }
        }
#endif
    }
    return result;
}

bool RenderPass::UsesDepthStencilAttachment(uint32_t subpass_num) const {
    bool result = false;
    if (subpass_num < create_info.subpassCount) {
        const auto &subpass = create_info.pSubpasses[subpass_num];
        if (subpass.pDepthStencilAttachment && subpass.pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
            result = true;
        }
    }
    return result;
}

// vkspec.html#renderpass-noattachments
bool RenderPass::UsesNoAttachment(uint32_t subpass) const {
    // If using dynamic rendering, there is no subpass, so return 'false'
    return !UsesDynamicRendering() && !UsesColorAttachment(subpass) && !UsesDepthStencilAttachment(subpass);
}

uint32_t RenderPass::GetDynamicRenderingViewMask() const {
    if (use_dynamic_rendering_inherited) {
        return inheritance_rendering_info.viewMask;
    } else if (use_dynamic_rendering) {
        return dynamic_rendering_begin_rendering_info.viewMask;
    }
    return 0;
}

VkRenderingFlags RenderPass::GetRenderingFlags() const {
    if (use_dynamic_rendering_inherited) {
        return inheritance_rendering_info.flags;
    } else if (use_dynamic_rendering) {
        return dynamic_rendering_begin_rendering_info.flags;
    }
    return 0;
}

const VkMultisampledRenderToSingleSampledInfoEXT *RenderPass::GetMSRTSSInfo(uint32_t subpass) const {
    if (UsesDynamicRendering()) {
        return vku::FindStructInPNextChain<VkMultisampledRenderToSingleSampledInfoEXT>(
            dynamic_rendering_begin_rendering_info.pNext);
    }
    return vku::FindStructInPNextChain<VkMultisampledRenderToSingleSampledInfoEXT>(create_info.pSubpasses[subpass].pNext);
}

// vkCmdBeginRendering
RenderPass::RenderPass(const VkRenderingInfo& rendering_info)
    : StateObject(static_cast<VkRenderPass>(VK_NULL_HANDLE), kVulkanObjectTypeRenderPass),
      use_dynamic_rendering(true),
      use_dynamic_rendering_inherited(false),
      dynamic_rendering_begin_rendering_info(&rendering_info),
      dynamic_rendering_color_attachment_count(dynamic_rendering_begin_rendering_info.colorAttachmentCount),
      has_multiview_enabled(rendering_info.viewMask != 0u) {}

// vkBeginCommandBuffer (dynamic rendering in secondary command buffer)
RenderPass::RenderPass(VkCommandBufferInheritanceRenderingInfo const* pInheritanceRenderingInfo)
    : StateObject(static_cast<VkRenderPass>(VK_NULL_HANDLE), kVulkanObjectTypeRenderPass),
      use_dynamic_rendering(false),
      use_dynamic_rendering_inherited(true),
      inheritance_rendering_info(pInheritanceRenderingInfo),
      dynamic_rendering_color_attachment_count(inheritance_rendering_info.colorAttachmentCount),
      has_multiview_enabled(false) {}

Framebuffer::Framebuffer(VkFramebuffer handle, const VkFramebufferCreateInfo *pCreateInfo, std::shared_ptr<RenderPass> &&rpstate,
                         std::vector<std::shared_ptr<vvl::ImageView>> &&attachments)
    : StateObject(handle, kVulkanObjectTypeFramebuffer),
      safe_create_info(pCreateInfo),
      create_info(*safe_create_info.ptr()),
      rp_state(rpstate),
      attachments_view_state(std::move(attachments)) {}

void Framebuffer::LinkChildNodes() {
    // Connect child node(s), which cannot safely be done in the constructor.
    for (auto &a : attachments_view_state) {
        a->AddParent(this);
    }
}

void Framebuffer::Destroy() {
    for (auto &view : attachments_view_state) {
        view->RemoveParent(this);
    }
    attachments_view_state.clear();
    StateObject::Destroy();
}

}  // namespace vvl
