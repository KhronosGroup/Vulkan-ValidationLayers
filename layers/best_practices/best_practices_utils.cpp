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

struct VendorSpecificInfo {
    EnableFlags vendor_id;
    std::string name;
};

const std::map<BPVendorFlagBits, VendorSpecificInfo> kVendorInfo = {{kBPVendorArm, {vendor_specific_arm, "Arm"}},
                                                                    {kBPVendorAMD, {vendor_specific_amd, "AMD"}},
                                                                    {kBPVendorIMG, {vendor_specific_img, "IMG"}},
                                                                    {kBPVendorNVIDIA, {vendor_specific_nvidia, "NVIDIA"}}};

static constexpr std::array<VkFormat, 12> kCustomClearColorCompressedFormatsNVIDIA = {
    VK_FORMAT_R8G8B8A8_UNORM,           VK_FORMAT_B8G8R8A8_UNORM,           VK_FORMAT_A8B8G8R8_UNORM_PACK32,
    VK_FORMAT_A2R10G10B10_UNORM_PACK32, VK_FORMAT_A2B10G10R10_UNORM_PACK32, VK_FORMAT_R16G16B16A16_UNORM,
    VK_FORMAT_R16G16B16A16_SNORM,       VK_FORMAT_R16G16B16A16_UINT,        VK_FORMAT_R16G16B16A16_SINT,
    VK_FORMAT_R16G16B16A16_SFLOAT,      VK_FORMAT_R32G32B32A32_SFLOAT,      VK_FORMAT_B10G11R11_UFLOAT_PACK32,
};

ReadLockGuard BestPractices::ReadLock() const {
    if (fine_grained_locking) {
        return ReadLockGuard(validation_object_mutex, std::defer_lock);
    } else {
        return ReadLockGuard(validation_object_mutex);
    }
}

WriteLockGuard BestPractices::WriteLock() {
    if (fine_grained_locking) {
        return WriteLockGuard(validation_object_mutex, std::defer_lock);
    } else {
        return WriteLockGuard(validation_object_mutex);
    }
}

std::shared_ptr<CMD_BUFFER_STATE> BestPractices::CreateCmdBufferState(VkCommandBuffer cb,
                                                                      const VkCommandBufferAllocateInfo* pCreateInfo,
                                                                      const COMMAND_POOL_STATE* pool) {
    return std::static_pointer_cast<CMD_BUFFER_STATE>(std::make_shared<bp_state::CommandBuffer>(this, cb, pCreateInfo, pool));
}

bp_state::CommandBuffer::CommandBuffer(BestPractices* bp, VkCommandBuffer cb, const VkCommandBufferAllocateInfo* pCreateInfo,
                                       const COMMAND_POOL_STATE* pool)
    : CMD_BUFFER_STATE(bp, cb, pCreateInfo, pool) {}

bool BestPractices::VendorCheckEnabled(BPVendorFlags vendors) const {
    for (const auto& vendor : kVendorInfo) {
        if (vendors & vendor.first && enabled[vendor.second.vendor_id]) {
            return true;
        }
    }
    return false;
}

const char* BestPractices::VendorSpecificTag(BPVendorFlags vendors) const {
    // Cache built vendor tags in a map
    static vvl::unordered_map<BPVendorFlags, std::string> tag_map;

    auto res = tag_map.find(vendors);
    if (res == tag_map.end()) {
        // Build the vendor tag string
        std::stringstream vendor_tag;

        vendor_tag << "[";
        bool first_vendor = true;
        for (const auto& vendor : kVendorInfo) {
            if (vendors & vendor.first) {
                if (!first_vendor) {
                    vendor_tag << ", ";
                }
                vendor_tag << vendor.second.name;
                first_vendor = false;
            }
        }
        vendor_tag << "]";

        tag_map[vendors] = vendor_tag.str();
        res = tag_map.find(vendors);
    }

    return res->second.c_str();
}

// Despite the return code being successful this can be a useful utility for some developers in niche debugging situation.
void BestPractices::LogPositiveSuccessCode(const RecordObject& record_obj) const {
    assert(record_obj.result > VK_SUCCESS);

    LogVerbose(kVUID_BestPractices_Verbose_Success_Logging, instance, record_obj.location, "Returned %s.",
               string_VkResult(record_obj.result));
}

