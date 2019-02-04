/* Copyright (c) 2015-2019 The Khronos Group Inc.
 * Copyright (c) 2015-2019 Valve Corporation
 * Copyright (c) 2015-2019 LunarG, Inc.
 * Copyright (C) 2015-2019 Google Inc.
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
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Shannon McPherson <shannon@lunarg.com>
 */

// Allow use of STL min and max functions in Windows
#define NOMINMAX

#include <cmath>
#include <set>
#include <sstream>
#include <string>

#include "vk_enum_string_helper.h"
#include "vk_layer_data.h"
#include "vk_layer_utils.h"
#include "vk_layer_logging.h"
#include "vk_typemap_helper.h"

#include "buffer_validation.h"

namespace core_validation {
extern unordered_map<void *, layer_data *> layer_data_map;
extern unordered_map<void *, instance_layer_data *> instance_layer_data_map;
};  // namespace core_validation

using core_validation::instance_layer_data_map;
using core_validation::layer_data_map;

uint32_t FullMipChainLevels(uint32_t height, uint32_t width, uint32_t depth) {
    // uint cast applies floor()
    return 1u + (uint32_t)log2(std::max({height, width, depth}));
}

uint32_t FullMipChainLevels(VkExtent3D extent) { return FullMipChainLevels(extent.height, extent.width, extent.depth); }

uint32_t FullMipChainLevels(VkExtent2D extent) { return FullMipChainLevels(extent.height, extent.width); }

void SetLayout(layer_data *device_data, GLOBAL_CB_NODE *pCB, ImageSubresourcePair imgpair, const VkImageLayout &layout) {
    auto it = pCB->imageLayoutMap.find(imgpair);
    if (it != pCB->imageLayoutMap.end()) {
        it->second.layout = layout;
    } else {
        assert(imgpair.hasSubresource);
        IMAGE_CMD_BUF_LAYOUT_NODE node;
        if (!FindCmdBufLayout(device_data, pCB, imgpair.image, imgpair.subresource, node)) {
            node.initialLayout = layout;
        }
        SetLayout(device_data, pCB, imgpair, {node.initialLayout, layout});
    }
}
template <class OBJECT, class LAYOUT>
void SetLayout(layer_data *device_data, OBJECT *pObject, VkImage image, VkImageSubresource range, const LAYOUT &layout) {
    ImageSubresourcePair imgpair = {image, true, range};
    SetLayout(device_data, pObject, imgpair, layout, VK_IMAGE_ASPECT_COLOR_BIT);
    SetLayout(device_data, pObject, imgpair, layout, VK_IMAGE_ASPECT_DEPTH_BIT);
    SetLayout(device_data, pObject, imgpair, layout, VK_IMAGE_ASPECT_STENCIL_BIT);
    SetLayout(device_data, pObject, imgpair, layout, VK_IMAGE_ASPECT_METADATA_BIT);
    if (GetDeviceExtensions(device_data)->vk_khr_sampler_ycbcr_conversion) {
        SetLayout(device_data, pObject, imgpair, layout, VK_IMAGE_ASPECT_PLANE_0_BIT_KHR);
        SetLayout(device_data, pObject, imgpair, layout, VK_IMAGE_ASPECT_PLANE_1_BIT_KHR);
        SetLayout(device_data, pObject, imgpair, layout, VK_IMAGE_ASPECT_PLANE_2_BIT_KHR);
    }
}

template <class OBJECT, class LAYOUT>
void SetLayout(layer_data *device_data, OBJECT *pObject, ImageSubresourcePair imgpair, const LAYOUT &layout,
               VkImageAspectFlags aspectMask) {
    if (imgpair.subresource.aspectMask & aspectMask) {
        imgpair.subresource.aspectMask = aspectMask;
        SetLayout(device_data, pObject, imgpair, layout);
    }
}

// Set the layout in supplied map
void SetLayout(std::unordered_map<ImageSubresourcePair, IMAGE_LAYOUT_NODE> &imageLayoutMap, ImageSubresourcePair imgpair,
               VkImageLayout layout) {
    auto it = imageLayoutMap.find(imgpair);
    if (it != imageLayoutMap.end()) {
        it->second.layout = layout;  // Update
    } else {
        imageLayoutMap[imgpair].layout = layout;  // Insert
    }
}

bool FindLayoutVerifyNode(layer_data const *device_data, GLOBAL_CB_NODE const *pCB, ImageSubresourcePair imgpair,
                          IMAGE_CMD_BUF_LAYOUT_NODE &node, const VkImageAspectFlags aspectMask) {
    const debug_report_data *report_data = core_validation::GetReportData(device_data);

    if (!(imgpair.subresource.aspectMask & aspectMask)) {
        return false;
    }
    VkImageAspectFlags oldAspectMask = imgpair.subresource.aspectMask;
    imgpair.subresource.aspectMask = aspectMask;
    auto imgsubIt = pCB->imageLayoutMap.find(imgpair);
    if (imgsubIt == pCB->imageLayoutMap.end()) {
        return false;
    }
    if (node.layout != VK_IMAGE_LAYOUT_MAX_ENUM && node.layout != imgsubIt->second.layout) {
        log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, HandleToUint64(imgpair.image),
                kVUID_Core_DrawState_InvalidLayout,
                "Cannot query for VkImage 0x%" PRIx64 " layout when combined aspect mask %d has multiple layout types: %s and %s",
                HandleToUint64(imgpair.image), oldAspectMask, string_VkImageLayout(node.layout),
                string_VkImageLayout(imgsubIt->second.layout));
    }
    if (node.initialLayout != VK_IMAGE_LAYOUT_MAX_ENUM && node.initialLayout != imgsubIt->second.initialLayout) {
        log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, HandleToUint64(imgpair.image),
                kVUID_Core_DrawState_InvalidLayout,
                "Cannot query for VkImage 0x%" PRIx64
                " layout when combined aspect mask %d has multiple initial layout types: %s and %s",
                HandleToUint64(imgpair.image), oldAspectMask, string_VkImageLayout(node.initialLayout),
                string_VkImageLayout(imgsubIt->second.initialLayout));
    }
    node = imgsubIt->second;
    return true;
}

bool FindLayoutVerifyLayout(layer_data const *device_data, ImageSubresourcePair imgpair, VkImageLayout &layout,
                            const VkImageAspectFlags aspectMask) {
    if (!(imgpair.subresource.aspectMask & aspectMask)) {
        return false;
    }
    const debug_report_data *report_data = core_validation::GetReportData(device_data);
    VkImageAspectFlags oldAspectMask = imgpair.subresource.aspectMask;
    imgpair.subresource.aspectMask = aspectMask;
    auto imgsubIt = (*core_validation::GetImageLayoutMap(device_data)).find(imgpair);
    if (imgsubIt == (*core_validation::GetImageLayoutMap(device_data)).end()) {
        return false;
    }
    if (layout != VK_IMAGE_LAYOUT_MAX_ENUM && layout != imgsubIt->second.layout) {
        log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, HandleToUint64(imgpair.image),
                kVUID_Core_DrawState_InvalidLayout,
                "Cannot query for VkImage 0x%" PRIx64 " layout when combined aspect mask %d has multiple layout types: %s and %s",
                HandleToUint64(imgpair.image), oldAspectMask, string_VkImageLayout(layout),
                string_VkImageLayout(imgsubIt->second.layout));
    }
    layout = imgsubIt->second.layout;
    return true;
}

// Find layout(s) on the command buffer level
bool FindCmdBufLayout(layer_data const *device_data, GLOBAL_CB_NODE const *pCB, VkImage image, VkImageSubresource range,
                      IMAGE_CMD_BUF_LAYOUT_NODE &node) {
    ImageSubresourcePair imgpair = {image, true, range};
    node = IMAGE_CMD_BUF_LAYOUT_NODE(VK_IMAGE_LAYOUT_MAX_ENUM, VK_IMAGE_LAYOUT_MAX_ENUM);
    FindLayoutVerifyNode(device_data, pCB, imgpair, node, VK_IMAGE_ASPECT_COLOR_BIT);
    FindLayoutVerifyNode(device_data, pCB, imgpair, node, VK_IMAGE_ASPECT_DEPTH_BIT);
    FindLayoutVerifyNode(device_data, pCB, imgpair, node, VK_IMAGE_ASPECT_STENCIL_BIT);
    FindLayoutVerifyNode(device_data, pCB, imgpair, node, VK_IMAGE_ASPECT_METADATA_BIT);
    if (GetDeviceExtensions(device_data)->vk_khr_sampler_ycbcr_conversion) {
        FindLayoutVerifyNode(device_data, pCB, imgpair, node, VK_IMAGE_ASPECT_PLANE_0_BIT_KHR);
        FindLayoutVerifyNode(device_data, pCB, imgpair, node, VK_IMAGE_ASPECT_PLANE_1_BIT_KHR);
        FindLayoutVerifyNode(device_data, pCB, imgpair, node, VK_IMAGE_ASPECT_PLANE_2_BIT_KHR);
    }
    if (node.layout == VK_IMAGE_LAYOUT_MAX_ENUM) {
        imgpair = {image, false, VkImageSubresource()};
        auto imgsubIt = pCB->imageLayoutMap.find(imgpair);
        if (imgsubIt == pCB->imageLayoutMap.end()) return false;
        // TODO: This is ostensibly a find function but it changes state here
        node = imgsubIt->second;
    }
    return true;
}

// Find layout(s) on the global level
bool FindGlobalLayout(layer_data *device_data, ImageSubresourcePair imgpair, VkImageLayout &layout) {
    layout = VK_IMAGE_LAYOUT_MAX_ENUM;
    FindLayoutVerifyLayout(device_data, imgpair, layout, VK_IMAGE_ASPECT_COLOR_BIT);
    FindLayoutVerifyLayout(device_data, imgpair, layout, VK_IMAGE_ASPECT_DEPTH_BIT);
    FindLayoutVerifyLayout(device_data, imgpair, layout, VK_IMAGE_ASPECT_STENCIL_BIT);
    FindLayoutVerifyLayout(device_data, imgpair, layout, VK_IMAGE_ASPECT_METADATA_BIT);
    if (GetDeviceExtensions(device_data)->vk_khr_sampler_ycbcr_conversion) {
        FindLayoutVerifyLayout(device_data, imgpair, layout, VK_IMAGE_ASPECT_PLANE_0_BIT_KHR);
        FindLayoutVerifyLayout(device_data, imgpair, layout, VK_IMAGE_ASPECT_PLANE_1_BIT_KHR);
        FindLayoutVerifyLayout(device_data, imgpair, layout, VK_IMAGE_ASPECT_PLANE_2_BIT_KHR);
    }
    if (layout == VK_IMAGE_LAYOUT_MAX_ENUM) {
        imgpair = {imgpair.image, false, VkImageSubresource()};
        auto imgsubIt = (*core_validation::GetImageLayoutMap(device_data)).find(imgpair);
        if (imgsubIt == (*core_validation::GetImageLayoutMap(device_data)).end()) return false;
        layout = imgsubIt->second.layout;
    }
    return true;
}

bool FindLayouts(layer_data *device_data, VkImage image, std::vector<VkImageLayout> &layouts) {
    auto sub_data = (*core_validation::GetImageSubresourceMap(device_data)).find(image);
    if (sub_data == (*core_validation::GetImageSubresourceMap(device_data)).end()) return false;
    auto image_state = GetImageState(device_data, image);
    if (!image_state) return false;
    bool ignoreGlobal = false;
    // TODO: Make this robust for >1 aspect mask. Now it will just say ignore potential errors in this case.
    if (sub_data->second.size() >= (image_state->createInfo.arrayLayers * image_state->createInfo.mipLevels + 1)) {
        ignoreGlobal = true;
    }
    for (auto imgsubpair : sub_data->second) {
        if (ignoreGlobal && !imgsubpair.hasSubresource) continue;
        auto img_data = (*core_validation::GetImageLayoutMap(device_data)).find(imgsubpair);
        if (img_data != (*core_validation::GetImageLayoutMap(device_data)).end()) {
            layouts.push_back(img_data->second.layout);
        }
    }
    return true;
}
bool FindLayout(const std::unordered_map<ImageSubresourcePair, IMAGE_LAYOUT_NODE> &imageLayoutMap, ImageSubresourcePair imgpair,
                VkImageLayout &layout, const VkImageAspectFlags aspectMask) {
    if (!(imgpair.subresource.aspectMask & aspectMask)) {
        return false;
    }
    imgpair.subresource.aspectMask = aspectMask;
    auto imgsubIt = imageLayoutMap.find(imgpair);
    if (imgsubIt == imageLayoutMap.end()) {
        return false;
    }
    layout = imgsubIt->second.layout;
    return true;
}

// find layout in supplied map
bool FindLayout(layer_data *device_data, const std::unordered_map<ImageSubresourcePair, IMAGE_LAYOUT_NODE> &imageLayoutMap,
                ImageSubresourcePair imgpair, VkImageLayout &layout) {
    layout = VK_IMAGE_LAYOUT_MAX_ENUM;
    FindLayout(imageLayoutMap, imgpair, layout, VK_IMAGE_ASPECT_COLOR_BIT);
    FindLayout(imageLayoutMap, imgpair, layout, VK_IMAGE_ASPECT_DEPTH_BIT);
    FindLayout(imageLayoutMap, imgpair, layout, VK_IMAGE_ASPECT_STENCIL_BIT);
    FindLayout(imageLayoutMap, imgpair, layout, VK_IMAGE_ASPECT_METADATA_BIT);
    if (GetDeviceExtensions(device_data)->vk_khr_sampler_ycbcr_conversion) {
        FindLayout(imageLayoutMap, imgpair, layout, VK_IMAGE_ASPECT_PLANE_0_BIT_KHR);
        FindLayout(imageLayoutMap, imgpair, layout, VK_IMAGE_ASPECT_PLANE_1_BIT_KHR);
        FindLayout(imageLayoutMap, imgpair, layout, VK_IMAGE_ASPECT_PLANE_2_BIT_KHR);
    }
    // Image+subresource not found, look for image handle w/o subresource
    if (layout == VK_IMAGE_LAYOUT_MAX_ENUM) {
        imgpair = {imgpair.image, false, VkImageSubresource()};
        auto imgsubIt = imageLayoutMap.find(imgpair);
        if (imgsubIt == imageLayoutMap.end()) return false;
        layout = imgsubIt->second.layout;
    }
    return true;
}

// Set the layout on the global level
void SetGlobalLayout(layer_data *device_data, ImageSubresourcePair imgpair, const VkImageLayout &layout) {
    VkImage &image = imgpair.image;
    auto &lmap = (*core_validation::GetImageLayoutMap(device_data));
    auto data = lmap.find(imgpair);
    if (data != lmap.end()) {
        data->second.layout = layout;  // Update
    } else {
        lmap[imgpair].layout = layout;  // Insert
    }
    auto &image_subresources = (*core_validation::GetImageSubresourceMap(device_data))[image];
    auto subresource = std::find(image_subresources.begin(), image_subresources.end(), imgpair);
    if (subresource == image_subresources.end()) {
        image_subresources.push_back(imgpair);
    }
}

// Set the layout on the cmdbuf level
void SetLayout(layer_data *device_data, GLOBAL_CB_NODE *pCB, ImageSubresourcePair imgpair, const IMAGE_CMD_BUF_LAYOUT_NODE &node) {
    auto it = pCB->imageLayoutMap.find(imgpair);
    if (it != pCB->imageLayoutMap.end()) {
        it->second = node;  // Update
    } else {
        pCB->imageLayoutMap[imgpair] = node;  // Insert
    }
}
// Set image layout for given VkImageSubresourceRange struct
void SetImageLayout(layer_data *device_data, GLOBAL_CB_NODE *cb_node, const IMAGE_STATE *image_state,
                    VkImageSubresourceRange image_subresource_range, const VkImageLayout &layout) {
    assert(image_state);
    cb_node->image_layout_change_count++;  // Change the version of this data to force revalidation
    for (uint32_t level_index = 0; level_index < image_subresource_range.levelCount; ++level_index) {
        uint32_t level = image_subresource_range.baseMipLevel + level_index;
        for (uint32_t layer_index = 0; layer_index < image_subresource_range.layerCount; layer_index++) {
            uint32_t layer = image_subresource_range.baseArrayLayer + layer_index;
            VkImageSubresource sub = {image_subresource_range.aspectMask, level, layer};
            // TODO: If ImageView was created with depth or stencil, transition both layouts as the aspectMask is ignored and both
            // are used. Verify that the extra implicit layout is OK for descriptor set layout validation
            if (image_subresource_range.aspectMask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) {
                if (FormatIsDepthAndStencil(image_state->createInfo.format)) {
                    sub.aspectMask |= (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
                }
            }
            // For multiplane images, IMAGE_ASPECT_COLOR is an alias for all of the plane bits
            if (GetDeviceExtensions(device_data)->vk_khr_sampler_ycbcr_conversion) {
                if (FormatIsMultiplane(image_state->createInfo.format)) {
                    if (sub.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) {
                        sub.aspectMask &= ~VK_IMAGE_ASPECT_COLOR_BIT;
                        sub.aspectMask |= VK_IMAGE_ASPECT_PLANE_0_BIT_KHR | VK_IMAGE_ASPECT_PLANE_1_BIT_KHR;
                        if (FormatPlaneCount(image_state->createInfo.format) > 2) {
                            sub.aspectMask |= VK_IMAGE_ASPECT_PLANE_2_BIT_KHR;
                        }
                    }
                }
            }
            SetLayout(device_data, cb_node, image_state->image, sub, layout);
        }
    }
}
// Set image layout for given VkImageSubresourceLayers struct
void SetImageLayout(layer_data *device_data, GLOBAL_CB_NODE *cb_node, const IMAGE_STATE *image_state,
                    VkImageSubresourceLayers image_subresource_layers, const VkImageLayout &layout) {
    // Transfer VkImageSubresourceLayers into VkImageSubresourceRange struct
    VkImageSubresourceRange image_subresource_range;
    image_subresource_range.aspectMask = image_subresource_layers.aspectMask;
    image_subresource_range.baseArrayLayer = image_subresource_layers.baseArrayLayer;
    image_subresource_range.layerCount = image_subresource_layers.layerCount;
    image_subresource_range.baseMipLevel = image_subresource_layers.mipLevel;
    image_subresource_range.levelCount = 1;
    SetImageLayout(device_data, cb_node, image_state, image_subresource_range, layout);
}

// Set image layout for all slices of an image view
void SetImageViewLayout(layer_data *device_data, GLOBAL_CB_NODE *cb_node, IMAGE_VIEW_STATE *view_state,
                        const VkImageLayout &layout) {
    assert(view_state);

    IMAGE_STATE *image_state = GetImageState(device_data, view_state->create_info.image);
    VkImageSubresourceRange sub_range = view_state->create_info.subresourceRange;

    // When changing the layout of a 3D image subresource via a 2D or 2D_ARRRAY image view, all depth slices of
    // the subresource mip level(s) are transitioned, ignoring any layers restriction in the subresource info.
    if ((image_state->createInfo.imageType == VK_IMAGE_TYPE_3D) && (view_state->create_info.viewType != VK_IMAGE_VIEW_TYPE_3D)) {
        sub_range.baseArrayLayer = 0;
        sub_range.layerCount = image_state->createInfo.extent.depth;
    }

    SetImageLayout(device_data, cb_node, image_state, sub_range, layout);
}

void SetImageViewLayout(layer_data *device_data, GLOBAL_CB_NODE *cb_node, VkImageView imageView, const VkImageLayout &layout) {
    auto view_state = GetImageViewState(device_data, imageView);
    SetImageViewLayout(device_data, cb_node, view_state, layout);
}

bool ValidateRenderPassLayoutAgainstFramebufferImageUsage(layer_data *device_data, RenderPassCreateVersion rp_version,
                                                          VkImageLayout layout, VkImage image, VkImageView image_view,
                                                          VkFramebuffer framebuffer, VkRenderPass renderpass,
                                                          uint32_t attachment_index, const char *variable_name) {
    bool skip = false;
    const auto report_data = core_validation::GetReportData(device_data);
    auto image_state = GetImageState(device_data, image);
    const char *vuid;
    const bool use_rp2 = (rp_version == RENDER_PASS_VERSION_2);

    if (!image_state) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, HandleToUint64(image),
                        "VUID-VkRenderPassBeginInfo-framebuffer-parameter",
                        "Render Pass begin with renderpass 0x%" PRIx64 " uses framebuffer 0x%" PRIx64 " where pAttachments[%" PRIu32
                        "] = image view 0x%" PRIx64 ", which refers to an invalid image",
                        HandleToUint64(renderpass), HandleToUint64(framebuffer), attachment_index, HandleToUint64(image_view));
        return skip;
    }

    auto image_usage = image_state->createInfo.usage;

    // Check for layouts that mismatch image usages in the framebuffer
    if (layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && !(image_usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
        vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2KHR-initialLayout-03094" : "VUID-vkCmdBeginRenderPass-initialLayout-00895";
        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, HandleToUint64(image), vuid,
                    "Layout/usage mismatch for attachment %u in render pass 0x%" PRIx64
                    " - the %s is %s but the image attached to framebuffer 0x%" PRIx64 " via image view 0x%" PRIx64
                    " was not created with VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT",
                    attachment_index, HandleToUint64(framebuffer), variable_name, string_VkImageLayout(layout),
                    HandleToUint64(renderpass), HandleToUint64(image_view));
    }

    if (layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
        !(image_usage & (VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT))) {
        vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2KHR-initialLayout-03097" : "VUID-vkCmdBeginRenderPass-initialLayout-00897";
        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, HandleToUint64(image), vuid,
                    "Layout/usage mismatch for attachment %u in render pass 0x%" PRIx64
                    " - the %s is %s but the image attached to framebuffer 0x%" PRIx64 " via image view 0x%" PRIx64
                    " was not created with VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT or VK_IMAGE_USAGE_SAMPLED_BIT",
                    attachment_index, HandleToUint64(framebuffer), variable_name, string_VkImageLayout(layout),
                    HandleToUint64(renderpass), HandleToUint64(image_view));
    }

    if (layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && !(image_usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)) {
        vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2KHR-initialLayout-03098" : "VUID-vkCmdBeginRenderPass-initialLayout-00898";
        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, HandleToUint64(image), vuid,
                    "Layout/usage mismatch for attachment %u in render pass 0x%" PRIx64
                    " - the %s is %s but the image attached to framebuffer 0x%" PRIx64 " via image view 0x%" PRIx64
                    " was not created with VK_IMAGE_USAGE_TRANSFER_SRC_BIT",
                    attachment_index, HandleToUint64(framebuffer), variable_name, string_VkImageLayout(layout),
                    HandleToUint64(renderpass), HandleToUint64(image_view));
    }

    if (layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && !(image_usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2KHR-initialLayout-03099" : "VUID-vkCmdBeginRenderPass-initialLayout-00899";
        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, HandleToUint64(image), vuid,
                    "Layout/usage mismatch for attachment %u in render pass 0x%" PRIx64
                    " - the %s is %s but the image attached to framebuffer 0x%" PRIx64 " via image view 0x%" PRIx64
                    " was not created with VK_IMAGE_USAGE_TRANSFER_DST_BIT",
                    attachment_index, HandleToUint64(framebuffer), variable_name, string_VkImageLayout(layout),
                    HandleToUint64(renderpass), HandleToUint64(image_view));
    }

    if (GetDeviceExtensions(device_data)->vk_khr_maintenance2) {
        if ((layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL ||
             layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL ||
             layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
             layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) &&
            !(image_usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
            vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2KHR-initialLayout-03096" : "VUID-vkCmdBeginRenderPass-initialLayout-01758";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(image), vuid,
                            "Layout/usage mismatch for attachment %u in render pass 0x%" PRIx64
                            " - the %s is %s but the image attached to framebuffer 0x%" PRIx64 " via image view 0x%" PRIx64
                            " was not created with VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT",
                            attachment_index, HandleToUint64(framebuffer), variable_name, string_VkImageLayout(layout),
                            HandleToUint64(renderpass), HandleToUint64(image_view));
        }
    } else {
        // The create render pass 2 extension requires maintenance 2 (the previous branch), so no vuid switch needed here.
        if ((layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
             layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) &&
            !(image_usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(image), "VUID-vkCmdBeginRenderPass-initialLayout-00896",
                            "Layout/usage mismatch for attachment %u in render pass 0x%" PRIx64
                            " - the %s is %s but the image attached to framebuffer 0x%" PRIx64 " via image view 0x%" PRIx64
                            " was not created with VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT",
                            attachment_index, HandleToUint64(framebuffer), variable_name, string_VkImageLayout(layout),
                            HandleToUint64(renderpass), HandleToUint64(image_view));
        }
    }
    return skip;
}

bool VerifyFramebufferAndRenderPassLayouts(layer_data *device_data, RenderPassCreateVersion rp_version, GLOBAL_CB_NODE *pCB,
                                           const VkRenderPassBeginInfo *pRenderPassBegin,
                                           const FRAMEBUFFER_STATE *framebuffer_state) {
    bool skip = false;
    auto const pRenderPassInfo = GetRenderPassState(device_data, pRenderPassBegin->renderPass)->createInfo.ptr();
    auto const &framebufferInfo = framebuffer_state->createInfo;
    const auto report_data = core_validation::GetReportData(device_data);

    auto render_pass = GetRenderPassState(device_data, pRenderPassBegin->renderPass)->renderPass;
    auto framebuffer = framebuffer_state->framebuffer;

    if (pRenderPassInfo->attachmentCount != framebufferInfo.attachmentCount) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(pCB->commandBuffer), kVUID_Core_DrawState_InvalidRenderpass,
                        "You cannot start a render pass using a framebuffer with a different number of attachments.");
    }
    for (uint32_t i = 0; i < pRenderPassInfo->attachmentCount; ++i) {
        const VkImageView &image_view = framebufferInfo.pAttachments[i];
        auto view_state = GetImageViewState(device_data, image_view);

        if (!view_state) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
                            HandleToUint64(pRenderPassBegin->renderPass), "VUID-VkRenderPassBeginInfo-framebuffer-parameter",
                            "vkCmdBeginRenderPass(): framebuffer 0x%" PRIx64 " pAttachments[%" PRIu32 "] = 0x%" PRIx64
                            " is not a valid VkImageView handle",
                            HandleToUint64(framebuffer_state->framebuffer), i, HandleToUint64(image_view));
            continue;
        }

        const VkImage &image = view_state->create_info.image;
        const VkImageSubresourceRange &subRange = view_state->create_info.subresourceRange;
        auto initial_layout = pRenderPassInfo->pAttachments[i].initialLayout;
        auto final_layout = pRenderPassInfo->pAttachments[i].finalLayout;

        // TODO: Do not iterate over every possibility - consolidate where possible
        for (uint32_t j = 0; j < subRange.levelCount; j++) {
            uint32_t level = subRange.baseMipLevel + j;
            for (uint32_t k = 0; k < subRange.layerCount; k++) {
                uint32_t layer = subRange.baseArrayLayer + k;
                VkImageSubresource sub = {subRange.aspectMask, level, layer};
                IMAGE_CMD_BUF_LAYOUT_NODE node;
                if (!FindCmdBufLayout(device_data, pCB, image, sub, node)) {
                    // Missing layouts will be added during state update
                    continue;
                }
                if (initial_layout != VK_IMAGE_LAYOUT_UNDEFINED && initial_layout != node.layout) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    kVUID_Core_DrawState_InvalidRenderpass,
                                    "You cannot start a render pass using attachment %u where the render pass initial layout is %s "
                                    "and the previous known layout of the attachment is %s. The layouts must match, or the render "
                                    "pass initial layout for the attachment must be VK_IMAGE_LAYOUT_UNDEFINED",
                                    i, string_VkImageLayout(initial_layout), string_VkImageLayout(node.layout));
                }
            }
        }

        ValidateRenderPassLayoutAgainstFramebufferImageUsage(device_data, rp_version, initial_layout, image, image_view,
                                                             framebuffer, render_pass, i, "initial layout");

        ValidateRenderPassLayoutAgainstFramebufferImageUsage(device_data, rp_version, final_layout, image, image_view, framebuffer,
                                                             render_pass, i, "final layout");
    }

    for (uint32_t j = 0; j < pRenderPassInfo->subpassCount; ++j) {
        auto &subpass = pRenderPassInfo->pSubpasses[j];
        for (uint32_t k = 0; k < pRenderPassInfo->pSubpasses[j].inputAttachmentCount; ++k) {
            auto &attachment_ref = subpass.pInputAttachments[k];
            if (attachment_ref.attachment != VK_ATTACHMENT_UNUSED) {
                auto image_view = framebufferInfo.pAttachments[attachment_ref.attachment];
                auto view_state = GetImageViewState(device_data, image_view);

                if (view_state) {
                    auto image = view_state->create_info.image;
                    ValidateRenderPassLayoutAgainstFramebufferImageUsage(device_data, rp_version, attachment_ref.layout, image,
                                                                         image_view, framebuffer, render_pass,
                                                                         attachment_ref.attachment, "input attachment layout");
                }
            }
        }

        for (uint32_t k = 0; k < pRenderPassInfo->pSubpasses[j].colorAttachmentCount; ++k) {
            auto &attachment_ref = subpass.pColorAttachments[k];
            if (attachment_ref.attachment != VK_ATTACHMENT_UNUSED) {
                auto image_view = framebufferInfo.pAttachments[attachment_ref.attachment];
                auto view_state = GetImageViewState(device_data, image_view);

                if (view_state) {
                    auto image = view_state->create_info.image;
                    ValidateRenderPassLayoutAgainstFramebufferImageUsage(device_data, rp_version, attachment_ref.layout, image,
                                                                         image_view, framebuffer, render_pass,
                                                                         attachment_ref.attachment, "color attachment layout");
                    if (subpass.pResolveAttachments) {
                        ValidateRenderPassLayoutAgainstFramebufferImageUsage(
                            device_data, rp_version, attachment_ref.layout, image, image_view, framebuffer, render_pass,
                            attachment_ref.attachment, "resolve attachment layout");
                    }
                }
            }
        }

        if (pRenderPassInfo->pSubpasses[j].pDepthStencilAttachment) {
            auto &attachment_ref = *subpass.pDepthStencilAttachment;
            if (attachment_ref.attachment != VK_ATTACHMENT_UNUSED) {
                auto image_view = framebufferInfo.pAttachments[attachment_ref.attachment];
                auto view_state = GetImageViewState(device_data, image_view);

                if (view_state) {
                    auto image = view_state->create_info.image;
                    ValidateRenderPassLayoutAgainstFramebufferImageUsage(device_data, rp_version, attachment_ref.layout, image,
                                                                         image_view, framebuffer, render_pass,
                                                                         attachment_ref.attachment, "input attachment layout");
                }
            }
        }
    }
    return skip;
}

void TransitionAttachmentRefLayout(layer_data *device_data, GLOBAL_CB_NODE *pCB, FRAMEBUFFER_STATE *pFramebuffer,
                                   const safe_VkAttachmentReference2KHR &ref) {
    if (ref.attachment != VK_ATTACHMENT_UNUSED) {
        auto image_view = GetAttachmentImageViewState(device_data, pFramebuffer, ref.attachment);
        if (image_view) {
            SetImageViewLayout(device_data, pCB, image_view, ref.layout);
        }
    }
}

void TransitionSubpassLayouts(layer_data *device_data, GLOBAL_CB_NODE *pCB, const RENDER_PASS_STATE *render_pass_state,
                              const int subpass_index, FRAMEBUFFER_STATE *framebuffer_state) {
    assert(render_pass_state);

    if (framebuffer_state) {
        auto const &subpass = render_pass_state->createInfo.pSubpasses[subpass_index];
        for (uint32_t j = 0; j < subpass.inputAttachmentCount; ++j) {
            TransitionAttachmentRefLayout(device_data, pCB, framebuffer_state, subpass.pInputAttachments[j]);
        }
        for (uint32_t j = 0; j < subpass.colorAttachmentCount; ++j) {
            TransitionAttachmentRefLayout(device_data, pCB, framebuffer_state, subpass.pColorAttachments[j]);
        }
        if (subpass.pDepthStencilAttachment) {
            TransitionAttachmentRefLayout(device_data, pCB, framebuffer_state, *subpass.pDepthStencilAttachment);
        }
    }
}

bool ValidateImageAspectLayout(layer_data *device_data, GLOBAL_CB_NODE const *pCB, const VkImageMemoryBarrier *mem_barrier,
                               uint32_t level, uint32_t layer, VkImageAspectFlags aspect) {
    if (!(mem_barrier->subresourceRange.aspectMask & aspect)) {
        return false;
    }
    VkImageSubresource sub = {aspect, level, layer};
    IMAGE_CMD_BUF_LAYOUT_NODE node;
    if (!FindCmdBufLayout(device_data, pCB, mem_barrier->image, sub, node)) {
        return false;
    }
    bool skip = false;
    if (mem_barrier->oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
        // TODO: Set memory invalid which is in mem_tracker currently
    } else if (node.layout != mem_barrier->oldLayout) {
        skip = log_msg(core_validation::GetReportData(device_data), VK_DEBUG_REPORT_ERROR_BIT_EXT,
                       VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, HandleToUint64(pCB->commandBuffer),
                       "VUID-VkImageMemoryBarrier-oldLayout-01197",
                       "For image 0x%" PRIx64
                       " you cannot transition the layout of aspect=%d level=%d layer=%d from %s when current layout is %s.",
                       HandleToUint64(mem_barrier->image), aspect, level, layer, string_VkImageLayout(mem_barrier->oldLayout),
                       string_VkImageLayout(node.layout));
    }
    return skip;
}

// Transition the layout state for renderpass attachments based on the BeginRenderPass() call. This includes:
// 1. Transition into initialLayout state
// 2. Transition from initialLayout to layout used in subpass 0
void TransitionBeginRenderPassLayouts(layer_data *device_data, GLOBAL_CB_NODE *cb_state, const RENDER_PASS_STATE *render_pass_state,
                                      FRAMEBUFFER_STATE *framebuffer_state) {
    // First transition into initialLayout
    auto const rpci = render_pass_state->createInfo.ptr();
    for (uint32_t i = 0; i < rpci->attachmentCount; ++i) {
        auto view_state = GetAttachmentImageViewState(device_data, framebuffer_state, i);
        if (view_state) {
            SetImageViewLayout(device_data, cb_state, view_state, rpci->pAttachments[i].initialLayout);
        }
    }
    // Now transition for first subpass (index 0)
    TransitionSubpassLayouts(device_data, cb_state, render_pass_state, 0, framebuffer_state);
}

void TransitionImageAspectLayout(layer_data *device_data, GLOBAL_CB_NODE *pCB, const VkImageMemoryBarrier *mem_barrier,
                                 uint32_t level, uint32_t layer, VkImageAspectFlags aspect_mask, VkImageAspectFlags aspect) {
    if (!(aspect_mask & aspect)) {
        return;
    }
    VkImageSubresource sub = {aspect, level, layer};
    IMAGE_CMD_BUF_LAYOUT_NODE node;
    if (!FindCmdBufLayout(device_data, pCB, mem_barrier->image, sub, node)) {
        pCB->image_layout_change_count++;  // Change the version of this data to force revalidation
        SetLayout(device_data, pCB, mem_barrier->image, sub,
                  IMAGE_CMD_BUF_LAYOUT_NODE(mem_barrier->oldLayout, mem_barrier->newLayout));
        return;
    }
    if (mem_barrier->oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
        // TODO: Set memory invalid
    }
    SetLayout(device_data, pCB, mem_barrier->image, sub, mem_barrier->newLayout);
}

