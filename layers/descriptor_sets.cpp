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
 *         Jeremy Gebben <jeremyg@lunarg.com>
 */

#include "descriptor_sets.h"
#include "cmd_buffer_state.h"

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

cvdescriptorset::DescriptorClass cvdescriptorset::DescriptorTypeToClass(VkDescriptorType type) {
    switch (type) {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
            return PlainSampler;
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            return ImageSampler;
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            return Image;
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            return TexelBuffer;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            return GeneralBuffer;
        case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
            return InlineUniform;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
            return AccelerationStructure;
        case VK_DESCRIPTOR_TYPE_MUTABLE_VALVE:
            return Mutable;
	default:
	    break;
    }
    return NoDescriptorClass;
}


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

// If our layout is compatible with rh_ds_layout, return true.
bool cvdescriptorset::DescriptorSetLayout::IsCompatible(DescriptorSetLayout const *rh_ds_layout) const {
    bool compatible = (this == rh_ds_layout) || (GetLayoutDef() == rh_ds_layout->GetLayoutDef());
    return compatible;
}

// The DescriptorSetLayout stores the per handle data for a descriptor set layout, and references the common defintion for the
// handle invariant portion
cvdescriptorset::DescriptorSetLayout::DescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo *p_create_info,
                                                          const VkDescriptorSetLayout layout)
    : BASE_NODE(layout, kVulkanObjectTypeDescriptorSetLayout), layout_id_(GetCanonicalId(p_create_info)) {}

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
    uint32_t size = 0;
    for (uint32_t i = 0; i < layout_->GetBindingCount(); ++i) {
        if (layout_->GetDescriptorBindingFlagsFromIndex(i) & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT) {
            size += variable_count;
        } else {
            size += layout_->GetDescriptorCountFromIndex(i);
        }
    }
    descriptors_.reserve(size);
    descriptor_store_.resize(size);
    auto free_descriptor = descriptor_store_.data();
    for (uint32_t i = 0; i < layout_->GetBindingCount(); ++i) {
        uint32_t descriptor_count = layout_->GetDescriptorCountFromIndex(i);
        if (layout_->GetDescriptorBindingFlagsFromIndex(i) & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT) {
            descriptor_count = variable_count;
        }
        auto type = layout_->GetTypeFromIndex(i);
        auto descriptor_class = DescriptorTypeToClass(type);
        switch (descriptor_class) {
            case PlainSampler: {
                auto immut_sampler = layout_->GetImmutableSamplerPtrFromIndex(i);
                for (uint32_t di = 0; di < descriptor_count; ++di) {
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
            case ImageSampler: {
                auto immut = layout_->GetImmutableSamplerPtrFromIndex(i);
                for (uint32_t di = 0; di < descriptor_count; ++di) {
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
            case Image:
                for (uint32_t di = 0; di < descriptor_count; ++di) {
                    descriptors_.emplace_back(new ((free_descriptor++)->Image()) ImageDescriptor(type));
                }
                break;
            case TexelBuffer:
                for (uint32_t di = 0; di < descriptor_count; ++di) {
                    descriptors_.emplace_back(new ((free_descriptor++)->Texel()) TexelDescriptor(type));
                }
                break;
            case GeneralBuffer:
                for (uint32_t di = 0; di < descriptor_count; ++di) {
                    if (IsDynamicDescriptor(type)) {
                        dynamic_offset_idx_to_descriptor_list_.push_back(descriptors_.size());
                    }
                    descriptors_.emplace_back(new ((free_descriptor++)->Buffer()) BufferDescriptor(type));
                }
                break;
            case InlineUniform:
                for (uint32_t di = 0; di < descriptor_count; ++di) {
                    descriptors_.emplace_back(new ((free_descriptor++)->InlineUniform()) InlineUniformDescriptor(type));
                }
                break;
            case AccelerationStructure:
                for (uint32_t di = 0; di < descriptor_count; ++di) {
                    descriptors_.emplace_back(new ((free_descriptor++)->AccelerationStructure())
                                                  AccelerationStructureDescriptor(type));
                }
                break;
            case Mutable:
                for (uint32_t di = 0; di < descriptor_count; ++di) {
                    descriptors_.emplace_back(new ((free_descriptor++)->Mutable()) MutableDescriptor());
                }
                break;
            default:
                assert(0);  // Bad descriptor type specified
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

    if (!IsPushDescriptor() &&
        !(layout_->GetDescriptorBindingFlagsFromBinding(update->dstBinding) &
          (VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT))) {
        Invalidate(false);
    }
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
            dst->CopyUpdate(this, state_data_, src, src_set->IsBindless(update->dstBinding));
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
        auto type = layout_->GetTypeFromIndex(index);

        // We aren't validating descriptors created with PARTIALLY_BOUND or UPDATE_AFTER_BIND, so don't record state
        auto flags = layout_->GetDescriptorBindingFlagsFromIndex(index);
        if (flags & (VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT)) {
            if (!(flags & VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT)) {
                cmd_info.binding_infos.emplace_back(binding_req_pair);
            }
            continue;
        }
        switch (DescriptorTypeToClass(type)) {
            case Image:
            case ImageSampler: {
                auto range = layout_->GetGlobalIndexRangeFromIndex(index);
                assert(range.start < descriptors_.size());
                assert(range.end <= descriptors_.size());
                for (uint32_t i = range.start; i < range.end; ++i) {
                    auto *image_desc = static_cast<ImageDescriptor *>(descriptors_[i].get());
                    image_desc->UpdateDrawState(device_data, cb_node);
                }
            } break;
            case Mutable: {
                auto range = layout_->GetGlobalIndexRangeFromIndex(index);
                assert(range.start < descriptors_.size());
                assert(range.end <= descriptors_.size());
                for (uint32_t i = range.start; i < range.end; ++i) {
                    auto *mutable_desc = static_cast<MutableDescriptor *>(descriptors_[i].get());
                    mutable_desc->UpdateDrawState(device_data, cb_node);
                }
            } break;
            default:
                break;
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
// Helper template to change shared pointer members of a Descriptor, while
// correctly managing links to the parent DescriptorSet.
// src and dst are shared pointers.
template <typename T>
static void ReplaceStatePtr(DescriptorSet *set_state, T &dst, const T &src, bool is_bindless) {
    if (dst && !is_bindless) {
        dst->RemoveParent(set_state);
    }
    dst = src;
    // For descriptor bindings with UPDATE_AFTER_BIND or PARTIALLY_BOUND only set the object as a child, but not the descriptor as a
    // parent, so that destroying the object wont invalidate the descriptor
    if (dst && !is_bindless) {
        dst->AddParent(set_state);
    }
}

void cvdescriptorset::SamplerDescriptor::WriteUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data,
                                                     const VkWriteDescriptorSet *update, const uint32_t index) {
    if (!immutable_) {
        ReplaceStatePtr(set_state, sampler_state_, dev_data->GetConstCastShared<SAMPLER_STATE>(update->pImageInfo[index].sampler),
                        set_state->IsBindless(update->dstBinding));
    }
    updated = true;
}

void cvdescriptorset::SamplerDescriptor::CopyUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data,
                                                    const Descriptor *src, bool is_bindless) {
    updated = true;
    if (src->descriptor_class == Mutable) {
        auto *sampler_src = static_cast<const MutableDescriptor *>(src);
        if (!immutable_) {
            ReplaceStatePtr(set_state, sampler_state_, sampler_src->GetSharedSamplerState(), is_bindless);
        }
        return;
    }
    auto *sampler_src = static_cast<const SamplerDescriptor *>(src);
    if (!immutable_) {
        ReplaceStatePtr(set_state, sampler_state_, sampler_src->sampler_state_, is_bindless);
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
        ReplaceStatePtr(set_state, sampler_state_, dev_data->GetConstCastShared<SAMPLER_STATE>(image_info.sampler),
                        set_state->IsBindless(update->dstBinding));
    }
    image_layout_ = image_info.imageLayout;
    ReplaceStatePtr(set_state, image_view_state_, dev_data->GetConstCastShared<IMAGE_VIEW_STATE>(image_info.imageView),
                    set_state->IsBindless(update->dstBinding));
}

void cvdescriptorset::ImageSamplerDescriptor::CopyUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data,
                                                         const Descriptor *src, bool is_bindless) {
    updated = true;
    if (src->descriptor_class == Mutable) {
        auto *image_src = static_cast<const MutableDescriptor *>(src);
        if (!immutable_) {
            ReplaceStatePtr(set_state, sampler_state_, image_src->GetSharedSamplerState(), is_bindless);
        }
        ImageDescriptor::CopyUpdate(set_state, dev_data, src, is_bindless);
        return;
    }
    auto *image_src = static_cast<const ImageSamplerDescriptor *>(src);
    if (!immutable_) {
        ReplaceStatePtr(set_state, sampler_state_, image_src->sampler_state_, is_bindless);
    }
    ImageDescriptor::CopyUpdate(set_state, dev_data, src, is_bindless);
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
    ReplaceStatePtr(set_state, image_view_state_, dev_data->GetConstCastShared<IMAGE_VIEW_STATE>(image_info.imageView),
                    set_state->IsBindless(update->dstBinding));
}

void cvdescriptorset::ImageDescriptor::CopyUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data,
                                                  const Descriptor *src, bool is_bindless) {
    updated = true;
    if (src->descriptor_class == Mutable) {
        auto *image_src = static_cast<const MutableDescriptor *>(src);

        image_layout_ = image_src->GetImageLayout();
        ReplaceStatePtr(set_state, image_view_state_, image_src->GetSharedImageViewState(), is_bindless);
        return;
    }
    auto *image_src = static_cast<const ImageDescriptor *>(src);

    image_layout_ = image_src->image_layout_;
    ReplaceStatePtr(set_state, image_view_state_, image_src->image_view_state_, is_bindless);
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
    ReplaceStatePtr(set_state, buffer_state_, dev_data->GetConstCastShared<BUFFER_STATE>(buffer_info.buffer),
                    set_state->IsBindless(update->dstBinding));
}

void cvdescriptorset::BufferDescriptor::CopyUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data,
                                                   const Descriptor *src, bool is_bindless) {
    updated = true;
    if (src->descriptor_class == Mutable) {
        const auto buff_desc = static_cast<const MutableDescriptor *>(src);
        offset_ = buff_desc->GetOffset();
        range_ = buff_desc->GetRange();
        ReplaceStatePtr(set_state, buffer_state_, buff_desc->GetSharedBufferState(), is_bindless);
        return;
    }
    const auto buff_desc = static_cast<const BufferDescriptor *>(src);
    offset_ = buff_desc->offset_;
    range_ = buff_desc->range_;
    ReplaceStatePtr(set_state, buffer_state_, buff_desc->buffer_state_, is_bindless);
}

cvdescriptorset::TexelDescriptor::TexelDescriptor(const VkDescriptorType type) : Descriptor(TexelBuffer) {}

void cvdescriptorset::TexelDescriptor::WriteUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data,
                                                   const VkWriteDescriptorSet *update, const uint32_t index) {
    updated = true;
    ReplaceStatePtr(set_state, buffer_view_state_, dev_data->GetConstCastShared<BUFFER_VIEW_STATE>(update->pTexelBufferView[index]),
                    set_state->IsBindless(update->dstBinding));
}

void cvdescriptorset::TexelDescriptor::CopyUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data,
                                                  const Descriptor *src, bool is_bindless) {
    updated = true;
    if (src->descriptor_class == Mutable) {
        ReplaceStatePtr(set_state, buffer_view_state_, static_cast<const MutableDescriptor *>(src)->GetSharedBufferViewState(),
                        is_bindless);
        return;
    }
    ReplaceStatePtr(set_state, buffer_view_state_, static_cast<const TexelDescriptor *>(src)->buffer_view_state_, is_bindless);
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
        ReplaceStatePtr(set_state, acc_state_, dev_data->GetConstCastShared<ACCELERATION_STRUCTURE_STATE_KHR>(acc_),
                        set_state->IsBindless(update->dstBinding));
    } else {
        acc_nv_ = acc_info_nv->pAccelerationStructures[index];
        ReplaceStatePtr(set_state, acc_state_nv_, dev_data->GetConstCastShared<ACCELERATION_STRUCTURE_STATE>(acc_nv_),
                        set_state->IsBindless(update->dstBinding));
    }
}

