/* Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (C) 2015-2022 Google Inc.
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

#include "base_node.h"
#include "buffer_state.h"
#include "image_state.h"
#include "pipeline_state.h"
#include "ray_tracing_state.h"
#include "sampler_state.h"
#include "hash_vk_types.h"
#include "vk_layer_logging.h"
#include "vk_layer_utils.h"
#include "vk_safe_struct.h"
#include "vulkan/vk_layer.h"
#include "vk_object_types.h"
#include "command_validation.h"
#include <map>
#include <memory>
#include <set>
#include <vector>

class CoreChecks;
class ValidationObject;
class ValidationStateTracker;
class CMD_BUFFER_STATE;
class UPDATE_TEMPLATE_STATE;
struct DeviceExtensions;
class SAMPLER_STATE;

namespace cvdescriptorset {
class DescriptorSet;
struct AllocateDescriptorSetsData;
}

class DESCRIPTOR_POOL_STATE : public BASE_NODE {
  public:
    DESCRIPTOR_POOL_STATE(ValidationStateTracker *dev, const VkDescriptorPool pool, const VkDescriptorPoolCreateInfo *pCreateInfo);
    ~DESCRIPTOR_POOL_STATE() { Destroy(); }

    void Allocate(const VkDescriptorSetAllocateInfo *alloc_info, const VkDescriptorSet *descriptor_sets,
                  const cvdescriptorset::AllocateDescriptorSetsData *ds_data);
    void Free(uint32_t count, const VkDescriptorSet *descriptor_sets);
    void Reset();
    void Destroy() override;

    bool InUse() const override;
    uint32_t GetAvailableCount(uint32_t type) const {
        auto guard = ReadLock();
        auto iter = available_counts_.find(type);
        return iter != available_counts_.end() ? iter->second : 0;
    }

    uint32_t GetAvailableSets() const {
        auto guard = ReadLock();
        return available_sets_;
    }

    const uint32_t maxSets;  // Max descriptor sets allowed in this pool
    const safe_VkDescriptorPoolCreateInfo createInfo;
    using TypeCountMap = layer_data::unordered_map<uint32_t, uint32_t>;
    const TypeCountMap maxDescriptorTypeCount;  // Max # of descriptors of each type in this pool
  private:
    ReadLockGuard ReadLock() const { return ReadLockGuard(lock_); }
    WriteLockGuard WriteLock() { return WriteLockGuard(lock_); }
    uint32_t available_sets_;  // Available descriptor sets in this pool
    TypeCountMap available_counts_;         // Available # of descriptors of each type in this pool
    layer_data::unordered_map<VkDescriptorSet, cvdescriptorset::DescriptorSet *> sets_;  // Collection of all sets in this pool
    ValidationStateTracker *dev_data_;
    mutable ReadWriteLock lock_;
};

class UPDATE_TEMPLATE_STATE : public BASE_NODE {
  public:
    const safe_VkDescriptorUpdateTemplateCreateInfo create_info;

    UPDATE_TEMPLATE_STATE(VkDescriptorUpdateTemplate update_template, const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo)
        : BASE_NODE(update_template, kVulkanObjectTypeDescriptorUpdateTemplate), create_info(pCreateInfo) {}
};

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
    bool IsTypeMutable(const VkDescriptorType type, uint32_t binding) const;
    const std::vector<std::vector<VkDescriptorType>> &GetMutableTypes() const;
    const std::vector<VkDescriptorType> &GetMutableTypes(uint32_t binding) const;
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
    };
    const BindingTypeStats &GetBindingTypeStats() const { return binding_type_stats_; }

  private:
    // Only the first three data members are used for hash and equality checks, the other members are derived from them, and are
    // used to speed up the various lookups/queries/validations
    VkDescriptorSetLayoutCreateFlags flags_;
    std::vector<safe_VkDescriptorSetLayoutBinding> bindings_;
    std::vector<VkDescriptorBindingFlags> binding_flags_;
    // List of mutable types for each binding: [binding][mutable type]
    std::vector<std::vector<VkDescriptorType>> mutable_types_;

    // Convenience data structures for rapid lookup of various descriptor set layout properties
    std::set<uint32_t> non_empty_bindings_;  // Containing non-emtpy bindings in numerical order
    layer_data::unordered_map<uint32_t, uint32_t> binding_to_index_map_;
    // The following map allows an non-iterative lookup of a binding from a global index...
    std::vector<IndexRange> global_index_range_;  // range is exclusive of .end

    uint32_t binding_count_;     // # of bindings in this layout
    uint32_t descriptor_count_;  // total # descriptors in this layout
    uint32_t dynamic_descriptor_count_;
    BindingTypeStats binding_type_stats_;
};

static inline bool operator==(const DescriptorSetLayoutDef &lhs, const DescriptorSetLayoutDef &rhs) {
    bool result =
        (lhs.GetCreateFlags() == rhs.GetCreateFlags()) && (lhs.GetBindings() == rhs.GetBindings()) &&
        (lhs.GetBindingFlags() == rhs.GetBindingFlags() && lhs.GetMutableTypes() == rhs.GetMutableTypes());
    return result;
}

// Canonical dictionary of DSL definitions -- independent of device or handle
using DescriptorSetLayoutDict = hash_util::Dictionary<DescriptorSetLayoutDef, hash_util::HasHashMember<DescriptorSetLayoutDef>>;
using DescriptorSetLayoutId = DescriptorSetLayoutDict::Id;

class DescriptorSetLayout : public BASE_NODE {
  public:
    // Constructors and destructor
    DescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo *p_create_info, const VkDescriptorSetLayout layout);
    virtual ~DescriptorSetLayout() { Destroy(); }

    bool HasBinding(const uint32_t binding) const { return layout_id_->HasBinding(binding); }
    // Return true if this layout is compatible with passed in layout from a pipelineLayout,
    //   else return false and update error_msg with description of incompatibility
    // Return true if this layout is compatible with passed in layout
    bool IsCompatible(DescriptorSetLayout const *rh_ds_layout) const;
    // Straightforward Get functions
    VkDescriptorSetLayout GetDescriptorSetLayout() const { return handle_.Cast<VkDescriptorSetLayout>(); };
    const DescriptorSetLayoutDef *GetLayoutDef() const { return layout_id_.get(); }
    DescriptorSetLayoutId GetLayoutId() const { return layout_id_; }
    uint32_t GetTotalDescriptorCount() const { return layout_id_->GetTotalDescriptorCount(); };
    uint32_t GetDynamicDescriptorCount() const { return layout_id_->GetDynamicDescriptorCount(); };
    uint32_t GetBindingCount() const { return layout_id_->GetBindingCount(); };
    VkDescriptorSetLayoutCreateFlags GetCreateFlags() const { return layout_id_->GetCreateFlags(); }
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
    bool IsTypeMutable(const VkDescriptorType type, uint32_t binding) const { return layout_id_->IsTypeMutable(type, binding); }
    const std::vector<VkDescriptorType> &GetMutableTypes(uint32_t binding) const { return layout_id_->GetMutableTypes(binding); }
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

                // A write update can overlap over following binding but bindings with descriptorCount == 0 must be skipped.
                // Therefore we consider "consistent" a binding that should be skipped
                if(other_binding_ci->descriptorCount != 0) {
                    return false;
                }
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
    DescriptorSetLayoutId layout_id_;
};

/*
 * Descriptor classes
 *  Descriptor is an abstract base class from which 5 separate descriptor types are derived.
 *   This allows the WriteUpdate() and CopyUpdate() operations to be specialized per
 *   descriptor type, but all descriptors in a set can be accessed via the common Descriptor*.
 */