bool VerifyAspectsPresent(VkImageAspectFlags aspect_mask, VkFormat format) {
    if ((aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) != 0) {
        if (!(FormatIsColor(format) || FormatIsMultiplane(format))) return false;
    }
    if ((aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) != 0) {
        if (!FormatHasDepth(format)) return false;
    }
    if ((aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) != 0) {
        if (!FormatHasStencil(format)) return false;
    }
    if (0 !=
        (aspect_mask & (VK_IMAGE_ASPECT_PLANE_0_BIT_KHR | VK_IMAGE_ASPECT_PLANE_1_BIT_KHR | VK_IMAGE_ASPECT_PLANE_2_BIT_KHR))) {
        if (FormatPlaneCount(format) == 1) return false;
    }
    return true;
}

// Verify an ImageMemoryBarrier's old/new ImageLayouts are compatible with the Image's ImageUsageFlags.
bool ValidateBarrierLayoutToImageUsage(layer_data *device_data, const VkImageMemoryBarrier *img_barrier, bool new_not_old,
                                       VkImageUsageFlags usage_flags, const char *func_name) {
    const auto report_data = core_validation::GetReportData(device_data);
    bool skip = false;
    const VkImageLayout layout = (new_not_old) ? img_barrier->newLayout : img_barrier->oldLayout;
    std::string msg_code = kVUIDUndefined;  // sentinel value meaning "no error"

    switch (layout) {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            if ((usage_flags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == 0) {
                msg_code = "VUID-VkImageMemoryBarrier-oldLayout-01208";
            }
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            if ((usage_flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0) {
                msg_code = "VUID-VkImageMemoryBarrier-oldLayout-01209";
            }
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
            if ((usage_flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0) {
                msg_code = "VUID-VkImageMemoryBarrier-oldLayout-01210";
            }
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            if ((usage_flags & (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) == 0) {
                msg_code = "VUID-VkImageMemoryBarrier-oldLayout-01211";
            }
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            if ((usage_flags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) == 0) {
                msg_code = "VUID-VkImageMemoryBarrier-oldLayout-01212";
            }
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            if ((usage_flags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0) {
                msg_code = "VUID-VkImageMemoryBarrier-oldLayout-01213";
            }
            break;
        case VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV:
            if ((usage_flags & VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV) == 0) {
                msg_code = "VUID-VkImageMemoryBarrier-oldLayout-02088";
            }
            break;
        default:
            // Other VkImageLayout values do not have VUs defined in this context.
            break;
    }

    if (msg_code != kVUIDUndefined) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                        HandleToUint64(img_barrier->image), msg_code,
                        "%s: Image barrier 0x%p %sLayout=%s is not compatible with image 0x%" PRIx64 " usage flags 0x%" PRIx32 ".",
                        func_name, static_cast<const void *>(img_barrier), ((new_not_old) ? "new" : "old"),
                        string_VkImageLayout(layout), HandleToUint64(img_barrier->image), usage_flags);
    }
    return skip;
}

// Scoreboard for checking for duplicate and inconsistent barriers to images
struct ImageBarrierScoreboardEntry {
    uint32_t index;
    // This is designed for temporary storage within the scope of the API call.  If retained storage of the barriers is
    // required, copies should be made and smart or unique pointers used in some other stucture (or this one refactored)
    const VkImageMemoryBarrier *barrier;
};
using ImageBarrierScoreboardSubresMap = std::unordered_map<VkImageSubresourceRange, ImageBarrierScoreboardEntry>;
using ImageBarrierScoreboardImageMap = std::unordered_map<VkImage, ImageBarrierScoreboardSubresMap>;

// Verify image barriers are compatible with the images they reference.
bool ValidateBarriersToImages(layer_data *device_data, GLOBAL_CB_NODE const *cb_state, uint32_t imageMemoryBarrierCount,
                              const VkImageMemoryBarrier *pImageMemoryBarriers, const char *func_name) {
    bool skip = false;

    // Scoreboard for duplicate layout transition barriers within the list
    // Pointers retained in the scoreboard only have the lifetime of *this* call (i.e. within the scope of the API call)
    ImageBarrierScoreboardImageMap layout_transitions;

    for (uint32_t i = 0; i < imageMemoryBarrierCount; ++i) {
        auto img_barrier = &pImageMemoryBarriers[i];
        if (!img_barrier) continue;

        // Update the scoreboard of layout transitions and check for barriers affecting the same image and subresource
        // TODO: a higher precision could be gained by adapting the command_buffer image_layout_map logic looking for conflicts
        // at a per sub-resource level
        if (img_barrier->oldLayout != img_barrier->newLayout) {
            ImageBarrierScoreboardEntry new_entry{i, img_barrier};
            auto image_it = layout_transitions.find(img_barrier->image);
            if (image_it != layout_transitions.end()) {
                auto &subres_map = image_it->second;
                auto subres_it = subres_map.find(img_barrier->subresourceRange);
                if (subres_it != subres_map.end()) {
                    auto &entry = subres_it->second;
                    if ((entry.barrier->newLayout != img_barrier->oldLayout) &&
                        (img_barrier->oldLayout != VK_IMAGE_LAYOUT_UNDEFINED)) {
                        const VkImageSubresourceRange &range = img_barrier->subresourceRange;
                        skip = log_msg(
                            core_validation::GetReportData(device_data), VK_DEBUG_REPORT_ERROR_BIT_EXT,
                            VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, HandleToUint64(cb_state->commandBuffer),
                            "VUID-VkImageMemoryBarrier-oldLayout-01197",
                            "%s: pImageMemoryBarrier[%u] conflicts with earlier entry pImageMemoryBarrier[%u]. Image 0x%" PRIx64
                            " subresourceRange: aspectMask=%u baseMipLevel=%u levelCount=%u, baseArrayLayer=%u, layerCount=%u; "
                            "conflicting barrier transitions image layout from %s when earlier barrier transitioned to layout %s.",
                            func_name, i, entry.index, HandleToUint64(img_barrier->image), range.aspectMask, range.baseMipLevel,
                            range.levelCount, range.baseArrayLayer, range.layerCount, string_VkImageLayout(img_barrier->oldLayout),
                            string_VkImageLayout(entry.barrier->newLayout));
                    }
                    entry = new_entry;
                } else {
                    subres_map[img_barrier->subresourceRange] = new_entry;
                }
            } else {
                layout_transitions[img_barrier->image][img_barrier->subresourceRange] = new_entry;
            }
        }

        auto image_state = GetImageState(device_data, img_barrier->image);
        if (image_state) {
            VkImageUsageFlags usage_flags = image_state->createInfo.usage;
            skip |= ValidateBarrierLayoutToImageUsage(device_data, img_barrier, false, usage_flags, func_name);
            skip |= ValidateBarrierLayoutToImageUsage(device_data, img_barrier, true, usage_flags, func_name);

            // Make sure layout is able to be transitioned, currently only presented shared presentable images are locked
            if (image_state->layout_locked) {
                // TODO: Add unique id for error when available
                skip |= log_msg(
                    core_validation::GetReportData(device_data), VK_DEBUG_REPORT_ERROR_BIT_EXT,
                    VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, HandleToUint64(img_barrier->image), 0,
                    "Attempting to transition shared presentable image 0x%" PRIx64
                    " from layout %s to layout %s, but image has already been presented and cannot have its layout transitioned.",
                    HandleToUint64(img_barrier->image), string_VkImageLayout(img_barrier->oldLayout),
                    string_VkImageLayout(img_barrier->newLayout));
            }
        }

        VkImageCreateInfo *image_create_info = &(GetImageState(device_data, img_barrier->image)->createInfo);
        // For a Depth/Stencil image both aspects MUST be set
        if (FormatIsDepthAndStencil(image_create_info->format)) {
            auto const aspect_mask = img_barrier->subresourceRange.aspectMask;
            auto const ds_mask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            if ((aspect_mask & ds_mask) != (ds_mask)) {
                skip |=
                    log_msg(core_validation::GetReportData(device_data), VK_DEBUG_REPORT_ERROR_BIT_EXT,
                            VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, HandleToUint64(img_barrier->image),
                            "VUID-VkImageMemoryBarrier-image-01207",
                            "%s: Image barrier 0x%p references image 0x%" PRIx64
                            " of format %s that must have the depth and stencil aspects set, but its aspectMask is 0x%" PRIx32 ".",
                            func_name, static_cast<const void *>(img_barrier), HandleToUint64(img_barrier->image),
                            string_VkFormat(image_create_info->format), aspect_mask);
            }
        }
        uint32_t level_count = ResolveRemainingLevels(&img_barrier->subresourceRange, image_create_info->mipLevels);
        uint32_t layer_count = ResolveRemainingLayers(&img_barrier->subresourceRange, image_create_info->arrayLayers);

        for (uint32_t j = 0; j < level_count; j++) {
            uint32_t level = img_barrier->subresourceRange.baseMipLevel + j;
            for (uint32_t k = 0; k < layer_count; k++) {
                uint32_t layer = img_barrier->subresourceRange.baseArrayLayer + k;
                skip |= ValidateImageAspectLayout(device_data, cb_state, img_barrier, level, layer, VK_IMAGE_ASPECT_COLOR_BIT);
                skip |= ValidateImageAspectLayout(device_data, cb_state, img_barrier, level, layer, VK_IMAGE_ASPECT_DEPTH_BIT);
                skip |= ValidateImageAspectLayout(device_data, cb_state, img_barrier, level, layer, VK_IMAGE_ASPECT_STENCIL_BIT);
                skip |= ValidateImageAspectLayout(device_data, cb_state, img_barrier, level, layer, VK_IMAGE_ASPECT_METADATA_BIT);
                if (GetDeviceExtensions(device_data)->vk_khr_sampler_ycbcr_conversion) {
                    skip |= ValidateImageAspectLayout(device_data, cb_state, img_barrier, level, layer,
                                                      VK_IMAGE_ASPECT_PLANE_0_BIT_KHR);
                    skip |= ValidateImageAspectLayout(device_data, cb_state, img_barrier, level, layer,
                                                      VK_IMAGE_ASPECT_PLANE_1_BIT_KHR);
                    skip |= ValidateImageAspectLayout(device_data, cb_state, img_barrier, level, layer,
                                                      VK_IMAGE_ASPECT_PLANE_2_BIT_KHR);
                }
            }
        }
    }
    return skip;
}

static bool IsReleaseOp(layer_data *device_data, GLOBAL_CB_NODE *cb_state, VkImageMemoryBarrier const *barrier) {
    if (!IsTransferOp(barrier)) return false;

    auto pool = GetCommandPoolNode(device_data, cb_state->createInfo.commandPool);
    return pool && IsReleaseOp<VkImageMemoryBarrier, true>(pool, barrier);
}

template <typename Barrier>
bool ValidateQFOTransferBarrierUniqueness(layer_data *device_data, const char *func_name, GLOBAL_CB_NODE *cb_state,
                                          uint32_t barrier_count, const Barrier *barriers) {
    using BarrierRecord = QFOTransferBarrier<Barrier>;
    bool skip = false;
    const auto report_data = core_validation::GetReportData(device_data);
    auto pool = GetCommandPoolNode(device_data, cb_state->createInfo.commandPool);
    auto &barrier_sets = GetQFOBarrierSets(cb_state, typename BarrierRecord::Tag());
    const char *barrier_name = BarrierRecord::BarrierName();
    const char *handle_name = BarrierRecord::HandleName();
    const char *transfer_type = nullptr;
    for (uint32_t b = 0; b < barrier_count; b++) {
        if (!IsTransferOp(&barriers[b])) continue;
        const BarrierRecord *barrier_record = nullptr;
        if (IsReleaseOp<Barrier, true /* Assume IsTransfer */>(pool, &barriers[b]) && !IsSpecial(barriers[b].dstQueueFamilyIndex)) {
            const auto found = barrier_sets.release.find(barriers[b]);
            if (found != barrier_sets.release.cend()) {
                barrier_record = &(*found);
                transfer_type = "releasing";
            }
        } else if (IsAcquireOp<Barrier, true /*Assume IsTransfer */>(pool, &barriers[b]) &&
                   !IsSpecial(barriers[b].srcQueueFamilyIndex)) {
            const auto found = barrier_sets.acquire.find(barriers[b]);
            if (found != barrier_sets.acquire.cend()) {
                barrier_record = &(*found);
                transfer_type = "acquiring";
            }
        }
        if (barrier_record != nullptr) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(cb_state->commandBuffer), BarrierRecord::ErrMsgDuplicateQFOInCB(),
                        "%s: %s at index %" PRIu32 " %s queue ownership of %s (0x%" PRIx64 "), from srcQueueFamilyIndex %" PRIu32
                        " to dstQueueFamilyIndex %" PRIu32 " duplicates existing barrier recorded in this command buffer.",
                        func_name, barrier_name, b, transfer_type, handle_name, HandleToUint64(barrier_record->handle),
                        barrier_record->srcQueueFamilyIndex, barrier_record->dstQueueFamilyIndex);
        }
    }
    return skip;
}

template <typename Barrier>
void RecordQFOTransferBarriers(layer_data *device_data, GLOBAL_CB_NODE *cb_state, uint32_t barrier_count, const Barrier *barriers) {
    auto pool = GetCommandPoolNode(device_data, cb_state->createInfo.commandPool);
    auto &barrier_sets = GetQFOBarrierSets(cb_state, typename QFOTransferBarrier<Barrier>::Tag());
    for (uint32_t b = 0; b < barrier_count; b++) {
        if (!IsTransferOp(&barriers[b])) continue;
        if (IsReleaseOp<Barrier, true /* Assume IsTransfer*/>(pool, &barriers[b]) && !IsSpecial(barriers[b].dstQueueFamilyIndex)) {
            barrier_sets.release.emplace(barriers[b]);
        } else if (IsAcquireOp<Barrier, true /*Assume IsTransfer */>(pool, &barriers[b]) &&
                   !IsSpecial(barriers[b].srcQueueFamilyIndex)) {
            barrier_sets.acquire.emplace(barriers[b]);
        }
    }
}

bool ValidateBarriersQFOTransferUniqueness(layer_data *device_data, const char *func_name, GLOBAL_CB_NODE *cb_state,
                                           uint32_t bufferBarrierCount, const VkBufferMemoryBarrier *pBufferMemBarriers,
                                           uint32_t imageMemBarrierCount, const VkImageMemoryBarrier *pImageMemBarriers) {
    bool skip = false;
    skip |= ValidateQFOTransferBarrierUniqueness(device_data, func_name, cb_state, bufferBarrierCount, pBufferMemBarriers);
    skip |= ValidateQFOTransferBarrierUniqueness(device_data, func_name, cb_state, imageMemBarrierCount, pImageMemBarriers);
    return skip;
}

void RecordBarriersQFOTransfers(layer_data *device_data, GLOBAL_CB_NODE *cb_state, uint32_t bufferBarrierCount,
                                const VkBufferMemoryBarrier *pBufferMemBarriers, uint32_t imageMemBarrierCount,
                                const VkImageMemoryBarrier *pImageMemBarriers) {
    RecordQFOTransferBarriers(device_data, cb_state, bufferBarrierCount, pBufferMemBarriers);
    RecordQFOTransferBarriers(device_data, cb_state, imageMemBarrierCount, pImageMemBarriers);
}

template <typename BarrierRecord, typename Scoreboard>
static bool ValidateAndUpdateQFOScoreboard(const debug_report_data *report_data, const GLOBAL_CB_NODE *cb_state,
                                           const char *operation, const BarrierRecord &barrier, Scoreboard *scoreboard) {
    // Record to the scoreboard or report that we have a duplication
    bool skip = false;
    auto inserted = scoreboard->insert(std::make_pair(barrier, cb_state));
    if (!inserted.second && inserted.first->second != cb_state) {
        // This is a duplication (but don't report duplicates from the same CB, as we do that at record time
        skip = log_msg(
            report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
            HandleToUint64(cb_state->commandBuffer), BarrierRecord::ErrMsgDuplicateQFOInSubmit(),
            "%s: %s %s queue ownership of %s (0x%" PRIx64 "), from srcQueueFamilyIndex %" PRIu32 " to dstQueueFamilyIndex %" PRIu32
            " duplicates existing barrier submitted in this batch from command buffer 0x%" PRIx64 ".",
            "vkQueueSubmit()", BarrierRecord::BarrierName(), operation, BarrierRecord::HandleName(), HandleToUint64(barrier.handle),
            barrier.srcQueueFamilyIndex, barrier.dstQueueFamilyIndex, HandleToUint64(inserted.first->second));
    }
    return skip;
}

template <typename Barrier>
static bool ValidateQueuedQFOTransferBarriers(layer_data *device_data, GLOBAL_CB_NODE *cb_state,
                                              QFOTransferCBScoreboards<Barrier> *scoreboards) {
    using BarrierRecord = QFOTransferBarrier<Barrier>;
    using TypeTag = typename BarrierRecord::Tag;
    bool skip = false;
    const auto report_data = core_validation::GetReportData(device_data);
    const auto &cb_barriers = GetQFOBarrierSets(cb_state, TypeTag());
    const GlobalQFOTransferBarrierMap<Barrier> &global_release_barriers =
        core_validation::GetGlobalQFOReleaseBarrierMap(device_data, TypeTag());
    const char *barrier_name = BarrierRecord::BarrierName();
    const char *handle_name = BarrierRecord::HandleName();
    // No release should have an extant duplicate (WARNING)
    for (const auto &release : cb_barriers.release) {
        // Check the global pending release barriers
        const auto set_it = global_release_barriers.find(release.handle);
        if (set_it != global_release_barriers.cend()) {
            const QFOTransferBarrierSet<Barrier> &set_for_handle = set_it->second;
            const auto found = set_for_handle.find(release);
            if (found != set_for_handle.cend()) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(cb_state->commandBuffer), BarrierRecord::ErrMsgDuplicateQFOSubmitted(),
                                "%s: %s releasing queue ownership of %s (0x%" PRIx64 "), from srcQueueFamilyIndex %" PRIu32
                                " to dstQueueFamilyIndex %" PRIu32
                                " duplicates existing barrier queued for execution, without intervening acquire operation.",
                                "vkQueueSubmit()", barrier_name, handle_name, HandleToUint64(found->handle),
                                found->srcQueueFamilyIndex, found->dstQueueFamilyIndex);
            }
        }
        skip |= ValidateAndUpdateQFOScoreboard(report_data, cb_state, "releasing", release, &scoreboards->release);
    }
    // Each acquire must have a matching release (ERROR)
    for (const auto &acquire : cb_barriers.acquire) {
        const auto set_it = global_release_barriers.find(acquire.handle);
        bool matching_release_found = false;
        if (set_it != global_release_barriers.cend()) {
            const QFOTransferBarrierSet<Barrier> &set_for_handle = set_it->second;
            matching_release_found = set_for_handle.find(acquire) != set_for_handle.cend();
        }
        if (!matching_release_found) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_state->commandBuffer), BarrierRecord::ErrMsgMissingQFOReleaseInSubmit(),
                            "%s: in submitted command buffer %s acquiring ownership of %s (0x%" PRIx64
                            "), from srcQueueFamilyIndex %" PRIu32 " to dstQueueFamilyIndex %" PRIu32
                            " has no matching release barrier queued for execution.",
                            "vkQueueSubmit()", barrier_name, handle_name, HandleToUint64(acquire.handle),
                            acquire.srcQueueFamilyIndex, acquire.dstQueueFamilyIndex);
        }
        skip |= ValidateAndUpdateQFOScoreboard(report_data, cb_state, "acquiring", acquire, &scoreboards->acquire);
    }
    return skip;
}

bool ValidateQueuedQFOTransfers(layer_data *device_data, GLOBAL_CB_NODE *cb_state,
                                QFOTransferCBScoreboards<VkImageMemoryBarrier> *qfo_image_scoreboards,
                                QFOTransferCBScoreboards<VkBufferMemoryBarrier> *qfo_buffer_scoreboards) {
    bool skip = false;
    skip |= ValidateQueuedQFOTransferBarriers<VkImageMemoryBarrier>(device_data, cb_state, qfo_image_scoreboards);
    skip |= ValidateQueuedQFOTransferBarriers<VkBufferMemoryBarrier>(device_data, cb_state, qfo_buffer_scoreboards);
    return skip;
}

template <typename Barrier>
static void RecordQueuedQFOTransferBarriers(layer_data *device_data, GLOBAL_CB_NODE *cb_state) {
    using BarrierRecord = QFOTransferBarrier<Barrier>;
    using TypeTag = typename BarrierRecord::Tag;
    const auto &cb_barriers = GetQFOBarrierSets(cb_state, TypeTag());
    GlobalQFOTransferBarrierMap<Barrier> &global_release_barriers =
        core_validation::GetGlobalQFOReleaseBarrierMap(device_data, TypeTag());

    // Add release barriers from this submit to the global map
    for (const auto &release : cb_barriers.release) {
        // the global barrier list is mapped by resource handle to allow cleanup on resource destruction
        // NOTE: We're using [] because creation of a Set is a needed side effect for new handles
        global_release_barriers[release.handle].insert(release);
    }

    // Erase acquired barriers from this submit from the global map -- essentially marking releases as consumed
    for (const auto &acquire : cb_barriers.acquire) {
        // NOTE: We're not using [] because we don't want to create entries for missing releases
        auto set_it = global_release_barriers.find(acquire.handle);
        if (set_it != global_release_barriers.end()) {
            QFOTransferBarrierSet<Barrier> &set_for_handle = set_it->second;
            set_for_handle.erase(acquire);
            if (set_for_handle.size() == 0) {  // Clean up empty sets
                global_release_barriers.erase(set_it);
            }
        }
    }
}

void RecordQueuedQFOTransfers(layer_data *device_data, GLOBAL_CB_NODE *cb_state) {
    RecordQueuedQFOTransferBarriers<VkImageMemoryBarrier>(device_data, cb_state);
    RecordQueuedQFOTransferBarriers<VkBufferMemoryBarrier>(device_data, cb_state);
}

// Remove the pending QFO release records from the global set
// Note that the type of the handle argument constrained to match Barrier type
// The defaulted BarrierRecord argument allows use to declare the type once, but is not intended to be specified by the caller
template <typename Barrier, typename BarrierRecord = QFOTransferBarrier<Barrier>>
static void EraseQFOReleaseBarriers(layer_data *device_data, const typename BarrierRecord::HandleType &handle) {
    GlobalQFOTransferBarrierMap<Barrier> &global_release_barriers =
        core_validation::GetGlobalQFOReleaseBarrierMap(device_data, typename BarrierRecord::Tag());
    global_release_barriers.erase(handle);
}

// Avoid making the template globally visible by exporting the one instance of it we need.
void EraseQFOImageRelaseBarriers(layer_data *device_data, const VkImage &image) {
    EraseQFOReleaseBarriers<VkImageMemoryBarrier>(device_data, image);
}

void TransitionImageLayouts(layer_data *device_data, GLOBAL_CB_NODE *cb_state, uint32_t memBarrierCount,
                            const VkImageMemoryBarrier *pImgMemBarriers) {
    for (uint32_t i = 0; i < memBarrierCount; ++i) {
        auto mem_barrier = &pImgMemBarriers[i];
        if (!mem_barrier) continue;

        // For ownership transfers, the barrier is specified twice; as a release
        // operation on the yielding queue family, and as an acquire operation
        // on the acquiring queue family. This barrier may also include a layout
        // transition, which occurs 'between' the two operations. For validation
        // purposes it doesn't seem important which side performs the layout
        // transition, but it must not be performed twice. We'll arbitrarily
        // choose to perform it as part of the acquire operation.
        if (IsReleaseOp(device_data, cb_state, mem_barrier)) {
            continue;
        }

        VkImageCreateInfo *image_create_info = &(GetImageState(device_data, mem_barrier->image)->createInfo);
        uint32_t level_count = ResolveRemainingLevels(&mem_barrier->subresourceRange, image_create_info->mipLevels);
        uint32_t layer_count = ResolveRemainingLayers(&mem_barrier->subresourceRange, image_create_info->arrayLayers);

        // Special case for 3D images with VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR flag bit, where <extent.depth> and
        // <arrayLayers> can potentially alias. When recording layout for the entire image, pre-emptively record layouts
        // for all (potential) layer sub_resources.
        if ((0 != (image_create_info->flags & VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR)) &&
            (mem_barrier->subresourceRange.baseArrayLayer == 0) && (layer_count == 1)) {
            layer_count = image_create_info->extent.depth;  // Treat each depth slice as a layer subresource
        }

        // For multiplanar formats, IMAGE_ASPECT_COLOR is equivalent to adding the aspect of the individual planes
        VkImageAspectFlags aspect_mask = mem_barrier->subresourceRange.aspectMask;
        if (GetDeviceExtensions(device_data)->vk_khr_sampler_ycbcr_conversion) {
            if (FormatIsMultiplane(image_create_info->format)) {
                if (aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) {
                    aspect_mask &= ~VK_IMAGE_ASPECT_COLOR_BIT;
                    aspect_mask |= (VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT);
                    if (FormatPlaneCount(image_create_info->format) > 2) {
                        aspect_mask |= VK_IMAGE_ASPECT_PLANE_2_BIT;
                    }
                }
            }
        }

        for (uint32_t j = 0; j < level_count; j++) {
            uint32_t level = mem_barrier->subresourceRange.baseMipLevel + j;
            for (uint32_t k = 0; k < layer_count; k++) {
                uint32_t layer = mem_barrier->subresourceRange.baseArrayLayer + k;
                TransitionImageAspectLayout(device_data, cb_state, mem_barrier, level, layer, aspect_mask,
                                            VK_IMAGE_ASPECT_COLOR_BIT);
                TransitionImageAspectLayout(device_data, cb_state, mem_barrier, level, layer, aspect_mask,
                                            VK_IMAGE_ASPECT_DEPTH_BIT);
                TransitionImageAspectLayout(device_data, cb_state, mem_barrier, level, layer, aspect_mask,
                                            VK_IMAGE_ASPECT_STENCIL_BIT);
                TransitionImageAspectLayout(device_data, cb_state, mem_barrier, level, layer, aspect_mask,
                                            VK_IMAGE_ASPECT_METADATA_BIT);
                if (GetDeviceExtensions(device_data)->vk_khr_sampler_ycbcr_conversion) {
                    TransitionImageAspectLayout(device_data, cb_state, mem_barrier, level, layer, aspect_mask,
                                                VK_IMAGE_ASPECT_PLANE_0_BIT_KHR);
                    TransitionImageAspectLayout(device_data, cb_state, mem_barrier, level, layer, aspect_mask,
                                                VK_IMAGE_ASPECT_PLANE_1_BIT_KHR);
                    TransitionImageAspectLayout(device_data, cb_state, mem_barrier, level, layer, aspect_mask,
                                                VK_IMAGE_ASPECT_PLANE_2_BIT_KHR);
                }
            }
        }
    }
}

bool VerifyImageLayout(layer_data const *device_data, GLOBAL_CB_NODE const *cb_node, IMAGE_STATE *image_state,
                       VkImageSubresourceLayers subLayers, VkImageLayout explicit_layout, VkImageLayout optimal_layout,
                       const char *caller, const std::string &layout_invalid_msg_code, const std::string &layout_mismatch_msg_code,
                       bool *error) {
    const auto report_data = core_validation::GetReportData(device_data);
    const auto image = image_state->image;
    bool skip = false;

    for (uint32_t i = 0; i < subLayers.layerCount; ++i) {
        uint32_t layer = i + subLayers.baseArrayLayer;
        VkImageSubresource sub = {subLayers.aspectMask, subLayers.mipLevel, layer};
        IMAGE_CMD_BUF_LAYOUT_NODE node;
        if (FindCmdBufLayout(device_data, cb_node, image, sub, node)) {
            if (node.layout != explicit_layout) {
                *error = true;
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(cb_node->commandBuffer), layout_mismatch_msg_code,
                                "%s: Cannot use image 0x%" PRIx64
                                " (layer=%u mip=%u) with specific layout %s that doesn't match the actual current layout %s.",
                                caller, HandleToUint64(image), layer, subLayers.mipLevel, string_VkImageLayout(explicit_layout),
                                string_VkImageLayout(node.layout));
            }
        }
    }
    // If optimal_layout is not UNDEFINED, check that layout matches optimal for this case
    if ((VK_IMAGE_LAYOUT_UNDEFINED != optimal_layout) && (explicit_layout != optimal_layout)) {
        if (VK_IMAGE_LAYOUT_GENERAL == explicit_layout) {
            if (image_state->createInfo.tiling != VK_IMAGE_TILING_LINEAR) {
                // LAYOUT_GENERAL is allowed, but may not be performance optimal, flag as perf warning.
                skip |= log_msg(report_data, VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, HandleToUint64(cb_node->commandBuffer),
                                kVUID_Core_DrawState_InvalidImageLayout,
                                "%s: For optimal performance image 0x%" PRIx64 " layout should be %s instead of GENERAL.", caller,
                                HandleToUint64(image), string_VkImageLayout(optimal_layout));
            }
        } else if (GetDeviceExtensions(device_data)->vk_khr_shared_presentable_image) {
            if (image_state->shared_presentable) {
                if (VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR != explicit_layout) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    layout_invalid_msg_code,
                                    "Layout for shared presentable image is %s but must be VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR.",
                                    string_VkImageLayout(optimal_layout));
                }
            }
        } else {
            *error = true;
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_node->commandBuffer), layout_invalid_msg_code,
                            "%s: Layout for image 0x%" PRIx64 " is %s but can only be %s or VK_IMAGE_LAYOUT_GENERAL.", caller,
                            HandleToUint64(image), string_VkImageLayout(explicit_layout), string_VkImageLayout(optimal_layout));
        }
    }
    return skip;
}

void TransitionFinalSubpassLayouts(layer_data *device_data, GLOBAL_CB_NODE *pCB, const VkRenderPassBeginInfo *pRenderPassBegin,
                                   FRAMEBUFFER_STATE *framebuffer_state) {
    auto renderPass = GetRenderPassState(device_data, pRenderPassBegin->renderPass);
    if (!renderPass) return;

    const VkRenderPassCreateInfo2KHR *pRenderPassInfo = renderPass->createInfo.ptr();
    if (framebuffer_state) {
        for (uint32_t i = 0; i < pRenderPassInfo->attachmentCount; ++i) {
            auto view_state = GetAttachmentImageViewState(device_data, framebuffer_state, i);
            if (view_state) {
                SetImageViewLayout(device_data, pCB, view_state, pRenderPassInfo->pAttachments[i].finalLayout);
            }
        }
    }
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR
// Android-specific validation that uses types defined only with VK_USE_PLATFORM_ANDROID_KHR
// This could also move into a seperate core_validation_android.cpp file... ?

//
// AHB-specific validation within non-AHB APIs
//
bool ValidateCreateImageANDROID(layer_data *device_data, const debug_report_data *report_data,
                                const VkImageCreateInfo *create_info) {
    bool skip = false;

    const VkExternalFormatANDROID *ext_fmt_android = lvl_find_in_chain<VkExternalFormatANDROID>(create_info->pNext);
    if (ext_fmt_android) {
        if (0 != ext_fmt_android->externalFormat) {
            if (VK_FORMAT_UNDEFINED != create_info->format) {
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkImageCreateInfo-pNext-01974",
                            "vkCreateImage(): VkImageCreateInfo struct has a chained VkExternalFormatANDROID struct with non-zero "
                            "externalFormat, but the VkImageCreateInfo's format is not VK_FORMAT_UNDEFINED.");
            }

            if (0 != (VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT & create_info->flags)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkImageCreateInfo-pNext-02396",
                                "vkCreateImage(): VkImageCreateInfo struct has a chained VkExternalFormatANDROID struct with "
                                "non-zero externalFormat, but flags include VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT.");
            }

            if (0 != (~VK_IMAGE_USAGE_SAMPLED_BIT & create_info->usage)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkImageCreateInfo-pNext-02397",
                                "vkCreateImage(): VkImageCreateInfo struct has a chained VkExternalFormatANDROID struct with "
                                "non-zero externalFormat, but usage includes bits other than VK_IMAGE_USAGE_SAMPLED_BIT.");
            }

            if (VK_IMAGE_TILING_OPTIMAL != create_info->tiling) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkImageCreateInfo-pNext-02398",
                                "vkCreateImage(): VkImageCreateInfo struct has a chained VkExternalFormatANDROID struct with "
                                "non-zero externalFormat, but layout is not VK_IMAGE_TILING_OPTIMAL.");
            }
        }

        auto ahb_formats = GetAHBExternalFormatsSet(device_data);
        if ((0 != ext_fmt_android->externalFormat) && (0 == ahb_formats->count(ext_fmt_android->externalFormat))) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkExternalFormatANDROID-externalFormat-01894",
                            "vkCreateImage(): Chained VkExternalFormatANDROID struct contains a non-zero externalFormat which has "
                            "not been previously retrieved by vkGetAndroidHardwareBufferPropertiesANDROID().");
        }
    }

    if ((nullptr == ext_fmt_android) || (0 == ext_fmt_android->externalFormat)) {
        if (VK_FORMAT_UNDEFINED == create_info->format) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkImageCreateInfo-pNext-01975",
                            "vkCreateImage(): VkImageCreateInfo struct's format is VK_FORMAT_UNDEFINED, but either does not have a "
                            "chained VkExternalFormatANDROID struct or the struct exists but has an externalFormat of 0.");
        }
    }

    const VkExternalMemoryImageCreateInfo *emici = lvl_find_in_chain<VkExternalMemoryImageCreateInfo>(create_info->pNext);
    if (emici && (emici->handleTypes & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID)) {
        if (create_info->imageType != VK_IMAGE_TYPE_2D) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkImageCreateInfo-pNext-02393",
                        "vkCreateImage(): VkImageCreateInfo struct with imageType %s has chained VkExternalMemoryImageCreateInfo "
                        "struct with handleType VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID.",
                        string_VkImageType(create_info->imageType));
        }

        if ((create_info->mipLevels != 1) && (create_info->mipLevels != FullMipChainLevels(create_info->extent))) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkImageCreateInfo-pNext-02394",
                            "vkCreateImage(): VkImageCreateInfo struct with chained VkExternalMemoryImageCreateInfo struct of "
                            "handleType VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID "
                            "specifies mipLevels = %" PRId32 " (full chain mipLevels are %" PRId32 ").",
                            create_info->mipLevels, FullMipChainLevels(create_info->extent));
        }
    }

    return skip;
}