void BestPractices::LogErrorCode(const RecordObject& record_obj) const {
    assert(record_obj.result < VK_SUCCESS);  // Anything less than VK_SUCCESS is an error.

    // Despite being error codes log these results as informational.
    // That is because they are returned frequently during window resizing.
    // They are expected to occur during the normal application lifecycle.
    constexpr std::array common_failure_codes = {VK_ERROR_OUT_OF_DATE_KHR, VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT};
    const auto result_string = string_VkResult(record_obj.result);

    if (IsValueIn(record_obj.result, common_failure_codes)) {
        LogInfo(kVUID_BestPractices_Failure_Result, instance, record_obj.location, "Returned error %s.", result_string);
    } else {
        LogWarning(kVUID_BestPractices_Error_Result, instance, record_obj.location, "Returned error %s.", result_string);
    }
}

void BestPractices::RecordSetDepthTestState(bp_state::CommandBuffer& cmd_state, VkCompareOp new_depth_compare_op,
                                            bool new_depth_test_enable) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    if (cmd_state.nv.depth_compare_op != new_depth_compare_op) {
        switch (new_depth_compare_op) {
            case VK_COMPARE_OP_LESS:
            case VK_COMPARE_OP_LESS_OR_EQUAL:
                cmd_state.nv.zcull_direction = bp_state::CommandBufferStateNV::ZcullDirection::Less;
                break;
            case VK_COMPARE_OP_GREATER:
            case VK_COMPARE_OP_GREATER_OR_EQUAL:
                cmd_state.nv.zcull_direction = bp_state::CommandBufferStateNV::ZcullDirection::Greater;
                break;
            default:
                // The other ops carry over the previous state.
                break;
        }
    }
    cmd_state.nv.depth_compare_op = new_depth_compare_op;
    cmd_state.nv.depth_test_enable = new_depth_test_enable;
}

void BestPractices::RecordBindZcullScope(bp_state::CommandBuffer& cmd_state, VkImage depth_attachment,
                                         const VkImageSubresourceRange& subresource_range) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    if (depth_attachment == VK_NULL_HANDLE) {
        cmd_state.nv.zcull_scope = {};
        return;
    }

    assert((subresource_range.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) != 0U);

    auto image_state = Get<IMAGE_STATE>(depth_attachment);
    assert(image_state);

    const uint32_t mip_levels = image_state->createInfo.mipLevels;
    const uint32_t array_layers = image_state->createInfo.arrayLayers;

    auto& tree = cmd_state.nv.zcull_per_image[depth_attachment];
    if (tree.states.empty()) {
        tree.mip_levels = mip_levels;
        tree.array_layers = array_layers;
        tree.states.resize(array_layers * mip_levels);
    }

    cmd_state.nv.zcull_scope.image = depth_attachment;
    cmd_state.nv.zcull_scope.range = subresource_range;
    cmd_state.nv.zcull_scope.tree = &tree;
}

void BestPractices::RecordUnbindZcullScope(bp_state::CommandBuffer& cmd_state) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    RecordBindZcullScope(cmd_state, VK_NULL_HANDLE, VkImageSubresourceRange{});
}

void BestPractices::RecordResetScopeZcullDirection(bp_state::CommandBuffer& cmd_state) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    auto& scope = cmd_state.nv.zcull_scope;
    RecordResetZcullDirection(cmd_state, scope.image, scope.range);
}

template <typename Func>
static void ForEachSubresource(const IMAGE_STATE& image, const VkImageSubresourceRange& range, Func&& func) {
    const uint32_t layerCount =
        (range.layerCount == VK_REMAINING_ARRAY_LAYERS) ? (image.full_range.layerCount - range.baseArrayLayer) : range.layerCount;
    const uint32_t levelCount =
        (range.levelCount == VK_REMAINING_MIP_LEVELS) ? (image.full_range.levelCount - range.baseMipLevel) : range.levelCount;

    for (uint32_t i = 0; i < layerCount; ++i) {
        const uint32_t layer = range.baseArrayLayer + i;
        for (uint32_t j = 0; j < levelCount; ++j) {
            const uint32_t level = range.baseMipLevel + j;
            func(layer, level);
        }
    }
}

