/* Copyright (c) 2019-2021 The Khronos Group Inc.
 * Copyright (c) 2019-2021 Valve Corporation
 * Copyright (c) 2019-2021 LunarG, Inc.
 * Copyright (C) 2019-2021 Google Inc.
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
#ifndef IMAGE_LAYOUT_MAP_H_
#define IMAGE_LAYOUT_MAP_H_

#include <functional>
#include <memory>
#include <vector>

#include "range_vector.h"
#include "subresource_adapter.h"
#ifndef SPARSE_CONTAINER_UNIT_TEST
#ifdef VULKANSC
#include "vulkan/vulkan_sc.h"
#else
#include "vulkan/vulkan.h"
#endif
#include "vk_layer_logging.h"

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
using NoSplit = sparse_container::insert_range_no_split_bounds;
using RangeGenerator = subresource_adapter::RangeGenerator;
using SubresourceGenerator = subresource_adapter::SubresourceGenerator;
using WritePolicy = subresource_adapter::WritePolicy;

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
            layer_data::optional<LayoutEntry> insert(const LayoutEntry& src) const { return layer_data::optional<LayoutEntry>(layer_data::in_place, src); }
        };
    };
    using RangeMap = subresource_adapter::BothRangeMap<LayoutEntry, 16>;
    using LayoutMap = RangeMap;
    using InitialLayoutStates = small_vector<InitialLayoutState, 2, uint32_t>;

    class ConstIterator {
      public:
        ConstIterator& operator++() {
            Increment();
            return *this;
        }
        void IncrementInterval();
        const SubresourceLayout* operator->() const { return &pos_; }
        const SubresourceLayout& operator*() const { return pos_; }

        ConstIterator() : range_gen_(), layouts_(nullptr), iter_(), skip_invalid_(false), always_get_initial_(false), pos_() {}
        bool AtEnd() const { return pos_.subresource.aspectMask == 0; }

        // Only for comparisons to end()
        // Note: if a fully function == is needed, the AtEnd needs to be maintained, as end_iterator is a static.
        bool operator==(const ConstIterator& other) const { return AtEnd() && other.AtEnd(); };
        bool operator!=(const ConstIterator& other) const { return AtEnd() != other.AtEnd(); };

      protected:
        void Increment();
        friend ImageSubresourceLayoutMap;
        ConstIterator(const RangeMap& layouts, const Encoder& encoder, const VkImageSubresourceRange& subres, bool skip_invalid,
                      bool always_get_initial);
        void UpdateRangeAndValue();
        void ForceEndCondition() { pos_.subresource.aspectMask = 0; }

        RangeGenerator range_gen_;
        const RangeMap* layouts_;
        RangeMap::const_iterator iter_;
        bool skip_invalid_;
        bool always_get_initial_;
        SubresourceLayout pos_;
        IndexType current_index_ = 0;
        IndexType constant_value_bound_ = 0;
    };

    ConstIterator Find(const VkImageSubresourceRange& subres_range, bool skip_invalid = true,
                       bool always_get_initial = false) const {
        if (InRange(subres_range)) {
            return ConstIterator(layouts_, encoder_, subres_range, skip_invalid, always_get_initial);
        }
        return End();
    }

    // Begin is a find of the full range with the default skip/ always get parameters
    ConstIterator Begin(bool always_get_initial = true) const;
    inline ConstIterator begin() const { return Begin(); }  // STL style, for range based loops and familiarity
    const ConstIterator& End() const { return end_iterator; }
    const ConstIterator& end() const { return End(); }  // STL style, for range based loops and familiarity.

    bool SetSubresourceRangeLayout(const CMD_BUFFER_STATE& cb_state, const VkImageSubresourceRange& range, VkImageLayout layout,
                                   VkImageLayout expected_layout = kInvalidLayout);
    void SetSubresourceRangeInitialLayout(const CMD_BUFFER_STATE& cb_state, const VkImageSubresourceRange& range,
                                          VkImageLayout layout);
    void SetSubresourceRangeInitialLayout(const CMD_BUFFER_STATE& cb_state, VkImageLayout layout,
                                          const IMAGE_VIEW_STATE& view_state);
    const LayoutEntry* GetSubresourceLayouts(const VkImageSubresource& subresource) const;
    const InitialLayoutState* GetSubresourceInitialLayoutState(const IndexType index) const;
    const InitialLayoutState* GetSubresourceInitialLayoutState(const VkImageSubresource& subresource) const;
    bool UpdateFrom(const ImageSubresourceLayoutMap& from);
    uintptr_t CompatibilityKey() const;
    const LayoutMap& GetLayoutMap() const { return layouts_; }
    ImageSubresourceLayoutMap(const IMAGE_STATE& image_state);
    ~ImageSubresourceLayoutMap() {}
    const IMAGE_STATE* GetImageView() const { return &image_state_; };

  protected:
    // This looks a bit ponderous but kAspectCount is a compile time constant
    VkImageSubresource Decode(IndexType index) const {
        const auto subres = encoder_.Decode(index);
        return encoder_.MakeVkSubresource(subres);
    }

    inline uint32_t LevelLimit(uint32_t level) const { return std::min(encoder_.Limits().mipLevel, level); }
    inline uint32_t LayerLimit(uint32_t layer) const { return std::min(encoder_.Limits().arrayLayer, layer); }

    bool InRange(const VkImageSubresource& subres) const { return encoder_.InRange(subres); }
    bool InRange(const VkImageSubresourceRange& range) const { return encoder_.InRange(range); }

    // This map *also* needs "write once" semantics
    using InitialLayoutStateMap = subresource_adapter::BothRangeMap<InitialLayoutState*, 16>;

  private:
    const IMAGE_STATE& image_state_;
    const Encoder& encoder_;
    LayoutMap layouts_;
    InitialLayoutStates initial_layout_states_;

    static const ConstIterator end_iterator;  // Just to hold the end condition tombstone (aspectMask == 0)
};
}  // namespace image_layout_map
#endif
