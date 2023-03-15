/* Copyright (c) 2019-2023 The Khronos Group Inc.
 * Copyright (c) 2019-2023 Valve Corporation
 * Copyright (c) 2019-2023 LunarG, Inc.
 * Copyright (C) 2019-2023 Google Inc.
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
#include <memory>
#include <vector>

#include "containers/range_vector.h"
#include "containers/subresource_adapter.h"
#ifndef SPARSE_CONTAINER_UNIT_TEST
#include "vulkan/vulkan.h"
#include "error_message/logging.h"

// Forward declarations...
class CMD_BUFFER_STATE;
class IMAGE_STATE;
class IMAGE_VIEW_STATE;
#endif

namespace image_layout_map {
const static VkImageLayout kInvalidLayout = VK_IMAGE_LAYOUT_MAX_ENUM;

// Common types for this namespace
using IndexType = subresource_adapter::IndexType;
using IndexRange = sparse_container::range<IndexType>;
using Encoder = subresource_adapter::RangeEncoder;
using RangeGenerator = subresource_adapter::RangeGenerator;

struct InitialLayoutState {
    VkImageView image_view;          // For relaxed matching rule evaluation, else VK_NULL_HANDLE
    VkImageAspectFlags aspect_mask;  // For relaxed matching rules... else 0
    LoggingLabel label;
    InitialLayoutState(const CMD_BUFFER_STATE& cb_state_, const IMAGE_VIEW_STATE* view_state_);
    InitialLayoutState() : image_view(VK_NULL_HANDLE), aspect_mask(0), label() {}
};

class ImageSubresourceLayoutMap {
  public:
    typedef std::function<bool(const VkImageSubresource&, VkImageLayout, VkImageLayout)> Callback;

    struct SubresourceLayout {
        VkImageSubresource subresource;
        VkImageLayout current_layout;
        VkImageLayout initial_layout;

        bool operator==(const SubresourceLayout& rhs) const;
        bool operator!=(const SubresourceLayout& rhs) const { return !(*this == rhs); }
        SubresourceLayout(const VkImageSubresource& subresource_, VkImageLayout current_layout_, VkImageLayout initial_layout_)
            : subresource(subresource_), current_layout(current_layout_), initial_layout(initial_layout_) {}
        SubresourceLayout() = default;
    };

    struct LayoutEntry {
        VkImageLayout initial_layout;
        VkImageLayout current_layout;
        InitialLayoutState* state;

        LayoutEntry(VkImageLayout initial_ = kInvalidLayout, VkImageLayout current_ = kInvalidLayout,
                    InitialLayoutState* s = nullptr)
            : initial_layout(initial_), current_layout(current_), state(s) {}

        bool operator!=(const LayoutEntry& rhs) const {
            return initial_layout != rhs.initial_layout || current_layout != rhs.current_layout || state != rhs.state;
        }
        bool CurrentWillChange(VkImageLayout new_layout) const {
            return new_layout != kInvalidLayout && current_layout != new_layout;
        }
        bool Update(const LayoutEntry& src) {
            bool updated_current = false;
            // current_layout can be updated repeatedly.
            if (CurrentWillChange(src.current_layout)) {
                current_layout = src.current_layout;
                updated_current = true;
            }
            // initial_layout and state cannot be updated once they have a valid value.
            if (initial_layout == kInvalidLayout) {
                initial_layout = src.initial_layout;
            }
            if (state == nullptr) {
                state = src.state;
            }
            return updated_current;
        }
        // updater for splice()
        struct Updater {
            bool update(LayoutEntry& dst, const LayoutEntry& src) const { return dst.Update(src); }
            std::optional<LayoutEntry> insert(const LayoutEntry& src) const {
                return std::optional<LayoutEntry>(vvl::in_place, src);
            }
        };
    };
    using InitialLayoutStates = small_vector<InitialLayoutState, 2, uint32_t>;
    using LayoutMap = subresource_adapter::BothRangeMap<LayoutEntry, 16>;
    using RangeType = LayoutMap::key_type;

    bool SetSubresourceRangeLayout(const CMD_BUFFER_STATE& cb_state, const VkImageSubresourceRange& range, VkImageLayout layout,
                                   VkImageLayout expected_layout = kInvalidLayout);
    void SetSubresourceRangeInitialLayout(const CMD_BUFFER_STATE& cb_state, const VkImageSubresourceRange& range,
                                          VkImageLayout layout);
    void SetSubresourceRangeInitialLayout(const CMD_BUFFER_STATE& cb_state, VkImageLayout layout,
                                          const IMAGE_VIEW_STATE& view_state);
    bool UpdateFrom(const ImageSubresourceLayoutMap& from);
    uintptr_t CompatibilityKey() const;
    const LayoutMap& GetLayoutMap() const { return layouts_; }
    ImageSubresourceLayoutMap(const IMAGE_STATE& image_state);
    ~ImageSubresourceLayoutMap() {}
    const IMAGE_STATE* GetImageView() const { return &image_state_; };

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
            for (auto pos = layouts_.lower_bound(*gen); (pos != layouts_.end()) && (gen->intersects(pos->first)); ++pos) {
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
    const IMAGE_STATE& image_state_;
    const Encoder& encoder_;
    LayoutMap layouts_;
    InitialLayoutStates initial_layout_states_;
};
}  // namespace image_layout_map
