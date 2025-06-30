/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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

#pragma once

#include "best_practices/bp_constants.h"
#include "chassis/validation_object.h"
#include "state_tracker/shader_module.h"
#include "state_tracker/state_tracker.h"
#include "state_tracker/cmd_buffer_state.h"
#include <string>
#include <deque>
#include <chrono>
#include <set>

enum class DeprecationReason {
    Empty = 0,
    Promoted,
    Obsoleted,
    Deprecated,
};

struct DeprecationData {
    DeprecationReason reason;
    vvl::Requirement target;
};

struct ShaderStageState;
struct LastBound;

namespace spirv {
struct EntryPoint;
struct Module;
}  // namespace spirv

DeprecationData GetDeprecatedData(vvl::Extension extension);
std::string GetSpecialUse(vvl::Extension extension);

namespace bp_state {
class CommandBufferSubState;

template <typename StateObject, typename Handle>
void LogResult(const StateObject& state, Handle handle, const RecordObject& record_obj) {
    if (record_obj.result == VK_SUCCESS) {
        return;
    }
    // Despite being error codes log these results as informational.
    // That is because they are returned frequently during window resizing.
    // They are expected to occur during the normal application lifecycle.
    constexpr std::array common_failure_codes = {VK_ERROR_OUT_OF_DATE_KHR, VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT};
    const auto result_string = string_VkResult(record_obj.result);

    if (record_obj.result > VK_SUCCESS) {
        state.LogVerbose("BestPractices-Verbose-Success-Logging", handle, record_obj.location, "Returned %s.", result_string);
    } else if (IsValueIn(record_obj.result, common_failure_codes)) {
        state.LogInfo("BestPractices-Failure-Result", handle, record_obj.location, "Returned error %s.", result_string);
    } else {
        state.LogWarning("BestPractices-Error-Result", handle, record_obj.location, "Returned error %s.", result_string);
    }
}

const char* VendorSpecificTag(BPVendorFlags vendors);

bool VendorCheckEnabled(const ValidationEnabled& enabled, BPVendorFlags vendors);

class Instance : public vvl::InstanceProxy {
    using BaseClass = vvl::InstanceProxy;

  public:
    using Func = vvl::Func;
    using Struct = vvl::Struct;
    using Field = vvl::Field;

    Instance(vvl::dispatch::Instance* dispatch) : BaseClass(dispatch, LayerObjectTypeBestPractices) {}

    bool VendorCheckEnabled(BPVendorFlags vendors) const { return bp_state::VendorCheckEnabled(enabled, vendors); }

    bool PreCallValidateCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                       VkInstance* pInstance, const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                     const ErrorObject& error_obj) const override;
    bool ValidateDeprecatedExtensions(const Location& loc, vvl::Extension extension, APIVersion version) const;

    bool ValidateSpecialUseExtensions(const Location& loc, vvl::Extension extension) const;

    bool ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(VkPhysicalDevice physicalDevice, const Location& loc) const;
    bool PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                            uint32_t* pDisplayCount, VkDisplayKHR* pDisplays,
                                                            const ErrorObject& error_obj) const override;
    bool PreCallValidateGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex,
                                                       VkDisplayPlaneCapabilitiesKHR* pCapabilities,
                                                       const ErrorObject& error_obj) const override;
    bool PreCallValidateGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                        const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                        VkDisplayPlaneCapabilities2KHR* pCapabilities,
                                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                               VkQueueFamilyProperties* pQueueFamilyProperties,
                                                               const ErrorObject& error_obj) const override;
    bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                uint32_t* pQueueFamilyPropertyCount,
                                                                VkQueueFamilyProperties2* pQueueFamilyProperties,
                                                                const ErrorObject& error_obj) const override;
    bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                   uint32_t* pQueueFamilyPropertyCount,
                                                                   VkQueueFamilyProperties2* pQueueFamilyProperties,
                                                                   const ErrorObject& error_obj) const override;
    bool PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                           uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats,
                                                           const ErrorObject& error_obj) const override;
    bool ValidateCommonGetPhysicalDeviceQueueFamilyProperties(const vvl::PhysicalDevice& pd_state,
                                                              uint32_t requested_queue_family_property_count,
                                                              const Location& loc) const;
