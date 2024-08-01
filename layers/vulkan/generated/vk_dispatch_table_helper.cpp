// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See dispatch_table_helper_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google Inc.
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
 ****************************************************************************/

// NOLINTBEGIN
#include "vk_dispatch_table_helper.h"

// These are stub functions so Android can make use of debug extensions not supported in the Android Loader
static VKAPI_ATTR VkResult VKAPI_CALL StubDebugMarkerSetObjectTagEXT(VkDevice, const VkDebugMarkerObjectTagInfoEXT*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubDebugMarkerSetObjectNameEXT(VkDevice, const VkDebugMarkerObjectNameInfoEXT*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdDebugMarkerBeginEXT(VkCommandBuffer, const VkDebugMarkerMarkerInfoEXT*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDebugMarkerEndEXT(VkCommandBuffer) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDebugMarkerInsertEXT(VkCommandBuffer, const VkDebugMarkerMarkerInfoEXT*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubSetDebugUtilsObjectNameEXT(VkDevice, const VkDebugUtilsObjectNameInfoEXT*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubSetDebugUtilsObjectTagEXT(VkDevice, const VkDebugUtilsObjectTagInfoEXT*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubQueueBeginDebugUtilsLabelEXT(VkQueue, const VkDebugUtilsLabelEXT*) {}
static VKAPI_ATTR void VKAPI_CALL StubQueueEndDebugUtilsLabelEXT(VkQueue) {}
static VKAPI_ATTR void VKAPI_CALL StubQueueInsertDebugUtilsLabelEXT(VkQueue, const VkDebugUtilsLabelEXT*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBeginDebugUtilsLabelEXT(VkCommandBuffer, const VkDebugUtilsLabelEXT*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdEndDebugUtilsLabelEXT(VkCommandBuffer) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdInsertDebugUtilsLabelEXT(VkCommandBuffer, const VkDebugUtilsLabelEXT*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateDebugUtilsMessengerEXT(VkInstance, const VkDebugUtilsMessengerCreateInfoEXT*,
                                                                       const VkAllocationCallbacks*, VkDebugUtilsMessengerEXT*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyDebugUtilsMessengerEXT(VkInstance, VkDebugUtilsMessengerEXT,
                                                                    const VkAllocationCallbacks*) {}
static VKAPI_ATTR void VKAPI_CALL StubSubmitDebugUtilsMessageEXT(VkInstance, VkDebugUtilsMessageSeverityFlagBitsEXT,
                                                                 VkDebugUtilsMessageTypeFlagsEXT,
                                                                 const VkDebugUtilsMessengerCallbackDataEXT*) {}

const auto& GetApiPromotedMap() {
    static const vvl::unordered_map<std::string, std::string> api_promoted_map{
        {"vkBindBufferMemory2", {"VK_VERSION_1_1"}},
        {"vkBindImageMemory2", {"VK_VERSION_1_1"}},
        {"vkGetDeviceGroupPeerMemoryFeatures", {"VK_VERSION_1_1"}},
        {"vkCmdSetDeviceMask", {"VK_VERSION_1_1"}},
        {"vkCmdDispatchBase", {"VK_VERSION_1_1"}},
        {"vkGetImageMemoryRequirements2", {"VK_VERSION_1_1"}},
        {"vkGetBufferMemoryRequirements2", {"VK_VERSION_1_1"}},
        {"vkGetImageSparseMemoryRequirements2", {"VK_VERSION_1_1"}},
        {"vkTrimCommandPool", {"VK_VERSION_1_1"}},
        {"vkGetDeviceQueue2", {"VK_VERSION_1_1"}},
        {"vkCreateSamplerYcbcrConversion", {"VK_VERSION_1_1"}},
        {"vkDestroySamplerYcbcrConversion", {"VK_VERSION_1_1"}},
        {"vkCreateDescriptorUpdateTemplate", {"VK_VERSION_1_1"}},
        {"vkDestroyDescriptorUpdateTemplate", {"VK_VERSION_1_1"}},
        {"vkUpdateDescriptorSetWithTemplate", {"VK_VERSION_1_1"}},
        {"vkGetDescriptorSetLayoutSupport", {"VK_VERSION_1_1"}},
        {"vkCmdDrawIndirectCount", {"VK_VERSION_1_2"}},
        {"vkCmdDrawIndexedIndirectCount", {"VK_VERSION_1_2"}},
        {"vkCreateRenderPass2", {"VK_VERSION_1_2"}},
        {"vkCmdBeginRenderPass2", {"VK_VERSION_1_2"}},
        {"vkCmdNextSubpass2", {"VK_VERSION_1_2"}},
        {"vkCmdEndRenderPass2", {"VK_VERSION_1_2"}},
        {"vkResetQueryPool", {"VK_VERSION_1_2"}},
        {"vkGetSemaphoreCounterValue", {"VK_VERSION_1_2"}},
        {"vkWaitSemaphores", {"VK_VERSION_1_2"}},
        {"vkSignalSemaphore", {"VK_VERSION_1_2"}},
        {"vkGetBufferDeviceAddress", {"VK_VERSION_1_2"}},
        {"vkGetBufferOpaqueCaptureAddress", {"VK_VERSION_1_2"}},
        {"vkGetDeviceMemoryOpaqueCaptureAddress", {"VK_VERSION_1_2"}},
        {"vkCreatePrivateDataSlot", {"VK_VERSION_1_3"}},
        {"vkDestroyPrivateDataSlot", {"VK_VERSION_1_3"}},
        {"vkSetPrivateData", {"VK_VERSION_1_3"}},
        {"vkGetPrivateData", {"VK_VERSION_1_3"}},
        {"vkCmdSetEvent2", {"VK_VERSION_1_3"}},
        {"vkCmdResetEvent2", {"VK_VERSION_1_3"}},
        {"vkCmdWaitEvents2", {"VK_VERSION_1_3"}},
        {"vkCmdPipelineBarrier2", {"VK_VERSION_1_3"}},
        {"vkCmdWriteTimestamp2", {"VK_VERSION_1_3"}},
        {"vkQueueSubmit2", {"VK_VERSION_1_3"}},
        {"vkCmdCopyBuffer2", {"VK_VERSION_1_3"}},
        {"vkCmdCopyImage2", {"VK_VERSION_1_3"}},
        {"vkCmdCopyBufferToImage2", {"VK_VERSION_1_3"}},
        {"vkCmdCopyImageToBuffer2", {"VK_VERSION_1_3"}},
        {"vkCmdBlitImage2", {"VK_VERSION_1_3"}},
        {"vkCmdResolveImage2", {"VK_VERSION_1_3"}},
        {"vkCmdBeginRendering", {"VK_VERSION_1_3"}},
        {"vkCmdEndRendering", {"VK_VERSION_1_3"}},
        {"vkCmdSetCullMode", {"VK_VERSION_1_3"}},
        {"vkCmdSetFrontFace", {"VK_VERSION_1_3"}},
        {"vkCmdSetPrimitiveTopology", {"VK_VERSION_1_3"}},
        {"vkCmdSetViewportWithCount", {"VK_VERSION_1_3"}},
        {"vkCmdSetScissorWithCount", {"VK_VERSION_1_3"}},
        {"vkCmdBindVertexBuffers2", {"VK_VERSION_1_3"}},
        {"vkCmdSetDepthTestEnable", {"VK_VERSION_1_3"}},
        {"vkCmdSetDepthWriteEnable", {"VK_VERSION_1_3"}},
        {"vkCmdSetDepthCompareOp", {"VK_VERSION_1_3"}},
        {"vkCmdSetDepthBoundsTestEnable", {"VK_VERSION_1_3"}},
        {"vkCmdSetStencilTestEnable", {"VK_VERSION_1_3"}},
        {"vkCmdSetStencilOp", {"VK_VERSION_1_3"}},
        {"vkCmdSetRasterizerDiscardEnable", {"VK_VERSION_1_3"}},
        {"vkCmdSetDepthBiasEnable", {"VK_VERSION_1_3"}},
        {"vkCmdSetPrimitiveRestartEnable", {"VK_VERSION_1_3"}},
        {"vkGetDeviceBufferMemoryRequirements", {"VK_VERSION_1_3"}},
        {"vkGetDeviceImageMemoryRequirements", {"VK_VERSION_1_3"}},
        {"vkGetDeviceImageSparseMemoryRequirements", {"VK_VERSION_1_3"}},
    };
    return api_promoted_map;
}
const auto& GetApiExtensionMap() {
    static const vvl::unordered_map<std::string, small_vector<vvl::Extension, 2, size_t>> api_extension_map{
        {"vkCreateSwapchainKHR", {vvl::Extension::_VK_KHR_swapchain}},
        {"vkDestroySwapchainKHR", {vvl::Extension::_VK_KHR_swapchain}},
        {"vkGetSwapchainImagesKHR", {vvl::Extension::_VK_KHR_swapchain}},
        {"vkAcquireNextImageKHR", {vvl::Extension::_VK_KHR_swapchain}},
        {"vkQueuePresentKHR", {vvl::Extension::_VK_KHR_swapchain}},
        {"vkGetDeviceGroupPresentCapabilitiesKHR", {vvl::Extension::_VK_KHR_swapchain, vvl::Extension::_VK_KHR_device_group}},
        {"vkGetDeviceGroupSurfacePresentModesKHR", {vvl::Extension::_VK_KHR_swapchain, vvl::Extension::_VK_KHR_device_group}},
        {"vkAcquireNextImage2KHR", {vvl::Extension::_VK_KHR_swapchain, vvl::Extension::_VK_KHR_device_group}},
        {"vkCreateSharedSwapchainsKHR", {vvl::Extension::_VK_KHR_display_swapchain}},
        {"vkCreateVideoSessionKHR", {vvl::Extension::_VK_KHR_video_queue}},
        {"vkDestroyVideoSessionKHR", {vvl::Extension::_VK_KHR_video_queue}},
        {"vkGetVideoSessionMemoryRequirementsKHR", {vvl::Extension::_VK_KHR_video_queue}},
        {"vkBindVideoSessionMemoryKHR", {vvl::Extension::_VK_KHR_video_queue}},
        {"vkCreateVideoSessionParametersKHR", {vvl::Extension::_VK_KHR_video_queue}},
        {"vkUpdateVideoSessionParametersKHR", {vvl::Extension::_VK_KHR_video_queue}},
        {"vkDestroyVideoSessionParametersKHR", {vvl::Extension::_VK_KHR_video_queue}},
        {"vkCmdBeginVideoCodingKHR", {vvl::Extension::_VK_KHR_video_queue}},
        {"vkCmdEndVideoCodingKHR", {vvl::Extension::_VK_KHR_video_queue}},
        {"vkCmdControlVideoCodingKHR", {vvl::Extension::_VK_KHR_video_queue}},
        {"vkCmdDecodeVideoKHR", {vvl::Extension::_VK_KHR_video_decode_queue}},
        {"vkCmdBeginRenderingKHR", {vvl::Extension::_VK_KHR_dynamic_rendering}},
        {"vkCmdEndRenderingKHR", {vvl::Extension::_VK_KHR_dynamic_rendering}},
        {"vkGetDeviceGroupPeerMemoryFeaturesKHR", {vvl::Extension::_VK_KHR_device_group}},
        {"vkCmdSetDeviceMaskKHR", {vvl::Extension::_VK_KHR_device_group}},
        {"vkCmdDispatchBaseKHR", {vvl::Extension::_VK_KHR_device_group}},
        {"vkTrimCommandPoolKHR", {vvl::Extension::_VK_KHR_maintenance1}},
        {"vkGetMemoryWin32HandleKHR", {vvl::Extension::_VK_KHR_external_memory_win32}},
        {"vkGetMemoryWin32HandlePropertiesKHR", {vvl::Extension::_VK_KHR_external_memory_win32}},
        {"vkGetMemoryFdKHR", {vvl::Extension::_VK_KHR_external_memory_fd}},
        {"vkGetMemoryFdPropertiesKHR", {vvl::Extension::_VK_KHR_external_memory_fd}},
        {"vkImportSemaphoreWin32HandleKHR", {vvl::Extension::_VK_KHR_external_semaphore_win32}},
        {"vkGetSemaphoreWin32HandleKHR", {vvl::Extension::_VK_KHR_external_semaphore_win32}},
        {"vkImportSemaphoreFdKHR", {vvl::Extension::_VK_KHR_external_semaphore_fd}},
        {"vkGetSemaphoreFdKHR", {vvl::Extension::_VK_KHR_external_semaphore_fd}},
        {"vkCmdPushDescriptorSetKHR", {vvl::Extension::_VK_KHR_push_descriptor}},
        {"vkCmdPushDescriptorSetWithTemplateKHR",
         {vvl::Extension::_VK_KHR_push_descriptor, vvl::Extension::_VK_KHR_descriptor_update_template}},
        {"vkCreateDescriptorUpdateTemplateKHR", {vvl::Extension::_VK_KHR_descriptor_update_template}},
        {"vkDestroyDescriptorUpdateTemplateKHR", {vvl::Extension::_VK_KHR_descriptor_update_template}},
        {"vkUpdateDescriptorSetWithTemplateKHR", {vvl::Extension::_VK_KHR_descriptor_update_template}},
        {"vkCreateRenderPass2KHR", {vvl::Extension::_VK_KHR_create_renderpass2}},
        {"vkCmdBeginRenderPass2KHR", {vvl::Extension::_VK_KHR_create_renderpass2}},
        {"vkCmdNextSubpass2KHR", {vvl::Extension::_VK_KHR_create_renderpass2}},
        {"vkCmdEndRenderPass2KHR", {vvl::Extension::_VK_KHR_create_renderpass2}},
        {"vkGetSwapchainStatusKHR", {vvl::Extension::_VK_KHR_shared_presentable_image}},
        {"vkImportFenceWin32HandleKHR", {vvl::Extension::_VK_KHR_external_fence_win32}},
        {"vkGetFenceWin32HandleKHR", {vvl::Extension::_VK_KHR_external_fence_win32}},
        {"vkImportFenceFdKHR", {vvl::Extension::_VK_KHR_external_fence_fd}},
        {"vkGetFenceFdKHR", {vvl::Extension::_VK_KHR_external_fence_fd}},
        {"vkAcquireProfilingLockKHR", {vvl::Extension::_VK_KHR_performance_query}},
        {"vkReleaseProfilingLockKHR", {vvl::Extension::_VK_KHR_performance_query}},
        {"vkGetImageMemoryRequirements2KHR", {vvl::Extension::_VK_KHR_get_memory_requirements2}},
        {"vkGetBufferMemoryRequirements2KHR", {vvl::Extension::_VK_KHR_get_memory_requirements2}},
        {"vkGetImageSparseMemoryRequirements2KHR", {vvl::Extension::_VK_KHR_get_memory_requirements2}},
        {"vkCreateSamplerYcbcrConversionKHR", {vvl::Extension::_VK_KHR_sampler_ycbcr_conversion}},
        {"vkDestroySamplerYcbcrConversionKHR", {vvl::Extension::_VK_KHR_sampler_ycbcr_conversion}},
        {"vkBindBufferMemory2KHR", {vvl::Extension::_VK_KHR_bind_memory2}},
        {"vkBindImageMemory2KHR", {vvl::Extension::_VK_KHR_bind_memory2}},
        {"vkGetDescriptorSetLayoutSupportKHR", {vvl::Extension::_VK_KHR_maintenance3}},
        {"vkCmdDrawIndirectCountKHR", {vvl::Extension::_VK_KHR_draw_indirect_count}},
        {"vkCmdDrawIndexedIndirectCountKHR", {vvl::Extension::_VK_KHR_draw_indirect_count}},
        {"vkGetSemaphoreCounterValueKHR", {vvl::Extension::_VK_KHR_timeline_semaphore}},
        {"vkWaitSemaphoresKHR", {vvl::Extension::_VK_KHR_timeline_semaphore}},
        {"vkSignalSemaphoreKHR", {vvl::Extension::_VK_KHR_timeline_semaphore}},
        {"vkCmdSetFragmentShadingRateKHR", {vvl::Extension::_VK_KHR_fragment_shading_rate}},
        {"vkCmdSetRenderingAttachmentLocationsKHR", {vvl::Extension::_VK_KHR_dynamic_rendering_local_read}},
        {"vkCmdSetRenderingInputAttachmentIndicesKHR", {vvl::Extension::_VK_KHR_dynamic_rendering_local_read}},
        {"vkWaitForPresentKHR", {vvl::Extension::_VK_KHR_present_wait}},
        {"vkGetBufferDeviceAddressKHR", {vvl::Extension::_VK_KHR_buffer_device_address}},
        {"vkGetBufferOpaqueCaptureAddressKHR", {vvl::Extension::_VK_KHR_buffer_device_address}},
        {"vkGetDeviceMemoryOpaqueCaptureAddressKHR", {vvl::Extension::_VK_KHR_buffer_device_address}},
        {"vkCreateDeferredOperationKHR", {vvl::Extension::_VK_KHR_deferred_host_operations}},
        {"vkDestroyDeferredOperationKHR", {vvl::Extension::_VK_KHR_deferred_host_operations}},
        {"vkGetDeferredOperationMaxConcurrencyKHR", {vvl::Extension::_VK_KHR_deferred_host_operations}},
        {"vkGetDeferredOperationResultKHR", {vvl::Extension::_VK_KHR_deferred_host_operations}},
        {"vkDeferredOperationJoinKHR", {vvl::Extension::_VK_KHR_deferred_host_operations}},
        {"vkGetPipelineExecutablePropertiesKHR", {vvl::Extension::_VK_KHR_pipeline_executable_properties}},
        {"vkGetPipelineExecutableStatisticsKHR", {vvl::Extension::_VK_KHR_pipeline_executable_properties}},
        {"vkGetPipelineExecutableInternalRepresentationsKHR", {vvl::Extension::_VK_KHR_pipeline_executable_properties}},
        {"vkMapMemory2KHR", {vvl::Extension::_VK_KHR_map_memory2}},
        {"vkUnmapMemory2KHR", {vvl::Extension::_VK_KHR_map_memory2}},
        {"vkGetEncodedVideoSessionParametersKHR", {vvl::Extension::_VK_KHR_video_encode_queue}},
        {"vkCmdEncodeVideoKHR", {vvl::Extension::_VK_KHR_video_encode_queue}},
        {"vkCmdSetEvent2KHR", {vvl::Extension::_VK_KHR_synchronization2}},
        {"vkCmdResetEvent2KHR", {vvl::Extension::_VK_KHR_synchronization2}},
        {"vkCmdWaitEvents2KHR", {vvl::Extension::_VK_KHR_synchronization2}},
        {"vkCmdPipelineBarrier2KHR", {vvl::Extension::_VK_KHR_synchronization2}},
        {"vkCmdWriteTimestamp2KHR", {vvl::Extension::_VK_KHR_synchronization2}},
        {"vkQueueSubmit2KHR", {vvl::Extension::_VK_KHR_synchronization2}},
        {"vkCmdWriteBufferMarker2AMD", {vvl::Extension::_VK_KHR_synchronization2}},
        {"vkGetQueueCheckpointData2NV", {vvl::Extension::_VK_KHR_synchronization2}},
        {"vkCmdCopyBuffer2KHR", {vvl::Extension::_VK_KHR_copy_commands2}},
        {"vkCmdCopyImage2KHR", {vvl::Extension::_VK_KHR_copy_commands2}},
        {"vkCmdCopyBufferToImage2KHR", {vvl::Extension::_VK_KHR_copy_commands2}},
        {"vkCmdCopyImageToBuffer2KHR", {vvl::Extension::_VK_KHR_copy_commands2}},
        {"vkCmdBlitImage2KHR", {vvl::Extension::_VK_KHR_copy_commands2}},
        {"vkCmdResolveImage2KHR", {vvl::Extension::_VK_KHR_copy_commands2}},
        {"vkCmdTraceRaysIndirect2KHR", {vvl::Extension::_VK_KHR_ray_tracing_maintenance1}},
        {"vkGetDeviceBufferMemoryRequirementsKHR", {vvl::Extension::_VK_KHR_maintenance4}},
        {"vkGetDeviceImageMemoryRequirementsKHR", {vvl::Extension::_VK_KHR_maintenance4}},
        {"vkGetDeviceImageSparseMemoryRequirementsKHR", {vvl::Extension::_VK_KHR_maintenance4}},
        {"vkCmdBindIndexBuffer2KHR", {vvl::Extension::_VK_KHR_maintenance5}},
        {"vkGetRenderingAreaGranularityKHR", {vvl::Extension::_VK_KHR_maintenance5}},
        {"vkGetDeviceImageSubresourceLayoutKHR", {vvl::Extension::_VK_KHR_maintenance5}},
        {"vkGetImageSubresourceLayout2KHR", {vvl::Extension::_VK_KHR_maintenance5}},
        {"vkCmdSetLineStippleKHR", {vvl::Extension::_VK_KHR_line_rasterization}},
        {"vkGetCalibratedTimestampsKHR", {vvl::Extension::_VK_KHR_calibrated_timestamps}},
        {"vkCmdBindDescriptorSets2KHR", {vvl::Extension::_VK_KHR_maintenance6}},
        {"vkCmdPushConstants2KHR", {vvl::Extension::_VK_KHR_maintenance6}},
        {"vkCmdPushDescriptorSet2KHR", {vvl::Extension::_VK_KHR_maintenance6}},
        {"vkCmdPushDescriptorSetWithTemplate2KHR", {vvl::Extension::_VK_KHR_maintenance6}},
        {"vkCmdSetDescriptorBufferOffsets2EXT", {vvl::Extension::_VK_KHR_maintenance6}},
        {"vkCmdBindDescriptorBufferEmbeddedSamplers2EXT", {vvl::Extension::_VK_KHR_maintenance6}},
        {"vkDebugMarkerSetObjectTagEXT", {vvl::Extension::_VK_EXT_debug_marker}},
        {"vkDebugMarkerSetObjectNameEXT", {vvl::Extension::_VK_EXT_debug_marker}},
        {"vkCmdDebugMarkerBeginEXT", {vvl::Extension::_VK_EXT_debug_marker}},
        {"vkCmdDebugMarkerEndEXT", {vvl::Extension::_VK_EXT_debug_marker}},
        {"vkCmdDebugMarkerInsertEXT", {vvl::Extension::_VK_EXT_debug_marker}},
        {"vkCmdBindTransformFeedbackBuffersEXT", {vvl::Extension::_VK_EXT_transform_feedback}},
        {"vkCmdBeginTransformFeedbackEXT", {vvl::Extension::_VK_EXT_transform_feedback}},
        {"vkCmdEndTransformFeedbackEXT", {vvl::Extension::_VK_EXT_transform_feedback}},
        {"vkCmdBeginQueryIndexedEXT", {vvl::Extension::_VK_EXT_transform_feedback}},
        {"vkCmdEndQueryIndexedEXT", {vvl::Extension::_VK_EXT_transform_feedback}},
        {"vkCmdDrawIndirectByteCountEXT", {vvl::Extension::_VK_EXT_transform_feedback}},
        {"vkCreateCuModuleNVX", {vvl::Extension::_VK_NVX_binary_import}},
        {"vkCreateCuFunctionNVX", {vvl::Extension::_VK_NVX_binary_import}},
        {"vkDestroyCuModuleNVX", {vvl::Extension::_VK_NVX_binary_import}},
        {"vkDestroyCuFunctionNVX", {vvl::Extension::_VK_NVX_binary_import}},
        {"vkCmdCuLaunchKernelNVX", {vvl::Extension::_VK_NVX_binary_import}},
        {"vkGetImageViewHandleNVX", {vvl::Extension::_VK_NVX_image_view_handle}},
        {"vkGetImageViewAddressNVX", {vvl::Extension::_VK_NVX_image_view_handle}},
        {"vkCmdDrawIndirectCountAMD", {vvl::Extension::_VK_AMD_draw_indirect_count}},
        {"vkCmdDrawIndexedIndirectCountAMD", {vvl::Extension::_VK_AMD_draw_indirect_count}},
        {"vkGetShaderInfoAMD", {vvl::Extension::_VK_AMD_shader_info}},
        {"vkGetMemoryWin32HandleNV", {vvl::Extension::_VK_NV_external_memory_win32}},
        {"vkCmdBeginConditionalRenderingEXT", {vvl::Extension::_VK_EXT_conditional_rendering}},
        {"vkCmdEndConditionalRenderingEXT", {vvl::Extension::_VK_EXT_conditional_rendering}},
        {"vkCmdSetViewportWScalingNV", {vvl::Extension::_VK_NV_clip_space_w_scaling}},
        {"vkDisplayPowerControlEXT", {vvl::Extension::_VK_EXT_display_control}},
        {"vkRegisterDeviceEventEXT", {vvl::Extension::_VK_EXT_display_control}},
        {"vkRegisterDisplayEventEXT", {vvl::Extension::_VK_EXT_display_control}},
        {"vkGetSwapchainCounterEXT", {vvl::Extension::_VK_EXT_display_control}},
        {"vkGetRefreshCycleDurationGOOGLE", {vvl::Extension::_VK_GOOGLE_display_timing}},
        {"vkGetPastPresentationTimingGOOGLE", {vvl::Extension::_VK_GOOGLE_display_timing}},
        {"vkCmdSetDiscardRectangleEXT", {vvl::Extension::_VK_EXT_discard_rectangles}},
        {"vkCmdSetDiscardRectangleEnableEXT", {vvl::Extension::_VK_EXT_discard_rectangles}},
        {"vkCmdSetDiscardRectangleModeEXT", {vvl::Extension::_VK_EXT_discard_rectangles}},
        {"vkSetHdrMetadataEXT", {vvl::Extension::_VK_EXT_hdr_metadata}},
        {"vkSetDebugUtilsObjectNameEXT", {vvl::Extension::_VK_EXT_debug_utils}},
        {"vkSetDebugUtilsObjectTagEXT", {vvl::Extension::_VK_EXT_debug_utils}},
        {"vkQueueBeginDebugUtilsLabelEXT", {vvl::Extension::_VK_EXT_debug_utils}},
        {"vkQueueEndDebugUtilsLabelEXT", {vvl::Extension::_VK_EXT_debug_utils}},
        {"vkQueueInsertDebugUtilsLabelEXT", {vvl::Extension::_VK_EXT_debug_utils}},
        {"vkCmdBeginDebugUtilsLabelEXT", {vvl::Extension::_VK_EXT_debug_utils}},
        {"vkCmdEndDebugUtilsLabelEXT", {vvl::Extension::_VK_EXT_debug_utils}},
        {"vkCmdInsertDebugUtilsLabelEXT", {vvl::Extension::_VK_EXT_debug_utils}},
        {"vkGetAndroidHardwareBufferPropertiesANDROID", {vvl::Extension::_VK_ANDROID_external_memory_android_hardware_buffer}},
        {"vkGetMemoryAndroidHardwareBufferANDROID", {vvl::Extension::_VK_ANDROID_external_memory_android_hardware_buffer}},
        {"vkCreateExecutionGraphPipelinesAMDX", {vvl::Extension::_VK_AMDX_shader_enqueue}},
        {"vkGetExecutionGraphPipelineScratchSizeAMDX", {vvl::Extension::_VK_AMDX_shader_enqueue}},
        {"vkGetExecutionGraphPipelineNodeIndexAMDX", {vvl::Extension::_VK_AMDX_shader_enqueue}},
        {"vkCmdInitializeGraphScratchMemoryAMDX", {vvl::Extension::_VK_AMDX_shader_enqueue}},
        {"vkCmdDispatchGraphAMDX", {vvl::Extension::_VK_AMDX_shader_enqueue}},
        {"vkCmdDispatchGraphIndirectAMDX", {vvl::Extension::_VK_AMDX_shader_enqueue}},
        {"vkCmdDispatchGraphIndirectCountAMDX", {vvl::Extension::_VK_AMDX_shader_enqueue}},
        {"vkCmdSetSampleLocationsEXT", {vvl::Extension::_VK_EXT_sample_locations}},
        {"vkGetImageDrmFormatModifierPropertiesEXT", {vvl::Extension::_VK_EXT_image_drm_format_modifier}},
        {"vkCreateValidationCacheEXT", {vvl::Extension::_VK_EXT_validation_cache}},
        {"vkDestroyValidationCacheEXT", {vvl::Extension::_VK_EXT_validation_cache}},
        {"vkMergeValidationCachesEXT", {vvl::Extension::_VK_EXT_validation_cache}},
        {"vkGetValidationCacheDataEXT", {vvl::Extension::_VK_EXT_validation_cache}},
        {"vkCmdBindShadingRateImageNV", {vvl::Extension::_VK_NV_shading_rate_image}},
        {"vkCmdSetViewportShadingRatePaletteNV", {vvl::Extension::_VK_NV_shading_rate_image}},
        {"vkCmdSetCoarseSampleOrderNV", {vvl::Extension::_VK_NV_shading_rate_image}},
        {"vkCreateAccelerationStructureNV", {vvl::Extension::_VK_NV_ray_tracing}},
        {"vkDestroyAccelerationStructureNV", {vvl::Extension::_VK_NV_ray_tracing}},
        {"vkGetAccelerationStructureMemoryRequirementsNV", {vvl::Extension::_VK_NV_ray_tracing}},
        {"vkBindAccelerationStructureMemoryNV", {vvl::Extension::_VK_NV_ray_tracing}},
        {"vkCmdBuildAccelerationStructureNV", {vvl::Extension::_VK_NV_ray_tracing}},
        {"vkCmdCopyAccelerationStructureNV", {vvl::Extension::_VK_NV_ray_tracing}},
        {"vkCmdTraceRaysNV", {vvl::Extension::_VK_NV_ray_tracing}},
        {"vkCreateRayTracingPipelinesNV", {vvl::Extension::_VK_NV_ray_tracing}},
        {"vkGetRayTracingShaderGroupHandlesKHR", {vvl::Extension::_VK_KHR_ray_tracing_pipeline}},
        {"vkGetRayTracingShaderGroupHandlesNV", {vvl::Extension::_VK_NV_ray_tracing}},
        {"vkGetAccelerationStructureHandleNV", {vvl::Extension::_VK_NV_ray_tracing}},
        {"vkCmdWriteAccelerationStructuresPropertiesNV", {vvl::Extension::_VK_NV_ray_tracing}},
        {"vkCompileDeferredNV", {vvl::Extension::_VK_NV_ray_tracing}},
        {"vkGetMemoryHostPointerPropertiesEXT", {vvl::Extension::_VK_EXT_external_memory_host}},
        {"vkCmdWriteBufferMarkerAMD", {vvl::Extension::_VK_AMD_buffer_marker}},
        {"vkGetCalibratedTimestampsEXT", {vvl::Extension::_VK_EXT_calibrated_timestamps}},
        {"vkCmdDrawMeshTasksNV", {vvl::Extension::_VK_NV_mesh_shader}},
        {"vkCmdDrawMeshTasksIndirectNV", {vvl::Extension::_VK_NV_mesh_shader}},
        {"vkCmdDrawMeshTasksIndirectCountNV", {vvl::Extension::_VK_NV_mesh_shader}},
        {"vkCmdSetExclusiveScissorEnableNV", {vvl::Extension::_VK_NV_scissor_exclusive}},
        {"vkCmdSetExclusiveScissorNV", {vvl::Extension::_VK_NV_scissor_exclusive}},
        {"vkCmdSetCheckpointNV", {vvl::Extension::_VK_NV_device_diagnostic_checkpoints}},
        {"vkGetQueueCheckpointDataNV", {vvl::Extension::_VK_NV_device_diagnostic_checkpoints}},
        {"vkInitializePerformanceApiINTEL", {vvl::Extension::_VK_INTEL_performance_query}},
        {"vkUninitializePerformanceApiINTEL", {vvl::Extension::_VK_INTEL_performance_query}},
        {"vkCmdSetPerformanceMarkerINTEL", {vvl::Extension::_VK_INTEL_performance_query}},
        {"vkCmdSetPerformanceStreamMarkerINTEL", {vvl::Extension::_VK_INTEL_performance_query}},
        {"vkCmdSetPerformanceOverrideINTEL", {vvl::Extension::_VK_INTEL_performance_query}},
        {"vkAcquirePerformanceConfigurationINTEL", {vvl::Extension::_VK_INTEL_performance_query}},
        {"vkReleasePerformanceConfigurationINTEL", {vvl::Extension::_VK_INTEL_performance_query}},
        {"vkQueueSetPerformanceConfigurationINTEL", {vvl::Extension::_VK_INTEL_performance_query}},
        {"vkGetPerformanceParameterINTEL", {vvl::Extension::_VK_INTEL_performance_query}},
        {"vkSetLocalDimmingAMD", {vvl::Extension::_VK_AMD_display_native_hdr}},
        {"vkGetBufferDeviceAddressEXT", {vvl::Extension::_VK_EXT_buffer_device_address}},
        {"vkAcquireFullScreenExclusiveModeEXT", {vvl::Extension::_VK_EXT_full_screen_exclusive}},
        {"vkReleaseFullScreenExclusiveModeEXT", {vvl::Extension::_VK_EXT_full_screen_exclusive}},
        {"vkGetDeviceGroupSurfacePresentModes2EXT", {vvl::Extension::_VK_EXT_full_screen_exclusive}},
        {"vkCmdSetLineStippleEXT", {vvl::Extension::_VK_EXT_line_rasterization}},
        {"vkResetQueryPoolEXT", {vvl::Extension::_VK_EXT_host_query_reset}},
        {"vkCmdSetCullModeEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetFrontFaceEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetPrimitiveTopologyEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetViewportWithCountEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetScissorWithCountEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdBindVertexBuffers2EXT", {vvl::Extension::_VK_EXT_extended_dynamic_state, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetDepthTestEnableEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetDepthWriteEnableEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetDepthCompareOpEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetDepthBoundsTestEnableEXT",
         {vvl::Extension::_VK_EXT_extended_dynamic_state, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetStencilTestEnableEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetStencilOpEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCopyMemoryToImageEXT", {vvl::Extension::_VK_EXT_host_image_copy}},
        {"vkCopyImageToMemoryEXT", {vvl::Extension::_VK_EXT_host_image_copy}},
        {"vkCopyImageToImageEXT", {vvl::Extension::_VK_EXT_host_image_copy}},
        {"vkTransitionImageLayoutEXT", {vvl::Extension::_VK_EXT_host_image_copy}},
        {"vkGetImageSubresourceLayout2EXT",
         {vvl::Extension::_VK_EXT_host_image_copy, vvl::Extension::_VK_EXT_image_compression_control}},
        {"vkReleaseSwapchainImagesEXT", {vvl::Extension::_VK_EXT_swapchain_maintenance1}},
        {"vkGetGeneratedCommandsMemoryRequirementsNV", {vvl::Extension::_VK_NV_device_generated_commands}},
        {"vkCmdPreprocessGeneratedCommandsNV", {vvl::Extension::_VK_NV_device_generated_commands}},
        {"vkCmdExecuteGeneratedCommandsNV", {vvl::Extension::_VK_NV_device_generated_commands}},
        {"vkCmdBindPipelineShaderGroupNV", {vvl::Extension::_VK_NV_device_generated_commands}},
        {"vkCreateIndirectCommandsLayoutNV", {vvl::Extension::_VK_NV_device_generated_commands}},
        {"vkDestroyIndirectCommandsLayoutNV", {vvl::Extension::_VK_NV_device_generated_commands}},
        {"vkCmdSetDepthBias2EXT", {vvl::Extension::_VK_EXT_depth_bias_control}},
        {"vkCreatePrivateDataSlotEXT", {vvl::Extension::_VK_EXT_private_data}},
        {"vkDestroyPrivateDataSlotEXT", {vvl::Extension::_VK_EXT_private_data}},
        {"vkSetPrivateDataEXT", {vvl::Extension::_VK_EXT_private_data}},
        {"vkGetPrivateDataEXT", {vvl::Extension::_VK_EXT_private_data}},
        {"vkCreateCudaModuleNV", {vvl::Extension::_VK_NV_cuda_kernel_launch}},
        {"vkGetCudaModuleCacheNV", {vvl::Extension::_VK_NV_cuda_kernel_launch}},
        {"vkCreateCudaFunctionNV", {vvl::Extension::_VK_NV_cuda_kernel_launch}},
        {"vkDestroyCudaModuleNV", {vvl::Extension::_VK_NV_cuda_kernel_launch}},
        {"vkDestroyCudaFunctionNV", {vvl::Extension::_VK_NV_cuda_kernel_launch}},
        {"vkCmdCudaLaunchKernelNV", {vvl::Extension::_VK_NV_cuda_kernel_launch}},
        {"vkExportMetalObjectsEXT", {vvl::Extension::_VK_EXT_metal_objects}},
        {"vkGetDescriptorSetLayoutSizeEXT", {vvl::Extension::_VK_EXT_descriptor_buffer}},
        {"vkGetDescriptorSetLayoutBindingOffsetEXT", {vvl::Extension::_VK_EXT_descriptor_buffer}},
        {"vkGetDescriptorEXT", {vvl::Extension::_VK_EXT_descriptor_buffer}},
        {"vkCmdBindDescriptorBuffersEXT", {vvl::Extension::_VK_EXT_descriptor_buffer}},
        {"vkCmdSetDescriptorBufferOffsetsEXT", {vvl::Extension::_VK_EXT_descriptor_buffer}},
        {"vkCmdBindDescriptorBufferEmbeddedSamplersEXT", {vvl::Extension::_VK_EXT_descriptor_buffer}},
        {"vkGetBufferOpaqueCaptureDescriptorDataEXT", {vvl::Extension::_VK_EXT_descriptor_buffer}},
        {"vkGetImageOpaqueCaptureDescriptorDataEXT", {vvl::Extension::_VK_EXT_descriptor_buffer}},
        {"vkGetImageViewOpaqueCaptureDescriptorDataEXT", {vvl::Extension::_VK_EXT_descriptor_buffer}},
        {"vkGetSamplerOpaqueCaptureDescriptorDataEXT", {vvl::Extension::_VK_EXT_descriptor_buffer}},
        {"vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT", {vvl::Extension::_VK_EXT_descriptor_buffer}},
        {"vkCmdSetFragmentShadingRateEnumNV", {vvl::Extension::_VK_NV_fragment_shading_rate_enums}},
        {"vkGetDeviceFaultInfoEXT", {vvl::Extension::_VK_EXT_device_fault}},
        {"vkCmdSetVertexInputEXT", {vvl::Extension::_VK_EXT_vertex_input_dynamic_state, vvl::Extension::_VK_EXT_shader_object}},
        {"vkGetMemoryZirconHandleFUCHSIA", {vvl::Extension::_VK_FUCHSIA_external_memory}},
        {"vkGetMemoryZirconHandlePropertiesFUCHSIA", {vvl::Extension::_VK_FUCHSIA_external_memory}},
        {"vkImportSemaphoreZirconHandleFUCHSIA", {vvl::Extension::_VK_FUCHSIA_external_semaphore}},
        {"vkGetSemaphoreZirconHandleFUCHSIA", {vvl::Extension::_VK_FUCHSIA_external_semaphore}},
        {"vkCreateBufferCollectionFUCHSIA", {vvl::Extension::_VK_FUCHSIA_buffer_collection}},
        {"vkSetBufferCollectionImageConstraintsFUCHSIA", {vvl::Extension::_VK_FUCHSIA_buffer_collection}},
        {"vkSetBufferCollectionBufferConstraintsFUCHSIA", {vvl::Extension::_VK_FUCHSIA_buffer_collection}},
        {"vkDestroyBufferCollectionFUCHSIA", {vvl::Extension::_VK_FUCHSIA_buffer_collection}},
        {"vkGetBufferCollectionPropertiesFUCHSIA", {vvl::Extension::_VK_FUCHSIA_buffer_collection}},
        {"vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI", {vvl::Extension::_VK_HUAWEI_subpass_shading}},
        {"vkCmdSubpassShadingHUAWEI", {vvl::Extension::_VK_HUAWEI_subpass_shading}},
        {"vkCmdBindInvocationMaskHUAWEI", {vvl::Extension::_VK_HUAWEI_invocation_mask}},
        {"vkGetMemoryRemoteAddressNV", {vvl::Extension::_VK_NV_external_memory_rdma}},
        {"vkGetPipelinePropertiesEXT", {vvl::Extension::_VK_EXT_pipeline_properties}},
        {"vkCmdSetPatchControlPointsEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state2, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetRasterizerDiscardEnableEXT",
         {vvl::Extension::_VK_EXT_extended_dynamic_state2, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetDepthBiasEnableEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state2, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetLogicOpEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state2, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetPrimitiveRestartEnableEXT",
         {vvl::Extension::_VK_EXT_extended_dynamic_state2, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetColorWriteEnableEXT", {vvl::Extension::_VK_EXT_color_write_enable}},
        {"vkCmdDrawMultiEXT", {vvl::Extension::_VK_EXT_multi_draw}},
        {"vkCmdDrawMultiIndexedEXT", {vvl::Extension::_VK_EXT_multi_draw}},
        {"vkCreateMicromapEXT", {vvl::Extension::_VK_EXT_opacity_micromap}},
        {"vkDestroyMicromapEXT", {vvl::Extension::_VK_EXT_opacity_micromap}},
        {"vkCmdBuildMicromapsEXT", {vvl::Extension::_VK_EXT_opacity_micromap}},
        {"vkBuildMicromapsEXT", {vvl::Extension::_VK_EXT_opacity_micromap}},
        {"vkCopyMicromapEXT", {vvl::Extension::_VK_EXT_opacity_micromap}},
        {"vkCopyMicromapToMemoryEXT", {vvl::Extension::_VK_EXT_opacity_micromap}},
        {"vkCopyMemoryToMicromapEXT", {vvl::Extension::_VK_EXT_opacity_micromap}},
        {"vkWriteMicromapsPropertiesEXT", {vvl::Extension::_VK_EXT_opacity_micromap}},
        {"vkCmdCopyMicromapEXT", {vvl::Extension::_VK_EXT_opacity_micromap}},
        {"vkCmdCopyMicromapToMemoryEXT", {vvl::Extension::_VK_EXT_opacity_micromap}},
        {"vkCmdCopyMemoryToMicromapEXT", {vvl::Extension::_VK_EXT_opacity_micromap}},
        {"vkCmdWriteMicromapsPropertiesEXT", {vvl::Extension::_VK_EXT_opacity_micromap}},
        {"vkGetDeviceMicromapCompatibilityEXT", {vvl::Extension::_VK_EXT_opacity_micromap}},
        {"vkGetMicromapBuildSizesEXT", {vvl::Extension::_VK_EXT_opacity_micromap}},
        {"vkCmdDrawClusterHUAWEI", {vvl::Extension::_VK_HUAWEI_cluster_culling_shader}},
        {"vkCmdDrawClusterIndirectHUAWEI", {vvl::Extension::_VK_HUAWEI_cluster_culling_shader}},
        {"vkSetDeviceMemoryPriorityEXT", {vvl::Extension::_VK_EXT_pageable_device_local_memory}},
        {"vkGetDescriptorSetLayoutHostMappingInfoVALVE", {vvl::Extension::_VK_VALVE_descriptor_set_host_mapping}},
        {"vkGetDescriptorSetHostMappingVALVE", {vvl::Extension::_VK_VALVE_descriptor_set_host_mapping}},
        {"vkCmdCopyMemoryIndirectNV", {vvl::Extension::_VK_NV_copy_memory_indirect}},
        {"vkCmdCopyMemoryToImageIndirectNV", {vvl::Extension::_VK_NV_copy_memory_indirect}},
        {"vkCmdDecompressMemoryNV", {vvl::Extension::_VK_NV_memory_decompression}},
        {"vkCmdDecompressMemoryIndirectCountNV", {vvl::Extension::_VK_NV_memory_decompression}},
        {"vkGetPipelineIndirectMemoryRequirementsNV", {vvl::Extension::_VK_NV_device_generated_commands_compute}},
        {"vkCmdUpdatePipelineIndirectBufferNV", {vvl::Extension::_VK_NV_device_generated_commands_compute}},
        {"vkGetPipelineIndirectDeviceAddressNV", {vvl::Extension::_VK_NV_device_generated_commands_compute}},
        {"vkCmdSetDepthClampEnableEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetPolygonModeEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetRasterizationSamplesEXT",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetSampleMaskEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetAlphaToCoverageEnableEXT",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetAlphaToOneEnableEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetLogicOpEnableEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetColorBlendEnableEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetColorBlendEquationEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetColorWriteMaskEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetTessellationDomainOriginEXT",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetRasterizationStreamEXT",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetConservativeRasterizationModeEXT",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetExtraPrimitiveOverestimationSizeEXT",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetDepthClipEnableEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetSampleLocationsEnableEXT",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetColorBlendAdvancedEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetProvokingVertexModeEXT",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetLineRasterizationModeEXT",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetLineStippleEnableEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetDepthClipNegativeOneToOneEXT",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetViewportWScalingEnableNV",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetViewportSwizzleNV", {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetCoverageToColorEnableNV",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetCoverageToColorLocationNV",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetCoverageModulationModeNV",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetCoverageModulationTableEnableNV",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetCoverageModulationTableNV",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetShadingRateImageEnableNV",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetRepresentativeFragmentTestEnableNV",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetCoverageReductionModeNV",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkGetShaderModuleIdentifierEXT", {vvl::Extension::_VK_EXT_shader_module_identifier}},
        {"vkGetShaderModuleCreateInfoIdentifierEXT", {vvl::Extension::_VK_EXT_shader_module_identifier}},
        {"vkCreateOpticalFlowSessionNV", {vvl::Extension::_VK_NV_optical_flow}},
        {"vkDestroyOpticalFlowSessionNV", {vvl::Extension::_VK_NV_optical_flow}},
        {"vkBindOpticalFlowSessionImageNV", {vvl::Extension::_VK_NV_optical_flow}},
        {"vkCmdOpticalFlowExecuteNV", {vvl::Extension::_VK_NV_optical_flow}},
        {"vkCreateShadersEXT", {vvl::Extension::_VK_EXT_shader_object}},
        {"vkDestroyShaderEXT", {vvl::Extension::_VK_EXT_shader_object}},
        {"vkGetShaderBinaryDataEXT", {vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdBindShadersEXT", {vvl::Extension::_VK_EXT_shader_object}},
        {"vkGetFramebufferTilePropertiesQCOM", {vvl::Extension::_VK_QCOM_tile_properties}},
        {"vkGetDynamicRenderingTilePropertiesQCOM", {vvl::Extension::_VK_QCOM_tile_properties}},
        {"vkSetLatencySleepModeNV", {vvl::Extension::_VK_NV_low_latency2}},
        {"vkLatencySleepNV", {vvl::Extension::_VK_NV_low_latency2}},
        {"vkSetLatencyMarkerNV", {vvl::Extension::_VK_NV_low_latency2}},
        {"vkGetLatencyTimingsNV", {vvl::Extension::_VK_NV_low_latency2}},
        {"vkQueueNotifyOutOfBandNV", {vvl::Extension::_VK_NV_low_latency2}},
        {"vkCmdSetAttachmentFeedbackLoopEnableEXT", {vvl::Extension::_VK_EXT_attachment_feedback_loop_dynamic_state}},
        {"vkGetScreenBufferPropertiesQNX", {vvl::Extension::_VK_QNX_external_memory_screen_buffer}},
        {"vkCreateAccelerationStructureKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkDestroyAccelerationStructureKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkCmdBuildAccelerationStructuresKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkCmdBuildAccelerationStructuresIndirectKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkBuildAccelerationStructuresKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkCopyAccelerationStructureKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkCopyAccelerationStructureToMemoryKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkCopyMemoryToAccelerationStructureKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkWriteAccelerationStructuresPropertiesKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkCmdCopyAccelerationStructureKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkCmdCopyAccelerationStructureToMemoryKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkCmdCopyMemoryToAccelerationStructureKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkGetAccelerationStructureDeviceAddressKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkCmdWriteAccelerationStructuresPropertiesKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkGetDeviceAccelerationStructureCompatibilityKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkGetAccelerationStructureBuildSizesKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkCmdTraceRaysKHR", {vvl::Extension::_VK_KHR_ray_tracing_pipeline}},
        {"vkCreateRayTracingPipelinesKHR", {vvl::Extension::_VK_KHR_ray_tracing_pipeline}},
        {"vkGetRayTracingCaptureReplayShaderGroupHandlesKHR", {vvl::Extension::_VK_KHR_ray_tracing_pipeline}},
        {"vkCmdTraceRaysIndirectKHR", {vvl::Extension::_VK_KHR_ray_tracing_pipeline}},
        {"vkGetRayTracingShaderGroupStackSizeKHR", {vvl::Extension::_VK_KHR_ray_tracing_pipeline}},
        {"vkCmdSetRayTracingPipelineStackSizeKHR", {vvl::Extension::_VK_KHR_ray_tracing_pipeline}},
        {"vkCmdDrawMeshTasksEXT", {vvl::Extension::_VK_EXT_mesh_shader}},
        {"vkCmdDrawMeshTasksIndirectEXT", {vvl::Extension::_VK_EXT_mesh_shader}},
        {"vkCmdDrawMeshTasksIndirectCountEXT", {vvl::Extension::_VK_EXT_mesh_shader}},
    };
    return api_extension_map;
}

// Using the above code-generated map of APINames-to-parent extension names, this function will:
//   o  Determine if the API has an associated extension
//   o  If it does, determine if that extension name is present in the passed-in set of device or instance enabled_ext_names
//   If the APIname has no parent extension, OR its parent extension name is IN one of the sets, return TRUE, else FALSE
bool ApiParentExtensionEnabled(const std::string api_name, const DeviceExtensions* device_extension_info) {
    auto promoted_api = GetApiPromotedMap().find(api_name);
    if (promoted_api != GetApiPromotedMap().end()) {
        auto info = GetDeviceVersionMap(promoted_api->second.c_str());
        assert(info.state);
        return (device_extension_info->*(info.state) == kEnabledByCreateinfo);
    }

    auto has_ext = GetApiExtensionMap().find(api_name);
    // Is this API part of an extension or feature group?
    if (has_ext != GetApiExtensionMap().end()) {
        // Was the extension for this API enabled in the CreateDevice call?
        for (const auto& extension : has_ext->second) {
            auto info = device_extension_info->GetInfo(extension);
            if (info.state) {
                if (device_extension_info->*(info.state) == kEnabledByCreateinfo ||
                    device_extension_info->*(info.state) == kEnabledByInteraction) {
                    return true;
                }
            }
        }

        // Was the extension for this API enabled in the CreateInstance call?
        auto instance_extension_info = static_cast<const InstanceExtensions*>(device_extension_info);
        for (const auto& extension : has_ext->second) {
            auto info = instance_extension_info->GetInfo(extension);
            if (info.state) {
                if (instance_extension_info->*(info.state) == kEnabledByCreateinfo ||
                    instance_extension_info->*(info.state) == kEnabledByInteraction) {
                    return true;
                }
            }
        }
        return false;
    }
    return true;
}

void layer_init_device_dispatch_table(VkDevice device, VkLayerDispatchTable* table, PFN_vkGetDeviceProcAddr gpa) {
    memset(table, 0, sizeof(*table));
    // Device function pointers
    table->GetDeviceProcAddr = gpa;
    table->DestroyDevice = (PFN_vkDestroyDevice)gpa(device, "vkDestroyDevice");
    table->GetDeviceQueue = (PFN_vkGetDeviceQueue)gpa(device, "vkGetDeviceQueue");
    table->QueueSubmit = (PFN_vkQueueSubmit)gpa(device, "vkQueueSubmit");
    table->QueueWaitIdle = (PFN_vkQueueWaitIdle)gpa(device, "vkQueueWaitIdle");
    table->DeviceWaitIdle = (PFN_vkDeviceWaitIdle)gpa(device, "vkDeviceWaitIdle");
    table->AllocateMemory = (PFN_vkAllocateMemory)gpa(device, "vkAllocateMemory");
    table->FreeMemory = (PFN_vkFreeMemory)gpa(device, "vkFreeMemory");
    table->MapMemory = (PFN_vkMapMemory)gpa(device, "vkMapMemory");
    table->UnmapMemory = (PFN_vkUnmapMemory)gpa(device, "vkUnmapMemory");
    table->FlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges)gpa(device, "vkFlushMappedMemoryRanges");
    table->InvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges)gpa(device, "vkInvalidateMappedMemoryRanges");
    table->GetDeviceMemoryCommitment = (PFN_vkGetDeviceMemoryCommitment)gpa(device, "vkGetDeviceMemoryCommitment");
    table->BindBufferMemory = (PFN_vkBindBufferMemory)gpa(device, "vkBindBufferMemory");
    table->BindImageMemory = (PFN_vkBindImageMemory)gpa(device, "vkBindImageMemory");
    table->GetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements)gpa(device, "vkGetBufferMemoryRequirements");
    table->GetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements)gpa(device, "vkGetImageMemoryRequirements");
    table->GetImageSparseMemoryRequirements =
        (PFN_vkGetImageSparseMemoryRequirements)gpa(device, "vkGetImageSparseMemoryRequirements");
    table->QueueBindSparse = (PFN_vkQueueBindSparse)gpa(device, "vkQueueBindSparse");
    table->CreateFence = (PFN_vkCreateFence)gpa(device, "vkCreateFence");
    table->DestroyFence = (PFN_vkDestroyFence)gpa(device, "vkDestroyFence");
    table->ResetFences = (PFN_vkResetFences)gpa(device, "vkResetFences");
    table->GetFenceStatus = (PFN_vkGetFenceStatus)gpa(device, "vkGetFenceStatus");
    table->WaitForFences = (PFN_vkWaitForFences)gpa(device, "vkWaitForFences");
    table->CreateSemaphore = (PFN_vkCreateSemaphore)gpa(device, "vkCreateSemaphore");
    table->DestroySemaphore = (PFN_vkDestroySemaphore)gpa(device, "vkDestroySemaphore");
    table->CreateEvent = (PFN_vkCreateEvent)gpa(device, "vkCreateEvent");
    table->DestroyEvent = (PFN_vkDestroyEvent)gpa(device, "vkDestroyEvent");
    table->GetEventStatus = (PFN_vkGetEventStatus)gpa(device, "vkGetEventStatus");
    table->SetEvent = (PFN_vkSetEvent)gpa(device, "vkSetEvent");
    table->ResetEvent = (PFN_vkResetEvent)gpa(device, "vkResetEvent");
    table->CreateQueryPool = (PFN_vkCreateQueryPool)gpa(device, "vkCreateQueryPool");
    table->DestroyQueryPool = (PFN_vkDestroyQueryPool)gpa(device, "vkDestroyQueryPool");
    table->GetQueryPoolResults = (PFN_vkGetQueryPoolResults)gpa(device, "vkGetQueryPoolResults");
    table->CreateBuffer = (PFN_vkCreateBuffer)gpa(device, "vkCreateBuffer");
    table->DestroyBuffer = (PFN_vkDestroyBuffer)gpa(device, "vkDestroyBuffer");
    table->CreateBufferView = (PFN_vkCreateBufferView)gpa(device, "vkCreateBufferView");
    table->DestroyBufferView = (PFN_vkDestroyBufferView)gpa(device, "vkDestroyBufferView");
    table->CreateImage = (PFN_vkCreateImage)gpa(device, "vkCreateImage");
    table->DestroyImage = (PFN_vkDestroyImage)gpa(device, "vkDestroyImage");
    table->GetImageSubresourceLayout = (PFN_vkGetImageSubresourceLayout)gpa(device, "vkGetImageSubresourceLayout");
    table->CreateImageView = (PFN_vkCreateImageView)gpa(device, "vkCreateImageView");
    table->DestroyImageView = (PFN_vkDestroyImageView)gpa(device, "vkDestroyImageView");
    table->CreateShaderModule = (PFN_vkCreateShaderModule)gpa(device, "vkCreateShaderModule");
    table->DestroyShaderModule = (PFN_vkDestroyShaderModule)gpa(device, "vkDestroyShaderModule");
    table->CreatePipelineCache = (PFN_vkCreatePipelineCache)gpa(device, "vkCreatePipelineCache");
    table->DestroyPipelineCache = (PFN_vkDestroyPipelineCache)gpa(device, "vkDestroyPipelineCache");
    table->GetPipelineCacheData = (PFN_vkGetPipelineCacheData)gpa(device, "vkGetPipelineCacheData");
    table->MergePipelineCaches = (PFN_vkMergePipelineCaches)gpa(device, "vkMergePipelineCaches");
    table->CreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines)gpa(device, "vkCreateGraphicsPipelines");
    table->CreateComputePipelines = (PFN_vkCreateComputePipelines)gpa(device, "vkCreateComputePipelines");
    table->DestroyPipeline = (PFN_vkDestroyPipeline)gpa(device, "vkDestroyPipeline");
    table->CreatePipelineLayout = (PFN_vkCreatePipelineLayout)gpa(device, "vkCreatePipelineLayout");
    table->DestroyPipelineLayout = (PFN_vkDestroyPipelineLayout)gpa(device, "vkDestroyPipelineLayout");
    table->CreateSampler = (PFN_vkCreateSampler)gpa(device, "vkCreateSampler");
    table->DestroySampler = (PFN_vkDestroySampler)gpa(device, "vkDestroySampler");
    table->CreateDescriptorSetLayout = (PFN_vkCreateDescriptorSetLayout)gpa(device, "vkCreateDescriptorSetLayout");
    table->DestroyDescriptorSetLayout = (PFN_vkDestroyDescriptorSetLayout)gpa(device, "vkDestroyDescriptorSetLayout");
    table->CreateDescriptorPool = (PFN_vkCreateDescriptorPool)gpa(device, "vkCreateDescriptorPool");
    table->DestroyDescriptorPool = (PFN_vkDestroyDescriptorPool)gpa(device, "vkDestroyDescriptorPool");
    table->ResetDescriptorPool = (PFN_vkResetDescriptorPool)gpa(device, "vkResetDescriptorPool");
    table->AllocateDescriptorSets = (PFN_vkAllocateDescriptorSets)gpa(device, "vkAllocateDescriptorSets");
    table->FreeDescriptorSets = (PFN_vkFreeDescriptorSets)gpa(device, "vkFreeDescriptorSets");
    table->UpdateDescriptorSets = (PFN_vkUpdateDescriptorSets)gpa(device, "vkUpdateDescriptorSets");
    table->CreateFramebuffer = (PFN_vkCreateFramebuffer)gpa(device, "vkCreateFramebuffer");
    table->DestroyFramebuffer = (PFN_vkDestroyFramebuffer)gpa(device, "vkDestroyFramebuffer");
    table->CreateRenderPass = (PFN_vkCreateRenderPass)gpa(device, "vkCreateRenderPass");
    table->DestroyRenderPass = (PFN_vkDestroyRenderPass)gpa(device, "vkDestroyRenderPass");
    table->GetRenderAreaGranularity = (PFN_vkGetRenderAreaGranularity)gpa(device, "vkGetRenderAreaGranularity");
    table->CreateCommandPool = (PFN_vkCreateCommandPool)gpa(device, "vkCreateCommandPool");
    table->DestroyCommandPool = (PFN_vkDestroyCommandPool)gpa(device, "vkDestroyCommandPool");
    table->ResetCommandPool = (PFN_vkResetCommandPool)gpa(device, "vkResetCommandPool");
    table->AllocateCommandBuffers = (PFN_vkAllocateCommandBuffers)gpa(device, "vkAllocateCommandBuffers");
    table->FreeCommandBuffers = (PFN_vkFreeCommandBuffers)gpa(device, "vkFreeCommandBuffers");
    table->BeginCommandBuffer = (PFN_vkBeginCommandBuffer)gpa(device, "vkBeginCommandBuffer");
    table->EndCommandBuffer = (PFN_vkEndCommandBuffer)gpa(device, "vkEndCommandBuffer");
    table->ResetCommandBuffer = (PFN_vkResetCommandBuffer)gpa(device, "vkResetCommandBuffer");
    table->CmdBindPipeline = (PFN_vkCmdBindPipeline)gpa(device, "vkCmdBindPipeline");
    table->CmdSetViewport = (PFN_vkCmdSetViewport)gpa(device, "vkCmdSetViewport");
    table->CmdSetScissor = (PFN_vkCmdSetScissor)gpa(device, "vkCmdSetScissor");
    table->CmdSetLineWidth = (PFN_vkCmdSetLineWidth)gpa(device, "vkCmdSetLineWidth");
    table->CmdSetDepthBias = (PFN_vkCmdSetDepthBias)gpa(device, "vkCmdSetDepthBias");
    table->CmdSetBlendConstants = (PFN_vkCmdSetBlendConstants)gpa(device, "vkCmdSetBlendConstants");
    table->CmdSetDepthBounds = (PFN_vkCmdSetDepthBounds)gpa(device, "vkCmdSetDepthBounds");
    table->CmdSetStencilCompareMask = (PFN_vkCmdSetStencilCompareMask)gpa(device, "vkCmdSetStencilCompareMask");
    table->CmdSetStencilWriteMask = (PFN_vkCmdSetStencilWriteMask)gpa(device, "vkCmdSetStencilWriteMask");
    table->CmdSetStencilReference = (PFN_vkCmdSetStencilReference)gpa(device, "vkCmdSetStencilReference");
    table->CmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets)gpa(device, "vkCmdBindDescriptorSets");
    table->CmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer)gpa(device, "vkCmdBindIndexBuffer");
    table->CmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers)gpa(device, "vkCmdBindVertexBuffers");
    table->CmdDraw = (PFN_vkCmdDraw)gpa(device, "vkCmdDraw");
    table->CmdDrawIndexed = (PFN_vkCmdDrawIndexed)gpa(device, "vkCmdDrawIndexed");
    table->CmdDrawIndirect = (PFN_vkCmdDrawIndirect)gpa(device, "vkCmdDrawIndirect");
    table->CmdDrawIndexedIndirect = (PFN_vkCmdDrawIndexedIndirect)gpa(device, "vkCmdDrawIndexedIndirect");
    table->CmdDispatch = (PFN_vkCmdDispatch)gpa(device, "vkCmdDispatch");
    table->CmdDispatchIndirect = (PFN_vkCmdDispatchIndirect)gpa(device, "vkCmdDispatchIndirect");
    table->CmdCopyBuffer = (PFN_vkCmdCopyBuffer)gpa(device, "vkCmdCopyBuffer");
    table->CmdCopyImage = (PFN_vkCmdCopyImage)gpa(device, "vkCmdCopyImage");
    table->CmdBlitImage = (PFN_vkCmdBlitImage)gpa(device, "vkCmdBlitImage");
    table->CmdCopyBufferToImage = (PFN_vkCmdCopyBufferToImage)gpa(device, "vkCmdCopyBufferToImage");
    table->CmdCopyImageToBuffer = (PFN_vkCmdCopyImageToBuffer)gpa(device, "vkCmdCopyImageToBuffer");
    table->CmdUpdateBuffer = (PFN_vkCmdUpdateBuffer)gpa(device, "vkCmdUpdateBuffer");
    table->CmdFillBuffer = (PFN_vkCmdFillBuffer)gpa(device, "vkCmdFillBuffer");
    table->CmdClearColorImage = (PFN_vkCmdClearColorImage)gpa(device, "vkCmdClearColorImage");
    table->CmdClearDepthStencilImage = (PFN_vkCmdClearDepthStencilImage)gpa(device, "vkCmdClearDepthStencilImage");
    table->CmdClearAttachments = (PFN_vkCmdClearAttachments)gpa(device, "vkCmdClearAttachments");
    table->CmdResolveImage = (PFN_vkCmdResolveImage)gpa(device, "vkCmdResolveImage");
    table->CmdSetEvent = (PFN_vkCmdSetEvent)gpa(device, "vkCmdSetEvent");
    table->CmdResetEvent = (PFN_vkCmdResetEvent)gpa(device, "vkCmdResetEvent");
    table->CmdWaitEvents = (PFN_vkCmdWaitEvents)gpa(device, "vkCmdWaitEvents");
    table->CmdPipelineBarrier = (PFN_vkCmdPipelineBarrier)gpa(device, "vkCmdPipelineBarrier");
    table->CmdBeginQuery = (PFN_vkCmdBeginQuery)gpa(device, "vkCmdBeginQuery");
    table->CmdEndQuery = (PFN_vkCmdEndQuery)gpa(device, "vkCmdEndQuery");
    table->CmdResetQueryPool = (PFN_vkCmdResetQueryPool)gpa(device, "vkCmdResetQueryPool");
    table->CmdWriteTimestamp = (PFN_vkCmdWriteTimestamp)gpa(device, "vkCmdWriteTimestamp");
    table->CmdCopyQueryPoolResults = (PFN_vkCmdCopyQueryPoolResults)gpa(device, "vkCmdCopyQueryPoolResults");
    table->CmdPushConstants = (PFN_vkCmdPushConstants)gpa(device, "vkCmdPushConstants");
    table->CmdBeginRenderPass = (PFN_vkCmdBeginRenderPass)gpa(device, "vkCmdBeginRenderPass");
    table->CmdNextSubpass = (PFN_vkCmdNextSubpass)gpa(device, "vkCmdNextSubpass");
    table->CmdEndRenderPass = (PFN_vkCmdEndRenderPass)gpa(device, "vkCmdEndRenderPass");
    table->CmdExecuteCommands = (PFN_vkCmdExecuteCommands)gpa(device, "vkCmdExecuteCommands");
    table->BindBufferMemory2 = (PFN_vkBindBufferMemory2)gpa(device, "vkBindBufferMemory2");
    table->BindImageMemory2 = (PFN_vkBindImageMemory2)gpa(device, "vkBindImageMemory2");
    table->GetDeviceGroupPeerMemoryFeatures =
        (PFN_vkGetDeviceGroupPeerMemoryFeatures)gpa(device, "vkGetDeviceGroupPeerMemoryFeatures");
    table->CmdSetDeviceMask = (PFN_vkCmdSetDeviceMask)gpa(device, "vkCmdSetDeviceMask");
    table->CmdDispatchBase = (PFN_vkCmdDispatchBase)gpa(device, "vkCmdDispatchBase");
    table->GetImageMemoryRequirements2 = (PFN_vkGetImageMemoryRequirements2)gpa(device, "vkGetImageMemoryRequirements2");
    table->GetBufferMemoryRequirements2 = (PFN_vkGetBufferMemoryRequirements2)gpa(device, "vkGetBufferMemoryRequirements2");
    table->GetImageSparseMemoryRequirements2 =
        (PFN_vkGetImageSparseMemoryRequirements2)gpa(device, "vkGetImageSparseMemoryRequirements2");
    table->TrimCommandPool = (PFN_vkTrimCommandPool)gpa(device, "vkTrimCommandPool");
    table->GetDeviceQueue2 = (PFN_vkGetDeviceQueue2)gpa(device, "vkGetDeviceQueue2");
    table->CreateSamplerYcbcrConversion = (PFN_vkCreateSamplerYcbcrConversion)gpa(device, "vkCreateSamplerYcbcrConversion");
    table->DestroySamplerYcbcrConversion = (PFN_vkDestroySamplerYcbcrConversion)gpa(device, "vkDestroySamplerYcbcrConversion");
    table->CreateDescriptorUpdateTemplate = (PFN_vkCreateDescriptorUpdateTemplate)gpa(device, "vkCreateDescriptorUpdateTemplate");
    table->DestroyDescriptorUpdateTemplate =
        (PFN_vkDestroyDescriptorUpdateTemplate)gpa(device, "vkDestroyDescriptorUpdateTemplate");
    table->UpdateDescriptorSetWithTemplate =
        (PFN_vkUpdateDescriptorSetWithTemplate)gpa(device, "vkUpdateDescriptorSetWithTemplate");
    table->GetDescriptorSetLayoutSupport = (PFN_vkGetDescriptorSetLayoutSupport)gpa(device, "vkGetDescriptorSetLayoutSupport");
    table->CmdDrawIndirectCount = (PFN_vkCmdDrawIndirectCount)gpa(device, "vkCmdDrawIndirectCount");
    table->CmdDrawIndexedIndirectCount = (PFN_vkCmdDrawIndexedIndirectCount)gpa(device, "vkCmdDrawIndexedIndirectCount");
    table->CreateRenderPass2 = (PFN_vkCreateRenderPass2)gpa(device, "vkCreateRenderPass2");
    table->CmdBeginRenderPass2 = (PFN_vkCmdBeginRenderPass2)gpa(device, "vkCmdBeginRenderPass2");
    table->CmdNextSubpass2 = (PFN_vkCmdNextSubpass2)gpa(device, "vkCmdNextSubpass2");
    table->CmdEndRenderPass2 = (PFN_vkCmdEndRenderPass2)gpa(device, "vkCmdEndRenderPass2");
    table->ResetQueryPool = (PFN_vkResetQueryPool)gpa(device, "vkResetQueryPool");
    table->GetSemaphoreCounterValue = (PFN_vkGetSemaphoreCounterValue)gpa(device, "vkGetSemaphoreCounterValue");
    table->WaitSemaphores = (PFN_vkWaitSemaphores)gpa(device, "vkWaitSemaphores");
    table->SignalSemaphore = (PFN_vkSignalSemaphore)gpa(device, "vkSignalSemaphore");
    table->GetBufferDeviceAddress = (PFN_vkGetBufferDeviceAddress)gpa(device, "vkGetBufferDeviceAddress");
    table->GetBufferOpaqueCaptureAddress = (PFN_vkGetBufferOpaqueCaptureAddress)gpa(device, "vkGetBufferOpaqueCaptureAddress");
    table->GetDeviceMemoryOpaqueCaptureAddress =
        (PFN_vkGetDeviceMemoryOpaqueCaptureAddress)gpa(device, "vkGetDeviceMemoryOpaqueCaptureAddress");
    table->CreatePrivateDataSlot = (PFN_vkCreatePrivateDataSlot)gpa(device, "vkCreatePrivateDataSlot");
    table->DestroyPrivateDataSlot = (PFN_vkDestroyPrivateDataSlot)gpa(device, "vkDestroyPrivateDataSlot");
    table->SetPrivateData = (PFN_vkSetPrivateData)gpa(device, "vkSetPrivateData");
    table->GetPrivateData = (PFN_vkGetPrivateData)gpa(device, "vkGetPrivateData");
    table->CmdSetEvent2 = (PFN_vkCmdSetEvent2)gpa(device, "vkCmdSetEvent2");
    table->CmdResetEvent2 = (PFN_vkCmdResetEvent2)gpa(device, "vkCmdResetEvent2");
    table->CmdWaitEvents2 = (PFN_vkCmdWaitEvents2)gpa(device, "vkCmdWaitEvents2");
    table->CmdPipelineBarrier2 = (PFN_vkCmdPipelineBarrier2)gpa(device, "vkCmdPipelineBarrier2");
    table->CmdWriteTimestamp2 = (PFN_vkCmdWriteTimestamp2)gpa(device, "vkCmdWriteTimestamp2");
    table->QueueSubmit2 = (PFN_vkQueueSubmit2)gpa(device, "vkQueueSubmit2");
    table->CmdCopyBuffer2 = (PFN_vkCmdCopyBuffer2)gpa(device, "vkCmdCopyBuffer2");
    table->CmdCopyImage2 = (PFN_vkCmdCopyImage2)gpa(device, "vkCmdCopyImage2");
    table->CmdCopyBufferToImage2 = (PFN_vkCmdCopyBufferToImage2)gpa(device, "vkCmdCopyBufferToImage2");
    table->CmdCopyImageToBuffer2 = (PFN_vkCmdCopyImageToBuffer2)gpa(device, "vkCmdCopyImageToBuffer2");
    table->CmdBlitImage2 = (PFN_vkCmdBlitImage2)gpa(device, "vkCmdBlitImage2");
    table->CmdResolveImage2 = (PFN_vkCmdResolveImage2)gpa(device, "vkCmdResolveImage2");
    table->CmdBeginRendering = (PFN_vkCmdBeginRendering)gpa(device, "vkCmdBeginRendering");
    table->CmdEndRendering = (PFN_vkCmdEndRendering)gpa(device, "vkCmdEndRendering");
    table->CmdSetCullMode = (PFN_vkCmdSetCullMode)gpa(device, "vkCmdSetCullMode");
    table->CmdSetFrontFace = (PFN_vkCmdSetFrontFace)gpa(device, "vkCmdSetFrontFace");
    table->CmdSetPrimitiveTopology = (PFN_vkCmdSetPrimitiveTopology)gpa(device, "vkCmdSetPrimitiveTopology");
    table->CmdSetViewportWithCount = (PFN_vkCmdSetViewportWithCount)gpa(device, "vkCmdSetViewportWithCount");
    table->CmdSetScissorWithCount = (PFN_vkCmdSetScissorWithCount)gpa(device, "vkCmdSetScissorWithCount");
    table->CmdBindVertexBuffers2 = (PFN_vkCmdBindVertexBuffers2)gpa(device, "vkCmdBindVertexBuffers2");
    table->CmdSetDepthTestEnable = (PFN_vkCmdSetDepthTestEnable)gpa(device, "vkCmdSetDepthTestEnable");
    table->CmdSetDepthWriteEnable = (PFN_vkCmdSetDepthWriteEnable)gpa(device, "vkCmdSetDepthWriteEnable");
    table->CmdSetDepthCompareOp = (PFN_vkCmdSetDepthCompareOp)gpa(device, "vkCmdSetDepthCompareOp");
    table->CmdSetDepthBoundsTestEnable = (PFN_vkCmdSetDepthBoundsTestEnable)gpa(device, "vkCmdSetDepthBoundsTestEnable");
    table->CmdSetStencilTestEnable = (PFN_vkCmdSetStencilTestEnable)gpa(device, "vkCmdSetStencilTestEnable");
    table->CmdSetStencilOp = (PFN_vkCmdSetStencilOp)gpa(device, "vkCmdSetStencilOp");
    table->CmdSetRasterizerDiscardEnable = (PFN_vkCmdSetRasterizerDiscardEnable)gpa(device, "vkCmdSetRasterizerDiscardEnable");
    table->CmdSetDepthBiasEnable = (PFN_vkCmdSetDepthBiasEnable)gpa(device, "vkCmdSetDepthBiasEnable");
    table->CmdSetPrimitiveRestartEnable = (PFN_vkCmdSetPrimitiveRestartEnable)gpa(device, "vkCmdSetPrimitiveRestartEnable");
    table->GetDeviceBufferMemoryRequirements =
        (PFN_vkGetDeviceBufferMemoryRequirements)gpa(device, "vkGetDeviceBufferMemoryRequirements");
    table->GetDeviceImageMemoryRequirements =
        (PFN_vkGetDeviceImageMemoryRequirements)gpa(device, "vkGetDeviceImageMemoryRequirements");
    table->GetDeviceImageSparseMemoryRequirements =
        (PFN_vkGetDeviceImageSparseMemoryRequirements)gpa(device, "vkGetDeviceImageSparseMemoryRequirements");
    table->CreateSwapchainKHR = (PFN_vkCreateSwapchainKHR)gpa(device, "vkCreateSwapchainKHR");
    table->DestroySwapchainKHR = (PFN_vkDestroySwapchainKHR)gpa(device, "vkDestroySwapchainKHR");
    table->GetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR)gpa(device, "vkGetSwapchainImagesKHR");
    table->AcquireNextImageKHR = (PFN_vkAcquireNextImageKHR)gpa(device, "vkAcquireNextImageKHR");
    table->QueuePresentKHR = (PFN_vkQueuePresentKHR)gpa(device, "vkQueuePresentKHR");
    table->GetDeviceGroupPresentCapabilitiesKHR =
        (PFN_vkGetDeviceGroupPresentCapabilitiesKHR)gpa(device, "vkGetDeviceGroupPresentCapabilitiesKHR");
    table->GetDeviceGroupSurfacePresentModesKHR =
        (PFN_vkGetDeviceGroupSurfacePresentModesKHR)gpa(device, "vkGetDeviceGroupSurfacePresentModesKHR");
    table->AcquireNextImage2KHR = (PFN_vkAcquireNextImage2KHR)gpa(device, "vkAcquireNextImage2KHR");
    table->CreateSharedSwapchainsKHR = (PFN_vkCreateSharedSwapchainsKHR)gpa(device, "vkCreateSharedSwapchainsKHR");
    table->CreateVideoSessionKHR = (PFN_vkCreateVideoSessionKHR)gpa(device, "vkCreateVideoSessionKHR");
    table->DestroyVideoSessionKHR = (PFN_vkDestroyVideoSessionKHR)gpa(device, "vkDestroyVideoSessionKHR");
    table->GetVideoSessionMemoryRequirementsKHR =
        (PFN_vkGetVideoSessionMemoryRequirementsKHR)gpa(device, "vkGetVideoSessionMemoryRequirementsKHR");
    table->BindVideoSessionMemoryKHR = (PFN_vkBindVideoSessionMemoryKHR)gpa(device, "vkBindVideoSessionMemoryKHR");
    table->CreateVideoSessionParametersKHR =
        (PFN_vkCreateVideoSessionParametersKHR)gpa(device, "vkCreateVideoSessionParametersKHR");
    table->UpdateVideoSessionParametersKHR =
        (PFN_vkUpdateVideoSessionParametersKHR)gpa(device, "vkUpdateVideoSessionParametersKHR");
    table->DestroyVideoSessionParametersKHR =
        (PFN_vkDestroyVideoSessionParametersKHR)gpa(device, "vkDestroyVideoSessionParametersKHR");
    table->CmdBeginVideoCodingKHR = (PFN_vkCmdBeginVideoCodingKHR)gpa(device, "vkCmdBeginVideoCodingKHR");
    table->CmdEndVideoCodingKHR = (PFN_vkCmdEndVideoCodingKHR)gpa(device, "vkCmdEndVideoCodingKHR");
    table->CmdControlVideoCodingKHR = (PFN_vkCmdControlVideoCodingKHR)gpa(device, "vkCmdControlVideoCodingKHR");
    table->CmdDecodeVideoKHR = (PFN_vkCmdDecodeVideoKHR)gpa(device, "vkCmdDecodeVideoKHR");
    table->CmdBeginRenderingKHR = (PFN_vkCmdBeginRenderingKHR)gpa(device, "vkCmdBeginRenderingKHR");
    table->CmdEndRenderingKHR = (PFN_vkCmdEndRenderingKHR)gpa(device, "vkCmdEndRenderingKHR");
    table->GetDeviceGroupPeerMemoryFeaturesKHR =
        (PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR)gpa(device, "vkGetDeviceGroupPeerMemoryFeaturesKHR");
    table->CmdSetDeviceMaskKHR = (PFN_vkCmdSetDeviceMaskKHR)gpa(device, "vkCmdSetDeviceMaskKHR");
    table->CmdDispatchBaseKHR = (PFN_vkCmdDispatchBaseKHR)gpa(device, "vkCmdDispatchBaseKHR");
    table->TrimCommandPoolKHR = (PFN_vkTrimCommandPoolKHR)gpa(device, "vkTrimCommandPoolKHR");
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->GetMemoryWin32HandleKHR = (PFN_vkGetMemoryWin32HandleKHR)gpa(device, "vkGetMemoryWin32HandleKHR");
    table->GetMemoryWin32HandlePropertiesKHR =
        (PFN_vkGetMemoryWin32HandlePropertiesKHR)gpa(device, "vkGetMemoryWin32HandlePropertiesKHR");
#endif  // VK_USE_PLATFORM_WIN32_KHR
    table->GetMemoryFdKHR = (PFN_vkGetMemoryFdKHR)gpa(device, "vkGetMemoryFdKHR");
    table->GetMemoryFdPropertiesKHR = (PFN_vkGetMemoryFdPropertiesKHR)gpa(device, "vkGetMemoryFdPropertiesKHR");
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->ImportSemaphoreWin32HandleKHR = (PFN_vkImportSemaphoreWin32HandleKHR)gpa(device, "vkImportSemaphoreWin32HandleKHR");
    table->GetSemaphoreWin32HandleKHR = (PFN_vkGetSemaphoreWin32HandleKHR)gpa(device, "vkGetSemaphoreWin32HandleKHR");
#endif  // VK_USE_PLATFORM_WIN32_KHR
    table->ImportSemaphoreFdKHR = (PFN_vkImportSemaphoreFdKHR)gpa(device, "vkImportSemaphoreFdKHR");
    table->GetSemaphoreFdKHR = (PFN_vkGetSemaphoreFdKHR)gpa(device, "vkGetSemaphoreFdKHR");
    table->CmdPushDescriptorSetKHR = (PFN_vkCmdPushDescriptorSetKHR)gpa(device, "vkCmdPushDescriptorSetKHR");
    table->CmdPushDescriptorSetWithTemplateKHR =
        (PFN_vkCmdPushDescriptorSetWithTemplateKHR)gpa(device, "vkCmdPushDescriptorSetWithTemplateKHR");
    table->CreateDescriptorUpdateTemplateKHR =
        (PFN_vkCreateDescriptorUpdateTemplateKHR)gpa(device, "vkCreateDescriptorUpdateTemplateKHR");
    table->DestroyDescriptorUpdateTemplateKHR =
        (PFN_vkDestroyDescriptorUpdateTemplateKHR)gpa(device, "vkDestroyDescriptorUpdateTemplateKHR");
    table->UpdateDescriptorSetWithTemplateKHR =
        (PFN_vkUpdateDescriptorSetWithTemplateKHR)gpa(device, "vkUpdateDescriptorSetWithTemplateKHR");
    table->CreateRenderPass2KHR = (PFN_vkCreateRenderPass2KHR)gpa(device, "vkCreateRenderPass2KHR");
    table->CmdBeginRenderPass2KHR = (PFN_vkCmdBeginRenderPass2KHR)gpa(device, "vkCmdBeginRenderPass2KHR");
    table->CmdNextSubpass2KHR = (PFN_vkCmdNextSubpass2KHR)gpa(device, "vkCmdNextSubpass2KHR");
    table->CmdEndRenderPass2KHR = (PFN_vkCmdEndRenderPass2KHR)gpa(device, "vkCmdEndRenderPass2KHR");
    table->GetSwapchainStatusKHR = (PFN_vkGetSwapchainStatusKHR)gpa(device, "vkGetSwapchainStatusKHR");
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->ImportFenceWin32HandleKHR = (PFN_vkImportFenceWin32HandleKHR)gpa(device, "vkImportFenceWin32HandleKHR");
    table->GetFenceWin32HandleKHR = (PFN_vkGetFenceWin32HandleKHR)gpa(device, "vkGetFenceWin32HandleKHR");
#endif  // VK_USE_PLATFORM_WIN32_KHR
    table->ImportFenceFdKHR = (PFN_vkImportFenceFdKHR)gpa(device, "vkImportFenceFdKHR");
    table->GetFenceFdKHR = (PFN_vkGetFenceFdKHR)gpa(device, "vkGetFenceFdKHR");
    table->AcquireProfilingLockKHR = (PFN_vkAcquireProfilingLockKHR)gpa(device, "vkAcquireProfilingLockKHR");
    table->ReleaseProfilingLockKHR = (PFN_vkReleaseProfilingLockKHR)gpa(device, "vkReleaseProfilingLockKHR");
    table->GetImageMemoryRequirements2KHR = (PFN_vkGetImageMemoryRequirements2KHR)gpa(device, "vkGetImageMemoryRequirements2KHR");
    table->GetBufferMemoryRequirements2KHR =
        (PFN_vkGetBufferMemoryRequirements2KHR)gpa(device, "vkGetBufferMemoryRequirements2KHR");
    table->GetImageSparseMemoryRequirements2KHR =
        (PFN_vkGetImageSparseMemoryRequirements2KHR)gpa(device, "vkGetImageSparseMemoryRequirements2KHR");
    table->CreateSamplerYcbcrConversionKHR =
        (PFN_vkCreateSamplerYcbcrConversionKHR)gpa(device, "vkCreateSamplerYcbcrConversionKHR");
    table->DestroySamplerYcbcrConversionKHR =
        (PFN_vkDestroySamplerYcbcrConversionKHR)gpa(device, "vkDestroySamplerYcbcrConversionKHR");
    table->BindBufferMemory2KHR = (PFN_vkBindBufferMemory2KHR)gpa(device, "vkBindBufferMemory2KHR");
    table->BindImageMemory2KHR = (PFN_vkBindImageMemory2KHR)gpa(device, "vkBindImageMemory2KHR");
    table->GetDescriptorSetLayoutSupportKHR =
        (PFN_vkGetDescriptorSetLayoutSupportKHR)gpa(device, "vkGetDescriptorSetLayoutSupportKHR");
    table->CmdDrawIndirectCountKHR = (PFN_vkCmdDrawIndirectCountKHR)gpa(device, "vkCmdDrawIndirectCountKHR");
    table->CmdDrawIndexedIndirectCountKHR = (PFN_vkCmdDrawIndexedIndirectCountKHR)gpa(device, "vkCmdDrawIndexedIndirectCountKHR");
    table->GetSemaphoreCounterValueKHR = (PFN_vkGetSemaphoreCounterValueKHR)gpa(device, "vkGetSemaphoreCounterValueKHR");
    table->WaitSemaphoresKHR = (PFN_vkWaitSemaphoresKHR)gpa(device, "vkWaitSemaphoresKHR");
    table->SignalSemaphoreKHR = (PFN_vkSignalSemaphoreKHR)gpa(device, "vkSignalSemaphoreKHR");
    table->CmdSetFragmentShadingRateKHR = (PFN_vkCmdSetFragmentShadingRateKHR)gpa(device, "vkCmdSetFragmentShadingRateKHR");
    table->CmdSetRenderingAttachmentLocationsKHR =
        (PFN_vkCmdSetRenderingAttachmentLocationsKHR)gpa(device, "vkCmdSetRenderingAttachmentLocationsKHR");
    table->CmdSetRenderingInputAttachmentIndicesKHR =
        (PFN_vkCmdSetRenderingInputAttachmentIndicesKHR)gpa(device, "vkCmdSetRenderingInputAttachmentIndicesKHR");
    table->WaitForPresentKHR = (PFN_vkWaitForPresentKHR)gpa(device, "vkWaitForPresentKHR");
    table->GetBufferDeviceAddressKHR = (PFN_vkGetBufferDeviceAddressKHR)gpa(device, "vkGetBufferDeviceAddressKHR");
    table->GetBufferOpaqueCaptureAddressKHR =
        (PFN_vkGetBufferOpaqueCaptureAddressKHR)gpa(device, "vkGetBufferOpaqueCaptureAddressKHR");
    table->GetDeviceMemoryOpaqueCaptureAddressKHR =
        (PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR)gpa(device, "vkGetDeviceMemoryOpaqueCaptureAddressKHR");
    table->CreateDeferredOperationKHR = (PFN_vkCreateDeferredOperationKHR)gpa(device, "vkCreateDeferredOperationKHR");
    table->DestroyDeferredOperationKHR = (PFN_vkDestroyDeferredOperationKHR)gpa(device, "vkDestroyDeferredOperationKHR");
    table->GetDeferredOperationMaxConcurrencyKHR =
        (PFN_vkGetDeferredOperationMaxConcurrencyKHR)gpa(device, "vkGetDeferredOperationMaxConcurrencyKHR");
    table->GetDeferredOperationResultKHR = (PFN_vkGetDeferredOperationResultKHR)gpa(device, "vkGetDeferredOperationResultKHR");
    table->DeferredOperationJoinKHR = (PFN_vkDeferredOperationJoinKHR)gpa(device, "vkDeferredOperationJoinKHR");
    table->GetPipelineExecutablePropertiesKHR =
        (PFN_vkGetPipelineExecutablePropertiesKHR)gpa(device, "vkGetPipelineExecutablePropertiesKHR");
    table->GetPipelineExecutableStatisticsKHR =
        (PFN_vkGetPipelineExecutableStatisticsKHR)gpa(device, "vkGetPipelineExecutableStatisticsKHR");
    table->GetPipelineExecutableInternalRepresentationsKHR =
        (PFN_vkGetPipelineExecutableInternalRepresentationsKHR)gpa(device, "vkGetPipelineExecutableInternalRepresentationsKHR");
    table->MapMemory2KHR = (PFN_vkMapMemory2KHR)gpa(device, "vkMapMemory2KHR");
    table->UnmapMemory2KHR = (PFN_vkUnmapMemory2KHR)gpa(device, "vkUnmapMemory2KHR");
    table->GetEncodedVideoSessionParametersKHR =
        (PFN_vkGetEncodedVideoSessionParametersKHR)gpa(device, "vkGetEncodedVideoSessionParametersKHR");
    table->CmdEncodeVideoKHR = (PFN_vkCmdEncodeVideoKHR)gpa(device, "vkCmdEncodeVideoKHR");
    table->CmdSetEvent2KHR = (PFN_vkCmdSetEvent2KHR)gpa(device, "vkCmdSetEvent2KHR");
    table->CmdResetEvent2KHR = (PFN_vkCmdResetEvent2KHR)gpa(device, "vkCmdResetEvent2KHR");
    table->CmdWaitEvents2KHR = (PFN_vkCmdWaitEvents2KHR)gpa(device, "vkCmdWaitEvents2KHR");
    table->CmdPipelineBarrier2KHR = (PFN_vkCmdPipelineBarrier2KHR)gpa(device, "vkCmdPipelineBarrier2KHR");
    table->CmdWriteTimestamp2KHR = (PFN_vkCmdWriteTimestamp2KHR)gpa(device, "vkCmdWriteTimestamp2KHR");
    table->QueueSubmit2KHR = (PFN_vkQueueSubmit2KHR)gpa(device, "vkQueueSubmit2KHR");
    table->CmdWriteBufferMarker2AMD = (PFN_vkCmdWriteBufferMarker2AMD)gpa(device, "vkCmdWriteBufferMarker2AMD");
    table->GetQueueCheckpointData2NV = (PFN_vkGetQueueCheckpointData2NV)gpa(device, "vkGetQueueCheckpointData2NV");
    table->CmdCopyBuffer2KHR = (PFN_vkCmdCopyBuffer2KHR)gpa(device, "vkCmdCopyBuffer2KHR");
    table->CmdCopyImage2KHR = (PFN_vkCmdCopyImage2KHR)gpa(device, "vkCmdCopyImage2KHR");
    table->CmdCopyBufferToImage2KHR = (PFN_vkCmdCopyBufferToImage2KHR)gpa(device, "vkCmdCopyBufferToImage2KHR");
    table->CmdCopyImageToBuffer2KHR = (PFN_vkCmdCopyImageToBuffer2KHR)gpa(device, "vkCmdCopyImageToBuffer2KHR");
    table->CmdBlitImage2KHR = (PFN_vkCmdBlitImage2KHR)gpa(device, "vkCmdBlitImage2KHR");
    table->CmdResolveImage2KHR = (PFN_vkCmdResolveImage2KHR)gpa(device, "vkCmdResolveImage2KHR");
    table->CmdTraceRaysIndirect2KHR = (PFN_vkCmdTraceRaysIndirect2KHR)gpa(device, "vkCmdTraceRaysIndirect2KHR");
    table->GetDeviceBufferMemoryRequirementsKHR =
        (PFN_vkGetDeviceBufferMemoryRequirementsKHR)gpa(device, "vkGetDeviceBufferMemoryRequirementsKHR");
    table->GetDeviceImageMemoryRequirementsKHR =
        (PFN_vkGetDeviceImageMemoryRequirementsKHR)gpa(device, "vkGetDeviceImageMemoryRequirementsKHR");
    table->GetDeviceImageSparseMemoryRequirementsKHR =
        (PFN_vkGetDeviceImageSparseMemoryRequirementsKHR)gpa(device, "vkGetDeviceImageSparseMemoryRequirementsKHR");
    table->CmdBindIndexBuffer2KHR = (PFN_vkCmdBindIndexBuffer2KHR)gpa(device, "vkCmdBindIndexBuffer2KHR");
    table->GetRenderingAreaGranularityKHR = (PFN_vkGetRenderingAreaGranularityKHR)gpa(device, "vkGetRenderingAreaGranularityKHR");
    table->GetDeviceImageSubresourceLayoutKHR =
        (PFN_vkGetDeviceImageSubresourceLayoutKHR)gpa(device, "vkGetDeviceImageSubresourceLayoutKHR");
    table->GetImageSubresourceLayout2KHR = (PFN_vkGetImageSubresourceLayout2KHR)gpa(device, "vkGetImageSubresourceLayout2KHR");
    table->CmdSetLineStippleKHR = (PFN_vkCmdSetLineStippleKHR)gpa(device, "vkCmdSetLineStippleKHR");
    table->GetCalibratedTimestampsKHR = (PFN_vkGetCalibratedTimestampsKHR)gpa(device, "vkGetCalibratedTimestampsKHR");
    table->CmdBindDescriptorSets2KHR = (PFN_vkCmdBindDescriptorSets2KHR)gpa(device, "vkCmdBindDescriptorSets2KHR");
    table->CmdPushConstants2KHR = (PFN_vkCmdPushConstants2KHR)gpa(device, "vkCmdPushConstants2KHR");
    table->CmdPushDescriptorSet2KHR = (PFN_vkCmdPushDescriptorSet2KHR)gpa(device, "vkCmdPushDescriptorSet2KHR");
    table->CmdPushDescriptorSetWithTemplate2KHR =
        (PFN_vkCmdPushDescriptorSetWithTemplate2KHR)gpa(device, "vkCmdPushDescriptorSetWithTemplate2KHR");
    table->CmdSetDescriptorBufferOffsets2EXT =
        (PFN_vkCmdSetDescriptorBufferOffsets2EXT)gpa(device, "vkCmdSetDescriptorBufferOffsets2EXT");
    table->CmdBindDescriptorBufferEmbeddedSamplers2EXT =
        (PFN_vkCmdBindDescriptorBufferEmbeddedSamplers2EXT)gpa(device, "vkCmdBindDescriptorBufferEmbeddedSamplers2EXT");
    table->DebugMarkerSetObjectTagEXT = (PFN_vkDebugMarkerSetObjectTagEXT)gpa(device, "vkDebugMarkerSetObjectTagEXT");
    if (table->DebugMarkerSetObjectTagEXT == nullptr) {
        table->DebugMarkerSetObjectTagEXT = (PFN_vkDebugMarkerSetObjectTagEXT)StubDebugMarkerSetObjectTagEXT;
    }
    table->DebugMarkerSetObjectNameEXT = (PFN_vkDebugMarkerSetObjectNameEXT)gpa(device, "vkDebugMarkerSetObjectNameEXT");
    if (table->DebugMarkerSetObjectNameEXT == nullptr) {
        table->DebugMarkerSetObjectNameEXT = (PFN_vkDebugMarkerSetObjectNameEXT)StubDebugMarkerSetObjectNameEXT;
    }
    table->CmdDebugMarkerBeginEXT = (PFN_vkCmdDebugMarkerBeginEXT)gpa(device, "vkCmdDebugMarkerBeginEXT");
    if (table->CmdDebugMarkerBeginEXT == nullptr) {
        table->CmdDebugMarkerBeginEXT = (PFN_vkCmdDebugMarkerBeginEXT)StubCmdDebugMarkerBeginEXT;
    }
    table->CmdDebugMarkerEndEXT = (PFN_vkCmdDebugMarkerEndEXT)gpa(device, "vkCmdDebugMarkerEndEXT");
    if (table->CmdDebugMarkerEndEXT == nullptr) {
        table->CmdDebugMarkerEndEXT = (PFN_vkCmdDebugMarkerEndEXT)StubCmdDebugMarkerEndEXT;
    }
    table->CmdDebugMarkerInsertEXT = (PFN_vkCmdDebugMarkerInsertEXT)gpa(device, "vkCmdDebugMarkerInsertEXT");
    if (table->CmdDebugMarkerInsertEXT == nullptr) {
        table->CmdDebugMarkerInsertEXT = (PFN_vkCmdDebugMarkerInsertEXT)StubCmdDebugMarkerInsertEXT;
    }
    table->CmdBindTransformFeedbackBuffersEXT =
        (PFN_vkCmdBindTransformFeedbackBuffersEXT)gpa(device, "vkCmdBindTransformFeedbackBuffersEXT");
    table->CmdBeginTransformFeedbackEXT = (PFN_vkCmdBeginTransformFeedbackEXT)gpa(device, "vkCmdBeginTransformFeedbackEXT");
    table->CmdEndTransformFeedbackEXT = (PFN_vkCmdEndTransformFeedbackEXT)gpa(device, "vkCmdEndTransformFeedbackEXT");
    table->CmdBeginQueryIndexedEXT = (PFN_vkCmdBeginQueryIndexedEXT)gpa(device, "vkCmdBeginQueryIndexedEXT");
    table->CmdEndQueryIndexedEXT = (PFN_vkCmdEndQueryIndexedEXT)gpa(device, "vkCmdEndQueryIndexedEXT");
    table->CmdDrawIndirectByteCountEXT = (PFN_vkCmdDrawIndirectByteCountEXT)gpa(device, "vkCmdDrawIndirectByteCountEXT");
    table->CreateCuModuleNVX = (PFN_vkCreateCuModuleNVX)gpa(device, "vkCreateCuModuleNVX");
    table->CreateCuFunctionNVX = (PFN_vkCreateCuFunctionNVX)gpa(device, "vkCreateCuFunctionNVX");
    table->DestroyCuModuleNVX = (PFN_vkDestroyCuModuleNVX)gpa(device, "vkDestroyCuModuleNVX");
    table->DestroyCuFunctionNVX = (PFN_vkDestroyCuFunctionNVX)gpa(device, "vkDestroyCuFunctionNVX");
    table->CmdCuLaunchKernelNVX = (PFN_vkCmdCuLaunchKernelNVX)gpa(device, "vkCmdCuLaunchKernelNVX");
    table->GetImageViewHandleNVX = (PFN_vkGetImageViewHandleNVX)gpa(device, "vkGetImageViewHandleNVX");
    table->GetImageViewAddressNVX = (PFN_vkGetImageViewAddressNVX)gpa(device, "vkGetImageViewAddressNVX");
    table->CmdDrawIndirectCountAMD = (PFN_vkCmdDrawIndirectCountAMD)gpa(device, "vkCmdDrawIndirectCountAMD");
    table->CmdDrawIndexedIndirectCountAMD = (PFN_vkCmdDrawIndexedIndirectCountAMD)gpa(device, "vkCmdDrawIndexedIndirectCountAMD");
    table->GetShaderInfoAMD = (PFN_vkGetShaderInfoAMD)gpa(device, "vkGetShaderInfoAMD");
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->GetMemoryWin32HandleNV = (PFN_vkGetMemoryWin32HandleNV)gpa(device, "vkGetMemoryWin32HandleNV");
#endif  // VK_USE_PLATFORM_WIN32_KHR
    table->CmdBeginConditionalRenderingEXT =
        (PFN_vkCmdBeginConditionalRenderingEXT)gpa(device, "vkCmdBeginConditionalRenderingEXT");
    table->CmdEndConditionalRenderingEXT = (PFN_vkCmdEndConditionalRenderingEXT)gpa(device, "vkCmdEndConditionalRenderingEXT");
    table->CmdSetViewportWScalingNV = (PFN_vkCmdSetViewportWScalingNV)gpa(device, "vkCmdSetViewportWScalingNV");
    table->DisplayPowerControlEXT = (PFN_vkDisplayPowerControlEXT)gpa(device, "vkDisplayPowerControlEXT");
    table->RegisterDeviceEventEXT = (PFN_vkRegisterDeviceEventEXT)gpa(device, "vkRegisterDeviceEventEXT");
    table->RegisterDisplayEventEXT = (PFN_vkRegisterDisplayEventEXT)gpa(device, "vkRegisterDisplayEventEXT");
    table->GetSwapchainCounterEXT = (PFN_vkGetSwapchainCounterEXT)gpa(device, "vkGetSwapchainCounterEXT");
    table->GetRefreshCycleDurationGOOGLE = (PFN_vkGetRefreshCycleDurationGOOGLE)gpa(device, "vkGetRefreshCycleDurationGOOGLE");
    table->GetPastPresentationTimingGOOGLE =
        (PFN_vkGetPastPresentationTimingGOOGLE)gpa(device, "vkGetPastPresentationTimingGOOGLE");
    table->CmdSetDiscardRectangleEXT = (PFN_vkCmdSetDiscardRectangleEXT)gpa(device, "vkCmdSetDiscardRectangleEXT");
    table->CmdSetDiscardRectangleEnableEXT =
        (PFN_vkCmdSetDiscardRectangleEnableEXT)gpa(device, "vkCmdSetDiscardRectangleEnableEXT");
    table->CmdSetDiscardRectangleModeEXT = (PFN_vkCmdSetDiscardRectangleModeEXT)gpa(device, "vkCmdSetDiscardRectangleModeEXT");
    table->SetHdrMetadataEXT = (PFN_vkSetHdrMetadataEXT)gpa(device, "vkSetHdrMetadataEXT");
    table->SetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)gpa(device, "vkSetDebugUtilsObjectNameEXT");
    if (table->SetDebugUtilsObjectNameEXT == nullptr) {
        table->SetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)StubSetDebugUtilsObjectNameEXT;
    }
    table->SetDebugUtilsObjectTagEXT = (PFN_vkSetDebugUtilsObjectTagEXT)gpa(device, "vkSetDebugUtilsObjectTagEXT");
    if (table->SetDebugUtilsObjectTagEXT == nullptr) {
        table->SetDebugUtilsObjectTagEXT = (PFN_vkSetDebugUtilsObjectTagEXT)StubSetDebugUtilsObjectTagEXT;
    }
    table->QueueBeginDebugUtilsLabelEXT = (PFN_vkQueueBeginDebugUtilsLabelEXT)gpa(device, "vkQueueBeginDebugUtilsLabelEXT");
    if (table->QueueBeginDebugUtilsLabelEXT == nullptr) {
        table->QueueBeginDebugUtilsLabelEXT = (PFN_vkQueueBeginDebugUtilsLabelEXT)StubQueueBeginDebugUtilsLabelEXT;
    }
    table->QueueEndDebugUtilsLabelEXT = (PFN_vkQueueEndDebugUtilsLabelEXT)gpa(device, "vkQueueEndDebugUtilsLabelEXT");
    if (table->QueueEndDebugUtilsLabelEXT == nullptr) {
        table->QueueEndDebugUtilsLabelEXT = (PFN_vkQueueEndDebugUtilsLabelEXT)StubQueueEndDebugUtilsLabelEXT;
    }
    table->QueueInsertDebugUtilsLabelEXT = (PFN_vkQueueInsertDebugUtilsLabelEXT)gpa(device, "vkQueueInsertDebugUtilsLabelEXT");
    if (table->QueueInsertDebugUtilsLabelEXT == nullptr) {
        table->QueueInsertDebugUtilsLabelEXT = (PFN_vkQueueInsertDebugUtilsLabelEXT)StubQueueInsertDebugUtilsLabelEXT;
    }
    table->CmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)gpa(device, "vkCmdBeginDebugUtilsLabelEXT");
    if (table->CmdBeginDebugUtilsLabelEXT == nullptr) {
        table->CmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)StubCmdBeginDebugUtilsLabelEXT;
    }
    table->CmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)gpa(device, "vkCmdEndDebugUtilsLabelEXT");
    if (table->CmdEndDebugUtilsLabelEXT == nullptr) {
        table->CmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)StubCmdEndDebugUtilsLabelEXT;
    }
    table->CmdInsertDebugUtilsLabelEXT = (PFN_vkCmdInsertDebugUtilsLabelEXT)gpa(device, "vkCmdInsertDebugUtilsLabelEXT");
    if (table->CmdInsertDebugUtilsLabelEXT == nullptr) {
        table->CmdInsertDebugUtilsLabelEXT = (PFN_vkCmdInsertDebugUtilsLabelEXT)StubCmdInsertDebugUtilsLabelEXT;
    }
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    table->GetAndroidHardwareBufferPropertiesANDROID =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)gpa(device, "vkGetAndroidHardwareBufferPropertiesANDROID");
    table->GetMemoryAndroidHardwareBufferANDROID =
        (PFN_vkGetMemoryAndroidHardwareBufferANDROID)gpa(device, "vkGetMemoryAndroidHardwareBufferANDROID");
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_ENABLE_BETA_EXTENSIONS
    table->CreateExecutionGraphPipelinesAMDX =
        (PFN_vkCreateExecutionGraphPipelinesAMDX)gpa(device, "vkCreateExecutionGraphPipelinesAMDX");
    table->GetExecutionGraphPipelineScratchSizeAMDX =
        (PFN_vkGetExecutionGraphPipelineScratchSizeAMDX)gpa(device, "vkGetExecutionGraphPipelineScratchSizeAMDX");
    table->GetExecutionGraphPipelineNodeIndexAMDX =
        (PFN_vkGetExecutionGraphPipelineNodeIndexAMDX)gpa(device, "vkGetExecutionGraphPipelineNodeIndexAMDX");
    table->CmdInitializeGraphScratchMemoryAMDX =
        (PFN_vkCmdInitializeGraphScratchMemoryAMDX)gpa(device, "vkCmdInitializeGraphScratchMemoryAMDX");
    table->CmdDispatchGraphAMDX = (PFN_vkCmdDispatchGraphAMDX)gpa(device, "vkCmdDispatchGraphAMDX");
    table->CmdDispatchGraphIndirectAMDX = (PFN_vkCmdDispatchGraphIndirectAMDX)gpa(device, "vkCmdDispatchGraphIndirectAMDX");
    table->CmdDispatchGraphIndirectCountAMDX =
        (PFN_vkCmdDispatchGraphIndirectCountAMDX)gpa(device, "vkCmdDispatchGraphIndirectCountAMDX");
#endif  // VK_ENABLE_BETA_EXTENSIONS
    table->CmdSetSampleLocationsEXT = (PFN_vkCmdSetSampleLocationsEXT)gpa(device, "vkCmdSetSampleLocationsEXT");
    table->GetImageDrmFormatModifierPropertiesEXT =
        (PFN_vkGetImageDrmFormatModifierPropertiesEXT)gpa(device, "vkGetImageDrmFormatModifierPropertiesEXT");
    table->CreateValidationCacheEXT = (PFN_vkCreateValidationCacheEXT)gpa(device, "vkCreateValidationCacheEXT");
    table->DestroyValidationCacheEXT = (PFN_vkDestroyValidationCacheEXT)gpa(device, "vkDestroyValidationCacheEXT");
    table->MergeValidationCachesEXT = (PFN_vkMergeValidationCachesEXT)gpa(device, "vkMergeValidationCachesEXT");
    table->GetValidationCacheDataEXT = (PFN_vkGetValidationCacheDataEXT)gpa(device, "vkGetValidationCacheDataEXT");
    table->CmdBindShadingRateImageNV = (PFN_vkCmdBindShadingRateImageNV)gpa(device, "vkCmdBindShadingRateImageNV");
    table->CmdSetViewportShadingRatePaletteNV =
        (PFN_vkCmdSetViewportShadingRatePaletteNV)gpa(device, "vkCmdSetViewportShadingRatePaletteNV");
    table->CmdSetCoarseSampleOrderNV = (PFN_vkCmdSetCoarseSampleOrderNV)gpa(device, "vkCmdSetCoarseSampleOrderNV");
    table->CreateAccelerationStructureNV = (PFN_vkCreateAccelerationStructureNV)gpa(device, "vkCreateAccelerationStructureNV");
    table->DestroyAccelerationStructureNV = (PFN_vkDestroyAccelerationStructureNV)gpa(device, "vkDestroyAccelerationStructureNV");
    table->GetAccelerationStructureMemoryRequirementsNV =
        (PFN_vkGetAccelerationStructureMemoryRequirementsNV)gpa(device, "vkGetAccelerationStructureMemoryRequirementsNV");
    table->BindAccelerationStructureMemoryNV =
        (PFN_vkBindAccelerationStructureMemoryNV)gpa(device, "vkBindAccelerationStructureMemoryNV");
    table->CmdBuildAccelerationStructureNV =
        (PFN_vkCmdBuildAccelerationStructureNV)gpa(device, "vkCmdBuildAccelerationStructureNV");
    table->CmdCopyAccelerationStructureNV = (PFN_vkCmdCopyAccelerationStructureNV)gpa(device, "vkCmdCopyAccelerationStructureNV");
    table->CmdTraceRaysNV = (PFN_vkCmdTraceRaysNV)gpa(device, "vkCmdTraceRaysNV");
    table->CreateRayTracingPipelinesNV = (PFN_vkCreateRayTracingPipelinesNV)gpa(device, "vkCreateRayTracingPipelinesNV");
    table->GetRayTracingShaderGroupHandlesKHR =
        (PFN_vkGetRayTracingShaderGroupHandlesKHR)gpa(device, "vkGetRayTracingShaderGroupHandlesKHR");
    table->GetRayTracingShaderGroupHandlesNV =
        (PFN_vkGetRayTracingShaderGroupHandlesNV)gpa(device, "vkGetRayTracingShaderGroupHandlesNV");
    table->GetAccelerationStructureHandleNV =
        (PFN_vkGetAccelerationStructureHandleNV)gpa(device, "vkGetAccelerationStructureHandleNV");
    table->CmdWriteAccelerationStructuresPropertiesNV =
        (PFN_vkCmdWriteAccelerationStructuresPropertiesNV)gpa(device, "vkCmdWriteAccelerationStructuresPropertiesNV");
    table->CompileDeferredNV = (PFN_vkCompileDeferredNV)gpa(device, "vkCompileDeferredNV");
    table->GetMemoryHostPointerPropertiesEXT =
        (PFN_vkGetMemoryHostPointerPropertiesEXT)gpa(device, "vkGetMemoryHostPointerPropertiesEXT");
    table->CmdWriteBufferMarkerAMD = (PFN_vkCmdWriteBufferMarkerAMD)gpa(device, "vkCmdWriteBufferMarkerAMD");
    table->GetCalibratedTimestampsEXT = (PFN_vkGetCalibratedTimestampsEXT)gpa(device, "vkGetCalibratedTimestampsEXT");
    table->CmdDrawMeshTasksNV = (PFN_vkCmdDrawMeshTasksNV)gpa(device, "vkCmdDrawMeshTasksNV");
    table->CmdDrawMeshTasksIndirectNV = (PFN_vkCmdDrawMeshTasksIndirectNV)gpa(device, "vkCmdDrawMeshTasksIndirectNV");
    table->CmdDrawMeshTasksIndirectCountNV =
        (PFN_vkCmdDrawMeshTasksIndirectCountNV)gpa(device, "vkCmdDrawMeshTasksIndirectCountNV");
    table->CmdSetExclusiveScissorEnableNV = (PFN_vkCmdSetExclusiveScissorEnableNV)gpa(device, "vkCmdSetExclusiveScissorEnableNV");
    table->CmdSetExclusiveScissorNV = (PFN_vkCmdSetExclusiveScissorNV)gpa(device, "vkCmdSetExclusiveScissorNV");
    table->CmdSetCheckpointNV = (PFN_vkCmdSetCheckpointNV)gpa(device, "vkCmdSetCheckpointNV");
    table->GetQueueCheckpointDataNV = (PFN_vkGetQueueCheckpointDataNV)gpa(device, "vkGetQueueCheckpointDataNV");
    table->InitializePerformanceApiINTEL = (PFN_vkInitializePerformanceApiINTEL)gpa(device, "vkInitializePerformanceApiINTEL");
    table->UninitializePerformanceApiINTEL =
        (PFN_vkUninitializePerformanceApiINTEL)gpa(device, "vkUninitializePerformanceApiINTEL");
    table->CmdSetPerformanceMarkerINTEL = (PFN_vkCmdSetPerformanceMarkerINTEL)gpa(device, "vkCmdSetPerformanceMarkerINTEL");
    table->CmdSetPerformanceStreamMarkerINTEL =
        (PFN_vkCmdSetPerformanceStreamMarkerINTEL)gpa(device, "vkCmdSetPerformanceStreamMarkerINTEL");
    table->CmdSetPerformanceOverrideINTEL = (PFN_vkCmdSetPerformanceOverrideINTEL)gpa(device, "vkCmdSetPerformanceOverrideINTEL");
    table->AcquirePerformanceConfigurationINTEL =
        (PFN_vkAcquirePerformanceConfigurationINTEL)gpa(device, "vkAcquirePerformanceConfigurationINTEL");
    table->ReleasePerformanceConfigurationINTEL =
        (PFN_vkReleasePerformanceConfigurationINTEL)gpa(device, "vkReleasePerformanceConfigurationINTEL");
    table->QueueSetPerformanceConfigurationINTEL =
        (PFN_vkQueueSetPerformanceConfigurationINTEL)gpa(device, "vkQueueSetPerformanceConfigurationINTEL");
    table->GetPerformanceParameterINTEL = (PFN_vkGetPerformanceParameterINTEL)gpa(device, "vkGetPerformanceParameterINTEL");
    table->SetLocalDimmingAMD = (PFN_vkSetLocalDimmingAMD)gpa(device, "vkSetLocalDimmingAMD");
    table->GetBufferDeviceAddressEXT = (PFN_vkGetBufferDeviceAddressEXT)gpa(device, "vkGetBufferDeviceAddressEXT");
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->AcquireFullScreenExclusiveModeEXT =
        (PFN_vkAcquireFullScreenExclusiveModeEXT)gpa(device, "vkAcquireFullScreenExclusiveModeEXT");
    table->ReleaseFullScreenExclusiveModeEXT =
        (PFN_vkReleaseFullScreenExclusiveModeEXT)gpa(device, "vkReleaseFullScreenExclusiveModeEXT");
    table->GetDeviceGroupSurfacePresentModes2EXT =
        (PFN_vkGetDeviceGroupSurfacePresentModes2EXT)gpa(device, "vkGetDeviceGroupSurfacePresentModes2EXT");
#endif  // VK_USE_PLATFORM_WIN32_KHR
    table->CmdSetLineStippleEXT = (PFN_vkCmdSetLineStippleEXT)gpa(device, "vkCmdSetLineStippleEXT");
    table->ResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)gpa(device, "vkResetQueryPoolEXT");
    table->CmdSetCullModeEXT = (PFN_vkCmdSetCullModeEXT)gpa(device, "vkCmdSetCullModeEXT");
    table->CmdSetFrontFaceEXT = (PFN_vkCmdSetFrontFaceEXT)gpa(device, "vkCmdSetFrontFaceEXT");
    table->CmdSetPrimitiveTopologyEXT = (PFN_vkCmdSetPrimitiveTopologyEXT)gpa(device, "vkCmdSetPrimitiveTopologyEXT");
    table->CmdSetViewportWithCountEXT = (PFN_vkCmdSetViewportWithCountEXT)gpa(device, "vkCmdSetViewportWithCountEXT");
    table->CmdSetScissorWithCountEXT = (PFN_vkCmdSetScissorWithCountEXT)gpa(device, "vkCmdSetScissorWithCountEXT");
    table->CmdBindVertexBuffers2EXT = (PFN_vkCmdBindVertexBuffers2EXT)gpa(device, "vkCmdBindVertexBuffers2EXT");
    table->CmdSetDepthTestEnableEXT = (PFN_vkCmdSetDepthTestEnableEXT)gpa(device, "vkCmdSetDepthTestEnableEXT");
    table->CmdSetDepthWriteEnableEXT = (PFN_vkCmdSetDepthWriteEnableEXT)gpa(device, "vkCmdSetDepthWriteEnableEXT");
    table->CmdSetDepthCompareOpEXT = (PFN_vkCmdSetDepthCompareOpEXT)gpa(device, "vkCmdSetDepthCompareOpEXT");
    table->CmdSetDepthBoundsTestEnableEXT = (PFN_vkCmdSetDepthBoundsTestEnableEXT)gpa(device, "vkCmdSetDepthBoundsTestEnableEXT");
    table->CmdSetStencilTestEnableEXT = (PFN_vkCmdSetStencilTestEnableEXT)gpa(device, "vkCmdSetStencilTestEnableEXT");
    table->CmdSetStencilOpEXT = (PFN_vkCmdSetStencilOpEXT)gpa(device, "vkCmdSetStencilOpEXT");
    table->CopyMemoryToImageEXT = (PFN_vkCopyMemoryToImageEXT)gpa(device, "vkCopyMemoryToImageEXT");
    table->CopyImageToMemoryEXT = (PFN_vkCopyImageToMemoryEXT)gpa(device, "vkCopyImageToMemoryEXT");
    table->CopyImageToImageEXT = (PFN_vkCopyImageToImageEXT)gpa(device, "vkCopyImageToImageEXT");
    table->TransitionImageLayoutEXT = (PFN_vkTransitionImageLayoutEXT)gpa(device, "vkTransitionImageLayoutEXT");
    table->GetImageSubresourceLayout2EXT = (PFN_vkGetImageSubresourceLayout2EXT)gpa(device, "vkGetImageSubresourceLayout2EXT");
    table->ReleaseSwapchainImagesEXT = (PFN_vkReleaseSwapchainImagesEXT)gpa(device, "vkReleaseSwapchainImagesEXT");
    table->GetGeneratedCommandsMemoryRequirementsNV =
        (PFN_vkGetGeneratedCommandsMemoryRequirementsNV)gpa(device, "vkGetGeneratedCommandsMemoryRequirementsNV");
    table->CmdPreprocessGeneratedCommandsNV =
        (PFN_vkCmdPreprocessGeneratedCommandsNV)gpa(device, "vkCmdPreprocessGeneratedCommandsNV");
    table->CmdExecuteGeneratedCommandsNV = (PFN_vkCmdExecuteGeneratedCommandsNV)gpa(device, "vkCmdExecuteGeneratedCommandsNV");
    table->CmdBindPipelineShaderGroupNV = (PFN_vkCmdBindPipelineShaderGroupNV)gpa(device, "vkCmdBindPipelineShaderGroupNV");
    table->CreateIndirectCommandsLayoutNV = (PFN_vkCreateIndirectCommandsLayoutNV)gpa(device, "vkCreateIndirectCommandsLayoutNV");
    table->DestroyIndirectCommandsLayoutNV =
        (PFN_vkDestroyIndirectCommandsLayoutNV)gpa(device, "vkDestroyIndirectCommandsLayoutNV");
    table->CmdSetDepthBias2EXT = (PFN_vkCmdSetDepthBias2EXT)gpa(device, "vkCmdSetDepthBias2EXT");
    table->CreatePrivateDataSlotEXT = (PFN_vkCreatePrivateDataSlotEXT)gpa(device, "vkCreatePrivateDataSlotEXT");
    table->DestroyPrivateDataSlotEXT = (PFN_vkDestroyPrivateDataSlotEXT)gpa(device, "vkDestroyPrivateDataSlotEXT");
    table->SetPrivateDataEXT = (PFN_vkSetPrivateDataEXT)gpa(device, "vkSetPrivateDataEXT");
    table->GetPrivateDataEXT = (PFN_vkGetPrivateDataEXT)gpa(device, "vkGetPrivateDataEXT");
    table->CreateCudaModuleNV = (PFN_vkCreateCudaModuleNV)gpa(device, "vkCreateCudaModuleNV");
    table->GetCudaModuleCacheNV = (PFN_vkGetCudaModuleCacheNV)gpa(device, "vkGetCudaModuleCacheNV");
    table->CreateCudaFunctionNV = (PFN_vkCreateCudaFunctionNV)gpa(device, "vkCreateCudaFunctionNV");
    table->DestroyCudaModuleNV = (PFN_vkDestroyCudaModuleNV)gpa(device, "vkDestroyCudaModuleNV");
    table->DestroyCudaFunctionNV = (PFN_vkDestroyCudaFunctionNV)gpa(device, "vkDestroyCudaFunctionNV");
    table->CmdCudaLaunchKernelNV = (PFN_vkCmdCudaLaunchKernelNV)gpa(device, "vkCmdCudaLaunchKernelNV");
#ifdef VK_USE_PLATFORM_METAL_EXT
    table->ExportMetalObjectsEXT = (PFN_vkExportMetalObjectsEXT)gpa(device, "vkExportMetalObjectsEXT");
#endif  // VK_USE_PLATFORM_METAL_EXT
    table->GetDescriptorSetLayoutSizeEXT = (PFN_vkGetDescriptorSetLayoutSizeEXT)gpa(device, "vkGetDescriptorSetLayoutSizeEXT");
    table->GetDescriptorSetLayoutBindingOffsetEXT =
        (PFN_vkGetDescriptorSetLayoutBindingOffsetEXT)gpa(device, "vkGetDescriptorSetLayoutBindingOffsetEXT");
    table->GetDescriptorEXT = (PFN_vkGetDescriptorEXT)gpa(device, "vkGetDescriptorEXT");
    table->CmdBindDescriptorBuffersEXT = (PFN_vkCmdBindDescriptorBuffersEXT)gpa(device, "vkCmdBindDescriptorBuffersEXT");
    table->CmdSetDescriptorBufferOffsetsEXT =
        (PFN_vkCmdSetDescriptorBufferOffsetsEXT)gpa(device, "vkCmdSetDescriptorBufferOffsetsEXT");
    table->CmdBindDescriptorBufferEmbeddedSamplersEXT =
        (PFN_vkCmdBindDescriptorBufferEmbeddedSamplersEXT)gpa(device, "vkCmdBindDescriptorBufferEmbeddedSamplersEXT");
    table->GetBufferOpaqueCaptureDescriptorDataEXT =
        (PFN_vkGetBufferOpaqueCaptureDescriptorDataEXT)gpa(device, "vkGetBufferOpaqueCaptureDescriptorDataEXT");
    table->GetImageOpaqueCaptureDescriptorDataEXT =
        (PFN_vkGetImageOpaqueCaptureDescriptorDataEXT)gpa(device, "vkGetImageOpaqueCaptureDescriptorDataEXT");
    table->GetImageViewOpaqueCaptureDescriptorDataEXT =
        (PFN_vkGetImageViewOpaqueCaptureDescriptorDataEXT)gpa(device, "vkGetImageViewOpaqueCaptureDescriptorDataEXT");
    table->GetSamplerOpaqueCaptureDescriptorDataEXT =
        (PFN_vkGetSamplerOpaqueCaptureDescriptorDataEXT)gpa(device, "vkGetSamplerOpaqueCaptureDescriptorDataEXT");
    table->GetAccelerationStructureOpaqueCaptureDescriptorDataEXT =
        (PFN_vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT)gpa(
            device, "vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT");
    table->CmdSetFragmentShadingRateEnumNV =
        (PFN_vkCmdSetFragmentShadingRateEnumNV)gpa(device, "vkCmdSetFragmentShadingRateEnumNV");
    table->GetDeviceFaultInfoEXT = (PFN_vkGetDeviceFaultInfoEXT)gpa(device, "vkGetDeviceFaultInfoEXT");
    table->CmdSetVertexInputEXT = (PFN_vkCmdSetVertexInputEXT)gpa(device, "vkCmdSetVertexInputEXT");
#ifdef VK_USE_PLATFORM_FUCHSIA
    table->GetMemoryZirconHandleFUCHSIA = (PFN_vkGetMemoryZirconHandleFUCHSIA)gpa(device, "vkGetMemoryZirconHandleFUCHSIA");
    table->GetMemoryZirconHandlePropertiesFUCHSIA =
        (PFN_vkGetMemoryZirconHandlePropertiesFUCHSIA)gpa(device, "vkGetMemoryZirconHandlePropertiesFUCHSIA");
    table->ImportSemaphoreZirconHandleFUCHSIA =
        (PFN_vkImportSemaphoreZirconHandleFUCHSIA)gpa(device, "vkImportSemaphoreZirconHandleFUCHSIA");
    table->GetSemaphoreZirconHandleFUCHSIA =
        (PFN_vkGetSemaphoreZirconHandleFUCHSIA)gpa(device, "vkGetSemaphoreZirconHandleFUCHSIA");
    table->CreateBufferCollectionFUCHSIA = (PFN_vkCreateBufferCollectionFUCHSIA)gpa(device, "vkCreateBufferCollectionFUCHSIA");
    table->SetBufferCollectionImageConstraintsFUCHSIA =
        (PFN_vkSetBufferCollectionImageConstraintsFUCHSIA)gpa(device, "vkSetBufferCollectionImageConstraintsFUCHSIA");
    table->SetBufferCollectionBufferConstraintsFUCHSIA =
        (PFN_vkSetBufferCollectionBufferConstraintsFUCHSIA)gpa(device, "vkSetBufferCollectionBufferConstraintsFUCHSIA");
    table->DestroyBufferCollectionFUCHSIA = (PFN_vkDestroyBufferCollectionFUCHSIA)gpa(device, "vkDestroyBufferCollectionFUCHSIA");
    table->GetBufferCollectionPropertiesFUCHSIA =
        (PFN_vkGetBufferCollectionPropertiesFUCHSIA)gpa(device, "vkGetBufferCollectionPropertiesFUCHSIA");
#endif  // VK_USE_PLATFORM_FUCHSIA
    table->GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI =
        (PFN_vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI)gpa(device, "vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI");
    table->CmdSubpassShadingHUAWEI = (PFN_vkCmdSubpassShadingHUAWEI)gpa(device, "vkCmdSubpassShadingHUAWEI");
    table->CmdBindInvocationMaskHUAWEI = (PFN_vkCmdBindInvocationMaskHUAWEI)gpa(device, "vkCmdBindInvocationMaskHUAWEI");
    table->GetMemoryRemoteAddressNV = (PFN_vkGetMemoryRemoteAddressNV)gpa(device, "vkGetMemoryRemoteAddressNV");
    table->GetPipelinePropertiesEXT = (PFN_vkGetPipelinePropertiesEXT)gpa(device, "vkGetPipelinePropertiesEXT");
    table->CmdSetPatchControlPointsEXT = (PFN_vkCmdSetPatchControlPointsEXT)gpa(device, "vkCmdSetPatchControlPointsEXT");
    table->CmdSetRasterizerDiscardEnableEXT =
        (PFN_vkCmdSetRasterizerDiscardEnableEXT)gpa(device, "vkCmdSetRasterizerDiscardEnableEXT");
    table->CmdSetDepthBiasEnableEXT = (PFN_vkCmdSetDepthBiasEnableEXT)gpa(device, "vkCmdSetDepthBiasEnableEXT");
    table->CmdSetLogicOpEXT = (PFN_vkCmdSetLogicOpEXT)gpa(device, "vkCmdSetLogicOpEXT");
    table->CmdSetPrimitiveRestartEnableEXT =
        (PFN_vkCmdSetPrimitiveRestartEnableEXT)gpa(device, "vkCmdSetPrimitiveRestartEnableEXT");
    table->CmdSetColorWriteEnableEXT = (PFN_vkCmdSetColorWriteEnableEXT)gpa(device, "vkCmdSetColorWriteEnableEXT");
    table->CmdDrawMultiEXT = (PFN_vkCmdDrawMultiEXT)gpa(device, "vkCmdDrawMultiEXT");
    table->CmdDrawMultiIndexedEXT = (PFN_vkCmdDrawMultiIndexedEXT)gpa(device, "vkCmdDrawMultiIndexedEXT");
    table->CreateMicromapEXT = (PFN_vkCreateMicromapEXT)gpa(device, "vkCreateMicromapEXT");
    table->DestroyMicromapEXT = (PFN_vkDestroyMicromapEXT)gpa(device, "vkDestroyMicromapEXT");
    table->CmdBuildMicromapsEXT = (PFN_vkCmdBuildMicromapsEXT)gpa(device, "vkCmdBuildMicromapsEXT");
    table->BuildMicromapsEXT = (PFN_vkBuildMicromapsEXT)gpa(device, "vkBuildMicromapsEXT");
    table->CopyMicromapEXT = (PFN_vkCopyMicromapEXT)gpa(device, "vkCopyMicromapEXT");
    table->CopyMicromapToMemoryEXT = (PFN_vkCopyMicromapToMemoryEXT)gpa(device, "vkCopyMicromapToMemoryEXT");
    table->CopyMemoryToMicromapEXT = (PFN_vkCopyMemoryToMicromapEXT)gpa(device, "vkCopyMemoryToMicromapEXT");
    table->WriteMicromapsPropertiesEXT = (PFN_vkWriteMicromapsPropertiesEXT)gpa(device, "vkWriteMicromapsPropertiesEXT");
    table->CmdCopyMicromapEXT = (PFN_vkCmdCopyMicromapEXT)gpa(device, "vkCmdCopyMicromapEXT");
    table->CmdCopyMicromapToMemoryEXT = (PFN_vkCmdCopyMicromapToMemoryEXT)gpa(device, "vkCmdCopyMicromapToMemoryEXT");
    table->CmdCopyMemoryToMicromapEXT = (PFN_vkCmdCopyMemoryToMicromapEXT)gpa(device, "vkCmdCopyMemoryToMicromapEXT");
    table->CmdWriteMicromapsPropertiesEXT = (PFN_vkCmdWriteMicromapsPropertiesEXT)gpa(device, "vkCmdWriteMicromapsPropertiesEXT");
    table->GetDeviceMicromapCompatibilityEXT =
        (PFN_vkGetDeviceMicromapCompatibilityEXT)gpa(device, "vkGetDeviceMicromapCompatibilityEXT");
    table->GetMicromapBuildSizesEXT = (PFN_vkGetMicromapBuildSizesEXT)gpa(device, "vkGetMicromapBuildSizesEXT");
    table->CmdDrawClusterHUAWEI = (PFN_vkCmdDrawClusterHUAWEI)gpa(device, "vkCmdDrawClusterHUAWEI");
    table->CmdDrawClusterIndirectHUAWEI = (PFN_vkCmdDrawClusterIndirectHUAWEI)gpa(device, "vkCmdDrawClusterIndirectHUAWEI");
    table->SetDeviceMemoryPriorityEXT = (PFN_vkSetDeviceMemoryPriorityEXT)gpa(device, "vkSetDeviceMemoryPriorityEXT");
    table->GetDescriptorSetLayoutHostMappingInfoVALVE =
        (PFN_vkGetDescriptorSetLayoutHostMappingInfoVALVE)gpa(device, "vkGetDescriptorSetLayoutHostMappingInfoVALVE");
    table->GetDescriptorSetHostMappingVALVE =
        (PFN_vkGetDescriptorSetHostMappingVALVE)gpa(device, "vkGetDescriptorSetHostMappingVALVE");
    table->CmdCopyMemoryIndirectNV = (PFN_vkCmdCopyMemoryIndirectNV)gpa(device, "vkCmdCopyMemoryIndirectNV");
    table->CmdCopyMemoryToImageIndirectNV = (PFN_vkCmdCopyMemoryToImageIndirectNV)gpa(device, "vkCmdCopyMemoryToImageIndirectNV");
    table->CmdDecompressMemoryNV = (PFN_vkCmdDecompressMemoryNV)gpa(device, "vkCmdDecompressMemoryNV");
    table->CmdDecompressMemoryIndirectCountNV =
        (PFN_vkCmdDecompressMemoryIndirectCountNV)gpa(device, "vkCmdDecompressMemoryIndirectCountNV");
    table->GetPipelineIndirectMemoryRequirementsNV =
        (PFN_vkGetPipelineIndirectMemoryRequirementsNV)gpa(device, "vkGetPipelineIndirectMemoryRequirementsNV");
    table->CmdUpdatePipelineIndirectBufferNV =
        (PFN_vkCmdUpdatePipelineIndirectBufferNV)gpa(device, "vkCmdUpdatePipelineIndirectBufferNV");
    table->GetPipelineIndirectDeviceAddressNV =
        (PFN_vkGetPipelineIndirectDeviceAddressNV)gpa(device, "vkGetPipelineIndirectDeviceAddressNV");
    table->CmdSetDepthClampEnableEXT = (PFN_vkCmdSetDepthClampEnableEXT)gpa(device, "vkCmdSetDepthClampEnableEXT");
    table->CmdSetPolygonModeEXT = (PFN_vkCmdSetPolygonModeEXT)gpa(device, "vkCmdSetPolygonModeEXT");
    table->CmdSetRasterizationSamplesEXT = (PFN_vkCmdSetRasterizationSamplesEXT)gpa(device, "vkCmdSetRasterizationSamplesEXT");
    table->CmdSetSampleMaskEXT = (PFN_vkCmdSetSampleMaskEXT)gpa(device, "vkCmdSetSampleMaskEXT");
    table->CmdSetAlphaToCoverageEnableEXT = (PFN_vkCmdSetAlphaToCoverageEnableEXT)gpa(device, "vkCmdSetAlphaToCoverageEnableEXT");
    table->CmdSetAlphaToOneEnableEXT = (PFN_vkCmdSetAlphaToOneEnableEXT)gpa(device, "vkCmdSetAlphaToOneEnableEXT");
    table->CmdSetLogicOpEnableEXT = (PFN_vkCmdSetLogicOpEnableEXT)gpa(device, "vkCmdSetLogicOpEnableEXT");
    table->CmdSetColorBlendEnableEXT = (PFN_vkCmdSetColorBlendEnableEXT)gpa(device, "vkCmdSetColorBlendEnableEXT");
    table->CmdSetColorBlendEquationEXT = (PFN_vkCmdSetColorBlendEquationEXT)gpa(device, "vkCmdSetColorBlendEquationEXT");
    table->CmdSetColorWriteMaskEXT = (PFN_vkCmdSetColorWriteMaskEXT)gpa(device, "vkCmdSetColorWriteMaskEXT");
    table->CmdSetTessellationDomainOriginEXT =
        (PFN_vkCmdSetTessellationDomainOriginEXT)gpa(device, "vkCmdSetTessellationDomainOriginEXT");
    table->CmdSetRasterizationStreamEXT = (PFN_vkCmdSetRasterizationStreamEXT)gpa(device, "vkCmdSetRasterizationStreamEXT");
    table->CmdSetConservativeRasterizationModeEXT =
        (PFN_vkCmdSetConservativeRasterizationModeEXT)gpa(device, "vkCmdSetConservativeRasterizationModeEXT");
    table->CmdSetExtraPrimitiveOverestimationSizeEXT =
        (PFN_vkCmdSetExtraPrimitiveOverestimationSizeEXT)gpa(device, "vkCmdSetExtraPrimitiveOverestimationSizeEXT");
    table->CmdSetDepthClipEnableEXT = (PFN_vkCmdSetDepthClipEnableEXT)gpa(device, "vkCmdSetDepthClipEnableEXT");
    table->CmdSetSampleLocationsEnableEXT = (PFN_vkCmdSetSampleLocationsEnableEXT)gpa(device, "vkCmdSetSampleLocationsEnableEXT");
    table->CmdSetColorBlendAdvancedEXT = (PFN_vkCmdSetColorBlendAdvancedEXT)gpa(device, "vkCmdSetColorBlendAdvancedEXT");
    table->CmdSetProvokingVertexModeEXT = (PFN_vkCmdSetProvokingVertexModeEXT)gpa(device, "vkCmdSetProvokingVertexModeEXT");
    table->CmdSetLineRasterizationModeEXT = (PFN_vkCmdSetLineRasterizationModeEXT)gpa(device, "vkCmdSetLineRasterizationModeEXT");
    table->CmdSetLineStippleEnableEXT = (PFN_vkCmdSetLineStippleEnableEXT)gpa(device, "vkCmdSetLineStippleEnableEXT");
    table->CmdSetDepthClipNegativeOneToOneEXT =
        (PFN_vkCmdSetDepthClipNegativeOneToOneEXT)gpa(device, "vkCmdSetDepthClipNegativeOneToOneEXT");
    table->CmdSetViewportWScalingEnableNV = (PFN_vkCmdSetViewportWScalingEnableNV)gpa(device, "vkCmdSetViewportWScalingEnableNV");
    table->CmdSetViewportSwizzleNV = (PFN_vkCmdSetViewportSwizzleNV)gpa(device, "vkCmdSetViewportSwizzleNV");
    table->CmdSetCoverageToColorEnableNV = (PFN_vkCmdSetCoverageToColorEnableNV)gpa(device, "vkCmdSetCoverageToColorEnableNV");
    table->CmdSetCoverageToColorLocationNV =
        (PFN_vkCmdSetCoverageToColorLocationNV)gpa(device, "vkCmdSetCoverageToColorLocationNV");
    table->CmdSetCoverageModulationModeNV = (PFN_vkCmdSetCoverageModulationModeNV)gpa(device, "vkCmdSetCoverageModulationModeNV");
    table->CmdSetCoverageModulationTableEnableNV =
        (PFN_vkCmdSetCoverageModulationTableEnableNV)gpa(device, "vkCmdSetCoverageModulationTableEnableNV");
    table->CmdSetCoverageModulationTableNV =
        (PFN_vkCmdSetCoverageModulationTableNV)gpa(device, "vkCmdSetCoverageModulationTableNV");
    table->CmdSetShadingRateImageEnableNV = (PFN_vkCmdSetShadingRateImageEnableNV)gpa(device, "vkCmdSetShadingRateImageEnableNV");
    table->CmdSetRepresentativeFragmentTestEnableNV =
        (PFN_vkCmdSetRepresentativeFragmentTestEnableNV)gpa(device, "vkCmdSetRepresentativeFragmentTestEnableNV");
    table->CmdSetCoverageReductionModeNV = (PFN_vkCmdSetCoverageReductionModeNV)gpa(device, "vkCmdSetCoverageReductionModeNV");
    table->GetShaderModuleIdentifierEXT = (PFN_vkGetShaderModuleIdentifierEXT)gpa(device, "vkGetShaderModuleIdentifierEXT");
    table->GetShaderModuleCreateInfoIdentifierEXT =
        (PFN_vkGetShaderModuleCreateInfoIdentifierEXT)gpa(device, "vkGetShaderModuleCreateInfoIdentifierEXT");
    table->CreateOpticalFlowSessionNV = (PFN_vkCreateOpticalFlowSessionNV)gpa(device, "vkCreateOpticalFlowSessionNV");
    table->DestroyOpticalFlowSessionNV = (PFN_vkDestroyOpticalFlowSessionNV)gpa(device, "vkDestroyOpticalFlowSessionNV");
    table->BindOpticalFlowSessionImageNV = (PFN_vkBindOpticalFlowSessionImageNV)gpa(device, "vkBindOpticalFlowSessionImageNV");
    table->CmdOpticalFlowExecuteNV = (PFN_vkCmdOpticalFlowExecuteNV)gpa(device, "vkCmdOpticalFlowExecuteNV");
    table->CreateShadersEXT = (PFN_vkCreateShadersEXT)gpa(device, "vkCreateShadersEXT");
    table->DestroyShaderEXT = (PFN_vkDestroyShaderEXT)gpa(device, "vkDestroyShaderEXT");
    table->GetShaderBinaryDataEXT = (PFN_vkGetShaderBinaryDataEXT)gpa(device, "vkGetShaderBinaryDataEXT");
    table->CmdBindShadersEXT = (PFN_vkCmdBindShadersEXT)gpa(device, "vkCmdBindShadersEXT");
    table->GetFramebufferTilePropertiesQCOM =
        (PFN_vkGetFramebufferTilePropertiesQCOM)gpa(device, "vkGetFramebufferTilePropertiesQCOM");
    table->GetDynamicRenderingTilePropertiesQCOM =
        (PFN_vkGetDynamicRenderingTilePropertiesQCOM)gpa(device, "vkGetDynamicRenderingTilePropertiesQCOM");
    table->SetLatencySleepModeNV = (PFN_vkSetLatencySleepModeNV)gpa(device, "vkSetLatencySleepModeNV");
    table->LatencySleepNV = (PFN_vkLatencySleepNV)gpa(device, "vkLatencySleepNV");
    table->SetLatencyMarkerNV = (PFN_vkSetLatencyMarkerNV)gpa(device, "vkSetLatencyMarkerNV");
    table->GetLatencyTimingsNV = (PFN_vkGetLatencyTimingsNV)gpa(device, "vkGetLatencyTimingsNV");
    table->QueueNotifyOutOfBandNV = (PFN_vkQueueNotifyOutOfBandNV)gpa(device, "vkQueueNotifyOutOfBandNV");
    table->CmdSetAttachmentFeedbackLoopEnableEXT =
        (PFN_vkCmdSetAttachmentFeedbackLoopEnableEXT)gpa(device, "vkCmdSetAttachmentFeedbackLoopEnableEXT");
#ifdef VK_USE_PLATFORM_SCREEN_QNX
    table->GetScreenBufferPropertiesQNX = (PFN_vkGetScreenBufferPropertiesQNX)gpa(device, "vkGetScreenBufferPropertiesQNX");
#endif  // VK_USE_PLATFORM_SCREEN_QNX
    table->CreateAccelerationStructureKHR = (PFN_vkCreateAccelerationStructureKHR)gpa(device, "vkCreateAccelerationStructureKHR");
    table->DestroyAccelerationStructureKHR =
        (PFN_vkDestroyAccelerationStructureKHR)gpa(device, "vkDestroyAccelerationStructureKHR");
    table->CmdBuildAccelerationStructuresKHR =
        (PFN_vkCmdBuildAccelerationStructuresKHR)gpa(device, "vkCmdBuildAccelerationStructuresKHR");
    table->CmdBuildAccelerationStructuresIndirectKHR =
        (PFN_vkCmdBuildAccelerationStructuresIndirectKHR)gpa(device, "vkCmdBuildAccelerationStructuresIndirectKHR");
    table->BuildAccelerationStructuresKHR = (PFN_vkBuildAccelerationStructuresKHR)gpa(device, "vkBuildAccelerationStructuresKHR");
    table->CopyAccelerationStructureKHR = (PFN_vkCopyAccelerationStructureKHR)gpa(device, "vkCopyAccelerationStructureKHR");
    table->CopyAccelerationStructureToMemoryKHR =
        (PFN_vkCopyAccelerationStructureToMemoryKHR)gpa(device, "vkCopyAccelerationStructureToMemoryKHR");
    table->CopyMemoryToAccelerationStructureKHR =
        (PFN_vkCopyMemoryToAccelerationStructureKHR)gpa(device, "vkCopyMemoryToAccelerationStructureKHR");
    table->WriteAccelerationStructuresPropertiesKHR =
        (PFN_vkWriteAccelerationStructuresPropertiesKHR)gpa(device, "vkWriteAccelerationStructuresPropertiesKHR");
    table->CmdCopyAccelerationStructureKHR =
        (PFN_vkCmdCopyAccelerationStructureKHR)gpa(device, "vkCmdCopyAccelerationStructureKHR");
    table->CmdCopyAccelerationStructureToMemoryKHR =
        (PFN_vkCmdCopyAccelerationStructureToMemoryKHR)gpa(device, "vkCmdCopyAccelerationStructureToMemoryKHR");
    table->CmdCopyMemoryToAccelerationStructureKHR =
        (PFN_vkCmdCopyMemoryToAccelerationStructureKHR)gpa(device, "vkCmdCopyMemoryToAccelerationStructureKHR");
    table->GetAccelerationStructureDeviceAddressKHR =
        (PFN_vkGetAccelerationStructureDeviceAddressKHR)gpa(device, "vkGetAccelerationStructureDeviceAddressKHR");
    table->CmdWriteAccelerationStructuresPropertiesKHR =
        (PFN_vkCmdWriteAccelerationStructuresPropertiesKHR)gpa(device, "vkCmdWriteAccelerationStructuresPropertiesKHR");
    table->GetDeviceAccelerationStructureCompatibilityKHR =
        (PFN_vkGetDeviceAccelerationStructureCompatibilityKHR)gpa(device, "vkGetDeviceAccelerationStructureCompatibilityKHR");
    table->GetAccelerationStructureBuildSizesKHR =
        (PFN_vkGetAccelerationStructureBuildSizesKHR)gpa(device, "vkGetAccelerationStructureBuildSizesKHR");
    table->CmdTraceRaysKHR = (PFN_vkCmdTraceRaysKHR)gpa(device, "vkCmdTraceRaysKHR");
    table->CreateRayTracingPipelinesKHR = (PFN_vkCreateRayTracingPipelinesKHR)gpa(device, "vkCreateRayTracingPipelinesKHR");
    table->GetRayTracingCaptureReplayShaderGroupHandlesKHR =
        (PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR)gpa(device, "vkGetRayTracingCaptureReplayShaderGroupHandlesKHR");
    table->CmdTraceRaysIndirectKHR = (PFN_vkCmdTraceRaysIndirectKHR)gpa(device, "vkCmdTraceRaysIndirectKHR");
    table->GetRayTracingShaderGroupStackSizeKHR =
        (PFN_vkGetRayTracingShaderGroupStackSizeKHR)gpa(device, "vkGetRayTracingShaderGroupStackSizeKHR");
    table->CmdSetRayTracingPipelineStackSizeKHR =
        (PFN_vkCmdSetRayTracingPipelineStackSizeKHR)gpa(device, "vkCmdSetRayTracingPipelineStackSizeKHR");
    table->CmdDrawMeshTasksEXT = (PFN_vkCmdDrawMeshTasksEXT)gpa(device, "vkCmdDrawMeshTasksEXT");
    table->CmdDrawMeshTasksIndirectEXT = (PFN_vkCmdDrawMeshTasksIndirectEXT)gpa(device, "vkCmdDrawMeshTasksIndirectEXT");
    table->CmdDrawMeshTasksIndirectCountEXT =
        (PFN_vkCmdDrawMeshTasksIndirectCountEXT)gpa(device, "vkCmdDrawMeshTasksIndirectCountEXT");
}

void layer_init_instance_dispatch_table(VkInstance instance, VkLayerInstanceDispatchTable* table, PFN_vkGetInstanceProcAddr gpa) {
    memset(table, 0, sizeof(*table));
    // Instance function pointers
    table->GetInstanceProcAddr = gpa;
    table->GetPhysicalDeviceProcAddr = (PFN_GetPhysicalDeviceProcAddr)gpa(instance, "vk_layerGetPhysicalDeviceProcAddr");
    table->DestroyInstance = (PFN_vkDestroyInstance)gpa(instance, "vkDestroyInstance");
    table->EnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)gpa(instance, "vkEnumeratePhysicalDevices");
    table->GetPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures)gpa(instance, "vkGetPhysicalDeviceFeatures");
    table->GetPhysicalDeviceFormatProperties =
        (PFN_vkGetPhysicalDeviceFormatProperties)gpa(instance, "vkGetPhysicalDeviceFormatProperties");
    table->GetPhysicalDeviceImageFormatProperties =
        (PFN_vkGetPhysicalDeviceImageFormatProperties)gpa(instance, "vkGetPhysicalDeviceImageFormatProperties");
    table->GetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)gpa(instance, "vkGetPhysicalDeviceProperties");
    table->GetPhysicalDeviceQueueFamilyProperties =
        (PFN_vkGetPhysicalDeviceQueueFamilyProperties)gpa(instance, "vkGetPhysicalDeviceQueueFamilyProperties");
    table->GetPhysicalDeviceMemoryProperties =
        (PFN_vkGetPhysicalDeviceMemoryProperties)gpa(instance, "vkGetPhysicalDeviceMemoryProperties");
    table->EnumerateDeviceExtensionProperties =
        (PFN_vkEnumerateDeviceExtensionProperties)gpa(instance, "vkEnumerateDeviceExtensionProperties");
    table->EnumerateDeviceLayerProperties = (PFN_vkEnumerateDeviceLayerProperties)gpa(instance, "vkEnumerateDeviceLayerProperties");
    table->GetPhysicalDeviceSparseImageFormatProperties =
        (PFN_vkGetPhysicalDeviceSparseImageFormatProperties)gpa(instance, "vkGetPhysicalDeviceSparseImageFormatProperties");
    table->EnumeratePhysicalDeviceGroups = (PFN_vkEnumeratePhysicalDeviceGroups)gpa(instance, "vkEnumeratePhysicalDeviceGroups");
    table->GetPhysicalDeviceFeatures2 = (PFN_vkGetPhysicalDeviceFeatures2)gpa(instance, "vkGetPhysicalDeviceFeatures2");
    table->GetPhysicalDeviceProperties2 = (PFN_vkGetPhysicalDeviceProperties2)gpa(instance, "vkGetPhysicalDeviceProperties2");
    table->GetPhysicalDeviceFormatProperties2 =
        (PFN_vkGetPhysicalDeviceFormatProperties2)gpa(instance, "vkGetPhysicalDeviceFormatProperties2");
    table->GetPhysicalDeviceImageFormatProperties2 =
        (PFN_vkGetPhysicalDeviceImageFormatProperties2)gpa(instance, "vkGetPhysicalDeviceImageFormatProperties2");
    table->GetPhysicalDeviceQueueFamilyProperties2 =
        (PFN_vkGetPhysicalDeviceQueueFamilyProperties2)gpa(instance, "vkGetPhysicalDeviceQueueFamilyProperties2");
    table->GetPhysicalDeviceMemoryProperties2 =
        (PFN_vkGetPhysicalDeviceMemoryProperties2)gpa(instance, "vkGetPhysicalDeviceMemoryProperties2");
    table->GetPhysicalDeviceSparseImageFormatProperties2 =
        (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2)gpa(instance, "vkGetPhysicalDeviceSparseImageFormatProperties2");
    table->GetPhysicalDeviceExternalBufferProperties =
        (PFN_vkGetPhysicalDeviceExternalBufferProperties)gpa(instance, "vkGetPhysicalDeviceExternalBufferProperties");
    table->GetPhysicalDeviceExternalFenceProperties =
        (PFN_vkGetPhysicalDeviceExternalFenceProperties)gpa(instance, "vkGetPhysicalDeviceExternalFenceProperties");
    table->GetPhysicalDeviceExternalSemaphoreProperties =
        (PFN_vkGetPhysicalDeviceExternalSemaphoreProperties)gpa(instance, "vkGetPhysicalDeviceExternalSemaphoreProperties");
    table->GetPhysicalDeviceToolProperties =
        (PFN_vkGetPhysicalDeviceToolProperties)gpa(instance, "vkGetPhysicalDeviceToolProperties");
    table->DestroySurfaceKHR = (PFN_vkDestroySurfaceKHR)gpa(instance, "vkDestroySurfaceKHR");
    table->GetPhysicalDeviceSurfaceSupportKHR =
        (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)gpa(instance, "vkGetPhysicalDeviceSurfaceSupportKHR");
    table->GetPhysicalDeviceSurfaceCapabilitiesKHR =
        (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)gpa(instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    table->GetPhysicalDeviceSurfaceFormatsKHR =
        (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)gpa(instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
    table->GetPhysicalDeviceSurfacePresentModesKHR =
        (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)gpa(instance, "vkGetPhysicalDeviceSurfacePresentModesKHR");
    table->GetPhysicalDevicePresentRectanglesKHR =
        (PFN_vkGetPhysicalDevicePresentRectanglesKHR)gpa(instance, "vkGetPhysicalDevicePresentRectanglesKHR");
    table->GetPhysicalDeviceDisplayPropertiesKHR =
        (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR)gpa(instance, "vkGetPhysicalDeviceDisplayPropertiesKHR");
    table->GetPhysicalDeviceDisplayPlanePropertiesKHR =
        (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR)gpa(instance, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
    table->GetDisplayPlaneSupportedDisplaysKHR =
        (PFN_vkGetDisplayPlaneSupportedDisplaysKHR)gpa(instance, "vkGetDisplayPlaneSupportedDisplaysKHR");
    table->GetDisplayModePropertiesKHR = (PFN_vkGetDisplayModePropertiesKHR)gpa(instance, "vkGetDisplayModePropertiesKHR");
    table->CreateDisplayModeKHR = (PFN_vkCreateDisplayModeKHR)gpa(instance, "vkCreateDisplayModeKHR");
    table->GetDisplayPlaneCapabilitiesKHR = (PFN_vkGetDisplayPlaneCapabilitiesKHR)gpa(instance, "vkGetDisplayPlaneCapabilitiesKHR");
    table->CreateDisplayPlaneSurfaceKHR = (PFN_vkCreateDisplayPlaneSurfaceKHR)gpa(instance, "vkCreateDisplayPlaneSurfaceKHR");
#ifdef VK_USE_PLATFORM_XLIB_KHR
    table->CreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR)gpa(instance, "vkCreateXlibSurfaceKHR");
    table->GetPhysicalDeviceXlibPresentationSupportKHR =
        (PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR)gpa(instance, "vkGetPhysicalDeviceXlibPresentationSupportKHR");
#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
    table->CreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR)gpa(instance, "vkCreateXcbSurfaceKHR");
    table->GetPhysicalDeviceXcbPresentationSupportKHR =
        (PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR)gpa(instance, "vkGetPhysicalDeviceXcbPresentationSupportKHR");
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    table->CreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR)gpa(instance, "vkCreateWaylandSurfaceKHR");
    table->GetPhysicalDeviceWaylandPresentationSupportKHR =
        (PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)gpa(instance, "vkGetPhysicalDeviceWaylandPresentationSupportKHR");
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    table->CreateAndroidSurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR)gpa(instance, "vkCreateAndroidSurfaceKHR");
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->CreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)gpa(instance, "vkCreateWin32SurfaceKHR");
    table->GetPhysicalDeviceWin32PresentationSupportKHR =
        (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)gpa(instance, "vkGetPhysicalDeviceWin32PresentationSupportKHR");