void BestPractices::RecordResetZcullDirection(bp_state::CommandBuffer& cmd_state, VkImage depth_image,
                                              const VkImageSubresourceRange& subresource_range) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    RecordSetZcullDirection(cmd_state, depth_image, subresource_range, bp_state::CommandBufferStateNV::ZcullDirection::Unknown);

    const auto image_it = cmd_state.nv.zcull_per_image.find(depth_image);
    if (image_it == cmd_state.nv.zcull_per_image.end()) {
        return;
    }
    auto& tree = image_it->second;

    auto image = Get<IMAGE_STATE>(depth_image);
    if (!image) return;

    ForEachSubresource(*image, subresource_range, [&tree](uint32_t layer, uint32_t level) {
        auto& subresource = tree.GetState(layer, level);
        subresource.num_less_draws = 0;
        subresource.num_greater_draws = 0;
    });
}

void BestPractices::RecordSetScopeZcullDirection(bp_state::CommandBuffer& cmd_state,
                                                 bp_state::CommandBufferStateNV::ZcullDirection mode) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    auto& scope = cmd_state.nv.zcull_scope;
    RecordSetZcullDirection(cmd_state, scope.image, scope.range, mode);
}

void BestPractices::RecordSetZcullDirection(bp_state::CommandBuffer& cmd_state, VkImage depth_image,
                                            const VkImageSubresourceRange& subresource_range,
                                            bp_state::CommandBufferStateNV::ZcullDirection mode) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    const auto image_it = cmd_state.nv.zcull_per_image.find(depth_image);
    if (image_it == cmd_state.nv.zcull_per_image.end()) {
        return;
    }
    auto& tree = image_it->second;

    auto image = Get<IMAGE_STATE>(depth_image);
    if (!image) return;

    ForEachSubresource(*image, subresource_range, [&tree, &cmd_state](uint32_t layer, uint32_t level) {
        tree.GetState(layer, level).direction = cmd_state.nv.zcull_direction;
    });
}

void BestPractices::RecordZcullDraw(bp_state::CommandBuffer& cmd_state) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    // Add one draw to each subresource depending on the current Z-cull direction
    auto& scope = cmd_state.nv.zcull_scope;

    auto image = Get<IMAGE_STATE>(scope.image);
    if (!image) return;

    ForEachSubresource(*image, scope.range, [&scope](uint32_t layer, uint32_t level) {
        auto& subresource = scope.tree->GetState(layer, level);

        switch (subresource.direction) {
            case bp_state::CommandBufferStateNV::ZcullDirection::Unknown:
                // Unreachable
                assert(0);
                break;
            case bp_state::CommandBufferStateNV::ZcullDirection::Less:
                ++subresource.num_less_draws;
                break;
            case bp_state::CommandBufferStateNV::ZcullDirection::Greater:
                ++subresource.num_greater_draws;
                break;
        }
    });
}

bool BestPractices::ValidateZcullScope(const bp_state::CommandBuffer& cmd_state, const Location& loc) const {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    bool skip = false;

    if (cmd_state.nv.depth_test_enable) {
        auto& scope = cmd_state.nv.zcull_scope;
        skip |= ValidateZcull(cmd_state, scope.image, scope.range, loc);
    }

    return skip;
}

