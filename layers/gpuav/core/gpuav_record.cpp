/* Copyright (c) 2018-2025 The Khronos Group Inc.
 * Copyright (c) 2018-2025 Valve Corporation
 * Copyright (c) 2018-2025 LunarG, Inc.
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

#include "chassis/chassis_modification_state.h"
#include "gpuav/core/gpuav.h"
#include "gpuav/debug_printf/debug_printf.h"
#include "gpuav/descriptor_validation/gpuav_descriptor_validation.h"
#include "gpuav/instrumentation/buffer_device_address.h"
#include "gpuav/instrumentation/descriptor_checks.h"
#include "gpuav/instrumentation/gpuav_instrumentation.h"
#include "gpuav/instrumentation/post_process_descriptor_indexing.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "gpuav/validation_cmd/gpuav_copy_buffer_to_image.h"
#include "gpuav/validation_cmd/gpuav_dispatch.h"
#include "gpuav/validation_cmd/gpuav_draw.h"
#include "gpuav/validation_cmd/gpuav_trace_rays.h"
#include "utils/math_utils.h"

namespace gpuav {

void Validator::PreCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer,
                                          const RecordObject &record_obj, chassis::CreateBuffer &chassis_state) {
    const auto *flags2 = vku::FindStructInPNextChain<VkBufferUsageFlags2CreateInfo>(chassis_state.modified_create_info.pNext);
    const VkBufferUsageFlags2 in_usage = flags2 ? flags2->usage : chassis_state.modified_create_info.usage;

    // Ray tracing acceleration structure instance buffers also need the storage buffer usage as
    // acceleration structure build validation will find and replace invalid acceleration structure
    // handles inside of a compute shader.
    if (in_usage & VK_BUFFER_USAGE_2_SHADER_BINDING_TABLE_BIT_KHR) {
        if (flags2) {
            const_cast<VkBufferUsageFlags2CreateInfo *>(flags2)->usage |= VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT;
        } else {
            chassis_state.modified_create_info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        }
    }

    // Indirect buffers will require validation shader to bind the indirect buffers as a storage buffer.
    if (gpuav_settings.IsBufferValidationEnabled() &&
        (in_usage & (VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_2_INDEX_BUFFER_BIT))) {
        if (flags2) {
            const_cast<VkBufferUsageFlags2CreateInfo *>(flags2)->usage |= VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT;
        } else {
            chassis_state.modified_create_info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        }
    }

    // Align index buffer size to 4: validation shader reads DWORDS
    if (gpuav_settings.IsBufferValidationEnabled()) {
        chassis_state.modified_create_info.size = Align<VkDeviceSize>(chassis_state.modified_create_info.size, 4);
    }
}

void Validator::PreCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo,
                                                const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }

    CommandBufferSubState &gpuav_cb_state = SubState(*cb_state);
    RegisterDescriptorChecksValidation(*this, gpuav_cb_state);
    RegisterPostProcessingValidation(*this, gpuav_cb_state);
    RegisterBufferDeviceAddressValidation(*this, gpuav_cb_state);
    debug_printf::RegisterDebugPrintf(*this, gpuav_cb_state);
}

// Dedicated warning VUID that likely can be ignored. We want to always warn the user when adjusting settings/limits/features/etc on
// them
void Instance::AdjustmentWarning(LogObjectList objlist, const Location &loc, const char *const specific_message) const {
    LogWarning("WARNING-Setting-Limit-Adjusted", objlist, loc, "Internal Warning: %s", specific_message);
}

void Instance::InternalWarning(LogObjectList objlist, const Location &loc, const char *const specific_message) const {
    const char *vuid = gpuav_settings.debug_printf_only ? "WARNING-DEBUG-PRINTF" : "WARNING-GPU-Assisted-Validation";
    LogWarning(vuid, objlist, loc, "Internal Warning: %s", specific_message);
}

void Instance::ReserveBindingSlot(VkPhysicalDevice physicalDevice, VkPhysicalDeviceLimits &limits, const Location &loc) {
    // There is an implicit layer that can cause this call to return 0 for maxBoundDescriptorSets - Ignore such calls
    if (limits.maxBoundDescriptorSets == 0) return;

    if (limits.maxBoundDescriptorSets > kMaxAdjustedBoundDescriptorSet) {
        std::stringstream ss;
        ss << "A descriptor binding slot is required to store GPU-side information, but the device maxBoundDescriptorSets is "
           << limits.maxBoundDescriptorSets << " which is too large, so we will be trying to use slot "
           << kMaxAdjustedBoundDescriptorSet;
        InternalWarning(physicalDevice, loc, ss.str().c_str());
    }

    if (enabled[gpu_validation_reserve_binding_slot]) {
        if (limits.maxBoundDescriptorSets > 1) {
            limits.maxBoundDescriptorSets -= 1;
        } else {
            InternalWarning(physicalDevice, loc, "Unable to reserve descriptor binding slot on a device with only one slot.");
        }
    }
}

void Instance::PostCallRecordGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties *device_props,
                                                         const RecordObject &record_obj) {
    ReserveBindingSlot(physicalDevice, device_props->limits, record_obj.location);
}

void Instance::PostCallRecordGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                                          VkPhysicalDeviceProperties2 *device_props2,
                                                          const RecordObject &record_obj) {
    // override all possible places maxUpdateAfterBindDescriptorsInAllPools can be set
    auto *desc_indexing_props = vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorIndexingProperties>(device_props2->pNext);
    if (desc_indexing_props &&
        desc_indexing_props->maxUpdateAfterBindDescriptorsInAllPools > glsl::kDebugInputBindlessMaxDescriptors) {
        std::stringstream ss;
        ss << "Setting VkPhysicalDeviceDescriptorIndexingProperties::maxUpdateAfterBindDescriptorsInAllPools to "
           << glsl::kDebugInputBindlessMaxDescriptors;
        AdjustmentWarning(physicalDevice, record_obj.location, ss.str().c_str());
        desc_indexing_props->maxUpdateAfterBindDescriptorsInAllPools = glsl::kDebugInputBindlessMaxDescriptors;
    }

    auto *vk12_props = vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Properties>(device_props2->pNext);
    if (vk12_props && vk12_props->maxUpdateAfterBindDescriptorsInAllPools > glsl::kDebugInputBindlessMaxDescriptors) {
        std::stringstream ss;
        ss << "Setting VkPhysicalDeviceVulkan12Properties::maxUpdateAfterBindDescriptorsInAllPools to "
           << glsl::kDebugInputBindlessMaxDescriptors;
        AdjustmentWarning(physicalDevice, record_obj.location, ss.str().c_str());
        vk12_props->maxUpdateAfterBindDescriptorsInAllPools = glsl::kDebugInputBindlessMaxDescriptors;
    }

    ReserveBindingSlot(physicalDevice, device_props2->properties.limits, record_obj.location);
}

// Clean up device-related resources
void Validator::PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator,
                                           const RecordObject &record_obj) {
    // Need to destroy substate before memory backed in things like shared_resources_manager are cleared
    DestroySubstate();

    shared_resources_manager.Clear();

    global_indices_buffer_.Destroy();

    BaseClass::PreCallRecordDestroyDevice(device, pAllocator, record_obj);

    // State Tracker (BaseClass) can end up making vma calls through callbacks - so destroy allocator last
    if (vma_allocator_) {
        vmaDestroyAllocator(vma_allocator_);
    }

    desc_set_manager_.reset();
}

// Common logic before any draw/dispatch/traceRays
void Validator::PreCallActionCommand(Validator &gpuav, CommandBufferSubState &cb_state, VkPipelineBindPoint bind_point,
                                     const Location &loc) {
    PreCallSetupShaderInstrumentationResources(gpuav, cb_state, bind_point, loc);
}

void Validator::PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                     uint32_t firstVertex, uint32_t firstInstance, const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    auto &sub_state = SubState(*cb_state);
    PreCallActionCommand(*this, sub_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                             const VkMultiDrawInfoEXT *pVertexInfo, uint32_t instanceCount, uint32_t firstInstance,
                                             uint32_t stride, const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    auto &sub_state = SubState(*cb_state);
    PreCallActionCommand(*this, sub_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                            uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                            const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }

    auto &sub_state = SubState(*cb_state);
    PreCallActionCommand(*this, sub_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                    const VkMultiDrawIndexedInfoEXT *pIndexInfo, uint32_t instanceCount,
                                                    uint32_t firstInstance, uint32_t stride, const int32_t *pVertexOffset,
                                                    const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    auto &sub_state = SubState(*cb_state);
    PreCallActionCommand(*this, sub_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                             uint32_t stride, const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    auto indirect_buffer_state = Get<vvl::Buffer>(buffer);
    if (!indirect_buffer_state) {
        InternalError(commandBuffer, record_obj.location, "buffer must be a valid VkBuffer handle");
        return;
    }
    auto &sub_state = SubState(*cb_state);

    valcmd::FirstInstance<VkDrawIndirectCommand>(*this, sub_state, record_obj.location, buffer, offset, count, VK_NULL_HANDLE, 0,
                                                 "VUID-VkDrawIndirectCommand-firstInstance-00501");
    PreCallActionCommand(*this, sub_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    uint32_t count, uint32_t stride, const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    auto &sub_state = SubState(*cb_state);

    valcmd::DrawIndexedIndirectIndexBuffer(*this, sub_state, record_obj.location, buffer, offset, stride, count, VK_NULL_HANDLE, 0,
                                           "VUID-VkDrawIndexedIndirectCommand-robustBufferAccess2-08798");

    valcmd::FirstInstance<VkDrawIndexedIndirectCommand>(*this, sub_state, record_obj.location, buffer, offset, count,
                                                        VK_NULL_HANDLE, 0, "VUID-VkDrawIndexedIndirectCommand-firstInstance-00554");
    PreCallActionCommand(*this, sub_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride, const RecordObject &record_obj) {
    PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                      record_obj);
}

void Validator::PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                  uint32_t stride, const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    auto indirect_buffer_state = Get<vvl::Buffer>(buffer);
    if (!indirect_buffer_state) {
        InternalError(commandBuffer, record_obj.location, "buffer must be a valid VkBuffer handle");
        return;
    }
    auto &sub_state = SubState(*cb_state);

    valcmd::CountBuffer(*this, sub_state, record_obj.location, buffer, offset, sizeof(VkDrawIndirectCommand),
                        vvl::Struct::VkDrawIndirectCommand, stride, countBuffer, countBufferOffset,
                        "VUID-vkCmdDrawIndirectCount-countBuffer-02717");
    valcmd::FirstInstance<VkDrawIndirectCommand>(*this, sub_state, record_obj.location, buffer, offset, maxDrawCount, countBuffer,
                                                 countBufferOffset, "VUID-VkDrawIndirectCommand-firstInstance-00501");
    PreCallActionCommand(*this, sub_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                         uint32_t firstInstance, VkBuffer counterBuffer,
                                                         VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                         uint32_t vertexStride, const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    auto &sub_state = SubState(*cb_state);
    PreCallActionCommand(*this, sub_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                            VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                            uint32_t maxDrawCount, uint32_t stride,
                                                            const RecordObject &record_obj) {
    PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                             record_obj);
}

void Validator::PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                         uint32_t maxDrawCount, uint32_t stride, const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    auto &sub_state = SubState(*cb_state);
    valcmd::CountBuffer(*this, sub_state, record_obj.location, buffer, offset, sizeof(VkDrawIndexedIndirectCommand),
                        vvl::Struct::VkDrawIndexedIndirectCommand, stride, countBuffer, countBufferOffset,
                        "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-02717");
    valcmd::FirstInstance<VkDrawIndexedIndirectCommand>(*this, sub_state, record_obj.location, buffer, offset, maxDrawCount,
                                                        countBuffer, countBufferOffset,
                                                        "VUID-VkDrawIndexedIndirectCommand-firstInstance-00554");

    valcmd::DrawIndexedIndirectIndexBuffer(*this, sub_state, record_obj.location, buffer, offset, stride, maxDrawCount, countBuffer,
                                           countBufferOffset, "VUID-VkDrawIndexedIndirectCommand-robustBufferAccess2-08798");

    PreCallActionCommand(*this, sub_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                                const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    auto &sub_state = SubState(*cb_state);
    PreCallActionCommand(*this, sub_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        uint32_t drawCount, uint32_t stride, const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    auto &sub_state = SubState(*cb_state);
    PreCallActionCommand(*this, sub_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                             uint32_t maxDrawCount, uint32_t stride,
                                                             const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    auto indirect_buffer_state = Get<vvl::Buffer>(buffer);
    if (!indirect_buffer_state) {
        InternalError(commandBuffer, record_obj.location, "buffer must be a valid VkBuffer handle");
        return;
    }

    auto &sub_state = SubState(*cb_state);
    valcmd::CountBuffer(*this, sub_state, record_obj.location, buffer, offset, sizeof(VkDrawMeshTasksIndirectCommandNV),
                        vvl::Struct::VkDrawMeshTasksIndirectCommandNV, stride, countBuffer, countBufferOffset,
                        "VUID-vkCmdDrawMeshTasksIndirectCountNV-countBuffer-02717");

    PreCallActionCommand(*this, sub_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                                 uint32_t groupCountZ, const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    auto &sub_state = SubState(*cb_state);
    PreCallActionCommand(*this, sub_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         uint32_t drawCount, uint32_t stride, const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    auto &sub_state = SubState(*cb_state);
    valcmd::DrawMeshIndirect(*this, sub_state, record_obj.location, buffer, offset, stride, VK_NULL_HANDLE, 0, drawCount);
    PreCallActionCommand(*this, sub_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                              uint32_t maxDrawCount, uint32_t stride,
                                                              const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    auto indirect_buffer_state = Get<vvl::Buffer>(buffer);
    if (!indirect_buffer_state) {
        InternalError(commandBuffer, record_obj.location, "buffer must be a valid VkBuffer handle");
        return;
    }

    auto &sub_state = SubState(*cb_state);
    valcmd::DrawMeshIndirect(*this, sub_state, record_obj.location, buffer, offset, stride, countBuffer, countBufferOffset,
                             maxDrawCount);

    valcmd::CountBuffer(*this, sub_state, record_obj.location, buffer, offset, sizeof(VkDrawMeshTasksIndirectCommandEXT),
                        vvl::Struct::VkDrawMeshTasksIndirectCommandEXT, stride, countBuffer, countBufferOffset,
                        "VUID-vkCmdDrawMeshTasksIndirectCountEXT-countBuffer-02717");
    PreCallActionCommand(*this, sub_state, VK_PIPELINE_BIND_POINT_GRAPHICS, record_obj.location);
}

void Validator::PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z,
                                         const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    auto &sub_state = SubState(*cb_state);
    PreCallActionCommand(*this, sub_state, VK_PIPELINE_BIND_POINT_COMPUTE, record_obj.location);
}
void Validator::PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    auto &sub_state = SubState(*cb_state);
    valcmd::DispatchIndirect(*this, record_obj.location, sub_state, buffer, offset);
    PreCallActionCommand(*this, sub_state, VK_PIPELINE_BIND_POINT_COMPUTE, record_obj.location);
}

void Validator::PreCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                             uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                             const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    auto &sub_state = SubState(*cb_state);
    PreCallActionCommand(*this, sub_state, VK_PIPELINE_BIND_POINT_COMPUTE, record_obj.location);
}

void Validator::PreCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                                uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                                                uint32_t groupCountZ, const RecordObject &record_obj) {
    PreCallRecordCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ,
                                 record_obj);
}

void Validator::PreCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                            VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                            VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                            VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                            VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                            VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                            uint32_t width, uint32_t height, uint32_t depth, const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    auto &sub_state = SubState(*cb_state);
    PreCallActionCommand(*this, sub_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, record_obj.location);
}

void Validator::PreCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                             const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                             const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                             const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                             const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, uint32_t width,
                                             uint32_t height, uint32_t depth, const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    auto &sub_state = SubState(*cb_state);
    PreCallActionCommand(*this, sub_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, record_obj.location);
}

void Validator::PreCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                     const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                     const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                     const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                     const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                                     VkDeviceAddress indirectDeviceAddress, const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    auto &sub_state = SubState(*cb_state);
    valcmd::TraceRaysIndirect(*this, record_obj.location, sub_state, indirectDeviceAddress);
    PreCallActionCommand(*this, sub_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, record_obj.location);
}

void Validator::PreCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                                      const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    auto &sub_state = SubState(*cb_state);
    PreCallActionCommand(*this, sub_state, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, record_obj.location);
}

void Validator::PreCallRecordCmdExecuteGeneratedCommandsEXT(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                            const VkGeneratedCommandsInfoEXT *pGeneratedCommandsInfo,
                                                            const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) {
        InternalError(commandBuffer, record_obj.location, "Unrecognized command buffer.");
        return;
    }
    const VkPipelineBindPoint bind_point = ConvertStageToBindPoint(pGeneratedCommandsInfo->shaderStages);
    auto &sub_state = SubState(*cb_state);
    PreCallActionCommand(*this, sub_state, bind_point, record_obj.location);
};

void Validator::PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                  VkImageLayout dstImageLayout, uint32_t regionCount,
                                                  const VkBufferImageCopy *pRegions, const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);

    std::vector<VkBufferImageCopy2> regions_2(regionCount);
    for (const auto [i, region] : vvl::enumerate(pRegions, regionCount)) {
        regions_2[i].bufferOffset = region.bufferOffset;
        regions_2[i].bufferRowLength = region.bufferRowLength;
        regions_2[i].bufferImageHeight = region.bufferImageHeight;
        regions_2[i].imageSubresource = region.imageSubresource;
        regions_2[i].imageOffset = region.imageOffset;
        regions_2[i].imageExtent = region.imageExtent;
    }

    VkCopyBufferToImageInfo2 copy_buffer_to_image_info = vku::InitStructHelper();
    copy_buffer_to_image_info.srcBuffer = srcBuffer;
    copy_buffer_to_image_info.dstImage = dstImage;
    copy_buffer_to_image_info.dstImageLayout = dstImageLayout;
    copy_buffer_to_image_info.regionCount = regionCount;
    copy_buffer_to_image_info.pRegions = regions_2.data();

    valcmd::CopyBufferToImage(*this, record_obj.location, SubState(*cb_state), &copy_buffer_to_image_info);
}

void Validator::PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                      const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo2KHR,
                                                      const RecordObject &record_obj) {
    PreCallRecordCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo2KHR, record_obj);
}

void Validator::PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                   const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo,
                                                   const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    valcmd::CopyBufferToImage(*this, record_obj.location, SubState(*cb_state), pCopyBufferToImageInfo);
}

// Validates the buffer is allowed to be protected
bool Validator::ValidateProtectedBuffer(const vvl::CommandBuffer &cb_state, const vvl::Buffer &buffer_state,
                                        const Location &buffer_loc, const char *vuid, const char *more_message) const {
    bool skip = false;

    // if driver supports protectedNoFault the operation is valid, just has undefined values
    if ((!phys_dev_props_core11.protectedNoFault) && (cb_state.unprotected == true) && (buffer_state.unprotected == false)) {
        const LogObjectList objlist(cb_state.Handle(), buffer_state.Handle());
        skip |= LogError(vuid, objlist, buffer_loc, "(%s) is a protected buffer, but command buffer (%s) is unprotected.%s",
                         FormatHandle(buffer_state).c_str(), FormatHandle(cb_state).c_str(), more_message);
    }
    return skip;
}

// Validates the buffer is allowed to be unprotected
bool Validator::ValidateUnprotectedBuffer(const vvl::CommandBuffer &cb_state, const vvl::Buffer &buffer_state,
                                          const Location &buffer_loc, const char *vuid, const char *more_message) const {
    bool skip = false;

    // if driver supports protectedNoFault the operation is valid, just has undefined values
    if ((!phys_dev_props_core11.protectedNoFault) && (cb_state.unprotected == false) && (buffer_state.unprotected == true)) {
        const LogObjectList objlist(cb_state.Handle(), buffer_state.Handle());
        skip |= LogError(vuid, objlist, buffer_loc, "(%s) is an unprotected buffer, but command buffer (%s) is protected.%s",
                         FormatHandle(buffer_state).c_str(), FormatHandle(cb_state).c_str(), more_message);
    }
    return skip;
}

// Validates the image is allowed to be protected
bool Validator::ValidateProtectedImage(const vvl::CommandBuffer &cb_state, const vvl::Image &image_state, const Location &loc,
                                       const char *vuid, const char *more_message) const {
    bool skip = false;

    // if driver supports protectedNoFault the operation is valid, just has undefined values
    if ((!phys_dev_props_core11.protectedNoFault) && (cb_state.unprotected == true) && (image_state.unprotected == false)) {
        const LogObjectList objlist(cb_state.Handle(), image_state.Handle());
        skip |= LogError(vuid, objlist, loc, "(%s) is a protected image, but command buffer (%s) is unprotected.%s",
                         FormatHandle(image_state).c_str(), FormatHandle(cb_state).c_str(), more_message);
    }
    return skip;
}

// Validates the image is allowed to be unprotected
bool Validator::ValidateUnprotectedImage(const vvl::CommandBuffer &cb_state, const vvl::Image &image_state, const Location &loc,
                                         const char *vuid, const char *more_message) const {
    bool skip = false;

    // if driver supports protectedNoFault the operation is valid, just has undefined values
    if ((!phys_dev_props_core11.protectedNoFault) && (cb_state.unprotected == false) && (image_state.unprotected == true)) {
        const LogObjectList objlist(cb_state.Handle(), image_state.Handle());
        skip |= LogError(vuid, objlist, loc, "(%s) is an unprotected image, but command buffer (%s) is protected.%s",
                         FormatHandle(image_state).c_str(), FormatHandle(cb_state).c_str(), more_message);
    }
    return skip;
}

// Validates the buffer is allowed to be protected
bool Validator::ValidateProtectedTensor(const vvl::CommandBuffer &cb_state, const vvl::Tensor &tensor_state,
                                        const Location &tensor_loc, const char *vuid, const char *more_message) const {
    /* don't use on an unprotected tensor */
    assert(tensor_state.unprotected == false);

    bool skip = false;

    // if driver supports protectedNoFault the operation is valid, just has undefined values
    if ((!phys_dev_props_core11.protectedNoFault) && (cb_state.unprotected == true)) {
        const LogObjectList objlist(cb_state.Handle(), tensor_state.Handle());
        skip |= LogError(vuid, objlist, tensor_loc, "(%s) is a protected tensor, but command buffer (%s) is unprotected.%s",
                         FormatHandle(tensor_state).c_str(), FormatHandle(cb_state).c_str(), more_message);
    }
    return skip;
}

// Validates the buffer is allowed to be unprotected
bool Validator::ValidateUnprotectedTensor(const vvl::CommandBuffer &cb_state, const vvl::Tensor &tensor_state,
                                          const Location &tensor_loc, const char *vuid, const char *more_message) const {
    /* don't use on a protected tensor */
    assert(tensor_state.unprotected == true);

    bool skip = false;

    // if driver supports protectedNoFault the operation is valid, just has undefined values
    if ((!phys_dev_props_core11.protectedNoFault) && (cb_state.unprotected == false)) {
        const LogObjectList objlist(cb_state.Handle(), tensor_state.Handle());
        skip |= LogError(vuid, objlist, tensor_loc, "(%s) is an unprotected tensor, but command buffer (%s) is protected.%s",
                         FormatHandle(tensor_state).c_str(), FormatHandle(cb_state).c_str(), more_message);
    }
    return skip;
}

}  // namespace gpuav
