/* Copyright (c) 2018-2026 The Khronos Group Inc.
 * Copyright (c) 2018-2026 Valve Corporation
 * Copyright (c) 2018-2026 LunarG, Inc.
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

#include "gpuav/core/gpuav_validation_pipeline.h"
#include <optional>

#include "error_message/error_location.h"
#include "generated/dispatch_functions.h"
#include "generated/error_location_helper.h"
#include "gpuav/core/gpuav.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "gpuav/validation_cmd/gpuav_validation_cmd_common.h"
#include "state_tracker/bind_point.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/descriptor_mode.h"
#include "state_tracker/pipeline_state.h"
#include "state_tracker/render_pass_state.h"
#include "vulkan/utility/vk_struct_helper.hpp"
#include "vulkan/vulkan_core.h"

namespace gpuav {
namespace valpipe {
namespace internal {

void CreateDescSetComputePipelineHelper(Validator& gpuav, const Location& loc,
                                        const std::vector<VkDescriptorSetLayoutBinding>& specific_bindings,
                                        VkDescriptorSetLayout additional_desc_set_layout, uint32_t push_constants_byte_size,
                                        uint32_t spirv_size, const uint32_t* spirv, VkDevice& out_device,
                                        VkDescriptorSetLayout& out_specific_descriptor_set_layout,
                                        VkPipelineLayout& out_pipeline_layout, VkShaderModule& out_shader_module,
                                        VkPipeline& out_desc_set_pipeline) {
    out_device = gpuav.device;

    out_specific_descriptor_set_layout = VK_NULL_HANDLE;
    out_pipeline_layout = VK_NULL_HANDLE;
    out_shader_module = VK_NULL_HANDLE;
    out_desc_set_pipeline = VK_NULL_HANDLE;

    VkPushConstantRange push_constant_range = {};
    push_constant_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    push_constant_range.offset = 0;
    push_constant_range.size = push_constants_byte_size;

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = vku::InitStructHelper();

    ds_layout_ci.bindingCount = static_cast<uint32_t>(specific_bindings.size());
    ds_layout_ci.pBindings = specific_bindings.data();
    VkResult result = DispatchCreateDescriptorSetLayout(gpuav.device, &ds_layout_ci, nullptr, &out_specific_descriptor_set_layout);
    if (result != VK_SUCCESS) {
        gpuav.InternalError(gpuav.device, loc, "Failed to create descriptor set layout.");
        return;
    }

    std::vector<VkDescriptorSetLayout> set_layouts = {out_specific_descriptor_set_layout};
    if (additional_desc_set_layout != VK_NULL_HANDLE) {
        set_layouts.emplace_back(additional_desc_set_layout);
    }
    VkPipelineLayoutCreateInfo pipeline_layout_ci = vku::InitStructHelper();
    // Any push constants byte size below 4 is illegal. Can come from empty push constant struct
    if (push_constant_range.size >= 4) {
        pipeline_layout_ci.pushConstantRangeCount = 1;
        pipeline_layout_ci.pPushConstantRanges = &push_constant_range;
    }
    pipeline_layout_ci.setLayoutCount = static_cast<uint32_t>(set_layouts.size());
    pipeline_layout_ci.pSetLayouts = set_layouts.data();
    result = DispatchCreatePipelineLayout(gpuav.device, &pipeline_layout_ci, nullptr, &out_pipeline_layout);
    if (result != VK_SUCCESS) {
        gpuav.InternalError(gpuav.device, loc, "Failed to create pipeline layout.");
        return;
    }

    VkShaderModuleCreateInfo shader_module_ci = vku::InitStructHelper();
    shader_module_ci.codeSize = spirv_size;
    shader_module_ci.pCode = spirv;
    result = DispatchCreateShaderModule(gpuav.device, &shader_module_ci, nullptr, &out_shader_module);
    if (result != VK_SUCCESS) {
        gpuav.InternalError(gpuav.device, loc, "Failed to create shader module.");
        return;
    }

    VkComputePipelineCreateInfo cpci = vku::InitStructHelper();
    cpci.stage = vku::InitStructHelper();
    cpci.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    cpci.stage.module = out_shader_module;
    cpci.stage.pName = "main";
    cpci.layout = out_pipeline_layout;
    result = DispatchCreateComputePipelines(gpuav.device, VK_NULL_HANDLE, 1, &cpci, nullptr, &out_desc_set_pipeline);
    if (result != VK_SUCCESS) {
        gpuav.InternalError(gpuav.device, loc, "Failed to create compute validation pipeline.");
        return;
    }
}

void CreateDescHeapComputePipelineHelper(Validator& gpuav, const Location& loc,
                                         const std::vector<VkDescriptorSetLayoutBinding>& specific_bindings,
                                         bool add_error_logging_descriptors, uint32_t push_data_byte_size, uint32_t spirv_size,
                                         const uint32_t* spirv, std::optional<VkShaderModule>& out_shader_module,
                                         std::optional<VkPipeline>& out_pipeline) {
    if (!gpuav.enabled_features.descriptorHeap) {
        out_shader_module = std::nullopt;
        out_pipeline = std::nullopt;
        return;
    }
    out_shader_module = VK_NULL_HANDLE;
    out_pipeline = VK_NULL_HANDLE;

    VkShaderModuleCreateInfo shader_module_ci = vku::InitStructHelper();
    shader_module_ci.codeSize = spirv_size;
    shader_module_ci.pCode = spirv;
    VkResult result = DispatchCreateShaderModule(gpuav.device, &shader_module_ci, nullptr, &(*out_shader_module));
    if (result != VK_SUCCESS) {
        gpuav.InternalError(gpuav.device, loc, "Failed to create shader module.");
        return;
    }

    std::vector<VkDescriptorSetAndBindingMappingEXT> mappings;
    uint32_t push_address_offset = 0;
    if (push_data_byte_size >= 4) {
        push_address_offset += push_data_byte_size;
    }

    auto add_mapping = [&](const VkDescriptorSetLayoutBinding& binding, uint32_t desc_set) {
        // Anything other than 1 will not be handled properly with current code
        assert(binding.descriptorCount == 1);

        VkDescriptorSetAndBindingMappingEXT& mapping = mappings.emplace_back();
        mapping = vku::InitStructHelper();
        mapping.descriptorSet = desc_set;
        mapping.firstBinding = binding.binding;
        mapping.bindingCount = binding.descriptorCount;
        mapping.resourceMask = VK_SPIRV_RESOURCE_TYPE_ALL_EXT;

        mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT;
        mapping.sourceData.pushAddressOffset = push_address_offset;

        push_address_offset += sizeof(VkDeviceAddress);
    };

    for (const VkDescriptorSetLayoutBinding& binding : specific_bindings) {
        add_mapping(binding, glsl::kValPipeDescSet);
    }

    if (add_error_logging_descriptors) {
        valcmd::ValidationCommandsGpuavState& val_cmd_gpuav_state =
            gpuav.shared_resources_cache.GetOrCreate<valcmd::ValidationCommandsGpuavState>(gpuav, Location(vvl::Func::Empty));

        for (const VkDescriptorSetLayoutBinding& binding : val_cmd_gpuav_state.GetBindings()) {
            add_mapping(binding, glsl::kDiagCommonDescriptorSet);
        }
    }

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = (uint32_t)mappings.size();
    mapping_info.pMappings = mappings.data();

    VkPipelineCreateFlags2CreateInfo pcf = vku::InitStructHelper();
    pcf.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;
    VkComputePipelineCreateInfo cpci = vku::InitStructHelper();
    cpci.pNext = &pcf;
    cpci.stage = vku::InitStructHelper();
    cpci.stage.pNext = &mapping_info;
    cpci.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    cpci.stage.module = *out_shader_module;
    cpci.stage.pName = "main";

    result = DispatchCreateComputePipelines(gpuav.device, VK_NULL_HANDLE, 1, &cpci, nullptr, &(*out_pipeline));
    if (result != VK_SUCCESS) {
        gpuav.InternalError(gpuav.device, loc, "Failed to create compute validation pipeline.");
        return;
    }
}

void DestroyComputePipelineHelper(VkDevice device, VkDescriptorSetLayout specific_descriptor_set_layout,
                                  VkPipelineLayout pipeline_layout, VkShaderModule shader_module, VkPipeline pipeline,
                                  std::optional<VkShaderModule> desc_heap_shader_module,
                                  std::optional<VkPipeline> desc_heap_pipeline) {
    if (specific_descriptor_set_layout != VK_NULL_HANDLE) {
        DispatchDestroyDescriptorSetLayout(device, specific_descriptor_set_layout, nullptr);
    }

    if (pipeline_layout != VK_NULL_HANDLE) {
        DispatchDestroyPipelineLayout(device, pipeline_layout, nullptr);
    }

    if (shader_module != VK_NULL_HANDLE) {
        DispatchDestroyShaderModule(device, shader_module, nullptr);
    }

    if (pipeline != VK_NULL_HANDLE) {
        DispatchDestroyPipeline(device, pipeline, nullptr);
    }

    if (desc_heap_shader_module.has_value() && *desc_heap_shader_module != VK_NULL_HANDLE) {
        DispatchDestroyShaderModule(device, *desc_heap_shader_module, nullptr);
    }

    if (desc_heap_pipeline.has_value() && *desc_heap_pipeline != VK_NULL_HANDLE) {
        DispatchDestroyPipeline(device, *desc_heap_pipeline, nullptr);
    }
}

vvl::DescriptorMode GetCbLastUsedDescriptorMode(CommandBufferSubState& cb_state) {
    return cb_state.base.GetLastUsedDescriptorMode();
}

VkDescriptorSet GetDescriptorSetHelper(CommandBufferSubState& cb_state, VkDescriptorSetLayout desc_set_layout) {
    return cb_state.gpu_resources_manager.GetManagedDescriptorSet(desc_set_layout);
}

void BindDescSetShaderResourcesHelper(Validator& gpuav, CommandBufferSubState& cb_state, VkPipelineLayout pipeline_layout,
                                      VkDescriptorSet desc_set, const std::vector<VkWriteDescriptorSet>& descriptor_writes,
                                      const uint32_t push_constants_byte_size, const void* push_constants,
                                      const std::optional<ErrorLogging>& error_logging) {
    // Any push constants byte size below 4 is illegal. Can come from empty push constant struct
    if (push_constants_byte_size >= 4) {
        DispatchCmdPushConstants(cb_state.VkHandle(), pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, push_constants_byte_size,
                                 push_constants);
    }

    if (error_logging.has_value()) {
        assert(error_logging->cmd_i < gpuav.gpuav_settings.indices_buffer_count);
        assert(error_logging->error_logger_i < gpuav.gpuav_settings.indices_buffer_count);
        std::array<uint32_t, 2> dynamic_offsets = {{error_logging->cmd_i * gpuav.indices_buffer_alignment_,
                                                    error_logging->error_logger_i * gpuav.indices_buffer_alignment_}};

        valcmd::ValidationCommandsGpuavState& val_cmd_gpuav_state =
            gpuav.shared_resources_cache.GetOrCreate<valcmd::ValidationCommandsGpuavState>(gpuav, Location(vvl::Func::Empty));
        valcmd::ValidationCommandsCbState& val_cmd_cb_state =
            cb_state.shared_resources_cache.GetOrCreate<valcmd::ValidationCommandsCbState>(
                gpuav, cb_state, val_cmd_gpuav_state.desc_set_mode.error_logging_desc_set_layout_, Location(vvl::Func::Empty));
        DispatchCmdBindDescriptorSets(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout,
                                      glsl::kDiagCommonDescriptorSet, 1, &val_cmd_cb_state.error_logging_desc_set_,
                                      static_cast<uint32_t>(dynamic_offsets.size()), dynamic_offsets.data());
    }

    if (!descriptor_writes.empty()) {
        // Specific resources
        DispatchUpdateDescriptorSets(gpuav.device, uint32_t(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);

        DispatchCmdBindDescriptorSets(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, glsl::kValPipeDescSet,
                                      1, &desc_set, 0, nullptr);
    }
}

void BindDescHeapShaderResourcesHelper(Validator& gpuav, CommandBufferSubState& cb_state,
                                       const std::vector<VkWriteDescriptorSet>& descriptor_writes,
                                       const uint32_t push_constants_byte_size, const void* push_constants,
                                       const std::optional<ErrorLogging>& error_logging) {
    assert(gpuav.enabled_features.descriptorHeap);
    constexpr uint32_t error_logging_buffers_count = 4;

    VkDeviceSize pd_size = 0;

    // Any push constants byte size below 4 is illegal. Can come from empty push constant struct
    if (push_constants_byte_size >= 4) {
        pd_size += push_constants_byte_size;
    }

    for (const VkWriteDescriptorSet& wds : descriptor_writes) {
        // Assume only buffers are used
        assert(wds.pBufferInfo != nullptr);
        // Assume no buffer descriptor arrays are used
        assert(wds.descriptorCount == 1);
        pd_size += sizeof(VkDeviceAddress);
    }

    if (error_logging.has_value()) {
        assert(error_logging->cmd_i < gpuav.gpuav_settings.indices_buffer_count);
        assert(error_logging->error_logger_i < gpuav.gpuav_settings.indices_buffer_count);

        pd_size += error_logging_buffers_count * sizeof(VkDeviceAddress);
    }

    assert(pd_size <= gpuav.device_state->phys_dev_ext_props.descriptor_heap_props.maxPushDataSize);

    auto push_data = std::make_unique<uint8_t[]>(pd_size);
    uint8_t* push_data_ptr = &push_data[0];

    if (push_constants_byte_size >= 4) {
        std::memcpy(push_data_ptr, push_constants, push_constants_byte_size);
        push_data_ptr += push_constants_byte_size;
    }

    // Tightly pack buffer addresses,
    // in the order defined by descriptor_writes
    for (const VkWriteDescriptorSet& wds : descriptor_writes) {
        auto addr = (VkDeviceAddress*)push_data_ptr;
        VkBufferDeviceAddressInfo bdai = vku::InitStructHelper();
        bdai.buffer = wds.pBufferInfo->buffer;
        *addr = DispatchGetBufferDeviceAddress(gpuav.device, &bdai);
        *addr += wds.pBufferInfo->offset;

        push_data_ptr += sizeof(VkDeviceAddress);
    }

    if (error_logging.has_value()) {
        static_assert(error_logging_buffers_count == 4);
        auto addr = (VkDeviceAddress*)push_data_ptr;
        addr[glsl::kBindingDiagErrorBuffer] = cb_state.error_output_buffer_range_.offset_address;
        addr[glsl::kBindingDiagActionIndex] = gpuav.global_indices_buffer_.Address() + error_logging->cmd_i * sizeof(uint32_t);
        addr[glsl::kBindingDiagCmdResourceIndex] =
            gpuav.global_indices_buffer_.Address() + error_logging->error_logger_i * sizeof(uint32_t);
        addr[glsl::kBindingDiagCmdErrorsCount] = cb_state.GetCmdErrorsCountsBuffer().Address();

        push_data_ptr += error_logging_buffers_count * sizeof(VkDeviceAddress);
    }

    VkPushDataInfoEXT pdi = vku::InitStructHelper();
    pdi.offset = 0;
    pdi.data.address = &push_data[0];
    pdi.data.size = pd_size;

    DispatchCmdPushDataEXT(cb_state.VkHandle(), &pdi);
}

void BindPushConstants(Validator& gpuav, CommandBufferSubState& cb_state, VkPipelineLayout pipeline_layout,
                       vvl::DescriptorMode descriptor_mode, const uint32_t push_constants_byte_size, const void* push_constants) {
    if (descriptor_mode == vvl::DescriptorModeHeap) {
        VkPushDataInfoEXT pdi = vku::InitStructHelper();
        pdi.offset = 0;
        pdi.data.address = push_constants;
        pdi.data.size = push_constants_byte_size;

        DispatchCmdPushDataEXT(cb_state.VkHandle(), &pdi);
    } else {
        DispatchCmdPushConstants(cb_state.VkHandle(), pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, push_constants_byte_size,
                                 push_constants);
    }
}

bool BindComputePipeline(Validator* gpuav, vvl::CommandBuffer& cb_state, VkPipeline pipeline) {
    if (pipeline == VK_NULL_HANDLE) {
        if (gpuav) {
            gpuav->InternalError(cb_state.VkHandle(), Location(vvl::Func::Empty), "Failed to bind validation pipeline");
        }
        return false;
    }
    DispatchCmdBindPipeline(cb_state.VkHandle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    return true;
}

}  // namespace internal

void RestorablePipelineState::Create(CommandBufferSubState& cb_state, VkPipelineBindPoint bind_point) {
    pipeline_bind_point_ = bind_point;
    const vvl::BindPoint vvl_bind_point = ConvertToVvlBindPoint(bind_point);

    LastBound& last_bound = cb_state.base.lastBound[vvl_bind_point];
    if (last_bound.pipeline_state) {
        pipeline_ = last_bound.pipeline_state->VkHandle();
    } else {
        assert(shader_objects_.empty());
        if (vvl_bind_point == vvl::BindPointGraphics) {
            shader_objects_ = last_bound.GetAllBoundGraphicsShaderObjects();
        } else if (vvl_bind_point == vvl::BindPointCompute) {
            auto compute_shader = last_bound.GetShaderObjectState(ShaderObjectStage::COMPUTE);
            if (compute_shader) {
                shader_objects_.emplace_back(compute_shader);
            }
        }
    }

    const vvl::DescriptorMode last_desc_mode = last_bound.GetActionDescriptorMode();
    if (last_desc_mode != vvl::DescriptorModeHeap) {
        desc_set_pipeline_layout_ =
            last_bound.desc_set_pipeline_layout ? last_bound.desc_set_pipeline_layout->VkHandle() : VK_NULL_HANDLE;

        push_constants_data_ = cb_state.push_constant_data_chunks;

        descriptor_sets_.reserve(last_bound.ds_slots.size());
        for (std::size_t set_i = 0; set_i < last_bound.ds_slots.size(); set_i++) {
            const auto& bound_descriptor_set = last_bound.ds_slots[set_i].ds_state;
            if (bound_descriptor_set) {
                descriptor_sets_.emplace_back(bound_descriptor_set->VkHandle(), static_cast<uint32_t>(set_i));
                if (bound_descriptor_set->IsPushDescriptor()) {
                    push_descriptor_set_index_ = static_cast<uint32_t>(set_i);
                }
                dynamic_offsets_.push_back(last_bound.ds_slots[set_i].dynamic_offsets);
            }
        }

        if (last_bound.push_descriptor_set) {
            push_descriptor_set_writes_ = last_bound.push_descriptor_set->GetWrites();
        }
    } else {
        push_data_ = cb_state.push_data_value;
    }

    // Do not handle cb_state.active_render_pass->use_dynamic_rendering_inherited for now
    if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS && cb_state.base.active_render_pass &&
        cb_state.base.active_render_pass->use_dynamic_rendering) {
        rendering_info_ = &cb_state.base.active_render_pass->dynamic_rendering_begin_rendering_info;
        DispatchCmdEndRendering(cb_state.VkHandle());

        VkRenderingInfo rendering_info = vku::InitStructHelper();
        rendering_info.renderArea = {{0, 0}, {1, 1}};
        rendering_info.layerCount = 1;
        rendering_info.viewMask = 0;
        rendering_info.colorAttachmentCount = 0;
        DispatchCmdBeginRendering(cb_state.VkHandle(), &rendering_info);
    }
}

void RestorablePipelineState::Restore() const {
    if (rendering_info_) {
        DispatchCmdEndRendering(cb_state_.VkHandle());
        DispatchCmdBeginRendering(cb_state_.VkHandle(), rendering_info_->ptr());
    }

    if (pipeline_ != VK_NULL_HANDLE) {
        DispatchCmdBindPipeline(cb_state_.VkHandle(), pipeline_bind_point_, pipeline_);
    }
    if (!shader_objects_.empty()) {
        std::vector<VkShaderStageFlagBits> stages;
        std::vector<VkShaderEXT> shaders;
        for (const vvl::ShaderObject* shader_obj : shader_objects_) {
            stages.emplace_back(shader_obj->GetStage());
            shaders.emplace_back(shader_obj->VkHandle());
        }
        DispatchCmdBindShadersEXT(cb_state_.VkHandle(), static_cast<uint32_t>(shader_objects_.size()), stages.data(),
                                  shaders.data());
    }

    for (std::size_t i = 0; i < descriptor_sets_.size(); i++) {
        VkDescriptorSet descriptor_set = descriptor_sets_[i].first;
        if (descriptor_set != VK_NULL_HANDLE) {
            DispatchCmdBindDescriptorSets(cb_state_.VkHandle(), pipeline_bind_point_, desc_set_pipeline_layout_,
                                          descriptor_sets_[i].second, 1, &descriptor_set,
                                          static_cast<uint32_t>(dynamic_offsets_[i].size()), dynamic_offsets_[i].data());
        }
    }

    if (!push_descriptor_set_writes_.empty()) {
        DispatchCmdPushDescriptorSetKHR(cb_state_.VkHandle(), pipeline_bind_point_, desc_set_pipeline_layout_,
                                        push_descriptor_set_index_, static_cast<uint32_t>(push_descriptor_set_writes_.size()),
                                        reinterpret_cast<const VkWriteDescriptorSet*>(push_descriptor_set_writes_.data()));
    }

    for (const auto& push_constant_range : push_constants_data_) {
        DispatchCmdPushConstants(cb_state_.VkHandle(), push_constant_range.layout, push_constant_range.stage_flags,
                                 push_constant_range.offset, static_cast<uint32_t>(push_constant_range.values.size()),
                                 push_constant_range.values.data());
    }

    if (!push_data_.empty()) {
        VkPushDataInfoEXT pdi = vku::InitStructHelper();
        pdi.offset = 0;
        pdi.data.address = push_data_.data();
        pdi.data.size = push_data_.size();
        DispatchCmdPushDataEXT(cb_state_.VkHandle(), &pdi);
    }
}
}  // namespace valpipe
}  // namespace gpuav
