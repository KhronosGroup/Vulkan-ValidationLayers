/* Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (C) 2015-2021 Google Inc.
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
 * Author: Tobin Ehlis <tobine@google.com>
 *         John Zulauf <jzulauf@lunarg.com>
 */
#ifndef CORE_VALIDATION_DESCRIPTOR_SETS_H_
#define CORE_VALIDATION_DESCRIPTOR_SETS_H_

#include "hash_vk_types.h"
#include "vk_layer_logging.h"
#include "vk_layer_utils.h"
#include "vk_safe_struct.h"
#include "vulkan/vk_layer.h"
#include "vk_object_types.h"
#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class CoreChecks;
class ValidationStateTracker;

// Descriptor Data structures
namespace cvdescriptorset {

// Utility structs/classes/types
// Index range for global indices below, end is exclusive, i.e. [start,end)
struct IndexRange {
    IndexRange(uint32_t start_in, uint32_t end_in) : start(start_in), end(end_in) {}
    IndexRange() = default;
    uint32_t start;
    uint32_t end;
};

/*
 * DescriptorSetLayoutDef/DescriptorSetLayout classes
 *
 * Overview - These two classes encapsulate the Vulkan VkDescriptorSetLayout data (layout).
 *   A layout consists of some number of bindings, each of which has a binding#, a
 *   type, descriptor count, stage flags, and pImmutableSamplers.

 *   The DescriptorSetLayoutDef represents a canonicalization of the input data and contains
 *   neither per handle or per device state.  It is possible for different handles on
 *   different devices to share a common def.  This is used and useful for quick compatibiltiy
 *   validation.  The DescriptorSetLayout refers to a DescriptorSetLayoutDef and contains
 *   all per handle state.
 *
 * Index vs Binding - A layout is created with an array of VkDescriptorSetLayoutBinding
 *  where each array index will have a corresponding binding# that is defined in that struct.
 *  The binding#, then, is decoupled from VkDescriptorSetLayoutBinding index, which allows
 *  bindings to be defined out-of-order. This DescriptorSetLayout class, however, stores
 *  the bindings internally in-order. This is useful for operations which may "roll over"
 *  from a single binding to the next consecutive binding.
 *
 *  Note that although the bindings are stored in-order, there still may be "gaps" in the
 *  binding#. For example, if the binding creation order is 8, 7, 10, 3, 4, then the
 *  internal binding array will have five entries stored in binding order 3, 4, 7, 8, 10.
 *  To process all of the bindings in a layout you can iterate from 0 to GetBindingCount()
 *  and use the Get*FromIndex() functions for each index. To just process a single binding,
 *  use the Get*FromBinding() functions.
 *
 * Global Index - The binding vector index has as many indices as there are bindings.
 *  This class also has the concept of a Global Index. For the global index functions,
 *  there are as many global indices as there are descriptors in the layout.
 *  For the global index, consider all of the bindings to be a flat array where
 *  descriptor 0 of of the lowest binding# is index 0 and each descriptor in the layout
 *  increments from there. So if the lowest binding# in this example had descriptorCount of
 *  10, then the GlobalStartIndex of the 2nd lowest binding# will be 10 where 0-9 are the
 *  global indices for the lowest binding#.
 */
class DescriptorSetLayoutDef {
  public:
    // Constructors and destructor
    DescriptorSetLayoutDef(const VkDescriptorSetLayoutCreateInfo *p_create_info);
    size_t hash() const;

