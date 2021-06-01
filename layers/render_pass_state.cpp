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
#include "render_pass_state.h"
#include "convert_to_renderpass2.h"
#include "image_state.h"

static const VkImageLayout kInvalidLayout = VK_IMAGE_LAYOUT_MAX_ENUM;

static void RecordRenderPassDAG(const VkRenderPassCreateInfo2 *pCreateInfo, RENDER_PASS_STATE *render_pass) {
    auto &subpass_to_node = render_pass->subpassToNode;
    subpass_to_node.resize(pCreateInfo->subpassCount);
    auto &self_dependencies = render_pass->self_dependencies;
    self_dependencies.resize(pCreateInfo->subpassCount);
    auto &subpass_dependencies = render_pass->subpass_dependencies;
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
                depends[j] = depends[j] | prev_depends[j];
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

struct AttachmentTracker {  // This is really only of local interest, but a bit big for a lambda
    RENDER_PASS_STATE *const rp;
    std::vector<uint32_t> &first;
    std::vector<bool> &first_is_transition;
    std::vector<uint32_t> &last;
    std::vector<std::vector<RENDER_PASS_STATE::AttachmentTransition>> &subpass_transitions;
    layer_data::unordered_map<uint32_t, bool> &first_read;
    const uint32_t attachment_count;
    std::vector<VkImageLayout> attachment_layout;
    std::vector<std::vector<VkImageLayout>> subpass_attachment_layout;
    explicit AttachmentTracker(RENDER_PASS_STATE *render_pass)
        : rp(render_pass),
          first(rp->attachment_first_subpass),
          first_is_transition(rp->attachment_first_is_transition),
          last(rp->attachment_last_subpass),
          subpass_transitions(rp->subpass_transitions),
          first_read(rp->attachment_first_read),
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

    void Update(uint32_t subpass, const VkAttachmentReference2 *attach_ref, uint32_t count, bool is_read) {
        if (nullptr == attach_ref) return;
        for (uint32_t j = 0; j < count; ++j) {
            const auto attachment = attach_ref[j].attachment;
            if (attachment != VK_ATTACHMENT_UNUSED) {
                const auto layout = attach_ref[j].layout;
                // Take advantage of the fact that insert won't overwrite, so we'll only write the first time.
                first_read.emplace(attachment, is_read);
                if (first[attachment] == VK_SUBPASS_EXTERNAL) {
                    first[attachment] = subpass;
                    const auto initial_layout = rp->createInfo.pAttachments[attachment].initialLayout;
                    if (initial_layout != layout) {
                        subpass_transitions[subpass].emplace_back(VK_SUBPASS_EXTERNAL, attachment, initial_layout, layout);
                        first_is_transition[attachment] = true;
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
                attachment_layout[attachment] = layout;
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
    }
    attachment_tracker.FinalTransitions();

    // Add implicit dependencies
    for (uint32_t attachment = 0; attachment < attachment_tracker.attachment_count; attachment++) {
        const auto first_use = attachment_tracker.first[attachment];
        if (first_use != VK_SUBPASS_EXTERNAL) {
            auto &subpass_dep = render_pass->subpass_dependencies[first_use];
            if (subpass_dep.barrier_from_external.size() == 0) {
                // Add implicit from barrier if they're aren't any
                subpass_dep.implicit_barrier_from_external.reset(
                    new VkSubpassDependency2(ImplicitDependencyFromExternal(first_use)));
                subpass_dep.barrier_from_external.emplace_back(subpass_dep.implicit_barrier_from_external.get());
            }
        }

        const auto last_use = attachment_tracker.last[attachment];
        if (last_use != VK_SUBPASS_EXTERNAL) {
            auto &subpass_dep = render_pass->subpass_dependencies[last_use];
            if (render_pass->subpass_dependencies[last_use].barrier_to_external.size() == 0) {
                // Add implicit to barrier  if they're aren't any
                subpass_dep.implicit_barrier_to_external.reset(new VkSubpassDependency2(ImplicitDependencyToExternal(last_use)));
                subpass_dep.barrier_to_external.emplace_back(subpass_dep.implicit_barrier_to_external.get());
            }
        }
    }
}

RENDER_PASS_STATE::RENDER_PASS_STATE(VkRenderPass rp, VkRenderPassCreateInfo2 const *pCreateInfo)
    : BASE_NODE(rp, kVulkanObjectTypeRenderPass), createInfo(pCreateInfo) {
    InitRenderPassState(this);
}

RENDER_PASS_STATE::RENDER_PASS_STATE(VkRenderPass rp, VkRenderPassCreateInfo const *pCreateInfo)
    : BASE_NODE(rp, kVulkanObjectTypeRenderPass) {
    ConvertVkRenderPassCreateInfoToV2KHR(*pCreateInfo, &createInfo);
    InitRenderPassState(this);
}

FRAMEBUFFER_STATE::FRAMEBUFFER_STATE(VkFramebuffer fb, const VkFramebufferCreateInfo *pCreateInfo,
                                     std::shared_ptr<RENDER_PASS_STATE> &&rpstate,
                                     std::vector<std::shared_ptr<IMAGE_VIEW_STATE>> &&attachments)
    : BASE_NODE(fb, kVulkanObjectTypeFramebuffer), createInfo(pCreateInfo), rp_state(rpstate) {
    for (auto &a : attachments) {
        a->AddParent(this);
    }
    attachments_view_state = std::move(attachments);
}

void FRAMEBUFFER_STATE::Destroy() {
    for (auto& view: attachments_view_state) {
        view->RemoveParent(this);
    }
    attachments_view_state.clear();
    BASE_NODE::Destroy();
}

