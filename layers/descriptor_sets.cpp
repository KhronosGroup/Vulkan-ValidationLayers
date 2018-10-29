/* Copyright (c) 2015-2018 The Khronos Group Inc.
 * Copyright (c) 2015-2018 Valve Corporation
 * Copyright (c) 2015-2018 LunarG, Inc.
 * Copyright (C) 2015-2018 Google Inc.
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

#include "descriptor_sets.h"
#include "hash_vk_types.h"
#include "vk_enum_string_helper.h"
#include "vk_safe_struct.h"
#include "vk_typemap_helper.h"
#include "buffer_validation.h"
#include <sstream>
#include <algorithm>
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
// Proactively reserve and resize as possible, as the reallocation was visible in profiling
cvdescriptorset::DescriptorSetLayoutDef::DescriptorSetLayoutDef(const VkDescriptorSetLayoutCreateInfo *p_create_info)
    : flags_(p_create_info->flags), binding_count_(0), descriptor_count_(0), dynamic_descriptor_count_(0) {
    const auto *flags_create_info = lvl_find_in_chain<VkDescriptorSetLayoutBindingFlagsCreateInfoEXT>(p_create_info->pNext);

    binding_type_stats_ = {0, 0, 0};
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
            binding_type_stats_.dynamic_buffer_count++;
        } else if ((binding_info.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) ||
                   (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)) {
            binding_type_stats_.non_dynamic_buffer_count++;
        } else {
            binding_type_stats_.image_sampler_count++;
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
VkDescriptorBindingFlagsEXT cvdescriptorset::DescriptorSetLayoutDef::GetDescriptorBindingFlagsFromIndex(
    const uint32_t index) const {
    if (index >= binding_flags_.size()) return 0;
    return binding_flags_[index];
}

// For the given global index, return index
uint32_t cvdescriptorset::DescriptorSetLayoutDef::GetIndexFromGlobalIndex(const uint32_t global_index) const {
    auto start_it = global_start_to_index_map_.upper_bound(global_index);
    uint32_t index = binding_count_;
    assert(start_it != global_start_to_index_map_.cbegin());
    if (start_it != global_start_to_index_map_.cbegin()) {
        --start_it;
        index = start_it->second;
#ifndef NDEBUG
        const auto &range = GetGlobalIndexRangeFromBinding(bindings_[index].binding);
        assert(range.start <= global_index && global_index < range.end);
#endif
    }
    return index;
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
// If our layout is compatible with rh_ds_layout, return true,
//  else return false and fill in error_msg will description of what causes incompatibility
bool cvdescriptorset::DescriptorSetLayout::IsCompatible(DescriptorSetLayout const *const rh_ds_layout,
                                                        std::string *error_msg) const {
    // Trivial case
    if (layout_ == rh_ds_layout->GetDescriptorSetLayout()) return true;
    if (GetLayoutDef() == rh_ds_layout->GetLayoutDef()) return true;
    bool detailed_compat_check =
        GetLayoutDef()->IsCompatible(layout_, rh_ds_layout->GetDescriptorSetLayout(), rh_ds_layout->GetLayoutDef(), error_msg);
    // The detailed check should never tell us mismatching DSL are compatible
    assert(!detailed_compat_check);
    return detailed_compat_check;
}

// Do a detailed compatibility check of this def (referenced by ds_layout), vs. the rhs (layout and def)
// Should only be called if trivial accept has failed, and in that context should return false.
bool cvdescriptorset::DescriptorSetLayoutDef::IsCompatible(VkDescriptorSetLayout ds_layout, VkDescriptorSetLayout rh_ds_layout,
                                                           DescriptorSetLayoutDef const *const rh_ds_layout_def,
                                                           std::string *error_msg) const {
    if (descriptor_count_ != rh_ds_layout_def->descriptor_count_) {
        std::stringstream error_str;
        error_str << "DescriptorSetLayout " << ds_layout << " has " << descriptor_count_ << " descriptors, but DescriptorSetLayout "
                  << rh_ds_layout << ", which comes from pipelineLayout, has " << rh_ds_layout_def->descriptor_count_
                  << " descriptors.";
        *error_msg = error_str.str();
        return false;  // trivial fail case
    }

    // Descriptor counts match so need to go through bindings one-by-one
    //  and verify that type and stageFlags match
    for (auto binding : bindings_) {
        // TODO : Do we also need to check immutable samplers?
        // VkDescriptorSetLayoutBinding *rh_binding;
        if (binding.descriptorCount != rh_ds_layout_def->GetDescriptorCountFromBinding(binding.binding)) {
            std::stringstream error_str;
            error_str << "Binding " << binding.binding << " for DescriptorSetLayout " << ds_layout << " has a descriptorCount of "
                      << binding.descriptorCount << " but binding " << binding.binding << " for DescriptorSetLayout "
                      << rh_ds_layout << ", which comes from pipelineLayout, has a descriptorCount of "
                      << rh_ds_layout_def->GetDescriptorCountFromBinding(binding.binding);
            *error_msg = error_str.str();
            return false;
        } else if (binding.descriptorType != rh_ds_layout_def->GetTypeFromBinding(binding.binding)) {
            std::stringstream error_str;
            error_str << "Binding " << binding.binding << " for DescriptorSetLayout " << ds_layout << " is type '"
                      << string_VkDescriptorType(binding.descriptorType) << "' but binding " << binding.binding
                      << " for DescriptorSetLayout " << rh_ds_layout << ", which comes from pipelineLayout, is type '"
                      << string_VkDescriptorType(rh_ds_layout_def->GetTypeFromBinding(binding.binding)) << "'";
            *error_msg = error_str.str();
            return false;
        } else if (binding.stageFlags != rh_ds_layout_def->GetStageFlagsFromBinding(binding.binding)) {
            std::stringstream error_str;
            error_str << "Binding " << binding.binding << " for DescriptorSetLayout " << ds_layout << " has stageFlags "
                      << binding.stageFlags << " but binding " << binding.binding << " for DescriptorSetLayout " << rh_ds_layout
                      << ", which comes from pipelineLayout, has stageFlags "
                      << rh_ds_layout_def->GetStageFlagsFromBinding(binding.binding);
            *error_msg = error_str.str();
            return false;
        }
    }
    return true;
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
// Starting at offset descriptor of given binding, parse over update_count
//  descriptor updates and verify that for any binding boundaries that are crossed, the next binding(s) are all consistent
//  Consistency means that their type, stage flags, and whether or not they use immutable samplers matches
//  If so, return true. If not, fill in error_msg and return false
bool cvdescriptorset::DescriptorSetLayoutDef::VerifyUpdateConsistency(uint32_t current_binding, uint32_t offset,
                                                                      uint32_t update_count, const char *type,
                                                                      const VkDescriptorSet set, std::string *error_msg) const {
    // Verify consecutive bindings match (if needed)
    auto orig_binding = current_binding;
    // Track count of descriptors in the current_bindings that are remaining to be updated
    auto binding_remaining = GetDescriptorCountFromBinding(current_binding);
    // First, it's legal to offset beyond your own binding so handle that case
    //  Really this is just searching for the binding in which the update begins and adjusting offset accordingly
    while (offset >= binding_remaining) {
        // Advance to next binding, decrement offset by binding size
        offset -= binding_remaining;
        binding_remaining = GetDescriptorCountFromBinding(++current_binding);
    }
    binding_remaining -= offset;
    while (update_count > binding_remaining) {  // While our updates overstep current binding
        // Verify next consecutive binding matches type, stage flags & immutable sampler use
        if (!IsNextBindingConsistent(current_binding++)) {
            std::stringstream error_str;
            error_str << "Attempting " << type << " descriptor set " << set << " binding #" << orig_binding << " with #"
                      << update_count
                      << " descriptors being updated but this update oversteps the bounds of this binding and the next binding is "
                         "not consistent with current binding so this update is invalid.";
            *error_msg = error_str.str();
            return false;
        }
        // For sake of this check consider the bindings updated and grab count for next binding
        update_count -= binding_remaining;
        binding_remaining = GetDescriptorCountFromBinding(current_binding);
    }
    return true;
}

// The DescriptorSetLayout stores the per handle data for a descriptor set layout, and references the common defintion for the
// handle invariant portion
cvdescriptorset::DescriptorSetLayout::DescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo *p_create_info,
                                                          const VkDescriptorSetLayout layout)
    : layout_(layout), layout_destroyed_(false), layout_id_(GetCanonicalId(p_create_info)) {}

// Validate descriptor set layout create info
bool cvdescriptorset::DescriptorSetLayout::ValidateCreateInfo(
    const debug_report_data *report_data, const VkDescriptorSetLayoutCreateInfo *create_info, const bool push_descriptor_ext,
    const uint32_t max_push_descriptors, const bool descriptor_indexing_ext,
    const VkPhysicalDeviceDescriptorIndexingFeaturesEXT *descriptor_indexing_features,
    const VkPhysicalDeviceInlineUniformBlockFeaturesEXT *inline_uniform_block_features,
    const VkPhysicalDeviceInlineUniformBlockPropertiesEXT *inline_uniform_block_props) {
    bool skip = false;
    std::unordered_set<uint32_t> bindings;
    uint64_t total_descriptors = 0;

    const auto *flags_create_info = lvl_find_in_chain<VkDescriptorSetLayoutBindingFlagsCreateInfoEXT>(create_info->pNext);

    const bool push_descriptor_set = !!(create_info->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);
    if (push_descriptor_set && !push_descriptor_ext) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        kVUID_Core_DrawState_ExtensionNotEnabled,
                        "Attempted to use %s in %s but its required extension %s has not been enabled.\n",
                        "VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR", "VkDescriptorSetLayoutCreateInfo::flags",
                        VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    }

    const bool update_after_bind_set = !!(create_info->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT);
    if (update_after_bind_set && !descriptor_indexing_ext) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        kVUID_Core_DrawState_ExtensionNotEnabled,
                        "Attemped to use %s in %s but its required extension %s has not been enabled.\n",
                        "VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT", "VkDescriptorSetLayoutCreateInfo::flags",
                        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    }

    auto valid_type = [push_descriptor_set](const VkDescriptorType type) {
        return !push_descriptor_set ||
               ((type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) && (type != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) &&
                (type != VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT));
    };

    uint32_t max_binding = 0;

    for (uint32_t i = 0; i < create_info->bindingCount; ++i) {
        const auto &binding_info = create_info->pBindings[i];
        max_binding = std::max(max_binding, binding_info.binding);

        if (!bindings.insert(binding_info.binding).second) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkDescriptorSetLayoutCreateInfo-binding-00279",
                            "duplicated binding number in VkDescriptorSetLayoutBinding.");
        }
        if (!valid_type(binding_info.descriptorType)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT)
                                ? "VUID-VkDescriptorSetLayoutCreateInfo-flags-02208"
                                : "VUID-VkDescriptorSetLayoutCreateInfo-flags-00280",
                            "invalid type %s ,for push descriptors in VkDescriptorSetLayoutBinding entry %" PRIu32 ".",
                            string_VkDescriptorType(binding_info.descriptorType), i);
        }

        if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT) {
            if ((binding_info.descriptorCount % 4) != 0) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkDescriptorSetLayoutBinding-descriptorType-02209",
                                "descriptorCount =(%" PRIu32 ") must be a multiple of 4", binding_info.descriptorCount);
            }
            if (binding_info.descriptorCount > inline_uniform_block_props->maxInlineUniformBlockSize) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                "VUID-VkDescriptorSetLayoutBinding-descriptorType-02210",
                                "descriptorCount =(%" PRIu32 ") must be less than or equal to maxInlineUniformBlockSize",
                                binding_info.descriptorCount);
            }
        }

        total_descriptors += binding_info.descriptorCount;
    }

    if (flags_create_info) {
        if (flags_create_info->bindingCount != 0 && flags_create_info->bindingCount != create_info->bindingCount) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfoEXT-bindingCount-03002",
                            "VkDescriptorSetLayoutCreateInfo::bindingCount (%d) != "
                            "VkDescriptorSetLayoutBindingFlagsCreateInfoEXT::bindingCount (%d)",
                            create_info->bindingCount, flags_create_info->bindingCount);
        }

        if (flags_create_info->bindingCount == create_info->bindingCount) {
            for (uint32_t i = 0; i < create_info->bindingCount; ++i) {
                const auto &binding_info = create_info->pBindings[i];

                if (flags_create_info->pBindingFlags[i] & VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT) {
                    if (!update_after_bind_set) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkDescriptorSetLayoutCreateInfo-flags-03000",
                                        "Invalid flags for VkDescriptorSetLayoutBinding entry %" PRIu32, i);
                    }

                    if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER &&
                        !descriptor_indexing_features->descriptorBindingUniformBufferUpdateAfterBind) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfoEXT-"
                                        "descriptorBindingUniformBufferUpdateAfterBind-03005",
                                        "Invalid flags for VkDescriptorSetLayoutBinding entry %" PRIu32, i);
                    }
                    if ((binding_info.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER ||
                         binding_info.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
                         binding_info.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) &&
                        !descriptor_indexing_features->descriptorBindingSampledImageUpdateAfterBind) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfoEXT-"
                                        "descriptorBindingSampledImageUpdateAfterBind-03006",
                                        "Invalid flags for VkDescriptorSetLayoutBinding entry %" PRIu32, i);
                    }
                    if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE &&
                        !descriptor_indexing_features->descriptorBindingStorageImageUpdateAfterBind) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfoEXT-"
                                        "descriptorBindingStorageImageUpdateAfterBind-03007",
                                        "Invalid flags for VkDescriptorSetLayoutBinding entry %" PRIu32, i);
                    }
                    if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER &&
                        !descriptor_indexing_features->descriptorBindingStorageBufferUpdateAfterBind) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfoEXT-"
                                        "descriptorBindingStorageBufferUpdateAfterBind-03008",
                                        "Invalid flags for VkDescriptorSetLayoutBinding entry %" PRIu32, i);
                    }
                    if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER &&
                        !descriptor_indexing_features->descriptorBindingUniformTexelBufferUpdateAfterBind) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfoEXT-"
                                        "descriptorBindingUniformTexelBufferUpdateAfterBind-03009",
                                        "Invalid flags for VkDescriptorSetLayoutBinding entry %" PRIu32, i);
                    }
                    if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER &&
                        !descriptor_indexing_features->descriptorBindingStorageTexelBufferUpdateAfterBind) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfoEXT-"
                                        "descriptorBindingStorageTexelBufferUpdateAfterBind-03010",
                                        "Invalid flags for VkDescriptorSetLayoutBinding entry %" PRIu32, i);
                    }
                    if ((binding_info.descriptorType == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT ||
                         binding_info.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
                         binding_info.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfoEXT-None-03011",
                                        "Invalid flags for VkDescriptorSetLayoutBinding entry %" PRIu32, i);
                    }

                    if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT &&
                        !inline_uniform_block_features->descriptorBindingInlineUniformBlockUpdateAfterBind) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfoEXT-"
                                        "descriptorBindingInlineUniformBlockUpdateAfterBind-02211",
                                        "Invalid flags (VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT) for "
                                        "VkDescriptorSetLayoutBinding entry %" PRIu32
                                        " with descriptorBindingInlineUniformBlockUpdateAfterBind not enabled",
                                        i);
                    }
                }

                if (flags_create_info->pBindingFlags[i] & VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT) {
                    if (!descriptor_indexing_features->descriptorBindingUpdateUnusedWhilePending) {
                        skip |= log_msg(
                            report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfoEXT-descriptorBindingUpdateUnusedWhilePending-03012",
                            "Invalid flags for VkDescriptorSetLayoutBinding entry %" PRIu32, i);
                    }
                }

                if (flags_create_info->pBindingFlags[i] & VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT) {
                    if (!descriptor_indexing_features->descriptorBindingPartiallyBound) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfoEXT-descriptorBindingPartiallyBound-03013",
                                        "Invalid flags for VkDescriptorSetLayoutBinding entry %" PRIu32, i);
                    }
                }

                if (flags_create_info->pBindingFlags[i] & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT) {
                    if (binding_info.binding != max_binding) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfoEXT-pBindingFlags-03004",
                                        "Invalid flags for VkDescriptorSetLayoutBinding entry %" PRIu32, i);
                    }

                    if (!descriptor_indexing_features->descriptorBindingVariableDescriptorCount) {
                        skip |= log_msg(
                            report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfoEXT-descriptorBindingVariableDescriptorCount-03014",
                            "Invalid flags for VkDescriptorSetLayoutBinding entry %" PRIu32, i);
                    }
                    if ((binding_info.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
                         binding_info.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)) {
                        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                        "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfoEXT-pBindingFlags-03015",
                                        "Invalid flags for VkDescriptorSetLayoutBinding entry %" PRIu32, i);
                    }
                }

                if (push_descriptor_set &&
                    (flags_create_info->pBindingFlags[i] &
                     (VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT |
                      VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT))) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                    "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfoEXT-flags-03003",
                                    "Invalid flags for VkDescriptorSetLayoutBinding entry %" PRIu32, i);
                }
            }
        }
    }

    if ((push_descriptor_set) && (total_descriptors > max_push_descriptors)) {
        const char *undefined = push_descriptor_ext ? "" : " -- undefined";
        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                    "VUID-VkDescriptorSetLayoutCreateInfo-flags-00281",
                    "for push descriptor, total descriptor count in layout (%" PRIu64
                    ") must not be greater than VkPhysicalDevicePushDescriptorPropertiesKHR::maxPushDescriptors (%" PRIu32 "%s).",
                    total_descriptors, max_push_descriptors, undefined);
    }

    return skip;
}

cvdescriptorset::AllocateDescriptorSetsData::AllocateDescriptorSetsData(uint32_t count)
    : required_descriptors_by_type{}, layout_nodes(count, nullptr) {}

cvdescriptorset::DescriptorSet::DescriptorSet(const VkDescriptorSet set, const VkDescriptorPool pool,
                                              const std::shared_ptr<DescriptorSetLayout const> &layout, uint32_t variable_count,
                                              layer_data *dev_data)
    : some_update_(false),
      set_(set),
      pool_state_(nullptr),
      p_layout_(layout),
      device_data_(dev_data),
      limits_(GetPhysDevProperties(dev_data)->properties.limits),
      variable_count_(variable_count) {
    pool_state_ = GetDescriptorPoolState(dev_data, pool);
    // Foreach binding, create default descriptors of given type
    descriptors_.reserve(p_layout_->GetTotalDescriptorCount());
    for (uint32_t i = 0; i < p_layout_->GetBindingCount(); ++i) {
        auto type = p_layout_->GetTypeFromIndex(i);
        switch (type) {
            case VK_DESCRIPTOR_TYPE_SAMPLER: {
                auto immut_sampler = p_layout_->GetImmutableSamplerPtrFromIndex(i);
                for (uint32_t di = 0; di < p_layout_->GetDescriptorCountFromIndex(i); ++di) {
                    if (immut_sampler) {
                        descriptors_.emplace_back(new SamplerDescriptor(immut_sampler + di));
                        some_update_ = true;  // Immutable samplers are updated at creation
                    } else
                        descriptors_.emplace_back(new SamplerDescriptor(nullptr));
                }
                break;
            }
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
                auto immut = p_layout_->GetImmutableSamplerPtrFromIndex(i);
                for (uint32_t di = 0; di < p_layout_->GetDescriptorCountFromIndex(i); ++di) {
                    if (immut) {
                        descriptors_.emplace_back(new ImageSamplerDescriptor(immut + di));
                        some_update_ = true;  // Immutable samplers are updated at creation
                    } else
                        descriptors_.emplace_back(new ImageSamplerDescriptor(nullptr));
                }
                break;
            }
            // ImageDescriptors
            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                for (uint32_t di = 0; di < p_layout_->GetDescriptorCountFromIndex(i); ++di)
                    descriptors_.emplace_back(new ImageDescriptor(type));
                break;
            case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                for (uint32_t di = 0; di < p_layout_->GetDescriptorCountFromIndex(i); ++di)
                    descriptors_.emplace_back(new TexelDescriptor(type));
                break;
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                for (uint32_t di = 0; di < p_layout_->GetDescriptorCountFromIndex(i); ++di)
                    descriptors_.emplace_back(new BufferDescriptor(type));
                break;
            case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT:
                for (uint32_t di = 0; di < p_layout_->GetDescriptorCountFromIndex(i); ++di)
                    descriptors_.emplace_back(new InlineUniformDescriptor(type));
                break;
            case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NVX:
                for (uint32_t di = 0; di < p_layout_->GetDescriptorCountFromIndex(i); ++di)
                    descriptors_.emplace_back(new AccelerationStructureDescriptor(type));
                break;
            default:
                assert(0);  // Bad descriptor type specified
                break;
        }
    }
}

cvdescriptorset::DescriptorSet::~DescriptorSet() { InvalidateBoundCmdBuffers(); }

static std::string StringDescriptorReqViewType(descriptor_req req) {
    std::string result("");
    for (unsigned i = 0; i <= VK_IMAGE_VIEW_TYPE_END_RANGE; i++) {
        if (req & (1 << i)) {
            if (result.size()) result += ", ";
            result += string_VkImageViewType(VkImageViewType(i));
        }
    }

    if (!result.size()) result = "(none)";

    return result;
}

static char const *StringDescriptorReqComponentType(descriptor_req req) {
    if (req & DESCRIPTOR_REQ_COMPONENT_TYPE_SINT) return "SINT";
    if (req & DESCRIPTOR_REQ_COMPONENT_TYPE_UINT) return "UINT";
    if (req & DESCRIPTOR_REQ_COMPONENT_TYPE_FLOAT) return "FLOAT";
    return "(none)";
}

// Is this sets underlying layout compatible with passed in layout according to "Pipeline Layout Compatibility" in spec?
bool cvdescriptorset::DescriptorSet::IsCompatible(DescriptorSetLayout const *const layout, std::string *error) const {
    return layout->IsCompatible(p_layout_.get(), error);
}

static unsigned DescriptorRequirementsBitsFromFormat(VkFormat fmt) {
    if (FormatIsSInt(fmt)) return DESCRIPTOR_REQ_COMPONENT_TYPE_SINT;
    if (FormatIsUInt(fmt)) return DESCRIPTOR_REQ_COMPONENT_TYPE_UINT;
    if (FormatIsDepthAndStencil(fmt)) return DESCRIPTOR_REQ_COMPONENT_TYPE_FLOAT | DESCRIPTOR_REQ_COMPONENT_TYPE_UINT;
    if (fmt == VK_FORMAT_UNDEFINED) return 0;
    // everything else -- UNORM/SNORM/FLOAT/USCALED/SSCALED is all float in the shader.
    return DESCRIPTOR_REQ_COMPONENT_TYPE_FLOAT;
}

// Validate that the state of this set is appropriate for the given bindings and dynamic_offsets at Draw time
//  This includes validating that all descriptors in the given bindings are updated,
//  that any update buffers are valid, and that any dynamic offsets are within the bounds of their buffers.
// Return true if state is acceptable, or false and write an error message into error string
bool cvdescriptorset::DescriptorSet::ValidateDrawState(const std::map<uint32_t, descriptor_req> &bindings,
                                                       const std::vector<uint32_t> &dynamic_offsets, GLOBAL_CB_NODE *cb_node,
                                                       const char *caller, std::string *error) const {
    for (auto binding_pair : bindings) {
        auto binding = binding_pair.first;
        if (!p_layout_->HasBinding(binding)) {
            std::stringstream error_str;
            error_str << "Attempting to validate DrawState for binding #" << binding
                      << " which is an invalid binding for this descriptor set.";
            *error = error_str.str();
            return false;
        }
        IndexRange index_range = p_layout_->GetGlobalIndexRangeFromBinding(binding);
        auto array_idx = 0;  // Track array idx if we're dealing with array descriptors

        if (IsVariableDescriptorCount(binding)) {
            // Only validate the first N descriptors if it uses variable_count
            index_range.end = index_range.start + GetVariableDescriptorCount();
        }

        for (uint32_t i = index_range.start; i < index_range.end; ++i, ++array_idx) {
            if ((p_layout_->GetDescriptorBindingFlagsFromBinding(binding) & VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT) ||
                descriptors_[i]->GetClass() == InlineUniform) {
                // Can't validate the descriptor because it may not have been updated,
                // or the view could have been destroyed
                continue;
            } else if (!descriptors_[i]->updated) {
                std::stringstream error_str;
                error_str << "Descriptor in binding #" << binding << " at global descriptor index " << i
                          << " is being used in draw but has not been updated.";
                *error = error_str.str();
                return false;
            } else {
                auto descriptor_class = descriptors_[i]->GetClass();
                if (descriptor_class == GeneralBuffer) {
                    // Verify that buffers are valid
                    auto buffer = static_cast<BufferDescriptor *>(descriptors_[i].get())->GetBuffer();
                    auto buffer_node = GetBufferState(device_data_, buffer);
                    if (!buffer_node) {
                        std::stringstream error_str;
                        error_str << "Descriptor in binding #" << binding << " at global descriptor index " << i
                                  << " references invalid buffer " << buffer << ".";
                        *error = error_str.str();
                        return false;
                    } else if (!buffer_node->sparse) {
                        for (auto mem_binding : buffer_node->GetBoundMemory()) {
                            if (!GetMemObjInfo(device_data_, mem_binding)) {
                                std::stringstream error_str;
                                error_str << "Descriptor in binding #" << binding << " at global descriptor index " << i
                                          << " uses buffer " << buffer << " that references invalid memory " << mem_binding << ".";
                                *error = error_str.str();
                                return false;
                            }
                        }
                    }
                    if (descriptors_[i]->IsDynamic()) {
                        // Validate that dynamic offsets are within the buffer
                        auto buffer_size = buffer_node->createInfo.size;
                        auto range = static_cast<BufferDescriptor *>(descriptors_[i].get())->GetRange();
                        auto desc_offset = static_cast<BufferDescriptor *>(descriptors_[i].get())->GetOffset();
                        auto dyn_offset = dynamic_offsets[GetDynamicOffsetIndexFromBinding(binding) + array_idx];
                        if (VK_WHOLE_SIZE == range) {
                            if ((dyn_offset + desc_offset) > buffer_size) {
                                std::stringstream error_str;
                                error_str << "Dynamic descriptor in binding #" << binding << " at global descriptor index " << i
                                          << " uses buffer " << buffer << " with update range of VK_WHOLE_SIZE has dynamic offset "
                                          << dyn_offset << " combined with offset " << desc_offset
                                          << " that oversteps the buffer size of " << buffer_size << ".";
                                *error = error_str.str();
                                return false;
                            }
                        } else {
                            if ((dyn_offset + desc_offset + range) > buffer_size) {
                                std::stringstream error_str;
                                error_str << "Dynamic descriptor in binding #" << binding << " at global descriptor index " << i
                                          << " uses buffer " << buffer << " with dynamic offset " << dyn_offset
                                          << " combined with offset " << desc_offset << " and range " << range
                                          << " that oversteps the buffer size of " << buffer_size << ".";
                                *error = error_str.str();
                                return false;
                            }
                        }
                    }
                } else if (descriptor_class == ImageSampler || descriptor_class == Image) {
                    VkImageView image_view;
                    VkImageLayout image_layout;
                    if (descriptor_class == ImageSampler) {
                        image_view = static_cast<ImageSamplerDescriptor *>(descriptors_[i].get())->GetImageView();
                        image_layout = static_cast<ImageSamplerDescriptor *>(descriptors_[i].get())->GetImageLayout();
                    } else {
                        image_view = static_cast<ImageDescriptor *>(descriptors_[i].get())->GetImageView();
                        image_layout = static_cast<ImageDescriptor *>(descriptors_[i].get())->GetImageLayout();
                    }
                    auto reqs = binding_pair.second;

                    auto image_view_state = GetImageViewState(device_data_, image_view);
                    if (nullptr == image_view_state) {
                        // Image view must have been destroyed since initial update. Could potentially flag the descriptor
                        //  as "invalid" (updated = false) at DestroyImageView() time and detect this error at bind time
                        std::stringstream error_str;
                        error_str << "Descriptor in binding #" << binding << " at global descriptor index " << i
                                  << " is using imageView " << image_view << " that has been destroyed.";
                        *error = error_str.str();
                        return false;
                    }
                    auto image_view_ci = image_view_state->create_info;

                    if ((reqs & DESCRIPTOR_REQ_ALL_VIEW_TYPE_BITS) && (~reqs & (1 << image_view_ci.viewType))) {
                        // bad view type
                        std::stringstream error_str;
                        error_str << "Descriptor in binding #" << binding << " at global descriptor index " << i
                                  << " requires an image view of type " << StringDescriptorReqViewType(reqs) << " but got "
                                  << string_VkImageViewType(image_view_ci.viewType) << ".";
                        *error = error_str.str();
                        return false;
                    }

                    auto format_bits = DescriptorRequirementsBitsFromFormat(image_view_ci.format);
                    if (!(reqs & format_bits)) {
                        // bad component type
                        std::stringstream error_str;
                        error_str << "Descriptor in binding #" << binding << " at global descriptor index " << i << " requires "
                                  << StringDescriptorReqComponentType(reqs) << " component type, but bound descriptor format is "
                                  << string_VkFormat(image_view_ci.format) << ".";
                        *error = error_str.str();
                        return false;
                    }

                    auto image_node = GetImageState(device_data_, image_view_ci.image);
                    assert(image_node);
                    // Verify Image Layout
                    // Copy first mip level into sub_layers and loop over each mip level to verify layout
                    VkImageSubresourceLayers sub_layers;
                    sub_layers.aspectMask = image_view_ci.subresourceRange.aspectMask;
                    sub_layers.baseArrayLayer = image_view_ci.subresourceRange.baseArrayLayer;
                    sub_layers.layerCount = image_view_ci.subresourceRange.layerCount;
                    bool hit_error = false;
                    for (auto cur_level = image_view_ci.subresourceRange.baseMipLevel;
                         cur_level < image_view_ci.subresourceRange.levelCount; ++cur_level) {
                        sub_layers.mipLevel = cur_level;
                        // No "invalid layout" VUID required for this call, since the optimal_layout parameter is UNDEFINED.
                        VerifyImageLayout(device_data_, cb_node, image_node, sub_layers, image_layout, VK_IMAGE_LAYOUT_UNDEFINED,
                                          caller, kVUIDUndefined, "VUID-VkDescriptorImageInfo-imageLayout-00344", &hit_error);
                        if (hit_error) {
                            *error =
                                "Image layout specified at vkUpdateDescriptorSets() time doesn't match actual image layout at time "
                                "descriptor is used. See previous error callback for specific details.";
                            return false;
                        }
                    }
                    // Verify Sample counts
                    if ((reqs & DESCRIPTOR_REQ_SINGLE_SAMPLE) && image_node->createInfo.samples != VK_SAMPLE_COUNT_1_BIT) {
                        std::stringstream error_str;
                        error_str << "Descriptor in binding #" << binding << " at global descriptor index " << i
                                  << " requires bound image to have VK_SAMPLE_COUNT_1_BIT but got "
                                  << string_VkSampleCountFlagBits(image_node->createInfo.samples) << ".";
                        *error = error_str.str();
                        return false;
                    }
                    if ((reqs & DESCRIPTOR_REQ_MULTI_SAMPLE) && image_node->createInfo.samples == VK_SAMPLE_COUNT_1_BIT) {
                        std::stringstream error_str;
                        error_str << "Descriptor in binding #" << binding << " at global descriptor index " << i
                                  << " requires bound image to have multiple samples, but got VK_SAMPLE_COUNT_1_BIT.";
                        *error = error_str.str();
                        return false;
                    }
                }
                if (descriptor_class == ImageSampler || descriptor_class == PlainSampler) {
                    // Verify Sampler still valid
                    VkSampler sampler;
                    if (descriptor_class == ImageSampler) {
                        sampler = static_cast<ImageSamplerDescriptor *>(descriptors_[i].get())->GetSampler();
                    } else {
                        sampler = static_cast<SamplerDescriptor *>(descriptors_[i].get())->GetSampler();
                    }
                    if (!ValidateSampler(sampler, device_data_)) {
                        std::stringstream error_str;
                        error_str << "Descriptor in binding #" << binding << " at global descriptor index " << i
                                  << " is using sampler " << sampler << " that has been destroyed.";
                        *error = error_str.str();
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

// For given bindings, place any update buffers or images into the passed-in unordered_sets
uint32_t cvdescriptorset::DescriptorSet::GetStorageUpdates(const std::map<uint32_t, descriptor_req> &bindings,
                                                           std::unordered_set<VkBuffer> *buffer_set,
                                                           std::unordered_set<VkImageView> *image_set) const {
    auto num_updates = 0;
    for (auto binding_pair : bindings) {
        auto binding = binding_pair.first;
        // If a binding doesn't exist, skip it
        if (!p_layout_->HasBinding(binding)) {
            continue;
        }
        uint32_t start_idx = p_layout_->GetGlobalIndexRangeFromBinding(binding).start;
        if (descriptors_[start_idx]->IsStorage()) {
            if (Image == descriptors_[start_idx]->descriptor_class) {
                for (uint32_t i = 0; i < p_layout_->GetDescriptorCountFromBinding(binding); ++i) {
                    if (descriptors_[start_idx + i]->updated) {
                        image_set->insert(static_cast<ImageDescriptor *>(descriptors_[start_idx + i].get())->GetImageView());
                        num_updates++;
                    }
                }
            } else if (TexelBuffer == descriptors_[start_idx]->descriptor_class) {
                for (uint32_t i = 0; i < p_layout_->GetDescriptorCountFromBinding(binding); ++i) {
                    if (descriptors_[start_idx + i]->updated) {
                        auto bufferview = static_cast<TexelDescriptor *>(descriptors_[start_idx + i].get())->GetBufferView();
                        auto bv_state = GetBufferViewState(device_data_, bufferview);
                        if (bv_state) {
                            buffer_set->insert(bv_state->create_info.buffer);
                            num_updates++;
                        }
                    }
                }
            } else if (GeneralBuffer == descriptors_[start_idx]->descriptor_class) {
                for (uint32_t i = 0; i < p_layout_->GetDescriptorCountFromBinding(binding); ++i) {
                    if (descriptors_[start_idx + i]->updated) {
                        buffer_set->insert(static_cast<BufferDescriptor *>(descriptors_[start_idx + i].get())->GetBuffer());
                        num_updates++;
                    }
                }
            }
        }
    }
    return num_updates;
}
// Set is being deleted or updates so invalidate all bound cmd buffers
void cvdescriptorset::DescriptorSet::InvalidateBoundCmdBuffers() {
    core_validation::InvalidateCommandBuffers(device_data_, cb_bindings, {HandleToUint64(set_), kVulkanObjectTypeDescriptorSet});
}
// Perform write update in given update struct
void cvdescriptorset::DescriptorSet::PerformWriteUpdate(const VkWriteDescriptorSet *update) {
    // Perform update on a per-binding basis as consecutive updates roll over to next binding
    auto descriptors_remaining = update->descriptorCount;
    auto binding_being_updated = update->dstBinding;
    auto offset = update->dstArrayElement;
    uint32_t update_index = 0;
    while (descriptors_remaining) {
        uint32_t update_count = std::min(descriptors_remaining, GetDescriptorCountFromBinding(binding_being_updated));
        auto global_idx = p_layout_->GetGlobalIndexRangeFromBinding(binding_being_updated).start + offset;
        // Loop over the updates for a single binding at a time
        for (uint32_t di = 0; di < update_count; ++di, ++update_index) {
            descriptors_[global_idx + di]->WriteUpdate(update, update_index);
        }
        // Roll over to next binding in case of consecutive update
        descriptors_remaining -= update_count;
        offset = 0;
        binding_being_updated++;
    }
    if (update->descriptorCount) some_update_ = true;

    if (!(p_layout_->GetDescriptorBindingFlagsFromBinding(update->dstBinding) &
          (VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT))) {
        InvalidateBoundCmdBuffers();
    }
}
// Validate Copy update
bool cvdescriptorset::DescriptorSet::ValidateCopyUpdate(const debug_report_data *report_data, const VkCopyDescriptorSet *update,
                                                        const DescriptorSet *src_set, std::string *error_code,
                                                        std::string *error_msg) {
    // Verify dst layout still valid
    if (p_layout_->IsDestroyed()) {
        *error_code = "VUID-VkCopyDescriptorSet-dstSet-parameter";
        string_sprintf(error_msg,
                       "Cannot call vkUpdateDescriptorSets() to perform copy update on descriptor set dstSet 0x%" PRIxLEAST64
                       " created with destroyed VkDescriptorSetLayout 0x%" PRIxLEAST64,
                       HandleToUint64(set_), HandleToUint64(p_layout_->GetDescriptorSetLayout()));
        return false;
    }

    // Verify src layout still valid
    if (src_set->p_layout_->IsDestroyed()) {
        *error_code = "VUID-VkCopyDescriptorSet-srcSet-parameter";
        string_sprintf(
            error_msg,
            "Cannot call vkUpdateDescriptorSets() to perform copy update of dstSet 0x%" PRIxLEAST64
            " from descriptor set srcSet 0x%" PRIxLEAST64 " created with destroyed VkDescriptorSetLayout 0x%" PRIxLEAST64,
            HandleToUint64(set_), HandleToUint64(src_set->set_), HandleToUint64(src_set->p_layout_->GetDescriptorSetLayout()));
        return false;
    }

    if (!p_layout_->HasBinding(update->dstBinding)) {
        *error_code = "VUID-VkCopyDescriptorSet-dstBinding-00347";
        std::stringstream error_str;
        error_str << "DescriptorSet " << set_ << " does not have copy update dest binding of " << update->dstBinding;
        *error_msg = error_str.str();
        return false;
    }
    if (!src_set->HasBinding(update->srcBinding)) {
        *error_code = "VUID-VkCopyDescriptorSet-srcBinding-00345";
        std::stringstream error_str;
        error_str << "DescriptorSet " << set_ << " does not have copy update src binding of " << update->srcBinding;
        *error_msg = error_str.str();
        return false;
    }
    // Verify idle ds
    if (in_use.load() &&
        !(p_layout_->GetDescriptorBindingFlagsFromBinding(update->dstBinding) &
          (VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT))) {
        // TODO : Re-using Free Idle error code, need copy update idle error code
        *error_code = "VUID-vkFreeDescriptorSets-pDescriptorSets-00309";
        std::stringstream error_str;
        error_str << "Cannot call vkUpdateDescriptorSets() to perform copy update on descriptor set " << set_
                  << " that is in use by a command buffer";
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
        error_str << "Attempting copy update from descriptorSet " << update->srcSet << " binding#" << update->srcBinding
                  << " with offset index of " << src_set->GetGlobalIndexRangeFromBinding(update->srcBinding).start
                  << " plus update array offset of " << update->srcArrayElement << " and update of " << update->descriptorCount
                  << " descriptors oversteps total number of descriptors in set: " << src_set->GetTotalDescriptorCount();
        *error_msg = error_str.str();
        return false;
    }
    auto dst_start_idx = p_layout_->GetGlobalIndexRangeFromBinding(update->dstBinding).start + update->dstArrayElement;
    if ((dst_start_idx + update->descriptorCount) > p_layout_->GetTotalDescriptorCount()) {
        // DST update out of bounds
        *error_code = "VUID-VkCopyDescriptorSet-dstArrayElement-00348";
        std::stringstream error_str;
        error_str << "Attempting copy update to descriptorSet " << set_ << " binding#" << update->dstBinding
                  << " with offset index of " << p_layout_->GetGlobalIndexRangeFromBinding(update->dstBinding).start
                  << " plus update array offset of " << update->dstArrayElement << " and update of " << update->descriptorCount
                  << " descriptors oversteps total number of descriptors in set: " << p_layout_->GetTotalDescriptorCount();
        *error_msg = error_str.str();
        return false;
    }
    // Check that types match
    // TODO : Base default error case going from here is "VUID-VkAcquireNextImageInfoKHR-semaphore-parameter"2ba which covers all
    // consistency issues, need more fine-grained error codes
    *error_code = "VUID-VkCopyDescriptorSet-srcSet-00349";
    auto src_type = src_set->GetTypeFromBinding(update->srcBinding);
    auto dst_type = p_layout_->GetTypeFromBinding(update->dstBinding);
    if (src_type != dst_type) {
        std::stringstream error_str;
        error_str << "Attempting copy update to descriptorSet " << set_ << " binding #" << update->dstBinding << " with type "
                  << string_VkDescriptorType(dst_type) << " from descriptorSet " << src_set->GetSet() << " binding #"
                  << update->srcBinding << " with type " << string_VkDescriptorType(src_type) << ". Types do not match";
        *error_msg = error_str.str();
        return false;
    }
    // Verify consistency of src & dst bindings if update crosses binding boundaries
    if ((!src_set->GetLayout()->VerifyUpdateConsistency(update->srcBinding, update->srcArrayElement, update->descriptorCount,
                                                        "copy update from", src_set->GetSet(), error_msg)) ||
        (!p_layout_->VerifyUpdateConsistency(update->dstBinding, update->dstArrayElement, update->descriptorCount, "copy update to",
                                             set_, error_msg))) {
        return false;
    }

    if ((src_set->GetLayout()->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT) &&
        !(GetLayout()->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT)) {
        *error_code = "VUID-VkCopyDescriptorSet-srcSet-01918";
        std::stringstream error_str;
        error_str << "If pname:srcSet's (" << update->srcSet
                  << ") layout was created with the "
                     "ename:VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT flag "
                     "set, then pname:dstSet's ("
                  << update->dstSet
                  << ") layout must: also have been created with the "
                     "ename:VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT flag set";
        *error_msg = error_str.str();
        return false;
    }

    if (!(src_set->GetLayout()->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT) &&
        (GetLayout()->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT)) {
        *error_code = "VUID-VkCopyDescriptorSet-srcSet-01919";
        std::stringstream error_str;
        error_str << "If pname:srcSet's (" << update->srcSet
                  << ") layout was created without the "
                     "ename:VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT flag "
                     "set, then pname:dstSet's ("
                  << update->dstSet
                  << ") layout must: also have been created without the "
                     "ename:VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT flag set";
        *error_msg = error_str.str();
        return false;
    }

    if ((src_set->GetPoolState()->createInfo.flags & VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT) &&
        !(GetPoolState()->createInfo.flags & VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT)) {
        *error_code = "VUID-VkCopyDescriptorSet-srcSet-01920";
        std::stringstream error_str;
        error_str << "If the descriptor pool from which pname:srcSet (" << update->srcSet
                  << ") was allocated was created "
                     "with the ename:VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT flag "
                     "set, then the descriptor pool from which pname:dstSet ("
                  << update->dstSet
                  << ") was allocated must: "
                     "also have been created with the ename:VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT flag set";
        *error_msg = error_str.str();
        return false;
    }

    if (!(src_set->GetPoolState()->createInfo.flags & VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT) &&
        (GetPoolState()->createInfo.flags & VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT)) {
        *error_code = "VUID-VkCopyDescriptorSet-srcSet-01921";
        std::stringstream error_str;
        error_str << "If the descriptor pool from which pname:srcSet (" << update->srcSet
                  << ") was allocated was created "
                     "without the ename:VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT flag "
                     "set, then the descriptor pool from which pname:dstSet ("
                  << update->dstSet
                  << ") was allocated must: "
                     "also have been created without the ename:VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT flag set";
        *error_msg = error_str.str();
        return false;
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

    // Update parameters all look good and descriptor updated so verify update contents
    if (!VerifyCopyUpdateContents(update, src_set, src_type, src_start_idx, error_code, error_msg)) return false;

    // All checks passed so update is good
    return true;
}
// Perform Copy update
void cvdescriptorset::DescriptorSet::PerformCopyUpdate(const VkCopyDescriptorSet *update, const DescriptorSet *src_set) {
    auto src_start_idx = src_set->GetGlobalIndexRangeFromBinding(update->srcBinding).start + update->srcArrayElement;
    auto dst_start_idx = p_layout_->GetGlobalIndexRangeFromBinding(update->dstBinding).start + update->dstArrayElement;
    // Update parameters all look good so perform update
    for (uint32_t di = 0; di < update->descriptorCount; ++di) {
        auto src = src_set->descriptors_[src_start_idx + di].get();
        auto dst = descriptors_[dst_start_idx + di].get();
        if (src->updated) {
            dst->CopyUpdate(src);
            some_update_ = true;
        } else {
            dst->updated = false;
        }
    }

    if (!(p_layout_->GetDescriptorBindingFlagsFromBinding(update->dstBinding) &
          (VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT))) {
        InvalidateBoundCmdBuffers();
    }
}

// Update the drawing state for the affected descriptors.
// Set cb_node to this set and this set to cb_node.
// Add the bindings of the descriptor
// Set the layout based on the current descriptor layout (will mask subsequent layer mismatch errors)
// TODO: Modify the UpdateDrawState virtural functions to *only* set initial layout and not change layouts
// Prereq: This should be called for a set that has been confirmed to be active for the given cb_node, meaning it's going
//   to be used in a draw by the given cb_node
void cvdescriptorset::DescriptorSet::UpdateDrawState(GLOBAL_CB_NODE *cb_node,
                                                     const std::map<uint32_t, descriptor_req> &binding_req_map) {
    // bind cb to this descriptor set
    cb_bindings.insert(cb_node);
    // Add bindings for descriptor set, the set's pool, and individual objects in the set
    cb_node->object_bindings.insert({HandleToUint64(set_), kVulkanObjectTypeDescriptorSet});
    pool_state_->cb_bindings.insert(cb_node);
    cb_node->object_bindings.insert({HandleToUint64(pool_state_->pool), kVulkanObjectTypeDescriptorPool});
    // For the active slots, use set# to look up descriptorSet from boundDescriptorSets, and bind all of that descriptor set's
    // resources
    for (auto binding_req_pair : binding_req_map) {
        auto binding = binding_req_pair.first;
        auto range = p_layout_->GetGlobalIndexRangeFromBinding(binding);
        for (uint32_t i = range.start; i < range.end; ++i) {
            descriptors_[i]->UpdateDrawState(device_data_, cb_node);
        }
    }
}

void cvdescriptorset::DescriptorSet::FilterAndTrackOneBindingReq(const BindingReqMap::value_type &binding_req_pair,
                                                                 const BindingReqMap &in_req, BindingReqMap *out_req,
                                                                 TrackedBindings *bindings) {
    assert(out_req);
    assert(bindings);
    const auto binding = binding_req_pair.first;
    // Use insert and look at the boolean ("was inserted") in the returned pair to see if this is a new set member.
    // Saves one hash lookup vs. find ... compare w/ end ... insert.
    const auto it_bool_pair = bindings->insert(binding);
    if (it_bool_pair.second) {
        out_req->emplace(binding_req_pair);
    }
}

void cvdescriptorset::DescriptorSet::FilterAndTrackOneBindingReq(const BindingReqMap::value_type &binding_req_pair,
                                                                 const BindingReqMap &in_req, BindingReqMap *out_req,
                                                                 TrackedBindings *bindings, uint32_t limit) {
    if (bindings->size() < limit) FilterAndTrackOneBindingReq(binding_req_pair, in_req, out_req, bindings);
}

void cvdescriptorset::DescriptorSet::FilterAndTrackBindingReqs(GLOBAL_CB_NODE *cb_state, const BindingReqMap &in_req,
                                                               BindingReqMap *out_req) {
    TrackedBindings &bound = cached_validation_[cb_state].command_binding_and_usage;
    if (bound.size() == GetBindingCount()) {
        return;  // All bindings are bound, out req is empty
    }
    for (const auto &binding_req_pair : in_req) {
        const auto binding = binding_req_pair.first;
        // If a binding doesn't exist, or has already been bound, skip it
        if (p_layout_->HasBinding(binding)) {
            FilterAndTrackOneBindingReq(binding_req_pair, in_req, out_req, &bound);
        }
    }
}

void cvdescriptorset::DescriptorSet::FilterAndTrackBindingReqs(GLOBAL_CB_NODE *cb_state, PIPELINE_STATE *pipeline,
                                                               const BindingReqMap &in_req, BindingReqMap *out_req) {
    auto &validated = cached_validation_[cb_state];
    auto &image_sample_val = validated.image_samplers[pipeline];
    auto *const dynamic_buffers = &validated.dynamic_buffers;
    auto *const non_dynamic_buffers = &validated.non_dynamic_buffers;
    const auto &stats = p_layout_->GetBindingTypeStats();
    for (const auto &binding_req_pair : in_req) {
        auto binding = binding_req_pair.first;
        VkDescriptorSetLayoutBinding const *layout_binding = p_layout_->GetDescriptorSetLayoutBindingPtrFromBinding(binding);
        if (!layout_binding) {
            continue;
        }
        // Caching criteria differs per type.
        // If image_layout have changed , the image descriptors need to be validated against them.
        if ((layout_binding->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) ||
            (layout_binding->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)) {
            FilterAndTrackOneBindingReq(binding_req_pair, in_req, out_req, dynamic_buffers, stats.dynamic_buffer_count);
        } else if ((layout_binding->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) ||
                   (layout_binding->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)) {
            FilterAndTrackOneBindingReq(binding_req_pair, in_req, out_req, non_dynamic_buffers, stats.non_dynamic_buffer_count);
        } else {
            // This is rather crude, as the changed layouts may not impact the bound descriptors,
            // but the simple "versioning" is a simple "dirt" test.
            auto &version = image_sample_val[binding];  // Take advantage of default construtor zero initialzing new entries
            if (version != cb_state->image_layout_change_count) {
                version = cb_state->image_layout_change_count;
                out_req->emplace(binding_req_pair);
            }
        }
    }
}

cvdescriptorset::SamplerDescriptor::SamplerDescriptor(const VkSampler *immut) : sampler_(VK_NULL_HANDLE), immutable_(false) {
    updated = false;
    descriptor_class = PlainSampler;
    if (immut) {
        sampler_ = *immut;
        immutable_ = true;
        updated = true;
    }
}
// Validate given sampler. Currently this only checks to make sure it exists in the samplerMap
bool cvdescriptorset::ValidateSampler(const VkSampler sampler, const layer_data *dev_data) {
    return (GetSamplerState(dev_data, sampler) != nullptr);
}

bool cvdescriptorset::ValidateImageUpdate(VkImageView image_view, VkImageLayout image_layout, VkDescriptorType type,
                                          const layer_data *dev_data, std::string *error_code, std::string *error_msg) {
    // TODO : Defaulting to 00943 for all cases here. Need to create new error codes for various cases.
    *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00326";
    auto iv_state = GetImageViewState(dev_data, image_view);
    if (!iv_state) {
        std::stringstream error_str;
        error_str << "Invalid VkImageView: " << image_view;
        *error_msg = error_str.str();
        return false;
    }
    // Note that when an imageview is created, we validated that memory is bound so no need to re-check here
    // Validate that imageLayout is compatible with aspect_mask and image format
    //  and validate that image usage bits are correct for given usage
    VkImageAspectFlags aspect_mask = iv_state->create_info.subresourceRange.aspectMask;
    VkImage image = iv_state->create_info.image;
    VkFormat format = VK_FORMAT_MAX_ENUM;
    VkImageUsageFlags usage = 0;
    auto image_node = GetImageState(dev_data, image);
    if (image_node) {
        format = image_node->createInfo.format;
        usage = image_node->createInfo.usage;
        // Validate that memory is bound to image
        // TODO: This should have its own valid usage id apart from 2524 which is from CreateImageView case. The only
        //  the error here occurs is if memory bound to a created imageView has been freed.
        if (ValidateMemoryIsBoundToImage(dev_data, image_node, "vkUpdateDescriptorSets()",
                                         "VUID-VkImageViewCreateInfo-image-01020")) {
            *error_code = "VUID-VkImageViewCreateInfo-image-01020";
            *error_msg = "No memory bound to image.";
            return false;
        }

        // KHR_maintenance1 allows rendering into 2D or 2DArray views which slice a 3D image,
        // but not binding them to descriptor sets.
        if (image_node->createInfo.imageType == VK_IMAGE_TYPE_3D &&
            (iv_state->create_info.viewType == VK_IMAGE_VIEW_TYPE_2D ||
             iv_state->create_info.viewType == VK_IMAGE_VIEW_TYPE_2D_ARRAY)) {
            *error_code = "VUID-VkDescriptorImageInfo-imageView-00343";
            *error_msg = "ImageView must not be a 2D or 2DArray view of a 3D image";
            return false;
        }
    }
    // First validate that format and layout are compatible
    if (format == VK_FORMAT_MAX_ENUM) {
        std::stringstream error_str;
        error_str << "Invalid image (" << image << ") in imageView (" << image_view << ").";
        *error_msg = error_str.str();
        return false;
    }
    // TODO : The various image aspect and format checks here are based on general spec language in 11.5 Image Views section under
    // vkCreateImageView(). What's the best way to create unique id for these cases?
    bool ds = FormatIsDepthOrStencil(format);
    switch (image_layout) {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            // Only Color bit must be set
            if ((aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) != VK_IMAGE_ASPECT_COLOR_BIT) {
                std::stringstream error_str;
                error_str
                    << "ImageView (" << image_view
                    << ") uses layout VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL but does not have VK_IMAGE_ASPECT_COLOR_BIT set.";
                *error_msg = error_str.str();
                return false;
            }
            // format must NOT be DS
            if (ds) {
                std::stringstream error_str;
                error_str << "ImageView (" << image_view
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
                    error_str << "ImageView (" << image_view << ") has both STENCIL and DEPTH aspects set";
                    *error_msg = error_str.str();
                    return false;
                }
            } else if (!(aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT)) {
                // Neither were set
                std::stringstream error_str;
                error_str << "ImageView (" << image_view << ") has layout " << string_VkImageLayout(image_layout)
                          << " but does not have STENCIL or DEPTH aspects set";
                *error_msg = error_str.str();
                return false;
            }
            // format must be DS
            if (!ds) {
                std::stringstream error_str;
                error_str << "ImageView (" << image_view << ") has layout " << string_VkImageLayout(image_layout)
                          << " but the image format is " << string_VkFormat(format) << " which is not a depth/stencil format.";
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
                        error_str << "ImageView (" << image_view << ") has layout " << string_VkImageLayout(image_layout)
                                  << " and is using depth/stencil image of format " << string_VkFormat(format)
                                  << " but it has both STENCIL and DEPTH aspects set, which is illegal. When using a depth/stencil "
                                     "image in a descriptor set, please only set either VK_IMAGE_ASPECT_DEPTH_BIT or "
                                     "VK_IMAGE_ASPECT_STENCIL_BIT depending on whether it will be used for depth reads or stencil "
                                     "reads respectively.";
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
    // TODO : Need to also validate case "VUID-VkWriteDescriptorSet-descriptorType-00336" where STORAGE_IMAGE & INPUT_ATTACH types
    // must have been created with identify swizzle
    std::string error_usage_bit;
    switch (type) {
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
            if (!(usage & VK_IMAGE_USAGE_SAMPLED_BIT)) {
                error_usage_bit = "VK_IMAGE_USAGE_SAMPLED_BIT";
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: {
            if (!(usage & VK_IMAGE_USAGE_STORAGE_BIT)) {
                error_usage_bit = "VK_IMAGE_USAGE_STORAGE_BIT";
            } else if (VK_IMAGE_LAYOUT_GENERAL != image_layout) {
                std::stringstream error_str;
                // TODO : Need to create custom enum error codes for these cases
                if (image_node->shared_presentable) {
                    if (VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR != image_layout) {
                        error_str << "ImageView (" << image_view
                                  << ") of VK_DESCRIPTOR_TYPE_STORAGE_IMAGE type with a front-buffered image is being updated with "
                                     "layout "
                                  << string_VkImageLayout(image_layout)
                                  << " but according to spec section 13.1 Descriptor Types, 'Front-buffered images that report "
                                     "support for VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT must be in the "
                                     "VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR layout.'";
                        *error_msg = error_str.str();
                        return false;
                    }
                } else if (VK_IMAGE_LAYOUT_GENERAL != image_layout) {
                    error_str << "ImageView (" << image_view
                              << ") of VK_DESCRIPTOR_TYPE_STORAGE_IMAGE type is being updated with layout "
                              << string_VkImageLayout(image_layout)
                              << " but according to spec section 13.1 Descriptor Types, 'Load and store operations on storage "
                                 "images can only be done on images in VK_IMAGE_LAYOUT_GENERAL layout.'";
                    *error_msg = error_str.str();
                    return false;
                }
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: {
            if (!(usage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) {
                error_usage_bit = "VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT";
            }
            break;
        }
        default:
            break;
    }
    if (!error_usage_bit.empty()) {
        std::stringstream error_str;
        error_str << "ImageView (" << image_view << ") with usage mask 0x" << usage
                  << " being used for a descriptor update of type " << string_VkDescriptorType(type) << " does not have "
                  << error_usage_bit << " set.";
        *error_msg = error_str.str();
        return false;
    }
    return true;
}

void cvdescriptorset::SamplerDescriptor::WriteUpdate(const VkWriteDescriptorSet *update, const uint32_t index) {
    if (!immutable_) {
        sampler_ = update->pImageInfo[index].sampler;
    }
    updated = true;
}

void cvdescriptorset::SamplerDescriptor::CopyUpdate(const Descriptor *src) {
    if (!immutable_) {
        auto update_sampler = static_cast<const SamplerDescriptor *>(src)->sampler_;
        sampler_ = update_sampler;
    }
    updated = true;
}

void cvdescriptorset::SamplerDescriptor::UpdateDrawState(layer_data *dev_data, GLOBAL_CB_NODE *cb_node) {
    if (!immutable_) {
        auto sampler_state = GetSamplerState(dev_data, sampler_);
        if (sampler_state) core_validation::AddCommandBufferBindingSampler(cb_node, sampler_state);
    }
}

cvdescriptorset::ImageSamplerDescriptor::ImageSamplerDescriptor(const VkSampler *immut)
    : sampler_(VK_NULL_HANDLE), immutable_(false), image_view_(VK_NULL_HANDLE), image_layout_(VK_IMAGE_LAYOUT_UNDEFINED) {
    updated = false;
    descriptor_class = ImageSampler;
    if (immut) {
        sampler_ = *immut;
        immutable_ = true;
    }
}

void cvdescriptorset::ImageSamplerDescriptor::WriteUpdate(const VkWriteDescriptorSet *update, const uint32_t index) {
    updated = true;
    const auto &image_info = update->pImageInfo[index];
    if (!immutable_) {
        sampler_ = image_info.sampler;
    }
    image_view_ = image_info.imageView;
    image_layout_ = image_info.imageLayout;
}

void cvdescriptorset::ImageSamplerDescriptor::CopyUpdate(const Descriptor *src) {
    if (!immutable_) {
        auto update_sampler = static_cast<const ImageSamplerDescriptor *>(src)->sampler_;
        sampler_ = update_sampler;
    }
    auto image_view = static_cast<const ImageSamplerDescriptor *>(src)->image_view_;
    auto image_layout = static_cast<const ImageSamplerDescriptor *>(src)->image_layout_;
    updated = true;
    image_view_ = image_view;
    image_layout_ = image_layout;
}

void cvdescriptorset::ImageSamplerDescriptor::UpdateDrawState(layer_data *dev_data, GLOBAL_CB_NODE *cb_node) {
    // First add binding for any non-immutable sampler
    if (!immutable_) {
        auto sampler_state = GetSamplerState(dev_data, sampler_);
        if (sampler_state) core_validation::AddCommandBufferBindingSampler(cb_node, sampler_state);
    }
    // Add binding for image
    auto iv_state = GetImageViewState(dev_data, image_view_);
    if (iv_state) {
        core_validation::AddCommandBufferBindingImageView(dev_data, cb_node, iv_state);
    }
    SetImageViewLayout(dev_data, cb_node, image_view_, image_layout_);
}

cvdescriptorset::ImageDescriptor::ImageDescriptor(const VkDescriptorType type)
    : storage_(false), image_view_(VK_NULL_HANDLE), image_layout_(VK_IMAGE_LAYOUT_UNDEFINED) {
    updated = false;
    descriptor_class = Image;
    if (VK_DESCRIPTOR_TYPE_STORAGE_IMAGE == type) storage_ = true;
}

void cvdescriptorset::ImageDescriptor::WriteUpdate(const VkWriteDescriptorSet *update, const uint32_t index) {
    updated = true;
    const auto &image_info = update->pImageInfo[index];
    image_view_ = image_info.imageView;
    image_layout_ = image_info.imageLayout;
}

void cvdescriptorset::ImageDescriptor::CopyUpdate(const Descriptor *src) {
    auto image_view = static_cast<const ImageDescriptor *>(src)->image_view_;
    auto image_layout = static_cast<const ImageDescriptor *>(src)->image_layout_;
    updated = true;
    image_view_ = image_view;
    image_layout_ = image_layout;
}

void cvdescriptorset::ImageDescriptor::UpdateDrawState(layer_data *dev_data, GLOBAL_CB_NODE *cb_node) {
    // Add binding for image
    auto iv_state = GetImageViewState(dev_data, image_view_);
    if (iv_state) {
        core_validation::AddCommandBufferBindingImageView(dev_data, cb_node, iv_state);
    }
    SetImageViewLayout(dev_data, cb_node, image_view_, image_layout_);
}

cvdescriptorset::BufferDescriptor::BufferDescriptor(const VkDescriptorType type)
    : storage_(false), dynamic_(false), buffer_(VK_NULL_HANDLE), offset_(0), range_(0) {
    updated = false;
    descriptor_class = GeneralBuffer;
    if (VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC == type) {
        dynamic_ = true;
    } else if (VK_DESCRIPTOR_TYPE_STORAGE_BUFFER == type) {
        storage_ = true;
    } else if (VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC == type) {
        dynamic_ = true;
        storage_ = true;
    }
}
void cvdescriptorset::BufferDescriptor::WriteUpdate(const VkWriteDescriptorSet *update, const uint32_t index) {
    updated = true;
    const auto &buffer_info = update->pBufferInfo[index];
    buffer_ = buffer_info.buffer;
    offset_ = buffer_info.offset;
    range_ = buffer_info.range;
}

void cvdescriptorset::BufferDescriptor::CopyUpdate(const Descriptor *src) {
    auto buff_desc = static_cast<const BufferDescriptor *>(src);
    updated = true;
    buffer_ = buff_desc->buffer_;
    offset_ = buff_desc->offset_;
    range_ = buff_desc->range_;
}

void cvdescriptorset::BufferDescriptor::UpdateDrawState(layer_data *dev_data, GLOBAL_CB_NODE *cb_node) {
    auto buffer_node = GetBufferState(dev_data, buffer_);
    if (buffer_node) core_validation::AddCommandBufferBindingBuffer(dev_data, cb_node, buffer_node);
}

cvdescriptorset::TexelDescriptor::TexelDescriptor(const VkDescriptorType type) : buffer_view_(VK_NULL_HANDLE), storage_(false) {
    updated = false;
    descriptor_class = TexelBuffer;
    if (VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER == type) storage_ = true;
}

void cvdescriptorset::TexelDescriptor::WriteUpdate(const VkWriteDescriptorSet *update, const uint32_t index) {
    updated = true;
    buffer_view_ = update->pTexelBufferView[index];
}

void cvdescriptorset::TexelDescriptor::CopyUpdate(const Descriptor *src) {
    updated = true;
    buffer_view_ = static_cast<const TexelDescriptor *>(src)->buffer_view_;
}

void cvdescriptorset::TexelDescriptor::UpdateDrawState(layer_data *dev_data, GLOBAL_CB_NODE *cb_node) {
    auto bv_state = GetBufferViewState(dev_data, buffer_view_);
    if (bv_state) {
        core_validation::AddCommandBufferBindingBufferView(dev_data, cb_node, bv_state);
    }
}

// This is a helper function that iterates over a set of Write and Copy updates, pulls the DescriptorSet* for updated
//  sets, and then calls their respective Validate[Write|Copy]Update functions.
// If the update hits an issue for which the callback returns "true", meaning that the call down the chain should
//  be skipped, then true is returned.
// If there is no issue with the update, then false is returned.
bool cvdescriptorset::ValidateUpdateDescriptorSets(const debug_report_data *report_data, const layer_data *dev_data,
                                                   uint32_t write_count, const VkWriteDescriptorSet *p_wds, uint32_t copy_count,
                                                   const VkCopyDescriptorSet *p_cds) {
    bool skip = false;
    // Validate Write updates
    for (uint32_t i = 0; i < write_count; i++) {
        auto dest_set = p_wds[i].dstSet;
        auto set_node = core_validation::GetSetNode(dev_data, dest_set);
        if (!set_node) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT,
                        HandleToUint64(dest_set), kVUID_Core_DrawState_InvalidDescriptorSet,
                        "Cannot call vkUpdateDescriptorSets() on descriptor set 0x%" PRIxLEAST64 " that has not been allocated.",
                        HandleToUint64(dest_set));
        } else {
            std::string error_code;
            std::string error_str;
            if (!set_node->ValidateWriteUpdate(report_data, &p_wds[i], &error_code, &error_str)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT,
                                HandleToUint64(dest_set), error_code,
                                "vkUpdateDescriptorSets() failed write update validation for Descriptor Set 0x%" PRIx64
                                " with error: %s.",
                                HandleToUint64(dest_set), error_str.c_str());
            }
        }
    }
    // Now validate copy updates
    for (uint32_t i = 0; i < copy_count; ++i) {
        auto dst_set = p_cds[i].dstSet;
        auto src_set = p_cds[i].srcSet;
        auto src_node = core_validation::GetSetNode(dev_data, src_set);
        auto dst_node = core_validation::GetSetNode(dev_data, dst_set);
        // Object_tracker verifies that src & dest descriptor set are valid
        assert(src_node);
        assert(dst_node);
        std::string error_code;
        std::string error_str;
        if (!dst_node->ValidateCopyUpdate(report_data, &p_cds[i], src_node, &error_code, &error_str)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT,
                            HandleToUint64(dst_set), error_code,
                            "vkUpdateDescriptorSets() failed copy update from Descriptor Set 0x%" PRIx64
                            " to Descriptor Set 0x%" PRIx64 " with error: %s.",
                            HandleToUint64(src_set), HandleToUint64(dst_set), error_str.c_str());
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
void cvdescriptorset::PerformUpdateDescriptorSets(const layer_data *dev_data, uint32_t write_count,
                                                  const VkWriteDescriptorSet *p_wds, uint32_t copy_count,
                                                  const VkCopyDescriptorSet *p_cds) {
    // Write updates first
    uint32_t i = 0;
    for (i = 0; i < write_count; ++i) {
        auto dest_set = p_wds[i].dstSet;
        auto set_node = core_validation::GetSetNode(dev_data, dest_set);
        if (set_node) {
            set_node->PerformWriteUpdate(&p_wds[i]);
        }
    }
    // Now copy updates
    for (i = 0; i < copy_count; ++i) {
        auto dst_set = p_cds[i].dstSet;
        auto src_set = p_cds[i].srcSet;
        auto src_node = core_validation::GetSetNode(dev_data, src_set);
        auto dst_node = core_validation::GetSetNode(dev_data, dst_set);
        if (src_node && dst_node) {
            dst_node->PerformCopyUpdate(&p_cds[i], src_node);
        }
    }
}
// This helper function carries out the state updates for descriptor updates peformed via update templates. It basically collects
// data and leverages the PerformUpdateDescriptor helper functions to do this.
void cvdescriptorset::PerformUpdateDescriptorSetsWithTemplateKHR(layer_data *device_data, VkDescriptorSet descriptorSet,
                                                                 std::unique_ptr<TEMPLATE_STATE> const &template_state,
                                                                 const void *pData) {
    auto const &create_info = template_state->create_info;

    // Create a vector of write structs
    std::vector<VkWriteDescriptorSet> desc_writes;
    std::vector<VkWriteDescriptorSetInlineUniformBlockEXT> inline_infos(create_info.descriptorUpdateEntryCount);
    auto layout_obj = GetDescriptorSetLayout(device_data, create_info.descriptorSetLayout);

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
                    // skip the rest of the array, they just represent bytes in the update
                    j = create_info.pDescriptorUpdateEntries[i].descriptorCount;
                    break;
                }
                default:
                    assert(0);
                    break;
            }
            dst_array_element++;
        }
    }
    PerformUpdateDescriptorSets(device_data, static_cast<uint32_t>(desc_writes.size()), desc_writes.data(), 0, NULL);
}
// Validate the state for a given write update but don't actually perform the update
//  If an error would occur for this update, return false and fill in details in error_msg string
bool cvdescriptorset::DescriptorSet::ValidateWriteUpdate(const debug_report_data *report_data, const VkWriteDescriptorSet *update,
                                                         std::string *error_code, std::string *error_msg) {
    // Verify dst layout still valid
    if (p_layout_->IsDestroyed()) {
        *error_code = "VUID-VkWriteDescriptorSet-dstSet-00320";
        string_sprintf(error_msg,
                       "Cannot call vkUpdateDescriptorSets() to perform write update on descriptor set 0x%" PRIxLEAST64
                       " created with destroyed VkDescriptorSetLayout 0x%" PRIxLEAST64,
                       HandleToUint64(set_), HandleToUint64(p_layout_->GetDescriptorSetLayout()));
        return false;
    }
    // Verify dst binding exists
    if (!p_layout_->HasBinding(update->dstBinding)) {
        *error_code = "VUID-VkWriteDescriptorSet-dstBinding-00315";
        std::stringstream error_str;
        error_str << "DescriptorSet " << set_ << " does not have binding " << update->dstBinding;
        *error_msg = error_str.str();
        return false;
    } else {
        // Make sure binding isn't empty
        if (0 == p_layout_->GetDescriptorCountFromBinding(update->dstBinding)) {
            *error_code = "VUID-VkWriteDescriptorSet-dstBinding-00316";
            std::stringstream error_str;
            error_str << "DescriptorSet " << set_ << " cannot updated binding " << update->dstBinding << " that has 0 descriptors";
            *error_msg = error_str.str();
            return false;
        }
    }
    // Verify idle ds
    if (in_use.load() &&
        !(p_layout_->GetDescriptorBindingFlagsFromBinding(update->dstBinding) &
          (VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT))) {
        // TODO : Re-using Free Idle error code, need write update idle error code
        *error_code = "VUID-vkFreeDescriptorSets-pDescriptorSets-00309";
        std::stringstream error_str;
        error_str << "Cannot call vkUpdateDescriptorSets() to perform write update on descriptor set " << set_
                  << " that is in use by a command buffer";
        *error_msg = error_str.str();
        return false;
    }
    // We know that binding is valid, verify update and do update on each descriptor
    auto start_idx = p_layout_->GetGlobalIndexRangeFromBinding(update->dstBinding).start + update->dstArrayElement;
    auto type = p_layout_->GetTypeFromBinding(update->dstBinding);
    if (type != update->descriptorType) {
        *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00319";
        std::stringstream error_str;
        error_str << "Attempting write update to descriptor set " << set_ << " binding #" << update->dstBinding << " with type "
                  << string_VkDescriptorType(type) << " but update type is " << string_VkDescriptorType(update->descriptorType);
        *error_msg = error_str.str();
        return false;
    }
    if (update->descriptorCount > (descriptors_.size() - start_idx)) {
        *error_code = "VUID-VkWriteDescriptorSet-dstArrayElement-00321";
        std::stringstream error_str;
        error_str << "Attempting write update to descriptor set " << set_ << " binding #" << update->dstBinding << " with "
                  << descriptors_.size() - start_idx
                  << " descriptors in that binding and all successive bindings of the set, but update of "
                  << update->descriptorCount << " descriptors combined with update array element offset of "
                  << update->dstArrayElement << " oversteps the available number of consecutive descriptors";
        *error_msg = error_str.str();
        return false;
    }
    if (type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT) {
        if ((update->dstArrayElement % 4) != 0) {
            *error_code = "VUID-VkWriteDescriptorSet-descriptorType-02219";
            std::stringstream error_str;
            error_str << "Attempting write update to descriptor set " << set_ << " binding #" << update->dstBinding << " with "
                      << "dstArrayElement " << update->dstArrayElement << " not a multiple of 4";
            *error_msg = error_str.str();
            return false;
        }
        if ((update->descriptorCount % 4) != 0) {
            *error_code = "VUID-VkWriteDescriptorSet-descriptorType-02220";
            std::stringstream error_str;
            error_str << "Attempting write update to descriptor set " << set_ << " binding #" << update->dstBinding << " with "
                      << "descriptorCount  " << update->descriptorCount << " not a multiple of 4";
            *error_msg = error_str.str();
            return false;
        }
        const auto *write_inline_info = lvl_find_in_chain<VkWriteDescriptorSetInlineUniformBlockEXT>(update->pNext);
        if (!write_inline_info || write_inline_info->dataSize != update->descriptorCount) {
            *error_code = "VUID-VkWriteDescriptorSet-descriptorType-02221";
            std::stringstream error_str;
            if (!write_inline_info) {
                error_str << "Attempting write update to descriptor set " << set_ << " binding #" << update->dstBinding << " with "
                          << "VkWriteDescriptorSetInlineUniformBlockEXT missing";
            } else {
                error_str << "Attempting write update to descriptor set " << set_ << " binding #" << update->dstBinding << " with "
                          << "VkWriteDescriptorSetInlineUniformBlockEXT dataSize " << write_inline_info->dataSize
                          << " not equal to "
                          << "VkWriteDescriptorSet descriptorCount " << update->descriptorCount;
            }
            *error_msg = error_str.str();
            return false;
        }
        // This error is probably unreachable due to the previous two errors
        if (write_inline_info && (write_inline_info->dataSize % 4) != 0) {
            *error_code = "VUID-VkWriteDescriptorSetInlineUniformBlockEXT-dataSize-02222";
            std::stringstream error_str;
            error_str << "Attempting write update to descriptor set " << set_ << " binding #" << update->dstBinding << " with "
                      << "VkWriteDescriptorSetInlineUniformBlockEXT dataSize " << write_inline_info->dataSize
                      << " not a multiple of 4";
            *error_msg = error_str.str();
            return false;
        }
    }
    // Verify consecutive bindings match (if needed)
    if (!p_layout_->VerifyUpdateConsistency(update->dstBinding, update->dstArrayElement, update->descriptorCount, "write update to",
                                            set_, error_msg)) {
        // TODO : Should break out "consecutive binding updates" language into valid usage statements
        *error_code = "VUID-VkWriteDescriptorSet-dstArrayElement-00321";
        return false;
    }
    // Update is within bounds and consistent so last step is to validate update contents
    if (!VerifyWriteUpdateContents(update, start_idx, error_code, error_msg)) {
        std::stringstream error_str;
        error_str << "Write update to descriptor in set " << set_ << " binding #" << update->dstBinding
                  << " failed with error message: " << error_msg->c_str();
        *error_msg = error_str.str();
        return false;
    }
    // All checks passed, update is clean
    return true;
}
// For the given buffer, verify that its creation parameters are appropriate for the given type
//  If there's an error, update the error_msg string with details and return false, else return true
bool cvdescriptorset::DescriptorSet::ValidateBufferUsage(BUFFER_STATE const *buffer_node, VkDescriptorType type,
                                                         std::string *error_code, std::string *error_msg) const {
    // Verify that usage bits set correctly for given type
    auto usage = buffer_node->createInfo.usage;
    std::string error_usage_bit;
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
    if (!error_usage_bit.empty()) {
        std::stringstream error_str;
        error_str << "Buffer (" << buffer_node->buffer << ") with usage mask 0x" << usage
                  << " being used for a descriptor update of type " << string_VkDescriptorType(type) << " does not have "
                  << error_usage_bit << " set.";
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
bool cvdescriptorset::DescriptorSet::ValidateBufferUpdate(VkDescriptorBufferInfo const *buffer_info, VkDescriptorType type,
                                                          std::string *error_code, std::string *error_msg) const {
    // First make sure that buffer is valid
    auto buffer_node = GetBufferState(device_data_, buffer_info->buffer);
    // Any invalid buffer should already be caught by object_tracker
    assert(buffer_node);
    if (ValidateMemoryIsBoundToBuffer(device_data_, buffer_node, "vkUpdateDescriptorSets()",
                                      "VUID-VkWriteDescriptorSet-descriptorType-00329")) {
        *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00329";
        *error_msg = "No memory bound to buffer.";
        return false;
    }
    // Verify usage bits
    if (!ValidateBufferUsage(buffer_node, type, error_code, error_msg)) {
        // error_msg will have been updated by ValidateBufferUsage()
        return false;
    }
    // offset must be less than buffer size
    if (buffer_info->offset >= buffer_node->createInfo.size) {
        *error_code = "VUID-VkDescriptorBufferInfo-offset-00340";
        std::stringstream error_str;
        error_str << "VkDescriptorBufferInfo offset of " << buffer_info->offset << " is greater than or equal to buffer "
                  << buffer_node->buffer << " size of " << buffer_node->createInfo.size;
        *error_msg = error_str.str();
        return false;
    }
    if (buffer_info->range != VK_WHOLE_SIZE) {
        // Range must be VK_WHOLE_SIZE or > 0
        if (!buffer_info->range) {
            *error_code = "VUID-VkDescriptorBufferInfo-range-00341";
            std::stringstream error_str;
            error_str << "VkDescriptorBufferInfo range is not VK_WHOLE_SIZE and is zero, which is not allowed.";
            *error_msg = error_str.str();
            return false;
        }
        // Range must be VK_WHOLE_SIZE or <= (buffer size - offset)
        if (buffer_info->range > (buffer_node->createInfo.size - buffer_info->offset)) {
            *error_code = "VUID-VkDescriptorBufferInfo-range-00342";
            std::stringstream error_str;
            error_str << "VkDescriptorBufferInfo range is " << buffer_info->range << " which is greater than buffer size ("
                      << buffer_node->createInfo.size << ") minus requested offset of " << buffer_info->offset;
            *error_msg = error_str.str();
            return false;
        }
    }
    // Check buffer update sizes against device limits
    if (VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER == type || VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC == type) {
        auto max_ub_range = limits_.maxUniformBufferRange;
        if (buffer_info->range != VK_WHOLE_SIZE && buffer_info->range > max_ub_range) {
            *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00332";
            std::stringstream error_str;
            error_str << "VkDescriptorBufferInfo range is " << buffer_info->range
                      << " which is greater than this device's maxUniformBufferRange (" << max_ub_range << ")";
            *error_msg = error_str.str();
            return false;
        } else if (buffer_info->range == VK_WHOLE_SIZE && (buffer_node->createInfo.size - buffer_info->offset) > max_ub_range) {
            *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00332";
            std::stringstream error_str;
            error_str << "VkDescriptorBufferInfo range is VK_WHOLE_SIZE but effective range "
                      << "(" << (buffer_node->createInfo.size - buffer_info->offset) << ") is greater than this device's "
                      << "maxUniformBufferRange (" << max_ub_range << ")";
            *error_msg = error_str.str();
            return false;
        }
    } else if (VK_DESCRIPTOR_TYPE_STORAGE_BUFFER == type || VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC == type) {
        auto max_sb_range = limits_.maxStorageBufferRange;
        if (buffer_info->range != VK_WHOLE_SIZE && buffer_info->range > max_sb_range) {
            *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00333";
            std::stringstream error_str;
            error_str << "VkDescriptorBufferInfo range is " << buffer_info->range
                      << " which is greater than this device's maxStorageBufferRange (" << max_sb_range << ")";
            *error_msg = error_str.str();
            return false;
        } else if (buffer_info->range == VK_WHOLE_SIZE && (buffer_node->createInfo.size - buffer_info->offset) > max_sb_range) {
            *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00333";
            std::stringstream error_str;
            error_str << "VkDescriptorBufferInfo range is VK_WHOLE_SIZE but effective range "
                      << "(" << (buffer_node->createInfo.size - buffer_info->offset) << ") is greater than this device's "
                      << "maxStorageBufferRange (" << max_sb_range << ")";
            *error_msg = error_str.str();
            return false;
        }
    }
    return true;
}

// Verify that the contents of the update are ok, but don't perform actual update
bool cvdescriptorset::DescriptorSet::VerifyWriteUpdateContents(const VkWriteDescriptorSet *update, const uint32_t index,
                                                               std::string *error_code, std::string *error_msg) const {
    switch (update->descriptorType) {
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
            for (uint32_t di = 0; di < update->descriptorCount; ++di) {
                // Validate image
                auto image_view = update->pImageInfo[di].imageView;
                auto image_layout = update->pImageInfo[di].imageLayout;
                if (!ValidateImageUpdate(image_view, image_layout, update->descriptorType, device_data_, error_code, error_msg)) {
                    std::stringstream error_str;
                    error_str << "Attempted write update to combined image sampler descriptor failed due to: "
                              << error_msg->c_str();
                    *error_msg = error_str.str();
                    return false;
                }
            }
        }
        // fall through
        case VK_DESCRIPTOR_TYPE_SAMPLER: {
            for (uint32_t di = 0; di < update->descriptorCount; ++di) {
                if (!descriptors_[index + di].get()->IsImmutableSampler()) {
                    if (!ValidateSampler(update->pImageInfo[di].sampler, device_data_)) {
                        *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00325";
                        std::stringstream error_str;
                        error_str << "Attempted write update to sampler descriptor with invalid sampler: "
                                  << update->pImageInfo[di].sampler << ".";
                        *error_msg = error_str.str();
                        return false;
                    }
                } else {
                    // TODO : Warn here
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
                if (!ValidateImageUpdate(image_view, image_layout, update->descriptorType, device_data_, error_code, error_msg)) {
                    std::stringstream error_str;
                    error_str << "Attempted write update to image descriptor failed due to: " << error_msg->c_str();
                    *error_msg = error_str.str();
                    return false;
                }
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: {
            for (uint32_t di = 0; di < update->descriptorCount; ++di) {
                auto buffer_view = update->pTexelBufferView[di];
                auto bv_state = GetBufferViewState(device_data_, buffer_view);
                if (!bv_state) {
                    *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00323";
                    std::stringstream error_str;
                    error_str << "Attempted write update to texel buffer descriptor with invalid buffer view: " << buffer_view;
                    *error_msg = error_str.str();
                    return false;
                }
                auto buffer = bv_state->create_info.buffer;
                auto buffer_state = GetBufferState(device_data_, buffer);
                // Verify that buffer underlying the view hasn't been destroyed prematurely
                if (!buffer_state) {
                    *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00323";
                    std::stringstream error_str;
                    error_str << "Attempted write update to texel buffer descriptor failed because underlying buffer (" << buffer
                              << ") has been destroyed: " << error_msg->c_str();
                    *error_msg = error_str.str();
                    return false;
                } else if (!ValidateBufferUsage(buffer_state, update->descriptorType, error_code, error_msg)) {
                    std::stringstream error_str;
                    error_str << "Attempted write update to texel buffer descriptor failed due to: " << error_msg->c_str();
                    *error_msg = error_str.str();
                    return false;
                }
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
            for (uint32_t di = 0; di < update->descriptorCount; ++di) {
                if (!ValidateBufferUpdate(update->pBufferInfo + di, update->descriptorType, error_code, error_msg)) {
                    std::stringstream error_str;
                    error_str << "Attempted write update to buffer descriptor failed due to: " << error_msg->c_str();
                    *error_msg = error_str.str();
                    return false;
                }
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT:
            break;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NVX:
            // XXX TODO
            break;
        default:
            assert(0);  // We've already verified update type so should never get here
            break;
    }
    // All checks passed so update contents are good
    return true;
}
// Verify that the contents of the update are ok, but don't perform actual update
bool cvdescriptorset::DescriptorSet::VerifyCopyUpdateContents(const VkCopyDescriptorSet *update, const DescriptorSet *src_set,
                                                              VkDescriptorType type, uint32_t index, std::string *error_code,
                                                              std::string *error_msg) const {
    // Note : Repurposing some Write update error codes here as specific details aren't called out for copy updates like they are
    // for write updates
    switch (src_set->descriptors_[index]->descriptor_class) {
        case PlainSampler: {
            for (uint32_t di = 0; di < update->descriptorCount; ++di) {
                const auto src_desc = src_set->descriptors_[index + di].get();
                if (!src_desc->updated) continue;
                if (!src_desc->IsImmutableSampler()) {
                    auto update_sampler = static_cast<SamplerDescriptor *>(src_desc)->GetSampler();
                    if (!ValidateSampler(update_sampler, device_data_)) {
                        *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00325";
                        std::stringstream error_str;
                        error_str << "Attempted copy update to sampler descriptor with invalid sampler: " << update_sampler << ".";
                        *error_msg = error_str.str();
                        return false;
                    }
                } else {
                    // TODO : Warn here
                }
            }
            break;
        }
        case ImageSampler: {
            for (uint32_t di = 0; di < update->descriptorCount; ++di) {
                const auto src_desc = src_set->descriptors_[index + di].get();
                if (!src_desc->updated) continue;
                auto img_samp_desc = static_cast<const ImageSamplerDescriptor *>(src_desc);
                // First validate sampler
                if (!img_samp_desc->IsImmutableSampler()) {
                    auto update_sampler = img_samp_desc->GetSampler();
                    if (!ValidateSampler(update_sampler, device_data_)) {
                        *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00325";
                        std::stringstream error_str;
                        error_str << "Attempted copy update to sampler descriptor with invalid sampler: " << update_sampler << ".";
                        *error_msg = error_str.str();
                        return false;
                    }
                } else {
                    // TODO : Warn here
                }
                // Validate image
                auto image_view = img_samp_desc->GetImageView();
                auto image_layout = img_samp_desc->GetImageLayout();
                if (!ValidateImageUpdate(image_view, image_layout, type, device_data_, error_code, error_msg)) {
                    std::stringstream error_str;
                    error_str << "Attempted copy update to combined image sampler descriptor failed due to: " << error_msg->c_str();
                    *error_msg = error_str.str();
                    return false;
                }
            }
            break;
        }
        case Image: {
            for (uint32_t di = 0; di < update->descriptorCount; ++di) {
                const auto src_desc = src_set->descriptors_[index + di].get();
                if (!src_desc->updated) continue;
                auto img_desc = static_cast<const ImageDescriptor *>(src_desc);
                auto image_view = img_desc->GetImageView();
                auto image_layout = img_desc->GetImageLayout();
                if (!ValidateImageUpdate(image_view, image_layout, type, device_data_, error_code, error_msg)) {
                    std::stringstream error_str;
                    error_str << "Attempted copy update to image descriptor failed due to: " << error_msg->c_str();
                    *error_msg = error_str.str();
                    return false;
                }
            }
            break;
        }
        case TexelBuffer: {
            for (uint32_t di = 0; di < update->descriptorCount; ++di) {
                const auto src_desc = src_set->descriptors_[index + di].get();
                if (!src_desc->updated) continue;
                auto buffer_view = static_cast<TexelDescriptor *>(src_desc)->GetBufferView();
                auto bv_state = GetBufferViewState(device_data_, buffer_view);
                if (!bv_state) {
                    *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00323";
                    std::stringstream error_str;
                    error_str << "Attempted copy update to texel buffer descriptor with invalid buffer view: " << buffer_view;
                    *error_msg = error_str.str();
                    return false;
                }
                auto buffer = bv_state->create_info.buffer;
                if (!ValidateBufferUsage(GetBufferState(device_data_, buffer), type, error_code, error_msg)) {
                    std::stringstream error_str;
                    error_str << "Attempted copy update to texel buffer descriptor failed due to: " << error_msg->c_str();
                    *error_msg = error_str.str();
                    return false;
                }
            }
            break;
        }
        case GeneralBuffer: {
            for (uint32_t di = 0; di < update->descriptorCount; ++di) {
                const auto src_desc = src_set->descriptors_[index + di].get();
                if (!src_desc->updated) continue;
                auto buffer = static_cast<BufferDescriptor *>(src_desc)->GetBuffer();
                if (!ValidateBufferUsage(GetBufferState(device_data_, buffer), type, error_code, error_msg)) {
                    std::stringstream error_str;
                    error_str << "Attempted copy update to buffer descriptor failed due to: " << error_msg->c_str();
                    *error_msg = error_str.str();
                    return false;
                }
            }
            break;
        }
        case InlineUniform:
        case AccelerationStructure:
            break;
        default:
            assert(0);  // We've already verified update type so should never get here
            break;
    }
    // All checks passed so update contents are good
    return true;
}
// Update the common AllocateDescriptorSetsData
void cvdescriptorset::UpdateAllocateDescriptorSetsData(const layer_data *dev_data, const VkDescriptorSetAllocateInfo *p_alloc_info,
                                                       AllocateDescriptorSetsData *ds_data) {
    for (uint32_t i = 0; i < p_alloc_info->descriptorSetCount; i++) {
        auto layout = GetDescriptorSetLayout(dev_data, p_alloc_info->pSetLayouts[i]);
        if (layout) {
            ds_data->layout_nodes[i] = layout;
            // Count total descriptors required per type
            for (uint32_t j = 0; j < layout->GetBindingCount(); ++j) {
                const auto &binding_layout = layout->GetDescriptorSetLayoutBindingPtrFromIndex(j);
                uint32_t typeIndex = static_cast<uint32_t>(binding_layout->descriptorType);
                ds_data->required_descriptors_by_type[typeIndex] += binding_layout->descriptorCount;
            }
        }
        // Any unknown layouts will be flagged as errors during ValidateAllocateDescriptorSets() call
    }
}
// Verify that the state at allocate time is correct, but don't actually allocate the sets yet
bool cvdescriptorset::ValidateAllocateDescriptorSets(const core_validation::layer_data *dev_data,
                                                     const VkDescriptorSetAllocateInfo *p_alloc_info,
                                                     const AllocateDescriptorSetsData *ds_data) {
    bool skip = false;
    auto report_data = core_validation::GetReportData(dev_data);
    auto pool_state = GetDescriptorPoolState(dev_data, p_alloc_info->descriptorPool);

    for (uint32_t i = 0; i < p_alloc_info->descriptorSetCount; i++) {
        auto layout = GetDescriptorSetLayout(dev_data, p_alloc_info->pSetLayouts[i]);
        if (layout) {  // nullptr layout indicates no valid layout handle for this device, validated/logged in object_tracker
            if (layout->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT,
                                HandleToUint64(p_alloc_info->pSetLayouts[i]), "VUID-VkDescriptorSetAllocateInfo-pSetLayouts-00308",
                                "Layout 0x%" PRIxLEAST64 " specified at pSetLayouts[%" PRIu32
                                "] in vkAllocateDescriptorSets() was created with invalid flag %s set.",
                                HandleToUint64(p_alloc_info->pSetLayouts[i]), i,
                                "VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR");
            }
            if (layout->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT &&
                !(pool_state->createInfo.flags & VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT,
                                0, "VUID-VkDescriptorSetAllocateInfo-pSetLayouts-03044",
                                "Descriptor set layout create flags and pool create flags mismatch for index (%d)", i);
            }
        }
    }
    if (!GetDeviceExtensions(dev_data)->vk_khr_maintenance1) {
        // Track number of descriptorSets allowable in this pool
        if (pool_state->availableSets < p_alloc_info->descriptorSetCount) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT,
                            HandleToUint64(pool_state->pool), "VUID-VkDescriptorSetAllocateInfo-descriptorSetCount-00306",
                            "Unable to allocate %u descriptorSets from pool 0x%" PRIxLEAST64
                            ". This pool only has %d descriptorSets remaining.",
                            p_alloc_info->descriptorSetCount, HandleToUint64(pool_state->pool), pool_state->availableSets);
        }
        // Determine whether descriptor counts are satisfiable
        for (auto it = ds_data->required_descriptors_by_type.begin(); it != ds_data->required_descriptors_by_type.end(); ++it) {
            if (ds_data->required_descriptors_by_type.at(it->first) > pool_state->availableDescriptorTypeCount[it->first]) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT,
                                HandleToUint64(pool_state->pool), "VUID-VkDescriptorSetAllocateInfo-descriptorPool-00307",
                                "Unable to allocate %u descriptors of type %s from pool 0x%" PRIxLEAST64
                                ". This pool only has %d descriptors of this type remaining.",
                                ds_data->required_descriptors_by_type.at(it->first),
                                string_VkDescriptorType(VkDescriptorType(it->first)), HandleToUint64(pool_state->pool),
                                pool_state->availableDescriptorTypeCount[it->first]);
            }
        }
    }

    const auto *count_allocate_info = lvl_find_in_chain<VkDescriptorSetVariableDescriptorCountAllocateInfoEXT>(p_alloc_info->pNext);

    if (count_allocate_info) {
        if (count_allocate_info->descriptorSetCount != 0 &&
            count_allocate_info->descriptorSetCount != p_alloc_info->descriptorSetCount) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, 0,
                            "VUID-VkDescriptorSetVariableDescriptorCountAllocateInfoEXT-descriptorSetCount-03045",
                            "VkDescriptorSetAllocateInfo::descriptorSetCount (%d) != "
                            "VkDescriptorSetVariableDescriptorCountAllocateInfoEXT::descriptorSetCount (%d)",
                            p_alloc_info->descriptorSetCount, count_allocate_info->descriptorSetCount);
        }
        if (count_allocate_info->descriptorSetCount == p_alloc_info->descriptorSetCount) {
            for (uint32_t i = 0; i < p_alloc_info->descriptorSetCount; i++) {
                auto layout = GetDescriptorSetLayout(dev_data, p_alloc_info->pSetLayouts[i]);
                if (count_allocate_info->pDescriptorCounts[i] > layout->GetDescriptorCountFromBinding(layout->GetMaxBinding())) {
                    skip |= log_msg(
                        report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, 0,
                        "VUID-VkDescriptorSetVariableDescriptorCountAllocateInfoEXT-pSetLayouts-03046",
                        "pDescriptorCounts[%d] = (%d), binding's descriptorCount = (%d)", i,
                        count_allocate_info->pDescriptorCounts[i], layout->GetDescriptorCountFromBinding(layout->GetMaxBinding()));
                }
            }
        }
    }

    return skip;
}
// Decrement allocated sets from the pool and insert new sets into set_map
void cvdescriptorset::PerformAllocateDescriptorSets(const VkDescriptorSetAllocateInfo *p_alloc_info,
                                                    const VkDescriptorSet *descriptor_sets,
                                                    const AllocateDescriptorSetsData *ds_data,
                                                    std::unordered_map<VkDescriptorPool, DESCRIPTOR_POOL_STATE *> *pool_map,
                                                    std::unordered_map<VkDescriptorSet, cvdescriptorset::DescriptorSet *> *set_map,
                                                    layer_data *dev_data) {
    auto pool_state = (*pool_map)[p_alloc_info->descriptorPool];
    // Account for sets and individual descriptors allocated from pool
    pool_state->availableSets -= p_alloc_info->descriptorSetCount;
    for (auto it = ds_data->required_descriptors_by_type.begin(); it != ds_data->required_descriptors_by_type.end(); ++it) {
        pool_state->availableDescriptorTypeCount[it->first] -= ds_data->required_descriptors_by_type.at(it->first);
    }

    const auto *variable_count_info = lvl_find_in_chain<VkDescriptorSetVariableDescriptorCountAllocateInfoEXT>(p_alloc_info->pNext);
    bool variable_count_valid = variable_count_info && variable_count_info->descriptorSetCount == p_alloc_info->descriptorSetCount;

    // Create tracking object for each descriptor set; insert into global map and the pool's set.
    for (uint32_t i = 0; i < p_alloc_info->descriptorSetCount; i++) {
        uint32_t variable_count = variable_count_valid ? variable_count_info->pDescriptorCounts[i] : 0;

        auto new_ds = new cvdescriptorset::DescriptorSet(descriptor_sets[i], p_alloc_info->descriptorPool, ds_data->layout_nodes[i],
                                                         variable_count, dev_data);

        pool_state->sets.insert(new_ds);
        new_ds->in_use.store(0);
        (*set_map)[descriptor_sets[i]] = new_ds;
    }
}

cvdescriptorset::PrefilterBindRequestMap::PrefilterBindRequestMap(cvdescriptorset::DescriptorSet &ds, const BindingReqMap &in_map,
                                                                  GLOBAL_CB_NODE *cb_state)
    : filtered_map_(), orig_map_(in_map) {
    if (ds.GetTotalDescriptorCount() > kManyDescriptors_) {
        filtered_map_.reset(new std::map<uint32_t, descriptor_req>());
        ds.FilterAndTrackBindingReqs(cb_state, orig_map_, filtered_map_.get());
    }
}
cvdescriptorset::PrefilterBindRequestMap::PrefilterBindRequestMap(cvdescriptorset::DescriptorSet &ds, const BindingReqMap &in_map,
                                                                  GLOBAL_CB_NODE *cb_state, PIPELINE_STATE *pipeline)
    : filtered_map_(), orig_map_(in_map) {
    if (ds.GetTotalDescriptorCount() > kManyDescriptors_) {
        filtered_map_.reset(new std::map<uint32_t, descriptor_req>());
        ds.FilterAndTrackBindingReqs(cb_state, pipeline, orig_map_, filtered_map_.get());
    }
}
