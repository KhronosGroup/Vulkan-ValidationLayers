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

// DescriptorSetLayoutDef/DescriptorSetLayout classes

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
    uint32_t GetIndexFromBinding(uint32_t binding) const;
    // Various Get functions that can either be passed a binding#, which will be automatically translated into the appropriate
    // index, or the index# can be passed in directly
    uint32_t GetMaxBinding() const { return bindings_[bindings_.size() - 1].binding; }
    VkDescriptorSetLayoutBinding const *GetDescriptorSetLayoutBindingPtrFromIndex(const uint32_t) const;
    VkDescriptorSetLayoutBinding const *GetDescriptorSetLayoutBindingPtrFromBinding(uint32_t binding) const {
        return GetDescriptorSetLayoutBindingPtrFromIndex(GetIndexFromBinding(binding));
    }
    const std::vector<safe_VkDescriptorSetLayoutBinding> &GetBindings() const { return bindings_; }
    const std::vector<VkDescriptorBindingFlagsEXT> &GetBindingFlags() const { return binding_flags_; }
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
    VkDescriptorBindingFlagsEXT GetDescriptorBindingFlagsFromIndex(const uint32_t) const;
    VkDescriptorBindingFlagsEXT GetDescriptorBindingFlagsFromBinding(const uint32_t binding) const {
        return GetDescriptorBindingFlagsFromIndex(GetIndexFromBinding(binding));
    }
    uint32_t GetIndexFromGlobalIndex(const uint32_t global_index) const;
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

    // Helper function to get the next valid binding for a descriptor
    uint32_t GetNextValidBinding(const uint32_t) const;
    bool IsPushDescriptor() const { return GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR; };

   private:
    // Only the first three data members are used for hash and equality checks, the other members are derived from them, and are
    // used to speed up the various lookups/queries/validations
    VkDescriptorSetLayoutCreateFlags flags_;
    std::vector<safe_VkDescriptorSetLayoutBinding> bindings_;
    std::vector<VkDescriptorBindingFlagsEXT> binding_flags_;

    // Convenience data structures for rapid lookup of various descriptor set layout properties
    std::set<uint32_t> non_empty_bindings_;  // Containing non-emtpy bindings in numerical order
    std::unordered_map<uint32_t, uint32_t> binding_to_index_map_;
    // The following map allows an non-iterative lookup of a binding from a global index...
    std::map<uint32_t, uint32_t> global_start_to_index_map_;  // The index corresponding for a starting global (descriptor) index
    std::unordered_map<uint32_t, IndexRange> binding_to_global_index_range_map_;  // range is exclusive of .end
    // For a given binding map to associated index in the dynamic offset array
    std::unordered_map<uint32_t, uint32_t> binding_to_dynamic_array_idx_map_;

    uint32_t binding_count_;     // # of bindings in this layout
    uint32_t descriptor_count_;  // total # descriptors in this layout
    uint32_t dynamic_descriptor_count_;
};

static inline bool operator==(const DescriptorSetLayoutDef &lhs, const DescriptorSetLayoutDef &rhs) {
    bool result = (lhs.GetCreateFlags() == rhs.GetCreateFlags()) && (lhs.GetBindings() == rhs.GetBindings()) &&
                  (lhs.GetBindingFlags() == rhs.GetBindingFlags());
    return result;
}

// Canonical dictionary of DSL definitions -- independent of device or handle
using DescriptorSetLayoutDict = hash_util::Dictionary<DescriptorSetLayoutDef, hash_util::HasHashMember<DescriptorSetLayoutDef>>;
using DescriptorSetLayoutId = DescriptorSetLayoutDict::Id;

