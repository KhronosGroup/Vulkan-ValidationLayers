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
 *         Jeremy Kniager <jeremyk@lunarg.com>
 */

#include "chassis.h"
#include "core_validation_error_enums.h"
#include "core_validation.h"
#include "descriptor_sets.h"
#include "hash_vk_types.h"
#include "vk_enum_string_helper.h"
#include "vk_safe_struct.h"
#include "vk_typemap_helper.h"
#include "buffer_validation.h"
#include <sstream>
#include <algorithm>
#include <array>
#include <memory>

static DESCRIPTOR_POOL_STATE::TypeCountMap GetMaxTypeCounts(const VkDescriptorPoolCreateInfo *create_info) {
    DESCRIPTOR_POOL_STATE::TypeCountMap counts;
    // Collect maximums per descriptor type.
    for (uint32_t i = 0; i < create_info->poolSizeCount; ++i) {
        const auto &pool_size = create_info->pPoolSizes[i];
        uint32_t type = static_cast<uint32_t>(pool_size.type);
        // Same descriptor types can appear several times
        counts[type] += pool_size.descriptorCount;
    }
    return counts;
}

DESCRIPTOR_POOL_STATE::DESCRIPTOR_POOL_STATE(ValidationStateTracker *dev, const VkDescriptorPool pool,
                                             const VkDescriptorPoolCreateInfo *pCreateInfo)
    : BASE_NODE(pool, kVulkanObjectTypeDescriptorPool),
      maxSets(pCreateInfo->maxSets),
      createInfo(pCreateInfo),
      maxDescriptorTypeCount(GetMaxTypeCounts(pCreateInfo)),
      available_sets_(pCreateInfo->maxSets),
      available_counts_(maxDescriptorTypeCount),
      dev_data_(dev) {}

void DESCRIPTOR_POOL_STATE::Allocate(const VkDescriptorSetAllocateInfo *alloc_info, const VkDescriptorSet *descriptor_sets,
                                     const cvdescriptorset::AllocateDescriptorSetsData *ds_data) {
    auto guard = WriteLock();
    // Account for sets and individual descriptors allocated from pool
    available_sets_ -= alloc_info->descriptorSetCount;
    for (auto it = ds_data->required_descriptors_by_type.begin(); it != ds_data->required_descriptors_by_type.end(); ++it) {
        available_counts_[it->first] -= ds_data->required_descriptors_by_type.at(it->first);
    }

    const auto *variable_count_info = LvlFindInChain<VkDescriptorSetVariableDescriptorCountAllocateInfo>(alloc_info->pNext);
    bool variable_count_valid = variable_count_info && variable_count_info->descriptorSetCount == alloc_info->descriptorSetCount;

    // Create tracking object for each descriptor set; insert into global map and the pool's set.
    for (uint32_t i = 0; i < alloc_info->descriptorSetCount; i++) {
        uint32_t variable_count = variable_count_valid ? variable_count_info->pDescriptorCounts[i] : 0;

        auto new_ds = std::make_shared<cvdescriptorset::DescriptorSet>(descriptor_sets[i], this, ds_data->layout_nodes[i],
                                                                       variable_count, dev_data_);
        sets_.emplace(descriptor_sets[i], new_ds.get());
        dev_data_->Add(std::move(new_ds));
    }
}

void DESCRIPTOR_POOL_STATE::Free(uint32_t count, const VkDescriptorSet *descriptor_sets) {
    auto guard = WriteLock();
    // Update available descriptor sets in pool
    available_sets_ += count;

    // For each freed descriptor add its resources back into the pool as available and remove from pool and device data
    for (uint32_t i = 0; i < count; ++i) {
        if (descriptor_sets[i] != VK_NULL_HANDLE) {
            auto iter = sets_.find(descriptor_sets[i]);
            assert(iter != sets_.end());
            auto *set_state = iter->second;
            uint32_t type_index = 0, descriptor_count = 0;
            for (uint32_t j = 0; j < set_state->GetBindingCount(); ++j) {
                type_index = static_cast<uint32_t>(set_state->GetTypeFromIndex(j));
                descriptor_count = set_state->GetDescriptorCountFromIndex(j);
                available_counts_[type_index] += descriptor_count;
            }
            dev_data_->Destroy<cvdescriptorset::DescriptorSet>(iter->first);
            sets_.erase(iter);
        }
    }
}

void DESCRIPTOR_POOL_STATE::Reset() {
    auto guard = WriteLock();
    // For every set off of this pool, clear it, remove from setMap, and free cvdescriptorset::DescriptorSet
    for (auto entry : sets_) {
        dev_data_->Destroy<cvdescriptorset::DescriptorSet>(entry.first);
    }
    sets_.clear();
    // Reset available count for each type and available sets for this pool
    available_counts_ = maxDescriptorTypeCount;
    available_sets_ = maxSets;
}

bool DESCRIPTOR_POOL_STATE::InUse() const {
    auto guard = ReadLock();
    for (const auto &entry : sets_) {
        const auto *ds = entry.second;
        if (ds && ds->InUse()) {
            return true;
        }
    }
    return false;
}

void DESCRIPTOR_POOL_STATE::Destroy() {
    Reset();
    BASE_NODE::Destroy();
}

// ExtendedBinding collects a VkDescriptorSetLayoutBinding and any extended
// state that comes from a different array/structure so they can stay together
// while being sorted by binding number.
struct ExtendedBinding {
    ExtendedBinding(const VkDescriptorSetLayoutBinding *l, VkDescriptorBindingFlags f) : layout_binding(l), binding_flags(f) {}

    const VkDescriptorSetLayoutBinding *layout_binding;
    VkDescriptorBindingFlags binding_flags;
};

struct BindingNumCmp {
    bool operator()(const ExtendedBinding &a, const ExtendedBinding &b) const {
        return a.layout_binding->binding < b.layout_binding->binding;
    }
};

using DescriptorSet = cvdescriptorset::DescriptorSet;
using DescriptorSetLayout = cvdescriptorset::DescriptorSetLayout;
using DescriptorSetLayoutDef = cvdescriptorset::DescriptorSetLayoutDef;
using DescriptorSetLayoutId = cvdescriptorset::DescriptorSetLayoutId;

// Canonical dictionary of DescriptorSetLayoutDef (without any handle/device specific information)
cvdescriptorset::DescriptorSetLayoutDict descriptor_set_layout_dict;

DescriptorSetLayoutId GetCanonicalId(const VkDescriptorSetLayoutCreateInfo *p_create_info) {
    return descriptor_set_layout_dict.look_up(DescriptorSetLayoutDef(p_create_info));
}

// Construct DescriptorSetLayout instance from given create info
// Proactively reserve and resize as possible, as the reallocation was visible in profiling
cvdescriptorset::DescriptorSetLayoutDef::DescriptorSetLayoutDef(const VkDescriptorSetLayoutCreateInfo *p_create_info)
    : flags_(p_create_info->flags), binding_count_(0), descriptor_count_(0), dynamic_descriptor_count_(0) {
    const auto *flags_create_info = LvlFindInChain<VkDescriptorSetLayoutBindingFlagsCreateInfo>(p_create_info->pNext);

    binding_type_stats_ = {0, 0};
    std::set<ExtendedBinding, BindingNumCmp> sorted_bindings;
    const uint32_t input_bindings_count = p_create_info->bindingCount;
    // Sort the input bindings in binding number order, eliminating duplicates
    for (uint32_t i = 0; i < input_bindings_count; i++) {
        VkDescriptorBindingFlags flags = 0;
        if (flags_create_info && flags_create_info->bindingCount == p_create_info->bindingCount) {
            flags = flags_create_info->pBindingFlags[i];
        }
        sorted_bindings.emplace(p_create_info->pBindings + i, flags);
    }

    const auto *mutable_descriptor_type_create_info = LvlFindInChain<VkMutableDescriptorTypeCreateInfoVALVE>(p_create_info->pNext);
    if (mutable_descriptor_type_create_info) {
        mutable_types_.resize(mutable_descriptor_type_create_info->mutableDescriptorTypeListCount);
        for (uint32_t i = 0; i < mutable_descriptor_type_create_info->mutableDescriptorTypeListCount; ++i) {
            const auto &list = mutable_descriptor_type_create_info->pMutableDescriptorTypeLists[i];
            mutable_types_[i].reserve(list.descriptorTypeCount);
            for (uint32_t j = 0; j < list.descriptorTypeCount; ++j) {
                mutable_types_[i].push_back(list.pDescriptorTypes[j]);
            }
            std::sort(mutable_types_[i].begin(), mutable_types_[i].end());
        }
    }

    // Store the create info in the sorted order from above
    uint32_t index = 0;
    binding_count_ = static_cast<uint32_t>(sorted_bindings.size());
    bindings_.reserve(binding_count_);
    binding_flags_.reserve(binding_count_);
    binding_to_index_map_.reserve(binding_count_);
    for (const auto &input_binding : sorted_bindings) {
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

        if (IsDynamicDescriptor(binding_info.descriptorType)) {
            dynamic_descriptor_count_ += binding_info.descriptorCount;
        }

        // Get stats depending on descriptor type for caching later
        if (IsBufferDescriptor(binding_info.descriptorType)) {
            if (IsDynamicDescriptor(binding_info.descriptorType)) {
                binding_type_stats_.dynamic_buffer_count++;
            } else {
                binding_type_stats_.non_dynamic_buffer_count++;
            }
        }
    }
    assert(bindings_.size() == binding_count_);
    assert(binding_flags_.size() == binding_count_);
    uint32_t global_index = 0;
    global_index_range_.reserve(binding_count_);
    // Vector order is finalized so build vectors of descriptors and dynamic offsets by binding index
    for (uint32_t i = 0; i < binding_count_; ++i) {
        auto final_index = global_index + bindings_[i].descriptorCount;
        global_index_range_.emplace_back(global_index, final_index);
        global_index = final_index;
    }
}

size_t cvdescriptorset::DescriptorSetLayoutDef::hash() const {
    hash_util::HashCombiner hc;
    hc << flags_;
    hc.Combine(bindings_);
    hc.Combine(binding_flags_);
    return hc.Value();
}
//

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
VkDescriptorBindingFlags cvdescriptorset::DescriptorSetLayoutDef::GetDescriptorBindingFlagsFromIndex(const uint32_t index) const {
    if (index >= binding_flags_.size()) return 0;
    return binding_flags_[index];
}

const cvdescriptorset::IndexRange &cvdescriptorset::DescriptorSetLayoutDef::GetGlobalIndexRangeFromIndex(uint32_t index) const {
    const static IndexRange k_invalid_range = {0xFFFFFFFF, 0xFFFFFFFF};
    if (index >= binding_flags_.size()) return k_invalid_range;
    return global_index_range_[index];
}

// For the given binding, return the global index range (half open)
// As start and end are often needed in pairs, get both with a single lookup.
const cvdescriptorset::IndexRange &cvdescriptorset::DescriptorSetLayoutDef::GetGlobalIndexRangeFromBinding(
    const uint32_t binding) const {
    uint32_t index = GetIndexFromBinding(binding);
    return GetGlobalIndexRangeFromIndex(index);
}

// For given binding, return ptr to ImmutableSampler array
VkSampler const *cvdescriptorset::DescriptorSetLayoutDef::GetImmutableSamplerPtrFromBinding(const uint32_t binding) const {
    const auto &bi_itr = binding_to_index_map_.find(binding);
    if (bi_itr != binding_to_index_map_.end()) {
        return bindings_[bi_itr->second].pImmutableSamplers;
    }
    return nullptr;
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

bool cvdescriptorset::DescriptorSetLayoutDef::IsTypeMutable(const VkDescriptorType type, uint32_t binding) const {
    if (binding < mutable_types_.size()) {
        if (mutable_types_[binding].size() > 0) {
            for (const auto mutable_type : mutable_types_[binding]) {
                if (type == mutable_type) {
                    return true;
                }
            }
            return false;
        }
    }
    // If mutableDescriptorTypeListCount is zero or if VkMutableDescriptorTypeCreateInfoVALVE structure is not included in the pNext
    // chain, the VkMutableDescriptorTypeListVALVE for each element is considered to be zero or NULL for each member.
    return false;
}

const std::vector<std::vector<VkDescriptorType>>& cvdescriptorset::DescriptorSetLayoutDef::GetMutableTypes() const {
    return mutable_types_;
}

const std::vector<VkDescriptorType> &cvdescriptorset::DescriptorSetLayoutDef::GetMutableTypes(uint32_t binding) const {
    if (binding >= mutable_types_.size()) {
        static const std::vector<VkDescriptorType> empty = {};
        return empty;
    }
    return mutable_types_[binding];
}

// If our layout is compatible with rh_ds_layout, return true.
bool cvdescriptorset::DescriptorSetLayout::IsCompatible(DescriptorSetLayout const *rh_ds_layout) const {
    bool compatible = (this == rh_ds_layout) || (GetLayoutDef() == rh_ds_layout->GetLayoutDef());
    return compatible;
}

// TODO: Find a way to add smarts to the autogenerated version of this
static std::string smart_string_VkShaderStageFlags(VkShaderStageFlags stage_flags) {
    if (stage_flags == VK_SHADER_STAGE_ALL) {
        return string_VkShaderStageFlagBits(VK_SHADER_STAGE_ALL);
    }

    return string_VkShaderStageFlags(stage_flags);
}

// If our layout is compatible with bound_dsl, return true,
//  else return false and fill in error_msg will description of what causes incompatibility
bool cvdescriptorset::VerifySetLayoutCompatibility(const debug_report_data *report_data, DescriptorSetLayout const *layout_dsl,
                                                   DescriptorSetLayout const *bound_dsl, std::string *error_msg) {
    // Short circuit the detailed check.
    if (layout_dsl->IsCompatible(bound_dsl)) return true;

    // Do a detailed compatibility check of this lhs def (referenced by layout_dsl), vs. the rhs (layout and def)
    // Should only be run if trivial accept has failed, and in that context should return false.
    VkDescriptorSetLayout layout_dsl_handle = layout_dsl->GetDescriptorSetLayout();
    VkDescriptorSetLayout bound_dsl_handle = bound_dsl->GetDescriptorSetLayout();
    DescriptorSetLayoutDef const *layout_ds_layout_def = layout_dsl->GetLayoutDef();
    DescriptorSetLayoutDef const *bound_ds_layout_def = bound_dsl->GetLayoutDef();

    // Check descriptor counts
    const auto bound_total_count = bound_ds_layout_def->GetTotalDescriptorCount();
    if (layout_ds_layout_def->GetTotalDescriptorCount() != bound_ds_layout_def->GetTotalDescriptorCount()) {
        std::stringstream error_str;
        error_str << report_data->FormatHandle(layout_dsl_handle) << " from pipeline layout has "
                  << layout_ds_layout_def->GetTotalDescriptorCount() << " total descriptors, but "
                  << report_data->FormatHandle(bound_dsl_handle) << ", which is bound, has " << bound_total_count
                  << " total descriptors.";
        *error_msg = error_str.str();
        return false;  // trivial fail case
    }

    // Descriptor counts match so need to go through bindings one-by-one
    //  and verify that type and stageFlags match
    for (const auto &layout_binding : layout_ds_layout_def->GetBindings()) {
        // TODO : Do we also need to check immutable samplers?
        const auto bound_binding = bound_ds_layout_def->GetBindingInfoFromBinding(layout_binding.binding);
        if (layout_binding.descriptorCount != bound_binding->descriptorCount) {
            std::stringstream error_str;
            error_str << "Binding " << layout_binding.binding << " for " << report_data->FormatHandle(layout_dsl_handle)
                      << " from pipeline layout has a descriptorCount of " << layout_binding.descriptorCount << " but binding "
                      << layout_binding.binding << " for " << report_data->FormatHandle(bound_dsl_handle)
                      << ", which is bound, has a descriptorCount of " << bound_binding->descriptorCount;
            *error_msg = error_str.str();
            return false;
        } else if (layout_binding.descriptorType != bound_binding->descriptorType) {
            std::stringstream error_str;
            error_str << "Binding " << layout_binding.binding << " for " << report_data->FormatHandle(layout_dsl_handle)
                      << " from pipeline layout is type '" << string_VkDescriptorType(layout_binding.descriptorType)
                      << "' but binding " << layout_binding.binding << " for " << report_data->FormatHandle(bound_dsl_handle)
                      << ", which is bound, is type '" << string_VkDescriptorType(bound_binding->descriptorType) << "'";
            *error_msg = error_str.str();
            return false;
        } else if (layout_binding.stageFlags != bound_binding->stageFlags) {
            std::stringstream error_str;
            error_str << "Binding " << layout_binding.binding << " for " << report_data->FormatHandle(layout_dsl_handle)
                      << " from pipeline layout has stageFlags " << smart_string_VkShaderStageFlags(layout_binding.stageFlags)
                      << " but binding " << layout_binding.binding << " for " << report_data->FormatHandle(bound_dsl_handle)
                      << ", which is bound, has stageFlags " << smart_string_VkShaderStageFlags(bound_binding->stageFlags);
            *error_msg = error_str.str();
            return false;
        }
    }

    const auto &ds_layout_flags = layout_ds_layout_def->GetBindingFlags();
    const auto &bound_layout_flags = bound_ds_layout_def->GetBindingFlags();
    if (bound_layout_flags != ds_layout_flags) {
        std::stringstream error_str;
        assert(ds_layout_flags.size() == bound_layout_flags.size());
        size_t i;
        for (i = 0; i < ds_layout_flags.size(); i++) {
            if (ds_layout_flags[i] != bound_layout_flags[i]) break;
        }
        error_str << report_data->FormatHandle(layout_dsl_handle)
                  << " from pipeline layout does not have the same binding flags at binding " << i << " ( "
                  << string_VkDescriptorBindingFlagsEXT(ds_layout_flags[i]) << " ) as "
                  << report_data->FormatHandle(bound_dsl_handle) << " ( "
                  << string_VkDescriptorBindingFlagsEXT(bound_layout_flags[i]) << " ), which is bound";
        *error_msg = error_str.str();
        return false;
    }

    // No detailed check should succeed if the trivial check failed -- or the dictionary has failed somehow.
    bool compatible = true;
    assert(!compatible);
    return compatible;
}

bool cvdescriptorset::DescriptorSetLayoutDef::IsNextBindingConsistent(const uint32_t binding) const {
    if (!binding_to_index_map_.count(binding + 1)) return false;
    auto const &bi_itr = binding_to_index_map_.find(binding);
    if (bi_itr != binding_to_index_map_.end()) {
        const auto &next_bi_itr = binding_to_index_map_.find(binding + 1);
        if (next_bi_itr != binding_to_index_map_.end()) {
            auto type = bindings_[bi_itr->second].descriptorType;
            auto stage_flags = bindings_[bi_itr->second].stageFlags;
            auto immut_samp = bindings_[bi_itr->second].pImmutableSamplers ? true : false;
            auto flags = binding_flags_[bi_itr->second];
            if ((type != bindings_[next_bi_itr->second].descriptorType) ||
                (stage_flags != bindings_[next_bi_itr->second].stageFlags) ||
                (immut_samp != (bindings_[next_bi_itr->second].pImmutableSamplers ? true : false)) ||
                (flags != binding_flags_[next_bi_itr->second])) {
                return false;
            }
            return true;
        }
    }
    return false;
}

// The DescriptorSetLayout stores the per handle data for a descriptor set layout, and references the common defintion for the
// handle invariant portion
cvdescriptorset::DescriptorSetLayout::DescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo *p_create_info,
                                                          const VkDescriptorSetLayout layout)
    : BASE_NODE(layout, kVulkanObjectTypeDescriptorSetLayout), layout_id_(GetCanonicalId(p_create_info)) {}

