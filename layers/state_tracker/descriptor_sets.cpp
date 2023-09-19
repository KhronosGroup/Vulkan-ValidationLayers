/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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
 */

#include "state_tracker/descriptor_sets.h"
#include "state_tracker/cmd_buffer_state.h"

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

    const auto *variable_count_info = vku::FindStructInPNextChain<VkDescriptorSetVariableDescriptorCountAllocateInfo>(alloc_info->pNext);
    const bool variable_count_valid =
        variable_count_info && variable_count_info->descriptorSetCount == alloc_info->descriptorSetCount;

    // Create tracking object for each descriptor set; insert into global map and the pool's set.
    for (uint32_t i = 0; i < alloc_info->descriptorSetCount; i++) {
        uint32_t variable_count = variable_count_valid ? variable_count_info->pDescriptorCounts[i] : 0;

        auto new_ds = dev_data_->CreateDescriptorSet(descriptor_sets[i], this, ds_data->layout_nodes[i], variable_count);

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
            const auto &layout = set_state->Layout();
            uint32_t type_index = 0, descriptor_count = 0;
            for (uint32_t j = 0; j < layout.GetBindingCount(); ++j) {
                type_index = static_cast<uint32_t>(layout.GetTypeFromIndex(j));
                descriptor_count = layout.GetDescriptorCountFromIndex(j);
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

const VulkanTypedHandle *DESCRIPTOR_POOL_STATE::InUse() const {
    auto guard = ReadLock();
    for (const auto &entry : sets_) {
        const auto *ds = entry.second;
        if (ds) {
            return ds->InUse();
        }
    }
    return nullptr;
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
        case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:
        case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM:
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
        case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
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
    const auto *flags_create_info = vku::FindStructInPNextChain<VkDescriptorSetLayoutBindingFlagsCreateInfo>(p_create_info->pNext);

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

    const auto *mutable_descriptor_type_create_info = vku::FindStructInPNextChain<VkMutableDescriptorTypeCreateInfoEXT>(p_create_info->pNext);
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
    // If mutableDescriptorTypeListCount is zero or if VkMutableDescriptorTypeCreateInfoEXT structure is not included in the pNext
    // chain, the VkMutableDescriptorTypeListEXT for each element is considered to be zero or NULL for each member.
    return false;
}

const std::vector<std::vector<VkDescriptorType>> &cvdescriptorset::DescriptorSetLayoutDef::GetMutableTypes() const {
    return mutable_types_;
}

const std::vector<VkDescriptorType> &cvdescriptorset::DescriptorSetLayoutDef::GetMutableTypes(uint32_t binding) const {
    if (binding >= mutable_types_.size()) {
        static const std::vector<VkDescriptorType> empty = {};
        return empty;
    }
    return mutable_types_[binding];
}

void cvdescriptorset::DescriptorSetLayout::SetLayoutSizeInBytes(const VkDeviceSize *layout_size_in_bytes_) {
    if (layout_size_in_bytes_) {
        layout_size_in_bytes = std::make_unique<VkDeviceSize>(*layout_size_in_bytes_);
    } else {
        layout_size_in_bytes.reset();
    }
}

const VkDeviceSize *cvdescriptorset::DescriptorSetLayout::GetLayoutSizeInBytes() const { return layout_size_in_bytes.get(); }

// If our layout is compatible with rh_ds_layout, return true.
bool cvdescriptorset::DescriptorSetLayout::IsCompatible(DescriptorSetLayout const *rh_ds_layout) const {
    return (this == rh_ds_layout) || (GetLayoutDef() == rh_ds_layout->GetLayoutDef());
}

// The DescriptorSetLayout stores the per handle data for a descriptor set layout, and references the common defintion for the
// handle invariant portion
cvdescriptorset::DescriptorSetLayout::DescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo *p_create_info,
                                                          const VkDescriptorSetLayout layout)
    : BASE_NODE(layout, kVulkanObjectTypeDescriptorSetLayout), layout_id_(GetCanonicalId(p_create_info)) {}

void cvdescriptorset::AllocateDescriptorSetsData::Init(uint32_t count) { layout_nodes.resize(count); }

cvdescriptorset::DescriptorSet::DescriptorSet(const VkDescriptorSet set, DESCRIPTOR_POOL_STATE *pool_state,
                                              const std::shared_ptr<DescriptorSetLayout const> &layout, uint32_t variable_count,
                                              cvdescriptorset::DescriptorSet::StateTracker *state_data)
    : BASE_NODE(set, kVulkanObjectTypeDescriptorSet),
      some_update_(false),
      pool_state_(pool_state),
      layout_(layout),
      state_data_(state_data),
      variable_count_(variable_count),
      change_count_(0) {
    // Foreach binding, create default descriptors of given type
    auto binding_count = layout_->GetBindingCount();
    bindings_.reserve(binding_count);
    bindings_store_.resize(binding_count);
    auto free_binding = bindings_store_.data();
    for (uint32_t i = 0; i < binding_count; ++i) {
        auto create_info = layout_->GetDescriptorSetLayoutBindingPtrFromIndex(i);
        assert(create_info);
        uint32_t descriptor_count = create_info->descriptorCount;
        auto flags = layout_->GetDescriptorBindingFlagsFromIndex(i);
        if (flags & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT) {
            descriptor_count = variable_count;
        }
        auto type = layout_->GetTypeFromIndex(i);
        auto descriptor_class = DescriptorTypeToClass(type);
        switch (descriptor_class) {
            case PlainSampler: {
                auto binding = MakeBinding<SamplerBinding>(free_binding++, *create_info, descriptor_count, flags);
                auto immut = layout_->GetImmutableSamplerPtrFromIndex(i);
                if (immut) {
                    for (uint32_t di = 0; di < descriptor_count; ++di) {
                        auto sampler = state_data->GetConstCastShared<SAMPLER_STATE>(immut[di]);
                        if (sampler) {
                            some_update_ = true;  // Immutable samplers are updated at creation
                            binding->updated[di] = true;
                            binding->descriptors[di].SetSamplerState(std::move(sampler));
                        }
                    }
                }
                bindings_.push_back(std::move(binding));
                break;
            }
            case ImageSampler: {
                auto binding = MakeBinding<ImageSamplerBinding>(free_binding++, *create_info, descriptor_count, flags);
                auto immut = layout_->GetImmutableSamplerPtrFromIndex(i);
                if (immut) {
                    for (uint32_t di = 0; di < descriptor_count; ++di) {
                        auto sampler = state_data->GetConstCastShared<SAMPLER_STATE>(immut[di]);
                        if (sampler) {
                            some_update_ = true;  // Immutable samplers are updated at creation
                            binding->updated[di] = true;
                            binding->descriptors[di].SetSamplerState(std::move(sampler));
                        }
                    }
                }
                bindings_.push_back(std::move(binding));
                break;
            }
            // ImageDescriptors
            case Image: {
                bindings_.push_back(MakeBinding<ImageBinding>(free_binding++, *create_info, descriptor_count, flags));
                break;
            }
            case TexelBuffer: {
                bindings_.push_back(MakeBinding<TexelBinding>(free_binding++, *create_info, descriptor_count, flags));
                break;
            }
            case GeneralBuffer: {
                auto binding = MakeBinding<BufferBinding>(free_binding++, *create_info, descriptor_count, flags);
                if (IsDynamicDescriptor(type)) {
                    for (uint32_t di = 0; di < descriptor_count; ++di) {
                        dynamic_offset_idx_to_descriptor_list_.push_back({i, di});
                    }
                }
                bindings_.push_back(std::move(binding));
                break;
            }
            case InlineUniform: {
                bindings_.push_back(MakeBinding<InlineUniformBinding>(free_binding++, *create_info, descriptor_count, flags));
                break;
            }
            case AccelerationStructure: {
                bindings_.push_back(
                    MakeBinding<AccelerationStructureBinding>(free_binding++, *create_info, descriptor_count, flags));
                break;
            }
            case Mutable: {
                bindings_.push_back(MakeBinding<MutableBinding>(free_binding++, *create_info, descriptor_count, flags));
                break;
            }
            default:
                assert(0);  // Bad descriptor type specified
                break;
        }
    }
}

void cvdescriptorset::DescriptorSet::LinkChildNodes() {
    // Connect child node(s), which cannot safely be done in the constructor.
    for (auto &binding : bindings_) {
        binding->AddParent(this);
    }
}

void cvdescriptorset::DescriptorSet::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    BaseClass::NotifyInvalidate(invalid_nodes, unlink);
    for (auto &binding : bindings_) {
        binding->NotifyInvalidate(invalid_nodes, unlink);
    }
}

