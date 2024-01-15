/* Copyright (c) 2023-2024 The Khronos Group Inc.
 * Copyright (c) 2023-2024 Valve Corporation
 * Copyright (c) 2023-2024 LunarG, Inc.
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

#include "descriptor_validator.h"
#include "generated/spirv_grammar_helper.h"
#include "state_tracker/cmd_buffer_state.h"

template <typename T>
bool vvl::DescriptorValidator::ValidateDescriptors(const DescriptorBindingInfo &binding_info, const T &binding) const {
    bool skip = false;
    for (uint32_t index = 0; !skip && index < binding.count; index++) {
        const auto &descriptor = binding.descriptors[index];

        if (!binding.updated[index]) {
            auto set = descriptor_set.Handle();
            return dev_state.LogError(vuids.descriptor_buffer_bit_set_08114, set, loc,
                            "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                            ") is being used in draw but has never been updated via vkUpdateDescriptorSets() or a similar call.",
                            FormatHandle(set).c_str(), binding_info.first, index);
        }
        skip = ValidateDescriptor(binding_info, index, binding.type, descriptor);
    }
    return skip;
}

bool vvl::DescriptorValidator::ValidateBinding(const DescriptorBindingInfo &binding_info, const vvl::DescriptorBinding &binding) const {
    using DescriptorClass = vvl::DescriptorClass;
    bool skip = false;
    switch (binding.descriptor_class) {
        case DescriptorClass::InlineUniform:
            // Can't validate the descriptor because it may not have been updated.
            break;
        case DescriptorClass::GeneralBuffer:
            skip = ValidateDescriptors(binding_info, static_cast<const vvl::BufferBinding &>(binding));
            break;
        case DescriptorClass::ImageSampler:
            skip = ValidateDescriptors(binding_info, static_cast<const vvl::ImageSamplerBinding &>(binding));
            break;
        case DescriptorClass::Image:
            skip = ValidateDescriptors(binding_info, static_cast<const vvl::ImageBinding &>(binding));
            break;
        case DescriptorClass::PlainSampler:
            skip = ValidateDescriptors(binding_info, static_cast<const vvl::SamplerBinding &>(binding));
            break;
        case DescriptorClass::TexelBuffer:
            skip = ValidateDescriptors(binding_info, static_cast<const vvl::TexelBinding &>(binding));
            break;
        case DescriptorClass::AccelerationStructure:
            skip = ValidateDescriptors(binding_info,
                                       static_cast<const vvl::AccelerationStructureBinding &>(binding));
            break;
        default:
            break;
    }
    return skip;
}

template <typename T>
bool vvl::DescriptorValidator::ValidateDescriptors(const DescriptorBindingInfo &binding_info, const T &binding,
                                                    const std::vector<uint32_t> &indices) {
    bool skip = false;
    for (auto index : indices) {
        const auto &descriptor = binding.descriptors[index];

        if (!binding.updated[index]) {
            auto set = descriptor_set.Handle();
            return dev_state.LogError(vuids.descriptor_buffer_bit_set_08114, set, loc,
                            "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                            ") is being used in draw but has never been updated via vkUpdateDescriptorSets() or a similar call.",
                            FormatHandle(set).c_str(), binding_info.first, index);
        }
        skip = ValidateDescriptor(binding_info, index, binding.type, descriptor);
    }
    return skip;
}

bool vvl::DescriptorValidator::ValidateBinding(const DescriptorBindingInfo &binding_info, const std::vector<uint32_t> &indices) {
    using DescriptorClass = vvl::DescriptorClass;
    auto &binding = *descriptor_set.GetBinding(binding_info.first);
    bool skip = false;
    switch (binding.descriptor_class) {
        case DescriptorClass::InlineUniform:
            // Can't validate the descriptor because it may not have been updated.
            break;
        case DescriptorClass::GeneralBuffer:
            skip = ValidateDescriptors(binding_info, static_cast<const vvl::BufferBinding &>(binding), indices);
            break;
        case DescriptorClass::ImageSampler: {
            auto &imgs_binding = static_cast<vvl::ImageSamplerBinding &>(binding);
            for (auto index : indices) {
                auto &descriptor = imgs_binding.descriptors[index];
                descriptor.UpdateDrawState(&dev_state, &cb_state);
            }
            skip = ValidateDescriptors(binding_info, imgs_binding, indices);
            break;
        }
        case DescriptorClass::Image: {
            auto &img_binding = static_cast<vvl::ImageBinding &>(binding);
            for (auto index : indices) {
                auto &descriptor = img_binding.descriptors[index];
                descriptor.UpdateDrawState(&dev_state, &cb_state);
            }
            skip = ValidateDescriptors(binding_info, img_binding, indices);
            break;
        }
        case DescriptorClass::PlainSampler:
            skip = ValidateDescriptors(binding_info, static_cast<const vvl::SamplerBinding &>(binding), indices);
            break;
        case DescriptorClass::TexelBuffer:
            skip = ValidateDescriptors(binding_info, static_cast<const vvl::TexelBinding &>(binding), indices);
            break;
        case DescriptorClass::AccelerationStructure:
            skip = ValidateDescriptors(binding_info,
                                       static_cast<const vvl::AccelerationStructureBinding &>(binding), indices);
            break;
        default:
            break;
    }
    return skip;
}

bool vvl::DescriptorValidator::ValidateDescriptor(const DescriptorBindingInfo &binding_info, uint32_t index,
                                    VkDescriptorType descriptor_type, const vvl::BufferDescriptor &descriptor) const {
    // Verify that buffers are valid
    const VkBuffer buffer = descriptor.GetBuffer();
    auto buffer_node = descriptor.GetBufferState();
    if ((!buffer_node && !dev_state.enabled_features.nullDescriptor) || (buffer_node && buffer_node->Destroyed())) {
        auto set = descriptor_set.Handle();
        return dev_state.LogError(vuids.descriptor_buffer_bit_set_08114, set, loc,
                        "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                        ") is using buffer %s that is invalid or has been destroyed.",
                        FormatHandle(set).c_str(), binding_info.first, index, FormatHandle(buffer).c_str());
    }

    if (buffer == VK_NULL_HANDLE) {
        return false;
    }
    if (buffer_node /* && !buffer_node->sparse*/) {
        for (const auto &binding : buffer_node->GetInvalidMemory()) {
            auto set = descriptor_set.Handle();
            return dev_state.LogError(vuids.descriptor_buffer_bit_set_08114, set, loc,
                            "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                            ") is using buffer %s that references invalid memory %s.",
                            FormatHandle(set).c_str(), binding_info.first, index, FormatHandle(buffer).c_str(),
                            FormatHandle(binding->deviceMemory()).c_str());
        }
    }
    if (dev_state.enabled_features.protectedMemory == VK_TRUE) {
        if (dev_state.ValidateProtectedBuffer(cb_state, *buffer_node, loc, vuids.unprotected_command_buffer_02707,
                                    "Buffer is in a descriptorSet")) {
            return true;
        }
        bool is_written_to = false;
        for (const auto &req : binding_info.second) {
            if (req.variable->is_written_to) {
                is_written_to = true;
                break;
            }
        }
        if (is_written_to &&
            dev_state.ValidateUnprotectedBuffer(cb_state, *buffer_node, loc, vuids.protected_command_buffer_02712,
                                                "Buffer is in a descriptorSet")) {
            return true;
        }
    }
    return false;
}