// Validate descriptor set layout create info
bool cvdescriptorset::ValidateDescriptorSetLayoutCreateInfo(
    const ValidationObject *val_obj, const VkDescriptorSetLayoutCreateInfo *create_info, const bool push_descriptor_ext,
    const uint32_t max_push_descriptors, const bool descriptor_indexing_ext,
    const VkPhysicalDeviceVulkan12Features *core12_features,
    const VkPhysicalDeviceVulkan13Features* core13_features,
    const VkPhysicalDeviceInlineUniformBlockPropertiesEXT *inline_uniform_block_props,
    const VkPhysicalDeviceAccelerationStructureFeaturesKHR *acceleration_structure_features,
    const DeviceExtensions *device_extensions) {
    bool skip = false;
    layer_data::unordered_set<uint32_t> bindings;
    uint64_t total_descriptors = 0;

    const auto *flags_create_info = LvlFindInChain<VkDescriptorSetLayoutBindingFlagsCreateInfo>(create_info->pNext);

    const bool push_descriptor_set = !!(create_info->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);
    if (push_descriptor_set && !push_descriptor_ext) {
        skip |= val_obj->LogError(
            val_obj->device, kVUID_Core_DrawState_ExtensionNotEnabled,
            "vkCreateDescriptorSetLayout(): Attempted to use %s in %s but its required extension %s has not been enabled.\n",
            "VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR", "VkDescriptorSetLayoutCreateInfo::flags",
            VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    }

    const bool update_after_bind_set = !!(create_info->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT);
    if (update_after_bind_set && !descriptor_indexing_ext) {
        skip |= val_obj->LogError(
            val_obj->device, kVUID_Core_DrawState_ExtensionNotEnabled,
            "vkCreateDescriptorSetLayout(): Attemped to use %s in %s but its required extension %s has not been enabled.\n",
            "VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT", "VkDescriptorSetLayoutCreateInfo::flags",
            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    }

    auto valid_type = [push_descriptor_set](const VkDescriptorType type) {
        return !push_descriptor_set ||
               ((type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) && (type != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) &&
                (type != VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT));
    };

    uint32_t max_binding = 0;

    uint32_t update_after_bind = create_info->bindingCount;
    uint32_t uniform_buffer_dynamic = create_info->bindingCount;
    uint32_t storage_buffer_dynamic = create_info->bindingCount;

    for (uint32_t i = 0; i < create_info->bindingCount; ++i) {
        const auto &binding_info = create_info->pBindings[i];
        max_binding = std::max(max_binding, binding_info.binding);

        if (!bindings.insert(binding_info.binding).second) {
            skip |= val_obj->LogError(val_obj->device, "VUID-VkDescriptorSetLayoutCreateInfo-binding-00279",
                                      "vkCreateDescriptorSetLayout(): pBindings[%u] has duplicated binding number (%u).", i,
                                      binding_info.binding);
        }
        if (!valid_type(binding_info.descriptorType)) {
            skip |= val_obj->LogError(val_obj->device,
                                      (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT)
                                          ? "VUID-VkDescriptorSetLayoutCreateInfo-flags-02208"
                                          : "VUID-VkDescriptorSetLayoutCreateInfo-flags-00280",
                                      "vkCreateDescriptorSetLayout(): pBindings[%u] has invalid type %s , for push descriptors.", i,
                                      string_VkDescriptorType(binding_info.descriptorType));
        }

        if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT) {
            if (!core13_features->inlineUniformBlock) {
                skip |= val_obj->LogError(val_obj->device, "VUID-VkDescriptorSetLayoutBinding-descriptorType-04604",
                                          "vkCreateDescriptorSetLayout(): pBindings[%u] is creating VkDescriptorSetLayout with "
                                          "descriptor type VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT "
                                          "but the inlineUniformBlock feature is not enabled",
                                          i);
            } else {
                if ((binding_info.descriptorCount % 4) != 0) {
                    skip |= val_obj->LogError(val_obj->device, "VUID-VkDescriptorSetLayoutBinding-descriptorType-02209",
                                              "vkCreateDescriptorSetLayout(): pBindings[%u] has descriptorCount =(%" PRIu32
                                              ") but must be a multiple of 4",
                                              i, binding_info.descriptorCount);
                }
                if (binding_info.descriptorCount > inline_uniform_block_props->maxInlineUniformBlockSize) {
                    skip |=
                        val_obj->LogError(val_obj->device, "VUID-VkDescriptorSetLayoutBinding-descriptorType-02210",
                                          "vkCreateDescriptorSetLayout(): pBindings[%u] has descriptorCount =(%" PRIu32
                                          ") but must be less than or equal to maxInlineUniformBlockSize (%u)",
                                          i, binding_info.descriptorCount, inline_uniform_block_props->maxInlineUniformBlockSize);
                }
            }
        } else if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
            uniform_buffer_dynamic = i;
        } else if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) {
            storage_buffer_dynamic = i;
        }

        if ((binding_info.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER ||
             binding_info.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) &&
            binding_info.pImmutableSamplers && IsExtEnabled(device_extensions->vk_ext_custom_border_color)) {
            const CoreChecks *core_checks = reinterpret_cast<const CoreChecks *>(val_obj);
            for (uint32_t j = 0; j < binding_info.descriptorCount; j++) {
                auto sampler_state = core_checks->Get<SAMPLER_STATE>(binding_info.pImmutableSamplers[j]);
                if (sampler_state && (sampler_state->createInfo.borderColor == VK_BORDER_COLOR_INT_CUSTOM_EXT ||
                                      sampler_state->createInfo.borderColor == VK_BORDER_COLOR_FLOAT_CUSTOM_EXT)) {
                    skip |= val_obj->LogError(
                        val_obj->device, "VUID-VkDescriptorSetLayoutBinding-pImmutableSamplers-04009",
                        "vkCreateDescriptorSetLayout(): pBindings[%u].pImmutableSamplers[%u] has VkSampler %s"
                        " presented as immutable has a custom border color",
                        i, j, val_obj->report_data->FormatHandle(binding_info.pImmutableSamplers[j]).c_str());
                }
            }
        }

        if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_MUTABLE_VALVE && binding_info.pImmutableSamplers != nullptr) {
            skip |= val_obj->LogError(val_obj->device, "VUID-VkDescriptorSetLayoutBinding-descriptorType-04605",
                                      "vkCreateDescriptorSetLayout(): pBindings[%u] has descriptorType "
                                      "VK_DESCRIPTOR_TYPE_MUTABLE_VALVE but pImmutableSamplers is not NULL.",
                                      i);
        }

        total_descriptors += binding_info.descriptorCount;
    }

    if (flags_create_info) {
        if (flags_create_info->bindingCount != 0 && flags_create_info->bindingCount != create_info->bindingCount) {
            skip |= val_obj->LogError(val_obj->device, "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-bindingCount-03002",
                                      "vkCreateDescriptorSetLayout(): VkDescriptorSetLayoutCreateInfo::bindingCount (%d) != "
                                      "VkDescriptorSetLayoutBindingFlagsCreateInfo::bindingCount (%d)",
                                      create_info->bindingCount, flags_create_info->bindingCount);
        }

        if (flags_create_info->bindingCount == create_info->bindingCount) {
            for (uint32_t i = 0; i < create_info->bindingCount; ++i) {
                const auto &binding_info = create_info->pBindings[i];

                if (flags_create_info->pBindingFlags[i] & VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT) {
                    update_after_bind = i;
                    if (!update_after_bind_set) {
                        skip |= val_obj->LogError(val_obj->device, "VUID-VkDescriptorSetLayoutCreateInfo-flags-03000",
                                                  "vkCreateDescriptorSetLayout(): pBindings[%u] does not have "
                                                  "VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT.",
                                                  i);
                    }

                    if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER &&
                        !core12_features->descriptorBindingUniformBufferUpdateAfterBind) {
                        skip |= val_obj->LogError(
                            val_obj->device,
                            "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-"
                            "descriptorBindingUniformBufferUpdateAfterBind-03005",
                            "vkCreateDescriptorSetLayout(): pBindings[%u] can't have VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT "
                            "for %s since descriptorBindingUniformBufferUpdateAfterBind is not enabled.",
                            i, string_VkDescriptorType(binding_info.descriptorType));
                    }
                    if ((binding_info.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER ||
                         binding_info.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
                         binding_info.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) &&
                        !core12_features->descriptorBindingSampledImageUpdateAfterBind) {
                        skip |= val_obj->LogError(
                            val_obj->device,
                            "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-"
                            "descriptorBindingSampledImageUpdateAfterBind-03006",
                            "vkCreateDescriptorSetLayout(): pBindings[%u] can't have VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT "
                            "for %s since descriptorBindingSampledImageUpdateAfterBind is not enabled.",
                            i, string_VkDescriptorType(binding_info.descriptorType));
                    }
                    if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE &&
                        !core12_features->descriptorBindingStorageImageUpdateAfterBind) {
                        skip |= val_obj->LogError(
                            val_obj->device,
                            "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-"
                            "descriptorBindingStorageImageUpdateAfterBind-03007",
                            "vkCreateDescriptorSetLayout(): pBindings[%u] can't have VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT "
                            "for %s since descriptorBindingStorageImageUpdateAfterBind is not enabled.",
                            i, string_VkDescriptorType(binding_info.descriptorType));
                    }
                    if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER &&
                        !core12_features->descriptorBindingStorageBufferUpdateAfterBind) {
                        skip |= val_obj->LogError(
                            val_obj->device,
                            "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-"
                            "descriptorBindingStorageBufferUpdateAfterBind-03008",
                            "vkCreateDescriptorSetLayout(): pBindings[%u] can't have VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT "
                            "for %s since descriptorBindingStorageBufferUpdateAfterBind is not enabled.",
                            i, string_VkDescriptorType(binding_info.descriptorType));
                    }
                    if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER &&
                        !core12_features->descriptorBindingUniformTexelBufferUpdateAfterBind) {
                        skip |= val_obj->LogError(
                            val_obj->device,
                            "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-"
                            "descriptorBindingUniformTexelBufferUpdateAfterBind-03009",
                            "vkCreateDescriptorSetLayout(): pBindings[%u] can't have VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT "
                            "for %s since descriptorBindingUniformTexelBufferUpdateAfterBind is not enabled.",
                            i, string_VkDescriptorType(binding_info.descriptorType));
                    }
                    if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER &&
                        !core12_features->descriptorBindingStorageTexelBufferUpdateAfterBind) {
                        skip |= val_obj->LogError(
                            val_obj->device,
                            "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-"
                            "descriptorBindingStorageTexelBufferUpdateAfterBind-03010",
                            "vkCreateDescriptorSetLayout(): pBindings[%u] can't have VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT "
                            "for %s since descriptorBindingStorageTexelBufferUpdateAfterBind is not enabled.",
                            i, string_VkDescriptorType(binding_info.descriptorType));
                    }
                    if ((binding_info.descriptorType == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT ||
                         binding_info.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
                         binding_info.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)) {
                        skip |= val_obj->LogError(val_obj->device, "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-None-03011",
                                                  "vkCreateDescriptorSetLayout(): pBindings[%u] can't have "
                                                  "VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT for %s.",
                                                  i, string_VkDescriptorType(binding_info.descriptorType));
                    }

                    if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT &&
                        !core13_features->descriptorBindingInlineUniformBlockUpdateAfterBind) {
                        skip |= val_obj->LogError(
                            val_obj->device,
                            "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-"
                            "descriptorBindingInlineUniformBlockUpdateAfterBind-02211",
                            "vkCreateDescriptorSetLayout(): pBindings[%u] can't have VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT "
                            "for %s since descriptorBindingInlineUniformBlockUpdateAfterBind is not enabled.",
                            i, string_VkDescriptorType(binding_info.descriptorType));
                    }
                    if ((binding_info.descriptorType == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR ||
                         binding_info.descriptorType == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV) &&
                        !acceleration_structure_features->descriptorBindingAccelerationStructureUpdateAfterBind) {
                        skip |= val_obj->LogError(val_obj->device,
                                                  "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-"
                                                  "descriptorBindingAccelerationStructureUpdateAfterBind-03570",
                                                  "vkCreateDescriptorSetLayout(): pBindings[%" PRIu32
                                                  "] can't have VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT "
                                                  "for %s if "
                                                  "VkPhysicalDeviceAccelerationStructureFeaturesKHR::"
                                                  "descriptorBindingAccelerationStructureUpdateAfterBind is not enabled.",
                                                  i, string_VkDescriptorType(binding_info.descriptorType));
                    }
                }

                if (flags_create_info->pBindingFlags[i] & VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT) {
                    if (!core12_features->descriptorBindingUpdateUnusedWhilePending) {
                        skip |= val_obj->LogError(
                            val_obj->device,
                            "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingUpdateUnusedWhilePending-03012",
                            "vkCreateDescriptorSetLayout(): pBindings[%u] can't have "
                            "VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT for %s since "
                            "descriptorBindingUpdateUnusedWhilePending is not enabled.",
                            i, string_VkDescriptorType(binding_info.descriptorType));
                    }
                }

                if (flags_create_info->pBindingFlags[i] & VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT) {
                    if (!core12_features->descriptorBindingPartiallyBound) {
                        skip |= val_obj->LogError(
                            val_obj->device,
                            "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingPartiallyBound-03013",
                            "vkCreateDescriptorSetLayout(): pBindings[%u] can't have VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT for "
                            "%s since descriptorBindingPartiallyBound is not enabled.",
                            i, string_VkDescriptorType(binding_info.descriptorType));
                    }
                }

                if (flags_create_info->pBindingFlags[i] & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT) {
                    if (binding_info.binding != max_binding) {
                        skip |= val_obj->LogError(
                            val_obj->device, "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-pBindingFlags-03004",
                            "vkCreateDescriptorSetLayout(): pBindings[%u] has VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT "
                            "but %u is the largest value of all the bindings.",
                            i, binding_info.binding);
                    }

                    if (!core12_features->descriptorBindingVariableDescriptorCount) {
                        skip |= val_obj->LogError(
                            val_obj->device,
                            "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingVariableDescriptorCount-03014",
                            "vkCreateDescriptorSetLayout(): pBindings[%u] can't have "
                            "VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT for %s since "
                            "descriptorBindingVariableDescriptorCount is not enabled.",
                            i, string_VkDescriptorType(binding_info.descriptorType));
                    }
                    if ((binding_info.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) ||
                        (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)) {
                        skip |= val_obj->LogError(val_obj->device,
                                                  "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-pBindingFlags-03015",
                                                  "vkCreateDescriptorSetLayout(): pBindings[%u] can't have "
                                                  "VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT for %s.",
                                                  i, string_VkDescriptorType(binding_info.descriptorType));
                    }
                }

                if (push_descriptor_set &&
                    (flags_create_info->pBindingFlags[i] &
                     (VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT |
                      VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT))) {
                    skip |= val_obj->LogError(
                        val_obj->device, "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-flags-03003",
                        "vkCreateDescriptorSetLayout(): pBindings[%u] can't have VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT, "
                        "VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT, or "
                        "VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT for with "
                        "VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR.",
                        i);
                }
            }
        }
    }

    if (update_after_bind < create_info->bindingCount) {
        if (uniform_buffer_dynamic < create_info->bindingCount) {
            skip |=
                val_obj->LogError(val_obj->device, "VUID-VkDescriptorSetLayoutCreateInfo-descriptorType-03001",
                                  "vkCreateDescriptorSetLayout(): binding (%" PRIi32
                                  ") has VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT "
                                  "flag, but binding (%" PRIi32 ") has descriptor type VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC.",
                                  update_after_bind, uniform_buffer_dynamic);
        }
        if (storage_buffer_dynamic < create_info->bindingCount) {
            skip |=
                val_obj->LogError(val_obj->device, "VUID-VkDescriptorSetLayoutCreateInfo-descriptorType-03001",
                                  "vkCreateDescriptorSetLayout(): binding (%" PRIi32
                                  ") has VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT "
                                  "flag, but binding (%" PRIi32 ") has descriptor type VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC.",
                                  update_after_bind, storage_buffer_dynamic);
        }
    }

    if ((push_descriptor_set) && (total_descriptors > max_push_descriptors)) {
        const char *undefined = push_descriptor_ext ? "" : " -- undefined";
        skip |= val_obj->LogError(
            val_obj->device, "VUID-VkDescriptorSetLayoutCreateInfo-flags-00281",
            "vkCreateDescriptorSetLayout(): for push descriptor, total descriptor count in layout (%" PRIu64
            ") must not be greater than VkPhysicalDevicePushDescriptorPropertiesKHR::maxPushDescriptors (%" PRIu32 "%s).",
            total_descriptors, max_push_descriptors, undefined);
    }

    return skip;
}

void cvdescriptorset::AllocateDescriptorSetsData::Init(uint32_t count) {
    layout_nodes.resize(count);
}

cvdescriptorset::DescriptorSet::DescriptorSet(const VkDescriptorSet set, DESCRIPTOR_POOL_STATE *pool_state,
                                              const std::shared_ptr<DescriptorSetLayout const> &layout, uint32_t variable_count,
                                              const cvdescriptorset::DescriptorSet::StateTracker *state_data)
    : BASE_NODE(set, kVulkanObjectTypeDescriptorSet),
      some_update_(false),
      pool_state_(pool_state),
      layout_(layout),
      state_data_(state_data),
      variable_count_(variable_count),
      change_count_(0) {
    // Foreach binding, create default descriptors of given type
    descriptors_.reserve(layout_->GetTotalDescriptorCount());
    descriptor_store_.resize(layout_->GetTotalDescriptorCount());
    auto free_descriptor = descriptor_store_.data();
    for (uint32_t i = 0; i < layout_->GetBindingCount(); ++i) {
        auto type = layout_->GetTypeFromIndex(i);
        switch (type) {
            case VK_DESCRIPTOR_TYPE_SAMPLER: {
                auto immut_sampler = layout_->GetImmutableSamplerPtrFromIndex(i);
                for (uint32_t di = 0; di < layout_->GetDescriptorCountFromIndex(i); ++di) {
                    if (immut_sampler) {
                        descriptors_.emplace_back(new ((free_descriptor++)->Sampler())
                                                      SamplerDescriptor(state_data, immut_sampler + di));
                        some_update_ = true;  // Immutable samplers are updated at creation
                    } else {
                        descriptors_.emplace_back(new ((free_descriptor++)->Sampler()) SamplerDescriptor(state_data, nullptr));
                    }
                }
                break;
            }
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
                auto immut = layout_->GetImmutableSamplerPtrFromIndex(i);
                for (uint32_t di = 0; di < layout_->GetDescriptorCountFromIndex(i); ++di) {
                    if (immut) {
                        descriptors_.emplace_back(new ((free_descriptor++)->ImageSampler())
                                                      ImageSamplerDescriptor(state_data, immut + di));
                        some_update_ = true;  // Immutable samplers are updated at creation
                    } else {
                        descriptors_.emplace_back(new ((free_descriptor++)->ImageSampler())
                                                      ImageSamplerDescriptor(state_data, nullptr));
                    }
                }
                break;
            }
            // ImageDescriptors
            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                for (uint32_t di = 0; di < layout_->GetDescriptorCountFromIndex(i); ++di) {
                    descriptors_.emplace_back(new ((free_descriptor++)->Image()) ImageDescriptor(type));
                }
                break;
            case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                for (uint32_t di = 0; di < layout_->GetDescriptorCountFromIndex(i); ++di) {
                    descriptors_.emplace_back(new ((free_descriptor++)->Texel()) TexelDescriptor(type));
                }
                break;
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                for (uint32_t di = 0; di < layout_->GetDescriptorCountFromIndex(i); ++di) {
                    descriptors_.emplace_back(new ((free_descriptor++)->Buffer()) BufferDescriptor(type));
                }
                break;
            case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT:
                for (uint32_t di = 0; di < layout_->GetDescriptorCountFromIndex(i); ++di) {
                    descriptors_.emplace_back(new ((free_descriptor++)->InlineUniform()) InlineUniformDescriptor(type));
                }
                break;
            case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
            case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
                for (uint32_t di = 0; di < layout_->GetDescriptorCountFromIndex(i); ++di) {
                    descriptors_.emplace_back(new ((free_descriptor++)->AccelerationStructure())
                                                  AccelerationStructureDescriptor(type));
                }
                break;
            case VK_DESCRIPTOR_TYPE_MUTABLE_VALVE:
                for (uint32_t di = 0; di < layout_->GetDescriptorCountFromIndex(i); ++di) {
                    descriptors_.emplace_back(new ((free_descriptor++)->Mutable()) MutableDescriptor());
                }
                break;
            default:
                if (IsDynamicDescriptor(type) && IsBufferDescriptor(type)) {
                    for (uint32_t di = 0; di < layout_->GetDescriptorCountFromIndex(i); ++di) {
                        dynamic_offset_idx_to_descriptor_list_.push_back(descriptors_.size());
                        descriptors_.emplace_back(new ((free_descriptor++)->Buffer()) BufferDescriptor(type));
                    }
                } else {
                    assert(0);  // Bad descriptor type specified
                }
                break;
        }
    }
}

void cvdescriptorset::DescriptorSet::LinkChildNodes() {
    // Connect child node(s), which cannot safely be done in the constructor.
    for (auto &desc : descriptors_) {
        desc->AddParent(this);
    }
}

void cvdescriptorset::DescriptorSet::Destroy() {
    for (auto &desc: descriptors_) {
        desc->RemoveParent(this);
    }
    BASE_NODE::Destroy();
}

static std::string StringDescriptorReqViewType(DescriptorReqFlags req) {
    std::string result("");
    for (unsigned i = 0; i <= VK_IMAGE_VIEW_TYPE_CUBE_ARRAY; i++) {
        if (req & (1 << i)) {
            if (result.size()) result += ", ";
            result += string_VkImageViewType(VkImageViewType(i));
        }
    }

    if (!result.size()) result = "(none)";

    return result;
}

static char const *StringDescriptorReqComponentType(DescriptorReqFlags req) {
    if (req & DESCRIPTOR_REQ_COMPONENT_TYPE_SINT) return "SINT";
    if (req & DESCRIPTOR_REQ_COMPONENT_TYPE_UINT) return "UINT";
    if (req & DESCRIPTOR_REQ_COMPONENT_TYPE_FLOAT) return "FLOAT";
    return "(none)";
}

unsigned DescriptorRequirementsBitsFromFormat(VkFormat fmt) {
    if (FormatIsSINT(fmt)) return DESCRIPTOR_REQ_COMPONENT_TYPE_SINT;
    if (FormatIsUINT(fmt)) return DESCRIPTOR_REQ_COMPONENT_TYPE_UINT;
    // Formats such as VK_FORMAT_D16_UNORM_S8_UINT are both
    if (FormatIsDepthAndStencil(fmt)) return DESCRIPTOR_REQ_COMPONENT_TYPE_FLOAT | DESCRIPTOR_REQ_COMPONENT_TYPE_UINT;
    if (fmt == VK_FORMAT_UNDEFINED) return 0;
    // everything else -- UNORM/SNORM/FLOAT/USCALED/SSCALED is all float in the shader.
    return DESCRIPTOR_REQ_COMPONENT_TYPE_FLOAT;
}

// Validate that the state of this set is appropriate for the given bindings and dynamic_offsets at Draw time
//  This includes validating that all descriptors in the given bindings are updated,
//  that any update buffers are valid, and that any dynamic offsets are within the bounds of their buffers.
// Return true if state is acceptable, or false and write an error message into error string
bool CoreChecks::ValidateDrawState(const DescriptorSet *descriptor_set, const BindingReqMap &bindings,
                                   const std::vector<uint32_t> &dynamic_offsets, const CMD_BUFFER_STATE *cb_node,
                                   const std::vector<IMAGE_VIEW_STATE *> *attachments, const std::vector<SUBPASS_INFO> *subpasses,
                                   const char *caller, const DrawDispatchVuid &vuids) const {
    layer_data::optional<layer_data::unordered_map<VkImageView, VkImageLayout>> checked_layouts;
    if (descriptor_set->GetTotalDescriptorCount() > cvdescriptorset::PrefilterBindRequestMap::kManyDescriptors_) {
        checked_layouts.emplace();
    }
    bool result = false;
    VkFramebuffer framebuffer = cb_node->activeFramebuffer ? cb_node->activeFramebuffer->framebuffer() : VK_NULL_HANDLE;
    for (const auto &binding_pair : bindings) {
        const auto binding = binding_pair.first;
        DescriptorSetLayout::ConstBindingIterator binding_it(descriptor_set->GetLayout().get(), binding);
        if (binding_it.AtEnd()) {  //  End at construction is the condition for an invalid binding.
            auto set = descriptor_set->GetSet();
            result |= LogError(set, vuids.descriptor_valid,
                               "%s encountered the following validation error at %s time: Attempting to "
                               "validate DrawState for binding #%u  which is an invalid binding for this descriptor set.",
                               report_data->FormatHandle(set).c_str(), caller, binding);
            return result;
        }

        if (binding_it.GetDescriptorBindingFlags() &
            (VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT)) {
            // Can't validate the descriptor because it may not have been updated,
            // or the view could have been destroyed
            continue;
        }
        // // This is a record time only path
        const bool record_time_validate = true;
        result |= ValidateDescriptorSetBindingData(cb_node, descriptor_set, dynamic_offsets, binding_pair, framebuffer, attachments,
                                                   subpasses, record_time_validate, caller, vuids, checked_layouts);
    }
    return result;
}