void cvdescriptorset::DescriptorSet::Destroy() {
    for (auto &binding : bindings_) {
        binding->RemoveParent(this);
    }
    BASE_NODE::Destroy();
}
// Loop through the write updates to do for a push descriptor set, ignoring dstSet
void cvdescriptorset::DescriptorSet::PerformPushDescriptorsUpdate(uint32_t write_count, const VkWriteDescriptorSet *write_descs) {
    assert(IsPushDescriptor());
    for (uint32_t i = 0; i < write_count; i++) {
        PerformWriteUpdate(write_descs[i]);
    }

    push_descriptor_set_writes.clear();
    push_descriptor_set_writes.reserve(static_cast<std::size_t>(write_count));
    for (uint32_t i = 0; i < write_count; i++) {
        push_descriptor_set_writes.push_back(safe_VkWriteDescriptorSet(&write_descs[i]));
    }
}

// Perform write update in given update struct
void cvdescriptorset::DescriptorSet::PerformWriteUpdate(const VkWriteDescriptorSet &update) {
    // Perform update on a per-binding basis as consecutive updates roll over to next binding
    auto descriptors_remaining = update.descriptorCount;
    auto iter = FindDescriptor(update.dstBinding, update.dstArrayElement);
    assert(!iter.AtEnd());
    auto &orig_binding = iter.CurrentBinding();

    // Verify next consecutive binding matches type, stage flags & immutable sampler use and if AtEnd
    for (uint32_t i = 0; i < descriptors_remaining; ++i, ++iter) {
        if (iter.AtEnd() || !orig_binding.IsConsistent(iter.CurrentBinding())) {
            break;
        }
        iter->WriteUpdate(*this, *state_data_, update, i, iter.CurrentBinding().IsBindless());
        iter.updated(true);
    }
    if (update.descriptorCount) {
        some_update_ = true;
        ++change_count_;
    }

    if (!IsPushDescriptor() && !(orig_binding.binding_flags & (VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT |
                                                               VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT))) {
        Invalidate(false);
    }
}
// Perform Copy update
void cvdescriptorset::DescriptorSet::PerformCopyUpdate(const VkCopyDescriptorSet &update, const DescriptorSet &src_set) {
    auto src_iter = src_set.FindDescriptor(update.srcBinding, update.srcArrayElement);
    auto dst_iter = FindDescriptor(update.dstBinding, update.dstArrayElement);
    // Update parameters all look good so perform update
    for (uint32_t i = 0; i < update.descriptorCount; ++i, ++src_iter, ++dst_iter) {
        auto &src = *src_iter;
        auto &dst = *dst_iter;
        if (src_iter.updated()) {
            auto type = src_iter.CurrentBinding().type;
            if (type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
                const auto &mutable_src = static_cast<const MutableDescriptor &>(src);
                type = mutable_src.ActiveType();
            }
            dst.CopyUpdate(*this, *state_data_, src, src_iter.CurrentBinding().IsBindless(), type);
            some_update_ = true;
            ++change_count_;
            dst_iter.updated(true);
        } else {
            dst_iter.updated(false);
        }
    }

    if (!(layout_->GetDescriptorBindingFlagsFromBinding(update.dstBinding) &
          (VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT))) {
        Invalidate(false);
    }
}