    uint32_t GetTotalDescriptorCount() const { return descriptor_count_; };
    uint32_t GetDynamicDescriptorCount() const { return dynamic_descriptor_count_; };
    VkDescriptorSetLayoutCreateFlags GetCreateFlags() const { return flags_; }
    // For a given binding, return the number of descriptors in that binding and all successive bindings
    uint32_t GetBindingCount() const { return binding_count_; };
    // Non-empty binding numbers in order
    const std::set<uint32_t> &GetSortedBindingSet() const { return non_empty_bindings_; }
    // Return true if given binding is present in this layout
    bool HasBinding(const uint32_t binding) const { return binding_to_index_map_.count(binding) > 0; };
    // Return true if binding 1 beyond given exists and has same type, stageFlags & immutable sampler use
    bool IsNextBindingConsistent(const uint32_t) const;
    uint32_t GetIndexFromBinding(uint32_t binding) const;
    // Various Get functions that can either be passed a binding#, which will
    //  be automatically translated into the appropriate index, or the index# can be passed in directly
    uint32_t GetMaxBinding() const { return bindings_[bindings_.size() - 1].binding; }
    VkDescriptorSetLayoutBinding const *GetDescriptorSetLayoutBindingPtrFromIndex(const uint32_t) const;
    VkDescriptorSetLayoutBinding const *GetDescriptorSetLayoutBindingPtrFromBinding(uint32_t binding) const {
        return GetDescriptorSetLayoutBindingPtrFromIndex(GetIndexFromBinding(binding));
    }
    const std::vector<safe_VkDescriptorSetLayoutBinding> &GetBindings() const { return bindings_; }
    const VkDescriptorSetLayoutBinding *GetBindingInfoFromIndex(const uint32_t index) const { return bindings_[index].ptr(); }
    const VkDescriptorSetLayoutBinding *GetBindingInfoFromBinding(const uint32_t binding) const {
        return GetBindingInfoFromIndex(GetIndexFromBinding(binding));
    }
    const std::vector<VkDescriptorBindingFlags> &GetBindingFlags() const { return binding_flags_; }
    uint32_t GetDescriptorCountFromIndex(const uint32_t) const;
    uint32_t GetDescriptorCountFromBinding(const uint32_t binding) const {
        return GetDescriptorCountFromIndex(GetIndexFromBinding(binding));
    }
    VkDescriptorType GetTypeFromIndex(const uint32_t) const;
    VkDescriptorType GetTypeFromBinding(const uint32_t binding) const { return GetTypeFromIndex(GetIndexFromBinding(binding)); }
    VkShaderStageFlags GetStageFlagsFromIndex(const uint32_t) const;
    VkShaderStageFlags GetStageFlagsFromBinding(const uint32_t binding) const {
        return GetStageFlagsFromIndex(GetIndexFromBinding(binding));
    }
    VkDescriptorBindingFlags GetDescriptorBindingFlagsFromIndex(const uint32_t) const;
    VkDescriptorBindingFlags GetDescriptorBindingFlagsFromBinding(const uint32_t binding) const {
        return GetDescriptorBindingFlagsFromIndex(GetIndexFromBinding(binding));
    }
    VkSampler const *GetImmutableSamplerPtrFromBinding(const uint32_t) const;
    VkSampler const *GetImmutableSamplerPtrFromIndex(const uint32_t) const;
    // For a given binding and array index, return the corresponding index into the dynamic offset array
    int32_t GetDynamicOffsetIndexFromBinding(uint32_t binding) const {
        auto dyn_off = binding_to_dynamic_array_idx_map_.find(binding);
        if (dyn_off == binding_to_dynamic_array_idx_map_.end()) {
            assert(0);  // Requesting dyn offset for invalid binding/array idx pair
            return -1;
        }
        return dyn_off->second;
    }
    // For a particular binding, get the global index range
    //  This call should be guarded by a call to "HasBinding(binding)" to verify that the given binding exists
    const IndexRange &GetGlobalIndexRangeFromBinding(const uint32_t) const;
    const cvdescriptorset::IndexRange &GetGlobalIndexRangeFromIndex(uint32_t index) const;

    // Helper function to get the next valid binding for a descriptor
    uint32_t GetNextValidBinding(const uint32_t) const;
    bool IsPushDescriptor() const { return GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR; };

    struct BindingTypeStats {
        uint32_t dynamic_buffer_count;
        uint32_t non_dynamic_buffer_count;
        uint32_t image_sampler_count;
    };
    const BindingTypeStats &GetBindingTypeStats() const { return binding_type_stats_; }

  private:
    // Only the first three data members are used for hash and equality checks, the other members are derived from them, and are
    // used to speed up the various lookups/queries/validations
    VkDescriptorSetLayoutCreateFlags flags_;
    std::vector<safe_VkDescriptorSetLayoutBinding> bindings_;
    std::vector<VkDescriptorBindingFlags> binding_flags_;

    // Convenience data structures for rapid lookup of various descriptor set layout properties
    std::set<uint32_t> non_empty_bindings_;  // Containing non-emtpy bindings in numerical order
    std::unordered_map<uint32_t, uint32_t> binding_to_index_map_;
    // The following map allows an non-iterative lookup of a binding from a global index...
    std::vector<IndexRange> global_index_range_;  // range is exclusive of .end
    // For a given binding map to associated index in the dynamic offset array
    std::unordered_map<uint32_t, uint32_t> binding_to_dynamic_array_idx_map_;