void cvdescriptorset::AccelerationStructureDescriptor::CopyUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data,
                                                                  const Descriptor *src, bool is_bindless) {
    updated = true;
    if (src->descriptor_class == Mutable) {
        auto acc_desc = static_cast<const MutableDescriptor *>(src);
        if (is_khr_) {
            acc_ = acc_desc->GetAccelerationStructure();
            ReplaceStatePtr(set_state, acc_state_, dev_data->GetConstCastShared<ACCELERATION_STRUCTURE_STATE_KHR>(acc_),
                            is_bindless);
        } else {
            acc_nv_ = acc_desc->GetAccelerationStructureNV();
            ReplaceStatePtr(set_state, acc_state_nv_, dev_data->GetConstCastShared<ACCELERATION_STRUCTURE_STATE>(acc_nv_),
                            is_bindless);
        }
        return;
    }
    auto acc_desc = static_cast<const AccelerationStructureDescriptor *>(src);
    if (is_khr_) {
        acc_ = acc_desc->acc_;
        ReplaceStatePtr(set_state, acc_state_, dev_data->GetConstCastShared<ACCELERATION_STRUCTURE_STATE_KHR>(acc_), is_bindless);
    } else {
        acc_nv_ = acc_desc->acc_nv_;
        ReplaceStatePtr(set_state, acc_state_nv_, dev_data->GetConstCastShared<ACCELERATION_STRUCTURE_STATE>(acc_nv_), is_bindless);
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
                            dev_data->GetConstCastShared<SAMPLER_STATE>(update->pImageInfo[index].sampler),
                            set_state->IsBindless(update->dstBinding));
        }
    } else if (update->descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
        const auto &image_info = update->pImageInfo[index];
        if (!immutable_) {
            ReplaceStatePtr(set_state, sampler_state_, dev_data->GetConstCastShared<SAMPLER_STATE>(image_info.sampler),
                            set_state->IsBindless(update->dstBinding));
        }
        image_layout_ = image_info.imageLayout;
        ReplaceStatePtr(set_state, image_view_state_, dev_data->GetConstCastShared<IMAGE_VIEW_STATE>(image_info.imageView),
                        set_state->IsBindless(update->dstBinding));
    } else if (update->descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
               update->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ||
               update->descriptorType == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
        const auto &image_info = update->pImageInfo[index];
        image_layout_ = image_info.imageLayout;
        ReplaceStatePtr(set_state, image_view_state_, dev_data->GetConstCastShared<IMAGE_VIEW_STATE>(image_info.imageView),
                        set_state->IsBindless(update->dstBinding));
    } else if (update->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
               update->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
               update->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
               update->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) {
        const auto &buffer_info = update->pBufferInfo[index];
        offset_ = buffer_info.offset;
        range_ = buffer_info.range;
        ReplaceStatePtr(set_state, buffer_state_, dev_data->GetConstCastShared<BUFFER_STATE>(buffer_info.buffer),
                        set_state->IsBindless(update->dstBinding));
    } else if (update->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER ||
               update->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER) {
        ReplaceStatePtr(set_state, buffer_view_state_,
                        dev_data->GetConstCastShared<BUFFER_VIEW_STATE>(update->pTexelBufferView[index]),
                        set_state->IsBindless(update->dstBinding));
    } else if (update->descriptorType == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR) {
        const auto *acc_info = LvlFindInChain<VkWriteDescriptorSetAccelerationStructureKHR>(update->pNext);
        const auto *acc_info_nv = LvlFindInChain<VkWriteDescriptorSetAccelerationStructureNV>(update->pNext);
        assert(acc_info || acc_info_nv);
        is_khr_ = (acc_info != NULL);
        updated = true;
        if (is_khr_) {
            acc_ = acc_info->pAccelerationStructures[index];
            ReplaceStatePtr(set_state, acc_state_, dev_data->GetConstCastShared<ACCELERATION_STRUCTURE_STATE_KHR>(acc_),
                            set_state->IsBindless(update->dstBinding));
        } else {
            acc_nv_ = acc_info_nv->pAccelerationStructures[index];
            ReplaceStatePtr(set_state, acc_state_nv_, dev_data->GetConstCastShared<ACCELERATION_STRUCTURE_STATE>(acc_nv_),
                            set_state->IsBindless(update->dstBinding));
        }
    }
}