void RecordCreateImageANDROID(const VkImageCreateInfo *create_info, IMAGE_STATE *is_node) {
    const VkExternalMemoryImageCreateInfo *emici = lvl_find_in_chain<VkExternalMemoryImageCreateInfo>(create_info->pNext);
    if (emici && (emici->handleTypes & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID)) {
        is_node->imported_ahb = true;
    }
    const VkExternalFormatANDROID *ext_fmt_android = lvl_find_in_chain<VkExternalFormatANDROID>(create_info->pNext);
    if (ext_fmt_android && (0 != ext_fmt_android->externalFormat)) {
        is_node->has_ahb_format = true;
        is_node->ahb_format = ext_fmt_android->externalFormat;
    }
}

bool ValidateCreateImageViewANDROID(layer_data *device_data, const VkImageViewCreateInfo *create_info) {
    bool skip = false;
    const debug_report_data *report_data = core_validation::GetReportData(device_data);
    IMAGE_STATE *image_state = GetImageState(device_data, create_info->image);

    if (image_state->has_ahb_format) {
        if (VK_FORMAT_UNDEFINED != create_info->format) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(create_info->image), "VUID-VkImageViewCreateInfo-image-02399",
                            "vkCreateImageView(): VkImageViewCreateInfo struct has a chained VkExternalFormatANDROID struct, but "
                            "format member is %s.",
                            string_VkFormat(create_info->format));
        }

        // Chain must include a compatible ycbcr conversion
        bool conv_found = false;
        uint64_t external_format = 0;
        const VkSamplerYcbcrConversionInfo *ycbcr_conv_info = lvl_find_in_chain<VkSamplerYcbcrConversionInfo>(create_info->pNext);
        if (ycbcr_conv_info != nullptr) {
            VkSamplerYcbcrConversion conv_handle = ycbcr_conv_info->conversion;
            auto fmap = GetYcbcrConversionFormatMap(device_data);
            if (fmap->find(conv_handle) != fmap->end()) {
                conv_found = true;
                external_format = fmap->at(conv_handle);
            }
        }
        if ((!conv_found) || (external_format != image_state->ahb_format)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(create_info->image), "VUID-VkImageViewCreateInfo-image-02400",
                            "vkCreateImageView(): VkImageViewCreateInfo struct has a chained VkExternalFormatANDROID struct, but "
                            "without a chained VkSamplerYcbcrConversionInfo struct with the same external format.");
        }

        // Errors in create_info swizzles
        if ((create_info->components.r != VK_COMPONENT_SWIZZLE_IDENTITY) ||
            (create_info->components.g != VK_COMPONENT_SWIZZLE_IDENTITY) ||
            (create_info->components.b != VK_COMPONENT_SWIZZLE_IDENTITY) ||
            (create_info->components.a != VK_COMPONENT_SWIZZLE_IDENTITY)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(create_info->image), "VUID-VkImageViewCreateInfo-image-02401",
                            "vkCreateImageView(): VkImageViewCreateInfo struct has a chained VkExternalFormatANDROID struct, but "
                            "includes one or more non-identity component swizzles.");
        }
    }

    return skip;
}

bool ValidateGetImageSubresourceLayoutANDROID(layer_data *device_data, const VkImage image) {
    bool skip = false;
    const debug_report_data *report_data = core_validation::GetReportData(device_data);

    IMAGE_STATE *image_state = GetImageState(device_data, image);
    if (image_state->imported_ahb && (0 == image_state->GetBoundMemory().size())) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, HandleToUint64(image),
                        "VUID-vkGetImageSubresourceLayout-image-01895",
                        "vkGetImageSubresourceLayout(): Attempt to query layout from an image created with "
                        "VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID handleType which has not yet been "
                        "bound to memory.");
    }
    return skip;
}

#else

bool ValidateCreateImageANDROID(layer_data *device_data, const debug_report_data *report_data,
                                const VkImageCreateInfo *create_info) {
    return false;
}

void RecordCreateImageANDROID(const VkImageCreateInfo *create_info, IMAGE_STATE *is_node) {}

bool ValidateCreateImageViewANDROID(layer_data *device_data, const VkImageViewCreateInfo *create_info) { return false; }

bool ValidateGetImageSubresourceLayoutANDROID(layer_data *device_data, const VkImage image) { return false; }

#endif  // VK_USE_PLATFORM_ANDROID_KHR

bool PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                VkImage *pImage) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), core_validation::layer_data_map);
    bool skip = false;
    const debug_report_data *report_data = GetReportData(device_data);

    if (GetDeviceExtensions(device_data)->vk_android_external_memory_android_hardware_buffer) {
        skip |= ValidateCreateImageANDROID(device_data, report_data, pCreateInfo);
    } else {  // These checks are omitted or replaced when Android HW Buffer extension is active
        if (pCreateInfo->format == VK_FORMAT_UNDEFINED) {
            return log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                           "VUID-VkImageCreateInfo-format-00943",
                           "vkCreateImage(): VkFormat for image must not be VK_FORMAT_UNDEFINED.");
        }
    }

    if ((pCreateInfo->flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) && (VK_IMAGE_TYPE_2D != pCreateInfo->imageType)) {
        skip |= log_msg(
            report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
            "VUID-VkImageCreateInfo-flags-00949",
            "vkCreateImage(): Image type must be VK_IMAGE_TYPE_2D when VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT flag bit is set");
    }

    const VkPhysicalDeviceLimits *device_limits = &(GetPDProperties(device_data)->limits);
    VkImageUsageFlags attach_flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                     VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    if ((pCreateInfo->usage & attach_flags) && (pCreateInfo->extent.width > device_limits->maxFramebufferWidth)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkImageCreateInfo-usage-00964",
                        "vkCreateImage(): Image usage flags include a frame buffer attachment bit and image width exceeds device "
                        "maxFramebufferWidth.");
    }

    if ((pCreateInfo->usage & attach_flags) && (pCreateInfo->extent.height > device_limits->maxFramebufferHeight)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkImageCreateInfo-usage-00965",
                        "vkCreateImage(): Image usage flags include a frame buffer attachment bit and image height exceeds device "
                        "maxFramebufferHeight");
    }

    VkImageFormatProperties format_limits = {};
    VkResult res = GetPDImageFormatProperties(device_data, pCreateInfo, &format_limits);
    if (res == VK_ERROR_FORMAT_NOT_SUPPORTED) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, kVUIDUndefined,
                        "vkCreateImage(): Format %s is not supported for this combination of parameters.",
                        string_VkFormat(pCreateInfo->format));
    } else {
        if (pCreateInfo->mipLevels > format_limits.maxMipLevels) {
            const char *format_string = string_VkFormat(pCreateInfo->format);
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkImageCreateInfo-mipLevels-02255",
                            "vkCreateImage(): Image mip levels=%d exceed image format maxMipLevels=%d for format %s.",
                            pCreateInfo->mipLevels, format_limits.maxMipLevels, format_string);
        }

        uint64_t texel_count = (uint64_t)pCreateInfo->extent.width * (uint64_t)pCreateInfo->extent.height *
                               (uint64_t)pCreateInfo->extent.depth * (uint64_t)pCreateInfo->arrayLayers *
                               (uint64_t)pCreateInfo->samples;
        uint64_t total_size = (uint64_t)std::ceil(FormatTexelSize(pCreateInfo->format) * texel_count);

        // Round up to imageGranularity boundary
        VkDeviceSize imageGranularity = GetPDProperties(device_data)->limits.bufferImageGranularity;
        uint64_t ig_mask = imageGranularity - 1;
        total_size = (total_size + ig_mask) & ~ig_mask;

        if (total_size > format_limits.maxResourceSize) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, 0,
                            kVUID_Core_Image_InvalidFormatLimitsViolation,
                            "vkCreateImage(): resource size exceeds allowable maximum Image resource size = 0x%" PRIxLEAST64
                            ", maximum resource size = 0x%" PRIxLEAST64 " ",
                            total_size, format_limits.maxResourceSize);
        }

        if (pCreateInfo->arrayLayers > format_limits.maxArrayLayers) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, 0,
                            "VUID-VkImageCreateInfo-arrayLayers-02256",
                            "vkCreateImage(): arrayLayers=%d exceeds allowable maximum supported by format of %d.",
                            pCreateInfo->arrayLayers, format_limits.maxArrayLayers);
        }

        if ((pCreateInfo->samples & format_limits.sampleCounts) == 0) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, 0,
                        "VUID-VkImageCreateInfo-samples-02258", "vkCreateImage(): samples %s is not supported by format 0x%.8X.",
                        string_VkSampleCountFlagBits(pCreateInfo->samples), format_limits.sampleCounts);
        }
    }

    if ((pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_ALIASED_BIT) &&
        (!GetEnabledFeatures(device_data)->core.sparseResidencyAliased)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkImageCreateInfo-flags-01924",
                        "vkCreateImage(): the sparseResidencyAliased device feature is disabled: Images cannot be created with the "
                        "VK_IMAGE_CREATE_SPARSE_ALIASED_BIT set.");
    }

    if (GetDeviceExtensions(device_data)->vk_khr_maintenance2) {
        if (pCreateInfo->flags & VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT_KHR) {
            if (!(FormatIsCompressed_BC(pCreateInfo->format) || FormatIsCompressed_ASTC_LDR(pCreateInfo->format) ||
                  FormatIsCompressed_ETC2_EAC(pCreateInfo->format))) {
                // TODO: Add Maintenance2 VUID
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, kVUIDUndefined,
                            "vkCreateImage(): If pCreateInfo->flags contains VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT_KHR, "
                            "format must be block, ETC or ASTC compressed, but is %s",
                            string_VkFormat(pCreateInfo->format));
            }
            if (!(pCreateInfo->flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT)) {
                // TODO: Add Maintenance2 VUID
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, kVUIDUndefined,
                            "vkCreateImage(): If pCreateInfo->flags contains VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT_KHR, "
                            "flags must also contain VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT.");
            }
        }
    }

    return skip;
}

void PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                               VkImage *pImage, VkResult result) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), core_validation::layer_data_map);
    if (VK_SUCCESS != result) return;
    IMAGE_LAYOUT_NODE image_state;
    image_state.layout = pCreateInfo->initialLayout;
    image_state.format = pCreateInfo->format;
    IMAGE_STATE *is_node = new IMAGE_STATE(*pImage, pCreateInfo);
    if (GetDeviceExtensions(device_data)->vk_android_external_memory_android_hardware_buffer) {
        RecordCreateImageANDROID(pCreateInfo, is_node);
    }
    GetImageMap(device_data)->insert(std::make_pair(*pImage, std::unique_ptr<IMAGE_STATE>(is_node)));
    ImageSubresourcePair subpair{*pImage, false, VkImageSubresource()};
    (*core_validation::GetImageSubresourceMap(device_data))[*pImage].push_back(subpair);
    (*core_validation::GetImageLayoutMap(device_data))[subpair] = image_state;
}

bool PreCallValidateDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    IMAGE_STATE *image_state = core_validation::GetImageState(device_data, image);
    const VK_OBJECT obj_struct = {HandleToUint64(image), kVulkanObjectTypeImage};
    bool skip = false;
    if (image_state) {
        skip |= core_validation::ValidateObjectNotInUse(device_data, image_state, obj_struct, "vkDestroyImage",
                                                        "VUID-vkDestroyImage-image-01000");
    }
    return skip;
}

void PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    IMAGE_STATE *image_state = core_validation::GetImageState(device_data, image);
    VK_OBJECT obj_struct = {HandleToUint64(image), kVulkanObjectTypeImage};
    core_validation::InvalidateCommandBuffers(device_data, image_state->cb_bindings, obj_struct);
    // Clean up memory mapping, bindings and range references for image
    for (auto mem_binding : image_state->GetBoundMemory()) {
        auto mem_info = core_validation::GetMemObjInfo(device_data, mem_binding);
        if (mem_info) {
            core_validation::RemoveImageMemoryRange(obj_struct.handle, mem_info);
        }
    }
    core_validation::ClearMemoryObjectBindings(device_data, obj_struct.handle, kVulkanObjectTypeImage);
    EraseQFOReleaseBarriers<VkImageMemoryBarrier>(device_data, image);
    // Remove image from imageMap
    core_validation::GetImageMap(device_data)->erase(image);
    std::unordered_map<VkImage, std::vector<ImageSubresourcePair>> *imageSubresourceMap =
        core_validation::GetImageSubresourceMap(device_data);

    const auto &sub_entry = imageSubresourceMap->find(image);
    if (sub_entry != imageSubresourceMap->end()) {
        for (const auto &pair : sub_entry->second) {
            core_validation::GetImageLayoutMap(device_data)->erase(pair);
        }
        imageSubresourceMap->erase(sub_entry);
    }
}

bool ValidateImageAttributes(layer_data *device_data, IMAGE_STATE *image_state, VkImageSubresourceRange range) {
    bool skip = false;
    const debug_report_data *report_data = core_validation::GetReportData(device_data);

    if (range.aspectMask != VK_IMAGE_ASPECT_COLOR_BIT) {
        char const str[] = "vkCmdClearColorImage aspectMasks for all subresource ranges must be set to VK_IMAGE_ASPECT_COLOR_BIT";
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                        HandleToUint64(image_state->image), kVUID_Core_DrawState_InvalidImageAspect, str);
    }

    if (FormatIsDepthOrStencil(image_state->createInfo.format)) {
        char const str[] = "vkCmdClearColorImage called with depth/stencil image.";
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                        HandleToUint64(image_state->image), "VUID-vkCmdClearColorImage-image-00007", "%s.", str);
    } else if (FormatIsCompressed(image_state->createInfo.format)) {
        char const str[] = "vkCmdClearColorImage called with compressed image.";
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                        HandleToUint64(image_state->image), "VUID-vkCmdClearColorImage-image-00007", "%s.", str);
    }

    if (!(image_state->createInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        char const str[] = "vkCmdClearColorImage called with image created without VK_IMAGE_USAGE_TRANSFER_DST_BIT.";
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                        HandleToUint64(image_state->image), "VUID-vkCmdClearColorImage-image-00002", "%s.", str);
    }
    return skip;
}

uint32_t ResolveRemainingLevels(const VkImageSubresourceRange *range, uint32_t mip_levels) {
    // Return correct number of mip levels taking into account VK_REMAINING_MIP_LEVELS
    uint32_t mip_level_count = range->levelCount;
    if (range->levelCount == VK_REMAINING_MIP_LEVELS) {
        mip_level_count = mip_levels - range->baseMipLevel;
    }
    return mip_level_count;
}

uint32_t ResolveRemainingLayers(const VkImageSubresourceRange *range, uint32_t layers) {
    // Return correct number of layers taking into account VK_REMAINING_ARRAY_LAYERS
    uint32_t array_layer_count = range->layerCount;
    if (range->layerCount == VK_REMAINING_ARRAY_LAYERS) {
        array_layer_count = layers - range->baseArrayLayer;
    }
    return array_layer_count;
}

bool VerifyClearImageLayout(layer_data *device_data, GLOBAL_CB_NODE *cb_node, IMAGE_STATE *image_state,
                            VkImageSubresourceRange range, VkImageLayout dest_image_layout, const char *func_name) {
    bool skip = false;
    const debug_report_data *report_data = core_validation::GetReportData(device_data);

    uint32_t level_count = ResolveRemainingLevels(&range, image_state->createInfo.mipLevels);
    uint32_t layer_count = ResolveRemainingLayers(&range, image_state->createInfo.arrayLayers);

    if (dest_image_layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        if (dest_image_layout == VK_IMAGE_LAYOUT_GENERAL) {
            if (image_state->createInfo.tiling != VK_IMAGE_TILING_LINEAR) {
                // LAYOUT_GENERAL is allowed, but may not be performance optimal, flag as perf warning.
                skip |= log_msg(report_data, VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                HandleToUint64(image_state->image), kVUID_Core_DrawState_InvalidImageLayout,
                                "%s: Layout for cleared image should be TRANSFER_DST_OPTIMAL instead of GENERAL.", func_name);
            }
        } else if (VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR == dest_image_layout) {
            if (!GetDeviceExtensions(device_data)->vk_khr_shared_presentable_image) {
                // TODO: Add unique error id when available.
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                HandleToUint64(image_state->image), 0,
                                "Must enable VK_KHR_shared_presentable_image extension before creating images with a layout type "
                                "of VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR.");

            } else {
                if (image_state->shared_presentable) {
                    skip |= log_msg(
                        report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                        HandleToUint64(image_state->image), 0,
                        "Layout for shared presentable cleared image is %s but can only be VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR.",
                        string_VkImageLayout(dest_image_layout));
                }
            }
        } else {
            std::string error_code = "VUID-vkCmdClearColorImage-imageLayout-00005";
            if (strcmp(func_name, "vkCmdClearDepthStencilImage()") == 0) {
                error_code = "VUID-vkCmdClearDepthStencilImage-imageLayout-00012";
            } else {
                assert(strcmp(func_name, "vkCmdClearColorImage()") == 0);
            }
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(image_state->image), error_code,
                            "%s: Layout for cleared image is %s but can only be TRANSFER_DST_OPTIMAL or GENERAL.", func_name,
                            string_VkImageLayout(dest_image_layout));
        }
    }

    for (uint32_t level_index = 0; level_index < level_count; ++level_index) {
        uint32_t level = level_index + range.baseMipLevel;
        for (uint32_t layer_index = 0; layer_index < layer_count; ++layer_index) {
            uint32_t layer = layer_index + range.baseArrayLayer;
            VkImageSubresource sub = {range.aspectMask, level, layer};
            IMAGE_CMD_BUF_LAYOUT_NODE node;
            if (FindCmdBufLayout(device_data, cb_node, image_state->image, sub, node)) {
                if (node.layout != dest_image_layout) {
                    std::string error_code = "VUID-vkCmdClearColorImage-imageLayout-00004";
                    if (strcmp(func_name, "vkCmdClearDepthStencilImage()") == 0) {
                        error_code = "VUID-vkCmdClearDepthStencilImage-imageLayout-00011";
                    } else {
                        assert(strcmp(func_name, "vkCmdClearColorImage()") == 0);
                    }
                    skip |=
                        log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, 0,
                                error_code, "%s: Cannot clear an image whose layout is %s and doesn't match the current layout %s.",
                                func_name, string_VkImageLayout(dest_image_layout), string_VkImageLayout(node.layout));
                }
            }
        }
    }

    return skip;
}

void RecordClearImageLayout(layer_data *device_data, GLOBAL_CB_NODE *cb_node, VkImage image, VkImageSubresourceRange range,
                            VkImageLayout dest_image_layout) {
    VkImageCreateInfo *image_create_info = &(GetImageState(device_data, image)->createInfo);
    uint32_t level_count = ResolveRemainingLevels(&range, image_create_info->mipLevels);
    uint32_t layer_count = ResolveRemainingLayers(&range, image_create_info->arrayLayers);

    for (uint32_t level_index = 0; level_index < level_count; ++level_index) {
        uint32_t level = level_index + range.baseMipLevel;
        for (uint32_t layer_index = 0; layer_index < layer_count; ++layer_index) {
            uint32_t layer = layer_index + range.baseArrayLayer;
            VkImageSubresource sub = {range.aspectMask, level, layer};
            IMAGE_CMD_BUF_LAYOUT_NODE node;
            if (!FindCmdBufLayout(device_data, cb_node, image, sub, node)) {
                SetLayout(device_data, cb_node, image, sub, IMAGE_CMD_BUF_LAYOUT_NODE(dest_image_layout, dest_image_layout));
            }
        }
    }
}

bool PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                       const VkClearColorValue *pColor, uint32_t rangeCount,
                                       const VkImageSubresourceRange *pRanges) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    bool skip = false;
    // TODO : Verify memory is in VK_IMAGE_STATE_CLEAR state
    auto cb_node = GetCBNode(device_data, commandBuffer);
    auto image_state = GetImageState(device_data, image);
    if (cb_node && image_state) {
        skip |= ValidateMemoryIsBoundToImage(device_data, image_state, "vkCmdClearColorImage()",
                                             "VUID-vkCmdClearColorImage-image-00003");
        skip |= ValidateCmdQueueFlags(device_data, cb_node, "vkCmdClearColorImage()", VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
                                      "VUID-vkCmdClearColorImage-commandBuffer-cmdpool");
        skip |= ValidateCmd(device_data, cb_node, CMD_CLEARCOLORIMAGE, "vkCmdClearColorImage()");
        if (GetApiVersion(device_data) >= VK_API_VERSION_1_1 || GetDeviceExtensions(device_data)->vk_khr_maintenance1) {
            skip |= ValidateImageFormatFeatureFlags(device_data, image_state, VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                                                    "vkCmdClearColorImage", "VUID-vkCmdClearColorImage-image-01993",
                                                    "VUID-vkCmdClearColorImage-image-01993");
        }
        skip |= InsideRenderPass(device_data, cb_node, "vkCmdClearColorImage()", "VUID-vkCmdClearColorImage-renderpass");
        for (uint32_t i = 0; i < rangeCount; ++i) {
            std::string param_name = "pRanges[" + std::to_string(i) + "]";
            skip |= ValidateCmdClearColorSubresourceRange(device_data, image_state, pRanges[i], param_name.c_str());
            skip |= ValidateImageAttributes(device_data, image_state, pRanges[i]);
            skip |= VerifyClearImageLayout(device_data, cb_node, image_state, pRanges[i], imageLayout, "vkCmdClearColorImage()");
        }
    }
    return skip;
}

void PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                     const VkClearColorValue *pColor, uint32_t rangeCount, const VkImageSubresourceRange *pRanges) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    auto cb_node = GetCBNode(device_data, commandBuffer);
    auto image_state = GetImageState(device_data, image);
    if (cb_node && image_state) {
        AddCommandBufferBindingImage(device_data, cb_node, image_state);
        for (uint32_t i = 0; i < rangeCount; ++i) {
            RecordClearImageLayout(device_data, cb_node, image, pRanges[i], imageLayout);
        }
    }
}

bool PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                              const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                              const VkImageSubresourceRange *pRanges) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    const debug_report_data *report_data = core_validation::GetReportData(device_data);

    // TODO : Verify memory is in VK_IMAGE_STATE_CLEAR state
    auto cb_node = GetCBNode(device_data, commandBuffer);
    auto image_state = GetImageState(device_data, image);
    if (cb_node && image_state) {
        skip |= ValidateMemoryIsBoundToImage(device_data, image_state, "vkCmdClearDepthStencilImage()",
                                             "VUID-vkCmdClearDepthStencilImage-image-00010");
        skip |= ValidateCmdQueueFlags(device_data, cb_node, "vkCmdClearDepthStencilImage()", VK_QUEUE_GRAPHICS_BIT,
                                      "VUID-vkCmdClearDepthStencilImage-commandBuffer-cmdpool");
        skip |= ValidateCmd(device_data, cb_node, CMD_CLEARDEPTHSTENCILIMAGE, "vkCmdClearDepthStencilImage()");
        if (GetApiVersion(device_data) >= VK_API_VERSION_1_1 || GetDeviceExtensions(device_data)->vk_khr_maintenance1) {
            skip |= ValidateImageFormatFeatureFlags(device_data, image_state, VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                                                    "vkCmdClearDepthStencilImage", "VUID-vkCmdClearDepthStencilImage-image-01994",
                                                    "VUID-vkCmdClearDepthStencilImage-image-01994");
        }
        skip |=
            InsideRenderPass(device_data, cb_node, "vkCmdClearDepthStencilImage()", "VUID-vkCmdClearDepthStencilImage-renderpass");
        for (uint32_t i = 0; i < rangeCount; ++i) {
            std::string param_name = "pRanges[" + std::to_string(i) + "]";
            skip |= ValidateCmdClearDepthSubresourceRange(device_data, image_state, pRanges[i], param_name.c_str());
            skip |=
                VerifyClearImageLayout(device_data, cb_node, image_state, pRanges[i], imageLayout, "vkCmdClearDepthStencilImage()");
            // Image aspect must be depth or stencil or both
            VkImageAspectFlags valid_aspects = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            if (((pRanges[i].aspectMask & valid_aspects) == 0) || ((pRanges[i].aspectMask & ~valid_aspects) != 0)) {
                char const str[] =
                    "vkCmdClearDepthStencilImage aspectMasks for all subresource ranges must be set to VK_IMAGE_ASPECT_DEPTH_BIT "
                    "and/or VK_IMAGE_ASPECT_STENCIL_BIT";
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(commandBuffer), kVUID_Core_DrawState_InvalidImageAspect, str);
            }
        }
        if (image_state && !FormatIsDepthOrStencil(image_state->createInfo.format)) {
            char const str[] = "vkCmdClearDepthStencilImage called without a depth/stencil image.";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(image), "VUID-vkCmdClearDepthStencilImage-image-00014", "%s.", str);
        }
        if (VK_IMAGE_USAGE_TRANSFER_DST_BIT != (VK_IMAGE_USAGE_TRANSFER_DST_BIT & image_state->createInfo.usage)) {
            char const str[] =
                "vkCmdClearDepthStencilImage() called with an image that was not created with the VK_IMAGE_USAGE_TRANSFER_DST_BIT "
                "set.";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(image), "VUID-vkCmdClearDepthStencilImage-image-00009", "%s.", str);
        }
    }
    return skip;
}

void PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                            const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                            const VkImageSubresourceRange *pRanges) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    auto cb_node = GetCBNode(device_data, commandBuffer);
    auto image_state = GetImageState(device_data, image);
    if (cb_node && image_state) {
        AddCommandBufferBindingImage(device_data, cb_node, image_state);
        for (uint32_t i = 0; i < rangeCount; ++i) {
            RecordClearImageLayout(device_data, cb_node, image, pRanges[i], imageLayout);
        }
    }
}

// Returns true if [x, xoffset] and [y, yoffset] overlap
static bool RangesIntersect(int32_t start, uint32_t start_offset, int32_t end, uint32_t end_offset) {
    bool result = false;
    uint32_t intersection_min = std::max(static_cast<uint32_t>(start), static_cast<uint32_t>(end));
    uint32_t intersection_max = std::min(static_cast<uint32_t>(start) + start_offset, static_cast<uint32_t>(end) + end_offset);

    if (intersection_max > intersection_min) {
        result = true;
    }
    return result;
}

// Returns true if source area of first copy region intersects dest area of second region
// It is assumed that these are copy regions within a single image (otherwise no possibility of collision)
static bool RegionIntersects(const VkImageCopy *rgn0, const VkImageCopy *rgn1, VkImageType type, bool is_multiplane) {
    bool result = false;

    // Separate planes within a multiplane image cannot intersect
    if (is_multiplane && (rgn0->srcSubresource.aspectMask != rgn1->dstSubresource.aspectMask)) {
        return result;
    }

    if ((rgn0->srcSubresource.mipLevel == rgn1->dstSubresource.mipLevel) &&
        (RangesIntersect(rgn0->srcSubresource.baseArrayLayer, rgn0->srcSubresource.layerCount, rgn1->dstSubresource.baseArrayLayer,
                         rgn1->dstSubresource.layerCount))) {
        result = true;
        switch (type) {
            case VK_IMAGE_TYPE_3D:
                result &= RangesIntersect(rgn0->srcOffset.z, rgn0->extent.depth, rgn1->dstOffset.z, rgn1->extent.depth);
                // fall through
            case VK_IMAGE_TYPE_2D:
                result &= RangesIntersect(rgn0->srcOffset.y, rgn0->extent.height, rgn1->dstOffset.y, rgn1->extent.height);
                // fall through
            case VK_IMAGE_TYPE_1D:
                result &= RangesIntersect(rgn0->srcOffset.x, rgn0->extent.width, rgn1->dstOffset.x, rgn1->extent.width);
                break;
            default:
                // Unrecognized or new IMAGE_TYPE enums will be caught in parameter_validation
                assert(false);
        }
    }
    return result;
}

// Returns non-zero if offset and extent exceed image extents
static const uint32_t x_bit = 1;
static const uint32_t y_bit = 2;
static const uint32_t z_bit = 4;
static uint32_t ExceedsBounds(const VkOffset3D *offset, const VkExtent3D *extent, const VkExtent3D *image_extent) {
    uint32_t result = 0;
    // Extents/depths cannot be negative but checks left in for clarity
    if ((offset->z + extent->depth > image_extent->depth) || (offset->z < 0) ||
        ((offset->z + static_cast<int32_t>(extent->depth)) < 0)) {
        result |= z_bit;
    }
    if ((offset->y + extent->height > image_extent->height) || (offset->y < 0) ||
        ((offset->y + static_cast<int32_t>(extent->height)) < 0)) {
        result |= y_bit;
    }
    if ((offset->x + extent->width > image_extent->width) || (offset->x < 0) ||
        ((offset->x + static_cast<int32_t>(extent->width)) < 0)) {
        result |= x_bit;
    }
    return result;
}

// Test if two VkExtent3D structs are equivalent
static inline bool IsExtentEqual(const VkExtent3D *extent, const VkExtent3D *other_extent) {
    bool result = true;
    if ((extent->width != other_extent->width) || (extent->height != other_extent->height) ||
        (extent->depth != other_extent->depth)) {
        result = false;
    }
    return result;
}

// For image copies between compressed/uncompressed formats, the extent is provided in source image texels
// Destination image texel extents must be adjusted by block size for the dest validation checks
VkExtent3D GetAdjustedDestImageExtent(VkFormat src_format, VkFormat dst_format, VkExtent3D extent) {
    VkExtent3D adjusted_extent = extent;
    if ((FormatIsCompressed(src_format) && (!FormatIsCompressed(dst_format)))) {
        VkExtent3D block_size = FormatTexelBlockExtent(src_format);
        adjusted_extent.width /= block_size.width;
        adjusted_extent.height /= block_size.height;
        adjusted_extent.depth /= block_size.depth;
    } else if ((!FormatIsCompressed(src_format) && (FormatIsCompressed(dst_format)))) {
        VkExtent3D block_size = FormatTexelBlockExtent(dst_format);
        adjusted_extent.width *= block_size.width;
        adjusted_extent.height *= block_size.height;
        adjusted_extent.depth *= block_size.depth;
    }
    return adjusted_extent;
}

// Returns the effective extent of an image subresource, adjusted for mip level and array depth.
static inline VkExtent3D GetImageSubresourceExtent(const IMAGE_STATE *img, const VkImageSubresourceLayers *subresource) {
    const uint32_t mip = subresource->mipLevel;

    // Return zero extent if mip level doesn't exist
    if (mip >= img->createInfo.mipLevels) {
        return VkExtent3D{0, 0, 0};
    }

    // Don't allow mip adjustment to create 0 dim, but pass along a 0 if that's what subresource specified
    VkExtent3D extent = img->createInfo.extent;

    // If multi-plane, adjust per-plane extent
    if (FormatIsMultiplane(img->createInfo.format)) {
        VkExtent2D divisors = FindMultiplaneExtentDivisors(img->createInfo.format, subresource->aspectMask);
        extent.width /= divisors.width;
        extent.height /= divisors.height;
    }

    if (img->createInfo.flags & VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV) {
        extent.width = (0 == extent.width ? 0 : std::max(2U, 1 + ((extent.width - 1) >> mip)));
        extent.height = (0 == extent.height ? 0 : std::max(2U, 1 + ((extent.height - 1) >> mip)));
        extent.depth = (0 == extent.depth ? 0 : std::max(2U, 1 + ((extent.depth - 1) >> mip)));
    } else {
        extent.width = (0 == extent.width ? 0 : std::max(1U, extent.width >> mip));
        extent.height = (0 == extent.height ? 0 : std::max(1U, extent.height >> mip));
        extent.depth = (0 == extent.depth ? 0 : std::max(1U, extent.depth >> mip));
    }

    // Image arrays have an effective z extent that isn't diminished by mip level
    if (VK_IMAGE_TYPE_3D != img->createInfo.imageType) {
        extent.depth = img->createInfo.arrayLayers;
    }

    return extent;
}

// Test if the extent argument has all dimensions set to 0.
static inline bool IsExtentAllZeroes(const VkExtent3D *extent) {
    return ((extent->width == 0) && (extent->height == 0) && (extent->depth == 0));
}

// Test if the extent argument has any dimensions set to 0.
static inline bool IsExtentSizeZero(const VkExtent3D *extent) {
    return ((extent->width == 0) || (extent->height == 0) || (extent->depth == 0));
}

// Returns the image transfer granularity for a specific image scaled by compressed block size if necessary.
static inline VkExtent3D GetScaledItg(layer_data *device_data, const GLOBAL_CB_NODE *cb_node, const IMAGE_STATE *img) {
    // Default to (0, 0, 0) granularity in case we can't find the real granularity for the physical device.
    VkExtent3D granularity = {0, 0, 0};
    auto pPool = GetCommandPoolNode(device_data, cb_node->createInfo.commandPool);
    if (pPool) {
        granularity =
            GetPhysDevProperties(device_data)->queue_family_properties[pPool->queueFamilyIndex].minImageTransferGranularity;
        if (FormatIsCompressed(img->createInfo.format)) {
            auto block_size = FormatTexelBlockExtent(img->createInfo.format);
            granularity.width *= block_size.width;
            granularity.height *= block_size.height;
        }
    }
    return granularity;
}

// Test elements of a VkExtent3D structure against alignment constraints contained in another VkExtent3D structure
static inline bool IsExtentAligned(const VkExtent3D *extent, const VkExtent3D *granularity) {
    bool valid = true;
    if ((SafeModulo(extent->depth, granularity->depth) != 0) || (SafeModulo(extent->width, granularity->width) != 0) ||
        (SafeModulo(extent->height, granularity->height) != 0)) {
        valid = false;
    }
    return valid;
}

// Check elements of a VkOffset3D structure against a queue family's Image Transfer Granularity values
static inline bool CheckItgOffset(layer_data *device_data, const GLOBAL_CB_NODE *cb_node, const VkOffset3D *offset,
                                  const VkExtent3D *granularity, const uint32_t i, const char *function, const char *member,
                                  std::string vuid) {
    const debug_report_data *report_data = core_validation::GetReportData(device_data);
    bool skip = false;
    VkExtent3D offset_extent = {};
    offset_extent.width = static_cast<uint32_t>(abs(offset->x));
    offset_extent.height = static_cast<uint32_t>(abs(offset->y));
    offset_extent.depth = static_cast<uint32_t>(abs(offset->z));
    if (IsExtentAllZeroes(granularity)) {
        // If the queue family image transfer granularity is (0, 0, 0), then the offset must always be (0, 0, 0)
        if (IsExtentAllZeroes(&offset_extent) == false) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_node->commandBuffer), vuid,
                            "%s: pRegion[%d].%s (x=%d, y=%d, z=%d) must be (x=0, y=0, z=0) when the command buffer's queue family "
                            "image transfer granularity is (w=0, h=0, d=0).",
                            function, i, member, offset->x, offset->y, offset->z);
        }
    } else {
        // If the queue family image transfer granularity is not (0, 0, 0), then the offset dimensions must always be even
        // integer multiples of the image transfer granularity.
        if (IsExtentAligned(&offset_extent, granularity) == false) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_node->commandBuffer), vuid,
                            "%s: pRegion[%d].%s (x=%d, y=%d, z=%d) dimensions must be even integer multiples of this command "
                            "buffer's queue family image transfer granularity (w=%d, h=%d, d=%d).",
                            function, i, member, offset->x, offset->y, offset->z, granularity->width, granularity->height,
                            granularity->depth);
        }
    }
    return skip;
}

