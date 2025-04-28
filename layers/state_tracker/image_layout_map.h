/* Copyright (c) 2019-2025 The Khronos Group Inc.
 * Copyright (c) 2019-2025 Valve Corporation
 * Copyright (c) 2019-2025 LunarG, Inc.
 * Copyright (C) 2019-2025 Google Inc.
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
 * John Zulauf <jzulauf@lunarg.com>
 *
 */
#pragma once

#include <functional>

#include "containers/range.h"
#include "containers/subresource_adapter.h"
#include "containers/small_vector.h"
#include "containers/container_utils.h"
#include "utils/vk_layer_utils.h"
#include "error_message/logging.h"

namespace vvl {
class Image;
class ImageView;
class CommandBuffer;
}  // namespace vvl

namespace image_layout_map {
const static VkImageLayout kInvalidLayout = VK_IMAGE_LAYOUT_MAX_ENUM;

// Common types for this namespace
using IndexType = subresource_adapter::IndexType;
using IndexRange = vvl::range<IndexType>;
using Encoder = subresource_adapter::RangeEncoder;
using RangeGenerator = subresource_adapter::RangeGenerator;

// Contains all info around an image, its subresources and layout map
class ImageLayoutRegistry {
  public:
    struct LayoutEntry {
        // This tracks the first known layout of the subresource in the command buffer (that's why initial).
        // This value is tracked based on the expected layout parameters from various API functions.
        // For example, for vkCmdCopyImageToBuffer the expected layout is the srcImageLayout parameter,
        // and for image barrier it is the oldLayout.
        VkImageLayout initial_layout;

        // This tracks current subresource layout as we progress through the command buffer
        VkImageLayout current_layout;

        // For relaxed matching rules
        VkImageAspectFlags aspect_mask;

        // Initialize entry with the current layout. If API also defines the expected layout it can be specified as second parameter
        static LayoutEntry ForCurrentLayout(VkImageLayout current_layout, VkImageLayout expected_layout = kInvalidLayout);

        // Initialize entry with the expected layout. This is usually used by the APIs that do not perform layout transitions
        // but just manifest the expected layout, e.g. srcImageLayout parameter in vkCmdCopyImageToBuffer.
        // The aspect mask is used if API additionally restricts subresource to specific aspect (descriptor image views).
        static LayoutEntry ForExpectedLayout(VkImageLayout expected_layout, VkImageAspectFlags aspect_mask = 0);

        bool CurrentWillChange(VkImageLayout new_layout) const;
        bool Update(const LayoutEntry& src);

        // updater for splice()
        struct Updater {
            bool update(LayoutEntry& dst, const LayoutEntry& src) const { return dst.Update(src); }
            std::optional<LayoutEntry> insert(const LayoutEntry& src) const {
                return std::optional<LayoutEntry>(vvl::in_place, src);
            }
        };
    };
    using LayoutMap = subresource_adapter::BothRangeMap<LayoutEntry, 16>;
    using RangeType = LayoutMap::key_type;

    bool SetSubresourceRangeLayout(const VkImageSubresourceRange& range, VkImageLayout layout,
                                   VkImageLayout expected_layout = kInvalidLayout);
    void SetSubresourceRangeInitialLayout(const VkImageSubresourceRange& range, VkImageLayout layout);
    void SetSubresourceRangeInitialLayout(VkImageLayout layout, const vvl::ImageView& view_state);
    bool UpdateFrom(const ImageLayoutRegistry& from);
    uintptr_t CompatibilityKey() const;
    const LayoutMap& GetLayoutMap() const { return layout_map_; }
    ImageLayoutRegistry(const vvl::Image& image_state);
    ~ImageLayoutRegistry() {}
    uint32_t GetImageId() const;

    // This looks a bit ponderous but kAspectCount is a compile time constant
    VkImageSubresource Decode(IndexType index) const {
        const auto subres = encoder_.Decode(index);
        return encoder_.MakeVkSubresource(subres);
    }

    RangeGenerator RangeGen(const VkImageSubresourceRange& subres_range) const {
        if (encoder_.InRange(subres_range)) {
            return (RangeGenerator(encoder_, subres_range));
        }
        // Return empty range generator
        return RangeGenerator();
    }

    bool AnyInRange(const VkImageSubresourceRange& normalized_range,
                    std::function<bool(const RangeType& range, const LayoutEntry& state)>&& func) const {
        return AnyInRange(RangeGen(normalized_range), std::move(func));
    }

    bool AnyInRange(const RangeGenerator& gen, std::function<bool(const RangeType& range, const LayoutEntry& state)>&& func) const {
        return AnyInRange(RangeGenerator(gen), std::move(func));
    }

    bool AnyInRange(RangeGenerator&& gen, std::function<bool(const RangeType& range, const LayoutEntry& state)>&& func) const {
        for (; gen->non_empty(); ++gen) {
            for (auto pos = layout_map_.lower_bound(*gen); (pos != layout_map_.end()) && (gen->intersects(pos->first)); ++pos) {
                if (func(pos->first, pos->second)) {
                    return true;
                }
            }
        }
        return false;
    }

  protected:
    bool InRange(const VkImageSubresource& subres) const { return encoder_.InRange(subres); }
    bool InRange(const VkImageSubresourceRange& range) const { return encoder_.InRange(range); }

  private:
    const vvl::Image& image_state_;
    const Encoder& encoder_;
    LayoutMap layout_map_;
};
}  // namespace image_layout_map

class ImageLayoutRangeMap : public subresource_adapter::BothRangeMap<VkImageLayout, 16> {
  public:
    using RangeGenerator = image_layout_map::RangeGenerator;

    ImageLayoutRangeMap(index_type index) : BothRangeMap<VkImageLayout, 16>(index) {}
    ReadLockGuard ReadLock() const { return ReadLockGuard(*lock); }
    WriteLockGuard WriteLock() { return WriteLockGuard(*lock); }

    bool AnyInRange(RangeGenerator& gen, std::function<bool(const key_type& range, const mapped_type& state)>&& func) const;

    // Not null if this layout map is owned by the vvl::Image and points to vvl::Image::layout_range_map_lock.
    // The layout maps that are not owned by the images do not use locking functionality.
    std::shared_mutex* lock = nullptr;
};

using SubmissionImageLayoutMap = vvl::unordered_map<const vvl::Image*, std::optional<ImageLayoutRangeMap>>;

// Not declared in the CommandBuffer class to allow other files to forward reference this
// (was slow to have ever file need to compile in the Image Layout map)
using CommandBufferImageLayoutMap = vvl::unordered_map<VkImage, std::shared_ptr<image_layout_map::ImageLayoutRegistry>>;