// Slightly broader than type, each c++ "class" will has a corresponding "DescriptorClass"
enum DescriptorClass { PlainSampler, ImageSampler, Image, TexelBuffer, GeneralBuffer, InlineUniform, AccelerationStructure, Mutable, NoDescriptorClass };

DescriptorClass DescriptorTypeToClass(VkDescriptorType type);

class DescriptorSet;

class Descriptor {
  public:
    Descriptor(DescriptorClass class_) : updated(false), descriptor_class(class_), active_descriptor_type(VK_DESCRIPTOR_TYPE_MUTABLE_VALVE) {}
    virtual ~Descriptor(){};
    virtual void WriteUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data, const VkWriteDescriptorSet *,
                             const uint32_t) = 0;
    virtual void CopyUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data, const Descriptor *,
                            bool is_bindless) = 0;
    // Create binding between resources of this descriptor and given cb_node
    DescriptorClass GetClass() const { return descriptor_class; };
    // Special fast-path check for SamplerDescriptors that are immutable
    virtual bool IsImmutableSampler() const { return false; };
    virtual bool AddParent(BASE_NODE *base_node) { return false; }
    virtual void RemoveParent(BASE_NODE *base_node) {}
    virtual void SetDescriptorType(VkDescriptorType type, VkDeviceSize buffer_size) { active_descriptor_type = type; }
    virtual void SetDescriptorType(const Descriptor *src) { active_descriptor_type = src->active_descriptor_type; }

    // return true if resources used by this descriptor are destroyed or otherwise missing
    virtual bool Invalid() const { return false; }

    bool updated;  // Has descriptor been updated?
    DescriptorClass descriptor_class;
    VkDescriptorType active_descriptor_type;
};