// Check elements of a VkExtent3D structure against a queue family's Image Transfer Granularity values
static inline bool CheckItgExtent(layer_data *device_data, const GLOBAL_CB_NODE *cb_node, const VkExtent3D *extent,
                                  const VkOffset3D *offset, const VkExtent3D *granularity, const VkExtent3D *subresource_extent,
                                  const VkImageType image_type, const uint32_t i, const char *function, const char *member,
                                  std::string vuid) {
    const debug_report_data *report_data = core_validation::GetReportData(device_data);
    bool skip = false;
    if (IsExtentAllZeroes(granularity)) {
        // If the queue family image transfer granularity is (0, 0, 0), then the extent must always match the image
        // subresource extent.
        if (IsExtentEqual(extent, subresource_extent) == false) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_node->commandBuffer), vuid,
                            "%s: pRegion[%d].%s (w=%d, h=%d, d=%d) must match the image subresource extents (w=%d, h=%d, d=%d) "
                            "when the command buffer's queue family image transfer granularity is (w=0, h=0, d=0).",
                            function, i, member, extent->width, extent->height, extent->depth, subresource_extent->width,
                            subresource_extent->height, subresource_extent->depth);
        }
    } else {
        // If the queue family image transfer granularity is not (0, 0, 0), then the extent dimensions must always be even
        // integer multiples of the image transfer granularity or the offset + extent dimensions must always match the image
        // subresource extent dimensions.
        VkExtent3D offset_extent_sum = {};
        offset_extent_sum.width = static_cast<uint32_t>(abs(offset->x)) + extent->width;
        offset_extent_sum.height = static_cast<uint32_t>(abs(offset->y)) + extent->height;
        offset_extent_sum.depth = static_cast<uint32_t>(abs(offset->z)) + extent->depth;
        bool x_ok = true;
        bool y_ok = true;
        bool z_ok = true;
        switch (image_type) {
            case VK_IMAGE_TYPE_3D:
                z_ok = ((0 == SafeModulo(extent->depth, granularity->depth)) ||
                        (subresource_extent->depth == offset_extent_sum.depth));
                // fall through
            case VK_IMAGE_TYPE_2D:
                y_ok = ((0 == SafeModulo(extent->height, granularity->height)) ||
                        (subresource_extent->height == offset_extent_sum.height));
                // fall through
            case VK_IMAGE_TYPE_1D:
                x_ok = ((0 == SafeModulo(extent->width, granularity->width)) ||
                        (subresource_extent->width == offset_extent_sum.width));
                break;
            default:
                // Unrecognized or new IMAGE_TYPE enums will be caught in parameter_validation
                assert(false);
        }
        if (!(x_ok && y_ok && z_ok)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_node->commandBuffer), vuid,
                            "%s: pRegion[%d].%s (w=%d, h=%d, d=%d) dimensions must be even integer multiples of this command "
                            "buffer's queue family image transfer granularity (w=%d, h=%d, d=%d) or offset (x=%d, y=%d, z=%d) + "
                            "extent (w=%d, h=%d, d=%d) must match the image subresource extents (w=%d, h=%d, d=%d).",
                            function, i, member, extent->width, extent->height, extent->depth, granularity->width,
                            granularity->height, granularity->depth, offset->x, offset->y, offset->z, extent->width, extent->height,
                            extent->depth, subresource_extent->width, subresource_extent->height, subresource_extent->depth);
        }
    }
    return skip;
}

bool ValidateImageMipLevel(layer_data *device_data, const GLOBAL_CB_NODE *cb_node, const IMAGE_STATE *img, uint32_t mip_level,
                           const uint32_t i, const char *function, const char *member, const std::string &vuid) {
    const debug_report_data *report_data = core_validation::GetReportData(device_data);
    bool skip = false;
    if (mip_level >= img->createInfo.mipLevels) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(cb_node->commandBuffer), vuid,
                        "In %s, pRegions[%u].%s.mipLevel is %u, but provided image %" PRIx64 " has %u mip levels.", function, i,
                        member, mip_level, HandleToUint64(img->image), img->createInfo.mipLevels);
    }
    return skip;
}

bool ValidateImageArrayLayerRange(layer_data *device_data, const GLOBAL_CB_NODE *cb_node, const IMAGE_STATE *img,
                                  const uint32_t base_layer, const uint32_t layer_count, const uint32_t i, const char *function,
                                  const char *member, const std::string &vuid) {
    const debug_report_data *report_data = core_validation::GetReportData(device_data);
    bool skip = false;
    if (base_layer >= img->createInfo.arrayLayers || layer_count > img->createInfo.arrayLayers ||
        (base_layer + layer_count) > img->createInfo.arrayLayers) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(cb_node->commandBuffer), vuid,
                        "In %s, pRegions[%u].%s.baseArrayLayer is %u and .layerCount is "
                        "%u, but provided image %" PRIx64 " has %u array layers.",
                        function, i, member, base_layer, layer_count, HandleToUint64(img->image), img->createInfo.arrayLayers);
    }
    return skip;
}

// Check valid usage Image Transfer Granularity requirements for elements of a VkBufferImageCopy structure
bool ValidateCopyBufferImageTransferGranularityRequirements(layer_data *device_data, const GLOBAL_CB_NODE *cb_node,
                                                            const IMAGE_STATE *img, const VkBufferImageCopy *region,
                                                            const uint32_t i, const char *function, const std::string &vuid) {
    bool skip = false;
    VkExtent3D granularity = GetScaledItg(device_data, cb_node, img);
    skip |= CheckItgOffset(device_data, cb_node, &region->imageOffset, &granularity, i, function, "imageOffset", vuid);
    VkExtent3D subresource_extent = GetImageSubresourceExtent(img, &region->imageSubresource);
    skip |= CheckItgExtent(device_data, cb_node, &region->imageExtent, &region->imageOffset, &granularity, &subresource_extent,
                           img->createInfo.imageType, i, function, "imageExtent", vuid);
    return skip;
}

// Check valid usage Image Transfer Granularity requirements for elements of a VkImageCopy structure
bool ValidateCopyImageTransferGranularityRequirements(layer_data *device_data, const GLOBAL_CB_NODE *cb_node,
                                                      const IMAGE_STATE *src_img, const IMAGE_STATE *dst_img,
                                                      const VkImageCopy *region, const uint32_t i, const char *function) {
    bool skip = false;
    // Source image checks
    VkExtent3D granularity = GetScaledItg(device_data, cb_node, src_img);
    skip |= CheckItgOffset(device_data, cb_node, &region->srcOffset, &granularity, i, function, "srcOffset",
                           "VUID-vkCmdCopyImage-srcOffset-01783");
    VkExtent3D subresource_extent = GetImageSubresourceExtent(src_img, &region->srcSubresource);
    const VkExtent3D extent = region->extent;
    skip |= CheckItgExtent(device_data, cb_node, &extent, &region->srcOffset, &granularity, &subresource_extent,
                           src_img->createInfo.imageType, i, function, "extent", "VUID-vkCmdCopyImage-srcOffset-01783");

    // Destination image checks
    granularity = GetScaledItg(device_data, cb_node, dst_img);
    skip |= CheckItgOffset(device_data, cb_node, &region->dstOffset, &granularity, i, function, "dstOffset",
                           "VUID-vkCmdCopyImage-dstOffset-01784");
    // Adjust dest extent, if necessary
    const VkExtent3D dest_effective_extent =
        GetAdjustedDestImageExtent(src_img->createInfo.format, dst_img->createInfo.format, extent);
    subresource_extent = GetImageSubresourceExtent(dst_img, &region->dstSubresource);
    skip |= CheckItgExtent(device_data, cb_node, &dest_effective_extent, &region->dstOffset, &granularity, &subresource_extent,
                           dst_img->createInfo.imageType, i, function, "extent", "VUID-vkCmdCopyImage-dstOffset-01784");
    return skip;
}

// Validate contents of a VkImageCopy struct
bool ValidateImageCopyData(const layer_data *device_data, const debug_report_data *report_data, const uint32_t regionCount,
                           const VkImageCopy *ic_regions, const IMAGE_STATE *src_state, const IMAGE_STATE *dst_state) {
    bool skip = false;

    for (uint32_t i = 0; i < regionCount; i++) {
        const VkImageCopy region = ic_regions[i];

        // For comp<->uncomp copies, the copy extent for the dest image must be adjusted
        const VkExtent3D src_copy_extent = region.extent;
        const VkExtent3D dst_copy_extent =
            GetAdjustedDestImageExtent(src_state->createInfo.format, dst_state->createInfo.format, region.extent);

        bool slice_override = false;
        uint32_t depth_slices = 0;

        // Special case for copying between a 1D/2D array and a 3D image
        // TBD: This seems like the only way to reconcile 3 mutually-exclusive VU checks for 2D/3D copies. Heads up.
        if ((VK_IMAGE_TYPE_3D == src_state->createInfo.imageType) && (VK_IMAGE_TYPE_3D != dst_state->createInfo.imageType)) {
            depth_slices = region.dstSubresource.layerCount;  // Slice count from 2D subresource
            slice_override = (depth_slices != 1);
        } else if ((VK_IMAGE_TYPE_3D == dst_state->createInfo.imageType) && (VK_IMAGE_TYPE_3D != src_state->createInfo.imageType)) {
            depth_slices = region.srcSubresource.layerCount;  // Slice count from 2D subresource
            slice_override = (depth_slices != 1);
        }

        // Do all checks on source image
        //
        if (src_state->createInfo.imageType == VK_IMAGE_TYPE_1D) {
            if ((0 != region.srcOffset.y) || (1 != src_copy_extent.height)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                HandleToUint64(src_state->image), "VUID-VkImageCopy-srcImage-00146",
                                "vkCmdCopyImage(): pRegion[%d] srcOffset.y is %d and extent.height is %d. For 1D images these must "
                                "be 0 and 1, respectively.",
                                i, region.srcOffset.y, src_copy_extent.height);
            }
        }

        // VUID-VkImageCopy-srcImage-01785
        if ((src_state->createInfo.imageType == VK_IMAGE_TYPE_1D) && ((0 != region.srcOffset.z) || (1 != src_copy_extent.depth))) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(src_state->image), "VUID-VkImageCopy-srcImage-01785",
                            "vkCmdCopyImage(): pRegion[%d] srcOffset.z is %d and extent.depth is %d. For 1D images "
                            "these must be 0 and 1, respectively.",
                            i, region.srcOffset.z, src_copy_extent.depth);
        }

        // VUID-VkImageCopy-srcImage-01787
        if ((src_state->createInfo.imageType == VK_IMAGE_TYPE_2D) && (0 != region.srcOffset.z)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(src_state->image), "VUID-VkImageCopy-srcImage-01787",
                            "vkCmdCopyImage(): pRegion[%d] srcOffset.z is %d. For 2D images the z-offset must be 0.", i,
                            region.srcOffset.z);
        }

        if (GetDeviceExtensions(device_data)->vk_khr_maintenance1) {
            if (src_state->createInfo.imageType == VK_IMAGE_TYPE_3D) {
                if ((0 != region.srcSubresource.baseArrayLayer) || (1 != region.srcSubresource.layerCount)) {
                    skip |=
                        log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                HandleToUint64(src_state->image), "VUID-VkImageCopy-srcImage-00141",
                                "vkCmdCopyImage(): pRegion[%d] srcSubresource.baseArrayLayer is %d and srcSubresource.layerCount "
                                "is %d. For VK_IMAGE_TYPE_3D images these must be 0 and 1, respectively.",
                                i, region.srcSubresource.baseArrayLayer, region.srcSubresource.layerCount);
                }
            }
        } else {  // Pre maint 1
            if (src_state->createInfo.imageType == VK_IMAGE_TYPE_3D || dst_state->createInfo.imageType == VK_IMAGE_TYPE_3D) {
                if ((0 != region.srcSubresource.baseArrayLayer) || (1 != region.srcSubresource.layerCount)) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                    HandleToUint64(src_state->image), "VUID-VkImageCopy-srcImage-00141",
                                    "vkCmdCopyImage(): pRegion[%d] srcSubresource.baseArrayLayer is %d and "
                                    "srcSubresource.layerCount is %d. For copies with either source or dest of type "
                                    "VK_IMAGE_TYPE_3D, these must be 0 and 1, respectively.",
                                    i, region.srcSubresource.baseArrayLayer, region.srcSubresource.layerCount);
                }
            }
        }

        // Source checks that apply only to compressed images (or to _422 images if ycbcr enabled)
        bool ext_ycbcr = GetDeviceExtensions(device_data)->vk_khr_sampler_ycbcr_conversion;
        if (FormatIsCompressed(src_state->createInfo.format) ||
            (ext_ycbcr && FormatIsSinglePlane_422(src_state->createInfo.format))) {
            const VkExtent3D block_size = FormatTexelBlockExtent(src_state->createInfo.format);
            //  image offsets must be multiples of block dimensions
            if ((SafeModulo(region.srcOffset.x, block_size.width) != 0) ||
                (SafeModulo(region.srcOffset.y, block_size.height) != 0) ||
                (SafeModulo(region.srcOffset.z, block_size.depth) != 0)) {
                std::string vuid = ext_ycbcr ? "VUID-VkImageCopy-srcImage-01727" : "VUID-VkImageCopy-srcOffset-00157";
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                HandleToUint64(src_state->image), vuid,
                                "vkCmdCopyImage(): pRegion[%d] srcOffset (%d, %d) must be multiples of the compressed image's "
                                "texel width & height (%d, %d).",
                                i, region.srcOffset.x, region.srcOffset.y, block_size.width, block_size.height);
            }

            const VkExtent3D mip_extent = GetImageSubresourceExtent(src_state, &(region.srcSubresource));
            if ((SafeModulo(src_copy_extent.width, block_size.width) != 0) &&
                (src_copy_extent.width + region.srcOffset.x != mip_extent.width)) {
                std::string vuid = ext_ycbcr ? "VUID-VkImageCopy-srcImage-01728" : "VUID-VkImageCopy-extent-00158";
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(src_state->image), vuid,
                            "vkCmdCopyImage(): pRegion[%d] extent width (%d) must be a multiple of the compressed texture block "
                            "width (%d), or when added to srcOffset.x (%d) must equal the image subresource width (%d).",
                            i, src_copy_extent.width, block_size.width, region.srcOffset.x, mip_extent.width);
            }

            // Extent height must be a multiple of block height, or extent+offset height must equal subresource height
            if ((SafeModulo(src_copy_extent.height, block_size.height) != 0) &&
                (src_copy_extent.height + region.srcOffset.y != mip_extent.height)) {
                std::string vuid = ext_ycbcr ? "VUID-VkImageCopy-srcImage-01729" : "VUID-VkImageCopy-extent-00159";
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(src_state->image), vuid,
                            "vkCmdCopyImage(): pRegion[%d] extent height (%d) must be a multiple of the compressed texture block "
                            "height (%d), or when added to srcOffset.y (%d) must equal the image subresource height (%d).",
                            i, src_copy_extent.height, block_size.height, region.srcOffset.y, mip_extent.height);
            }

            // Extent depth must be a multiple of block depth, or extent+offset depth must equal subresource depth
            uint32_t copy_depth = (slice_override ? depth_slices : src_copy_extent.depth);
            if ((SafeModulo(copy_depth, block_size.depth) != 0) && (copy_depth + region.srcOffset.z != mip_extent.depth)) {
                std::string vuid = ext_ycbcr ? "VUID-VkImageCopy-srcImage-01730" : "VUID-VkImageCopy-extent-00160";
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(src_state->image), vuid,
                            "vkCmdCopyImage(): pRegion[%d] extent width (%d) must be a multiple of the compressed texture block "
                            "depth (%d), or when added to srcOffset.z (%d) must equal the image subresource depth (%d).",
                            i, src_copy_extent.depth, block_size.depth, region.srcOffset.z, mip_extent.depth);
            }
        }  // Compressed

        // Do all checks on dest image
        //
        if (dst_state->createInfo.imageType == VK_IMAGE_TYPE_1D) {
            if ((0 != region.dstOffset.y) || (1 != dst_copy_extent.height)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                HandleToUint64(dst_state->image), "VUID-VkImageCopy-dstImage-00152",
                                "vkCmdCopyImage(): pRegion[%d] dstOffset.y is %d and dst_copy_extent.height is %d. For 1D images "
                                "these must be 0 and 1, respectively.",
                                i, region.dstOffset.y, dst_copy_extent.height);
            }
        }

        // VUID-VkImageCopy-dstImage-01786
        if ((dst_state->createInfo.imageType == VK_IMAGE_TYPE_1D) && ((0 != region.dstOffset.z) || (1 != dst_copy_extent.depth))) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(dst_state->image), "VUID-VkImageCopy-dstImage-01786",
                            "vkCmdCopyImage(): pRegion[%d] dstOffset.z is %d and extent.depth is %d. For 1D images these must be 0 "
                            "and 1, respectively.",
                            i, region.dstOffset.z, dst_copy_extent.depth);
        }

        // VUID-VkImageCopy-dstImage-01788
        if ((dst_state->createInfo.imageType == VK_IMAGE_TYPE_2D) && (0 != region.dstOffset.z)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(dst_state->image), "VUID-VkImageCopy-dstImage-01788",
                            "vkCmdCopyImage(): pRegion[%d] dstOffset.z is %d. For 2D images the z-offset must be 0.", i,
                            region.dstOffset.z);
        }

        if (dst_state->createInfo.imageType == VK_IMAGE_TYPE_3D) {
            if ((0 != region.dstSubresource.baseArrayLayer) || (1 != region.dstSubresource.layerCount)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                HandleToUint64(dst_state->image), "VUID-VkImageCopy-srcImage-00141",
                                "vkCmdCopyImage(): pRegion[%d] dstSubresource.baseArrayLayer is %d and dstSubresource.layerCount "
                                "is %d. For VK_IMAGE_TYPE_3D images these must be 0 and 1, respectively.",
                                i, region.dstSubresource.baseArrayLayer, region.dstSubresource.layerCount);
            }
        }
        // VU01199 changed with mnt1
        if (GetDeviceExtensions(device_data)->vk_khr_maintenance1) {
            if (dst_state->createInfo.imageType == VK_IMAGE_TYPE_3D) {
                if ((0 != region.dstSubresource.baseArrayLayer) || (1 != region.dstSubresource.layerCount)) {
                    skip |=
                        log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                HandleToUint64(dst_state->image), "VUID-VkImageCopy-srcImage-00141",
                                "vkCmdCopyImage(): pRegion[%d] dstSubresource.baseArrayLayer is %d and dstSubresource.layerCount "
                                "is %d. For VK_IMAGE_TYPE_3D images these must be 0 and 1, respectively.",
                                i, region.dstSubresource.baseArrayLayer, region.dstSubresource.layerCount);
                }
            }
        } else {  // Pre maint 1
            if (src_state->createInfo.imageType == VK_IMAGE_TYPE_3D || dst_state->createInfo.imageType == VK_IMAGE_TYPE_3D) {
                if ((0 != region.dstSubresource.baseArrayLayer) || (1 != region.dstSubresource.layerCount)) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                    HandleToUint64(dst_state->image), "VUID-VkImageCopy-srcImage-00141",
                                    "vkCmdCopyImage(): pRegion[%d] dstSubresource.baseArrayLayer is %d and "
                                    "dstSubresource.layerCount is %d. For copies with either source or dest of type "
                                    "VK_IMAGE_TYPE_3D, these must be 0 and 1, respectively.",
                                    i, region.dstSubresource.baseArrayLayer, region.dstSubresource.layerCount);
                }
            }
        }

        // Dest checks that apply only to compressed images (or to _422 images if ycbcr enabled)
        if (FormatIsCompressed(dst_state->createInfo.format) ||
            (ext_ycbcr && FormatIsSinglePlane_422(dst_state->createInfo.format))) {
            const VkExtent3D block_size = FormatTexelBlockExtent(dst_state->createInfo.format);

            //  image offsets must be multiples of block dimensions
            if ((SafeModulo(region.dstOffset.x, block_size.width) != 0) ||
                (SafeModulo(region.dstOffset.y, block_size.height) != 0) ||
                (SafeModulo(region.dstOffset.z, block_size.depth) != 0)) {
                std::string vuid = ext_ycbcr ? "VUID-VkImageCopy-dstImage-01731" : "VUID-VkImageCopy-dstOffset-00162";
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                HandleToUint64(dst_state->image), vuid,
                                "vkCmdCopyImage(): pRegion[%d] dstOffset (%d, %d) must be multiples of the compressed image's "
                                "texel width & height (%d, %d).",
                                i, region.dstOffset.x, region.dstOffset.y, block_size.width, block_size.height);
            }

            const VkExtent3D mip_extent = GetImageSubresourceExtent(dst_state, &(region.dstSubresource));
            if ((SafeModulo(dst_copy_extent.width, block_size.width) != 0) &&
                (dst_copy_extent.width + region.dstOffset.x != mip_extent.width)) {
                std::string vuid = ext_ycbcr ? "VUID-VkImageCopy-dstImage-01732" : "VUID-VkImageCopy-extent-00163";
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(dst_state->image), vuid,
                            "vkCmdCopyImage(): pRegion[%d] dst_copy_extent width (%d) must be a multiple of the compressed texture "
                            "block width (%d), or when added to dstOffset.x (%d) must equal the image subresource width (%d).",
                            i, dst_copy_extent.width, block_size.width, region.dstOffset.x, mip_extent.width);
            }

            // Extent height must be a multiple of block height, or dst_copy_extent+offset height must equal subresource height
            if ((SafeModulo(dst_copy_extent.height, block_size.height) != 0) &&
                (dst_copy_extent.height + region.dstOffset.y != mip_extent.height)) {
                std::string vuid = ext_ycbcr ? "VUID-VkImageCopy-dstImage-01733" : "VUID-VkImageCopy-extent-00164";
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                HandleToUint64(dst_state->image), vuid,
                                "vkCmdCopyImage(): pRegion[%d] dst_copy_extent height (%d) must be a multiple of the compressed "
                                "texture block height (%d), or when added to dstOffset.y (%d) must equal the image subresource "
                                "height (%d).",
                                i, dst_copy_extent.height, block_size.height, region.dstOffset.y, mip_extent.height);
            }

            // Extent depth must be a multiple of block depth, or dst_copy_extent+offset depth must equal subresource depth
            uint32_t copy_depth = (slice_override ? depth_slices : dst_copy_extent.depth);
            if ((SafeModulo(copy_depth, block_size.depth) != 0) && (copy_depth + region.dstOffset.z != mip_extent.depth)) {
                std::string vuid = ext_ycbcr ? "VUID-VkImageCopy-dstImage-01734" : "VUID-VkImageCopy-extent-00165";
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(dst_state->image), vuid,
                            "vkCmdCopyImage(): pRegion[%d] dst_copy_extent width (%d) must be a multiple of the compressed texture "
                            "block depth (%d), or when added to dstOffset.z (%d) must equal the image subresource depth (%d).",
                            i, dst_copy_extent.depth, block_size.depth, region.dstOffset.z, mip_extent.depth);
            }
        }  // Compressed
    }
    return skip;
}

// vkCmdCopyImage checks that only apply if the multiplane extension is enabled
bool CopyImageMultiplaneValidation(const layer_data *dev_data, VkCommandBuffer command_buffer, const IMAGE_STATE *src_image_state,
                                   const IMAGE_STATE *dst_image_state, const VkImageCopy region) {
    bool skip = false;
    const debug_report_data *report_data = core_validation::GetReportData(dev_data);

    // Neither image is multiplane
    if ((!FormatIsMultiplane(src_image_state->createInfo.format)) && (!FormatIsMultiplane(dst_image_state->createInfo.format))) {
        // If neither image is multi-plane the aspectMask member of src and dst must match
        if (region.srcSubresource.aspectMask != region.dstSubresource.aspectMask) {
            std::stringstream ss;
            ss << "vkCmdCopyImage(): Copy between non-multiplane images with differing aspectMasks ( 0x" << std::hex
               << region.srcSubresource.aspectMask << " and 0x" << region.dstSubresource.aspectMask << " )";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(command_buffer), "VUID-VkImageCopy-srcImage-01551", "%s.", ss.str().c_str());
        }
    } else {
        // Source image multiplane checks
        uint32_t planes = FormatPlaneCount(src_image_state->createInfo.format);
        VkImageAspectFlags aspect = region.srcSubresource.aspectMask;
        if ((2 == planes) && (aspect != VK_IMAGE_ASPECT_PLANE_0_BIT_KHR) && (aspect != VK_IMAGE_ASPECT_PLANE_1_BIT_KHR)) {
            std::stringstream ss;
            ss << "vkCmdCopyImage(): Source image aspect mask (0x" << std::hex << aspect << ") is invalid for 2-plane format";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(command_buffer), "VUID-VkImageCopy-srcImage-01552", "%s.", ss.str().c_str());
        }
        if ((3 == planes) && (aspect != VK_IMAGE_ASPECT_PLANE_0_BIT_KHR) && (aspect != VK_IMAGE_ASPECT_PLANE_1_BIT_KHR) &&
            (aspect != VK_IMAGE_ASPECT_PLANE_2_BIT_KHR)) {
            std::stringstream ss;
            ss << "vkCmdCopyImage(): Source image aspect mask (0x" << std::hex << aspect << ") is invalid for 3-plane format";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(command_buffer), "VUID-VkImageCopy-srcImage-01553", "%s.", ss.str().c_str());
        }
        // Single-plane to multi-plane
        if ((!FormatIsMultiplane(src_image_state->createInfo.format)) && (FormatIsMultiplane(dst_image_state->createInfo.format)) &&
            (VK_IMAGE_ASPECT_COLOR_BIT != aspect)) {
            std::stringstream ss;
            ss << "vkCmdCopyImage(): Source image aspect mask (0x" << std::hex << aspect << ") is not VK_IMAGE_ASPECT_COLOR_BIT";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(command_buffer), "VUID-VkImageCopy-dstImage-01557", "%s.", ss.str().c_str());
        }

        // Dest image multiplane checks
        planes = FormatPlaneCount(dst_image_state->createInfo.format);
        aspect = region.dstSubresource.aspectMask;
        if ((2 == planes) && (aspect != VK_IMAGE_ASPECT_PLANE_0_BIT_KHR) && (aspect != VK_IMAGE_ASPECT_PLANE_1_BIT_KHR)) {
            std::stringstream ss;
            ss << "vkCmdCopyImage(): Dest image aspect mask (0x" << std::hex << aspect << ") is invalid for 2-plane format";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(command_buffer), "VUID-VkImageCopy-dstImage-01554", "%s.", ss.str().c_str());
        }
        if ((3 == planes) && (aspect != VK_IMAGE_ASPECT_PLANE_0_BIT_KHR) && (aspect != VK_IMAGE_ASPECT_PLANE_1_BIT_KHR) &&
            (aspect != VK_IMAGE_ASPECT_PLANE_2_BIT_KHR)) {
            std::stringstream ss;
            ss << "vkCmdCopyImage(): Dest image aspect mask (0x" << std::hex << aspect << ") is invalid for 3-plane format";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(command_buffer), "VUID-VkImageCopy-dstImage-01555", "%s.", ss.str().c_str());
        }
        // Multi-plane to single-plane
        if ((FormatIsMultiplane(src_image_state->createInfo.format)) && (!FormatIsMultiplane(dst_image_state->createInfo.format)) &&
            (VK_IMAGE_ASPECT_COLOR_BIT != aspect)) {
            std::stringstream ss;
            ss << "vkCmdCopyImage(): Dest image aspect mask (0x" << std::hex << aspect << ") is not VK_IMAGE_ASPECT_COLOR_BIT";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(command_buffer), "VUID-VkImageCopy-srcImage-01556", "%s.", ss.str().c_str());
        }
    }

    return skip;
}

