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

#include <valarray>

#include "core_validation.h"
#include "state_tracker/descriptor_sets.h"
#include "cc_buffer_address.h"
#include "generated/spirv_grammar_helper.h"
#include "drawdispatch/descriptor_validator.h"

using DescriptorSet = vvl::DescriptorSet;
using DescriptorSetLayout = vvl::DescriptorSetLayout;
using DescriptorSetLayoutDef = vvl::DescriptorSetLayoutDef;
using DescriptorSetLayoutId = vvl::DescriptorSetLayoutId;

template <typename DSLayoutBindingA, typename DSLayoutBindingB>
bool ImmutableSamplersAreEqual(const DSLayoutBindingA &b1, const DSLayoutBindingB &b2) {
    if (b1.pImmutableSamplers == b2.pImmutableSamplers) {
        return true;
    } else if (b1.pImmutableSamplers && b2.pImmutableSamplers) {
        if ((b1.descriptorType == b2.descriptorType) &&
            ((b1.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER) || (b1.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)) &&
            (b1.descriptorCount == b2.descriptorCount)) {
            for (uint32_t i = 0; i < b1.descriptorCount; ++i) {
                if (b1.pImmutableSamplers[i] != b2.pImmutableSamplers[i]) {
                    return false;
                }
            }
            return true;
        } else {
            return false;
        }
    } else {
        // One pointer is null, the other is not
        return false;
    }
}

// If our layout is compatible with bound_dsl, return true,
//  else return false and fill in error_msg will description of what causes incompatibility
bool CoreChecks::VerifySetLayoutCompatibility(const DescriptorSetLayout &layout_dsl, const DescriptorSetLayout &bound_dsl,
                                              std::string &error_msg) const {
    // Short circuit the detailed check.
    if (layout_dsl.IsCompatible(&bound_dsl)) return true;

    // Do a detailed compatibility check of this lhs def (referenced by layout_dsl), vs. the rhs (layout and def)
    // Should only be run if trivial accept has failed, and in that context should return false.
    VkDescriptorSetLayout layout_dsl_handle = layout_dsl.VkHandle();
    VkDescriptorSetLayout bound_dsl_handle = bound_dsl.VkHandle();
    DescriptorSetLayoutDef const *layout_ds_layout_def = layout_dsl.GetLayoutDef();
    DescriptorSetLayoutDef const *bound_ds_layout_def = bound_dsl.GetLayoutDef();

    // Check descriptor counts
    const auto bound_total_count = bound_ds_layout_def->GetTotalDescriptorCount();
    if (layout_ds_layout_def->GetTotalDescriptorCount() != bound_ds_layout_def->GetTotalDescriptorCount()) {
        std::stringstream error_str;
        error_str << FormatHandle(layout_dsl_handle) << " from pipeline layout has "
                  << layout_ds_layout_def->GetTotalDescriptorCount() << " total descriptors, but " << FormatHandle(bound_dsl_handle)
                  << ", which is bound, has " << bound_total_count << " total descriptors.";
        error_msg = error_str.str();
        return false;  // trivial fail case
    }

    // Descriptor counts match so need to go through bindings one-by-one
    //  and verify that type and stageFlags match
    for (const auto &layout_binding : layout_ds_layout_def->GetBindings()) {
        const auto bound_binding = bound_ds_layout_def->GetBindingInfoFromBinding(layout_binding.binding);
        if (layout_binding.descriptorCount != bound_binding->descriptorCount) {
            std::stringstream error_str;
            error_str << "Binding " << layout_binding.binding << " for " << FormatHandle(layout_dsl_handle)
                      << " from pipeline layout has a descriptorCount of " << layout_binding.descriptorCount << " but binding "
                      << layout_binding.binding << " for " << FormatHandle(bound_dsl_handle)
                      << ", which is bound, has a descriptorCount of " << bound_binding->descriptorCount;
            error_msg = error_str.str();
            return false;
        } else if (layout_binding.descriptorType != bound_binding->descriptorType) {
            std::stringstream error_str;
            error_str << "Binding " << layout_binding.binding << " for " << FormatHandle(layout_dsl_handle)
                      << " from pipeline layout is type '" << string_VkDescriptorType(layout_binding.descriptorType)
                      << "' but binding " << layout_binding.binding << " for " << FormatHandle(bound_dsl_handle)
                      << ", which is bound, is type '" << string_VkDescriptorType(bound_binding->descriptorType) << "'";
            error_msg = error_str.str();
            return false;
        } else if (layout_binding.stageFlags != bound_binding->stageFlags) {
            std::stringstream error_str;
            error_str << "Binding " << layout_binding.binding << " for " << FormatHandle(layout_dsl_handle)
                      << " from pipeline layout has stageFlags " << string_VkShaderStageFlags(layout_binding.stageFlags)
                      << " but binding " << layout_binding.binding << " for " << FormatHandle(bound_dsl_handle)
                      << ", which is bound, has stageFlags " << string_VkShaderStageFlags(bound_binding->stageFlags);
            error_msg = error_str.str();
            return false;
        } else if (!ImmutableSamplersAreEqual(layout_binding, *bound_binding)) {
            error_msg = "Immutable samplers from binding " + std::to_string(layout_binding.binding) + " in pipeline layout " +
                        FormatHandle(layout_dsl_handle) + " do not match the immutable samplers in the layout currently bound (" +
                        FormatHandle(bound_dsl_handle) + ")";
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
        error_str << FormatHandle(layout_dsl_handle) << " from pipeline layout does not have the same binding flags at binding "
                  << i << " ( " << string_VkDescriptorBindingFlags(ds_layout_flags[i]) << " ) as " << FormatHandle(bound_dsl_handle)
                  << " ( " << string_VkDescriptorBindingFlags(bound_layout_flags[i]) << " ), which is bound";
        error_msg = error_str.str();
        return false;
    }

    // No detailed check should succeed if the trivial check failed -- or the dictionary has failed somehow.
    bool compatible = true;
    assert(!compatible);
    return compatible;
}

// For given vvl::DescriptorSet, verify that its Set is compatible w/ the setLayout corresponding to
// pipelineLayout[layoutIndex]
bool CoreChecks::VerifySetLayoutCompatibility(
    const vvl::DescriptorSet &descriptor_set,
    const std::vector<std::shared_ptr<vvl::DescriptorSetLayout const>> &set_layouts, const VulkanTypedHandle &handle,
    const uint32_t layoutIndex, std::string &errorMsg) const {
    auto num_sets = set_layouts.size();
    if (layoutIndex >= num_sets) {
        std::stringstream error_str;
        error_str << FormatHandle(handle) << ") only contains " << num_sets << " setLayouts corresponding to sets 0-" << num_sets - 1
                  << ", but you're attempting to bind set to index " << layoutIndex;
        errorMsg = error_str.str();
        return false;
    }
    if (descriptor_set.IsPushDescriptor()) return true;
    const auto *layout_node = set_layouts[layoutIndex].get();
    if (layout_node) {
        return VerifySetLayoutCompatibility(*layout_node, *descriptor_set.GetLayout(), errorMsg);
    } else {
        // It's possible the DSL is null when creating a graphics pipeline library, in which case we can't verify compatibility
        // here.
        return true;
    }
}

bool CoreChecks::VerifySetLayoutCompatibility(const vvl::PipelineLayout &layout_a, const vvl::PipelineLayout &layout_b,
                                              std::string &error_msg) const {
    const uint32_t num_sets = static_cast<uint32_t>(std::min(layout_a.set_layouts.size(), layout_b.set_layouts.size()));
    for (uint32_t i = 0; i < num_sets; ++i) {
        const auto ds_a = layout_a.set_layouts[i];
        const auto ds_b = layout_b.set_layouts[i];
        if (ds_a && ds_b) {
            if (!VerifySetLayoutCompatibility(*ds_a, *ds_b, error_msg)) {
                return false;
            }
        }
    }
    return true;
}

bool CoreChecks::ValidateCmdBindDescriptorSets(const vvl::CommandBuffer &cb_state, VkPipelineLayout layout, uint32_t firstSet,
                                               uint32_t setCount, const VkDescriptorSet *pDescriptorSets,
                                               uint32_t dynamicOffsetCount, const uint32_t *pDynamicOffsets,
                                               const Location &loc) const {
    bool skip = false;
    const bool is_2 = loc.function != Func::vkCmdBindDescriptorSets;

    auto pipeline_layout = Get<vvl::PipelineLayout>(layout);
    if (!pipeline_layout) {
        return skip;  // dynamicPipelineLayout feature
    }

    // Track total count of dynamic descriptor types to make sure we have an offset for each one
    uint32_t total_dynamic_descriptors = 0;

    for (uint32_t set_idx = 0; set_idx < setCount; set_idx++) {
        const Location set_loc = loc.dot(Field::pDescriptorSets, set_idx);
        auto descriptor_set = Get<vvl::DescriptorSet>(pDescriptorSets[set_idx]);
        if (descriptor_set) {
            // Verify that set being bound is compatible with overlapping setLayout of pipelineLayout
            std::string error_string = "";
            if (!VerifySetLayoutCompatibility(*descriptor_set, pipeline_layout->set_layouts,
                                              pipeline_layout->Handle(), set_idx + firstSet, error_string)) {
                const LogObjectList objlist(cb_state.commandBuffer(), pDescriptorSets[set_idx]);
                const char *vuid = is_2 ? "VUID-VkBindDescriptorSetsInfoKHR-pDescriptorSets-00358"
                                        : "VUID-vkCmdBindDescriptorSets-pDescriptorSets-00358";
                skip |= LogError(vuid, objlist, set_loc,
                                 "(%s) being bound is not compatible with overlapping "
                                 "descriptorSetLayout at index %" PRIu32
                                 " of "
                                 "%s due to: %s.",
                                 FormatHandle(pDescriptorSets[set_idx]).c_str(), set_idx + firstSet, FormatHandle(layout).c_str(),
                                 error_string.c_str());
            }

            const auto &dsl = descriptor_set->GetLayout();
            if (dsl->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) {
                const LogObjectList objlist(cb_state.commandBuffer(), pDescriptorSets[set_idx], dsl->VkHandle());
                const char *vuid = is_2 ? "VUID-VkBindDescriptorSetsInfoKHR-pDescriptorSets-08010"
                                        : "VUID-vkCmdBindDescriptorSets-pDescriptorSets-08010";
                skip |= LogError(vuid, objlist, set_loc, "was allocated from VkDescriptorSetLayout with %s flags.",
                                 string_VkDescriptorSetLayoutCreateFlags(dsl->GetCreateFlags()).c_str());
            }

            auto set_dynamic_descriptor_count = descriptor_set->GetDynamicDescriptorCount();
            if (set_dynamic_descriptor_count) {
                // First make sure we won't overstep bounds of pDynamicOffsets array
                if ((total_dynamic_descriptors + set_dynamic_descriptor_count) > dynamicOffsetCount) {
                    // Test/report this here, such that we don't run past the end of pDynamicOffsets in the else clause
                    const LogObjectList objlist(cb_state.commandBuffer(), pDescriptorSets[set_idx]);
                    const char *vuid = is_2 ? "VUID-VkBindDescriptorSetsInfoKHR-dynamicOffsetCount-00359"
                                            : "VUID-vkCmdBindDescriptorSets-dynamicOffsetCount-00359";
                    skip |=
                        LogError(vuid, objlist, set_loc,
                                 "(%s) requires %" PRIu32 " dynamicOffsets, but only %" PRIu32
                                 " "
                                 "dynamicOffsets are left in "
                                 "pDynamicOffsets array. There must be one dynamic offset for each dynamic descriptor being bound.",
                                 FormatHandle(pDescriptorSets[set_idx]).c_str(), descriptor_set->GetDynamicDescriptorCount(),
                                 (dynamicOffsetCount - total_dynamic_descriptors));
                    // Set the number found to the maximum to prevent duplicate messages, or subsquent descriptor sets from
                    // testing against the "short tail" we're skipping below.
                    total_dynamic_descriptors = dynamicOffsetCount;
                } else {  // Validate dynamic offsets and Dynamic Offset Minimums
                    // offset for all sets (pDynamicOffsets)
                    uint32_t cur_dyn_offset = total_dynamic_descriptors;
                    // offset into this descriptor set
                    uint32_t set_dyn_offset = 0;
                    const auto binding_count = dsl->GetBindingCount();
                    const auto &limits = phys_dev_props.limits;
                    for (uint32_t i = 0; i < binding_count; i++) {
                        const auto *binding = dsl->GetDescriptorSetLayoutBindingPtrFromIndex(i);
                        // skip checking binding if not needed
                        if (vvl::IsDynamicDescriptor(binding->descriptorType) == false) {
                            continue;
                        }

                        // If a descriptor set has only binding 0 and 2 the binding_index will be 0 and 2
                        const uint32_t binding_index = binding->binding;
                        const uint32_t descriptorCount = binding->descriptorCount;

                        // Need to loop through each descriptor count inside the binding
                        // if descriptorCount is zero the binding with a dynamic descriptor type does not count
                        for (uint32_t j = 0; j < descriptorCount; j++) {
                            const uint32_t offset = pDynamicOffsets[cur_dyn_offset];
                            if (offset == 0) {
                                // offset of zero is equivalent of not having the dynamic offset
                                cur_dyn_offset++;
                                set_dyn_offset++;
                                continue;
                            }

                            // Validate alignment with limit
                            if ((binding->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) &&
                                (SafeModulo(offset, limits.minUniformBufferOffsetAlignment) != 0)) {
                                const char *vuid = is_2 ? "VUID-VkBindDescriptorSetsInfoKHR-pDynamicOffsets-01971"
                                                        : "VUID-vkCmdBindDescriptorSets-pDynamicOffsets-01971";
                                skip |= LogError(vuid, cb_state.commandBuffer(), loc.dot(Field::pDynamicOffsets, cur_dyn_offset),
                                                 "is %" PRIu32
                                                 ", but must be a multiple of "
                                                 "device limit minUniformBufferOffsetAlignment %" PRIu64 ".",
                                                 offset, limits.minUniformBufferOffsetAlignment);
                            }
                            if ((binding->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) &&
                                (SafeModulo(offset, limits.minStorageBufferOffsetAlignment) != 0)) {
                                const char *vuid = is_2 ? "VUID-VkBindDescriptorSetsInfoKHR-pDynamicOffsets-01972"
                                                        : "VUID-vkCmdBindDescriptorSets-pDynamicOffsets-01972";
                                skip |= LogError(vuid, cb_state.commandBuffer(), loc.dot(Field::pDynamicOffsets, cur_dyn_offset),
                                                 "is %" PRIu32
                                                 ", but must be a multiple of "
                                                 "device limit minStorageBufferOffsetAlignment %" PRIu64 ".",
                                                 offset, limits.minStorageBufferOffsetAlignment);
                            }

                            auto *descriptor = descriptor_set->GetDescriptorFromDynamicOffsetIndex(set_dyn_offset);
                            assert(descriptor != nullptr);
                            // Currently only GeneralBuffer are dynamic and need to be checked
                            if (descriptor->GetClass() == vvl::DescriptorClass::GeneralBuffer) {
                                const auto *buffer_descriptor = static_cast<const vvl::BufferDescriptor *>(descriptor);
                                const VkDeviceSize bound_range = buffer_descriptor->GetRange();
                                const VkDeviceSize bound_offset = buffer_descriptor->GetOffset();
                                // NOTE: null / invalid buffers may show up here, errors are raised elsewhere for this.
                                auto buffer_state = buffer_descriptor->GetBufferState();

                                // Validate offset didn't go over buffer
                                if ((bound_range == VK_WHOLE_SIZE) && (offset > 0)) {
                                    const LogObjectList objlist(cb_state.commandBuffer(), pDescriptorSets[set_idx],
                                                                buffer_descriptor->GetBuffer());
                                    const char *vuid = is_2 ? "VUID-VkBindDescriptorSetsInfoKHR-pDescriptorSets-06715"
                                                            : "VUID-vkCmdBindDescriptorSets-pDescriptorSets-06715";
                                    skip |= LogError(vuid, objlist, loc.dot(Field::pDynamicOffsets, cur_dyn_offset),
                                                     "is %" PRIu32
                                                     ", but must be zero since "
                                                     "the buffer descriptor's range is VK_WHOLE_SIZE in descriptorSet #%" PRIu32
                                                     " binding #%" PRIu32
                                                     " "
                                                     "descriptor[%" PRIu32 "].",
                                                     offset, set_idx, binding_index, j);

                                } else if (buffer_state && (bound_range != VK_WHOLE_SIZE) &&
                                           ((offset + bound_range + bound_offset) > buffer_state->createInfo.size)) {
                                    const LogObjectList objlist(cb_state.commandBuffer(), pDescriptorSets[set_idx],
                                                                buffer_descriptor->GetBuffer());
                                    const char *vuid = is_2 ? "VUID-VkBindDescriptorSetsInfoKHR-pDescriptorSets-01979"
                                                            : "VUID-vkCmdBindDescriptorSets-pDescriptorSets-01979";
                                    skip |=
                                        LogError(vuid, objlist, loc.dot(Field::pDynamicOffsets, cur_dyn_offset),
                                                 "is %" PRIu32 ", which when added to the buffer descriptor's range (%" PRIu64
                                                 ") and offset (%" PRIu64 ") is greater than the size of the buffer (%" PRIu64
                                                 ") in descriptorSet #%" PRIu32 " binding #%" PRIu32 " descriptor[%" PRIu32 "].",
                                                 offset, bound_range, bound_offset, buffer_state->createInfo.size, set_idx,
                                                 binding_index, j);
                                }
                            }
                            cur_dyn_offset++;
                            set_dyn_offset++;
                        }  // descriptorCount loop
                    }      // bindingCount loop
                    // Keep running total of dynamic descriptor count to verify at the end
                    total_dynamic_descriptors += set_dynamic_descriptor_count;
                }
            }
            if (descriptor_set->GetPoolState()->createInfo.flags & VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT) {
                const LogObjectList objlist(cb_state.commandBuffer(), pDescriptorSets[set_idx],
                                            descriptor_set->GetPoolState()->Handle());
                const char *vuid = is_2 ? "VUID-VkBindDescriptorSetsInfoKHR-pDescriptorSets-04616"
                                        : "VUID-vkCmdBindDescriptorSets-pDescriptorSets-04616";
                skip |= LogError(vuid, objlist, set_loc,
                                 "was allocated from a pool that was created with VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT.");
            }
        } else if (!enabled_features.graphicsPipelineLibrary) {
            const LogObjectList objlist(cb_state.commandBuffer(), pDescriptorSets[set_idx]);
            const char *vuid = is_2 ? "VUID-VkBindDescriptorSetsInfoKHR-pDescriptorSets-06563"
                                    : "VUID-vkCmdBindDescriptorSets-pDescriptorSets-06563";
            skip |= LogError(vuid, objlist, set_loc,
                             "(%s) that does not exist, and the layout was not created "
                             "VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT.",
                             FormatHandle(pDescriptorSets[set_idx]).c_str());
        }
    }
    //  dynamicOffsetCount must equal the total number of dynamic descriptors in the sets being bound
    if (total_dynamic_descriptors != dynamicOffsetCount) {
        const char *vuid = is_2 ? "VUID-VkBindDescriptorSetsInfoKHR-dynamicOffsetCount-00359"
                                : "VUID-vkCmdBindDescriptorSets-dynamicOffsetCount-00359";
        skip |= LogError(vuid, cb_state.commandBuffer(), loc,
                         "Attempting to bind %" PRIu32 " descriptorSets with %" PRIu32
                         " dynamic descriptors, but "
                         "dynamicOffsetCount is %" PRIu32
                         ". It should "
                         "exactly match the number of dynamic descriptors.",
                         setCount, total_dynamic_descriptors, dynamicOffsetCount);
    }
    // firstSet and descriptorSetCount sum must be less than setLayoutCount
    if ((firstSet + setCount) > static_cast<uint32_t>(pipeline_layout->set_layouts.size())) {
        const char *vuid = is_2 ? "VUID-VkBindDescriptorSetsInfoKHR-firstSet-00360" : "VUID-vkCmdBindDescriptorSets-firstSet-00360";
        skip |= LogError(vuid, cb_state.commandBuffer(), loc,
                         "Sum of firstSet (%" PRIu32 ") and descriptorSetCount (%" PRIu32
                         ") is greater than "
                         "VkPipelineLayoutCreateInfo::setLayoutCount "
                         "(%zu) when pipeline layout was created",
                         firstSet, setCount, pipeline_layout->set_layouts.size());
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                      VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                                      const VkDescriptorSet *pDescriptorSets, uint32_t dynamicOffsetCount,
                                                      const uint32_t *pDynamicOffsets, const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);
    skip |= ValidateCmdBindDescriptorSets(*cb_state, layout, firstSet, setCount, pDescriptorSets, dynamicOffsetCount,
                                          pDynamicOffsets, error_obj.location);
    skip |= ValidatePipelineBindPoint(cb_state.get(), pipelineBindPoint, error_obj.location);

    return skip;
}

bool CoreChecks::PreCallValidateCmdBindDescriptorSets2KHR(VkCommandBuffer commandBuffer,
                                                          const VkBindDescriptorSetsInfoKHR *pBindDescriptorSetsInfo,
                                                          const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    bool skip = false;

    skip |= ValidateCmd(*cb_state, error_obj.location);
    skip |= ValidateCmdBindDescriptorSets(*cb_state, pBindDescriptorSetsInfo->layout, pBindDescriptorSetsInfo->firstSet,
                                          pBindDescriptorSetsInfo->descriptorSetCount, pBindDescriptorSetsInfo->pDescriptorSets,
                                          pBindDescriptorSetsInfo->dynamicOffsetCount, pBindDescriptorSetsInfo->pDynamicOffsets,
                                          error_obj.location.dot(Field::pBindDescriptorSetsInfo));

    if (!enabled_features.dynamicPipelineLayout && pBindDescriptorSetsInfo->layout == VK_NULL_HANDLE) {
        skip |= LogError("VUID-VkBindDescriptorSetsInfoKHR-None-09495", device,
                         error_obj.location.dot(Field::pBindDescriptorSetsInfo).dot(Field::layout), "is not valid.");
    }

    if (IsStageInPipelineBindPoint(pBindDescriptorSetsInfo->stageFlags, VK_PIPELINE_BIND_POINT_GRAPHICS)) {
        skip |= ValidatePipelineBindPoint(cb_state.get(), VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    }
    if (IsStageInPipelineBindPoint(pBindDescriptorSetsInfo->stageFlags, VK_PIPELINE_BIND_POINT_COMPUTE)) {
        skip |= ValidatePipelineBindPoint(cb_state.get(), VK_PIPELINE_BIND_POINT_COMPUTE, error_obj.location);
    }
    if (IsStageInPipelineBindPoint(pBindDescriptorSetsInfo->stageFlags, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR)) {
        skip |= ValidatePipelineBindPoint(cb_state.get(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, error_obj.location);
    }

    return skip;
}

bool CoreChecks::ValidateDescriptorSetLayoutBindingFlags(const VkDescriptorSetLayoutCreateInfo *pCreateInfo, uint32_t max_binding,
                                                         uint32_t *update_after_bind, const Location &loc) const {
    bool skip = false;
    const auto *flags_info = vku::FindStructInPNextChain<VkDescriptorSetLayoutBindingFlagsCreateInfo>(pCreateInfo->pNext);
    if (!flags_info) {
        return skip;
    }
    if (flags_info->bindingCount != 0 && flags_info->bindingCount != pCreateInfo->bindingCount) {
        skip |= LogError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-bindingCount-03002", device,
                         loc.pNext(Struct::VkDescriptorSetLayoutBindingFlagsCreateInfo, Field::bindingCount),
                         "(%" PRIu32 ") is different from pCreateInfo->bindingCount (%" PRIu32 ").", flags_info->bindingCount,
                         pCreateInfo->bindingCount);
    }

    if (flags_info->bindingCount != pCreateInfo->bindingCount) {
        return skip;  // nothing left to validate
    }
    for (uint32_t i = 0; i < pCreateInfo->bindingCount; ++i) {
        const auto &binding_info = pCreateInfo->pBindings[i];
        const Location binding_flags_loc = loc.pNext(Struct::VkDescriptorSetLayoutBindingFlagsCreateInfo, Field::pBindingFlags, i);

        if (flags_info->pBindingFlags[i] & VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT) {
            *update_after_bind = i;
            if ((pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT) == 0) {
                skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-flags-03000", device, binding_flags_loc,
                                 "includes VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT but pCreateInfo->flags is %s.",
                                 string_VkDescriptorSetLayoutCreateFlags(pCreateInfo->flags).c_str());
            }

            if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER &&
                !enabled_features.descriptorBindingUniformBufferUpdateAfterBind) {
                skip |=
                    LogError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingUniformBufferUpdateAfterBind-03005",
                             device, binding_flags_loc,
                             "includes VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT but pBindings[%" PRIu32
                             "].descriptorType is VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT "
                             "but descriptorBindingUniformBufferUpdateAfterBind was not enabled.",
                             i);
            }
            if ((binding_info.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER ||
                 binding_info.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
                 binding_info.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) &&
                !enabled_features.descriptorBindingSampledImageUpdateAfterBind) {
                skip |=
                    LogError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingSampledImageUpdateAfterBind-03006",
                             device, binding_flags_loc,
                             "includes VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT but pBindings[%" PRIu32
                             "].descriptorType is %s "
                             "but descriptorBindingSampledImageUpdateAfterBind was not enabled.",
                             i, string_VkDescriptorType(binding_info.descriptorType));
            }
            if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE &&
                !enabled_features.descriptorBindingStorageImageUpdateAfterBind) {
                skip |=
                    LogError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingStorageImageUpdateAfterBind-03007",
                             device, binding_flags_loc,
                             "includes VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT but pBindings[%" PRIu32
                             "].descriptorType is VK_DESCRIPTOR_TYPE_STORAGE_IMAGE "
                             "but descriptorBindingStorageImageUpdateAfterBind was not enabled.",
                             i);
            }
            if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER &&
                !enabled_features.descriptorBindingStorageBufferUpdateAfterBind) {
                skip |=
                    LogError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingStorageBufferUpdateAfterBind-03008",
                             device, binding_flags_loc,
                             "includes VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT but pBindings[%" PRIu32
                             "].descriptorType is VK_DESCRIPTOR_TYPE_STORAGE_BUFFER "
                             "but descriptorBindingStorageBufferUpdateAfterBind was not enabled.",
                             i);
            }
            if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER &&
                !enabled_features.descriptorBindingUniformTexelBufferUpdateAfterBind) {
                skip |= LogError(
                    "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingUniformTexelBufferUpdateAfterBind-03009",
                    device, binding_flags_loc,
                    "includes VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT but pBindings[%" PRIu32
                    "].descriptorType is VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER "
                    "but descriptorBindingUniformTexelBufferUpdateAfterBind was not enabled.",
                    i);
            }
            if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER &&
                !enabled_features.descriptorBindingStorageTexelBufferUpdateAfterBind) {
                skip |= LogError(
                    "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingStorageTexelBufferUpdateAfterBind-03010",
                    device, binding_flags_loc,
                    "includes VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT but pBindings[%" PRIu32
                    "].descriptorType is VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER "
                    "but descriptorBindingStorageTexelBufferUpdateAfterBind was not enabled.",
                    i);
            }
            if ((binding_info.descriptorType == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT ||
                 binding_info.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
                 binding_info.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)) {
                skip |= LogError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-None-03011", device, binding_flags_loc,
                                 "includes VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT but pBindings[%" PRIu32
                                 "].descriptorType is %s.",
                                 i, string_VkDescriptorType(binding_info.descriptorType));
            }

            if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT &&
                !enabled_features.descriptorBindingInlineUniformBlockUpdateAfterBind) {
                skip |= LogError(
                    "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingInlineUniformBlockUpdateAfterBind-02211",
                    device, binding_flags_loc,
                    "includes VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT but pBindings[%" PRIu32
                    "].descriptorType is VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT "
                    "but descriptorBindingInlineUniformBlockUpdateAfterBind was not enabled.",
                    i);
            }
            if ((binding_info.descriptorType == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR ||
                 binding_info.descriptorType == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV) &&
                !enabled_features.descriptorBindingAccelerationStructureUpdateAfterBind) {
                skip |= LogError(
                    "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingAccelerationStructureUpdateAfterBind-03570",
                    device, binding_flags_loc,
                    "includes VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT but pBindings[%" PRIu32
                    "].descriptorType is %s, but the descriptorBindingAccelerationStructureUpdateAfterBind was not enabled.",
                    i, string_VkDescriptorType(binding_info.descriptorType));
            }
        }

        if (flags_info->pBindingFlags[i] & VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT) {
            if (!enabled_features.descriptorBindingUpdateUnusedWhilePending) {
                skip |= LogError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingUpdateUnusedWhilePending-03012",
                                 device, binding_flags_loc,
                                 "includes VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT but pBindings[%" PRIu32
                                 "].descriptorType is %s, but the descriptorBindingUpdateUnusedWhilePending was not enabled.",
                                 i, string_VkDescriptorType(binding_info.descriptorType));
            }
        }

        if (flags_info->pBindingFlags[i] & VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT) {
            if (!enabled_features.descriptorBindingPartiallyBound) {
                skip |= LogError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingPartiallyBound-03013", device,
                                 binding_flags_loc,
                                 "includes VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT but pBindings[%" PRIu32
                                 "].descriptorType is %s, but the descriptorBindingPartiallyBound was not enabled.",
                                 i, string_VkDescriptorType(binding_info.descriptorType));
            }
        }

        if (flags_info->pBindingFlags[i] & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT) {
            if (binding_info.binding != max_binding) {
                skip |= LogError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-pBindingFlags-03004", device, binding_flags_loc,
                                 "includes VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT "
                                 "but %" PRIu32 " is the largest value of all the bindings.",
                                 binding_info.binding);
            }

            if (!enabled_features.descriptorBindingVariableDescriptorCount) {
                skip |= LogError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingVariableDescriptorCount-03014",
                                 device, binding_flags_loc,
                                 "includes VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT, but the "
                                 "descriptorBindingVariableDescriptorCount feature was not enabled.");
            }
            if ((binding_info.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) ||
                (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)) {
                skip |= LogError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-pBindingFlags-03015", device, binding_flags_loc,
                                 "includes VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT but pBindings[%" PRIu32
                                 "].descriptorType is %s.",
                                 i, string_VkDescriptorType(binding_info.descriptorType));
            }
        }

        const bool push_descriptor_set = (pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR) != 0;
        if (push_descriptor_set && (flags_info->pBindingFlags[i] & (VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
                                                                    VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT |
                                                                    VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT))) {
            skip |= LogError("VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-flags-03003", device, binding_flags_loc,
                             "is %s (which includes CREATE_PUSH_DESCRIPTOR_BIT).",
                             string_VkDescriptorBindingFlags(flags_info->pBindingFlags[i]).c_str());
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                          const VkAllocationCallbacks *pAllocator,
                                                          VkDescriptorSetLayout *pSetLayout, const ErrorObject &error_obj) const {
    bool skip = false;
    vvl::unordered_set<uint32_t> bindings;
    uint64_t total_descriptors = 0;

    const bool push_descriptor_set = (pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR) != 0;

    uint32_t max_binding = 0;

    uint32_t update_after_bind = pCreateInfo->bindingCount;
    uint32_t uniform_buffer_dynamic = pCreateInfo->bindingCount;
    uint32_t storage_buffer_dynamic = pCreateInfo->bindingCount;
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);

    for (uint32_t i = 0; i < pCreateInfo->bindingCount; ++i) {
        const Location binding_loc = create_info_loc.dot(Field::pBindings, i);
        const auto &binding_info = pCreateInfo->pBindings[i];
        max_binding = std::max(max_binding, binding_info.binding);

        if (!bindings.insert(binding_info.binding).second) {
            skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-binding-00279", device, binding_loc.dot(Field::binding),
                             "is duplicated at pBindings[%" PRIu32 "].binding.", binding_info.binding);
        }

        if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT) {
            if (!enabled_features.inlineUniformBlock) {
                skip |= LogError(
                    "VUID-VkDescriptorSetLayoutBinding-descriptorType-04604", device, binding_loc.dot(Field::descriptorType),
                    "is VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT, but the inlineUniformBlock feature was not enabled.");
            } else if (push_descriptor_set) {
                skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-flags-02208", device, binding_loc.dot(Field::descriptorType),
                                 "is VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT but "
                                 "pCreateInfo->flags includes VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR.");
            } else {
                if ((binding_info.descriptorCount % 4) != 0) {
                    skip |= LogError("VUID-VkDescriptorSetLayoutBinding-descriptorType-02209", device,
                                     binding_loc.dot(Field::descriptorCount), "(%" PRIu32 ") (must be a multiple of 4).",
                                     binding_info.descriptorCount);
                }
                if ((binding_info.descriptorCount > phys_dev_ext_props.inline_uniform_block_props.maxInlineUniformBlockSize) &&
                    !(pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT)) {
                    skip |= LogError(
                        "VUID-VkDescriptorSetLayoutBinding-descriptorType-08004", device, binding_loc.dot(Field::descriptorCount),
                        "(%" PRIu32 ") but must be less than or equal to maxInlineUniformBlockSize (%" PRIu32
                        "), but "
                        "pCreateInfo->flags is %s.",
                        binding_info.descriptorCount, phys_dev_ext_props.inline_uniform_block_props.maxInlineUniformBlockSize,
                        string_VkDescriptorSetLayoutCreateFlags(pCreateInfo->flags).c_str());
                }
            }
        } else if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
            uniform_buffer_dynamic = i;
            if (push_descriptor_set) {
                skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-flags-00280", device, binding_loc.dot(Field::descriptorType),
                                 "is VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, but pCreateInfo->flags includes "
                                 "VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR.");
            }
        } else if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) {
            storage_buffer_dynamic = i;
            if (push_descriptor_set) {
                skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-flags-00280", device, binding_loc.dot(Field::descriptorType),
                                 "is VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, but pCreateInfo->flags includes "
                                 "VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR.");
            }
        }

        if ((binding_info.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER ||
             binding_info.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) &&
            binding_info.pImmutableSamplers) {
            for (uint32_t j = 0; j < binding_info.descriptorCount; j++) {
                auto sampler_state = Get<vvl::Sampler>(binding_info.pImmutableSamplers[j]);
                if (sampler_state && (sampler_state->createInfo.borderColor == VK_BORDER_COLOR_INT_CUSTOM_EXT ||
                                      sampler_state->createInfo.borderColor == VK_BORDER_COLOR_FLOAT_CUSTOM_EXT)) {
                    skip |= LogError("VUID-VkDescriptorSetLayoutBinding-pImmutableSamplers-04009", device,
                                     binding_loc.dot(Field::pImmutableSamplers, j),
                                     "(%s) presented as immutable has a custom border color.",
                                     FormatHandle(binding_info.pImmutableSamplers[j]).c_str());
                }
            }
        }

        if (binding_info.descriptorType == VK_DESCRIPTOR_TYPE_MUTABLE_EXT && binding_info.pImmutableSamplers != nullptr) {
            skip |=
                LogError("VUID-VkDescriptorSetLayoutBinding-descriptorType-04605", device, binding_loc.dot(Field::descriptorType),
                         "is VK_DESCRIPTOR_TYPE_MUTABLE_EXT but pImmutableSamplers is not NULL.");
        }

        if (pCreateInfo->flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT) {
            if (binding_info.descriptorType != VK_DESCRIPTOR_TYPE_SAMPLER) {
                skip |= LogError(
                    "VUID-VkDescriptorSetLayoutBinding-flags-08005", device, binding_loc.dot(Field::descriptorType),
                    "is %s but pCreateInfo->flags includes VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT.",
                    string_VkDescriptorType(binding_info.descriptorType));
            }

            if (binding_info.descriptorCount > 1) {
                skip |= LogError(
                    "VUID-VkDescriptorSetLayoutBinding-flags-08006", device, binding_loc.dot(Field::descriptorCount),
                    "is %" PRIu32
                    " but pCreateInfo->flags includes VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT.",
                    binding_info.descriptorCount);
            }

            if ((binding_info.descriptorCount == 1) && (binding_info.pImmutableSamplers == nullptr)) {
                skip |= LogError("VUID-VkDescriptorSetLayoutBinding-flags-08007", device, binding_loc.dot(Field::descriptorCount),
                                 "is 1 and pImmutableSamplers is NULL, but pCreateInfo->flags includes "
                                 "VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT.");
            }
        }

        total_descriptors += binding_info.descriptorCount;
    }

    skip |= ValidateDescriptorSetLayoutBindingFlags(pCreateInfo, max_binding, &update_after_bind, create_info_loc);

    if (update_after_bind < pCreateInfo->bindingCount) {
        if (uniform_buffer_dynamic < pCreateInfo->bindingCount) {
            skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-descriptorType-03001", device,
                             create_info_loc.dot(Field::pBindings, update_after_bind),
                             "has VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT "
                             "flag, but pBindings[%" PRIu32 "] has descriptor type VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC.",
                             uniform_buffer_dynamic);
        }
        if (storage_buffer_dynamic < pCreateInfo->bindingCount) {
            skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-descriptorType-03001", device,
                             create_info_loc.dot(Field::pBindings, update_after_bind),
                             "has VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT "
                             "flag, but pBindings[%" PRIu32 "] has descriptor type VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC.",
                             storage_buffer_dynamic);
        }
    }

    if ((push_descriptor_set) && (total_descriptors > phys_dev_ext_props.push_descriptor_props.maxPushDescriptors)) {
        skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-flags-00281", device, error_obj.location,
                         "for push descriptor, total descriptor count in layout (%" PRIu64
                         ") must not be greater than maxPushDescriptors (%" PRIu32 ").",
                         total_descriptors, phys_dev_ext_props.push_descriptor_props.maxPushDescriptors);
    }

    return skip;
}