    uint32_t binding_count_;     // # of bindings in this layout
    uint32_t descriptor_count_;  // total # descriptors in this layout
    uint32_t dynamic_descriptor_count_;
    BindingTypeStats binding_type_stats_;
};

static inline bool operator==(const DescriptorSetLayoutDef &lhs, const DescriptorSetLayoutDef &rhs) {
    bool result = (lhs.GetCreateFlags() == rhs.GetCreateFlags()) && (lhs.GetBindings() == rhs.GetBindings()) &&
                  (lhs.GetBindingFlags() == rhs.GetBindingFlags());
    return result;
}

// Canonical dictionary of DSL definitions -- independent of device or handle
using DescriptorSetLayoutDict = hash_util::Dictionary<DescriptorSetLayoutDef, hash_util::HasHashMember<DescriptorSetLayoutDef>>;
using DescriptorSetLayoutId = DescriptorSetLayoutDict::Id;

class DescriptorSetLayout : public BASE_NODE {
  public:
    // Constructors and destructor
    DescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo *p_create_info, const VkDescriptorSetLayout layout);
    bool HasBinding(const uint32_t binding) const { return layout_id_->HasBinding(binding); }
    // Return true if this layout is compatible with passed in layout from a pipelineLayout,
    //   else return false and update error_msg with description of incompatibility
    // Return true if this layout is compatible with passed in layout
    bool IsCompatible(DescriptorSetLayout const *rh_ds_layout) const;
    // Straightforward Get functions
    VkDescriptorSetLayout GetDescriptorSetLayout() const { return layout_; };
    const DescriptorSetLayoutDef *GetLayoutDef() const { return layout_id_.get(); }
    DescriptorSetLayoutId GetLayoutId() const { return layout_id_; }
    uint32_t GetTotalDescriptorCount() const { return layout_id_->GetTotalDescriptorCount(); };
    uint32_t GetDynamicDescriptorCount() const { return layout_id_->GetDynamicDescriptorCount(); };
    uint32_t GetBindingCount() const { return layout_id_->GetBindingCount(); };
    VkDescriptorSetLayoutCreateFlags GetCreateFlags() const { return layout_id_->GetCreateFlags(); }
    bool IsNextBindingConsistent(const uint32_t) const;
    uint32_t GetIndexFromBinding(uint32_t binding) const { return layout_id_->GetIndexFromBinding(binding); }
    // Various Get functions that can either be passed a binding#, which will
    //  be automatically translated into the appropriate index, or the index# can be passed in directly
    uint32_t GetMaxBinding() const { return layout_id_->GetMaxBinding(); }
    VkDescriptorSetLayoutBinding const *GetDescriptorSetLayoutBindingPtrFromIndex(const uint32_t index) const {
        return layout_id_->GetDescriptorSetLayoutBindingPtrFromIndex(index);
    }
    VkDescriptorSetLayoutBinding const *GetDescriptorSetLayoutBindingPtrFromBinding(uint32_t binding) const {
        return layout_id_->GetDescriptorSetLayoutBindingPtrFromBinding(binding);
    }
    const std::vector<safe_VkDescriptorSetLayoutBinding> &GetBindings() const { return layout_id_->GetBindings(); }
    const std::set<uint32_t> &GetSortedBindingSet() const { return layout_id_->GetSortedBindingSet(); }
    uint32_t GetDescriptorCountFromIndex(const uint32_t index) const { return layout_id_->GetDescriptorCountFromIndex(index); }
    uint32_t GetDescriptorCountFromBinding(const uint32_t binding) const {
        return layout_id_->GetDescriptorCountFromBinding(binding);
    }
    VkDescriptorType GetTypeFromIndex(const uint32_t index) const { return layout_id_->GetTypeFromIndex(index); }
    VkDescriptorType GetTypeFromBinding(const uint32_t binding) const { return layout_id_->GetTypeFromBinding(binding); }
    VkShaderStageFlags GetStageFlagsFromIndex(const uint32_t index) const { return layout_id_->GetStageFlagsFromIndex(index); }
    VkShaderStageFlags GetStageFlagsFromBinding(const uint32_t binding) const {
        return layout_id_->GetStageFlagsFromBinding(binding);
    }
    VkDescriptorBindingFlags GetDescriptorBindingFlagsFromIndex(const uint32_t index) const {
        return layout_id_->GetDescriptorBindingFlagsFromIndex(index);
    }
    VkDescriptorBindingFlags GetDescriptorBindingFlagsFromBinding(const uint32_t binding) const {
        return layout_id_->GetDescriptorBindingFlagsFromBinding(binding);
    }
    VkSampler const *GetImmutableSamplerPtrFromBinding(const uint32_t binding) const {
        return layout_id_->GetImmutableSamplerPtrFromBinding(binding);
    }
    VkSampler const *GetImmutableSamplerPtrFromIndex(const uint32_t index) const {
        return layout_id_->GetImmutableSamplerPtrFromIndex(index);
    }
    // For a given binding and array index, return the corresponding index into the dynamic offset array
    int32_t GetDynamicOffsetIndexFromBinding(uint32_t binding) const {
        return layout_id_->GetDynamicOffsetIndexFromBinding(binding);
    }
    // For a particular binding, get the global index range
    //  This call should be guarded by a call to "HasBinding(binding)" to verify that the given binding exists
    const IndexRange &GetGlobalIndexRangeFromBinding(const uint32_t binding) const {
        return layout_id_->GetGlobalIndexRangeFromBinding(binding);
    }
    const IndexRange &GetGlobalIndexRangeFromIndex(uint32_t index) const { return layout_id_->GetGlobalIndexRangeFromIndex(index); }

    // Helper function to get the next valid binding for a descriptor
    uint32_t GetNextValidBinding(const uint32_t binding) const { return layout_id_->GetNextValidBinding(binding); }
    bool IsPushDescriptor() const { return layout_id_->IsPushDescriptor(); }
    bool IsVariableDescriptorCountFromIndex(uint32_t index) const {
        return !!(GetDescriptorBindingFlagsFromIndex(index) & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT);
    }
    bool IsVariableDescriptorCount(uint32_t binding) const {
        return IsVariableDescriptorCountFromIndex(GetIndexFromBinding(binding));
    }

    using BindingTypeStats = DescriptorSetLayoutDef::BindingTypeStats;
    const BindingTypeStats &GetBindingTypeStats() const { return layout_id_->GetBindingTypeStats(); }

    // Binding Iterator
    class ConstBindingIterator {
      public:
        ConstBindingIterator() = delete;
        ConstBindingIterator(const ConstBindingIterator &other) = default;
        ConstBindingIterator &operator=(const ConstBindingIterator &rhs) = default;

        ConstBindingIterator(const DescriptorSetLayout *layout) : layout_(layout), index_(0) { assert(layout); }
        ConstBindingIterator(const DescriptorSetLayout *layout, uint32_t binding) : ConstBindingIterator(layout) {
            index_ = layout->GetIndexFromBinding(binding);
        }

        VkDescriptorSetLayoutBinding const *GetDescriptorSetLayoutBindingPtr() const {
            return layout_->GetDescriptorSetLayoutBindingPtrFromIndex(index_);
        }
        uint32_t GetDescriptorCount() const { return layout_->GetDescriptorCountFromIndex(index_); }
        VkDescriptorType GetType() const { return layout_->GetTypeFromIndex(index_); }
        VkShaderStageFlags GetStageFlags() const { return layout_->GetStageFlagsFromIndex(index_); }