// All Dynamic descriptor types
inline bool IsDynamicDescriptor(VkDescriptorType type) {
    return ((type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) || (type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC));
}

inline bool IsBufferDescriptor(VkDescriptorType type) {
    return ((type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) || (type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) ||
            (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) || (type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER));
}

class SamplerDescriptor : public Descriptor {
  public:
    SamplerDescriptor(const ValidationStateTracker *dev_data, const VkSampler *);
    void WriteUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data, const VkWriteDescriptorSet *, const uint32_t) override;
    void CopyUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data, const Descriptor *,
                    bool is_bindless) override;
    virtual bool IsImmutableSampler() const override { return immutable_; };
    VkSampler GetSampler() const { return sampler_state_ ? sampler_state_->sampler() : VK_NULL_HANDLE; }
    const SAMPLER_STATE *GetSamplerState() const { return sampler_state_.get(); }
    SAMPLER_STATE *GetSamplerState() { return sampler_state_.get(); }
    std::shared_ptr<SAMPLER_STATE> GetSharedSamplerState() const { return sampler_state_; }

    bool AddParent(BASE_NODE *base_node) override {
        bool result = false;
        if (sampler_state_) {
            result = sampler_state_->AddParent(base_node);
        }
        return result;
    }
    void RemoveParent(BASE_NODE *base_node) override {
        if (sampler_state_) {
            sampler_state_->RemoveParent(base_node);
        }
    }
    bool Invalid() const override { return !sampler_state_ || sampler_state_->Invalid(); }

  private:
    bool immutable_;
    std::shared_ptr<SAMPLER_STATE> sampler_state_;
};

class ImageDescriptor : public Descriptor {
  public:
    ImageDescriptor(const VkDescriptorType);
    void WriteUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data, const VkWriteDescriptorSet *,
                     const uint32_t) override;
    void CopyUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data, const Descriptor *,
                    bool is_bindless) override;
    void UpdateDrawState(ValidationStateTracker *, CMD_BUFFER_STATE *);
    VkImageView GetImageView() const { return image_view_state_ ? image_view_state_->image_view() : VK_NULL_HANDLE; }
    const IMAGE_VIEW_STATE *GetImageViewState() const { return image_view_state_.get(); }
    IMAGE_VIEW_STATE *GetImageViewState() { return image_view_state_.get(); }
    std::shared_ptr<IMAGE_VIEW_STATE> GetSharedImageViewState() const { return image_view_state_; }
    VkImageLayout GetImageLayout() const { return image_layout_; }

    bool AddParent(BASE_NODE *base_node) override {
        bool result = false;
        if (image_view_state_) {
            result = image_view_state_->AddParent(base_node);
        }
        return result;
    }
    void RemoveParent(BASE_NODE *base_node) override {
        if (image_view_state_) {
            image_view_state_->RemoveParent(base_node);
        }
    }

    bool Invalid() const override { return !image_view_state_ || image_view_state_->Invalid(); }

  protected:
    ImageDescriptor(DescriptorClass class_);
    std::shared_ptr<IMAGE_VIEW_STATE> image_view_state_;
    VkImageLayout image_layout_;
};