bool CoreChecks::ValidateDescriptorSetBindingData(const CMD_BUFFER_STATE *cb_node, const DescriptorSet *descriptor_set,
                                                  const std::vector<uint32_t> &dynamic_offsets,
                                                  const std::pair<const uint32_t, DescriptorRequirement> &binding_info,
                                                  VkFramebuffer framebuffer, const std::vector<IMAGE_VIEW_STATE *> *attachments,
                                                  const std::vector<SUBPASS_INFO> *subpasses, bool record_time_validate,
                                                  const char *caller, const DrawDispatchVuid &vuids,
                                                  layer_data::optional<layer_data::unordered_map<VkImageView, VkImageLayout>> &checked_layouts) const {
    using DescriptorClass = cvdescriptorset::DescriptorClass;
    using BufferDescriptor = cvdescriptorset::BufferDescriptor;
    using ImageDescriptor = cvdescriptorset::ImageDescriptor;
    using ImageSamplerDescriptor = cvdescriptorset::ImageSamplerDescriptor;
    using SamplerDescriptor = cvdescriptorset::SamplerDescriptor;
    using TexelDescriptor = cvdescriptorset::TexelDescriptor;
    using AccelerationStructureDescriptor = cvdescriptorset::AccelerationStructureDescriptor;
    const auto binding = binding_info.first;
    bool skip = false;
    DescriptorSetLayout::ConstBindingIterator binding_it(descriptor_set->GetLayout().get(), binding);
    {
        // Copy the range, the end range is subject to update based on variable length descriptor arrays.
        cvdescriptorset::IndexRange index_range = binding_it.GetGlobalIndexRange();
        auto array_idx = 0;  // Track array idx if we're dealing with array descriptors

        if (binding_it.IsVariableDescriptorCount()) {
            // Only validate the first N descriptors if it uses variable_count
            index_range.end = index_range.start + descriptor_set->GetVariableDescriptorCount();
        }
        for (uint32_t i = index_range.start; !skip && i < index_range.end; ++i, ++array_idx) {
            uint32_t index = i - index_range.start;
            const auto *descriptor = descriptor_set->GetDescriptorFromGlobalIndex(i);
            const auto descriptor_class = descriptor->GetClass();

            if (descriptor_class == DescriptorClass::InlineUniform) {
                // Can't validate the descriptor because it may not have been updated.
                continue;
            } else if (!descriptor->updated) {
                auto set = descriptor_set->GetSet();
                return LogError(
                    set, vuids.descriptor_valid,
                    "Descriptor set %s encountered the following validation error at %s time: Descriptor in binding #%" PRIu32
                    " index %" PRIu32
                    " is being used in draw but has never been updated via vkUpdateDescriptorSets() or a similar call.",
                    report_data->FormatHandle(set).c_str(), caller, binding, index);
            }
            switch (descriptor_class) {
                case DescriptorClass::GeneralBuffer: {
                    const auto *buffer_desc = static_cast<const BufferDescriptor *>(descriptor);
                    skip =
                        ValidateGeneralBufferDescriptor(caller, vuids, cb_node, descriptor_set, *buffer_desc, binding_info, index);
                } break;
                case DescriptorClass::ImageSampler: {
                    const auto *image_sampler_desc = static_cast<const ImageSamplerDescriptor *>(descriptor);
                    skip = ValidateImageDescriptor(caller, vuids, cb_node, descriptor_set, *image_sampler_desc, binding_info, index,
                                                   record_time_validate, attachments, subpasses, framebuffer, binding_it.GetType(),
                                                   checked_layouts);
                    if (!skip) {
                        skip = ValidateSamplerDescriptor(caller, vuids, cb_node, descriptor_set, binding_info, index,
                                                         image_sampler_desc->GetSampler(), image_sampler_desc->IsImmutableSampler(),
                                                         image_sampler_desc->GetSamplerState());
                    }
                } break;
                case DescriptorClass::Image: {
                    const auto *image_desc = static_cast<const ImageDescriptor *>(descriptor);
                    skip = ValidateImageDescriptor(caller, vuids, cb_node, descriptor_set, *image_desc, binding_info, index,
                                                   record_time_validate, attachments, subpasses, framebuffer, binding_it.GetType(),
                                                   checked_layouts);
                } break;
                case DescriptorClass::PlainSampler: {
                    const auto *sampler_desc = static_cast<const SamplerDescriptor *>(descriptor);
                    skip = ValidateSamplerDescriptor(caller, vuids, cb_node, descriptor_set, binding_info, index,
                                                     sampler_desc->GetSampler(), sampler_desc->IsImmutableSampler(),
                                                     sampler_desc->GetSamplerState());
                } break;
                case DescriptorClass::TexelBuffer: {
                    const auto *texel_desc = static_cast<const TexelDescriptor *>(descriptor);
                    skip = ValidateTexelDescriptor(caller, vuids, cb_node, descriptor_set, *texel_desc, binding_info, index);
                } break;
                case DescriptorClass::AccelerationStructure: {
                    const auto *accel_desc = static_cast<const AccelerationStructureDescriptor *>(descriptor);
                    skip = ValidateAccelerationDescriptor(caller, vuids, cb_node, descriptor_set, *accel_desc, binding_info, index);
                } break;
                default:
                    break;
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateGeneralBufferDescriptor(const char *caller, const DrawDispatchVuid &vuids, const CMD_BUFFER_STATE *cb_node,
                                                 const cvdescriptorset::DescriptorSet *descriptor_set,
                                                 const cvdescriptorset::BufferDescriptor &descriptor,
                                                 const std::pair<const uint32_t, DescriptorRequirement> &binding_info,
                                                 uint32_t index) const {
    // Verify that buffers are valid
    auto buffer = descriptor.GetBuffer();
    auto buffer_node = descriptor.GetBufferState();
    if ((!buffer_node && !enabled_features.robustness2_features.nullDescriptor) || (buffer_node && buffer_node->Destroyed())) {
        auto set = descriptor_set->GetSet();
        return LogError(set, vuids.descriptor_valid,
                        "Descriptor set %s encountered the following validation error at %s time: Descriptor in "
                        "binding #%" PRIu32 " index %" PRIu32 " is using buffer %s that is invalid or has been destroyed.",
                        report_data->FormatHandle(set).c_str(), caller, binding_info.first, index,
                        report_data->FormatHandle(buffer).c_str());
    }
    if (buffer) {
        if (buffer_node && !buffer_node->sparse) {
            for (const auto &item: buffer_node->GetBoundMemory()) {
                auto &binding = item.second;
                if (binding.mem_state->Destroyed()) {
                    auto set = descriptor_set->GetSet();
                    return LogError(set, vuids.descriptor_valid,
                                    "Descriptor set %s encountered the following validation error at %s time: Descriptor in "
                                    "binding #%" PRIu32 " index %" PRIu32 " is uses buffer %s that references invalid memory %s.",
                                    report_data->FormatHandle(set).c_str(), caller, binding_info.first, index,
                                    report_data->FormatHandle(buffer).c_str(),
                                    report_data->FormatHandle(binding.mem_state->mem()).c_str());
                }
            }
        }
        if (enabled_features.core11.protectedMemory == VK_TRUE) {
            if (ValidateProtectedBuffer(cb_node, buffer_node, caller, vuids.unprotected_command_buffer,
                                        "Buffer is in a descriptorSet")) {
                return true;
            }
            if (binding_info.second.is_writable &&
                ValidateUnprotectedBuffer(cb_node, buffer_node, caller, vuids.protected_command_buffer,
                                          "Buffer is in a descriptorSet")) {
                return true;
            }
        }
    }
    return false;
}

bool CoreChecks::ValidateImageDescriptor(const char *caller, const DrawDispatchVuid &vuids, const CMD_BUFFER_STATE *cb_node,
                                         const cvdescriptorset::DescriptorSet *descriptor_set,
                                         const cvdescriptorset::ImageDescriptor &image_descriptor,
                                         const std::pair<const uint32_t, DescriptorRequirement> &binding_info, uint32_t index,
                                         bool record_time_validate, const std::vector<IMAGE_VIEW_STATE *> *attachments,
                                         const std::vector<SUBPASS_INFO> *subpasses, VkFramebuffer framebuffer,
                                         VkDescriptorType descriptor_type,
                                         layer_data::optional<layer_data::unordered_map<VkImageView, VkImageLayout>> &checked_layouts) const {
    std::vector<const SAMPLER_STATE *> sampler_states;
    VkImageView image_view = image_descriptor.GetImageView();
    const IMAGE_VIEW_STATE *image_view_state = image_descriptor.GetImageViewState();
    VkImageLayout image_layout = image_descriptor.GetImageLayout();
    const auto binding = binding_info.first;
    const auto reqs = binding_info.second.reqs;

    if (image_descriptor.GetClass() == cvdescriptorset::DescriptorClass::ImageSampler) {
        sampler_states.emplace_back(
            static_cast<const cvdescriptorset::ImageSamplerDescriptor &>(image_descriptor).GetSamplerState());
    } else {
        if (binding_info.second.samplers_used_by_image.size() > index) {
            for (const auto &desc_index : binding_info.second.samplers_used_by_image[index]) {
                const auto *desc = descriptor_set->GetDescriptorFromBinding(desc_index.sampler_slot.binding, desc_index.sampler_index);
                // NOTE: This check _shouldn't_ be necessary due to the checks made in IsSpecificDescriptorType in
                //       shader_validation.cpp. However, without this check some traces still crash.
                if (desc && (desc->GetClass() == cvdescriptorset::DescriptorClass::PlainSampler)) {
                    const auto *sampler_state = static_cast<const cvdescriptorset::SamplerDescriptor *>(desc)->GetSamplerState();
                    if (sampler_state) sampler_states.emplace_back(sampler_state);
                }
            }
        }
    }

    if ((!image_view_state && !enabled_features.robustness2_features.nullDescriptor) ||
        (image_view_state && image_view_state->Destroyed())) {
        // Image view must have been destroyed since initial update. Could potentially flag the descriptor
        //  as "invalid" (updated = false) at DestroyImageView() time and detect this error at bind time

        auto set = descriptor_set->GetSet();
        return LogError(set, vuids.descriptor_valid,
                        "Descriptor set %s encountered the following validation error at %s time: Descriptor in "
                        "binding #%" PRIu32 " index %" PRIu32 " is using imageView %s that is invalid or has been destroyed.",
                        report_data->FormatHandle(set).c_str(), caller, binding, index,
                        report_data->FormatHandle(image_view).c_str());
    }
    if (image_view) {
        const auto &image_view_ci = image_view_state->create_info;
        const auto *image_state = image_view_state->image_state.get();

        if (reqs & DESCRIPTOR_REQ_ALL_VIEW_TYPE_BITS) {
            if (~reqs & (1 << image_view_ci.viewType)) {
                auto set = descriptor_set->GetSet();
                return LogError(set, vuids.descriptor_valid,
                                "Descriptor set %s encountered the following validation error at %s time: Descriptor "
                                "in binding #%" PRIu32 " index %" PRIu32 " requires an image view of type %s but got %s.",
                                report_data->FormatHandle(set).c_str(), caller, binding, index,
                                StringDescriptorReqViewType(reqs).c_str(), string_VkImageViewType(image_view_ci.viewType));
            }

            if (!(reqs & image_view_state->descriptor_format_bits)) {
                // bad component type
                auto set = descriptor_set->GetSet();
                return LogError(set, vuids.descriptor_valid,
                                "Descriptor set %s encountered the following validation error at %s time: "
                                "Descriptor in binding "
                                "#%" PRIu32 " index %" PRIu32 " requires %s component type, but bound descriptor format is %s.",
                                report_data->FormatHandle(set).c_str(), caller, binding, index,
                                StringDescriptorReqComponentType(reqs), string_VkFormat(image_view_ci.format));
            }
        }

        // NOTE: Submit time validation of UPDATE_AFTER_BIND image layout is not possible with the
        // image layout tracking as currently implemented, so only record_time_validation is done
        if (!disabled[image_layout_validation] && record_time_validate) {
            // Verify Image Layout
            // No "invalid layout" VUID required for this call, since the optimal_layout parameter is UNDEFINED.
            // The caller provides a checked_layouts map when there are a large number of layouts to check,
            // making it worthwhile to keep track of verified layouts and not recheck them.
            bool already_validated = false;
            if (checked_layouts) {
                auto search = checked_layouts->find(image_view);
                if (search != checked_layouts->end() && search->second == image_layout) {
                    already_validated = true;
                }
            }
            if (!already_validated) {
                bool hit_error = false;
                VerifyImageLayout(cb_node, image_state, image_view_state->normalized_subresource_range,
                                  image_view_ci.subresourceRange.aspectMask, image_layout, VK_IMAGE_LAYOUT_UNDEFINED, caller,
                                  kVUIDUndefined, "VUID-VkDescriptorImageInfo-imageLayout-00344", &hit_error);
                if (hit_error) {
                    auto set = descriptor_set->GetSet();
                    return LogError(set, vuids.descriptor_valid,
                                    "Descriptor set %s encountered the following validation error at %s time: Image layout "
                                    "specified "
                                    "at vkUpdateDescriptorSet* or vkCmdPushDescriptorSet* time "
                                    "doesn't match actual image layout at time descriptor is used. See previous error callback for "
                                    "specific details.",
                                    report_data->FormatHandle(set).c_str(), caller);
                }
                if (checked_layouts) {
                    checked_layouts->emplace(image_view, image_layout);
                }
            }
        }

        // Verify Sample counts
        if ((reqs & DESCRIPTOR_REQ_SINGLE_SAMPLE) && image_view_state->samples != VK_SAMPLE_COUNT_1_BIT) {
            auto set = descriptor_set->GetSet();
            return LogError(set, vuids.descriptor_valid,
                            "Descriptor set %s encountered the following validation error at %s time: Descriptor in "
                            "binding #%" PRIu32 " index %" PRIu32 " requires bound image to have VK_SAMPLE_COUNT_1_BIT but got %s.",
                            report_data->FormatHandle(set).c_str(), caller, binding, index,
                            string_VkSampleCountFlagBits(image_view_state->samples));
        }
        if ((reqs & DESCRIPTOR_REQ_MULTI_SAMPLE) && image_view_state->samples == VK_SAMPLE_COUNT_1_BIT) {
            auto set = descriptor_set->GetSet();
            return LogError(set, vuids.descriptor_valid,
                            "Descriptor set %s encountered the following validation error at %s time: Descriptor "
                            "in binding #%" PRIu32 " index %" PRIu32
                            " requires bound image to have multiple samples, but got VK_SAMPLE_COUNT_1_BIT.",
                            report_data->FormatHandle(set).c_str(), caller, binding, index);
        }

        // Verify VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT
        if ((reqs & DESCRIPTOR_REQ_VIEW_ATOMIC_OPERATION) && (descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) &&
            !(image_view_state->format_features & VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT)) {
            auto set = descriptor_set->GetSet();
            LogObjectList objlist(set);
            objlist.add(image_view);
            return LogError(objlist, vuids.imageview_atomic,
                            "Descriptor set %s encountered the following validation error at %s time: Descriptor "
                            "in binding #%" PRIu32 " index %" PRIu32
                            ", %s, format %s, doesn't "
                            "contain VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT.",
                            report_data->FormatHandle(set).c_str(), caller, binding, index,
                            report_data->FormatHandle(image_view).c_str(), string_VkFormat(image_view_ci.format));
        }

        // When KHR_format_feature_flags2 is supported, the read/write without
        // format support is reported per format rather as a blankey physical
        // device feature.
        if (has_format_feature2) {
            const VkFormatFeatureFlags2 format_features = image_view_state->format_features;

            if (descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) {
                if ((reqs & DESCRIPTOR_REQ_IMAGE_READ_WITHOUT_FORMAT) &&
                    !(format_features & VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT)) {
                    auto set = descriptor_set->GetSet();
                    LogObjectList objlist(set);
                    objlist.add(image_view);
                    return LogError(objlist, vuids.storage_image_read_without_format,
                                    "Descriptor set %s encountered the following validation error at %s time: Descriptor "
                                    "in binding #%" PRIu32 " index %" PRIu32
                                    ", %s, image view format %s feature flags (%s) doesn't "
                                    "contain VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT",
                                    report_data->FormatHandle(set).c_str(), caller, binding, index,
                                    report_data->FormatHandle(image_view).c_str(), string_VkFormat(image_view_ci.format),
                                    string_VkFormatFeatureFlags2(format_features).c_str());
                }

                if ((reqs & DESCRIPTOR_REQ_IMAGE_WRITE_WITHOUT_FORMAT) &&
                    !(format_features & VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT)) {
                    auto set = descriptor_set->GetSet();
                    LogObjectList objlist(set);
                    objlist.add(image_view);
                    return LogError(objlist, vuids.storage_image_write_without_format,
                                    "Descriptor set %s encountered the following validation error at %s time: Descriptor "
                                    "in binding #%" PRIu32 " index %" PRIu32
                                    ", %s, image view format %s feature flags (%s) doesn't "
                                    "contain VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT",
                                    report_data->FormatHandle(set).c_str(), caller, binding, index,
                                    report_data->FormatHandle(image_view).c_str(), string_VkFormat(image_view_ci.format),
                                    string_VkFormatFeatureFlags2(format_features).c_str());
                }
            }

            if ((reqs & DESCRIPTOR_REQ_IMAGE_DREF) && !(format_features & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT)) {
                auto set = descriptor_set->GetSet();
                LogObjectList objlist(set);
                objlist.add(image_view);
                return LogError(objlist, vuids.depth_compare_sample,
                                "Descriptor set %s encountered the following validation error at %s time: Descriptor "
                                "in binding #%" PRIu32 " index %" PRIu32
                                ", %s, image view format %s feature flags (%s) doesn't "
                                "contain VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT",
                                report_data->FormatHandle(set).c_str(), caller, binding, index,
                                report_data->FormatHandle(image_view).c_str(), string_VkFormat(image_view_ci.format),
                                string_VkFormatFeatureFlags2(format_features).c_str());
            }
        }

        // Verify if attachments are used in DescriptorSet
        if (attachments && attachments->size() > 0 && subpasses && (descriptor_type != VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)) {
            for (uint32_t att_index = 0; att_index < attachments->size(); ++att_index) {
                const auto &view_state = (*attachments)[att_index];
                const SUBPASS_INFO &subpass = (*subpasses)[att_index];
                if (!view_state || view_state->Destroyed()) {
                    continue;
                }
                bool same_view = view_state->image_view() == image_view;
                bool overlapping_view = image_view_state->OverlapSubresource(*view_state);
                if (!same_view && !overlapping_view) {
                    continue;
                }

                bool readable = false;
                bool writable = false;
                uint32_t set_index = std::numeric_limits<uint32_t>::max();
                for (uint32_t i = 0; i < cb_node->lastBound[VK_PIPELINE_BIND_POINT_GRAPHICS].per_set.size(); ++i) {
                    const auto &set = cb_node->lastBound[VK_PIPELINE_BIND_POINT_GRAPHICS].per_set[i];
                    if (set.bound_descriptor_set.get() == descriptor_set) {
                        set_index = i;
                        break;
                    }
                }
                assert(set_index != std::numeric_limits<uint32_t>::max());
                const auto pipeline = cb_node->GetCurrentPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS);
                for (const auto &stage : pipeline->stage_state) {
                    for (const auto &descriptor : stage.descriptor_uses) {
                        if (descriptor.first.set == set_index && descriptor.first.binding == binding) {
                            writable |= descriptor.second.is_writable;
                            readable |= descriptor.second.is_readable | descriptor.second.is_sampler_implicitLod_dref_proj;
                            break;
                        }
                    }
                }

                bool write_attachment =
                    (subpass.usage & (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) > 0;
                if (write_attachment && readable) {
                    if (same_view) {
                        auto set = descriptor_set->GetSet();
                        LogObjectList objlist(set);
                        objlist.add(image_view);
                        objlist.add(framebuffer);
                        return LogError(
                            objlist, vuids.image_subresources_subpass_read,
                            "Descriptor set %s encountered the following validation error at %s time: %s is being read from in "
                            "Descriptor in binding #%" PRIu32 " index %" PRIu32
                            " and will be written to as %s attachment # %" PRIu32 ".",
                            report_data->FormatHandle(set).c_str(), caller, report_data->FormatHandle(image_view).c_str(), binding,
                            index, report_data->FormatHandle(framebuffer).c_str(), att_index);
                    } else if (overlapping_view) {
                        auto set = descriptor_set->GetSet();
                        LogObjectList objlist(set);
                        objlist.add(image_view);
                        objlist.add(framebuffer);
                        objlist.add(view_state->image_view());
                        return LogError(objlist, vuids.image_subresources_subpass_read,
                                        "Descriptor set %s encountered the following validation error at %s time: "
                                        "Image subresources of %s is being read from in "
                                        "Descriptor in binding #%" PRIu32 " index %" PRIu32
                                        " and will be written to as %s in %s attachment # %" PRIu32 " overlap.",
                                        report_data->FormatHandle(set).c_str(), caller,
                                        report_data->FormatHandle(image_view).c_str(), binding, index,
                                        report_data->FormatHandle(view_state->image_view()).c_str(),
                                        report_data->FormatHandle(framebuffer).c_str(), att_index);
                    }
                }
                bool read_attachment = (subpass.usage & (VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) > 0;
                if (read_attachment && writable) {
                    if (same_view) {
                        auto set = descriptor_set->GetSet();
                        LogObjectList objlist(set);
                        objlist.add(image_view);
                        objlist.add(framebuffer);
                        return LogError(
                            objlist, vuids.image_subresources_subpass_write,
                            "Descriptor set %s encountered the following validation error at %s time: %s is being written to in "
                            "Descriptor in binding #%" PRIu32 " index %" PRIu32 " and read from as %s attachment # %" PRIu32 ".",
                            report_data->FormatHandle(set).c_str(), caller, report_data->FormatHandle(image_view).c_str(), binding,
                            index, report_data->FormatHandle(framebuffer).c_str(), att_index);
                    } else if (overlapping_view) {
                        auto set = descriptor_set->GetSet();
                        LogObjectList objlist(set);
                        objlist.add(image_view);
                        objlist.add(framebuffer);
                        objlist.add(view_state->image_view());
                        return LogError(objlist, vuids.image_subresources_subpass_write,
                                        "Descriptor set %s encountered the following validation error at %s time: "
                                        "Image subresources of %s is being written to in "
                                        "Descriptor in binding #%" PRIu32 " index %" PRIu32
                                        " and will be read from as %s in %s attachment # %" PRIu32 " overlap.",
                                        report_data->FormatHandle(set).c_str(), caller,
                                        report_data->FormatHandle(image_view).c_str(), binding, index,
                                        report_data->FormatHandle(view_state->image_view()).c_str(),
                                        report_data->FormatHandle(framebuffer).c_str(), att_index);
                    }
                }

                if (writable) {
                    if (same_view) {
                        auto set = descriptor_set->GetSet();
                        LogObjectList objlist(set);
                        objlist.add(image_view);
                        objlist.add(framebuffer);
                        return LogError(
                            objlist, vuids.image_subresources_render_pass_write,
                            "Descriptor set %s encountered the following validation error at %s time: %s is used in "
                            "Descriptor in binding #%" PRIu32 " index %" PRIu32 " as writable and %s attachment # %" PRIu32 ".",
                            report_data->FormatHandle(set).c_str(), caller, report_data->FormatHandle(image_view).c_str(), binding,
                            index, report_data->FormatHandle(framebuffer).c_str(), att_index);
                    } else if (overlapping_view) {
                        auto set = descriptor_set->GetSet();
                        LogObjectList objlist(set);
                        objlist.add(image_view);
                        objlist.add(framebuffer);
                        objlist.add(view_state->image_view());
                        return LogError(objlist, vuids.image_subresources_render_pass_write,
                                        "Descriptor set %s encountered the following validation error at %s time: "
                                        "Image subresources of %s in "
                                        "writable Descriptor in binding #%" PRIu32 " index %" PRIu32
                                        " and %s in %s attachment # %" PRIu32 " overlap.",
                                        report_data->FormatHandle(set).c_str(), caller,
                                        report_data->FormatHandle(image_view).c_str(), binding, index,
                                        report_data->FormatHandle(view_state->image_view()).c_str(),
                                        report_data->FormatHandle(framebuffer).c_str(), att_index);
                    }
                }
            }
            if (enabled_features.core11.protectedMemory == VK_TRUE) {
                if (ValidateProtectedImage(cb_node, image_view_state->image_state.get(), caller, vuids.unprotected_command_buffer,
                                           "Image is in a descriptorSet")) {
                    return true;
                }
                if (binding_info.second.is_writable &&
                    ValidateUnprotectedImage(cb_node, image_view_state->image_state.get(), caller, vuids.protected_command_buffer,
                                             "Image is in a descriptorSet")) {
                    return true;
                }
            }
        }

        for (const auto *sampler_state : sampler_states) {
            if (!sampler_state || sampler_state->Destroyed()) {
                continue;
            }

            // TODO: Validate 04015 for DescriptorClass::PlainSampler
            if ((sampler_state->createInfo.borderColor == VK_BORDER_COLOR_INT_CUSTOM_EXT ||
                 sampler_state->createInfo.borderColor == VK_BORDER_COLOR_FLOAT_CUSTOM_EXT) &&
                (sampler_state->customCreateInfo.format == VK_FORMAT_UNDEFINED)) {
                if (image_view_state->create_info.format == VK_FORMAT_B4G4R4A4_UNORM_PACK16 ||
                    image_view_state->create_info.format == VK_FORMAT_B5G6R5_UNORM_PACK16 ||
                    image_view_state->create_info.format == VK_FORMAT_B5G5R5A1_UNORM_PACK16) {
                    auto set = descriptor_set->GetSet();
                    LogObjectList objlist(set);
                    objlist.add(sampler_state->sampler());
                    objlist.add(image_view_state->image_view());
                    return LogError(objlist, "VUID-VkSamplerCustomBorderColorCreateInfoEXT-format-04015",
                                    "Descriptor set %s encountered the following validation error at %s time: Sampler %s in "
                                    "binding #%" PRIu32 " index %" PRIu32
                                    " has a custom border color with format = VK_FORMAT_UNDEFINED and is used to "
                                    "sample an image view %s with format %s",
                                    report_data->FormatHandle(set).c_str(), caller,
                                    report_data->FormatHandle(sampler_state->sampler()).c_str(), binding, index,
                                    report_data->FormatHandle(image_view_state->image_view()).c_str(),
                                    string_VkFormat(image_view_state->create_info.format));
                }
            }
            VkFilter sampler_mag_filter = sampler_state->createInfo.magFilter;
            VkFilter sampler_min_filter = sampler_state->createInfo.minFilter;
            VkBool32 sampler_compare_enable = sampler_state->createInfo.compareEnable;
            if ((sampler_mag_filter == VK_FILTER_LINEAR || sampler_min_filter == VK_FILTER_LINEAR) &&
                (sampler_compare_enable == VK_FALSE) &&
                !(image_view_state->format_features & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
                auto set = descriptor_set->GetSet();
                LogObjectList objlist(set);
                objlist.add(sampler_state->sampler());
                objlist.add(image_view_state->image_view());
                return LogError(objlist, vuids.linear_sampler,
                                "Descriptor set %s encountered the following validation error at %s time: Sampler "
                                "(%s) is set to use VK_FILTER_LINEAR with "
                                "compareEnable is set to VK_FALSE, but image view's (%s) format (%s) does not "
                                "contain VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT in its format features.",
                                report_data->FormatHandle(set).c_str(), caller,
                                report_data->FormatHandle(sampler_state->sampler()).c_str(),
                                report_data->FormatHandle(image_view_state->image_view()).c_str(),
                                string_VkFormat(image_view_state->create_info.format));
            }
            if (sampler_mag_filter == VK_FILTER_CUBIC_EXT || sampler_min_filter == VK_FILTER_CUBIC_EXT) {
                if (!(image_view_state->format_features & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT)) {
                    auto set = descriptor_set->GetSet();
                    LogObjectList objlist(set);
                    objlist.add(sampler_state->sampler());
                    objlist.add(image_view_state->image_view());
                    return LogError(objlist, vuids.cubic_sampler,
                                    "Descriptor set %s encountered the following validation error at %s time: "
                                    "Sampler (%s) is set to use VK_FILTER_CUBIC_EXT, then "
                                    "image view's (%s) format (%s) MUST contain "
                                    "VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT in its format features.",
                                    report_data->FormatHandle(set).c_str(), caller,
                                    report_data->FormatHandle(sampler_state->sampler()).c_str(),
                                    report_data->FormatHandle(image_view_state->image_view()).c_str(),
                                    string_VkFormat(image_view_state->create_info.format));
                }

                if (IsExtEnabled(device_extensions.vk_ext_filter_cubic)) {
                    const auto reduction_mode_info =
                        LvlFindInChain<VkSamplerReductionModeCreateInfo>(sampler_state->createInfo.pNext);
                    if (reduction_mode_info &&
                        (reduction_mode_info->reductionMode == VK_SAMPLER_REDUCTION_MODE_MIN ||
                         reduction_mode_info->reductionMode == VK_SAMPLER_REDUCTION_MODE_MAX) &&
                        !image_view_state->filter_cubic_props.filterCubicMinmax) {
                        auto set = descriptor_set->GetSet();
                        LogObjectList objlist(set);
                        objlist.add(sampler_state->sampler());
                        objlist.add(image_view_state->image_view());
                        return LogError(objlist, vuids.filter_cubic_min_max,
                                        "Descriptor set %s encountered the following validation error at %s time: "
                                        "Sampler (%s) is set to use VK_FILTER_CUBIC_EXT & %s, "
                                        "but image view (%s) doesn't support filterCubicMinmax.",
                                        report_data->FormatHandle(set).c_str(), caller,
                                        report_data->FormatHandle(sampler_state->sampler()).c_str(),
                                        string_VkSamplerReductionMode(reduction_mode_info->reductionMode),
                                        report_data->FormatHandle(image_view_state->image_view()).c_str());
                    }

                    if (!image_view_state->filter_cubic_props.filterCubic) {
                        auto set = descriptor_set->GetSet();
                        LogObjectList objlist(set);
                        objlist.add(sampler_state->sampler());
                        objlist.add(image_view_state->image_view());
                        return LogError(objlist, vuids.filter_cubic,
                                        "Descriptor set %s encountered the following validation error at %s time: "
                                        "Sampler (%s) is set to use VK_FILTER_CUBIC_EXT, "
                                        "but image view (%s) doesn't support filterCubic.",
                                        report_data->FormatHandle(set).c_str(), caller,
                                        report_data->FormatHandle(sampler_state->sampler()).c_str(),
                                        report_data->FormatHandle(image_view_state->image_view()).c_str());
                    }
                }

                if (IsExtEnabled(device_extensions.vk_img_filter_cubic)) {
                    if (image_view_state->create_info.viewType == VK_IMAGE_VIEW_TYPE_3D ||
                        image_view_state->create_info.viewType == VK_IMAGE_VIEW_TYPE_CUBE ||
                        image_view_state->create_info.viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY) {
                        auto set = descriptor_set->GetSet();
                        LogObjectList objlist(set);
                        objlist.add(sampler_state->sampler());
                        objlist.add(image_view_state->image_view());
                        return LogError(objlist, vuids.img_filter_cubic,
                                        "Descriptor set %s encountered the following validation error at %s time: Sampler "
                                        "(%s)is set to use VK_FILTER_CUBIC_EXT while the VK_IMG_filter_cubic extension "
                                        "is enabled, but image view (%s) has an invalid imageViewType (%s).",
                                        report_data->FormatHandle(set).c_str(), caller,
                                        report_data->FormatHandle(sampler_state->sampler()).c_str(),
                                        report_data->FormatHandle(image_view_state->image_view()).c_str(),
                                        string_VkImageViewType(image_view_state->create_info.viewType));
                    }
                }
            }

            if ((image_state->createInfo.flags & VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV) &&
                (sampler_state->createInfo.addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE ||
                 sampler_state->createInfo.addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE ||
                 sampler_state->createInfo.addressModeW != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)) {
                std::string address_mode_letter =
                    (sampler_state->createInfo.addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
                        ? "U"
                        : (sampler_state->createInfo.addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE) ? "V" : "W";
                VkSamplerAddressMode address_mode =
                    (sampler_state->createInfo.addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
                        ? sampler_state->createInfo.addressModeU
                        : (sampler_state->createInfo.addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
                              ? sampler_state->createInfo.addressModeV
                              : sampler_state->createInfo.addressModeW;
                auto set = descriptor_set->GetSet();
                LogObjectList objlist(set);
                objlist.add(sampler_state->sampler());
                objlist.add(image_state->image());
                objlist.add(image_view_state->image_view());
                return LogError(objlist, vuids.corner_sampled_address_mode,
                                "Descriptor set %s encountered the following validation error at %s time: Image "
                                "(%s) in image view (%s) is created with flag "
                                "VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV and can only be sampled using "
                                "VK_SAMPLER_ADDRESS_MODE_CLAMP_EDGE, but sampler (%s) has "
                                "createInfo.addressMode%s set to %s.",
                                report_data->FormatHandle(set).c_str(), caller,
                                report_data->FormatHandle(image_state->image()).c_str(),
                                report_data->FormatHandle(image_view_state->image_view()).c_str(),
                                report_data->FormatHandle(sampler_state->sampler()).c_str(), address_mode_letter.c_str(),
                                string_VkSamplerAddressMode(address_mode));
            }

            // UnnormalizedCoordinates sampler validations
            if (sampler_state->createInfo.unnormalizedCoordinates) {
                // If ImageView is used by a unnormalizedCoordinates sampler, it needs to check ImageView type
                if (image_view_ci.viewType == VK_IMAGE_VIEW_TYPE_3D || image_view_ci.viewType == VK_IMAGE_VIEW_TYPE_CUBE ||
                    image_view_ci.viewType == VK_IMAGE_VIEW_TYPE_1D_ARRAY ||
                    image_view_ci.viewType == VK_IMAGE_VIEW_TYPE_2D_ARRAY ||
                    image_view_ci.viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY) {
                    auto set = descriptor_set->GetSet();
                    LogObjectList objlist(set);
                    objlist.add(image_view);
                    objlist.add(sampler_state->sampler());
                    return LogError(objlist, vuids.sampler_imageview_type,
                                    "Descriptor set %s encountered the following validation error at %s time: %s, type: %s in "
                                    "Descriptor in binding #%" PRIu32 " index %" PRIu32 "is used by %s.",
                                    report_data->FormatHandle(set).c_str(), caller, report_data->FormatHandle(image_view).c_str(),
                                    string_VkImageViewType(image_view_ci.viewType), binding, index,
                                    report_data->FormatHandle(sampler_state->sampler()).c_str());
                }

                // sampler must not be used with any of the SPIR-V OpImageSample* or OpImageSparseSample*
                // instructions with ImplicitLod, Dref or Proj in their name
                if (reqs & DESCRIPTOR_REQ_SAMPLER_IMPLICITLOD_DREF_PROJ) {
                    auto set = descriptor_set->GetSet();
                    LogObjectList objlist(set);
                    objlist.add(image_view);
                    objlist.add(sampler_state->sampler());
                    return LogError(objlist, vuids.sampler_implicitLod_dref_proj,
                                    "Descriptor set %s encountered the following validation error at %s time: %s in "
                                    "Descriptor in binding #%" PRIu32 " index %" PRIu32
                                    " is used by %s that uses invalid operator.",
                                    report_data->FormatHandle(set).c_str(), caller, report_data->FormatHandle(image_view).c_str(),
                                    binding, index, report_data->FormatHandle(sampler_state->sampler()).c_str());
                }

                // sampler must not be used with any of the SPIR-V OpImageSample* or OpImageSparseSample*
                // instructions that includes a LOD bias or any offset values
                if (reqs & DESCRIPTOR_REQ_SAMPLER_BIAS_OFFSET) {
                    auto set = descriptor_set->GetSet();
                    LogObjectList objlist(set);
                    objlist.add(image_view);
                    objlist.add(sampler_state->sampler());
                    return LogError(objlist, vuids.sampler_bias_offset,
                                    "Descriptor set %s encountered the following validation error at %s time: %s in "
                                    "Descriptor in binding #%" PRIu32 " index %" PRIu32
                                    " is used by %s that uses invalid bias or offset operator.",
                                    report_data->FormatHandle(set).c_str(), caller, report_data->FormatHandle(image_view).c_str(),
                                    binding, index, report_data->FormatHandle(sampler_state->sampler()).c_str());
                }
            }
        }
    }
    return false;
}

bool CoreChecks::ValidateTexelDescriptor(const char *caller, const DrawDispatchVuid &vuids, const CMD_BUFFER_STATE *cb_node,
                                         const cvdescriptorset::DescriptorSet *descriptor_set,
                                         const cvdescriptorset::TexelDescriptor &texel_descriptor,
                                         const std::pair<const uint32_t, DescriptorRequirement> &binding_info,
                                         uint32_t index) const {
    auto buffer_view = texel_descriptor.GetBufferView();
    auto buffer_view_state = texel_descriptor.GetBufferViewState();
    const auto binding = binding_info.first;
    const auto reqs = binding_info.second.reqs;
    if ((!buffer_view_state && !enabled_features.robustness2_features.nullDescriptor) ||
        (buffer_view_state && buffer_view_state->Destroyed())) {
        auto set = descriptor_set->GetSet();
        return LogError(set, vuids.descriptor_valid,
                        "Descriptor set %s encountered the following validation error at %s time: Descriptor in "
                        "binding #%" PRIu32 " index %" PRIu32 " is using bufferView %s that is invalid or has been destroyed.",
                        report_data->FormatHandle(set).c_str(), caller, binding, index,
                        report_data->FormatHandle(buffer_view).c_str());
    }
    if (buffer_view) {
        auto buffer = buffer_view_state->create_info.buffer;
        const auto *buffer_state = buffer_view_state->buffer_state.get();
        const VkFormat buffer_view_format = buffer_view_state->create_info.format;
        if (buffer_state->Destroyed()) {
            auto set = descriptor_set->GetSet();
            return LogError(set, vuids.descriptor_valid,
                            "Descriptor set %s encountered the following validation error at %s time: Descriptor in "
                            "binding #%" PRIu32 " index %" PRIu32 " is using buffer %s that has been destroyed.",
                            report_data->FormatHandle(set).c_str(), caller, binding, index,
                            report_data->FormatHandle(buffer).c_str());
        }
        auto format_bits = DescriptorRequirementsBitsFromFormat(buffer_view_format);

        if (!(reqs & format_bits)) {
            // bad component type
            auto set = descriptor_set->GetSet();
            return LogError(set, vuids.descriptor_valid,
                            "Descriptor set %s encountered the following validation error at %s time: Descriptor in "
                            "binding #%" PRIu32 " index %" PRIu32 " requires %s component type, but bound descriptor format is %s.",
                            report_data->FormatHandle(set).c_str(), caller, binding, index, StringDescriptorReqComponentType(reqs),
                            string_VkFormat(buffer_view_format));
        }

        const VkFormatFeatureFlags2KHR format_features = buffer_view_state->format_features;
        const VkDescriptorType descriptor_type = descriptor_set->GetTypeFromBinding(binding);

        // Verify VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT
        if ((reqs & DESCRIPTOR_REQ_VIEW_ATOMIC_OPERATION) && (descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER) &&
            !(format_features & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT)) {
            auto set = descriptor_set->GetSet();
            LogObjectList objlist(set);
            objlist.add(buffer_view);
            return LogError(objlist, "UNASSIGNED-None-MismatchAtomicBufferFeature",
                            "Descriptor set %s encountered the following validation error at %s time: Descriptor "
                            "in binding #%" PRIu32 " index %" PRIu32
                            ", %s, format %s, doesn't "
                            "contain VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT.",
                            report_data->FormatHandle(set).c_str(), caller, binding, index,
                            report_data->FormatHandle(buffer_view).c_str(), string_VkFormat(buffer_view_format));
        }

        if (descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER) {
            if ((reqs & DESCRIPTOR_REQ_IMAGE_READ_WITHOUT_FORMAT) &&
                !(format_features & VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT_KHR)) {
                auto set = descriptor_set->GetSet();
                LogObjectList objlist(set);
                objlist.add(buffer_view);
                return LogError(objlist, vuids.storage_image_read_without_format,
                                "Descriptor set %s encountered the following validation error at %s time: Descriptor "
                                "in binding #%" PRIu32 " index %" PRIu32
                                ", %s, buffer view format %s feature flags (%s) doesn't "
                                "contain VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT_KHR",
                                report_data->FormatHandle(set).c_str(), caller, binding, index,
                                report_data->FormatHandle(buffer_view).c_str(), string_VkFormat(buffer_view_format),
                                string_VkFormatFeatureFlags2KHR(format_features).c_str());
            }

            if ((reqs & DESCRIPTOR_REQ_IMAGE_WRITE_WITHOUT_FORMAT) &&
                !(format_features & VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT_KHR)) {
                auto set = descriptor_set->GetSet();
                LogObjectList objlist(set);
                objlist.add(buffer_view);
                return LogError(objlist, vuids.storage_image_write_without_format,
                                "Descriptor set %s encountered the following validation error at %s time: Descriptor "
                                "in binding #%" PRIu32 " index %" PRIu32
                                ", %s, buffer view format %s feature flags (%s) doesn't "
                                "contain VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT_KHR",
                                report_data->FormatHandle(set).c_str(), caller, binding, index,
                                report_data->FormatHandle(buffer_view).c_str(), string_VkFormat(buffer_view_format),
                                string_VkFormatFeatureFlags2KHR(format_features).c_str());
            }
        }

        if (enabled_features.core11.protectedMemory == VK_TRUE) {
            if (ValidateProtectedBuffer(cb_node, buffer_view_state->buffer_state.get(), caller, vuids.unprotected_command_buffer,
                                        "Buffer is in a descriptorSet")) {
                return true;
            }
            if (binding_info.second.is_writable &&
                ValidateUnprotectedBuffer(cb_node, buffer_view_state->buffer_state.get(), caller, vuids.protected_command_buffer,
                                          "Buffer is in a descriptorSet")) {
                return true;
            }
        }
    }
    return false;
}

bool CoreChecks::ValidateAccelerationDescriptor(const char *caller, const DrawDispatchVuid &vuids, const CMD_BUFFER_STATE *cb_node,
                                                const cvdescriptorset::DescriptorSet *descriptor_set,
                                                const cvdescriptorset::AccelerationStructureDescriptor &descriptor,
                                                const std::pair<const uint32_t, DescriptorRequirement> &binding_info,
                                                uint32_t index) const {
    // Verify that acceleration structures are valid
    const auto binding = binding_info.first;
    if (descriptor.is_khr()) {
        auto acc = descriptor.GetAccelerationStructure();
        auto acc_node = descriptor.GetAccelerationStructureStateKHR();
        if (!acc_node || acc_node->Destroyed()) {
            if (acc != VK_NULL_HANDLE || !enabled_features.robustness2_features.nullDescriptor) {
                auto set = descriptor_set->GetSet();
                return LogError(set, vuids.descriptor_valid,
                                "Descriptor set %s encountered the following validation error at %s time: "
                                "Descriptor in binding #%" PRIu32 " index %" PRIu32
                                " is using acceleration structure %s that is invalid or has been destroyed.",
                                report_data->FormatHandle(set).c_str(), caller, binding, index,
                                report_data->FormatHandle(acc).c_str());
            }
        } else {
            for (const auto &item: acc_node->buffer_state->GetBoundMemory()) {
                auto &mem_binding = item.second;
                if (mem_binding.mem_state->Destroyed()) {
                    auto set = descriptor_set->GetSet();
                    return LogError(set, vuids.descriptor_valid,
                                    "Descriptor set %s encountered the following validation error at %s time: Descriptor in "
                                    "binding #%" PRIu32 " index %" PRIu32
                                    " is using acceleration structure %s that references invalid memory %s.",
                                    report_data->FormatHandle(set).c_str(), caller, binding, index,
                                    report_data->FormatHandle(acc).c_str(),
                                    report_data->FormatHandle(mem_binding.mem_state->mem()).c_str());
                }
            }
        }
    } else {
        auto acc = descriptor.GetAccelerationStructureNV();
        auto acc_node = descriptor.GetAccelerationStructureStateNV();
        if (!acc_node || acc_node->Destroyed()) {
            if (acc != VK_NULL_HANDLE || !enabled_features.robustness2_features.nullDescriptor) {
                auto set = descriptor_set->GetSet();
                return LogError(set, vuids.descriptor_valid,
                                "Descriptor set %s encountered the following validation error at %s time: "
                                "Descriptor in binding #%" PRIu32 " index %" PRIu32
                                " is using acceleration structure %s that is invalid or has been destroyed.",
                                report_data->FormatHandle(set).c_str(), caller, binding, index,
                                report_data->FormatHandle(acc).c_str());
            }
        } else {
            for (const auto &item : acc_node->GetBoundMemory()) {
                auto &mem_binding = item.second;
                if (mem_binding.mem_state->Destroyed()) {
                    auto set = descriptor_set->GetSet();
                    return LogError(set, vuids.descriptor_valid,
                                    "Descriptor set %s encountered the following validation error at %s time: Descriptor in "
                                    "binding #%" PRIu32 " index %" PRIu32
                                    " is using acceleration structure %s that references invalid memory %s.",
                                    report_data->FormatHandle(set).c_str(), caller, binding, index,
                                    report_data->FormatHandle(acc).c_str(),
                                    report_data->FormatHandle(mem_binding.mem_state->mem()).c_str());
                }
            }
        }
    }
    return false;
}

// If the validation is related to both of image and sampler,
// please leave it in (descriptor_class == DescriptorClass::ImageSampler || descriptor_class ==
// DescriptorClass::Image) Here is to validate for only sampler.
bool CoreChecks::ValidateSamplerDescriptor(const char *caller, const DrawDispatchVuid &vuids, const CMD_BUFFER_STATE *cb_node,
                                           const cvdescriptorset::DescriptorSet *descriptor_set,
                                           const std::pair<const uint32_t, DescriptorRequirement> &binding_info, uint32_t index,
                                           VkSampler sampler, bool is_immutable, const SAMPLER_STATE *sampler_state) const {
    // Verify Sampler still valid
    if (!sampler_state || sampler_state->Destroyed()) {
        auto set = descriptor_set->GetSet();
        return LogError(set, vuids.descriptor_valid,
                        "Descriptor set %s encountered the following validation error at %s time: Descriptor in "
                        "binding #%" PRIu32 " index %" PRIu32 " is using sampler %s that is invalid or has been destroyed.",
                        report_data->FormatHandle(set).c_str(), caller, binding_info.first, index,
                        report_data->FormatHandle(sampler).c_str());
    } else {
        if (sampler_state->samplerConversion && !is_immutable) {
            auto set = descriptor_set->GetSet();
            return LogError(set, vuids.descriptor_valid,
                            "Descriptor set %s encountered the following validation error at %s time: sampler (%s) "
                            "in the descriptor set (%s) contains a YCBCR conversion (%s), then the sampler MUST "
                            "also exist as an immutable sampler.",
                            report_data->FormatHandle(set).c_str(), caller, report_data->FormatHandle(sampler).c_str(),
                            report_data->FormatHandle(descriptor_set->GetSet()).c_str(),
                            report_data->FormatHandle(sampler_state->samplerConversion).c_str());
        }
    }
    return false;
}

// Loop through the write updates to do for a push descriptor set, ignoring dstSet
void cvdescriptorset::DescriptorSet::PerformPushDescriptorsUpdate(ValidationStateTracker *dev_data, uint32_t write_count,
                                                                  const VkWriteDescriptorSet *p_wds) {
    assert(IsPushDescriptor());
    for (uint32_t i = 0; i < write_count; i++) {
        PerformWriteUpdate(dev_data, &p_wds[i]);
    }

    push_descriptor_set_writes.clear();
    push_descriptor_set_writes.reserve(static_cast<std::size_t>(write_count));
    for (uint32_t i = 0; i < write_count; i++) {
        push_descriptor_set_writes.push_back(safe_VkWriteDescriptorSet(&p_wds[i]));
    }
}

// Perform write update in given update struct
void cvdescriptorset::DescriptorSet::PerformWriteUpdate(ValidationStateTracker *dev_data, const VkWriteDescriptorSet *update) {
    // Perform update on a per-binding basis as consecutive updates roll over to next binding
    auto descriptors_remaining = update->descriptorCount;
    auto offset = update->dstArrayElement;
    auto orig_binding = DescriptorSetLayout::ConstBindingIterator(layout_.get(), update->dstBinding);
    auto current_binding = orig_binding;

    uint32_t update_index = 0;
    // Verify next consecutive binding matches type, stage flags & immutable sampler use and if AtEnd
    while (descriptors_remaining && orig_binding.IsConsistent(current_binding)) {
        const auto &index_range = current_binding.GetGlobalIndexRange();
        auto global_idx = index_range.start + offset;
        // global_idx is which descriptor is needed to update. If global_idx > index_range.end, it means the descriptor isn't in
        // this binding, maybe in next binding.
        if (global_idx >= index_range.end) {
            offset -= current_binding.GetDescriptorCount();
            ++current_binding;
            continue;
        }

        // Loop over the updates for a single binding at a time
        uint32_t update_count = std::min(descriptors_remaining, current_binding.GetDescriptorCount() - offset);
        for (uint32_t di = 0; di < update_count; ++di, ++update_index) {
            descriptors_[global_idx + di]->WriteUpdate(this, state_data_, update, update_index);
            VkDeviceSize buffer_size = 0;
            if ((update->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
                 update->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
                 update->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
                 update->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) &&
                update->pBufferInfo) {
                const auto buffer_state = dev_data->GetConstCastShared<BUFFER_STATE>(update->pBufferInfo->buffer);
                if (buffer_state) {
                    buffer_size = buffer_state->createInfo.size;
                }
            } else if ((update->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER ||
                        update->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER) &&
                       update->pTexelBufferView) {
                const auto buffer_view = dev_data->GetConstCastShared<BUFFER_VIEW_STATE>(update->pTexelBufferView[di]);
                if (buffer_view) {
                    buffer_size = buffer_view->buffer_state->createInfo.size;
                }
            }
            descriptors_[global_idx + di]->SetDescriptorType(update->descriptorType, buffer_size);
        }
        // Roll over to next binding in case of consecutive update
        descriptors_remaining -= update_count;
        if (descriptors_remaining) {
            // Starting offset is beyond the current binding. Check consistency, update counters and advance to the next binding,
            // looking for the start point. All bindings (even those skipped) must be consistent with the update and with the
            // original binding.
            offset = 0;
            ++current_binding;
        }
    }
    if (update->descriptorCount) {
        some_update_ = true;
        change_count_++;
    }

    if (!IsPushDescriptor() && !(layout_->GetDescriptorBindingFlagsFromBinding(update->dstBinding) &
          (VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT))) {
        Invalidate(false);
    }
}
// Validate Copy update
bool CoreChecks::ValidateCopyUpdate(const VkCopyDescriptorSet *update, const DescriptorSet *dst_set, const DescriptorSet *src_set,
                                    const char *func_name, std::string *error_code, std::string *error_msg) const {
    const auto *dst_layout = dst_set->GetLayout().get();
    const auto *src_layout = src_set->GetLayout().get();

    // Verify dst layout still valid
    if (dst_layout->Destroyed()) {
        *error_code = "VUID-VkCopyDescriptorSet-dstSet-parameter";
        std::ostringstream str;
        str << "Cannot call " << func_name << " to perform copy update on dstSet " << report_data->FormatHandle(dst_set->GetSet())
            << " created with destroyed " << report_data->FormatHandle(dst_layout->GetDescriptorSetLayout()) << ".";
        *error_msg = str.str();
        return false;
    }

    // Verify src layout still valid
    if (src_layout->Destroyed()) {
        *error_code = "VUID-VkCopyDescriptorSet-srcSet-parameter";
        std::ostringstream str;
        str << "Cannot call " << func_name << " to perform copy update on dstSet " << report_data->FormatHandle(dst_set->GetSet())
            << " from srcSet " << report_data->FormatHandle(src_set->GetSet()) << " created with destroyed "
            << report_data->FormatHandle(src_layout->GetDescriptorSetLayout()) << ".";
        *error_msg = str.str();
        return false;
    }

    if (!dst_layout->HasBinding(update->dstBinding)) {
        *error_code = "VUID-VkCopyDescriptorSet-dstBinding-00347";
        std::stringstream error_str;
        error_str << "DescriptorSet " << report_data->FormatHandle(dst_set->GetSet())
                  << " does not have copy update dest binding of " << update->dstBinding;
        *error_msg = error_str.str();
        return false;
    }
    if (!src_set->HasBinding(update->srcBinding)) {
        *error_code = "VUID-VkCopyDescriptorSet-srcBinding-00345";
        std::stringstream error_str;
        error_str << "DescriptorSet " << report_data->FormatHandle(src_set->GetSet())
                  << " does not have copy update src binding of " << update->srcBinding;
        *error_msg = error_str.str();
        return false;
    }
    // Verify idle ds
    if (dst_set->InUse() &&
        !(dst_layout->GetDescriptorBindingFlagsFromBinding(update->dstBinding) &
          (VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT))) {
        *error_code = "VUID-vkUpdateDescriptorSets-None-03047";
        std::stringstream error_str;
        error_str << "Cannot call " << func_name << " to perform copy update on descriptor set "
                  << report_data->FormatHandle(dst_set->GetSet()) << " that is in use by a command buffer";
        *error_msg = error_str.str();
        return false;
    }
    // src & dst set bindings are valid
    // Check bounds of src & dst
    auto src_start_idx = src_set->GetGlobalIndexRangeFromBinding(update->srcBinding).start + update->srcArrayElement;
    if ((src_start_idx + update->descriptorCount) > src_set->GetTotalDescriptorCount()) {
        // SRC update out of bounds
        *error_code = "VUID-VkCopyDescriptorSet-srcArrayElement-00346";
        std::stringstream error_str;
        error_str << "Attempting copy update from descriptorSet " << report_data->FormatHandle(update->srcSet) << " binding#"
                  << update->srcBinding << " with offset index of "
                  << src_set->GetGlobalIndexRangeFromBinding(update->srcBinding).start << " plus update array offset of "
                  << update->srcArrayElement << " and update of " << update->descriptorCount
                  << " descriptors oversteps total number of descriptors in set: " << src_set->GetTotalDescriptorCount();
        *error_msg = error_str.str();
        return false;
    }
    auto dst_start_idx = dst_layout->GetGlobalIndexRangeFromBinding(update->dstBinding).start + update->dstArrayElement;
    if ((dst_start_idx + update->descriptorCount) > dst_layout->GetTotalDescriptorCount()) {
        // DST update out of bounds
        *error_code = "VUID-VkCopyDescriptorSet-dstArrayElement-00348";
        std::stringstream error_str;
        error_str << "Attempting copy update to descriptorSet " << report_data->FormatHandle(dst_set->GetSet()) << " binding#"
                  << update->dstBinding << " with offset index of "
                  << dst_layout->GetGlobalIndexRangeFromBinding(update->dstBinding).start << " plus update array offset of "
                  << update->dstArrayElement << " and update of " << update->descriptorCount
                  << " descriptors oversteps total number of descriptors in set: " << dst_layout->GetTotalDescriptorCount();
        *error_msg = error_str.str();
        return false;
    }
    // Check that types match
    // TODO : Base default error case going from here is "VUID-VkAcquireNextImageInfoKHR-semaphore-parameter" 2ba which covers all
    // consistency issues, need more fine-grained error codes
    *error_code = "VUID-VkCopyDescriptorSet-srcSet-00349";
    auto src_type = src_set->GetTypeFromBinding(update->srcBinding);
    auto dst_type = dst_layout->GetTypeFromBinding(update->dstBinding);
    if (src_type != VK_DESCRIPTOR_TYPE_MUTABLE_VALVE && dst_type != VK_DESCRIPTOR_TYPE_MUTABLE_VALVE && src_type != dst_type) {
        *error_code = "VUID-VkCopyDescriptorSet-dstBinding-02632";
        std::stringstream error_str;
        error_str << "Attempting copy update to descriptorSet " << report_data->FormatHandle(dst_set->GetSet()) << " binding #"
                  << update->dstBinding << " with type " << string_VkDescriptorType(dst_type) << " from descriptorSet "
                  << report_data->FormatHandle(src_set->GetSet()) << " binding #" << update->srcBinding << " with type "
                  << string_VkDescriptorType(src_type) << ". Types do not match";
        *error_msg = error_str.str();
        return false;
    }
    // Verify consistency of src & dst bindings if update crosses binding boundaries
    if ((!VerifyUpdateConsistency(report_data, DescriptorSetLayout::ConstBindingIterator(src_layout, update->srcBinding),
                                  update->srcArrayElement, update->descriptorCount, "copy update from", src_set->GetSet(),
                                  error_msg)) ||
        (!VerifyUpdateConsistency(report_data, DescriptorSetLayout::ConstBindingIterator(dst_layout, update->dstBinding),
                                  update->dstArrayElement, update->descriptorCount, "copy update to", dst_set->GetSet(),
                                  error_msg))) {
        return false;
    }

    if ((src_layout->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT) &&
        !(dst_layout->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)) {
        *error_code = "VUID-VkCopyDescriptorSet-srcSet-01918";
        std::stringstream error_str;
        error_str << "If pname:srcSet's (" << report_data->FormatHandle(update->srcSet)
                  << ") layout was created with the "
                     "ename:VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT flag "
                     "set, then pname:dstSet's ("
                  << report_data->FormatHandle(update->dstSet)
                  << ") layout must: also have been created with the "
                     "ename:VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT flag set";
        *error_msg = error_str.str();
        return false;
    }

    if (IsExtEnabled(device_extensions.vk_valve_mutable_descriptor_type)) {
        if (!(src_layout->GetCreateFlags() & (VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT |
                                              VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_VALVE)) &&
            (dst_layout->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)) {
            *error_code = "VUID-VkCopyDescriptorSet-srcSet-04885";
            std::stringstream error_str;
            error_str << "If pname:srcSet's (" << report_data->FormatHandle(update->srcSet)
                      << ") layout was created with neither ename:VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT nor "
                         "ename:VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_VALVE flags set, then pname:dstSet's ("
                      << report_data->FormatHandle(update->dstSet)
                      << ") layout must: have been created without the "
                         "ename:VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT flag set";
            *error_msg = error_str.str();
            return false;
        }
    } else {
        if (!(src_layout->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT) &&
            (dst_layout->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)) {
            *error_code = "VUID-VkCopyDescriptorSet-srcSet-04886";
            std::stringstream error_str;
            error_str << "If pname:srcSet's (" << report_data->FormatHandle(update->srcSet)
                      << ") layout was created without the ename:VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT flag "
                         "set, then pname:dstSet's ("
                      << report_data->FormatHandle(update->dstSet)
                      << ") layout must: also have been created without the "
                         "ename:VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT flag set";
            *error_msg = error_str.str();
            return false;
        }
    }

    if ((src_set->GetPoolState()->createInfo.flags & VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT) &&
        !(dst_set->GetPoolState()->createInfo.flags & VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)) {
        *error_code = "VUID-VkCopyDescriptorSet-srcSet-01920";
        std::stringstream error_str;
        error_str << "If the descriptor pool from which pname:srcSet (" << report_data->FormatHandle(update->srcSet)
                  << ") was allocated was created "
                     "with the ename:VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT flag "
                     "set, then the descriptor pool from which pname:dstSet ("
                  << report_data->FormatHandle(update->dstSet)
                  << ") was allocated must: "
                     "also have been created with the ename:VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT flag set";
        *error_msg = error_str.str();
        return false;
    }

    if (IsExtEnabled(device_extensions.vk_valve_mutable_descriptor_type)) {
        if (!(src_set->GetPoolState()->createInfo.flags &
              (VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_VALVE)) &&
            (dst_set->GetPoolState()->createInfo.flags & VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)) {
            *error_code = "VUID-VkCopyDescriptorSet-srcSet-04887";
            std::stringstream error_str;
            error_str << "If the descriptor pool from which pname:srcSet (" << report_data->FormatHandle(update->srcSet)
                      << ") was allocated was created with neither ename:VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT nor "
                         "ename:VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_VALVE flags set, then the descriptor pool from which "
                         "pname:dstSet ("
                      << report_data->FormatHandle(update->dstSet)
                      << ") was allocated must: have been created without the "
                         "ename:VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT flag set";
            *error_msg = error_str.str();
            return false;
        }
    } else {
        if (!(src_set->GetPoolState()->createInfo.flags & VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT) &&
            (dst_set->GetPoolState()->createInfo.flags & VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)) {
            *error_code = "VUID-VkCopyDescriptorSet-srcSet-04888";
            std::stringstream error_str;
            error_str << "If the descriptor pool from which pname:srcSet (" << report_data->FormatHandle(update->srcSet)
                      << ") was allocated was created without the ename:VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT flag set, "
                         "then the descriptor pool from which pname:dstSet ("
                      << report_data->FormatHandle(update->dstSet)
                      << ") was allocated must: also have been created without the "
                         "ename:VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT flag set";
            *error_msg = error_str.str();
            return false;
        }
    }

    if (src_type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT) {
        if ((update->srcArrayElement % 4) != 0) {
            *error_code = "VUID-VkCopyDescriptorSet-srcBinding-02223";
            std::stringstream error_str;
            error_str << "Attempting copy update to VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT binding with "
                      << "srcArrayElement " << update->srcArrayElement << " not a multiple of 4";
            *error_msg = error_str.str();
            return false;
        }
        if ((update->dstArrayElement % 4) != 0) {
            *error_code = "VUID-VkCopyDescriptorSet-dstBinding-02224";
            std::stringstream error_str;
            error_str << "Attempting copy update to VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT binding with "
                      << "dstArrayElement " << update->dstArrayElement << " not a multiple of 4";
            *error_msg = error_str.str();
            return false;
        }
        if ((update->descriptorCount % 4) != 0) {
            *error_code = "VUID-VkCopyDescriptorSet-srcBinding-02225";
            std::stringstream error_str;
            error_str << "Attempting copy update to VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT binding with "
                      << "descriptorCount " << update->descriptorCount << " not a multiple of 4";
            *error_msg = error_str.str();
            return false;
        }
    }

    if (dst_type == VK_DESCRIPTOR_TYPE_MUTABLE_VALVE) {
        if (src_type != VK_DESCRIPTOR_TYPE_MUTABLE_VALVE) {
            if (!dst_layout->IsTypeMutable(src_type, update->dstBinding)) {
                *error_code = "VUID-VkCopyDescriptorSet-dstSet-04612";
                std::stringstream error_str;
                error_str << "Attempting copy update with dstBinding descriptor type VK_DESCRIPTOR_TYPE_MUTABLE_VALVE, but the new "
                             "active descriptor type "
                          << string_VkDescriptorType(src_type)
                          << " is not in the corresponding pMutableDescriptorTypeLists list.";
                *error_msg = error_str.str();
                return false;
            }
        }
    } else if (src_type == VK_DESCRIPTOR_TYPE_MUTABLE_VALVE) {
        const auto *descriptor = src_set->GetDescriptorFromGlobalIndex(update->srcBinding);
        if (descriptor->active_descriptor_type != dst_type) {
            *error_code = "VUID-VkCopyDescriptorSet-srcSet-04613";
            std::stringstream error_str;
            error_str << "Attempting copy update with srcBinding descriptor type VK_DESCRIPTOR_TYPE_MUTABLE_VALVE, but the "
                         "active descriptor type ("
                      << string_VkDescriptorType(descriptor->active_descriptor_type)
                      << ") does not match the dstBinding descriptor type " << string_VkDescriptorType(dst_type) << ".";
            *error_msg = error_str.str();
            return false;
        }
    }

    if (dst_type == VK_DESCRIPTOR_TYPE_MUTABLE_VALVE) {
        if (src_type == VK_DESCRIPTOR_TYPE_MUTABLE_VALVE) {
            const auto &mutable_src_types = src_layout->GetMutableTypes(update->srcBinding);
            const auto &mutable_dst_types = dst_layout->GetMutableTypes(update->dstBinding);
            bool complete_match = mutable_src_types.size() == mutable_dst_types.size();
            if (complete_match) {
                for (const auto mutable_src_type : mutable_src_types) {
                    if (std::find(mutable_dst_types.begin(), mutable_dst_types.end(), mutable_src_type) ==
                        mutable_dst_types.end()) {
                        complete_match = false;
                        break;
                    }
                }
            }
            if (!complete_match) {
                *error_code = "VUID-VkCopyDescriptorSet-dstSet-04614";
                std::stringstream error_str;
                error_str << "Attempting copy update with dstBinding and new active descriptor type being "
                             "VK_DESCRIPTOR_TYPE_MUTABLE_VALVE, but their corresponding pMutableDescriptorTypeLists do not match.";
                *error_msg = error_str.str();
                return false;
            }
        }
    }

    // Update mutable types
    if (src_type == VK_DESCRIPTOR_TYPE_MUTABLE_VALVE) {
        src_type = src_set->GetDescriptorFromGlobalIndex(update->srcBinding)->active_descriptor_type;
    }
    if (dst_type == VK_DESCRIPTOR_TYPE_MUTABLE_VALVE) {
        dst_type = dst_set->GetDescriptorFromGlobalIndex(update->dstBinding)->active_descriptor_type;
    }

    // Update parameters all look good and descriptor updated so verify update contents
    if (!VerifyCopyUpdateContents(update, src_set, src_type, src_start_idx, dst_set, dst_type, dst_start_idx, func_name, error_code,
                                  error_msg)) {
        return false;
    }

    // All checks passed so update is good
    return true;
}
// Perform Copy update
void cvdescriptorset::DescriptorSet::PerformCopyUpdate(ValidationStateTracker *dev_data, const VkCopyDescriptorSet *update,
                                                       const DescriptorSet *src_set) {
    auto src_start_idx = src_set->GetGlobalIndexRangeFromBinding(update->srcBinding).start + update->srcArrayElement;
    auto dst_start_idx = layout_->GetGlobalIndexRangeFromBinding(update->dstBinding).start + update->dstArrayElement;
    // Update parameters all look good so perform update
    for (uint32_t di = 0; di < update->descriptorCount; ++di) {
        auto *src = src_set->descriptors_[src_start_idx + di].get();
        auto *dst = descriptors_[dst_start_idx + di].get();
        if (src->updated) {
            dst->CopyUpdate(this, state_data_, src);
            some_update_ = true;
            change_count_++;
        } else {
            dst->updated = false;
        }
        dst->SetDescriptorType(src);
    }

    if (!(layout_->GetDescriptorBindingFlagsFromBinding(update->dstBinding) &
          (VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT))) {
        Invalidate(false);
    }
}

// Update the drawing state for the affected descriptors.
// Set cb_node to this set and this set to cb_node.
// Add the bindings of the descriptor
// Set the layout based on the current descriptor layout (will mask subsequent layer mismatch errors)
// TODO: Modify the UpdateDrawState virtural functions to *only* set initial layout and not change layouts
// Prereq: This should be called for a set that has been confirmed to be active for the given cb_node, meaning it's going
//   to be used in a draw by the given cb_node
void cvdescriptorset::DescriptorSet::UpdateDrawState(ValidationStateTracker *device_data, CMD_BUFFER_STATE *cb_node,
                                                     CMD_TYPE cmd_type, const PIPELINE_STATE *pipe,
                                                     const BindingReqMap &binding_req_map) {
    // Descriptor UpdateDrawState only call image layout validation callbacks. If it is disabled, skip the entire loop.
    if (device_data->disabled[image_layout_validation]) {
        return;
    }

    // For the active slots, use set# to look up descriptorSet from boundDescriptorSets, and bind all of that descriptor set's
    // resources
    CMD_BUFFER_STATE::CmdDrawDispatchInfo cmd_info = {};
    for (const auto &binding_req_pair : binding_req_map) {
        auto index = layout_->GetIndexFromBinding(binding_req_pair.first);

        // We aren't validating descriptors created with PARTIALLY_BOUND or UPDATE_AFTER_BIND, so don't record state
        auto flags = layout_->GetDescriptorBindingFlagsFromIndex(index);
        if (flags & (VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT)) {
            if (!(flags & VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT)) {
                cmd_info.binding_infos.emplace_back(binding_req_pair);
            }
            continue;
        }
        auto range = layout_->GetGlobalIndexRangeFromIndex(index);
        for (uint32_t i = range.start; i < range.end; ++i) {
            const auto descriptor_class = descriptors_[i]->GetClass();
            switch (descriptor_class) {
            case DescriptorClass::Image:
            case DescriptorClass::ImageSampler: {
                auto *image_desc = static_cast<ImageDescriptor *>(descriptors_[i].get());
                image_desc->UpdateDrawState(device_data, cb_node);
                break;
            }
            default:
                break;
            }
        }
    }

    if (cmd_info.binding_infos.size() > 0) {
        cmd_info.cmd_type = cmd_type;
        if (cb_node->activeFramebuffer) {
            cmd_info.framebuffer = cb_node->activeFramebuffer->framebuffer();
            cmd_info.attachments = cb_node->active_attachments;
            cmd_info.subpasses = cb_node->active_subpasses;
        }
        cb_node->validate_descriptorsets_in_queuesubmit[GetSet()].emplace_back(cmd_info);
    }
}

void cvdescriptorset::DescriptorSet::FilterOneBindingReq(const BindingReqMap::value_type &binding_req_pair, BindingReqMap *out_req,
                                                         const TrackedBindings &bindings, uint32_t limit) {
    if (bindings.size() < limit) {
        const auto it = bindings.find(binding_req_pair.first);
        if (it == bindings.cend()) out_req->emplace(binding_req_pair);
    }
}

void cvdescriptorset::DescriptorSet::FilterBindingReqs(const CMD_BUFFER_STATE &cb_state, const PIPELINE_STATE &pipeline,
                                                       const BindingReqMap &in_req, BindingReqMap *out_req) const {
    // For const cleanliness we have to find in the maps...
    const auto validated_it = cb_state.descriptorset_cache.find(this);
    if (validated_it == cb_state.descriptorset_cache.end()) {
        // We have nothing validated, copy in to out
        for (const auto &binding_req_pair : in_req) {
            out_req->emplace(binding_req_pair);
        }
        return;
    }
    const auto &validated = validated_it->second;

    const auto image_sample_version_it = validated.image_samplers.find(&pipeline);
    const VersionedBindings *image_sample_version = nullptr;
    if (image_sample_version_it != validated.image_samplers.cend()) {
        image_sample_version = &(image_sample_version_it->second);
    }
    const auto &dynamic_buffers = validated.dynamic_buffers;
    const auto &non_dynamic_buffers = validated.non_dynamic_buffers;
    const auto &stats = layout_->GetBindingTypeStats();
    for (const auto &binding_req_pair : in_req) {
        auto binding = binding_req_pair.first;
        VkDescriptorSetLayoutBinding const *layout_binding = layout_->GetDescriptorSetLayoutBindingPtrFromBinding(binding);
        if (!layout_binding) {
            continue;
        }
        // Caching criteria differs per type.
        // If image_layout have changed , the image descriptors need to be validated against them.
        if (IsBufferDescriptor(layout_binding->descriptorType)) {
            if (IsDynamicDescriptor(layout_binding->descriptorType)) {
                FilterOneBindingReq(binding_req_pair, out_req, dynamic_buffers, stats.dynamic_buffer_count);
            } else {
                FilterOneBindingReq(binding_req_pair, out_req, non_dynamic_buffers, stats.non_dynamic_buffer_count);
            }
        } else {
            // This is rather crude, as the changed layouts may not impact the bound descriptors,
            // but the simple "versioning" is a simple "dirt" test.
            bool stale = true;
            if (image_sample_version) {
                const auto version_it = image_sample_version->find(binding);
                if (version_it != image_sample_version->cend() && (version_it->second == cb_state.image_layout_change_count)) {
                    stale = false;
                }
            }
            if (stale) {
                out_req->emplace(binding_req_pair);
            }
        }
    }
}

void cvdescriptorset::DescriptorSet::UpdateValidationCache(CMD_BUFFER_STATE &cb_state, const PIPELINE_STATE &pipeline,
                                                           const BindingReqMap &updated_bindings) {
    auto &validated = cb_state.descriptorset_cache[this];

    auto &image_sample_version = validated.image_samplers[&pipeline];
    auto &dynamic_buffers = validated.dynamic_buffers;
    auto &non_dynamic_buffers = validated.non_dynamic_buffers;
    for (const auto &binding_req_pair : updated_bindings) {
        auto binding = binding_req_pair.first;
        VkDescriptorSetLayoutBinding const *layout_binding = layout_->GetDescriptorSetLayoutBindingPtrFromBinding(binding);
        if (!layout_binding) {
            continue;
        }
        // Caching criteria differs per type.
        if (IsBufferDescriptor(layout_binding->descriptorType)) {
            if (IsDynamicDescriptor(layout_binding->descriptorType)) {
                dynamic_buffers.emplace(binding);
            } else {
                non_dynamic_buffers.emplace(binding);
            }
        } else {
            // Save the layout change version...
            image_sample_version[binding] = cb_state.image_layout_change_count;
        }
    }
}

cvdescriptorset::SamplerDescriptor::SamplerDescriptor(const ValidationStateTracker *dev_data, const VkSampler *immut)
    : Descriptor(PlainSampler), immutable_(false) {
    if (immut) {
        sampler_state_ = dev_data->GetConstCastShared<SAMPLER_STATE>(*immut);
        immutable_ = true;
        updated = true;
    }
}
// Validate given sampler. Currently this only checks to make sure it exists in the samplerMap
bool CoreChecks::ValidateSampler(const VkSampler sampler) const { return Get<SAMPLER_STATE>(sampler).get() != nullptr; }

bool CoreChecks::ValidateImageUpdate(VkImageView image_view, VkImageLayout image_layout, VkDescriptorType type,
                                     const char *func_name, std::string *error_code, std::string *error_msg) const {
    auto iv_state = Get<IMAGE_VIEW_STATE>(image_view);
    assert(iv_state);

    // Note that when an imageview is created, we validated that memory is bound so no need to re-check here
    // Validate that imageLayout is compatible with aspect_mask and image format
    //  and validate that image usage bits are correct for given usage
    VkImageAspectFlags aspect_mask = iv_state->normalized_subresource_range.aspectMask;
    VkImage image = iv_state->create_info.image;
    VkFormat format = VK_FORMAT_MAX_ENUM;
    VkImageUsageFlags usage = 0;
    auto *image_node = iv_state->image_state.get();
    assert(image_node);

    format = image_node->createInfo.format;
    const auto image_view_usage_info = LvlFindInChain<VkImageViewUsageCreateInfo>(iv_state->create_info.pNext);
    const auto stencil_usage_info = LvlFindInChain<VkImageStencilUsageCreateInfo>(image_node->createInfo.pNext);
    if (image_view_usage_info) {
        usage = image_view_usage_info->usage;
    } else {
        usage = image_node->createInfo.usage;
    }
    if (stencil_usage_info) {
        bool stencil_aspect = (aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) > 0;
        bool depth_aspect = (aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) > 0;
        if (stencil_aspect && !depth_aspect) {
            usage = stencil_usage_info->stencilUsage;
        } else if (stencil_aspect && depth_aspect) {
            usage &= stencil_usage_info->stencilUsage;
        }
    }

    // Validate that memory is bound to image
    if (ValidateMemoryIsBoundToImage(image_node, func_name, kVUID_Core_Bound_Resource_FreedMemoryAccess)) {
        *error_code = kVUID_Core_Bound_Resource_FreedMemoryAccess;
        *error_msg = "No memory bound to image.";
        return false;
    }

    // KHR_maintenance1 allows rendering into 2D or 2DArray views which slice a 3D image,
    // but not binding them to descriptor sets.
    if (iv_state->IsDepthSliced()) {
        *error_code = "VUID-VkDescriptorImageInfo-imageView-00343";
        *error_msg = "ImageView must not be a 2D or 2DArray view of a 3D image";
        return false;
    }

    // TODO : The various image aspect and format checks here are based on general spec language in 11.5 Image Views section under
    // vkCreateImageView(). What's the best way to create unique id for these cases?
    *error_code = kVUID_Core_DrawState_InvalidImageView;
    bool ds = FormatIsDepthOrStencil(format);
    switch (image_layout) {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            // Only Color bit must be set
            if ((aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) != VK_IMAGE_ASPECT_COLOR_BIT) {
                std::stringstream error_str;
                error_str
                    << "ImageView (" << report_data->FormatHandle(image_view)
                    << ") uses layout VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL but does not have VK_IMAGE_ASPECT_COLOR_BIT set.";
                *error_msg = error_str.str();
                return false;
            }
            // format must NOT be DS
            if (ds) {
                std::stringstream error_str;
                error_str << "ImageView (" << report_data->FormatHandle(image_view)
                          << ") uses layout VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL but the image format is "
                          << string_VkFormat(format) << " which is not a color format.";
                *error_msg = error_str.str();
                return false;
            }
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
            // Depth or stencil bit must be set, but both must NOT be set
            if (aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) {
                if (aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) {
                    // both  must NOT be set
                    std::stringstream error_str;
                    error_str << "ImageView (" << report_data->FormatHandle(image_view)
                              << ") has both STENCIL and DEPTH aspects set";
                    *error_msg = error_str.str();
                    return false;
                }
            } else if (!(aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT)) {
                // Neither were set
                std::stringstream error_str;
                error_str << "ImageView (" << report_data->FormatHandle(image_view) << ") has layout "
                          << string_VkImageLayout(image_layout) << " but does not have STENCIL or DEPTH aspects set";
                *error_msg = error_str.str();
                return false;
            }
            // format must be DS
            if (!ds) {
                std::stringstream error_str;
                error_str << "ImageView (" << report_data->FormatHandle(image_view) << ") has layout "
                          << string_VkImageLayout(image_layout) << " but the image format is " << string_VkFormat(format)
                          << " which is not a depth/stencil format.";
                *error_msg = error_str.str();
                return false;
            }
            break;
        default:
            // For other layouts if the source is depth/stencil image, both aspect bits must not be set
            if (ds) {
                if (aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) {
                    if (aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) {
                        // both  must NOT be set
                        std::stringstream error_str;
                        error_str << "ImageView (" << report_data->FormatHandle(image_view) << ") has layout "
                                  << string_VkImageLayout(image_layout) << " and is using depth/stencil image of format "
                                  << string_VkFormat(format)
                                  << " but it has both STENCIL and DEPTH aspects set, which is illegal. When using a depth/stencil "
                                     "image in a descriptor set, please only set either VK_IMAGE_ASPECT_DEPTH_BIT or "
                                     "VK_IMAGE_ASPECT_STENCIL_BIT depending on whether it will be used for depth reads or stencil "
                                     "reads respectively.";
                        *error_code = "VUID-VkDescriptorImageInfo-imageView-01976";
                        *error_msg = error_str.str();
                        return false;
                    }
                }
            }
            break;
    }
    // Now validate that usage flags are correctly set for given type of update
    //  As we're switching per-type, if any type has specific layout requirements, check those here as well
    // TODO : The various image usage bit requirements are in general spec language for VkImageUsageFlags bit block in 11.3 Images
    // under vkCreateImage()
    const char *error_usage_bit = nullptr;
    switch (type) {
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            if (iv_state->samplerConversion != VK_NULL_HANDLE) {
                *error_code = "VUID-VkWriteDescriptorSet-descriptorType-01946";
                std::stringstream error_str;
                error_str << "ImageView (" << report_data->FormatHandle(image_view) << ")"
                          << "used as a VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE can't be created with VkSamplerYcbcrConversion";
                *error_msg = error_str.str();
                return false;
            }
            // drop through
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
            if (!(usage & VK_IMAGE_USAGE_SAMPLED_BIT)) {
                error_usage_bit = "VK_IMAGE_USAGE_SAMPLED_BIT";
                *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00337";
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: {
            if (!(usage & VK_IMAGE_USAGE_STORAGE_BIT)) {
                error_usage_bit = "VK_IMAGE_USAGE_STORAGE_BIT";
                *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00339";
            } else if ((VK_IMAGE_LAYOUT_GENERAL != image_layout) &&
                       (!IsExtEnabled(device_extensions.vk_khr_shared_presentable_image) ||
                        (VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR != image_layout))) {
                *error_code = "VUID-VkWriteDescriptorSet-descriptorType-04152";
                std::stringstream error_str;
                error_str << "Descriptor update with descriptorType VK_DESCRIPTOR_TYPE_STORAGE_IMAGE"
                          << " is being updated with invalid imageLayout " << string_VkImageLayout(image_layout) << " for image "
                          << report_data->FormatHandle(image) << " in imageView " << report_data->FormatHandle(image_view)
                          << ". Allowed layouts are: VK_IMAGE_LAYOUT_GENERAL";
                if (IsExtEnabled(device_extensions.vk_khr_shared_presentable_image)) {
                    error_str << " or VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR";
                }
                *error_msg = error_str.str();
                return false;
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: {
            if (!(usage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) {
                error_usage_bit = "VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT";
                *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00338";
            }
            break;
        }
        default:
            break;
    }
    if (error_usage_bit) {
        std::stringstream error_str;
        error_str << "ImageView (" << report_data->FormatHandle(image_view) << ") with usage mask " << std::hex << std::showbase
                  << usage << " being used for a descriptor update of type " << string_VkDescriptorType(type) << " does not have "
                  << error_usage_bit << " set.";
        *error_msg = error_str.str();
        return false;
    }

    // All the following types share the same image layouts
    // checkf or Storage Images above
    if ((type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) || (type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) ||
        (type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)) {
        // Test that the layout is compatible with the descriptorType for the two sampled image types
        const static std::array<VkImageLayout, 3> valid_layouts = {
            {VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL}};

        struct ExtensionLayout {
            VkImageLayout layout;
            ExtEnabled DeviceExtensions::*extension;
        };
        const static std::array<ExtensionLayout, 7> extended_layouts{{
            //  Note double brace req'd for aggregate initialization
            {VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, &DeviceExtensions::vk_khr_shared_presentable_image},
            {VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, &DeviceExtensions::vk_khr_maintenance2},
            {VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, &DeviceExtensions::vk_khr_maintenance2},
            {VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR, &DeviceExtensions::vk_khr_synchronization2},
            {VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR, &DeviceExtensions::vk_khr_synchronization2},
            {VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL, &DeviceExtensions::vk_khr_separate_depth_stencil_layouts},
            {VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL, &DeviceExtensions::vk_khr_separate_depth_stencil_layouts},
        }};
        auto is_layout = [image_layout, this](const ExtensionLayout &ext_layout) {
            return IsExtEnabled(device_extensions.*(ext_layout.extension)) && (ext_layout.layout == image_layout);
        };

        bool valid_layout = (std::find(valid_layouts.cbegin(), valid_layouts.cend(), image_layout) != valid_layouts.cend()) ||
                            std::any_of(extended_layouts.cbegin(), extended_layouts.cend(), is_layout);

        if (!valid_layout) {
            // The following works as currently all 3 descriptor types share the same set of valid layouts
            switch (type) {
                case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                    *error_code = "VUID-VkWriteDescriptorSet-descriptorType-04149";
                    break;
                case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                    *error_code = "VUID-VkWriteDescriptorSet-descriptorType-04150";
                    break;
                case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                    *error_code = "VUID-VkWriteDescriptorSet-descriptorType-04151";
                    break;
                default:
                    break;
            }
            std::stringstream error_str;
            error_str << "Descriptor update with descriptorType " << string_VkDescriptorType(type)
                      << " is being updated with invalid imageLayout " << string_VkImageLayout(image_layout) << " for image "
                      << report_data->FormatHandle(image) << " in imageView " << report_data->FormatHandle(image_view)
                      << ". Allowed layouts are: VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, "
                      << "VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL";
            for (auto &ext_layout : extended_layouts) {
                if (IsExtEnabled(device_extensions.*(ext_layout.extension))) {
                    error_str << ", " << string_VkImageLayout(ext_layout.layout);
                }
            }
            *error_msg = error_str.str();
            return false;
        }
    }

    if ((type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) || (type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)) {
        const VkComponentMapping components = iv_state->create_info.components;
        if (IsIdentitySwizzle(components) == false) {
            *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00336";
            std::stringstream error_str;
            error_str << "ImageView (" << report_data->FormatHandle(image_view) << ") has a non-identiy swizzle component, "
                      << " r swizzle = " << string_VkComponentSwizzle(components.r) << ","
                      << " g swizzle = " << string_VkComponentSwizzle(components.g) << ","
                      << " b swizzle = " << string_VkComponentSwizzle(components.b) << ","
                      << " a swizzle = " << string_VkComponentSwizzle(components.a) << ".";
            *error_msg = error_str.str();
            return false;
        }
    }

    if ((type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) && (iv_state->min_lod != 0.0f)) {
        *error_code = "VUID-VkWriteDescriptorSet-descriptorType-06450";
        std::stringstream error_str;
        error_str << "ImageView (" << report_data->FormatHandle(image_view)
                  << ") , written to a descriptor of type VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT with a minLod (" << iv_state->min_lod
                  << ") that is not 0.0";
        *error_msg = error_str.str();
        return false;
    }

    return true;
}

// Helper template to change shared pointer members of a Descriptor, while
// correctly managing links to the parent DescriptorSet.
// src and dst are shared pointers.
template <typename T>
static void ReplaceStatePtr(DescriptorSet *set_state, T &dst, const T &src) {
    if (dst) {
        dst->RemoveParent(set_state);
    }
    dst = src;
    if (dst) {
        dst->AddParent(set_state);
    }
}

void cvdescriptorset::SamplerDescriptor::WriteUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data,
                                                     const VkWriteDescriptorSet *update, const uint32_t index) {
    if (!immutable_) {
        ReplaceStatePtr(set_state, sampler_state_ , dev_data->GetConstCastShared<SAMPLER_STATE>(update->pImageInfo[index].sampler));
    }
    updated = true;
}

void cvdescriptorset::SamplerDescriptor::CopyUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data,
                                                    const Descriptor *src) {
    updated = true;
    if (src->descriptor_class == Mutable) {
        auto *sampler_src = static_cast<const MutableDescriptor *>(src);
        if (!immutable_) {
            ReplaceStatePtr(set_state, sampler_state_, sampler_src->GetSharedSamplerState());
        }
        return;
    }
    auto *sampler_src = static_cast<const SamplerDescriptor *>(src);
    if (!immutable_) {
        ReplaceStatePtr(set_state, sampler_state_, sampler_src->sampler_state_);
    }
}

cvdescriptorset::ImageSamplerDescriptor::ImageSamplerDescriptor(const ValidationStateTracker *dev_data, const VkSampler *immut)
    : ImageDescriptor(ImageSampler), immutable_(false) {
    if (immut) {
        sampler_state_ = dev_data->GetConstCastShared<SAMPLER_STATE>(*immut);
        immutable_ = true;
    }
}

void cvdescriptorset::ImageSamplerDescriptor::WriteUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data,
                                                          const VkWriteDescriptorSet *update, const uint32_t index) {
    updated = true;
    const auto &image_info = update->pImageInfo[index];
    if (!immutable_) {
        ReplaceStatePtr(set_state, sampler_state_, dev_data->GetConstCastShared<SAMPLER_STATE>(image_info.sampler));
    }
    image_layout_ = image_info.imageLayout;
    ReplaceStatePtr(set_state, image_view_state_, dev_data->GetConstCastShared<IMAGE_VIEW_STATE>(image_info.imageView));
}

void cvdescriptorset::ImageSamplerDescriptor::CopyUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data,
                                                         const Descriptor *src) {
    updated = true;
    if (src->descriptor_class == Mutable) {
        auto *image_src = static_cast<const MutableDescriptor *>(src);
        if (!immutable_) {
            ReplaceStatePtr(set_state, sampler_state_, image_src->GetSharedSamplerState());
        }
        ImageDescriptor::CopyUpdate(set_state, dev_data, src);
        return;
    }
    auto *image_src = static_cast<const ImageSamplerDescriptor *>(src);
    if (!immutable_) {
        ReplaceStatePtr(set_state, sampler_state_, image_src->sampler_state_);
    }
    ImageDescriptor::CopyUpdate(set_state, dev_data, src);
}

cvdescriptorset::ImageDescriptor::ImageDescriptor(const VkDescriptorType type)
    : Descriptor(Image), image_layout_(VK_IMAGE_LAYOUT_UNDEFINED) {}

cvdescriptorset::ImageDescriptor::ImageDescriptor(DescriptorClass class_)
    : Descriptor(class_), image_layout_(VK_IMAGE_LAYOUT_UNDEFINED) {}

void cvdescriptorset::ImageDescriptor::WriteUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data,
                                                   const VkWriteDescriptorSet *update, const uint32_t index) {
    updated = true;
    const auto &image_info = update->pImageInfo[index];
    image_layout_ = image_info.imageLayout;
    ReplaceStatePtr(set_state, image_view_state_, dev_data->GetConstCastShared<IMAGE_VIEW_STATE>(image_info.imageView));
}

void cvdescriptorset::ImageDescriptor::CopyUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data,
                                                  const Descriptor *src) {
    updated = true;
    if (src->descriptor_class == Mutable) {
        auto *image_src = static_cast<const MutableDescriptor *>(src);

        image_layout_ = image_src->GetImageLayout();
        ReplaceStatePtr(set_state, image_view_state_, image_src->GetSharedImageViewState());
        return;
    }
    auto *image_src = static_cast<const ImageDescriptor *>(src);

    image_layout_ = image_src->image_layout_;
    ReplaceStatePtr(set_state, image_view_state_, image_src->image_view_state_);
}

void cvdescriptorset::ImageDescriptor::UpdateDrawState(ValidationStateTracker *dev_data, CMD_BUFFER_STATE *cb_node) {
    // Add binding for image
    auto iv_state = GetImageViewState();
    if (iv_state) {
        dev_data->CallSetImageViewInitialLayoutCallback(cb_node, *iv_state, image_layout_);
    }
}

cvdescriptorset::BufferDescriptor::BufferDescriptor(const VkDescriptorType type)
    : Descriptor(GeneralBuffer), offset_(0), range_(0) {}

void cvdescriptorset::BufferDescriptor::WriteUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data,
                                                    const VkWriteDescriptorSet *update, const uint32_t index) {
    updated = true;
    const auto &buffer_info = update->pBufferInfo[index];
    offset_ = buffer_info.offset;
    range_ = buffer_info.range;
    ReplaceStatePtr(set_state, buffer_state_, dev_data->GetConstCastShared<BUFFER_STATE>(buffer_info.buffer));
}

void cvdescriptorset::BufferDescriptor::CopyUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data,
                                                   const Descriptor *src) {
    updated = true;
    if (src->descriptor_class == Mutable) {
        const auto buff_desc = static_cast<const MutableDescriptor *>(src);
        offset_ = buff_desc->GetOffset();
        range_ = buff_desc->GetRange();
        ReplaceStatePtr(set_state, buffer_state_, buff_desc->GetSharedBufferState());
        return;
    }
    const auto buff_desc = static_cast<const BufferDescriptor *>(src);
    offset_ = buff_desc->offset_;
    range_ = buff_desc->range_;
    ReplaceStatePtr(set_state, buffer_state_, buff_desc->buffer_state_);
}

cvdescriptorset::TexelDescriptor::TexelDescriptor(const VkDescriptorType type) : Descriptor(TexelBuffer) {}

void cvdescriptorset::TexelDescriptor::WriteUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data,
                                                   const VkWriteDescriptorSet *update, const uint32_t index) {
    updated = true;
    ReplaceStatePtr(set_state, buffer_view_state_,
                    dev_data->GetConstCastShared<BUFFER_VIEW_STATE>(update->pTexelBufferView[index]));
}

void cvdescriptorset::TexelDescriptor::CopyUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data,
                                                  const Descriptor *src) {
    updated = true;
    if (src->descriptor_class == Mutable) {
        ReplaceStatePtr(set_state, buffer_view_state_, static_cast<const MutableDescriptor *>(src)->GetSharedBufferViewState());
        return;
    }
    ReplaceStatePtr(set_state, buffer_view_state_, static_cast<const TexelDescriptor *>(src)->buffer_view_state_);
}

cvdescriptorset::AccelerationStructureDescriptor::AccelerationStructureDescriptor(const VkDescriptorType type)
    : Descriptor(AccelerationStructure), acc_(VK_NULL_HANDLE), acc_nv_(VK_NULL_HANDLE) {
    is_khr_ = false;
}
void cvdescriptorset::AccelerationStructureDescriptor::WriteUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data,
                                                                   const VkWriteDescriptorSet *update, const uint32_t index) {
    const auto *acc_info = LvlFindInChain<VkWriteDescriptorSetAccelerationStructureKHR>(update->pNext);
    const auto *acc_info_nv = LvlFindInChain<VkWriteDescriptorSetAccelerationStructureNV>(update->pNext);
    assert(acc_info || acc_info_nv);
    is_khr_ = (acc_info != NULL);
    updated = true;
    if (is_khr_) {
        acc_ = acc_info->pAccelerationStructures[index];
        ReplaceStatePtr(set_state, acc_state_, dev_data->GetConstCastShared<ACCELERATION_STRUCTURE_STATE_KHR>(acc_));
    } else {
        acc_nv_ = acc_info_nv->pAccelerationStructures[index];
        ReplaceStatePtr(set_state, acc_state_nv_, dev_data->GetConstCastShared<ACCELERATION_STRUCTURE_STATE>(acc_nv_));
    }
}

void cvdescriptorset::AccelerationStructureDescriptor::CopyUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data,
                                                                  const Descriptor *src) {
    updated = true;
    if (src->descriptor_class == Mutable) {
        auto acc_desc = static_cast<const MutableDescriptor *>(src);
        if (is_khr_) {
            acc_ = acc_desc->GetAccelerationStructure();
            ReplaceStatePtr(set_state, acc_state_, dev_data->GetConstCastShared<ACCELERATION_STRUCTURE_STATE_KHR>(acc_));
        } else {
            acc_nv_ = acc_desc->GetAccelerationStructureNV();
            ReplaceStatePtr(set_state, acc_state_nv_, dev_data->GetConstCastShared<ACCELERATION_STRUCTURE_STATE>(acc_nv_));
        }
        return;
    }
    auto acc_desc = static_cast<const AccelerationStructureDescriptor *>(src);
    if (is_khr_) {
        acc_ = acc_desc->acc_;
        ReplaceStatePtr(set_state, acc_state_, dev_data->GetConstCastShared<ACCELERATION_STRUCTURE_STATE_KHR>(acc_));
    } else {
        acc_nv_ = acc_desc->acc_nv_;
        ReplaceStatePtr(set_state, acc_state_nv_, dev_data->GetConstCastShared<ACCELERATION_STRUCTURE_STATE>(acc_nv_));
    }
}

cvdescriptorset::MutableDescriptor::MutableDescriptor()
    : Descriptor(Mutable),
      buffer_size_(0),
      immutable_(false),
      image_layout_(VK_IMAGE_LAYOUT_UNDEFINED),
      offset_(0),
      range_(0),
      is_khr_(false),
      acc_(VK_NULL_HANDLE),
      acc_nv_(VK_NULL_HANDLE) {
    active_descriptor_class_ = NoDescriptorClass;
}

void cvdescriptorset::MutableDescriptor::WriteUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data,
                                                     const VkWriteDescriptorSet *update, const uint32_t index) {
    updated = true;
    if (update->descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER) {
        if (!immutable_) {
            ReplaceStatePtr(set_state, sampler_state_,
                            dev_data->GetConstCastShared<SAMPLER_STATE>(update->pImageInfo[index].sampler));
        }
    } else if (update->descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
        const auto &image_info = update->pImageInfo[index];
        if (!immutable_) {
            ReplaceStatePtr(set_state, sampler_state_, dev_data->GetConstCastShared<SAMPLER_STATE>(image_info.sampler));
        }
        image_layout_ = image_info.imageLayout;
        ReplaceStatePtr(set_state, image_view_state_, dev_data->GetConstCastShared<IMAGE_VIEW_STATE>(image_info.imageView));
    } else if (update->descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
               update->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ||
               update->descriptorType == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
        const auto &image_info = update->pImageInfo[index];
        image_layout_ = image_info.imageLayout;
        ReplaceStatePtr(set_state, image_view_state_, dev_data->GetConstCastShared<IMAGE_VIEW_STATE>(image_info.imageView));
    } else if (update->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
               update->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
               update->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
               update->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) {
        const auto &buffer_info = update->pBufferInfo[index];
        offset_ = buffer_info.offset;
        range_ = buffer_info.range;
        ReplaceStatePtr(set_state, buffer_state_, dev_data->GetConstCastShared<BUFFER_STATE>(buffer_info.buffer));
    } else if (update->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER ||
               update->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER) {
        ReplaceStatePtr(set_state, buffer_view_state_,
                        dev_data->GetConstCastShared<BUFFER_VIEW_STATE>(update->pTexelBufferView[index]));
    } else if (update->descriptorType == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR) {
        const auto *acc_info = LvlFindInChain<VkWriteDescriptorSetAccelerationStructureKHR>(update->pNext);
        const auto *acc_info_nv = LvlFindInChain<VkWriteDescriptorSetAccelerationStructureNV>(update->pNext);
        assert(acc_info || acc_info_nv);
        is_khr_ = (acc_info != NULL);
        updated = true;
        if (is_khr_) {
            acc_ = acc_info->pAccelerationStructures[index];
            ReplaceStatePtr(set_state, acc_state_, dev_data->GetConstCastShared<ACCELERATION_STRUCTURE_STATE_KHR>(acc_));
        } else {
            acc_nv_ = acc_info_nv->pAccelerationStructures[index];
            ReplaceStatePtr(set_state, acc_state_nv_, dev_data->GetConstCastShared<ACCELERATION_STRUCTURE_STATE>(acc_nv_));
        }
    }
}