        VkDescriptorBindingFlags GetDescriptorBindingFlags() const { return layout_->GetDescriptorBindingFlagsFromIndex(index_); }

        bool IsVariableDescriptorCount() const { return layout_->IsVariableDescriptorCountFromIndex(index_); }

        VkSampler const *GetImmutableSamplerPtr() const { return layout_->GetImmutableSamplerPtrFromIndex(index_); }
        const IndexRange &GetGlobalIndexRange() const { return layout_->GetGlobalIndexRangeFromIndex(index_); }
        uint32_t GetIndex() const { return index_; }
        bool AtEnd() const { return index_ == layout_->GetBindingCount(); }

        // Return index into dynamic offset array for given binding
        int32_t GetDynamicOffsetIndex() const {
            return layout_->GetDynamicOffsetIndexFromBinding(Binding());  //  There is only binding mapped access in layout_
        }

        bool operator==(const ConstBindingIterator &rhs) { return (index_ = rhs.index_) && (layout_ == rhs.layout_); }

        ConstBindingIterator &operator++() {
            if (!AtEnd()) {
                index_++;
            }
            return *this;
        }

        bool IsConsistent(const ConstBindingIterator &other) const {
            if (AtEnd() || other.AtEnd()) {
                return false;
            }
            const auto *binding_ci = GetDescriptorSetLayoutBindingPtr();
            const auto *other_binding_ci = other.GetDescriptorSetLayoutBindingPtr();
            assert((binding_ci != nullptr) && (other_binding_ci != nullptr));

            if ((binding_ci->descriptorType != other_binding_ci->descriptorType) ||
                (binding_ci->stageFlags != other_binding_ci->stageFlags) ||
                (!hash_util::similar_for_nullity(binding_ci->pImmutableSamplers, other_binding_ci->pImmutableSamplers)) ||
                (GetDescriptorBindingFlags() != other.GetDescriptorBindingFlags())) {
                return false;
            }
            return true;
        }

        const DescriptorSetLayout *Layout() const { return layout_; }
        uint32_t Binding() const { return layout_->GetBindings()[index_].binding; }
        ConstBindingIterator Next() {
            ConstBindingIterator next(*this);
            ++next;
            return next;
        }

      private:
        const DescriptorSetLayout *layout_;
        uint32_t index_;
    };
    ConstBindingIterator end() const { return ConstBindingIterator(this, GetBindingCount()); }

  private:
    VkDescriptorSetLayout layout_;
    DescriptorSetLayoutId layout_id_;
};

/*
 * Descriptor classes
 *  Descriptor is an abstract base class from which 5 separate descriptor types are derived.
 *   This allows the WriteUpdate() and CopyUpdate() operations to be specialized per
 *   descriptor type, but all descriptors in a set can be accessed via the common Descriptor*.
 */

// Slightly broader than type, each c++ "class" will has a corresponding "DescriptorClass"
enum DescriptorClass { PlainSampler, ImageSampler, Image, TexelBuffer, GeneralBuffer, InlineUniform, AccelerationStructure };

class Descriptor {
  public:
    virtual ~Descriptor(){};
    virtual void WriteUpdate(const ValidationStateTracker *dev_data, const VkWriteDescriptorSet *, const uint32_t) = 0;
    virtual void CopyUpdate(const ValidationStateTracker *dev_data, const Descriptor *) = 0;
    // Create binding between resources of this descriptor and given cb_node
    virtual void UpdateDrawState(ValidationStateTracker *, CMD_BUFFER_STATE *) = 0;
    virtual DescriptorClass GetClass() const { return descriptor_class; };
    // Special fast-path check for SamplerDescriptors that are immutable
    virtual bool IsImmutableSampler() const { return false; };
    // Check for dynamic descriptor type
    virtual bool IsDynamic() const { return false; };
    // Check for storage descriptor type
    virtual bool IsStorage() const { return false; };
    bool updated;  // Has descriptor been updated?
    DescriptorClass descriptor_class;
};

// Return true if this layout is compatible with passed in layout from a pipelineLayout,
//   else return false and update error_msg with description of incompatibility
bool VerifySetLayoutCompatibility(const debug_report_data *report_data, DescriptorSetLayout const *lh_ds_layout,
                                  DescriptorSetLayout const *rh_ds_layout, std::string *error_msg);
bool ValidateDescriptorSetLayoutCreateInfo(const ValidationObject *val_obj, const VkDescriptorSetLayoutCreateInfo *create_info,
                                           const bool push_descriptor_ext, const uint32_t max_push_descriptors,
                                           const bool descriptor_indexing_ext,
                                           const VkPhysicalDeviceVulkan12Features *core12_features,
                                           const VkPhysicalDeviceInlineUniformBlockFeaturesEXT *inline_uniform_block_features,
                                           const VkPhysicalDeviceInlineUniformBlockPropertiesEXT *inline_uniform_block_props,
                                           const DeviceExtensions *device_extensions);

class SamplerDescriptor : public Descriptor {
  public:
    SamplerDescriptor(const ValidationStateTracker *dev_data, const VkSampler *);
    void WriteUpdate(const ValidationStateTracker *dev_data, const VkWriteDescriptorSet *, const uint32_t) override;
    void CopyUpdate(const ValidationStateTracker *dev_data, const Descriptor *) override;
    void UpdateDrawState(ValidationStateTracker *, CMD_BUFFER_STATE *) override;
    virtual bool IsImmutableSampler() const override { return immutable_; };
    VkSampler GetSampler() const { return sampler_state_ ? sampler_state_->sampler : VK_NULL_HANDLE; }
    const SAMPLER_STATE *GetSamplerState() const { return sampler_state_.get(); }
    SAMPLER_STATE *GetSamplerState() { return sampler_state_.get(); }
    std::shared_ptr<SAMPLER_STATE> GetSharedSamplerState() const { return sampler_state_; }

