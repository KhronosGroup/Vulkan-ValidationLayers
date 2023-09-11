/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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

#include "stateless/stateless_validation.h"

bool StatelessValidation::manual_PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
                                                                  const VkAllocationCallbacks *pAllocator,
                                                                  VkFramebuffer *pFramebuffer, const ErrorObject &error_obj) const {
    // Validation for pAttachments which is excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    bool skip = false;
    if ((pCreateInfo->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) == 0) {
        skip |= ValidateArray(error_obj.location.dot(Field::attachmentCount), error_obj.location.dot(Field::pAttachments),
                              pCreateInfo->attachmentCount, &pCreateInfo->pAttachments, false, true, kVUIDUndefined,
                              "VUID-VkFramebufferCreateInfo-flags-02778");
    }

    // Verify FB dimensions are greater than zero
    if (pCreateInfo->width == 0) {
        skip |= LogError("VUID-VkFramebufferCreateInfo-width-00885", device,
                         error_obj.location.dot(Field::pCreateInfo).dot(Field::width), "is zero.");
    }
    if (pCreateInfo->height == 0) {
        skip |= LogError("VUID-VkFramebufferCreateInfo-height-00887", device,
                         error_obj.location.dot(Field::pCreateInfo).dot(Field::height), "is zero.");
    }
    if (pCreateInfo->layers == 0) {
        skip |= LogError("VUID-VkFramebufferCreateInfo-layers-00889", device,
                         error_obj.location.dot(Field::pCreateInfo).dot(Field::layers), "is zero.");
    }
    return skip;
}