// Include code-generated functions
#include "generated/best_practices_instance_methods.h"
};
}  // namespace bp_state

class BestPractices : public vvl::DeviceProxy {
    using BaseClass = vvl::DeviceProxy;

  public:
    using Func = vvl::Func;
    using Struct = vvl::Struct;
    using Field = vvl::Field;

    BestPractices(vvl::dispatch::Device* dev, bp_state::Instance* instance_vo)
        : BaseClass(dev, instance_vo, LayerObjectTypeBestPractices) {}

    ReadLockGuard ReadLock() const override;
    WriteLockGuard WriteLock() override;

    std::string GetAPIVersionName(uint32_t version) const;

    bool ValidateCmdDrawType(const bp_state::CommandBufferSubState& cb_state, const Location& loc) const;

    bool ValidateCmdDispatchType(const bp_state::CommandBufferSubState& cb_state, const Location& loc) const;

    bool ValidatePushConstants(const bp_state::CommandBufferSubState& cb_state, const Location& loc) const;

    void RecordCmdDrawType(bp_state::CommandBufferSubState& cb_state, uint32_t draw_count);

    bool PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer,
                                     const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                    VkImage* pImage, const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain,
                                           const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                  const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                  const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains,
                                                  const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                         const ErrorObject& error_obj) const override;
    bool ValidateAttachments(const VkRenderPassCreateInfo2* rpci, uint32_t attachment_count, const VkImageView* attachments,
                             const Location& loc) const;
    bool PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer,
                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                               VkDescriptorSet* pDescriptorSets, const ErrorObject& error_obj,
                                               vvl::AllocateDescriptorSetsData& ads_state_data) const override;
    bool PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                       const ErrorObject& error_obj) const override;
    void PreCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                     const RecordObject& record_obj) override;
    void ManualPostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                            const RecordObject& record_obj);
    bool ValidateBindBufferMemory(VkBuffer buffer, VkDeviceMemory memory, const Location& loc) const;
    bool PreCallValidateBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                         const ErrorObject& error_obj) const override;
    bool PreCallValidateBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                             const ErrorObject& error_obj) const override;
    bool ValidateBindImageMemory(VkImage image, VkDeviceMemory memory, const Location& loc) const;
    bool PreCallValidateBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                         const ErrorObject& error_obj) const override;
    bool PreCallValidateBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                            const ErrorObject& error_obj) const override;
    bool PreCallValidateGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                             uint32_t* pMemoryRequirementsCount,
                                                             VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements,
                                                             const ErrorObject& error_obj) const override;
    bool PreCallValidateBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                  uint32_t bindSessionMemoryInfoCount,
                                                  const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos,
                                                  const ErrorObject& error_obj) const override;
    bool PreCallValidateAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo,
                                               VkCommandBuffer* pCommandBuffers, const ErrorObject& error_obj) const override;
    void PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator,
                                 const RecordObject& record_obj) override;
    bool ValidateMultisampledBlendingArm(const VkGraphicsPipelineCreateInfo& create_info, const Location& create_info_loc) const;

    bool ValidateCreateGraphicsPipeline(const VkGraphicsPipelineCreateInfo& create_info, const vvl::Pipeline& pipeline,
                                        const Location create_info_loc) const;
    bool PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                const ErrorObject& error_obj, PipelineStates& pipeline_states,
                                                chassis::CreateGraphicsPipelines& chassis_state) const override;
    bool PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                               const VkComputePipelineCreateInfo* pCreateInfos,
                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                               const ErrorObject& error_obj, PipelineStates& pipeline_states,
                                               chassis::CreateComputePipelines& chassis_state) const override;

    bool ValidateShaderStage(const ShaderStageState& stage_state, const vvl::Pipeline* pipeline, const Location& loc) const;
    bool ValidateComputeShaderArm(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                  const Location& loc) const;
    bool ValidateComputeShaderAmd(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                  const Location& loc) const;

    bool CheckPipelineStageFlags(const LogObjectList& objlist, const Location& loc, VkPipelineStageFlags flags) const;
    bool CheckPipelineStageFlags(const LogObjectList& objlist, const Location& loc, VkPipelineStageFlags2KHR flags) const;
    bool CheckDependencyInfo(const LogObjectList& objlist, const Location& dep_loc, const VkDependencyInfo& dep_info,
                             VkCommandBuffer commandBuffer) const;
    bool PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                    const ErrorObject& error_obj) const override;
    bool PreCallValidateQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits, VkFence fence,
                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                     const ErrorObject& error_obj) const override;
    bool PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo,
                                           const ErrorObject& error_obj) const override;
    bool CheckEventSignalingState(const bp_state::CommandBufferSubState& command_buffer, VkEvent event,
                                  const Location& cb_loc) const;
    bool PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                    const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfoKHR* pDependencyInfo,
                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo,
                                     const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2KHR stageMask,
                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                          const VkDependencyInfoKHR* pDependencyInfos, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                       const VkDependencyInfo* pDependencyInfos, const ErrorObject& error_obj) const override;
    bool ValidateAccessLayoutCombination(const Location& loc, VkImage image, VkAccessFlags2 access, VkImageLayout layout,
                                         VkImageAspectFlags aspect) const;
    bool ValidateImageMemoryBarrier(const Location& loc, VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout,
                                    VkImageLayout newLayout, VkAccessFlags2 srcAccessMask, VkAccessFlags2 dstAccessMask,
                                    VkImageAspectFlags aspectMask, uint32_t srcQueueFamilyIndex,
                                    uint32_t dstQueueFamilyIndex) const;
    bool ValidateBufferMemoryBarrier(const Location& loc, VkCommandBuffer commandBuffer, VkBuffer buffer,
                                     uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex) const;
    bool PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                           VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                           uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                           uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                           uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                           const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR* pDependencyInfo,
                                               const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                            const ErrorObject& error_obj) const override;

    template <typename ImageMemoryBarrier>
    bool ValidateCmdPipelineBarrierImageBarrier(VkCommandBuffer commandBuffer, const ImageMemoryBarrier& barrier,
                                                const Location& loc) const;
    bool PreCallValidateCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR pipelineStage,
                                              VkQueryPool queryPool, uint32_t query, const ErrorObject& error_obj) const override;
    bool PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                            size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags,
                                            const ErrorObject& error_obj) const override;
    bool ValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                    const Location& loc) const;
    bool ValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo, const Location& loc) const;

    bool PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                           VkSubpassContents contents, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                               const VkSubpassBeginInfo* pSubpassBeginInfo,
                                               const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                            const VkSubpassBeginInfo* pSubpassBeginInfo,
                                            const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                             const ErrorObject& error_obj) const override;
    void UpdateBoundDescriptorSets(bp_state::CommandBufferSubState& commandBuffer, const LastBound& last_bound_state,
                                   const Location& loc);
    bool PreCallValidateCmdEndRendering(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;

    bool PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                                uint32_t firstInstance, const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                               uint32_t firstInstance, const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                       uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                       const ErrorObject& error_obj) const override;
    bool ValidateIndexBufferArm(const bp_state::CommandBufferSubState& cb_state, uint32_t indexCount, uint32_t instanceCount,
                                uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance, const Location& loc) const;
    void PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                      uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                      const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                        uint32_t stride, const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                       uint32_t stride, const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               uint32_t drawCount, uint32_t stride, const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                              uint32_t stride, const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                    uint32_t stride, const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                   uint32_t stride, const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride, const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride, const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride, const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride, const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance,
                                                    VkBuffer counterBuffer, VkDeviceSize counterBufferOffset,
                                                    uint32_t counterOffset, uint32_t vertexStride,
                                                    const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance,
                                                   VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                   uint32_t vertexStride, const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                             uint32_t stride, const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                            VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                            uint32_t stride, const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                uint32_t stride, const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                               uint32_t stride, const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                uint32_t stride, const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                               uint32_t stride, const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride, const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride, const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   uint32_t drawCount, uint32_t stride,
                                                   const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  uint32_t drawCount, uint32_t strCmdDrawMeshTasksIndirectNVide,
                                                  const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                           const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                          const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                               const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                               uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset,
                                               const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                              const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                              uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset,
                                              const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                        uint32_t instanceCount, uint32_t firstInstance, uint32_t stride,
                                        const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                       uint32_t instanceCount, uint32_t firstInstance, uint32_t stride,
                                       const RecordObject& record_obj) override;

    bool PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                    const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                            const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                        uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                           uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                           const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                             const ErrorObject& error_obj) const override;
    bool PreCallValidateBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                          const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence,
                                        const ErrorObject& error_obj) const override;
    void ManualPostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                             VkFence fence, const RecordObject& record_obj);
    bool PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                            const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects,
                                            const ErrorObject& error_obj) const override;
    bool ValidateCmdResolveImage(VkCommandBuffer command_buffer, VkImage src_image, VkImage dst_image, const Location& loc) const;
    bool PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                        VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                        const VkImageResolve* pRegions, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR* pResolveImageInfo,
                                            const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                         const ErrorObject& error_obj) const override;

    using QueueCallback = std::function<bool(const class vvl::Queue& queue_state, const vvl::CommandBuffer& cb_state)>;
    using QueueCallbacks = std::vector<QueueCallback>;

    void QueueValidateImageView(QueueCallbacks& func, const Location& loc, const vvl::ImageView& image_view,
                                IMAGE_SUBRESOURCE_USAGE_BP usage);
    void QueueValidateImage(QueueCallbacks& func, const Location& loc, vvl::Image& image_state, IMAGE_SUBRESOURCE_USAGE_BP usage,
                            const VkImageSubresourceRange& subresource_range);
    void QueueValidateImage(QueueCallbacks& func, const Location& loc, vvl::Image& image_state, IMAGE_SUBRESOURCE_USAGE_BP usage,
                            const VkImageSubresourceLayers& range);
    void QueueValidateImage(QueueCallbacks& func, const Location& loc, vvl::Image& image_state, IMAGE_SUBRESOURCE_USAGE_BP usage,
                            uint32_t array_layer, uint32_t mip_level);
    void ValidateImageInQueue(const vvl::Queue& qs, const vvl::CommandBuffer& cbs, const Location& loc, vvl::Image& image_state,
                              IMAGE_SUBRESOURCE_USAGE_BP usage, uint32_t array_layer, uint32_t mip_level);
    void ValidateImageInQueueArmImg(const Location& loc, vvl::Image& image_state, IMAGE_SUBRESOURCE_USAGE_BP last_usage,
                                    IMAGE_SUBRESOURCE_USAGE_BP usage, uint32_t array_layer, uint32_t mip_level);

    template <typename RegionType>
    bool ValidateCmdBlitImage(VkCommandBuffer command_buffer, uint32_t region_count, const RegionType* regions,
                              const Location& loc) const;
    bool PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageBlit* pRegions, VkFilter filter, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR* pBlitImageInfo,
                                         const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                      const ErrorObject& error_obj) const override;

    bool PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkSampler* pSampler,
                                      const ErrorObject& error_obj) const override;
    void ManualPostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo, const RecordObject& record_obj);
    void ManualPostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                     const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                     const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                     const RecordObject& record_obj, PipelineStates& pipeline_states,
                                                     chassis::CreateGraphicsPipelines& chassis_state);

    bool PreCallValidateAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                            VkFence fence, uint32_t* pImageIndex, const ErrorObject& error_obj) const override;

    void ManualPostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                         const RecordObject& record_obj);

    void ManualPostCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer, const RecordObject& record_obj);

    void ManualPostCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                               const RecordObject& record_obj);
    void ManualPostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                              const RecordObject& record_obj);

    void PostCallRecordGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements,
                                                  const RecordObject& record_obj) override;
    void PostCallRecordGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                   VkMemoryRequirements2* pMemoryRequirements,
                                                   const RecordObject& record_obj) override;
    void PostCallRecordGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                      VkMemoryRequirements2* pMemoryRequirements,
                                                      const RecordObject& record_obj) override;
    void PostCallRecordGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount,
                                                        VkSparseImageMemoryRequirements* pSparseMemoryRequirements,
                                                        const RecordObject& record_obj) override;
    void PostCallRecordGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                         uint32_t* pSparseMemoryRequirementCount,
                                                         VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                         const RecordObject& record_obj) override;
    void PostCallRecordGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                            uint32_t* pSparseMemoryRequirementCount,
                                                            VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                            const RecordObject& record_obj) override;

    void RecordGetImageMemoryRequirementsState(vvl::Image& image_state, const VkImageMemoryRequirementsInfo2* pInfo);

    void ManualPostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                    const VkComputePipelineCreateInfo* pCreateInfos,
                                                    const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                    const RecordObject& record_obj, PipelineStates& pipeline_states,
                                                    chassis::CreateComputePipelines& chassis_state);

    void PostCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                          VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                          uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                          uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                          uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                          const RecordObject& record_obj) override;
    void PostCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                           const RecordObject& record_obj) override;
    void PostCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                              const RecordObject& record_obj) override;
    template <typename ImageMemoryBarrier>
    void RecordCmdPipelineBarrierImageBarrier(VkCommandBuffer commandBuffer, const ImageMemoryBarrier& barrier);

    void PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                              const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                              const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                              const RecordObject& record_obj) override;

    bool PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                             const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                             const VkCopyDescriptorSet* pDescriptorCopies,
                                             const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                       const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool,
                                             const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                           const VkClearColorValue* pColor, uint32_t rangeCount,
                                           const VkImageSubresourceRange* pRanges, const ErrorObject& error_obj) const override;

    bool PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                  const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                  const VkImageSubresourceRange* pRanges,
                                                  const ErrorObject& error_obj) const override;

    bool PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                             const ErrorObject& error_obj) const override;

    bool PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageCopy* pRegions, const ErrorObject& error_obj) const override;

    bool PreCallValidateCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline,
                                        const ErrorObject& error_obj) const override;

    bool PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo,
                                        const ErrorObject& error_obj) const override;

    bool PreCallValidateCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore,
                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                    VkFence* pFence, const ErrorObject& error_obj) const override;

    void PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                  const RecordObject& record_obj) override;

    bool PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                           const VkCommandBuffer* pCommandBuffers, const ErrorObject& error_obj) const override;

    bool PreCallValidateCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo,
                                                        VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update,
                                                        VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                        VkBuffer scratch, VkDeviceSize scratchOffset,
                                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                                  const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                                  const VkDeviceAddress* pIndirectDeviceAddresses,
                                                                  const uint32_t* pIndirectStrides,
                                                                  const uint32_t* const* ppMaxPrimitiveCounts,
                                                                  const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBuildAccelerationStructuresKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                          const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                          const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos,
                                                          const ErrorObject& error_obj) const override;

    void Created(vvl::CommandBuffer& cb_state) override;
    void Created(vvl::Image& image_state) override;

    // Check that vendor-specific checks are enabled for at least one of the vendors
    bool VendorCheckEnabled(BPVendorFlags vendors) const { return bp_state::VendorCheckEnabled(enabled, vendors); }
    const char* VendorSpecificTag(BPVendorFlags vendors) const { return bp_state::VendorSpecificTag(vendors); }

    // TODO - Move these to CommandBufferSubState
    void RecordCmdDrawTypeArm(bp_state::CommandBufferSubState& cb_state, uint32_t draw_count);
    void RecordClearColor(VkFormat format, const VkClearColorValue& clear_value);
    bool ClearAttachmentsIsFullClear(const bp_state::CommandBufferSubState& cb_state, uint32_t rectCount,
                                     const VkClearRect* pRects) const;
    void PipelineUsedInFrame(VkPipeline pipeline) {
        WriteLockGuard guard(pipeline_lock_);
        pipelines_used_in_frame_.insert(pipeline);
    }