class DescriptorSetLayout {
   public:
    // Constructors and destructor
    DescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo *p_create_info, const VkDescriptorSetLayout layout);
    bool HasBinding(const uint32_t binding) const { return layout_id_->HasBinding(binding); }
    // Straightforward Get functions
    VkDescriptorSetLayout GetDescriptorSetLayout() const { return layout_; };
    bool IsDestroyed() const { return layout_destroyed_; }
    void MarkDestroyed() { layout_destroyed_ = true; }
    const DescriptorSetLayoutDef *GetLayoutDef() const { return layout_id_.get(); }
    DescriptorSetLayoutId GetLayoutId() const { return layout_id_; }
    uint32_t GetTotalDescriptorCount() const { return layout_id_->GetTotalDescriptorCount(); };
    uint32_t GetDynamicDescriptorCount() const { return layout_id_->GetDynamicDescriptorCount(); };
    uint32_t GetBindingCount() const { return layout_id_->GetBindingCount(); };
    VkDescriptorSetLayoutCreateFlags GetCreateFlags() const { return layout_id_->GetCreateFlags(); }
    uint32_t GetIndexFromBinding(uint32_t binding) const { return layout_id_->GetIndexFromBinding(binding); }
    // Various Get functions that can either be passed a binding#, which will be automatically translated into the appropriate
    // index, or the index# can be passed in directly
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
    VkDescriptorBindingFlagsEXT GetDescriptorBindingFlagsFromIndex(const uint32_t index) const {
        return layout_id_->GetDescriptorBindingFlagsFromIndex(index);
    }
    VkDescriptorBindingFlagsEXT GetDescriptorBindingFlagsFromBinding(const uint32_t binding) const {
        return layout_id_->GetDescriptorBindingFlagsFromBinding(binding);
    }
    uint32_t GetIndexFromGlobalIndex(const uint32_t global_index) const {
        return layout_id_->GetIndexFromGlobalIndex(global_index);
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
    // Helper function to get the next valid binding for a descriptor
    uint32_t GetNextValidBinding(const uint32_t binding) const { return layout_id_->GetNextValidBinding(binding); }
    bool IsPushDescriptor() const { return layout_id_->IsPushDescriptor(); }

   private:
    VkDescriptorSetLayout layout_;
    bool layout_destroyed_;
    DescriptorSetLayoutId layout_id_;
};

// Slightly broader than type, each c++ "class" will has a corresponding "DescriptorClass"
enum DescriptorClass { PlainSampler, ImageSampler, Image, TexelBuffer, GeneralBuffer, InlineUniform, AccelerationStructure };

class Descriptor {
   public:
    Descriptor();
    virtual ~Descriptor(){};
    virtual void WriteUpdate(const VkWriteDescriptorSet *, const uint32_t) { updated = true; };
    virtual void CopyUpdate(const Descriptor *) { updated = true; };
    void BindCommandBuffer(CMD_BUFFER_STATE *);
    void UpdateDrawState(CoreChecks *, CMD_BUFFER_STATE *);
    bool updated;  // Has descriptor been updated?
};

struct AllocateDescriptorSetsData {
    std::map<uint32_t, uint32_t> required_descriptors_by_type;
    std::vector<std::shared_ptr<DescriptorSetLayout const>> layout_nodes;
    AllocateDescriptorSetsData(uint32_t);
};