void cvdescriptorset::MutableDescriptor::CopyUpdate(DescriptorSet *set_state, const ValidationStateTracker *dev_data,
                                                    const Descriptor *src, bool is_bindless) {
    updated = true;
    if (src->descriptor_class == DescriptorClass::PlainSampler) {
        auto *sampler_src = static_cast<const SamplerDescriptor *>(src);
        if (!immutable_) {
            ReplaceStatePtr(set_state, sampler_state_, sampler_src->GetSharedSamplerState(), is_bindless);
        }
    } else if (src->descriptor_class == DescriptorClass::ImageSampler) {
        auto *image_src = static_cast<const ImageSamplerDescriptor *>(src);
        if (!immutable_) {
            ReplaceStatePtr(set_state, sampler_state_, image_src->GetSharedSamplerState(), is_bindless);
        }

        image_layout_ = image_src->GetImageLayout();
        ReplaceStatePtr(set_state, image_view_state_, image_src->GetSharedImageViewState(), is_bindless);
    } else if (src->descriptor_class == DescriptorClass::Image) {
        auto *image_src = static_cast<const ImageDescriptor *>(src);

        image_layout_ = image_src->GetImageLayout();
        ReplaceStatePtr(set_state, image_view_state_, image_src->GetSharedImageViewState(), is_bindless);
    } else if (src->descriptor_class == DescriptorClass::TexelBuffer) {
        ReplaceStatePtr(set_state, buffer_view_state_, static_cast<const TexelDescriptor *>(src)->GetSharedBufferViewState(),
                        is_bindless);
    } else if (src->descriptor_class == DescriptorClass::GeneralBuffer) {
        const auto buff_desc = static_cast<const BufferDescriptor *>(src);
        offset_ = buff_desc->GetOffset();
        range_ = buff_desc->GetRange();
        ReplaceStatePtr(set_state, buffer_state_, buff_desc->GetSharedBufferState(), is_bindless);
    } else if (src->descriptor_class == DescriptorClass::AccelerationStructure) {
        auto acc_desc = static_cast<const AccelerationStructureDescriptor *>(src);
        if (is_khr_) {
            acc_ = acc_desc->GetAccelerationStructure();
            ReplaceStatePtr(set_state, acc_state_, dev_data->GetConstCastShared<ACCELERATION_STRUCTURE_STATE_KHR>(acc_),
                            is_bindless);
        } else {
            acc_nv_ = acc_desc->GetAccelerationStructureNV();
            ReplaceStatePtr(set_state, acc_state_nv_, dev_data->GetConstCastShared<ACCELERATION_STRUCTURE_STATE>(acc_nv_),
                            is_bindless);
        }
    } else if (src->descriptor_class == DescriptorClass::Mutable) {
        auto active_class = DescriptorTypeToClass(src->active_descriptor_type);
        switch (active_class) {
            case PlainSampler: {
                auto *sampler_src = static_cast<const MutableDescriptor *>(src);
                if (!immutable_) {
                    ReplaceStatePtr(set_state, sampler_state_, sampler_src->GetSharedSamplerState(), is_bindless);
                }
            } break;
            case ImageSampler: {
                auto *image_src = static_cast<const MutableDescriptor *>(src);
                if (!immutable_) {
                    ReplaceStatePtr(set_state, sampler_state_, image_src->GetSharedSamplerState(), is_bindless);
                }

                image_layout_ = image_src->GetImageLayout();
                ReplaceStatePtr(set_state, image_view_state_, image_src->GetSharedImageViewState(), is_bindless);
            } break;
            case Image: {
                auto *image_src = static_cast<const MutableDescriptor *>(src);

                image_layout_ = image_src->GetImageLayout();
                ReplaceStatePtr(set_state, image_view_state_, image_src->GetSharedImageViewState(), is_bindless);
            } break;
            case GeneralBuffer: {
                const auto buff_desc = static_cast<const MutableDescriptor *>(src);
                offset_ = buff_desc->GetOffset();
                range_ = buff_desc->GetRange();
                ReplaceStatePtr(set_state, buffer_state_, buff_desc->GetSharedBufferState(), is_bindless);
            } break;
            case TexelBuffer: {
                ReplaceStatePtr(set_state, buffer_view_state_,
                                static_cast<const MutableDescriptor *>(src)->GetSharedBufferViewState(), is_bindless);
            } break;
            case AccelerationStructure: {
                auto acc_desc = static_cast<const MutableDescriptor *>(src);
                if (is_khr_) {
                    acc_ = acc_desc->GetAccelerationStructure();
                    ReplaceStatePtr(set_state, acc_state_, dev_data->GetConstCastShared<ACCELERATION_STRUCTURE_STATE_KHR>(acc_),
                                    is_bindless);
                } else {
                    acc_nv_ = acc_desc->GetAccelerationStructureNV();
                    ReplaceStatePtr(set_state, acc_state_nv_, dev_data->GetConstCastShared<ACCELERATION_STRUCTURE_STATE>(acc_nv_),
                                    is_bindless);
                }

            } break;
            default:
                break;
        }
    }
}