// Include code-generated functions
#include "generated/best_practices_device_methods.h"
  private:
    // CacheEntry and PostTransformLRUCacheModel are used on the stack
    struct CacheEntry {
        uint32_t value;
        uint32_t age;
    };

    class PostTransformLRUCacheModel {
      public:
        typedef std::vector<CacheEntry>::iterator cache_iterator;

        void resize(size_t size);

        // Returns true if there was a cache hit - also models LRU behavior which will effect subsequent calls.
        bool query_cache(uint32_t value);

      private:
        std::vector<CacheEntry> _entries = {};
        uint32_t iteration = 0;
    };

    bool ValidateClearAttachment(const bp_state::CommandBufferSubState& cb_state, uint32_t fb_attachment, uint32_t color_attachment,
                                 VkImageAspectFlags aspects, const Location& loc) const;

    bool ValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const Location& loc) const;

    bool ValidateBuildAccelerationStructure(VkCommandBuffer commandBuffer, const Location& loc) const;

    bool ValidateBindMemory(VkDevice device, VkDeviceMemory memory, const Location& loc) const;

    bool ValidateZcullScope(const bp_state::CommandBufferSubState& cb_state, const Location& loc) const;
    bool ValidateZcull(const bp_state::CommandBufferSubState& cb_state, VkImage image,
                       const VkImageSubresourceRange& subresource_range, const Location& loc) const;

    bool ValidateClearColor(VkCommandBuffer commandBuffer, VkFormat format, const VkClearColorValue& clear_value,
                            const Location& loc) const;

    void ClearPipelinesUsedInFrame() {
        WriteLockGuard guard(pipeline_lock_);
        pipelines_used_in_frame_.clear();
    }

    bool IsPipelineUsedInFrame(VkPipeline pipeline) const {
        ReadLockGuard guard(pipeline_lock_);
        return pipelines_used_in_frame_.count(pipeline) != 0;
    }

    // AMD tracked
    std::atomic<uint32_t> num_barriers_objects_{0};
    std::atomic<uint32_t> num_pso_{0};
    std::atomic<uint32_t> num_queue_submissions_{0};

    std::atomic<VkPipelineCache> pipeline_cache_{VK_NULL_HANDLE};

    // NVIDIA tracked
    struct MemoryFreeEvent {
        typename std::chrono::high_resolution_clock::time_point time{};
        VkDeviceSize allocation_size = 0;
        uint32_t memory_type_index = 0;
    };
    std::deque<MemoryFreeEvent> memory_free_events_;
    mutable std::shared_mutex memory_free_events_lock_;

    // Can't get vvl::unordered_set to work with std::array
    std::set<std::array<uint32_t, 4>> clear_colors_;
    mutable std::shared_mutex clear_colors_lock_;

    vvl::unordered_set<VkPipeline> pipelines_used_in_frame_;
    mutable std::shared_mutex pipeline_lock_;
};