bool PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                 VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy *pRegions) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    auto cb_node = GetCBNode(device_data, commandBuffer);
    auto src_image_state = GetImageState(device_data, srcImage);
    auto dst_image_state = GetImageState(device_data, dstImage);
    bool skip = false;
    const debug_report_data *report_data = device_data->report_data;

    skip = ValidateImageCopyData(device_data, report_data, regionCount, pRegions, src_image_state, dst_image_state);

    VkCommandBuffer command_buffer = cb_node->commandBuffer;

    for (uint32_t i = 0; i < regionCount; i++) {
        const VkImageCopy region = pRegions[i];

        // For comp/uncomp copies, the copy extent for the dest image must be adjusted
        VkExtent3D src_copy_extent = region.extent;
        VkExtent3D dst_copy_extent =
            GetAdjustedDestImageExtent(src_image_state->createInfo.format, dst_image_state->createInfo.format, region.extent);

        bool slice_override = false;
        uint32_t depth_slices = 0;

        // Special case for copying between a 1D/2D array and a 3D image
        // TBD: This seems like the only way to reconcile 3 mutually-exclusive VU checks for 2D/3D copies. Heads up.
        if ((VK_IMAGE_TYPE_3D == src_image_state->createInfo.imageType) &&
            (VK_IMAGE_TYPE_3D != dst_image_state->createInfo.imageType)) {
            depth_slices = region.dstSubresource.layerCount;  // Slice count from 2D subresource
            slice_override = (depth_slices != 1);
        } else if ((VK_IMAGE_TYPE_3D == dst_image_state->createInfo.imageType) &&
                   (VK_IMAGE_TYPE_3D != src_image_state->createInfo.imageType)) {
            depth_slices = region.srcSubresource.layerCount;  // Slice count from 2D subresource
            slice_override = (depth_slices != 1);
        }

        skip |= ValidateImageSubresourceLayers(device_data, cb_node, &region.srcSubresource, "vkCmdCopyImage", "srcSubresource", i);
        skip |= ValidateImageSubresourceLayers(device_data, cb_node, &region.dstSubresource, "vkCmdCopyImage", "dstSubresource", i);
        skip |= ValidateImageMipLevel(device_data, cb_node, src_image_state, region.srcSubresource.mipLevel, i, "vkCmdCopyImage",
                                      "srcSubresource", "VUID-vkCmdCopyImage-srcSubresource-01696");
        skip |= ValidateImageMipLevel(device_data, cb_node, dst_image_state, region.dstSubresource.mipLevel, i, "vkCmdCopyImage",
                                      "dstSubresource", "VUID-vkCmdCopyImage-dstSubresource-01697");
        skip |= ValidateImageArrayLayerRange(device_data, cb_node, src_image_state, region.srcSubresource.baseArrayLayer,
                                             region.srcSubresource.layerCount, i, "vkCmdCopyImage", "srcSubresource",
                                             "VUID-vkCmdCopyImage-srcSubresource-01698");
        skip |= ValidateImageArrayLayerRange(device_data, cb_node, dst_image_state, region.dstSubresource.baseArrayLayer,
                                             region.dstSubresource.layerCount, i, "vkCmdCopyImage", "dstSubresource",
                                             "VUID-vkCmdCopyImage-dstSubresource-01699");

        if (GetDeviceExtensions(device_data)->vk_khr_maintenance1) {
            // No chance of mismatch if we're overriding depth slice count
            if (!slice_override) {
                // The number of depth slices in srcSubresource and dstSubresource must match
                // Depth comes from layerCount for 1D,2D resources, from extent.depth for 3D
                uint32_t src_slices =
                    (VK_IMAGE_TYPE_3D == src_image_state->createInfo.imageType ? src_copy_extent.depth
                                                                               : region.srcSubresource.layerCount);
                uint32_t dst_slices =
                    (VK_IMAGE_TYPE_3D == dst_image_state->createInfo.imageType ? dst_copy_extent.depth
                                                                               : region.dstSubresource.layerCount);
                if (src_slices != dst_slices) {
                    std::stringstream ss;
                    ss << "vkCmdCopyImage(): number of depth slices in source and destination subresources for pRegions[" << i
                       << "] do not match";
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                    HandleToUint64(command_buffer), "VUID-VkImageCopy-extent-00140", "%s.", ss.str().c_str());
                }
            }
        } else {
            // For each region the layerCount member of srcSubresource and dstSubresource must match
            if (region.srcSubresource.layerCount != region.dstSubresource.layerCount) {
                std::stringstream ss;
                ss << "vkCmdCopyImage(): number of layers in source and destination subresources for pRegions[" << i
                   << "] do not match";
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(command_buffer), "VUID-VkImageCopy-extent-00140", "%s.", ss.str().c_str());
            }
        }

        // Do multiplane-specific checks, if extension enabled
        if (GetDeviceExtensions(device_data)->vk_khr_sampler_ycbcr_conversion) {
            skip |= CopyImageMultiplaneValidation(device_data, command_buffer, src_image_state, dst_image_state, region);
        }

        if (!GetDeviceExtensions(device_data)->vk_khr_sampler_ycbcr_conversion) {
            // not multi-plane, the aspectMask member of srcSubresource and dstSubresource must match
            if (region.srcSubresource.aspectMask != region.dstSubresource.aspectMask) {
                char const str[] = "vkCmdCopyImage(): Src and dest aspectMasks for each region must match";
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(command_buffer), "VUID-VkImageCopy-aspectMask-00137", "%s.", str);
            }
        }

        // For each region, the aspectMask member of srcSubresource must be present in the source image
        if (!VerifyAspectsPresent(region.srcSubresource.aspectMask, src_image_state->createInfo.format)) {
            std::stringstream ss;
            ss << "vkCmdCopyImage(): pRegion[" << i
               << "] srcSubresource.aspectMask cannot specify aspects not present in source image";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(command_buffer), "VUID-VkImageCopy-aspectMask-00142", "%s.", ss.str().c_str());
        }

        // For each region, the aspectMask member of dstSubresource must be present in the destination image
        if (!VerifyAspectsPresent(region.dstSubresource.aspectMask, dst_image_state->createInfo.format)) {
            std::stringstream ss;
            ss << "vkCmdCopyImage(): pRegion[" << i
               << "] dstSubresource.aspectMask cannot specify aspects not present in dest image";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(command_buffer), "VUID-VkImageCopy-aspectMask-00143", "%s.", ss.str().c_str());
        }

        // Check region extents for 1D-1D, 2D-2D, and 3D-3D copies
        if (src_image_state->createInfo.imageType == dst_image_state->createInfo.imageType) {
            // The source region specified by a given element of regions must be a region that is contained within srcImage
            VkExtent3D img_extent = GetImageSubresourceExtent(src_image_state, &(region.srcSubresource));
            if (0 != ExceedsBounds(&region.srcOffset, &src_copy_extent, &img_extent)) {
                std::stringstream ss;
                ss << "vkCmdCopyImage(): Source pRegion[" << i << "] with mipLevel [ " << region.srcSubresource.mipLevel
                   << " ], offset [ " << region.srcOffset.x << ", " << region.srcOffset.y << ", " << region.srcOffset.z
                   << " ], extent [ " << src_copy_extent.width << ", " << src_copy_extent.height << ", " << src_copy_extent.depth
                   << " ] exceeds the source image dimensions";
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(command_buffer), "VUID-vkCmdCopyImage-pRegions-00122", "%s.", ss.str().c_str());
            }

            // The destination region specified by a given element of regions must be a region that is contained within dst_image
            img_extent = GetImageSubresourceExtent(dst_image_state, &(region.dstSubresource));
            if (0 != ExceedsBounds(&region.dstOffset, &dst_copy_extent, &img_extent)) {
                std::stringstream ss;
                ss << "vkCmdCopyImage(): Dest pRegion[" << i << "] with mipLevel [ " << region.dstSubresource.mipLevel
                   << " ], offset [ " << region.dstOffset.x << ", " << region.dstOffset.y << ", " << region.dstOffset.z
                   << " ], extent [ " << dst_copy_extent.width << ", " << dst_copy_extent.height << ", " << dst_copy_extent.depth
                   << " ] exceeds the destination image dimensions";
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(command_buffer), "VUID-vkCmdCopyImage-pRegions-00123", "%s.", ss.str().c_str());
            }
        }

        // Each dimension offset + extent limits must fall with image subresource extent
        VkExtent3D subresource_extent = GetImageSubresourceExtent(src_image_state, &(region.srcSubresource));
        if (slice_override) src_copy_extent.depth = depth_slices;
        uint32_t extent_check = ExceedsBounds(&(region.srcOffset), &src_copy_extent, &subresource_extent);
        if (extent_check & x_bit) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(command_buffer), "VUID-VkImageCopy-srcOffset-00144",
                        "vkCmdCopyImage(): Source image pRegion %1d x-dimension offset [%1d] + extent [%1d] exceeds subResource "
                        "width [%1d].",
                        i, region.srcOffset.x, src_copy_extent.width, subresource_extent.width);
        }

        if (extent_check & y_bit) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(command_buffer), "VUID-VkImageCopy-srcOffset-00145",
                        "vkCmdCopyImage(): Source image pRegion %1d y-dimension offset [%1d] + extent [%1d] exceeds subResource "
                        "height [%1d].",
                        i, region.srcOffset.y, src_copy_extent.height, subresource_extent.height);
        }
        if (extent_check & z_bit) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(command_buffer), "VUID-VkImageCopy-srcOffset-00147",
                        "vkCmdCopyImage(): Source image pRegion %1d z-dimension offset [%1d] + extent [%1d] exceeds subResource "
                        "depth [%1d].",
                        i, region.srcOffset.z, src_copy_extent.depth, subresource_extent.depth);
        }

        // Adjust dest extent if necessary
        subresource_extent = GetImageSubresourceExtent(dst_image_state, &(region.dstSubresource));
        if (slice_override) dst_copy_extent.depth = depth_slices;

        extent_check = ExceedsBounds(&(region.dstOffset), &dst_copy_extent, &subresource_extent);
        if (extent_check & x_bit) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(command_buffer), "VUID-VkImageCopy-dstOffset-00150",
                            "vkCmdCopyImage(): Dest image pRegion %1d x-dimension offset [%1d] + extent [%1d] exceeds subResource "
                            "width [%1d].",
                            i, region.dstOffset.x, dst_copy_extent.width, subresource_extent.width);
        }
        if (extent_check & y_bit) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(command_buffer), "VUID-VkImageCopy-dstOffset-00151",
                            "vkCmdCopyImage(): Dest image pRegion %1d y-dimension offset [%1d] + extent [%1d] exceeds subResource "
                            "height [%1d].",
                            i, region.dstOffset.y, dst_copy_extent.height, subresource_extent.height);
        }
        if (extent_check & z_bit) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(command_buffer), "VUID-VkImageCopy-dstOffset-00153",
                            "vkCmdCopyImage(): Dest image pRegion %1d z-dimension offset [%1d] + extent [%1d] exceeds subResource "
                            "depth [%1d].",
                            i, region.dstOffset.z, dst_copy_extent.depth, subresource_extent.depth);
        }

        // The union of all source regions, and the union of all destination regions, specified by the elements of regions,
        // must not overlap in memory
        if (src_image_state->image == dst_image_state->image) {
            for (uint32_t j = 0; j < regionCount; j++) {
                if (RegionIntersects(&region, &pRegions[j], src_image_state->createInfo.imageType,
                                     FormatIsMultiplane(src_image_state->createInfo.format))) {
                    std::stringstream ss;
                    ss << "vkCmdCopyImage(): pRegions[" << i << "] src overlaps with pRegions[" << j << "].";
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                    HandleToUint64(command_buffer), "VUID-vkCmdCopyImage-pRegions-00124", "%s.", ss.str().c_str());
                }
            }
        }
    }

    // The formats of src_image and dst_image must be compatible. Formats are considered compatible if their texel size in bytes
    // is the same between both formats. For example, VK_FORMAT_R8G8B8A8_UNORM is compatible with VK_FORMAT_R32_UINT because
    // because both texels are 4 bytes in size. Depth/stencil formats must match exactly.
    if (FormatIsDepthOrStencil(src_image_state->createInfo.format) || FormatIsDepthOrStencil(dst_image_state->createInfo.format)) {
        if (src_image_state->createInfo.format != dst_image_state->createInfo.format) {
            char const str[] = "vkCmdCopyImage called with unmatched source and dest image depth/stencil formats.";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(command_buffer), kVUID_Core_DrawState_MismatchedImageFormat, str);
        }
    } else {
        if (!FormatSizesAreEqual(src_image_state->createInfo.format, dst_image_state->createInfo.format, regionCount, pRegions)) {
            char const str[] = "vkCmdCopyImage called with unmatched source and dest image format sizes.";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(command_buffer), "VUID-vkCmdCopyImage-srcImage-00135", "%s.", str);
        }
    }

    // Source and dest image sample counts must match
    if (src_image_state->createInfo.samples != dst_image_state->createInfo.samples) {
        char const str[] = "vkCmdCopyImage() called on image pair with non-identical sample counts.";
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(command_buffer), "VUID-vkCmdCopyImage-srcImage-00136", "%s", str);
    }

    skip |= ValidateMemoryIsBoundToImage(device_data, src_image_state, "vkCmdCopyImage()", "VUID-vkCmdCopyImage-srcImage-00127");
    skip |= ValidateMemoryIsBoundToImage(device_data, dst_image_state, "vkCmdCopyImage()", "VUID-vkCmdCopyImage-dstImage-00132");
    // Validate that SRC & DST images have correct usage flags set
    skip |= ValidateImageUsageFlags(device_data, src_image_state, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, true,
                                    "VUID-vkCmdCopyImage-srcImage-00126", "vkCmdCopyImage()", "VK_IMAGE_USAGE_TRANSFER_SRC_BIT");
    skip |= ValidateImageUsageFlags(device_data, dst_image_state, VK_IMAGE_USAGE_TRANSFER_DST_BIT, true,
                                    "VUID-vkCmdCopyImage-dstImage-00131", "vkCmdCopyImage()", "VK_IMAGE_USAGE_TRANSFER_DST_BIT");
    if (GetApiVersion(device_data) >= VK_API_VERSION_1_1 || GetDeviceExtensions(device_data)->vk_khr_maintenance1) {
        skip |=
            ValidateImageFormatFeatureFlags(device_data, src_image_state, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT, "vkCmdCopyImage()",
                                            "VUID-vkCmdCopyImage-srcImage-01995", "VUID-vkCmdCopyImage-srcImage-01995");
        skip |=
            ValidateImageFormatFeatureFlags(device_data, dst_image_state, VK_FORMAT_FEATURE_TRANSFER_DST_BIT, "vkCmdCopyImage()",
                                            "VUID-vkCmdCopyImage-dstImage-01996", "VUID-vkCmdCopyImage-dstImage-01996");
    }
    skip |= ValidateCmdQueueFlags(device_data, cb_node, "vkCmdCopyImage()",
                                  VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
                                  "VUID-vkCmdCopyImage-commandBuffer-cmdpool");
    skip |= ValidateCmd(device_data, cb_node, CMD_COPYIMAGE, "vkCmdCopyImage()");
    skip |= InsideRenderPass(device_data, cb_node, "vkCmdCopyImage()", "VUID-vkCmdCopyImage-renderpass");
    bool hit_error = false;
    const std::string invalid_src_layout_vuid =
        (src_image_state->shared_presentable && core_validation::GetDeviceExtensions(device_data)->vk_khr_shared_presentable_image)
            ? "VUID-vkCmdCopyImage-srcImageLayout-01917"
            : "VUID-vkCmdCopyImage-srcImageLayout-00129";
    const std::string invalid_dst_layout_vuid =
        (dst_image_state->shared_presentable && core_validation::GetDeviceExtensions(device_data)->vk_khr_shared_presentable_image)
            ? "VUID-vkCmdCopyImage-dstImageLayout-01395"
            : "VUID-vkCmdCopyImage-dstImageLayout-00134";
    for (uint32_t i = 0; i < regionCount; ++i) {
        skip |= VerifyImageLayout(device_data, cb_node, src_image_state, pRegions[i].srcSubresource, srcImageLayout,
                                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, "vkCmdCopyImage()", invalid_src_layout_vuid,
                                  "VUID-vkCmdCopyImage-srcImageLayout-00128", &hit_error);
        skip |= VerifyImageLayout(device_data, cb_node, dst_image_state, pRegions[i].dstSubresource, dstImageLayout,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, "vkCmdCopyImage()", invalid_dst_layout_vuid,
                                  "VUID-vkCmdCopyImage-dstImageLayout-00133", &hit_error);
        skip |= ValidateCopyImageTransferGranularityRequirements(device_data, cb_node, src_image_state, dst_image_state,
                                                                 &pRegions[i], i, "vkCmdCopyImage()");
    }

    return skip;
}

void PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                               VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy *pRegions) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    auto cb_node = GetCBNode(device_data, commandBuffer);
    auto src_image_state = GetImageState(device_data, srcImage);
    auto dst_image_state = GetImageState(device_data, dstImage);

    // Make sure that all image slices are updated to correct layout
    for (uint32_t i = 0; i < regionCount; ++i) {
        SetImageLayout(device_data, cb_node, src_image_state, pRegions[i].srcSubresource, srcImageLayout);
        SetImageLayout(device_data, cb_node, dst_image_state, pRegions[i].dstSubresource, dstImageLayout);
    }
    // Update bindings between images and cmd buffer
    AddCommandBufferBindingImage(device_data, cb_node, src_image_state);
    AddCommandBufferBindingImage(device_data, cb_node, dst_image_state);
}

// Returns true if sub_rect is entirely contained within rect
static inline bool ContainsRect(VkRect2D rect, VkRect2D sub_rect) {
    if ((sub_rect.offset.x < rect.offset.x) || (sub_rect.offset.x + sub_rect.extent.width > rect.offset.x + rect.extent.width) ||
        (sub_rect.offset.y < rect.offset.y) || (sub_rect.offset.y + sub_rect.extent.height > rect.offset.y + rect.extent.height))
        return false;
    return true;
}

bool PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                        const VkClearAttachment *pAttachments, uint32_t rectCount, const VkClearRect *pRects) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);

    GLOBAL_CB_NODE *cb_node = GetCBNode(device_data, commandBuffer);
    const debug_report_data *report_data = device_data->report_data;

    bool skip = false;
    if (cb_node) {
        skip |= ValidateCmdQueueFlags(device_data, cb_node, "vkCmdClearAttachments()", VK_QUEUE_GRAPHICS_BIT,
                                      "VUID-vkCmdClearAttachments-commandBuffer-cmdpool");
        skip |= ValidateCmd(device_data, cb_node, CMD_CLEARATTACHMENTS, "vkCmdClearAttachments()");
        // Warn if this is issued prior to Draw Cmd and clearing the entire attachment
        if (!cb_node->hasDrawCmd && (cb_node->activeRenderPassBeginInfo.renderArea.extent.width == pRects[0].rect.extent.width) &&
            (cb_node->activeRenderPassBeginInfo.renderArea.extent.height == pRects[0].rect.extent.height)) {
            // There are times where app needs to use ClearAttachments (generally when reusing a buffer inside of a render pass)
            // This warning should be made more specific. It'd be best to avoid triggering this test if it's a use that must call
            // CmdClearAttachments.
            skip |= log_msg(
                report_data, VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                HandleToUint64(commandBuffer), kVUID_Core_DrawState_ClearCmdBeforeDraw,
                "vkCmdClearAttachments() issued on command buffer object 0x%" PRIx64
                " prior to any Draw Cmds. It is recommended you use RenderPass LOAD_OP_CLEAR on Attachments prior to any Draw.",
                HandleToUint64(commandBuffer));
        }
        skip |= OutsideRenderPass(device_data, cb_node, "vkCmdClearAttachments()", "VUID-vkCmdClearAttachments-renderpass");
    }

    // Validate that attachment is in reference list of active subpass
    if (cb_node->activeRenderPass) {
        const VkRenderPassCreateInfo2KHR *renderpass_create_info = cb_node->activeRenderPass->createInfo.ptr();
        const VkSubpassDescription2KHR *subpass_desc = &renderpass_create_info->pSubpasses[cb_node->activeSubpass];
        auto framebuffer = GetFramebufferState(device_data, cb_node->activeFramebuffer);

        for (uint32_t i = 0; i < attachmentCount; i++) {
            auto clear_desc = &pAttachments[i];
            VkImageView image_view = VK_NULL_HANDLE;

            if (0 == clear_desc->aspectMask) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(commandBuffer), "VUID-VkClearAttachment-aspectMask-requiredbitmask", " ");
            } else if (clear_desc->aspectMask & VK_IMAGE_ASPECT_METADATA_BIT) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(commandBuffer), "VUID-VkClearAttachment-aspectMask-00020", " ");
            } else if (clear_desc->aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) {
                if ((subpass_desc->pColorAttachments[clear_desc->colorAttachment].attachment != VK_ATTACHMENT_UNUSED) &&
                    (clear_desc->colorAttachment >= subpass_desc->colorAttachmentCount)) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                    HandleToUint64(commandBuffer), "VUID-vkCmdClearAttachments-aspectMask-02501",
                                    "vkCmdClearAttachments() color attachment index %d is not VK_ATTACHMENT_UNUSED and is out of "
                                    "range for active subpass %d.",
                                    clear_desc->colorAttachment, cb_node->activeSubpass);
                } else {
                    image_view = framebuffer->createInfo
                                     .pAttachments[subpass_desc->pColorAttachments[clear_desc->colorAttachment].attachment];
                }
                if ((clear_desc->aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) ||
                    (clear_desc->aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT)) {
                    char const str[] =
                        "vkCmdClearAttachments() aspectMask [%d] must set only VK_IMAGE_ASPECT_COLOR_BIT of a color attachment.";
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                    HandleToUint64(commandBuffer), "VUID-VkClearAttachment-aspectMask-00019", str, i);
                }
            } else {  // Must be depth and/or stencil
                if (((clear_desc->aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) != VK_IMAGE_ASPECT_DEPTH_BIT) &&
                    ((clear_desc->aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) != VK_IMAGE_ASPECT_STENCIL_BIT)) {
                    char const str[] = "vkCmdClearAttachments() aspectMask [%d] is not a valid combination of bits.";
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                    HandleToUint64(commandBuffer), "VUID-VkClearAttachment-aspectMask-parameter", str, i);
                }
                if (!subpass_desc->pDepthStencilAttachment ||
                    (subpass_desc->pDepthStencilAttachment->attachment == VK_ATTACHMENT_UNUSED)) {
                    skip |= log_msg(
                        report_data, VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(commandBuffer), kVUID_Core_DrawState_MissingAttachmentReference,
                        "vkCmdClearAttachments() depth/stencil clear with no depth/stencil attachment in subpass; ignored");
                } else {
                    image_view = framebuffer->createInfo.pAttachments[subpass_desc->pDepthStencilAttachment->attachment];
                }
            }
            if (image_view) {
                auto image_view_state = GetImageViewState(device_data, image_view);
                for (uint32_t j = 0; j < rectCount; j++) {
                    // The rectangular region specified by a given element of pRects must be contained within the render area of
                    // the current render pass instance
                    if (cb_node->createInfo.level == VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
                        if (false == ContainsRect(cb_node->activeRenderPassBeginInfo.renderArea, pRects[j].rect)) {
                            skip |=
                                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                        HandleToUint64(commandBuffer), "VUID-vkCmdClearAttachments-pRects-00016",
                                        "vkCmdClearAttachments(): The area defined by pRects[%d] is not contained in the area of "
                                        "the current render pass instance.",
                                        j);
                        }
                    } else {
                        const auto local_rect =
                            pRects[j].rect;  // local copy of rect captured by value below to preserve original contents
                        cb_node->cmd_execute_commands_functions.emplace_back([=](GLOBAL_CB_NODE *prim_cb, VkFramebuffer fb) {
                            if (false == ContainsRect(prim_cb->activeRenderPassBeginInfo.renderArea, local_rect)) {
                                return log_msg(
                                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                    HandleToUint64(commandBuffer), "VUID-vkCmdClearAttachments-pRects-00016",
                                    "vkCmdClearAttachments(): The area defined by pRects[%d] is not contained in the area of "
                                    "the current render pass instance.",
                                    j);
                            }
                            return false;
                        });
                    }
                    // The layers specified by a given element of pRects must be contained within every attachment that
                    // pAttachments refers to
                    auto attachment_layer_count = image_view_state->create_info.subresourceRange.layerCount;
                    if ((pRects[j].baseArrayLayer >= attachment_layer_count) ||
                        (pRects[j].baseArrayLayer + pRects[j].layerCount > attachment_layer_count)) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                        HandleToUint64(commandBuffer), "VUID-vkCmdClearAttachments-pRects-00017",
                                        "vkCmdClearAttachments(): The layers defined in pRects[%d] are not contained in the layers "
                                        "of pAttachment[%d].",
                                        j, i);
                    }
                }
            }
        }
    }
    return skip;
}

bool PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                    VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve *pRegions) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    auto cb_node = GetCBNode(device_data, commandBuffer);
    auto src_image_state = GetImageState(device_data, srcImage);
    auto dst_image_state = GetImageState(device_data, dstImage);

    const debug_report_data *report_data = device_data->report_data;
    bool skip = false;
    if (cb_node && src_image_state && dst_image_state) {
        skip |= ValidateMemoryIsBoundToImage(device_data, src_image_state, "vkCmdResolveImage()",
                                             "VUID-vkCmdResolveImage-srcImage-00256");
        skip |= ValidateMemoryIsBoundToImage(device_data, dst_image_state, "vkCmdResolveImage()",
                                             "VUID-vkCmdResolveImage-dstImage-00258");
        skip |= ValidateCmdQueueFlags(device_data, cb_node, "vkCmdResolveImage()", VK_QUEUE_GRAPHICS_BIT,
                                      "VUID-vkCmdResolveImage-commandBuffer-cmdpool");
        skip |= ValidateCmd(device_data, cb_node, CMD_RESOLVEIMAGE, "vkCmdResolveImage()");
        skip |= InsideRenderPass(device_data, cb_node, "vkCmdResolveImage()", "VUID-vkCmdResolveImage-renderpass");
        skip |= ValidateImageFormatFeatureFlags(device_data, dst_image_state, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT,
                                                "vkCmdResolveImage()", "VUID-vkCmdResolveImage-dstImage-02003",
                                                "VUID-vkCmdResolveImage-dstImage-02003");

        bool hit_error = false;
        const std::string invalid_src_layout_vuid =
            (src_image_state->shared_presentable &&
             core_validation::GetDeviceExtensions(device_data)->vk_khr_shared_presentable_image)
                ? "VUID-vkCmdResolveImage-srcImageLayout-01400"
                : "VUID-vkCmdResolveImage-srcImageLayout-00261";
        const std::string invalid_dst_layout_vuid =
            (dst_image_state->shared_presentable &&
             core_validation::GetDeviceExtensions(device_data)->vk_khr_shared_presentable_image)
                ? "VUID-vkCmdResolveImage-dstImageLayout-01401"
                : "VUID-vkCmdResolveImage-dstImageLayout-00263";
        // For each region, the number of layers in the image subresource should not be zero
        // For each region, src and dest image aspect must be color only
        for (uint32_t i = 0; i < regionCount; i++) {
            skip |= ValidateImageSubresourceLayers(device_data, cb_node, &pRegions[i].srcSubresource, "vkCmdResolveImage()",
                                                   "srcSubresource", i);
            skip |= ValidateImageSubresourceLayers(device_data, cb_node, &pRegions[i].dstSubresource, "vkCmdResolveImage()",
                                                   "dstSubresource", i);
            skip |= VerifyImageLayout(device_data, cb_node, src_image_state, pRegions[i].srcSubresource, srcImageLayout,
                                      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, "vkCmdResolveImage()", invalid_src_layout_vuid,
                                      "VUID-vkCmdResolveImage-srcImageLayout-00260", &hit_error);
            skip |= VerifyImageLayout(device_data, cb_node, dst_image_state, pRegions[i].dstSubresource, dstImageLayout,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, "vkCmdResolveImage()", invalid_dst_layout_vuid,
                                      "VUID-vkCmdResolveImage-dstImageLayout-00262", &hit_error);
            skip |= ValidateImageMipLevel(device_data, cb_node, src_image_state, pRegions[i].srcSubresource.mipLevel, i,
                                          "vkCmdResolveImage()", "srcSubresource", "VUID-vkCmdResolveImage-srcSubresource-01709");
            skip |= ValidateImageMipLevel(device_data, cb_node, dst_image_state, pRegions[i].dstSubresource.mipLevel, i,
                                          "vkCmdResolveImage()", "dstSubresource", "VUID-vkCmdResolveImage-dstSubresource-01710");
            skip |= ValidateImageArrayLayerRange(device_data, cb_node, src_image_state, pRegions[i].srcSubresource.baseArrayLayer,
                                                 pRegions[i].srcSubresource.layerCount, i, "vkCmdResolveImage()", "srcSubresource",
                                                 "VUID-vkCmdResolveImage-srcSubresource-01711");
            skip |= ValidateImageArrayLayerRange(device_data, cb_node, dst_image_state, pRegions[i].dstSubresource.baseArrayLayer,
                                                 pRegions[i].dstSubresource.layerCount, i, "vkCmdResolveImage()", "srcSubresource",
                                                 "VUID-vkCmdResolveImage-dstSubresource-01712");

            // layer counts must match
            if (pRegions[i].srcSubresource.layerCount != pRegions[i].dstSubresource.layerCount) {
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                    HandleToUint64(cb_node->commandBuffer), "VUID-VkImageResolve-layerCount-00267",
                    "vkCmdResolveImage(): layerCount in source and destination subresource of pRegions[%d] does not match.", i);
            }
            // For each region, src and dest image aspect must be color only
            if ((pRegions[i].srcSubresource.aspectMask != VK_IMAGE_ASPECT_COLOR_BIT) ||
                (pRegions[i].dstSubresource.aspectMask != VK_IMAGE_ASPECT_COLOR_BIT)) {
                char const str[] =
                    "vkCmdResolveImage(): src and dest aspectMasks for each region must specify only VK_IMAGE_ASPECT_COLOR_BIT";
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(cb_node->commandBuffer), "VUID-VkImageResolve-aspectMask-00266", "%s.", str);
            }
        }

        if (src_image_state->createInfo.format != dst_image_state->createInfo.format) {
            char const str[] = "vkCmdResolveImage called with unmatched source and dest formats.";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_node->commandBuffer), kVUID_Core_DrawState_MismatchedImageFormat, str);
        }
        if (src_image_state->createInfo.imageType != dst_image_state->createInfo.imageType) {
            char const str[] = "vkCmdResolveImage called with unmatched source and dest image types.";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_node->commandBuffer), kVUID_Core_DrawState_MismatchedImageType, str);
        }
        if (src_image_state->createInfo.samples == VK_SAMPLE_COUNT_1_BIT) {
            char const str[] = "vkCmdResolveImage called with source sample count less than 2.";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_node->commandBuffer), "VUID-vkCmdResolveImage-srcImage-00257", "%s.", str);
        }
        if (dst_image_state->createInfo.samples != VK_SAMPLE_COUNT_1_BIT) {
            char const str[] = "vkCmdResolveImage called with dest sample count greater than 1.";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_node->commandBuffer), "VUID-vkCmdResolveImage-dstImage-00259", "%s.", str);
        }
    } else {
        assert(0);
    }
    return skip;
}

void PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                  VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve *pRegions) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    auto cb_node = GetCBNode(device_data, commandBuffer);
    auto src_image_state = GetImageState(device_data, srcImage);
    auto dst_image_state = GetImageState(device_data, dstImage);

    // Update bindings between images and cmd buffer
    AddCommandBufferBindingImage(device_data, cb_node, src_image_state);
    AddCommandBufferBindingImage(device_data, cb_node, dst_image_state);
}

bool PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                 VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit *pRegions, VkFilter filter) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    auto cb_node = GetCBNode(device_data, commandBuffer);
    auto src_image_state = GetImageState(device_data, srcImage);
    auto dst_image_state = GetImageState(device_data, dstImage);
    const debug_report_data *report_data = device_data->report_data;

    bool skip = false;
    if (cb_node) {
        skip |= ValidateCmd(device_data, cb_node, CMD_BLITIMAGE, "vkCmdBlitImage()");
    }
    if (cb_node && src_image_state && dst_image_state) {
        skip |= ValidateImageSampleCount(device_data, src_image_state, VK_SAMPLE_COUNT_1_BIT, "vkCmdBlitImage(): srcImage",
                                         "VUID-vkCmdBlitImage-srcImage-00233");
        skip |= ValidateImageSampleCount(device_data, dst_image_state, VK_SAMPLE_COUNT_1_BIT, "vkCmdBlitImage(): dstImage",
                                         "VUID-vkCmdBlitImage-dstImage-00234");
        skip |=
            ValidateMemoryIsBoundToImage(device_data, src_image_state, "vkCmdBlitImage()", "VUID-vkCmdBlitImage-srcImage-00220");
        skip |=
            ValidateMemoryIsBoundToImage(device_data, dst_image_state, "vkCmdBlitImage()", "VUID-vkCmdBlitImage-dstImage-00225");
        skip |=
            ValidateImageUsageFlags(device_data, src_image_state, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, true,
                                    "VUID-vkCmdBlitImage-srcImage-00219", "vkCmdBlitImage()", "VK_IMAGE_USAGE_TRANSFER_SRC_BIT");
        skip |=
            ValidateImageUsageFlags(device_data, dst_image_state, VK_IMAGE_USAGE_TRANSFER_DST_BIT, true,
                                    "VUID-vkCmdBlitImage-dstImage-00224", "vkCmdBlitImage()", "VK_IMAGE_USAGE_TRANSFER_DST_BIT");
        skip |= ValidateCmdQueueFlags(device_data, cb_node, "vkCmdBlitImage()", VK_QUEUE_GRAPHICS_BIT,
                                      "VUID-vkCmdBlitImage-commandBuffer-cmdpool");
        skip |= ValidateCmd(device_data, cb_node, CMD_BLITIMAGE, "vkCmdBlitImage()");
        skip |= InsideRenderPass(device_data, cb_node, "vkCmdBlitImage()", "VUID-vkCmdBlitImage-renderpass");
        skip |= ValidateImageFormatFeatureFlags(device_data, src_image_state, VK_FORMAT_FEATURE_BLIT_SRC_BIT, "vkCmdBlitImage()",
                                                "VUID-vkCmdBlitImage-srcImage-01999", "VUID-vkCmdBlitImage-srcImage-01999");
        skip |= ValidateImageFormatFeatureFlags(device_data, dst_image_state, VK_FORMAT_FEATURE_BLIT_DST_BIT, "vkCmdBlitImage()",
                                                "VUID-vkCmdBlitImage-dstImage-02000", "VUID-vkCmdBlitImage-dstImage-02000");

        // TODO: Need to validate image layouts, which will include layout validation for shared presentable images

        VkFormat src_format = src_image_state->createInfo.format;
        VkFormat dst_format = dst_image_state->createInfo.format;
        VkImageType src_type = src_image_state->createInfo.imageType;
        VkImageType dst_type = dst_image_state->createInfo.imageType;

        if (VK_FILTER_LINEAR == filter) {
            skip |= ValidateImageFormatFeatureFlags(device_data, src_image_state, VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT,
                                                    "vkCmdBlitImage()", "VUID-vkCmdBlitImage-filter-02001",
                                                    "VUID-vkCmdBlitImage-filter-02001");
        } else if (VK_FILTER_CUBIC_IMG == filter) {
            skip |= ValidateImageFormatFeatureFlags(device_data, src_image_state,
                                                    VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG, "vkCmdBlitImage()",
                                                    "VUID-vkCmdBlitImage-filter-02002", "VUID-vkCmdBlitImage-filter-02002");
        }

        if ((VK_FILTER_CUBIC_IMG == filter) && (VK_IMAGE_TYPE_3D != src_type)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_node->commandBuffer), "VUID-vkCmdBlitImage-filter-00237",
                            "vkCmdBlitImage(): source image type must be VK_IMAGE_TYPE_3D when cubic filtering is specified.");
        }

        if ((VK_SAMPLE_COUNT_1_BIT != src_image_state->createInfo.samples) ||
            (VK_SAMPLE_COUNT_1_BIT != dst_image_state->createInfo.samples)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_node->commandBuffer), "VUID-vkCmdBlitImage-srcImage-00228",
                            "vkCmdBlitImage(): source or dest image has sample count other than VK_SAMPLE_COUNT_1_BIT.");
        }

        // Validate consistency for unsigned formats
        if (FormatIsUInt(src_format) != FormatIsUInt(dst_format)) {
            std::stringstream ss;
            ss << "vkCmdBlitImage(): If one of srcImage and dstImage images has unsigned integer format, "
               << "the other one must also have unsigned integer format.  "
               << "Source format is " << string_VkFormat(src_format) << " Destination format is " << string_VkFormat(dst_format);
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_node->commandBuffer), "VUID-vkCmdBlitImage-srcImage-00230", "%s.", ss.str().c_str());
        }

        // Validate consistency for signed formats
        if (FormatIsSInt(src_format) != FormatIsSInt(dst_format)) {
            std::stringstream ss;
            ss << "vkCmdBlitImage(): If one of srcImage and dstImage images has signed integer format, "
               << "the other one must also have signed integer format.  "
               << "Source format is " << string_VkFormat(src_format) << " Destination format is " << string_VkFormat(dst_format);
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_node->commandBuffer), "VUID-vkCmdBlitImage-srcImage-00229", "%s.", ss.str().c_str());
        }

        // Validate filter for Depth/Stencil formats
        if (FormatIsDepthOrStencil(src_format) && (filter != VK_FILTER_NEAREST)) {
            std::stringstream ss;
            ss << "vkCmdBlitImage(): If the format of srcImage is a depth, stencil, or depth stencil "
               << "then filter must be VK_FILTER_NEAREST.";
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_node->commandBuffer), "VUID-vkCmdBlitImage-srcImage-00232", "%s.", ss.str().c_str());
        }

        // Validate aspect bits and formats for depth/stencil images
        if (FormatIsDepthOrStencil(src_format) || FormatIsDepthOrStencil(dst_format)) {
            if (src_format != dst_format) {
                std::stringstream ss;
                ss << "vkCmdBlitImage(): If one of srcImage and dstImage images has a format of depth, stencil or depth "
                   << "stencil, the other one must have exactly the same format.  "
                   << "Source format is " << string_VkFormat(src_format) << " Destination format is "
                   << string_VkFormat(dst_format);
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_node->commandBuffer), "VUID-vkCmdBlitImage-srcImage-00231", "%s.", ss.str().c_str());
            }
        }  // Depth or Stencil

        // Do per-region checks
        const std::string invalid_src_layout_vuid =
            (src_image_state->shared_presentable &&
             core_validation::GetDeviceExtensions(device_data)->vk_khr_shared_presentable_image)
                ? "VUID-vkCmdBlitImage-srcImageLayout-01398"
                : "VUID-vkCmdBlitImage-srcImageLayout-00222";
        const std::string invalid_dst_layout_vuid =
            (dst_image_state->shared_presentable &&
             core_validation::GetDeviceExtensions(device_data)->vk_khr_shared_presentable_image)
                ? "VUID-vkCmdBlitImage-dstImageLayout-01399"
                : "VUID-vkCmdBlitImage-dstImageLayout-00227";
        for (uint32_t i = 0; i < regionCount; i++) {
            const VkImageBlit rgn = pRegions[i];
            bool hit_error = false;
            skip |= VerifyImageLayout(device_data, cb_node, src_image_state, rgn.srcSubresource, srcImageLayout,
                                      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, "vkCmdBlitImage()", invalid_src_layout_vuid,
                                      "VUID-vkCmdBlitImage-srcImageLayout-00221", &hit_error);
            skip |= VerifyImageLayout(device_data, cb_node, dst_image_state, rgn.dstSubresource, dstImageLayout,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, "vkCmdBlitImage()", invalid_dst_layout_vuid,
                                      "VUID-vkCmdBlitImage-dstImageLayout-00226", &hit_error);
            skip |=
                ValidateImageSubresourceLayers(device_data, cb_node, &rgn.srcSubresource, "vkCmdBlitImage()", "srcSubresource", i);
            skip |=
                ValidateImageSubresourceLayers(device_data, cb_node, &rgn.dstSubresource, "vkCmdBlitImage()", "dstSubresource", i);
            skip |= ValidateImageMipLevel(device_data, cb_node, src_image_state, rgn.srcSubresource.mipLevel, i, "vkCmdBlitImage()",
                                          "srcSubresource", "VUID-vkCmdBlitImage-srcSubresource-01705");
            skip |= ValidateImageMipLevel(device_data, cb_node, dst_image_state, rgn.dstSubresource.mipLevel, i, "vkCmdBlitImage()",
                                          "dstSubresource", "VUID-vkCmdBlitImage-dstSubresource-01706");
            skip |= ValidateImageArrayLayerRange(device_data, cb_node, src_image_state, rgn.srcSubresource.baseArrayLayer,
                                                 rgn.srcSubresource.layerCount, i, "vkCmdBlitImage()", "srcSubresource",
                                                 "VUID-vkCmdBlitImage-srcSubresource-01707");
            skip |= ValidateImageArrayLayerRange(device_data, cb_node, dst_image_state, rgn.dstSubresource.baseArrayLayer,
                                                 rgn.dstSubresource.layerCount, i, "vkCmdBlitImage()", "dstSubresource",
                                                 "VUID-vkCmdBlitImage-dstSubresource-01708");
            // Warn for zero-sized regions
            if ((rgn.srcOffsets[0].x == rgn.srcOffsets[1].x) || (rgn.srcOffsets[0].y == rgn.srcOffsets[1].y) ||
                (rgn.srcOffsets[0].z == rgn.srcOffsets[1].z)) {
                std::stringstream ss;
                ss << "vkCmdBlitImage(): pRegions[" << i << "].srcOffsets specify a zero-volume area.";
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_node->commandBuffer), kVUID_Core_DrawState_InvalidExtents, "%s", ss.str().c_str());
            }
            if ((rgn.dstOffsets[0].x == rgn.dstOffsets[1].x) || (rgn.dstOffsets[0].y == rgn.dstOffsets[1].y) ||
                (rgn.dstOffsets[0].z == rgn.dstOffsets[1].z)) {
                std::stringstream ss;
                ss << "vkCmdBlitImage(): pRegions[" << i << "].dstOffsets specify a zero-volume area.";
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            HandleToUint64(cb_node->commandBuffer), kVUID_Core_DrawState_InvalidExtents, "%s", ss.str().c_str());
            }

            // Check that src/dst layercounts match
            if (rgn.srcSubresource.layerCount != rgn.dstSubresource.layerCount) {
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                    HandleToUint64(cb_node->commandBuffer), "VUID-VkImageBlit-layerCount-00239",
                    "vkCmdBlitImage(): layerCount in source and destination subresource of pRegions[%d] does not match.", i);
            }

            if (rgn.srcSubresource.aspectMask != rgn.dstSubresource.aspectMask) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(cb_node->commandBuffer), "VUID-VkImageBlit-aspectMask-00238",
                                "vkCmdBlitImage(): aspectMask members for pRegion[%d] do not match.", i);
            }

            if (!VerifyAspectsPresent(rgn.srcSubresource.aspectMask, src_format)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(cb_node->commandBuffer), "VUID-VkImageBlit-aspectMask-00241",
                                "vkCmdBlitImage(): region [%d] source aspectMask (0x%x) specifies aspects not present in source "
                                "image format %s.",
                                i, rgn.srcSubresource.aspectMask, string_VkFormat(src_format));
            }

            if (!VerifyAspectsPresent(rgn.dstSubresource.aspectMask, dst_format)) {
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                    HandleToUint64(cb_node->commandBuffer), "VUID-VkImageBlit-aspectMask-00242",
                    "vkCmdBlitImage(): region [%d] dest aspectMask (0x%x) specifies aspects not present in dest image format %s.",
                    i, rgn.dstSubresource.aspectMask, string_VkFormat(dst_format));
            }

            // Validate source image offsets
            VkExtent3D src_extent = GetImageSubresourceExtent(src_image_state, &(rgn.srcSubresource));
            if (VK_IMAGE_TYPE_1D == src_type) {
                if ((0 != rgn.srcOffsets[0].y) || (1 != rgn.srcOffsets[1].y)) {
                    skip |=
                        log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(cb_node->commandBuffer), "VUID-VkImageBlit-srcImage-00245",
                                "vkCmdBlitImage(): region [%d], source image of type VK_IMAGE_TYPE_1D with srcOffset[].y values "
                                "of (%1d, %1d). These must be (0, 1).",
                                i, rgn.srcOffsets[0].y, rgn.srcOffsets[1].y);
                }
            }

            if ((VK_IMAGE_TYPE_1D == src_type) || (VK_IMAGE_TYPE_2D == src_type)) {
                if ((0 != rgn.srcOffsets[0].z) || (1 != rgn.srcOffsets[1].z)) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                    HandleToUint64(cb_node->commandBuffer), "VUID-VkImageBlit-srcImage-00247",
                                    "vkCmdBlitImage(): region [%d], source image of type VK_IMAGE_TYPE_1D or VK_IMAGE_TYPE_2D with "
                                    "srcOffset[].z values of (%1d, %1d). These must be (0, 1).",
                                    i, rgn.srcOffsets[0].z, rgn.srcOffsets[1].z);
                }
            }

            bool oob = false;
            if ((rgn.srcOffsets[0].x < 0) || (rgn.srcOffsets[0].x > static_cast<int32_t>(src_extent.width)) ||
                (rgn.srcOffsets[1].x < 0) || (rgn.srcOffsets[1].x > static_cast<int32_t>(src_extent.width))) {
                oob = true;
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                    HandleToUint64(cb_node->commandBuffer), "VUID-VkImageBlit-srcOffset-00243",
                    "vkCmdBlitImage(): region [%d] srcOffset[].x values (%1d, %1d) exceed srcSubresource width extent (%1d).", i,
                    rgn.srcOffsets[0].x, rgn.srcOffsets[1].x, src_extent.width);
            }
            if ((rgn.srcOffsets[0].y < 0) || (rgn.srcOffsets[0].y > static_cast<int32_t>(src_extent.height)) ||
                (rgn.srcOffsets[1].y < 0) || (rgn.srcOffsets[1].y > static_cast<int32_t>(src_extent.height))) {
                oob = true;
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                    HandleToUint64(cb_node->commandBuffer), "VUID-VkImageBlit-srcOffset-00244",
                    "vkCmdBlitImage(): region [%d] srcOffset[].y values (%1d, %1d) exceed srcSubresource height extent (%1d).", i,
                    rgn.srcOffsets[0].y, rgn.srcOffsets[1].y, src_extent.height);
            }
            if ((rgn.srcOffsets[0].z < 0) || (rgn.srcOffsets[0].z > static_cast<int32_t>(src_extent.depth)) ||
                (rgn.srcOffsets[1].z < 0) || (rgn.srcOffsets[1].z > static_cast<int32_t>(src_extent.depth))) {
                oob = true;
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                    HandleToUint64(cb_node->commandBuffer), "VUID-VkImageBlit-srcOffset-00246",
                    "vkCmdBlitImage(): region [%d] srcOffset[].z values (%1d, %1d) exceed srcSubresource depth extent (%1d).", i,
                    rgn.srcOffsets[0].z, rgn.srcOffsets[1].z, src_extent.depth);
            }
            if (oob) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(cb_node->commandBuffer), "VUID-vkCmdBlitImage-pRegions-00215",
                                "vkCmdBlitImage(): region [%d] source image blit region exceeds image dimensions.", i);
            }

            // Validate dest image offsets
            VkExtent3D dst_extent = GetImageSubresourceExtent(dst_image_state, &(rgn.dstSubresource));
            if (VK_IMAGE_TYPE_1D == dst_type) {
                if ((0 != rgn.dstOffsets[0].y) || (1 != rgn.dstOffsets[1].y)) {
                    skip |=
                        log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(cb_node->commandBuffer), "VUID-VkImageBlit-dstImage-00250",
                                "vkCmdBlitImage(): region [%d], dest image of type VK_IMAGE_TYPE_1D with dstOffset[].y values of "
                                "(%1d, %1d). These must be (0, 1).",
                                i, rgn.dstOffsets[0].y, rgn.dstOffsets[1].y);
                }
            }

            if ((VK_IMAGE_TYPE_1D == dst_type) || (VK_IMAGE_TYPE_2D == dst_type)) {
                if ((0 != rgn.dstOffsets[0].z) || (1 != rgn.dstOffsets[1].z)) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                    HandleToUint64(cb_node->commandBuffer), "VUID-VkImageBlit-dstImage-00252",
                                    "vkCmdBlitImage(): region [%d], dest image of type VK_IMAGE_TYPE_1D or VK_IMAGE_TYPE_2D with "
                                    "dstOffset[].z values of (%1d, %1d). These must be (0, 1).",
                                    i, rgn.dstOffsets[0].z, rgn.dstOffsets[1].z);
                }
            }

            oob = false;
            if ((rgn.dstOffsets[0].x < 0) || (rgn.dstOffsets[0].x > static_cast<int32_t>(dst_extent.width)) ||
                (rgn.dstOffsets[1].x < 0) || (rgn.dstOffsets[1].x > static_cast<int32_t>(dst_extent.width))) {
                oob = true;
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                    HandleToUint64(cb_node->commandBuffer), "VUID-VkImageBlit-dstOffset-00248",
                    "vkCmdBlitImage(): region [%d] dstOffset[].x values (%1d, %1d) exceed dstSubresource width extent (%1d).", i,
                    rgn.dstOffsets[0].x, rgn.dstOffsets[1].x, dst_extent.width);
            }
            if ((rgn.dstOffsets[0].y < 0) || (rgn.dstOffsets[0].y > static_cast<int32_t>(dst_extent.height)) ||
                (rgn.dstOffsets[1].y < 0) || (rgn.dstOffsets[1].y > static_cast<int32_t>(dst_extent.height))) {
                oob = true;
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                    HandleToUint64(cb_node->commandBuffer), "VUID-VkImageBlit-dstOffset-00249",
                    "vkCmdBlitImage(): region [%d] dstOffset[].y values (%1d, %1d) exceed dstSubresource height extent (%1d).", i,
                    rgn.dstOffsets[0].y, rgn.dstOffsets[1].y, dst_extent.height);
            }
            if ((rgn.dstOffsets[0].z < 0) || (rgn.dstOffsets[0].z > static_cast<int32_t>(dst_extent.depth)) ||
                (rgn.dstOffsets[1].z < 0) || (rgn.dstOffsets[1].z > static_cast<int32_t>(dst_extent.depth))) {
                oob = true;
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                    HandleToUint64(cb_node->commandBuffer), "VUID-VkImageBlit-dstOffset-00251",
                    "vkCmdBlitImage(): region [%d] dstOffset[].z values (%1d, %1d) exceed dstSubresource depth extent (%1d).", i,
                    rgn.dstOffsets[0].z, rgn.dstOffsets[1].z, dst_extent.depth);
            }
            if (oob) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(cb_node->commandBuffer), "VUID-vkCmdBlitImage-pRegions-00216",
                                "vkCmdBlitImage(): region [%d] destination image blit region exceeds image dimensions.", i);
            }

            if ((VK_IMAGE_TYPE_3D == src_type) || (VK_IMAGE_TYPE_3D == dst_type)) {
                if ((0 != rgn.srcSubresource.baseArrayLayer) || (1 != rgn.srcSubresource.layerCount) ||
                    (0 != rgn.dstSubresource.baseArrayLayer) || (1 != rgn.dstSubresource.layerCount)) {
                    skip |=
                        log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(cb_node->commandBuffer), "VUID-VkImageBlit-srcImage-00240",
                                "vkCmdBlitImage(): region [%d] blit to/from a 3D image type with a non-zero baseArrayLayer, or a "
                                "layerCount other than 1.",
                                i);
                }
            }
        }  // per-region checks
    } else {
        assert(0);
    }
    return skip;
}

void PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                               VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit *pRegions, VkFilter filter) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    auto cb_node = GetCBNode(device_data, commandBuffer);
    auto src_image_state = GetImageState(device_data, srcImage);
    auto dst_image_state = GetImageState(device_data, dstImage);

    // Make sure that all image slices are updated to correct layout
    for (uint32_t i = 0; i < regionCount; ++i) {
        SetImageLayout(device_data, cb_node, src_image_state, pRegions[i].srcSubresource, srcImageLayout);
        SetImageLayout(device_data, cb_node, dst_image_state, pRegions[i].dstSubresource, dstImageLayout);
    }
    // Update bindings between images and cmd buffer
    AddCommandBufferBindingImage(device_data, cb_node, src_image_state);
    AddCommandBufferBindingImage(device_data, cb_node, dst_image_state);
}

// This validates that the initial layout specified in the command buffer for the IMAGE is the same as the global IMAGE layout
bool ValidateCmdBufImageLayouts(layer_data *device_data, GLOBAL_CB_NODE *pCB,
                                std::unordered_map<ImageSubresourcePair, IMAGE_LAYOUT_NODE> const &globalImageLayoutMap,
                                std::unordered_map<ImageSubresourcePair, IMAGE_LAYOUT_NODE> &overlayLayoutMap) {
    bool skip = false;
    const debug_report_data *report_data = core_validation::GetReportData(device_data);
    for (auto cb_image_data : pCB->imageLayoutMap) {
        VkImageLayout imageLayout;

        if (FindLayout(device_data, overlayLayoutMap, cb_image_data.first, imageLayout) ||
            FindLayout(device_data, globalImageLayoutMap, cb_image_data.first, imageLayout)) {
            if (cb_image_data.second.initialLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
                // TODO: Set memory invalid which is in mem_tracker currently
            } else if (imageLayout != cb_image_data.second.initialLayout) {
                if (cb_image_data.first.hasSubresource) {
                    skip |= log_msg(
                        report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(pCB->commandBuffer), kVUID_Core_DrawState_InvalidImageLayout,
                        "Submitted command buffer expects image 0x%" PRIx64
                        " (subresource: aspectMask 0x%X array layer %u, mip level %u) to be in layout %s--instead, image 0x%" PRIx64
                        "'s current layout is %s.",
                        HandleToUint64(cb_image_data.first.image), cb_image_data.first.subresource.aspectMask,
                        cb_image_data.first.subresource.arrayLayer, cb_image_data.first.subresource.mipLevel,
                        string_VkImageLayout(cb_image_data.second.initialLayout), HandleToUint64(cb_image_data.first.image),
                        string_VkImageLayout(imageLayout));
                } else {
                    skip |=
                        log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                HandleToUint64(pCB->commandBuffer), kVUID_Core_DrawState_InvalidImageLayout,
                                "Submitted command buffer expects image 0x%" PRIx64 " to be in layout %s--instead, image 0x%" PRIx64
                                "'s current layout is %s.",
                                HandleToUint64(cb_image_data.first.image), string_VkImageLayout(cb_image_data.second.initialLayout),
                                HandleToUint64(cb_image_data.first.image), string_VkImageLayout(imageLayout));
                }
            }
            SetLayout(overlayLayoutMap, cb_image_data.first, cb_image_data.second.layout);
        }
    }
    return skip;
}

void UpdateCmdBufImageLayouts(layer_data *device_data, GLOBAL_CB_NODE *pCB) {
    for (auto cb_image_data : pCB->imageLayoutMap) {
        VkImageLayout imageLayout;
        FindGlobalLayout(device_data, cb_image_data.first, imageLayout);
        SetGlobalLayout(device_data, cb_image_data.first, cb_image_data.second.layout);
    }
}

// ValidateLayoutVsAttachmentDescription is a general function where we can validate various state associated with the
// VkAttachmentDescription structs that are used by the sub-passes of a renderpass. Initial check is to make sure that READ_ONLY
// layout attachments don't have CLEAR as their loadOp.
bool ValidateLayoutVsAttachmentDescription(const debug_report_data *report_data, RenderPassCreateVersion rp_version,
                                           const VkImageLayout first_layout, const uint32_t attachment,
                                           const VkAttachmentDescription2KHR &attachment_description) {
    bool skip = false;
    const char *vuid;
    const bool use_rp2 = (rp_version == RENDER_PASS_VERSION_2);

    // Verify that initial loadOp on READ_ONLY attachments is not CLEAR
    if (attachment_description.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
        if (use_rp2 && ((first_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) ||
                        (first_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) ||
                        (first_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL))) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkRenderPassCreateInfo2KHR-pAttachments-02522",
                        "Cannot clear attachment %d with invalid first layout %s.", attachment, string_VkImageLayout(first_layout));
        } else if (!use_rp2 && ((first_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) ||
                                (first_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkRenderPassCreateInfo-pAttachments-00836",
                        "Cannot clear attachment %d with invalid first layout %s.", attachment, string_VkImageLayout(first_layout));
        }
    }
    if (attachment_description.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
        if (first_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) {
            vuid = use_rp2 ? kVUID_Core_DrawState_InvalidRenderpass : "VUID-VkRenderPassCreateInfo-pAttachments-01566";
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                        "Cannot clear attachment %d with invalid first layout %s.", attachment, string_VkImageLayout(first_layout));
        }
    }

    if (attachment_description.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
        if (first_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL) {
            vuid = use_rp2 ? kVUID_Core_DrawState_InvalidRenderpass : "VUID-VkRenderPassCreateInfo-pAttachments-01567";
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                        "Cannot clear attachment %d with invalid first layout %s.", attachment, string_VkImageLayout(first_layout));
        }
    }
    return skip;
}

bool ValidateLayouts(const core_validation::layer_data *device_data, RenderPassCreateVersion rp_version, VkDevice device,
                     const VkRenderPassCreateInfo2KHR *pCreateInfo) {
    const debug_report_data *report_data = core_validation::GetReportData(device_data);
    bool skip = false;
    const char *vuid;
    const bool use_rp2 = (rp_version == RENDER_PASS_VERSION_2);
    const char *const function_name = use_rp2 ? "vkCreateRenderPass2KHR()" : "vkCreateRenderPass()";

    for (uint32_t i = 0; i < pCreateInfo->attachmentCount; ++i) {
        VkFormat format = pCreateInfo->pAttachments[i].format;
        if (pCreateInfo->pAttachments[i].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
            if ((FormatIsColor(format) || FormatHasDepth(format)) &&
                pCreateInfo->pAttachments[i].loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                kVUID_Core_DrawState_InvalidRenderpass,
                                "Render pass has an attachment with loadOp == VK_ATTACHMENT_LOAD_OP_LOAD and initialLayout == "
                                "VK_IMAGE_LAYOUT_UNDEFINED.  This is probably not what you intended.  Consider using "
                                "VK_ATTACHMENT_LOAD_OP_DONT_CARE instead if the image truely is undefined at the start of the "
                                "render pass.");
            }
            if (FormatHasStencil(format) && pCreateInfo->pAttachments[i].stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                kVUID_Core_DrawState_InvalidRenderpass,
                                "Render pass has an attachment with stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD and initialLayout "
                                "== VK_IMAGE_LAYOUT_UNDEFINED.  This is probably not what you intended.  Consider using "
                                "VK_ATTACHMENT_LOAD_OP_DONT_CARE instead if the image truely is undefined at the start of the "
                                "render pass.");
            }
        }
    }

    // Track when we're observing the first use of an attachment
    std::vector<bool> attach_first_use(pCreateInfo->attachmentCount, true);
    for (uint32_t i = 0; i < pCreateInfo->subpassCount; ++i) {
        const VkSubpassDescription2KHR &subpass = pCreateInfo->pSubpasses[i];

        // Check input attachments first, so we can detect first-use-as-input for VU #00349
        for (uint32_t j = 0; j < subpass.inputAttachmentCount; ++j) {
            auto attach_index = subpass.pInputAttachments[j].attachment;
            if (attach_index == VK_ATTACHMENT_UNUSED) continue;
            switch (subpass.pInputAttachments[j].layout) {
                case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
                case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                    // These are ideal.
                    break;

                case VK_IMAGE_LAYOUT_GENERAL:
                    // May not be optimal. TODO: reconsider this warning based on other constraints.
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, kVUID_Core_DrawState_InvalidImageLayout,
                                    "Layout for input attachment is GENERAL but should be READ_ONLY_OPTIMAL.");
                    break;

                case VK_IMAGE_LAYOUT_UNDEFINED:
                case VK_IMAGE_LAYOUT_PREINITIALIZED:
                    vuid = use_rp2 ? "VUID-VkAttachmentReference2KHR-layout-03077" : "VUID-VkAttachmentReference-layout-00857";
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                                    "Layout for input attachment reference %u in subpass %u is %s but must be "
                                    "DEPTH_STENCIL_READ_ONLY, SHADER_READ_ONLY_OPTIMAL, or GENERAL.",
                                    j, i, string_VkImageLayout(subpass.pDepthStencilAttachment->layout));
                    break;

                case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR:
                case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR:
                    if (GetDeviceExtensions(device_data)->vk_khr_maintenance2) {
                        break;
                    } else {
                        // Intentionally fall through to generic error message
                    }
                    // fall through

                default:
                    // No other layouts are acceptable
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    kVUID_Core_DrawState_InvalidImageLayout,
                                    "Layout for input attachment is %s but can only be READ_ONLY_OPTIMAL or GENERAL.",
                                    string_VkImageLayout(subpass.pInputAttachments[j].layout));
            }

            if (attach_first_use[attach_index]) {
                skip |= ValidateLayoutVsAttachmentDescription(report_data, rp_version, subpass.pInputAttachments[j].layout,
                                                              attach_index, pCreateInfo->pAttachments[attach_index]);

                bool used_as_depth =
                    (subpass.pDepthStencilAttachment != NULL && subpass.pDepthStencilAttachment->attachment == attach_index);
                bool used_as_color = false;
                for (uint32_t k = 0; !used_as_depth && !used_as_color && k < subpass.colorAttachmentCount; ++k) {
                    used_as_color = (subpass.pColorAttachments[k].attachment == attach_index);
                }
                if (!used_as_depth && !used_as_color &&
                    pCreateInfo->pAttachments[attach_index].loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
                    vuid = use_rp2 ? "VUID-VkSubpassDescription2KHR-loadOp-03064" : "VUID-VkSubpassDescription-loadOp-00846";
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                                    "%s: attachment %u is first used as an input attachment in subpass %u with loadOp=CLEAR.",
                                    function_name, attach_index, attach_index);
                }
            }
            attach_first_use[attach_index] = false;
        }

        for (uint32_t j = 0; j < subpass.colorAttachmentCount; ++j) {
            auto attach_index = subpass.pColorAttachments[j].attachment;
            if (attach_index == VK_ATTACHMENT_UNUSED) continue;

            // TODO: Need a way to validate shared presentable images here, currently just allowing
            // VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR
            //  as an acceptable layout, but need to make sure shared presentable images ONLY use that layout
            switch (subpass.pColorAttachments[j].layout) {
                case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // This is ideal.
                case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
                    // TODO: See note above, just assuming that attachment is shared presentable and allowing this for now.
                    break;

                case VK_IMAGE_LAYOUT_GENERAL:
                    // May not be optimal; TODO: reconsider this warning based on other constraints?
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, kVUID_Core_DrawState_InvalidImageLayout,
                                    "Layout for color attachment is GENERAL but should be COLOR_ATTACHMENT_OPTIMAL.");
                    break;

                case VK_IMAGE_LAYOUT_UNDEFINED:
                case VK_IMAGE_LAYOUT_PREINITIALIZED:
                    vuid = use_rp2 ? "VUID-VkAttachmentReference2KHR-layout-03077" : "VUID-VkAttachmentReference-layout-00857";
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                                    "Layout for color attachment reference %u in subpass %u is %s but should be "
                                    "COLOR_ATTACHMENT_OPTIMAL or GENERAL.",
                                    j, i, string_VkImageLayout(subpass.pColorAttachments[j].layout));
                    break;

                default:
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    kVUID_Core_DrawState_InvalidImageLayout,
                                    "Layout for color attachment is %s but can only be COLOR_ATTACHMENT_OPTIMAL or GENERAL.",
                                    string_VkImageLayout(subpass.pColorAttachments[j].layout));
            }

            if (subpass.pResolveAttachments && (subpass.pResolveAttachments[j].layout == VK_IMAGE_LAYOUT_UNDEFINED ||
                                                subpass.pResolveAttachments[j].layout == VK_IMAGE_LAYOUT_PREINITIALIZED)) {
                vuid = use_rp2 ? "VUID-VkAttachmentReference2KHR-layout-03077" : "VUID-VkAttachmentReference-layout-00857";
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                                "Layout for color attachment reference %u in subpass %u is %s but should be "
                                "COLOR_ATTACHMENT_OPTIMAL or GENERAL.",
                                j, i, string_VkImageLayout(subpass.pColorAttachments[j].layout));
            }

            if (attach_first_use[attach_index]) {
                skip |= ValidateLayoutVsAttachmentDescription(report_data, rp_version, subpass.pColorAttachments[j].layout,
                                                              attach_index, pCreateInfo->pAttachments[attach_index]);
            }
            attach_first_use[attach_index] = false;
        }

        if (subpass.pDepthStencilAttachment && subpass.pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
            switch (subpass.pDepthStencilAttachment->layout) {
                case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
                    // These are ideal.
                    break;

                case VK_IMAGE_LAYOUT_GENERAL:
                    // May not be optimal; TODO: reconsider this warning based on other constraints? GENERAL can be better than
                    // doing a bunch of transitions.
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, kVUID_Core_DrawState_InvalidImageLayout,
                                    "GENERAL layout for depth attachment may not give optimal performance.");
                    break;

                case VK_IMAGE_LAYOUT_UNDEFINED:
                case VK_IMAGE_LAYOUT_PREINITIALIZED:
                    vuid = use_rp2 ? "VUID-VkAttachmentReference2KHR-layout-03077" : "VUID-VkAttachmentReference-layout-00857";
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, vuid,
                                    "Layout for depth attachment reference in subpass %u is %s but must be a valid depth/stencil "
                                    "layout or GENERAL.",
                                    i, string_VkImageLayout(subpass.pDepthStencilAttachment->layout));
                    break;

                case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR:
                case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR:
                    if (GetDeviceExtensions(device_data)->vk_khr_maintenance2) {
                        break;
                    } else {
                        // Intentionally fall through to generic error message
                    }
                    // fall through

                default:
                    // No other layouts are acceptable
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    kVUID_Core_DrawState_InvalidImageLayout,
                                    "Layout for depth attachment is %s but can only be DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "
                                    "DEPTH_STENCIL_READ_ONLY_OPTIMAL or GENERAL.",
                                    string_VkImageLayout(subpass.pDepthStencilAttachment->layout));
            }

            auto attach_index = subpass.pDepthStencilAttachment->attachment;
            if (attach_first_use[attach_index]) {
                skip |= ValidateLayoutVsAttachmentDescription(report_data, rp_version, subpass.pDepthStencilAttachment->layout,
                                                              attach_index, pCreateInfo->pAttachments[attach_index]);
            }
            attach_first_use[attach_index] = false;
        }
    }
    return skip;
}

// For any image objects that overlap mapped memory, verify that their layouts are PREINIT or GENERAL
bool ValidateMapImageLayouts(core_validation::layer_data *device_data, VkDevice device, DEVICE_MEM_INFO const *mem_info,
                             VkDeviceSize offset, VkDeviceSize end_offset) {
    const debug_report_data *report_data = core_validation::GetReportData(device_data);
    bool skip = false;
    // Iterate over all bound image ranges and verify that for any that overlap the map ranges, the layouts are
    // VK_IMAGE_LAYOUT_PREINITIALIZED or VK_IMAGE_LAYOUT_GENERAL
    // TODO : This can be optimized if we store ranges based on starting address and early exit when we pass our range
    for (auto image_handle : mem_info->bound_images) {
        auto img_it = mem_info->bound_ranges.find(image_handle);
        if (img_it != mem_info->bound_ranges.end()) {
            if (RangesIntersect(device_data, &img_it->second, offset, end_offset)) {
                std::vector<VkImageLayout> layouts;
                if (FindLayouts(device_data, VkImage(image_handle), layouts)) {
                    for (auto layout : layouts) {
                        if (layout != VK_IMAGE_LAYOUT_PREINITIALIZED && layout != VK_IMAGE_LAYOUT_GENERAL) {
                            skip |=
                                log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT,
                                        HandleToUint64(mem_info->mem), kVUID_Core_DrawState_InvalidImageLayout,
                                        "Mapping an image with layout %s can result in undefined behavior if this memory is used "
                                        "by the device. Only GENERAL or PREINITIALIZED should be used.",
                                        string_VkImageLayout(layout));
                        }
                    }
                }
            }
        }
    }
    return skip;
}

// Helper function to validate correct usage bits set for buffers or images. Verify that (actual & desired) flags != 0 or, if strict
// is true, verify that (actual & desired) flags == desired
static bool ValidateUsageFlags(const layer_data *device_data, VkFlags actual, VkFlags desired, VkBool32 strict, uint64_t obj_handle,
                               VulkanObjectType obj_type, std::string msgCode, char const *func_name, char const *usage_str) {
    const debug_report_data *report_data = core_validation::GetReportData(device_data);

    bool correct_usage = false;
    bool skip = false;
    const char *type_str = object_string[obj_type];
    if (strict) {
        correct_usage = ((actual & desired) == desired);
    } else {
        correct_usage = ((actual & desired) != 0);
    }
    if (!correct_usage) {
        if (msgCode == kVUIDUndefined) {
            // TODO: Fix callers with kVUIDUndefined to use correct validation checks.
            skip =
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, get_debug_report_enum[obj_type], obj_handle,
                        kVUID_Core_MemTrack_InvalidUsageFlag,
                        "Invalid usage flag for %s 0x%" PRIx64 " used by %s. In this case, %s should have %s set during creation.",
                        type_str, obj_handle, func_name, type_str, usage_str);
        } else {
            skip =
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, get_debug_report_enum[obj_type], obj_handle, msgCode,
                        "Invalid usage flag for %s 0x%" PRIx64 " used by %s. In this case, %s should have %s set during creation.",
                        type_str, obj_handle, func_name, type_str, usage_str);
        }
    }
    return skip;
}

// Helper function to validate usage flags for buffers. For given buffer_state send actual vs. desired usage off to helper above
// where an error will be flagged if usage is not correct
bool ValidateImageUsageFlags(layer_data *device_data, IMAGE_STATE const *image_state, VkFlags desired, bool strict,
                             const std::string &msgCode, char const *func_name, char const *usage_string) {
    return ValidateUsageFlags(device_data, image_state->createInfo.usage, desired, strict, HandleToUint64(image_state->image),
                              kVulkanObjectTypeImage, msgCode, func_name, usage_string);
}

bool ValidateImageFormatFeatureFlags(layer_data *dev_data, IMAGE_STATE const *image_state, VkFormatFeatureFlags desired,
                                     char const *func_name, const std::string &linear_vuid, const std::string &optimal_vuid) {
    VkFormatProperties format_properties = GetPDFormatProperties(dev_data, image_state->createInfo.format);
    const debug_report_data *report_data = core_validation::GetReportData(dev_data);
    bool skip = false;
    if (image_state->createInfo.tiling == VK_IMAGE_TILING_LINEAR) {
        if ((format_properties.linearTilingFeatures & desired) != desired) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                        HandleToUint64(image_state->image), linear_vuid,
                        "In %s, invalid linearTilingFeatures (0x%08X) for format %u used by image %" PRIx64 ".", func_name,
                        format_properties.linearTilingFeatures, image_state->createInfo.format, HandleToUint64(image_state->image));
        }
    } else if (image_state->createInfo.tiling == VK_IMAGE_TILING_OPTIMAL) {
        if ((format_properties.optimalTilingFeatures & desired) != desired) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(image_state->image), optimal_vuid,
                            "In %s, invalid optimalTilingFeatures (0x%08X) for format %u used by image %" PRIx64 ".", func_name,
                            format_properties.optimalTilingFeatures, image_state->createInfo.format,
                            HandleToUint64(image_state->image));
        }
    }
    return skip;
}

bool ValidateImageSubresourceLayers(layer_data *dev_data, const GLOBAL_CB_NODE *cb_node,
                                    const VkImageSubresourceLayers *subresource_layers, char const *func_name, char const *member,
                                    uint32_t i) {
    bool skip = false;
    const debug_report_data *report_data = core_validation::GetReportData(dev_data);
    // layerCount must not be zero
    if (subresource_layers->layerCount == 0) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(cb_node->commandBuffer), "VUID-VkImageSubresourceLayers-layerCount-01700",
                        "In %s, pRegions[%u].%s.layerCount must not be zero.", func_name, i, member);
    }
    // aspectMask must not contain VK_IMAGE_ASPECT_METADATA_BIT
    if (subresource_layers->aspectMask & VK_IMAGE_ASPECT_METADATA_BIT) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(cb_node->commandBuffer), "VUID-VkImageSubresourceLayers-aspectMask-00168",
                        "In %s, pRegions[%u].%s.aspectMask has VK_IMAGE_ASPECT_METADATA_BIT set.", func_name, i, member);
    }
    // if aspectMask contains COLOR, it must not contain either DEPTH or STENCIL
    if ((subresource_layers->aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) &&
        (subresource_layers->aspectMask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(cb_node->commandBuffer), "VUID-VkImageSubresourceLayers-aspectMask-00167",
                        "In %s, pRegions[%u].%s.aspectMask has VK_IMAGE_ASPECT_COLOR_BIT and either VK_IMAGE_ASPECT_DEPTH_BIT or "
                        "VK_IMAGE_ASPECT_STENCIL_BIT set.",
                        func_name, i, member);
    }
    return skip;
}

// Helper function to validate usage flags for buffers. For given buffer_state send actual vs. desired usage off to helper above
// where an error will be flagged if usage is not correct
bool ValidateBufferUsageFlags(const layer_data *device_data, BUFFER_STATE const *buffer_state, VkFlags desired, bool strict,
                              const std::string &msgCode, char const *func_name, char const *usage_string) {
    return ValidateUsageFlags(device_data, buffer_state->createInfo.usage, desired, strict, HandleToUint64(buffer_state->buffer),
                              kVulkanObjectTypeBuffer, msgCode, func_name, usage_string);
}

bool ValidateBufferViewRange(const layer_data *device_data, const BUFFER_STATE *buffer_state,
                             const VkBufferViewCreateInfo *pCreateInfo, const VkPhysicalDeviceLimits *device_limits) {
    bool skip = false;
    const debug_report_data *report_data = core_validation::GetReportData(device_data);

    const VkDeviceSize &range = pCreateInfo->range;
    if (range != VK_WHOLE_SIZE) {
        // Range must be greater than 0
        if (range <= 0) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,
                            HandleToUint64(buffer_state->buffer), "VUID-VkBufferViewCreateInfo-range-00928",
                            "If VkBufferViewCreateInfo range (%" PRIuLEAST64
                            ") does not equal VK_WHOLE_SIZE, range must be greater than 0.",
                            range);
        }
        // Range must be a multiple of the element size of format
        const size_t format_size = FormatElementSize(pCreateInfo->format);
        if (range % format_size != 0) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,
                            HandleToUint64(buffer_state->buffer), "VUID-VkBufferViewCreateInfo-range-00929",
                            "If VkBufferViewCreateInfo range (%" PRIuLEAST64
                            ") does not equal VK_WHOLE_SIZE, range must be a multiple of the element size of the format "
                            "(" PRINTF_SIZE_T_SPECIFIER ").",
                            range, format_size);
        }
        // Range divided by the element size of format must be less than or equal to VkPhysicalDeviceLimits::maxTexelBufferElements
        if (range / format_size > device_limits->maxTexelBufferElements) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,
                        HandleToUint64(buffer_state->buffer), "VUID-VkBufferViewCreateInfo-range-00930",
                        "If VkBufferViewCreateInfo range (%" PRIuLEAST64
                        ") does not equal VK_WHOLE_SIZE, range divided by the element size of the format (" PRINTF_SIZE_T_SPECIFIER
                        ") must be less than or equal to VkPhysicalDeviceLimits::maxTexelBufferElements (%" PRIuLEAST32 ").",
                        range, format_size, device_limits->maxTexelBufferElements);
        }
        // The sum of range and offset must be less than or equal to the size of buffer
        if (range + pCreateInfo->offset > buffer_state->createInfo.size) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,
                            HandleToUint64(buffer_state->buffer), "VUID-VkBufferViewCreateInfo-offset-00931",
                            "If VkBufferViewCreateInfo range (%" PRIuLEAST64
                            ") does not equal VK_WHOLE_SIZE, the sum of offset (%" PRIuLEAST64
                            ") and range must be less than or equal to the size of the buffer (%" PRIuLEAST64 ").",
                            range, pCreateInfo->offset, buffer_state->createInfo.size);
        }
    }
    return skip;
}

bool ValidateBufferViewBuffer(const layer_data *device_data, const BUFFER_STATE *buffer_state,
                              const VkBufferViewCreateInfo *pCreateInfo) {
    bool skip = false;
    const debug_report_data *report_data = GetReportData(device_data);
    const VkFormatProperties format_properties = GetPDFormatProperties(device_data, pCreateInfo->format);
    if ((buffer_state->createInfo.usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT) &&
        !(format_properties.bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,
                        HandleToUint64(buffer_state->buffer), "VUID-VkBufferViewCreateInfo-buffer-00933",
                        "If buffer was created with `usage` containing VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, format must "
                        "be supported for uniform texel buffers");
    }
    if ((buffer_state->createInfo.usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT) &&
        !(format_properties.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,
                        HandleToUint64(buffer_state->buffer), "VUID-VkBufferViewCreateInfo-buffer-00934",
                        "If buffer was created with `usage` containing VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, format must "
                        "be supported for storage texel buffers");
    }
    return skip;
}

