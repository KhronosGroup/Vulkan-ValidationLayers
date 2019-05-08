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

// Allow use of STL min and max functions in Windows
#define NOMINMAX

#include "chassis.h"
#include "core_validation_error_enums.h"
#include "core_validation.h"
#include "descriptor_sets.h"
#include "hash_vk_types.h"
#include "vk_enum_string_helper.h"
#include "vk_safe_struct.h"
#include "vk_typemap_helper.h"
#include <sstream>
#include <algorithm>
#include <array>
#include <memory>

// ExtendedBinding collects a VkDescriptorSetLayoutBinding and any extended
// state that comes from a different array/structure so they can stay together
// while being sorted by binding number.
struct ExtendedBinding {
    ExtendedBinding(const VkDescriptorSetLayoutBinding *l, VkDescriptorBindingFlagsEXT f) : layout_binding(l), binding_flags(f) {}

    const VkDescriptorSetLayoutBinding *layout_binding;
    VkDescriptorBindingFlagsEXT binding_flags;
};

struct BindingNumCmp {
    bool operator()(const ExtendedBinding &a, const ExtendedBinding &b) const {
        return a.layout_binding->binding < b.layout_binding->binding;
    }
};

using DescriptorSetLayoutDef = cvdescriptorset::DescriptorSetLayoutDef;
using DescriptorSetLayoutId = cvdescriptorset::DescriptorSetLayoutId;

// Canonical dictionary of DescriptorSetLayoutDef (without any handle/device specific information)
cvdescriptorset::DescriptorSetLayoutDict descriptor_set_layout_dict;

DescriptorSetLayoutId GetCanonicalId(const VkDescriptorSetLayoutCreateInfo *p_create_info) {
    return descriptor_set_layout_dict.look_up(DescriptorSetLayoutDef(p_create_info));
}

// Construct DescriptorSetLayout instance from given create info
cvdescriptorset::DescriptorSetLayoutDef::DescriptorSetLayoutDef(const VkDescriptorSetLayoutCreateInfo *p_create_info)
    : flags_(p_create_info->flags), binding_count_(0), descriptor_count_(0), dynamic_descriptor_count_(0) {
    const auto *flags_create_info = lvl_find_in_chain<VkDescriptorSetLayoutBindingFlagsCreateInfoEXT>(p_create_info->pNext);

    std::set<ExtendedBinding, BindingNumCmp> sorted_bindings;
    const uint32_t input_bindings_count = p_create_info->bindingCount;
    // Sort the input bindings in binding number order, eliminating duplicates
    for (uint32_t i = 0; i < input_bindings_count; i++) {
        VkDescriptorBindingFlagsEXT flags = 0;
        if (flags_create_info && flags_create_info->bindingCount == p_create_info->bindingCount) {
            flags = flags_create_info->pBindingFlags[i];
        }
        sorted_bindings.insert(ExtendedBinding(p_create_info->pBindings + i, flags));
    }

    // Store the create info in the sorted order from above
    std::map<uint32_t, uint32_t> binding_to_dyn_count;
    uint32_t index = 0;
    binding_count_ = static_cast<uint32_t>(sorted_bindings.size());
    bindings_.reserve(binding_count_);
    binding_flags_.reserve(binding_count_);
    binding_to_index_map_.reserve(binding_count_);
    for (auto input_binding : sorted_bindings) {
        // Add to binding and map, s.t. it is robust to invalid duplication of binding_num
        const auto binding_num = input_binding.layout_binding->binding;
        binding_to_index_map_[binding_num] = index++;
        bindings_.emplace_back(input_binding.layout_binding);
        auto &binding_info = bindings_.back();
        binding_flags_.emplace_back(input_binding.binding_flags);

        descriptor_count_ += binding_info.descriptorCount;
        if (binding_info.descriptorCount > 0) {
            non_empty_bindings_.insert(binding_num);
        }

        if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
            binding_info.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) {
            binding_to_dyn_count[binding_num] = binding_info.descriptorCount;
            dynamic_descriptor_count_ += binding_info.descriptorCount;
        }
    }
    assert(bindings_.size() == binding_count_);
    assert(binding_flags_.size() == binding_count_);
    uint32_t global_index = 0;
    binding_to_global_index_range_map_.reserve(binding_count_);
    // Vector order is finalized so create maps of bindings to descriptors and descriptors to indices
    for (uint32_t i = 0; i < binding_count_; ++i) {
        auto binding_num = bindings_[i].binding;
        auto final_index = global_index + bindings_[i].descriptorCount;
        binding_to_global_index_range_map_[binding_num] = IndexRange(global_index, final_index);
        if (final_index != global_index) {
            global_start_to_index_map_[global_index] = i;
        }
        global_index = final_index;
    }

    // Now create dyn offset array mapping for any dynamic descriptors
    uint32_t dyn_array_idx = 0;
    binding_to_dynamic_array_idx_map_.reserve(binding_to_dyn_count.size());
    for (const auto &bc_pair : binding_to_dyn_count) {
        binding_to_dynamic_array_idx_map_[bc_pair.first] = dyn_array_idx;
        dyn_array_idx += bc_pair.second;
    }
}