bool BestPractices::ValidateZcull(const bp_state::CommandBuffer& cmd_state, VkImage image,
                                  const VkImageSubresourceRange& subresource_range, const Location& loc) const {
    bool skip = false;

    const char* good_mode = nullptr;
    const char* bad_mode = nullptr;
    bool is_balanced = false;

    const auto image_it = cmd_state.nv.zcull_per_image.find(image);
    if (image_it == cmd_state.nv.zcull_per_image.end()) {
        return skip;
    }
    const auto& tree = image_it->second;

    auto image_state = Get<IMAGE_STATE>(image);
    if (!image_state) {
        return skip;
    }

    ForEachSubresource(*image_state, subresource_range, [&](uint32_t layer, uint32_t level) {
        if (is_balanced) {
            return;
        }
        const auto& resource = tree.GetState(layer, level);
        const uint64_t num_draws = resource.num_less_draws + resource.num_greater_draws;

        if (num_draws == 0) {
            return;
        }
        const uint64_t less_ratio = (resource.num_less_draws * 100) / num_draws;
        const uint64_t greater_ratio = (resource.num_greater_draws * 100) / num_draws;

        if ((less_ratio > kZcullDirectionBalanceRatioNVIDIA) && (greater_ratio > kZcullDirectionBalanceRatioNVIDIA)) {
            is_balanced = true;

            if (greater_ratio > less_ratio) {
                good_mode = "GREATER";
                bad_mode = "LESS";
            } else {
                good_mode = "LESS";
                bad_mode = "GREATER";
            }
        }
    });

    if (is_balanced) {
        skip |= LogPerformanceWarning(
            kVUID_BestPractices_Zcull_LessGreaterRatio, cmd_state.commandBuffer(), loc,
            "%s Depth attachment %s is primarily rendered with depth compare op %s, but some draws use %s. "
            "Z-cull is disabled for the least used direction, which harms depth testing performance. "
            "The Z-cull direction can be reset by clearing the depth attachment, transitioning from VK_IMAGE_LAYOUT_UNDEFINED, "
            "using VK_ATTACHMENT_LOAD_OP_DONT_CARE, or using VK_ATTACHMENT_STORE_OP_DONT_CARE.",
            VendorSpecificTag(kBPVendorNVIDIA), FormatHandle(cmd_state.nv.zcull_scope.image).c_str(), good_mode, bad_mode);
    }

    return skip;
}

static std::array<uint32_t, 4> GetRawClearColor(VkFormat format, const VkClearColorValue& clear_value) {
    std::array<uint32_t, 4> raw_color{};
    std::copy_n(clear_value.uint32, raw_color.size(), raw_color.data());

    // Zero out unused components to avoid polluting the cache with garbage
    if (!vkuFormatHasRed(format)) raw_color[0] = 0;
    if (!vkuFormatHasGreen(format)) raw_color[1] = 0;
    if (!vkuFormatHasBlue(format)) raw_color[2] = 0;
    if (!vkuFormatHasAlpha(format)) raw_color[3] = 0;

    return raw_color;
}

static bool IsClearColorZeroOrOne(VkFormat format, const std::array<uint32_t, 4> clear_color) {
    static_assert(sizeof(float) == sizeof(uint32_t), "Mismatching float <-> uint32 sizes");
    const float one = 1.0f;
    const float zero = 0.0f;
    uint32_t raw_one{};
    uint32_t raw_zero{};
    memcpy(&raw_one, &one, sizeof(one));
    memcpy(&raw_zero, &zero, sizeof(zero));

    const bool is_one =
        (!vkuFormatHasRed(format) || (clear_color[0] == raw_one)) && (!vkuFormatHasGreen(format) || (clear_color[1] == raw_one)) &&
        (!vkuFormatHasBlue(format) || (clear_color[2] == raw_one)) && (!vkuFormatHasAlpha(format) || (clear_color[3] == raw_one));
    const bool is_zero =
        (!vkuFormatHasRed(format) || (clear_color[0] == raw_zero)) && (!vkuFormatHasGreen(format) || (clear_color[1] == raw_zero)) &&
        (!vkuFormatHasBlue(format) || (clear_color[2] == raw_zero)) && (!vkuFormatHasAlpha(format) || (clear_color[3] == raw_zero));
    return is_one || is_zero;
}

static std::string MakeCompressedFormatListNVIDIA() {
    std::string format_list;
    for (VkFormat compressed_format : kCustomClearColorCompressedFormatsNVIDIA) {
        if (compressed_format == kCustomClearColorCompressedFormatsNVIDIA.back()) {
            format_list += "or ";
        }
        format_list += string_VkFormat(compressed_format);
        if (compressed_format != kCustomClearColorCompressedFormatsNVIDIA.back()) {
            format_list += ", ";
        }
    }
    return format_list;
}