// Validate that the state of this set is appropriate for the given bindings and dynamic_offsets at Draw time
//  This includes validating that all descriptors in the given bindings are updated,
//  that any update buffers are valid, and that any dynamic offsets are within the bounds of their buffers.
// Return true if state is acceptable, or false and write an error message into error string
bool CoreChecks::ValidateDrawState(const DescriptorSet &descriptor_set, const BindingVariableMap &bindings,
                                   const std::vector<uint32_t> &dynamic_offsets, const vvl::CommandBuffer &cb_state,
                                   const Location &loc, const vvl::DrawDispatchVuid &vuids) const {
    bool result = false;
    VkFramebuffer framebuffer = cb_state.activeFramebuffer ? cb_state.activeFramebuffer->framebuffer() : VK_NULL_HANDLE;
    // NOTE: GPU-AV needs non-const state objects to do lazy updates of descriptor state of only the dynamically used
    // descriptors, via the non-const version of ValidateBinding(), this code uses the const path only even it gives up
    // non-const versions of its state objects here.
    const vvl::DescriptorValidator desc_val(const_cast<CoreChecks &>(*this), const_cast<vvl::CommandBuffer &>(cb_state),
                                            const_cast<DescriptorSet &>(descriptor_set), framebuffer, loc);

    for (const auto &binding_pair : bindings) {
        const auto *binding = descriptor_set.GetBinding(binding_pair.first);
        if (!binding) {  //  End at construction is the condition for an invalid binding.
            auto set = descriptor_set.Handle();
            result |= LogError(vuids.descriptor_buffer_bit_set_08114, set, loc, "%s binding #%" PRIu32 " is invalid.",
                               FormatHandle(set).c_str(), binding_pair.first);
            return result;
        }

        if (descriptor_set.SkipBinding(*binding)) {
            continue;
        }
        result |= desc_val.ValidateBinding(binding_pair, *binding);
    }
    return result;
}

// Starting at offset descriptor of given binding, parse over update_count
//  descriptor updates and verify that for any binding boundaries that are crossed, the next binding(s) are all consistent
//  Consistency means that their type, stage flags, and whether or not they use immutable samplers matches
bool CoreChecks::VerifyUpdateConsistency(const DescriptorSet &set, uint32_t binding, uint32_t offset, uint32_t update_count,
                                         const char *vuid, const Location &set_loc) const {
    bool skip = false;
    auto current_iter = set.FindBinding(binding);
    // Verify consecutive bindings match (if needed)
    auto &orig_binding = **current_iter;
    while (!skip && update_count) {
        // First, it's legal to offset beyond your own binding so handle that case
        if (offset > 0) {
            // index_range.start + offset is which descriptor is needed to update. If it > index_range.end, it means the descriptor
            // isn't in this binding, maybe in next binding.
            if (offset > (*current_iter)->count) {
                // Advance to next binding, decrement offset by binding size
                offset -= (*current_iter)->count;
                ++current_iter;
                // Verify next consecutive binding matches type, stage flags & immutable sampler use and if AtEnd
                if (current_iter == set.end() || !orig_binding.IsConsistent(**current_iter)) {
                    skip = true;
                }
                continue;
            }
        }

        update_count -= std::min(update_count, (*current_iter)->count - offset);
        if (update_count) {
            // Starting offset is beyond the current binding. Check consistency, update counters and advance to the next binding,
            // looking for the start point. All bindings (even those skipped) must be consistent with the update and with the
            // original binding.
            offset = 0;
            ++current_iter;
            // Verify next consecutive binding matches type, stage flags & immutable sampler use and if AtEnd
            if (current_iter == set.end() || !orig_binding.IsConsistent(**current_iter)) {
                skip = true;
            }
        }
    }

    if (skip) {
        std::stringstream error_str;
        if (set.IsPushDescriptor()) {
            error_str << " push descriptors";
        } else {
            error_str << " descriptor set " << FormatHandle(set);
        }
        error_str << " binding #" << orig_binding.binding << " with #" << update_count
                  << " descriptors being updated but this update oversteps the bounds of this binding and the next binding is "
                     "not consistent with current binding";
        if (current_iter == set.end()) {
            error_str << " (update past the end of the descriptor set)";
        } else {
            auto current_binding = current_iter->get();
            // Get what was not consistent in IsConsistent() as a more detailed error message
            if (current_binding->type != orig_binding.type) {
                error_str << " (" << string_VkDescriptorType(current_binding->type)
                          << " != " << string_VkDescriptorType(orig_binding.type) << ")";
            } else if (current_binding->stage_flags != orig_binding.stage_flags) {
                error_str << " (" << string_VkShaderStageFlags(current_binding->stage_flags)
                          << " != " << string_VkShaderStageFlags(orig_binding.stage_flags) << ")";
            } else if (current_binding->has_immutable_samplers != orig_binding.has_immutable_samplers) {
                error_str << " (pImmutableSamplers don't match)";
            } else if (current_binding->binding_flags != orig_binding.binding_flags) {
                error_str << " (" << string_VkDescriptorBindingFlags(current_binding->binding_flags)
                          << " != " << string_VkDescriptorBindingFlags(orig_binding.binding_flags) << ")";
            }
        }

        error_str << " so this update is invalid";
        LogError(vuid, set.Handle(), set_loc, "%s", error_str.str().c_str());
    }
    return skip;
}

// Validate Copy update
bool CoreChecks::ValidateCopyUpdate(const VkCopyDescriptorSet &update, const Location& copy_loc) const {
    bool skip = false;
    const auto &src_set = *Get<vvl::DescriptorSet>(update.srcSet);
    const auto &dst_set = *Get<vvl::DescriptorSet>(update.dstSet);
    const auto *dst_layout = dst_set.GetLayout().get();
    const auto *src_layout = src_set.GetLayout().get();

    if (dst_layout->Destroyed()) {
        const LogObjectList objlist(update.dstSet, dst_layout->Handle());
        return LogError("VUID-VkCopyDescriptorSet-dstSet-parameter", objlist, copy_loc.dot(Field::dstSet),
                        "(%s) has been destroyed.", FormatHandle(dst_layout->Handle()).c_str());
    }

    if (src_layout->Destroyed()) {
        const LogObjectList objlist(update.srcSet, src_layout->Handle());
        return LogError("VUID-VkCopyDescriptorSet-srcSet-parameter", objlist, copy_loc.dot(Field::srcSet),
                        "(%s) has been destroyed.", FormatHandle(src_layout->Handle()).c_str());
    }

    if (!dst_layout->HasBinding(update.dstBinding)) {
        const LogObjectList objlist(update.dstSet, dst_layout->Handle());
        return LogError("VUID-VkCopyDescriptorSet-dstBinding-00347", objlist, copy_loc.dot(Field::dstBinding),
                        "(%" PRIu32 ") does not exist in %s.", update.dstBinding, FormatHandle(dst_set.Handle()).c_str());
    }
    if (!src_set.HasBinding(update.srcBinding)) {
        const LogObjectList objlist(update.srcSet, src_layout->Handle());
        return LogError("VUID-VkCopyDescriptorSet-srcBinding-00345", objlist, copy_loc.dot(Field::srcBinding),
                        "(%" PRIu32 ") does not exist in %s.", update.srcBinding, FormatHandle(src_set.Handle()).c_str());
    }

    // src & dst set bindings are valid
    // Check bounds of src & dst
    auto src_start_idx = src_set.GetGlobalIndexRangeFromBinding(update.srcBinding).start + update.srcArrayElement;
    if ((src_start_idx + update.descriptorCount) > src_set.GetTotalDescriptorCount()) {
        const LogObjectList objlist(update.srcSet, src_layout->Handle());
        skip |= LogError("VUID-VkCopyDescriptorSet-srcArrayElement-00346", objlist, copy_loc.dot(Field::srcArrayElement),
                         "(%" PRIu32 ") plus descriptorCount (%" PRIu32 ") (plus offset index of %" PRIu32
                         ") is larger than the total descriptors count (%" PRIu32 ") for the binding at srcBinding (%" PRIu32 ").",
                         update.srcArrayElement, update.descriptorCount,
                         src_set.GetGlobalIndexRangeFromBinding(update.srcBinding).start, src_set.GetTotalDescriptorCount(),
                         update.srcBinding);
    }
    auto dst_start_idx = dst_layout->GetGlobalIndexRangeFromBinding(update.dstBinding).start + update.dstArrayElement;
    if ((dst_start_idx + update.descriptorCount) > dst_layout->GetTotalDescriptorCount()) {
        const LogObjectList objlist(update.dstSet, dst_layout->Handle());
        skip |= LogError("VUID-VkCopyDescriptorSet-dstArrayElement-00348", objlist, copy_loc.dot(Field::dstArrayElement),
                         "(%" PRIu32 ") plus descriptorCount (%" PRIu32 ") (plus offset index of %" PRIu32
                         ") is larger than the total descriptors count (%" PRIu32 ") for the binding at srcBinding (%" PRIu32 ").",
                         update.dstArrayElement, update.descriptorCount,
                         dst_set.GetGlobalIndexRangeFromBinding(update.dstBinding).start, dst_set.GetTotalDescriptorCount(),
                         update.dstBinding);
    }
    // Check that types match
    VkDescriptorType src_type = src_layout->GetTypeFromBinding(update.srcBinding);
    VkDescriptorType dst_type = dst_layout->GetTypeFromBinding(update.dstBinding);
    if (src_type != VK_DESCRIPTOR_TYPE_MUTABLE_EXT && dst_type != VK_DESCRIPTOR_TYPE_MUTABLE_EXT && src_type != dst_type) {
        const LogObjectList objlist(update.srcSet, update.dstSet, src_layout->Handle(), dst_layout->Handle());
        skip |= LogError("VUID-VkCopyDescriptorSet-dstBinding-02632", objlist, copy_loc.dot(Field::dstBinding),
                         "(%" PRIu32 ") (from %s) is %s, but srcBinding (%" PRIu32 ") (from %s) is %s.", update.dstBinding,
                         FormatHandle(dst_set.Handle()).c_str(), string_VkDescriptorType(dst_type), update.srcBinding,
                         FormatHandle(src_set.Handle()).c_str(), string_VkDescriptorType(src_type));
    }

    if (skip) {
        return skip;  // consistency will likley be wrong if already bad
    }
    // Verify consistency of src & dst bindings if update crosses binding boundaries
    skip |= VerifyUpdateConsistency(src_set, update.srcBinding, update.srcArrayElement, update.descriptorCount,
                                    "VUID-VkCopyDescriptorSet-srcSet-00349", copy_loc.dot(Field::dstSet));
    skip |= VerifyUpdateConsistency(dst_set, update.dstBinding, update.dstArrayElement, update.descriptorCount,
                                    "VUID-VkCopyDescriptorSet-srcSet-00349", copy_loc.dot(Field::srcSet));

    if ((src_layout->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT) &&
        !(dst_layout->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)) {
        const LogObjectList objlist(update.srcSet, update.dstSet, src_layout->Handle(), dst_layout->Handle());
        skip |= LogError("VUID-VkCopyDescriptorSet-srcSet-01918", objlist, copy_loc.dot(Field::srcSet),
                         "layout was created with %s, but dstSet layout was created with %s.",
                         string_VkDescriptorSetLayoutCreateFlags(src_layout->GetCreateFlags()).c_str(),
                         string_VkDescriptorSetLayoutCreateFlags(dst_layout->GetCreateFlags()).c_str());
    }

    if (!(src_layout->GetCreateFlags() &
          (VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT | VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT)) &&
        (dst_layout->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)) {
        const LogObjectList objlist(update.srcSet, update.dstSet, src_layout->Handle(), dst_layout->Handle());
        skip |= LogError("VUID-VkCopyDescriptorSet-srcSet-04885", objlist, copy_loc.dot(Field::srcSet),
                         "layout was created with %s, but dstSet layout was created with %s.",
                         string_VkDescriptorSetLayoutCreateFlags(src_layout->GetCreateFlags()).c_str(),
                         string_VkDescriptorSetLayoutCreateFlags(dst_layout->GetCreateFlags()).c_str());
    }

    const auto &src_pool = *src_set.GetPoolState();
    const auto &dst_pool = *dst_set.GetPoolState();
    if ((src_pool.createInfo.flags & VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT) &&
        !(dst_pool.createInfo.flags & VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)) {
        const LogObjectList objlist(update.srcSet, update.dstSet, src_pool.Handle(), dst_pool.Handle());
        skip |= LogError("VUID-VkCopyDescriptorSet-srcSet-01920", objlist, copy_loc.dot(Field::srcSet),
                         "descriptor pool was created with %s, but dstSet descriptor pool was created with %s.",
                         string_VkDescriptorPoolCreateFlags(src_pool.createInfo.flags).c_str(),
                         string_VkDescriptorPoolCreateFlags(dst_pool.createInfo.flags).c_str());
    }

    if (!(src_pool.createInfo.flags &
          (VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT)) &&
        (dst_pool.createInfo.flags & VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)) {
        const LogObjectList objlist(update.srcSet, update.dstSet, src_pool.Handle(), dst_pool.Handle());
        skip |= LogError("VUID-VkCopyDescriptorSet-srcSet-04887", objlist, copy_loc.dot(Field::srcSet),
                         "descriptor pool was created with %s, but dstSet descriptor pool was created with %s.",
                         string_VkDescriptorPoolCreateFlags(src_pool.createInfo.flags).c_str(),
                         string_VkDescriptorPoolCreateFlags(dst_pool.createInfo.flags).c_str());
    }

    if (src_type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT && ((update.srcArrayElement % 4) != 0)) {
        const LogObjectList objlist(update.srcSet, src_layout->Handle());
        skip |= LogError("VUID-VkCopyDescriptorSet-srcBinding-02223", objlist, copy_loc.dot(Field::srcArrayElement),
                         "is %" PRIu32 ", but srcBinding (%" PRIu32 ") type is VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT.",
                         update.srcArrayElement, update.srcBinding);
    }
    if (dst_type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT && ((update.dstArrayElement % 4) != 0)) {
        const LogObjectList objlist(update.dstSet, dst_layout->Handle());
        skip |= LogError("VUID-VkCopyDescriptorSet-dstBinding-02224", objlist, copy_loc.dot(Field::dstArrayElement),
                         "is %" PRIu32 ", but dstBinding (%" PRIu32 ") type is VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT.",
                         update.dstArrayElement, update.dstBinding);
    }
    if (src_type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT || dst_type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT) {
        if ((update.descriptorCount % 4) != 0) {
            const LogObjectList objlist(update.srcSet, update.dstSet, src_layout->Handle(), dst_layout->Handle());
            skip |= LogError("VUID-VkCopyDescriptorSet-srcBinding-02225", objlist, copy_loc.dot(Field::descriptorCount),
                             "is %" PRIu32 ", but srcBinding (%" PRIu32 ") type is %s and dstBinding (%" PRIu32 ") type is %s.",
                             update.descriptorCount, update.srcBinding, string_VkDescriptorType(src_type), update.dstBinding,
                             string_VkDescriptorType(dst_type));
        }
    }

    if (dst_type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
        if (src_type != VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
            if (!dst_layout->IsTypeMutable(src_type, update.dstBinding)) {
                const LogObjectList objlist(update.srcSet, update.dstSet, src_layout->Handle(), dst_layout->Handle());
                skip |= LogError("VUID-VkCopyDescriptorSet-dstSet-04612", objlist, copy_loc.dot(Field::dstBinding),
                                 "(%" PRIu32
                                 ") descriptor type  is VK_DESCRIPTOR_TYPE_MUTABLE_EXT, but the new active descriptor type %s is "
                                 "not in the corresponding pMutableDescriptorTypeLists list.",
                                 update.dstBinding, string_VkDescriptorType(src_type));
            }
        }
    } else if (src_type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
        auto src_iter = src_set.FindDescriptor(update.srcBinding, update.srcArrayElement);
        for (uint32_t i = 0; i < update.descriptorCount; i++, ++src_iter) {
            const auto &mutable_src = static_cast<const vvl::MutableDescriptor &>(*src_iter);
            if (mutable_src.ActiveType() != dst_type) {
                const LogObjectList objlist(update.srcSet, update.dstSet, src_layout->Handle(), dst_layout->Handle());
                skip |= LogError("VUID-VkCopyDescriptorSet-srcSet-04613", objlist, copy_loc.dot(Field::srcBinding),
                                 "(%" PRIu32
                                 ") descriptor type  is VK_DESCRIPTOR_TYPE_MUTABLE_EXT, but the active descriptor type %sdoes not "
                                 "match the dstBinding descriptor type %s.",
                                 update.srcBinding, string_VkDescriptorType(mutable_src.ActiveType()),
                                 string_VkDescriptorType(dst_type));
            }
        }
    }

    if (dst_type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
        if (src_type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
            const auto &mutable_src_types = src_layout->GetMutableTypes(update.srcBinding);
            const auto &mutable_dst_types = dst_layout->GetMutableTypes(update.dstBinding);
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
                const LogObjectList objlist(update.srcSet, update.dstSet, src_layout->Handle(), dst_layout->Handle());
                skip |=
                    LogError("VUID-VkCopyDescriptorSet-dstSet-04614", objlist, copy_loc,
                             "Attempting copy update with dstBinding and new active descriptor type being "
                             "VK_DESCRIPTOR_TYPE_MUTABLE_EXT, but their corresponding pMutableDescriptorTypeLists do not match.");
            }
        }
    }

    // Update mutable types
    if (src_type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
        src_type = static_cast<const vvl::MutableDescriptor *>(
                       src_set.GetDescriptorFromBinding(update.srcBinding, update.srcArrayElement))
                       ->ActiveType();
    }
    if (dst_type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
        dst_type = static_cast<const vvl::MutableDescriptor *>(
                       dst_set.GetDescriptorFromBinding(update.dstBinding, update.dstArrayElement))
                       ->ActiveType();
    }

    // Update parameters all look good and descriptor updated so verify update contents
    skip |= VerifyCopyUpdateContents(update, src_set, src_type, src_start_idx, dst_set, dst_type, dst_start_idx, copy_loc);

    return skip;
}

// Validate given sampler. Currently this only checks to make sure it exists in the samplerMap
bool CoreChecks::ValidateSampler(const VkSampler sampler) const { return Get<vvl::Sampler>(sampler).get() != nullptr; }