size_t cvdescriptorset::DescriptorSetLayoutDef::hash() const {
    hash_util::HashCombiner hc;
    hc << flags_;
    hc.Combine(bindings_);
    hc.Combine(binding_flags_);
    return hc.Value();
}

// Return valid index or "end" i.e. binding_count_;
// The asserts in "Get" are reduced to the set where no valid answer(like null or 0) could be given
// Common code for all binding lookups.
uint32_t cvdescriptorset::DescriptorSetLayoutDef::GetIndexFromBinding(uint32_t binding) const {
    const auto &bi_itr = binding_to_index_map_.find(binding);
    if (bi_itr != binding_to_index_map_.cend()) return bi_itr->second;
    return GetBindingCount();
}
VkDescriptorSetLayoutBinding const *cvdescriptorset::DescriptorSetLayoutDef::GetDescriptorSetLayoutBindingPtrFromIndex(
    const uint32_t index) const {
    if (index >= bindings_.size()) return nullptr;
    return bindings_[index].ptr();
}
// Return descriptorCount for given index, 0 if index is unavailable
uint32_t cvdescriptorset::DescriptorSetLayoutDef::GetDescriptorCountFromIndex(const uint32_t index) const {
    if (index >= bindings_.size()) return 0;
    return bindings_[index].descriptorCount;
}
// For the given index, return descriptorType
VkDescriptorType cvdescriptorset::DescriptorSetLayoutDef::GetTypeFromIndex(const uint32_t index) const {
    assert(index < bindings_.size());
    if (index < bindings_.size()) return bindings_[index].descriptorType;
    return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}
// For the given index, return stageFlags
VkShaderStageFlags cvdescriptorset::DescriptorSetLayoutDef::GetStageFlagsFromIndex(const uint32_t index) const {
    assert(index < bindings_.size());
    if (index < bindings_.size()) return bindings_[index].stageFlags;
    return VkShaderStageFlags(0);
}
// Return binding flags for given index, 0 if index is unavailable
VkDescriptorBindingFlagsEXT cvdescriptorset::DescriptorSetLayoutDef::GetDescriptorBindingFlagsFromIndex(
    const uint32_t index) const {
    if (index >= binding_flags_.size()) return 0;
    return binding_flags_[index];
}

// For the given binding, return the global index range
// As start and end are often needed in pairs, get both with a single hash lookup.
const cvdescriptorset::IndexRange &cvdescriptorset::DescriptorSetLayoutDef::GetGlobalIndexRangeFromBinding(
    const uint32_t binding) const {
    assert(binding_to_global_index_range_map_.count(binding));
    // In error case max uint32_t so index is out of bounds to break ASAP
    const static IndexRange kInvalidRange = {0xFFFFFFFF, 0xFFFFFFFF};
    const auto &range_it = binding_to_global_index_range_map_.find(binding);
    if (range_it != binding_to_global_index_range_map_.end()) {
        return range_it->second;
    }
    return kInvalidRange;
}

// Move to next valid binding having a non-zero binding count
uint32_t cvdescriptorset::DescriptorSetLayoutDef::GetNextValidBinding(const uint32_t binding) const {
    auto it = non_empty_bindings_.upper_bound(binding);
    assert(it != non_empty_bindings_.cend());
    if (it != non_empty_bindings_.cend()) return *it;
    return GetMaxBinding() + 1;
}
// For given index, return ptr to ImmutableSampler array
VkSampler const *cvdescriptorset::DescriptorSetLayoutDef::GetImmutableSamplerPtrFromIndex(const uint32_t index) const {
    if (index < bindings_.size()) {
        return bindings_[index].pImmutableSamplers;
    }
    return nullptr;
}

// The DescriptorSetLayout stores the per handle data for a descriptor set layout, and references the common defintion for the
// handle invariant portion
cvdescriptorset::DescriptorSetLayout::DescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo *p_create_info,
                                                          const VkDescriptorSetLayout layout)
    : layout_(layout), layout_destroyed_(false), layout_id_(GetCanonicalId(p_create_info)) {}

cvdescriptorset::AllocateDescriptorSetsData::AllocateDescriptorSetsData(uint32_t count)
    : required_descriptors_by_type{}, layout_nodes(count, nullptr) {}