static const spirv::ResourceInterfaceVariable *FindMatchingImageVar(const std::vector<DescriptorRequirement> &reqs,
                                                                    const VkImageViewCreateInfo &image_view_ci) {
    if (reqs.empty()) {
        return nullptr;
    }
    // Attempt to find a variable associated with this binding that matches
    // the setup of the image view that is bound to it.
    for (const auto &req : reqs) {
        if (!req.variable || !req.variable->IsImage()) {
            continue;
        }
        const auto dim = req.variable->info.image_dim;
        const auto is_image_array = req.variable->info.is_image_array;
        switch (image_view_ci.viewType) {
            case VK_IMAGE_VIEW_TYPE_1D:
                if ((dim == spv::Dim1D) && !is_image_array) {
                    return req.variable;
                }
                break;
            case VK_IMAGE_VIEW_TYPE_2D:
                if ((dim == spv::Dim2D) && !is_image_array) {
                    return req.variable;
                }
                break;
            case VK_IMAGE_VIEW_TYPE_3D:
                if ((dim == spv::Dim3D) && !is_image_array) {
                    return req.variable;
                }
                break;
            case VK_IMAGE_VIEW_TYPE_CUBE:
                if ((dim == spv::DimCube) && !is_image_array) {
                    return req.variable;
                }
                break;
            case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
                if ((dim == spv::Dim1D) && is_image_array) {
                    return req.variable;
                }
                break;
            case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
                if ((dim == spv::Dim2D) && is_image_array) {
                    return req.variable;
                }
                break;
            case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
                if ((dim == spv::DimCube) && is_image_array) {
                    return req.variable;
                }
                break;
            default:
                break;
        }
    }
    // if nothing matches just use the first entry
    return reqs.begin()->variable;
}