void cvdescriptorset::MutableDescriptor::UpdateDrawState(ValidationStateTracker *dev_data, CMD_BUFFER_STATE *cb_node) {
    auto active_class = DescriptorTypeToClass(active_descriptor_type);
    if (active_class == Image || active_class == ImageSampler) {
        if (image_view_state_) {
            dev_data->CallSetImageViewInitialLayoutCallback(cb_node, *image_view_state_, image_layout_);
        }
    }
}

bool cvdescriptorset::MutableDescriptor::AddParent(BASE_NODE *base_node) {
    bool result = false;
    auto active_class = DescriptorTypeToClass(active_descriptor_type);
    switch (active_class) {
        case PlainSampler:
            if (sampler_state_) {
                result |= sampler_state_->AddParent(base_node);
            }
            break;
        case ImageSampler:
            if (sampler_state_) {
                result |= sampler_state_->AddParent(base_node);
            }
            if (image_view_state_) {
                result = image_view_state_->AddParent(base_node);
            }
            break;
        case TexelBuffer:
            if (buffer_view_state_) {
                result = buffer_view_state_->AddParent(base_node);
            }
            break;
        case Image:
            if (image_view_state_) {
                result = image_view_state_->AddParent(base_node);
            }
            break;
        case GeneralBuffer:
            if (buffer_state_) {
                result = buffer_state_->AddParent(base_node);
            }
            break;
        case AccelerationStructure:
            if (acc_state_) {
                result |= acc_state_->AddParent(base_node);
            }
            if (acc_state_nv_) {
                result |= acc_state_nv_->AddParent(base_node);
            }
            break;
        default:
            break;
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
    switch (DescriptorTypeToClass(active_descriptor_type)) {
        case PlainSampler:
            return !sampler_state_ || sampler_state_->Destroyed();

        case ImageSampler:
            return !sampler_state_ || sampler_state_->Invalid() || !image_view_state_ || image_view_state_->Invalid();

        case TexelBuffer:
            return !buffer_view_state_ || buffer_view_state_->Invalid();

        case Image:
            return !image_view_state_ || image_view_state_->Invalid();

        case GeneralBuffer:
            return !buffer_state_ || buffer_state_->Invalid();

        case AccelerationStructure:
            if (is_khr_) {
                return !acc_state_ || acc_state_->Invalid();
            } else {
                return !acc_state_nv_ || acc_state_nv_->Invalid();
            }
        default:
            return false;
    }
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
const BindingReqMap &cvdescriptorset::PrefilterBindRequestMap::FilteredMap(const CMD_BUFFER_STATE &cb_state,
                                                                           const PIPELINE_STATE &pipeline) {
    if (IsManyDescriptors()) {
        filtered_map_.reset(new BindingReqMap);
        descriptor_set_.FilterBindingReqs(cb_state, pipeline, orig_map_, filtered_map_.get());
        return *filtered_map_;
    }
    return orig_map_;
}