void cvdescriptorset::MutableDescriptor::CopyUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data,
                                                    const Descriptor *src) {
    updated = true;
    if (src->descriptor_class == DescriptorClass::PlainSampler) {
        auto *sampler_src = static_cast<const SamplerDescriptor *>(src);
        if (!immutable_) {
            ReplaceStatePtr(set_state, sampler_state_, sampler_src->GetSharedSamplerState());
        }
    } else if (src->descriptor_class == DescriptorClass::ImageSampler) {
        auto *image_src = static_cast<const ImageSamplerDescriptor *>(src);
        if (!immutable_) {
            ReplaceStatePtr(set_state, sampler_state_, image_src->GetSharedSamplerState());
        }

        image_layout_ = image_src->GetImageLayout();
        ReplaceStatePtr(set_state, image_view_state_, image_src->GetSharedImageViewState());
    } else if (src->descriptor_class == DescriptorClass::Image) {
        auto *image_src = static_cast<const ImageDescriptor *>(src);

        image_layout_ = image_src->GetImageLayout();
        ReplaceStatePtr(set_state, image_view_state_, image_src->GetSharedImageViewState());
    } else if (src->descriptor_class == DescriptorClass::TexelBuffer) {
        ReplaceStatePtr(set_state, buffer_view_state_, static_cast<const TexelDescriptor *>(src)->GetSharedBufferViewState());
    } else if (src->descriptor_class == DescriptorClass::GeneralBuffer) {
        const auto buff_desc = static_cast<const BufferDescriptor *>(src);
        offset_ = buff_desc->GetOffset();
        range_ = buff_desc->GetRange();
        ReplaceStatePtr(set_state, buffer_state_, buff_desc->GetSharedBufferState());
    } else if (src->descriptor_class == DescriptorClass::AccelerationStructure) {
        auto acc_desc = static_cast<const AccelerationStructureDescriptor *>(src);
        if (is_khr_) {
            acc_ = acc_desc->GetAccelerationStructure();
            ReplaceStatePtr(set_state, acc_state_, dev_data->GetConstCastShared<ACCELERATION_STRUCTURE_STATE_KHR>(acc_));
        } else {
            acc_nv_ = acc_desc->GetAccelerationStructureNV();
            ReplaceStatePtr(set_state, acc_state_nv_, dev_data->GetConstCastShared<ACCELERATION_STRUCTURE_STATE>(acc_nv_));
        }
    } else if (src->descriptor_class == DescriptorClass::Mutable) {
        if (src->active_descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLER) {
            auto *sampler_src = static_cast<const MutableDescriptor *>(src);
            if (!immutable_) {
                ReplaceStatePtr(set_state, sampler_state_, sampler_src->GetSharedSamplerState());
            }
        } else if (src->active_descriptor_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
            auto *image_src = static_cast<const MutableDescriptor *>(src);
            if (!immutable_) {
                ReplaceStatePtr(set_state, sampler_state_, image_src->GetSharedSamplerState());
            }

            image_layout_ = image_src->GetImageLayout();
            ReplaceStatePtr(set_state, image_view_state_, image_src->GetSharedImageViewState());
        } else if (src->active_descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
                   src->active_descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ||
                   src->active_descriptor_type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
            auto *image_src = static_cast<const MutableDescriptor *>(src);

            image_layout_ = image_src->GetImageLayout();
            ReplaceStatePtr(set_state, image_view_state_, image_src->GetSharedImageViewState());
        } else if (src->active_descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
                   src->active_descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
                   src->active_descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
                   src->active_descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) {
            const auto buff_desc = static_cast<const MutableDescriptor *>(src);
            offset_ = buff_desc->GetOffset();
            range_ = buff_desc->GetRange();
            ReplaceStatePtr(set_state, buffer_state_, buff_desc->GetSharedBufferState());
        } else if (src->active_descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER ||
                   src->active_descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER) {
            ReplaceStatePtr(set_state, buffer_view_state_, static_cast<const MutableDescriptor *>(src)->GetSharedBufferViewState());
        } else if (src->active_descriptor_type == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR ||
                   src->active_descriptor_type == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV) {
            auto acc_desc = static_cast<const MutableDescriptor *>(src);
            if (is_khr_) {
                acc_ = acc_desc->GetAccelerationStructure();
                ReplaceStatePtr(set_state, acc_state_, dev_data->GetConstCastShared<ACCELERATION_STRUCTURE_STATE_KHR>(acc_));
            } else {
                acc_nv_ = acc_desc->GetAccelerationStructureNV();
                ReplaceStatePtr(set_state, acc_state_nv_, dev_data->GetConstCastShared<ACCELERATION_STRUCTURE_STATE>(acc_nv_));
            }
        }
    }
}