bool vvl::DescriptorValidator::ValidateDescriptor(const DescriptorBindingInfo &binding_info, uint32_t index,
                                    VkDescriptorType descriptor_type,
                                    const vvl::ImageDescriptor &image_descriptor) const {
    std::vector<const vvl::Sampler *> sampler_states;
    const VkImageView image_view = image_descriptor.GetImageView();
    const vvl::ImageView *image_view_state = image_descriptor.GetImageViewState();
    const auto binding = binding_info.first;

    if (image_descriptor.GetClass() == vvl::DescriptorClass::ImageSampler) {
        sampler_states.emplace_back(
            static_cast<const vvl::ImageSamplerDescriptor &>(image_descriptor).GetSamplerState());
    } else {
        for (const auto &req : binding_info.second) {
            if (!req.variable || req.variable->samplers_used_by_image.size() <= index) {
                continue;
            }
            for (const auto &desc_index : req.variable->samplers_used_by_image[index]) {
                const auto *desc =
                    descriptor_set.GetDescriptorFromBinding(desc_index.sampler_slot.binding, desc_index.sampler_index);
                // TODO: This check _shouldn't_ be necessary due to the checks made in ResourceInterfaceVariable() in
                //       shader_validation.cpp. However, without this check some traces still crash.
                if (desc && (desc->GetClass() == vvl::DescriptorClass::PlainSampler)) {
                    const auto *sampler_state = static_cast<const vvl::SamplerDescriptor *>(desc)->GetSamplerState();
                    if (sampler_state) sampler_states.emplace_back(sampler_state);
                }
            }
        }
    }

    if ((!image_view_state && !dev_state.enabled_features.nullDescriptor) || (image_view_state && image_view_state->Destroyed())) {
        // Image view must have been destroyed since initial update. Could potentially flag the descriptor
        //  as "invalid" (updated = false) at DestroyImageView() time and detect this error at bind time

        auto set = descriptor_set.Handle();
        return dev_state.LogError(vuids.descriptor_buffer_bit_set_08114, set, loc,
                        "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                        ") is using imageView %s that is invalid or has been destroyed.",
                        FormatHandle(set).c_str(), binding, index, FormatHandle(image_view).c_str());
    }

    if (image_view == VK_NULL_HANDLE) {
        return false;
    }
    const auto &image_view_ci = image_view_state->create_info;
    const auto *variable = FindMatchingImageVar(binding_info.second, image_view_ci);
    if (variable == nullptr) {
        return false;
    }
    if (!variable->info.is_image_accessed) {
	return false;
    }

    const spv::Dim dim = variable->info.image_dim;
    const bool is_image_array = variable->info.is_image_array;

    // if combined sampler, this variable might not be a OpTypeImage
    // SubpassData gets validated elsewhere
    if (variable->IsImage() && dim != spv::DimSubpassData) {
        bool valid_dim = true;
        // From vkspec.html#textures-operation-validation
        switch (image_view_ci.viewType) {
            case VK_IMAGE_VIEW_TYPE_1D:
                valid_dim = (dim == spv::Dim1D) && !is_image_array;
                break;
            case VK_IMAGE_VIEW_TYPE_2D:
                valid_dim = (dim == spv::Dim2D) && !is_image_array;
                break;
            case VK_IMAGE_VIEW_TYPE_3D:
                valid_dim = (dim == spv::Dim3D) && !is_image_array;
                break;
            case VK_IMAGE_VIEW_TYPE_CUBE:
                valid_dim = (dim == spv::DimCube) && !is_image_array;
                break;
            case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
                valid_dim = (dim == spv::Dim1D) && is_image_array;
                break;
            case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
                valid_dim = (dim == spv::Dim2D) && is_image_array;
                break;
            case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
                valid_dim = (dim == spv::DimCube) && is_image_array;
                break;
            default:
                break;  // incase a new VkImageViewType is added, let it be valid by default
        }
        if (!valid_dim) {
            auto set = descriptor_set.Handle();
            const LogObjectList objlist(set, image_view);
            return dev_state.LogError(vuids.image_view_dim_07752, objlist, loc,
                                      "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                                      ") ImageView type is %s but the OpTypeImage has (Dim = %s) and (Arrayed = %" PRIu32 ").",
                                      FormatHandle(set).c_str(), binding, index, string_VkImageViewType(image_view_ci.viewType),
                                      string_SpvDim(dim), is_image_array);
        }

        if ((variable->info.image_format_type & image_view_state->descriptor_format_bits) == 0) {
            const bool signed_override =
                ((variable->info.image_format_type & spirv::NumericTypeUint) && variable->info.is_sign_extended);
            const bool unsigned_override =
                ((variable->info.image_format_type & spirv::NumericTypeSint) && variable->info.is_zero_extended);
            if (!signed_override && !unsigned_override) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, image_view);
                return dev_state.LogError(vuids.image_view_numeric_format_07753, objlist, loc,
                                          "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                                          ") requires %s component type, but bound descriptor format is %s.",
                                          FormatHandle(set).c_str(), binding, index,
                                          spirv::string_NumericType(variable->info.image_format_type),
                                          string_VkFormat(image_view_ci.format));
            }
        }

        const bool image_format_width_64 = vkuFormatHasComponentSize(image_view_ci.format, 64);
        if (image_format_width_64) {
            if (variable->image_sampled_type_width != 64) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, image_view);
                return dev_state.LogError(
                    vuids.image_view_access_64_04470, objlist, loc,
                    "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                    ") has a 64-bit component ImageView format (%s) but the OpTypeImage's Sampled Type has a width of %" PRIu32 ".",
                    FormatHandle(set).c_str(), binding, index, string_VkFormat(image_view_ci.format),
                    variable->image_sampled_type_width);
            } else if (!dev_state.enabled_features.sparseImageInt64Atomics && image_view_state->image_state->sparse_residency) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, image_view, image_view_state->image_state->image());
                return dev_state.LogError(vuids.image_view_sparse_64_04474, objlist, loc,
                                "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                                ") has a OpTypeImage's Sampled Type has a width of 64 backed by a sparse Image, but "
                                "sparseImageInt64Atomics is not enabled.",
                                FormatHandle(set).c_str(), binding, index);
            }
        } else if (!image_format_width_64 && variable->image_sampled_type_width != 32) {
            auto set = descriptor_set.Handle();
            const LogObjectList objlist(set, image_view);
            return dev_state.LogError(
                vuids.image_view_access_32_04471, objlist, loc,
                "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                ") has a 32-bit component ImageView format (%s) but the OpTypeImage's Sampled Type has a width of %" PRIu32 ".",
                FormatHandle(set).c_str(), binding, index, string_VkFormat(image_view_ci.format),
                variable->image_sampled_type_width);
        }
    }

    if (!dev_state.disabled[image_layout_validation]) {
        VkImageLayout image_layout = image_descriptor.GetImageLayout();
        // Verify Image Layout
        // No "invalid layout" VUID required for this call, since the optimal_layout parameter is UNDEFINED.
        bool hit_error = false;
        dev_state.VerifyImageLayout(cb_state, *image_view_state, image_layout, loc,
                          "VUID-VkDescriptorImageInfo-imageLayout-00344", &hit_error);
        if (hit_error) {
            auto set = descriptor_set.Handle();
            std::stringstream msg;
            if (!descriptor_set.IsPushDescriptor()) {
                msg << "Descriptor set " << FormatHandle(set)
                    << " Image layout specified by vkCmdBindDescriptorSets doesn't match actual image layout at time "
                       "descriptor is used.";
            } else {
                msg << "Image layout specified by vkCmdPushDescriptorSetKHR doesn't match actual image layout at time "
                       "descriptor is used";
            }
            return dev_state.LogError(vuids.descriptor_buffer_bit_set_08114, set, loc,
                            "%s. See previous error callback for specific details.", msg.str().c_str());
        }
    }

    // Verify Sample counts
    if (variable->IsImage() && !variable->info.is_multisampled && image_view_state->samples != VK_SAMPLE_COUNT_1_BIT) {
        auto set = descriptor_set.Handle();
        return dev_state.LogError("VUID-RuntimeSpirv-samples-08725", set, loc,
                        "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32 ") has image created with %s.",
                        FormatHandle(set).c_str(), binding, index, string_VkSampleCountFlagBits(image_view_state->samples));
    }
    if (variable->IsImage() && variable->info.is_multisampled && image_view_state->samples == VK_SAMPLE_COUNT_1_BIT) {
        auto set = descriptor_set.Handle();
        return dev_state.LogError("VUID-RuntimeSpirv-samples-08726", set, loc,
                        "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32 ") has image created with VK_SAMPLE_COUNT_1_BIT.",
                        FormatHandle(set).c_str(), binding, index);
    }

    if (image_view_state->samplerConversion) {
        if (variable->info.is_not_sampler_sampled) {
            auto set = descriptor_set.Handle();
            const LogObjectList objlist(set, image_view);
            return dev_state.LogError(
                vuids.image_ycbcr_sampled_06550, set, loc,
                "the image descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                ") was created with a sampler Ycbcr conversion, but was accessed with a non OpImage*Sample* command.",
                FormatHandle(set).c_str(), binding, index);
        }
        if (variable->info.is_sampler_offset) {
            auto set = descriptor_set.Handle();
            const LogObjectList objlist(set, image_view);
            return dev_state.LogError(
                vuids.image_ycbcr_offset_06551, set, loc,
                "the image descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                ") was created with a sampler Ycbcr conversion, but was accessed with ConstOffset/Offset image operands.",
                FormatHandle(set).c_str(), binding, index);
        }
    }

    // Verify VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT
    if (variable->info.is_atomic_operation && (descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) &&
        !(image_view_state->format_features & VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT)) {
        auto set = descriptor_set.Handle();
        const LogObjectList objlist(set, image_view);
        return dev_state.LogError(
            vuids.imageview_atomic_02691, objlist, loc,
            "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
            ") has %s with format of %s which is missing VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT in its features (%s).",
            FormatHandle(set).c_str(), binding, index, FormatHandle(image_view).c_str(), string_VkFormat(image_view_ci.format),
            string_VkFormatFeatureFlags2(image_view_state->format_features).c_str());
    }

    // When KHR_format_feature_flags2 is supported, the read/write without
    // format support is reported per format rather than a single physical
    // device feature.
    if (dev_state.has_format_feature2) {
        const VkFormatFeatureFlags2 format_features = image_view_state->format_features;

        if (descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) {
            if ((variable->info.is_read_without_format) &&
                !(format_features & VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT)) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, image_view);
                return dev_state.LogError(vuids.storage_image_read_without_format_07028, objlist, loc,
                                "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                                ") has %s with format of %s which is missing VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT "
                                "in its features (%s).",
                                FormatHandle(set).c_str(), binding, index, FormatHandle(image_view).c_str(),
                                string_VkFormat(image_view_ci.format), string_VkFormatFeatureFlags2(format_features).c_str());
            }

            if ((variable->info.is_write_without_format) &&
                !(format_features & VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT)) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, image_view);
                return dev_state.LogError(vuids.storage_image_write_without_format_07027, objlist, loc,
                                "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                                ") has %s with format of %s which is missing VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT "
                                "in its features (%s).",
                                FormatHandle(set).c_str(), binding, index, FormatHandle(image_view).c_str(),
                                string_VkFormat(image_view_ci.format), string_VkFormatFeatureFlags2(format_features).c_str());
            }
        }

        if ((variable->info.is_dref) && !(format_features & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT)) {
            auto set = descriptor_set.Handle();
            const LogObjectList objlist(set, image_view);
            return dev_state.LogError(vuids.depth_compare_sample_06479, objlist, loc,
                            "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                            ") has %s with format of %s which is missing VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT in "
                            "its features (%s).",
                            FormatHandle(set).c_str(), binding, index, FormatHandle(image_view).c_str(),
                            string_VkFormat(image_view_ci.format), string_VkFormatFeatureFlags2(format_features).c_str());
        }
    }

    // Verify if attachments are used in DescriptorSet
    const std::vector<vvl::ImageView *> *attachments = cb_state.active_attachments.get();
    const std::vector<SubpassInfo> *subpasses = cb_state.active_subpasses.get();
    if (attachments && attachments->size() > 0 && subpasses && (descriptor_type != VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)) {
        for (uint32_t att_index = 0; att_index < attachments->size(); ++att_index) {
            const auto &view_state = (*attachments)[att_index];
            const SubpassInfo &subpass = (*subpasses)[att_index];
            if (!view_state || view_state->Destroyed()) {
                continue;
            }
            const bool same_view = view_state->image_view() == image_view;
            const bool overlapping_view = image_view_state->OverlapSubresource(*view_state);
            if (!same_view && !overlapping_view) {
                continue;
            }

            bool descriptor_read_from = false;
            bool descriptor_written_to = false;
            uint32_t set_index = std::numeric_limits<uint32_t>::max();
            for (uint32_t i = 0; i < cb_state.lastBound[VK_PIPELINE_BIND_POINT_GRAPHICS].per_set.size(); ++i) {
                const auto &set = cb_state.lastBound[VK_PIPELINE_BIND_POINT_GRAPHICS].per_set[i];
                if (set.bound_descriptor_set.get() == &(descriptor_set)) {
                    set_index = i;
                    break;
                }
            }
            assert(set_index != std::numeric_limits<uint32_t>::max());
            const auto pipeline = cb_state.GetCurrentPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS);
            for (const auto &stage : pipeline->stage_states) {
                if (!stage.entrypoint) {
                    continue;
                }
                for (const auto &inteface_variable : stage.entrypoint->resource_interface_variables) {
                    if (inteface_variable.decorations.set == set_index && inteface_variable.decorations.binding == binding) {
                        descriptor_written_to |= inteface_variable.is_written_to;
                        descriptor_read_from |=
                            inteface_variable.is_read_from | inteface_variable.info.is_sampler_implicitLod_dref_proj;
                        break;
                    }
                }
            }

            const bool layout_read_only = IsImageLayoutReadOnly(subpass.layout);
            bool color_write_attachment = (subpass.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) != 0;
            bool depth_write_attachment = (subpass.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0 && !layout_read_only &&
                                          (subpass.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT);
            bool stencil_write_attachment = (subpass.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0 &&
                                            !layout_read_only && (subpass.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT);
            bool color_feedback_loop = subpass.layout == VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT;
            bool depth_feedback_loop = subpass.layout == VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT;
            bool stencil_feedback_loop = subpass.layout == VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT;
            if (pipeline && !pipeline->IsDynamic(VK_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT)) {
                if ((pipeline->create_flags & VK_PIPELINE_CREATE_COLOR_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT) == 0) {
                    color_feedback_loop = false;
                }
                if ((pipeline->create_flags & VK_PIPELINE_CREATE_DEPTH_STENCIL_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT) == 0) {
                    depth_feedback_loop = false;
                    stencil_feedback_loop = false;
                }
            } else if (cb_state.dynamic_state_status.cb[CB_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT]) {
                if ((cb_state.dynamic_state_value.attachment_feedback_loop_enable & VK_IMAGE_ASPECT_COLOR_BIT) == 0) {
                    color_feedback_loop = false;
                }
                if ((cb_state.dynamic_state_value.attachment_feedback_loop_enable & VK_IMAGE_ASPECT_DEPTH_BIT) == 0) {
                    depth_feedback_loop = false;
                }
                if ((cb_state.dynamic_state_value.attachment_feedback_loop_enable & VK_IMAGE_ASPECT_STENCIL_BIT) == 0) {
                    stencil_feedback_loop = false;
                }
            }
            if (color_feedback_loop || depth_feedback_loop || stencil_feedback_loop) {
                bool dependency_found = false;
                for (uint32_t i = 0; i < cb_state.activeRenderPass->createInfo.dependencyCount; ++i) {
                    const auto &dep = cb_state.activeRenderPass->createInfo.pDependencies[i];
                    if ((dep.dependencyFlags & VK_DEPENDENCY_FEEDBACK_LOOP_BIT_EXT) != 0 &&
                        dep.srcSubpass == cb_state.GetActiveSubpass() &&
                        dep.dstSubpass == cb_state.GetActiveSubpass()) {
                        dependency_found = true;
                        break;
                    }
                }
                if (!dependency_found) {
                    color_feedback_loop = false;
                    depth_feedback_loop = false;
                    stencil_feedback_loop = false;
                }
            }
            if (((color_write_attachment && !color_feedback_loop) || (depth_write_attachment && !depth_feedback_loop) ||
                 (stencil_write_attachment && !stencil_feedback_loop)) &&
                descriptor_read_from) {
                const auto vuid = color_write_attachment    ? vuids.attachment_access_09000
                                  : !depth_write_attachment ? vuids.attachment_access_09001
                                                            : vuids.attachment_access_09002;
                if (same_view) {
                    auto set = descriptor_set.Handle();
                    const LogObjectList objlist(set, image_view, framebuffer);
                    return dev_state.LogError(vuid, objlist, loc,
                                    "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                                    ") has %s which will be written to as %s attachment %" PRIu32 ".",
                                    FormatHandle(set).c_str(), binding, index, FormatHandle(image_view).c_str(),
                                    FormatHandle(framebuffer).c_str(), att_index);
                } else if (overlapping_view) {
                    auto set = descriptor_set.Handle();
                    const LogObjectList objlist(set, image_view, framebuffer, view_state->image_view());
                    return dev_state.LogError(vuid, objlist, loc,
                                    "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                                    ") has %s which will be overlap written to as %s in %s attachment %" PRIu32 ".",
                                    FormatHandle(set).c_str(), binding, index, FormatHandle(image_view).c_str(),
                                    FormatHandle(view_state->image_view()).c_str(), FormatHandle(framebuffer).c_str(),
                                    att_index);
                }
            }
            const bool read_attachment = (subpass.usage & (VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) > 0;
            if (read_attachment && descriptor_written_to) {
                if (same_view) {
                    auto set = descriptor_set.Handle();
                    const LogObjectList objlist(set, image_view, framebuffer);
                    return dev_state.LogError(vuids.image_subresources_subpass_write_06539, objlist, loc,
                                    "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                                    ") has %s which will be read from as %s attachment %" PRIu32 ".",
                                    FormatHandle(set).c_str(), binding, index, FormatHandle(image_view).c_str(),
                                    FormatHandle(framebuffer).c_str(), att_index);
                } else if (overlapping_view) {
                    auto set = descriptor_set.Handle();
                    const LogObjectList objlist(set, image_view, framebuffer, view_state->image_view());
                    return dev_state.LogError(vuids.image_subresources_subpass_write_06539, objlist, loc,
                                    "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                                    ") has %s which will be overlap read from as %s in %s attachment %" PRIu32 " overlap.",
                                    FormatHandle(set).c_str(), binding, index, FormatHandle(image_view).c_str(),
                                    FormatHandle(view_state->image_view()).c_str(), FormatHandle(framebuffer).c_str(),
                                    att_index);
                }
            }

            if (descriptor_written_to && !layout_read_only) {
                if (same_view) {
                    auto set = descriptor_set.Handle();
                    const LogObjectList objlist(set, image_view, framebuffer);
                    return dev_state.LogError(vuids.image_subresources_render_pass_write_06537, objlist, loc,
                                    "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                                    ") has %s which is written to but is also %s attachment %" PRIu32 ".",
                                    FormatHandle(set).c_str(), binding, index, FormatHandle(image_view).c_str(),
                                    FormatHandle(framebuffer).c_str(), att_index);
                } else if (overlapping_view) {
                    auto set = descriptor_set.Handle();
                    const LogObjectList objlist(set, image_view, framebuffer, view_state->image_view());
                    return dev_state.LogError(vuids.image_subresources_render_pass_write_06537, objlist, loc,
                                    "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                                    ") has %s which overlaps writes to %s but is also %s attachment %" PRIu32 ".",
                                    FormatHandle(set).c_str(), binding, index, FormatHandle(image_view).c_str(),
                                    FormatHandle(view_state->image_view()).c_str(), FormatHandle(framebuffer).c_str(),
                                    att_index);
                }
            }
        }
        if (dev_state.enabled_features.protectedMemory == VK_TRUE) {
            if (dev_state.ValidateProtectedImage(cb_state, *image_view_state->image_state, loc,
                                       vuids.unprotected_command_buffer_02707, "Image is in a descriptorSet")) {
                return true;
            }
            if (variable->is_written_to &&
                dev_state.ValidateUnprotectedImage(cb_state, *image_view_state->image_state, loc,
                                         vuids.protected_command_buffer_02712, "Image is in a descriptorSet")) {
                return true;
            }
        }
    }

    const VkFormat image_view_format = image_view_state->create_info.format;
    for (const auto *sampler_state : sampler_states) {
        if (!sampler_state || sampler_state->Destroyed()) {
            continue;
        }

        // TODO: Validate 04015 for DescriptorClass::PlainSampler
        if ((sampler_state->createInfo.borderColor == VK_BORDER_COLOR_INT_CUSTOM_EXT ||
             sampler_state->createInfo.borderColor == VK_BORDER_COLOR_FLOAT_CUSTOM_EXT) &&
            (sampler_state->customCreateInfo.format == VK_FORMAT_UNDEFINED)) {
            if (image_view_format == VK_FORMAT_B4G4R4A4_UNORM_PACK16 || image_view_format == VK_FORMAT_B5G6R5_UNORM_PACK16 ||
                image_view_format == VK_FORMAT_B5G5R5A1_UNORM_PACK16 || image_view_format == VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, sampler_state->sampler(), image_view_state->image_view());
                return dev_state.LogError(
                    "VUID-VkSamplerCustomBorderColorCreateInfoEXT-format-04015", objlist, loc,
                    "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                    ") has %s which has a custom border color with format = VK_FORMAT_UNDEFINED and is used to sample an image "
                    "view %s with format %s",
                    FormatHandle(set).c_str(), binding, index, FormatHandle(sampler_state->sampler()).c_str(),
                    FormatHandle(image_view_state->image_view()).c_str(), string_VkFormat(image_view_format));
            }
        }
        const VkFilter sampler_mag_filter = sampler_state->createInfo.magFilter;
        const VkFilter sampler_min_filter = sampler_state->createInfo.minFilter;
        const VkBool32 sampler_compare_enable = sampler_state->createInfo.compareEnable;
        if ((sampler_compare_enable == VK_FALSE) &&
            !(image_view_state->format_features & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
            if (sampler_mag_filter == VK_FILTER_LINEAR || sampler_min_filter == VK_FILTER_LINEAR) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, sampler_state->sampler(), image_view_state->image_view());
                return dev_state.LogError(vuids.linear_filter_sampler_04553, objlist, loc,
                                "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                                ") has %s which is set to use VK_FILTER_LINEAR with compareEnable is set "
                                "to VK_FALSE, but image view's (%s) format (%s) does not contain "
                                "VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT in its format features.",
                                FormatHandle(set).c_str(), binding, index, FormatHandle(sampler_state->sampler()).c_str(),
                                FormatHandle(image_view_state->image_view()).c_str(), string_VkFormat(image_view_format));
            }
            if (sampler_state->createInfo.mipmapMode == VK_SAMPLER_MIPMAP_MODE_LINEAR) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, sampler_state->sampler(), image_view_state->image_view());
                return dev_state.LogError(vuids.linear_mipmap_sampler_04770, objlist, loc,
                                "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                                ") has %s which is set to use VK_SAMPLER_MIPMAP_MODE_LINEAR with "
                                "compareEnable is set to VK_FALSE, but image view's (%s) format (%s) does not contain "
                                "VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT in its format features.",
                                FormatHandle(set).c_str(), binding, index, FormatHandle(sampler_state->sampler()).c_str(),
                                FormatHandle(image_view_state->image_view()).c_str(), string_VkFormat(image_view_format));
            }
        }

        if (sampler_mag_filter == VK_FILTER_CUBIC_EXT || sampler_min_filter == VK_FILTER_CUBIC_EXT) {
            if (!(image_view_state->format_features & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT)) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, sampler_state->sampler(), image_view_state->image_view());
                return dev_state.LogError(vuids.cubic_sampler_02692, objlist, loc,
                                "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                                ") has %s which is set to use VK_FILTER_CUBIC_EXT, then image view's (%s) format (%s) "
                                "MUST contain VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT in its format features.",
                                FormatHandle(set).c_str(), binding, index, FormatHandle(sampler_state->sampler()).c_str(),
                                FormatHandle(image_view_state->image_view()).c_str(),
                                string_VkFormat(image_view_state->create_info.format));
            }

            if (IsExtEnabled(dev_state.device_extensions.vk_ext_filter_cubic)) {
                const auto reduction_mode_info = vku::FindStructInPNextChain<VkSamplerReductionModeCreateInfo>(sampler_state->createInfo.pNext);
                if (reduction_mode_info &&
                    (reduction_mode_info->reductionMode == VK_SAMPLER_REDUCTION_MODE_MIN ||
                     reduction_mode_info->reductionMode == VK_SAMPLER_REDUCTION_MODE_MAX) &&
                    !image_view_state->filter_cubic_props.filterCubicMinmax) {
                    auto set = descriptor_set.Handle();
                    const LogObjectList objlist(set, sampler_state->sampler(), image_view_state->image_view());
                    return dev_state.LogError(vuids.filter_cubic_min_max_02695, objlist, loc,
                                    "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                                    ") has %s which is set to use VK_FILTER_CUBIC_EXT & %s, but image view "
                                    "(%s) doesn't support filterCubicMinmax.",
                                    FormatHandle(set).c_str(), binding, index, FormatHandle(sampler_state->sampler()).c_str(),
                                    string_VkSamplerReductionMode(reduction_mode_info->reductionMode),
                                    FormatHandle(image_view_state->image_view()).c_str());
                }

                if (!image_view_state->filter_cubic_props.filterCubic) {
                    auto set = descriptor_set.Handle();
                    const LogObjectList objlist(set, sampler_state->sampler(), image_view_state->image_view());
                    return dev_state.LogError(vuids.filter_cubic_02694, objlist, loc,
                                    "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                                    ") has %s which is set to use VK_FILTER_CUBIC_EXT, but image view (%s) "
                                    "doesn't support filterCubic.",
                                    FormatHandle(set).c_str(), binding, index, FormatHandle(sampler_state->sampler()).c_str(),
                                    FormatHandle(image_view_state->image_view()).c_str());
                }
            }

            if (IsExtEnabled(dev_state.device_extensions.vk_img_filter_cubic)) {
                if (image_view_state->create_info.viewType == VK_IMAGE_VIEW_TYPE_3D ||
                    image_view_state->create_info.viewType == VK_IMAGE_VIEW_TYPE_CUBE ||
                    image_view_state->create_info.viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY) {
                    auto set = descriptor_set.Handle();
                    const LogObjectList objlist(set, sampler_state->sampler(), image_view_state->image_view());
                    return dev_state.LogError(vuids.img_filter_cubic_02693, objlist, loc,
                                    "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                                    ") has %s which is set to use VK_FILTER_CUBIC_EXT while the VK_IMG_filter_cubic "
                                    "extension is enabled, but image view (%s) has an invalid imageViewType (%s).",
                                    FormatHandle(set).c_str(), binding, index, FormatHandle(sampler_state->sampler()).c_str(),
                                    FormatHandle(image_view_state->image_view()).c_str(),
                                    string_VkImageViewType(image_view_state->create_info.viewType));
                }
            }
        }
        const auto image_state = image_view_state->image_state.get();
        if ((image_state->createInfo.flags & VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV) &&
            (sampler_state->createInfo.addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE ||
             sampler_state->createInfo.addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE ||
             sampler_state->createInfo.addressModeW != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)) {
            std::string address_mode_letter =
                (sampler_state->createInfo.addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)   ? "U"
                : (sampler_state->createInfo.addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE) ? "V"
                                                                                                    : "W";
            VkSamplerAddressMode address_mode = (sampler_state->createInfo.addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
                                                    ? sampler_state->createInfo.addressModeU
                                                : (sampler_state->createInfo.addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
                                                    ? sampler_state->createInfo.addressModeV
                                                    : sampler_state->createInfo.addressModeW;
            auto set = descriptor_set.Handle();
            const LogObjectList objlist(set, sampler_state->sampler(), image_state->image(), image_view_state->image_view());
            return dev_state.LogError(vuids.corner_sampled_address_mode_02696, objlist, loc,
                            "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                            ") image (%s) in image view (%s) is created with flag "
                            "VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV and can only be sampled using "
                            "VK_SAMPLER_ADDRESS_MODE_CLAMP_EDGE, but sampler (%s) has "
                            "createInfo.addressMode%s set to %s.",
                            FormatHandle(set).c_str(), binding, index, FormatHandle(image_state->image()).c_str(),
                            FormatHandle(image_view_state->image_view()).c_str(), FormatHandle(sampler_state->sampler()).c_str(),
                            address_mode_letter.c_str(), string_VkSamplerAddressMode(address_mode));
        }

        // UnnormalizedCoordinates sampler validations
        // only check if sampled as could have a texelFetch on a combined image sampler
        if (sampler_state->createInfo.unnormalizedCoordinates && variable->info.is_sampler_sampled) {
            // If ImageView is used by a unnormalizedCoordinates sampler, it needs to check ImageView type
            if (image_view_ci.viewType == VK_IMAGE_VIEW_TYPE_3D || image_view_ci.viewType == VK_IMAGE_VIEW_TYPE_CUBE ||
                image_view_ci.viewType == VK_IMAGE_VIEW_TYPE_1D_ARRAY || image_view_ci.viewType == VK_IMAGE_VIEW_TYPE_2D_ARRAY ||
                image_view_ci.viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, image_view, sampler_state->sampler());
                return dev_state.LogError(vuids.sampler_imageview_type_08609, objlist, loc,
                                "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                                ") Image View %s, type %s, is used by %s.",
                                FormatHandle(set).c_str(), binding, index, FormatHandle(image_view).c_str(),
                                string_VkImageViewType(image_view_ci.viewType), FormatHandle(sampler_state->sampler()).c_str());
            }

            // sampler must not be used with any of the SPIR-V OpImageSample* or OpImageSparseSample*
            // instructions with ImplicitLod, Dref or Proj in their name
            if (variable->info.is_sampler_implicitLod_dref_proj) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, image_view, sampler_state->sampler());
                return dev_state.LogError(vuids.sampler_implicitLod_dref_proj_08610, objlist, loc,
                                "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                                ") Image View %s is used by %s that uses invalid operator.",
                                FormatHandle(set).c_str(), binding, index, FormatHandle(image_view).c_str(),
                                FormatHandle(sampler_state->sampler()).c_str());
            }

            // sampler must not be used with any of the SPIR-V OpImageSample* or OpImageSparseSample*
            // instructions that includes a LOD bias or any offset values
            if (variable->info.is_sampler_bias_offset) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, image_view, sampler_state->sampler());
                return dev_state.LogError(vuids.sampler_bias_offset_08611, objlist, loc,
                                "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                                ") Image View %s is used by %s that uses invalid bias or offset operator.",
                                FormatHandle(set).c_str(), binding, index, FormatHandle(image_view).c_str(),
                                FormatHandle(sampler_state->sampler()).c_str());
            }
        }

        if (sampler_state->samplerConversion) {
            if (variable->info.is_not_sampler_sampled) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, image_view, sampler_state->sampler());
                return dev_state.LogError(
                    vuids.image_ycbcr_sampled_06550, set, loc,
                    "the sampler descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                    ") was created with a sampler Ycbcr conversion, but was accessed with a non OpImage*Sample* command.",
                    FormatHandle(set).c_str(), binding, index);
            }
            if (variable->info.is_sampler_offset) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, image_view, sampler_state->sampler());
                return dev_state.LogError(
                    vuids.image_ycbcr_offset_06551, set, loc,
                    "the sampler descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                    ") was created with a sampler Ycbcr conversion, but was accessed with ConstOffset/Offset image operands.",
                    FormatHandle(set).c_str(), binding, index);
            }
        }
    }

    for (const uint32_t texel_component_count : variable->write_without_formats_component_count_list) {
        const uint32_t format_component_count = vkuFormatComponentCount(image_view_format);
        if (image_view_format == VK_FORMAT_A8_UNORM_KHR) {
            if (texel_component_count != 4) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, image_view);
                return dev_state.LogError(vuids.storage_image_write_texel_count_08796, objlist, loc,
                                "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                                ") VkImageView is mapped to a OpImage format of VK_FORMAT_A8_UNORM_KHR, but the OpImageWrite Texel "
                                "operand only contains %" PRIu32 " components.",
                                FormatHandle(set).c_str(), binding, index, texel_component_count);
            }
        } else if (texel_component_count < format_component_count) {
            auto set = descriptor_set.Handle();
            const LogObjectList objlist(set, image_view);
            return dev_state.LogError(vuids.storage_image_write_texel_count_08795, objlist, loc,
                            "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                            ") VkImageView is mapped to a OpImage format of %s which has %" PRIu32
                            " components, but the OpImageWrite Texel operand only contains %" PRIu32 " components.",
                            FormatHandle(set).c_str(), binding, index, string_VkFormat(image_view_format), format_component_count,
                            texel_component_count);
        }
    }

    return false;
}