  private:
    bool immutable_;
    std::shared_ptr<SAMPLER_STATE> sampler_state_;
};

class ImageSamplerDescriptor : public Descriptor {
  public:
    ImageSamplerDescriptor(const ValidationStateTracker *dev_data, const VkSampler *);
    void WriteUpdate(const ValidationStateTracker *dev_data, const VkWriteDescriptorSet *, const uint32_t) override;
    void CopyUpdate(const ValidationStateTracker *dev_data, const Descriptor *) override;
    void UpdateDrawState(ValidationStateTracker *, CMD_BUFFER_STATE *) override;
    virtual bool IsImmutableSampler() const override { return immutable_; };
    VkSampler GetSampler() const { return sampler_state_ ? sampler_state_->sampler : VK_NULL_HANDLE; }
    VkImageView GetImageView() const { return image_view_state_ ? image_view_state_->image_view : VK_NULL_HANDLE; }
    const IMAGE_VIEW_STATE *GetImageViewState() const { return image_view_state_.get(); }
    IMAGE_VIEW_STATE *GetImageViewState() { return image_view_state_.get(); }
    std::shared_ptr<IMAGE_VIEW_STATE> GetSharedImageViewState() const { return image_view_state_; }
    VkImageLayout GetImageLayout() const { return image_layout_; }
    const SAMPLER_STATE *GetSamplerState() const { return sampler_state_.get(); }
    SAMPLER_STATE *GetSamplerState() { return sampler_state_.get(); }
    std::shared_ptr<SAMPLER_STATE> GetSharedSamplerState() const { return sampler_state_; }

  private:
    std::shared_ptr<SAMPLER_STATE> sampler_state_;
    bool immutable_;
    std::shared_ptr<IMAGE_VIEW_STATE> image_view_state_;
    VkImageLayout image_layout_;
};

class ImageDescriptor : public Descriptor {
  public:
    ImageDescriptor(const VkDescriptorType);
    void WriteUpdate(const ValidationStateTracker *dev_data, const VkWriteDescriptorSet *, const uint32_t) override;
    void CopyUpdate(const ValidationStateTracker *dev_data, const Descriptor *) override;
    void UpdateDrawState(ValidationStateTracker *, CMD_BUFFER_STATE *) override;
    virtual bool IsStorage() const override { return storage_; }
    VkImageView GetImageView() const { return image_view_state_ ? image_view_state_->image_view : VK_NULL_HANDLE; }
    const IMAGE_VIEW_STATE *GetImageViewState() const { return image_view_state_.get(); }
    IMAGE_VIEW_STATE *GetImageViewState() { return image_view_state_.get(); }
    std::shared_ptr<IMAGE_VIEW_STATE> GetSharedImageViewState() const { return image_view_state_; }
    VkImageLayout GetImageLayout() const { return image_layout_; }

  private:
    bool storage_;
    std::shared_ptr<IMAGE_VIEW_STATE> image_view_state_;
    VkImageLayout image_layout_;
};

class TexelDescriptor : public Descriptor {
  public:
    TexelDescriptor(const VkDescriptorType);
    void WriteUpdate(const ValidationStateTracker *dev_data, const VkWriteDescriptorSet *, const uint32_t) override;
    void CopyUpdate(const ValidationStateTracker *dev_data, const Descriptor *) override;
    void UpdateDrawState(ValidationStateTracker *, CMD_BUFFER_STATE *) override;
    virtual bool IsStorage() const override { return storage_; }
    VkBufferView GetBufferView() const { return buffer_view_state_ ? buffer_view_state_->buffer_view : VK_NULL_HANDLE; }
    const BUFFER_VIEW_STATE *GetBufferViewState() const { return buffer_view_state_.get(); }
    BUFFER_VIEW_STATE *GetBufferViewState() { return buffer_view_state_.get(); }
    std::shared_ptr<BUFFER_VIEW_STATE> GetSharedBufferViewState() const { return buffer_view_state_; }

  private:
    bool storage_;
    std::shared_ptr<BUFFER_VIEW_STATE> buffer_view_state_;
};

class BufferDescriptor : public Descriptor {
  public:
    BufferDescriptor(const VkDescriptorType);
    void WriteUpdate(const ValidationStateTracker *dev_data, const VkWriteDescriptorSet *, const uint32_t) override;
    void CopyUpdate(const ValidationStateTracker *dev_data, const Descriptor *) override;
    void UpdateDrawState(ValidationStateTracker *, CMD_BUFFER_STATE *) override;
    virtual bool IsDynamic() const override { return dynamic_; }
    virtual bool IsStorage() const override { return storage_; }
    VkBuffer GetBuffer() const { return buffer_state_ ? buffer_state_->buffer : VK_NULL_HANDLE; }
    const BUFFER_STATE *GetBufferState() const { return buffer_state_.get(); }
    BUFFER_STATE *GetBufferState() { return buffer_state_.get(); }
    std::shared_ptr<BUFFER_STATE> GetSharedBufferState() const { return buffer_state_; }
    VkDeviceSize GetOffset() const { return offset_; }
    VkDeviceSize GetRange() const { return range_; }