class ImageSamplerDescriptor : public ImageDescriptor {
  public:
    ImageSamplerDescriptor(const ValidationStateTracker *dev_data, const VkSampler *);
    void WriteUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data, const VkWriteDescriptorSet *, const uint32_t) override;
    void CopyUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data, const Descriptor *,
                    bool is_bindless) override;
    virtual bool IsImmutableSampler() const override { return immutable_; };
    VkSampler GetSampler() const { return sampler_state_ ? sampler_state_->sampler() : VK_NULL_HANDLE; }
    const SAMPLER_STATE *GetSamplerState() const { return sampler_state_.get(); }
    SAMPLER_STATE *GetSamplerState() { return sampler_state_.get(); }
    std::shared_ptr<SAMPLER_STATE> GetSharedSamplerState() const { return sampler_state_; }

    bool AddParent(BASE_NODE *base_node) override {
        bool result = ImageDescriptor::AddParent(base_node);
        if (sampler_state_) {
            result |= sampler_state_->AddParent(base_node);
        }
        return result;
    }
    void RemoveParent(BASE_NODE *base_node) override {
        ImageDescriptor::RemoveParent(base_node);
        if (sampler_state_) {
            sampler_state_->RemoveParent(base_node);
        }
    }

    bool Invalid() const override { return ImageDescriptor::Invalid() || !sampler_state_ || sampler_state_->Invalid(); }

  private:
    std::shared_ptr<SAMPLER_STATE> sampler_state_;
    bool immutable_;
};

class TexelDescriptor : public Descriptor {
  public:
    TexelDescriptor(const VkDescriptorType);
    void WriteUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data, const VkWriteDescriptorSet *,
                     const uint32_t) override;
    void CopyUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data, const Descriptor *,
                    bool is_bindless) override;
    VkBufferView GetBufferView() const { return buffer_view_state_ ? buffer_view_state_->buffer_view() : VK_NULL_HANDLE; }
    const BUFFER_VIEW_STATE *GetBufferViewState() const { return buffer_view_state_.get(); }
    BUFFER_VIEW_STATE *GetBufferViewState() { return buffer_view_state_.get(); }
    std::shared_ptr<BUFFER_VIEW_STATE> GetSharedBufferViewState() const { return buffer_view_state_; }

    bool AddParent(BASE_NODE *base_node) override {
        bool result = false;
        if (buffer_view_state_) {
            result = buffer_view_state_->AddParent(base_node);
        }
        return result;
    }
    void RemoveParent(BASE_NODE *base_node) override {
        if (buffer_view_state_) {
            buffer_view_state_->RemoveParent(base_node);
        }
    }

    bool Invalid() const override { return !buffer_view_state_ || buffer_view_state_->Invalid(); }

  private:
    std::shared_ptr<BUFFER_VIEW_STATE> buffer_view_state_;
};

class BufferDescriptor : public Descriptor {
  public:
    BufferDescriptor(const VkDescriptorType);
    void WriteUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data, const VkWriteDescriptorSet *,
                     const uint32_t) override;
    void CopyUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data, const Descriptor *,
                    bool is_bindless) override;
    VkBuffer GetBuffer() const { return buffer_state_ ? buffer_state_->buffer() : VK_NULL_HANDLE; }
    const BUFFER_STATE *GetBufferState() const { return buffer_state_.get(); }
    BUFFER_STATE *GetBufferState() { return buffer_state_.get(); }
    std::shared_ptr<BUFFER_STATE> GetSharedBufferState() const { return buffer_state_; }
    VkDeviceSize GetOffset() const { return offset_; }
    VkDeviceSize GetRange() const { return range_; }

    bool AddParent(BASE_NODE *base_node) override {
        bool result = false;
        if (buffer_state_) {
            result = buffer_state_->AddParent(base_node);
        }
        return result;
    }
    void RemoveParent(BASE_NODE *base_node) override {
        if (buffer_state_) {
            buffer_state_->RemoveParent(base_node);
        }
    }
    bool Invalid() const override { return !buffer_state_ || buffer_state_->Invalid(); }

  private:
    VkDeviceSize offset_;
    VkDeviceSize range_;
    std::shared_ptr<BUFFER_STATE> buffer_state_;
};

class InlineUniformDescriptor : public Descriptor {
  public:
    InlineUniformDescriptor(const VkDescriptorType) : Descriptor(InlineUniform) {}
    void WriteUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data, const VkWriteDescriptorSet *, const uint32_t) override {
        updated = true;
    }
    void CopyUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data, const Descriptor *,
                    bool is_bindless) override {
        updated = true;
    }
};