cvdescriptorset::DescriptorSet::DescriptorSet(const VkDescriptorSet set, const VkDescriptorPool pool,
                                              const std::shared_ptr<DescriptorSetLayout const> &layout, uint32_t variable_count,
                                              CoreChecks *dev_data)
    : set_(set),
      pool_state_(nullptr),
      p_layout_(layout),
      device_data_(dev_data),
      limits_(dev_data->phys_dev_props.limits),
      variable_count_(variable_count) {
    pool_state_ = dev_data->GetDescriptorPoolState(pool);
    // Foreach binding, create default descriptors of given type
    descriptors_.reserve(p_layout_->GetTotalDescriptorCount());
    for (uint32_t i = 0; i < p_layout_->GetBindingCount(); ++i) {
        for (uint32_t di = 0; di < p_layout_->GetDescriptorCountFromIndex(i); ++di) descriptors_.emplace_back(new Descriptor);
    }
}

cvdescriptorset::DescriptorSet::~DescriptorSet() {}

// Bind cb_node to this set and this set to cb_node.
// Prereq: This should be called for a set that has been confirmed to be active for the given cb_node, meaning it's going
//   to be used in a draw by the given cb_node
void cvdescriptorset::DescriptorSet::BindCommandBuffer(CMD_BUFFER_STATE *cb_node,
                                                       const std::map<uint32_t, descriptor_req> &binding_req_map) {
    // bind cb to this descriptor set
    cb_bindings.insert(cb_node);
    // For the active slots, use set# to look up descriptorSet from boundDescriptorSets, and bind all of that descriptor set's
    // resources
    for (auto binding_req_pair : binding_req_map) {
        auto binding = binding_req_pair.first;
        auto range = p_layout_->GetGlobalIndexRangeFromBinding(binding);
        for (uint32_t i = range.start; i < range.end; ++i) {
            descriptors_[i]->BindCommandBuffer(cb_node);
        }
    }
}

// Update the drawing state for the affected descriptors.
// Set cb_node to this set and this set to cb_node.
// Add the bindings of the descriptor
// Set the layout based on the current descriptor layout (will mask subsequent layer mismatch errors)
void cvdescriptorset::DescriptorSet::UpdateDrawState(CoreChecks *device_data, CMD_BUFFER_STATE *cb_node,
                                                     const std::map<uint32_t, descriptor_req> &binding_req_map) {
    // bind cb to this descriptor set
    cb_bindings.insert(cb_node);
    // For the active slots, use set# to look up descriptorSet from boundDescriptorSets, and bind all of that descriptor set's
    // resources
    for (auto binding_req_pair : binding_req_map) {
        auto binding = binding_req_pair.first;
        // We aren't validating descriptors created with PARTIALLY_BOUND or UPDATE_AFTER_BIND, so don't record state
        if (p_layout_->GetDescriptorBindingFlagsFromBinding(binding) &
            (VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT)) {
            continue;
        }
        auto range = p_layout_->GetGlobalIndexRangeFromBinding(binding);
        for (uint32_t i = range.start; i < range.end; ++i) {
            descriptors_[i]->UpdateDrawState(device_data, cb_node);
        }
    }
}

void cvdescriptorset::Descriptor::BindCommandBuffer(CMD_BUFFER_STATE *cb_node) {}

void cvdescriptorset::Descriptor::UpdateDrawState(CoreChecks *dev_data, CMD_BUFFER_STATE *cb_node) {}

cvdescriptorset::Descriptor::Descriptor() { updated = false; }

// Update the common AllocateDescriptorSetsData
void CoreChecks::UpdateAllocateDescriptorSetsData(const VkDescriptorSetAllocateInfo *p_alloc_info,
                                                  cvdescriptorset::AllocateDescriptorSetsData *ds_data) {
    for (uint32_t i = 0; i < p_alloc_info->descriptorSetCount; i++) {
        auto layout = GetDescriptorSetLayout(this, p_alloc_info->pSetLayouts[i]);
        if (layout) {
            ds_data->layout_nodes[i] = layout;
        }
    }
}

// Decrement allocated sets from the pool and insert new sets into set_map
void CoreChecks::PerformAllocateDescriptorSets(const VkDescriptorSetAllocateInfo *p_alloc_info,
                                               const VkDescriptorSet *descriptor_sets,
                                               const cvdescriptorset::AllocateDescriptorSetsData *ds_data) {
    const auto *variable_count_info = lvl_find_in_chain<VkDescriptorSetVariableDescriptorCountAllocateInfoEXT>(p_alloc_info->pNext);
    bool variable_count_valid = variable_count_info && variable_count_info->descriptorSetCount == p_alloc_info->descriptorSetCount;

    // Create tracking object for each descriptor set; insert into global map and the pool's set.
    for (uint32_t i = 0; i < p_alloc_info->descriptorSetCount; i++) {
        uint32_t variable_count = variable_count_valid ? variable_count_info->pDescriptorCounts[i] : 0;

        std::unique_ptr<cvdescriptorset::DescriptorSet> new_ds(new cvdescriptorset::DescriptorSet(
            descriptor_sets[i], p_alloc_info->descriptorPool, ds_data->layout_nodes[i], variable_count, this));
        setMap[descriptor_sets[i]] = std::move(new_ds);
    }
}