void BestPractices::RecordClearColor(VkFormat format, const VkClearColorValue& clear_value) {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    const std::array<uint32_t, 4> raw_color = GetRawClearColor(format, clear_value);
    if (IsClearColorZeroOrOne(format, raw_color)) {
        // These colors are always compressed
        return;
    }

    const auto it =
        std::find(kCustomClearColorCompressedFormatsNVIDIA.begin(), kCustomClearColorCompressedFormatsNVIDIA.end(), format);
    if (it == kCustomClearColorCompressedFormatsNVIDIA.end()) {
        // The format cannot be compressed with a custom color
        return;
    }

    // Record custom clear color
    WriteLockGuard guard{clear_colors_lock_};
    if (clear_colors_.size() < kMaxRecommendedNumberOfClearColorsNVIDIA) {
        clear_colors_.insert(raw_color);
    }
}

bool BestPractices::ValidateClearColor(VkCommandBuffer commandBuffer, VkFormat format, const VkClearColorValue& clear_value,
                                       const Location& loc) const {
    assert(VendorCheckEnabled(kBPVendorNVIDIA));

    bool skip = false;

    const std::array<uint32_t, 4> raw_color = GetRawClearColor(format, clear_value);
    if (IsClearColorZeroOrOne(format, raw_color)) {
        return skip;
    }

    const auto it =
        std::find(kCustomClearColorCompressedFormatsNVIDIA.begin(), kCustomClearColorCompressedFormatsNVIDIA.end(), format);
    if (it == kCustomClearColorCompressedFormatsNVIDIA.end()) {
        // The format is not compressible
        static const std::string format_list = MakeCompressedFormatListNVIDIA();

        skip |= LogPerformanceWarning(kVUID_BestPractices_ClearColor_NotCompressed, commandBuffer, loc,
                                      "%s Clearing image with format %s without a 1.0f or 0.0f clear color. "
                                      "The clear will not get compressed in the GPU, harming performance. "
                                      "This can be fixed using a clear color of VkClearColorValue{0.0f, 0.0f, 0.0f, 0.0f}, or "
                                      "VkClearColorValue{1.0f, 1.0f, 1.0f, 1.0f}. Alternatively, use %s.",
                                      VendorSpecificTag(kBPVendorNVIDIA), string_VkFormat(format), format_list.c_str());
    } else {
        // The format is compressible
        bool registered = false;
        {
            ReadLockGuard guard{clear_colors_lock_};
            registered = clear_colors_.find(raw_color) != clear_colors_.end();

            if (!registered) {
                // If it's not in the list, it might be new. Check if there's still space for new entries.
                registered = clear_colors_.size() < kMaxRecommendedNumberOfClearColorsNVIDIA;
            }
        }
        if (!registered) {
            std::string clear_color_str;

            if (vkuFormatIsUINT(format)) {
                clear_color_str = std::to_string(clear_value.uint32[0]) + ", " + std::to_string(clear_value.uint32[1]) + ", " +
                                  std::to_string(clear_value.uint32[2]) + ", " + std::to_string(clear_value.uint32[3]);
            } else if (vkuFormatIsSINT(format)) {
                clear_color_str = std::to_string(clear_value.int32[0]) + ", " + std::to_string(clear_value.int32[1]) + ", " +
                                  std::to_string(clear_value.int32[2]) + ", " + std::to_string(clear_value.int32[3]);
            } else {
                clear_color_str = std::to_string(clear_value.float32[0]) + ", " + std::to_string(clear_value.float32[1]) + ", " +
                                  std::to_string(clear_value.float32[2]) + ", " + std::to_string(clear_value.float32[3]);
            }

            skip |= LogPerformanceWarning(
                kVUID_BestPractices_ClearColor_NotCompressed, commandBuffer, loc,
                "%s Clearing image with unregistered VkClearColorValue{%s}. "
                "This clear will not get compressed in the GPU, harming performance. "
                "The clear color is not registered because too many unique colors have been used. "
                "Select a discrete set of clear colors and stick to those. "
                "VkClearColorValue{0, 0, 0, 0} and VkClearColorValue{1.0f, 1.0f, 1.0f, 1.0f} are always registered.",
                VendorSpecificTag(kBPVendorNVIDIA), clear_color_str.c_str());
        }
    }

    return skip;
}