class AccelerationStructureDescriptor : public Descriptor {
  public:
    AccelerationStructureDescriptor(const VkDescriptorType);
    void WriteUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data, const VkWriteDescriptorSet *, const uint32_t) override;
    VkAccelerationStructureKHR GetAccelerationStructure() const { return acc_; }
    const ACCELERATION_STRUCTURE_STATE_KHR *GetAccelerationStructureStateKHR() const { return acc_state_.get(); }
    ACCELERATION_STRUCTURE_STATE_KHR *GetAccelerationStructureStateKHR() { return acc_state_.get(); }
    VkAccelerationStructureNV GetAccelerationStructureNV() const { return acc_nv_; }
    const ACCELERATION_STRUCTURE_STATE *GetAccelerationStructureStateNV() const { return acc_state_nv_.get(); }
    ACCELERATION_STRUCTURE_STATE *GetAccelerationStructureStateNV() { return acc_state_nv_.get(); }
    void CopyUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data, const Descriptor *,
                    bool is_bindless) override;
    bool is_khr() const { return is_khr_; }

    bool AddParent(BASE_NODE *base_node) override {
        bool result = false;
        if (acc_state_) {
            result |= acc_state_->AddParent(base_node);
        }
        if (acc_state_nv_) {
            result |= acc_state_nv_->AddParent(base_node);
        }
        return result;
    }
    void RemoveParent(BASE_NODE *base_node) override {
        if (acc_state_) {
            acc_state_->RemoveParent(base_node);
        }
        if (acc_state_nv_) {
            acc_state_nv_->RemoveParent(base_node);
        }
    }
    bool Invalid() const override {
        if (is_khr_) {
            return !acc_state_ || acc_state_->Invalid();
        } else {
            return !acc_state_nv_ || acc_state_nv_->Invalid();
        }
    }

  private:
    bool is_khr_;
    VkAccelerationStructureKHR acc_;
    std::shared_ptr<ACCELERATION_STRUCTURE_STATE_KHR> acc_state_;
    VkAccelerationStructureNV acc_nv_;
    std::shared_ptr<ACCELERATION_STRUCTURE_STATE> acc_state_nv_;
};

