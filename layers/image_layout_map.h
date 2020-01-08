/* Copyright (c) 2019-2020 The Khronos Group Inc.
 * Copyright (c) 2019-2020 Valve Corporation
 * Copyright (c) 2019-2020 LunarG, Inc.
 * Copyright (C) 2019-2020 Google Inc.
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
#include "vulkan/vulkan.h"
#include "vk_layer_logging.h"

// Forward declarations...
struct CMD_BUFFER_STATE;
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

    struct Layouts {
        VkImageLayout current_layout;
        VkImageLayout initial_layout;
    };

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

    struct SubresourceRangeLayout {
        VkImageSubresourceRange subresource_range;
        VkImageLayout current_layout;
        VkImageLayout initial_layout;
        bool operator==(const SubresourceRangeLayout& rhs) const {
            bool is_equal = (current_layout == rhs.current_layout) && (initial_layout == rhs.initial_layout) &&
                            (subresource_range == rhs.subresource_range);
            return is_equal;
        }
        bool operator!=(const SubresourceRangeLayout& rhs) const { return !(*this == rhs); }
    };

    using RangeMap = subresource_adapter::BothRangeMap<VkImageLayout, 16>;
    template <typename MapA, typename MapB>
    using ParallelIterator = sparse_container::parallel_iterator<MapA, MapB>;
    using LayoutMap = RangeMap;
    using InitialLayoutMap = RangeMap;
    using InitialLayoutStates = std::vector<std::unique_ptr<InitialLayoutState>>;

    class ConstIterator {
      public:
        ConstIterator& operator++() {
            Increment();
            return *this;
        }
        const SubresourceLayout* operator->() const { return &pos_; }
        const SubresourceLayout& operator*() const { return pos_; }

        ConstIterator() : range_gen_(), parallel_it_(), skip_invalid_(false), always_get_initial_(false), pos_() {}
        bool AtEnd() const { return pos_.subresource.aspectMask == 0; }

        // Only for comparisons to end()
        // Note: if a fully function == is needed, the AtEnd needs to be maintained, as end_iterator is a static.
        bool operator==(const ConstIterator& other) const { return AtEnd() && other.AtEnd(); };
        bool operator!=(const ConstIterator& other) const { return AtEnd() != other.AtEnd(); };

      protected:
        void Increment();
        friend ImageSubresourceLayoutMap;
        ConstIterator(const RangeMap& current, const RangeMap& initial, const Encoder& encoder,
                      const VkImageSubresourceRange& subres, bool skip_invalid, bool always_get_initial);
        void UpdateRangeAndValue();
        void ForceEndCondition() { pos_.subresource.aspectMask = 0; }

        RangeGenerator range_gen_;
        ParallelIterator<const RangeMap, const RangeMap> parallel_it_;
        bool skip_invalid_;
        bool always_get_initial_;
        SubresourceLayout pos_;
        IndexType current_index_ = 0;
        IndexType constant_value_bound_ = 0;
    };

    ConstIterator Find(const VkImageSubresourceRange& subres_range, bool skip_invalid = true,
                       bool always_get_initial = false) const {
        if (InRange(subres_range)) {
            return ConstIterator(layouts_.current, layouts_.initial, encoder_, subres_range, skip_invalid, always_get_initial);
        }
        return End();
    }

    // Begin is a find of the full range with the default skip/ always get parameters
    ConstIterator Begin(bool always_get_initial = true) const;
    inline ConstIterator begin() const { return Begin(); }  // STL style, for range based loops and familiarity
    const ConstIterator& End() const { return end_iterator; }
    const ConstIterator& end() const { return End(); }  // STL style, for range based loops and familiarity.
    inline size_t InitialLayoutSize() const { return layouts_.initial.size(); }
    inline size_t CurrentLayoutSize() const { return layouts_.current.size(); }

    bool SetSubresourceRangeLayout(const CMD_BUFFER_STATE& cb_state, const VkImageSubresourceRange& range, VkImageLayout layout,
                                   VkImageLayout expected_layout = kInvalidLayout);
    bool SetSubresourceRangeInitialLayout(const CMD_BUFFER_STATE& cb_state, const VkImageSubresourceRange& range,
                                          VkImageLayout layout, const IMAGE_VIEW_STATE* view_state = nullptr);
    bool SetSubresourceRangeInitialLayout(const CMD_BUFFER_STATE& cb_state, VkImageLayout layout,
                                          const IMAGE_VIEW_STATE& view_state);
    bool ForRange(const VkImageSubresourceRange& range, const Callback& callback, bool skip_invalid = true,
                  bool always_get_initial = false) const;
    VkImageLayout GetSubresourceLayout(const VkImageSubresource& subresource) const;
    VkImageLayout GetSubresourceInitialLayout(const VkImageSubresource& subresource) const;
    Layouts GetSubresourceLayouts(const VkImageSubresource& subresource, bool always_get_initial = true) const;
    const InitialLayoutState* GetSubresourceInitialLayoutState(const IndexType index) const;
    const InitialLayoutState* GetSubresourceInitialLayoutState(const VkImageSubresource& subresource) const;
    bool UpdateFrom(const ImageSubresourceLayoutMap& from);
    uintptr_t CompatibilityKey() const;
    const InitialLayoutMap& GetInitialLayoutMap() const { return layouts_.initial; }
    const LayoutMap& GetCurrentLayoutMap() const { return layouts_.current; }
    ImageSubresourceLayoutMap(const IMAGE_STATE& image_state);
    ~ImageSubresourceLayoutMap() {}
    const IMAGE_STATE* GetImageView() const { return &image_state_; };

    struct LayoutMaps {
        LayoutMap current;
        InitialLayoutMap initial;
        LayoutMaps(typename LayoutMap::index_type size) : current(size), initial(size) {}
    };

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

    inline InitialLayoutState* UpdateInitialLayoutState(const IndexRange& range, InitialLayoutState* initial_state,
                                                        const CMD_BUFFER_STATE& cb_state, const IMAGE_VIEW_STATE* view_state) {
        if (!initial_state) {
            // Allocate on demand...  initial_layout_states_ holds ownership as a unique_ptr, while
            // each subresource has a non-owning copy of the plain pointer.
            initial_state = new InitialLayoutState(cb_state, view_state);
            initial_layout_states_.emplace_back(initial_state);
        }
        assert(initial_state);
        sparse_container::update_range_value(initial_layout_state_map_, range, initial_state, WritePolicy::prefer_dest);
        return initial_state;
    }

    // This map *also* needs "write once" semantics
    using InitialLayoutStateMap = subresource_adapter::BothRangeMap<InitialLayoutState*, 16>;

  private:
    const IMAGE_STATE& image_state_;
    const Encoder& encoder_;
    LayoutMaps layouts_;
    InitialLayoutStates initial_layout_states_;
    InitialLayoutStateMap initial_layout_state_map_;

    static const ConstIterator end_iterator;  // Just to hold the end condition tombstone (aspectMask == 0)
};
}  // namespace image_layout_map
#endif