bool CoreChecks::ValidateImageUpdate(VkImageView image_view, VkImageLayout image_layout, VkDescriptorType type,
                                     const Location &image_info_loc) const {
    bool skip = false;
    auto iv_state = Get<vvl::ImageView>(image_view);

    // Note that when an imageview is created, we validated that memory is bound so no need to re-check here
    // Validate that imageLayout is compatible with aspect_mask and image format
    //  and validate that image usage bits are correct for given usage
    VkImageAspectFlags aspect_mask = iv_state->normalized_subresource_range.aspectMask;
    VkImage image = iv_state->create_info.image;
    VkImageUsageFlags usage = 0;
    auto *image_node = iv_state->image_state.get();
    assert(image_node);

    const auto image_view_usage_info = vku::FindStructInPNextChain<VkImageViewUsageCreateInfo>(iv_state->create_info.pNext);
    const auto stencil_usage_info = vku::FindStructInPNextChain<VkImageStencilUsageCreateInfo>(image_node->createInfo.pNext);
    if (image_view_usage_info) {
        usage = image_view_usage_info->usage;
    } else {
        usage = image_node->createInfo.usage;
    }
    if (stencil_usage_info) {
        const bool stencil_aspect = (aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) > 0;
        const bool depth_aspect = (aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) > 0;
        if (stencil_aspect && !depth_aspect) {
            usage = stencil_usage_info->stencilUsage;
        } else if (stencil_aspect && depth_aspect) {
            usage &= stencil_usage_info->stencilUsage;
        }
    }

    // Validate that memory is bound to image
    // VU being worked on https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/5598
    skip |= ValidateMemoryIsBoundToImage(LogObjectList(image), *image_node, image_info_loc.dot(Field::image),
                                         "UNASSIGNED-VkDescriptorImageInfo-BoundResourceFreedMemoryAccess");

    const LogObjectList objlist(iv_state->Handle(), image_node->Handle());
    // KHR_maintenance1 allows rendering into 2D or 2DArray views which slice a 3D image,
    // but not binding them to descriptor sets.
    if (iv_state->IsDepthSliced() && image_node->createInfo.imageType == VK_IMAGE_TYPE_3D) {
        // VK_EXT_image_2d_view_of_3d allows use of VIEW_TYPE_2D in descriptor
        if (iv_state->create_info.viewType == VK_IMAGE_VIEW_TYPE_2D_ARRAY) {
            skip |= LogError("VUID-VkDescriptorImageInfo-imageView-06712", objlist, image_info_loc.dot(Field::imageView),
                             "is VK_IMAGE_VIEW_TYPE_2D_ARRAY but the image is VK_IMAGE_TYPE_3D.");
        } else if (iv_state->create_info.viewType == VK_IMAGE_VIEW_TYPE_2D) {
            // Check 06713/06714 first to alert apps without VK_EXT_image_2d_view_of_3d that the features are needed
            if (type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE && !enabled_features.image2DViewOf3D) {
                skip |= LogError("VUID-VkDescriptorImageInfo-descriptorType-06713", objlist, image_info_loc.dot(Field::imageView),
                                 "is VK_IMAGE_VIEW_TYPE_2D, the image is VK_IMAGE_VIEW_TYPE_3D, and the descriptorType is "
                                 "VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, but the image2DViewOf3D feature was not enabled.");
            }

            if ((type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE || type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) &&
                !enabled_features.sampler2DViewOf3D) {
                skip |= LogError("VUID-VkDescriptorImageInfo-descriptorType-06714", objlist, image_info_loc.dot(Field::imageView),
                                 "is VK_IMAGE_VIEW_TYPE_2D, the image is VK_IMAGE_VIEW_TYPE_3D, and the descriptorType is %s, but "
                                 "the sampler2DViewOf3D feature was not enabled.",
                                 string_VkDescriptorType(type));
            }

            if ((type != VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) && (type != VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) &&
                (type != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)) {
                skip |= LogError("VUID-VkDescriptorImageInfo-imageView-07795", objlist, image_info_loc.dot(Field::imageView),
                                 "is VK_IMAGE_VIEW_TYPE_2D, the image is VK_IMAGE_VIEW_TYPE_3D, and the descriptorType is %s.",
                                 string_VkDescriptorType(type));
            }

            if (!(image_node->createInfo.flags & VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT)) {
                skip |= LogError("VUID-VkDescriptorImageInfo-imageView-07796", objlist, image_info_loc.dot(Field::imageView),
                                 "is VK_IMAGE_VIEW_TYPE_2D, the image is VK_IMAGE_VIEW_TYPE_3D, but the image was created with %s.",
                                 string_VkImageCreateFlags(image_node->createInfo.flags).c_str());
            }
        }
    }

    if (image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        if (aspect_mask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) {
            skip |= LogError("VUID-VkDescriptorImageInfo-imageLayout-09425", objlist, image_info_loc.dot(Field::imageView),
                             "uses layout VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL but aspectMask (%s) include depth/stencil bit.",
                             string_VkImageAspectFlags(aspect_mask).c_str());
        }
    }

    if (IsValueIn(
            image_layout,
            {VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
             VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
             VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
             VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL})) {
        if (aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) {
            skip |= LogError("VUID-VkDescriptorImageInfo-imageLayout-09426", objlist, image_info_loc.dot(Field::imageView),
                             "uses layout %s but aspectMask (%s) includes color bit.", string_VkImageLayout(image_layout),
                             string_VkImageAspectFlags(aspect_mask).c_str());
        }
    }

    if (vkuFormatIsDepthOrStencil(image_node->createInfo.format)) {
        if (aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) {
            if (aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) {
                skip |= LogError("VUID-VkDescriptorImageInfo-imageView-01976", objlist, image_info_loc.dot(Field::imageView),
                                 "use layout %s and the image format (%s), but it has both STENCIL and DEPTH aspects set",
                                 string_VkImageLayout(image_layout), string_VkFormat(image_node->createInfo.format));
            }
        }
    }

    switch (type) {
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            if (iv_state->samplerConversion != VK_NULL_HANDLE) {
                skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-01946", objlist, image_info_loc.dot(Field::imageView),
                                 "is used as VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, but was created with %s",
                                 FormatHandle(iv_state->samplerConversion).c_str());
            }
            [[fallthrough]];
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
            if (!(usage & VK_IMAGE_USAGE_SAMPLED_BIT)) {
                skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-00337", objlist, image_info_loc.dot(Field::imageView),
                                 "was created with %s, but descriptorType is %s.", string_VkImageUsageFlags(usage).c_str(),
                                 string_VkDescriptorType(type));
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: {
            if (!(usage & VK_IMAGE_USAGE_STORAGE_BIT)) {
                skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-00339", objlist, image_info_loc.dot(Field::imageView),
                                 "was created with %s, but descriptorType is VK_DESCRIPTOR_TYPE_STORAGE_IMAGE.",
                                 string_VkImageUsageFlags(usage).c_str());

            } else if ((VK_IMAGE_LAYOUT_GENERAL != image_layout) &&
                       (!IsExtEnabled(device_extensions.vk_khr_shared_presentable_image) ||
                        (VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR != image_layout))) {
                skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-04152", objlist, image_info_loc.dot(Field::imageView),
                                 "image layout is %s, but descriptorType is VK_DESCRIPTOR_TYPE_STORAGE_IMAGE. (allowed layouts are "
                                 "VK_IMAGE_LAYOUT_GENERAL or VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR).",
                                 string_VkImageLayout(image_layout));
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: {
            if (!(usage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) {
                skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-00338", objlist, image_info_loc.dot(Field::imageView),
                                 "was created with %s, but descriptorType is VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT.",
                                 string_VkImageUsageFlags(usage).c_str());
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM: {
            if (!(usage & VK_IMAGE_USAGE_SAMPLE_WEIGHT_BIT_QCOM)) {
                skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-06942", objlist, image_info_loc.dot(Field::imageView),
                                 "was created with %s, but descriptorType is VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM.",
                                 string_VkImageUsageFlags(usage).c_str());
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM: {
            if (!(usage & VK_IMAGE_USAGE_SAMPLE_BLOCK_MATCH_BIT_QCOM)) {
                skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-06943", objlist, image_info_loc.dot(Field::imageView),
                                 "was created with %s, but descriptorType is VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM.",
                                 string_VkImageUsageFlags(usage).c_str());
            }
            break;
        }
        default:
            break;
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
        const static std::array<ExtensionLayout, 8> extended_layouts{{
            //  Note double brace req'd for aggregate initialization
            {VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, &DeviceExtensions::vk_khr_shared_presentable_image},
            {VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, &DeviceExtensions::vk_khr_maintenance2},
            {VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, &DeviceExtensions::vk_khr_maintenance2},
            {VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR, &DeviceExtensions::vk_khr_synchronization2},
            {VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR, &DeviceExtensions::vk_khr_synchronization2},
            {VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL, &DeviceExtensions::vk_khr_separate_depth_stencil_layouts},
            {VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL, &DeviceExtensions::vk_khr_separate_depth_stencil_layouts},
            {VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT, &DeviceExtensions::vk_ext_attachment_feedback_loop_layout},
        }};
        auto is_layout = [image_layout, this](const ExtensionLayout &ext_layout) {
            return IsExtEnabled(device_extensions.*(ext_layout.extension)) && (ext_layout.layout == image_layout);
        };

        const bool valid_layout = (std::find(valid_layouts.cbegin(), valid_layouts.cend(), image_layout) != valid_layouts.cend()) ||
                                  std::any_of(extended_layouts.cbegin(), extended_layouts.cend(), is_layout);

        if (!valid_layout) {
            // The following works as currently all 3 descriptor types share the same set of valid layouts
            const char *vuid = kVUIDUndefined;
            switch (type) {
                case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                    vuid = "VUID-VkWriteDescriptorSet-descriptorType-04149";
                    break;
                case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                    vuid = "VUID-VkWriteDescriptorSet-descriptorType-04150";
                    break;
                case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                    vuid = "VUID-VkWriteDescriptorSet-descriptorType-04151";
                    break;
                default:
                    break;
            }
            std::stringstream error_str;
            error_str << "Descriptor update with descriptorType " << string_VkDescriptorType(type)
                      << " is being updated with invalid imageLayout " << string_VkImageLayout(image_layout) << " for image "
                      << FormatHandle(image) << " in imageView " << FormatHandle(image_view)
                      << ". Allowed layouts are: VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, "
                      << "VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL";
            for (auto &ext_layout : extended_layouts) {
                if (IsExtEnabled(device_extensions.*(ext_layout.extension))) {
                    error_str << ", " << string_VkImageLayout(ext_layout.layout);
                }
            }
            skip |= LogError(vuid, objlist, image_info_loc, "%s", error_str.str().c_str());
        }
    }

    if ((type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) || (type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)) {
        const VkComponentMapping components = iv_state->create_info.components;
        if (IsIdentitySwizzle(components) == false) {
            skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-00336", objlist, image_info_loc.dot(Field::imageView),
                             "has a non-identiy swizzle component, here are the actual swizzle values:\n"
                             "r swizzle = %s\n"
                             "g swizzle = %s\n"
                             "b swizzle = %s\n"
                             "a swizzle = %s\n",
                             string_VkComponentSwizzle(components.r), string_VkComponentSwizzle(components.g),
                             string_VkComponentSwizzle(components.b), string_VkComponentSwizzle(components.a));
        }
    }

    if ((type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) && (iv_state->min_lod != 0.0f)) {
        skip |=
            LogError("VUID-VkWriteDescriptorSet-descriptorType-06450", objlist, image_info_loc.dot(Field::imageView),
                     "was created with minLod %f, but descriptorType is VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT.", iv_state->min_lod);
    }

    return skip;
}

// This is a helper function that iterates over a set of Write and Copy updates, pulls the DescriptorSet* for updated
//  sets, and then calls their respective Validate[Write|Copy]Update functions.
// If the update hits an issue for which the callback returns "true", meaning that the call down the chain should
//  be skipped, then true is returned.
// If there is no issue with the update, then false is returned.
bool CoreChecks::ValidateUpdateDescriptorSets(uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                      const VkCopyDescriptorSet* pDescriptorCopies, const Location& loc) const {
    bool skip = false;
    // Validate Write updates
    for (uint32_t i = 0; i < descriptorWriteCount; i++) {
        const Location write_loc = loc.dot(Field::pDescriptorWrites, i);
        auto dst_set = pDescriptorWrites[i].dstSet;
        const auto &set_node = *Get<vvl::DescriptorSet>(dst_set);
        skip |= ValidateWriteUpdate(set_node, pDescriptorWrites[i], write_loc, false);

        const auto *acceleration_structure_khr = vku::FindStructInPNextChain<VkWriteDescriptorSetAccelerationStructureKHR>(pDescriptorWrites[i].pNext);
        if (acceleration_structure_khr) {
            for (uint32_t j = 0; j < acceleration_structure_khr->accelerationStructureCount; ++j) {
                auto as_state = Get<vvl::AccelerationStructureKHR>(acceleration_structure_khr->pAccelerationStructures[j]);
                if (as_state && (as_state->create_infoKHR.sType == VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR &&
                                    (as_state->create_infoKHR.type != VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR &&
                                    as_state->create_infoKHR.type != VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR))) {
                    const LogObjectList objlist(dst_set, as_state->Handle());
                    skip |= LogError(
                        "VUID-VkWriteDescriptorSetAccelerationStructureKHR-pAccelerationStructures-03579", objlist,
                        write_loc.pNext(Struct::VkWriteDescriptorSetAccelerationStructureKHR, Field::pAccelerationStructures, j),
                        "was created with %s.", string_VkAccelerationStructureTypeKHR(as_state->create_infoKHR.type));
                }
            }
        }

        const auto *acceleration_structure_nv = vku::FindStructInPNextChain<VkWriteDescriptorSetAccelerationStructureNV>(pDescriptorWrites[i].pNext);
        if (acceleration_structure_nv) {
            for (uint32_t j = 0; j < acceleration_structure_nv->accelerationStructureCount; ++j) {
                auto as_state = Get<vvl::AccelerationStructureNV>(acceleration_structure_nv->pAccelerationStructures[j]);
                if (as_state && (as_state->create_infoNV.sType == VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV &&
                                    as_state->create_infoNV.info.type != VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV)) {
                    const LogObjectList objlist(dst_set, as_state->Handle());
                    skip |= LogError("VUID-VkWriteDescriptorSetAccelerationStructureNV-pAccelerationStructures-03748", objlist,
                        write_loc.pNext(Struct::VkWriteDescriptorSetAccelerationStructureKHR, Field::pAccelerationStructures, j),
                        "was created with %s.", string_VkAccelerationStructureTypeKHR(as_state->create_infoNV.info.type));
                }
            }
        }
    }

    for (uint32_t i = 0; i < descriptorCopyCount; ++i) {
        const Location copy_loc = loc.dot(Field::pDescriptorCopies, i);
        skip |= ValidateCopyUpdate(pDescriptorCopies[i], copy_loc);
    }
    return skip;
}

vvl::DecodedTemplateUpdate::DecodedTemplateUpdate(const ValidationStateTracker *device_data,
                                                              VkDescriptorSet descriptorSet,
                                                              const vvl::DescriptorUpdateTemplate *template_state,
                                                              const void *pData, VkDescriptorSetLayout push_layout) {
    auto const &create_info = template_state->create_info;
    inline_infos.resize(create_info.descriptorUpdateEntryCount);  // Make sure we have one if we need it
    inline_infos_khr.resize(create_info.descriptorUpdateEntryCount);
    inline_infos_nv.resize(create_info.descriptorUpdateEntryCount);
    desc_writes.reserve(create_info.descriptorUpdateEntryCount);  // emplaced, so reserved without initialization
    VkDescriptorSetLayout effective_dsl = create_info.templateType == VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET
                                              ? create_info.descriptorSetLayout
                                              : push_layout;
    auto layout_obj = device_data->Get<vvl::DescriptorSetLayout>(effective_dsl);

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
                    VkWriteDescriptorSetInlineUniformBlock *inline_info = &inline_infos[i];
                    inline_info->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_INLINE_UNIFORM_BLOCK_EXT;
                    inline_info->pNext = nullptr;
                    inline_info->dataSize = create_info.pDescriptorUpdateEntries[i].descriptorCount;
                    inline_info->pData = update_entry;
                    write_entry.pNext = inline_info;
                    // descriptorCount must match the dataSize member of the VkWriteDescriptorSetInlineUniformBlock structure
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

std::string vvl::DescriptorSet::StringifySetAndLayout() const {
    auto layout_handle = layout_->Handle();
    std::ostringstream str;
    if (IsPushDescriptor()) {
        str << "Push Descriptors defined with " << state_data_->FormatHandle(layout_handle);
    } else {
        str << state_data_->FormatHandle(Handle()) << " allocated with " << state_data_->FormatHandle(layout_handle);
    }
    return str.str();
}

// Loop through the write updates to validate for a push descriptor set, ignoring dstSet
bool CoreChecks::ValidatePushDescriptorsUpdate(const DescriptorSet &push_set, uint32_t descriptorWriteCount,
                                               const VkWriteDescriptorSet *pDescriptorWrites, const Location &loc) const {
    bool skip = false;
    for (uint32_t i = 0; i < descriptorWriteCount; i++) {
        skip |= ValidateWriteUpdate(push_set, pDescriptorWrites[i], loc.dot(Field::pDescriptorWrites, i), true);
    }
    return skip;
}

// For the given buffer, verify that its creation parameters are appropriate for the given type
//  If there's an error, update the error_msg string with details and return false, else return true
bool CoreChecks::ValidateBufferUsage(const vvl::Buffer &buffer_state, VkDescriptorType type, const Location &buffer_loc) const {
    bool skip = false;
    switch (type) {
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            if (!(buffer_state.usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT)) {
                skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-08765", buffer_state.Handle(), buffer_loc,
                                 "was created with %s, but descriptorType is VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER.",
                                 string_VkBufferUsageFlags2KHR(buffer_state.usage).c_str());
            }
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            if (!(buffer_state.usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT)) {
                skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-08766", buffer_state.Handle(), buffer_loc,
                                 "was created with %s, but descriptorType is VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER.",
                                 string_VkBufferUsageFlags2KHR(buffer_state.usage).c_str());
            }
            break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            if (!(buffer_state.usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)) {
                skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-00330", buffer_state.Handle(), buffer_loc,
                                 "was created with %s, but descriptorType is %s.",
                                 string_VkBufferUsageFlags2KHR(buffer_state.usage).c_str(), string_VkDescriptorType(type));
            }
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            if (!(buffer_state.usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)) {
                skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-00331", buffer_state.Handle(), buffer_loc,
                                 "was created with %s, but descriptorType is %s.",
                                 string_VkBufferUsageFlags2KHR(buffer_state.usage).c_str(), string_VkDescriptorType(type));
            }
            break;
        default:
            break;
    }
    return skip;
}

bool CoreChecks::ValidateBufferUpdate(const VkDescriptorBufferInfo &buffer_info, VkDescriptorType type,
                                      const Location &buffer_info_loc) const {
    bool skip = false;
    const auto &buffer_state = *Get<vvl::Buffer>(buffer_info.buffer);
    skip |= ValidateMemoryIsBoundToBuffer(device, buffer_state, buffer_info_loc.dot(Field::buffer),
                                          "VUID-VkWriteDescriptorSet-descriptorType-00329");
    skip |= ValidateBufferUsage(buffer_state, type, buffer_info_loc.dot(Field::buffer));

    if (buffer_info.offset >= buffer_state.createInfo.size) {
        skip |= LogError("VUID-VkDescriptorBufferInfo-offset-00340", buffer_info.buffer, buffer_info_loc.dot(Field::offset),
                         "(%" PRIu64 ") is greater than or equal to buffer size (%" PRIu64 ").", buffer_info.offset,
                         buffer_state.createInfo.size);
    }
    if (buffer_info.range != VK_WHOLE_SIZE) {
        if (buffer_info.range == 0) {
            skip |= LogError("VUID-VkDescriptorBufferInfo-range-00341", buffer_info.buffer, buffer_info_loc.dot(Field::range),
                             "is not VK_WHOLE_SIZE and is zero.");
        }
        if (buffer_info.range > (buffer_state.createInfo.size - buffer_info.offset)) {
            skip |= LogError("VUID-VkDescriptorBufferInfo-range-00342", buffer_info.buffer, buffer_info_loc.dot(Field::range),
                             "(%" PRIu64 ") is larger than buffer size (%" PRIu64 ") + offset (%" PRIu64 ").", buffer_info.range,
                             buffer_state.createInfo.size, buffer_info.offset);
        }
    }

    if (VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER == type || VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC == type) {
        const uint32_t max_ub_range = phys_dev_props.limits.maxUniformBufferRange;
        if (buffer_info.range != VK_WHOLE_SIZE && buffer_info.range > max_ub_range) {
            skip |=
                LogError("VUID-VkWriteDescriptorSet-descriptorType-00332", buffer_info.buffer, buffer_info_loc.dot(Field::range),
                         "(%" PRIu64 ") is greater than maxUniformBufferRange (%" PRIu32 ") for descriptorType %s.",
                         buffer_info.range, max_ub_range, string_VkDescriptorType(type));
        } else if (buffer_info.range == VK_WHOLE_SIZE && (buffer_state.createInfo.size - buffer_info.offset) > max_ub_range) {
            skip |=
                LogError("VUID-VkWriteDescriptorSet-descriptorType-00332", buffer_info.buffer, buffer_info_loc.dot(Field::range),
                         "is VK_WHOLE_SIZE, but the effective range [size (%" PRIu64 ") - offset (%" PRIu64 ") = %" PRIu64
                         "] is greater than maxUniformBufferRange (%" PRIu32 ") for descriptorType %s.",
                         buffer_state.createInfo.size, buffer_info.offset, buffer_state.createInfo.size - buffer_info.offset,
                         max_ub_range, string_VkDescriptorType(type));
        }
    } else if (VK_DESCRIPTOR_TYPE_STORAGE_BUFFER == type || VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC == type) {
        const uint32_t max_sb_range = phys_dev_props.limits.maxStorageBufferRange;
        if (buffer_info.range != VK_WHOLE_SIZE && buffer_info.range > max_sb_range) {
            skip |=
                LogError("VUID-VkWriteDescriptorSet-descriptorType-00333", buffer_info.buffer, buffer_info_loc.dot(Field::range),
                         "(%" PRIu64 ") is greater than maxStorageBufferRange (%" PRIu32 ") for descriptorType %s.",
                         buffer_info.range, max_sb_range, string_VkDescriptorType(type));
        } else if (buffer_info.range == VK_WHOLE_SIZE && (buffer_state.createInfo.size - buffer_info.offset) > max_sb_range) {
            skip |=
                LogError("VUID-VkWriteDescriptorSet-descriptorType-00333", buffer_info.buffer, buffer_info_loc.dot(Field::range),
                         "is VK_WHOLE_SIZE, but the effective range [size (%" PRIu64 ") - offset (%" PRIu64 ") = %" PRIu64
                         "] is greater than maxStorageBufferRange (%" PRIu32 ") for descriptorType %s.",
                         buffer_state.createInfo.size, buffer_info.offset, buffer_state.createInfo.size - buffer_info.offset,
                         max_sb_range, string_VkDescriptorType(type));
        }
    }
    return skip;
}