class MutableDescriptor : public Descriptor {
  public:
      MutableDescriptor();
      void WriteUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data, const VkWriteDescriptorSet *,
                       const uint32_t) override;
      void CopyUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data, const Descriptor *,
                      bool is_bindless) override;

      void SetDescriptorType(VkDescriptorType type, VkDeviceSize buffer_size) override {
          active_descriptor_type = type;
          buffer_size_ = buffer_size;
      }
      void SetDescriptorType(const Descriptor *src) override {
          active_descriptor_type = src->active_descriptor_type;
          if (src->GetClass() == cvdescriptorset::DescriptorClass::GeneralBuffer) {
              auto buffer = static_cast<const cvdescriptorset::BufferDescriptor *>(src)->GetBuffer();
              if (buffer == VK_NULL_HANDLE) {
                  buffer_size_ = std::numeric_limits<uint32_t>::max();
              } else {
                  auto buffer_state = static_cast<const cvdescriptorset::BufferDescriptor *>(src)->GetBufferState();
                  buffer_size_ = static_cast<uint32_t>(buffer_state->createInfo.size);
              }
          } else if (src->GetClass() == cvdescriptorset::DescriptorClass::TexelBuffer) {
              auto buffer_view = static_cast<const cvdescriptorset::TexelDescriptor *>(src)->GetBufferView();
              if (buffer_view == VK_NULL_HANDLE) {
                  buffer_size_ = std::numeric_limits<uint32_t>::max();
              } else {
                  auto buffer_view_state = static_cast<const cvdescriptorset::TexelDescriptor *>(src)->GetBufferViewState();
                  buffer_size_ = static_cast<uint32_t>(buffer_view_state->buffer_state->createInfo.size);
              }
          } else if (src->GetClass() == cvdescriptorset::DescriptorClass::Mutable) {
              auto descriptor = static_cast<const cvdescriptorset::MutableDescriptor *>(src);
              buffer_size_ = descriptor->GetBufferSize();
          } else {
              buffer_size_ = 0;
          }
      }
      VkDeviceSize GetBufferSize() const { return buffer_size_; }

      std::shared_ptr<SAMPLER_STATE> GetSharedSamplerState() const { return sampler_state_; }
      std::shared_ptr<IMAGE_VIEW_STATE> GetSharedImageViewState() const { return image_view_state_; }
      VkImageLayout GetImageLayout() const { return image_layout_; }
      std::shared_ptr<BUFFER_STATE> GetSharedBufferState() const { return buffer_state_; }
      VkDeviceSize GetOffset() const { return offset_; }
      VkDeviceSize GetRange() const { return range_; }
      std::shared_ptr<BUFFER_VIEW_STATE> GetSharedBufferViewState() const { return buffer_view_state_; }
      VkAccelerationStructureKHR GetAccelerationStructure() const { return acc_; }
      const ACCELERATION_STRUCTURE_STATE_KHR *GetAccelerationStructureStateKHR() const { return acc_state_.get(); }
      ACCELERATION_STRUCTURE_STATE_KHR *GetAccelerationStructureStateKHR() { return acc_state_.get(); }
      VkAccelerationStructureNV GetAccelerationStructureNV() const { return acc_nv_; }
      const ACCELERATION_STRUCTURE_STATE *GetAccelerationStructureStateNV() const { return acc_state_nv_.get(); }
      ACCELERATION_STRUCTURE_STATE *GetAccelerationStructureStateNV() { return acc_state_nv_.get(); }

      void UpdateDrawState(ValidationStateTracker *, CMD_BUFFER_STATE *);

      bool AddParent(BASE_NODE *base_node) override;
      void RemoveParent(BASE_NODE *base_node) override;

      bool Invalid() const override;

    private:
      VkDeviceSize buffer_size_;
      DescriptorClass active_descriptor_class_;

      // Sampler and ImageSampler Descriptor
      bool immutable_;
      std::shared_ptr<SAMPLER_STATE> sampler_state_;
      // Image Descriptor
      std::shared_ptr<IMAGE_VIEW_STATE> image_view_state_;
      VkImageLayout image_layout_;
      // Texel Descriptor
      std::shared_ptr<BUFFER_VIEW_STATE> buffer_view_state_;
      // Buffer Descriptor
      VkDeviceSize offset_;
      VkDeviceSize range_;
      std::shared_ptr<BUFFER_STATE> buffer_state_;
      // Acceleration Structure Descriptor
      bool is_khr_;
      VkAccelerationStructureKHR acc_;
      std::shared_ptr<ACCELERATION_STRUCTURE_STATE_KHR> acc_state_;
      VkAccelerationStructureNV acc_nv_;
      std::shared_ptr<ACCELERATION_STRUCTURE_STATE> acc_state_nv_;
};

// Structs to contain common elements that need to be shared between Validate* and Perform* calls below
struct AllocateDescriptorSetsData {
    std::map<uint32_t, uint32_t> required_descriptors_by_type;
    std::vector<std::shared_ptr<DescriptorSetLayout const>> layout_nodes;
    void Init(uint32_t);
    AllocateDescriptorSetsData(){};
};
// "Perform" does the update with the assumption that ValidateUpdateDescriptorSets() has passed for the given update
void PerformUpdateDescriptorSets(ValidationStateTracker *, uint32_t, const VkWriteDescriptorSet *, uint32_t,
                                 const VkCopyDescriptorSet *);

class DescriptorBinding {
  public:
    DescriptorBinding(const VkDescriptorSetLayoutBinding &create_info, uint32_t count_, VkDescriptorBindingFlags binding_flags_)
        : binding(create_info.binding),
          type(create_info.descriptorType),
          descriptor_class(DescriptorTypeToClass(type)),
          stage_flags(create_info.stageFlags),
          binding_flags(binding_flags_),
          count(count_),
          has_immutable_samplers(create_info.pImmutableSamplers != nullptr) {}
    virtual ~DescriptorBinding() {}

    virtual void AddParent(DescriptorSet *ds) = 0;
    virtual void RemoveParent(DescriptorSet *ds) = 0;

    virtual const Descriptor *GetDescriptor(const uint32_t index) const = 0;
    virtual Descriptor *GetDescriptor(const uint32_t index) = 0;

    bool IsBindless() const {
        return (binding_flags & (VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT)) != 0;
    }