void BestPractices::QueueValidateImageView(QueueCallbacks& funcs, Func command, IMAGE_VIEW_STATE* view,
                                           IMAGE_SUBRESOURCE_USAGE_BP usage) {
    if (view) {
        auto image_state = std::static_pointer_cast<bp_state::Image>(view->image_state);
        QueueValidateImage(funcs, command, image_state, usage, view->normalized_subresource_range);
    }
}

void BestPractices::QueueValidateImage(QueueCallbacks& funcs, Func command, std::shared_ptr<bp_state::Image>& state,
                                       IMAGE_SUBRESOURCE_USAGE_BP usage, const VkImageSubresourceRange& subresource_range) {
    // If we're viewing a 3D slice, ignore base array layer.
    // The entire 3D subresource is accessed as one atomic unit.
    const uint32_t base_array_layer = state->createInfo.imageType == VK_IMAGE_TYPE_3D ? 0 : subresource_range.baseArrayLayer;

    const uint32_t max_layers = state->createInfo.arrayLayers - base_array_layer;
    const uint32_t array_layers = std::min(subresource_range.layerCount, max_layers);
    const uint32_t max_levels = state->createInfo.mipLevels - subresource_range.baseMipLevel;
    const uint32_t mip_levels = std::min(state->createInfo.mipLevels, max_levels);

    for (uint32_t layer = 0; layer < array_layers; layer++) {
        for (uint32_t level = 0; level < mip_levels; level++) {
            QueueValidateImage(funcs, command, state, usage, layer + base_array_layer, level + subresource_range.baseMipLevel);
        }
    }
}

void BestPractices::QueueValidateImage(QueueCallbacks& funcs, Func command, std::shared_ptr<bp_state::Image>& state,
                                       IMAGE_SUBRESOURCE_USAGE_BP usage, const VkImageSubresourceLayers& subresource_layers) {
    const uint32_t max_layers = state->createInfo.arrayLayers - subresource_layers.baseArrayLayer;
    const uint32_t array_layers = std::min(subresource_layers.layerCount, max_layers);

    for (uint32_t layer = 0; layer < array_layers; layer++) {
        QueueValidateImage(funcs, command, state, usage, layer + subresource_layers.baseArrayLayer, subresource_layers.mipLevel);
    }
}

void BestPractices::QueueValidateImage(QueueCallbacks& funcs, Func command, std::shared_ptr<bp_state::Image>& state,
                                       IMAGE_SUBRESOURCE_USAGE_BP usage, uint32_t array_layer, uint32_t mip_level) {
    funcs.push_back([this, command, state, usage, array_layer, mip_level](const ValidationStateTracker& vst, const QUEUE_STATE& qs,
                                                                          const CMD_BUFFER_STATE& cbs) -> bool {
        ValidateImageInQueue(qs, cbs, command, *state, usage, array_layer, mip_level);
        return false;
    });
}