bool cvdescriptorset::MutableDescriptor::AddParent(BASE_NODE *base_node) {
    bool result = false;
    if (active_descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLER) {
        if (sampler_state_) {
            result |= sampler_state_->AddParent(base_node);
        }
    } else if (active_descriptor_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
        if (sampler_state_) {
            result |= sampler_state_->AddParent(base_node);
        }
        if (image_view_state_) {
            result = image_view_state_->AddParent(base_node);
        }
    } else if (active_descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER ||
               active_descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER) {
        if (buffer_view_state_) {
            result = buffer_view_state_->AddParent(base_node);
        }
    } else if (active_descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
               active_descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ||
               active_descriptor_type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
        if (image_view_state_) {
            result = image_view_state_->AddParent(base_node);
        }
    } else if (active_descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
               active_descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
               active_descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
               active_descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) {
        if (buffer_state_) {
            result = buffer_state_->AddParent(base_node);
        }
    } else if (active_descriptor_type == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR ||
               active_descriptor_type == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV) {
        if (acc_state_) {
            result |= acc_state_->AddParent(base_node);
        }
        if (acc_state_nv_) {
            result |= acc_state_nv_->AddParent(base_node);
        }
    }
    return result;
}
void cvdescriptorset::MutableDescriptor::RemoveParent(BASE_NODE *base_node) {
    if (sampler_state_) {
        sampler_state_->RemoveParent(base_node);
    }
    if (image_view_state_) {
        image_view_state_->RemoveParent(base_node);
    }
    if (buffer_view_state_) {
        buffer_view_state_->RemoveParent(base_node);
    }
    if (buffer_state_) {
        buffer_state_->RemoveParent(base_node);
    }
    if (acc_state_) {
        acc_state_->RemoveParent(base_node);
    }
    if (acc_state_nv_) {
        acc_state_nv_->RemoveParent(base_node);
    }
}