    bool IsConsistent(const DescriptorBinding &other) const {
        // A write update can overlap over following binding but bindings with descriptorCount == 0 must be skipped.
        // Therefore we consider "consistent" a binding that should be skipped
        if (other.count == 0) {
            return true;
        }
        return type == other.type && stage_flags == other.stage_flags && binding_flags == other.binding_flags &&
               has_immutable_samplers == other.has_immutable_samplers;
    }

    const uint32_t binding;
    const VkDescriptorType type;
    const DescriptorClass descriptor_class;
    const VkShaderStageFlags stage_flags;
    const VkDescriptorBindingFlags binding_flags;
    const uint32_t count;
    const bool has_immutable_samplers;
};

template <typename T>
class DescriptorBindingImpl : public DescriptorBinding {
  public:
    DescriptorBindingImpl(const VkDescriptorSetLayoutBinding &create_info, uint32_t count_, VkDescriptorBindingFlags binding_flags_)
        : DescriptorBinding(create_info, count_, binding_flags_) {
        descriptors.reserve(count);
    }
    const Descriptor *GetDescriptor(const uint32_t index) const override {
        return index < descriptors.size() ? &descriptors[index] : nullptr;
    }
    Descriptor *GetDescriptor(const uint32_t index) override { return index < descriptors.size() ? &descriptors[index] : nullptr; }

    void AddParent(DescriptorSet *ds) override {
        for (auto &desc : descriptors) {
            desc.AddParent(ds);
        }
    }
    void RemoveParent(DescriptorSet *ds) override {
        for (auto &desc : descriptors) {
            desc.RemoveParent(ds);
        }
    }
    std::vector<T> descriptors;
};

using SamplerBinding = DescriptorBindingImpl<SamplerDescriptor>;
using ImageBinding = DescriptorBindingImpl<ImageDescriptor>;
using ImageSamplerBinding = DescriptorBindingImpl<ImageSamplerDescriptor>;
using TexelBinding = DescriptorBindingImpl<TexelDescriptor>;
using BufferBinding = DescriptorBindingImpl<BufferDescriptor>;
using InlineUniformBinding = DescriptorBindingImpl<InlineUniformDescriptor>;
using AccelerationStructureBinding = DescriptorBindingImpl<AccelerationStructureDescriptor>;
using MutableBinding = DescriptorBindingImpl<MutableDescriptor>;