#endif  // VK_USE_PLATFORM_WIN32_KHR
    table->GetPhysicalDeviceVideoCapabilitiesKHR =
        (PFN_vkGetPhysicalDeviceVideoCapabilitiesKHR)gpa(instance, "vkGetPhysicalDeviceVideoCapabilitiesKHR");
    table->GetPhysicalDeviceVideoFormatPropertiesKHR =
        (PFN_vkGetPhysicalDeviceVideoFormatPropertiesKHR)gpa(instance, "vkGetPhysicalDeviceVideoFormatPropertiesKHR");
    table->GetPhysicalDeviceFeatures2KHR = (PFN_vkGetPhysicalDeviceFeatures2KHR)gpa(instance, "vkGetPhysicalDeviceFeatures2KHR");
    table->GetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)gpa(instance, "vkGetPhysicalDeviceProperties2KHR");
    table->GetPhysicalDeviceFormatProperties2KHR =
        (PFN_vkGetPhysicalDeviceFormatProperties2KHR)gpa(instance, "vkGetPhysicalDeviceFormatProperties2KHR");
    table->GetPhysicalDeviceImageFormatProperties2KHR =
        (PFN_vkGetPhysicalDeviceImageFormatProperties2KHR)gpa(instance, "vkGetPhysicalDeviceImageFormatProperties2KHR");
    table->GetPhysicalDeviceQueueFamilyProperties2KHR =
        (PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR)gpa(instance, "vkGetPhysicalDeviceQueueFamilyProperties2KHR");
    table->GetPhysicalDeviceMemoryProperties2KHR =
        (PFN_vkGetPhysicalDeviceMemoryProperties2KHR)gpa(instance, "vkGetPhysicalDeviceMemoryProperties2KHR");
    table->GetPhysicalDeviceSparseImageFormatProperties2KHR =
        (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR)gpa(instance, "vkGetPhysicalDeviceSparseImageFormatProperties2KHR");
    table->EnumeratePhysicalDeviceGroupsKHR =
        (PFN_vkEnumeratePhysicalDeviceGroupsKHR)gpa(instance, "vkEnumeratePhysicalDeviceGroupsKHR");
    table->GetPhysicalDeviceExternalBufferPropertiesKHR =
        (PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR)gpa(instance, "vkGetPhysicalDeviceExternalBufferPropertiesKHR");
    table->GetPhysicalDeviceExternalSemaphorePropertiesKHR =
        (PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR)gpa(instance, "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");
    table->GetPhysicalDeviceExternalFencePropertiesKHR =
        (PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR)gpa(instance, "vkGetPhysicalDeviceExternalFencePropertiesKHR");
    table->EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR =
        (PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR)gpa(
            instance, "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
    table->GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR = (PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR)gpa(
        instance, "vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR");
    table->GetPhysicalDeviceSurfaceCapabilities2KHR =
        (PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR)gpa(instance, "vkGetPhysicalDeviceSurfaceCapabilities2KHR");
    table->GetPhysicalDeviceSurfaceFormats2KHR =
        (PFN_vkGetPhysicalDeviceSurfaceFormats2KHR)gpa(instance, "vkGetPhysicalDeviceSurfaceFormats2KHR");
    table->GetPhysicalDeviceDisplayProperties2KHR =
        (PFN_vkGetPhysicalDeviceDisplayProperties2KHR)gpa(instance, "vkGetPhysicalDeviceDisplayProperties2KHR");
    table->GetPhysicalDeviceDisplayPlaneProperties2KHR =
        (PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR)gpa(instance, "vkGetPhysicalDeviceDisplayPlaneProperties2KHR");
    table->GetDisplayModeProperties2KHR = (PFN_vkGetDisplayModeProperties2KHR)gpa(instance, "vkGetDisplayModeProperties2KHR");
    table->GetDisplayPlaneCapabilities2KHR =
        (PFN_vkGetDisplayPlaneCapabilities2KHR)gpa(instance, "vkGetDisplayPlaneCapabilities2KHR");
    table->GetPhysicalDeviceFragmentShadingRatesKHR =
        (PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR)gpa(instance, "vkGetPhysicalDeviceFragmentShadingRatesKHR");
    table->GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR = (PFN_vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR)gpa(
        instance, "vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR");
    table->GetPhysicalDeviceCooperativeMatrixPropertiesKHR =
        (PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR)gpa(instance, "vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR");
    table->GetPhysicalDeviceCalibrateableTimeDomainsKHR =
        (PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsKHR)gpa(instance, "vkGetPhysicalDeviceCalibrateableTimeDomainsKHR");
    table->CreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)gpa(instance, "vkCreateDebugReportCallbackEXT");
    table->DestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)gpa(instance, "vkDestroyDebugReportCallbackEXT");
    table->DebugReportMessageEXT = (PFN_vkDebugReportMessageEXT)gpa(instance, "vkDebugReportMessageEXT");
