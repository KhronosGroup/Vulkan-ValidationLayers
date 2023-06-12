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

#include "state_tracker/render_pass_state.h"
#include "utils/convert_utils.h"
#include "state_tracker/image_state.h"

static const VkImageLayout kInvalidLayout = VK_IMAGE_LAYOUT_MAX_ENUM;

static VkSubpassDependency2 ImplicitDependencyFromExternal(uint32_t subpass) {
    VkSubpassDependency2 from_external = {VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
                                          nullptr,
                                          VK_SUBPASS_EXTERNAL,
                                          subpass,
                                          VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                          VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                          0,
                                          VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                              VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                              VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                          0,
                                          0};
    return from_external;
}

static VkSubpassDependency2 ImplicitDependencyToExternal(uint32_t subpass) {
    VkSubpassDependency2 to_external = {VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
                                        nullptr,
                                        subpass,
                                        VK_SUBPASS_EXTERNAL,
                                        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                        VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                        0,
                                        0,
                                        0};
    return to_external;
}
// NOTE: The functions below are only called from the RENDER_PASS_STATE constructor, and use const_cast<> to set up
// members that never change after construction is finished.
static void RecordRenderPassDAG(const VkRenderPassCreateInfo2 *pCreateInfo, RENDER_PASS_STATE *render_pass) {
    auto &subpass_to_node = const_cast<RENDER_PASS_STATE::DAGNodeVec &>(render_pass->subpass_to_node);
    subpass_to_node.resize(pCreateInfo->subpassCount);
    auto &self_dependencies = const_cast<RENDER_PASS_STATE::SelfDepVec &>(render_pass->self_dependencies);
    self_dependencies.resize(pCreateInfo->subpassCount);
    auto &subpass_dependencies = const_cast<RENDER_PASS_STATE::SubpassGraphVec &>(render_pass->subpass_dependencies);
    subpass_dependencies.resize(pCreateInfo->subpassCount);

    for (uint32_t i = 0; i < pCreateInfo->subpassCount; ++i) {
        subpass_to_node[i].pass = i;
        self_dependencies[i].clear();
        subpass_dependencies[i].pass = i;
    }
    for (uint32_t i = 0; i < pCreateInfo->dependencyCount; ++i) {
        const auto &dependency = pCreateInfo->pDependencies[i];
        const auto src_subpass = dependency.srcSubpass;
        const auto dst_subpass = dependency.dstSubpass;
        if ((dependency.srcSubpass != VK_SUBPASS_EXTERNAL) && (dependency.dstSubpass != VK_SUBPASS_EXTERNAL)) {
            if (dependency.srcSubpass == dependency.dstSubpass) {
                self_dependencies[dependency.srcSubpass].push_back(i);
            } else {
                subpass_to_node[dependency.dstSubpass].prev.push_back(dependency.srcSubpass);
                subpass_to_node[dependency.srcSubpass].next.push_back(dependency.dstSubpass);
            }
        }
        if (src_subpass == VK_SUBPASS_EXTERNAL) {
            assert(dst_subpass != VK_SUBPASS_EXTERNAL);  // this is invalid per VUID-VkSubpassDependency-srcSubpass-00865
            subpass_dependencies[dst_subpass].barrier_from_external.emplace_back(&dependency);
        } else if (dst_subpass == VK_SUBPASS_EXTERNAL) {
            subpass_dependencies[src_subpass].barrier_to_external.emplace_back(&dependency);
        } else if (dependency.srcSubpass != dependency.dstSubpass) {
            // ignore self dependencies in prev and next
            subpass_dependencies[src_subpass].next[&subpass_dependencies[dst_subpass]].emplace_back(&dependency);
            subpass_dependencies[dst_subpass].prev[&subpass_dependencies[src_subpass]].emplace_back(&dependency);
        }
    }

    // If no barriers to external are provided for a given subpass, add them.
    for (auto &subpass_dep : subpass_dependencies) {
        const uint32_t pass = subpass_dep.pass;
        if (subpass_dep.barrier_from_external.size() == 0) {
            // Add implicit from barrier if they're aren't any
            subpass_dep.implicit_barrier_from_external =
                std::make_unique<VkSubpassDependency2>(ImplicitDependencyFromExternal(pass));
            subpass_dep.barrier_from_external.emplace_back(subpass_dep.implicit_barrier_from_external.get());
        }
        if (subpass_dep.barrier_to_external.size() == 0) {
            // Add implicit to barrier  if they're aren't any
            subpass_dep.implicit_barrier_to_external = std::make_unique<VkSubpassDependency2>(ImplicitDependencyToExternal(pass));
            subpass_dep.barrier_to_external.emplace_back(subpass_dep.implicit_barrier_to_external.get());
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
        auto &subpass_dep = subpass_dependencies[i];
        for (const auto &prev : subpass_dep.prev) {
            const auto prev_pass = prev.first->pass;
            const auto &prev_depends = pass_depends[prev_pass];
            for (uint32_t j = 0; j < prev_pass; j++) {
                depends[j] = depends[j] || prev_depends[j];
            }
            depends[prev_pass] = true;
        }
        for (uint32_t pass = 0; pass < subpass_dep.pass; pass++) {
            if (!depends[pass]) {
                subpass_dep.async.push_back(pass);
            }
        }
    }
}

struct AttachmentTracker {  // This is really only of local interest, but a bit big for a lambda
    RENDER_PASS_STATE *const rp;
    RENDER_PASS_STATE::SubpassVec &first;
    RENDER_PASS_STATE::FirstIsTransitionVec &first_is_transition;
    RENDER_PASS_STATE::SubpassVec &last;
    RENDER_PASS_STATE::TransitionVec &subpass_transitions;
    RENDER_PASS_STATE::FirstReadMap &first_read;
    const uint32_t attachment_count;
    std::vector<VkImageLayout> attachment_layout;
    std::vector<std::vector<VkImageLayout>> subpass_attachment_layout;
    explicit AttachmentTracker(RENDER_PASS_STATE *render_pass)
        : rp(render_pass),
          first(const_cast<RENDER_PASS_STATE::SubpassVec &>(rp->attachment_first_subpass)),
          first_is_transition(const_cast<RENDER_PASS_STATE::FirstIsTransitionVec &>(rp->attachment_first_is_transition)),
          last(const_cast<RENDER_PASS_STATE::SubpassVec &>(rp->attachment_last_subpass)),
          subpass_transitions(const_cast<RENDER_PASS_STATE::TransitionVec &>(rp->subpass_transitions)),
          first_read(const_cast<RENDER_PASS_STATE::FirstReadMap &>(rp->attachment_first_read)),
          attachment_count(rp->createInfo.attachmentCount),
          attachment_layout(),
          subpass_attachment_layout() {
        first.resize(attachment_count, VK_SUBPASS_EXTERNAL);
        first_is_transition.resize(attachment_count, false);
        last.resize(attachment_count, VK_SUBPASS_EXTERNAL);
        subpass_transitions.resize(rp->createInfo.subpassCount + 1);  // Add an extra for EndRenderPass
        attachment_layout.reserve(attachment_count);
        subpass_attachment_layout.resize(rp->createInfo.subpassCount);
        for (auto &subpass_layouts : subpass_attachment_layout) {
            subpass_layouts.resize(attachment_count, kInvalidLayout);
        }

        for (uint32_t j = 0; j < attachment_count; j++) {
            attachment_layout.push_back(rp->createInfo.pAttachments[j].initialLayout);
        }
    }

    void Update(uint32_t subpass, const uint32_t *preserved, uint32_t count) {
        // for preserved attachment, preserve the layout from the most recent (max subpass) dependency
        // or initial, if none

        // max_prev is invariant across attachments
        uint32_t max_prev = VK_SUBPASS_EXTERNAL;
        for (const auto &prev : rp->subpass_dependencies[subpass].prev) {
            const auto prev_pass = prev.first->pass;
            max_prev = (max_prev == VK_SUBPASS_EXTERNAL) ? prev_pass : std::max(prev_pass, max_prev);
        }

        for (const auto attachment : vvl::make_span(preserved, count)) {
            if (max_prev == VK_SUBPASS_EXTERNAL) {
                subpass_attachment_layout[subpass][attachment] = rp->createInfo.pAttachments[attachment].initialLayout;
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
                // Take advantage of the fact that insert won't overwrite, so we'll only write the first time.
                first_read.emplace(attachment, is_read);
                const auto initial_layout = rp->createInfo.pAttachments[attachment].initialLayout;
                bool no_external_transition = true;
                if (first[attachment] == VK_SUBPASS_EXTERNAL) {
                    first[attachment] = subpass;
                    if (initial_layout != layout) {
                        subpass_transitions[subpass].emplace_back(VK_SUBPASS_EXTERNAL, attachment, initial_layout, layout);
                        first_is_transition[attachment] = true;
                        no_external_transition = false;
                    }
                }
                last[attachment] = subpass;

                for (const auto &prev : rp->subpass_dependencies[subpass].prev) {
                    const auto prev_pass = prev.first->pass;
                    const auto prev_layout = subpass_attachment_layout[prev_pass][attachment];
                    if ((prev_layout != kInvalidLayout) && (prev_layout != layout)) {
                        subpass_transitions[subpass].emplace_back(prev_pass, attachment, prev_layout, layout);
                    }
                }

                if (no_external_transition && (rp->subpass_dependencies[subpass].prev.size() == 0)) {
                    // This will insert a layout transition when dependencies are missing between first and subsequent use
                    // but is consistent with the idea of an implicit external dependency
                    if (initial_layout != layout) {
                        subpass_transitions[subpass].emplace_back(VK_SUBPASS_EXTERNAL, attachment, initial_layout, layout);
                    }
                }

                attachment_layout[attachment] = layout;
                subpass_attachment_layout[subpass][attachment] = layout;
            }
        }
    }
    void FinalTransitions() {
        auto &final_transitions = subpass_transitions[rp->createInfo.subpassCount];

        for (uint32_t attachment = 0; attachment < attachment_count; ++attachment) {
            const auto final_layout = rp->createInfo.pAttachments[attachment].finalLayout;
            // Add final transitions for attachments that were used and change layout.
            if ((last[attachment] != VK_SUBPASS_EXTERNAL) && final_layout != attachment_layout[attachment]) {
                final_transitions.emplace_back(last[attachment], attachment, attachment_layout[attachment], final_layout);
            }
        }
    }
};

static void InitRenderPassState(RENDER_PASS_STATE *render_pass) {
    auto create_info = render_pass->createInfo.ptr();

    RecordRenderPassDAG(create_info, render_pass);

    AttachmentTracker attachment_tracker(render_pass);

    for (uint32_t subpass_index = 0; subpass_index < create_info->subpassCount; ++subpass_index) {
        const VkSubpassDescription2 &subpass = create_info->pSubpasses[subpass_index];
        attachment_tracker.Update(subpass_index, subpass.pColorAttachments, subpass.colorAttachmentCount, false);
        attachment_tracker.Update(subpass_index, subpass.pResolveAttachments, subpass.colorAttachmentCount, false);
        attachment_tracker.Update(subpass_index, subpass.pDepthStencilAttachment, 1, false);
        attachment_tracker.Update(subpass_index, subpass.pInputAttachments, subpass.inputAttachmentCount, true);
        attachment_tracker.Update(subpass_index, subpass.pPreserveAttachments, subpass.preserveAttachmentCount);

        // From the spec
        // If the VkSubpassDescription2::viewMask member of any element of pSubpasses is not zero, multiview functionality is
        // considered to be enabled for this render pass.
        (*const_cast<bool *>(&render_pass->has_multiview_enabled)) |= (subpass.viewMask != 0);
    }
    attachment_tracker.FinalTransitions();
}

RENDER_PASS_STATE::RENDER_PASS_STATE(VkRenderPass rp, VkRenderPassCreateInfo2 const *pCreateInfo)
    : BASE_NODE(rp, kVulkanObjectTypeRenderPass),
      use_dynamic_rendering(false),
      use_dynamic_rendering_inherited(false),
      has_multiview_enabled(false),
      createInfo(pCreateInfo) {
    InitRenderPassState(this);
}

static safe_VkRenderPassCreateInfo2 ConvertCreateInfo(const VkRenderPassCreateInfo &create_info) {
    safe_VkRenderPassCreateInfo2 create_info_2 = ConvertVkRenderPassCreateInfoToV2KHR(create_info);
    return create_info_2;
}

RENDER_PASS_STATE::RENDER_PASS_STATE(VkRenderPass rp, VkRenderPassCreateInfo const *pCreateInfo)
    : BASE_NODE(rp, kVulkanObjectTypeRenderPass),
      use_dynamic_rendering(false),
      use_dynamic_rendering_inherited(false),
      has_multiview_enabled(false),
      createInfo(ConvertCreateInfo(*pCreateInfo)) {
    InitRenderPassState(this);
}

const VkPipelineRenderingCreateInfo VkPipelineRenderingCreateInfo_default = {
    VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO, nullptr, 0, 0, nullptr, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED};

RENDER_PASS_STATE::RENDER_PASS_STATE(VkPipelineRenderingCreateInfo const *pPipelineRenderingCreateInfo, bool rasterization_enabled)
    : BASE_NODE(static_cast<VkRenderPass>(VK_NULL_HANDLE), kVulkanObjectTypeRenderPass),
      use_dynamic_rendering(true),
      use_dynamic_rendering_inherited(false),
      has_multiview_enabled(false),
      rasterization_enabled(rasterization_enabled),
      dynamic_rendering_pipeline_create_info((pPipelineRenderingCreateInfo && rasterization_enabled)
                                                 ? pPipelineRenderingCreateInfo
                                                 : &VkPipelineRenderingCreateInfo_default) {}

bool RENDER_PASS_STATE::UsesColorAttachment(uint32_t subpass_num) const {
    bool result = false;

    if (subpass_num < createInfo.subpassCount) {
        const auto &subpass = createInfo.pSubpasses[subpass_num];

        for (uint32_t i = 0; i < subpass.colorAttachmentCount; ++i) {
            if (subpass.pColorAttachments[i].attachment != VK_ATTACHMENT_UNUSED) {
                result = true;
                break;
            }
        }
    }
    return result;
}

bool RENDER_PASS_STATE::UsesDepthStencilAttachment(uint32_t subpass_num) const {
    bool result = false;
    if (subpass_num < createInfo.subpassCount) {
        const auto &subpass = createInfo.pSubpasses[subpass_num];
        if (subpass.pDepthStencilAttachment && subpass.pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
            result = true;
        }
    }
    return result;
}

uint32_t RENDER_PASS_STATE::GetDynamicRenderingColorAttachmentCount() const {
    if (use_dynamic_rendering_inherited) {
        return inheritance_rendering_info.colorAttachmentCount;
    } else if (use_dynamic_rendering) {
        return dynamic_rendering_begin_rendering_info.colorAttachmentCount;
    }
    return 0;
}

uint32_t RENDER_PASS_STATE::GetDynamicRenderingViewMask() const {
    if (use_dynamic_rendering_inherited) {
        return inheritance_rendering_info.viewMask;
    } else if (use_dynamic_rendering) {
        return dynamic_rendering_begin_rendering_info.viewMask;
    }
    return 0;
}

uint32_t RENDER_PASS_STATE::GetViewMaskBits(uint32_t subpass) const {
    if (use_dynamic_rendering_inherited) {
        return GetBitSetCount(inheritance_rendering_info.viewMask);
    } else if (use_dynamic_rendering) {
        return GetBitSetCount(dynamic_rendering_begin_rendering_info.viewMask);
    } else {
        const auto *subpass_desc = &createInfo.pSubpasses[subpass];
        if (subpass_desc) {
            return GetBitSetCount(subpass_desc->viewMask);
        }
    }
    return 0;
}

RENDER_PASS_STATE::RENDER_PASS_STATE(VkRenderingInfo const *pRenderingInfo, bool rasterization_enabled)
    : BASE_NODE(static_cast<VkRenderPass>(VK_NULL_HANDLE), kVulkanObjectTypeRenderPass),
      use_dynamic_rendering(true),
      use_dynamic_rendering_inherited(false),
      has_multiview_enabled(false),
      rasterization_enabled(rasterization_enabled),
      dynamic_rendering_begin_rendering_info((pRenderingInfo && rasterization_enabled) ? pRenderingInfo : nullptr) {}

RENDER_PASS_STATE::RENDER_PASS_STATE(VkCommandBufferInheritanceRenderingInfo const *pInheritanceRenderingInfo)
    : BASE_NODE(static_cast<VkRenderPass>(VK_NULL_HANDLE), kVulkanObjectTypeRenderPass),
      use_dynamic_rendering(false),
      use_dynamic_rendering_inherited(true),
      has_multiview_enabled(false),
      inheritance_rendering_info(pInheritanceRenderingInfo) {}

FRAMEBUFFER_STATE::FRAMEBUFFER_STATE(VkFramebuffer fb, const VkFramebufferCreateInfo *pCreateInfo,
                                     std::shared_ptr<RENDER_PASS_STATE> &&rpstate,
                                     std::vector<std::shared_ptr<IMAGE_VIEW_STATE>> &&attachments)
    : BASE_NODE(fb, kVulkanObjectTypeFramebuffer),
      createInfo(pCreateInfo),
      rp_state(rpstate),
      attachments_view_state(std::move(attachments)) {}

void FRAMEBUFFER_STATE::LinkChildNodes() {
    // Connect child node(s), which cannot safely be done in the constructor.
    for (auto &a : attachments_view_state) {
        a->AddParent(this);
    }
}

void FRAMEBUFFER_STATE::Destroy() {
    for (auto &view : attachments_view_state) {
        view->RemoveParent(this);
    }
    attachments_view_state.clear();
    BASE_NODE::Destroy();
}