void BestPractices::ValidateImageInQueueArmImg(Func command, const bp_state::Image& image, IMAGE_SUBRESOURCE_USAGE_BP last_usage,
                                               IMAGE_SUBRESOURCE_USAGE_BP usage, uint32_t array_layer, uint32_t mip_level) {
    // Swapchain images are implicitly read so clear after store is expected.
    const Location loc(command);
    if (usage == IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_CLEARED && last_usage == IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_STORED &&
        !image.IsSwapchainImage()) {
        LogPerformanceWarning(
            kVUID_BestPractices_RenderPass_RedundantStore, device, loc,
            "%s %s Subresource (arrayLayer: %u, mipLevel: %u) of image was cleared as part of LOAD_OP_CLEAR, but last time "
            "image was used, it was written to with STORE_OP_STORE. "
            "Storing to the image is probably redundant in this case, and wastes bandwidth on tile-based "
            "architectures.",
            VendorSpecificTag(kBPVendorArm), VendorSpecificTag(kBPVendorIMG), array_layer, mip_level);
    } else if (usage == IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_CLEARED && last_usage == IMAGE_SUBRESOURCE_USAGE_BP::CLEARED) {
        LogPerformanceWarning(
            kVUID_BestPractices_RenderPass_RedundantClear, device, loc,
            "%s %s Subresource (arrayLayer: %u, mipLevel: %u) of image was cleared as part of LOAD_OP_CLEAR, but last time "
            "image was used, it was written to with vkCmdClear*Image(). "
            "Clearing the image with vkCmdClear*Image() is probably redundant in this case, and wastes bandwidth on "
            "tile-based architectures.",
            VendorSpecificTag(kBPVendorArm), VendorSpecificTag(kBPVendorIMG), array_layer, mip_level);
    } else if (usage == IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_READ_TO_TILE &&
               (last_usage == IMAGE_SUBRESOURCE_USAGE_BP::BLIT_WRITE || last_usage == IMAGE_SUBRESOURCE_USAGE_BP::CLEARED ||
                last_usage == IMAGE_SUBRESOURCE_USAGE_BP::COPY_WRITE || last_usage == IMAGE_SUBRESOURCE_USAGE_BP::RESOLVE_WRITE)) {
        const char* last_cmd = nullptr;
        const char* vuid = nullptr;
        const char* suggestion = nullptr;

        switch (last_usage) {
            case IMAGE_SUBRESOURCE_USAGE_BP::BLIT_WRITE:
                vuid = kVUID_BestPractices_RenderPass_BlitImage_LoadOpLoad;
                last_cmd = "vkCmdBlitImage";
                suggestion =
                    "The blit is probably redundant in this case, and wastes bandwidth on tile-based architectures. "
                    "Rather than blitting, just render the source image in a fragment shader in this render pass, "
                    "which avoids the memory roundtrip.";
                break;
            case IMAGE_SUBRESOURCE_USAGE_BP::CLEARED:
                vuid = kVUID_BestPractices_RenderPass_InefficientClear;
                last_cmd = "vkCmdClear*Image";
                suggestion =
                    "Clearing the image with vkCmdClear*Image() is probably redundant in this case, and wastes bandwidth on "
                    "tile-based architectures. "
                    "Use LOAD_OP_CLEAR instead to clear the image for free.";
                break;
            case IMAGE_SUBRESOURCE_USAGE_BP::COPY_WRITE:
                vuid = kVUID_BestPractices_RenderPass_CopyImage_LoadOpLoad;
                last_cmd = "vkCmdCopy*Image";
                suggestion =
                    "The copy is probably redundant in this case, and wastes bandwidth on tile-based architectures. "
                    "Rather than copying, just render the source image in a fragment shader in this render pass, "
                    "which avoids the memory roundtrip.";
                break;
            case IMAGE_SUBRESOURCE_USAGE_BP::RESOLVE_WRITE:
                vuid = kVUID_BestPractices_RenderPass_ResolveImage_LoadOpLoad;
                last_cmd = "vkCmdResolveImage";
                suggestion =
                    "The resolve is probably redundant in this case, and wastes a lot of bandwidth on tile-based architectures. "
                    "Rather than resolving, and then loading, try to keep rendering in the same render pass, "
                    "which avoids the memory roundtrip.";
                break;
            default:
                break;
        }

        LogPerformanceWarning(
            vuid, device, loc,
            "%s %s Subresource (arrayLayer: %u, mipLevel: %u) of image was loaded to tile as part of LOAD_OP_LOAD, but last "
            "time image was used, it was written to with %s. %s",
            VendorSpecificTag(kBPVendorArm), VendorSpecificTag(kBPVendorIMG), array_layer, mip_level, last_cmd, suggestion);
    }
}