#ifdef VK_USE_PLATFORM_GGP
    table->CreateStreamDescriptorSurfaceGGP =
        (PFN_vkCreateStreamDescriptorSurfaceGGP)gpa(instance, "vkCreateStreamDescriptorSurfaceGGP");
#endif  // VK_USE_PLATFORM_GGP
    table->GetPhysicalDeviceExternalImageFormatPropertiesNV =
        (PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV)gpa(instance, "vkGetPhysicalDeviceExternalImageFormatPropertiesNV");
#ifdef VK_USE_PLATFORM_VI_NN
    table->CreateViSurfaceNN = (PFN_vkCreateViSurfaceNN)gpa(instance, "vkCreateViSurfaceNN");
#endif  // VK_USE_PLATFORM_VI_NN
    table->ReleaseDisplayEXT = (PFN_vkReleaseDisplayEXT)gpa(instance, "vkReleaseDisplayEXT");
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
    table->AcquireXlibDisplayEXT = (PFN_vkAcquireXlibDisplayEXT)gpa(instance, "vkAcquireXlibDisplayEXT");
    table->GetRandROutputDisplayEXT = (PFN_vkGetRandROutputDisplayEXT)gpa(instance, "vkGetRandROutputDisplayEXT");
#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT
    table->GetPhysicalDeviceSurfaceCapabilities2EXT =
        (PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT)gpa(instance, "vkGetPhysicalDeviceSurfaceCapabilities2EXT");