bool vvl::DescriptorValidator::ValidateDescriptor(const DescriptorBindingInfo &binding_info, uint32_t index,
                                    VkDescriptorType descriptor_type,
                                    const vvl::ImageSamplerDescriptor &descriptor) const {
    bool skip = ValidateDescriptor(binding_info, index, descriptor_type,
                                   static_cast<const vvl::ImageDescriptor &>(descriptor));
    if (!skip) {
        skip = ValidateSamplerDescriptor(binding_info, index, descriptor.GetSampler(),
                                         descriptor.IsImmutableSampler(), descriptor.GetSamplerState());
    }
    return skip;
}

static const spirv::ResourceInterfaceVariable *FindMatchingTexelVar(const std::vector<DescriptorRequirement> &reqs) {
    if (reqs.empty()) {
        return nullptr;
    }
    // Attempt to find a variable associated with this binding that matches
    // the setup of the image view that is bound to it.
    for (const auto &req : reqs) {
        if (!req.variable || req.variable->IsImage()) {
            continue;
        }
        if (req.variable->info.image_dim == spv::DimBuffer) {
            return req.variable;
        }
    }
    // if nothing matches just use the first entry
    return reqs.begin()->variable;
}

bool vvl::DescriptorValidator::ValidateDescriptor(const DescriptorBindingInfo &binding_info, uint32_t index,
                                    VkDescriptorType descriptor_type,
                                    const vvl::TexelDescriptor &texel_descriptor) const {
    const VkBufferView buffer_view = texel_descriptor.GetBufferView();
    auto buffer_view_state = texel_descriptor.GetBufferViewState();
    const auto binding = binding_info.first;
    if ((!buffer_view_state && !dev_state.enabled_features.nullDescriptor) || (buffer_view_state && buffer_view_state->Destroyed())) {
        auto set = descriptor_set.Handle();
        return dev_state.LogError(vuids.descriptor_buffer_bit_set_08114, set, loc,
                        "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                        ") is using bufferView %s that is invalid or has been destroyed.",
                        FormatHandle(set).c_str(), binding, index, FormatHandle(buffer_view).c_str());
    }

    if (buffer_view == VK_NULL_HANDLE) {
        return false;
    }
    const auto *variable = FindMatchingTexelVar(binding_info.second);
    if (!variable) {
        return false;
    }
    if (!variable->info.is_image_accessed) {
        return false;
    }

    auto buffer = buffer_view_state->create_info.buffer;
    const auto *buffer_state = buffer_view_state->buffer_state.get();
    const VkFormat buffer_view_format = buffer_view_state->create_info.format;
    if (buffer_state->Destroyed()) {
        auto set = descriptor_set.Handle();
        return dev_state.LogError(vuids.descriptor_buffer_bit_set_08114, set, loc,
                        "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32 ") is using buffer %s that has been destroyed.",
                        FormatHandle(set).c_str(), binding, index, FormatHandle(buffer).c_str());
    }
    const uint32_t format_bits = spirv::GetFormatType(buffer_view_format);

    if ((variable->info.image_format_type & format_bits) == 0) {
        const bool signed_override =
            ((variable->info.image_format_type & spirv::NumericTypeUint) && variable->info.is_sign_extended);
        const bool unsigned_override =
            ((variable->info.image_format_type & spirv::NumericTypeSint) && variable->info.is_zero_extended);
        if (!signed_override && !unsigned_override) {
            auto set = descriptor_set.Handle();
            return dev_state.LogError(vuids.descriptor_buffer_bit_set_08114, set, loc,
                                      "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                                      ") requires %s component type, but bound descriptor format is %s.",
                                      FormatHandle(set).c_str(), binding, index,
                                      spirv::string_NumericType(variable->info.image_format_type),
                                      string_VkFormat(buffer_view_format));
        }
    }

    const bool buffer_format_width_64 = vkuFormatHasComponentSize(buffer_view_format, 64);
    if (buffer_format_width_64 && variable->image_sampled_type_width != 64) {
        auto set = descriptor_set.Handle();
        const LogObjectList objlist(set, buffer_view);
        return dev_state.LogError(
            vuids.buffer_view_access_64_04472, objlist, loc,
            "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
            ") has a 64-bit component BufferView format (%s) but the OpTypeImage's Sampled Type has a width of %" PRIu32 ".",
            FormatHandle(set).c_str(), binding, index, string_VkFormat(buffer_view_format), variable->image_sampled_type_width);
    } else if (!buffer_format_width_64 && variable->image_sampled_type_width != 32) {
        auto set = descriptor_set.Handle();
        const LogObjectList objlist(set, buffer_view);
        return dev_state.LogError(
            vuids.buffer_view_access_32_04473, objlist, loc,
            "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
            ") has a 32-bit component BufferView format (%s) but the OpTypeImage's Sampled Type has a width of %" PRIu32 ".",
            FormatHandle(set).c_str(), binding, index, string_VkFormat(buffer_view_format), variable->image_sampled_type_width);
    }

    const VkFormatFeatureFlags2 buf_format_features = buffer_view_state->buf_format_features;

    // Verify VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT
    if ((variable->info.is_atomic_operation) && (descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER) &&
        !(buf_format_features & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT)) {
        auto set = descriptor_set.Handle();
        const LogObjectList objlist(set, buffer_view);
        return dev_state.LogError(
            vuids.bufferview_atomic_07888, objlist, loc,
            "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
            ") has %s with format of %s which is missing VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT in its features (%s).",
            FormatHandle(set).c_str(), binding, index, FormatHandle(buffer_view).c_str(), string_VkFormat(buffer_view_format),
            string_VkFormatFeatureFlags2(buf_format_features).c_str());
    }

    // When KHR_format_feature_flags2 is supported, the read/write without
    // format support is reported per format rather than a single physical
    // device feature.
    if (dev_state.has_format_feature2) {
        if (descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER) {
            if ((variable->info.is_read_without_format) &&
                !(buf_format_features & VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT_KHR)) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, buffer_view);
                return dev_state.LogError(vuids.storage_texel_buffer_read_without_format_07030, objlist, loc,
                                "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                                ") has %s with format of %s which is missing "
                                "VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT_KHR in its features (%s).",
                                FormatHandle(set).c_str(), binding, index, FormatHandle(buffer_view).c_str(),
                                string_VkFormat(buffer_view_format), string_VkFormatFeatureFlags2(buf_format_features).c_str());
            }

            if ((variable->info.is_write_without_format) &&
                !(buf_format_features & VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT_KHR)) {
                auto set = descriptor_set.Handle();
                const LogObjectList objlist(set, buffer_view);
                return dev_state.LogError(vuids.storage_texel_buffer_write_without_format_07029, objlist, loc,
                                "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                                ") has %s with format of %s which is missing "
                                "VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT_KHR in its features (%s).",
                                FormatHandle(set).c_str(), binding, index, FormatHandle(buffer_view).c_str(),
                                string_VkFormat(buffer_view_format), string_VkFormatFeatureFlags2(buf_format_features).c_str());
            }
        }
    }

    if (dev_state.enabled_features.protectedMemory == VK_TRUE) {
        if (dev_state.ValidateProtectedBuffer(cb_state, *buffer_view_state->buffer_state, loc,
                                    vuids.unprotected_command_buffer_02707, "Buffer is in a descriptorSet")) {
            return true;
        }
        if (variable->is_written_to &&
            dev_state.ValidateUnprotectedBuffer(cb_state, *buffer_view_state->buffer_state, loc,
                                                vuids.protected_command_buffer_02712, "Buffer is in a descriptorSet")) {
            return true;
        }
    }

    for (const uint32_t texel_component_count : variable->write_without_formats_component_count_list) {
        const uint32_t format_component_count = vkuFormatComponentCount(buffer_view_format);
        if (texel_component_count < format_component_count) {
            auto set = descriptor_set.Handle();
            const LogObjectList objlist(set, buffer_view);
            return dev_state.LogError(vuids.storage_texel_buffer_write_texel_count_04469, objlist, loc,
                            "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                            ") VkImageView is mapped to a OpImage format of %s which has %" PRIu32
                            " components, but the OpImageWrite Texel operand only contains %" PRIu32 " components.",
                            FormatHandle(set).c_str(), binding, index, string_VkFormat(buffer_view_format), format_component_count,
                            texel_component_count);
        }
    }

    return false;
}

