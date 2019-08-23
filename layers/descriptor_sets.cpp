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
#include "buffer_validation.h"
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
    global_index_range_.reserve(binding_count_);
    // Vector order is finalized so build vectors of descriptors and dynamic offsets by binding index
    for (uint32_t i = 0; i < binding_count_; ++i) {
        auto final_index = global_index + bindings_[i].descriptorCount;
        global_index_range_.emplace_back(global_index, final_index);
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

const cvdescriptorset::IndexRange &cvdescriptorset::DescriptorSetLayoutDef::GetGlobalIndexRangeFromIndex(uint32_t index) const {
    const static IndexRange kInvalidRange = {0xFFFFFFFF, 0xFFFFFFFF};
    if (index >= binding_flags_.size()) return kInvalidRange;
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

// If our layout is compatible with rh_ds_layout, return true.
bool cvdescriptorset::DescriptorSetLayout::IsCompatible(DescriptorSetLayout const *rh_ds_layout) const {
    bool compatible = (this == rh_ds_layout) || (GetLayoutDef() == rh_ds_layout->GetLayoutDef());
    return compatible;
}
// If our layout is compatible with rh_ds_layout, return true,
//  else return false and fill in error_msg will description of what causes incompatibility
bool cvdescriptorset::VerifySetLayoutCompatibility(DescriptorSetLayout const *lh_ds_layout, DescriptorSetLayout const *rh_ds_layout,
                                                   std::string *error_msg) {
    // Short circuit the detailed check.
    if (lh_ds_layout->IsCompatible(rh_ds_layout)) return true;

    // Do a detailed compatibility check of this lhs def (referenced by lh_ds_layout), vs. the rhs (layout and def)
    // Should only be run if trivial accept has failed, and in that context should return false.
    VkDescriptorSetLayout lh_dsl_handle = lh_ds_layout->GetDescriptorSetLayout();
    VkDescriptorSetLayout rh_dsl_handle = rh_ds_layout->GetDescriptorSetLayout();
    DescriptorSetLayoutDef const *lh_ds_layout_def = lh_ds_layout->GetLayoutDef();
    DescriptorSetLayoutDef const *rh_ds_layout_def = rh_ds_layout->GetLayoutDef();

    // Check descriptor counts
    if (lh_ds_layout_def->GetTotalDescriptorCount() != rh_ds_layout_def->GetTotalDescriptorCount()) {
        std::stringstream error_str;
        error_str << "DescriptorSetLayout " << lh_dsl_handle << " has " << lh_ds_layout_def->GetTotalDescriptorCount()
                  << " descriptors, but DescriptorSetLayout " << rh_dsl_handle << ", which comes from pipelineLayout, has "
                  << rh_ds_layout_def->GetTotalDescriptorCount() << " descriptors.";
        *error_msg = error_str.str();
        return false;  // trivial fail case
    }

    // Descriptor counts match so need to go through bindings one-by-one
    //  and verify that type and stageFlags match
    for (const auto &binding : lh_ds_layout_def->GetBindings()) {
        // TODO : Do we also need to check immutable samplers?
        // VkDescriptorSetLayoutBinding *rh_binding;
        if (binding.descriptorCount != rh_ds_layout_def->GetDescriptorCountFromBinding(binding.binding)) {
            std::stringstream error_str;
            error_str << "Binding " << binding.binding << " for DescriptorSetLayout " << lh_dsl_handle
                      << " has a descriptorCount of " << binding.descriptorCount << " but binding " << binding.binding
                      << " for DescriptorSetLayout " << rh_dsl_handle
                      << ", which comes from pipelineLayout, has a descriptorCount of "
                      << rh_ds_layout_def->GetDescriptorCountFromBinding(binding.binding);
            *error_msg = error_str.str();
            return false;
        } else if (binding.descriptorType != rh_ds_layout_def->GetTypeFromBinding(binding.binding)) {
            std::stringstream error_str;
            error_str << "Binding " << binding.binding << " for DescriptorSetLayout " << lh_dsl_handle << " is type '"
                      << string_VkDescriptorType(binding.descriptorType) << "' but binding " << binding.binding
                      << " for DescriptorSetLayout " << rh_dsl_handle << ", which comes from pipelineLayout, is type '"
                      << string_VkDescriptorType(rh_ds_layout_def->GetTypeFromBinding(binding.binding)) << "'";
            *error_msg = error_str.str();
            return false;
        } else if (binding.stageFlags != rh_ds_layout_def->GetStageFlagsFromBinding(binding.binding)) {
            std::stringstream error_str;
            error_str << "Binding " << binding.binding << " for DescriptorSetLayout " << lh_dsl_handle << " has stageFlags "
                      << binding.stageFlags << " but binding " << binding.binding << " for DescriptorSetLayout " << rh_dsl_handle
                      << ", which comes from pipelineLayout, has stageFlags "
                      << rh_ds_layout_def->GetStageFlagsFromBinding(binding.binding);
            *error_msg = error_str.str();
            return false;
        }
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
    : layout_(layout), layout_destroyed_(false), layout_id_(GetCanonicalId(p_create_info)) {}

// Validate descriptor set layout create info
bool cvdescriptorset::ValidateDescriptorSetLayoutCreateInfo(
    const debug_report_data *report_data, const VkDescriptorSetLayoutCreateInfo *create_info, const bool push_descriptor_ext,
    const uint32_t max_push_descriptors, const bool descriptor_indexing_ext,
    const VkPhysicalDeviceDescriptorIndexingFeaturesEXT *descriptor_indexing_features,
    const VkPhysicalDeviceInlineUniformBlockFeaturesEXT *inline_uniform_block_features,
    const VkPhysicalDeviceInlineUniformBlockPropertiesEXT *inline_uniform_block_props, const DeviceExtensions *device_extensions) {
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
            if (!device_extensions->vk_ext_inline_uniform_block) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, 0,
                                "UNASSIGNED-Extension not enabled",
                                "Creating VkDescriptorSetLayout with descriptor type  VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT "
                                "but extension %s is missing",
                                VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME);
            } else {
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
                                              cvdescriptorset::DescriptorSet::StateTracker *state_data)
    : some_update_(false),
      set_(set),
      pool_state_(nullptr),
      p_layout_(layout),
      state_data_(state_data),
      variable_count_(variable_count),
      change_count_(0) {
    pool_state_ = state_data->GetDescriptorPoolState(pool);
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
            case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
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

unsigned DescriptorRequirementsBitsFromFormat(VkFormat fmt) {
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
bool CoreChecks::ValidateDrawState(const DescriptorSet *descriptor_set, const std::map<uint32_t, descriptor_req> &bindings,
                                   const std::vector<uint32_t> &dynamic_offsets, const CMD_BUFFER_STATE *cb_node,
                                   const char *caller, std::string *error) const {
    using DescriptorClass = cvdescriptorset::DescriptorClass;
    using BufferDescriptor = cvdescriptorset::BufferDescriptor;
    using ImageDescriptor = cvdescriptorset::ImageDescriptor;
    using ImageSamplerDescriptor = cvdescriptorset::ImageSamplerDescriptor;
    using SamplerDescriptor = cvdescriptorset::SamplerDescriptor;
    using TexelDescriptor = cvdescriptorset::TexelDescriptor;
    for (auto binding_pair : bindings) {
        auto binding = binding_pair.first;
        DescriptorSetLayout::ConstBindingIterator binding_it(descriptor_set->GetLayout().get(), binding);
        if (binding_it.AtEnd()) {  //  End at construction is the condition for an invalid binding.
            std::stringstream error_str;
            error_str << "Attempting to validate DrawState for binding #" << binding
                      << " which is an invalid binding for this descriptor set.";
            *error = error_str.str();
            return false;
        }

        if (binding_it.GetDescriptorBindingFlags() &
            (VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT)) {
            // Can't validate the descriptor because it may not have been updated,
            // or the view could have been destroyed
            continue;
        }

        // Copy the range, the end range is subject to update based on variable length descriptor arrays.
        cvdescriptorset::IndexRange index_range = binding_it.GetGlobalIndexRange();
        auto array_idx = 0;  // Track array idx if we're dealing with array descriptors

        if (binding_it.IsVariableDescriptorCount()) {
            // Only validate the first N descriptors if it uses variable_count
            index_range.end = index_range.start + descriptor_set->GetVariableDescriptorCount();
        }

        for (uint32_t i = index_range.start; i < index_range.end; ++i, ++array_idx) {
            uint32_t index = i - index_range.start;
            const auto *descriptor = descriptor_set->GetDescriptorFromGlobalIndex(i);

            if (descriptor->GetClass() == DescriptorClass::InlineUniform) {
                // Can't validate the descriptor because it may not have been updated.
                continue;
            } else if (!descriptor->updated) {
                std::stringstream error_str;
                error_str << "Descriptor in binding #" << binding << " index " << index
                          << " is being used in draw but has never been updated via vkUpdateDescriptorSets() or a similar call.";
                *error = error_str.str();
                return false;
            } else {
                auto descriptor_class = descriptor->GetClass();
                if (descriptor_class == DescriptorClass::GeneralBuffer) {
                    // Verify that buffers are valid
                    auto buffer = static_cast<const BufferDescriptor *>(descriptor)->GetBuffer();
                    auto buffer_node = GetBufferState(buffer);
                    if (!buffer_node) {
                        std::stringstream error_str;
                        error_str << "Descriptor in binding #" << binding << " index " << index << " references invalid buffer "
                                  << buffer << ".";
                        *error = error_str.str();
                        return false;
                    } else if (!buffer_node->sparse) {
                        for (auto mem_binding : buffer_node->GetBoundMemory()) {
                            if (!GetDevMemState(mem_binding)) {
                                std::stringstream error_str;
                                error_str << "Descriptor in binding #" << binding << " index " << index << " uses buffer " << buffer
                                          << " that references invalid memory " << mem_binding << ".";
                                *error = error_str.str();
                                return false;
                            }
                        }
                    }
                    if (descriptor->IsDynamic()) {
                        // Validate that dynamic offsets are within the buffer
                        auto buffer_size = buffer_node->createInfo.size;
                        auto range = static_cast<const BufferDescriptor *>(descriptor)->GetRange();
                        auto desc_offset = static_cast<const BufferDescriptor *>(descriptor)->GetOffset();
                        auto dyn_offset = dynamic_offsets[binding_it.GetDynamicOffsetIndex() + array_idx];
                        if (VK_WHOLE_SIZE == range) {
                            if ((dyn_offset + desc_offset) > buffer_size) {
                                std::stringstream error_str;
                                error_str << "Dynamic descriptor in binding #" << binding << " index " << index << " uses buffer "
                                          << buffer << " with update range of VK_WHOLE_SIZE has dynamic offset " << dyn_offset
                                          << " combined with offset " << desc_offset << " that oversteps the buffer size of "
                                          << buffer_size << ".";
                                *error = error_str.str();
                                return false;
                            }
                        } else {
                            if ((dyn_offset + desc_offset + range) > buffer_size) {
                                std::stringstream error_str;
                                error_str << "Dynamic descriptor in binding #" << binding << " index " << index << " uses buffer "
                                          << buffer << " with dynamic offset " << dyn_offset << " combined with offset "
                                          << desc_offset << " and range " << range << " that oversteps the buffer size of "
                                          << buffer_size << ".";
                                *error = error_str.str();
                                return false;
                            }
                        }
                    }
                } else if (descriptor_class == DescriptorClass::ImageSampler || descriptor_class == DescriptorClass::Image) {
                    VkImageView image_view;
                    VkImageLayout image_layout;
                    if (descriptor_class == DescriptorClass::ImageSampler) {
                        image_view = static_cast<const ImageSamplerDescriptor *>(descriptor)->GetImageView();
                        image_layout = static_cast<const ImageSamplerDescriptor *>(descriptor)->GetImageLayout();
                    } else {
                        image_view = static_cast<const ImageDescriptor *>(descriptor)->GetImageView();
                        image_layout = static_cast<const ImageDescriptor *>(descriptor)->GetImageLayout();
                    }
                    auto reqs = binding_pair.second;

                    auto image_view_state = GetImageViewState(image_view);
                    if (nullptr == image_view_state) {
                        // Image view must have been destroyed since initial update. Could potentially flag the descriptor
                        //  as "invalid" (updated = false) at DestroyImageView() time and detect this error at bind time
                        std::stringstream error_str;
                        error_str << "Descriptor in binding #" << binding << " index " << index << " is using imageView "
                                  << report_data->FormatHandle(image_view).c_str() << " that has been destroyed.";
                        *error = error_str.str();
                        return false;
                    }
                    const auto &image_view_ci = image_view_state->create_info;

                    if (reqs & DESCRIPTOR_REQ_ALL_VIEW_TYPE_BITS) {
                        if (~reqs & (1 << image_view_ci.viewType)) {
                            // bad view type
                            std::stringstream error_str;
                            error_str << "Descriptor in binding #" << binding << " index " << index
                                      << " requires an image view of type " << StringDescriptorReqViewType(reqs) << " but got "
                                      << string_VkImageViewType(image_view_ci.viewType) << ".";
                            *error = error_str.str();
                            return false;
                        }

                        if (!(reqs & image_view_state->descriptor_format_bits)) {
                            // bad component type
                            std::stringstream error_str;
                            error_str << "Descriptor in binding #" << binding << " index " << index << " requires "
                                      << StringDescriptorReqComponentType(reqs)
                                      << " component type, but bound descriptor format is " << string_VkFormat(image_view_ci.format)
                                      << ".";
                            *error = error_str.str();
                            return false;
                        }
                    }

                    if (!disabled.image_layout_validation) {
                        auto image_node = GetImageState(image_view_ci.image);
                        assert(image_node);
                        // Verify Image Layout
                        // No "invalid layout" VUID required for this call, since the optimal_layout parameter is UNDEFINED.
                        bool hit_error = false;
                        VerifyImageLayout(cb_node, image_node, image_view_state->normalized_subresource_range,
                                          image_view_ci.subresourceRange.aspectMask, image_layout, VK_IMAGE_LAYOUT_UNDEFINED,
                                          caller, kVUIDUndefined, "VUID-VkDescriptorImageInfo-imageLayout-00344", &hit_error);
                        if (hit_error) {
                            *error =
                                "Image layout specified at vkUpdateDescriptorSet* or vkCmdPushDescriptorSet* time "
                                "doesn't match actual image layout at time descriptor is used. See previous error callback for "
                                "specific details.";
                            return false;
                        }
                    }

                    // Verify Sample counts
                    if ((reqs & DESCRIPTOR_REQ_SINGLE_SAMPLE) && image_view_state->samples != VK_SAMPLE_COUNT_1_BIT) {
                        std::stringstream error_str;
                        error_str << "Descriptor in binding #" << binding << " index " << index
                                  << " requires bound image to have VK_SAMPLE_COUNT_1_BIT but got "
                                  << string_VkSampleCountFlagBits(image_view_state->samples) << ".";
                        *error = error_str.str();
                        return false;
                    }
                    if ((reqs & DESCRIPTOR_REQ_MULTI_SAMPLE) && image_view_state->samples == VK_SAMPLE_COUNT_1_BIT) {
                        std::stringstream error_str;
                        error_str << "Descriptor in binding #" << binding << " index " << index
                                  << " requires bound image to have multiple samples, but got VK_SAMPLE_COUNT_1_BIT.";
                        *error = error_str.str();
                        return false;
                    }
                } else if (descriptor_class == DescriptorClass::TexelBuffer) {
                    auto texel_buffer = static_cast<const TexelDescriptor *>(descriptor);
                    auto buffer_view = GetBufferViewState(texel_buffer->GetBufferView());

                    if (nullptr == buffer_view) {
                        std::stringstream error_str;
                        error_str << "Descriptor in binding #" << binding << " index " << index << " is using bufferView "
                                  << buffer_view << " that has been destroyed.";
                        *error = error_str.str();
                        return false;
                    }
                    auto buffer = buffer_view->create_info.buffer;
                    auto buffer_state = GetBufferState(buffer);
                    if (!buffer_state) {
                        std::stringstream error_str;
                        error_str << "Descriptor in binding #" << binding << " index " << index << " is using buffer "
                                  << buffer_state << " that has been destroyed.";
                        *error = error_str.str();
                        return false;
                    }
                    auto reqs = binding_pair.second;
                    auto format_bits = DescriptorRequirementsBitsFromFormat(buffer_view->create_info.format);

                    if (!(reqs & format_bits)) {
                        // bad component type
                        std::stringstream error_str;
                        error_str << "Descriptor in binding #" << binding << " index " << index << " requires "
                                  << StringDescriptorReqComponentType(reqs) << " component type, but bound descriptor format is "
                                  << string_VkFormat(buffer_view->create_info.format) << ".";
                        *error = error_str.str();
                        return false;
                    }
                }
                if (descriptor_class == DescriptorClass::ImageSampler || descriptor_class == DescriptorClass::PlainSampler) {
                    // Verify Sampler still valid
                    VkSampler sampler;
                    if (descriptor_class == DescriptorClass::ImageSampler) {
                        sampler = static_cast<const ImageSamplerDescriptor *>(descriptor)->GetSampler();
                    } else {
                        sampler = static_cast<const SamplerDescriptor *>(descriptor)->GetSampler();
                    }
                    if (!ValidateSampler(sampler)) {
                        std::stringstream error_str;
                        error_str << "Descriptor in binding #" << binding << " index " << index << " is using sampler " << sampler
                                  << " that has been destroyed.";
                        *error = error_str.str();
                        return false;
                    } else {
                        const SAMPLER_STATE *sampler_state = GetSamplerState(sampler);
                        if (sampler_state->samplerConversion && !descriptor->IsImmutableSampler()) {
                            std::stringstream error_str;
                            error_str << "sampler (" << sampler << ") in the descriptor set (" << descriptor_set->GetSet()
                                      << ") contains a YCBCR conversion (" << sampler_state->samplerConversion
                                      << ") , then the sampler MUST also exists as an immutable sampler.";
                            *error = error_str.str();
                        }
                    }
                }
            }
        }
    }
    return true;
}

// Set is being deleted or updates so invalidate all bound cmd buffers
void cvdescriptorset::DescriptorSet::InvalidateBoundCmdBuffers() {
    state_data_->InvalidateCommandBuffers(cb_bindings, VulkanTypedHandle(set_, kVulkanObjectTypeDescriptorSet));
}

// Loop through the write updates to do for a push descriptor set, ignoring dstSet
void cvdescriptorset::DescriptorSet::PerformPushDescriptorsUpdate(uint32_t write_count, const VkWriteDescriptorSet *p_wds) {
    assert(IsPushDescriptor());
    for (uint32_t i = 0; i < write_count; i++) {
        PerformWriteUpdate(&p_wds[i]);
    }
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
    if (update->descriptorCount) {
        some_update_ = true;
        change_count_++;
    }

    if (!(p_layout_->GetDescriptorBindingFlagsFromBinding(update->dstBinding) &
          (VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT))) {
        InvalidateBoundCmdBuffers();
    }
}
// Validate Copy update
bool CoreChecks::ValidateCopyUpdate(const VkCopyDescriptorSet *update, const DescriptorSet *dst_set, const DescriptorSet *src_set,
                                    const char *func_name, std::string *error_code, std::string *error_msg) {
    auto dst_layout = dst_set->GetLayout();
    auto src_layout = src_set->GetLayout();

    // Verify dst layout still valid
    if (dst_layout->IsDestroyed()) {
        *error_code = "VUID-VkCopyDescriptorSet-dstSet-parameter";
        string_sprintf(error_msg,
                       "Cannot call %s to perform copy update on dstSet %s"
                       " created with destroyed %s.",
                       func_name, report_data->FormatHandle(dst_set->GetSet()).c_str(),
                       report_data->FormatHandle(dst_layout->GetDescriptorSetLayout()).c_str());
        return false;
    }

    // Verify src layout still valid
    if (src_layout->IsDestroyed()) {
        *error_code = "VUID-VkCopyDescriptorSet-srcSet-parameter";
        string_sprintf(error_msg,
                       "Cannot call %s to perform copy update of dstSet %s"
                       " from srcSet %s"
                       " created with destroyed %s.",
                       func_name, report_data->FormatHandle(dst_set->GetSet()).c_str(),
                       report_data->FormatHandle(src_set->GetSet()).c_str(),
                       report_data->FormatHandle(src_layout->GetDescriptorSetLayout()).c_str());
        return false;
    }

    if (!dst_layout->HasBinding(update->dstBinding)) {
        *error_code = "VUID-VkCopyDescriptorSet-dstBinding-00347";
        std::stringstream error_str;
        error_str << "DescriptorSet " << dst_set->GetSet() << " does not have copy update dest binding of " << update->dstBinding;
        *error_msg = error_str.str();
        return false;
    }
    if (!src_set->HasBinding(update->srcBinding)) {
        *error_code = "VUID-VkCopyDescriptorSet-srcBinding-00345";
        std::stringstream error_str;
        error_str << "DescriptorSet " << dst_set->GetSet() << " does not have copy update src binding of " << update->srcBinding;
        *error_msg = error_str.str();
        return false;
    }
    // Verify idle ds
    if (dst_set->in_use.load() &&
        !(dst_layout->GetDescriptorBindingFlagsFromBinding(update->dstBinding) &
          (VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT))) {
        // TODO : Re-using Free Idle error code, need copy update idle error code
        *error_code = "VUID-vkFreeDescriptorSets-pDescriptorSets-00309";
        std::stringstream error_str;
        error_str << "Cannot call " << func_name << " to perform copy update on descriptor set " << dst_set->GetSet()
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
    auto dst_start_idx = dst_layout->GetGlobalIndexRangeFromBinding(update->dstBinding).start + update->dstArrayElement;
    if ((dst_start_idx + update->descriptorCount) > dst_layout->GetTotalDescriptorCount()) {
        // DST update out of bounds
        *error_code = "VUID-VkCopyDescriptorSet-dstArrayElement-00348";
        std::stringstream error_str;
        error_str << "Attempting copy update to descriptorSet " << dst_set->GetSet() << " binding#" << update->dstBinding
                  << " with offset index of " << dst_layout->GetGlobalIndexRangeFromBinding(update->dstBinding).start
                  << " plus update array offset of " << update->dstArrayElement << " and update of " << update->descriptorCount
                  << " descriptors oversteps total number of descriptors in set: " << dst_layout->GetTotalDescriptorCount();
        *error_msg = error_str.str();
        return false;
    }
    // Check that types match
    // TODO : Base default error case going from here is "VUID-VkAcquireNextImageInfoKHR-semaphore-parameter"2ba which covers all
    // consistency issues, need more fine-grained error codes
    *error_code = "VUID-VkCopyDescriptorSet-srcSet-00349";
    auto src_type = src_set->GetTypeFromBinding(update->srcBinding);
    auto dst_type = dst_layout->GetTypeFromBinding(update->dstBinding);
    if (src_type != dst_type) {
        std::stringstream error_str;
        error_str << "Attempting copy update to descriptorSet " << dst_set->GetSet() << " binding #" << update->dstBinding
                  << " with type " << string_VkDescriptorType(dst_type) << " from descriptorSet " << src_set->GetSet()
                  << " binding #" << update->srcBinding << " with type " << string_VkDescriptorType(src_type)
                  << ". Types do not match";
        *error_msg = error_str.str();
        return false;
    }
    // Verify consistency of src & dst bindings if update crosses binding boundaries
    if ((!VerifyUpdateConsistency(DescriptorSetLayout::ConstBindingIterator(src_layout.get(), update->srcBinding),
                                  update->srcArrayElement, update->descriptorCount, "copy update from", src_set->GetSet(),
                                  error_msg)) ||
        (!VerifyUpdateConsistency(DescriptorSetLayout::ConstBindingIterator(dst_layout.get(), update->dstBinding),
                                  update->dstArrayElement, update->descriptorCount, "copy update to", dst_set->GetSet(),
                                  error_msg))) {
        return false;
    }

    if ((src_layout->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT) &&
        !(dst_layout->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT)) {
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

    if (!(src_layout->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT) &&
        (dst_layout->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT)) {
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
        !(dst_set->GetPoolState()->createInfo.flags & VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT)) {
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
        (dst_set->GetPoolState()->createInfo.flags & VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT)) {
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
    if (!VerifyCopyUpdateContents(update, src_set, src_type, src_start_idx, func_name, error_code, error_msg)) return false;

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
            change_count_++;
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
void cvdescriptorset::DescriptorSet::UpdateDrawState(ValidationStateTracker *device_data, CMD_BUFFER_STATE *cb_node,
                                                     const std::map<uint32_t, descriptor_req> &binding_req_map) {
    if (!device_data->disabled.command_buffer_state) {
        // bind cb to this descriptor set
        // Add bindings for descriptor set, the set's pool, and individual objects in the set
        auto inserted = cb_node->object_bindings.emplace(set_, kVulkanObjectTypeDescriptorSet);
        if (inserted.second) {
            cb_bindings.insert(cb_node);
            auto inserted2 = cb_node->object_bindings.emplace(pool_state_->pool, kVulkanObjectTypeDescriptorPool);
            if (inserted2.second) {
                pool_state_->cb_bindings.insert(cb_node);
            }
        }
    }

    // Descriptor UpdateDrawState functions do two things - associate resources to the command buffer,
    // and call image layout validation callbacks. If both are disabled, skip the entire loop.
    if (device_data->disabled.command_buffer_state && device_data->disabled.image_layout_validation) {
        return;
    }

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
    const auto validated_it = cached_validation_.find(&cb_state);
    if (validated_it == cached_validation_.cend()) {
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
            FilterOneBindingReq(binding_req_pair, out_req, dynamic_buffers, stats.dynamic_buffer_count);
        } else if ((layout_binding->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) ||
                   (layout_binding->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)) {
            FilterOneBindingReq(binding_req_pair, out_req, non_dynamic_buffers, stats.non_dynamic_buffer_count);
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

void cvdescriptorset::DescriptorSet::UpdateValidationCache(const CMD_BUFFER_STATE &cb_state, const PIPELINE_STATE &pipeline,
                                                           const BindingReqMap &updated_bindings) {
    // For const cleanliness we have to find in the maps...
    auto &validated = cached_validation_[&cb_state];

    auto &image_sample_version = validated.image_samplers[&pipeline];
    auto &dynamic_buffers = validated.dynamic_buffers;
    auto &non_dynamic_buffers = validated.non_dynamic_buffers;
    for (const auto &binding_req_pair : updated_bindings) {
        auto binding = binding_req_pair.first;
        VkDescriptorSetLayoutBinding const *layout_binding = p_layout_->GetDescriptorSetLayoutBindingPtrFromBinding(binding);
        if (!layout_binding) {
            continue;
        }
        // Caching criteria differs per type.
        if ((layout_binding->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) ||
            (layout_binding->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)) {
            dynamic_buffers.emplace(binding);
        } else if ((layout_binding->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) ||
                   (layout_binding->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)) {
            non_dynamic_buffers.emplace(binding);
        } else {
            // Save the layout change version...
            image_sample_version[binding] = cb_state.image_layout_change_count;
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
bool CoreChecks::ValidateSampler(const VkSampler sampler) const { return (GetSamplerState(sampler) != nullptr); }

bool CoreChecks::ValidateImageUpdate(VkImageView image_view, VkImageLayout image_layout, VkDescriptorType type,
                                     const char *func_name, std::string *error_code, std::string *error_msg) {
    *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00326";
    auto iv_state = GetImageViewState(image_view);
    assert(iv_state);

    // Note that when an imageview is created, we validated that memory is bound so no need to re-check here
    // Validate that imageLayout is compatible with aspect_mask and image format
    //  and validate that image usage bits are correct for given usage
    VkImageAspectFlags aspect_mask = iv_state->create_info.subresourceRange.aspectMask;
    VkImage image = iv_state->create_info.image;
    VkFormat format = VK_FORMAT_MAX_ENUM;
    VkImageUsageFlags usage = 0;
    auto image_node = GetImageState(image);
    assert(image_node);

    format = image_node->createInfo.format;
    usage = image_node->createInfo.usage;
    // Validate that memory is bound to image
    // TODO: This should have its own valid usage id apart from 2524 which is from CreateImageView case. The only
    //  the error here occurs is if memory bound to a created imageView has been freed.
    if (ValidateMemoryIsBoundToImage(image_node, func_name, "VUID-VkImageViewCreateInfo-image-01020")) {
        *error_code = "VUID-VkImageViewCreateInfo-image-01020";
        *error_msg = "No memory bound to image.";
        return false;
    }

    // KHR_maintenance1 allows rendering into 2D or 2DArray views which slice a 3D image,
    // but not binding them to descriptor sets.
    if (image_node->createInfo.imageType == VK_IMAGE_TYPE_3D && (iv_state->create_info.viewType == VK_IMAGE_VIEW_TYPE_2D ||
                                                                 iv_state->create_info.viewType == VK_IMAGE_VIEW_TYPE_2D_ARRAY)) {
        *error_code = "VUID-VkDescriptorImageInfo-imageView-00343";
        *error_msg = "ImageView must not be a 2D or 2DArray view of a 3D image";
        return false;
    }

    // TODO : The various image aspect and format checks here are based on general spec language in 11.5 Image Views section under
    // vkCreateImageView(). What's the best way to create unique id for these cases?
    *error_code = "UNASSIGNED-CoreValidation-DrawState-InvalidImageView";
    bool ds = FormatIsDepthOrStencil(format);
    switch (image_layout) {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            // Only Color bit must be set
            if ((aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) != VK_IMAGE_ASPECT_COLOR_BIT) {
                std::stringstream error_str;
                error_str
                    << "ImageView (" << report_data->FormatHandle(image_view).c_str()
                    << ") uses layout VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL but does not have VK_IMAGE_ASPECT_COLOR_BIT set.";
                *error_msg = error_str.str();
                return false;
            }
            // format must NOT be DS
            if (ds) {
                std::stringstream error_str;
                error_str << "ImageView (" << report_data->FormatHandle(image_view).c_str()
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
                    error_str << "ImageView (" << report_data->FormatHandle(image_view).c_str()
                              << ") has both STENCIL and DEPTH aspects set";
                    *error_msg = error_str.str();
                    return false;
                }
            } else if (!(aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT)) {
                // Neither were set
                std::stringstream error_str;
                error_str << "ImageView (" << report_data->FormatHandle(image_view).c_str() << ") has layout "
                          << string_VkImageLayout(image_layout) << " but does not have STENCIL or DEPTH aspects set";
                *error_msg = error_str.str();
                return false;
            }
            // format must be DS
            if (!ds) {
                std::stringstream error_str;
                error_str << "ImageView (" << report_data->FormatHandle(image_view).c_str() << ") has layout "
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
                        error_str << "ImageView (" << report_data->FormatHandle(image_view).c_str() << ") has layout "
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
    // TODO : Need to also validate case "VUID-VkWriteDescriptorSet-descriptorType-00336" where STORAGE_IMAGE & INPUT_ATTACH types
    // must have been created with identify swizzle
    const char *error_usage_bit = nullptr;
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
                        error_str << "ImageView (" << report_data->FormatHandle(image_view).c_str()
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
                    error_str << "ImageView (" << report_data->FormatHandle(image_view).c_str()
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
    if (error_usage_bit) {
        std::stringstream error_str;
        error_str << "ImageView (" << report_data->FormatHandle(image_view).c_str() << ") with usage mask " << std::hex
                  << std::showbase << usage << " being used for a descriptor update of type " << string_VkDescriptorType(type)
                  << " does not have " << error_usage_bit << " set.";
        *error_msg = error_str.str();
        return false;
    }

    if ((type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) || (type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)) {
        // Test that the layout is compatible with the descriptorType for the two sampled image types
        const static std::array<VkImageLayout, 3> valid_layouts = {
            {VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL}};

        struct ExtensionLayout {
            VkImageLayout layout;
            bool DeviceExtensions::*extension;
        };

        const static std::array<ExtensionLayout, 3> extended_layouts{
            {//  Note double brace req'd for aggregate initialization
             {VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, &DeviceExtensions::vk_khr_shared_presentable_image},
             {VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, &DeviceExtensions::vk_khr_maintenance2},
             {VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, &DeviceExtensions::vk_khr_maintenance2}}};
        auto is_layout = [image_layout, this](const ExtensionLayout &ext_layout) {
            return device_extensions.*(ext_layout.extension) && (ext_layout.layout == image_layout);
        };

        bool valid_layout = (std::find(valid_layouts.cbegin(), valid_layouts.cend(), image_layout) != valid_layouts.cend()) ||
                            std::any_of(extended_layouts.cbegin(), extended_layouts.cend(), is_layout);

        if (!valid_layout) {
            *error_code = "VUID-VkWriteDescriptorSet-descriptorType-01403";
            std::stringstream error_str;
            error_str << "Descriptor update with descriptorType " << string_VkDescriptorType(type)
                      << " is being updated with invalid imageLayout " << string_VkImageLayout(image_layout) << " for image "
                      << report_data->FormatHandle(image).c_str() << " in imageView "
                      << report_data->FormatHandle(image_view).c_str()
                      << ". Allowed layouts are: VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, "
                      << "VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL";
            for (auto &ext_layout : extended_layouts) {
                if (device_extensions.*(ext_layout.extension)) {
                    error_str << ", " << string_VkImageLayout(ext_layout.layout);
                }
            }
            *error_msg = error_str.str();
            return false;
        }
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

void cvdescriptorset::SamplerDescriptor::UpdateDrawState(ValidationStateTracker *dev_data, CMD_BUFFER_STATE *cb_node) {
    if (!immutable_) {
        auto sampler_state = dev_data->GetSamplerState(sampler_);
        if (sampler_state) dev_data->AddCommandBufferBindingSampler(cb_node, sampler_state);
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

void cvdescriptorset::ImageSamplerDescriptor::UpdateDrawState(ValidationStateTracker *dev_data, CMD_BUFFER_STATE *cb_node) {
    // First add binding for any non-immutable sampler
    if (!immutable_) {
        auto sampler_state = dev_data->GetSamplerState(sampler_);
        if (sampler_state) dev_data->AddCommandBufferBindingSampler(cb_node, sampler_state);
    }
    // Add binding for image
    auto iv_state = dev_data->GetImageViewState(image_view_);
    if (iv_state) {
        dev_data->AddCommandBufferBindingImageView(cb_node, iv_state);
        dev_data->CallSetImageViewInitialLayoutCallback(cb_node, *iv_state, image_layout_);
    }
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

void cvdescriptorset::ImageDescriptor::UpdateDrawState(ValidationStateTracker *dev_data, CMD_BUFFER_STATE *cb_node) {
    // Add binding for image
    auto iv_state = dev_data->GetImageViewState(image_view_);
    if (iv_state) {
        dev_data->AddCommandBufferBindingImageView(cb_node, iv_state);
        dev_data->CallSetImageViewInitialLayoutCallback(cb_node, *iv_state, image_layout_);
    }
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

void cvdescriptorset::BufferDescriptor::UpdateDrawState(ValidationStateTracker *dev_data, CMD_BUFFER_STATE *cb_node) {
    auto buffer_node = dev_data->GetBufferState(buffer_);
    if (buffer_node) dev_data->AddCommandBufferBindingBuffer(cb_node, buffer_node);
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

void cvdescriptorset::TexelDescriptor::UpdateDrawState(ValidationStateTracker *dev_data, CMD_BUFFER_STATE *cb_node) {
    auto bv_state = dev_data->GetBufferViewState(buffer_view_);
    if (bv_state) {
        dev_data->AddCommandBufferBindingBufferView(cb_node, bv_state);
    }
}

// This is a helper function that iterates over a set of Write and Copy updates, pulls the DescriptorSet* for updated
//  sets, and then calls their respective Validate[Write|Copy]Update functions.
// If the update hits an issue for which the callback returns "true", meaning that the call down the chain should
//  be skipped, then true is returned.
// If there is no issue with the update, then false is returned.
bool CoreChecks::ValidateUpdateDescriptorSets(uint32_t write_count, const VkWriteDescriptorSet *p_wds, uint32_t copy_count,
                                              const VkCopyDescriptorSet *p_cds, const char *func_name) {
    bool skip = false;
    // Validate Write updates
    for (uint32_t i = 0; i < write_count; i++) {
        auto dest_set = p_wds[i].dstSet;
        auto set_node = GetSetNode(dest_set);
        if (!set_node) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT,
                            HandleToUint64(dest_set), kVUID_Core_DrawState_InvalidDescriptorSet,
                            "Cannot call %s on %s that has not been allocated.", func_name,
                            report_data->FormatHandle(dest_set).c_str());
        } else {
            std::string error_code;
            std::string error_str;
            if (!ValidateWriteUpdate(set_node, &p_wds[i], func_name, &error_code, &error_str)) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT,
                                HandleToUint64(dest_set), error_code, "%s failed write update validation for %s with error: %s.",
                                func_name, report_data->FormatHandle(dest_set).c_str(), error_str.c_str());
            }
        }
    }
    // Now validate copy updates
    for (uint32_t i = 0; i < copy_count; ++i) {
        auto dst_set = p_cds[i].dstSet;
        auto src_set = p_cds[i].srcSet;
        auto src_node = GetSetNode(src_set);
        auto dst_node = GetSetNode(dst_set);
        // Object_tracker verifies that src & dest descriptor set are valid
        assert(src_node);
        assert(dst_node);
        std::string error_code;
        std::string error_str;
        if (!ValidateCopyUpdate(&p_cds[i], dst_node, src_node, func_name, &error_code, &error_str)) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT,
                        HandleToUint64(dst_set), error_code, "%s failed copy update from %s to %s with error: %s.", func_name,
                        report_data->FormatHandle(src_set).c_str(), report_data->FormatHandle(dst_set).c_str(), error_str.c_str());
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
        auto set_node = dev_data->GetSetNode(dest_set);
        if (set_node) {
            set_node->PerformWriteUpdate(&p_wds[i]);
        }
    }
    // Now copy updates
    for (i = 0; i < copy_count; ++i) {
        auto dst_set = p_cds[i].dstSet;
        auto src_set = p_cds[i].srcSet;
        auto src_node = dev_data->GetSetNode(src_set);
        auto dst_node = dev_data->GetSetNode(dst_set);
        if (src_node && dst_node) {
            dst_node->PerformCopyUpdate(&p_cds[i], src_node);
        }
    }
}

cvdescriptorset::DecodedTemplateUpdate::DecodedTemplateUpdate(const ValidationStateTracker *device_data,
                                                              VkDescriptorSet descriptorSet, const TEMPLATE_STATE *template_state,
                                                              const void *pData, VkDescriptorSetLayout push_layout) {
    auto const &create_info = template_state->create_info;
    inline_infos.resize(create_info.descriptorUpdateEntryCount);  // Make sure we have one if we need it
    desc_writes.reserve(create_info.descriptorUpdateEntryCount);  // emplaced, so reserved without initialization
    VkDescriptorSetLayout effective_dsl = create_info.templateType == VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET
                                              ? create_info.descriptorSetLayout
                                              : push_layout;
    auto layout_obj = GetDescriptorSetLayout(device_data, effective_dsl);

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
bool CoreChecks::ValidateUpdateDescriptorSetsWithTemplateKHR(VkDescriptorSet descriptorSet, const TEMPLATE_STATE *template_state,
                                                             const void *pData) {
    // Translate the templated update into a normal update for validation...
    cvdescriptorset::DecodedTemplateUpdate decoded_update(this, descriptorSet, template_state, pData);
    return ValidateUpdateDescriptorSets(static_cast<uint32_t>(decoded_update.desc_writes.size()), decoded_update.desc_writes.data(),
                                        0, NULL, "vkUpdateDescriptorSetWithTemplate()");
}

void ValidationStateTracker::PerformUpdateDescriptorSetsWithTemplateKHR(VkDescriptorSet descriptorSet,
                                                                        const TEMPLATE_STATE *template_state, const void *pData) {
    // Translate the templated update into a normal update for validation...
    cvdescriptorset::DecodedTemplateUpdate decoded_update(this, descriptorSet, template_state, pData);
    cvdescriptorset::PerformUpdateDescriptorSets(this, static_cast<uint32_t>(decoded_update.desc_writes.size()),
                                                 decoded_update.desc_writes.data(), 0, NULL);
}

std::string cvdescriptorset::DescriptorSet::StringifySetAndLayout() const {
    std::string out;
    auto layout_handle = p_layout_->GetDescriptorSetLayout();
    if (IsPushDescriptor()) {
        string_sprintf(&out, "Push Descriptors defined with VkDescriptorSetLayout %s",
                       state_data_->report_data->FormatHandle(layout_handle).c_str());
    } else {
        string_sprintf(&out, "VkDescriptorSet %s allocated with VkDescriptorSetLayout %s",
                       state_data_->report_data->FormatHandle(set_).c_str(),
                       state_data_->report_data->FormatHandle(layout_handle).c_str());
    }
    return out;
};

// Loop through the write updates to validate for a push descriptor set, ignoring dstSet
bool CoreChecks::ValidatePushDescriptorsUpdate(const DescriptorSet *push_set, uint32_t write_count,
                                               const VkWriteDescriptorSet *p_wds, const char *func_name) {
    assert(push_set->IsPushDescriptor());
    bool skip = false;
    for (uint32_t i = 0; i < write_count; i++) {
        std::string error_code;
        std::string error_str;
        if (!ValidateWriteUpdate(push_set, &p_wds[i], func_name, &error_code, &error_str)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT,
                            HandleToUint64(push_set->GetDescriptorSetLayout()), error_code, "%s failed update validation: %s.",
                            func_name, error_str.c_str());
        }
    }
    return skip;
}

// For the given buffer, verify that its creation parameters are appropriate for the given type
//  If there's an error, update the error_msg string with details and return false, else return true
bool cvdescriptorset::ValidateBufferUsage(BUFFER_STATE const *buffer_node, VkDescriptorType type, std::string *error_code,
                                          std::string *error_msg) {
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
        error_str << "Buffer (" << buffer_node->buffer << ") with usage mask " << std::hex << std::showbase << usage
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
bool CoreChecks::ValidateBufferUpdate(VkDescriptorBufferInfo const *buffer_info, VkDescriptorType type, const char *func_name,
                                      std::string *error_code, std::string *error_msg) {
    // First make sure that buffer is valid
    auto buffer_node = GetBufferState(buffer_info->buffer);
    // Any invalid buffer should already be caught by object_tracker
    assert(buffer_node);
    if (ValidateMemoryIsBoundToBuffer(buffer_node, func_name, "VUID-VkWriteDescriptorSet-descriptorType-00329")) {
        *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00329";
        *error_msg = "No memory bound to buffer.";
        return false;
    }
    // Verify usage bits
    if (!cvdescriptorset::ValidateBufferUsage(buffer_node, type, error_code, error_msg)) {
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
    const auto &limits = phys_dev_props.limits;
    if (VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER == type || VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC == type) {
        auto max_ub_range = limits.maxUniformBufferRange;
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
        auto max_sb_range = limits.maxStorageBufferRange;
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
bool CoreChecks::VerifyCopyUpdateContents(const VkCopyDescriptorSet *update, const DescriptorSet *src_set, VkDescriptorType type,
                                          uint32_t index, const char *func_name, std::string *error_code, std::string *error_msg) {
    // Note : Repurposing some Write update error codes here as specific details aren't called out for copy updates like they are
    // for write updates
    using DescriptorClass = cvdescriptorset::DescriptorClass;
    using BufferDescriptor = cvdescriptorset::BufferDescriptor;
    using ImageDescriptor = cvdescriptorset::ImageDescriptor;
    using ImageSamplerDescriptor = cvdescriptorset::ImageSamplerDescriptor;
    using SamplerDescriptor = cvdescriptorset::SamplerDescriptor;
    using TexelDescriptor = cvdescriptorset::TexelDescriptor;

    auto device_data = this;
    switch (src_set->GetDescriptorFromGlobalIndex(index)->descriptor_class) {
        case DescriptorClass::PlainSampler: {
            for (uint32_t di = 0; di < update->descriptorCount; ++di) {
                const auto src_desc = src_set->GetDescriptorFromGlobalIndex(index + di);
                if (!src_desc->updated) continue;
                if (!src_desc->IsImmutableSampler()) {
                    auto update_sampler = static_cast<const SamplerDescriptor *>(src_desc)->GetSampler();
                    if (!ValidateSampler(update_sampler)) {
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
        case DescriptorClass::ImageSampler: {
            for (uint32_t di = 0; di < update->descriptorCount; ++di) {
                const auto src_desc = src_set->GetDescriptorFromGlobalIndex(index + di);
                if (!src_desc->updated) continue;
                auto img_samp_desc = static_cast<const ImageSamplerDescriptor *>(src_desc);
                // First validate sampler
                if (!img_samp_desc->IsImmutableSampler()) {
                    auto update_sampler = img_samp_desc->GetSampler();
                    if (!ValidateSampler(update_sampler)) {
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
                if (!ValidateImageUpdate(image_view, image_layout, type, func_name, error_code, error_msg)) {
                    std::stringstream error_str;
                    error_str << "Attempted copy update to combined image sampler descriptor failed due to: " << error_msg->c_str();
                    *error_msg = error_str.str();
                    return false;
                }
            }
            break;
        }
        case DescriptorClass::Image: {
            for (uint32_t di = 0; di < update->descriptorCount; ++di) {
                const auto src_desc = src_set->GetDescriptorFromGlobalIndex(index + di);
                if (!src_desc->updated) continue;
                auto img_desc = static_cast<const ImageDescriptor *>(src_desc);
                auto image_view = img_desc->GetImageView();
                auto image_layout = img_desc->GetImageLayout();
                if (!ValidateImageUpdate(image_view, image_layout, type, func_name, error_code, error_msg)) {
                    std::stringstream error_str;
                    error_str << "Attempted copy update to image descriptor failed due to: " << error_msg->c_str();
                    *error_msg = error_str.str();
                    return false;
                }
            }
            break;
        }
        case DescriptorClass::TexelBuffer: {
            for (uint32_t di = 0; di < update->descriptorCount; ++di) {
                const auto src_desc = src_set->GetDescriptorFromGlobalIndex(index + di);
                if (!src_desc->updated) continue;
                auto buffer_view = static_cast<const TexelDescriptor *>(src_desc)->GetBufferView();
                auto bv_state = device_data->GetBufferViewState(buffer_view);
                if (!bv_state) {
                    *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00323";
                    std::stringstream error_str;
                    error_str << "Attempted copy update to texel buffer descriptor with invalid buffer view: " << buffer_view;
                    *error_msg = error_str.str();
                    return false;
                }
                auto buffer = bv_state->create_info.buffer;
                if (!cvdescriptorset::ValidateBufferUsage(GetBufferState(buffer), type, error_code, error_msg)) {
                    std::stringstream error_str;
                    error_str << "Attempted copy update to texel buffer descriptor failed due to: " << error_msg->c_str();
                    *error_msg = error_str.str();
                    return false;
                }
            }
            break;
        }
        case DescriptorClass::GeneralBuffer: {
            for (uint32_t di = 0; di < update->descriptorCount; ++di) {
                const auto src_desc = src_set->GetDescriptorFromGlobalIndex(index + di);
                if (!src_desc->updated) continue;
                auto buffer = static_cast<const BufferDescriptor *>(src_desc)->GetBuffer();
                if (!cvdescriptorset::ValidateBufferUsage(GetBufferState(buffer), type, error_code, error_msg)) {
                    std::stringstream error_str;
                    error_str << "Attempted copy update to buffer descriptor failed due to: " << error_msg->c_str();
                    *error_msg = error_str.str();
                    return false;
                }
            }
            break;
        }
        case DescriptorClass::InlineUniform:
        case DescriptorClass::AccelerationStructure:
            break;
        default:
            assert(0);  // We've already verified update type so should never get here
            break;
    }
    // All checks passed so update contents are good
    return true;
}
// Update the common AllocateDescriptorSetsData
void CoreChecks::UpdateAllocateDescriptorSetsData(const VkDescriptorSetAllocateInfo *p_alloc_info,
                                                  cvdescriptorset::AllocateDescriptorSetsData *ds_data) {
    for (uint32_t i = 0; i < p_alloc_info->descriptorSetCount; i++) {
        auto layout = GetDescriptorSetLayout(this, p_alloc_info->pSetLayouts[i]);
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
bool CoreChecks::ValidateAllocateDescriptorSets(const VkDescriptorSetAllocateInfo *p_alloc_info,
                                                const cvdescriptorset::AllocateDescriptorSetsData *ds_data) {
    bool skip = false;
    auto pool_state = GetDescriptorPoolState(p_alloc_info->descriptorPool);

    for (uint32_t i = 0; i < p_alloc_info->descriptorSetCount; i++) {
        auto layout = GetDescriptorSetLayout(this, p_alloc_info->pSetLayouts[i]);
        if (layout) {  // nullptr layout indicates no valid layout handle for this device, validated/logged in object_tracker
            if (layout->IsPushDescriptor()) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT,
                                HandleToUint64(p_alloc_info->pSetLayouts[i]), "VUID-VkDescriptorSetAllocateInfo-pSetLayouts-00308",
                                "%s specified at pSetLayouts[%" PRIu32
                                "] in vkAllocateDescriptorSets() was created with invalid flag %s set.",
                                report_data->FormatHandle(p_alloc_info->pSetLayouts[i]).c_str(), i,
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
    if (!device_extensions.vk_khr_maintenance1) {
        // Track number of descriptorSets allowable in this pool
        if (pool_state->availableSets < p_alloc_info->descriptorSetCount) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT,
                            HandleToUint64(pool_state->pool), "VUID-VkDescriptorSetAllocateInfo-descriptorSetCount-00306",
                            "Unable to allocate %u descriptorSets from %s"
                            ". This pool only has %d descriptorSets remaining.",
                            p_alloc_info->descriptorSetCount, report_data->FormatHandle(pool_state->pool).c_str(),
                            pool_state->availableSets);
        }
        // Determine whether descriptor counts are satisfiable
        for (auto it = ds_data->required_descriptors_by_type.begin(); it != ds_data->required_descriptors_by_type.end(); ++it) {
            if (ds_data->required_descriptors_by_type.at(it->first) > pool_state->availableDescriptorTypeCount[it->first]) {
                skip |= log_msg(
                    report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT,
                    HandleToUint64(pool_state->pool), "VUID-VkDescriptorSetAllocateInfo-descriptorPool-00307",
                    "Unable to allocate %u descriptors of type %s from %s"
                    ". This pool only has %d descriptors of this type remaining.",
                    ds_data->required_descriptors_by_type.at(it->first), string_VkDescriptorType(VkDescriptorType(it->first)),
                    report_data->FormatHandle(pool_state->pool).c_str(), pool_state->availableDescriptorTypeCount[it->first]);
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
                auto layout = GetDescriptorSetLayout(this, p_alloc_info->pSetLayouts[i]);
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
void ValidationStateTracker::PerformAllocateDescriptorSets(const VkDescriptorSetAllocateInfo *p_alloc_info,
                                                           const VkDescriptorSet *descriptor_sets,
                                                           const cvdescriptorset::AllocateDescriptorSetsData *ds_data) {
    auto pool_state = descriptorPoolMap[p_alloc_info->descriptorPool].get();
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

        std::unique_ptr<cvdescriptorset::DescriptorSet> new_ds(new cvdescriptorset::DescriptorSet(
            descriptor_sets[i], p_alloc_info->descriptorPool, ds_data->layout_nodes[i], variable_count, this));
        pool_state->sets.insert(new_ds.get());
        new_ds->in_use.store(0);
        setMap[descriptor_sets[i]] = std::move(new_ds);
    }
}

const BindingReqMap &cvdescriptorset::PrefilterBindRequestMap::FilteredMap(const CMD_BUFFER_STATE &cb_state,
                                                                           const PIPELINE_STATE &pipeline) {
    if (IsManyDescriptors()) {
        filtered_map_.reset(new std::map<uint32_t, descriptor_req>());
        descriptor_set_.FilterBindingReqs(cb_state, pipeline, orig_map_, filtered_map_.get());
        return *filtered_map_;
    }
    return orig_map_;
}

// Starting at offset descriptor of given binding, parse over update_count
//  descriptor updates and verify that for any binding boundaries that are crossed, the next binding(s) are all consistent
//  Consistency means that their type, stage flags, and whether or not they use immutable samplers matches
//  If so, return true. If not, fill in error_msg and return false
bool cvdescriptorset::VerifyUpdateConsistency(DescriptorSetLayout::ConstBindingIterator current_binding, uint32_t offset,
                                              uint32_t update_count, const char *type, const VkDescriptorSet set,
                                              std::string *error_msg) {
    // Verify consecutive bindings match (if needed)
    auto orig_binding = current_binding;
    // Track count of descriptors in the current_bindings that are remaining to be updated
    auto binding_remaining = current_binding.GetDescriptorCount();
    // First, it's legal to offset beyond your own binding so handle that case
    //  Really this is just searching for the binding in which the update begins and adjusting offset accordingly
    while (offset >= binding_remaining && !current_binding.AtEnd()) {
        // Advance to next binding, decrement offset by binding size
        offset -= binding_remaining;
        ++current_binding;
        binding_remaining = current_binding.GetDescriptorCount();  // Accessors are safe if AtEnd
    }
    assert(!current_binding.AtEnd());  // As written assumes range check has been made before calling
    binding_remaining -= offset;
    while (update_count > binding_remaining) {  // While our updates overstep current binding
        // Verify next consecutive binding matches type, stage flags & immutable sampler use
        auto next_binding = current_binding.Next();
        if (!current_binding.IsConsistent(next_binding)) {
            std::stringstream error_str;
            error_str << "Attempting " << type;
            if (current_binding.Layout()->IsPushDescriptor()) {
                error_str << " push descriptors";
            } else {
                error_str << " descriptor set " << set;
            }
            error_str << " binding #" << orig_binding.Binding() << " with #" << update_count
                      << " descriptors being updated but this update oversteps the bounds of this binding and the next binding is "
                         "not consistent with current binding so this update is invalid.";
            *error_msg = error_str.str();
            return false;
        }
        current_binding = next_binding;
        // For sake of this check consider the bindings updated and grab count for next binding
        update_count -= binding_remaining;
        binding_remaining = current_binding.GetDescriptorCount();
    }
    return true;
}

// Validate the state for a given write update but don't actually perform the update
//  If an error would occur for this update, return false and fill in details in error_msg string
bool CoreChecks::ValidateWriteUpdate(const DescriptorSet *dest_set, const VkWriteDescriptorSet *update, const char *func_name,
                                     std::string *error_code, std::string *error_msg) {
    const auto dest_layout = dest_set->GetLayout();

    // Verify dst layout still valid
    if (dest_layout->IsDestroyed()) {
        *error_code = "VUID-VkWriteDescriptorSet-dstSet-00320";
        string_sprintf(error_msg, "Cannot call %s to perform write update on %s which has been destroyed", func_name,
                       dest_set->StringifySetAndLayout().c_str());
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

    DescriptorSetLayout::ConstBindingIterator dest(dest_layout.get(), update->dstBinding);
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
    if (dest_set->in_use.load() && !(dest.GetDescriptorBindingFlags() & (VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT |
                                                                         VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT))) {
        // TODO : Re-using Free Idle error code, need write update idle error code
        *error_code = "VUID-vkFreeDescriptorSets-pDescriptorSets-00309";
        std::stringstream error_str;
        error_str << "Cannot call " << func_name << " to perform write update on " << dest_set->StringifySetAndLayout()
                  << " that is in use by a command buffer";
        *error_msg = error_str.str();
        return false;
    }
    // We know that binding is valid, verify update and do update on each descriptor
    auto start_idx = dest.GetGlobalIndexRange().start + update->dstArrayElement;
    auto type = dest.GetType();
    if (type != update->descriptorType) {
        *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00319";
        std::stringstream error_str;
        error_str << "Attempting write update to " << dest_set->StringifySetAndLayout() << " binding #" << update->dstBinding
                  << " with type " << string_VkDescriptorType(type) << " but update type is "
                  << string_VkDescriptorType(update->descriptorType);
        *error_msg = error_str.str();
        return false;
    }
    auto total_descriptors = dest_layout->GetTotalDescriptorCount();
    if (update->descriptorCount > (total_descriptors - start_idx)) {
        *error_code = "VUID-VkWriteDescriptorSet-dstArrayElement-00321";
        std::stringstream error_str;
        error_str << "Attempting write update to " << dest_set->StringifySetAndLayout() << " binding #" << update->dstBinding
                  << " with " << total_descriptors - start_idx
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
        const auto *write_inline_info = lvl_find_in_chain<VkWriteDescriptorSetInlineUniformBlockEXT>(update->pNext);
        if (!write_inline_info || write_inline_info->dataSize != update->descriptorCount) {
            *error_code = "VUID-VkWriteDescriptorSet-descriptorType-02221";
            std::stringstream error_str;
            if (!write_inline_info) {
                error_str << "Attempting write update to " << dest_set->StringifySetAndLayout() << " binding #"
                          << update->dstBinding << " with "
                          << "VkWriteDescriptorSetInlineUniformBlockEXT missing";
            } else {
                error_str << "Attempting write update to " << dest_set->StringifySetAndLayout() << " binding #"
                          << update->dstBinding << " with "
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
            error_str << "Attempting write update to " << dest_set->StringifySetAndLayout() << " binding #" << update->dstBinding
                      << " with "
                      << "VkWriteDescriptorSetInlineUniformBlockEXT dataSize " << write_inline_info->dataSize
                      << " not a multiple of 4";
            *error_msg = error_str.str();
            return false;
        }
    }
    // Verify consecutive bindings match (if needed)
    if (!VerifyUpdateConsistency(DescriptorSetLayout::ConstBindingIterator(dest_layout.get(), update->dstBinding),
                                 update->dstArrayElement, update->descriptorCount, "write update to", dest_set->GetSet(),
                                 error_msg)) {
        // TODO : Should break out "consecutive binding updates" language into valid usage statements
        *error_code = "VUID-VkWriteDescriptorSet-dstArrayElement-00321";
        return false;
    }
    // Update is within bounds and consistent so last step is to validate update contents
    if (!VerifyWriteUpdateContents(dest_set, update, start_idx, func_name, error_code, error_msg)) {
        std::stringstream error_str;
        error_str << "Write update to " << dest_set->StringifySetAndLayout() << " binding #" << update->dstBinding
                  << " failed with error message: " << error_msg->c_str();
        *error_msg = error_str.str();
        return false;
    }
    // All checks passed, update is clean
    return true;
}

// Verify that the contents of the update are ok, but don't perform actual update
bool CoreChecks::VerifyWriteUpdateContents(const DescriptorSet *dest_set, const VkWriteDescriptorSet *update, const uint32_t index,
                                           const char *func_name, std::string *error_code, std::string *error_msg) {
    using ImageSamplerDescriptor = cvdescriptorset::ImageSamplerDescriptor;
    using SamplerDescriptor = cvdescriptorset::SamplerDescriptor;

    switch (update->descriptorType) {
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
            for (uint32_t di = 0; di < update->descriptorCount; ++di) {
                // Validate image
                auto image_view = update->pImageInfo[di].imageView;
                auto image_layout = update->pImageInfo[di].imageLayout;
                if (!ValidateImageUpdate(image_view, image_layout, update->descriptorType, func_name, error_code, error_msg)) {
                    std::stringstream error_str;
                    error_str << "Attempted write update to combined image sampler descriptor failed due to: "
                              << error_msg->c_str();
                    *error_msg = error_str.str();
                    return false;
                }
                if (device_extensions.vk_khr_sampler_ycbcr_conversion) {
                    ImageSamplerDescriptor *desc = (ImageSamplerDescriptor *)dest_set->GetDescriptorFromGlobalIndex(index + di);
                    if (desc->IsImmutableSampler()) {
                        auto sampler_state = GetSamplerState(desc->GetSampler());
                        auto iv_state = GetImageViewState(image_view);
                        if (iv_state && sampler_state) {
                            if (iv_state->samplerConversion != sampler_state->samplerConversion) {
                                *error_code = "VUID-VkWriteDescriptorSet-descriptorType-01948";
                                std::stringstream error_str;
                                error_str << "Attempted write update to combined image sampler and image view and sampler ycbcr "
                                             "conversions are not identical, sampler: "
                                          << desc->GetSampler() << " image view: " << iv_state->image_view << ".";
                                *error_msg = error_str.str();
                                return false;
                            }
                        }
                    } else {
                        auto iv_state = GetImageViewState(image_view);
                        if (iv_state && (iv_state->samplerConversion != VK_NULL_HANDLE)) {
                            *error_code = "VUID-VkWriteDescriptorSet-descriptorType-02738";
                            std::stringstream error_str;
                            error_str << "Because dstSet (" << update->dstSet << ") is bound to image view ("
                                      << iv_state->image_view
                                      << ") that includes a YCBCR conversion, it must have been allocated with a layout that "
                                         "includes an immutable sampler.";
                            *error_msg = error_str.str();
                            return false;
                        }
                    }
                }
            }
        }
        // fall through
        case VK_DESCRIPTOR_TYPE_SAMPLER: {
            for (uint32_t di = 0; di < update->descriptorCount; ++di) {
                SamplerDescriptor *desc = (SamplerDescriptor *)dest_set->GetDescriptorFromGlobalIndex(index + di);
                if (!desc->IsImmutableSampler()) {
                    if (!ValidateSampler(update->pImageInfo[di].sampler)) {
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
                if (!ValidateImageUpdate(image_view, image_layout, update->descriptorType, func_name, error_code, error_msg)) {
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
                auto bv_state = GetBufferViewState(buffer_view);
                if (!bv_state) {
                    *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00323";
                    std::stringstream error_str;
                    error_str << "Attempted write update to texel buffer descriptor with invalid buffer view: " << buffer_view;
                    *error_msg = error_str.str();
                    return false;
                }
                auto buffer = bv_state->create_info.buffer;
                auto buffer_state = GetBufferState(buffer);
                // Verify that buffer underlying the view hasn't been destroyed prematurely
                if (!buffer_state) {
                    *error_code = "VUID-VkWriteDescriptorSet-descriptorType-00323";
                    std::stringstream error_str;
                    error_str << "Attempted write update to texel buffer descriptor failed because underlying buffer (" << buffer
                              << ") has been destroyed: " << error_msg->c_str();
                    *error_msg = error_str.str();
                    return false;
                } else if (!cvdescriptorset::ValidateBufferUsage(buffer_state, update->descriptorType, error_code, error_msg)) {
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
                if (!ValidateBufferUpdate(update->pBufferInfo + di, update->descriptorType, func_name, error_code, error_msg)) {
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
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
            // XXX TODO
            break;
        default:
            assert(0);  // We've already verified update type so should never get here
            break;
    }
    // All checks passed so update contents are good
    return true;
}
