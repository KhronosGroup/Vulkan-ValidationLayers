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

#pragma once

#include "containers/custom_containers.h"
#include "gpuav/descriptor_validation/gpuav_descriptor_set.h"
#include "gpuav/resources/gpuav_vulkan_objects.h"
#include "gpuav/instrumentation/gpuav_shader_instrumentor.h"

#include <memory>

struct LastBound;
namespace chassis {
struct ShaderObject;
struct CmdBindDescriptorBuffers;
}  // namespace chassis

namespace vvl {
class DeviceMemory;
}  // namespace vvl

namespace gpuav {
class CommandBufferSubState;
class DescriptorSetSubState;
class QueueSubState;
}  // namespace gpuav

namespace gpuav {

class Instance : public vvl::InstanceProxy {
    using BaseClass = vvl::InstanceProxy;

  public:
    Instance(vvl::dispatch::Instance* dispatch) : BaseClass(dispatch, LayerObjectTypeGpuAssisted) {}

    void PreCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, const RecordObject& record_obj,
                                   vku::safe_VkDeviceCreateInfo* modified_create_info) final;
    void ReserveBindingSlot(VkPhysicalDevice physicalDevice, VkPhysicalDeviceLimits& limits, const Location& loc);
    void PostCallRecordGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                                   VkPhysicalDeviceProperties* pPhysicalDeviceProperties,
                                                   const RecordObject& record_obj) override;
    void PostCallRecordGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                                    VkPhysicalDeviceProperties2* pPhysicalDeviceProperties2,
                                                    const RecordObject& record_obj) final;

    void InternalWarning(LogObjectList objlist, const Location& loc, const char* const specific_message) const;
    void AdjustmentWarning(LogObjectList objlist, const Location& loc, const char* const specific_message) const;
    void AddFeatures(VkPhysicalDevice physical_device, vku::safe_VkDeviceCreateInfo* modified_create_info, const Location& loc);
    bool timeline_khr_{false};
};

class Validator : public GpuShaderInstrumentor {
    using BaseClass = GpuShaderInstrumentor;
    using Func = vvl::Func;
    using Struct = vvl::Struct;
    using Field = vvl::Field;

  public:
    Validator(vvl::dispatch::Device* dev, Instance* instance_vo)
        : BaseClass(dev, instance_vo, LayerObjectTypeGpuAssisted),
          global_indices_buffer_(*this),
          global_resource_descriptor_buffer_(*this) {}

    // gpuav_setup.cpp
    // -------------
  public:
    void FinishDeviceSetup(const VkDeviceCreateInfo* pCreateInfo, const Location& loc) final;

    void InternalVmaError(LogObjectList objlist, VkResult result, const char* const specific_message) const;
    bool IsAllDeviceLocalMappable() const;

  private:
    void InitSettings(const Location& loc);
    void DestroySubstate();
    void BindBufferMemory(VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize offset);

