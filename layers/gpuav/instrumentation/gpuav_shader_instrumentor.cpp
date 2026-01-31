/* Copyright (c) 2020-2026 The Khronos Group Inc.
 * Copyright (c) 2020-2026 Valve Corporation
 * Copyright (c) 2020-2026 LunarG, Inc.
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

#include "gpuav/instrumentation/gpuav_shader_instrumentor.h"
#include <vulkan/vulkan_core.h>
#include <cstdint>

#include "error_message/error_location.h"
#include "generated/vk_extension_helper.h"
#include "generated/dispatch_functions.h"
#include "chassis/chassis_modification_state.h"
#include "gpuav/core/gpuav_constants.h"
#include "utils/shader_utils.h"
#include "utils/spirv_tools_utils.h"

#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "gpuav/shaders/gpuav_error_codes.h"
#include "gpuav/shaders/gpuav_error_header.h"
#include "gpuav/spirv/log_error_pass.h"
#include "error_message/spirv_logging.h"
#include <spirv/unified1/NonSemanticShaderDebugInfo100.h>
#include <spirv/unified1/spirv.hpp>

#include "state_tracker/pipeline_state.h"
#include "state_tracker/descriptor_sets.h"
#include "state_tracker/shader_object_state.h"
#include "state_tracker/descriptor_mode.h"
#include "gpuav/resources/gpuav_state_trackers.h"

#include "gpuav/spirv/module.h"
#include "gpuav/spirv/descriptor_indexing_oob_pass.h"
#include "gpuav/spirv/buffer_device_address_pass.h"
#include "gpuav/spirv/descriptor_indexing_oob_pass.h"
#include "gpuav/spirv/descriptor_class_general_buffer_pass.h"
#include "gpuav/spirv/descriptor_class_texel_buffer_pass.h"
#include "gpuav/spirv/ray_query_pass.h"
#include "gpuav/spirv/ray_hit_object_pass.h"
#include "gpuav/spirv/mesh_shading_pass.h"
#include "gpuav/spirv/debug_printf_pass.h"
#include "gpuav/spirv/post_process_descriptor_indexing_pass.h"
#include "gpuav/spirv/vertex_attribute_fetch_oob_pass.h"
#include "gpuav/spirv/sanitizer_pass.h"

#include <cassert>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

namespace gpuav {

ReadLockGuard GpuShaderInstrumentor::ReadLock() const {
    if (global_settings.fine_grained_locking) {
        return ReadLockGuard(validation_object_mutex, std::defer_lock);
    } else {
        return ReadLockGuard(validation_object_mutex);
    }
}

WriteLockGuard GpuShaderInstrumentor::WriteLock() {
    if (global_settings.fine_grained_locking) {
        return WriteLockGuard(validation_object_mutex, std::defer_lock);
    } else {
        return WriteLockGuard(validation_object_mutex);
    }
}

void GpuShaderInstrumentor::SetupClassicDescriptor(const Location &loc) {
    const VkDescriptorSetLayoutCreateInfo debug_desc_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0,
                                                                    static_cast<uint32_t>(instrumentation_bindings_.size()),
                                                                    instrumentation_bindings_.data()};

    VkResult result = DispatchCreateDescriptorSetLayout(device, &debug_desc_layout_info, nullptr,
                                                        &instrumentation_desc_layout_[vvl::DescriptorModeClassic]);
    if (result != VK_SUCCESS) {
        InternalError(device, loc, "vkCreateDescriptorSetLayout failed for internal descriptor set");
        Cleanup();
        return;
    }

    const VkDescriptorSetLayoutCreateInfo dummy_desc_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0,
                                                                    0, nullptr};
    result = DispatchCreateDescriptorSetLayout(device, &dummy_desc_layout_info, nullptr,
                                               &dummy_desc_layout_[vvl::DescriptorModeClassic]);
    if (result != VK_SUCCESS) {
        InternalError(device, loc, "vkCreateDescriptorSetLayout failed for internal dummy descriptor set");
        Cleanup();
        return;
    }

    std::vector<VkDescriptorSetLayout> debug_layouts;
    for (uint32_t j = 0; j < instrumentation_desc_set_bind_index_; ++j) {
        debug_layouts.push_back(dummy_desc_layout_[vvl::DescriptorModeClassic]);
    }
    debug_layouts.push_back(instrumentation_desc_layout_[vvl::DescriptorModeClassic]);

    const VkPipelineLayoutCreateInfo debug_pipeline_layout_info = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                                                   nullptr,
                                                                   0u,
                                                                   static_cast<uint32_t>(debug_layouts.size()),
                                                                   debug_layouts.data(),
                                                                   0u,
                                                                   nullptr};
    result = DispatchCreatePipelineLayout(device, &debug_pipeline_layout_info, nullptr,
                                          &instrumentation_pipeline_layout_[vvl::DescriptorModeClassic]);
    if (result != VK_SUCCESS) {
        InternalError(device, loc, "vkCreateDescriptorSetLayout failed for internal pipeline layout");
        Cleanup();
        return;
    }
}

void GpuShaderInstrumentor::SetupDescriptorBuffers(const Location &loc) {
    if (!IsExtEnabled(extensions.vk_ext_descriptor_buffer)) {
        return;
    }

    // We don't use dynamic offset in descriptor buffer, instead we just map the offset each call
    // This isn't ideal to set like this, will not be a problem when we get Root Node working
    instrumentation_bindings_[glsl::kBindingInstActionIndex].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    instrumentation_bindings_[glsl::kBindingInstCmdResourceIndex].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    const VkDescriptorSetLayoutCreateInfo descriptor_buffer_dsl_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
        static_cast<uint32_t>(instrumentation_bindings_.size()), instrumentation_bindings_.data()};

    VkResult result = DispatchCreateDescriptorSetLayout(device, &descriptor_buffer_dsl_info, nullptr,
                                                        &instrumentation_desc_layout_[vvl::DescriptorModeBuffer]);
    if (result != VK_SUCCESS) {
        InternalError(device, loc, "vkCreateDescriptorSetLayout failed for internal descriptor set for descriptor buffer");
        Cleanup();
        return;
    }

    const VkDescriptorSetLayoutCreateInfo descriptor_buffer_dummy_dsl_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT, 0,
        nullptr};
    result = DispatchCreateDescriptorSetLayout(device, &descriptor_buffer_dummy_dsl_info, nullptr,
                                               &dummy_desc_layout_[vvl::DescriptorModeBuffer]);
    if (result != VK_SUCCESS) {
        InternalError(device, loc, "vkCreateDescriptorSetLayout failed for internal dummy descriptor set for descriptor buffer");
        Cleanup();
        return;
    }

    std::vector<VkDescriptorSetLayout> debug_layouts;
    for (uint32_t j = 0; j < instrumentation_desc_set_bind_index_; ++j) {
        debug_layouts.push_back(dummy_desc_layout_[vvl::DescriptorModeBuffer]);
    }
    debug_layouts.push_back(instrumentation_desc_layout_[vvl::DescriptorModeBuffer]);

    const VkPipelineLayoutCreateInfo debug_pipeline_layout_db_info = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                                                      nullptr,
                                                                      0u,
                                                                      static_cast<uint32_t>(debug_layouts.size()),
                                                                      debug_layouts.data(),
                                                                      0u,
                                                                      nullptr};
    result = DispatchCreatePipelineLayout(device, &debug_pipeline_layout_db_info, nullptr,
                                          &instrumentation_pipeline_layout_[vvl::DescriptorModeBuffer]);
    if (result != VK_SUCCESS) {
        InternalError(device, loc, "vkCreateDescriptorSetLayout failed for internal pipeline layout for descriptor buffer");
        Cleanup();
        return;
    }

    VkDeviceSize bytes_to_reserve = 0;
    DispatchGetDescriptorSetLayoutSizeEXT(device, instrumentation_desc_layout_[vvl::DescriptorModeBuffer], &bytes_to_reserve);

    resource_descriptor_buffer_size_ = bytes_to_reserve;
    resource_descriptor_buffer_offsets_.resize(glsl::kTotalBindings);
    for (uint32_t i = 0; i < glsl::kTotalBindings; i++) {
        DispatchGetDescriptorSetLayoutBindingOffsetEXT(device, instrumentation_desc_layout_[vvl::DescriptorModeBuffer], i,
                                                       &resource_descriptor_buffer_offsets_[i]);
    }

    // Revert, because classic needs for fixing disturbed pipelines
    instrumentation_bindings_[glsl::kBindingInstActionIndex].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    instrumentation_bindings_[glsl::kBindingInstCmdResourceIndex].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
}

// In charge of getting things for shader instrumentation that both GPU-AV and DebugPrintF will need
void GpuShaderInstrumentor::FinishDeviceSetup(const VkDeviceCreateInfo *pCreateInfo, const Location &loc) {
    BaseClass::FinishDeviceSetup(pCreateInfo, loc);

    // Update feature and extension state based on changes made to the create info.
    GetEnabledDeviceFeatures(pCreateInfo, &modified_features, api_version);
    modified_extensions = DeviceExtensions(extensions, api_version, pCreateInfo);

    // Check hard requirements for GPU-AV against what we enabled.
    if (!modified_features.fragmentStoresAndAtomics) {
        InternalError(
            device, loc,
            "GPU Shader Instrumentation requires fragmentStoresAndAtomics to allow witting out data inside the fragment shader.");
        return;
    }
    if (!modified_features.vertexPipelineStoresAndAtomics) {
        InternalError(device, loc,
                      "GPU Shader Instrumentation requires vertexPipelineStoresAndAtomics to allow witting out data inside the "
                      "vertex shader.");
        return;
    }
    if (!modified_features.timelineSemaphore) {
        InternalError(device, loc,
                      "GPU Shader Instrumentation requires timelineSemaphore to manage when command buffers are submitted at queue "
                      "submit time.");
        return;
    }
    if (!modified_features.bufferDeviceAddress) {
        InternalError(device, loc, "GPU Shader Instrumentation requires bufferDeviceAddress to manage witting out of the shader.");
        return;
    }
    if (!modified_features.scalarBlockLayout) {
        InternalError(device, loc, "GPU Shader Instrumentation requires scalarBlockLayout to pack data in a shader.");
        return;
    }
    if (modified_features.vulkanMemoryModel && !modified_features.vulkanMemoryModelDeviceScope) {
        InternalError(device, loc,
                      "GPU Shader Instrumentation requires vulkanMemoryModelDeviceScope feature (if vulkanMemoryModel is enabled) "
                      "to let us call atomicAdd to the output buffer.");
        return;
    }

    // maxBoundDescriptorSets limit, but possibly adjusted
    const uint32_t adjusted_max_desc_sets_limit =
        std::min(kMaxAdjustedBoundDescriptorSet, phys_dev_props.limits.maxBoundDescriptorSets);
    // If gpu_validation_reserve_binding_slot: the max slot is where we reserved
    // else: always use the last possible set as least likely to be used
    instrumentation_desc_set_bind_index_ = adjusted_max_desc_sets_limit - 1;

    // We can't do anything if there is only one.
    // Device probably not a legit Vulkan device, since there should be at least 4. Protect ourselves.
    if (adjusted_max_desc_sets_limit == 1) {
        InternalError(device, loc, "Device can bind only a single descriptor set.");
        return;
    }

    SetupClassicDescriptor(loc);
    SetupDescriptorBuffers(loc);
}

void GpuShaderInstrumentor::Cleanup() {
    for (uint32_t i = 0; i < vvl::DescriptorModeCount; i++) {
        if (instrumentation_desc_layout_[i]) {
            DispatchDestroyDescriptorSetLayout(device, instrumentation_desc_layout_[i], nullptr);
            instrumentation_desc_layout_[i] = VK_NULL_HANDLE;
        }
        if (dummy_desc_layout_[i]) {
            DispatchDestroyDescriptorSetLayout(device, dummy_desc_layout_[i], nullptr);
            dummy_desc_layout_[i] = VK_NULL_HANDLE;
        }
        if (instrumentation_pipeline_layout_[i]) {
            DispatchDestroyPipelineLayout(device, instrumentation_pipeline_layout_[i], nullptr);
            instrumentation_pipeline_layout_[i] = VK_NULL_HANDLE;
        }
    }
}

void GpuShaderInstrumentor::PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator,
                                                       const RecordObject &record_obj) {
    Cleanup();
    BaseClass::PreCallRecordDestroyDevice(device, pAllocator, record_obj);
}

// Just gives a warning about a possible deadlock.
bool GpuShaderInstrumentor::ValidateCmdWaitEvents(VkCommandBuffer command_buffer, VkPipelineStageFlags2 src_stage_mask,
                                                  const Location &loc) const {
    if (src_stage_mask & VK_PIPELINE_STAGE_2_HOST_BIT) {
        std::ostringstream error_msg;
        error_msg << loc.Message()
                  << " recorded with VK_PIPELINE_STAGE_HOST_BIT set. GPU-Assisted validation waits on queue completion. This wait "
                     "could block the host's signaling of this event, resulting in deadlock.";
        InternalError(command_buffer, loc, error_msg.str().c_str());
    }
    return false;
}

bool GpuShaderInstrumentor::PreCallValidateCmdWaitEvents(
    VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents, VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier *pImageMemoryBarriers, const ErrorObject &error_obj) const {
    return ValidateCmdWaitEvents(commandBuffer, static_cast<VkPipelineStageFlags2>(srcStageMask), error_obj.location);
}

bool GpuShaderInstrumentor::PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount,
                                                             const VkEvent *pEvents, const VkDependencyInfoKHR *pDependencyInfos,
                                                             const ErrorObject &error_obj) const {
    return PreCallValidateCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, error_obj);
}

bool GpuShaderInstrumentor::PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount,
                                                          const VkEvent *pEvents, const VkDependencyInfo *pDependencyInfos,
                                                          const ErrorObject &error_obj) const {
    VkPipelineStageFlags2 src_stage_mask = 0;

    for (uint32_t i = 0; i < eventCount; i++) {
        auto exec_scopes = sync_utils::GetExecScopes(pDependencyInfos[i]);
        src_stage_mask |= exec_scopes.src;
    }

    return ValidateCmdWaitEvents(commandBuffer, src_stage_mask, error_obj.location);
}

vvl::DescriptorMode GpuShaderInstrumentor::SelectDescriptorModeFromDSL(uint32_t set_layout_count,
                                                                       const VkDescriptorSetLayout *set_layouts) const {
    vvl::DescriptorMode mode = vvl::DescriptorModeClassic;
    if (IsExtEnabled(extensions.vk_ext_descriptor_buffer)) {
        if (set_layout_count > 0) {
            // Only need to check the first one because they all have to be or not-be descriptor buffer
            const auto &dsl_state = Get<vvl::DescriptorSetLayout>(set_layouts[0]);
            if (dsl_state->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) {
                mode = vvl::DescriptorModeBuffer;
            }
        } else if (enabled_features.descriptorBuffer) {
            // At this point, we have actually zero way to know how this VkPipelineLayout/VkShaderEXT is going to be used because
            // the extension never added a flag for creation time here.... so assume that if the descriptorBuffer feature is
            // enabled, app is using it. This is such a rare case it likely is good enough of a solution for now, otherwise we will
            // have to create 2 versions a modified handle and swap it out later.
            mode = vvl::DescriptorModeBuffer;
        }
    }
    return mode;
}

void GpuShaderInstrumentor::PreCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkPipelineLayout *pPipelineLayout, const RecordObject &record_obj,
                                                              chassis::CreatePipelineLayout &chassis_state) {
    if (gpuav_settings.IsSpirvModified()) {
        if (chassis_state.modified_create_info.setLayoutCount > instrumentation_desc_set_bind_index_) {
            std::ostringstream strm;
            strm << "pCreateInfo::setLayoutCount (" << chassis_state.modified_create_info.setLayoutCount
                 << ") will conflicts with validation's descriptor set at slot " << instrumentation_desc_set_bind_index_ << ". "
                 << "This Pipeline Layout has too many descriptor sets that will not allow GPU shader instrumentation to be setup "
                    "for pipelines created with it, therefore no validation error will be repored for them by GPU-AV at runtime.";
            InternalWarning(device, record_obj.location, strm.str().c_str());
        } else {
            vvl::DescriptorMode mode = SelectDescriptorModeFromDSL(pCreateInfo->setLayoutCount, pCreateInfo->pSetLayouts);

            // Modify the pipeline layout by:
            // 1. Copying the caller's descriptor set desc_layouts
            // 2. Fill in dummy descriptor layouts up to the max binding
            // 3. Fill in with the debug descriptor layout at the max binding slot
            chassis_state.new_layouts.reserve(instrumentation_desc_set_bind_index_ + 1);
            chassis_state.new_layouts.insert(chassis_state.new_layouts.end(), &pCreateInfo->pSetLayouts[0],
                                             &pCreateInfo->pSetLayouts[pCreateInfo->setLayoutCount]);
            for (uint32_t i = pCreateInfo->setLayoutCount; i < instrumentation_desc_set_bind_index_; ++i) {
                chassis_state.new_layouts.push_back(dummy_desc_layout_[mode]);
            }
            chassis_state.new_layouts.push_back(instrumentation_desc_layout_[mode]);
            chassis_state.modified_create_info.pSetLayouts = chassis_state.new_layouts.data();
            chassis_state.modified_create_info.setLayoutCount = instrumentation_desc_set_bind_index_ + 1;
        }
    }
}

void GpuShaderInstrumentor::PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule,
                                                             const RecordObject &record_obj,
                                                             chassis::CreateShaderModule &chassis_state) {
    if (record_obj.result != VK_SUCCESS) {
        return;
    }

    // By default, we instrument everything, but if the setting is enabled, we only will instrument the shaders the app picks
    if (gpuav_settings.select_instrumented_shaders && IsSelectiveInstrumentationEnabled(pCreateInfo->pNext)) {
        // If this is being filled up, likely only a few shaders and the app scope is narrowed down, so no need to spend time
        // removing these later
        selected_instrumented_shaders.insert(*pShaderModule);
    };
}

// We on the spot create a VkShaderEXT without instrumentation to return to the user
// We assume people are not trying to use GPU-AV while calling vkGetShaderBinaryDataEXT
// But this is needed for things like CTS that are using this to mock a fake Binary Shader Object
void GpuShaderInstrumentor::PreCallRecordGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t *pDataSize, void *pData,
                                                                const RecordObject &record_obj,
                                                                chassis::ShaderBinaryData &chassis_state) {
    const auto &shader_object_state = Get<vvl::ShaderObject>(shader);
    ASSERT_AND_RETURN(shader_object_state);
    auto &sub_state = SubState(*shader_object_state);

    VkShaderEXT original_handle = VK_NULL_HANDLE;

    auto it = instrumented_shaders_map_.find(sub_state.unique_shader_id);
    if (it == instrumented_shaders_map_.end() || it->second.original_spirv.empty()) {
        // This will occur if the shader was so simple we didn't even instrument anything
        return;
    }

    // The original pCode might be gone, so need to make a shallow copy and put original SPIR-V inside
    VkShaderCreateInfoEXT create_info_copy = *sub_state.original_create_info.ptr();
    // The pCode doesn't live in the safe struct, we need to grab it from our other map
    const gpuav::InstrumentedShader *instrumented_shader = &it->second;
    create_info_copy.pCode = instrumented_shader->original_spirv.data();
    create_info_copy.codeSize = instrumented_shader->original_spirv.size() * sizeof(uint32_t);

    // Only warn on the first call to query the size
    if (pData == nullptr) {
        InternalWarning(
            shader, record_obj.location,
            "GPU-AV instruments all shaders at vkCreateShadersEXT time, this means there are embedded descriptors bound "
            "that we can't detect if needed or not later.\nWe will be calling vkCreateShadersEXT again now to create the "
            "original shader to pass down to the drivere.");
    }

    // vkGetShaderBinaryDataEXT will be called twice, only need to re-created once
    if (sub_state.original_handle == VK_NULL_HANDLE) {
        DispatchCreateShadersEXT(device, 1, &create_info_copy, nullptr, &original_handle);
        sub_state.original_handle = original_handle;  // will be destroyed later
    }

    chassis_state.modified_shader_handle = sub_state.original_handle;
}

bool GpuShaderInstrumentor::PreCallRecordShaderObjectInstrumentation(
    vku::safe_VkShaderCreateInfoEXT &modified_create_info, const Location &create_info_loc,
    chassis::ShaderObjectInstrumentationData &instrumentation_data) {
    if (gpuav_settings.select_instrumented_shaders && !IsSelectiveInstrumentationEnabled(modified_create_info.pNext)) {
        return false;
    }

    std::vector<uint32_t> &instrumented_spirv = instrumentation_data.instrumented_spirv;
    InstrumentationDescriptorSetLayouts instrumentation_dsl;
    BuildDescriptorSetLayoutInfo(modified_create_info, instrumentation_dsl);

    const uint32_t unique_shader_id = unique_shader_module_id_++;
    const bool is_shader_instrumented = InstrumentShader(
        vvl::make_span(static_cast<const uint32_t *>(modified_create_info.pCode), modified_create_info.codeSize / sizeof(uint32_t)),
        unique_shader_id, instrumentation_dsl, create_info_loc, instrumented_spirv);

    if (is_shader_instrumented) {
        instrumentation_data.unique_shader_id = unique_shader_id;
        modified_create_info.pCode = instrumented_spirv.data();
        modified_create_info.codeSize = instrumented_spirv.size() * sizeof(uint32_t);
    }
    return is_shader_instrumented;
}

void GpuShaderInstrumentor::PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                          const VkShaderCreateInfoEXT *pCreateInfos,
                                                          const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                                          const RecordObject &record_obj, chassis::ShaderObject &chassis_state) {
    if (!gpuav_settings.IsSpirvModified()) return;

    // Resize here so if using just CoreCheck we don't waste time allocating this
    chassis_state.instrumentations_data.resize(createInfoCount);
    chassis_state.modified_create_infos.resize(createInfoCount);

    for (uint32_t i = 0; i < createInfoCount; ++i) {
        // Need deep copy as there might be pNext items
        vku::safe_VkShaderCreateInfoEXT &new_create_info = chassis_state.modified_create_infos[i];
        new_create_info.initialize(&pCreateInfos[i]);

        const Location &create_info_loc = record_obj.location.dot(vvl::Field::pCreateInfos, i);
        auto &instrumentation_data = chassis_state.instrumentations_data[i];

        if (new_create_info.codeType != VK_SHADER_CODE_TYPE_SPIRV_EXT) {
            continue;
        }

        // See pipeline version for explanation
        if (new_create_info.flags & VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT) {
            InternalError(device, create_info_loc,
                          "Unable to instrument shader using VkIndirectExecutionSetEXT validly, things might work, but likely will "
                          "not because of GPU-AV's usage of VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC (If you don't "
                          "need VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT, turn it off).");
        }

        if (new_create_info.setLayoutCount > instrumentation_desc_set_bind_index_) {
            std::ostringstream strm;
            strm << "pCreateInfos[" << i << "]::setLayoutCount (" << new_create_info.setLayoutCount
                 << ") will conflicts with validation's descriptor set at slot " << instrumentation_desc_set_bind_index_ << ". "
                 << "This Shader Object has too many descriptor sets that will not allow GPU shader instrumentation to be setup "
                    "for VkShaderEXT created with it, therefore no validation error will be repored for them by GPU-AV at "
                    "runtime.";
            InternalWarning(device, record_obj.location, strm.str().c_str());
        } else {
            // Modify the pipeline layout by:
            // 1. Copying the caller's descriptor set desc_layouts
            // 2. Fill in dummy descriptor layouts up to the max binding
            // 3. Fill in with the debug descriptor layout at the max binding slot
            const VkShaderCreateInfoEXT &original_create_info = pCreateInfos[i];

            vvl::DescriptorMode mode =
                SelectDescriptorModeFromDSL(original_create_info.setLayoutCount, original_create_info.pSetLayouts);

            // We need to remove the old layouts we copied in safe_VkShaderCreateInfoEXT::initialize
            if (new_create_info.pSetLayouts) {
                delete[] new_create_info.pSetLayouts;
            }

            new_create_info.setLayoutCount = instrumentation_desc_set_bind_index_ + 1;
            new_create_info.pSetLayouts = new VkDescriptorSetLayout[new_create_info.setLayoutCount];
            for (uint32_t k = 0; k < original_create_info.setLayoutCount; ++k) {
                new_create_info.pSetLayouts[k] = original_create_info.pSetLayouts[k];
            }
            for (uint32_t k = original_create_info.setLayoutCount; k < instrumentation_desc_set_bind_index_; ++k) {
                new_create_info.pSetLayouts[k] = dummy_desc_layout_[mode];
            }
            new_create_info.pSetLayouts[instrumentation_desc_set_bind_index_] = instrumentation_desc_layout_[mode];

            chassis_state.is_modified |=
                PreCallRecordShaderObjectInstrumentation(new_create_info, create_info_loc, instrumentation_data);
        }
    }

    chassis_state.pCreateInfos = reinterpret_cast<VkShaderCreateInfoEXT *>(chassis_state.modified_create_infos.data());
}

void GpuShaderInstrumentor::PostCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                           const VkShaderCreateInfoEXT *pCreateInfos,
                                                           const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                                           const RecordObject &record_obj, chassis::ShaderObject &chassis_state) {
    if (!gpuav_settings.IsSpirvModified()) {
        return;
    }
    // This can occur if the driver failed to compile the instrumented shader or if a PreCall step failed
    if (!chassis_state.is_modified) {
        return;
    }
    for (uint32_t i = 0; i < createInfoCount; ++i) {
        // If there are multiple shaders being created, and one is bad, will return a non VK_SUCCESS but we need to check if the
        // VkShaderEXT was null or not to actually know if it was created
        const VkShaderEXT shader_handle = pShaders[i];
        if (shader_handle == VK_NULL_HANDLE) {
            continue;
        }

        auto &instrumentation_data = chassis_state.instrumentations_data[i];

        // if the shader for some reason was not instrumented, there is nothing to save
        // (like not using VK_SHADER_CODE_TYPE_SPIRV_EXT)
        if (!instrumentation_data.IsInstrumented()) {
            continue;
        }
        const auto &shader_object_state = Get<vvl::ShaderObject>(shader_handle);
        ASSERT_AND_CONTINUE(shader_object_state);
        auto &sub_state = SubState(*shader_object_state);

        sub_state.was_instrumented = true;
        sub_state.unique_shader_id = instrumentation_data.unique_shader_id;
        // Note - this doesn't make a deep copy of the pCode, but does of the DescriptorSetLayout which we
        sub_state.original_create_info.initialize(&pCreateInfos[i]);

        // We currently need to store a copy of the original, non-instrumented shader so if there is debug information.
        std::vector<uint32_t> code;
        if (shader_object_state->spirv) {
            code = shader_object_state->spirv->words_;
        }

        instrumented_shaders_map_.insert_or_assign(instrumentation_data.unique_shader_id, VK_NULL_HANDLE, VK_NULL_HANDLE,
                                                   shader_handle, std::move(code));
    }
}

void GpuShaderInstrumentor::PreCallRecordDestroyShaderEXT(VkDevice device, VkShaderEXT shader,
                                                          const VkAllocationCallbacks *pAllocator, const RecordObject &record_obj) {
    if (auto shader_object_state = Get<vvl::ShaderObject>(shader)) {
        auto &sub_state = SubState(*shader_object_state);
        instrumented_shaders_map_.pop(sub_state.unique_shader_id);

        if (sub_state.original_handle != VK_NULL_HANDLE) {
            DispatchDestroyShaderEXT(device, sub_state.original_handle, nullptr);
        }
    }
}

void GpuShaderInstrumentor::PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                 const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                                 const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                 const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                                 chassis::CreateGraphicsPipelines &chassis_state) {
    if (!gpuav_settings.IsSpirvModified()) return;

    chassis_state.shader_instrumentations_metadata.resize(count);
    chassis_state.modified_create_infos.resize(count);

    for (uint32_t i = 0; i < count; ++i) {
        const auto &pipeline_state = pipeline_states[i];
        const Location create_info_loc = record_obj.location.dot(vvl::Field::pCreateInfos, i);

        // Need to make a deep copy so if SPIR-V is inlined, user doesn't see it after the call
        auto &new_pipeline_ci = chassis_state.modified_create_infos[i];
        new_pipeline_ci.initialize(&pipeline_state->GraphicsCreateInfo());

        if (!NeedPipelineCreationShaderInstrumentation(*pipeline_state, create_info_loc)) {
            continue;
        }

        auto &shader_instrumentation_metadata = chassis_state.shader_instrumentations_metadata[i];

        bool success = false;
        if (pipeline_state->linking_shaders != 0) {
            success = PreCallRecordPipelineCreationShaderInstrumentationGPL(pAllocator, *pipeline_state, new_pipeline_ci,
                                                                            create_info_loc, shader_instrumentation_metadata);
        } else {
            success = PreCallRecordPipelineCreationShaderInstrumentation(pAllocator, *pipeline_state, new_pipeline_ci,
                                                                         uint32_t(pipeline_state->stage_states.size()),
                                                                         create_info_loc, shader_instrumentation_metadata);
        }
        if (!success) {
            return;
        }
    }

    chassis_state.is_modified = true;
    chassis_state.pCreateInfos = reinterpret_cast<VkGraphicsPipelineCreateInfo *>(chassis_state.modified_create_infos.data());
}

void GpuShaderInstrumentor::PreCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                const VkComputePipelineCreateInfo *pCreateInfos,
                                                                const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                                chassis::CreateComputePipelines &chassis_state) {
    if (!gpuav_settings.IsSpirvModified()) return;

    chassis_state.shader_instrumentations_metadata.resize(count);
    chassis_state.modified_create_infos.resize(count);

    for (uint32_t i = 0; i < count; ++i) {
        const auto &pipeline_state = pipeline_states[i];
        const Location create_info_loc = record_obj.location.dot(vvl::Field::pCreateInfos, i);

        // Need to make a deep copy so if SPIR-V is inlined, user doesn't see it after the call
        auto &new_pipeline_ci = chassis_state.modified_create_infos[i];
        new_pipeline_ci.initialize(&pipeline_state->ComputeCreateInfo());

        if (!NeedPipelineCreationShaderInstrumentation(*pipeline_state, create_info_loc)) {
            continue;
        }

        auto &shader_instrumentation_metadata = chassis_state.shader_instrumentations_metadata[i];

        bool success = PreCallRecordPipelineCreationShaderInstrumentation(pAllocator, *pipeline_state, new_pipeline_ci, 1,
                                                                          create_info_loc, shader_instrumentation_metadata);
        if (!success) {
            return;
        }
    }

    chassis_state.is_modified = true;
    chassis_state.pCreateInfos = reinterpret_cast<VkComputePipelineCreateInfo *>(chassis_state.modified_create_infos.data());
}

void GpuShaderInstrumentor::PreCallRecordCreateRayTracingPipelinesKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t count,
    const VkRayTracingPipelineCreateInfoKHR *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
    const RecordObject &record_obj, PipelineStates &pipeline_states, chassis::CreateRayTracingPipelinesKHR &chassis_state) {
    if (!gpuav_settings.IsSpirvModified()) {
        return;
    }

    chassis_state.shader_instrumentations_metadata.resize(count);
    chassis_state.modified_create_infos.resize(count);

    for (uint32_t i = 0; i < count; ++i) {
        const auto &pipeline_state = pipeline_states[i];
        const Location create_info_loc = record_obj.location.dot(vvl::Field::pCreateInfos, i);

        // Need to make a deep copy so if SPIR-V is inlined, user doesn't see it after the call
        auto &new_pipeline_ci = chassis_state.modified_create_infos[i];
        new_pipeline_ci.initialize(&pipeline_state->RayTracingCreateInfo());

        if (!NeedPipelineCreationShaderInstrumentation(*pipeline_state, create_info_loc)) {
            continue;
        }

        auto &shader_instrumentation_metadata = chassis_state.shader_instrumentations_metadata[i];

        // Ray tracing pipelines can be made of libraries, but contrary to GPL instrumentation is not postponed
        // to final link time, and done at ray tracing library creation time.
        // => No need to iterate over shader stages coming from libraries,
        // stop at VkRayTracingPipelineCreateInfoKHR::stageCount
        // Note: This code implicitly relies on the fact that in pipeline_state->stage_states,
        // stages coming from libraries are added last.
        bool success = PreCallRecordPipelineCreationShaderInstrumentation(pAllocator, *pipeline_state, new_pipeline_ci,
                                                                          new_pipeline_ci.stageCount, create_info_loc,
                                                                          shader_instrumentation_metadata);
        if (!success) {
            return;
        }
    }

    chassis_state.is_modified = true;
    chassis_state.pCreateInfos = reinterpret_cast<VkRayTracingPipelineCreateInfoKHR *>(chassis_state.modified_create_infos.data());
}

template <typename CreateInfos, typename SafeCreateInfos>
static void UtilCopyCreatePipelineFeedbackData(CreateInfos &create_info, SafeCreateInfos &safe_create_info) {
    auto src_feedback_struct = vku::FindStructInPNextChain<VkPipelineCreationFeedbackCreateInfo>(safe_create_info.pNext);
    if (!src_feedback_struct) return;
    auto dst_feedback_struct = const_cast<VkPipelineCreationFeedbackCreateInfo *>(
        vku::FindStructInPNextChain<VkPipelineCreationFeedbackCreateInfo>(create_info.pNext));
    *dst_feedback_struct->pPipelineCreationFeedback = *src_feedback_struct->pPipelineCreationFeedback;
    for (uint32_t j = 0; j < src_feedback_struct->pipelineStageCreationFeedbackCount; j++) {
        dst_feedback_struct->pPipelineStageCreationFeedbacks[j] = src_feedback_struct->pPipelineStageCreationFeedbacks[j];
    }
}

void GpuShaderInstrumentor::PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                  const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                                  const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                  const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                                  chassis::CreateGraphicsPipelines &chassis_state) {
    if (!gpuav_settings.IsSpirvModified()) return;
    // VK_PIPELINE_COMPILE_REQUIRED means that the current pipeline creation call was used to poke the driver cache,
    // no pipeline is created in this case
    if (record_obj.result == VK_PIPELINE_COMPILE_REQUIRED) return;
    // This can occur if the driver failed to compile the instrumented shader or if a PreCall step failed
    if (!chassis_state.is_modified) return;

    for (uint32_t i = 0; i < count; ++i) {
        const VkPipeline pipeline_handle = pPipelines[i];
        if (pipeline_handle == VK_NULL_HANDLE) {
            continue;  // vkspec.html#pipelines-multiple
        }

        UtilCopyCreatePipelineFeedbackData(pCreateInfos[i], chassis_state.modified_create_infos[i]);
        auto pipeline_state = Get<vvl::Pipeline>(pipeline_handle);
        ASSERT_AND_CONTINUE(pipeline_state);

        // Move all instrumentation until the final linking time
        if (pipeline_state->create_flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) continue;

        auto &shader_instrumentation_metadata = chassis_state.shader_instrumentations_metadata[i];
        if (pipeline_state->linking_shaders != 0) {
            PostCallRecordPipelineCreationShaderInstrumentationGPL(*pipeline_state, shader_instrumentation_metadata);
        } else {
            PostCallRecordPipelineCreationShaderInstrumentation(*pipeline_state, uint32_t(pipeline_state->stage_states.size()),
                                                                shader_instrumentation_metadata);
        }
    }
}

void GpuShaderInstrumentor::PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                 const VkComputePipelineCreateInfo *pCreateInfos,
                                                                 const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                 const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                                 chassis::CreateComputePipelines &chassis_state) {
    if (!gpuav_settings.IsSpirvModified()) return;
    // VK_PIPELINE_COMPILE_REQUIRED means that the current pipeline creation call was used to poke the driver cache,
    // no pipeline is created in this case
    if (record_obj.result == VK_PIPELINE_COMPILE_REQUIRED) return;
    // This can occur if the driver failed to compile the instrumented shader or if a PreCall step failed
    if (!chassis_state.is_modified) return;

    for (uint32_t i = 0; i < count; ++i) {
        const VkPipeline pipeline_handle = pPipelines[i];
        if (pipeline_handle == VK_NULL_HANDLE) {
            continue;  // vkspec.html#pipelines-multiple
        }

        UtilCopyCreatePipelineFeedbackData(pCreateInfos[i], chassis_state.modified_create_infos[i]);

        auto pipeline_state = Get<vvl::Pipeline>(pipeline_handle);
        ASSERT_AND_CONTINUE(pipeline_state);
        auto &shader_instrumentation_metadata = chassis_state.shader_instrumentations_metadata[i];
        PostCallRecordPipelineCreationShaderInstrumentation(*pipeline_state, 1, shader_instrumentation_metadata);
    }
}

void GpuShaderInstrumentor::PostCallRecordCreateRayTracingPipelinesKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t count,
    const VkRayTracingPipelineCreateInfoKHR *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
    const RecordObject &record_obj, PipelineStates &pipeline_states,
    std::shared_ptr<chassis::CreateRayTracingPipelinesKHR> chassis_state) {
    // This can occur if the driver failed to compile the instrumented shader or if a PreCall step failed
    if (!chassis_state->is_modified) {
        return;
    }

    if (!gpuav_settings.IsSpirvModified()) {
        return;
    }
    // VK_PIPELINE_COMPILE_REQUIRED means that the current pipeline creation call was used to poke the driver cache,
    // no pipeline is created in this case
    if (record_obj.result == VK_PIPELINE_COMPILE_REQUIRED) {
        return;
    }

    const bool is_operation_deferred = deferredOperation != VK_NULL_HANDLE && record_obj.result == VK_OPERATION_DEFERRED_KHR;

    if (is_operation_deferred) {
        for (uint32_t i = 0; i < count; ++i) {
            UtilCopyCreatePipelineFeedbackData(pCreateInfos[i], chassis_state->modified_create_infos[i]);
        }

        if (dispatch_device_->wrap_handles) {
            deferredOperation = dispatch_device_->Unwrap(deferredOperation);
        }

        auto found = dispatch_device_->deferred_operation_post_check.pop(deferredOperation);
        std::vector<std::function<void(std::pair<uint32_t, VkPipeline *>)>> deferred_op_post_checks;
        if (found->first) {
            deferred_op_post_checks = std::move(found->second);
        } else {
            // vvl::Device::PostCallRecordCreateRayTracingPipelinesKHR should have added a lambda in
            // deferred_operation_post_check for the current deferredOperation.
            // This lambda is responsible for initializing the pipeline state we maintain,
            // this state will be accessed in the following lambda.
            // Given how PostCallRecordCreateRayTracingPipelinesKHR is called in
            // GpuShaderInstrumentor::PostCallRecordCreateRayTracingPipelinesKHR
            // conditions holds as of writing. But it is something we need to be aware of.
            assert(false);
            return;
        }

        deferred_op_post_checks.emplace_back([this, held_chassis_state =
                                                        chassis_state](std::pair<uint32_t, VkPipeline *> pipelines) mutable {
            for (const auto [pipe_i, pipe] : vvl::enumerate(pipelines.second, pipelines.first)) {
                std::shared_ptr<vvl::Pipeline> pipeline_state = ((GpuShaderInstrumentor *)this)->Get<vvl::Pipeline>(pipe);
                ASSERT_AND_CONTINUE(pipeline_state);
                if (pipeline_state->ray_tracing_library_ci) {
                    for (VkPipeline lib : vvl::make_span(pipeline_state->ray_tracing_library_ci->pLibraries,
                                                         pipeline_state->ray_tracing_library_ci->libraryCount)) {
                        auto lib_state = ((GpuShaderInstrumentor *)this)->Get<vvl::Pipeline>(lib);
                        ASSERT_AND_CONTINUE(lib_state);
                        pipeline_state->instrumentation_data.was_instrumented |= lib_state->instrumentation_data.was_instrumented;
                    }
                }
                auto &shader_instrumentation_metadata = held_chassis_state->shader_instrumentations_metadata[pipe_i];
                // Ray tracing pipelines can be made of libraries, but contrary to GPL instrumentation is not postponed
                // to final link time, and done at ray tracing library creation time.
                // => No need to iterate over shader stages coming from libraries,
                // stop at VkRayTracingPipelineCreateInfoKHR::stageCount
                // Note: This code implicitly relies on the fact that in pipeline_state->stage_states,
                // stages coming from libraries are added last.
                PostCallRecordPipelineCreationShaderInstrumentation(
                    *pipeline_state, pipeline_state->RayTracingCreateInfo().stageCount, shader_instrumentation_metadata);
            }
        });
        dispatch_device_->deferred_operation_post_check.insert(deferredOperation, std::move(deferred_op_post_checks));
    } else {
        for (uint32_t i = 0; i < count; ++i) {
            const VkPipeline pipeline_handle = pPipelines[i];
            if (pipeline_handle == VK_NULL_HANDLE) {
                continue;  // vkspec.html#pipelines-multiple
            }

            UtilCopyCreatePipelineFeedbackData(pCreateInfos[i], chassis_state->modified_create_infos[i]);

            auto pipeline_state = Get<vvl::Pipeline>(pipeline_handle);

            if (pipeline_state->ray_tracing_library_ci) {
                for (VkPipeline lib : vvl::make_span(pipeline_state->ray_tracing_library_ci->pLibraries,
                                                     pipeline_state->ray_tracing_library_ci->libraryCount)) {
                    auto lib_state = Get<vvl::Pipeline>(lib);
                    ASSERT_AND_CONTINUE(lib_state);
                    pipeline_state->instrumentation_data.was_instrumented |= lib_state->instrumentation_data.was_instrumented;
                }
            }

            auto &shader_instrumentation_metadata = chassis_state->shader_instrumentations_metadata[i];
            // Ray tracing pipelines can be made of libraries, but contrary to GPL instrumentation is not postponed
            // to final link time, and done at ray tracing library creation time.
            // => No need to iterate over shader stages coming from libraries,
            // stop at VkRayTracingPipelineCreateInfoKHR::stageCount
            // Note: This code implicitly relies on the fact that in pipeline_state->stage_states,
            // stages coming from libraries are added last.
            PostCallRecordPipelineCreationShaderInstrumentation(*pipeline_state, pipeline_state->RayTracingCreateInfo().stageCount,
                                                                shader_instrumentation_metadata);
        }
    }
}

// Remove all the shader trackers associated with this destroyed pipeline.
void GpuShaderInstrumentor::PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline,
                                                         const VkAllocationCallbacks *pAllocator, const RecordObject &record_obj) {
    if (auto pipeline_state = Get<vvl::Pipeline>(pipeline)) {
        for (auto shader_module_handle : pipeline_state->instrumentation_data.shader_modules) {
            DispatchDestroyShaderModule(device, shader_module_handle, pAllocator);
        }
        if (pipeline_state->instrumentation_data.instrumented_pipeline_lib != VK_NULL_HANDLE) {
            DispatchDestroyPipeline(device, pipeline_state->instrumentation_data.instrumented_pipeline_lib, pAllocator);
        }
    }
}

template <typename CreateInfo>
VkShaderModule GetShaderModule(const CreateInfo &create_info, VkShaderStageFlagBits stage) {
    for (uint32_t i = 0; i < create_info.stageCount; ++i) {
        if (create_info.pStages[i].stage == stage) {
            return create_info.pStages[i].module;
        }
    }
    return {};
}

template <>
VkShaderModule GetShaderModule(const VkComputePipelineCreateInfo &create_info, VkShaderStageFlagBits) {
    return create_info.stage.module;
}

template <typename SafeType>
void SetShaderModule(SafeType &create_info, const vku::safe_VkPipelineShaderStageCreateInfo &stage_info,
                     VkShaderModule shader_module, uint32_t stage_ci_index) {
    create_info.pStages[stage_ci_index] = stage_info;
    create_info.pStages[stage_ci_index].module = shader_module;
}

template <>
void SetShaderModule(vku::safe_VkComputePipelineCreateInfo &create_info,
                     const vku::safe_VkPipelineShaderStageCreateInfo &stage_info, VkShaderModule shader_module,
                     uint32_t stage_ci_index) {
    assert(stage_ci_index == 0);
    create_info.stage = stage_info;
    create_info.stage.module = shader_module;
}

template <typename CreateInfo, typename StageInfo>
StageInfo &GetShaderStageCI(CreateInfo &ci, VkShaderStageFlagBits stage) {
    static StageInfo null_stage{};
    for (uint32_t i = 0; i < ci.stageCount; ++i) {
        if (ci.pStages[i].stage == stage) {
            return ci.pStages[i];
        }
    }
    return null_stage;
}

template <>
vku::safe_VkPipelineShaderStageCreateInfo &GetShaderStageCI(vku::safe_VkComputePipelineCreateInfo &ci, VkShaderStageFlagBits) {
    return ci.stage;
}

bool GpuShaderInstrumentor::IsSelectiveInstrumentationEnabled(const void *pNext) {
    if (auto features = vku::FindStructInPNextChain<VkValidationFeaturesEXT>(pNext)) {
        for (uint32_t i = 0; i < features->enabledValidationFeatureCount; i++) {
            if (features->pEnabledValidationFeatures[i] == VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT) {
                return true;
            }
        }
    }
    return false;
}

bool GpuShaderInstrumentor::NeedPipelineCreationShaderInstrumentation(vvl::Pipeline &pipeline_state, const Location &loc) {
    // Currently there is a VU (VUID-VkIndirectExecutionSetPipelineInfoEXT-initialPipeline-11019) that prevents
    // VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC in the pipeline layout, but we need it currently for GPU-AV.
    // As a temporary solution, we will just not support people using DGC with IES
    if (pipeline_state.create_flags & VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT) {
        InternalError(device, loc,
                      "Unable to instrument shader using VkIndirectExecutionSetEXT validly, things might work, but likely will not "
                      "because of GPU-AV's usage of VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC (If you don't need "
                      "VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT, turn it off).");
        // don't return false, some drivers seem to not care and app might get away with it
    }

    // will hit with using GPL without shaders in them (ex. fragment output)
    if (pipeline_state.stage_states.empty()) {
        return false;
    }

    // For graphics pipelines, move all instrumentation until the final linking time
    // This still needs to create a copy of the create_info (we *could* have a mix of GPL and non-GPL)
    // For ray tracing pipeline, because shader binding tables can be built from the libraries before
    // linking the exe pipeline, no choice but to always instrument libraries: SBT have to point to instrumented shaders
    if (pipeline_state.pipeline_type != VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR &&
        (pipeline_state.create_flags & VK_PIPELINE_CREATE_2_LIBRARY_BIT_KHR) && pipeline_state.linking_shaders == 0) {
        return false;
    }

    // If the app requests all available sets, the pipeline layout was not modified at pipeline layout creation and the
    // already instrumented shaders need to be replaced with uninstrumented shaders
    if (pipeline_state.active_slots.find(instrumentation_desc_set_bind_index_) != pipeline_state.active_slots.end()) {
        return false;
    }
    const auto pipeline_layout = pipeline_state.PipelineLayoutState();
    if (pipeline_layout && pipeline_layout->set_layouts.list.size() > instrumentation_desc_set_bind_index_) {
        return false;
    }

    return true;
}

void GpuShaderInstrumentor::BuildDescriptorSetLayoutInfo(const vvl::Pipeline &pipeline_state,
                                                         InstrumentationDescriptorSetLayouts &out_instrumentation_dsl) {
    const auto pipeline_layout = pipeline_state.PipelineLayoutState();
    if (!pipeline_layout) {
        return;
    }

    out_instrumentation_dsl.set_index_to_bindings_layout_lut.resize(pipeline_layout->set_layouts.list.size());
    for (uint32_t set_layout_index = 0; set_layout_index < pipeline_layout->set_layouts.list.size(); set_layout_index++) {
        if (const auto set_layout_state = pipeline_layout->set_layouts.list[set_layout_index]) {
            BuildDescriptorSetLayoutInfo(*set_layout_state, set_layout_index, out_instrumentation_dsl);
        }
    }

    // Set ray tracing pipeline flags for VUIDs 11886/11887
    out_instrumentation_dsl.pipeline_has_skip_aabbs_flag =
        (pipeline_state.create_flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR) != 0;
    out_instrumentation_dsl.pipeline_has_skip_triangles_flag =
        (pipeline_state.create_flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR) != 0;
    // For VUID 11888 - maxShaderBindingTableRecordIndex limit
    out_instrumentation_dsl.max_shader_binding_table_record_index =
        phys_dev_ext_props.ray_tracing_invocation_reorder_props.maxShaderBindingTableRecordIndex;
}

void GpuShaderInstrumentor::BuildDescriptorSetLayoutInfo(const vku::safe_VkShaderCreateInfoEXT &modified_create_info,
                                                         InstrumentationDescriptorSetLayouts &out_instrumentation_dsl) {
    out_instrumentation_dsl.set_index_to_bindings_layout_lut.resize(modified_create_info.setLayoutCount);
    for (const auto [set_layout_index, set_layout] :
         vvl::enumerate(modified_create_info.pSetLayouts, modified_create_info.setLayoutCount)) {
        if (auto set_layout_state = Get<vvl::DescriptorSetLayout>(set_layout)) {
            BuildDescriptorSetLayoutInfo(*set_layout_state, set_layout_index, out_instrumentation_dsl);
        }
    }
}

void GpuShaderInstrumentor::BuildDescriptorSetLayoutInfo(const vvl::DescriptorSetLayout &set_layout_state,
                                                         const uint32_t set_layout_index,
                                                         InstrumentationDescriptorSetLayouts &out_instrumentation_dsl) {
    if (set_layout_state.GetBindingCount() == 0) return;
    const uint32_t binding_count = set_layout_state.GetMaxBinding() + 1;

    auto &binding_layouts = out_instrumentation_dsl.set_index_to_bindings_layout_lut[set_layout_index];
    binding_layouts.resize(binding_count);

    uint32_t start = 0;
    auto dsl_bindings = set_layout_state.GetBindings();
    for (uint32_t binding_index = 0; binding_index < dsl_bindings.size(); binding_index++) {
        auto &dsl_binding = dsl_bindings[binding_index];
        if (dsl_binding.descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
            binding_layouts[dsl_binding.binding] = {start, 1};
            start += 1;
        } else {
            binding_layouts[dsl_binding.binding] = {start, dsl_binding.descriptorCount};
            start += dsl_binding.descriptorCount;
        }

        const VkDescriptorBindingFlags flags = set_layout_state.GetDescriptorBindingFlagsFromBinding(binding_index);
        if (vvl::IsBindless(flags)) {
            out_instrumentation_dsl.has_bindless_descriptors = true;
        }
    }
}

bool GpuShaderInstrumentor::IsPipelineSelectedForInstrumentation(VkPipeline pipeline, const Location &loc) {
    if (!gpuav_settings.select_instrumented_shaders) {
        return true;
    }

    bool should_instrument_pipeline = false;
    {
        std::string pipeline_debug_name;
        {
            std::unique_lock<std::mutex> lock(debug_report->debug_output_mutex);
            pipeline_debug_name = debug_report->GetUtilsObjectNameNoLock(HandleToUint64(pipeline));
        }

        should_instrument_pipeline = gpuav_settings.MatchesAnyShaderSelectionRegex(pipeline_debug_name);
    }
    if (should_instrument_pipeline) {
        LogInfo("GPU-AV::Selective shader instrumentation", LogObjectList(), loc, "(%s) will be instrumented for validation.",
                FormatHandle(pipeline).c_str());
    }
    return should_instrument_pipeline;
}

bool GpuShaderInstrumentor::IsShaderSelectedForInstrumentation(vku::safe_VkShaderModuleCreateInfo *modified_shader_module_ci,
                                                               VkShaderModule modified_shader, const Location &loc) {
    if (!gpuav_settings.select_instrumented_shaders) {
        return true;
    }

    bool should_instrument_shader = false;
    {
        if (modified_shader_module_ci && IsSelectiveInstrumentationEnabled(modified_shader_module_ci->pNext)) {
            should_instrument_shader = true;
        } else if (selected_instrumented_shaders.find(modified_shader) != selected_instrumented_shaders.end()) {
            should_instrument_shader = true;
        } else {
            std::string shader_debug_name;
            {
                std::unique_lock<std::mutex> lock(debug_report->debug_output_mutex);
                shader_debug_name = debug_report->GetUtilsObjectNameNoLock(HandleToUint64(modified_shader));
            }
            should_instrument_shader = gpuav_settings.MatchesAnyShaderSelectionRegex(shader_debug_name);
        }
        if (should_instrument_shader) {
            LogInfo("GPU-AV::Selective shader instrumentation", LogObjectList(), loc, "(%s) will be instrumented for validation.",
                    FormatHandle(modified_shader).c_str());
        }
    }
    return should_instrument_shader;
}

// Instrument all SPIR-V that is sent through pipeline. This can be done in various ways
// 1. VkCreateShaderModule and passed in VkShaderModule.
//    For this we create our own VkShaderModule with instrumented shader and manage it inside the pipeline state
// 2. GPL
//    We defer until linking time, otherwise we will instrument many libraries that might never be used.
//    (this also spreads the compile time cost evenly instead of a huge spike on startup)
// 3. Inlined via VkPipelineShaderStageCreateInfo pNext
//    We just instrument the shader and update the inlined SPIR-V
// 4. VK_EXT_shader_module_identifier
//    We will skip these as we don't know the incoming SPIR-V
// Note: Shader Objects are handled in their own path as they don't use pipelines
template <typename SafeCreateInfo>
bool GpuShaderInstrumentor::PreCallRecordPipelineCreationShaderInstrumentation(
    const VkAllocationCallbacks *pAllocator, vvl::Pipeline &pipeline_state, SafeCreateInfo &modified_pipeline_ci,
    uint32_t stages_count, const Location &loc,
    std::vector<chassis::ShaderInstrumentationMetadata> &shader_instrumentation_metadata) {
    // Init here instead of in chassis so we don't pay cost when GPU-AV is not used
    shader_instrumentation_metadata.resize(stages_count);

    InstrumentationDescriptorSetLayouts instrumentation_dsl;
    BuildDescriptorSetLayoutInfo(pipeline_state, instrumentation_dsl);

    for (uint32_t stage_state_i = 0; stage_state_i < stages_count; ++stage_state_i) {
        const auto &stage_state = pipeline_state.stage_states[stage_state_i];
        auto modified_module_state = std::const_pointer_cast<vvl::ShaderModule>(stage_state.module_state);
        ASSERT_AND_CONTINUE(modified_module_state);
        if (!modified_module_state->spirv) {
            continue;  // Hit when using VK_KHR_pipeline_binary
        }
        std::unique_lock<std::mutex> module_lock(modified_module_state->module_mutex_);

        auto &instrumentation_metadata = shader_instrumentation_metadata[stage_state_i];

        // Check pNext for inlined SPIR-V
        // ---
        vku::safe_VkShaderModuleCreateInfo *modified_shader_module_ci = nullptr;
        {
            const VkShaderStageFlagBits stage = stage_state.GetStage();
            auto &stage_ci =
                GetShaderStageCI<SafeCreateInfo, vku::safe_VkPipelineShaderStageCreateInfo>(modified_pipeline_ci, stage);
            modified_shader_module_ci =
                const_cast<vku::safe_VkShaderModuleCreateInfo *>(reinterpret_cast<const vku::safe_VkShaderModuleCreateInfo *>(
                    vku::FindStructInPNextChain<VkShaderModuleCreateInfo>(stage_ci.pNext)));

            if (!IsShaderSelectedForInstrumentation(modified_shader_module_ci, modified_module_state->VkHandle(),
                                                    loc.dot(vvl::Field::pStages, stage_state_i).dot(vvl::Field::module))) {
                continue;
            }
        }
        std::vector<uint32_t> instrumented_spirv;
        const uint32_t unique_shader_id = unique_shader_module_id_++;
        const bool is_shader_instrumented =
            InstrumentShader(modified_module_state->spirv->words_, unique_shader_id, instrumentation_dsl, loc, instrumented_spirv);
        if (is_shader_instrumented) {
            instrumentation_metadata.unique_shader_id = unique_shader_id;
            if (modified_module_state->VkHandle() != VK_NULL_HANDLE) {
                // If the user used vkCreateShaderModule, we create a new VkShaderModule to replace with the instrumented
                // shader
                VkShaderModuleCreateInfo instrumented_shader_module_ci = vku::InitStructHelper();
                instrumented_shader_module_ci.pCode = instrumented_spirv.data();
                instrumented_shader_module_ci.codeSize = instrumented_spirv.size() * sizeof(uint32_t);
                VkShaderModule instrumented_shader_module = VK_NULL_HANDLE;
                VkResult result =
                    DispatchCreateShaderModule(device, &instrumented_shader_module_ci, pAllocator, &instrumented_shader_module);
                if (result == VK_SUCCESS) {
                    SetShaderModule(modified_pipeline_ci, *stage_state.pipeline_create_info, instrumented_shader_module,
                                    stage_state_i);

                    pipeline_state.instrumentation_data.shader_modules.emplace_back(instrumented_shader_module);
                    pipeline_state.instrumentation_data.was_instrumented = true;
                } else {
                    InternalError(device, loc, "Unable to replace non-instrumented shader with instrumented one.");
                    return false;
                }
            } else if (modified_shader_module_ci) {
                // The user is inlining the Shader Module into the pipeline, so just need to update the spirv
                instrumentation_metadata.passed_in_shader_stage_ci = true;
                // TODO - This makes a copy, but could save on Chassis stack instead (then remove function from VUL).
                // The core issue is we always use std::vector<uint32_t> but Safe Struct manages its own version of the pCode
                // memory. It would be much harder to change everything from std::vector and instead to adjust Safe Struct to not
                // double-free the memory on us. If making any changes, we have to consider a case where the user inlines the
                // fragment shader, but use a normal VkShaderModule in the vertex shader.
                modified_shader_module_ci->SetCode(instrumented_spirv);
            } else {
                assert(false);
                return false;
            }
        }
    }
    return true;
}

// Now that we have created the pipeline (and have its handle) build up the shader map for each shader we instrumented
void GpuShaderInstrumentor::PostCallRecordPipelineCreationShaderInstrumentation(
    vvl::Pipeline &pipeline_state, uint32_t stages_count,
    std::vector<chassis::ShaderInstrumentationMetadata> &shader_instrumentation_metadata) {
    // if we return early from NeedPipelineCreationShaderInstrumentation, will need to skip at this point in PostCall
    if (shader_instrumentation_metadata.empty()) return;

    for (uint32_t stage_state_i = 0; stage_state_i < stages_count; ++stage_state_i) {
        auto &instrumentation_metadata = shader_instrumentation_metadata[stage_state_i];

        // if the shader for some reason was not instrumented, there is nothing to save
        if (!instrumentation_metadata.IsInstrumented()) {
            continue;
        }
        pipeline_state.instrumentation_data.was_instrumented = true;

        const auto &stage_state = pipeline_state.stage_states[stage_state_i];
        auto &module_state = stage_state.module_state;

        // We currently need to store a copy of the original, non-instrumented shader so if there is debug information,
        // we can reference it by the instruction number printed out in the shader. Since the application can destroy the
        // original VkShaderModule, there is a chance this will be gone, we need to copy it now.
        // TODO - in the instrumentation, instead of printing the instruction number only, if we print out debug info, we
        // can remove this copy
        std::vector<uint32_t> code;
        if (module_state && module_state->spirv) code = module_state->spirv->words_;

        VkShaderModule shader_module_handle = module_state->VkHandle();
        if (shader_module_handle == VK_NULL_HANDLE && instrumentation_metadata.passed_in_shader_stage_ci) {
            shader_module_handle = kPipelineStageInfoHandle;
        }

        instrumented_shaders_map_.insert_or_assign(instrumentation_metadata.unique_shader_id, pipeline_state.VkHandle(),
                                                   shader_module_handle, VK_NULL_HANDLE, std::move(code));
    }
}

// While have an almost duplicated function is not ideal, the core issue is we have a single, templated function designed for
// Graphics, Compute, and Ray Tracing. GPL is only for graphics, so we end up needing this "side code path" for graphics only and it
// doesn't fit in the "all pipeline" templated flow.
bool GpuShaderInstrumentor::PreCallRecordPipelineCreationShaderInstrumentationGPL(
    const VkAllocationCallbacks *pAllocator, vvl::Pipeline &linked_pipeline_state,
    vku::safe_VkGraphicsPipelineCreateInfo &modified_pipeline_ci, const Location &loc,
    std::vector<chassis::ShaderInstrumentationMetadata> &shader_instrumentation_metadata) {
    // Init here instead of in chassis so we don't pay cost when GPU-AV is not used
    const size_t total_stages = linked_pipeline_state.stage_states.size();
    shader_instrumentation_metadata.resize(total_stages);

    InstrumentationDescriptorSetLayouts instrumentation_dsl;
    BuildDescriptorSetLayoutInfo(linked_pipeline_state, instrumentation_dsl);

    auto modified_library_ci = const_cast<VkPipelineLibraryCreateInfoKHR *>(
        vku::FindStructInPNextChain<VkPipelineLibraryCreateInfoKHR>(modified_pipeline_ci.pNext));

    // the "pStages[]" is spread across libraries, so build it up in the double for loop
    uint32_t shader_i = 0;

    // This outer loop is the main difference between the GPL and non-GPL version and why its hard to merge them
    for (uint32_t modified_lib_i = 0; modified_lib_i < modified_library_ci->libraryCount; ++modified_lib_i) {
        const auto modified_lib = Get<vvl::Pipeline>(modified_library_ci->pLibraries[modified_lib_i]);
        if (!modified_lib) {
            continue;
        }
        if (modified_lib->stage_states.empty()) {
            continue;
        }

        // If a library is used to create multiple executable pipelines, we don't want to instrument it again.
        // Check that there is indeed an instrumented_pipeline_lib:
        // The library could be considered instrumented if itself it was made up of instrumented libraries,
        // but in this case instrumented_pipeline_lib would not have been set.
        // Note: Well in this case we could use modified_lib->VkHandle()?
        if (modified_lib->instrumentation_data.was_instrumented &&
            (modified_lib->instrumentation_data.instrumented_pipeline_lib != VK_NULL_HANDLE)) {
            assert(modified_lib->instrumentation_data.instrumented_pipeline_lib != VK_NULL_HANDLE);
            const_cast<VkPipeline *>(modified_library_ci->pLibraries)[modified_lib_i] =
                modified_lib->instrumentation_data.instrumented_pipeline_lib;
            linked_pipeline_state.instrumentation_data.was_instrumented = true;
            continue;
        }

        vku::safe_VkGraphicsPipelineCreateInfo new_lib_ci(modified_lib->GraphicsCreateInfo());
        // If the application supplied pipeline might be interested in failing to be created
        // if the driver does not find it in its cache, GPU-AV needs to succeed in the instrumented pipeline library
        // creation process no matter caching state.
        new_lib_ci.flags &= ~VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT;
        bool need_new_pipeline = false;

        // If pipeline library is selected for instrumentation, force instrumentation of all its shaders
        const bool should_instrument_pipeline =
            IsPipelineSelectedForInstrumentation(modified_lib->VkHandle(), loc.dot(vvl::Field::pLibraries, modified_lib_i));
        for (uint32_t stage_state_i = 0; stage_state_i < static_cast<uint32_t>(modified_lib->stage_states.size());
             ++stage_state_i) {
            const ShaderStageState &modified_stage_state = modified_lib->stage_states[stage_state_i];
            auto modified_module_state = std::const_pointer_cast<vvl::ShaderModule>(modified_stage_state.module_state);
            ASSERT_AND_CONTINUE(modified_module_state);
            if (!modified_module_state->spirv) {
                continue;  // Hit when using VK_KHR_pipeline_binary
            }
            std::unique_lock<std::mutex> module_lock(modified_module_state->module_mutex_);

            chassis::ShaderInstrumentationMetadata &instrumentation_metadata = shader_instrumentation_metadata[shader_i++];

            // Check pNext for inlined SPIR-V
            // ---
            vku::safe_VkShaderModuleCreateInfo *modified_shader_module_ci = nullptr;
            {
                vku::safe_VkPipelineShaderStageCreateInfo *modified_stage_ci = nullptr;
                const VkShaderStageFlagBits stage = modified_stage_state.GetStage();
                for (uint32_t i = 0; i < new_lib_ci.stageCount; ++i) {
                    if (new_lib_ci.pStages[i].stage == stage) {
                        modified_stage_ci = &new_lib_ci.pStages[i];
                    }
                }

                // Getting here means that the currently explored stage state comes
                // from a sub library, one used to composed the currently explored library.
                // This sub library itself went through the linking process, thus has
                // already been instrumented.
                // => Just proceed to the next shader stage.
                if (!modified_stage_ci) {
                    continue;
                }

                modified_shader_module_ci =
                    const_cast<vku::safe_VkShaderModuleCreateInfo *>(reinterpret_cast<const vku::safe_VkShaderModuleCreateInfo *>(
                        vku::FindStructInPNextChain<VkShaderModuleCreateInfo>(modified_stage_ci->pNext)));

                // TODO - this is in need of testing, when only selecting various library as well as selecting everything
                if (!should_instrument_pipeline &&
                    !IsShaderSelectedForInstrumentation(modified_shader_module_ci, modified_module_state->VkHandle(),
                                                        loc.dot(vvl::Field::pStages, stage_state_i).dot(vvl::Field::module))) {
                    continue;
                }
            }

            // Instrument shader
            // ---
            std::vector<uint32_t> instrumented_spirv;
            const uint32_t unique_shader_id = unique_shader_module_id_++;
            const bool is_shader_instrumented = InstrumentShader(modified_module_state->spirv->words_, unique_shader_id,
                                                                 instrumentation_dsl, loc, instrumented_spirv);

            if (is_shader_instrumented) {
                instrumentation_metadata.unique_shader_id = unique_shader_id;
                need_new_pipeline = true;
            }

            if (modified_module_state->VkHandle() != VK_NULL_HANDLE) {
                // If the user used vkCreateShaderModule, we create a new VkShaderModule to replace with the instrumented
                // shader
                VkShaderModule instrumented_shader_module;
                VkShaderModuleCreateInfo create_info = vku::InitStructHelper();
                if (is_shader_instrumented) {
                    create_info.pCode = instrumented_spirv.data();
                    create_info.codeSize = instrumented_spirv.size() * sizeof(uint32_t);
                } else {
                    // We need to replace the shader regardless as the user may have destroyed the original VkShaderModule and
                    // we will crash trying to unwrap it. So just make a duplicate VkShaderModule. (This is rare we hit this,
                    // only when the user has a shader with nothing to instrument, which tends to be passthrough vertex shaders
                    // which are quick enough to re-create)
                    create_info.pCode = modified_module_state->spirv->words_.data();
                    create_info.codeSize = modified_module_state->spirv->words_.size() * sizeof(uint32_t);
                }
                VkResult result = DispatchCreateShaderModule(device, &create_info, pAllocator, &instrumented_shader_module);
                if (result == VK_SUCCESS) {
                    new_lib_ci.pStages[stage_state_i] = *modified_stage_state.pipeline_create_info;
                    new_lib_ci.pStages[stage_state_i].module = instrumented_shader_module;

                    modified_lib->instrumentation_data.shader_modules.emplace_back(instrumented_shader_module);

                } else {
                    InternalError(device, loc, "Unable to replace non-instrumented shader with instrumented one.");
                    return false;
                }
            } else if (modified_shader_module_ci) {
                // If inlining and not instrumented, leave it alone
                if (is_shader_instrumented) {
                    // The user is inlining the Shader Module into the pipeline, so just need to update the spirv
                    instrumentation_metadata.passed_in_shader_stage_ci = true;
                    // TODO - This makes a copy, but could save on Chassis stack instead (then remove function from VUL).
                    // The core issue is we always use std::vector<uint32_t> but Safe Struct manages its own version of the pCode
                    // memory. It would be much harder to change everything from std::vector and instead to adjust Safe Struct to
                    // not double-free the memory on us. If making any changes, we have to consider a case where the user inlines
                    // the fragment shader, but use a normal VkShaderModule in the vertex shader.
                    modified_shader_module_ci->SetCode(instrumented_spirv);
                }
            } else {
                assert(false);
                return false;
            }
        }

        // Create instrumented pipeline library if we have instrumented one of the libraries inside of it
        if (need_new_pipeline) {
            VkPipeline instrumented_pipeline_lib = VK_NULL_HANDLE;

            // The library could have destroyed its pipelineLayout, but will have a valid, compatible, version when linking
            if (new_lib_ci.layout != VK_NULL_HANDLE && !Get<vvl::PipelineLayout>(new_lib_ci.layout)) {
                new_lib_ci.layout = modified_pipeline_ci.layout;
            }

            const VkResult result = DispatchCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, new_lib_ci.ptr(), pAllocator,
                                                                    &instrumented_pipeline_lib);
            if (result != VK_SUCCESS || instrumented_pipeline_lib == VK_NULL_HANDLE) {
                // could just check result, but being extra cautious around GPL and checking handle as well
                InternalError(device, loc, "Failed to recreate instrumented pipeline library.");
                return false;
            }

            modified_lib->instrumentation_data.instrumented_pipeline_lib = instrumented_pipeline_lib;
            modified_lib->instrumentation_data.was_instrumented = true;
            linked_pipeline_state.instrumentation_data.was_instrumented = true;

            const_cast<VkPipeline *>(modified_library_ci->pLibraries)[modified_lib_i] = instrumented_pipeline_lib;
        }
    }
    return true;
}

void GpuShaderInstrumentor::PostCallRecordPipelineCreationShaderInstrumentationGPL(
    vvl::Pipeline &pipeline_state, std::vector<chassis::ShaderInstrumentationMetadata> &shader_instrumentation_metadata) {
    // if we return early from NeedPipelineCreationShaderInstrumentation, will need to skip at this point in PostCall
    if (shader_instrumentation_metadata.empty()) {
        return;
    }

    uint32_t shader_index = 0;
    // Update instrumented_shaders_map_
    // #ARNO_TODO should be done at instrumentation time, why postpone? We don't need the linked pipeline handle
    for (uint32_t library_i = 0; library_i < pipeline_state.library_create_info->libraryCount; ++library_i) {
        const auto lib = Get<vvl::Pipeline>(pipeline_state.library_create_info->pLibraries[library_i]);
        if (!lib) {
            continue;
        }
        if (lib->stage_states.empty()) {
            continue;
        }

        vku::safe_VkGraphicsPipelineCreateInfo new_lib_pipeline_ci(lib->GraphicsCreateInfo());

        for (uint32_t stage_state_i = 0; stage_state_i < static_cast<uint32_t>(lib->stage_states.size()); ++stage_state_i) {
            auto &instrumentation_metadata = shader_instrumentation_metadata[shader_index++];

            // if the shader for some reason was not instrumented, there is nothing to save
            if (!instrumentation_metadata.IsInstrumented()) {
                continue;
            }

            const auto &stage_state = lib->stage_states[stage_state_i];
            auto &module_state = stage_state.module_state;

            // We currently need to store a copy of the original, non-instrumented shader so if there is debug information,
            // we can reference it by the instruction number printed out in the shader. Since the application can destroy the
            // original VkShaderModule, there is a chance this will be gone, we need to copy it now.
            // TODO - in the instrumentation, instead of printing the instruction number only, if we print out debug info, we
            // can remove this copy
            std::vector<uint32_t> code;
            if (module_state && module_state->spirv) {
                code = module_state->spirv->words_;
            }

            VkShaderModule shader_module_handle = module_state->VkHandle();
            if (shader_module_handle == VK_NULL_HANDLE && instrumentation_metadata.passed_in_shader_stage_ci) {
                shader_module_handle = kPipelineStageInfoHandle;
            }

            instrumented_shaders_map_.insert_or_assign(instrumentation_metadata.unique_shader_id, lib->VkHandle(),
                                                       shader_module_handle, VK_NULL_HANDLE, std::move(code));
        }
    }
}

static bool GpuValidateShader(const std::vector<uint32_t> &input, spv_target_env target_env, std::string &error) {
    // Use SPIRV-Tools validator to try and catch any issues with the module
    spv_context ctx = spvContextCreate(target_env);
    spv_const_binary_t binary{input.data(), input.size()};
    spv_diagnostic diag = nullptr;
    spv_validator_options options = spvValidatorOptionsCreate();
    // Everything is running scalar block layout now (only old 1.0 mobile GPU can't support this)
    spvValidatorOptionsSetScalarBlockLayout(options, true);
    spv_result_t result = spvValidateWithOptions(ctx, options, &binary, &diag);
    if (result != SPV_SUCCESS && diag) error = diag->error;
    return (result == SPV_SUCCESS);
}

// Call the SPIR-V Optimizer to run the instrumentation pass on the shader.
bool GpuShaderInstrumentor::InstrumentShader(const vvl::span<const uint32_t> &input_spirv, uint32_t unique_shader_id,
                                             const InstrumentationDescriptorSetLayouts &instrumentation_dsl, const Location &loc,
                                             std::vector<uint32_t> &out_instrumented_spirv) {
    if (input_spirv[0] != spv::MagicNumber) {
        return false;
    }

    if (unique_shader_id >= glsl::kMaxInstrumentedShaders) {
        InternalWarning(device, loc, "kMaxInstrumentedShaders limit has been hit, no shaders can be instrumented.");
        return false;
    } else if ((input_spirv.size() * sizeof(uint32_t)) > (1 << glsl::kStageId_Shift)) {
        // If we are hitting this, will need to rethink limit (if someone hits this, please raise an issue!)
        InternalWarning(
            device, loc,
            "The shader is larger than 128MB and there are only 27 bits to store the offset into the spirv where an error occurs.");
        return false;
    }

    if (gpuav_settings.debug_dump_instrumented_shaders) {
        const auto non_instrumented_spirv_file = fs::absolute("dump_" + std::to_string(unique_shader_id) + "_before.spv");
        DumpSpirvToFile(non_instrumented_spirv_file.string(), input_spirv.data(), input_spirv.size());
    }

    spirv::Settings module_settings(loc);
    // Use the unique_shader_id as a shader ID so we can look up its handle later in the shader_map.
    module_settings.shader_id = unique_shader_id;
    module_settings.output_buffer_descriptor_set = instrumentation_desc_set_bind_index_;
    module_settings.safe_mode = gpuav_settings.safe_mode;
    module_settings.print_debug_info = gpuav_settings.debug_print_instrumentation_info;
    module_settings.max_instrumentations_count = gpuav_settings.debug_max_instrumentations_count;
    module_settings.support_non_semantic_info =
        IsExtEnabled(extensions.vk_khr_shader_non_semantic_info) && !IsExtEnabled(extensions.vk_khr_portability_subset);
    module_settings.has_bindless_descriptors = instrumentation_dsl.has_bindless_descriptors;
    // Ray tracing pipeline flags for VUIDs 11886/11887
    module_settings.pipeline_has_skip_aabbs_flag = instrumentation_dsl.pipeline_has_skip_aabbs_flag;
    module_settings.pipeline_has_skip_triangles_flag = instrumentation_dsl.pipeline_has_skip_triangles_flag;
    // For VUID 11888 - maxShaderBindingTableRecordIndex limit
    module_settings.max_shader_binding_table_record_index = instrumentation_dsl.max_shader_binding_table_record_index;

    spirv::Module module(input_spirv, debug_report, module_settings, modified_features,
                         instrumentation_dsl.set_index_to_bindings_layout_lut);

    bool modified = false;

    // If descriptor indexing is enabled, enable length checks and updated descriptor checks
    if (gpuav_settings.shader_instrumentation.descriptor_checks) {
        // Will wrap descriptor indexing with if/else to prevent crashing if OOB
        spirv::DescriptorIndexingOOBPass oob_pass(module);
        modified |= oob_pass.Run();

        // Depending on the DescriptorClass, will add dedicated check
        if (!modified_features.robustBufferAccess) {
            // This check is for catching OOB in a UBO/SSBO which is caught with robustBufferAccess
            spirv::DescriptorClassGeneralBufferPass general_buffer_pass(module);
            modified |= general_buffer_pass.Run();

            // Details being worked out in https://gitlab.khronos.org/vulkan/vulkan/-/issues/3977
            // But for what we are checking for, can rely on robustBufferAccess
            spirv::DescriptorClassTexelBufferPass texel_buffer_pass(module);
            modified |= texel_buffer_pass.Run();
        } else if (modified_features.cooperativeMatrix && !modified_features.cooperativeMatrixRobustBufferAccess) {
            // Cooperative Matrix OOB rules are unique and have their own robustness feature, so still need to run the pass
            spirv::DescriptorClassGeneralBufferPass general_buffer_pass(module);
            modified |= general_buffer_pass.Run();
        }
    }

    if (gpuav_settings.shader_instrumentation.buffer_device_address) {
        spirv::BufferDeviceAddressPass pass(module);
        modified |= pass.Run();
    }

    if (gpuav_settings.shader_instrumentation.ray_query) {
        spirv::RayQueryPass pass(module);
        modified |= pass.Run();
    }

    if (gpuav_settings.shader_instrumentation.ray_hit_object) {
        spirv::RayHitObjectPass pass(module);
        modified |= pass.Run();
    }

    if (gpuav_settings.shader_instrumentation.mesh_shading) {
        spirv::MeshShading pass(module);
        modified |= pass.Run();
    }

    // Post Process instrumentation passes assume the things inside are valid, but putting at the end, things above will wrap checks
    // in a if/else, this means they will be gaurded as if they were inside the above passes
    if (gpuav_settings.shader_instrumentation.post_process_descriptor_indexing) {
        spirv::PostProcessDescriptorIndexingPass pass(module);
        modified |= pass.Run();
    }

    if (gpuav_settings.shader_instrumentation.vertex_attribute_fetch_oob) {
        if (!modified_features.robustBufferAccess) {
            spirv::VertexAttributeFetchOobPass pass(module);
            modified |= pass.Run();
        }
    }

    // Currently only runs in Safe Mode until have perf numbers how costly it is to enable
    if (gpuav_settings.shader_instrumentation.sanitizer && gpuav_settings.safe_mode) {
        spirv::SanitizerPass pass(module);
        modified |= pass.Run();
    }

    // If we have passes that require inject LogError before the shader end we do it now.
    // We have a dedicated pass to ensure the LogError is only added once
    if (module.need_log_error_) {
        spirv::LogErrorPass log_error_pass(module);
        modified |= log_error_pass.Run();
    }

    // If there were GLSL written function injected, we will grab them and link them in here
    for (const auto &info : module.link_infos_) {
        module.LinkFunctions(info);
    }

    // DebugPrintf goes at the end for 2 reasons:
    // 1. We use buffer device address in it and we don't want to validate the inside of this pass
    // 2. We might want to debug the above passes and want to inject our own debug printf calls
    if (gpuav_settings.debug_printf_enabled) {
        // binding slot allows debug printf to be slotted in the same set as GPU-AV if needed
        spirv::DebugPrintfPass pass(module, internal_only_debug_printf_, glsl::kBindingInstDebugPrintf);
        modified |= pass.Run();
    }

    // If nothing was instrumented, leave early to save time
    if (!modified) {
        return false;
    }

    // some small cleanup to make sure SPIR-V is legal
    module.PostProcess();
    // translate internal representation of SPIR-V into legal SPIR-V binary
    module.ToBinary(out_instrumented_spirv);

    // (Maybe) validate the instrumented and linked shader
    bool is_instrumented_spirv_valid = true;
    if (gpuav_settings.debug_validate_instrumented_shaders) {
        std::string spirv_val_error;

        spv_target_env target_env = PickSpirvEnv(api_version, IsExtEnabled(extensions.vk_khr_spirv_1_4));
        is_instrumented_spirv_valid = GpuValidateShader(out_instrumented_spirv, target_env, spirv_val_error);
        if (!is_instrumented_spirv_valid) {
            if (!gpuav_settings.debug_dump_instrumented_shaders) {
                const auto non_instrumented_spirv_file = fs::absolute("dump_" + std::to_string(unique_shader_id) + "_before.spv");
                DumpSpirvToFile(non_instrumented_spirv_file.string(), input_spirv.data(), input_spirv.size());
            }

            const auto instrumented_spirv_file = fs::absolute("dump_" + std::to_string(unique_shader_id) + "_after_invalid.spv");
            DumpSpirvToFile(instrumented_spirv_file.string(), out_instrumented_spirv.data(), out_instrumented_spirv.size());

            std::ostringstream strm;
            const auto invalid_file_path = std::filesystem::absolute(instrumented_spirv_file);
            strm << "Instrumented shader (id " << unique_shader_id << ") is invalid, spirv-val error:\n"
                 << spirv_val_error << "\nInvalid spirv dumped to " << invalid_file_path
                 << "\nProceeding with non instrumented shader.";
            InternalError(device, loc, strm.str().c_str());
            return false;
        }
    }
    if (is_instrumented_spirv_valid && gpuav_settings.debug_dump_instrumented_shaders) {
        const auto instrumented_spirv_file = fs::absolute("dump_" + std::to_string(unique_shader_id) + "_after.spv");
        DumpSpirvToFile(instrumented_spirv_file.string(), out_instrumented_spirv.data(), out_instrumented_spirv.size());
    }

    return true;
}

void GpuShaderInstrumentor::InternalError(LogObjectList objlist, const Location &loc, const char *const specific_message) const {
    aborted_ = true;
    std::string error_message = specific_message;

    const char *layer_name = gpuav_settings.debug_printf_only ? "DebugPrintf" : "GPU-AV";
    const char *vuid = gpuav_settings.debug_printf_only ? "UNASSIGNED-DEBUG-PRINTF" : "UNASSIGNED-GPU-Assisted-Validation";

    LogError(vuid, objlist, loc, "Internal Error, %s is being disabled. Details:\n%s", layer_name, error_message.c_str());

    // Once we encounter an internal issue disconnect everything.
    // This prevents need to check "if (aborted)" (which is awful when we easily forget to check somewhere and the user gets spammed
    // with errors making it hard to see the first error with the real source of the problem).
    dispatch_device_->ReleaseValidationObject(LayerObjectTypeGpuAssisted);
}

// Dedicated warning VUID that likely can be ignored.
// We want to always warn the user when adjusting settings/limits/features/etc on them
void GpuShaderInstrumentor::AdjustmentWarning(LogObjectList objlist, const Location &loc,
                                              const char *const specific_message) const {
    LogWarning("WARNING-Setting-Limit-Adjusted", objlist, loc, "Warning that validation is adjusting settings:\n%s",
               specific_message);
}

void GpuShaderInstrumentor::InternalWarning(LogObjectList objlist, const Location &loc, const char *const specific_message) const {
    const char *vuid = gpuav_settings.debug_printf_only ? "WARNING-DEBUG-PRINTF" : "WARNING-GPU-Assisted-Validation";
    LogWarning(vuid, objlist, loc, "Internal Warning: %s", specific_message);
}

void GpuShaderInstrumentor::InternalInfo(LogObjectList objlist, const Location &loc, const char *const specific_message) const {
    const char *vuid = gpuav_settings.debug_printf_only ? "INFO-DEBUG-PRINTF" : "INFO-GPU-Assisted-Validation";
    LogInfo(vuid, objlist, loc, "Internal Info: %s", specific_message);
}

// The lock (debug_output_mutex) is held by the caller,
// because the latter has code paths that make multiple calls of this function,
// and all such calls have to access the same debug reporting state to ensure consistency of output information.
static std::string LookupDebugUtilsNameNoLock(const DebugReport *debug_report, const uint64_t object) {
    auto object_label = debug_report->GetUtilsObjectNameNoLock(object);
    if (object_label != "") {
        object_label = "(" + object_label + ")";
    }
    return object_label;
}

// Generate the stage-specific part of the message.
static void GenerateStageMessage(std::ostringstream &ss, const GpuShaderInstrumentor::ShaderMessageInfo &shader_info,
                                 const std::vector<uint32_t> &instructions) {
    switch (shader_info.stage_id) {
        case glsl::kExecutionModel_MultiEntryPoint: {
            ss << "Stage has multiple OpEntryPoint (";
            ::spirv::GetExecutionModelNames(instructions, ss);
            ss << ") and could not detect stage. ";
        } break;
        case glsl::kExecutionModel_Vertex: {
            ss << "Stage = Vertex. Vertex Index = " << shader_info.stage_info_0 << " Instance Index = " << shader_info.stage_info_1
               << ". ";
        } break;
        case glsl::kExecutionModel_TessellationControl: {
            ss << "Stage = Tessellation Control.  Invocation ID = " << shader_info.stage_info_0
               << ", Primitive ID = " << shader_info.stage_info_1;
        } break;
        case glsl::kExecutionModel_TessellationEvaluation: {
            ss << "Stage = Tessellation Eval.  Primitive ID = " << shader_info.stage_info_0 << ", TessCoord (u, v) = ("
               << shader_info.stage_info_1 << ", " << shader_info.stage_info_2 << "). ";
        } break;
        case glsl::kExecutionModel_Geometry: {
            ss << "Stage = Geometry.  Primitive ID = " << shader_info.stage_info_0
               << " Invocation ID = " << shader_info.stage_info_1 << ". ";
        } break;
        case glsl::kExecutionModel_Fragment: {
            // Should use std::bit_cast but requires c++20
            float x_coord;
            float y_coord;
            std::memcpy(&x_coord, &shader_info.stage_info_0, sizeof(float));
            std::memcpy(&y_coord, &shader_info.stage_info_1, sizeof(float));
            ss << "Stage = Fragment.  Fragment coord (x,y) = (" << x_coord << ", " << y_coord << "). ";
        } break;
        case glsl::kExecutionModel_GLCompute: {
            ss << "Stage = Compute.  Global invocation ID (x, y, z) = (" << shader_info.stage_info_0 << ", "
               << shader_info.stage_info_1 << ", " << shader_info.stage_info_2 << ")";
        } break;
        case glsl::kExecutionModel_RayGenerationKHR: {
            ss << "Stage = Ray Generation.  Global Launch ID (x,y,z) = (" << shader_info.stage_info_0 << ", "
               << shader_info.stage_info_1 << ", " << shader_info.stage_info_2 << "). ";
        } break;
        case glsl::kExecutionModel_IntersectionKHR: {
            ss << "Stage = Intersection.  Global Launch ID (x,y,z) = (" << shader_info.stage_info_0 << ", "
               << shader_info.stage_info_1 << ", " << shader_info.stage_info_2 << "). ";
        } break;
        case glsl::kExecutionModel_AnyHitKHR: {
            ss << "Stage = Any Hit.  Global Launch ID (x,y,z) = (" << shader_info.stage_info_0 << ", " << shader_info.stage_info_1
               << ", " << shader_info.stage_info_2 << "). ";
        } break;
        case glsl::kExecutionModel_ClosestHitKHR: {
            ss << "Stage = Closest Hit.  Global Launch ID (x,y,z) = (" << shader_info.stage_info_0 << ", "
               << shader_info.stage_info_1 << ", " << shader_info.stage_info_2 << "). ";
        } break;
        case glsl::kExecutionModel_MissKHR: {
            ss << "Stage = Miss.  Global Launch ID (x,y,z) = (" << shader_info.stage_info_0 << ", " << shader_info.stage_info_1
               << ", " << shader_info.stage_info_2 << "). ";
        } break;
        case glsl::kExecutionModel_CallableKHR: {
            ss << "Stage = Callable.  Global Launch ID (x,y,z) = (" << shader_info.stage_info_0 << ", " << shader_info.stage_info_1
               << ", " << shader_info.stage_info_2 << "). ";
        } break;
        case glsl::kExecutionModel_TaskEXT: {
            ss << "Stage = TaskEXT. Global invocation ID (x, y, z) = (" << shader_info.stage_info_0 << ", "
               << shader_info.stage_info_1 << ", " << shader_info.stage_info_2 << ")";
        } break;
        case glsl::kExecutionModel_MeshEXT: {
            ss << "Stage = MeshEXT. Global invocation ID (x, y, z) = (" << shader_info.stage_info_0 << ", "
               << shader_info.stage_info_1 << ", " << shader_info.stage_info_2 << ")";
        } break;
        case glsl::kExecutionModel_TaskNV: {
            ss << "Stage = TaskNV. Global invocation ID (x, y, z) = (" << shader_info.stage_info_0 << ", "
               << shader_info.stage_info_1 << ", " << shader_info.stage_info_2 << ")";
        } break;
        case glsl::kExecutionModel_MeshNV: {
            ss << "Stage = MeshNV. Global invocation ID (x, y, z) = (" << shader_info.stage_info_0 << ", "
               << shader_info.stage_info_1 << ", " << shader_info.stage_info_2 << ")";
        } break;
        default: {
            ss << "Internal Error (unexpected stage = " << shader_info.stage_id << "). ";
            assert(false);
        } break;
    }
    ss << '\n';
}

// Where we build up the error message with all the useful debug information about where the error occured
std::string GpuShaderInstrumentor::GenerateDebugInfoMessage(VkCommandBuffer commandBuffer, const ShaderMessageInfo &shader_info,
                                                            const InstrumentedShader *instrumented_shader,
                                                            VkPipelineBindPoint pipeline_bind_point,
                                                            uint32_t action_command_index) const {
    std::ostringstream ss;
    if (!instrumented_shader || instrumented_shader->original_spirv.empty()) {
        ss << "[Internal Error] - Can't get instructions from shader_map\n";
        return ss.str();
    }

    GenerateStageMessage(ss, shader_info, instrumented_shader->original_spirv);

    ss << std::hex << std::showbase;
    if (instrumented_shader->shader_module == VK_NULL_HANDLE && instrumented_shader->shader_object == VK_NULL_HANDLE) {
        std::unique_lock<std::mutex> lock(debug_report->debug_output_mutex);
        ss << "[Internal Error] - Unable to locate shader/pipeline handles used in command buffer "
           << LookupDebugUtilsNameNoLock(debug_report, HandleToUint64(commandBuffer)) << "(" << HandleToUint64(commandBuffer)
           << ")\n";
        assert(true);
    } else {
        std::unique_lock<std::mutex> lock(debug_report->debug_output_mutex);
        ss << "Command buffer " << LookupDebugUtilsNameNoLock(debug_report, HandleToUint64(commandBuffer)) << "("
           << HandleToUint64(commandBuffer) << ")\n";
        ss << std::dec << std::noshowbase;
        ss << '\t';  // helps to show that the index is expressed with respect to the command buffer
        if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
            ss << "Draw ";
        } else if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
            ss << "Compute Dispatch ";
        } else if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
            ss << "Ray Trace ";
        } else {
            assert(false);
            ss << "Unknown Pipeline Operation ";
        }

        uint32_t invalid_index_command = dispatch_instance_->settings.gpuav_settings.GetInvalidIndexCommand();
        if (action_command_index == invalid_index_command) {
            ss << "Index Unknown (After " << invalid_index_command << " commands, we stop tracking) \n";
        } else {
            ss << "Index " << action_command_index << '\n';
        }
        ss << std::hex << std::noshowbase;

        if (instrumented_shader->shader_module == VK_NULL_HANDLE) {
            ss << "Shader Object " << LookupDebugUtilsNameNoLock(debug_report, HandleToUint64(instrumented_shader->shader_object))
               << "(0x" << HandleToUint64(instrumented_shader->shader_object) << ") (internal ID " << std::dec
               << shader_info.shader_id << ")\n";
        } else {
            if (instrumented_shader->shader_module == kPipelineStageInfoHandle) {
                ss << "Shader Module was passed in via VkPipelineShaderStageCreateInfo::pNext (internal ID " << std::dec
                   << shader_info.shader_id << ")\n";
            } else {
                ss << "Shader Module "
                   << LookupDebugUtilsNameNoLock(debug_report, HandleToUint64(instrumented_shader->shader_module)) << "(0x"
                   << HandleToUint64(instrumented_shader->shader_module) << ") (internal ID " << std::dec << shader_info.shader_id
                   << ")\n";
            }
        }
    }
    ss << std::dec << std::noshowbase;

    ::spirv::FindShaderSource(ss, instrumented_shader->original_spirv, shader_info.instruction_position_offset,
                              gpuav_settings.debug_printf_only);

    return ss.str();
}

}  // namespace gpuav