bool vvl::DescriptorValidator::ValidateDescriptor(const DescriptorBindingInfo &binding_info, uint32_t index,
                                    VkDescriptorType descriptor_type,
                                    const vvl::AccelerationStructureDescriptor &descriptor) const {
    // Verify that acceleration structures are valid
    const auto binding = binding_info.first;
    if (descriptor.is_khr()) {
        auto acc = descriptor.GetAccelerationStructure();
        auto acc_node = descriptor.GetAccelerationStructureStateKHR();
        if (!acc_node || acc_node->Destroyed()) {
            if (acc != VK_NULL_HANDLE || !dev_state.enabled_features.nullDescriptor) {
                auto set = descriptor_set.Handle();
                return dev_state.LogError(vuids.descriptor_buffer_bit_set_08114, set, loc,
                                "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                                ") is using acceleration structure %s that is invalid or has been destroyed.",
                                FormatHandle(set).c_str(), binding, index, FormatHandle(acc).c_str());
            }
        } else {
            for (const auto &mem_binding : acc_node->buffer_state->GetInvalidMemory()) {
                auto set = descriptor_set.Handle();
                return dev_state.LogError(vuids.descriptor_buffer_bit_set_08114, set, loc,
                                "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                                ") is using acceleration structure %s that references invalid memory %s.",
                                FormatHandle(set).c_str(), binding, index, FormatHandle(acc).c_str(),
                                FormatHandle(mem_binding->deviceMemory()).c_str());
            }
        }
    } else {
        auto acc = descriptor.GetAccelerationStructureNV();
        auto acc_node = descriptor.GetAccelerationStructureStateNV();
        if (!acc_node || acc_node->Destroyed()) {
            if (acc != VK_NULL_HANDLE || !dev_state.enabled_features.nullDescriptor) {
                auto set = descriptor_set.Handle();
                return dev_state.LogError(vuids.descriptor_buffer_bit_set_08114, set, loc,
                                "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                                ") is using acceleration structure %s that is invalid or has been destroyed.",
                                FormatHandle(set).c_str(), binding, index, FormatHandle(acc).c_str());
            }
        } else {
            for (const auto &mem_binding : acc_node->GetInvalidMemory()) {
                auto set = descriptor_set.Handle();
                return dev_state.LogError(vuids.descriptor_buffer_bit_set_08114, set, loc,
                                "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                                ") is using acceleration structure %s that references invalid memory %s.",
                                FormatHandle(set).c_str(), binding, index, FormatHandle(acc).c_str(),
                                FormatHandle(mem_binding->deviceMemory()).c_str());
            }
        }
    }
    return false;
}