bool cvdescriptorset::MutableDescriptor::Invalid() const {
    switch (active_descriptor_type) {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
            return !sampler_state_ || sampler_state_->Destroyed();

        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            return !sampler_state_ || sampler_state_->Invalid() || !image_view_state_ || image_view_state_->Invalid();

        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            return !buffer_view_state_ || buffer_view_state_->Invalid();

        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            return !image_view_state_ || image_view_state_->Invalid();

        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            return !buffer_state_ || buffer_state_->Invalid();

        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
            return !acc_state_ || acc_state_->Invalid();

        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
            return !acc_state_nv_ || acc_state_nv_->Invalid();
        default:
            return false;
    }
}

// This is a helper function that iterates over a set of Write and Copy updates, pulls the DescriptorSet* for updated
//  sets, and then calls their respective Validate[Write|Copy]Update functions.
// If the update hits an issue for which the callback returns "true", meaning that the call down the chain should
//  be skipped, then true is returned.
// If there is no issue with the update, then false is returned.
bool CoreChecks::ValidateUpdateDescriptorSets(uint32_t write_count, const VkWriteDescriptorSet *p_wds, uint32_t copy_count,
                                              const VkCopyDescriptorSet *p_cds, const char *func_name) const {
    bool skip = false;
    // Validate Write updates
    for (uint32_t i = 0; i < write_count; i++) {
        auto dest_set = p_wds[i].dstSet;
        auto set_node = Get<cvdescriptorset::DescriptorSet>(dest_set);
        if (!set_node) {
            skip |= LogError(dest_set, kVUID_Core_DrawState_InvalidDescriptorSet,
                             "Cannot call %s on %s that has not been allocated in pDescriptorWrites[%u].", func_name,
                             report_data->FormatHandle(dest_set).c_str(), i);
        } else {
            std::string error_code;
            std::string error_str;
            if (!ValidateWriteUpdate(set_node.get(), &p_wds[i], func_name, &error_code, &error_str, false)) {
                skip |=
                    LogError(dest_set, error_code, "%s pDescriptorWrites[%u] failed write update validation for %s with error: %s.",
                             func_name, i, report_data->FormatHandle(dest_set).c_str(), error_str.c_str());
            }
        }
        if (p_wds[i].pNext) {
            const auto *pnext_struct = LvlFindInChain<VkWriteDescriptorSetAccelerationStructureKHR>(p_wds[i].pNext);
            if (pnext_struct) {
                for (uint32_t j = 0; j < pnext_struct->accelerationStructureCount; ++j) {
                    auto as_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pnext_struct->pAccelerationStructures[j]);
                    if (as_state && (as_state->create_infoKHR.sType == VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR &&
                                     (as_state->create_infoKHR.type != VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR &&
                                      as_state->create_infoKHR.type != VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR))) {
                        skip |=
                            LogError(dest_set, "VUID-VkWriteDescriptorSetAccelerationStructureKHR-pAccelerationStructures-03579",
                                     "%s: For pDescriptorWrites[%u] acceleration structure in pAccelerationStructures[%u] must "
                                     "have been created with "
                                     "VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR or VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR.",
                                     func_name, i, j);
                    }
                }
            }
            const auto *pnext_struct_nv = LvlFindInChain<VkWriteDescriptorSetAccelerationStructureNV>(p_wds[i].pNext);
            if (pnext_struct_nv) {
                for (uint32_t j = 0; j < pnext_struct_nv->accelerationStructureCount; ++j) {
                    auto as_state = Get<ACCELERATION_STRUCTURE_STATE>(pnext_struct_nv->pAccelerationStructures[j]);
                    if (as_state && (as_state->create_infoNV.sType == VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV &&
                                     as_state->create_infoNV.info.type != VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV)) {
                        skip |= LogError(dest_set, "VUID-VkWriteDescriptorSetAccelerationStructureNV-pAccelerationStructures-03748",
                                         "%s: For pDescriptorWrites[%u] acceleration structure in pAccelerationStructures[%u] must "
                                         "have been created with"
                                         " VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV.",
                                         func_name, i, j);
                    }
                }
            }
        }
    }
    // Now validate copy updates
    for (uint32_t i = 0; i < copy_count; ++i) {
        auto dst_set = p_cds[i].dstSet;
        auto src_set = p_cds[i].srcSet;
        auto src_node = Get<cvdescriptorset::DescriptorSet>(src_set);
        auto dst_node = Get<cvdescriptorset::DescriptorSet>(dst_set);
        // Object_tracker verifies that src & dest descriptor set are valid
        assert(src_node);
        assert(dst_node);
        std::string error_code;
        std::string error_str;
        if (!ValidateCopyUpdate(&p_cds[i], dst_node.get(), src_node.get(), func_name, &error_code, &error_str)) {
            LogObjectList objlist(dst_set);
            objlist.add(src_set);
            skip |= LogError(objlist, error_code, "%s pDescriptorCopies[%u] failed copy update from %s to %s with error: %s.",
                             func_name, i, report_data->FormatHandle(src_set).c_str(), report_data->FormatHandle(dst_set).c_str(),
                             error_str.c_str());
        }
    }
    return skip;
}
// This is a helper function that iterates over a set of Write and Copy updates, pulls the DescriptorSet* for updated
//  sets, and then calls their respective Perform[Write|Copy]Update functions.
// Prerequisite : ValidateUpdateDescriptorSets() should be called and return "false" prior to calling PerformUpdateDescriptorSets()
//  with the same set of updates.
// This is split from the validate code to allow validation prior to calling down the chain, and then update after
//  calling down the chain.
void cvdescriptorset::PerformUpdateDescriptorSets(ValidationStateTracker *dev_data, uint32_t write_count,
                                                  const VkWriteDescriptorSet *p_wds, uint32_t copy_count,
                                                  const VkCopyDescriptorSet *p_cds) {
    // Write updates first
    uint32_t i = 0;
    for (i = 0; i < write_count; ++i) {
        auto dest_set = p_wds[i].dstSet;
        auto set_node = dev_data->Get<cvdescriptorset::DescriptorSet>(dest_set);
        if (set_node) {
            set_node->PerformWriteUpdate(dev_data, &p_wds[i]);
        }
    }
    // Now copy updates
    for (i = 0; i < copy_count; ++i) {
        auto dst_set = p_cds[i].dstSet;
        auto src_set = p_cds[i].srcSet;
        auto src_node = dev_data->Get<cvdescriptorset::DescriptorSet>(src_set);
        auto dst_node = dev_data->Get<cvdescriptorset::DescriptorSet>(dst_set);
        if (src_node && dst_node) {
            dst_node->PerformCopyUpdate(dev_data, &p_cds[i], src_node.get());
        }
    }
}

