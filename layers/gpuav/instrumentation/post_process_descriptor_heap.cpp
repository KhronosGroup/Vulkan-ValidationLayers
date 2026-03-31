/* Copyright (c) 2025-2026 LunarG, Inc.
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

#include "gpuav/instrumentation/post_process_descriptor_heap.h"

#include "gpuav/core/gpuav.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "drawdispatch/descriptor_heap_validator.h"
#include "state_tracker/shader_module.h"
#include "gpuav/shaders/gpuav_error_codes.h"
#include "gpuav/shaders/gpuav_error_header.h"
#include "generated/spirv_validation_helper.h"
#include "utils/math_utils.h"

namespace gpuav {

struct PostProcessingHeapCbState {
    vko::BufferRange post_process_variables;
};

struct HeapSizes {
    uint32_t resource_heap_size;
    uint32_t resource_reserved_offset;
    uint32_t resource_reserved_size;
    uint32_t sampler_heap_size;
    uint32_t sampler_reserved_offset;
    uint32_t sampler_reserved_size;
};

struct ValidDescriptors {
    vko::BufferRange descriptors;
    uint8_t* resource_heap_memory;
    uint8_t* sampler_heap_memory;
};

struct Slot {
    // see gpuav_shaders_constants.h for how we split this metadata up
    uint32_t meta_data;
    // OpVariable ID of descriptor accessed.
    // This is required to distinguish between 2 aliased descriptors
    uint32_t variable_id;
    // Used in order to print out information about which instruction caused the issue
    uint32_t instruction_position_offset;
    // Last accessed byte
    uint32_t byte_offset;
};

void RegisterPostProcessingDescriptorHeap(Validator& gpuav, CommandBufferSubState& cb) {
    if (!gpuav.gpuav_settings.shader_instrumentation.descriptor_heap) {
        return;
    }

    cb.on_instrumentation_common_desc_update_functions.emplace_back(
        [](CommandBufferSubState& cb, VkPipelineBindPoint, const Location& loc, CommonDescriptorUpdate& out_update) mutable {
            PostProcessingHeapCbState& pp_cb_state = cb.shared_resources_cache.GetOrCreate<PostProcessingHeapCbState>();
            pp_cb_state.post_process_variables =
                cb.gpu_resources_manager.GetHostCoherentBufferRange(sizeof(Slot) * kDebugMaxDescSetAndBindings);

            out_update.buffer = pp_cb_state.post_process_variables.buffer;
            out_update.offset = pp_cb_state.post_process_variables.offset;
            out_update.range = pp_cb_state.post_process_variables.size;
            out_update.address = pp_cb_state.post_process_variables.offset_address;
            out_update.binding = glsl::kBindingInstDescriptorHeapPostProcess;
            memset(pp_cb_state.post_process_variables.offset_mapped_ptr, 0, sizeof(VkDeviceAddressRangeEXT));
        });

    gpuav::vko::IndirectAccessMap indirect_access_map =
        std::make_shared<vvl::unordered_map<vko::IndirectKey, vko::StagingBuffer, gpuav::vko::IndirectKeyHash>>();
    cb.on_post_cb_submission_functions.emplace_back([indirect_access_map](Validator& gpuav, CommandBufferSubState& cb,
                                                                          VkCommandBuffer per_post_submission_cb) {
        PostProcessingHeapCbState* pp_cb_state = cb.shared_resources_cache.TryGet<PostProcessingHeapCbState>();
        if (pp_cb_state) {
            DispatchDeviceWaitIdle(gpuav.device);  // Todo
            const Slot* data = reinterpret_cast<Slot*>(pp_cb_state->post_process_variables.offset_mapped_ptr);
            if (!data) {
                return;
            }
            for (uint32_t i = 0; i < kDebugMaxDescSetAndBindings; ++i) {
                const Slot& slot = data[i];
                if ((slot.meta_data & glsl::kPostProcessMetaMaskAccessed) == 0) {
                    break;
                }

                uint32_t shader_id = slot.meta_data & glsl::kShaderIdMask;

                auto it = gpuav.instrumented_shaders_map_.find(shader_id);
                if (it == gpuav.instrumented_shaders_map_.end()) {
                    assert(false);
                    continue;
                }

                const ::spirv::ResourceInterfaceVariable* resource_variable = nullptr;
                const VkShaderDescriptorSetAndBindingMappingInfoEXT* mappings = nullptr;
                VkShaderStageFlagBits stage = VK_SHADER_STAGE_ALL;
                if (it->second.pipeline != VK_NULL_HANDLE) {
                    // We use pipeline over vkShaderModule as likely they will have been destroyed by now
                    const vvl::Pipeline* pipeline_state = gpuav.Get<vvl::Pipeline>(it->second.pipeline).get();
                    for (const ShaderStageState& stage_state : pipeline_state->stage_states) {
                        auto variable_it = stage_state.entrypoint->resource_interface_variable_map.find(slot.variable_id);
                        if (variable_it != stage_state.entrypoint->resource_interface_variable_map.end()) {
                            mappings =
                                vku::FindStructInPNextChain<VkShaderDescriptorSetAndBindingMappingInfoEXT>(stage_state.GetPNext());
                            resource_variable = variable_it->second;
                            stage = stage_state.GetStage();
                            break;
                        }
                    }
                } else if (it->second.shader_object != VK_NULL_HANDLE) {
                    const vvl::ShaderObject* shader_object_state = gpuav.Get<vvl::ShaderObject>(it->second.shader_object).get();
                    ASSERT_AND_CONTINUE(shader_object_state->stage.entrypoint);
                    mappings = vku::FindStructInPNextChain<VkShaderDescriptorSetAndBindingMappingInfoEXT>(
                        shader_object_state->create_info.pNext);
                    auto variable_it =
                        shader_object_state->stage.entrypoint->resource_interface_variable_map.find(slot.variable_id);
                    if (variable_it != shader_object_state->stage.entrypoint->resource_interface_variable_map.end()) {
                        resource_variable = variable_it->second;
                        stage = shader_object_state->create_info.stage;
                    }
                }
                ASSERT_AND_CONTINUE(resource_variable);
                ASSERT_AND_CONTINUE(mappings);

                for (uint32_t j = 0; j < mappings->mappingCount; ++j) {
                    const auto& mapping = mappings->pMappings[j];
                    if (mapping.descriptorSet != resource_variable->decorations.set ||
                        mapping.firstBinding != resource_variable->decorations.binding) {
                        continue;
                    }
                    bool shader_record = false;
                    uint32_t push_offset = 0u;
                    uint32_t address_offset = 0u;
                    uint32_t size = 0u;
                    if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT) {
                        if (mapping.resourceMask == VK_SPIRV_RESOURCE_TYPE_SAMPLER_BIT_EXT) {
                            push_offset = mapping.sourceData.indirectIndex.samplerPushOffset;
                            address_offset = mapping.sourceData.indirectIndex.samplerAddressOffset;
                        } else {
                            push_offset = mapping.sourceData.indirectIndex.pushOffset;
                            address_offset = mapping.sourceData.indirectIndex.addressOffset;
                        }
                        size = sizeof(uint32_t);
                    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT) {
                        // Todo, factor in array index
                        if (mapping.resourceMask == VK_SPIRV_RESOURCE_TYPE_SAMPLER_BIT_EXT) {
                            push_offset = mapping.sourceData.indirectIndexArray.samplerPushOffset;
                            address_offset = mapping.sourceData.indirectIndexArray.samplerAddressOffset;
                        } else {
                            push_offset = mapping.sourceData.indirectIndexArray.pushOffset;
                            address_offset = mapping.sourceData.indirectIndexArray.addressOffset;
                        }
                        size = sizeof(uint32_t);
                    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT) {
                        push_offset = mapping.sourceData.indirectAddress.pushOffset;
                        address_offset = mapping.sourceData.indirectAddress.addressOffset;
                        size = sizeof(VkDeviceAddress);
                    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_ADDRESS_EXT) {
                        shader_record = true;
                        address_offset = mapping.sourceData.shaderRecordAddressOffset;
                        size = sizeof(VkDeviceAddress);
                    } else {
                        continue;
                    }

                    VkDeviceAddress device_address = cb.GetPushData<VkDeviceAddress>(push_offset);
                    // Temporary to mute warnings, until validation that requires this field is added
                    (void)stage;

                    vko::IndirectKey key = {shader_record, push_offset, address_offset};
                    vko::StagingBuffer staging_buffer(cb.gpu_resources_manager, size, per_post_submission_cb);
                    indirect_access_map->insert({key, staging_buffer});

                    if (shader_record) {
                        address_offset += gpuav.phys_dev_ext_props.ray_tracing_props_khr.shaderGroupHandleSize;
                    }

                    const auto buffers = gpuav.GetBuffersByAddress(device_address);
                    if (buffers.size() == 1) {
                        VkBufferMemoryBarrier barrier_write_after_read = vku::InitStructHelper();
                        barrier_write_after_read.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                        barrier_write_after_read.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                        barrier_write_after_read.buffer = buffers[0]->VkHandle();
                        barrier_write_after_read.offset = address_offset;
                        barrier_write_after_read.size = size;

                        DispatchCmdPipelineBarrier(per_post_submission_cb, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                                   VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &barrier_write_after_read, 0,
                                                   nullptr);
                        VkBufferCopy copy;
                        copy.srcOffset = address_offset;
                        copy.dstOffset = staging_buffer.GetBufferRange().offset;
                        copy.size = size;
                        DispatchCmdCopyBuffer(per_post_submission_cb, buffers[0]->VkHandle(),
                                              staging_buffer.GetBufferRange().buffer, 1, &copy);

                        VkBufferMemoryBarrier barrier_read_before_write = vku::InitStructHelper();
                        barrier_read_before_write.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                        barrier_read_before_write.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                        barrier_read_before_write.buffer = buffers[0]->VkHandle();
                        barrier_read_before_write.offset = address_offset;
                        barrier_read_before_write.size = size;

                        DispatchCmdPipelineBarrier(per_post_submission_cb, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                   VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 1, &barrier_read_before_write,
                                                   0, nullptr);

                        staging_buffer.CmdCopyDeviceToHost(per_post_submission_cb);
                    }
                }
            }
        }
    });

    cb.on_cb_completion_functions.emplace_back([indirect_access_map](Validator& gpuav, CommandBufferSubState& cb,
                                                                     const CommandBufferSubState::LabelLogging& label_logging,
                                                                     const Location& submission_loc) {
        PostProcessingHeapCbState* pp_cb_state = cb.shared_resources_cache.TryGet<PostProcessingHeapCbState>();
        if (pp_cb_state) {
            const Slot* data = reinterpret_cast<Slot*>(pp_cb_state->post_process_variables.offset_mapped_ptr);
            if (!data) {
                return false;
            }
            std::vector<const ::spirv::ResourceInterfaceVariable*> validated_resource_variables;
            for (uint32_t i = 0; i < kDebugMaxDescSetAndBindings; ++i) {
                const Slot& slot = data[i];
                if ((slot.meta_data & glsl::kPostProcessMetaMaskAccessed) == 0) {
                    break;
                }

                uint32_t shader_id = slot.meta_data & glsl::kShaderIdMask;

                const uint32_t error_logger_i =
                    (slot.meta_data & glsl::kPostProcessMetaMaskErrorLoggerIndex) >> glsl::kPostProcessMetaShiftErrorLoggerIndex;
                const CommandBufferSubState::CommandErrorLogger& cmd_error_logger = cb.GetErrorLogger(error_logger_i);
                std::string debug_region_name =
                    cb.GetDebugLabelRegion(cmd_error_logger.label_cmd_i, label_logging.initial_label_stack);

                Location access_loc(cmd_error_logger.loc.Get(), debug_region_name);
                vvl::DescriptorHeapValidator context(gpuav, cb, 0, nullptr, access_loc);
                context.SetObjlistForGpuAv(&cmd_error_logger.objlist);

                const ::spirv::ResourceInterfaceVariable* resource_variable = nullptr;

                const vvl::Pipeline* pipeline_state = nullptr;
                const vvl::ShaderObject* shader_object_state = nullptr;

                auto it = gpuav.instrumented_shaders_map_.find(shader_id);
                if (it == gpuav.instrumented_shaders_map_.end()) {
                    assert(false);
                    continue;
                }

                if (it->second.pipeline != VK_NULL_HANDLE) {
                    // We use pipeline over vkShaderModule as likely they will have been destroyed by now
                    pipeline_state = gpuav.Get<vvl::Pipeline>(it->second.pipeline).get();
                } else if (it->second.shader_object != VK_NULL_HANDLE) {
                    shader_object_state = gpuav.Get<vvl::ShaderObject>(it->second.shader_object).get();
                    ASSERT_AND_CONTINUE(shader_object_state->stage.entrypoint);
                } else {
                    assert(false);
                    continue;
                }

                const VkShaderDescriptorSetAndBindingMappingInfoEXT* mappings = nullptr;
                bool pipeline = false;
                bool robustness = false;

                if (pipeline_state) {
                    for (const ShaderStageState& stage_state : pipeline_state->stage_states) {
                        ASSERT_AND_CONTINUE(stage_state.entrypoint);
                        auto variable_it = stage_state.entrypoint->resource_interface_variable_map.find(slot.variable_id);
                        if (variable_it != stage_state.entrypoint->resource_interface_variable_map.end()) {
                            mappings =
                                vku::FindStructInPNextChain<VkShaderDescriptorSetAndBindingMappingInfoEXT>(stage_state.GetPNext());
                            resource_variable = variable_it->second;
                            pipeline = true;
                            if (pipeline_state->uses_pipeline_robustness) {
                                robustness = true;
                            }
                            break;  // Only need to find a single entry point
                        }
                    }
                } else if (shader_object_state) {
                    ASSERT_AND_CONTINUE(shader_object_state->stage.entrypoint);
                    mappings = vku::FindStructInPNextChain<VkShaderDescriptorSetAndBindingMappingInfoEXT>(
                        shader_object_state->create_info.pNext);
                    auto variable_it =
                        shader_object_state->stage.entrypoint->resource_interface_variable_map.find(slot.variable_id);
                    if (variable_it != shader_object_state->stage.entrypoint->resource_interface_variable_map.end()) {
                        resource_variable = variable_it->second;
                    }
                }
                ASSERT_AND_CONTINUE(resource_variable);
                ASSERT_AND_CONTINUE(mappings);
                if (std::find(validated_resource_variables.begin(), validated_resource_variables.end(), resource_variable) !=
                    validated_resource_variables.end()) {
                    continue;
                }

                context.ValidateBinding(gpuav, *resource_variable, *mappings, indirect_access_map, slot.byte_offset, pipeline,
                                        robustness);
                validated_resource_variables.push_back(resource_variable);
            }
        }
        return true;
    });
}

}  // namespace gpuav