static inline bool RenderPassUsesAttachmentOnTile(const vku::safe_VkRenderPassCreateInfo2& create_info, uint32_t attachment) {
    for (uint32_t subpass = 0; subpass < create_info.subpassCount; subpass++) {
        const auto& subpass_info = create_info.pSubpasses[subpass];

        // If an attachment is ever used as a color attachment,
        // resolve attachment or depth stencil attachment,
        // it needs to exist on tile at some point.

        for (uint32_t i = 0; i < subpass_info.colorAttachmentCount; i++) {
            if (subpass_info.pColorAttachments[i].attachment == attachment) return true;
        }

        if (subpass_info.pResolveAttachments) {
            for (uint32_t i = 0; i < subpass_info.colorAttachmentCount; i++) {
                if (subpass_info.pResolveAttachments[i].attachment == attachment) return true;
            }
        }

        if (subpass_info.pDepthStencilAttachment && subpass_info.pDepthStencilAttachment->attachment == attachment) return true;
    }

    return false;
}

static inline bool RenderPassUsesAttachmentAsImageOnly(const vku::safe_VkRenderPassCreateInfo2& create_info, uint32_t attachment) {
    if (RenderPassUsesAttachmentOnTile(create_info, attachment)) {
        return false;
    }

    for (uint32_t subpass = 0; subpass < create_info.subpassCount; subpass++) {
        const auto& subpass_info = create_info.pSubpasses[subpass];

        for (uint32_t i = 0; i < subpass_info.inputAttachmentCount; i++) {
            if (subpass_info.pInputAttachments[i].attachment == attachment) {
                return true;
            }
        }
    }

    return false;
}
