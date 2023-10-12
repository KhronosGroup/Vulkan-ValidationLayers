/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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

#include "best_practices/best_practices_validation.h"
#include "best_practices/best_practices_error_enums.h"

bool BestPractices::ValidateAttachments(const VkRenderPassCreateInfo2* rpci, uint32_t attachmentCount,
                                        const VkImageView* image_views, const Location& loc) const {
    bool skip = false;

    // Check for non-transient attachments that should be transient and vice versa
    for (uint32_t i = 0; i < attachmentCount; ++i) {
        const auto& attachment = rpci->pAttachments[i];
        bool attachment_should_be_transient =
            (attachment.loadOp != VK_ATTACHMENT_LOAD_OP_LOAD && attachment.storeOp != VK_ATTACHMENT_STORE_OP_STORE);

        if (vkuFormatHasStencil(attachment.format)) {
            attachment_should_be_transient &= (attachment.stencilLoadOp != VK_ATTACHMENT_LOAD_OP_LOAD &&
                                               attachment.stencilStoreOp != VK_ATTACHMENT_STORE_OP_STORE);
        }

        auto view_state = Get<IMAGE_VIEW_STATE>(image_views[i]);
        if (view_state) {
            const auto& ici = view_state->image_state->createInfo;

            const bool image_is_transient = (ici.usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) != 0;

            // The check for an image that should not be transient applies to all GPUs
            if (!attachment_should_be_transient && image_is_transient) {
                skip |= LogPerformanceWarning(
                    kVUID_BestPractices_CreateFramebuffer_AttachmentShouldNotBeTransient, device, loc,
                    "Attachment %u in VkFramebuffer uses loadOp/storeOps which need to access physical memory, "
                    "but the image backing the image view has VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT set. "
                    "Physical memory will need to be backed lazily to this image, potentially causing stalls.",
                    i);
            }

            bool supports_lazy = false;
            for (uint32_t j = 0; j < phys_dev_mem_props.memoryTypeCount; j++) {
                if (phys_dev_mem_props.memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) {
                    supports_lazy = true;
                }
            }

            // The check for an image that should be transient only applies to GPUs supporting
            // lazily allocated memory
            if (supports_lazy && attachment_should_be_transient && !image_is_transient) {
                skip |= LogPerformanceWarning(
                    kVUID_BestPractices_CreateFramebuffer_AttachmentShouldBeTransient, device, loc,
                    "Attachment %u in VkFramebuffer uses loadOp/storeOps which never have to be backed by physical memory, "
                    "but the image backing the image view does not have VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT set. "
                    "You can save physical memory by using transient attachment backed by lazily allocated memory here.",
                    i);
            }
        }
    }
    return skip;
}

bool BestPractices::PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer,
                                                     const ErrorObject& error_obj) const {
    bool skip = false;

    auto rp_state = Get<RENDER_PASS_STATE>(pCreateInfo->renderPass);
    if (rp_state && !(pCreateInfo->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT)) {
        skip = ValidateAttachments(rp_state->createInfo.ptr(), pCreateInfo->attachmentCount, pCreateInfo->pAttachments,
                                   error_obj.location);
    }

    return skip;
}