void BestPractices::ValidateImageInQueue(const QUEUE_STATE& qs, const CMD_BUFFER_STATE& cbs, Func command, bp_state::Image& state,
                                         IMAGE_SUBRESOURCE_USAGE_BP usage, uint32_t array_layer, uint32_t mip_level) {
    auto queue_family = qs.queueFamilyIndex;
    auto last_usage = state.UpdateUsage(array_layer, mip_level, usage, queue_family);

    // Concurrent sharing usage of image with exclusive sharing mode
    if (state.createInfo.sharingMode == VK_SHARING_MODE_EXCLUSIVE && last_usage.queue_family_index != queue_family) {
        // if UNDEFINED then first use/acquisition of subresource
        if (last_usage.type != IMAGE_SUBRESOURCE_USAGE_BP::UNDEFINED) {
            // If usage might read from the subresource, as contents are undefined
            // so write only is fine
            if (usage == IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_READ_TO_TILE || usage == IMAGE_SUBRESOURCE_USAGE_BP::BLIT_READ ||
                usage == IMAGE_SUBRESOURCE_USAGE_BP::COPY_READ || usage == IMAGE_SUBRESOURCE_USAGE_BP::DESCRIPTOR_ACCESS ||
                usage == IMAGE_SUBRESOURCE_USAGE_BP::RESOLVE_READ) {
                Location loc(command);
                LogWarning(
                    kVUID_BestPractices_ConcurrentUsageOfExclusiveImage, state.image(), loc,
                    "Subresource (arrayLayer: %" PRIu32 ", mipLevel: %" PRIu32 ") of image is used on queue family index %" PRIu32
                    " after being used on "
                    "queue family index %" PRIu32
                    ", "
                    "but has VK_SHARING_MODE_EXCLUSIVE, and has not been acquired and released with a ownership transfer operation",
                    array_layer, mip_level, queue_family, last_usage.queue_family_index);
            }
        }
    }

    // When image was discarded with StoreOpDontCare but is now being read with LoadOpLoad
    if (last_usage.type == IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_DISCARDED &&
        usage == IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_READ_TO_TILE) {
        Location loc(command);
        LogWarning(kVUID_BestPractices_StoreOpDontCareThenLoadOpLoad, device, loc,
                   "Trying to load an attachment with LOAD_OP_LOAD that was previously stored with STORE_OP_DONT_CARE. This may "
                   "result in undefined behaviour.");
    }

    if (VendorCheckEnabled(kBPVendorArm) || VendorCheckEnabled(kBPVendorIMG)) {
        ValidateImageInQueueArmImg(command, state, last_usage.type, usage, array_layer, mip_level);
    }
}

void BestPractices::AddDeferredQueueOperations(bp_state::CommandBuffer& cb) {
    cb.queue_submit_functions.insert(cb.queue_submit_functions.end(), cb.queue_submit_functions_after_render_pass.begin(),
                                     cb.queue_submit_functions_after_render_pass.end());
    cb.queue_submit_functions_after_render_pass.clear();
}

void BestPractices::RecordAttachmentAccess(bp_state::CommandBuffer& cb_state, uint32_t fb_attachment, VkImageAspectFlags aspects) {
    auto& state = cb_state.render_pass_state;
    // Called when we have a partial clear attachment, or a normal draw call which accesses an attachment.
    auto itr =
        std::find_if(state.touchesAttachments.begin(), state.touchesAttachments.end(),
                     [fb_attachment](const bp_state::AttachmentInfo& info) { return info.framebufferAttachment == fb_attachment; });

    if (itr != state.touchesAttachments.end()) {
        itr->aspects |= aspects;
    } else {
        state.touchesAttachments.push_back({fb_attachment, aspects});
    }
}

void BestPractices::RecordAttachmentClearAttachments(bp_state::CommandBuffer& cmd_state, uint32_t fb_attachment,
                                                     uint32_t color_attachment, VkImageAspectFlags aspects, uint32_t rectCount,
                                                     const VkClearRect* pRects) {
    auto& state = cmd_state.render_pass_state;
    // If we observe a full clear before any other access to a frame buffer attachment,
    // we have candidate for redundant clear attachments.
    auto itr =
        std::find_if(state.touchesAttachments.begin(), state.touchesAttachments.end(),
                     [fb_attachment](const bp_state::AttachmentInfo& info) { return info.framebufferAttachment == fb_attachment; });

    uint32_t new_aspects = aspects;
    if (itr != state.touchesAttachments.end()) {
        new_aspects = aspects & ~itr->aspects;
        itr->aspects |= aspects;
    } else {
        state.touchesAttachments.push_back({fb_attachment, aspects});
    }

    if (new_aspects == 0) {
        return;
    }

    if (cmd_state.createInfo.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY) {
        // The first command might be a clear, but might not be the first in the render pass, defer any checks until
        // CmdExecuteCommands.
        state.earlyClearAttachments.push_back(
            {fb_attachment, color_attachment, new_aspects, std::vector<VkClearRect>{pRects, pRects + rectCount}});
    }
}