// Verify that the contents of the update are ok, but don't perform actual update
bool CoreChecks::VerifyCopyUpdateContents(const VkCopyDescriptorSet &update, const DescriptorSet &src_set,
                                          VkDescriptorType src_type, uint32_t src_index, const DescriptorSet &dst_set,
                                          VkDescriptorType dst_type, uint32_t dst_index, const Location &copy_loc) const {
    // Note : Repurposing some Write update error codes here as specific details aren't called out for copy updates like they are
    // for write updates
    using DescriptorClass = vvl::DescriptorClass;
    using BufferDescriptor = vvl::BufferDescriptor;
    using ImageDescriptor = vvl::ImageDescriptor;
    using ImageSamplerDescriptor = vvl::ImageSamplerDescriptor;
    using SamplerDescriptor = vvl::SamplerDescriptor;
    using TexelDescriptor = vvl::TexelDescriptor;
    bool skip = false;

    auto device_data = this;

    if (dst_type == VK_DESCRIPTOR_TYPE_SAMPLER) {
        auto dst_iter = dst_set.FindDescriptor(update.dstBinding, update.dstArrayElement);
        for (uint32_t di = 0; di < update.descriptorCount; ++di, ++dst_iter) {
            if (dst_iter.updated() && dst_iter->IsImmutableSampler()) {
                const LogObjectList objlist(update.srcSet, update.dstSet);
                skip |= LogError("VUID-VkCopyDescriptorSet-dstBinding-02753", objlist, copy_loc,
                                 "Attempted copy update to an immutable sampler descriptor.");
            }
        }
    }

    switch (src_set.GetBinding(update.srcBinding)->descriptor_class) {
        case DescriptorClass::PlainSampler: {
            auto src_iter = src_set.FindDescriptor(update.srcBinding, update.srcArrayElement);
            for (uint32_t di = 0; di < update.descriptorCount; ++di) {
                if (src_iter.updated()) {
                    if (!src_iter->IsImmutableSampler()) {
                        auto update_sampler = static_cast<const SamplerDescriptor &>(*src_iter).GetSampler();
                        if (!ValidateSampler(update_sampler)) {
                            const LogObjectList objlist(update.srcSet, update_sampler);
                            skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-00325", objlist, copy_loc,
                                             "Attempted copy update to sampler descriptor with invalid sampler (%s).",
                                             FormatHandle(update_sampler).c_str());
                        }
                    }
                }
            }
            break;
        }
        case DescriptorClass::ImageSampler: {
            auto src_iter = src_set.FindDescriptor(update.srcBinding, update.srcArrayElement);
            for (uint32_t di = 0; di < update.descriptorCount; ++di, ++src_iter) {
                if (!src_iter.updated()) continue;
                auto img_samp_desc = static_cast<const ImageSamplerDescriptor &>(*src_iter);
                // First validate sampler
                if (!img_samp_desc.IsImmutableSampler()) {
                    auto update_sampler = img_samp_desc.GetSampler();
                    if (!ValidateSampler(update_sampler)) {
                        const LogObjectList objlist(update.srcSet);
                        skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-00325", objlist, copy_loc,
                                         "Attempted copy update to sampler descriptor with invalid sampler (%s).",
                                         FormatHandle(update_sampler).c_str());
                    }
                }
                // Validate image
                auto image_view = img_samp_desc.GetImageView();
                auto image_layout = img_samp_desc.GetImageLayout();
                if (image_view) {
                    skip |= ValidateImageUpdate(image_view, image_layout, src_type, copy_loc);
                }
            }
            break;
        }
        case DescriptorClass::Image: {
            auto src_iter = src_set.FindDescriptor(update.srcBinding, update.srcArrayElement);
            for (uint32_t di = 0; di < update.descriptorCount; ++di, ++src_iter) {
                if (!src_iter.updated()) continue;
                auto img_desc = static_cast<const ImageDescriptor &>(*src_iter);
                auto image_view = img_desc.GetImageView();
                auto image_layout = img_desc.GetImageLayout();
                if (image_view) {
                    skip |= ValidateImageUpdate(image_view, image_layout, src_type, copy_loc);
                }
            }
            break;
        }
        case DescriptorClass::TexelBuffer: {
            auto src_iter = src_set.FindDescriptor(update.srcBinding, update.srcArrayElement);
            for (uint32_t di = 0; di < update.descriptorCount; ++di, ++src_iter) {
                if (!src_iter.updated()) continue;
                auto buffer_view = static_cast<const TexelDescriptor &>(*src_iter).GetBufferView();
                if (buffer_view) {
                    auto bv_state = device_data->Get<vvl::BufferView>(buffer_view);
                    if (!bv_state) {
                        const LogObjectList objlist(update.srcSet);
                        skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-02994", objlist, copy_loc,
                                         "Attempted copy update to texel buffer descriptor with invalid buffer view (%s).",
                                         FormatHandle(buffer_view).c_str());
                    } else {
                        auto buffer_state = Get<vvl::Buffer>(bv_state->create_info.buffer);
                        if (buffer_state) {
                            skip |= ValidateBufferUsage(*buffer_state, src_type, copy_loc);
                        }
                    }
                }
            }
            break;
        }
        case DescriptorClass::GeneralBuffer: {
            auto src_iter = src_set.FindDescriptor(update.srcBinding, update.srcArrayElement);
            for (uint32_t di = 0; di < update.descriptorCount; ++di, ++src_iter) {
                if (!src_iter.updated()) continue;
                auto buffer_state = static_cast<const BufferDescriptor &>(*src_iter).GetBufferState();
                if (buffer_state) {
                    skip |= ValidateBufferUsage(*buffer_state, src_type, copy_loc);
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
    return skip;
}

// Validate the state for a given write update but don't actually perform the update
//  If an error would occur for this update, return false and fill in details in error_msg string
bool CoreChecks::ValidateWriteUpdate(const DescriptorSet &dst_set, const VkWriteDescriptorSet &update, const Location &write_loc,
                                     bool push) const {
    bool skip = false;
    const auto *dst_layout = dst_set.GetLayout().get();
    // Even if PushDescriptor, the error logging will remove the null Set handle
    const LogObjectList objlist(update.dstSet, dst_layout->Handle());

    // Verify dst layout still valid (ObjectLifetimes only checks if null, we check if valid dstSet here)
    if (dst_layout->Destroyed()) {
        return LogError("VUID-VkWriteDescriptorSet-dstSet-00320", objlist, write_loc.dot(Field::dstSet), "(%s) has been destroyed.",
                        dst_set.StringifySetAndLayout().c_str());
    }

    const Location dst_binding_loc = write_loc.dot(Field::dstBinding);
    if (update.dstBinding > dst_layout->GetMaxBinding()) {
        return LogError("VUID-VkWriteDescriptorSet-dstBinding-00315", objlist, dst_binding_loc, "(%" PRIu32 ") is larger than %s binding count (%" PRIu32 ").", update.dstBinding, FormatHandle(dst_layout->Handle()).c_str(), dst_layout->GetBindingCount());
    }

    const auto &dest = *dst_set.GetBinding(update.dstBinding);
    if (0 == dest.count) {
        skip |= LogError("VUID-VkWriteDescriptorSet-dstBinding-00316", objlist, dst_binding_loc,
                         "(%" PRIu32 ") has VkDescriptorSetLayoutBinding::descriptorCount of zero in %s.", update.dstBinding,
                         FormatHandle(dst_layout->Handle()).c_str());
    }

    const auto *used_handle = dst_set.InUse();
    if (used_handle && !(dest.IsBindless())) {
        skip |= LogError("VUID-vkUpdateDescriptorSets-None-03047", objlist, dst_binding_loc,
                         "(%" PRIu32 ") was created with %s, but %s is in use by %s.", update.dstBinding,
                         string_VkDescriptorBindingFlags(dest.binding_flags).c_str(), FormatHandle(update.dstSet).c_str(),
                         FormatHandle(*used_handle).c_str());
    }
    // We know that binding is valid, verify update and do update on each descriptor
    if ((dest.type != VK_DESCRIPTOR_TYPE_MUTABLE_EXT) && (dest.type != update.descriptorType)) {
        skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-00319", objlist, write_loc.dot(Field::descriptorType),
                         "(%s) is different from pBinding[%" PRIu32 "].descriptorType (%s) of %s.",
                         string_VkDescriptorType(update.descriptorType), update.dstBinding, string_VkDescriptorType(dest.type),
                         dst_set.StringifySetAndLayout().c_str());
    }
    if (dest.type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT) {
        if ((update.dstArrayElement % 4) != 0) {
            skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-02219", objlist, dst_binding_loc,
                             "(%" PRIu32 ") is of type VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT, but dstArrayElement is %" PRIu32
                             ".",
                             update.dstBinding, update.dstArrayElement);
        }
        if ((update.descriptorCount % 4) != 0) {
            skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-02220", objlist, dst_binding_loc,
                             "(%" PRIu32 ") is of type VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT, but descriptorCount is %" PRIu32
                             ".",
                             update.dstBinding, update.descriptorCount);
        }
        const auto *write_inline_info = vku::FindStructInPNextChain<VkWriteDescriptorSetInlineUniformBlock>(update.pNext);
        if (!write_inline_info) {
            skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-02221", objlist, dst_binding_loc,
                             "(%" PRIu32
                             ") is of type VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT, but there is no "
                             "VkWriteDescriptorSetInlineUniformBlock in the pNext chain.",
                             update.dstBinding);
        } else if (write_inline_info->dataSize != update.descriptorCount) {
            skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-02221", objlist,
                             write_loc.pNext(Struct::VkWriteDescriptorSetInlineUniformBlock, Field::dataSize),
                             "(%" PRIu32 ") is different then descriptorCount (%" PRIu32 "), but dstBinding (%" PRIu32
                             ") is of type VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT.",
                             write_inline_info->dataSize, update.descriptorCount, update.dstBinding);
        } else if ((write_inline_info->dataSize % 4) != 0) {
            skip |= LogError("VUID-VkWriteDescriptorSetInlineUniformBlock-dataSize-02222", objlist,
                             write_loc.pNext(Struct::VkWriteDescriptorSetInlineUniformBlock, Field::dataSize), "is %" PRIu32 ".",
                             write_inline_info->dataSize);
        }
    }

    if (update.descriptorCount > 0) {
        // Save first binding information and error if something different is found
        auto current_iter = dst_set.FindBinding(update.dstBinding);
        VkShaderStageFlags stage_flags = (*current_iter)->stage_flags;
        VkDescriptorType descriptor_type = (*current_iter)->type;
        const bool immutable_samplers = (*current_iter)->has_immutable_samplers;
        uint32_t dst_array_element = update.dstArrayElement;

        for (uint32_t i = 0; i < update.descriptorCount;) {
            if (current_iter == dst_set.end()) {
                break;  // prevents setting error here if bindings don't exist
            }
            auto current_binding = current_iter->get();

            // All consecutive bindings updated, except those with a descriptorCount of zero, must have identical descType and
            // stageFlags
            if (current_binding->count > 0) {
                // Check for consistent stageFlags and descriptorType
                if ((current_binding->stage_flags != stage_flags) || (current_binding->type != descriptor_type)) {
                    skip |= LogError(
                        "VUID-VkWriteDescriptorSet-descriptorCount-00317", objlist, write_loc,
                        "binding #%" PRIu32 " (started on dstBinding [%" PRIu32 "] plus %" PRIu32
                        " descriptors offset) has stageFlags of %s and descriptorType of %s, but previous binding was %s and %s.",
                        current_binding->binding, update.dstBinding, i,
                        string_VkShaderStageFlags(current_binding->stage_flags).c_str(),
                        string_VkDescriptorType(current_binding->type), string_VkShaderStageFlags(stage_flags).c_str(),
                        string_VkDescriptorType(descriptor_type));
                }
                // Check if all immutableSamplers or not
                if (current_binding->has_immutable_samplers != immutable_samplers) {
                    skip |= LogError("VUID-VkWriteDescriptorSet-descriptorCount-00318", objlist, write_loc,
                                     "binding #%" PRIu32 " (started on dstBinding [%" PRIu32 "] plus %" PRIu32
                                     " descriptors offset) %s Immutable Samplers, which is different from the previous binding.",
                                     current_binding->binding, update.dstBinding, i,
                                     current_binding->has_immutable_samplers ? "has" : "doesn't have");
                }
            }

            // Skip the remaining descriptors for this binding, and move to the next binding
            i += (current_binding->count - dst_array_element);
            dst_array_element = 0;
            ++current_iter;
        }
    }

    if (skip) {
        return skip;  // consistency will likley be wrong if already bad
    }

    if (dest.IsVariableCount()) {
        if ((update.dstArrayElement + update.descriptorCount) > dst_set.GetVariableDescriptorCount()) {
            skip |= LogError("VUID-VkWriteDescriptorSet-dstArrayElement-00321", objlist, write_loc.dot(Field::dstArrayElement),
                             "(%" PRIu32 ") + descriptorCount (%" PRIu32 ") is larger than (%" PRIu32 ") for dstBinding (%" PRIu32
                             ") in %s.",
                             update.dstArrayElement, update.descriptorCount, dst_set.GetVariableDescriptorCount(),
                             update.dstBinding, dst_set.StringifySetAndLayout().c_str());
        }
    } else {
        skip |= VerifyUpdateConsistency(dst_set, update.dstBinding, update.dstArrayElement, update.descriptorCount,
                                        "VUID-VkWriteDescriptorSet-dstArrayElement-00321", write_loc.dot(Field::dstBinding));
    }

    auto start_idx = dst_set.GetGlobalIndexRangeFromBinding(update.dstBinding).start + update.dstArrayElement;
    // Update is within bounds and consistent so last step is to validate update contents
    skip |= VerifyWriteUpdateContents(dst_set, update, start_idx, write_loc, push);

    if (dest.type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
        // Check if the new descriptor descriptor type is in the list of allowed mutable types for this binding
        if (!dst_set.Layout().IsTypeMutable(update.descriptorType, update.dstBinding)) {
            skip |= LogError("VUID-VkWriteDescriptorSet-dstSet-04611", objlist, dst_binding_loc,
                             "(%" PRIu32
                             ") is of type VK_DESCRIPTOR_TYPE_MUTABLE_EXT, but the new descriptorType (%s) was not in "
                             "VkMutableDescriptorTypeListEXT::pDescriptorTypes.",
                             update.dstBinding, string_VkDescriptorType(update.descriptorType));
        }
    }

    return skip;
}