  private:
    bool storage_;
    bool dynamic_;
    VkDeviceSize offset_;
    VkDeviceSize range_;
    std::shared_ptr<BUFFER_STATE> buffer_state_;
};

class InlineUniformDescriptor : public Descriptor {
  public:
    InlineUniformDescriptor(const VkDescriptorType) {
        updated = false;
        descriptor_class = InlineUniform;
    }
    void WriteUpdate(const ValidationStateTracker *dev_data, const VkWriteDescriptorSet *, const uint32_t) override {
        updated = true;
    }
    void CopyUpdate(const ValidationStateTracker *dev_data, const Descriptor *) override { updated = true; }
    void UpdateDrawState(ValidationStateTracker *, CMD_BUFFER_STATE *) override {}
};

class AccelerationStructureDescriptor : public Descriptor {
  public:
    AccelerationStructureDescriptor(const VkDescriptorType);
    void WriteUpdate(const ValidationStateTracker *dev_data, const VkWriteDescriptorSet *, const uint32_t) override;
    VkAccelerationStructureKHR GetAccelerationStructure() const { return acc_; }
    const ACCELERATION_STRUCTURE_STATE_KHR *GetAccelerationStructureStateKHR() const { return acc_state_.get(); }
    ACCELERATION_STRUCTURE_STATE_KHR *GetAccelerationStructureStateKHR() { return acc_state_.get(); }
    VkAccelerationStructureNV GetAccelerationStructureNV() const { return acc_nv_; }
    const ACCELERATION_STRUCTURE_STATE *GetAccelerationStructureStateNV() const { return acc_state_nv_.get(); }
    ACCELERATION_STRUCTURE_STATE *GetAccelerationStructureStateNV() { return acc_state_nv_.get(); }
    void CopyUpdate(const ValidationStateTracker *dev_data, const Descriptor *) override;
    void UpdateDrawState(ValidationStateTracker *, CMD_BUFFER_STATE *) override;
    bool is_khr() { return is_khr_; }

  private:
    bool is_khr_;
    VkAccelerationStructureKHR acc_;
    std::shared_ptr<ACCELERATION_STRUCTURE_STATE_KHR> acc_state_;
    VkAccelerationStructureNV acc_nv_;
    std::shared_ptr<ACCELERATION_STRUCTURE_STATE> acc_state_nv_;
};

union AnyDescriptor {
    SamplerDescriptor sampler;
    ImageSamplerDescriptor image_sampler;
    ImageDescriptor image;
    TexelDescriptor texel;
    BufferDescriptor buffer;
    InlineUniformDescriptor inline_uniform;
    AccelerationStructureDescriptor accelerator_structure;
    ~AnyDescriptor() = delete;
};

struct alignas(alignof(AnyDescriptor)) DescriptorBackingStore {
    uint8_t data[sizeof(AnyDescriptor)];

    SamplerDescriptor *Sampler() { return &(reinterpret_cast<AnyDescriptor *>(this)->sampler); }
    ImageSamplerDescriptor *ImageSampler() { return &(reinterpret_cast<AnyDescriptor *>(this)->image_sampler); }
    ImageDescriptor *Image() { return &(reinterpret_cast<AnyDescriptor *>(this)->image); }
    TexelDescriptor *Texel() { return &(reinterpret_cast<AnyDescriptor *>(this)->texel); }
    BufferDescriptor *Buffer() { return &(reinterpret_cast<AnyDescriptor *>(this)->buffer); }
    InlineUniformDescriptor *InlineUniform() { return &(reinterpret_cast<AnyDescriptor *>(this)->inline_uniform); }
    AccelerationStructureDescriptor *AccelerationStructure() {
        return &(reinterpret_cast<AnyDescriptor *>(this)->accelerator_structure);
    }
};

// Structs to contain common elements that need to be shared between Validate* and Perform* calls below
struct AllocateDescriptorSetsData {
    std::map<uint32_t, uint32_t> required_descriptors_by_type;
    std::vector<std::shared_ptr<DescriptorSetLayout const>> layout_nodes;
    void Init(uint32_t);
    AllocateDescriptorSetsData(){};
};
// Helper functions for descriptor set functions that cross multiple sets
// "Validate" will make sure an update is ok without actually performing it
bool ValidateUpdateDescriptorSets(const debug_report_data *, const CoreChecks *, uint32_t, const VkWriteDescriptorSet *, uint32_t,
                                  const VkCopyDescriptorSet *, const char *func_name);
// "Perform" does the update with the assumption that ValidateUpdateDescriptorSets() has passed for the given update
void PerformUpdateDescriptorSets(ValidationStateTracker *, uint32_t, const VkWriteDescriptorSet *, uint32_t,
                                 const VkCopyDescriptorSet *);

// Core Validation specific validation checks using DescriptorSet and DescriptorSetLayoutAccessors
// TODO: migrate out of descriptor_set.cpp/h
// For a particular binding starting at offset and having update_count descriptors
// updated, verify that for any binding boundaries crossed, the update is consistent
bool VerifyUpdateConsistency(debug_report_data *report_data, DescriptorSetLayout::ConstBindingIterator current_binding,
                             uint32_t offset, uint32_t update_count, const char *type, const VkDescriptorSet set,
                             std::string *error_msg);