// If the validation is related to both of image and sampler,
// please leave it in (descriptor_class == DescriptorClass::ImageSampler || descriptor_class ==
// DescriptorClass::Image) Here is to validate for only sampler.
bool vvl::DescriptorValidator::ValidateSamplerDescriptor(const DescriptorBindingInfo &binding_info, uint32_t index,
                                                         VkSampler sampler, bool is_immutable,
                                                         const vvl::Sampler *sampler_state) const {
    // Verify Sampler still valid
    if (!sampler_state || sampler_state->Destroyed()) {
        auto set = descriptor_set.Handle();
        return dev_state.LogError(vuids.descriptor_buffer_bit_set_08114, set, loc,
                        "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                        ") is using sampler %s that is invalid or has been destroyed.",
                        FormatHandle(set).c_str(), binding_info.first, index, FormatHandle(sampler).c_str());
    } else {
        if (sampler_state->samplerConversion && !is_immutable) {
            auto set = descriptor_set.Handle();
            return dev_state.LogError(vuids.descriptor_buffer_bit_set_08114, set, loc,
                            "the descriptor (%s, binding %" PRIu32 ", index %" PRIu32
                            ") sampler (%s) contains a YCBCR conversion (%s), but the sampler is not an "
                            "immutable sampler.",
                            FormatHandle(set).c_str(), binding_info.first, index, FormatHandle(sampler).c_str(),
                            FormatHandle(sampler_state->samplerConversion).c_str());
        }
    }
    return false;
}

bool vvl::DescriptorValidator::ValidateDescriptor(const DescriptorBindingInfo &binding_info, uint32_t index,
                                    VkDescriptorType descriptor_type, const vvl::SamplerDescriptor &descriptor) const {
    return ValidateSamplerDescriptor(binding_info, index, descriptor.GetSampler(), descriptor.IsImmutableSampler(), descriptor.GetSamplerState());
}