// Update the drawing state for the affected descriptors.
// Set cb_state to this set and this set to cb_state.
// Add the bindings of the descriptor
// Set the layout based on the current descriptor layout (will mask subsequent layer mismatch errors)
// TODO: Modify the UpdateDrawState virtural functions to *only* set initial layout and not change layouts
// Prereq: This should be called for a set that has been confirmed to be active for the given cb_state, meaning it's going
//   to be used in a draw by the given cb_state
void cvdescriptorset::DescriptorSet::UpdateDrawState(ValidationStateTracker *device_data, CMD_BUFFER_STATE *cb_state,
                                                     vvl::Func command, const PIPELINE_STATE *pipe,
                                                     const BindingVariableMap &binding_req_map) {
    // Descriptor UpdateDrawState only call image layout validation callbacks. If it is disabled, skip the entire loop.
    if (device_data->disabled[image_layout_validation]) {
        return;
    }

    // For the active slots, use set# to look up descriptorSet from boundDescriptorSets, and bind all of that descriptor set's
    // resources
    CMD_BUFFER_STATE::CmdDrawDispatchInfo cmd_info = {};
    for (const auto &binding_req_pair : binding_req_map) {
        auto binding = GetBinding(binding_req_pair.first);
        assert(binding);

        // We aren't validating descriptors created with PARTIALLY_BOUND or UPDATE_AFTER_BIND, so don't record state
        if (binding->IsBindless()) {
            if (!(binding->binding_flags & VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT)) {
                cmd_info.binding_infos.emplace_back(binding_req_pair);
            }
            continue;
        }
        switch (binding->descriptor_class) {
            case Image: {
                auto *image_binding = static_cast<ImageBinding *>(binding);
                for (uint32_t i = 0; i < image_binding->count; ++i) {
                    image_binding->descriptors[i].UpdateDrawState(device_data, cb_state);
                }
                break;
            }
            case ImageSampler: {
                auto *image_binding = static_cast<ImageSamplerBinding *>(binding);
                for (uint32_t i = 0; i < image_binding->count; ++i) {
                    image_binding->descriptors[i].UpdateDrawState(device_data, cb_state);
                }
                break;
            }
            case Mutable: {
                auto *mutable_binding = static_cast<MutableBinding *>(binding);
                for (uint32_t i = 0; i < mutable_binding->count; ++i) {
                    mutable_binding->descriptors[i].UpdateDrawState(device_data, cb_state);
                }
                break;
            }
            default:
                break;
        }
    }

    if (cmd_info.binding_infos.size() > 0) {
        cmd_info.command = command;
        if (cb_state->activeFramebuffer) {
            cmd_info.framebuffer = cb_state->activeFramebuffer->framebuffer();
            cmd_info.attachments = cb_state->active_attachments;
            cmd_info.subpasses = cb_state->active_subpasses;
        }
        cb_state->validate_descriptorsets_in_queuesubmit[GetSet()].emplace_back(cmd_info);
    }
}