// Verify that the contents of the update are ok, but don't perform actual update
bool CoreChecks::VerifyWriteUpdateContents(const DescriptorSet &dst_set, const VkWriteDescriptorSet &update, const uint32_t index,
                                           const Location &write_loc, bool push) const {
    using ImageSamplerDescriptor = vvl::ImageSamplerDescriptor;
    bool skip = false;

    switch (update.descriptorType) {
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
            auto iter = dst_set.FindDescriptor(update.dstBinding, update.dstArrayElement);
            for (uint32_t di = 0; di < update.descriptorCount && !iter.AtEnd(); ++di, ++iter) {
                // Validate image
                const VkImageView image_view = update.pImageInfo[di].imageView;
                if (image_view == VK_NULL_HANDLE) {
                    continue;
                }
                auto image_layout = update.pImageInfo[di].imageLayout;
                auto sampler = update.pImageInfo[di].sampler;
                auto iv_state = Get<vvl::ImageView>(image_view);
                const ImageSamplerDescriptor &desc = (const ImageSamplerDescriptor &)*iter;

                const auto *image_state = iv_state->image_state.get();
                skip |= ValidateImageUpdate(image_view, image_layout, update.descriptorType, write_loc.dot(Field::pImageInfo, di));

                if (IsExtEnabled(device_extensions.vk_khr_sampler_ycbcr_conversion)) {
                    if (desc.IsImmutableSampler()) {
                        auto sampler_state = Get<vvl::Sampler>(desc.GetSampler());
                        if (iv_state && sampler_state) {
                            if (iv_state->samplerConversion != sampler_state->samplerConversion) {
                                const LogObjectList objlist(update.dstSet, desc.GetSampler(), iv_state->image_view());
                                skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-01948", objlist, write_loc,
                                                 "Attempted write update to combined image sampler and image view and sampler "
                                                 "YCbCr conversions are not identical.");
                            }
                        }
                    } else if (iv_state && (iv_state->samplerConversion != VK_NULL_HANDLE)) {
                        const LogObjectList objlist(update.dstSet, iv_state->image_view());
                        skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-02738", objlist, write_loc.dot(Field::dstSet),
                                         "is bound to image view that includes a YCbCr conversion, it must have been allocated "
                                         "with a layout that includes an immutable sampler.");
                    }
                }
                // If there is an immutable sampler then |sampler| isn't used, so the following VU does not apply.
                if (sampler && !desc.IsImmutableSampler() && vkuFormatIsMultiplane(image_state->createInfo.format)) {
                    // multiplane formats must be created with mutable format bit
                    const VkFormat image_format = image_state->createInfo.format;
                    if (0 == (image_state->createInfo.flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT)) {
                        const LogObjectList objlist(update.dstSet, image_state->image());
                        skip |= LogError("VUID-VkDescriptorImageInfo-sampler-01564", objlist, write_loc,
                                         "combined image sampler is a multi-planar format %s and was created with %s.",
                                         string_VkFormat(image_format),
                                         string_VkImageCreateFlags(image_state->createInfo.flags).c_str());
                    }
                    const VkImageAspectFlags image_aspect = iv_state->create_info.subresourceRange.aspectMask;
                    if (!IsValidPlaneAspect(image_format, image_aspect)) {
                        const LogObjectList objlist(update.dstSet, image_state->image(), iv_state->image_view());
                        skip |= LogError("VUID-VkDescriptorImageInfo-sampler-01564", objlist, write_loc,
                                         "combined image sampler is a multi-planar format %s and imageView aspectMask is %s.",
                                         string_VkFormat(image_format), string_VkImageAspectFlags(image_aspect).c_str());
                    }
                }

                // Verify portability
                auto sampler_state = Get<vvl::Sampler>(sampler);
                if (sampler_state) {
                    if (IsExtEnabled(device_extensions.vk_khr_portability_subset)) {
                        if ((VK_FALSE == enabled_features.mutableComparisonSamplers) &&
                            (VK_FALSE != sampler_state->createInfo.compareEnable)) {
                            skip |= LogError("VUID-VkDescriptorImageInfo-mutableComparisonSamplers-04450", device, write_loc,
                                             "(portability error): sampler comparison not available.");
                        }
                    }
                }
            }
        }
            [[fallthrough]];
        case VK_DESCRIPTOR_TYPE_SAMPLER: {
            auto iter = dst_set.FindDescriptor(update.dstBinding, update.dstArrayElement);
            for (uint32_t di = 0; di < update.descriptorCount && !iter.AtEnd(); ++di, ++iter) {
                const auto &desc = *iter;
                if (!desc.IsImmutableSampler()) {
                    if (!ValidateSampler(update.pImageInfo[di].sampler)) {
                        const LogObjectList objlist(update.dstSet, update.pImageInfo[di].sampler);
                        skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-00325", objlist, write_loc,
                                         "Attempted write update to sampler descriptor with invalid sample (%s).",
                                         FormatHandle(update.pImageInfo[di].sampler).c_str());
                    }
                } else if (update.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER && !push) {
                    skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-02752", update.dstSet, write_loc,
                                     "Attempted write update to an immutable sampler descriptor.");
                }
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:
        case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM: {
            for (uint32_t di = 0; di < update.descriptorCount; ++di) {
                const VkImageView image_view = update.pImageInfo[di].imageView;
                auto image_layout = update.pImageInfo[di].imageLayout;
                if (image_view) {
                    skip |=
                        ValidateImageUpdate(image_view, image_layout, update.descriptorType, write_loc.dot(Field::pImageInfo, di));
                }
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: {
            for (uint32_t di = 0; di < update.descriptorCount; ++di) {
                const VkBufferView buffer_view = update.pTexelBufferView[di];
                if (buffer_view == VK_NULL_HANDLE) {
                    continue;
                }
                auto bv_state = Get<vvl::BufferView>(buffer_view);
                if (!bv_state) {
                    skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-02994", device, write_loc,
                                     "Attempted write update to texel buffer descriptor with invalid buffer view (%s).",
                                     FormatHandle(buffer_view).c_str());
                    break;
                }
                auto buffer = bv_state->create_info.buffer;
                auto buffer_state = Get<vvl::Buffer>(buffer);
                // Verify that buffer underlying the view hasn't been destroyed prematurely
                if (!buffer_state) {
                    skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-02994", device, write_loc,
                                     "Attempted write update to texel buffer descriptor with invalid buffer (%s).",
                                     FormatHandle(buffer).c_str());
                    break;
                }
                skip |= ValidateBufferUsage(*buffer_state, update.descriptorType,
                                            write_loc.dot(Field::pBufferInfo, di).dot(Field::buffer));
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
            for (uint32_t di = 0; di < update.descriptorCount; ++di) {
                if (update.pBufferInfo[di].buffer) {
                    skip |=
                        ValidateBufferUpdate(update.pBufferInfo[di], update.descriptorType, write_loc.dot(Field::pBufferInfo, di));
                }
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT:
            break;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV: {
            const auto *acc_info = vku::FindStructInPNextChain<VkWriteDescriptorSetAccelerationStructureNV>(update.pNext);
            for (uint32_t di = 0; di < update.descriptorCount; ++di) {
                VkAccelerationStructureNV as = acc_info->pAccelerationStructures[di];
                auto as_state = Get<vvl::AccelerationStructureNV>(as);
                // nullDescriptor feature allows this to be VK_NULL_HANDLE
                if (as_state) {
                    skip |= VerifyBoundMemoryIsValid(
                        as_state->MemState(), LogObjectList(as), as_state->Handle(),
                        write_loc.pNext(Struct::VkWriteDescriptorSetAccelerationStructureNV, Field::pAccelerationStructures, di),
                        kVUIDUndefined);
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
    return skip;
}

bool CoreChecks::ValidateCmdSetDescriptorBufferOffsets(const vvl::CommandBuffer &cb_state, VkPipelineLayout layout,
                                                       uint32_t firstSet, uint32_t setCount, const uint32_t *pBufferIndices,
                                                       const VkDeviceSize *pOffsets, const Location &loc) const {
    auto pipeline_layout = Get<vvl::PipelineLayout>(layout);
    if (!pipeline_layout) {
        return false;  // dynamicPipelineLayout
    }

    bool skip = false;
    const bool is_2 = loc.function != Func::vkCmdSetDescriptorBufferOffsetsEXT;

    if (!enabled_features.descriptorBuffer) {
        const char *vuid = is_2 ? "VUID-vkCmdSetDescriptorBufferOffsets2EXT-descriptorBuffer-09470"
                                : "VUID-vkCmdSetDescriptorBufferOffsetsEXT-None-08060";
        skip |= LogError(vuid, cb_state.commandBuffer(), loc, "descriptorBuffer feature was not enabled.");
    }

    if ((firstSet + setCount) > pipeline_layout->set_layouts.size()) {
        const char *vuid = is_2 ? "VUID-VkSetDescriptorBufferOffsetsInfoEXT-firstSet-08066"
                                : "VUID-vkCmdSetDescriptorBufferOffsetsEXT-firstSet-08066";
        skip |= LogError(vuid, cb_state.commandBuffer(), loc,
                         "The sum of firstSet (%" PRIu32 ") and setCount (%" PRIu32
                         ") is greater than VkPipelineLayoutCreateInfo::setLayoutCount (%" PRIuLEAST64 ") when layout was created.",
                         firstSet, setCount, (uint64_t)pipeline_layout->set_layouts.size());

        // Clamp so that we don't attempt to access invalid stuff
        setCount = std::min(setCount, static_cast<uint32_t>(pipeline_layout->set_layouts.size()));
    }

    for (uint32_t i = 0; i < setCount; i++) {
        const uint32_t bufferIndex = pBufferIndices[i];
        const VkDeviceAddress offset = pOffsets[i];
        bool valid_buffer = false;
        bool valid_binding = false;

        const auto set_layout = pipeline_layout->set_layouts[firstSet + i];
        if ((set_layout->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) == 0) {
            const LogObjectList objlist(cb_state.commandBuffer(), set_layout->Handle(), pipeline_layout->layout());
            const char *vuid = is_2 ? "VUID-VkSetDescriptorBufferOffsetsInfoEXT-firstSet-09006"
                                    : "VUID-vkCmdSetDescriptorBufferOffsetsEXT-firstSet-09006";
            skip |= LogError(vuid, objlist, loc,
                             "Descriptor set layout (%s) for set %" PRIu32
                             " was created without VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT flag set.",
                             FormatHandle(set_layout->Handle()).c_str(), firstSet + i);
        }

        if (bufferIndex < cb_state.descriptor_buffer_binding_info.size()) {
            const VkDeviceAddress start = cb_state.descriptor_buffer_binding_info[bufferIndex].address;
            const auto buffer_states = GetBuffersByAddress(start);

            if (!buffer_states.empty()) {
                const auto buffer_state_starts = GetBuffersByAddress(start + offset);

                if (!buffer_state_starts.empty()) {
                    const auto bindings = set_layout->GetBindings();
                    const auto pSetLayoutSize = set_layout->GetLayoutSizeInBytes();
                    VkDeviceSize setLayoutSize = 0;

                    if (pSetLayoutSize == nullptr) {
                        const auto pool = cb_state.command_pool;
                        DispatchGetDescriptorSetLayoutSizeEXT(pool->dev_data->device, set_layout->VkHandle(),
                                                              &setLayoutSize);
                    } else {
                        setLayoutSize = *pSetLayoutSize;
                    }

                    if (setLayoutSize > 0) {
                        // It looks like enough to check last binding in set
                        for (uint32_t j = 0; j < set_layout->GetBindingCount(); j++) {
                            const VkDescriptorBindingFlags flags = set_layout->GetDescriptorBindingFlagsFromIndex(j);
                            const bool vdc = (flags & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT) != 0;

                            if (vdc) {
                                // If a binding is VARIABLE_DESCRIPTOR_COUNT, the effective setLayoutSize we
                                // must validate is just the offset of the last binding.
                                const auto pool = cb_state.command_pool;
                                uint32_t binding = set_layout->GetDescriptorSetLayoutBindingPtrFromIndex(j)->binding;
                                DispatchGetDescriptorSetLayoutBindingOffsetEXT(
                                    pool->dev_data->device, set_layout->VkHandle(), binding, &setLayoutSize);

                                // If the descriptor set only consists of VARIABLE_DESCRIPTOR_COUNT bindings, the
                                // offset may be 0. In this case, treat the descriptor set layout as size 1,
                                // so we validate that the offset is sensible.
                                if (set_layout->GetBindingCount() == 1) {
                                    setLayoutSize = 1;
                                }

                                // There can only be one binding with VARIABLE_COUNT.
                                break;
                            }
                        }
                    }

                    if (setLayoutSize > 0) {
                        const auto buffer_state_ends = GetBuffersByAddress(start + offset + setLayoutSize - 1);
                        if (!buffer_state_ends.empty()) {
                            valid_binding = true;
                        }
                    }
                }

                valid_buffer = true;
            }

            if (!valid_binding) {
                const char *vuid = is_2 ? "VUID-VkSetDescriptorBufferOffsetsInfoEXT-pOffsets-08063"
                                        : "VUID-vkCmdSetDescriptorBufferOffsetsEXT-pOffsets-08063";
                skip |= LogError(vuid, cb_state.commandBuffer(), loc.dot(Field::pOffsets, i),
                                 "%" PRIuLEAST64
                                 " must be small enough such that any descriptor binding"
                                 " referenced by layout without the VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT"
                                 " flag computes a valid address inside the underlying VkBuffer",
                                 pOffsets[i]);
            }
        }

        if (!valid_buffer) {
            const char *vuid = is_2 ? "VUID-VkSetDescriptorBufferOffsetsInfoEXT-pBufferIndices-08065"
                                    : "VUID-vkCmdSetDescriptorBufferOffsetsEXT-pBufferIndices-08065";
            skip |= LogError(vuid, cb_state.commandBuffer(), loc.dot(Field::pBufferIndices, i),
                             "(%" PRIu32
                             ") Each element of pBufferIndices must reference a valid descriptor buffer binding "
                             "set by a previous call to vkCmdBindDescriptorBuffersEXT in commandBuffer",
                             pBufferIndices[i]);
        }

        if (pBufferIndices[i] >= phys_dev_ext_props.descriptor_buffer_props.maxDescriptorBufferBindings) {
            const char *vuid = is_2 ? "VUID-VkSetDescriptorBufferOffsetsInfoEXT-pBufferIndices-08064"
                                    : "VUID-vkCmdSetDescriptorBufferOffsetsEXT-pBufferIndices-08064";
            skip |= LogError(vuid, cb_state.commandBuffer(), loc.dot(Field::pBufferIndices, i),
                             "(%" PRIu32
                             ") "
                             "is greater than maxDescriptorBufferBindings (%" PRIu32 ") ",
                             pBufferIndices[i], phys_dev_ext_props.descriptor_buffer_props.maxDescriptorBufferBindings);
        }

        if (SafeModulo(pOffsets[i], phys_dev_ext_props.descriptor_buffer_props.descriptorBufferOffsetAlignment) != 0) {
            const char *vuid = is_2 ? "VUID-VkSetDescriptorBufferOffsetsInfoEXT-pOffsets-08061"
                                    : "VUID-vkCmdSetDescriptorBufferOffsetsEXT-pOffsets-08061";
            skip |= LogError(vuid, cb_state.commandBuffer(), loc.dot(Field::pOffsets, i),
                             "(%" PRIuLEAST64
                             ") is not aligned to descriptorBufferOffsetAlignment"
                             " (%" PRIuLEAST64 ")",
                             pOffsets[i], phys_dev_ext_props.descriptor_buffer_props.descriptorBufferOffsetAlignment);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer,
                                                                 VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                                                 uint32_t firstSet, uint32_t setCount,
                                                                 const uint32_t *pBufferIndices, const VkDeviceSize *pOffsets,
                                                                 const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);

    bool skip = false;
    skip |= ValidatePipelineBindPoint(cb_state.get(), pipelineBindPoint, error_obj.location);
    skip |=
        ValidateCmdSetDescriptorBufferOffsets(*cb_state, layout, firstSet, setCount, pBufferIndices, pOffsets, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetDescriptorBufferOffsets2EXT(
    VkCommandBuffer commandBuffer, const VkSetDescriptorBufferOffsetsInfoEXT *pSetDescriptorBufferOffsetsInfo,
    const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    bool skip = false;

    skip |= ValidateCmdSetDescriptorBufferOffsets(
        *cb_state, pSetDescriptorBufferOffsetsInfo->layout, pSetDescriptorBufferOffsetsInfo->firstSet,
        pSetDescriptorBufferOffsetsInfo->setCount, pSetDescriptorBufferOffsetsInfo->pBufferIndices,
        pSetDescriptorBufferOffsetsInfo->pOffsets, error_obj.location);

    if (!enabled_features.dynamicPipelineLayout && pSetDescriptorBufferOffsetsInfo->layout == VK_NULL_HANDLE) {
        skip |= LogError("VUID-VkSetDescriptorBufferOffsetsInfoEXT-None-09495", device,
                         error_obj.location.dot(Field::pSetDescriptorBufferOffsetsInfo).dot(Field::layout), "is not valid.");
    }

    if (IsStageInPipelineBindPoint(pSetDescriptorBufferOffsetsInfo->stageFlags, VK_PIPELINE_BIND_POINT_GRAPHICS)) {
        skip |= ValidatePipelineBindPoint(cb_state.get(), VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    }
    if (IsStageInPipelineBindPoint(pSetDescriptorBufferOffsetsInfo->stageFlags, VK_PIPELINE_BIND_POINT_COMPUTE)) {
        skip |= ValidatePipelineBindPoint(cb_state.get(), VK_PIPELINE_BIND_POINT_COMPUTE, error_obj.location);
    }
    if (IsStageInPipelineBindPoint(pSetDescriptorBufferOffsetsInfo->stageFlags, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR)) {
        skip |= ValidatePipelineBindPoint(cb_state.get(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, error_obj.location);
    }

    return skip;
}

bool CoreChecks::ValidateCmdBindDescriptorBufferEmbeddedSamplers(const vvl::CommandBuffer &cb_state, VkPipelineLayout layout,
                                                                 uint32_t set, const Location &loc) const {
    bool skip = false;
    const bool is_2 = loc.function != Func::vkCmdBindDescriptorBufferEmbeddedSamplersEXT;

    if (!enabled_features.descriptorBuffer) {
        const char *vuid = is_2 ? "VUID-vkCmdBindDescriptorBufferEmbeddedSamplers2EXT-descriptorBuffer-09472"
                                : "VUID-vkCmdBindDescriptorBufferEmbeddedSamplersEXT-None-08068";
        skip |= LogError(vuid, cb_state.commandBuffer(), loc, "descriptorBuffer feature was not enabled.");
    }

    auto pipeline_layout = Get<vvl::PipelineLayout>(layout);
    if (!pipeline_layout) {
        return skip;  // dynamicPipelineLayout
    }

    if (set >= pipeline_layout->set_layouts.size()) {
        const char *vuid = is_2 ? "VUID-VkBindDescriptorBufferEmbeddedSamplersInfoEXT-set-08071"
                                : "VUID-vkCmdBindDescriptorBufferEmbeddedSamplersEXT-set-08071";
        skip |= LogError(vuid, cb_state.commandBuffer(), loc.dot(Field::set),
                         "(%" PRIu32
                         ") is greater than "
                         "VkPipelineLayoutCreateInfo::setLayoutCount (%" PRIuLEAST64 ") when layout was created.",
                         set, (uint64_t)pipeline_layout->set_layouts.size());
    } else {
        auto set_layout = pipeline_layout->set_layouts[set];
        if (!(set_layout->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT)) {
            const char *vuid = is_2 ? "VUID-VkBindDescriptorBufferEmbeddedSamplersInfoEXT-set-08070"
                                    : "VUID-vkCmdBindDescriptorBufferEmbeddedSamplersEXT-set-08070";
            skip |= LogError(vuid, cb_state.commandBuffer(), loc,
                             "layout must have been created with the "
                             "VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT flag set.");
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer,
                                                                           VkPipelineBindPoint pipelineBindPoint,
                                                                           VkPipelineLayout layout, uint32_t set,
                                                                           const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    bool skip = false;

    skip |= ValidatePipelineBindPoint(cb_state.get(), pipelineBindPoint, error_obj.location);
    skip |= ValidateCmdBindDescriptorBufferEmbeddedSamplers(*cb_state, layout, set, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdBindDescriptorBufferEmbeddedSamplers2EXT(
    VkCommandBuffer commandBuffer, const VkBindDescriptorBufferEmbeddedSamplersInfoEXT *pBindDescriptorBufferEmbeddedSamplersInfo,
    const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    bool skip = false;

    skip |= ValidateCmdBindDescriptorBufferEmbeddedSamplers(*cb_state, pBindDescriptorBufferEmbeddedSamplersInfo->layout,
                                                            pBindDescriptorBufferEmbeddedSamplersInfo->set, error_obj.location);
    if (!enabled_features.dynamicPipelineLayout && pBindDescriptorBufferEmbeddedSamplersInfo->layout == VK_NULL_HANDLE) {
        skip |=
            LogError("VUID-VkBindDescriptorBufferEmbeddedSamplersInfoEXT-None-09495", device,
                     error_obj.location.dot(Field::pBindDescriptorBufferEmbeddedSamplersInfo).dot(Field::layout), "is not valid.");
    }

    if (IsStageInPipelineBindPoint(pBindDescriptorBufferEmbeddedSamplersInfo->stageFlags, VK_PIPELINE_BIND_POINT_GRAPHICS)) {
        skip |= ValidatePipelineBindPoint(cb_state.get(), VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    }
    if (IsStageInPipelineBindPoint(pBindDescriptorBufferEmbeddedSamplersInfo->stageFlags, VK_PIPELINE_BIND_POINT_COMPUTE)) {
        skip |= ValidatePipelineBindPoint(cb_state.get(), VK_PIPELINE_BIND_POINT_COMPUTE, error_obj.location);
    }
    if (IsStageInPipelineBindPoint(pBindDescriptorBufferEmbeddedSamplersInfo->stageFlags, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR)) {
        skip |= ValidatePipelineBindPoint(cb_state.get(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, error_obj.location);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                            const VkDescriptorBufferBindingInfoEXT *pBindingInfos,
                                                            const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);

    bool skip = false;

    if (!enabled_features.descriptorBuffer) {
        skip |= LogError("VUID-vkCmdBindDescriptorBuffersEXT-None-08047", commandBuffer, error_obj.location,
                         "descriptorBuffer feature was not enabled.");
    }

    uint32_t num_sampler_buffers = 0;
    uint32_t num_resource_buffers = 0;
    uint32_t num_push_descriptor_buffers = 0;

    for (uint32_t i = 0; i < bufferCount; i++) {
        const Location binding_loc = error_obj.location.dot(Field::pBindingInfos, i);
        const VkDescriptorBufferBindingInfoEXT &bindingInfo = pBindingInfos[i];
        const auto buffer_states = GetBuffersByAddress(bindingInfo.address);
        // Try to find a valid buffer in buffer_states.
        // If none if found, output each violated VUIDs, with the list of buffers that violate it.
        {
            using BUFFER_STATE_PTR = ValidationStateTracker::BUFFER_STATE_PTR;
            BufferAddressValidation<5> buffer_address_validator = {{{
                {"VUID-vkCmdBindDescriptorBuffersEXT-pBindingInfos-08052", LogObjectList(device),
                 [this, commandBuffer, binding_loc](const BUFFER_STATE_PTR &buffer_state, std::string *out_error_msg) {
                     if (!out_error_msg) {
                         return !buffer_state->sparse && buffer_state->IsMemoryBound();
                     } else {
                         return ValidateMemoryIsBoundToBuffer(commandBuffer, *buffer_state, binding_loc.dot(Field::address),
                                                              "VUID-vkCmdBindDescriptorBuffersEXT-pBindingInfos-08052");
                     }
                 }},

                {"VUID-vkCmdBindDescriptorBuffersEXT-pBindingInfos-08055", LogObjectList(device),
                 [binding_usage = bindingInfo.usage](const BUFFER_STATE_PTR &buffer_state, std::string *out_error_msg) {
                     if ((buffer_state->usage &
                          (VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT |
                           VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT)) !=
                         (binding_usage &
                          (VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT |
                           VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT))) {
                         if (out_error_msg) {
                             *out_error_msg += "buffer has usage " + string_VkBufferUsageFlags2KHR(buffer_state->usage);
                         }
                         return false;
                     }
                     return true;
                 },
                 [binding_usage = bindingInfo.usage, i]() {
                     return "The following buffers have a usage that does not match pBindingInfos[" + std::to_string(i) +
                            "].usage (" + string_VkBufferUsageFlags2KHR(binding_usage) + "):\n";
                 }},

                {"VUID-VkDescriptorBufferBindingInfoEXT-usage-08122", LogObjectList(device),
                 [binding_usage = bindingInfo.usage, &num_sampler_buffers](const BUFFER_STATE_PTR &buffer_state,
                                                                           std::string *out_error_msg) {
                     if (binding_usage & VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT) {
                         ++num_sampler_buffers;
                         if (!(buffer_state->usage & VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT)) {
                             if (out_error_msg) {
                                 *out_error_msg += "has usage " + string_VkBufferUsageFlags2KHR(buffer_state->usage);
                             }
                             return false;
                         }
                     }
                     return true;
                 },
                 []() {
                     return "The following buffers were not created with VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT:\n";
                 }},

                {"VUID-VkDescriptorBufferBindingInfoEXT-usage-08123", LogObjectList(device),
                 [binding_usage = bindingInfo.usage, &num_resource_buffers](const BUFFER_STATE_PTR &buffer_state,
                                                                            std::string *out_error_msg) {
                     if (binding_usage & VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT) {
                         ++num_resource_buffers;
                         if (!(buffer_state->usage & VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT)) {
                             if (out_error_msg) {
                                 *out_error_msg += "buffer has usage " + string_VkBufferUsageFlags2KHR(buffer_state->usage);
                             }
                             return false;
                         }
                     }
                     return true;
                 },
                 []() {
                     return "The following buffers were not created with VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT:\n";
                 }},

                {"VUID-VkDescriptorBufferBindingInfoEXT-usage-08124", LogObjectList(device),
                 [binding_usage = bindingInfo.usage, &num_push_descriptor_buffers](const BUFFER_STATE_PTR &buffer_state,
                                                                                   std::string *out_error_msg) {
                     if (binding_usage & VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT) {
                         ++num_push_descriptor_buffers;
                         if (!(buffer_state->usage & VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT)) {
                             if (out_error_msg) {
                                 *out_error_msg += "buffer has usage " + string_VkBufferUsageFlags2KHR(buffer_state->usage);
                             }
                             return false;
                         }
                     }
                     return true;
                 },
                 []() {
                     return "The following buffers were not created with "
                            "VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT:\n";
                 }},
            }}};

            skip |= buffer_address_validator.LogErrorsIfNoValidBuffer(*this, buffer_states, binding_loc.dot(Field::address),
                                                                      bindingInfo.address);
        }

        const auto *buffer_handle = vku::FindStructInPNextChain<VkDescriptorBufferBindingPushDescriptorBufferHandleEXT>(pBindingInfos[i].pNext);
        if (!phys_dev_ext_props.descriptor_buffer_props.bufferlessPushDescriptors &&
            (pBindingInfos[i].usage & VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT) && !buffer_handle) {
            skip |= LogError("VUID-VkDescriptorBufferBindingInfoEXT-bufferlessPushDescriptors-08056", commandBuffer,
                             binding_loc.dot(Field::pNext),
                             "does not contain a VkDescriptorBufferBindingPushDescriptorBufferHandleEXT structure, but "
                             "bufferlessPushDescriptors is VK_FALSE and usage "
                             "contains VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT");
        }

        if (SafeModulo(pBindingInfos[i].address, phys_dev_ext_props.descriptor_buffer_props.descriptorBufferOffsetAlignment) != 0) {
            skip |= LogError("VUID-VkDescriptorBufferBindingInfoEXT-address-08057", commandBuffer, binding_loc.dot(Field::address),
                             "(%" PRIuLEAST64
                             ") is not aligned "
                             "to descriptorBufferOffsetAlignment (%" PRIuLEAST64 ")",
                             pBindingInfos[i].address, phys_dev_ext_props.descriptor_buffer_props.descriptorBufferOffsetAlignment);
        }

        if (buffer_handle && phys_dev_ext_props.descriptor_buffer_props.bufferlessPushDescriptors) {
            skip |= LogError("VUID-VkDescriptorBufferBindingPushDescriptorBufferHandleEXT-bufferlessPushDescriptors-08059",
                             commandBuffer, binding_loc.dot(Field::pNext),
                             "contains a VkDescriptorBufferBindingPushDescriptorBufferHandleEXT structure, "
                             "but bufferlessPushDescriptors is VK_TRUE");
        }
    }

    if (num_sampler_buffers > phys_dev_ext_props.descriptor_buffer_props.maxSamplerDescriptorBufferBindings) {
        skip |= LogError(
            "VUID-vkCmdBindDescriptorBuffersEXT-maxSamplerDescriptorBufferBindings-08048", commandBuffer, error_obj.location,
            "Number of sampler buffers is %" PRIu32
            ". There must be no more than "
            "maxSamplerDescriptorBufferBindings (%" PRIu32 ") descriptor buffers containing sampler descriptor data bound. ",
            num_sampler_buffers, phys_dev_ext_props.descriptor_buffer_props.maxSamplerDescriptorBufferBindings);
    }

    if (num_resource_buffers > phys_dev_ext_props.descriptor_buffer_props.maxResourceDescriptorBufferBindings) {
        skip |= LogError(
            "VUID-vkCmdBindDescriptorBuffersEXT-maxResourceDescriptorBufferBindings-08049", commandBuffer, error_obj.location,
            "Number of resource buffers is %" PRIu32
            ". There must be no more than "
            "maxResourceDescriptorBufferBindings (%" PRIu32 ") descriptor buffers containing resource descriptor data bound.",
            num_resource_buffers, phys_dev_ext_props.descriptor_buffer_props.maxResourceDescriptorBufferBindings);
    }

    if (num_push_descriptor_buffers > 1) {
        skip |= LogError("VUID-vkCmdBindDescriptorBuffersEXT-None-08050", commandBuffer, error_obj.location,
                         "Number of descriptor buffers is %" PRIu32
                         ". "
                         "There must be no more than 1 descriptor buffer bound that was created "
                         "with the VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT bit set.",
                         num_push_descriptor_buffers);
    }

    if (bufferCount > phys_dev_ext_props.descriptor_buffer_props.maxDescriptorBufferBindings) {
        skip |= LogError("VUID-vkCmdBindDescriptorBuffersEXT-bufferCount-08051", commandBuffer, error_obj.location,
                         "bufferCount (%" PRIu32
                         ") must be less than or equal to "
                         "VkPhysicalDeviceDescriptorBufferPropertiesEXT::maxDescriptorBufferBindings (%" PRIu32 ").",
                         bufferCount, phys_dev_ext_props.descriptor_buffer_props.maxDescriptorBufferBindings);
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                              VkDeviceSize *pLayoutSizeInBytes,
                                                              const ErrorObject &error_obj) const {
    bool skip = false;

    if (!enabled_features.descriptorBuffer) {
        skip |= LogError("VUID-vkGetDescriptorSetLayoutSizeEXT-None-08011", device, error_obj.location,
                         "descriptorBuffer feature was not enabled.");
    }

    auto setlayout = Get<vvl::DescriptorSetLayout>(layout);

    const auto create_flags = setlayout->GetCreateFlags();
    if (!(create_flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT)) {
        skip |= LogError("VUID-vkGetDescriptorSetLayoutSizeEXT-layout-08012", device, error_obj.location.dot(Field::layout),
                         "was created with %s.", string_VkDescriptorSetLayoutCreateFlags(create_flags).c_str());
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetDescriptorSetLayoutBindingOffsetEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                                       uint32_t binding, VkDeviceSize *pOffset,
                                                                       const ErrorObject &error_obj) const {
    bool skip = false;

    if (!enabled_features.descriptorBuffer) {
        skip |= LogError("VUID-vkGetDescriptorSetLayoutBindingOffsetEXT-None-08013", device, error_obj.location,
                         "descriptorBuffer feature was not enabled.");
    }

    auto setlayout = Get<vvl::DescriptorSetLayout>(layout);

    const auto create_flags = setlayout->GetCreateFlags();
    if (!(setlayout->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT)) {
        skip |=
            LogError("VUID-vkGetDescriptorSetLayoutBindingOffsetEXT-layout-08014", device, error_obj.location.dot(Field::layout),
                     "was created with %s.", string_VkDescriptorSetLayoutCreateFlags(create_flags).c_str());
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                        const VkBufferCaptureDescriptorDataInfoEXT *pInfo,
                                                                        void *pData, const ErrorObject &error_obj) const {
    bool skip = false;

    if (!enabled_features.descriptorBufferCaptureReplay) {
        skip |= LogError("VUID-vkGetBufferOpaqueCaptureDescriptorDataEXT-None-08072", pInfo->buffer, error_obj.location,
                         "descriptorBufferCaptureReplay feature was not enabled.");
    }

    if (physical_device_count > 1 && !enabled_features.bufferDeviceAddressMultiDevice &&
        !enabled_features.bufferDeviceAddressMultiDeviceEXT) {
        skip |= LogError("VUID-vkGetBufferOpaqueCaptureDescriptorDataEXT-device-08074", pInfo->buffer, error_obj.location,
                         "device was created with multiple physical devices (%" PRIu32
                         "), but the "
                         "bufferDeviceAddressMultiDevice feature was not enabled.",
                         physical_device_count);
    }

    auto buffer_state = Get<vvl::Buffer>(pInfo->buffer);

    if (buffer_state) {
        if (!(buffer_state->createInfo.flags & VK_BUFFER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
            skip |= LogError("VUID-VkBufferCaptureDescriptorDataInfoEXT-buffer-08075", pInfo->buffer,
                             error_obj.location.dot(Field::pInfo).dot(Field::buffer), "was created with %s.",
                             string_VkBufferCreateFlags(buffer_state->createInfo.flags).c_str());
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                       const VkImageCaptureDescriptorDataInfoEXT *pInfo,
                                                                       void *pData, const ErrorObject &error_obj) const {
    bool skip = false;

    if (!enabled_features.descriptorBufferCaptureReplay) {
        skip |= LogError("VUID-vkGetImageOpaqueCaptureDescriptorDataEXT-None-08076", pInfo->image, error_obj.location,
                         "descriptorBufferCaptureReplay feature was not enabled.");
    }

    if (physical_device_count > 1 && !enabled_features.bufferDeviceAddressMultiDevice &&
        !enabled_features.bufferDeviceAddressMultiDeviceEXT) {
        skip |= LogError("VUID-vkGetImageOpaqueCaptureDescriptorDataEXT-device-08078", pInfo->image, error_obj.location,
                         "device was created with multiple physical devices (%" PRIu32
                         "), but the "
                         "bufferDeviceAddressMultiDevice feature was not enabled.",
                         physical_device_count);
    }

    auto image_state = Get<vvl::Image>(pInfo->image);

    if (image_state) {
        if (!(image_state->createInfo.flags & VK_IMAGE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
            skip |= LogError("VUID-VkImageCaptureDescriptorDataInfoEXT-image-08079", pInfo->image,
                             error_obj.location.dot(Field::pInfo).dot(Field::image), "is %s.",
                             string_VkImageCreateFlags(image_state->createInfo.flags).c_str());
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                           const VkImageViewCaptureDescriptorDataInfoEXT *pInfo,
                                                                           void *pData, const ErrorObject &error_obj) const {
    bool skip = false;

    if (!enabled_features.descriptorBufferCaptureReplay) {
        skip |= LogError("VUID-vkGetImageViewOpaqueCaptureDescriptorDataEXT-None-08080", pInfo->imageView, error_obj.location,
                         "descriptorBufferCaptureReplay feature was not enabled.");
    }

    if (physical_device_count > 1 && !enabled_features.bufferDeviceAddressMultiDevice &&
        !enabled_features.bufferDeviceAddressMultiDeviceEXT) {
        skip |= LogError("VUID-vkGetImageViewOpaqueCaptureDescriptorDataEXT-device-08082", pInfo->imageView, error_obj.location,
                         "device was created with multiple physical devices (%" PRIu32
                         "), but the "
                         "bufferDeviceAddressMultiDevice feature was not enabled.",
                         physical_device_count);
    }

    auto image_view_state = Get<vvl::ImageView>(pInfo->imageView);

    if (image_view_state) {
        if (!(image_view_state->create_info.flags & VK_IMAGE_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
            skip |= LogError("VUID-VkImageViewCaptureDescriptorDataInfoEXT-imageView-08083", pInfo->imageView,
                             error_obj.location.dot(Field::pInfo).dot(Field::imageView), "is %s.",
                             string_VkImageViewCreateFlags(image_view_state->create_info.flags).c_str());
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device,
                                                                         const VkSamplerCaptureDescriptorDataInfoEXT *pInfo,
                                                                         void *pData, const ErrorObject &error_obj) const {
    bool skip = false;

    if (!enabled_features.descriptorBufferCaptureReplay) {
        skip |= LogError("VUID-vkGetSamplerOpaqueCaptureDescriptorDataEXT-None-08084", pInfo->sampler, error_obj.location,
                         "descriptorBufferCaptureReplay feature was not enabled.");
    }

    if (physical_device_count > 1 && !enabled_features.bufferDeviceAddressMultiDevice &&
        !enabled_features.bufferDeviceAddressMultiDeviceEXT) {
        skip |= LogError("VUID-vkGetSamplerOpaqueCaptureDescriptorDataEXT-device-08086", pInfo->sampler, error_obj.location,
                         "device was created with multiple physical devices (%" PRIu32
                         "), but the "
                         "bufferDeviceAddressMultiDevice feature was not enabled.",
                         physical_device_count);
    }

    auto sampler_state = Get<vvl::Sampler>(pInfo->sampler);

    if (sampler_state) {
        if (!(sampler_state->createInfo.flags & VK_SAMPLER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
            skip |= LogError("VUID-VkSamplerCaptureDescriptorDataInfoEXT-sampler-08087", pInfo->sampler,
                             error_obj.location.dot(Field::pInfo).dot(Field::sampler), "is %s.",
                             string_VkSamplerCreateFlags(sampler_state->createInfo.flags).c_str());
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT *pInfo, void *pData,
    const ErrorObject &error_obj) const {
    bool skip = false;

    if (!enabled_features.descriptorBufferCaptureReplay) {
        skip |= LogError("VUID-vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT-None-08088", device, error_obj.location,
                         "descriptorBufferCaptureReplay feature was not enabled.");
    }

    if (physical_device_count > 1 && !enabled_features.bufferDeviceAddressMultiDevice &&
        !enabled_features.bufferDeviceAddressMultiDeviceEXT) {
        skip |= LogError("VUID-vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT-device-08090", device, error_obj.location,
                         "device was created with multiple physical devices (%" PRIu32
                         "), but the "
                         "bufferDeviceAddressMultiDevice feature was not enabled.",
                         physical_device_count);
    }

    if (pInfo->accelerationStructure != VK_NULL_HANDLE) {
        auto acceleration_structure_state = Get<vvl::AccelerationStructureKHR>(pInfo->accelerationStructure);

        if (acceleration_structure_state) {
            if (!(acceleration_structure_state->create_infoKHR.createFlags &
                  VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
                skip |= LogError(
                    "VUID-VkAccelerationStructureCaptureDescriptorDataInfoEXT-accelerationStructure-08091",
                    pInfo->accelerationStructure, error_obj.location, "pInfo->accelerationStructure was %s.",
                    string_VkAccelerationStructureCreateFlagsKHR(acceleration_structure_state->create_infoKHR.createFlags).c_str());
            }
        }

        if (pInfo->accelerationStructureNV != VK_NULL_HANDLE) {
            LogError("VUID-VkAccelerationStructureCaptureDescriptorDataInfoEXT-accelerationStructure-08093", device,
                     error_obj.location,
                     "If accelerationStructure is not VK_NULL_HANDLE, accelerationStructureNV must be VK_NULL_HANDLE. ");
        }
    }

    if (pInfo->accelerationStructureNV != VK_NULL_HANDLE) {
        auto acceleration_structure_state = Get<vvl::AccelerationStructureNV>(pInfo->accelerationStructureNV);

        if (acceleration_structure_state) {
            if (!(acceleration_structure_state->create_infoNV.info.flags &
                  VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
                skip |= LogError(
                    "VUID-VkAccelerationStructureCaptureDescriptorDataInfoEXT-accelerationStructureNV-08092",
                    pInfo->accelerationStructureNV, error_obj.location, "pInfo->accelerationStructure was %s.",
                    string_VkAccelerationStructureCreateFlagsKHR(acceleration_structure_state->create_infoNV.info.flags).c_str());
            }
        }

        if (pInfo->accelerationStructure != VK_NULL_HANDLE) {
            LogError("VUID-VkAccelerationStructureCaptureDescriptorDataInfoEXT-accelerationStructureNV-08094", device,
                     error_obj.location,
                     "If accelerationStructureNV is not VK_NULL_HANDLE, accelerationStructure must be VK_NULL_HANDLE. ");
        }
    }

    return skip;
}

bool CoreChecks::ValidateDescriptorAddressInfoEXT(const VkDescriptorAddressInfoEXT *address_info,
                                                  const Location &address_loc) const {
    bool skip = false;

    if (address_info->range == 0) {
        skip |= LogError("VUID-VkDescriptorAddressInfoEXT-range-08940", device, address_loc.dot(Field::range), "is zero.");
    }

    if (address_info->address == 0) {
        if (!enabled_features.nullDescriptor) {
            skip |= LogError("VUID-VkDescriptorAddressInfoEXT-address-08043", device, address_loc.dot(Field::address),
                             "is zero, but the nullDescriptor feature was not enabled.");
        } else if (address_info->range != VK_WHOLE_SIZE) {
            skip |= LogError("VUID-VkDescriptorAddressInfoEXT-nullDescriptor-08938", device, address_loc.dot(Field::range),
                             "(%" PRIu64 ") is not VK_WHOLE_SIZE, but address is zero.", address_info->range);
        }
    } else {
        if (address_info->range == VK_WHOLE_SIZE) {
            skip |= LogError("VUID-VkDescriptorAddressInfoEXT-nullDescriptor-08939", device, address_loc.dot(Field::range),
                             "is VK_WHOLE_SIZE.");
        }
    }

    const auto buffer_states = GetBuffersByAddress(address_info->address);
    if ((address_info->address != 0) && buffer_states.empty()) {
        skip |= LogError("VUID-VkDescriptorAddressInfoEXT-None-08044", device, address_loc.dot(Field::address),
                         "(0x%" PRIx64 ") is not a valid buffer address.", address_info->address);
    } else {
        using BUFFER_STATE_PTR = ValidationStateTracker::BUFFER_STATE_PTR;
        BufferAddressValidation<1> buffer_address_validator = {
            {{{"VUID-VkDescriptorAddressInfoEXT-range-08045", LogObjectList(device),
               [&address_info](const BUFFER_STATE_PTR &buffer_state, std::string *out_error_msg) {
                   if (address_info->range >
                       buffer_state->createInfo.size - (address_info->address - buffer_state->deviceAddress)) {
                       if (out_error_msg) {
                           *out_error_msg += "range goes past buffer end";
                       }
                       return false;
                   }
                   return true;
               }}}}};

        skip |= buffer_address_validator.LogErrorsIfNoValidBuffer(*this, buffer_states, address_loc.dot(Field::address),
                                                                  address_info->address);
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetDescriptorEXT(VkDevice device, const VkDescriptorGetInfoEXT *pDescriptorInfo, size_t dataSize,
                                                 void *pDescriptor, const ErrorObject &error_obj) const {
    bool skip = false;

    if (!enabled_features.descriptorBuffer) {
        skip |=
            LogError("VUID-vkGetDescriptorEXT-None-08015", device, error_obj.location, "descriptorBuffer feature was not enabled.");
    }

    const Location descriptor_info_loc = error_obj.location.dot(Field::pDescriptorInfo);
    switch (pDescriptorInfo->type) {
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
            skip |= LogError("VUID-VkDescriptorGetInfoEXT-type-08018", device, descriptor_info_loc.dot(Field::type), "is %s.",
                             string_VkDescriptorType(pDescriptorInfo->type));
            break;
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            if (Get<vvl::Sampler>(pDescriptorInfo->data.pCombinedImageSampler->sampler).get() == nullptr) {
                skip |= LogError("VUID-VkDescriptorGetInfoEXT-type-08019", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, but "
                                 "pCombinedImageSampler->sampler is not a valid sampler.");
            }
            if ((pDescriptorInfo->data.pCombinedImageSampler->imageView != VK_NULL_HANDLE) &&
                (Get<vvl::ImageView>(pDescriptorInfo->data.pCombinedImageSampler->imageView).get() == nullptr)) {
                skip |= LogError("VUID-VkDescriptorGetInfoEXT-type-08020", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, but "
                                 "pCombinedImageSampler->imageView is not a valid image view.");
            }
            break;
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            if (Get<vvl::ImageView>(pDescriptorInfo->data.pInputAttachmentImage->imageView).get() == nullptr) {
                skip |= LogError("VUID-VkDescriptorGetInfoEXT-type-08021", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, but "
                                 "pInputAttachmentImage->imageView is not valid image view.");
            }
            break;
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            if (pDescriptorInfo->data.pSampledImage && (pDescriptorInfo->data.pSampledImage->imageView != VK_NULL_HANDLE) &&
                (Get<vvl::ImageView>(pDescriptorInfo->data.pSampledImage->imageView).get() == nullptr)) {
                skip |= LogError("VUID-VkDescriptorGetInfoEXT-type-08022", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, but "
                                 "pSampledImage->imageView is not a valid image view.");
            }
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            if (pDescriptorInfo->data.pStorageImage && (pDescriptorInfo->data.pStorageImage->imageView != VK_NULL_HANDLE) &&
                (Get<vvl::ImageView>(pDescriptorInfo->data.pStorageImage->imageView).get() == nullptr)) {
                skip |= LogError("VUID-VkDescriptorGetInfoEXT-type-08023", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, but "
                                 "pStorageImage->imageView is not a valid image view.");
            }
            break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            if (pDescriptorInfo->data.pUniformTexelBuffer && (pDescriptorInfo->data.pUniformTexelBuffer->address != 0) &&
                (GetBuffersByAddress(pDescriptorInfo->data.pUniformTexelBuffer->address).empty())) {
                skip |= LogError("VUID-VkDescriptorGetInfoEXT-type-08024", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, but "
                                 "pUniformTexelBuffer->address (%" PRIu64
                                 ") is not zero or "
                                 "an address within a buffer.",
                                 pDescriptorInfo->data.pUniformTexelBuffer->address);
            }
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            if (pDescriptorInfo->data.pStorageTexelBuffer && (pDescriptorInfo->data.pStorageTexelBuffer->address != 0) &&
                (GetBuffersByAddress(pDescriptorInfo->data.pStorageTexelBuffer->address).empty())) {
                skip |= LogError("VUID-VkDescriptorGetInfoEXT-type-08025", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, but "
                                 "pStorageTexelBuffer->address (%" PRIu64
                                 ") is not zero or "
                                 "an address within a buffer.",
                                 pDescriptorInfo->data.pStorageTexelBuffer->address);
            }
            break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            if (pDescriptorInfo->data.pUniformBuffer && (pDescriptorInfo->data.pUniformBuffer->address != 0) &&
                (GetBuffersByAddress(pDescriptorInfo->data.pStorageTexelBuffer->address).empty())) {
                skip |= LogError("VUID-VkDescriptorGetInfoEXT-type-08026", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, but "
                                 "pUniformBuffer->address (%" PRIu64
                                 ") is not zero or "
                                 "an address within a buffer.",
                                 pDescriptorInfo->data.pUniformBuffer->address);
            }
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            if (pDescriptorInfo->data.pStorageBuffer && (pDescriptorInfo->data.pStorageBuffer->address != 0) &&
                (GetBuffersByAddress(pDescriptorInfo->data.pStorageBuffer->address).empty())) {
                skip |= LogError("VUID-VkDescriptorGetInfoEXT-type-08027", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, but "
                                 "pStorageBuffer->address (%" PRIu64
                                 ") is not zero or "
                                 "an address within a buffer.",
                                 pDescriptorInfo->data.pStorageBuffer->address);
            }
            break;

        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
            if (pDescriptorInfo->data.accelerationStructure) {
                const VkAccelerationStructureNV as = (VkAccelerationStructureNV)pDescriptorInfo->data.accelerationStructure;
                auto as_state = Get<vvl::AccelerationStructureNV>(as);

                if (!as_state) {
                    skip |= LogError("VUID-VkDescriptorGetInfoEXT-type-08029", device, descriptor_info_loc.dot(Field::type),
                                     "is VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV and accelerationStructure is not 0, "
                                     "accelerationStructure must contain the handle of a VkAccelerationStructureNV created on "
                                     "device, returned by vkGetAccelerationStructureHandleNV");
                }
            }
            break;

        default:
            break;
    }

    std::string_view vuid_memory_bound = "";
    using BUFFER_STATE_PTR = ValidationStateTracker::BUFFER_STATE_PTR;
    BufferAddressValidation<1> buffer_address_validator = {
        // TODO - The Descriptor Buffer logic could be simplified as it takes this VUID but also captures a VUID and not easy to
        // distinguish which VUID maps where
        {{{kVUIDUndefined, LogObjectList(device),
           [this, device, &vuid_memory_bound, descriptor_info_loc](const BUFFER_STATE_PTR &buffer_state,
                                                                   std::string *out_error_msg) {
               if (!out_error_msg) {
                   return !buffer_state->sparse && buffer_state->IsMemoryBound();
               } else {
                   return ValidateMemoryIsBoundToBuffer(
                       device, *buffer_state, descriptor_info_loc.dot(Field::data).dot(Field::pUniformBuffer).dot(Field::address),
                       vuid_memory_bound.data());
               }
           }}}}};

    switch (pDescriptorInfo->type) {
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            if (pDescriptorInfo->data.pUniformBuffer) {
                const auto buffer_states = GetBuffersByAddress(pDescriptorInfo->data.pUniformBuffer->address);
                if (!buffer_states.empty()) {
                    vuid_memory_bound = "VUID-VkDescriptorDataEXT-type-08030";
                    skip |= buffer_address_validator.LogErrorsIfNoValidBuffer(
                        *this, buffer_states, descriptor_info_loc.dot(Field::data).dot(Field::pUniformBuffer).dot(Field::address),
                        pDescriptorInfo->data.pUniformBuffer->address);
                }
            } else if (!enabled_features.nullDescriptor) {
                skip |= LogError("VUID-VkDescriptorDataEXT-type-08039", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, but "
                                 "pUniformBuffer is NULL and the nullDescriptor feature was not enabled.");
            }
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            if (pDescriptorInfo->data.pStorageBuffer) {
                const auto buffer_states = GetBuffersByAddress(pDescriptorInfo->data.pUniformBuffer->address);
                if (!buffer_states.empty()) {
                    vuid_memory_bound = "VUID-VkDescriptorDataEXT-type-08031";
                    skip |= buffer_address_validator.LogErrorsIfNoValidBuffer(
                        *this, buffer_states, descriptor_info_loc.dot(Field::data).dot(Field::pUniformBuffer).dot(Field::address),
                        pDescriptorInfo->data.pUniformBuffer->address);
                }
            } else if (!enabled_features.nullDescriptor) {
                skip |= LogError("VUID-VkDescriptorDataEXT-type-08040", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, but "
                                 "pStorageBuffer is NULL and the nullDescriptor feature was not enabled.");
            }
            break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            if (pDescriptorInfo->data.pUniformTexelBuffer) {
                const auto buffer_states = GetBuffersByAddress(pDescriptorInfo->data.pUniformBuffer->address);
                if (!buffer_states.empty()) {
                    vuid_memory_bound = "VUID-VkDescriptorDataEXT-type-08032";
                    skip |= buffer_address_validator.LogErrorsIfNoValidBuffer(
                        *this, buffer_states, descriptor_info_loc.dot(Field::data).dot(Field::pUniformBuffer).dot(Field::address),
                        pDescriptorInfo->data.pUniformBuffer->address);
                }
            } else if (!enabled_features.nullDescriptor) {
                skip |= LogError("VUID-VkDescriptorDataEXT-type-08037", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, but "
                                 "pUniformTexelBuffer is NULL and the nullDescriptor feature was not enabled.");
            }
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            if (pDescriptorInfo->data.pStorageTexelBuffer) {
                const auto buffer_states = GetBuffersByAddress(pDescriptorInfo->data.pUniformBuffer->address);
                if (!buffer_states.empty()) {
                    vuid_memory_bound = "VUID-VkDescriptorDataEXT-type-08033";
                    skip |= buffer_address_validator.LogErrorsIfNoValidBuffer(
                        *this, buffer_states, descriptor_info_loc.dot(Field::data).dot(Field::pUniformBuffer).dot(Field::address),
                        pDescriptorInfo->data.pUniformBuffer->address);
                }
            } else if (!enabled_features.nullDescriptor) {
                skip |= LogError("VUID-VkDescriptorDataEXT-type-08038", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, but "
                                 "pStorageTexelBuffer is NULL and the nullDescriptor feature was not enabled.");
            }
            break;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
            if ((pDescriptorInfo->data.accelerationStructure == 0) && !enabled_features.nullDescriptor) {
                skip |= LogError("VUID-VkDescriptorDataEXT-type-08041", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, but "
                                 "accelerationStructure is 0 and the nullDescriptor feature was not enabled.");
            }
            break;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
            if ((pDescriptorInfo->data.accelerationStructure == 0) && !enabled_features.nullDescriptor) {
                skip |= LogError("VUID-VkDescriptorDataEXT-type-08042", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, but "
                                 "accelerationStructure is 0 and the nullDescriptor feature was not enabled.");
            }
            break;
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            if ((pDescriptorInfo->data.pCombinedImageSampler->imageView == VK_NULL_HANDLE) && !enabled_features.nullDescriptor) {
                skip |=
                    LogError("VUID-VkDescriptorDataEXT-type-08034", device, descriptor_info_loc.dot(Field::type),
                             "is VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, but "
                             "pCombinedImageSampler->imageView is VK_NULL_HANDLE and the nullDescriptor feature is not enabled.");
            }
            break;
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            if (!enabled_features.nullDescriptor &&
                (!pDescriptorInfo->data.pSampledImage || (pDescriptorInfo->data.pSampledImage->imageView == VK_NULL_HANDLE))) {
                skip |= LogError("VUID-VkDescriptorDataEXT-type-08035", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, but "
                                 "pSampledImage is NULL, or pSampledImage->imageView is VK_NULL_HANDLE, and the nullDescriptor "
                                 "feature is not enabled.");
            }
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            if (!enabled_features.nullDescriptor &&
                (!pDescriptorInfo->data.pStorageImage || (pDescriptorInfo->data.pStorageImage->imageView == VK_NULL_HANDLE))) {
                skip |= LogError("VUID-VkDescriptorDataEXT-type-08036", device, descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, but "
                                 "pStorageImage is NULL, or pStorageImage->imageView is VK_NULL_HANDLE, and the nullDescriptor "
                                 "feature is not enabled.");
            }
            break;
        default:
            break;
    }

    switch (pDescriptorInfo->type) {
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            if (pDescriptorInfo->data.pUniformTexelBuffer) {
                skip |= ValidateDescriptorAddressInfoEXT(pDescriptorInfo->data.pUniformTexelBuffer,
                                                         descriptor_info_loc.dot(Field::data).dot(Field::pUniformTexelBuffer));
            }
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            if (pDescriptorInfo->data.pStorageTexelBuffer) {
                skip |= ValidateDescriptorAddressInfoEXT(pDescriptorInfo->data.pStorageTexelBuffer,
                                                         descriptor_info_loc.dot(Field::data).dot(Field::pStorageTexelBuffer));
            }
            break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            if (pDescriptorInfo->data.pUniformBuffer) {
                skip |= ValidateDescriptorAddressInfoEXT(pDescriptorInfo->data.pUniformBuffer,
                                                         descriptor_info_loc.dot(Field::data).dot(Field::pUniformBuffer));
            }
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            if (pDescriptorInfo->data.pStorageBuffer) {
                skip |= ValidateDescriptorAddressInfoEXT(pDescriptorInfo->data.pStorageBuffer,
                                                         descriptor_info_loc.dot(Field::data).dot(Field::pStorageBuffer));
            }
            break;
        default:
            break;
    }

    bool checkDataSize = false;
    std::size_t size = 0u;

    switch (pDescriptorInfo->type) {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
            checkDataSize = true;
            size = phys_dev_ext_props.descriptor_buffer_props.samplerDescriptorSize;
            break;

        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            checkDataSize = true;
            size = phys_dev_ext_props.descriptor_buffer_props.combinedImageSamplerDescriptorSize;
            break;

        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            checkDataSize = true;
            size = phys_dev_ext_props.descriptor_buffer_props.sampledImageDescriptorSize;
            break;

        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            checkDataSize = true;
            size = phys_dev_ext_props.descriptor_buffer_props.storageImageDescriptorSize;
            break;

        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            checkDataSize = true;
            size = enabled_features.robustBufferAccess
                       ? phys_dev_ext_props.descriptor_buffer_props.robustUniformTexelBufferDescriptorSize
                       : phys_dev_ext_props.descriptor_buffer_props.uniformTexelBufferDescriptorSize;
            break;

        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            checkDataSize = true;
            size = enabled_features.robustBufferAccess
                       ? phys_dev_ext_props.descriptor_buffer_props.robustStorageTexelBufferDescriptorSize
                       : phys_dev_ext_props.descriptor_buffer_props.storageTexelBufferDescriptorSize;
            break;

        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            checkDataSize = true;
            size = enabled_features.robustBufferAccess
                       ? phys_dev_ext_props.descriptor_buffer_props.robustUniformBufferDescriptorSize
                       : phys_dev_ext_props.descriptor_buffer_props.uniformBufferDescriptorSize;
            break;

        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            checkDataSize = true;
            size = enabled_features.robustBufferAccess
                       ? phys_dev_ext_props.descriptor_buffer_props.robustStorageBufferDescriptorSize
                       : phys_dev_ext_props.descriptor_buffer_props.storageBufferDescriptorSize;
            break;

        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            checkDataSize = true;
            size = phys_dev_ext_props.descriptor_buffer_props.inputAttachmentDescriptorSize;
            break;

        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
            checkDataSize = true;
            size = phys_dev_ext_props.descriptor_buffer_props.accelerationStructureDescriptorSize;
            break;
        default:
            break;
    }

    if (pDescriptorInfo->type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER && pDescriptorInfo->data.pSampler != nullptr) {
        const auto sampler_state = Get<vvl::Sampler>(*pDescriptorInfo->data.pSampler);

        if (sampler_state && (0 != (sampler_state->createInfo.flags & VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT))) {
            dataSize = phys_dev_ext_props.descriptor_buffer_density_props.combinedImageSamplerDensityMapDescriptorSize;
            checkDataSize = true;
        }
    }

    if (checkDataSize && size != dataSize) {
        skip |= LogError("VUID-vkGetDescriptorEXT-dataSize-08125", device, error_obj.location,
                         "dataSize (%zu) must equal the size of a descriptor (%zu) of type "
                         "VkDescriptorGetInfoEXT::type "
                         "determined by the value in VkPhysicalDeviceDescriptorBufferPropertiesEXT, or "
                         "VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT::combinedImageSamplerDensityMapDescriptorSize if "
                         "pDescriptorInfo specifies a VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER whose VkSampler was created with "
                         "VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT set",
                         dataSize, size);
    }

    return skip;
}

bool CoreChecks::PreCallValidateResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                    VkDescriptorPoolResetFlags flags, const ErrorObject &error_obj) const {
    // Make sure sets being destroyed are not currently in-use
    if (disabled[object_in_use]) return false;
    bool skip = false;
    auto pool = Get<vvl::DescriptorPool>(descriptorPool);
    if (!pool) {
        return false;
    }
    const auto *used_handle = pool->InUse();
    if (used_handle) {
        skip |= LogError("VUID-vkResetDescriptorPool-descriptorPool-00313", descriptorPool,
                         error_obj.location.dot(Field::descriptorPool), "descriptor sets in use by %s.",
                         FormatHandle(*used_handle).c_str());
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                      const VkAllocationCallbacks *pAllocator, const ErrorObject &error_obj) const {
    auto desc_pool_state = Get<vvl::DescriptorPool>(descriptorPool);
    bool skip = false;
    if (desc_pool_state) {
        skip |=
            ValidateObjectNotInUse(desc_pool_state.get(), error_obj.location, "VUID-vkDestroyDescriptorPool-descriptorPool-00303");
    }
    return skip;
}

// Ensure the pool contains enough descriptors and descriptor sets to satisfy
// an allocation request. Fills common_data with the total number of descriptors of each type required,
// as well as DescriptorSetLayout ptrs used for later update.
bool CoreChecks::PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                       VkDescriptorSet *pDescriptorSets, const ErrorObject &error_obj,
                                                       void *ads_state_data) const {
    StateTracker::PreCallValidateAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, error_obj, ads_state_data);

    vvl::AllocateDescriptorSetsData *ds_data =
        reinterpret_cast<vvl::AllocateDescriptorSetsData *>(ads_state_data);

    bool skip = false;
    auto pool_state = Get<vvl::DescriptorPool>(pAllocateInfo->descriptorPool);
    const Location allocate_info_loc = error_obj.location.dot(Field::pAllocateInfo);

    for (uint32_t i = 0; i < pAllocateInfo->descriptorSetCount; i++) {
        const Location set_layout_loc = allocate_info_loc.dot(Field::pSetLayouts, i);
        auto layout = Get<vvl::DescriptorSetLayout>(pAllocateInfo->pSetLayouts[i]);
        if (!layout) {
            // nullptr layout indicates no valid layout handle for this device, validated/logged in object_tracker
            continue;
        }
        if (layout->IsPushDescriptor()) {
            skip |= LogError("VUID-VkDescriptorSetAllocateInfo-pSetLayouts-00308", pAllocateInfo->pSetLayouts[i], set_layout_loc,
                             "(%s) was created with VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR.",
                             FormatHandle(pAllocateInfo->pSetLayouts[i]).c_str());
        }
        if (layout->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) {
            skip |= LogError("VUID-VkDescriptorSetAllocateInfo-pSetLayouts-08009", pAllocateInfo->pSetLayouts[i], set_layout_loc,
                             "(%s) was created with VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT.",
                             FormatHandle(pAllocateInfo->pSetLayouts[i]).c_str());
        }
        if (layout->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT &&
            !(pool_state->createInfo.flags & VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)) {
            const LogObjectList objlist(pAllocateInfo->descriptorPool, pAllocateInfo->pSetLayouts[i]);
            skip |= LogError("VUID-VkDescriptorSetAllocateInfo-pSetLayouts-03044", objlist, set_layout_loc,
                             "was created with %s but the descriptorPool was created with %s",
                             string_VkDescriptorSetLayoutCreateFlags(layout->GetCreateFlags()).c_str(),
                             string_VkDescriptorPoolCreateFlags(pool_state->createInfo.flags).c_str());
        }
        if (layout->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT &&
            !(pool_state->createInfo.flags & VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT)) {
            const LogObjectList objlist(pAllocateInfo->descriptorPool, pAllocateInfo->pSetLayouts[i]);
            skip |= LogError("VUID-VkDescriptorSetAllocateInfo-pSetLayouts-04610", objlist, set_layout_loc,
                             "was created with %s but the descriptorPool was created with %s",
                             string_VkDescriptorSetLayoutCreateFlags(layout->GetCreateFlags()).c_str(),
                             string_VkDescriptorPoolCreateFlags(pool_state->createInfo.flags).c_str());
        }
    }
    if (!IsExtEnabled(device_extensions.vk_khr_maintenance1)) {
        // Track number of descriptorSets allowable in this pool
        if (pool_state->GetAvailableSets() < pAllocateInfo->descriptorSetCount) {
            skip |= LogError("VUID-VkDescriptorSetAllocateInfo-apiVersion-07895", pool_state->Handle(), error_obj.location,
                             "Unable to allocate %" PRIu32
                             " descriptorSets from %s"
                             ". This pool only has %" PRIu32 " descriptorSets remaining.",
                             pAllocateInfo->descriptorSetCount, FormatHandle(*pool_state).c_str(), pool_state->GetAvailableSets());
        }
        // Determine whether descriptor counts are satisfiable
        for (auto it = ds_data->required_descriptors_by_type.begin(); it != ds_data->required_descriptors_by_type.end(); ++it) {
            auto available_count = pool_state->GetAvailableCount(it->first);

            if (ds_data->required_descriptors_by_type.at(it->first) > available_count) {
                skip |= LogError("VUID-VkDescriptorSetAllocateInfo-apiVersion-07896", pool_state->Handle(), error_obj.location,
                                 "Unable to allocate %" PRIu32
                                 " descriptors of type %s from %s"
                                 ". This pool only has %" PRIu32 " descriptors of this type remaining.",
                                 ds_data->required_descriptors_by_type.at(it->first),
                                 string_VkDescriptorType(VkDescriptorType(it->first)), FormatHandle(*pool_state).c_str(),
                                 available_count);
            }
        }
    }

    const auto *count_allocate_info = vku::FindStructInPNextChain<VkDescriptorSetVariableDescriptorCountAllocateInfo>(pAllocateInfo->pNext);
    if (count_allocate_info) {
        if (count_allocate_info->descriptorSetCount != 0 &&
            count_allocate_info->descriptorSetCount != pAllocateInfo->descriptorSetCount) {
            skip |= LogError(
                "VUID-VkDescriptorSetVariableDescriptorCountAllocateInfo-descriptorSetCount-03045", device,
                allocate_info_loc.pNext(Struct::VkDescriptorSetVariableDescriptorCountAllocateInfo, Field::descriptorSetCount),
                "(%" PRIu32 ") != pAllocateInfo->descriptorSetCount (%" PRIu32 ").", count_allocate_info->descriptorSetCount,
                pAllocateInfo->descriptorSetCount);
        }
        if (count_allocate_info->descriptorSetCount == pAllocateInfo->descriptorSetCount) {
            for (uint32_t i = 0; i < pAllocateInfo->descriptorSetCount; i++) {
                auto layout = Get<vvl::DescriptorSetLayout>(pAllocateInfo->pSetLayouts[i]);
                if (count_allocate_info->pDescriptorCounts[i] > layout->GetDescriptorCountFromBinding(layout->GetMaxBinding())) {
                    skip |= LogError("VUID-VkDescriptorSetVariableDescriptorCountAllocateInfo-pSetLayouts-03046", device,
                                     allocate_info_loc.pNext(Struct::VkDescriptorSetVariableDescriptorCountAllocateInfo,
                                                             Field::pDescriptorCounts, i),
                                     "is %" PRIu32 ", but pAllocateInfo->pSetLayouts[%" PRIu32
                                     "] binding's descriptorCount = (%" PRIu32 ")",
                                     count_allocate_info->pDescriptorCounts[i], i,
                                     layout->GetDescriptorCountFromBinding(layout->GetMaxBinding()));
                }
            }
        }
    }

    return skip;
}

void CoreChecks::PostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                      VkDescriptorSet *pDescriptorSets, const RecordObject &record_obj,
                                                      void *ads_state) {
    // Discussed in https://gitlab.khronos.org/vulkan/vulkan/-/issues/3347
    // The issue if users see VK_ERROR_OUT_OF_POOL_MEMORY they think they over-allocated, but if they instead allocated type not
    // avaiable (so the pool size is zero), they will just keep getting this error mistakenly thinking they ran out. It was decided
    // that this deserves to be a Core Validation check
    if (record_obj.result == VK_ERROR_OUT_OF_POOL_MEMORY && pAllocateInfo) {
        // result type added in VK_KHR_maintenance1
        auto pool_state = Get<vvl::DescriptorPool>(pAllocateInfo->descriptorPool);
        if (!pool_state) {
            return;
        }
        for (uint32_t i = 0; i < pAllocateInfo->descriptorSetCount; i++) {
            auto layout = Get<vvl::DescriptorSetLayout>(pAllocateInfo->pSetLayouts[i]);
            if (!layout) {
                continue;
            }

            const uint32_t binding_count = layout->GetBindingCount();
            for (uint32_t j = 0; j < binding_count; ++j) {
                const VkDescriptorType type = layout->GetTypeFromIndex(j);
                if (!pool_state->IsAvailableType(type)) {
                    // This check would be caught by validation if VK_KHR_maintenance1 was not enabled
                    LogWarning("WARNING-CoreValidation-AllocateDescriptorSets-WrongType", pool_state->Handle(),
                               record_obj.location.dot(Field::pAllocateInfo).dot(Field::pSetLayouts, i),
                               "binding %" PRIu32
                               " was created with %s but the "
                               "Descriptor Pool was not created with this type and returned VK_ERROR_OUT_OF_POOL_MEMORY",
                               j, string_VkDescriptorType(type));
                }
            }
        }
    }

    StateTracker::PostCallRecordAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, record_obj, ads_state);
}

// Validate that given set is valid and that it's not being used by an in-flight CmdBuffer
// func_str is the name of the calling function
// Return false if no errors occur
// Return true if validation error occurs and callback returns true (to skip upcoming API call down the chain)
bool CoreChecks::ValidateIdleDescriptorSet(VkDescriptorSet set, const Location &loc) const {
    if (disabled[object_in_use]) return false;
    bool skip = false;
    auto set_node = Get<vvl::DescriptorSet>(set);
    if (!set_node) {
        return false;
    }
    // TODO : This covers various error cases so should pass error enum into this function and use passed in enum here
    const auto *used_handle = set_node->InUse();
    if (used_handle) {
        skip |= LogError("VUID-vkFreeDescriptorSets-pDescriptorSets-00309", set, loc, "%s is in use by %s.",
                         FormatHandle(set).c_str(), FormatHandle(*used_handle).c_str());
    }
    return skip;
}

bool CoreChecks::PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t count,
                                                   const VkDescriptorSet *pDescriptorSets, const ErrorObject &error_obj) const {
    // Make sure that no sets being destroyed are in-flight
    bool skip = false;
    // First make sure sets being destroyed are not currently in-use
    for (uint32_t i = 0; i < count; ++i) {
        if (pDescriptorSets[i] != VK_NULL_HANDLE) {
            skip |= ValidateIdleDescriptorSet(pDescriptorSets[i], error_obj.location.dot(Field::pDescriptorSets, i));
        }
    }
    auto pool_state = Get<vvl::DescriptorPool>(descriptorPool);
    if (pool_state && !(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT & pool_state->createInfo.flags)) {
        // Can't Free from a NON_FREE pool
        skip |= LogError("VUID-vkFreeDescriptorSets-descriptorPool-00312", descriptorPool,
                         error_obj.location.dot(Field::descriptorPool), "with a pool created with %s.",
                         string_VkDescriptorPoolCreateFlags(pool_state->createInfo.flags).c_str());
    }
    return skip;
}

bool CoreChecks::PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                     const VkWriteDescriptorSet *pDescriptorWrites, uint32_t descriptorCopyCount,
                                                     const VkCopyDescriptorSet *pDescriptorCopies,
                                                     const ErrorObject &error_obj) const {
    // First thing to do is perform map look-ups.
    // NOTE : UpdateDescriptorSets is somewhat unique in that it's operating on a number of DescriptorSets
    //  so we can't just do a single map look-up up-front, but do them individually in functions below

    // Now make call(s) that validate state, but don't perform state updates in this function
    // Note, here DescriptorSets is unique in that we don't yet have an instance. Using a helper function in the
    //  namespace which will parse params and make calls into specific class instances
    return ValidateUpdateDescriptorSets(descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies,
                                        error_obj.location);
}

bool CoreChecks::ValidateCmdPushDescriptorSet(const vvl::CommandBuffer &cb_state, VkPipelineLayout layout, uint32_t set,
                                              uint32_t descriptorWriteCount, const VkWriteDescriptorSet *pDescriptorWrites,
                                              const Location &loc) const {
    bool skip = false;
    const bool is_2 = loc.function != Func::vkCmdPushDescriptorSetKHR;

    auto layout_data = Get<vvl::PipelineLayout>(layout);
    if (!layout_data) {
        return skip;  // dynamicPipelineLayout
    }

    // Validate the set index points to a push descriptor set and is in range
    const LogObjectList objlist(cb_state.commandBuffer(), layout);
    const auto &set_layouts = layout_data->set_layouts;
    if (set < set_layouts.size()) {
        const auto &dsl = set_layouts[set];
        if (dsl) {
            if (!dsl->IsPushDescriptor()) {
                const char *vuid = is_2 ? "VUID-VkPushDescriptorSetInfoKHR-set-00365" : "VUID-vkCmdPushDescriptorSetKHR-set-00365";
                skip = LogError(vuid, objlist, loc, "Set index %" PRIu32 " does not match push descriptor set layout index for %s.",
                                set, FormatHandle(layout).c_str());
            } else {
                // Create an empty proxy in order to use the existing descriptor set update validation
                // TODO move the validation (like this) that doesn't need descriptor set state to the DSL object so we
                // don't have to do this. Note we need to const_cast<>(this) because GPU-AV needs a non-const version of
                // the state tracker. The proxy here could get away with const.
                vvl::DescriptorSet proxy_ds(VK_NULL_HANDLE, nullptr, dsl, 0, const_cast<CoreChecks *>(this));
                skip |= ValidatePushDescriptorsUpdate(proxy_ds, descriptorWriteCount, pDescriptorWrites, loc);
            }
        }
    } else {
        const char *vuid = is_2 ? "VUID-VkPushDescriptorSetInfoKHR-set-00364" : "VUID-vkCmdPushDescriptorSetKHR-set-00364";
        skip = LogError(vuid, objlist, loc, "Set index %" PRIu32 " is outside of range for %s (set < %" PRIu32 ").", set,
                        FormatHandle(layout).c_str(), static_cast<uint32_t>(set_layouts.size()));
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                        VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                        const VkWriteDescriptorSet *pDescriptorWrites,
                                                        const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);
    skip |= ValidatePipelineBindPoint(cb_state.get(), pipelineBindPoint, error_obj.location);
    skip |= ValidateCmdPushDescriptorSet(*cb_state, layout, set, descriptorWriteCount, pDescriptorWrites, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer,
                                                         const VkPushDescriptorSetInfoKHR *pPushDescriptorSetInfo,
                                                         const ErrorObject &error_obj) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);

    skip |= ValidateCmdPushDescriptorSet(*cb_state, pPushDescriptorSetInfo->layout, pPushDescriptorSetInfo->set,
                                         pPushDescriptorSetInfo->descriptorWriteCount, pPushDescriptorSetInfo->pDescriptorWrites,
                                         error_obj.location);
    if (!enabled_features.dynamicPipelineLayout && pPushDescriptorSetInfo->layout == VK_NULL_HANDLE) {
        skip |= LogError("VUID-VkPushDescriptorSetInfoKHR-None-09495", device,
                         error_obj.location.dot(Field::pPushDescriptorSetInfo).dot(Field::layout), "is not valid.");
    }

    if (IsStageInPipelineBindPoint(pPushDescriptorSetInfo->stageFlags, VK_PIPELINE_BIND_POINT_GRAPHICS)) {
        skip |= ValidatePipelineBindPoint(cb_state.get(), VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    }
    if (IsStageInPipelineBindPoint(pPushDescriptorSetInfo->stageFlags, VK_PIPELINE_BIND_POINT_COMPUTE)) {
        skip |= ValidatePipelineBindPoint(cb_state.get(), VK_PIPELINE_BIND_POINT_COMPUTE, error_obj.location);
    }
    if (IsStageInPipelineBindPoint(pPushDescriptorSetInfo->stageFlags, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR)) {
        skip |= ValidatePipelineBindPoint(cb_state.get(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, error_obj.location);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateDescriptorUpdateTemplate(VkDevice device,
                                                               const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator,
                                                               VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate,
                                                               const ErrorObject &error_obj) const {
    bool skip = false;
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    auto layout = Get<vvl::DescriptorSetLayout>(pCreateInfo->descriptorSetLayout);
    if (VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET == pCreateInfo->templateType && !layout) {
        skip |= LogError("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00350", pCreateInfo->descriptorSetLayout,
                         create_info_loc.dot(Field::descriptorSetLayout), "(%s) is invalid.",
                         FormatHandle(pCreateInfo->descriptorSetLayout).c_str());
    } else if (VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR == pCreateInfo->templateType) {
        auto bind_point = pCreateInfo->pipelineBindPoint;
        const bool valid_bp = (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) || (bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) ||
                              (bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
        if (!valid_bp) {
            skip |= LogError("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00351", device,
                             create_info_loc.dot(Field::pipelineBindPoint), "is %s.", string_VkPipelineBindPoint(bind_point));
        }
        auto pipeline_layout = Get<vvl::PipelineLayout>(pCreateInfo->pipelineLayout);
        if (!pipeline_layout) {
            skip |= LogError("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00352", pCreateInfo->pipelineLayout,
                             create_info_loc.dot(Field::pipelineLayout), "(%s) is invalid.",
                             FormatHandle(pCreateInfo->pipelineLayout).c_str());
        } else {
            const uint32_t pd_set = pCreateInfo->set;
            if ((pd_set >= pipeline_layout->set_layouts.size()) || !pipeline_layout->set_layouts[pd_set] ||
                !pipeline_layout->set_layouts[pd_set]->IsPushDescriptor()) {
                skip |=
                    LogError("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00353", pCreateInfo->pipelineLayout,
                             create_info_loc.dot(Field::set),
                             "(%" PRIu32 ") does not refer to the push descriptor set layout for pCreateInfo->pipelineLayout (%s).",
                             pd_set, FormatHandle(pCreateInfo->pipelineLayout).c_str());
            }
        }
    } else if (VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET == pCreateInfo->templateType) {
        for (const auto &binding : layout->GetBindings()) {
            if (binding.descriptorType == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
                skip |= LogError(
                    "VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-04615", device,
                    create_info_loc.dot(Field::templateType),
                    "is VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET, but "
                    "pCreateInfo->descriptorSetLayout contains a binding with descriptor type VK_DESCRIPTOR_TYPE_MUTABLE_EXT.");
            }
        }
    }
    for (uint32_t i = 0; i < pCreateInfo->descriptorUpdateEntryCount; ++i) {
        const auto &descriptor_update = pCreateInfo->pDescriptorUpdateEntries[i];
        if (descriptor_update.descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT) {
            if (descriptor_update.dstArrayElement & 3) {
                skip |= LogError("VUID-VkDescriptorUpdateTemplateEntry-descriptor-02226", pCreateInfo->pipelineLayout,
                                 create_info_loc.dot(Field::pDescriptorUpdateEntries, i),
                                 "has descriptorType VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT, but dstArrayElement (%" PRIu32
                                 ") is not a "
                                 "multiple of 4).",
                                 descriptor_update.dstArrayElement);
            }
            if (descriptor_update.descriptorCount & 3) {
                skip |= LogError("VUID-VkDescriptorUpdateTemplateEntry-descriptor-02227", pCreateInfo->pipelineLayout,
                                 create_info_loc.dot(Field::pDescriptorUpdateEntries, i),
                                 "has descriptorType VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT, but descriptorCount (%" PRIu32
                                 ") is not a "
                                 "multiple of 4).",
                                 descriptor_update.descriptorCount);
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                                  const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
                                                                  const VkAllocationCallbacks *pAllocator,
                                                                  VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate,
                                                                  const ErrorObject &error_obj) const {
    return PreCallValidateCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate, error_obj);
}

bool CoreChecks::PreCallValidateUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                                VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                const void *pData, const ErrorObject &error_obj) const {
    bool skip = false;
    auto template_state = Get<vvl::DescriptorUpdateTemplate>(descriptorUpdateTemplate);
    // Object tracker will report errors for invalid descriptorUpdateTemplate values, avoiding a crash in release builds
    // but retaining the assert as template support is new enough to want to investigate these in debug builds.
    assert(template_state);
    // TODO: Validate template push descriptor updates
    if (template_state->create_info.templateType == VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET) {
        // decode the templatized data and leverage the non-template UpdateDescriptor helper functions.
        // Translate the templated update into a normal update for validation...
        vvl::DecodedTemplateUpdate decoded_update(this, descriptorSet, template_state.get(), pData);
        return ValidateUpdateDescriptorSets(static_cast<uint32_t>(decoded_update.desc_writes.size()), decoded_update.desc_writes.data(),
                                            0, nullptr, error_obj.location);
    }
    return skip;
}

bool CoreChecks::PreCallValidateUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                                   VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                   const void *pData, const ErrorObject &error_obj) const {
    return PreCallValidateUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData, error_obj);
}

bool CoreChecks::ValidateCmdPushDescriptorSetWithTemplate(VkCommandBuffer commandBuffer,
                                                          VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                          VkPipelineLayout layout, uint32_t set, const void *pData,
                                                          const Location &loc) const {
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    bool skip = false;
    skip |= ValidateCmd(*cb_state, loc);

    const bool is_2 = loc.function != Func::vkCmdPushDescriptorSetWithTemplateKHR;
    auto layout_data = Get<vvl::PipelineLayout>(layout);
    const auto dsl = layout_data ? layout_data->GetDsl(set) : nullptr;
    // Validate the set index points to a push descriptor set and is in range
    if (dsl) {
        if (!dsl->IsPushDescriptor()) {
            const char *vuid = is_2 ? "VUID-VkPushDescriptorSetWithTemplateInfoKHR-set-07305"
                                    : "VUID-vkCmdPushDescriptorSetWithTemplateKHR-set-07305";
            skip = LogError(vuid, layout, loc, "Set index %" PRIu32 " does not match push descriptor set layout index for %s.", set,
                            FormatHandle(layout).c_str());
        }
    } else if (layout_data && (set >= layout_data->set_layouts.size())) {
        const char *vuid =
            is_2 ? "VUID-VkPushDescriptorSetWithTemplateInfoKHR-set-07304" : "VUID-vkCmdPushDescriptorSetWithTemplateKHR-set-07304";
        skip = LogError(vuid, layout, loc, "Set index %" PRIu32 " is outside of range for %s (set < %" PRIu32 ").", set,
                        FormatHandle(layout).c_str(), static_cast<uint32_t>(layout_data->set_layouts.size()));
    }

    auto template_state = Get<vvl::DescriptorUpdateTemplate>(descriptorUpdateTemplate);
    if (template_state) {
        const auto &template_ci = template_state->create_info;

        skip |= ValidatePipelineBindPoint(cb_state.get(), template_ci.pipelineBindPoint, loc);

        if (template_ci.templateType != VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR) {
            const char *vuid = is_2 ? "VUID-VkPushDescriptorSetWithTemplateInfoKHR-descriptorUpdateTemplate-07994"
                                    : "VUID-vkCmdPushDescriptorSetWithTemplateKHR-descriptorUpdateTemplate-07994";
            skip |= LogError(vuid, commandBuffer, loc.dot(Field::descriptorUpdateTemplate),
                             "%s was not created with flag "
                             "VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR.",
                             FormatHandle(descriptorUpdateTemplate).c_str());
        }
        if (template_ci.set != set) {
            const char *vuid = is_2 ? "VUID-VkPushDescriptorSetWithTemplateInfoKHR-set-07995"
                                    : "VUID-vkCmdPushDescriptorSetWithTemplateKHR-set-07995";
            skip |= LogError(vuid, commandBuffer, loc.dot(Field::descriptorUpdateTemplate),
                             "%s created with set %" PRIu32 " does not match command parameter set %" PRIu32 ".",
                             FormatHandle(descriptorUpdateTemplate).c_str(), template_ci.set, set);
        }
        auto template_layout = Get<vvl::PipelineLayout>(template_ci.pipelineLayout);
        if (!IsPipelineLayoutSetCompat(set, layout_data.get(), template_layout.get())) {
            const LogObjectList objlist(commandBuffer, descriptorUpdateTemplate, template_ci.pipelineLayout, layout);
            const char *vuid = is_2 ? "VUID-VkPushDescriptorSetWithTemplateInfoKHR-layout-07993"
                                    : "VUID-vkCmdPushDescriptorSetWithTemplateKHR-layout-07993";
            skip |= LogError(vuid, objlist, loc.dot(Field::descriptorUpdateTemplate),
                             "%s created with %s is incompatible "
                             "with command parameter "
                             "%s for set %" PRIu32,
                             FormatHandle(descriptorUpdateTemplate).c_str(), FormatHandle(template_ci.pipelineLayout).c_str(),
                             FormatHandle(layout).c_str(), set);
        }
    }

    if (dsl && template_state) {
        if (!Get<vvl::DescriptorSetLayout>(dsl->VkHandle())) {
            const LogObjectList objlist(commandBuffer, descriptorUpdateTemplate, layout);
            const char *vuid = is_2 ? "VUID-VkPushDescriptorSetWithTemplateInfoKHR-pData-01686"
                                    : "VUID-vkCmdPushDescriptorSetWithTemplateKHR-pData-01686";
            skip |= LogError(vuid, objlist, loc.dot(Field::pData),
                             "does not point to a valid layout, it possible the "
                             "VkDescriptorUpdateTemplateCreateInfo::descriptorSetLayout was accidentally destroy.");
        } else {
            // Create an empty proxy in order to use the existing descriptor set update validation
            vvl::DescriptorSet proxy_ds(VK_NULL_HANDLE, nullptr, dsl, 0, const_cast<CoreChecks *>(this));
            // Decode the template into a set of write updates
            vvl::DecodedTemplateUpdate decoded_template(this, VK_NULL_HANDLE, template_state.get(), pData,
                                                                    dsl->VkHandle());
            // Validate the decoded update against the proxy_ds
            skip |= ValidatePushDescriptorsUpdate(proxy_ds, static_cast<uint32_t>(decoded_template.desc_writes.size()),
                                                  decoded_template.desc_writes.data(), loc);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                                    VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                    VkPipelineLayout layout, uint32_t set, const void *pData,
                                                                    const ErrorObject &error_obj) const {
    return ValidateCmdPushDescriptorSetWithTemplate(commandBuffer, descriptorUpdateTemplate, layout, set, pData,
                                                    error_obj.location);
}

bool CoreChecks::PreCallValidateCmdPushDescriptorSetWithTemplate2KHR(
    VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfoKHR *pPushDescriptorSetWithTemplateInfo,
    const ErrorObject &error_obj) const {
    bool skip = false;
    skip |= ValidateCmdPushDescriptorSetWithTemplate(
        commandBuffer, pPushDescriptorSetWithTemplateInfo->descriptorUpdateTemplate, pPushDescriptorSetWithTemplateInfo->layout,
        pPushDescriptorSetWithTemplateInfo->set, pPushDescriptorSetWithTemplateInfo->pData, error_obj.location);
    if (!enabled_features.dynamicPipelineLayout && pPushDescriptorSetWithTemplateInfo->layout == VK_NULL_HANDLE) {
        skip |= LogError("VUID-VkPushDescriptorSetWithTemplateInfoKHR-None-09495", device,
                         error_obj.location.dot(Field::pPushDescriptorSetWithTemplateInfo).dot(Field::layout), "is not valid.");
    }
    return skip;
}

enum DSL_DESCRIPTOR_GROUPS {
    DSL_TYPE_SAMPLERS = 0,
    DSL_TYPE_UNIFORM_BUFFERS,
    DSL_TYPE_STORAGE_BUFFERS,
    DSL_TYPE_SAMPLED_IMAGES,
    DSL_TYPE_STORAGE_IMAGES,
    DSL_TYPE_INPUT_ATTACHMENTS,
    DSL_TYPE_INLINE_UNIFORM_BLOCK,
    DSL_TYPE_ACCELERATION_STRUCTURE,
    DSL_TYPE_ACCELERATION_STRUCTURE_NV,
    DSL_NUM_DESCRIPTOR_GROUPS
};

// Used by PreCallValidateCreatePipelineLayout.
// Returns an array of size DSL_NUM_DESCRIPTOR_GROUPS of the maximum number of descriptors used in any single pipeline stage
std::valarray<uint32_t> GetDescriptorCountMaxPerStage(
    const DeviceFeatures *enabled_features,
    const std::vector<std::shared_ptr<vvl::DescriptorSetLayout const>> &set_layouts, bool skip_update_after_bind) {
    // Identify active pipeline stages
    std::vector<VkShaderStageFlags> stage_flags = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                   VK_SHADER_STAGE_COMPUTE_BIT};
    if (enabled_features->geometryShader) {
        stage_flags.push_back(VK_SHADER_STAGE_GEOMETRY_BIT);
    }
    if (enabled_features->tessellationShader) {
        stage_flags.push_back(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
        stage_flags.push_back(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    }
    if (enabled_features->rayTracingPipeline) {
        stage_flags.push_back(VK_SHADER_STAGE_RAYGEN_BIT_KHR);
        stage_flags.push_back(VK_SHADER_STAGE_ANY_HIT_BIT_KHR);
        stage_flags.push_back(VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
        stage_flags.push_back(VK_SHADER_STAGE_MISS_BIT_KHR);
        stage_flags.push_back(VK_SHADER_STAGE_INTERSECTION_BIT_KHR);
        stage_flags.push_back(VK_SHADER_STAGE_CALLABLE_BIT_KHR);
    }

    // Allow iteration over enum values
    std::vector<DSL_DESCRIPTOR_GROUPS> dsl_groups = {
        DSL_TYPE_SAMPLERS,
        DSL_TYPE_UNIFORM_BUFFERS,
        DSL_TYPE_STORAGE_BUFFERS,
        DSL_TYPE_SAMPLED_IMAGES,
        DSL_TYPE_STORAGE_IMAGES,
        DSL_TYPE_INPUT_ATTACHMENTS,
        DSL_TYPE_INLINE_UNIFORM_BLOCK,
        DSL_TYPE_ACCELERATION_STRUCTURE,
        DSL_TYPE_ACCELERATION_STRUCTURE_NV,
    };

    // Sum by layouts per stage, then pick max of stages per type
    std::valarray<uint32_t> max_sum(0U, DSL_NUM_DESCRIPTOR_GROUPS);  // max descriptor sum among all pipeline stages
    for (auto stage : stage_flags) {
        std::valarray<uint32_t> stage_sum(0U, DSL_NUM_DESCRIPTOR_GROUPS);  // per-stage sums
        for (const auto &dsl : set_layouts) {
            if (!dsl) {
                continue;
            }
            if (skip_update_after_bind && (dsl->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)) {
                continue;
            }

            for (uint32_t binding_idx = 0; binding_idx < dsl->GetBindingCount(); binding_idx++) {
                const VkDescriptorSetLayoutBinding *binding = dsl->GetDescriptorSetLayoutBindingPtrFromIndex(binding_idx);
                // Bindings with a descriptorCount of 0 are "reserved" and should be skipped
                if (0 != (stage & binding->stageFlags) && binding->descriptorCount > 0) {
                    switch (binding->descriptorType) {
                        case VK_DESCRIPTOR_TYPE_SAMPLER:
                            stage_sum[DSL_TYPE_SAMPLERS] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                            stage_sum[DSL_TYPE_UNIFORM_BUFFERS] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                            stage_sum[DSL_TYPE_STORAGE_BUFFERS] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                        case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:
                        case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM:
                            stage_sum[DSL_TYPE_SAMPLED_IMAGES] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                            stage_sum[DSL_TYPE_STORAGE_IMAGES] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                            stage_sum[DSL_TYPE_SAMPLED_IMAGES] += binding->descriptorCount;
                            stage_sum[DSL_TYPE_SAMPLERS] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                            stage_sum[DSL_TYPE_INPUT_ATTACHMENTS] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT:
                            // count one block per binding. descriptorCount is number of bytes
                            stage_sum[DSL_TYPE_INLINE_UNIFORM_BLOCK]++;
                            break;
                        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
                            stage_sum[DSL_TYPE_ACCELERATION_STRUCTURE] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
                            stage_sum[DSL_TYPE_ACCELERATION_STRUCTURE_NV] += binding->descriptorCount;
                            break;
                        default:
                            break;
                    }
                }
            }
        }
        for (auto type : dsl_groups) {
            max_sum[type] = std::max(stage_sum[type], max_sum[type]);
        }
    }
    return max_sum;
}

// Used by PreCallValidateCreatePipelineLayout.
// Returns a map indexed by VK_DESCRIPTOR_TYPE_* enum of the summed descriptors by type.
// Note: descriptors only count against the limit once even if used by multiple stages.
std::map<uint32_t, uint32_t> GetDescriptorSum(
    const std::vector<std::shared_ptr<vvl::DescriptorSetLayout const>> &set_layouts, bool skip_update_after_bind) {
    std::map<uint32_t, uint32_t> sum_by_type;
    for (const auto &dsl : set_layouts) {
        if (!dsl) {
            continue;
        }
        if (skip_update_after_bind && (dsl->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)) {
            continue;
        }

        for (uint32_t binding_idx = 0; binding_idx < dsl->GetBindingCount(); binding_idx++) {
            const VkDescriptorSetLayoutBinding *binding = dsl->GetDescriptorSetLayoutBindingPtrFromIndex(binding_idx);
            // Bindings with a descriptorCount of 0 are "reserved" and should be skipped
            if (binding->descriptorCount > 0) {
                sum_by_type[binding->descriptorType] += binding->descriptorCount;
            }
        }
    }
    return sum_by_type;
}

uint32_t GetInlineUniformBlockBindingCount(
    const std::vector<std::shared_ptr<vvl::DescriptorSetLayout const>> &set_layouts, bool skip_update_after_bind) {
    uint32_t sum = 0;
    for (const auto &dsl : set_layouts) {
        if (!dsl) {
            continue;
        }
        if (skip_update_after_bind && (dsl->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)) {
            continue;
        }

        for (uint32_t binding_idx = 0; binding_idx < dsl->GetBindingCount(); binding_idx++) {
            const VkDescriptorSetLayoutBinding *binding = dsl->GetDescriptorSetLayoutBindingPtrFromIndex(binding_idx);
            // Bindings with a descriptorCount of 0 are "reserved" and should be skipped
            if (binding->descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT && binding->descriptorCount > 0) {
                ++sum;
            }
        }
    }
    return sum;
}

bool CoreChecks::PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout,
                                                     const ErrorObject &error_obj) const {
    bool skip = false;

    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    std::vector<std::shared_ptr<vvl::DescriptorSetLayout const>> set_layouts(pCreateInfo->setLayoutCount, nullptr);
    uint32_t descriptor_buffer_set_count = 0;
    uint32_t valid_set_count = 0;
    uint32_t push_descriptor_set_found = pCreateInfo->setLayoutCount;
    for (uint32_t i = 0; i < pCreateInfo->setLayoutCount; ++i) {
        set_layouts[i] = Get<vvl::DescriptorSetLayout>(pCreateInfo->pSetLayouts[i]);
        if (!set_layouts[i]) {
            continue;
        }
        if (set_layouts[i]->IsPushDescriptor()) {
            if (push_descriptor_set_found < pCreateInfo->setLayoutCount) {
                skip |= LogError("VUID-VkPipelineLayoutCreateInfo-pSetLayouts-00293", set_layouts[i]->VkHandle(),
                                 create_info_loc.dot(Field::pSetLayouts, i),
                                 "and pSetLayouts[%" PRIu32 "] both have push descriptor sets.", push_descriptor_set_found);
            }
            push_descriptor_set_found = i;
        }
        if (set_layouts[i]->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-pSetLayouts-04606", set_layouts[i]->VkHandle(),
                             create_info_loc.dot(Field::pSetLayouts, i),
                             "was created with VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT bit.");
        }
        ++valid_set_count;
        if (set_layouts[i]->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) {
            ++descriptor_buffer_set_count;
        }
    }

    if ((descriptor_buffer_set_count != 0) && (valid_set_count != descriptor_buffer_set_count)) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-pSetLayouts-08008", device, error_obj.location,
                         "All sets must be created with "
                         "VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT or none of them.");
    }

    // Max descriptors by type, within a single pipeline stage
    std::valarray<uint32_t> max_descriptors_per_stage = GetDescriptorCountMaxPerStage(&enabled_features, set_layouts, true);
    // Samplers
    if (max_descriptors_per_stage[DSL_TYPE_SAMPLERS] > phys_dev_props.limits.maxPerStageDescriptorSamplers) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03016", device, error_obj.location,
                         "max per-stage sampler bindings count (%" PRIu32
                         ") exceeds device "
                         "maxPerStageDescriptorSamplers limit (%" PRIu32 ").",
                         max_descriptors_per_stage[DSL_TYPE_SAMPLERS], phys_dev_props.limits.maxPerStageDescriptorSamplers);
    }

    // Uniform buffers
    if (max_descriptors_per_stage[DSL_TYPE_UNIFORM_BUFFERS] > phys_dev_props.limits.maxPerStageDescriptorUniformBuffers) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03017", device, error_obj.location,
                         "max per-stage uniform buffer bindings count (%" PRIu32
                         ") exceeds device "
                         "maxPerStageDescriptorUniformBuffers limit (%" PRIu32 ").",
                         max_descriptors_per_stage[DSL_TYPE_UNIFORM_BUFFERS],
                         phys_dev_props.limits.maxPerStageDescriptorUniformBuffers);
    }

    // Storage buffers
    if (max_descriptors_per_stage[DSL_TYPE_STORAGE_BUFFERS] > phys_dev_props.limits.maxPerStageDescriptorStorageBuffers) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03018", device, error_obj.location,
                         "max per-stage storage buffer bindings count (%" PRIu32
                         ") exceeds device "
                         "maxPerStageDescriptorStorageBuffers limit (%" PRIu32 ").",
                         max_descriptors_per_stage[DSL_TYPE_STORAGE_BUFFERS],
                         phys_dev_props.limits.maxPerStageDescriptorStorageBuffers);
    }

    // Sampled images
    if (max_descriptors_per_stage[DSL_TYPE_SAMPLED_IMAGES] > phys_dev_props.limits.maxPerStageDescriptorSampledImages) {
        skip |=
            LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-06939", device, error_obj.location,
                     "max per-stage sampled image bindings count (%" PRIu32
                     ") exceeds device "
                     "maxPerStageDescriptorSampledImages limit (%" PRIu32 ").",
                     max_descriptors_per_stage[DSL_TYPE_SAMPLED_IMAGES], phys_dev_props.limits.maxPerStageDescriptorSampledImages);
    }

    // Storage images
    if (max_descriptors_per_stage[DSL_TYPE_STORAGE_IMAGES] > phys_dev_props.limits.maxPerStageDescriptorStorageImages) {
        skip |=
            LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03020", device, error_obj.location,
                     "max per-stage storage image bindings count (%" PRIu32
                     ") exceeds device "
                     "maxPerStageDescriptorStorageImages limit (%" PRIu32 ").",
                     max_descriptors_per_stage[DSL_TYPE_STORAGE_IMAGES], phys_dev_props.limits.maxPerStageDescriptorStorageImages);
    }

    // Input attachments
    if (max_descriptors_per_stage[DSL_TYPE_INPUT_ATTACHMENTS] > phys_dev_props.limits.maxPerStageDescriptorInputAttachments) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03021", device, error_obj.location,
                         "max per-stage input attachment bindings count (%" PRIu32
                         ") exceeds device "
                         "maxPerStageDescriptorInputAttachments limit (%" PRIu32 ").",
                         max_descriptors_per_stage[DSL_TYPE_INPUT_ATTACHMENTS],
                         phys_dev_props.limits.maxPerStageDescriptorInputAttachments);
    }

    // Inline uniform blocks
    if (max_descriptors_per_stage[DSL_TYPE_INLINE_UNIFORM_BLOCK] >
        phys_dev_ext_props.inline_uniform_block_props.maxPerStageDescriptorInlineUniformBlocks) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-02214", device, error_obj.location,
                         "max per-stage inline uniform block bindings count (%" PRIu32
                         ") exceeds device "
                         "maxPerStageDescriptorInlineUniformBlocks limit (%" PRIu32 ").",
                         max_descriptors_per_stage[DSL_TYPE_INLINE_UNIFORM_BLOCK],
                         phys_dev_ext_props.inline_uniform_block_props.maxPerStageDescriptorInlineUniformBlocks);
    }

    // Acceleration structures
    if (max_descriptors_per_stage[DSL_TYPE_ACCELERATION_STRUCTURE] >
        phys_dev_ext_props.acc_structure_props.maxPerStageDescriptorAccelerationStructures) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03571", device, error_obj.location,
                         "max per-stage acceleration structure bindings count (%" PRIu32
                         ") exceeds device "
                         "maxPerStageDescriptorAccelerationStructures limit (%" PRIu32 ").",
                         max_descriptors_per_stage[DSL_TYPE_ACCELERATION_STRUCTURE],
                         phys_dev_ext_props.acc_structure_props.maxPerStageDescriptorAccelerationStructures);
    }

    // Total descriptors by type
    std::map<uint32_t, uint32_t> sum_all_stages = GetDescriptorSum(set_layouts, true);
    const uint32_t inline_uniform_block_bindings_all_stages = GetInlineUniformBlockBindingCount(set_layouts, true);
    // Samplers
    uint32_t sum = sum_all_stages[VK_DESCRIPTOR_TYPE_SAMPLER] + sum_all_stages[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER];
    if (sum > phys_dev_props.limits.maxDescriptorSetSamplers) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03028", device, error_obj.location,
                         "sum of sampler bindings among all stages (%" PRIu32
                         ") exceeds device "
                         "maxDescriptorSetSamplers limit (%" PRIu32 ").",
                         sum, phys_dev_props.limits.maxDescriptorSetSamplers);
    }

    // Uniform buffers
    if (sum_all_stages[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] > phys_dev_props.limits.maxDescriptorSetUniformBuffers) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03029", device, error_obj.location,
                         "sum of uniform buffer bindings among all stages (%" PRIu32
                         ") exceeds device "
                         "maxDescriptorSetUniformBuffers limit (%" PRIu32 ").",
                         sum_all_stages[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER], phys_dev_props.limits.maxDescriptorSetUniformBuffers);
    }

    // Dynamic uniform buffers
    if (sum_all_stages[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC] > phys_dev_props.limits.maxDescriptorSetUniformBuffersDynamic) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03030", device, error_obj.location,
                         "sum of dynamic uniform buffer bindings among all stages (%" PRIu32
                         ") exceeds device "
                         "maxDescriptorSetUniformBuffersDynamic limit (%" PRIu32 ").",
                         sum_all_stages[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC],
                         phys_dev_props.limits.maxDescriptorSetUniformBuffersDynamic);
    }

    // Storage buffers
    if (sum_all_stages[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER] > phys_dev_props.limits.maxDescriptorSetStorageBuffers) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03031", device, error_obj.location,
                         "sum of storage buffer bindings among all stages (%" PRIu32
                         ") exceeds device "
                         "maxDescriptorSetStorageBuffers limit (%" PRIu32 ").",
                         sum_all_stages[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER], phys_dev_props.limits.maxDescriptorSetStorageBuffers);
    }

    // Dynamic storage buffers
    if (sum_all_stages[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC] > phys_dev_props.limits.maxDescriptorSetStorageBuffersDynamic) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03032", device, error_obj.location,
                         "sum of dynamic storage buffer bindings among all stages (%" PRIu32
                         ") exceeds device "
                         "maxDescriptorSetStorageBuffersDynamic limit (%" PRIu32 ").",
                         sum_all_stages[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC],
                         phys_dev_props.limits.maxDescriptorSetStorageBuffersDynamic);
    }

    // Sampled images
    sum = sum_all_stages[VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE] + sum_all_stages[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] +
          sum_all_stages[VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER];
    if (sum > phys_dev_props.limits.maxDescriptorSetSampledImages) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03033", device, error_obj.location,
                         "sum of sampled image bindings among all stages (%" PRIu32
                         ") exceeds device "
                         "maxDescriptorSetSampledImages limit (%" PRIu32 ").",
                         sum, phys_dev_props.limits.maxDescriptorSetSampledImages);
    }

    // Storage images
    sum = sum_all_stages[VK_DESCRIPTOR_TYPE_STORAGE_IMAGE] + sum_all_stages[VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER];
    if (sum > phys_dev_props.limits.maxDescriptorSetStorageImages) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03034", device, error_obj.location,
                         "sum of storage image bindings among all stages (%" PRIu32
                         ") exceeds device "
                         "maxDescriptorSetStorageImages limit (%" PRIu32 ").",
                         sum, phys_dev_props.limits.maxDescriptorSetStorageImages);
    }

    // Input attachments
    if (sum_all_stages[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT] > phys_dev_props.limits.maxDescriptorSetInputAttachments) {
        skip |=
            LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03035", device, error_obj.location,
                     "sum of input attachment bindings among all stages (%" PRIu32
                     ") exceeds device "
                     "maxDescriptorSetInputAttachments limit (%" PRIu32 ").",
                     sum_all_stages[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT], phys_dev_props.limits.maxDescriptorSetInputAttachments);
    }

    // Inline uniform blocks
    if (inline_uniform_block_bindings_all_stages >
        phys_dev_ext_props.inline_uniform_block_props.maxDescriptorSetInlineUniformBlocks) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-02216", device, error_obj.location,
                         "sum of inline uniform block bindings among all stages (%" PRIu32
                         ") exceeds device "
                         "maxDescriptorSetInlineUniformBlocks limit (%" PRIu32 ").",
                         inline_uniform_block_bindings_all_stages,
                         phys_dev_ext_props.inline_uniform_block_props.maxDescriptorSetInlineUniformBlocks);
    }
    if (api_version >= VK_API_VERSION_1_3 &&
        sum_all_stages[VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT] > phys_dev_props_core13.maxInlineUniformTotalSize) {
        skip |=
            LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-06531", device, error_obj.location,
                     "sum of inline uniform block bytes among all stages (%" PRIu32
                     ") exceeds device "
                     "maxInlineUniformTotalSize limit (%" PRIu32 ").",
                     sum_all_stages[VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT], phys_dev_props_core13.maxInlineUniformTotalSize);
    }

    // Acceleration structures
    if (sum_all_stages[VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR] >
        phys_dev_ext_props.acc_structure_props.maxDescriptorSetAccelerationStructures) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03573", device, error_obj.location,
                         "sum of acceleration structures bindings among all stages (%" PRIu32
                         ") exceeds device "
                         "maxDescriptorSetAccelerationStructures limit (%" PRIu32 ").",
                         sum_all_stages[VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR],
                         phys_dev_ext_props.acc_structure_props.maxDescriptorSetAccelerationStructures);
    }

    // Acceleration structures NV
    if (sum_all_stages[VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV] >
        phys_dev_ext_props.ray_tracing_props_nv.maxDescriptorSetAccelerationStructures) {
        skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-02381", device, error_obj.location,
                         "sum of acceleration structures NV bindings among all stages (%" PRIu32
                         ") exceeds device "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxDescriptorSetAccelerationStructures limit (%" PRIu32 ").",
                         sum_all_stages[VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV],
                         phys_dev_ext_props.ray_tracing_props_nv.maxDescriptorSetAccelerationStructures);
    }

    // Extension exposes new properties limits
    if (IsExtEnabled(device_extensions.vk_ext_descriptor_indexing)) {
        // Max descriptors by type, within a single pipeline stage
        std::valarray<uint32_t> max_descriptors_per_stage_update_after_bind =
            GetDescriptorCountMaxPerStage(&enabled_features, set_layouts, false);
        // Samplers
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_SAMPLERS] >
            phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindSamplers) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03022", device, error_obj.location,
                             "max per-stage sampler bindings count (%" PRIu32
                             ") exceeds device "
                             "maxPerStageDescriptorUpdateAfterBindSamplers limit (%" PRIu32 ").",
                             max_descriptors_per_stage_update_after_bind[DSL_TYPE_SAMPLERS],
                             phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindSamplers);
        }

        // Uniform buffers
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_UNIFORM_BUFFERS] >
            phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindUniformBuffers) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03023", device, error_obj.location,
                             "max per-stage uniform buffer bindings count (%" PRIu32
                             ") exceeds device "
                             "maxPerStageDescriptorUpdateAfterBindUniformBuffers limit (%" PRIu32 ").",
                             max_descriptors_per_stage_update_after_bind[DSL_TYPE_UNIFORM_BUFFERS],
                             phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindUniformBuffers);
        }

        // Storage buffers
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_STORAGE_BUFFERS] >
            phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindStorageBuffers) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03024", device, error_obj.location,
                             "max per-stage storage buffer bindings count (%" PRIu32
                             ") exceeds device "
                             "maxPerStageDescriptorUpdateAfterBindStorageBuffers limit (%" PRIu32 ").",
                             max_descriptors_per_stage_update_after_bind[DSL_TYPE_STORAGE_BUFFERS],
                             phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindStorageBuffers);
        }

        // Sampled images
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_SAMPLED_IMAGES] >
            phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindSampledImages) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03025", device, error_obj.location,
                             "max per-stage sampled image bindings count (%" PRIu32
                             ") exceeds device "
                             "maxPerStageDescriptorUpdateAfterBindSampledImages limit (%" PRIu32 ").",
                             max_descriptors_per_stage_update_after_bind[DSL_TYPE_SAMPLED_IMAGES],
                             phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindSampledImages);
        }

        // Storage images
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_STORAGE_IMAGES] >
            phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindStorageImages) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03026", device, error_obj.location,
                             "max per-stage storage image bindings count (%" PRIu32
                             ") exceeds device "
                             "maxPerStageDescriptorUpdateAfterBindStorageImages limit (%" PRIu32 ").",
                             max_descriptors_per_stage_update_after_bind[DSL_TYPE_STORAGE_IMAGES],
                             phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindStorageImages);
        }

        // Input attachments
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_INPUT_ATTACHMENTS] >
            phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindInputAttachments) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03027", device, error_obj.location,
                             "max per-stage input attachment bindings count (%" PRIu32
                             ") exceeds device "
                             "maxPerStageDescriptorUpdateAfterBindInputAttachments limit (%" PRIu32 ").",
                             max_descriptors_per_stage_update_after_bind[DSL_TYPE_INPUT_ATTACHMENTS],
                             phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindInputAttachments);
        }

        // Inline uniform blocks
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_INLINE_UNIFORM_BLOCK] >
            phys_dev_ext_props.inline_uniform_block_props.maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-02215", device, error_obj.location,
                             "max per-stage inline uniform block bindings count (%" PRIu32
                             ") exceeds device "
                             "maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks limit (%" PRIu32 ").",
                             max_descriptors_per_stage_update_after_bind[DSL_TYPE_INLINE_UNIFORM_BLOCK],
                             phys_dev_ext_props.inline_uniform_block_props.maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks);
        }

        // Acceleration structures
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_ACCELERATION_STRUCTURE] >
            phys_dev_ext_props.acc_structure_props.maxPerStageDescriptorUpdateAfterBindAccelerationStructures) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03572", device, error_obj.location,
                             "max per-stage acceleration structure bindings count (%" PRIu32
                             ") exceeds device "
                             "maxPerStageDescriptorUpdateAfterBindAccelerationStructures limit (%" PRIu32 ").",
                             max_descriptors_per_stage_update_after_bind[DSL_TYPE_ACCELERATION_STRUCTURE],
                             phys_dev_ext_props.acc_structure_props.maxPerStageDescriptorUpdateAfterBindAccelerationStructures);
        }

        // Total descriptors by type, summed across all pipeline stages
        //
        std::map<uint32_t, uint32_t> sum_all_stages_update_after_bind = GetDescriptorSum(set_layouts, false);
        const uint32_t inline_uniform_block_bindings = GetInlineUniformBlockBindingCount(set_layouts, false);
        // Samplers
        sum = sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_SAMPLER] +
              sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER];
        if (sum > phys_dev_props_core12.maxDescriptorSetUpdateAfterBindSamplers) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03036", device, error_obj.location,
                             "sum of sampler bindings among all stages (%" PRIu32
                             ") exceeds device "
                             "maxDescriptorSetUpdateAfterBindSamplers limit (%" PRIu32 ").",
                             sum, phys_dev_props_core12.maxDescriptorSetUpdateAfterBindSamplers);
        }

        // Uniform buffers
        if (sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] >
            phys_dev_props_core12.maxDescriptorSetUpdateAfterBindUniformBuffers) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03037", device, error_obj.location,
                             "sum of uniform buffer bindings among all stages (%" PRIu32
                             ") exceeds device "
                             "maxDescriptorSetUpdateAfterBindUniformBuffers limit (%" PRIu32 ").",
                             sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER],
                             phys_dev_props_core12.maxDescriptorSetUpdateAfterBindUniformBuffers);
        }

        // Dynamic uniform buffers
        if (sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC] >
            phys_dev_props_core12.maxDescriptorSetUpdateAfterBindUniformBuffersDynamic) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03038", device, error_obj.location,
                             "sum of dynamic uniform buffer bindings among all stages (%" PRIu32
                             ") exceeds device "
                             "maxDescriptorSetUpdateAfterBindUniformBuffersDynamic limit (%" PRIu32 ").",
                             sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC],
                             phys_dev_props_core12.maxDescriptorSetUpdateAfterBindUniformBuffersDynamic);
        }

        // Storage buffers
        if (sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER] >
            phys_dev_props_core12.maxDescriptorSetUpdateAfterBindStorageBuffers) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03039", device, error_obj.location,
                             "sum of storage buffer bindings among all stages (%" PRIu32
                             ") exceeds device "
                             "maxDescriptorSetUpdateAfterBindStorageBuffers limit (%" PRIu32 ").",
                             sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER],
                             phys_dev_props_core12.maxDescriptorSetUpdateAfterBindStorageBuffers);
        }

        // Dynamic storage buffers
        if (sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC] >
            phys_dev_props_core12.maxDescriptorSetUpdateAfterBindStorageBuffersDynamic) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03040", device, error_obj.location,
                             "sum of dynamic storage buffer bindings among all stages (%" PRIu32
                             ") exceeds device "
                             "maxDescriptorSetUpdateAfterBindStorageBuffersDynamic limit (%" PRIu32 ").",
                             sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC],
                             phys_dev_props_core12.maxDescriptorSetUpdateAfterBindStorageBuffersDynamic);
        }

        // Sampled images
        sum = sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE] +
              sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] +
              sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER];
        if (sum > phys_dev_props_core12.maxDescriptorSetUpdateAfterBindSampledImages) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03041", device, error_obj.location,
                             "sum of sampled image bindings among all stages (%" PRIu32
                             ") exceeds device "
                             "maxDescriptorSetUpdateAfterBindSampledImages limit (%" PRIu32 ").",
                             sum, phys_dev_props_core12.maxDescriptorSetUpdateAfterBindSampledImages);
        }

        // Storage images
        sum = sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_STORAGE_IMAGE] +
              sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER];
        if (sum > phys_dev_props_core12.maxDescriptorSetUpdateAfterBindStorageImages) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03042", device, error_obj.location,
                             "sum of storage image bindings among all stages (%" PRIu32
                             ") exceeds device "
                             "maxDescriptorSetUpdateAfterBindStorageImages limit (%" PRIu32 ").",
                             sum, phys_dev_props_core12.maxDescriptorSetUpdateAfterBindStorageImages);
        }

        // Input attachments
        if (sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT] >
            phys_dev_props_core12.maxDescriptorSetUpdateAfterBindInputAttachments) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03043", device, error_obj.location,
                             "sum of input attachment bindings among all stages (%" PRIu32
                             ") exceeds device "
                             "maxDescriptorSetUpdateAfterBindInputAttachments limit (%" PRIu32 ").",
                             sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT],
                             phys_dev_props_core12.maxDescriptorSetUpdateAfterBindInputAttachments);
        }

        // Inline uniform blocks
        if (inline_uniform_block_bindings >
            phys_dev_ext_props.inline_uniform_block_props.maxDescriptorSetUpdateAfterBindInlineUniformBlocks) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-02217", device, error_obj.location,
                             "sum of inline uniform block bindings among all stages (%" PRIu32
                             ") exceeds device "
                             "maxDescriptorSetUpdateAfterBindInlineUniformBlocks limit (%" PRIu32 ").",
                             inline_uniform_block_bindings,
                             phys_dev_ext_props.inline_uniform_block_props.maxDescriptorSetUpdateAfterBindInlineUniformBlocks);
        }

        // Acceleration structures
        if (sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR] >
            phys_dev_ext_props.acc_structure_props.maxDescriptorSetUpdateAfterBindAccelerationStructures) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-descriptorType-03574", device, error_obj.location,
                             "sum of acceleration structures bindings among all stages (%" PRIu32
                             ") exceeds device "
                             "maxDescriptorSetUpdateAfterBindAccelerationStructures limit (%" PRIu32 ").",
                             sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR],
                             phys_dev_ext_props.acc_structure_props.maxDescriptorSetUpdateAfterBindAccelerationStructures);
        }
    }

    // Extension exposes new properties limits
    if (IsExtEnabled(device_extensions.vk_ext_fragment_density_map2)) {
        uint32_t sum_subsampled_samplers = 0;
        for (const auto &dsl : set_layouts) {
            // find the number of subsampled samplers across all stages
            // NOTE: this does not use the GetDescriptorSum patter because it needs the Get<vvl::Sampler> method
            if ((dsl->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)) {
                continue;
            }
            for (uint32_t binding_idx = 0; binding_idx < dsl->GetBindingCount(); binding_idx++) {
                const VkDescriptorSetLayoutBinding *binding = dsl->GetDescriptorSetLayoutBindingPtrFromIndex(binding_idx);

                // Bindings with a descriptorCount of 0 are "reserved" and should be skipped
                if (binding->descriptorCount == 0) {
                    continue;
                }

                if (((binding->descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) ||
                     (binding->descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER)) &&
                    (binding->pImmutableSamplers != nullptr)) {
                    for (uint32_t sampler_idx = 0; sampler_idx < binding->descriptorCount; sampler_idx++) {
                        auto state = Get<vvl::Sampler>(binding->pImmutableSamplers[sampler_idx]);
                        if (state && (state->createInfo.flags & (VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT |
                                                                 VK_SAMPLER_CREATE_SUBSAMPLED_COARSE_RECONSTRUCTION_BIT_EXT))) {
                            sum_subsampled_samplers++;
                        }
                    }
                }
            }
        }
        if (sum_subsampled_samplers > phys_dev_ext_props.fragment_density_map2_props.maxDescriptorSetSubsampledSamplers) {
            skip |= LogError("VUID-VkPipelineLayoutCreateInfo-pImmutableSamplers-03566", device, error_obj.location,
                             "sum of sampler bindings with flags containing "
                             "VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT or "
                             "VK_SAMPLER_CREATE_SUBSAMPLED_COARSE_RECONSTRUCTION_BIT_EXT among all stages(% d) "
                             "exceeds device maxDescriptorSetSubsampledSamplers limit (%" PRIu32 ").",
                             sum_subsampled_samplers,
                             phys_dev_ext_props.fragment_density_map2_props.maxDescriptorSetSubsampledSamplers);
        }
    }

    if (!enabled_features.graphicsPipelineLibrary) {
        for (uint32_t i = 0; i < pCreateInfo->setLayoutCount; ++i) {
            if (!pCreateInfo->pSetLayouts[i]) {
                skip |= LogError("VUID-VkPipelineLayoutCreateInfo-graphicsPipelineLibrary-06753", device,
                                 create_info_loc.dot(Field::pSetLayouts, i),
                                 "is VK_NULL_HANDLE, but the graphicsPipelineLibrary feature is not enabled.");
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                          uint32_t offset, uint32_t size, const Location &loc) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    assert(cb_state);
    skip |= ValidateCmd(*cb_state, loc);

    // Check if pipeline_layout VkPushConstantRange(s) overlapping offset, size have stageFlags set for each stage in the command
    // stageFlags argument, *and* that the command stageFlags argument has bits set for the stageFlags in each overlapping range.
    if (skip) {
        return skip;
    }
    auto layout_state = Get<vvl::PipelineLayout>(layout);
    if (!layout_state) {
        return skip;  // dynamicPipelineLayout feature
    }

    const bool is_2 = loc.function != Func::vkCmdPushConstants;
    const auto &ranges = *layout_state->push_constant_ranges;
    VkShaderStageFlags found_stages = 0;
    for (const auto &range : ranges) {
        if ((offset >= range.offset) && (offset + size <= range.offset + range.size)) {
            VkShaderStageFlags matching_stages = range.stageFlags & stageFlags;
            if (matching_stages != range.stageFlags) {
                const char *vuid = is_2 ? "VUID-VkPushConstantsInfoKHR-offset-01796" : "VUID-vkCmdPushConstants-offset-01796";
                skip |= LogError(vuid, commandBuffer, loc,
                                 "stageFlags (%s, offset (%" PRIu32 "), and size (%" PRIu32
                                 "),  must contain all stages in overlapping VkPushConstantRange stageFlags (%s), offset (%" PRIu32
                                 "), and size (%" PRIu32 ") in %s.",
                                 string_VkShaderStageFlags(stageFlags).c_str(), offset, size,
                                 string_VkShaderStageFlags(range.stageFlags).c_str(), range.offset, range.size,
                                 FormatHandle(layout).c_str());
            }

            // Accumulate all stages we've found
            found_stages = matching_stages | found_stages;
        }
    }
    if (found_stages != stageFlags) {
        uint32_t missing_stages = ~found_stages & stageFlags;
        const char *vuid = is_2 ? "VUID-VkPushConstantsInfoKHR-offset-01795" : "VUID-vkCmdPushConstants-offset-01795";
        skip |=
            LogError(vuid, commandBuffer, loc,
                     "%s, VkPushConstantRange in %s overlapping offset = %" PRIu32 " and size = %" PRIu32 ", do not contain %s.",
                     string_VkShaderStageFlags(stageFlags).c_str(), FormatHandle(layout).c_str(), offset, size,
                     string_VkShaderStageFlags(missing_stages).c_str());
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                                                 VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void *pValues,
                                                 const ErrorObject &error_obj) const {
    return ValidateCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdPushConstants2KHR(VkCommandBuffer commandBuffer,
                                                     const VkPushConstantsInfoKHR *pPushConstantsInfo,
                                                     const ErrorObject &error_obj) const {
    bool skip = false;
    skip |= ValidateCmdPushConstants(commandBuffer, pPushConstantsInfo->layout, pPushConstantsInfo->stageFlags,
                                     pPushConstantsInfo->offset, pPushConstantsInfo->size,
                                     error_obj.location.dot(Field::pPushConstantsInfo));

    if (!enabled_features.dynamicPipelineLayout && pPushConstantsInfo->layout == VK_NULL_HANDLE) {
        skip |= LogError("VUID-VkPushConstantsInfoKHR-None-09495", device,
                         error_obj.location.dot(Field::pPushConstantsInfo).dot(Field::layout), "is not valid.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo,
                                              const VkAllocationCallbacks *pAllocator, VkSampler *pSampler,
                                              const ErrorObject &error_obj) const {
    bool skip = false;

    auto num_samplers = Count<vvl::Sampler>();
    if (num_samplers >= phys_dev_props.limits.maxSamplerAllocationCount) {
        skip |= LogError("VUID-vkCreateSampler-maxSamplerAllocationCount-04110", device, error_obj.location,
                         "Number of currently valid sampler objects (%zu) is not less than the maximum allowed (%" PRIu32 ").",
                         num_samplers, phys_dev_props.limits.maxSamplerAllocationCount);
    }

    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    const auto sampler_reduction = vku::FindStructInPNextChain<VkSamplerReductionModeCreateInfo>(pCreateInfo->pNext);
    if (sampler_reduction && sampler_reduction->reductionMode != VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE) {
        if ((api_version >= VK_API_VERSION_1_2) && !enabled_features.samplerFilterMinmax) {
            skip |= LogError("VUID-VkSamplerCreateInfo-pNext-06726", device,
                             create_info_loc.pNext(Struct::VkSamplerReductionModeCreateInfo, Field::reductionMode),
                             "is %s but samplerFilterMinmax feature was not enabled.",
                             string_VkSamplerReductionMode(sampler_reduction->reductionMode));
        } else if ((api_version < VK_API_VERSION_1_2) && !IsExtEnabled(device_extensions.vk_ext_sampler_filter_minmax)) {
            // NOTE: technically this VUID is only if the corresponding _feature_ is not enabled, and only if on api_version
            // >= 1.2, but there doesn't appear to be a similar VUID for when api_version < 1.2
            skip |= LogError(
                "VUID-VkSamplerCreateInfo-pNext-06726", device, error_obj.location, "is %s, but extension %s is not enabled.",
                string_VkSamplerReductionMode(sampler_reduction->reductionMode), VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME);
        }
    }
    if (enabled_features.samplerYcbcrConversion == VK_TRUE) {
        const auto *conversion_info = vku::FindStructInPNextChain<VkSamplerYcbcrConversionInfo>(pCreateInfo->pNext);
        if (conversion_info) {
            const VkSamplerYcbcrConversion sampler_ycbcr_conversion = conversion_info->conversion;
            auto ycbcr_state = Get<vvl::SamplerYcbcrConversion>(sampler_ycbcr_conversion);
            if ((ycbcr_state->format_features &
                 VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT_KHR) == 0) {
                const VkFilter chroma_filter = ycbcr_state->chromaFilter;
                if (pCreateInfo->minFilter != chroma_filter) {
                    skip |= LogError(
                        "VUID-VkSamplerCreateInfo-minFilter-01645", device,
                        create_info_loc.pNext(Struct::VkSamplerYcbcrConversionInfo, Field::conversion),
                        "(%s) does not support VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT "
                        "for format %s and minFilter (%s) is different from "
                        "chromaFilter (%s)",
                        FormatHandle(sampler_ycbcr_conversion).c_str(), string_VkFormat(ycbcr_state->format),
                        string_VkFilter(pCreateInfo->minFilter), string_VkFilter(chroma_filter));
                }
                if (pCreateInfo->magFilter != chroma_filter) {
                    skip |= LogError(
                        "VUID-VkSamplerCreateInfo-minFilter-01645", device,
                        create_info_loc.pNext(Struct::VkSamplerYcbcrConversionInfo, Field::conversion),
                        "(%s) does not support VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT "
                        "for format %s and magFilter (%s) is different from "
                        "chromaFilter (%s)",
                        FormatHandle(sampler_ycbcr_conversion).c_str(), string_VkFormat(ycbcr_state->format),
                        string_VkFilter(pCreateInfo->magFilter), string_VkFilter(chroma_filter));
                }
            }
            // At this point there is a known sampler YCbCr conversion enabled
            if (sampler_reduction) {
                if (sampler_reduction->reductionMode != VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE) {
                    skip |= LogError("VUID-VkSamplerCreateInfo-None-01647", device,
                                     create_info_loc.pNext(Struct::VkSamplerReductionModeCreateInfo, Field::reductionMode),
                                     "is %s but a sampler YCbCr Conversion is being used creating this sampler.",
                                     string_VkSamplerReductionMode(sampler_reduction->reductionMode));
                }
            }
        }
    }

    if (pCreateInfo->borderColor == VK_BORDER_COLOR_INT_CUSTOM_EXT ||
        pCreateInfo->borderColor == VK_BORDER_COLOR_FLOAT_CUSTOM_EXT) {
        if (!enabled_features.customBorderColors) {
            skip |=
                LogError("VUID-VkSamplerCreateInfo-customBorderColors-04085", device, error_obj.location,
                         "is %s but customBorderColors feature was not enabled.", string_VkBorderColor(pCreateInfo->borderColor));
        }
        auto custom_create_info = vku::FindStructInPNextChain<VkSamplerCustomBorderColorCreateInfoEXT>(pCreateInfo->pNext);
        if (custom_create_info) {
            if (custom_create_info->format == VK_FORMAT_UNDEFINED && !enabled_features.customBorderColorWithoutFormat) {
                skip |= LogError("VUID-VkSamplerCustomBorderColorCreateInfoEXT-format-04014", device,
                                 create_info_loc.pNext(Struct::VkSamplerCustomBorderColorCreateInfoEXT, Field::format),
                                 "is VK_FORMAT_UNDEFINED but the "
                                 "customBorderColorWithoutFormat feature was not enabled.");
            }
        }
        if (custom_border_color_sampler_count >= phys_dev_ext_props.custom_border_color_props.maxCustomBorderColorSamplers) {
            skip |= LogError("VUID-VkSamplerCreateInfo-None-04012", device, error_obj.location,
                             "vkCreateSampler(): Creating a sampler with a custom border color will exceed the "
                             "maxCustomBorderColorSamplers limit of %" PRIu32 ".",
                             phys_dev_ext_props.custom_border_color_props.maxCustomBorderColorSamplers);
        }
    }

    if (IsExtEnabled(device_extensions.vk_khr_portability_subset)) {
        if ((VK_FALSE == enabled_features.samplerMipLodBias) && pCreateInfo->mipLodBias != 0) {
            skip |= LogError("VUID-VkSamplerCreateInfo-samplerMipLodBias-04467", device, error_obj.location,
                             "(portability error) mipLodBias is %f, but samplerMipLodBias not supported.", pCreateInfo->mipLodBias);
        }
    }

    // If any of addressModeU, addressModeV or addressModeW are VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE, the
    // VK_KHR_sampler_mirror_clamp_to_edge extension or promoted feature must be enabled
    if (enabled_features.samplerMirrorClampToEdge == VK_FALSE) {
        // Use 'else' because getting 3 large error messages is redundant and assume developer, if set all 3, will notice and fix
        // all at once
        if (pCreateInfo->addressModeU == VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE) {
            skip |= LogError("VUID-VkSamplerCreateInfo-addressModeU-01079", device, create_info_loc.dot(Field::addressModeU),
                             "is VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE but the "
                             "VK_KHR_sampler_mirror_clamp_to_edge extension or samplerMirrorClampToEdge feature was not enabled.");
        } else if (pCreateInfo->addressModeV == VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE) {
            skip |= LogError("VUID-VkSamplerCreateInfo-addressModeU-01079", device, create_info_loc.dot(Field::addressModeV),
                             "is VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE but the "
                             "VK_KHR_sampler_mirror_clamp_to_edge extension or samplerMirrorClampToEdge feature was not enabled.");
        } else if (pCreateInfo->addressModeW == VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE) {
            skip |= LogError("VUID-VkSamplerCreateInfo-addressModeU-01079", device, create_info_loc.dot(Field::addressModeW),
                             "is VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE but the "
                             "VK_KHR_sampler_mirror_clamp_to_edge extension or samplerMirrorClampToEdge feature was not enabled.");
        }
    }

    if ((pCreateInfo->flags & VK_SAMPLER_CREATE_NON_SEAMLESS_CUBE_MAP_BIT_EXT) && (!enabled_features.nonSeamlessCubeMap)) {
        skip |= LogError("VUID-VkSamplerCreateInfo-nonSeamlessCubeMap-06788", device, create_info_loc.dot(Field::flags),
                         "includes VK_SAMPLER_CREATE_NON_SEAMLESS_CUBE_MAP_BIT_EXT but the "
                         "nonSeamlessCubeMap feature was not enabled.");
    }

    if ((pCreateInfo->flags & VK_SAMPLER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT) &&
        !enabled_features.descriptorBufferCaptureReplay) {
        skip |= LogError("VUID-VkSamplerCreateInfo-flags-08110", device, create_info_loc.dot(Field::flags),
                         "includes VK_SAMPLER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT but descriptorBufferCaptureReplay "
                         "feature was not enabled.");
    }

    auto opaque_capture_descriptor_buffer = vku::FindStructInPNextChain<VkOpaqueCaptureDescriptorDataCreateInfoEXT>(pCreateInfo->pNext);
    if (opaque_capture_descriptor_buffer && !(pCreateInfo->flags & VK_SAMPLER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
        skip |= LogError("VUID-VkSamplerCreateInfo-pNext-08111", device, create_info_loc.dot(Field::flags),
                         "is %s but VkOpaqueCaptureDescriptorDataCreateInfoEXT is in pNext chain.",
                         string_VkSamplerCreateFlags(pCreateInfo->flags).c_str());
    }

    return skip;
}