    // gpuav_record.cpp
    // --------------
  public:
    void PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) final;
    void PreCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkBuffer* pBuffer, const RecordObject& record_obj, chassis::CreateBuffer& chassis_state) final;
    void PreCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo,
                                         const RecordObject& record_obj) final;

    void PostCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                    VkBuffer* pBuffer, const RecordObject& record_obj) final;
    void PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator,
                                    const RecordObject& record_obj) final;
    void PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator,
                                 const RecordObject& record_obj) final;
    void PostCallRecordBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                        const RecordObject& record_obj) final;
    void PostCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                         const RecordObject& record_obj) final;
    void PostCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                            const RecordObject& record_obj) final;
    void PreCallRecordCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                  const VkDescriptorBufferBindingInfoEXT* pBindingInfos,
                                                  const RecordObject& record_obj,
                                                  chassis::CmdBindDescriptorBuffers& chassis_state) final;

    void PreCallActionCommand(Validator& gpuav, CommandBufferSubState& cb_state, const LastBound& last_bound, const Location& loc);

    void PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                              uint32_t firstInstance, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                      uint32_t instanceCount, uint32_t firstInstance, uint32_t stride,
                                      const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                     uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                     const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                             const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                             uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset,
                                             const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                      uint32_t stride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                             uint32_t stride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                              uint32_t stride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                           uint32_t stride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance,
                                                  VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                  uint32_t vertexStride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                  uint32_t stride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                         const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                          uint32_t groupCountZ, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z,
                                  const RecordObject& record_obj) final;
    void PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                          const RecordObject& record_obj) final;
    void PreCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                                      uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                      const RecordObject& record_obj) final;
    void PreCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                         uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                         const RecordObject& record_obj) final;
    void PreCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                     VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                     VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                     VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                     VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                     VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                     uint32_t width, uint32_t height, uint32_t depth, const RecordObject& record_obj) final;
    void PreCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                      const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                      const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                      const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                      const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width,
                                      uint32_t height, uint32_t depth, const RecordObject& record_obj) final;
    void PreCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                              const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                              const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                              const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                              const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                              VkDeviceAddress indirectDeviceAddress, const RecordObject& record_obj) final;
    void PreCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                               const RecordObject& record_obj) final;
    void PreCallRecordCmdExecuteGeneratedCommandsEXT(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                     const VkGeneratedCommandsInfoEXT* pGeneratedCommandsInfo,
                                                     const RecordObject& record_obj) final;
    void PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                           VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                           const RecordObject&) final;
    void PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                               const VkCopyBufferToImageInfo2KHR* pCopyBufferToImageInfo2KHR,
                                               const RecordObject&) final;
    void PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                            const RecordObject&) final;

    bool ValidateProtectedImage(const vvl::CommandBuffer& cb_state, const vvl::Image& image_state, const Location& image_loc,
                                const char* vuid, const char* more_message = "") const final;
    bool ValidateUnprotectedImage(const vvl::CommandBuffer& cb_state, const vvl::Image& image_state, const Location& image_loc,
                                  const char* vuid, const char* more_message = "") const final;
    bool ValidateProtectedBuffer(const vvl::CommandBuffer& cb_state, const vvl::Buffer& buffer_state, const Location& buffer_loc,
                                 const char* vuid, const char* more_message = "") const final;
    bool ValidateUnprotectedBuffer(const vvl::CommandBuffer& cb_state, const vvl::Buffer& buffer_state, const Location& buffer_loc,
                                   const char* vuid, const char* more_message = "") const final;
    bool ValidateProtectedTensor(const vvl::CommandBuffer& cb_state, const vvl::Tensor& tensor_state, const Location& tensor_loc,
                                 const char* vuid, const char* more_message = "") const final;
    bool ValidateUnprotectedTensor(const vvl::CommandBuffer& cb_state, const vvl::Tensor& tensor_state, const Location& tensor_loc,
                                   const char* vuid, const char* more_message = "") const final;

    void Created(vvl::DescriptorSet& set) final;
    void Created(vvl::CommandBuffer& cb_state) final;
    void Created(vvl::Queue& queue) final;
    void Created(vvl::Image&) final;
    void Created(vvl::ImageView&) final;
    void Created(vvl::Buffer&) final;
    void Created(vvl::BufferView&) final;
    void Created(vvl::Sampler&) final;
    void Created(vvl::AccelerationStructureNV&) final;
    void Created(vvl::AccelerationStructureKHR&) final;
    void Created(vvl::Tensor&) final;
    void Created(vvl::TensorView&) final;
    void Created(vvl::ShaderObject&) final;
    void Created(vvl::Pipeline&) final;

    void DebugCapture() final;

  public:
    // We find ourselves constantly needing to create some resource for the "lifetime of GPU-AV"
    // We don't want a messy global space to managae it and use this to allow each check to manage the resource where it is used.
    // The goal is the first time we need the resource, we create it then, and afterwards, its cached and we can regain
    vko::SharedResourcesCache shared_resources_manager;

    PFN_vkSetDeviceLoaderData vk_set_device_loader_data_;

    VmaAllocator vma_allocator_ = {};
    std::unique_ptr<vko::DescriptorSetManager> desc_set_manager_;

    // This is so universally used, that we decided currently to not be in vko::SharedResourcesCache
    // This is just a buffer with a uint32_t value from [0, cts::indices_count - 1] so we can update prior to an action command
    // (draw/dispatch) to know where it came from
    vko::Buffer global_indices_buffer_;
    uint32_t indices_buffer_alignment_ = 0;

    // VK_EXT_descriptor_buffer global tracking
    //
    // Our internal Descriptor Buffer we will use
    vko::Buffer global_resource_descriptor_buffer_;
    // TODO - These are not needed for DebugPrintf, but will be needed for GPU-AV to track the descriptor used
    // Most common apps will have few, but large descriptor buffers
    vvl::unordered_set<VkBuffer> resource_descriptor_buffer_handles_;
    // We need to track handles in order to adjust vkMapMemory calls
    vvl::unordered_set<VkDeviceMemory> resource_descriptor_buffer_memory_handles_;

  private:
    std::string instrumented_shader_cache_path_{};

    // Make sure we call the right versions of any timeline semaphore functions.
    bool timeline_khr_{false};
};

}  // namespace gpuav