// Validate buffer descriptor update info
bool ValidateBufferUsage(debug_report_data *report_data, BUFFER_STATE const *buffer_node, VkDescriptorType type,
                         std::string *error_code, std::string *error_msg);

// Helper class to encapsulate the descriptor update template decoding logic
struct DecodedTemplateUpdate {
    std::vector<VkWriteDescriptorSet> desc_writes;
    std::vector<VkWriteDescriptorSetInlineUniformBlockEXT> inline_infos;
    std::vector<VkWriteDescriptorSetAccelerationStructureKHR> inline_infos_khr;
    std::vector<VkWriteDescriptorSetAccelerationStructureNV> inline_infos_nv;
    DecodedTemplateUpdate(const ValidationStateTracker *device_data, VkDescriptorSet descriptorSet,
                          const TEMPLATE_STATE *template_state, const void *pData,
                          VkDescriptorSetLayout push_layout = VK_NULL_HANDLE);
};

/*
 * DescriptorSet class
 *
 * Overview - This class encapsulates the Vulkan VkDescriptorSet data (set).
 *   A set has an underlying layout which defines the bindings in the set and the
 *   types and numbers of descriptors in each descriptor slot. Most of the layout
 *   interfaces are exposed through identically-named functions in the set class.
 *   Please refer to the DescriptorSetLayout comment above for a description of
 *   index, binding, and global index.
 *
 * At construction a vector of Descriptor* is created with types corresponding to the
 *   layout. The primary operation performed on the descriptors is to update them
 *   via write or copy updates, and validate that the update contents are correct.
 *   In order to validate update contents, the DescriptorSet stores a bunch of ptrs
 *   to data maps where various Vulkan objects can be looked up. The management of
 *   those maps is performed externally. The set class relies on their contents to
 *   be correct at the time of update.
 */
class DescriptorSet : public BASE_NODE {
  public:
    using StateTracker = ValidationStateTracker;
    DescriptorSet(const VkDescriptorSet, DESCRIPTOR_POOL_STATE *, const std::shared_ptr<DescriptorSetLayout const> &,
                  uint32_t variable_count, const StateTracker *state_data_const);
    ~DescriptorSet();
    // A number of common Get* functions that return data based on layout from which this set was created
    uint32_t GetTotalDescriptorCount() const { return layout_->GetTotalDescriptorCount(); };
    uint32_t GetDynamicDescriptorCount() const { return layout_->GetDynamicDescriptorCount(); };
    uint32_t GetBindingCount() const { return layout_->GetBindingCount(); };
    VkDescriptorType GetTypeFromIndex(const uint32_t index) const { return layout_->GetTypeFromIndex(index); };
    VkDescriptorType GetTypeFromBinding(const uint32_t binding) const { return layout_->GetTypeFromBinding(binding); };
    uint32_t GetDescriptorCountFromIndex(const uint32_t index) const { return layout_->GetDescriptorCountFromIndex(index); };
    uint32_t GetDescriptorCountFromBinding(const uint32_t binding) const {
        return layout_->GetDescriptorCountFromBinding(binding);
    };
    // Return index into dynamic offset array for given binding
    int32_t GetDynamicOffsetIndexFromBinding(uint32_t binding) const { return layout_->GetDynamicOffsetIndexFromBinding(binding); }
    // Return true if given binding is present in this set
    bool HasBinding(const uint32_t binding) const { return layout_->HasBinding(binding); };

    std::string StringifySetAndLayout() const;

    // Perform a push update whose contents were just validated using ValidatePushDescriptorsUpdate
    void PerformPushDescriptorsUpdate(ValidationStateTracker *dev_data, uint32_t write_count, const VkWriteDescriptorSet *p_wds);
    // Perform a WriteUpdate whose contents were just validated using ValidateWriteUpdate
    void PerformWriteUpdate(ValidationStateTracker *dev_data, const VkWriteDescriptorSet *);
    // Perform a CopyUpdate whose contents were just validated using ValidateCopyUpdate
    void PerformCopyUpdate(ValidationStateTracker *dev_data, const VkCopyDescriptorSet *, const DescriptorSet *);

    const std::shared_ptr<DescriptorSetLayout const> &GetLayout() const { return layout_; };
    VkDescriptorSetLayout GetDescriptorSetLayout() const { return layout_->GetDescriptorSetLayout(); }
    VkDescriptorSet GetSet() const { return set_; };
    // Bind given cmd_buffer to this descriptor set and
    // update CB image layout map with image/imagesampler descriptor image layouts
    void UpdateDrawState(ValidationStateTracker *, CMD_BUFFER_STATE *, CMD_TYPE cmd_type, const PIPELINE_STATE *,
                         const BindingReqMap &, const char *function);