#ifdef VK_USE_PLATFORM_IOS_MVK
    table->CreateIOSSurfaceMVK = (PFN_vkCreateIOSSurfaceMVK)gpa(instance, "vkCreateIOSSurfaceMVK");
#endif  // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
    table->CreateMacOSSurfaceMVK = (PFN_vkCreateMacOSSurfaceMVK)gpa(instance, "vkCreateMacOSSurfaceMVK");
#endif  // VK_USE_PLATFORM_MACOS_MVK
    table->CreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)gpa(instance, "vkCreateDebugUtilsMessengerEXT");
    if (table->CreateDebugUtilsMessengerEXT == nullptr) {
        table->CreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)StubCreateDebugUtilsMessengerEXT;
    }
    table->DestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)gpa(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (table->DestroyDebugUtilsMessengerEXT == nullptr) {
        table->DestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)StubDestroyDebugUtilsMessengerEXT;
    }
    table->SubmitDebugUtilsMessageEXT = (PFN_vkSubmitDebugUtilsMessageEXT)gpa(instance, "vkSubmitDebugUtilsMessageEXT");
    if (table->SubmitDebugUtilsMessageEXT == nullptr) {
        table->SubmitDebugUtilsMessageEXT = (PFN_vkSubmitDebugUtilsMessageEXT)StubSubmitDebugUtilsMessageEXT;
    }
    table->GetPhysicalDeviceMultisamplePropertiesEXT =
        (PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT)gpa(instance, "vkGetPhysicalDeviceMultisamplePropertiesEXT");
    table->GetPhysicalDeviceCalibrateableTimeDomainsEXT =
        (PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT)gpa(instance, "vkGetPhysicalDeviceCalibrateableTimeDomainsEXT");