bool PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                 VkBuffer *pBuffer) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    bool skip = false;
    const debug_report_data *report_data = device_data->report_data;

    // TODO: Add check for "VUID-vkCreateBuffer-flags-00911"        (sparse address space accounting)

    if ((pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_BINDING_BIT) && (!GetEnabledFeatures(device_data)->core.sparseBinding)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkBufferCreateInfo-flags-00915",
                        "vkCreateBuffer(): the sparseBinding device feature is disabled: Buffers cannot be created with the "
                        "VK_BUFFER_CREATE_SPARSE_BINDING_BIT set.");
    }

    if ((pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT) &&
        (!GetEnabledFeatures(device_data)->core.sparseResidencyBuffer)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkBufferCreateInfo-flags-00916",
                        "vkCreateBuffer(): the sparseResidencyBuffer device feature is disabled: Buffers cannot be created with "
                        "the VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT set.");
    }

    if ((pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_ALIASED_BIT) &&
        (!GetEnabledFeatures(device_data)->core.sparseResidencyAliased)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkBufferCreateInfo-flags-00917",
                        "vkCreateBuffer(): the sparseResidencyAliased device feature is disabled: Buffers cannot be created with "
                        "the VK_BUFFER_CREATE_SPARSE_ALIASED_BIT set.");
    }

    auto chained_devaddr_struct = lvl_find_in_chain<VkBufferDeviceAddressCreateInfoEXT>(pCreateInfo->pNext);
    if (chained_devaddr_struct) {
        if (!(pCreateInfo->flags & VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_EXT) &&
            chained_devaddr_struct->deviceAddress != 0) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkBufferCreateInfo-deviceAddress-02604",
                            "vkCreateBuffer(): Non-zero VkBufferDeviceAddressCreateInfoEXT::deviceAddress "
                            "requires VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_EXT.");
        }
    }

    if ((pCreateInfo->flags & VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_EXT) &&
        !GetEnabledFeatures(device_data)->buffer_address.bufferDeviceAddressCaptureReplay) {
        skip |= log_msg(
            report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
            "VUID-VkBufferCreateInfo-flags-02605",
            "vkCreateBuffer(): the bufferDeviceAddressCaptureReplay device feature is disabled: Buffers cannot be created with "
            "the VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_EXT set.");
    }

    if ((pCreateInfo->usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT) &&
        !GetEnabledFeatures(device_data)->buffer_address.bufferDeviceAddress) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        "VUID-VkBufferCreateInfo-usage-02606",
                        "vkCreateBuffer(): the bufferDeviceAddress device feature is disabled: Buffers cannot be created with "
                        "the VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT set.");
    }

    return skip;
}

void PostCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                VkBuffer *pBuffer, VkResult result) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (result != VK_SUCCESS) return;
    // TODO : This doesn't create deep copy of pQueueFamilyIndices so need to fix that if/when we want that data to be valid
    GetBufferMap(device_data)
        ->insert(std::make_pair(*pBuffer, std::unique_ptr<BUFFER_STATE>(new BUFFER_STATE(*pBuffer, pCreateInfo))));
}

bool PreCallValidateCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo,
                                     const VkAllocationCallbacks *pAllocator, VkBufferView *pView) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    bool skip = false;
    const debug_report_data *report_data = device_data->report_data;
    BUFFER_STATE *buffer_state = GetBufferState(device_data, pCreateInfo->buffer);
    // If this isn't a sparse buffer, it needs to have memory backing it at CreateBufferView time
    if (buffer_state) {
        skip |= ValidateMemoryIsBoundToBuffer(device_data, buffer_state, "vkCreateBufferView()",
                                              "VUID-VkBufferViewCreateInfo-buffer-00935");
        // In order to create a valid buffer view, the buffer must have been created with at least one of the following flags:
        // UNIFORM_TEXEL_BUFFER_BIT or STORAGE_TEXEL_BUFFER_BIT
        skip |= ValidateBufferUsageFlags(device_data, buffer_state,
                                         VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, false,
                                         "VUID-VkBufferViewCreateInfo-buffer-00932", "vkCreateBufferView()",
                                         "VK_BUFFER_USAGE_[STORAGE|UNIFORM]_TEXEL_BUFFER_BIT");

        // Buffer view offset must be less than the size of buffer
        if (pCreateInfo->offset >= buffer_state->createInfo.size) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,
                            HandleToUint64(buffer_state->buffer), "VUID-VkBufferViewCreateInfo-offset-00925",
                            "VkBufferViewCreateInfo offset (%" PRIuLEAST64
                            ") must be less than the size of the buffer (%" PRIuLEAST64 ").",
                            pCreateInfo->offset, buffer_state->createInfo.size);
        }

        const VkPhysicalDeviceLimits *device_limits = &(GetPDProperties(device_data)->limits);
        // Buffer view offset must be a multiple of VkPhysicalDeviceLimits::minTexelBufferOffsetAlignment
        if ((pCreateInfo->offset % device_limits->minTexelBufferOffsetAlignment) != 0) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,
                            HandleToUint64(buffer_state->buffer), "VUID-VkBufferViewCreateInfo-offset-00926",
                            "VkBufferViewCreateInfo offset (%" PRIuLEAST64
                            ") must be a multiple of VkPhysicalDeviceLimits::minTexelBufferOffsetAlignment (%" PRIuLEAST64 ").",
                            pCreateInfo->offset, device_limits->minTexelBufferOffsetAlignment);
        }

        skip |= ValidateBufferViewRange(device_data, buffer_state, pCreateInfo, device_limits);

        skip |= ValidateBufferViewBuffer(device_data, buffer_state, pCreateInfo);
    }
    return skip;
}

void PostCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo,
                                    const VkAllocationCallbacks *pAllocator, VkBufferView *pView, VkResult result) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (result != VK_SUCCESS) return;
    (*GetBufferViewMap(device_data))[*pView] = std::unique_ptr<BUFFER_VIEW_STATE>(new BUFFER_VIEW_STATE(*pView, pCreateInfo));
}

// For the given format verify that the aspect masks make sense
bool ValidateImageAspectMask(const layer_data *device_data, VkImage image, VkFormat format, VkImageAspectFlags aspect_mask,
                             const char *func_name, const char *vuid) {
    const debug_report_data *report_data = device_data->report_data;
    bool skip = false;
    VkDebugReportObjectTypeEXT objectType = VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
    if (image != VK_NULL_HANDLE) {
        objectType = VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT;
    }

    if (FormatIsColor(format)) {
        if ((aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) != VK_IMAGE_ASPECT_COLOR_BIT) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, objectType, HandleToUint64(image), vuid,
                            "%s: Color image formats must have the VK_IMAGE_ASPECT_COLOR_BIT set.", func_name);
        } else if ((aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) != aspect_mask) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, objectType, HandleToUint64(image), vuid,
                            "%s: Color image formats must have ONLY the VK_IMAGE_ASPECT_COLOR_BIT set.", func_name);
        }
    } else if (FormatIsDepthAndStencil(format)) {
        if ((aspect_mask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) == 0) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, objectType, HandleToUint64(image), vuid,
                            "%s: Depth/stencil image formats must have at least one of VK_IMAGE_ASPECT_DEPTH_BIT and "
                            "VK_IMAGE_ASPECT_STENCIL_BIT set.",
                            func_name);
        } else if ((aspect_mask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) != aspect_mask) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, objectType, HandleToUint64(image), vuid,
                            "%s: Combination depth/stencil image formats can have only the VK_IMAGE_ASPECT_DEPTH_BIT and "
                            "VK_IMAGE_ASPECT_STENCIL_BIT set.",
                            func_name);
        }
    } else if (FormatIsDepthOnly(format)) {
        if ((aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) != VK_IMAGE_ASPECT_DEPTH_BIT) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, objectType, HandleToUint64(image), vuid,
                            "%s: Depth-only image formats must have the VK_IMAGE_ASPECT_DEPTH_BIT set.", func_name);
        } else if ((aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) != aspect_mask) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, objectType, HandleToUint64(image), vuid,
                            "%s: Depth-only image formats can have only the VK_IMAGE_ASPECT_DEPTH_BIT set.", func_name);
        }
    } else if (FormatIsStencilOnly(format)) {
        if ((aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) != VK_IMAGE_ASPECT_STENCIL_BIT) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, objectType, HandleToUint64(image), vuid,
                            "%s: Stencil-only image formats must have the VK_IMAGE_ASPECT_STENCIL_BIT set.", func_name);
        } else if ((aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) != aspect_mask) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, objectType, HandleToUint64(image), vuid,
                            "%s: Stencil-only image formats can have only the VK_IMAGE_ASPECT_STENCIL_BIT set.", func_name);
        }
    } else if (FormatIsMultiplane(format)) {
        VkImageAspectFlags valid_flags = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT;
        if (3 == FormatPlaneCount(format)) {
            valid_flags = valid_flags | VK_IMAGE_ASPECT_PLANE_2_BIT;
        }
        if ((aspect_mask & valid_flags) != aspect_mask) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, objectType, HandleToUint64(image), vuid,
                            "%s: Multi-plane image formats may have only VK_IMAGE_ASPECT_COLOR_BIT or VK_IMAGE_ASPECT_PLANE_n_BITs "
                            "set, where n = [0, 1, 2].",
                            func_name);
        }
    }
    return skip;
}

struct SubresourceRangeErrorCodes {
    std::string base_mip_err, mip_count_err, base_layer_err, layer_count_err;
};

bool ValidateImageSubresourceRange(const layer_data *device_data, const uint32_t image_mip_count, const uint32_t image_layer_count,
                                   const VkImageSubresourceRange &subresourceRange, const char *cmd_name, const char *param_name,
                                   const char *image_layer_count_var_name, const uint64_t image_handle,
                                   SubresourceRangeErrorCodes errorCodes) {
    const debug_report_data *report_data = core_validation::GetReportData(device_data);
    bool skip = false;

    // Validate mip levels
    if (subresourceRange.baseMipLevel >= image_mip_count) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, image_handle,
                        errorCodes.base_mip_err,
                        "%s: %s.baseMipLevel (= %" PRIu32
                        ") is greater or equal to the mip level count of the image (i.e. greater or equal to %" PRIu32 ").",
                        cmd_name, param_name, subresourceRange.baseMipLevel, image_mip_count);
    }

    if (subresourceRange.levelCount != VK_REMAINING_MIP_LEVELS) {
        if (subresourceRange.levelCount == 0) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, image_handle,
                            errorCodes.mip_count_err, "%s: %s.levelCount is 0.", cmd_name, param_name);
        } else {
            const uint64_t necessary_mip_count = uint64_t{subresourceRange.baseMipLevel} + uint64_t{subresourceRange.levelCount};

            if (necessary_mip_count > image_mip_count) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, image_handle,
                                errorCodes.mip_count_err,
                                "%s: %s.baseMipLevel + .levelCount (= %" PRIu32 " + %" PRIu32 " = %" PRIu64
                                ") is greater than the mip level count of the image (i.e. greater than %" PRIu32 ").",
                                cmd_name, param_name, subresourceRange.baseMipLevel, subresourceRange.levelCount,
                                necessary_mip_count, image_mip_count);
            }
        }
    }

    // Validate array layers
    if (subresourceRange.baseArrayLayer >= image_layer_count) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, image_handle,
                        errorCodes.base_layer_err,
                        "%s: %s.baseArrayLayer (= %" PRIu32
                        ") is greater or equal to the %s of the image when it was created (i.e. greater or equal to %" PRIu32 ").",
                        cmd_name, param_name, subresourceRange.baseArrayLayer, image_layer_count_var_name, image_layer_count);
    }

    if (subresourceRange.layerCount != VK_REMAINING_ARRAY_LAYERS) {
        if (subresourceRange.layerCount == 0) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, image_handle,
                            errorCodes.layer_count_err, "%s: %s.layerCount is 0.", cmd_name, param_name);
        } else {
            const uint64_t necessary_layer_count =
                uint64_t{subresourceRange.baseArrayLayer} + uint64_t{subresourceRange.layerCount};

            if (necessary_layer_count > image_layer_count) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, image_handle,
                                errorCodes.layer_count_err,
                                "%s: %s.baseArrayLayer + .layerCount (= %" PRIu32 " + %" PRIu32 " = %" PRIu64
                                ") is greater than the %s of the image when it was created (i.e. greater than %" PRIu32 ").",
                                cmd_name, param_name, subresourceRange.baseArrayLayer, subresourceRange.layerCount,
                                necessary_layer_count, image_layer_count_var_name, image_layer_count);
            }
        }
    }

    return skip;
}

bool ValidateCreateImageViewSubresourceRange(const layer_data *device_data, const IMAGE_STATE *image_state,
                                             bool is_imageview_2d_type, const VkImageSubresourceRange &subresourceRange) {
    bool is_khr_maintenance1 = GetDeviceExtensions(device_data)->vk_khr_maintenance1;
    bool is_image_slicable = image_state->createInfo.imageType == VK_IMAGE_TYPE_3D &&
                             (image_state->createInfo.flags & VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR);
    bool is_3D_to_2D_map = is_khr_maintenance1 && is_image_slicable && is_imageview_2d_type;

    const auto image_layer_count = is_3D_to_2D_map ? image_state->createInfo.extent.depth : image_state->createInfo.arrayLayers;
    const auto image_layer_count_var_name = is_3D_to_2D_map ? "extent.depth" : "arrayLayers";

    SubresourceRangeErrorCodes subresourceRangeErrorCodes = {};
    subresourceRangeErrorCodes.base_mip_err = "VUID-VkImageViewCreateInfo-subresourceRange-01478";
    subresourceRangeErrorCodes.mip_count_err = "VUID-VkImageViewCreateInfo-subresourceRange-01718";
    subresourceRangeErrorCodes.base_layer_err = is_khr_maintenance1 ? (is_3D_to_2D_map ? "VUID-VkImageViewCreateInfo-image-01484"
                                                                                       : "VUID-VkImageViewCreateInfo-image-01482")
                                                                    : "VUID-VkImageViewCreateInfo-subresourceRange-01480";
    subresourceRangeErrorCodes.layer_count_err = is_khr_maintenance1
                                                     ? (is_3D_to_2D_map ? "VUID-VkImageViewCreateInfo-subresourceRange-01485"
                                                                        : "VUID-VkImageViewCreateInfo-subresourceRange-01483")
                                                     : "VUID-VkImageViewCreateInfo-subresourceRange-01719";

    return ValidateImageSubresourceRange(device_data, image_state->createInfo.mipLevels, image_layer_count, subresourceRange,
                                         "vkCreateImageView", "pCreateInfo->subresourceRange", image_layer_count_var_name,
                                         HandleToUint64(image_state->image), subresourceRangeErrorCodes);
}

bool ValidateCmdClearColorSubresourceRange(const layer_data *device_data, const IMAGE_STATE *image_state,
                                           const VkImageSubresourceRange &subresourceRange, const char *param_name) {
    SubresourceRangeErrorCodes subresourceRangeErrorCodes = {};
    subresourceRangeErrorCodes.base_mip_err = "VUID-vkCmdClearColorImage-baseMipLevel-01470";
    subresourceRangeErrorCodes.mip_count_err = "VUID-vkCmdClearColorImage-pRanges-01692";
    subresourceRangeErrorCodes.base_layer_err = "VUID-vkCmdClearColorImage-baseArrayLayer-01472";
    subresourceRangeErrorCodes.layer_count_err = "VUID-vkCmdClearColorImage-pRanges-01693";

    return ValidateImageSubresourceRange(device_data, image_state->createInfo.mipLevels, image_state->createInfo.arrayLayers,
                                         subresourceRange, "vkCmdClearColorImage", param_name, "arrayLayers",
                                         HandleToUint64(image_state->image), subresourceRangeErrorCodes);
}

bool ValidateCmdClearDepthSubresourceRange(const layer_data *device_data, const IMAGE_STATE *image_state,
                                           const VkImageSubresourceRange &subresourceRange, const char *param_name) {
    SubresourceRangeErrorCodes subresourceRangeErrorCodes = {};
    subresourceRangeErrorCodes.base_mip_err = "VUID-vkCmdClearDepthStencilImage-baseMipLevel-01474";
    subresourceRangeErrorCodes.mip_count_err = "VUID-vkCmdClearDepthStencilImage-pRanges-01694";
    subresourceRangeErrorCodes.base_layer_err = "VUID-vkCmdClearDepthStencilImage-baseArrayLayer-01476";
    subresourceRangeErrorCodes.layer_count_err = "VUID-vkCmdClearDepthStencilImage-pRanges-01695";

    return ValidateImageSubresourceRange(device_data, image_state->createInfo.mipLevels, image_state->createInfo.arrayLayers,
                                         subresourceRange, "vkCmdClearDepthStencilImage", param_name, "arrayLayers",
                                         HandleToUint64(image_state->image), subresourceRangeErrorCodes);
}

bool ValidateImageBarrierSubresourceRange(const layer_data *device_data, const IMAGE_STATE *image_state,
                                          const VkImageSubresourceRange &subresourceRange, const char *cmd_name,
                                          const char *param_name) {
    SubresourceRangeErrorCodes subresourceRangeErrorCodes = {};
    subresourceRangeErrorCodes.base_mip_err = "VUID-VkImageMemoryBarrier-subresourceRange-01486";
    subresourceRangeErrorCodes.mip_count_err = "VUID-VkImageMemoryBarrier-subresourceRange-01724";
    subresourceRangeErrorCodes.base_layer_err = "VUID-VkImageMemoryBarrier-subresourceRange-01488";
    subresourceRangeErrorCodes.layer_count_err = "VUID-VkImageMemoryBarrier-subresourceRange-01725";

    return ValidateImageSubresourceRange(device_data, image_state->createInfo.mipLevels, image_state->createInfo.arrayLayers,
                                         subresourceRange, cmd_name, param_name, "arrayLayers", HandleToUint64(image_state->image),
                                         subresourceRangeErrorCodes);
}

bool PreCallValidateCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                    const VkAllocationCallbacks *pAllocator, VkImageView *pView) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    const debug_report_data *report_data = device_data->report_data;
    bool skip = false;
    IMAGE_STATE *image_state = GetImageState(device_data, pCreateInfo->image);
    if (image_state) {
        skip |= ValidateImageUsageFlags(
            device_data, image_state,
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV,
            false, kVUIDUndefined, "vkCreateImageView()",
            "VK_IMAGE_USAGE_[SAMPLED|STORAGE|COLOR_ATTACHMENT|DEPTH_STENCIL_ATTACHMENT|INPUT_ATTACHMENT|SHADING_RATE_IMAGE]_BIT");
        // If this isn't a sparse image, it needs to have memory backing it at CreateImageView time
        skip |=
            ValidateMemoryIsBoundToImage(device_data, image_state, "vkCreateImageView()", "VUID-VkImageViewCreateInfo-image-01020");
        // Checks imported from image layer
        skip |= ValidateCreateImageViewSubresourceRange(
            device_data, image_state,
            pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_2D || pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_2D_ARRAY,
            pCreateInfo->subresourceRange);

        VkImageCreateFlags image_flags = image_state->createInfo.flags;
        VkFormat image_format = image_state->createInfo.format;
        VkImageUsageFlags image_usage = image_state->createInfo.usage;
        VkImageTiling image_tiling = image_state->createInfo.tiling;
        VkFormat view_format = pCreateInfo->format;
        VkImageAspectFlags aspect_mask = pCreateInfo->subresourceRange.aspectMask;
        VkImageType image_type = image_state->createInfo.imageType;
        VkImageViewType view_type = pCreateInfo->viewType;

        // If there's a chained VkImageViewUsageCreateInfo struct, modify image_usage to match
        auto chained_ivuci_struct = lvl_find_in_chain<VkImageViewUsageCreateInfoKHR>(pCreateInfo->pNext);
        if (chained_ivuci_struct) {
            if (chained_ivuci_struct->usage & ~image_usage) {
                std::stringstream ss;
                ss << "vkCreateImageView(): Chained VkImageViewUsageCreateInfo usage field (0x" << std::hex
                   << chained_ivuci_struct->usage << ") must not include flags not present in underlying image's usage (0x"
                   << image_usage << ").";
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                HandleToUint64(pCreateInfo->image), "VUID-VkImageViewUsageCreateInfo-usage-01587", "%s",
                                ss.str().c_str());
            }

            image_usage = chained_ivuci_struct->usage;
        }

        // Validate VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT state, if view/image formats differ
        if ((image_flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) && (image_format != view_format)) {
            if (FormatIsMultiplane(image_format)) {
                // View format must match the multiplane compatible format
                uint32_t plane = 3;  // invalid
                switch (aspect_mask) {
                    case VK_IMAGE_ASPECT_PLANE_0_BIT:
                        plane = 0;
                        break;
                    case VK_IMAGE_ASPECT_PLANE_1_BIT:
                        plane = 1;
                        break;
                    case VK_IMAGE_ASPECT_PLANE_2_BIT:
                        plane = 2;
                        break;
                    default:
                        break;
                }

                VkFormat compat_format = FindMultiplaneCompatibleFormat(image_format, plane);
                if (view_format != compat_format) {
                    std::stringstream ss;
                    ss << "vkCreateImageView(): ImageView format " << string_VkFormat(view_format)
                       << " is not compatible with plane " << plane << " of underlying image format "
                       << string_VkFormat(image_format) << ", must be " << string_VkFormat(compat_format) << ".";
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                    HandleToUint64(pCreateInfo->image), "VUID-VkImageViewCreateInfo-image-01586", "%s",
                                    ss.str().c_str());
                }
            } else {
                if ((!GetDeviceExtensions(device_data)->vk_khr_maintenance2 ||
                     !(image_flags & VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT_KHR))) {
                    // Format MUST be compatible (in the same format compatibility class) as the format the image was created with
                    if (FormatCompatibilityClass(image_format) != FormatCompatibilityClass(view_format)) {
                        std::stringstream ss;
                        ss << "vkCreateImageView(): ImageView format " << string_VkFormat(view_format)
                           << " is not in the same format compatibility class as image (" << HandleToUint64(pCreateInfo->image)
                           << ")  format " << string_VkFormat(image_format)
                           << ".  Images created with the VK_IMAGE_CREATE_MUTABLE_FORMAT BIT "
                           << "can support ImageViews with differing formats but they must be in the same compatibility class.";
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                        HandleToUint64(pCreateInfo->image), "VUID-VkImageViewCreateInfo-image-01018", "%s",
                                        ss.str().c_str());
                    }
                }
            }
        } else {
            // Format MUST be IDENTICAL to the format the image was created with
            if (image_format != view_format) {
                std::stringstream ss;
                ss << "vkCreateImageView() format " << string_VkFormat(view_format) << " differs from image "
                   << HandleToUint64(pCreateInfo->image) << " format " << string_VkFormat(image_format)
                   << ".  Formats MUST be IDENTICAL unless VK_IMAGE_CREATE_MUTABLE_FORMAT BIT was set on image creation.";
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(pCreateInfo->image), "VUID-VkImageViewCreateInfo-image-01019", "%s", ss.str().c_str());
            }
        }

        // Validate correct image aspect bits for desired formats and format consistency
        skip |= ValidateImageAspectMask(device_data, image_state->image, image_format, aspect_mask, "vkCreateImageView()");

        switch (image_type) {
            case VK_IMAGE_TYPE_1D:
                if (view_type != VK_IMAGE_VIEW_TYPE_1D && view_type != VK_IMAGE_VIEW_TYPE_1D_ARRAY) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                    HandleToUint64(pCreateInfo->image), "VUID-VkImageViewCreateInfo-subResourceRange-01021",
                                    "vkCreateImageView(): pCreateInfo->viewType %s is not compatible with image type %s.",
                                    string_VkImageViewType(view_type), string_VkImageType(image_type));
                }
                break;
            case VK_IMAGE_TYPE_2D:
                if (view_type != VK_IMAGE_VIEW_TYPE_2D && view_type != VK_IMAGE_VIEW_TYPE_2D_ARRAY) {
                    if ((view_type == VK_IMAGE_VIEW_TYPE_CUBE || view_type == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY) &&
                        !(image_flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                        HandleToUint64(pCreateInfo->image), "VUID-VkImageViewCreateInfo-image-01003",
                                        "vkCreateImageView(): pCreateInfo->viewType %s is not compatible with image type %s.",
                                        string_VkImageViewType(view_type), string_VkImageType(image_type));
                    } else if (view_type != VK_IMAGE_VIEW_TYPE_CUBE && view_type != VK_IMAGE_VIEW_TYPE_CUBE_ARRAY) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                        HandleToUint64(pCreateInfo->image), "VUID-VkImageViewCreateInfo-subResourceRange-01021",
                                        "vkCreateImageView(): pCreateInfo->viewType %s is not compatible with image type %s.",
                                        string_VkImageViewType(view_type), string_VkImageType(image_type));
                    }
                }
                break;
            case VK_IMAGE_TYPE_3D:
                if (GetDeviceExtensions(device_data)->vk_khr_maintenance1) {
                    if (view_type != VK_IMAGE_VIEW_TYPE_3D) {
                        if ((view_type == VK_IMAGE_VIEW_TYPE_2D || view_type == VK_IMAGE_VIEW_TYPE_2D_ARRAY)) {
                            if (!(image_flags & VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR)) {
                                skip |=
                                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                            HandleToUint64(pCreateInfo->image), "VUID-VkImageViewCreateInfo-image-01005",
                                            "vkCreateImageView(): pCreateInfo->viewType %s is not compatible with image type %s.",
                                            string_VkImageViewType(view_type), string_VkImageType(image_type));
                            } else if ((image_flags & (VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT |
                                                       VK_IMAGE_CREATE_SPARSE_ALIASED_BIT))) {
                                skip |=
                                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                            HandleToUint64(pCreateInfo->image), "VUID-VkImageViewCreateInfo-subResourceRange-01021",
                                            "vkCreateImageView(): pCreateInfo->viewType %s is not compatible with image type %s "
                                            "when the VK_IMAGE_CREATE_SPARSE_BINDING_BIT, VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT, or "
                                            "VK_IMAGE_CREATE_SPARSE_ALIASED_BIT flags are enabled.",
                                            string_VkImageViewType(view_type), string_VkImageType(image_type));
                            }
                        } else {
                            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                            HandleToUint64(pCreateInfo->image), "VUID-VkImageViewCreateInfo-subResourceRange-01021",
                                            "vkCreateImageView(): pCreateInfo->viewType %s is not compatible with image type %s.",
                                            string_VkImageViewType(view_type), string_VkImageType(image_type));
                        }
                    }
                } else {
                    if (view_type != VK_IMAGE_VIEW_TYPE_3D) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                        HandleToUint64(pCreateInfo->image), "VUID-VkImageViewCreateInfo-subResourceRange-01021",
                                        "vkCreateImageView(): pCreateInfo->viewType %s is not compatible with image type %s.",
                                        string_VkImageViewType(view_type), string_VkImageType(image_type));
                    }
                }
                break;
            default:
                break;
        }

        // External format checks needed when VK_ANDROID_external_memory_android_hardware_buffer enabled
        if (GetDeviceExtensions(device_data)->vk_android_external_memory_android_hardware_buffer) {
            skip |= ValidateCreateImageViewANDROID(device_data, pCreateInfo);
        }

        VkFormatProperties format_properties = GetPDFormatProperties(device_data, view_format);
        VkFormatFeatureFlags tiling_features = (image_tiling & VK_IMAGE_TILING_LINEAR) ? format_properties.linearTilingFeatures
                                                                                       : format_properties.optimalTilingFeatures;

        if (tiling_features == 0) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(pCreateInfo->image), "VUID-VkImageViewCreateInfo-None-02273",
                            "vkCreateImageView(): pCreateInfo->format %s with tiling %s has no supported format features on this "
                            "physical device.",
                            string_VkFormat(view_format), string_VkImageTiling(image_tiling));
        } else if ((image_usage & VK_IMAGE_USAGE_SAMPLED_BIT) && !(tiling_features & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(pCreateInfo->image), "VUID-VkImageViewCreateInfo-usage-02274",
                            "vkCreateImageView(): pCreateInfo->format %s with tiling %s does not support usage that includes "
                            "VK_IMAGE_USAGE_SAMPLED_BIT.",
                            string_VkFormat(view_format), string_VkImageTiling(image_tiling));
        } else if ((image_usage & VK_IMAGE_USAGE_STORAGE_BIT) && !(tiling_features & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(pCreateInfo->image), "VUID-VkImageViewCreateInfo-usage-02275",
                            "vkCreateImageView(): pCreateInfo->format %s with tiling %s does not support usage that includes "
                            "VK_IMAGE_USAGE_STORAGE_BIT.",
                            string_VkFormat(view_format), string_VkImageTiling(image_tiling));
        } else if ((image_usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) &&
                   !(tiling_features & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(pCreateInfo->image), "VUID-VkImageViewCreateInfo-usage-02276",
                            "vkCreateImageView(): pCreateInfo->format %s with tiling %s does not support usage that includes "
                            "VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT.",
                            string_VkFormat(view_format), string_VkImageTiling(image_tiling));
        } else if ((image_usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) &&
                   !(tiling_features & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(pCreateInfo->image), "VUID-VkImageViewCreateInfo-usage-02277",
                            "vkCreateImageView(): pCreateInfo->format %s with tiling %s does not support usage that includes "
                            "VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT.",
                            string_VkFormat(view_format), string_VkImageTiling(image_tiling));
        }

        if (image_usage & VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV) {
            if (view_type != VK_IMAGE_VIEW_TYPE_2D && view_type != VK_IMAGE_VIEW_TYPE_2D_ARRAY) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                HandleToUint64(pCreateInfo->image), "VUID-VkImageViewCreateInfo-image-02086",
                                "vkCreateImageView() If image was created with usage containing "
                                "VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV, viewType must be "
                                "VK_IMAGE_VIEW_TYPE_2D or VK_IMAGE_VIEW_TYPE_2D_ARRAY.");
            }
            if (view_format != VK_FORMAT_R8_UINT) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                HandleToUint64(pCreateInfo->image), "VUID-VkImageViewCreateInfo-image-02087",
                                "vkCreateImageView() If image was created with usage containing "
                                "VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV, format must be VK_FORMAT_R8_UINT.");
            }
        }
    }
    return skip;
}

void PostCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                   const VkAllocationCallbacks *pAllocator, VkImageView *pView, VkResult result) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (result != VK_SUCCESS) return;
    auto image_view_map = GetImageViewMap(device_data);
    (*image_view_map)[*pView] = std::unique_ptr<IMAGE_VIEW_STATE>(new IMAGE_VIEW_STATE(*pView, pCreateInfo));

    auto image_state = GetImageState(device_data, pCreateInfo->image);
    auto &sub_res_range = (*image_view_map)[*pView].get()->create_info.subresourceRange;
    sub_res_range.levelCount = ResolveRemainingLevels(&sub_res_range, image_state->createInfo.mipLevels);
    sub_res_range.layerCount = ResolveRemainingLayers(&sub_res_range, image_state->createInfo.arrayLayers);
}

bool PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                  const VkBufferCopy *pRegions) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    auto cb_node = GetCBNode(device_data, commandBuffer);
    auto src_buffer_state = GetBufferState(device_data, srcBuffer);
    auto dst_buffer_state = GetBufferState(device_data, dstBuffer);

    bool skip = false;
    skip |=
        ValidateMemoryIsBoundToBuffer(device_data, src_buffer_state, "vkCmdCopyBuffer()", "VUID-vkCmdCopyBuffer-srcBuffer-00119");
    skip |=
        ValidateMemoryIsBoundToBuffer(device_data, dst_buffer_state, "vkCmdCopyBuffer()", "VUID-vkCmdCopyBuffer-dstBuffer-00121");
    // Validate that SRC & DST buffers have correct usage flags set
    skip |=
        ValidateBufferUsageFlags(device_data, src_buffer_state, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true,
                                 "VUID-vkCmdCopyBuffer-srcBuffer-00118", "vkCmdCopyBuffer()", "VK_BUFFER_USAGE_TRANSFER_SRC_BIT");
    skip |=
        ValidateBufferUsageFlags(device_data, dst_buffer_state, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true,
                                 "VUID-vkCmdCopyBuffer-dstBuffer-00120", "vkCmdCopyBuffer()", "VK_BUFFER_USAGE_TRANSFER_DST_BIT");
    skip |= ValidateCmdQueueFlags(device_data, cb_node, "vkCmdCopyBuffer()",
                                  VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
                                  "VUID-vkCmdCopyBuffer-commandBuffer-cmdpool");
    skip |= ValidateCmd(device_data, cb_node, CMD_COPYBUFFER, "vkCmdCopyBuffer()");
    skip |= InsideRenderPass(device_data, cb_node, "vkCmdCopyBuffer()", "VUID-vkCmdCopyBuffer-renderpass");
    return skip;
}

void PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                const VkBufferCopy *pRegions) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    auto cb_node = GetCBNode(device_data, commandBuffer);
    auto src_buffer_state = GetBufferState(device_data, srcBuffer);
    auto dst_buffer_state = GetBufferState(device_data, dstBuffer);

    // Update bindings between buffers and cmd buffer
    AddCommandBufferBindingBuffer(device_data, cb_node, src_buffer_state);
    AddCommandBufferBindingBuffer(device_data, cb_node, dst_buffer_state);
}

static bool ValidateIdleBuffer(layer_data *device_data, VkBuffer buffer) {
    const debug_report_data *report_data = core_validation::GetReportData(device_data);
    bool skip = false;
    auto buffer_state = GetBufferState(device_data, buffer);
    if (!buffer_state) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, HandleToUint64(buffer),
                        kVUID_Core_DrawState_DoubleDestroy, "Cannot free buffer 0x%" PRIx64 " that has not been allocated.",
                        HandleToUint64(buffer));
    } else {
        if (buffer_state->in_use.load()) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,
                            HandleToUint64(buffer), "VUID-vkDestroyBuffer-buffer-00922",
                            "Cannot free buffer 0x%" PRIx64 " that is in use by a command buffer.", HandleToUint64(buffer));
        }
    }
    return skip;
}

bool PreCallValidateDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks *pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    IMAGE_VIEW_STATE *image_view_state = GetImageViewState(device_data, imageView);
    VK_OBJECT obj_struct = {HandleToUint64(imageView), kVulkanObjectTypeImageView};

    bool skip = false;
    if (image_view_state) {
        skip |= ValidateObjectNotInUse(device_data, image_view_state, obj_struct, "vkDestroyImageView",
                                       "VUID-vkDestroyImageView-imageView-01026");
    }
    return skip;
}

void PreCallRecordDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks *pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    IMAGE_VIEW_STATE *image_view_state = GetImageViewState(device_data, imageView);
    if (!image_view_state) return;
    VK_OBJECT obj_struct = {HandleToUint64(imageView), kVulkanObjectTypeImageView};

    // Any bound cmd buffers are now invalid
    InvalidateCommandBuffers(device_data, image_view_state->cb_bindings, obj_struct);
    (*GetImageViewMap(device_data)).erase(imageView);
}

bool PreCallValidateDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    auto buffer_state = GetBufferState(device_data, buffer);

    bool skip = false;
    if (buffer_state) {
        skip |= ValidateIdleBuffer(device_data, buffer);
    }
    return skip;
}

void PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!buffer) return;
    auto buffer_state = GetBufferState(device_data, buffer);
    VK_OBJECT obj_struct = {HandleToUint64(buffer), kVulkanObjectTypeBuffer};

    InvalidateCommandBuffers(device_data, buffer_state->cb_bindings, obj_struct);
    for (auto mem_binding : buffer_state->GetBoundMemory()) {
        auto mem_info = GetMemObjInfo(device_data, mem_binding);
        if (mem_info) {
            core_validation::RemoveBufferMemoryRange(HandleToUint64(buffer), mem_info);
        }
    }
    ClearMemoryObjectBindings(device_data, HandleToUint64(buffer), kVulkanObjectTypeBuffer);
    EraseQFOReleaseBarriers<VkBufferMemoryBarrier>(device_data, buffer);
    GetBufferMap(device_data)->erase(buffer_state->buffer);
}

bool PreCallValidateDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks *pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    auto buffer_view_state = GetBufferViewState(device_data, bufferView);
    VK_OBJECT obj_struct = {HandleToUint64(bufferView), kVulkanObjectTypeBufferView};
    bool skip = false;
    if (buffer_view_state) {
        skip |= ValidateObjectNotInUse(device_data, buffer_view_state, obj_struct, "vkDestroyBufferView",
                                       "VUID-vkDestroyBufferView-bufferView-00936");
    }
    return skip;
}

void PreCallRecordDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks *pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!bufferView) return;
    auto buffer_view_state = GetBufferViewState(device_data, bufferView);
    VK_OBJECT obj_struct = {HandleToUint64(bufferView), kVulkanObjectTypeBufferView};

    // Any bound cmd buffers are now invalid
    InvalidateCommandBuffers(device_data, buffer_view_state->cb_bindings, obj_struct);
    GetBufferViewMap(device_data)->erase(bufferView);
}

bool PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                  uint32_t data) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    auto cb_node = GetCBNode(device_data, commandBuffer);
    auto buffer_state = GetBufferState(device_data, dstBuffer);
    bool skip = false;
    skip |= ValidateMemoryIsBoundToBuffer(device_data, buffer_state, "vkCmdFillBuffer()", "VUID-vkCmdFillBuffer-dstBuffer-00031");
    skip |= ValidateCmdQueueFlags(device_data, cb_node, "vkCmdFillBuffer()",
                                  VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
                                  "VUID-vkCmdFillBuffer-commandBuffer-cmdpool");
    skip |= ValidateCmd(device_data, cb_node, CMD_FILLBUFFER, "vkCmdFillBuffer()");
    // Validate that DST buffer has correct usage flags set
    skip |=
        ValidateBufferUsageFlags(device_data, buffer_state, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true,
                                 "VUID-vkCmdFillBuffer-dstBuffer-00029", "vkCmdFillBuffer()", "VK_BUFFER_USAGE_TRANSFER_DST_BIT");
    skip |= InsideRenderPass(device_data, cb_node, "vkCmdFillBuffer()", "VUID-vkCmdFillBuffer-renderpass");
    return skip;
}

void PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                uint32_t data) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    auto cb_node = GetCBNode(device_data, commandBuffer);
    auto buffer_state = GetBufferState(device_data, dstBuffer);
    // Update bindings between buffer and cmd buffer
    AddCommandBufferBindingBuffer(device_data, cb_node, buffer_state);
}

bool ValidateBufferImageCopyData(const debug_report_data *report_data, uint32_t regionCount, const VkBufferImageCopy *pRegions,
                                 IMAGE_STATE *image_state, const char *function) {
    bool skip = false;

    for (uint32_t i = 0; i < regionCount; i++) {
        if (image_state->createInfo.imageType == VK_IMAGE_TYPE_1D) {
            if ((pRegions[i].imageOffset.y != 0) || (pRegions[i].imageExtent.height != 1)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                HandleToUint64(image_state->image), "VUID-VkBufferImageCopy-srcImage-00199",
                                "%s(): pRegion[%d] imageOffset.y is %d and imageExtent.height is %d. For 1D images these must be 0 "
                                "and 1, respectively.",
                                function, i, pRegions[i].imageOffset.y, pRegions[i].imageExtent.height);
            }
        }

        if ((image_state->createInfo.imageType == VK_IMAGE_TYPE_1D) || (image_state->createInfo.imageType == VK_IMAGE_TYPE_2D)) {
            if ((pRegions[i].imageOffset.z != 0) || (pRegions[i].imageExtent.depth != 1)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                HandleToUint64(image_state->image), "VUID-VkBufferImageCopy-srcImage-00201",
                                "%s(): pRegion[%d] imageOffset.z is %d and imageExtent.depth is %d. For 1D and 2D images these "
                                "must be 0 and 1, respectively.",
                                function, i, pRegions[i].imageOffset.z, pRegions[i].imageExtent.depth);
            }
        }

        if (image_state->createInfo.imageType == VK_IMAGE_TYPE_3D) {
            if ((0 != pRegions[i].imageSubresource.baseArrayLayer) || (1 != pRegions[i].imageSubresource.layerCount)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                HandleToUint64(image_state->image), "VUID-VkBufferImageCopy-baseArrayLayer-00213",
                                "%s(): pRegion[%d] imageSubresource.baseArrayLayer is %d and imageSubresource.layerCount is %d. "
                                "For 3D images these must be 0 and 1, respectively.",
                                function, i, pRegions[i].imageSubresource.baseArrayLayer, pRegions[i].imageSubresource.layerCount);
            }
        }

        // If the the calling command's VkImage parameter's format is not a depth/stencil format,
        // then bufferOffset must be a multiple of the calling command's VkImage parameter's element size
        uint32_t element_size = FormatElementSize(image_state->createInfo.format);
        if (!FormatIsDepthAndStencil(image_state->createInfo.format) && SafeModulo(pRegions[i].bufferOffset, element_size) != 0) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(image_state->image), "VUID-VkBufferImageCopy-bufferOffset-00193",
                            "%s(): pRegion[%d] bufferOffset 0x%" PRIxLEAST64
                            " must be a multiple of this format's texel size (%" PRIu32 ").",
                            function, i, pRegions[i].bufferOffset, element_size);
        }

        //  BufferOffset must be a multiple of 4
        if (SafeModulo(pRegions[i].bufferOffset, 4) != 0) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(image_state->image), "VUID-VkBufferImageCopy-bufferOffset-00194",
                            "%s(): pRegion[%d] bufferOffset 0x%" PRIxLEAST64 " must be a multiple of 4.", function, i,
                            pRegions[i].bufferOffset);
        }

        //  BufferRowLength must be 0, or greater than or equal to the width member of imageExtent
        if ((pRegions[i].bufferRowLength != 0) && (pRegions[i].bufferRowLength < pRegions[i].imageExtent.width)) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                        HandleToUint64(image_state->image), "VUID-VkBufferImageCopy-bufferRowLength-00195",
                        "%s(): pRegion[%d] bufferRowLength (%d) must be zero or greater-than-or-equal-to imageExtent.width (%d).",
                        function, i, pRegions[i].bufferRowLength, pRegions[i].imageExtent.width);
        }

        //  BufferImageHeight must be 0, or greater than or equal to the height member of imageExtent
        if ((pRegions[i].bufferImageHeight != 0) && (pRegions[i].bufferImageHeight < pRegions[i].imageExtent.height)) {
            skip |= log_msg(
                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                HandleToUint64(image_state->image), "VUID-VkBufferImageCopy-bufferImageHeight-00196",
                "%s(): pRegion[%d] bufferImageHeight (%d) must be zero or greater-than-or-equal-to imageExtent.height (%d).",
                function, i, pRegions[i].bufferImageHeight, pRegions[i].imageExtent.height);
        }

        // subresource aspectMask must have exactly 1 bit set
        const int num_bits = sizeof(VkFlags) * CHAR_BIT;
        std::bitset<num_bits> aspect_mask_bits(pRegions[i].imageSubresource.aspectMask);
        if (aspect_mask_bits.count() != 1) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(image_state->image), "VUID-VkBufferImageCopy-aspectMask-00212",
                            "%s: aspectMasks for imageSubresource in each region must have only a single bit set.", function);
        }

        // image subresource aspect bit must match format
        if (!VerifyAspectsPresent(pRegions[i].imageSubresource.aspectMask, image_state->createInfo.format)) {
            skip |= log_msg(
                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                HandleToUint64(image_state->image), "VUID-VkBufferImageCopy-aspectMask-00211",
                "%s(): pRegion[%d] subresource aspectMask 0x%x specifies aspects that are not present in image format 0x%x.",
                function, i, pRegions[i].imageSubresource.aspectMask, image_state->createInfo.format);
        }

        // Checks that apply only to compressed images
        if (FormatIsCompressed(image_state->createInfo.format) || FormatIsSinglePlane_422(image_state->createInfo.format)) {
            auto block_size = FormatTexelBlockExtent(image_state->createInfo.format);

            //  BufferRowLength must be a multiple of block width
            if (SafeModulo(pRegions[i].bufferRowLength, block_size.width) != 0) {
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                    HandleToUint64(image_state->image), "VUID-VkBufferImageCopy-bufferRowLength-00203",
                    "%s(): pRegion[%d] bufferRowLength (%d) must be a multiple of the compressed image's texel width (%d)..",
                    function, i, pRegions[i].bufferRowLength, block_size.width);
            }

            //  BufferRowHeight must be a multiple of block height
            if (SafeModulo(pRegions[i].bufferImageHeight, block_size.height) != 0) {
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                    HandleToUint64(image_state->image), "VUID-VkBufferImageCopy-bufferImageHeight-00204",
                    "%s(): pRegion[%d] bufferImageHeight (%d) must be a multiple of the compressed image's texel height (%d)..",
                    function, i, pRegions[i].bufferImageHeight, block_size.height);
            }

            //  image offsets must be multiples of block dimensions
            if ((SafeModulo(pRegions[i].imageOffset.x, block_size.width) != 0) ||
                (SafeModulo(pRegions[i].imageOffset.y, block_size.height) != 0) ||
                (SafeModulo(pRegions[i].imageOffset.z, block_size.depth) != 0)) {
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(image_state->image), "VUID-VkBufferImageCopy-imageOffset-00205",
                            "%s(): pRegion[%d] imageOffset(x,y) (%d, %d) must be multiples of the compressed image's texel "
                            "width & height (%d, %d)..",
                            function, i, pRegions[i].imageOffset.x, pRegions[i].imageOffset.y, block_size.width, block_size.height);
            }

            // bufferOffset must be a multiple of block size (linear bytes)
            uint32_t block_size_in_bytes = FormatElementSize(image_state->createInfo.format);
            if (SafeModulo(pRegions[i].bufferOffset, block_size_in_bytes) != 0) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                HandleToUint64(image_state->image), "VUID-VkBufferImageCopy-bufferOffset-00206",
                                "%s(): pRegion[%d] bufferOffset (0x%" PRIxLEAST64
                                ") must be a multiple of the compressed image's texel block size (%" PRIu32 ")..",
                                function, i, pRegions[i].bufferOffset, block_size_in_bytes);
            }

            // imageExtent width must be a multiple of block width, or extent+offset width must equal subresource width
            VkExtent3D mip_extent = GetImageSubresourceExtent(image_state, &(pRegions[i].imageSubresource));
            if ((SafeModulo(pRegions[i].imageExtent.width, block_size.width) != 0) &&
                (pRegions[i].imageExtent.width + pRegions[i].imageOffset.x != mip_extent.width)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                HandleToUint64(image_state->image), "VUID-VkBufferImageCopy-imageExtent-00207",
                                "%s(): pRegion[%d] extent width (%d) must be a multiple of the compressed texture block width "
                                "(%d), or when added to offset.x (%d) must equal the image subresource width (%d)..",
                                function, i, pRegions[i].imageExtent.width, block_size.width, pRegions[i].imageOffset.x,
                                mip_extent.width);
            }

            // imageExtent height must be a multiple of block height, or extent+offset height must equal subresource height
            if ((SafeModulo(pRegions[i].imageExtent.height, block_size.height) != 0) &&
                (pRegions[i].imageExtent.height + pRegions[i].imageOffset.y != mip_extent.height)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                HandleToUint64(image_state->image), "VUID-VkBufferImageCopy-imageExtent-00208",
                                "%s(): pRegion[%d] extent height (%d) must be a multiple of the compressed texture block height "
                                "(%d), or when added to offset.y (%d) must equal the image subresource height (%d)..",
                                function, i, pRegions[i].imageExtent.height, block_size.height, pRegions[i].imageOffset.y,
                                mip_extent.height);
            }

            // imageExtent depth must be a multiple of block depth, or extent+offset depth must equal subresource depth
            if ((SafeModulo(pRegions[i].imageExtent.depth, block_size.depth) != 0) &&
                (pRegions[i].imageExtent.depth + pRegions[i].imageOffset.z != mip_extent.depth)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                HandleToUint64(image_state->image), "VUID-VkBufferImageCopy-imageExtent-00209",
                                "%s(): pRegion[%d] extent width (%d) must be a multiple of the compressed texture block depth "
                                "(%d), or when added to offset.z (%d) must equal the image subresource depth (%d)..",
                                function, i, pRegions[i].imageExtent.depth, block_size.depth, pRegions[i].imageOffset.z,
                                mip_extent.depth);
            }
        }
    }

    return skip;
}

static bool ValidateImageBounds(const debug_report_data *report_data, const IMAGE_STATE *image_state, const uint32_t regionCount,
                                const VkBufferImageCopy *pRegions, const char *func_name, std::string msg_code) {
    bool skip = false;
    const VkImageCreateInfo *image_info = &(image_state->createInfo);

    for (uint32_t i = 0; i < regionCount; i++) {
        VkExtent3D extent = pRegions[i].imageExtent;
        VkOffset3D offset = pRegions[i].imageOffset;

        if (IsExtentSizeZero(&extent))  // Warn on zero area subresource
        {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, (uint64_t)0,
                        kVUID_Core_Image_ZeroAreaSubregion, "%s: pRegion[%d] imageExtent of {%1d, %1d, %1d} has zero area",
                        func_name, i, extent.width, extent.height, extent.depth);
        }

        VkExtent3D image_extent = GetImageSubresourceExtent(image_state, &(pRegions[i].imageSubresource));

        // If we're using a compressed format, valid extent is rounded up to multiple of block size (per 18.1)
        if (FormatIsCompressed(image_info->format)) {
            auto block_extent = FormatTexelBlockExtent(image_info->format);
            if (image_extent.width % block_extent.width) {
                image_extent.width += (block_extent.width - (image_extent.width % block_extent.width));
            }
            if (image_extent.height % block_extent.height) {
                image_extent.height += (block_extent.height - (image_extent.height % block_extent.height));
            }
            if (image_extent.depth % block_extent.depth) {
                image_extent.depth += (block_extent.depth - (image_extent.depth % block_extent.depth));
            }
        }

        if (0 != ExceedsBounds(&offset, &extent, &image_extent)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, (uint64_t)0,
                            msg_code, "%s: pRegion[%d] exceeds image bounds..", func_name, i);
        }
    }

    return skip;
}

static inline bool ValidateBufferBounds(const debug_report_data *report_data, IMAGE_STATE *image_state, BUFFER_STATE *buff_state,
                                        uint32_t regionCount, const VkBufferImageCopy *pRegions, const char *func_name,
                                        std::string msg_code) {
    bool skip = false;

    VkDeviceSize buffer_size = buff_state->createInfo.size;

    for (uint32_t i = 0; i < regionCount; i++) {
        VkExtent3D copy_extent = pRegions[i].imageExtent;

        VkDeviceSize buffer_width = (0 == pRegions[i].bufferRowLength ? copy_extent.width : pRegions[i].bufferRowLength);
        VkDeviceSize buffer_height = (0 == pRegions[i].bufferImageHeight ? copy_extent.height : pRegions[i].bufferImageHeight);
        VkDeviceSize unit_size = FormatElementSize(image_state->createInfo.format);  // size (bytes) of texel or block

        // Handle special buffer packing rules for specific depth/stencil formats
        if (pRegions[i].imageSubresource.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) {
            unit_size = FormatElementSize(VK_FORMAT_S8_UINT);
        } else if (pRegions[i].imageSubresource.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) {
            switch (image_state->createInfo.format) {
                case VK_FORMAT_D16_UNORM_S8_UINT:
                    unit_size = FormatElementSize(VK_FORMAT_D16_UNORM);
                    break;
                case VK_FORMAT_D32_SFLOAT_S8_UINT:
                    unit_size = FormatElementSize(VK_FORMAT_D32_SFLOAT);
                    break;
                case VK_FORMAT_X8_D24_UNORM_PACK32:  // Fall through
                case VK_FORMAT_D24_UNORM_S8_UINT:
                    unit_size = 4;
                    break;
                default:
                    break;
            }
        }

        if (FormatIsCompressed(image_state->createInfo.format) || FormatIsSinglePlane_422(image_state->createInfo.format)) {
            // Switch to texel block units, rounding up for any partially-used blocks
            auto block_dim = FormatTexelBlockExtent(image_state->createInfo.format);
            buffer_width = (buffer_width + block_dim.width - 1) / block_dim.width;
            buffer_height = (buffer_height + block_dim.height - 1) / block_dim.height;

            copy_extent.width = (copy_extent.width + block_dim.width - 1) / block_dim.width;
            copy_extent.height = (copy_extent.height + block_dim.height - 1) / block_dim.height;
            copy_extent.depth = (copy_extent.depth + block_dim.depth - 1) / block_dim.depth;
        }

        // Either depth or layerCount may be greater than 1 (not both). This is the number of 'slices' to copy
        uint32_t z_copies = std::max(copy_extent.depth, pRegions[i].imageSubresource.layerCount);
        if (IsExtentSizeZero(&copy_extent) || (0 == z_copies)) {
            // TODO: Issue warning here? Already warned in ValidateImageBounds()...
        } else {
            // Calculate buffer offset of final copied byte, + 1.
            VkDeviceSize max_buffer_offset = (z_copies - 1) * buffer_height * buffer_width;      // offset to slice
            max_buffer_offset += ((copy_extent.height - 1) * buffer_width) + copy_extent.width;  // add row,col
            max_buffer_offset *= unit_size;                                                      // convert to bytes
            max_buffer_offset += pRegions[i].bufferOffset;                                       // add initial offset (bytes)

            if (buffer_size < max_buffer_offset) {
                skip |=
                    log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, (uint64_t)0,
                            msg_code, "%s: pRegion[%d] exceeds buffer size of %" PRIu64 " bytes..", func_name, i, buffer_size);
            }
        }
    }

    return skip;
}

bool PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                         VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    auto cb_node = GetCBNode(device_data, commandBuffer);
    auto src_image_state = GetImageState(device_data, srcImage);
    auto dst_buffer_state = GetBufferState(device_data, dstBuffer);
    const debug_report_data *report_data = device_data->report_data;

    bool skip = ValidateBufferImageCopyData(report_data, regionCount, pRegions, src_image_state, "vkCmdCopyImageToBuffer");

    // Validate command buffer state
    skip |= ValidateCmd(device_data, cb_node, CMD_COPYIMAGETOBUFFER, "vkCmdCopyImageToBuffer()");

    // Command pool must support graphics, compute, or transfer operations
    auto pPool = GetCommandPoolNode(device_data, cb_node->createInfo.commandPool);

    VkQueueFlags queue_flags = GetPhysDevProperties(device_data)->queue_family_properties[pPool->queueFamilyIndex].queueFlags;
    if (0 == (queue_flags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT))) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(cb_node->createInfo.commandPool), "VUID-vkCmdCopyImageToBuffer-commandBuffer-cmdpool",
                        "Cannot call vkCmdCopyImageToBuffer() on a command buffer allocated from a pool without graphics, compute, "
                        "or transfer capabilities..");
    }
    skip |= ValidateImageBounds(report_data, src_image_state, regionCount, pRegions, "vkCmdCopyImageToBuffer()",
                                "VUID-vkCmdCopyImageToBuffer-pRegions-00182");
    skip |= ValidateBufferBounds(report_data, src_image_state, dst_buffer_state, regionCount, pRegions, "vkCmdCopyImageToBuffer()",
                                 "VUID-vkCmdCopyImageToBuffer-pRegions-00183");

    skip |= ValidateImageSampleCount(device_data, src_image_state, VK_SAMPLE_COUNT_1_BIT, "vkCmdCopyImageToBuffer(): srcImage",
                                     "VUID-vkCmdCopyImageToBuffer-srcImage-00188");
    skip |= ValidateMemoryIsBoundToImage(device_data, src_image_state, "vkCmdCopyImageToBuffer()",
                                         "VUID-vkCmdCopyImageToBuffer-srcImage-00187");
    skip |= ValidateMemoryIsBoundToBuffer(device_data, dst_buffer_state, "vkCmdCopyImageToBuffer()",
                                          "VUID-vkCmdCopyImageToBuffer-dstBuffer-00192");

    // Validate that SRC image & DST buffer have correct usage flags set
    skip |= ValidateImageUsageFlags(device_data, src_image_state, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, true,
                                    "VUID-vkCmdCopyImageToBuffer-srcImage-00186", "vkCmdCopyImageToBuffer()",
                                    "VK_IMAGE_USAGE_TRANSFER_SRC_BIT");
    skip |= ValidateBufferUsageFlags(device_data, dst_buffer_state, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true,
                                     "VUID-vkCmdCopyImageToBuffer-dstBuffer-00191", "vkCmdCopyImageToBuffer()",
                                     "VK_BUFFER_USAGE_TRANSFER_DST_BIT");
    if (GetApiVersion(device_data) >= VK_API_VERSION_1_1 || GetDeviceExtensions(device_data)->vk_khr_maintenance1) {
        skip |= ValidateImageFormatFeatureFlags(device_data, src_image_state, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT,
                                                "vkCmdCopyImageToBuffer()", "VUID-vkCmdCopyImageToBuffer-srcImage-01998",
                                                "VUID-vkCmdCopyImageToBuffer-srcImage-01998");
    }
    skip |= InsideRenderPass(device_data, cb_node, "vkCmdCopyImageToBuffer()", "VUID-vkCmdCopyImageToBuffer-renderpass");
    bool hit_error = false;
    const std::string src_invalid_layout_vuid =
        (src_image_state->shared_presentable && core_validation::GetDeviceExtensions(device_data)->vk_khr_shared_presentable_image)
            ? "VUID-vkCmdCopyImageToBuffer-srcImageLayout-01397"
            : "VUID-vkCmdCopyImageToBuffer-srcImageLayout-00190";
    for (uint32_t i = 0; i < regionCount; ++i) {
        skip |= ValidateImageSubresourceLayers(device_data, cb_node, &pRegions[i].imageSubresource, "vkCmdCopyImageToBuffer()",
                                               "imageSubresource", i);
        skip |= VerifyImageLayout(device_data, cb_node, src_image_state, pRegions[i].imageSubresource, srcImageLayout,
                                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, "vkCmdCopyImageToBuffer()", src_invalid_layout_vuid,
                                  "VUID-vkCmdCopyImageToBuffer-srcImageLayout-00189", &hit_error);
        skip |= ValidateCopyBufferImageTransferGranularityRequirements(device_data, cb_node, src_image_state, &pRegions[i], i,
                                                                       "vkCmdCopyImageToBuffer()",
                                                                       "VUID-vkCmdCopyImageToBuffer-imageOffset-01794");
        skip |= ValidateImageMipLevel(device_data, cb_node, src_image_state, pRegions[i].imageSubresource.mipLevel, i,
                                      "vkCmdCopyImageToBuffer()", "imageSubresource",
                                      "VUID-vkCmdCopyImageToBuffer-imageSubresource-01703");
        skip |= ValidateImageArrayLayerRange(device_data, cb_node, src_image_state, pRegions[i].imageSubresource.baseArrayLayer,
                                             pRegions[i].imageSubresource.layerCount, i, "vkCmdCopyImageToBuffer()",
                                             "imageSubresource", "VUID-vkCmdCopyImageToBuffer-imageSubresource-01704");
    }
    return skip;
}

void PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                       VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    auto cb_node = GetCBNode(device_data, commandBuffer);
    auto src_image_state = GetImageState(device_data, srcImage);
    auto dst_buffer_state = GetBufferState(device_data, dstBuffer);

    // Make sure that all image slices are updated to correct layout
    for (uint32_t i = 0; i < regionCount; ++i) {
        SetImageLayout(device_data, cb_node, src_image_state, pRegions[i].imageSubresource, srcImageLayout);
    }
    // Update bindings between buffer/image and cmd buffer
    AddCommandBufferBindingImage(device_data, cb_node, src_image_state);
    AddCommandBufferBindingBuffer(device_data, cb_node, dst_buffer_state);
}

bool PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                         VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy *pRegions) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    auto cb_node = GetCBNode(device_data, commandBuffer);
    auto src_buffer_state = GetBufferState(device_data, srcBuffer);
    auto dst_image_state = GetImageState(device_data, dstImage);
    const debug_report_data *report_data = device_data->report_data;

    bool skip = ValidateBufferImageCopyData(report_data, regionCount, pRegions, dst_image_state, "vkCmdCopyBufferToImage");

    // Validate command buffer state
    skip |= ValidateCmd(device_data, cb_node, CMD_COPYBUFFERTOIMAGE, "vkCmdCopyBufferToImage()");

    // Command pool must support graphics, compute, or transfer operations
    auto pPool = GetCommandPoolNode(device_data, cb_node->createInfo.commandPool);
    VkQueueFlags queue_flags = GetPhysDevProperties(device_data)->queue_family_properties[pPool->queueFamilyIndex].queueFlags;
    if (0 == (queue_flags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT))) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        HandleToUint64(cb_node->createInfo.commandPool), "VUID-vkCmdCopyBufferToImage-commandBuffer-cmdpool",
                        "Cannot call vkCmdCopyBufferToImage() on a command buffer allocated from a pool without graphics, compute, "
                        "or transfer capabilities..");
    }
    skip |= ValidateImageBounds(report_data, dst_image_state, regionCount, pRegions, "vkCmdCopyBufferToImage()",
                                "VUID-vkCmdCopyBufferToImage-pRegions-00172");
    skip |= ValidateBufferBounds(report_data, dst_image_state, src_buffer_state, regionCount, pRegions, "vkCmdCopyBufferToImage()",
                                 "VUID-vkCmdCopyBufferToImage-pRegions-00171");
    skip |= ValidateImageSampleCount(device_data, dst_image_state, VK_SAMPLE_COUNT_1_BIT, "vkCmdCopyBufferToImage(): dstImage",
                                     "VUID-vkCmdCopyBufferToImage-dstImage-00179");
    skip |= ValidateMemoryIsBoundToBuffer(device_data, src_buffer_state, "vkCmdCopyBufferToImage()",
                                          "VUID-vkCmdCopyBufferToImage-srcBuffer-00176");
    skip |= ValidateMemoryIsBoundToImage(device_data, dst_image_state, "vkCmdCopyBufferToImage()",
                                         "VUID-vkCmdCopyBufferToImage-dstImage-00178");
    skip |= ValidateBufferUsageFlags(device_data, src_buffer_state, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true,
                                     "VUID-vkCmdCopyBufferToImage-srcBuffer-00174", "vkCmdCopyBufferToImage()",
                                     "VK_BUFFER_USAGE_TRANSFER_SRC_BIT");
    skip |= ValidateImageUsageFlags(device_data, dst_image_state, VK_IMAGE_USAGE_TRANSFER_DST_BIT, true,
                                    "VUID-vkCmdCopyBufferToImage-dstImage-00177", "vkCmdCopyBufferToImage()",
                                    "VK_IMAGE_USAGE_TRANSFER_DST_BIT");
    if (GetApiVersion(device_data) >= VK_API_VERSION_1_1 || GetDeviceExtensions(device_data)->vk_khr_maintenance1) {
        skip |= ValidateImageFormatFeatureFlags(device_data, dst_image_state, VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
                                                "vkCmdCopyBufferToImage()", "VUID-vkCmdCopyBufferToImage-dstImage-01997",
                                                "VUID-vkCmdCopyBufferToImage-dstImage-01997");
    }
    skip |= InsideRenderPass(device_data, cb_node, "vkCmdCopyBufferToImage()", "VUID-vkCmdCopyBufferToImage-renderpass");
    bool hit_error = false;
    const std::string dst_invalid_layout_vuid =
        (dst_image_state->shared_presentable && core_validation::GetDeviceExtensions(device_data)->vk_khr_shared_presentable_image)
            ? "VUID-vkCmdCopyBufferToImage-dstImageLayout-01396"
            : "VUID-vkCmdCopyBufferToImage-dstImageLayout-00181";
    for (uint32_t i = 0; i < regionCount; ++i) {
        skip |= ValidateImageSubresourceLayers(device_data, cb_node, &pRegions[i].imageSubresource, "vkCmdCopyBufferToImage()",
                                               "imageSubresource", i);
        skip |= VerifyImageLayout(device_data, cb_node, dst_image_state, pRegions[i].imageSubresource, dstImageLayout,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, "vkCmdCopyBufferToImage()", dst_invalid_layout_vuid,
                                  "VUID-vkCmdCopyBufferToImage-dstImageLayout-00180", &hit_error);
        skip |= ValidateCopyBufferImageTransferGranularityRequirements(device_data, cb_node, dst_image_state, &pRegions[i], i,
                                                                       "vkCmdCopyBufferToImage()",
                                                                       "VUID-vkCmdCopyBufferToImage-imageOffset-01793");
        skip |= ValidateImageMipLevel(device_data, cb_node, dst_image_state, pRegions[i].imageSubresource.mipLevel, i,
                                      "vkCmdCopyBufferToImage()", "imageSubresource",
                                      "VUID-vkCmdCopyBufferToImage-imageSubresource-01701");
        skip |= ValidateImageArrayLayerRange(device_data, cb_node, dst_image_state, pRegions[i].imageSubresource.baseArrayLayer,
                                             pRegions[i].imageSubresource.layerCount, i, "vkCmdCopyBufferToImage()",
                                             "imageSubresource", "VUID-vkCmdCopyBufferToImage-imageSubresource-01702");
    }
    return skip;
}

void PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                       VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy *pRegions) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    auto cb_node = GetCBNode(device_data, commandBuffer);
    auto src_buffer_state = GetBufferState(device_data, srcBuffer);
    auto dst_image_state = GetImageState(device_data, dstImage);

    // Make sure that all image slices are updated to correct layout
    for (uint32_t i = 0; i < regionCount; ++i) {
        SetImageLayout(device_data, cb_node, dst_image_state, pRegions[i].imageSubresource, dstImageLayout);
    }
    AddCommandBufferBindingBuffer(device_data, cb_node, src_buffer_state);
    AddCommandBufferBindingImage(device_data, cb_node, dst_image_state);
}

bool PreCallValidateGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource *pSubresource,
                                              VkSubresourceLayout *pLayout) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    const auto report_data = device_data->report_data;
    bool skip = false;
    const VkImageAspectFlags sub_aspect = pSubresource->aspectMask;

    // The aspectMask member of pSubresource must only have a single bit set
    const int num_bits = sizeof(sub_aspect) * CHAR_BIT;
    std::bitset<num_bits> aspect_mask_bits(sub_aspect);
    if (aspect_mask_bits.count() != 1) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, HandleToUint64(image),
                        "VUID-vkGetImageSubresourceLayout-aspectMask-00997",
                        "vkGetImageSubresourceLayout(): VkImageSubresource.aspectMask must have exactly 1 bit set.");
    }

    IMAGE_STATE *image_entry = GetImageState(device_data, image);
    if (!image_entry) {
        return skip;
    }

    // image must have been created with tiling equal to VK_IMAGE_TILING_LINEAR
    if (image_entry->createInfo.tiling != VK_IMAGE_TILING_LINEAR) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, HandleToUint64(image),
                        "VUID-vkGetImageSubresourceLayout-image-00996",
                        "vkGetImageSubresourceLayout(): Image must have tiling of VK_IMAGE_TILING_LINEAR.");
    }

    // mipLevel must be less than the mipLevels specified in VkImageCreateInfo when the image was created
    if (pSubresource->mipLevel >= image_entry->createInfo.mipLevels) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, HandleToUint64(image),
                        "VUID-vkGetImageSubresourceLayout-mipLevel-01716",
                        "vkGetImageSubresourceLayout(): pSubresource.mipLevel (%d) must be less than %d.", pSubresource->mipLevel,
                        image_entry->createInfo.mipLevels);
    }

    // arrayLayer must be less than the arrayLayers specified in VkImageCreateInfo when the image was created
    if (pSubresource->arrayLayer >= image_entry->createInfo.arrayLayers) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, HandleToUint64(image),
                        "VUID-vkGetImageSubresourceLayout-arrayLayer-01717",
                        "vkGetImageSubresourceLayout(): pSubresource.arrayLayer (%d) must be less than %d.",
                        pSubresource->arrayLayer, image_entry->createInfo.arrayLayers);
    }

    // subresource's aspect must be compatible with image's format.
    const VkFormat img_format = image_entry->createInfo.format;
    if (FormatIsMultiplane(img_format)) {
        VkImageAspectFlags allowed_flags = (VK_IMAGE_ASPECT_PLANE_0_BIT_KHR | VK_IMAGE_ASPECT_PLANE_1_BIT_KHR);
        std::string vuid = "VUID-vkGetImageSubresourceLayout-format-01581";  // 2-plane version
        if (FormatPlaneCount(img_format) > 2u) {
            allowed_flags |= VK_IMAGE_ASPECT_PLANE_2_BIT_KHR;
            vuid = "VUID-vkGetImageSubresourceLayout-format-01582";  // 3-plane version
        }
        if (sub_aspect != (sub_aspect & allowed_flags)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(image), vuid,
                            "vkGetImageSubresourceLayout(): For multi-planar images, VkImageSubresource.aspectMask (0x%" PRIx32
                            ") must be a single-plane specifier flag.",
                            sub_aspect);
        }
    } else if (FormatIsColor(img_format)) {
        if (sub_aspect != VK_IMAGE_ASPECT_COLOR_BIT) {
            skip |= log_msg(
                report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, HandleToUint64(image),
                "VUID-VkImageSubresource-aspectMask-parameter",
                "vkGetImageSubresourceLayout(): For color formats, VkImageSubresource.aspectMask must be VK_IMAGE_ASPECT_COLOR.");
        }
    } else if (FormatIsDepthOrStencil(img_format)) {
        if ((sub_aspect != VK_IMAGE_ASPECT_DEPTH_BIT) && (sub_aspect != VK_IMAGE_ASPECT_STENCIL_BIT)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                            HandleToUint64(image), "VUID-VkImageSubresource-aspectMask-parameter",
                            "vkGetImageSubresourceLayout(): For depth/stencil formats, VkImageSubresource.aspectMask must be "
                            "either VK_IMAGE_ASPECT_DEPTH_BIT or VK_IMAGE_ASPECT_STENCIL_BIT.");
        }
    }

    if (GetDeviceExtensions(device_data)->vk_android_external_memory_android_hardware_buffer) {
        skip |= ValidateGetImageSubresourceLayoutANDROID(device_data, image);
    }

    return skip;
}