    // Track work that has been bound or validated to avoid duplicate work, important when large descriptor arrays
    // are present
    typedef std::unordered_set<uint32_t> TrackedBindings;
    static void FilterOneBindingReq(const BindingReqMap::value_type &binding_req_pair, BindingReqMap *out_req,
                                    const TrackedBindings &set, uint32_t limit);
    void FilterBindingReqs(const CMD_BUFFER_STATE &, const PIPELINE_STATE &, const BindingReqMap &in_req,
                           BindingReqMap *out_req) const;
    void UpdateValidationCache(const CMD_BUFFER_STATE &cb_state, const PIPELINE_STATE &pipeline,
                               const BindingReqMap &updated_bindings);
    void ClearCachedDynamicDescriptorValidation(CMD_BUFFER_STATE *cb_state) {
        cached_validation_[cb_state].dynamic_buffers.clear();
    }
    void ClearCachedValidation(CMD_BUFFER_STATE *cb_state) { cached_validation_.erase(cb_state); }
    VkSampler const *GetImmutableSamplerPtrFromBinding(const uint32_t index) const {
        return layout_->GetImmutableSamplerPtrFromBinding(index);
    };
    // For a particular binding, get the global index
    const IndexRange GetGlobalIndexRangeFromBinding(const uint32_t binding, bool actual_length = false) const {
        if (actual_length && binding == layout_->GetMaxBinding() && IsVariableDescriptorCount(binding)) {
            IndexRange range = layout_->GetGlobalIndexRangeFromBinding(binding);
            auto diff = GetDescriptorCountFromBinding(binding) - GetVariableDescriptorCount();
            range.end -= diff;
            return range;
        }
        return layout_->GetGlobalIndexRangeFromBinding(binding);
    };
    // Return true if any part of set has ever been updated
    bool IsUpdated() const { return some_update_; };
    bool IsPushDescriptor() const { return layout_->IsPushDescriptor(); };
    bool IsVariableDescriptorCount(uint32_t binding) const { return layout_->IsVariableDescriptorCount(binding); }
    bool IsUpdateAfterBind(uint32_t binding) const {
        return !!(layout_->GetDescriptorBindingFlagsFromBinding(binding) & VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);
    }
    uint32_t GetVariableDescriptorCount() const { return variable_count_; }
    DESCRIPTOR_POOL_STATE *GetPoolState() const { return pool_state_; }
    const Descriptor *GetDescriptorFromGlobalIndex(const uint32_t index) const { return descriptors_[index].get(); }
    const Descriptor *GetDescriptorFromBinding(const uint32_t binding, const uint32_t index = 0) const {
        const auto range = GetGlobalIndexRangeFromBinding(binding);
        if ((range.start + index) >= range.end) {
            return nullptr;
        }
        return descriptors_[range.start + index].get();
    }
    uint64_t GetChangeCount() const { return change_count_; }

    const std::vector<safe_VkWriteDescriptorSet> &GetWrites() const { return push_descriptor_set_writes; }

    // Given that we are providing placement new allocation for descriptors, the deleter needs to *only* call the destructor
    struct DescriptorDeleter {
        void operator()(Descriptor *desc) { desc->~Descriptor(); }
    };

  private:
    // Private helper to set all bound cmd buffers to INVALID state
    void InvalidateBoundCmdBuffers(ValidationStateTracker *state_data);
    bool some_update_;  // has any part of the set ever been updated?
    VkDescriptorSet set_;
    DESCRIPTOR_POOL_STATE *pool_state_;
    const std::shared_ptr<DescriptorSetLayout const> layout_;
    // NOTE: the the backing store for the descriptors must be declared *before* it so it will be destructed *after* it
    // "Destructors for nonstatic member objects are called in the reverse order in which they appear in the class declaration."
    std::vector<DescriptorBackingStore> descriptor_store_;
    std::vector<std::unique_ptr<Descriptor, DescriptorDeleter>> descriptors_;
    const StateTracker *state_data_;
    uint32_t variable_count_;
    uint64_t change_count_;

    // If this descriptor set is a push descriptor set, the descriptor
    // set writes that were last pushed.
    std::vector<safe_VkWriteDescriptorSet> push_descriptor_set_writes;

    // Cached binding and validation support:
    //
    // For the lifespan of a given command buffer recording, do lazy evaluation, caching, and dirtying of
    // expensive validation operation (typically per-draw)
    typedef std::unordered_map<CMD_BUFFER_STATE *, TrackedBindings> TrackedBindingMap;
    // Track the validation caching of bindings vs. the command buffer and draw state
    typedef std::unordered_map<uint32_t, CMD_BUFFER_STATE::ImageLayoutUpdateCount> VersionedBindings;
    struct CachedValidation {
        TrackedBindings command_binding_and_usage;                                     // Persistent for the life of the recording
        TrackedBindings non_dynamic_buffers;                                           // Persistent for the life of the recording
        TrackedBindings dynamic_buffers;                                               // Dirtied (flushed) each BindDescriptorSet
        std::unordered_map<const PIPELINE_STATE *, VersionedBindings> image_samplers;  // Tested vs. changes to CB's ImageLayout
    };
    typedef std::unordered_map<const CMD_BUFFER_STATE *, CachedValidation> CachedValidationMap;
    // Image and ImageView bindings are validated per pipeline and not invalidate by repeated binding
    CachedValidationMap cached_validation_;
};
// For the "bindless" style resource usage with many descriptors, need to optimize binding and validation
class PrefilterBindRequestMap {
  public:
    static const uint32_t kManyDescriptors_ = 64;  // TODO base this number on measured data
    std::unique_ptr<BindingReqMap> filtered_map_;
    const BindingReqMap &orig_map_;
    const DescriptorSet &descriptor_set_;

    PrefilterBindRequestMap(const DescriptorSet &ds, const BindingReqMap &in_map)
        : filtered_map_(), orig_map_(in_map), descriptor_set_(ds) {}
    const BindingReqMap &FilteredMap(const CMD_BUFFER_STATE &cb_state, const PIPELINE_STATE &);
    bool IsManyDescriptors() const { return descriptor_set_.GetTotalDescriptorCount() > kManyDescriptors_; }
};
}  // namespace cvdescriptorset
#endif  // CORE_VALIDATION_DESCRIPTOR_SETS_H_