#ifdef VK_USE_PLATFORM_FUCHSIA
    table->CreateImagePipeSurfaceFUCHSIA = (PFN_vkCreateImagePipeSurfaceFUCHSIA)gpa(instance, "vkCreateImagePipeSurfaceFUCHSIA");
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_METAL_EXT
    table->CreateMetalSurfaceEXT = (PFN_vkCreateMetalSurfaceEXT)gpa(instance, "vkCreateMetalSurfaceEXT");
#endif  // VK_USE_PLATFORM_METAL_EXT
    table->GetPhysicalDeviceToolPropertiesEXT =
        (PFN_vkGetPhysicalDeviceToolPropertiesEXT)gpa(instance, "vkGetPhysicalDeviceToolPropertiesEXT");
    table->GetPhysicalDeviceCooperativeMatrixPropertiesNV =
        (PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesNV)gpa(instance, "vkGetPhysicalDeviceCooperativeMatrixPropertiesNV");
    table->GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV =
        (PFN_vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV)gpa(
            instance, "vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV");
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->GetPhysicalDeviceSurfacePresentModes2EXT =
        (PFN_vkGetPhysicalDeviceSurfacePresentModes2EXT)gpa(instance, "vkGetPhysicalDeviceSurfacePresentModes2EXT");
#endif  // VK_USE_PLATFORM_WIN32_KHR
    table->CreateHeadlessSurfaceEXT = (PFN_vkCreateHeadlessSurfaceEXT)gpa(instance, "vkCreateHeadlessSurfaceEXT");
    table->AcquireDrmDisplayEXT = (PFN_vkAcquireDrmDisplayEXT)gpa(instance, "vkAcquireDrmDisplayEXT");
    table->GetDrmDisplayEXT = (PFN_vkGetDrmDisplayEXT)gpa(instance, "vkGetDrmDisplayEXT");
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->AcquireWinrtDisplayNV = (PFN_vkAcquireWinrtDisplayNV)gpa(instance, "vkAcquireWinrtDisplayNV");
    table->GetWinrtDisplayNV = (PFN_vkGetWinrtDisplayNV)gpa(instance, "vkGetWinrtDisplayNV");
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
    table->CreateDirectFBSurfaceEXT = (PFN_vkCreateDirectFBSurfaceEXT)gpa(instance, "vkCreateDirectFBSurfaceEXT");
    table->GetPhysicalDeviceDirectFBPresentationSupportEXT =
        (PFN_vkGetPhysicalDeviceDirectFBPresentationSupportEXT)gpa(instance, "vkGetPhysicalDeviceDirectFBPresentationSupportEXT");
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT
#ifdef VK_USE_PLATFORM_SCREEN_QNX
    table->CreateScreenSurfaceQNX = (PFN_vkCreateScreenSurfaceQNX)gpa(instance, "vkCreateScreenSurfaceQNX");
    table->GetPhysicalDeviceScreenPresentationSupportQNX =
        (PFN_vkGetPhysicalDeviceScreenPresentationSupportQNX)gpa(instance, "vkGetPhysicalDeviceScreenPresentationSupportQNX");
#endif  // VK_USE_PLATFORM_SCREEN_QNX
    table->GetPhysicalDeviceOpticalFlowImageFormatsNV =
        (PFN_vkGetPhysicalDeviceOpticalFlowImageFormatsNV)gpa(instance, "vkGetPhysicalDeviceOpticalFlowImageFormatsNV");
}

// NOLINTEND