// Helper class to encapsulate the descriptor update template decoding logic
struct DecodedTemplateUpdate {
    std::vector<VkWriteDescriptorSet> desc_writes;
    std::vector<VkWriteDescriptorSetInlineUniformBlockEXT> inline_infos;
    std::vector<VkWriteDescriptorSetAccelerationStructureKHR> inline_infos_khr;
    std::vector<VkWriteDescriptorSetAccelerationStructureNV> inline_infos_nv;
    DecodedTemplateUpdate(const ValidationStateTracker *device_data, VkDescriptorSet descriptorSet,
                          const UPDATE_TEMPLATE_STATE *template_state, const void *pData,
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
    void LinkChildNodes() override;
    ~DescriptorSet() { Destroy(); }

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
    VkDescriptorSet GetSet() const { return handle_.Cast<VkDescriptorSet>(); };
    // Bind given cmd_buffer to this descriptor set and
    // update CB image layout map with image/imagesampler descriptor image layouts
    void UpdateDrawState(ValidationStateTracker *, CMD_BUFFER_STATE *, CMD_TYPE cmd_type, const PIPELINE_STATE *,
                         const BindingReqMap &);

    // Track work that has been bound or validated to avoid duplicate work, important when large descriptor arrays
    // are present
    typedef layer_data::unordered_set<uint32_t> TrackedBindings;
    static void FilterOneBindingReq(const BindingReqMap::value_type &binding_req_pair, BindingReqMap *out_req,
                                    const TrackedBindings &set, uint32_t limit);
    void FilterBindingReqs(const CMD_BUFFER_STATE &, const PIPELINE_STATE &, const BindingReqMap &in_req,
                           BindingReqMap *out_req) const;
    void UpdateValidationCache(CMD_BUFFER_STATE &cb_state, const PIPELINE_STATE &pipeline, const BindingReqMap &updated_bindings);

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
    bool IsBindless(uint32_t binding) const {
        return (layout_->GetDescriptorBindingFlagsFromBinding(binding) &
                (VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT)) > 0;
    }
    uint32_t GetVariableDescriptorCount() const { return variable_count_; }
    DESCRIPTOR_POOL_STATE *GetPoolState() const { return pool_state_; }

    const Descriptor *GetDescriptorFromGlobalIndex(const uint32_t index) const {
        for (uint32_t i = 0; i < bindings_.size(); i++) {
            auto range = layout_->GetGlobalIndexRangeFromIndex(i);
            if (range.start <= index && index < range.end) {
                return bindings_[i]->GetDescriptor(index - range.start);
            }
        }
        return nullptr;
    }
    Descriptor *GetDescriptorFromGlobalIndex(const uint32_t index) {
        for (uint32_t i = 0; i < bindings_.size(); i++) {
            auto range = layout_->GetGlobalIndexRangeFromIndex(i);
            if (range.start <= index && index < range.end) {
                return bindings_[i]->GetDescriptor(index - range.start);
            }
        }
        return nullptr;
    }

    const DescriptorBinding *GetBinding(uint32_t binding) const {
        auto index = layout_->GetIndexFromBinding(binding);
        return index < bindings_.size() ? bindings_[index].get() : nullptr;
    }

    DescriptorBinding *GetBinding(uint32_t binding) {
        auto index = layout_->GetIndexFromBinding(binding);
        return index < bindings_.size() ? bindings_[index].get() : nullptr;
    }

    const Descriptor *GetDescriptorFromBinding(const uint32_t binding, const uint32_t index = 0) const {
        const auto *binding_data = GetBinding(binding);
        return binding_data ? binding_data->GetDescriptor(index) : nullptr;
    }

    Descriptor *GetDescriptorFromBinding(const uint32_t binding, const uint32_t index = 0) {
        auto *binding_data = GetBinding(binding);
        return binding_data ? binding_data->GetDescriptor(index) : nullptr;
    }

    // For a given dynamic offset array, return the corresponding index into the list of descriptors in set
    const Descriptor *GetDescriptorFromDynamicOffsetIndex(const uint32_t index) const {
        auto pos = dynamic_offset_idx_to_descriptor_list_.at(index);
        return bindings_[pos.first]->GetDescriptor(pos.second);
    }
    uint64_t GetChangeCount() const { return change_count_; }

    const std::vector<safe_VkWriteDescriptorSet> &GetWrites() const { return push_descriptor_set_writes; }

    void Destroy() override;

    // Cached binding and validation support:
    //
    // For the lifespan of a given command buffer recording, do lazy evaluation, caching, and dirtying of
    // expensive validation operation (typically per-draw)
    // Track the validation caching of bindings vs. the command buffer and draw state
    typedef layer_data::unordered_map<uint32_t, uint64_t> VersionedBindings;
    // this structure is stored in a map in CMD_BUFFER_STATE, with an entry for every descriptor set.
    struct CachedValidation {
        TrackedBindings command_binding_and_usage;                                     // Persistent for the life of the recording
        TrackedBindings non_dynamic_buffers;                                           // Persistent for the life of the recording
        TrackedBindings dynamic_buffers;                                               // Dirtied (flushed) each BindDescriptorSet
        layer_data::unordered_map<const PIPELINE_STATE *, VersionedBindings> image_samplers;  // Tested vs. changes to CB's ImageLayout
    };
  private:
    // Private helper to set all bound cmd buffers to INVALID state
    void InvalidateBoundCmdBuffers(ValidationStateTracker *state_data);
    bool some_update_;  // has any part of the set ever been updated?
    DESCRIPTOR_POOL_STATE *pool_state_;
    const std::shared_ptr<DescriptorSetLayout const> layout_;
    // NOTE: the the backing store for the descriptors must be declared *before* it so it will be destructed *after* it
    // "Destructors for nonstatic member objects are called in the reverse order in which they appear in the class declaration."
    std::vector<std::unique_ptr<DescriptorBinding>> bindings_;
    const StateTracker *state_data_;
    uint32_t variable_count_;
    uint64_t change_count_;

    // For a given dynamic offset index in the set, map to associated index of the descriptors in the set
    std::vector<std::pair<uint32_t, uint32_t>> dynamic_offset_idx_to_descriptor_list_;

    // If this descriptor set is a push descriptor set, the descriptor
    // set writes that were last pushed.
    std::vector<safe_VkWriteDescriptorSet> push_descriptor_set_writes;
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