void cvdescriptorset::DescriptorSet::FilterOneBindingReq(const BindingVariableMap::value_type &binding_req_pair,
                                                         BindingVariableMap *out_req, const TrackedBindings &bindings,
                                                         uint32_t limit) {
    if (bindings.size() < limit) {
        const auto it = bindings.find(binding_req_pair.first);
        if (it == bindings.cend()) out_req->emplace(binding_req_pair);
    }
}

void cvdescriptorset::DescriptorSet::FilterBindingReqs(const CMD_BUFFER_STATE &cb_state, const PIPELINE_STATE *pipeline,
                                                       const BindingVariableMap &in_req, BindingVariableMap *out_req) const {
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

    const auto image_sample_version_it = validated.image_samplers.find(pipeline);
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
                                                           const BindingVariableMap &updated_bindings) {
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

// Helper template to change shared pointer members of a Descriptor, while
// correctly managing links to the parent DescriptorSet.
// src and dst are shared pointers.
template <typename T>
static void ReplaceStatePtr(DescriptorSet &set_state, T &dst, const T &src, bool is_bindless) {
    if (dst && !is_bindless) {
        dst->RemoveParent(&set_state);
    }
    dst = src;
    // For descriptor bindings with UPDATE_AFTER_BIND or PARTIALLY_BOUND only set the object as a child, but not the descriptor as a
    // parent, so that destroying the object wont invalidate the descriptor
    if (dst && !is_bindless) {
        dst->AddParent(&set_state);
    }
}

void cvdescriptorset::SamplerDescriptor::WriteUpdate(DescriptorSet &set_state, const ValidationStateTracker &dev_data,
                                                     const VkWriteDescriptorSet &update, const uint32_t index, bool is_bindless) {
    if (!immutable_) {
        ReplaceStatePtr(set_state, sampler_state_, dev_data.GetConstCastShared<SAMPLER_STATE>(update.pImageInfo[index].sampler),
                        is_bindless);
    }
}

void cvdescriptorset::SamplerDescriptor::CopyUpdate(DescriptorSet &set_state, const ValidationStateTracker &dev_data,
                                                    const Descriptor &src, bool is_bindless, VkDescriptorType) {
    if (src.GetClass() == Mutable) {
        auto &sampler_src = static_cast<const MutableDescriptor &>(src);
        if (!immutable_) {
            ReplaceStatePtr(set_state, sampler_state_, sampler_src.GetSharedSamplerState(), is_bindless);
        }
        return;
    }
    auto &sampler_src = static_cast<const SamplerDescriptor &>(src);
    if (!immutable_) {
        ReplaceStatePtr(set_state, sampler_state_, sampler_src.sampler_state_, is_bindless);
    }
}

void cvdescriptorset::ImageSamplerDescriptor::WriteUpdate(DescriptorSet &set_state, const ValidationStateTracker &dev_data,
                                                          const VkWriteDescriptorSet &update, const uint32_t index,
                                                          bool is_bindless) {
    const auto &image_info = update.pImageInfo[index];
    if (!immutable_) {
        ReplaceStatePtr(set_state, sampler_state_, dev_data.GetConstCastShared<SAMPLER_STATE>(image_info.sampler), is_bindless);
    }
    image_layout_ = image_info.imageLayout;
    ReplaceStatePtr(set_state, image_view_state_, dev_data.GetConstCastShared<IMAGE_VIEW_STATE>(image_info.imageView), is_bindless);
    UpdateKnownValidView(is_bindless);
}

void cvdescriptorset::ImageSamplerDescriptor::CopyUpdate(DescriptorSet &set_state, const ValidationStateTracker &dev_data,
                                                         const Descriptor &src, bool is_bindless, VkDescriptorType src_type) {
    if (src.GetClass() == Mutable) {
        auto &image_src = static_cast<const MutableDescriptor &>(src);
        if (!immutable_) {
            ReplaceStatePtr(set_state, sampler_state_, image_src.GetSharedSamplerState(), is_bindless);
        }
        ImageDescriptor::CopyUpdate(set_state, dev_data, src, is_bindless, src_type);
        return;
    }
    auto &image_src = static_cast<const ImageSamplerDescriptor &>(src);
    if (!immutable_) {
        ReplaceStatePtr(set_state, sampler_state_, image_src.sampler_state_, is_bindless);
    }
    ImageDescriptor::CopyUpdate(set_state, dev_data, src, is_bindless, src_type);
}

void cvdescriptorset::ImageDescriptor::WriteUpdate(DescriptorSet &set_state, const ValidationStateTracker &dev_data,
                                                   const VkWriteDescriptorSet &update, const uint32_t index, bool is_bindless) {
    const auto &image_info = update.pImageInfo[index];
    image_layout_ = image_info.imageLayout;
    ReplaceStatePtr(set_state, image_view_state_, dev_data.GetConstCastShared<IMAGE_VIEW_STATE>(image_info.imageView), is_bindless);
    UpdateKnownValidView(is_bindless);
}

void cvdescriptorset::ImageDescriptor::CopyUpdate(DescriptorSet &set_state, const ValidationStateTracker &dev_data,
                                                  const Descriptor &src, bool is_bindless, VkDescriptorType src_type) {
    if (src.GetClass() == Mutable) {
        auto &image_src = static_cast<const MutableDescriptor &>(src);

        image_layout_ = image_src.GetImageLayout();
        ReplaceStatePtr(set_state, image_view_state_, image_src.GetSharedImageViewState(), is_bindless);
        UpdateKnownValidView(is_bindless);
        return;
    }
    auto &image_src = static_cast<const ImageDescriptor &>(src);

    image_layout_ = image_src.image_layout_;
    ReplaceStatePtr(set_state, image_view_state_, image_src.image_view_state_, is_bindless);
    UpdateKnownValidView(is_bindless);
}

void cvdescriptorset::ImageDescriptor::UpdateDrawState(ValidationStateTracker *dev_data, CMD_BUFFER_STATE *cb_state) {
    // Add binding for image
    auto iv_state = GetImageViewState();
    if (iv_state) {
        dev_data->CallSetImageViewInitialLayoutCallback(cb_state, *iv_state, image_layout_);
    }
}

void cvdescriptorset::BufferDescriptor::WriteUpdate(DescriptorSet &set_state, const ValidationStateTracker &dev_data,
                                                    const VkWriteDescriptorSet &update, const uint32_t index, bool is_bindless) {
    const auto &buffer_info = update.pBufferInfo[index];
    offset_ = buffer_info.offset;
    range_ = buffer_info.range;
    auto buffer_state = dev_data.GetConstCastShared<BUFFER_STATE>(buffer_info.buffer);
    ReplaceStatePtr(set_state, buffer_state_, buffer_state, is_bindless);
}

void cvdescriptorset::BufferDescriptor::CopyUpdate(DescriptorSet &set_state, const ValidationStateTracker &dev_data,
                                                   const Descriptor &src, bool is_bindless, VkDescriptorType src_type) {
    if (src.GetClass() == Mutable) {
        const auto &buff_desc = static_cast<const MutableDescriptor &>(src);
        offset_ = buff_desc.GetOffset();
        range_ = buff_desc.GetRange();
        ReplaceStatePtr(set_state, buffer_state_, buff_desc.GetSharedBufferState(), is_bindless);
        return;
    }
    const auto &buff_desc = static_cast<const BufferDescriptor &>(src);
    offset_ = buff_desc.offset_;
    range_ = buff_desc.range_;
    ReplaceStatePtr(set_state, buffer_state_, buff_desc.buffer_state_, is_bindless);
}

void cvdescriptorset::TexelDescriptor::WriteUpdate(DescriptorSet &set_state, const ValidationStateTracker &dev_data,
                                                   const VkWriteDescriptorSet &update, const uint32_t index, bool is_bindless) {
    auto buffer_view = dev_data.GetConstCastShared<BUFFER_VIEW_STATE>(update.pTexelBufferView[index]);
    ReplaceStatePtr(set_state, buffer_view_state_, buffer_view, is_bindless);
}

void cvdescriptorset::TexelDescriptor::CopyUpdate(DescriptorSet &set_state, const ValidationStateTracker &dev_data,
                                                  const Descriptor &src, bool is_bindless, VkDescriptorType src_type) {
    if (src.GetClass() == Mutable) {
        ReplaceStatePtr(set_state, buffer_view_state_, static_cast<const MutableDescriptor &>(src).GetSharedBufferViewState(),
                        is_bindless);
        return;
    }
    ReplaceStatePtr(set_state, buffer_view_state_, static_cast<const TexelDescriptor &>(src).buffer_view_state_, is_bindless);
}

void cvdescriptorset::AccelerationStructureDescriptor::WriteUpdate(DescriptorSet &set_state, const ValidationStateTracker &dev_data,
                                                                   const VkWriteDescriptorSet &update, const uint32_t index,
                                                                   bool is_bindless) {
    const auto *acc_info = vku::FindStructInPNextChain<VkWriteDescriptorSetAccelerationStructureKHR>(update.pNext);
    const auto *acc_info_nv = vku::FindStructInPNextChain<VkWriteDescriptorSetAccelerationStructureNV>(update.pNext);
    assert(acc_info || acc_info_nv);
    is_khr_ = (acc_info != NULL);
    if (is_khr_) {
        acc_ = acc_info->pAccelerationStructures[index];
        ReplaceStatePtr(set_state, acc_state_, dev_data.GetConstCastShared<ACCELERATION_STRUCTURE_STATE_KHR>(acc_), is_bindless);
    } else {
        acc_nv_ = acc_info_nv->pAccelerationStructures[index];
        ReplaceStatePtr(set_state, acc_state_nv_, dev_data.GetConstCastShared<ACCELERATION_STRUCTURE_STATE_NV>(acc_nv_),
                        is_bindless);
    }
}

void cvdescriptorset::AccelerationStructureDescriptor::CopyUpdate(DescriptorSet &set_state, const ValidationStateTracker &dev_data,
                                                                  const Descriptor &src, bool is_bindless,
                                                                  VkDescriptorType src_type) {
    if (src.GetClass() == Mutable) {
        auto &acc_desc = static_cast<const MutableDescriptor &>(src);
        is_khr_ = acc_desc.IsAccelerationStructureKHR();
        if (is_khr_) {
            acc_ = acc_desc.GetAccelerationStructureKHR();
            ReplaceStatePtr(set_state, acc_state_, dev_data.GetConstCastShared<ACCELERATION_STRUCTURE_STATE_KHR>(acc_),
                            is_bindless);
        } else {
            acc_nv_ = acc_desc.GetAccelerationStructureNV();
            ReplaceStatePtr(set_state, acc_state_nv_, dev_data.GetConstCastShared<ACCELERATION_STRUCTURE_STATE_NV>(acc_nv_),
                            is_bindless);
        }
        return;
    }
    auto acc_desc = static_cast<const AccelerationStructureDescriptor &>(src);
    is_khr_ = acc_desc.is_khr_;
    if (is_khr_) {
        acc_ = acc_desc.acc_;
        ReplaceStatePtr(set_state, acc_state_, dev_data.GetConstCastShared<ACCELERATION_STRUCTURE_STATE_KHR>(acc_), is_bindless);
    } else {
        acc_nv_ = acc_desc.acc_nv_;
        ReplaceStatePtr(set_state, acc_state_nv_, dev_data.GetConstCastShared<ACCELERATION_STRUCTURE_STATE_NV>(acc_nv_),
                        is_bindless);
    }
}

cvdescriptorset::MutableDescriptor::MutableDescriptor()
    : Descriptor(),
      buffer_size_(0),
      active_descriptor_type_(VK_DESCRIPTOR_TYPE_MUTABLE_EXT),
      immutable_(false),
      image_layout_(VK_IMAGE_LAYOUT_UNDEFINED),
      offset_(0),
      range_(0),
      is_khr_(false),
      acc_(VK_NULL_HANDLE) {}

void cvdescriptorset::MutableDescriptor::WriteUpdate(DescriptorSet &set_state, const ValidationStateTracker &dev_data,
                                                     const VkWriteDescriptorSet &update, const uint32_t index, bool is_bindless) {
    VkDeviceSize buffer_size = 0;
    switch (DescriptorTypeToClass(update.descriptorType)) {
        case DescriptorClass::PlainSampler:
            if (!immutable_) {
                ReplaceStatePtr(set_state, sampler_state_,
                                dev_data.GetConstCastShared<SAMPLER_STATE>(update.pImageInfo[index].sampler), is_bindless);
            }
            break;
        case DescriptorClass::ImageSampler: {
            const auto &image_info = update.pImageInfo[index];
            if (!immutable_) {
                ReplaceStatePtr(set_state, sampler_state_, dev_data.GetConstCastShared<SAMPLER_STATE>(image_info.sampler),
                                is_bindless);
            }
            image_layout_ = image_info.imageLayout;
            ReplaceStatePtr(set_state, image_view_state_, dev_data.GetConstCastShared<IMAGE_VIEW_STATE>(image_info.imageView),
                            is_bindless);
            break;
        }
        case DescriptorClass::Image: {
            const auto &image_info = update.pImageInfo[index];
            image_layout_ = image_info.imageLayout;
            ReplaceStatePtr(set_state, image_view_state_, dev_data.GetConstCastShared<IMAGE_VIEW_STATE>(image_info.imageView),
                            is_bindless);
            break;
        }
        case DescriptorClass::GeneralBuffer: {
            const auto &buffer_info = update.pBufferInfo[index];
            offset_ = buffer_info.offset;
            range_ = buffer_info.range;
            const auto buffer_state = dev_data.GetConstCastShared<BUFFER_STATE>(update.pBufferInfo->buffer);
            if (buffer_state) {
                buffer_size = buffer_state->createInfo.size;
            }
            ReplaceStatePtr(set_state, buffer_state_, buffer_state, is_bindless);
            break;
        }
        case DescriptorClass::TexelBuffer: {
            const auto buffer_view = dev_data.GetConstCastShared<BUFFER_VIEW_STATE>(update.pTexelBufferView[index]);
            if (buffer_view) {
                buffer_size = buffer_view->buffer_state->createInfo.size;
            }
            ReplaceStatePtr(set_state, buffer_view_state_, buffer_view, is_bindless);
            break;
        }
        case DescriptorClass::AccelerationStructure: {
            const auto *acc_info = vku::FindStructInPNextChain<VkWriteDescriptorSetAccelerationStructureKHR>(update.pNext);
            const auto *acc_info_nv = vku::FindStructInPNextChain<VkWriteDescriptorSetAccelerationStructureNV>(update.pNext);
            assert(acc_info || acc_info_nv);
            is_khr_ = (acc_info != NULL);
            if (is_khr_) {
                acc_ = acc_info->pAccelerationStructures[index];
                ReplaceStatePtr(set_state, acc_state_, dev_data.GetConstCastShared<ACCELERATION_STRUCTURE_STATE_KHR>(acc_),
                                is_bindless);
            } else {
                acc_nv_ = acc_info_nv->pAccelerationStructures[index];
                ReplaceStatePtr(set_state, acc_state_nv_, dev_data.GetConstCastShared<ACCELERATION_STRUCTURE_STATE_NV>(acc_nv_),
                                is_bindless);
            }
            break;
        }
        default:
            break;
    }
    SetDescriptorType(update.descriptorType, buffer_size);
}

void cvdescriptorset::MutableDescriptor::CopyUpdate(DescriptorSet &set_state, const ValidationStateTracker &dev_data,
                                                    const Descriptor &src, bool is_bindless, VkDescriptorType src_type) {
    VkDeviceSize src_size = 0;
    if (src.GetClass() == DescriptorClass::PlainSampler) {
        auto &sampler_src = static_cast<const SamplerDescriptor &>(src);
        if (!immutable_) {
            ReplaceStatePtr(set_state, sampler_state_, sampler_src.GetSharedSamplerState(), is_bindless);
        }
    } else if (src.GetClass() == DescriptorClass::ImageSampler) {
        auto &image_src = static_cast<const ImageSamplerDescriptor &>(src);
        if (!immutable_) {
            ReplaceStatePtr(set_state, sampler_state_, image_src.GetSharedSamplerState(), is_bindless);
        }

        image_layout_ = image_src.GetImageLayout();
        ReplaceStatePtr(set_state, image_view_state_, image_src.GetSharedImageViewState(), is_bindless);
    } else if (src.GetClass() == DescriptorClass::Image) {
        auto &image_src = static_cast<const ImageDescriptor &>(src);

        image_layout_ = image_src.GetImageLayout();
        ReplaceStatePtr(set_state, image_view_state_, image_src.GetSharedImageViewState(), is_bindless);
    } else if (src.GetClass() == DescriptorClass::TexelBuffer) {
        ReplaceStatePtr(set_state, buffer_view_state_, static_cast<const TexelDescriptor &>(src).GetSharedBufferViewState(),
                        is_bindless);
        src_size = buffer_view_state_ ? buffer_view_state_->Size() : vvl::kU32Max;
    } else if (src.GetClass() == DescriptorClass::GeneralBuffer) {
        const auto buff_desc = static_cast<const BufferDescriptor &>(src);
        offset_ = buff_desc.GetOffset();
        range_ = buff_desc.GetRange();
        ReplaceStatePtr(set_state, buffer_state_, buff_desc.GetSharedBufferState(), is_bindless);
        src_size = range_;
    } else if (src.GetClass() == DescriptorClass::AccelerationStructure) {
        auto &acc_desc = static_cast<const AccelerationStructureDescriptor &>(src);
        if (is_khr_) {
            acc_ = acc_desc.GetAccelerationStructure();
            ReplaceStatePtr(set_state, acc_state_, dev_data.GetConstCastShared<ACCELERATION_STRUCTURE_STATE_KHR>(acc_),
                            is_bindless);
        } else {
            acc_nv_ = acc_desc.GetAccelerationStructureNV();
            ReplaceStatePtr(set_state, acc_state_nv_, dev_data.GetConstCastShared<ACCELERATION_STRUCTURE_STATE_NV>(acc_nv_),
                            is_bindless);
        }
    } else if (src.GetClass() == DescriptorClass::Mutable) {
        const auto &mutable_src = static_cast<const MutableDescriptor &>(src);
        auto active_class = DescriptorTypeToClass(mutable_src.ActiveType());
        switch (active_class) {
            case PlainSampler: {
                if (!immutable_) {
                    ReplaceStatePtr(set_state, sampler_state_, mutable_src.GetSharedSamplerState(), is_bindless);
                }
            } break;
            case ImageSampler: {
                if (!immutable_) {
                    ReplaceStatePtr(set_state, sampler_state_, mutable_src.GetSharedSamplerState(), is_bindless);
                }

                image_layout_ = mutable_src.GetImageLayout();
                ReplaceStatePtr(set_state, image_view_state_, mutable_src.GetSharedImageViewState(), is_bindless);
            } break;
            case Image: {
                image_layout_ = mutable_src.GetImageLayout();
                ReplaceStatePtr(set_state, image_view_state_, mutable_src.GetSharedImageViewState(), is_bindless);
            } break;
            case GeneralBuffer: {
                offset_ = mutable_src.GetOffset();
                range_ = mutable_src.GetRange();
                ReplaceStatePtr(set_state, buffer_state_, mutable_src.GetSharedBufferState(), is_bindless);
            } break;
            case TexelBuffer: {
                ReplaceStatePtr(set_state, buffer_view_state_, mutable_src.GetSharedBufferViewState(), is_bindless);
            } break;
            case AccelerationStructure: {
                if (is_khr_) {
                    acc_ = mutable_src.GetAccelerationStructureKHR();
                    ReplaceStatePtr(set_state, acc_state_, dev_data.GetConstCastShared<ACCELERATION_STRUCTURE_STATE_KHR>(acc_),
                                    is_bindless);
                } else {
                    acc_nv_ = mutable_src.GetAccelerationStructureNV();
                    ReplaceStatePtr(set_state, acc_state_nv_, dev_data.GetConstCastShared<ACCELERATION_STRUCTURE_STATE_NV>(acc_nv_),
                                    is_bindless);
                }

            } break;
            default:
                break;
        }
        src_size = mutable_src.GetBufferSize();
    }
    SetDescriptorType(src_type, src_size);
}

void cvdescriptorset::MutableDescriptor::UpdateDrawState(ValidationStateTracker *dev_data, CMD_BUFFER_STATE *cb_state) {
    auto active_class = DescriptorTypeToClass(active_descriptor_type_);
    if (active_class == Image || active_class == ImageSampler) {
        if (image_view_state_) {
            dev_data->CallSetImageViewInitialLayoutCallback(cb_state, *image_view_state_, image_layout_);
        }
    }
}

bool cvdescriptorset::MutableDescriptor::AddParent(BASE_NODE *base_node) {
    bool result = false;
    auto active_class = DescriptorTypeToClass(active_descriptor_type_);
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
    switch (ActiveClass()) {
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

const BindingVariableMap &cvdescriptorset::PrefilterBindRequestMap::FilteredMap(const CMD_BUFFER_STATE &cb_state,
                                                                                const PIPELINE_STATE *pipeline) {
    if (IsManyDescriptors()) {
        filtered_map_.reset(new BindingVariableMap);
        descriptor_set_.FilterBindingReqs(cb_state, pipeline, orig_map_, filtered_map_.get());
        return *filtered_map_;
    }
    return orig_map_;
}