cvdescriptorset::DecodedTemplateUpdate::DecodedTemplateUpdate(const ValidationStateTracker *device_data,
                                                              VkDescriptorSet descriptorSet,
                                                              const UPDATE_TEMPLATE_STATE *template_state, const void *pData,
                                                              VkDescriptorSetLayout push_layout) {
    auto const &create_info = template_state->create_info;
    inline_infos.resize(create_info.descriptorUpdateEntryCount);  // Make sure we have one if we need it
    inline_infos_khr.resize(create_info.descriptorUpdateEntryCount);
    inline_infos_nv.resize(create_info.descriptorUpdateEntryCount);
    desc_writes.reserve(create_info.descriptorUpdateEntryCount);  // emplaced, so reserved without initialization
    VkDescriptorSetLayout effective_dsl = create_info.templateType == VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET
                                              ? create_info.descriptorSetLayout
                                              : push_layout;
    auto layout_obj = device_data->Get<cvdescriptorset::DescriptorSetLayout>(effective_dsl);

    // Create a WriteDescriptorSet struct for each template update entry
    for (uint32_t i = 0; i < create_info.descriptorUpdateEntryCount; i++) {
        auto binding_count = layout_obj->GetDescriptorCountFromBinding(create_info.pDescriptorUpdateEntries[i].dstBinding);
        auto binding_being_updated = create_info.pDescriptorUpdateEntries[i].dstBinding;
        auto dst_array_element = create_info.pDescriptorUpdateEntries[i].dstArrayElement;

        desc_writes.reserve(desc_writes.size() + create_info.pDescriptorUpdateEntries[i].descriptorCount);
        for (uint32_t j = 0; j < create_info.pDescriptorUpdateEntries[i].descriptorCount; j++) {
            desc_writes.emplace_back();
            auto &write_entry = desc_writes.back();

            size_t offset = create_info.pDescriptorUpdateEntries[i].offset + j * create_info.pDescriptorUpdateEntries[i].stride;
            char *update_entry = (char *)(pData) + offset;

            if (dst_array_element >= binding_count) {
                dst_array_element = 0;
                binding_being_updated = layout_obj->GetNextValidBinding(binding_being_updated);
            }

            write_entry.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_entry.pNext = NULL;
            write_entry.dstSet = descriptorSet;
            write_entry.dstBinding = binding_being_updated;
            write_entry.dstArrayElement = dst_array_element;
            write_entry.descriptorCount = 1;
            write_entry.descriptorType = create_info.pDescriptorUpdateEntries[i].descriptorType;

            switch (create_info.pDescriptorUpdateEntries[i].descriptorType) {
                case VK_DESCRIPTOR_TYPE_SAMPLER:
                case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                    write_entry.pImageInfo = reinterpret_cast<VkDescriptorImageInfo *>(update_entry);
                    break;

                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                    write_entry.pBufferInfo = reinterpret_cast<VkDescriptorBufferInfo *>(update_entry);
                    break;

                case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                    write_entry.pTexelBufferView = reinterpret_cast<VkBufferView *>(update_entry);
                    break;
                case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT: {
                    VkWriteDescriptorSetInlineUniformBlockEXT *inline_info = &inline_infos[i];
                    inline_info->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_INLINE_UNIFORM_BLOCK_EXT;
                    inline_info->pNext = nullptr;
                    inline_info->dataSize = create_info.pDescriptorUpdateEntries[i].descriptorCount;
                    inline_info->pData = update_entry;
                    write_entry.pNext = inline_info;
                    // descriptorCount must match the dataSize member of the VkWriteDescriptorSetInlineUniformBlockEXT structure
                    write_entry.descriptorCount = inline_info->dataSize;
                    // skip the rest of the array, they just represent bytes in the update
                    j = create_info.pDescriptorUpdateEntries[i].descriptorCount;
                    break;
                }
                case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: {
                    VkWriteDescriptorSetAccelerationStructureKHR *inline_info_khr = &inline_infos_khr[i];
                    inline_info_khr->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
                    inline_info_khr->pNext = nullptr;
                    inline_info_khr->accelerationStructureCount = create_info.pDescriptorUpdateEntries[i].descriptorCount;
                    inline_info_khr->pAccelerationStructures = reinterpret_cast<VkAccelerationStructureKHR *>(update_entry);
                    write_entry.pNext = inline_info_khr;
                    break;
                }
                case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV: {
                    VkWriteDescriptorSetAccelerationStructureNV *inline_info_nv = &inline_infos_nv[i];
                    inline_info_nv->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV;
                    inline_info_nv->pNext = nullptr;
                    inline_info_nv->accelerationStructureCount = create_info.pDescriptorUpdateEntries[i].descriptorCount;
                    inline_info_nv->pAccelerationStructures = reinterpret_cast<VkAccelerationStructureNV *>(update_entry);
                    write_entry.pNext = inline_info_nv;
                    break;
                }
                default:
                    assert(0);
                    break;
            }
            dst_array_element++;
        }
    }
}
// These helper functions carry out the validate and record descriptor updates peformed via update templates. They decode
// the templatized data and leverage the non-template UpdateDescriptor helper functions.
bool CoreChecks::ValidateUpdateDescriptorSetsWithTemplateKHR(VkDescriptorSet descriptorSet,
                                                             const UPDATE_TEMPLATE_STATE *template_state, const void *pData) const {
    // Translate the templated update into a normal update for validation...
    cvdescriptorset::DecodedTemplateUpdate decoded_update(this, descriptorSet, template_state, pData);
    return ValidateUpdateDescriptorSets(static_cast<uint32_t>(decoded_update.desc_writes.size()), decoded_update.desc_writes.data(),
                                        0, NULL, "vkUpdateDescriptorSetWithTemplate()");
}

std::string cvdescriptorset::DescriptorSet::StringifySetAndLayout() const {
    std::string out;
    auto layout_handle = layout_->GetDescriptorSetLayout();
    if (IsPushDescriptor()) {
        std::ostringstream str;
        str << "Push Descriptors defined with " << state_data_->report_data->FormatHandle(layout_handle);
        out = str.str();
    } else {
        std::ostringstream str;
        str << state_data_->report_data->FormatHandle(GetSet()) << " allocated with "
            << state_data_->report_data->FormatHandle(layout_handle);
        out = str.str();
    }
    return out;
};

// Loop through the write updates to validate for a push descriptor set, ignoring dstSet
bool CoreChecks::ValidatePushDescriptorsUpdate(const DescriptorSet *push_set, uint32_t write_count,
                                               const VkWriteDescriptorSet *p_wds, const char *func_name) const {
    assert(push_set->IsPushDescriptor());
    bool skip = false;
    for (uint32_t i = 0; i < write_count; i++) {
        std::string error_code;
        std::string error_str;
        if (!ValidateWriteUpdate(push_set, &p_wds[i], func_name, &error_code, &error_str, true)) {
            skip |= LogError(push_set->GetDescriptorSetLayout(), error_code,
                             "%s VkWriteDescriptorSet[%u] failed update validation: %s.", func_name, i, error_str.c_str());
        }
    }
    return skip;
}

// For the given buffer, verify that its creation parameters are appropriate for the given type
//  If there's an error, update the error_msg string with details and return false, else return true
bool cvdescriptorset::ValidateBufferUsage(debug_report_data *report_data, BUFFER_STATE const *buffer_node, VkDescriptorType type,
                                          std::string *error_code, std::string *error_msg) {
    // Verify that usage bits set correctly for given type
    auto usage = buffer_node->createInfo.usage;
    const char *error_usage_bit = nullptr;
    switch (type) {
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            if (!(usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT)) {
                *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00334";
                error_usage_bit = "VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT";
            }
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            if (!(usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT)) {
                *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00335";
                error_usage_bit = "VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT";
            }
            break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            if (!(usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)) {
                *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00330";
                error_usage_bit = "VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT";
            }
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            if (!(usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)) {
                *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00331";
                error_usage_bit = "VK_BUFFER_USAGE_STORAGE_BUFFER_BIT";
            }
            break;
        default:
            break;
    }
    if (error_usage_bit) {
        std::stringstream error_str;
        error_str << "Buffer (" << report_data->FormatHandle(buffer_node->buffer()) << ") with usage mask " << std::hex
                  << std::showbase << usage << " being used for a descriptor update of type " << string_VkDescriptorType(type)
                  << " does not have " << error_usage_bit << " set.";
        *error_msg = error_str.str();
        return false;
    }
    return true;
}
// For buffer descriptor updates, verify the buffer usage and VkDescriptorBufferInfo struct which includes:
//  1. buffer is valid
//  2. buffer was created with correct usage flags
//  3. offset is less than buffer size
//  4. range is either VK_WHOLE_SIZE or falls in (0, (buffer size - offset)]
//  5. range and offset are within the device's limits
// If there's an error, update the error_msg string with details and return false, else return true
bool CoreChecks::ValidateBufferUpdate(VkDescriptorBufferInfo const *buffer_info, VkDescriptorType type, const char *func_name,
                                      std::string *error_code, std::string *error_msg) const {
    // First make sure that buffer is valid
    auto buffer_node = Get<BUFFER_STATE>(buffer_info->buffer);
    // Any invalid buffer should already be caught by object_tracker
    assert(buffer_node);
    if (ValidateMemoryIsBoundToBuffer(buffer_node.get(), func_name, "VUID-VkWriteDescriptorSet-descriptorType-00329")) {
        *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00329";
        *error_msg = "No memory bound to buffer.";
        return false;
    }
    // Verify usage bits
    if (!cvdescriptorset::ValidateBufferUsage(report_data, buffer_node.get(), type, error_code, error_msg)) {
        // error_msg will have been updated by ValidateBufferUsage()
        return false;
    }
    // offset must be less than buffer size
    if (buffer_info->offset >= buffer_node->createInfo.size) {
        *error_code = "VUID-VkDescriptorBufferInfo-offset-00340";
        std::stringstream error_str;
        error_str << "VkDescriptorBufferInfo offset of " << buffer_info->offset << " is greater than or equal to buffer "
                  << report_data->FormatHandle(buffer_node->buffer()) << " size of " << buffer_node->createInfo.size;
        *error_msg = error_str.str();
        return false;
    }
    if (buffer_info->range != VK_WHOLE_SIZE) {
        // Range must be VK_WHOLE_SIZE or > 0
        if (!buffer_info->range) {
            *error_code = "VUID-VkDescriptorBufferInfo-range-00341";
            std::stringstream error_str;
            error_str << "For buffer " << report_data->FormatHandle(buffer_node->buffer())
                      << " VkDescriptorBufferInfo range is not VK_WHOLE_SIZE and is zero, which is not allowed.";
            *error_msg = error_str.str();
            return false;
        }
        // Range must be VK_WHOLE_SIZE or <= (buffer size - offset)
        if (buffer_info->range > (buffer_node->createInfo.size - buffer_info->offset)) {
            *error_code = "VUID-VkDescriptorBufferInfo-range-00342";
            std::stringstream error_str;
            error_str << "For buffer " << report_data->FormatHandle(buffer_node->buffer()) << " VkDescriptorBufferInfo range is "
                      << buffer_info->range << " which is greater than buffer size (" << buffer_node->createInfo.size
                      << ") minus requested offset of " << buffer_info->offset;
            *error_msg = error_str.str();
            return false;
        }
    }
    // Check buffer update sizes against device limits
    const auto &limits = phys_dev_props.limits;
    if (VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER == type || VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC == type) {
        auto max_ub_range = limits.maxUniformBufferRange;
        if (buffer_info->range != VK_WHOLE_SIZE && buffer_info->range > max_ub_range) {
            *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00332";
            std::stringstream error_str;
            error_str << "For buffer " << report_data->FormatHandle(buffer_node->buffer()) << " VkDescriptorBufferInfo range is "
                      << buffer_info->range << " which is greater than this device's maxUniformBufferRange (" << max_ub_range
                      << ")";
            *error_msg = error_str.str();
            return false;
        } else if (buffer_info->range == VK_WHOLE_SIZE && (buffer_node->createInfo.size - buffer_info->offset) > max_ub_range) {
            *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00332";
            std::stringstream error_str;
            error_str << "For buffer " << report_data->FormatHandle(buffer_node->buffer())
                      << " VkDescriptorBufferInfo range is VK_WHOLE_SIZE but effective range "
                      << "(" << (buffer_node->createInfo.size - buffer_info->offset) << ") is greater than this device's "
                      << "maxUniformBufferRange (" << max_ub_range << ")";
            *error_msg = error_str.str();
            return false;
        }
    } else if (VK_DESCRIPTOR_TYPE_STORAGE_BUFFER == type || VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC == type) {
        auto max_sb_range = limits.maxStorageBufferRange;
        if (buffer_info->range != VK_WHOLE_SIZE && buffer_info->range > max_sb_range) {
            *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00333";
            std::stringstream error_str;
            error_str << "For buffer " << report_data->FormatHandle(buffer_node->buffer()) << " VkDescriptorBufferInfo range is "
                      << buffer_info->range << " which is greater than this device's maxStorageBufferRange (" << max_sb_range
                      << ")";
            *error_msg = error_str.str();
            return false;
        } else if (buffer_info->range == VK_WHOLE_SIZE && (buffer_node->createInfo.size - buffer_info->offset) > max_sb_range) {
            *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00333";
            std::stringstream error_str;
            error_str << "For buffer " << report_data->FormatHandle(buffer_node->buffer())
                      << " VkDescriptorBufferInfo range is VK_WHOLE_SIZE but effective range "
                      << "(" << (buffer_node->createInfo.size - buffer_info->offset) << ") is greater than this device's "
                      << "maxStorageBufferRange (" << max_sb_range << ")";
            *error_msg = error_str.str();
            return false;
        }
    }
    return true;
}
template <typename T>
bool CoreChecks::ValidateAccelerationStructureUpdate(T acc_node, const char *func_name, std::string *error_code,
                                                     std::string *error_msg) const {
    // nullDescriptor feature allows this to be VK_NULL_HANDLE
    if (acc_node) {
        if (ValidateMemoryIsBoundToAccelerationStructure(acc_node, func_name, kVUIDUndefined)) {
            *error_code = kVUIDUndefined;
            *error_msg = "No memory bound to acceleration structure.";
            return false;
        }
    }
    return true;
}

// Verify that the contents of the update are ok, but don't perform actual update
bool CoreChecks::VerifyCopyUpdateContents(const VkCopyDescriptorSet *update, const DescriptorSet *src_set,
                                          VkDescriptorType src_type, uint32_t src_index, const DescriptorSet *dst_set,
                                          VkDescriptorType dst_type, uint32_t dst_index, const char *func_name,
                                          std::string *error_code, std::string *error_msg) const {
    // Note : Repurposing some Write update error codes here as specific details aren't called out for copy updates like they are
    // for write updates
    using DescriptorClass = cvdescriptorset::DescriptorClass;
    using BufferDescriptor = cvdescriptorset::BufferDescriptor;
    using ImageDescriptor = cvdescriptorset::ImageDescriptor;
    using ImageSamplerDescriptor = cvdescriptorset::ImageSamplerDescriptor;
    using SamplerDescriptor = cvdescriptorset::SamplerDescriptor;
    using TexelDescriptor = cvdescriptorset::TexelDescriptor;

    auto device_data = this;

    if (dst_type == VK_DESCRIPTOR_TYPE_SAMPLER) {
        for (uint32_t di = 0; di < update->descriptorCount; ++di) {
            const auto dst_desc = dst_set->GetDescriptorFromGlobalIndex(dst_index + di);
            if (!dst_desc->updated) continue;
            if (dst_desc->IsImmutableSampler()) {
                *error_code = "VUID-VkCopyDescriptorSet-dstBinding-02753";
                std::stringstream error_str;
                error_str << "Attempted copy update to an immutable sampler descriptor.";
                *error_msg = error_str.str();
                return false;
            }
        }
    }

    switch (src_set->GetDescriptorFromGlobalIndex(src_index)->descriptor_class) {
        case DescriptorClass::PlainSampler: {
            for (uint32_t di = 0; di < update->descriptorCount; ++di) {
                const auto src_desc = src_set->GetDescriptorFromGlobalIndex(src_index + di);
                if (!src_desc->updated) continue;
                if (!src_desc->IsImmutableSampler()) {
                    auto update_sampler = static_cast<const SamplerDescriptor *>(src_desc)->GetSampler();
                    if (!ValidateSampler(update_sampler)) {
                        *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00325";
                        std::stringstream error_str;
                        error_str << "Attempted copy update to sampler descriptor with invalid sampler: "
                                  << report_data->FormatHandle(update_sampler) << ".";
                        *error_msg = error_str.str();
                        return false;
                    }
                } else {
                    // TODO : Warn here
                }
            }
            break;
        }
        case DescriptorClass::ImageSampler: {
            for (uint32_t di = 0; di < update->descriptorCount; ++di) {
                const auto src_desc = src_set->GetDescriptorFromGlobalIndex(src_index + di);
                if (!src_desc->updated) continue;
                auto img_samp_desc = static_cast<const ImageSamplerDescriptor *>(src_desc);
                // First validate sampler
                if (!img_samp_desc->IsImmutableSampler()) {
                    auto update_sampler = img_samp_desc->GetSampler();
                    if (!ValidateSampler(update_sampler)) {
                        *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00325";
                        std::stringstream error_str;
                        error_str << "Attempted copy update to sampler descriptor with invalid sampler: "
                                  << report_data->FormatHandle(update_sampler) << ".";
                        *error_msg = error_str.str();
                        return false;
                    }
                } else {
                    // TODO : Warn here
                }
                // Validate image
                auto image_view = img_samp_desc->GetImageView();
                auto image_layout = img_samp_desc->GetImageLayout();
                if (image_view) {
                    if (!ValidateImageUpdate(image_view, image_layout, src_type, func_name, error_code, error_msg)) {
                        std::stringstream error_str;
                        error_str << "Attempted copy update to combined image sampler descriptor failed due to: "
                                  << error_msg->c_str();
                        *error_msg = error_str.str();
                        return false;
                    }
                }
            }
            break;
        }
        case DescriptorClass::Image: {
            for (uint32_t di = 0; di < update->descriptorCount; ++di) {
                const auto src_desc = src_set->GetDescriptorFromGlobalIndex(src_index + di);
                if (!src_desc->updated) continue;
                auto img_desc = static_cast<const ImageDescriptor *>(src_desc);
                auto image_view = img_desc->GetImageView();
                auto image_layout = img_desc->GetImageLayout();
                if (image_view) {
                    if (!ValidateImageUpdate(image_view, image_layout, src_type, func_name, error_code, error_msg)) {
                        std::stringstream error_str;
                        error_str << "Attempted copy update to image descriptor failed due to: " << error_msg->c_str();
                        *error_msg = error_str.str();
                        return false;
                    }
                }
            }
            break;
        }
        case DescriptorClass::TexelBuffer: {
            for (uint32_t di = 0; di < update->descriptorCount; ++di) {
                const auto src_desc = src_set->GetDescriptorFromGlobalIndex(src_index + di);
                if (!src_desc->updated) continue;
                auto buffer_view = static_cast<const TexelDescriptor *>(src_desc)->GetBufferView();
                if (buffer_view) {
                    auto bv_state = device_data->Get<BUFFER_VIEW_STATE>(buffer_view);
                    if (!bv_state) {
                        *error_code = "VUID-VkWriteDescriptorSet-descriptorType-02994";
                        std::stringstream error_str;
                        error_str << "Attempted copy update to texel buffer descriptor with invalid buffer view: "
                                  << report_data->FormatHandle(buffer_view);
                        *error_msg = error_str.str();
                        return false;
                    }
                    auto buffer = bv_state->create_info.buffer;
                    auto buffer_state = Get<BUFFER_STATE>(buffer);
                    if (!cvdescriptorset::ValidateBufferUsage(report_data, buffer_state.get(), src_type, error_code, error_msg)) {
                        std::stringstream error_str;
                        error_str << "Attempted copy update to texel buffer descriptor failed due to: " << error_msg->c_str();
                        *error_msg = error_str.str();
                        return false;
                    }
                }
            }
            break;
        }
        case DescriptorClass::GeneralBuffer: {
            for (uint32_t di = 0; di < update->descriptorCount; ++di) {
                const auto src_desc = src_set->GetDescriptorFromGlobalIndex(src_index + di);
                if (!src_desc->updated) continue;
                auto buffer_state = static_cast<const BufferDescriptor *>(src_desc)->GetBufferState();
                if (buffer_state) {
                    if (!cvdescriptorset::ValidateBufferUsage(report_data, buffer_state, src_type, error_code, error_msg)) {
                        std::stringstream error_str;
                        error_str << "Attempted copy update to buffer descriptor failed due to: " << error_msg->c_str();
                        *error_msg = error_str.str();
                        return false;
                    }
                }
            }
            break;
        }
        case DescriptorClass::InlineUniform:
        case DescriptorClass::AccelerationStructure:
        case DescriptorClass::Mutable:
            break;
        default:
            assert(0);  // We've already verified update type so should never get here
            break;
    }
    // All checks passed so update contents are good
    return true;
}
// Verify that the state at allocate time is correct, but don't actually allocate the sets yet
bool CoreChecks::ValidateAllocateDescriptorSets(const VkDescriptorSetAllocateInfo *p_alloc_info,
                                                const cvdescriptorset::AllocateDescriptorSetsData *ds_data) const {
    bool skip = false;
    auto pool_state = Get<DESCRIPTOR_POOL_STATE>(p_alloc_info->descriptorPool);

    for (uint32_t i = 0; i < p_alloc_info->descriptorSetCount; i++) {
        auto layout = Get<cvdescriptorset::DescriptorSetLayout>(p_alloc_info->pSetLayouts[i]);
        if (layout) {  // nullptr layout indicates no valid layout handle for this device, validated/logged in object_tracker
            if (layout->IsPushDescriptor()) {
                skip |= LogError(p_alloc_info->pSetLayouts[i], "VUID-VkDescriptorSetAllocateInfo-pSetLayouts-00308",
                                 "%s specified at pSetLayouts[%" PRIu32
                                 "] in vkAllocateDescriptorSets() was created with invalid flag %s set.",
                                 report_data->FormatHandle(p_alloc_info->pSetLayouts[i]).c_str(), i,
                                 "VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR");
            }
            if (layout->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT &&
                !(pool_state->createInfo.flags & VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)) {
                skip |= LogError(
                    device, "VUID-VkDescriptorSetAllocateInfo-pSetLayouts-03044",
                    "vkAllocateDescriptorSets(): Descriptor set layout create flags and pool create flags mismatch for index (%d)",
                    i);
            }
            if (layout->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_VALVE &&
                !(pool_state->createInfo.flags & VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_VALVE)) {
                skip |= LogError(device, "VUID-VkDescriptorSetAllocateInfo-pSetLayouts-04610",
                                 "vkAllocateDescriptorSets(): pSetLayouts[%d].flags contain "
                                 "VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_VALVE bit, but the pool was not created "
                                 "with the VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_VALVE bit.",
                                 i);
            }
        }
    }
    if (!IsExtEnabled(device_extensions.vk_khr_maintenance1)) {
        // Track number of descriptorSets allowable in this pool
        if (pool_state->GetAvailableSets() < p_alloc_info->descriptorSetCount) {
            skip |= LogError(pool_state->Handle(), "VUID-VkDescriptorSetAllocateInfo-descriptorSetCount-00306",
                             "vkAllocateDescriptorSets(): Unable to allocate %u descriptorSets from %s"
                             ". This pool only has %d descriptorSets remaining.",
                             p_alloc_info->descriptorSetCount, report_data->FormatHandle(pool_state->Handle()).c_str(),
                             pool_state->GetAvailableSets());
        }
        // Determine whether descriptor counts are satisfiable
        for (auto it = ds_data->required_descriptors_by_type.begin(); it != ds_data->required_descriptors_by_type.end(); ++it) {
            auto available_count = pool_state->GetAvailableCount(it->first);

            if (ds_data->required_descriptors_by_type.at(it->first) > available_count) {
                skip |= LogError(pool_state->Handle(), "VUID-VkDescriptorSetAllocateInfo-descriptorPool-00307",
                                 "vkAllocateDescriptorSets(): Unable to allocate %u descriptors of type %s from %s"
                                 ". This pool only has %d descriptors of this type remaining.",
                                 ds_data->required_descriptors_by_type.at(it->first),
                                 string_VkDescriptorType(VkDescriptorType(it->first)),
                                 report_data->FormatHandle(pool_state->Handle()).c_str(), available_count);
            }
        }
    }

    const auto *count_allocate_info = LvlFindInChain<VkDescriptorSetVariableDescriptorCountAllocateInfo>(p_alloc_info->pNext);

    if (count_allocate_info) {
        if (count_allocate_info->descriptorSetCount != 0 &&
            count_allocate_info->descriptorSetCount != p_alloc_info->descriptorSetCount) {
            skip |= LogError(device, "VUID-VkDescriptorSetVariableDescriptorCountAllocateInfo-descriptorSetCount-03045",
                             "vkAllocateDescriptorSets(): VkDescriptorSetAllocateInfo::descriptorSetCount (%d) != "
                             "VkDescriptorSetVariableDescriptorCountAllocateInfo::descriptorSetCount (%d)",
                             p_alloc_info->descriptorSetCount, count_allocate_info->descriptorSetCount);
        }
        if (count_allocate_info->descriptorSetCount == p_alloc_info->descriptorSetCount) {
            for (uint32_t i = 0; i < p_alloc_info->descriptorSetCount; i++) {
                auto layout = Get<cvdescriptorset::DescriptorSetLayout>(p_alloc_info->pSetLayouts[i]);
                if (count_allocate_info->pDescriptorCounts[i] > layout->GetDescriptorCountFromBinding(layout->GetMaxBinding())) {
                    skip |= LogError(device, "VUID-VkDescriptorSetVariableDescriptorCountAllocateInfo-pSetLayouts-03046",
                                     "vkAllocateDescriptorSets(): pDescriptorCounts[%d] = (%d), binding's descriptorCount = (%d)",
                                     i, count_allocate_info->pDescriptorCounts[i],
                                     layout->GetDescriptorCountFromBinding(layout->GetMaxBinding()));
                }
            }
        }
    }

    return skip;
}

