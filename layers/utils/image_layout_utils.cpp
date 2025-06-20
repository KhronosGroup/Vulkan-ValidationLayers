/* Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
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

#include "image_layout_utils.h"

static VkImageLayout NormalizeDepthImageLayout(VkImageLayout layout) {
    switch (layout) {
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
            return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
            return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

        default:
            return layout;
    }
}

static VkImageLayout NormalizeStencilImageLayout(VkImageLayout layout) {
    switch (layout) {
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
            return VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
            return VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;

        default:
            return layout;
    }
}

VkImageLayout NormalizeSynchronization2Layout(const VkImageAspectFlags aspect_mask, VkImageLayout layout) {
    if (layout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL) {
        if (aspect_mask == VK_IMAGE_ASPECT_COLOR_BIT) {
            layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        } else if (aspect_mask == (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) {
            layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        } else if (aspect_mask == VK_IMAGE_ASPECT_DEPTH_BIT) {
            layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        } else if (aspect_mask == VK_IMAGE_ASPECT_STENCIL_BIT) {
            layout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        }
    } else if (layout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL) {
        if (aspect_mask == VK_IMAGE_ASPECT_COLOR_BIT) {
            layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        } else if (aspect_mask == (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) {
            layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        } else if (aspect_mask == VK_IMAGE_ASPECT_DEPTH_BIT) {
            layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
        } else if (aspect_mask == VK_IMAGE_ASPECT_STENCIL_BIT) {
            layout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
        }
    }
    return layout;
}

bool ImageLayoutMatches(const VkImageAspectFlags aspect_mask, VkImageLayout a, VkImageLayout b) {
    if (a == b) {
        return true;
    }
    // Map sync2 generic layouts (ATTACHMENT_OPTIMAL, READ_ONLY_OPTIMAL) to equivalent basic layouts
    a = NormalizeSynchronization2Layout(aspect_mask, a);
    b = NormalizeSynchronization2Layout(aspect_mask, b);
    if (a == b) {
        return true;
    }
    // Relaxed rules when referencing *only* the depth or stencil aspects.
    // When accessing both, normalize layouts for aspects separately.
    if (aspect_mask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) {
        bool matches = true;
        if ((aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) != 0) {
            matches &= NormalizeDepthImageLayout(a) == NormalizeDepthImageLayout(b);
        }
        if ((aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) != 0) {
            matches &= NormalizeStencilImageLayout(a) == NormalizeStencilImageLayout(b);
        }
        return matches;
    }
    return false;
}