// DescriptorSet class
//
// Overview - This class encapsulates the Vulkan VkDescriptorSet data (set).
//  A set has an underlying layout which defines the bindings in the set and the types and numbers of descriptors in each descriptor
//  slot. Most of the layout interfaces are exposed through identically-named functions in the set class. Please refer to the
//  DescriptorSetLayout comment above for a description of index, binding, and global index.
// At construction a vector of Descriptor* is created with types corresponding to the layout. The primary operation performed on the
// descriptors is to update them via write or copy updates.
class DescriptorSet : public BASE_NODE {
   public:
    DescriptorSet(const VkDescriptorSet, const VkDescriptorPool, const std::shared_ptr<DescriptorSetLayout const> &,
                  uint32_t variable_count, CoreChecks *);
    ~DescriptorSet();
    // A number of common Get* functions that return data based on layout from which this set was created
    uint32_t GetTotalDescriptorCount() const { return p_layout_->GetTotalDescriptorCount(); };
    uint32_t GetDynamicDescriptorCount() const { return p_layout_->GetDynamicDescriptorCount(); };
    uint32_t GetBindingCount() const { return p_layout_->GetBindingCount(); };
    VkDescriptorType GetTypeFromIndex(const uint32_t index) const { return p_layout_->GetTypeFromIndex(index); };
    VkDescriptorType GetTypeFromBinding(const uint32_t binding) const { return p_layout_->GetTypeFromBinding(binding); };
    uint32_t GetDescriptorCountFromIndex(const uint32_t index) const { return p_layout_->GetDescriptorCountFromIndex(index); };
    uint32_t GetDescriptorCountFromBinding(const uint32_t binding) const {
        return p_layout_->GetDescriptorCountFromBinding(binding);
    };
    // Return index into dynamic offset array for given binding
    int32_t GetDynamicOffsetIndexFromBinding(uint32_t binding) const {
        return p_layout_->GetDynamicOffsetIndexFromBinding(binding);
    }
    // Return true if given binding is present in this set
    bool HasBinding(const uint32_t binding) const { return p_layout_->HasBinding(binding); };

    const std::shared_ptr<DescriptorSetLayout const> GetLayout() const { return p_layout_; };
    VkDescriptorSet GetSet() const { return set_; };
    // Return unordered_set of all command buffers that this set is bound to
    std::unordered_set<CMD_BUFFER_STATE *> GetBoundCmdBuffers() const { return cb_bindings; }
    // Bind given cmd_buffer to this descriptor set and  update CB image layout map with image/imagesampler descriptor image layouts
    void UpdateDrawState(CoreChecks *, CMD_BUFFER_STATE *, const std::map<uint32_t, descriptor_req> &);
    void BindCommandBuffer(CMD_BUFFER_STATE *, const std::map<uint32_t, descriptor_req> &);

    // If given cmd_buffer is in the cb_bindings set, remove it
    void RemoveBoundCommandBuffer(CMD_BUFFER_STATE *cb_node) { cb_bindings.erase(cb_node); }
    // For a particular binding, get the global index
    const IndexRange GetGlobalIndexRangeFromBinding(const uint32_t binding, bool actual_length = false) const {
        if (actual_length && binding == p_layout_->GetMaxBinding() && IsVariableDescriptorCount(binding)) {
            IndexRange range = p_layout_->GetGlobalIndexRangeFromBinding(binding);
            auto diff = GetDescriptorCountFromBinding(binding) - GetVariableDescriptorCount();
            range.end -= diff;
            return range;
        }
        return p_layout_->GetGlobalIndexRangeFromBinding(binding);
    };
    // Return true if any part of set has ever been updated
    bool IsPushDescriptor() const { return p_layout_->IsPushDescriptor(); };
    bool IsVariableDescriptorCount(uint32_t binding) const {
        return !!(p_layout_->GetDescriptorBindingFlagsFromBinding(binding) &
                  VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT);
    }
    bool IsUpdateAfterBind(uint32_t binding) const {
        return !!(p_layout_->GetDescriptorBindingFlagsFromBinding(binding) & VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT);
    }
    uint32_t GetVariableDescriptorCount() const { return variable_count_; }
    DESCRIPTOR_POOL_STATE *GetPoolState() const { return pool_state_; }
    const Descriptor *GetDescriptorFromGlobalIndex(const uint32_t index) const { return descriptors_[index].get(); }

   private:
    VkDescriptorSet set_;
    DESCRIPTOR_POOL_STATE *pool_state_;
    const std::shared_ptr<DescriptorSetLayout const> p_layout_;
    std::vector<std::unique_ptr<Descriptor>> descriptors_;
    CoreChecks *device_data_;
    const VkPhysicalDeviceLimits limits_;
    uint32_t variable_count_;
};
}  // namespace cvdescriptorset
#endif  // CORE_VALIDATION_DESCRIPTOR_SETS_H_