const BindingReqMap &cvdescriptorset::PrefilterBindRequestMap::FilteredMap(const CMD_BUFFER_STATE &cb_state,
                                                                           const PIPELINE_STATE &pipeline) {
    if (IsManyDescriptors()) {
        filtered_map_.reset(new BindingReqMap);
        descriptor_set_.FilterBindingReqs(cb_state, pipeline, orig_map_, filtered_map_.get());
        return *filtered_map_;
    }
    return orig_map_;
}

// Starting at offset descriptor of given binding, parse over update_count
//  descriptor updates and verify that for any binding boundaries that are crossed, the next binding(s) are all consistent
//  Consistency means that their type, stage flags, and whether or not they use immutable samplers matches
//  If so, return true. If not, fill in error_msg and return false
bool cvdescriptorset::VerifyUpdateConsistency(debug_report_data *report_data,
                                              DescriptorSetLayout::ConstBindingIterator current_binding, uint32_t offset,
                                              uint32_t update_count, const char *type, const VkDescriptorSet set,
                                              std::string *error_msg) {
    bool pass = true;
    // Verify consecutive bindings match (if needed)
    auto orig_binding = current_binding;

    while (pass && update_count) {
        // First, it's legal to offset beyond your own binding so handle that case
        if (offset > 0) {
            const auto &index_range = current_binding.GetGlobalIndexRange();
            // index_range.start + offset is which descriptor is needed to update. If it > index_range.end, it means the descriptor
            // isn't in this binding, maybe in next binding.
            if ((index_range.start + offset) >= index_range.end) {
                // Advance to next binding, decrement offset by binding size
                offset -= current_binding.GetDescriptorCount();
                ++current_binding;
                // Verify next consecutive binding matches type, stage flags & immutable sampler use and if AtEnd
                if (!orig_binding.IsConsistent(current_binding)) {
                    pass = false;
                }
                continue;
            }
        }

        update_count -= std::min(update_count, current_binding.GetDescriptorCount() - offset);
        if (update_count) {
            // Starting offset is beyond the current binding. Check consistency, update counters and advance to the next binding,
            // looking for the start point. All bindings (even those skipped) must be consistent with the update and with the
            // original binding.
            offset = 0;
            ++current_binding;
            // Verify next consecutive binding matches type, stage flags & immutable sampler use and if AtEnd
            if (!orig_binding.IsConsistent(current_binding)) {
                pass = false;
            }
        }
    }

    if (!pass) {
        std::stringstream error_str;
        error_str << "Attempting " << type;
        if (current_binding.Layout()->IsPushDescriptor()) {
            error_str << " push descriptors";
        } else {
            error_str << " descriptor set " << report_data->FormatHandle(set);
        }
        error_str << " binding #" << orig_binding.Binding() << " with #" << update_count
                  << " descriptors being updated but this update oversteps the bounds of this binding and the next binding is "
                     "not consistent with current binding";

        // Get what was not consistent in IsConsistent() as a more detailed error message
        const auto *binding_ci = orig_binding.GetDescriptorSetLayoutBindingPtr();
        const auto *other_binding_ci = current_binding.GetDescriptorSetLayoutBindingPtr();
        if (binding_ci == nullptr || other_binding_ci == nullptr) {
            error_str << " (No two valid DescriptorSetLayoutBinding to compare)";
        } else if (binding_ci->descriptorType != other_binding_ci->descriptorType) {
            error_str << " (" << string_VkDescriptorType(binding_ci->descriptorType)
                      << " != " << string_VkDescriptorType(other_binding_ci->descriptorType) << ")";
        } else if (binding_ci->stageFlags != other_binding_ci->stageFlags) {
            error_str << " (" << string_VkShaderStageFlags(binding_ci->stageFlags)
                      << " != " << string_VkShaderStageFlags(other_binding_ci->stageFlags) << ")";
        } else if (!hash_util::similar_for_nullity(binding_ci->pImmutableSamplers, other_binding_ci->pImmutableSamplers)) {
            error_str << " (pImmutableSamplers don't match)";
        } else if (orig_binding.GetDescriptorBindingFlags() != current_binding.GetDescriptorBindingFlags()) {
            error_str << " (" << string_VkDescriptorBindingFlags(orig_binding.GetDescriptorBindingFlags())
                      << " != " << string_VkDescriptorBindingFlags(current_binding.GetDescriptorBindingFlags()) << ")";
        }

        error_str << " so this update is invalid";
        *error_msg = error_str.str();
    }
    return pass;
}

// Validate the state for a given write update but don't actually perform the update
//  If an error would occur for this update, return false and fill in details in error_msg string
bool CoreChecks::ValidateWriteUpdate(const DescriptorSet *dest_set, const VkWriteDescriptorSet *update, const char *func_name,
                                     std::string *error_code, std::string *error_msg, bool push) const {
    const auto *dest_layout = dest_set->GetLayout().get();

    // Verify dst layout still valid
    if (dest_layout->Destroyed()) {
        *error_code = "VUID-VkWriteDescriptorSet-dstSet-00320";
        std::ostringstream str;
        str << "Cannot call " << func_name << " to perform write update on " << dest_set->StringifySetAndLayout()
            << " which has been destroyed";
        *error_msg = str.str();
        return false;
    }
    // Verify dst binding exists
    if (!dest_layout->HasBinding(update->dstBinding)) {
        *error_code = "VUID-VkWriteDescriptorSet-dstBinding-00315";
        std::stringstream error_str;
        error_str << dest_set->StringifySetAndLayout() << " does not have binding " << update->dstBinding;
        *error_msg = error_str.str();
        return false;
    }

    DescriptorSetLayout::ConstBindingIterator dest(dest_layout, update->dstBinding);
    // Make sure binding isn't empty
    if (0 == dest.GetDescriptorCount()) {
        *error_code = "VUID-VkWriteDescriptorSet-dstBinding-00316";
        std::stringstream error_str;
        error_str << dest_set->StringifySetAndLayout() << " cannot updated binding " << update->dstBinding
                  << " that has 0 descriptors";
        *error_msg = error_str.str();
        return false;
    }

    // Verify idle ds
    if (dest_set->InUse() && !(dest.GetDescriptorBindingFlags() & (VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT |
                                                                         VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT))) {
        *error_code = "VUID-vkUpdateDescriptorSets-None-03047";
        std::stringstream error_str;
        error_str << "Cannot call " << func_name << " to perform write update on " << dest_set->StringifySetAndLayout()
                  << " that is in use by a command buffer";
        *error_msg = error_str.str();
        return false;
    }
    // We know that binding is valid, verify update and do update on each descriptor
    auto start_idx = dest.GetGlobalIndexRange().start + update->dstArrayElement;
    auto type = dest.GetType();
    if ((type != VK_DESCRIPTOR_TYPE_MUTABLE_VALVE) && (type != update->descriptorType)) {
        *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00319";
        std::stringstream error_str;
        error_str << "Attempting write update to " << dest_set->StringifySetAndLayout() << " binding #" << update->dstBinding
                  << " with type " << string_VkDescriptorType(type) << " but update type is "
                  << string_VkDescriptorType(update->descriptorType);
        *error_msg = error_str.str();
        return false;
    }
    if (type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT) {
        if ((update->dstArrayElement % 4) != 0) {
            *error_code = "VUID-VkWriteDescriptorSet-descriptorType-02219";
            std::stringstream error_str;
            error_str << "Attempting write update to " << dest_set->StringifySetAndLayout() << " binding #" << update->dstBinding
                      << " with "
                      << "dstArrayElement " << update->dstArrayElement << " not a multiple of 4";
            *error_msg = error_str.str();
            return false;
        }
        if ((update->descriptorCount % 4) != 0) {
            *error_code = "VUID-VkWriteDescriptorSet-descriptorType-02220";
            std::stringstream error_str;
            error_str << "Attempting write update to " << dest_set->StringifySetAndLayout() << " binding #" << update->dstBinding
                      << " with "
                      << "descriptorCount  " << update->descriptorCount << " not a multiple of 4";
            *error_msg = error_str.str();
            return false;
        }
        const auto *write_inline_info = LvlFindInChain<VkWriteDescriptorSetInlineUniformBlockEXT>(update->pNext);
        if (!write_inline_info || write_inline_info->dataSize != update->descriptorCount) {
            *error_code = "VUID-VkWriteDescriptorSet-descriptorType-02221";
            std::stringstream error_str;
            if (!write_inline_info) {
                error_str << "Attempting write update to " << dest_set->StringifySetAndLayout() << " binding #"
                          << update->dstBinding << " with "
                          << "VkWriteDescriptorSetInlineUniformBlock missing";
            } else {
                error_str << "Attempting write update to " << dest_set->StringifySetAndLayout() << " binding #"
                          << update->dstBinding << " with "
                          << "VkWriteDescriptorSetInlineUniformBlock dataSize " << write_inline_info->dataSize
                          << " not equal to "
                          << "VkWriteDescriptorSet descriptorCount " << update->descriptorCount;
            }
            *error_msg = error_str.str();
            return false;
        }
        // This error is probably unreachable due to the previous two errors
        if (write_inline_info && (write_inline_info->dataSize % 4) != 0) {
            *error_code = "VUID-VkWriteDescriptorSetInlineUniformBlock-dataSize-02222";
            std::stringstream error_str;
            error_str << "Attempting write update to " << dest_set->StringifySetAndLayout() << " binding #" << update->dstBinding
                      << " with "
                      << "VkWriteDescriptorSetInlineUniformBlock dataSize " << write_inline_info->dataSize
                      << " not a multiple of 4";
            *error_msg = error_str.str();
            return false;
        }
    }
    // Verify all bindings update share identical properties across all items
    if (update->descriptorCount > 0) {
        // Save first binding information and error if something different is found
        DescriptorSetLayout::ConstBindingIterator current_binding(dest_layout, update->dstBinding);
        VkShaderStageFlags stage_flags = current_binding.GetStageFlags();
        VkDescriptorType descriptor_type = current_binding.GetType();
        bool immutable_samplers = (current_binding.GetImmutableSamplerPtr() == nullptr);
        uint32_t dst_array_element = update->dstArrayElement;

        for (uint32_t i = 0; i < update->descriptorCount;) {
            if (current_binding.AtEnd() == true) {
                break;  // prevents setting error here if bindings don't exist
            }

            // All consecutive bindings updated, except those with a descriptorCount of zero, must have identical descType and stageFlags
            if(current_binding.GetDescriptorCount() > 0) {
                // Check for consistent stageFlags and descriptorType
                if ((current_binding.GetStageFlags() != stage_flags) || (current_binding.GetType() != descriptor_type)) {
                    *error_code = "VUID-VkWriteDescriptorSet-descriptorCount-00317";
                    std::stringstream error_str;
                    error_str << "Attempting write update to " << dest_set->StringifySetAndLayout() << " binding index #"
                              << current_binding.GetIndex() << " (" << i << " from dstBinding offset)"
                              << " with a different stageFlag and/or descriptorType from previous bindings."
                              << " All bindings must have consecutive stageFlag and/or descriptorType across a VkWriteDescriptorSet";
                    *error_msg = error_str.str();
                    return false;
                }
                // Check if all immutableSamplers or not
                if ((current_binding.GetImmutableSamplerPtr() == nullptr) != immutable_samplers) {
                    *error_code = "VUID-VkWriteDescriptorSet-descriptorCount-00318";
                    std::stringstream error_str;
                    error_str << "Attempting write update to " << dest_set->StringifySetAndLayout() << " binding index #"
                              << current_binding.GetIndex() << " (" << i << " from dstBinding offset)"
                              << " with a different usage of immutable samplers from previous bindings."
                              << " All bindings must have all or none usage of immutable samplers across a VkWriteDescriptorSet";
                    *error_msg = error_str.str();
                    return false;
                }
            }

            // Skip the remaining descriptors for this binding, and move to the next binding
            i += (current_binding.GetDescriptorCount() - dst_array_element);
            dst_array_element = 0;
            ++current_binding;
        }
    }

    // Verify consecutive bindings match (if needed)
    if (!VerifyUpdateConsistency(report_data, DescriptorSetLayout::ConstBindingIterator(dest_layout, update->dstBinding),
                                 update->dstArrayElement, update->descriptorCount, "write update to", dest_set->GetSet(),
                                 error_msg)) {
        *error_code = "VUID-VkWriteDescriptorSet-dstArrayElement-00321";
        return false;
    }
    // Verify write to variable descriptor
    if (dest_set->IsVariableDescriptorCount(update->dstBinding)) {
        if ((update->dstArrayElement + update->descriptorCount) > dest_set->GetVariableDescriptorCount()) {
            std::stringstream error_str;
            *error_code = "VUID-VkWriteDescriptorSet-dstArrayElement-00321";
            error_str << "Attempting write update to " << dest_set->StringifySetAndLayout() << " binding index #"
                      << update->dstBinding << " array element " << update->dstArrayElement << " with " << update->descriptorCount
                      << " writes but variable descriptor size is " << dest_set->GetVariableDescriptorCount();
            *error_msg = error_str.str();
            return false;
        }
    }
    // Update is within bounds and consistent so last step is to validate update contents
    if (!VerifyWriteUpdateContents(dest_set, update, start_idx, func_name, error_code, error_msg, push)) {
        std::stringstream error_str;
        error_str << "Write update to " << dest_set->StringifySetAndLayout() << " binding #" << update->dstBinding
                  << " failed with error message: " << error_msg->c_str();
        *error_msg = error_str.str();
        return false;
    }
    const auto orig_binding = DescriptorSetLayout::ConstBindingIterator(dest_set->GetLayout().get(), update->dstBinding);
    if (!orig_binding.AtEnd() && orig_binding.GetType() == VK_DESCRIPTOR_TYPE_MUTABLE_VALVE) {
        // Check if the new descriptor descriptor type is in the list of allowed mutable types for this binding
        if (!orig_binding.Layout()->IsTypeMutable(update->descriptorType, update->dstBinding)) {
            *error_code = "VUID-VkWriteDescriptorSet-dstSet-04611";
            std::stringstream error_str;
            error_str << "Write update type is " << string_VkDescriptorType(update->descriptorType)
                      << ", but descriptor set layout binding was created with type VK_DESCRIPTOR_TYPE_MUTABLE_VALVE and used type "
                         "is not in VkMutableDescriptorTypeListVALVE::pDescriptorTypes for this binding.";
            *error_msg = error_str.str();
            return false;
        }
    }
    // All checks passed, update is clean
    return true;
}

// Verify that the contents of the update are ok, but don't perform actual update
bool CoreChecks::VerifyWriteUpdateContents(const DescriptorSet *dest_set, const VkWriteDescriptorSet *update, const uint32_t index,
                                           const char *func_name, std::string *error_code, std::string *error_msg,
                                           bool push) const {
    using ImageSamplerDescriptor = cvdescriptorset::ImageSamplerDescriptor;
    using Descriptor = cvdescriptorset::Descriptor;

    switch (update->descriptorType) {
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
            for (uint32_t di = 0; di < update->descriptorCount; ++di) {
                // Validate image
                auto image_view = update->pImageInfo[di].imageView;
                auto image_layout = update->pImageInfo[di].imageLayout;
                auto sampler = update->pImageInfo[di].sampler;
                auto iv_state = Get<IMAGE_VIEW_STATE>(image_view);
                const ImageSamplerDescriptor *desc =
                    (const ImageSamplerDescriptor *)dest_set->GetDescriptorFromGlobalIndex(index + di);
                if (image_view) {
                    const auto *image_state = iv_state->image_state.get();
                    if (!ValidateImageUpdate(image_view, image_layout, update->descriptorType, func_name, error_code, error_msg)) {
                        std::stringstream error_str;
                        error_str << "Attempted write update to combined image sampler descriptor failed due to: "
                                  << error_msg->c_str();
                        *error_msg = error_str.str();
                        return false;
                    }
                    if (IsExtEnabled(device_extensions.vk_khr_sampler_ycbcr_conversion)) {
                        if (desc->IsImmutableSampler()) {
                            auto sampler_state = Get<SAMPLER_STATE>(desc->GetSampler());
                            if (iv_state && sampler_state) {
                                if (iv_state->samplerConversion != sampler_state->samplerConversion) {
                                    *error_code = "VUID-VkWriteDescriptorSet-descriptorType-01948";
                                    std::stringstream error_str;
                                    error_str
                                        << "Attempted write update to combined image sampler and image view and sampler ycbcr "
                                           "conversions are not identical, sampler: "
                                        << report_data->FormatHandle(desc->GetSampler())
                                        << " image view: " << report_data->FormatHandle(iv_state->image_view()) << ".";
                                    *error_msg = error_str.str();
                                    return false;
                                }
                            }
                        } else {
                            if (iv_state && (iv_state->samplerConversion != VK_NULL_HANDLE)) {
                                *error_code = "VUID-VkWriteDescriptorSet-descriptorType-02738";
                                std::stringstream error_str;
                                error_str << "Because dstSet (" << report_data->FormatHandle(update->dstSet)
                                          << ") is bound to image view (" << report_data->FormatHandle(iv_state->image_view())
                                          << ") that includes a YCBCR conversion, it must have been allocated with a layout that "
                                             "includes an immutable sampler.";
                                *error_msg = error_str.str();
                                return false;
                            }
                        }
                    }
                    // If there is an immutable sampler then |sampler| isn't used, so the following VU does not apply.
                    if (sampler && !desc->IsImmutableSampler() && FormatIsMultiplane(image_state->createInfo.format)) {
                        // multiplane formats must be created with mutable format bit
                        if (0 == (image_state->createInfo.flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT)) {
                            *error_code = "VUID-VkDescriptorImageInfo-sampler-01564";
                            std::stringstream error_str;
                            error_str << "image " << report_data->FormatHandle(image_state->image())
                                      << " combined image sampler is a multi-planar "
                                      << "format and was not was not created with the VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT";
                            *error_msg = error_str.str();
                            return false;
                        }
                        // image view need aspect mask for only the planes supported of format
                        VkImageAspectFlags legal_aspect_flags = (VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT);
                        legal_aspect_flags |=
                            (FormatPlaneCount(image_state->createInfo.format) == 3) ? VK_IMAGE_ASPECT_PLANE_2_BIT : 0;
                        if (0 != (iv_state->create_info.subresourceRange.aspectMask & (~legal_aspect_flags))) {
                            *error_code = "VUID-VkDescriptorImageInfo-sampler-01564";
                            std::stringstream error_str;
                            error_str << "image " << report_data->FormatHandle(image_state->image())
                                      << " combined image sampler is a multi-planar "
                                      << "format and " << report_data->FormatHandle(iv_state->image_view())
                                      << " aspectMask must only include " << string_VkImageAspectFlags(legal_aspect_flags);
                            *error_msg = error_str.str();
                            return false;
                        }
                    }

                    // Verify portability
                    auto sampler_state = Get<SAMPLER_STATE>(sampler);
                    if (sampler_state) {
                        if (IsExtEnabled(device_extensions.vk_khr_portability_subset)) {
                            if ((VK_FALSE == enabled_features.portability_subset_features.mutableComparisonSamplers) &&
                                (VK_FALSE != sampler_state->createInfo.compareEnable)) {
                                LogError(device, "VUID-VkDescriptorImageInfo-mutableComparisonSamplers-04450",
                                         "%s (portability error): sampler comparison not available.", func_name);
                            }
                        }
                    }
                }
            }
        }
        // Fall through
        case VK_DESCRIPTOR_TYPE_SAMPLER: {
            for (uint32_t di = 0; di < update->descriptorCount; ++di) {
                const auto *desc = static_cast<const Descriptor *>(dest_set->GetDescriptorFromGlobalIndex(index + di));
                if (!desc->IsImmutableSampler()) {
                    if (!ValidateSampler(update->pImageInfo[di].sampler)) {
                        *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00325";
                        std::stringstream error_str;
                        error_str << "Attempted write update to sampler descriptor with invalid sampler: "
                                  << report_data->FormatHandle(update->pImageInfo[di].sampler) << ".";
                        *error_msg = error_str.str();
                        return false;
                    }
                } else if (update->descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER && !push) {
                    *error_code = "VUID-VkWriteDescriptorSet-descriptorType-02752";
                    std::stringstream error_str;
                    error_str << "Attempted write update to an immutable sampler descriptor.";
                    *error_msg = error_str.str();
                    return false;
                }
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: {
            for (uint32_t di = 0; di < update->descriptorCount; ++di) {
                auto image_view = update->pImageInfo[di].imageView;
                auto image_layout = update->pImageInfo[di].imageLayout;
                if (image_view) {
                    if (!ValidateImageUpdate(image_view, image_layout, update->descriptorType, func_name, error_code, error_msg)) {
                        std::stringstream error_str;
                        error_str << "Attempted write update to image descriptor failed due to: " << error_msg->c_str();
                        *error_msg = error_str.str();
                        return false;
                    }
                }
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: {
            for (uint32_t di = 0; di < update->descriptorCount; ++di) {
                auto buffer_view = update->pTexelBufferView[di];
                if (buffer_view) {
                    auto bv_state = Get<BUFFER_VIEW_STATE>(buffer_view);
                    if (!bv_state) {
                        *error_code = "VUID-VkWriteDescriptorSet-descriptorType-02994";
                        std::stringstream error_str;
                        error_str << "Attempted write update to texel buffer descriptor with invalid buffer view: "
                                  << report_data->FormatHandle(buffer_view);
                        *error_msg = error_str.str();
                        return false;
                    }
                    auto buffer = bv_state->create_info.buffer;
                    auto buffer_state = Get<BUFFER_STATE>(buffer);
                    // Verify that buffer underlying the view hasn't been destroyed prematurely
                    if (!buffer_state) {
                        *error_code = "VUID-VkWriteDescriptorSet-descriptorType-02994";
                        std::stringstream error_str;
                        error_str << "Attempted write update to texel buffer descriptor failed because underlying buffer ("
                                  << report_data->FormatHandle(buffer) << ") has been destroyed: " << error_msg->c_str();
                        *error_msg = error_str.str();
                        return false;
                    } else if (!cvdescriptorset::ValidateBufferUsage(report_data, buffer_state.get(), update->descriptorType,
                                                                     error_code, error_msg)) {
                        std::stringstream error_str;
                        error_str << "Attempted write update to texel buffer descriptor failed due to: " << error_msg->c_str();
                        *error_msg = error_str.str();
                        return false;
                    }
                }
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
            for (uint32_t di = 0; di < update->descriptorCount; ++di) {
                if (update->pBufferInfo[di].buffer) {
                    if (!ValidateBufferUpdate(update->pBufferInfo + di, update->descriptorType, func_name, error_code, error_msg)) {
                        std::stringstream error_str;
                        error_str << "Attempted write update to buffer descriptor failed due to: " << error_msg->c_str();
                        *error_msg = error_str.str();
                        return false;
                    }
                }
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT:
            break;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV: {
            const auto *acc_info = LvlFindInChain<VkWriteDescriptorSetAccelerationStructureNV>(update->pNext);
            for (uint32_t di = 0; di < update->descriptorCount; ++di) {
                auto as_state = Get<ACCELERATION_STRUCTURE_STATE>(acc_info->pAccelerationStructures[di]);
                if (!ValidateAccelerationStructureUpdate(as_state.get(), func_name, error_code, error_msg)) {
                    std::stringstream error_str;
                    error_str << "Attempted write update to acceleration structure descriptor failed due to: "
                              << error_msg->c_str();
                    *error_msg = error_str.str();
                    return false;
                }
            }

        } break;
        // KHR acceleration structures don't require memory to be bound manually to them.
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
            break;
        default:
            assert(0);  // We've already verified update type so should never get here
            break;
    }
    // All checks passed so update contents are good
    return true;
}
