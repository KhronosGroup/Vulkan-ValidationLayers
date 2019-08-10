/* Copyright (c) 2015-2019 The Khronos Group Inc.
 * Copyright (c) 2015-2019 Valve Corporation
 * Copyright (c) 2015-2019 LunarG, Inc.
 * Copyright (C) 2015-2019 Google Inc.
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
 *
 * Author: Courtney Goeltzenleuchter <courtneygo@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 */
#ifndef CORE_VALIDATION_ERROR_ENUMS_H_
#define CORE_VALIDATION_ERROR_ENUMS_H_

// Suppress unused warning on Linux
#if defined(__GNUC__)
#define DECORATE_UNUSED __attribute__((unused))
#else
#define DECORATE_UNUSED
#endif

// clang-format off

static const char DECORATE_UNUSED *kVUID_Core_MemTrack_FenceState = "UNASSIGNED-CoreValidation-MemTrack-FenceState";
static const char DECORATE_UNUSED *kVUID_Core_MemTrack_FreedMemRef = "UNASSIGNED-CoreValidation-MemTrack-FreedMemRef";
static const char DECORATE_UNUSED *kVUID_Core_MemTrack_InvalidMap = "UNASSIGNED-CoreValidation-MemTrack-InvalidMap";
static const char DECORATE_UNUSED *kVUID_Core_MemTrack_InvalidState = "UNASSIGNED-CoreValidation-MemTrack-InvalidState";
static const char DECORATE_UNUSED *kVUID_Core_MemTrack_InvalidUsageFlag = "UNASSIGNED-CoreValidation-MemTrack-InvalidUsageFlag";
static const char DECORATE_UNUSED *kVUID_Core_MemTrack_RebindObject = "UNASSIGNED-CoreValidation-MemTrack-RebindObject";
// Previously defined but unused - uncomment as needed
//static const char DECORATE_UNUSED *kVUID_Core_MemTrack_InternalError = "UNASSIGNED-CoreValidation-MemTrack-InternalError";
//static const char DECORATE_UNUSED *kVUID_Core_MemTrack_InvalidCB = "UNASSIGNED-CoreValidation-MemTrack-InvalidCB";
//static const char DECORATE_UNUSED *kVUID_Core_MemTrack_InvalidMemObj = "UNASSIGNED-CoreValidation-MemTrack-InvalidMemObj";
//static const char DECORATE_UNUSED *kVUID_Core_MemTrack_InvalidMemRegion = "UNASSIGNED-CoreValidation-MemTrack-InvalidMemRegion";
//static const char DECORATE_UNUSED *kVUID_Core_MemTrack_InvalidMemType = "UNASSIGNED-CoreValidation-MemTrack-InvalidMemType";
//static const char DECORATE_UNUSED *kVUID_Core_MemTrack_InvalidObject = "UNASSIGNED-CoreValidation-MemTrack-InvalidObject";
//static const char DECORATE_UNUSED *kVUID_Core_MemTrack_MemoryLeak = "UNASSIGNED-CoreValidation-MemTrack-MemoryLeak";
//static const char DECORATE_UNUSED *kVUID_Core_MemTrack_ObjNotBound = "UNASSIGNED-CoreValidation-MemTrack-ObjNotBound";
//static const char DECORATE_UNUSED *kVUID_Core_MemTrack_ResetCBWhileInFlight = "UNASSIGNED-CoreValidation-MemTrack-ResetCBWhileInFlight";
//static const char DECORATE_UNUSED *kVUID_Core_MemTrack_InvalidAliasing = "UNASSIGNED-CoreValidation-MemTrack-InvalidAliasing";

static const char DECORATE_UNUSED *kVUID_Core_DrawState_ClearCmdBeforeDraw = "UNASSIGNED-CoreValidation-DrawState-ClearCmdBeforeDraw";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_CommandBufferSingleSubmitViolation = "UNASSIGNED-CoreValidation-DrawState-CommandBufferSingleSubmitViolation";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_DescriptorSetNotBound = "UNASSIGNED-CoreValidation-DrawState-DescriptorSetNotBound";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_DescriptorSetNotUpdated = "UNASSIGNED-CoreValidation-DrawState-DescriptorSetNotUpdated";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_DoubleDestroy = "UNASSIGNED-CoreValidation-DrawState-DoubleDestroy";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_ExtensionNotEnabled = "UNASSIGNED-CoreValidation-DrawState-ExtensionNotEnabled";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InternalError = "UNASSIGNED-CoreValidation-DrawState-InternalError";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidBarrier = "UNASSIGNED-CoreValidation-DrawState-InvalidBarrier";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidBuffer = "UNASSIGNED-CoreValidation-DrawState-InvalidBuffer";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidCommandBuffer = "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidCommandBufferSimultaneousUse = "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBufferSimultaneousUse";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidDescriptorSet = "UNASSIGNED-CoreValidation-DrawState-InvalidDescriptorSet";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidEvent = "UNASSIGNED-CoreValidation-DrawState-InvalidEvent";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidExtents = "UNASSIGNED-CoreValidation-DrawState-InvalidExtents";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidFeature = "UNASSIGNED-CoreValidation-DrawState-InvalidFeature";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidFence = "UNASSIGNED-CoreValidation-DrawState-InvalidFence";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidImage = "UNASSIGNED-CoreValidation-DrawState-InvalidImage";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidImageAspect = "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidImageLayout = "UNASSIGNED-CoreValidation-DrawState-InvalidImageLayout";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidLayout = "UNASSIGNED-CoreValidation-DrawState-InvalidLayout";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidPipeline = "UNASSIGNED-CoreValidation-DrawState-InvalidPipeline";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidPipelineCreateState = "UNASSIGNED-CoreValidation-DrawState-InvalidPipelineCreateState";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidQuery = "UNASSIGNED-CoreValidation-DrawState-InvalidQuery";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_QueryNotReset = "UNASSIGNED-CoreValidation-DrawState-QueryNotReset";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidQueueFamily = "UNASSIGNED-CoreValidation-DrawState-InvalidQueueFamily";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidRenderArea = "UNASSIGNED-CoreValidation-DrawState-InvalidRenderArea";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidRenderpass = "UNASSIGNED-CoreValidation-DrawState-InvalidRenderpass";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidSecondaryCommandBuffer = "UNASSIGNED-CoreValidation-DrawState-InvalidSecondaryCommandBuffer";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidSet = "UNASSIGNED-CoreValidation-DrawState-InvalidSet";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_MismatchedImageFormat = "UNASSIGNED-CoreValidation-DrawState-MismatchedImageFormat";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_MismatchedImageType = "UNASSIGNED-CoreValidation-DrawState-MismatchedImageType";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_MissingAttachmentReference = "UNASSIGNED-CoreValidation-DrawState-MissingAttachmentReference";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_NoActiveRenderpass = "UNASSIGNED-CoreValidation-DrawState-NoActiveRenderpass";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_NoEndCommandBuffer = "UNASSIGNED-CoreValidation-DrawState-NoEndCommandBuffer";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_NumSamplesMismatch = "UNASSIGNED-CoreValidation-DrawState-NumSamplesMismatch";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_OutOfMemory = "UNASSIGNED-CoreValidation-DrawState-OutOfMemory";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_PipelineLayoutsIncompatible = "UNASSIGNED-CoreValidation-DrawState-PipelineLayoutsIncompatible";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_QueueForwardProgress = "UNASSIGNED-CoreValidation-DrawState-QueueForwardProgress";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_SwapchainCreateBeforeQuery = "UNASSIGNED-CoreValidation-DrawState-SwapchainCreateBeforeQuery";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_SwapchainImageNotAcquired = "UNASSIGNED-CoreValidation-DrawState-SwapchainImageNotAcquired";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_SwapchainImagesNotFound = "UNASSIGNED-CoreValidation-DrawState-SwapchainImagesNotFound";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_SwapchainInvalidImage = "UNASSIGNED-CoreValidation-DrawState-SwapchainInvalidImage";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_SwapchainNoSyncForAcquire = "UNASSIGNED-CoreValidation-DrawState-SwapchainNoSyncForAcquire";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_SwapchainReplaced = "UNASSIGNED-CoreValidation-DrawState-SwapchainReplaced";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_SwapchainTooManyImages = "UNASSIGNED-CoreValidation-DrawState-SwapchainTooManyImages";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_SwapchainUnsupportedQueue = "UNASSIGNED-CoreValidation-DrawState-SwapchainUnsupportedQueue";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_ViewportScissorMismatch = "UNASSIGNED-CoreValidation-DrawState-ViewportScissorMismatch";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_VtxIndexOutOfBounds = "UNASSIGNED-CoreValidation-DrawState-VtxIndexOutOfBounds";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidVtxAttributeAlignment = "UNASSIGNED-CoreValidation-DrawState-InvalidVtxAttributeAlignment";
static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidImageView = "UNASSIGNED-CoreValidation-DrawState-InvalidImageView";
// Previously defined but unused - uncomment as needed
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_BeginCommandBufferInvalidState = "UNASSIGNED-CoreValidation-DrawState-BeginCommandBufferInvalidState";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_BlendNotBound = "UNASSIGNED-CoreValidation-DrawState-BlendNotBound";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_BufferinfoDescriptorError = "UNASSIGNED-CoreValidation-DrawState-BufferinfoDescriptorError";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_BufferviewDescriptorError = "UNASSIGNED-CoreValidation-DrawState-BufferviewDescriptorError";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_CantFreeFromNonFreePool = "UNASSIGNED-CoreValidation-DrawState-CantFreeFromNonFreePool";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_DepthBiasNotBound = "UNASSIGNED-CoreValidation-DrawState-DepthBiasNotBound";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_DepthBoundsNotBound = "UNASSIGNED-CoreValidation-DrawState-DepthBoundsNotBound";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_DescriptorPoolEmpty = "UNASSIGNED-CoreValidation-DrawState-DescriptorPoolEmpty";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_DescriptorStageflagsMismatch = "UNASSIGNED-CoreValidation-DrawState-DescriptorStageflagsMismatch";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_DescriptorTypeMismatch = "UNASSIGNED-CoreValidation-DrawState-DescriptorTypeMismatch";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_DescriptorUpdateOutOfBounds = "UNASSIGNED-CoreValidation-DrawState-DescriptorUpdateOutOfBounds";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_DisabledLogicOp = "UNASSIGNED-CoreValidation-DrawState-DisabledLogicOp";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_DynamicOffsetOverflow = "UNASSIGNED-CoreValidation-DrawState-DynamicOffsetOverflow";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_FramebufferIncompatible = "UNASSIGNED-CoreValidation-DrawState-FramebufferIncompatible";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_ImageTransferGranularity = "UNASSIGNED-CoreValidation-DrawState-ImageTransferGranularity";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_ImageviewDescriptorError = "UNASSIGNED-CoreValidation-DrawState-ImageviewDescriptorError";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_InconsistentImmutableSamplerUpdate = "UNASSIGNED-CoreValidation-DrawState-InconsistentImmutableSamplerUpdate";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_IndependentBlend = "UNASSIGNED-CoreValidation-DrawState-IndependentBlend";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_IndexBufferNotBound = "UNASSIGNED-CoreValidation-DrawState-IndexBufferNotBound";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidAttachmentIndex = "UNASSIGNED-CoreValidation-DrawState-InvalidAttachmentIndex";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidBufferMemoryOffset = "UNASSIGNED-CoreValidation-DrawState-InvalidBufferMemoryOffset";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidBufferView = "UNASSIGNED-CoreValidation-DrawState-InvalidBufferView";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidCommandBufferReset = "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBufferReset";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidCommandPool = "UNASSIGNED-CoreValidation-DrawState-InvalidCommandPool";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidCopyUpdate = "UNASSIGNED-CoreValidation-DrawState-InvalidCopyUpdate";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidDescriptorPool = "UNASSIGNED-CoreValidation-DrawState-InvalidDescriptorPool";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidDeviceMemory = "UNASSIGNED-CoreValidation-DrawState-InvalidDeviceMemory";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidFramebuffer = "UNASSIGNED-CoreValidation-DrawState-InvalidFramebuffer";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidFramebufferCreateInfo = "UNASSIGNED-CoreValidation-DrawState-InvalidFramebufferCreateInfo";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidImageFilter = "UNASSIGNED-CoreValidation-DrawState-InvalidImageFilter";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidImageSubrange = "UNASSIGNED-CoreValidation-DrawState-InvalidImageSubrange";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidImageUsage = "UNASSIGNED-CoreValidation-DrawState-InvalidImageUsage";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidImageView = "UNASSIGNED-CoreValidation-DrawState-InvalidImageView";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidQueryPool = "UNASSIGNED-CoreValidation-DrawState-InvalidQueryPool";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidQueueIndex = "UNASSIGNED-CoreValidation-DrawState-InvalidQueueIndex";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidRenderpassCmd = "UNASSIGNED-CoreValidation-DrawState-InvalidRenderpassCmd";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidSampler = "UNASSIGNED-CoreValidation-DrawState-InvalidSampler";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidStorageBufferOffset = "UNASSIGNED-CoreValidation-DrawState-InvalidStorageBufferOffset";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidSubpassIndex = "UNASSIGNED-CoreValidation-DrawState-InvalidSubpassIndex";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidTexelBufferOffset = "UNASSIGNED-CoreValidation-DrawState-InvalidTexelBufferOffset";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidUniformBufferOffset = "UNASSIGNED-CoreValidation-DrawState-InvalidUniformBufferOffset";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidUpdateStruct = "UNASSIGNED-CoreValidation-DrawState-InvalidUpdateStruct";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_InvalidWriteUpdate = "UNASSIGNED-CoreValidation-DrawState-InvalidWriteUpdate";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_LineWidthNotBound = "UNASSIGNED-CoreValidation-DrawState-LineWidthNotBound";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_MismatchedImageAspect = "UNASSIGNED-CoreValidation-DrawState-MismatchedImageAspect";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_NoBeginCommandBuffer = "UNASSIGNED-CoreValidation-DrawState-NoBeginCommandBuffer";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_ObjectInUse = "UNASSIGNED-CoreValidation-DrawState-ObjectInUse";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_PushConstantsError = "UNASSIGNED-CoreValidation-DrawState-PushConstantsError";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_RenderpassIncompatible = "UNASSIGNED-CoreValidation-DrawState-RenderpassIncompatible";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_RenderpassTooManyClearValues = "UNASSIGNED-CoreValidation-DrawState-RenderpassTooManyClearValues";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_SamplerDescriptorError = "UNASSIGNED-CoreValidation-DrawState-SamplerDescriptorError";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_ScissorNotBound = "UNASSIGNED-CoreValidation-DrawState-ScissorNotBound";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_StencilNotBound = "UNASSIGNED-CoreValidation-DrawState-StencilNotBound";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_SwapchainBadCompositeAlpha = "UNASSIGNED-CoreValidation-DrawState-SwapchainBadCompositeAlpha";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_SwapchainBadExtents = "UNASSIGNED-CoreValidation-DrawState-SwapchainBadExtents";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_SwapchainBadFormat = "UNASSIGNED-CoreValidation-DrawState-SwapchainBadFormat";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_SwapchainBadImageCount = "UNASSIGNED-CoreValidation-DrawState-SwapchainBadImageCount";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_SwapchainBadLayerCount = "UNASSIGNED-CoreValidation-DrawState-SwapchainBadLayerCount";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_SwapchainBadPreTransform = "UNASSIGNED-CoreValidation-DrawState-SwapchainBadPreTransform";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_SwapchainBadPresentMode = "UNASSIGNED-CoreValidation-DrawState-SwapchainBadPresentMode";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_SwapchainBadUsageFlags = "UNASSIGNED-CoreValidation-DrawState-SwapchainBadUsageFlags";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_ViewportNotBound = "UNASSIGNED-CoreValidation-DrawState-ViewportNotBound";
//static const char DECORATE_UNUSED *kVUID_Core_DrawState_VtxIndexAlignmentError = "UNASSIGNED-CoreValidation-DrawState-VtxIndexAlignmentError";

static const char DECORATE_UNUSED *kVUID_Core_Shader_DescriptorNotAccessibleFromStage = "UNASSIGNED-CoreValidation-Shader-DescriptorNotAccessibleFromStage";
static const char DECORATE_UNUSED *kVUID_Core_Shader_DescriptorTypeMismatch = "UNASSIGNED-CoreValidation-Shader-DescriptorTypeMismatch";
static const char DECORATE_UNUSED *kVUID_Core_Shader_ExceedDeviceLimit = "UNASSIGNED-CoreValidation-Shader-ExceedDeviceLimit";
static const char DECORATE_UNUSED *kVUID_Core_Shader_FeatureNotEnabled = "UNASSIGNED-CoreValidation-Shader-FeatureNotEnabled";
static const char DECORATE_UNUSED *kVUID_Core_Shader_InconsistentSpirv = "UNASSIGNED-CoreValidation-Shader-InconsistentSpirv";
static const char DECORATE_UNUSED *kVUID_Core_Shader_InconsistentVi = "UNASSIGNED-CoreValidation-Shader-InconsistentVi";
static const char DECORATE_UNUSED *kVUID_Core_Shader_InputAttachmentTypeMismatch = "UNASSIGNED-CoreValidation-Shader-InputAttachmentTypeMismatch";
static const char DECORATE_UNUSED *kVUID_Core_Shader_InputNotProduced = "UNASSIGNED-CoreValidation-Shader-InputNotProduced";
static const char DECORATE_UNUSED *kVUID_Core_Shader_InterfaceTypeMismatch = "UNASSIGNED-CoreValidation-Shader-InterfaceTypeMismatch";
static const char DECORATE_UNUSED *kVUID_Core_Shader_MissingDescriptor = "UNASSIGNED-CoreValidation-Shader-MissingDescriptor";
static const char DECORATE_UNUSED *kVUID_Core_Shader_MissingInputAttachment = "UNASSIGNED-CoreValidation-Shader-MissingInputAttachment";
static const char DECORATE_UNUSED *kVUID_Core_Shader_OutputNotConsumed = "UNASSIGNED-CoreValidation-Shader-OutputNotConsumed";
static const char DECORATE_UNUSED *kVUID_Core_Shader_PushConstantNotAccessibleFromStage = "UNASSIGNED-CoreValidation-Shader-PushConstantNotAccessibleFromStage";
static const char DECORATE_UNUSED *kVUID_Core_Shader_PushConstantOutOfRange = "UNASSIGNED-CoreValidation-Shader-PushConstantOutOfRange";
static const char DECORATE_UNUSED *kVUID_Core_Shader_MissingPointSizeBuiltIn = "UNASSIGNED-CoreValidation-Shader-PointSizeMissing";
static const char DECORATE_UNUSED *kVUID_Core_Shader_PointSizeBuiltInOverSpecified = "UNASSIGNED-CoreValidation-Shader-PointSizeOverSpecified";
static const char DECORATE_UNUSED *kVUID_Core_Shader_NoAlphaAtLocation0WithAlphaToCoverage = "UNASSIGNED-CoreValidation-Shader-NoAlphaAtLocation0WithAlphaToCoverage";
static const char DECORATE_UNUSED *kVUID_Core_Shader_CooperativeMatrixSupportedStages = "UNASSIGNED-CoreValidation-Shader-CooperativeMatrixSupportedStages";
static const char DECORATE_UNUSED *kVUID_Core_Shader_CooperativeMatrixType = "UNASSIGNED-CoreValidation-Shader-CooperativeMatrixType";
static const char DECORATE_UNUSED *kVUID_Core_Shader_CooperativeMatrixMulAdd = "UNASSIGNED-CoreValidation-Shader-CooperativeMatrixMulAdd";
// Previously defined but unused - uncomment as needed
//static const char DECORATE_UNUSED *kVUID_Core_Shader_BadCapability = "UNASSIGNED-CoreValidation-Shader-BadCapability";
//static const char DECORATE_UNUSED *kVUID_Core_Shader_BadSpecialization = "UNASSIGNED-CoreValidation-Shader-BadSpecialization";
//static const char DECORATE_UNUSED *kVUID_Core_Shader_MissingEntrypoint = "UNASSIGNED-CoreValidation-Shader-MissingEntrypoint";
//static const char DECORATE_UNUSED *kVUID_Core_Shader_NonSpirvShader = "UNASSIGNED-CoreValidation-Shader-NonSpirvShader";
//static const char DECORATE_UNUSED *kVUID_Core_Shader_UnknownStage = "UNASSIGNED-CoreValidation-Shader-UnknownStage";

static const char DECORATE_UNUSED *kVUID_Core_DevLimit_CountMismatch = "UNASSIGNED-CoreValidation-DevLimitCountMismatch";
static const char DECORATE_UNUSED *kVUID_Core_DevLimit_InvalidFeatureRequested = "UNASSIGNED-CoreValidation-DevLimit-InvalidFeatureRequested";
static const char DECORATE_UNUSED *kVUID_Core_DevLimit_InvalidInstance = "UNASSIGNED-CoreValidation-DevLimit-InvalidInstance";
static const char DECORATE_UNUSED *kVUID_Core_DevLimit_MissingQueryCount = "UNASSIGNED-CoreValidation-DevLimit-MissingQueryCount";
static const char DECORATE_UNUSED *kVUID_Core_DevLimit_MustQueryCount = "UNASSIGNED-CoreValidation-DevLimit-MustQueryCount";
// Previously defined but unused - uncomment as needed
//static const char DECORATE_UNUSED *kVUID_Core_DevLimit_InvalidPhysicalDevice = "UNASSIGNED-CoreValidation-DevLimit-InvalidPhysicalDevice";

static const char DECORATE_UNUSED *kVUID_Core_Swapchain_GetSupportedDisplaysWithoutQuery = "UNASSIGNED-CoreValidation-Swapchain-GetSupportedDisplaysWithoutQuery";
static const char DECORATE_UNUSED *kVUID_Core_Swapchain_InvalidCount = "UNASSIGNED-CoreValidation-SwapchainInvalidCount";
static const char DECORATE_UNUSED *kVUID_Core_Swapchain_PriorCount = "UNASSIGNED-CoreValidation-SwapchainPriorCount";
static const char DECORATE_UNUSED *kVUID_Core_Swapchain_PreTransform = "UNASSIGNED-CoreValidation-SwapchainPreTransform";

// Previously defined but unused - uncomment as needed
//static const char DECORATE_UNUSED *kVUID_Core_Swapchain_BadBool = "UNASSIGNED-CoreValidation-SwapchainBadBool";
//static const char DECORATE_UNUSED *kVUID_Core_Swapchain_CreateSwapBadCompositeAlpha = "UNASSIGNED-CoreValidation-Swapchain-CreateSwapBadCompositeAlpha";
//static const char DECORATE_UNUSED *kVUID_Core_Swapchain_CreateSwapBadImgArrayLayers = "UNASSIGNED-CoreValidation-Swapchain-CreateSwapBadImgArrayLayers";
//static const char DECORATE_UNUSED *kVUID_Core_Swapchain_CreateSwapBadImgColorSpace = "UNASSIGNED-CoreValidation-Swapchain-CreateSwapBadImgColorSpace";
//static const char DECORATE_UNUSED *kVUID_Core_Swapchain_CreateSwapBadImgFmtClrSp = "UNASSIGNED-CoreValidation-Swapchain-CreateSwapBadImgFmtClrSp";
//static const char DECORATE_UNUSED *kVUID_Core_Swapchain_CreateSwapBadImgFormat = "UNASSIGNED-CoreValidation-Swapchain-CreateSwapBadImgFormat";
//static const char DECORATE_UNUSED *kVUID_Core_Swapchain_CreateSwapBadImgUsageFlags = "UNASSIGNED-CoreValidation-Swapchain-CreateSwapBadImgUsageFlags";
//static const char DECORATE_UNUSED *kVUID_Core_Swapchain_CreateSwapBadPreTransform = "UNASSIGNED-CoreValidation-Swapchain-CreateSwapBadPreTransform";
//static const char DECORATE_UNUSED *kVUID_Core_Swapchain_CreateSwapBadPresentMode = "UNASSIGNED-CoreValidation-Swapchain-CreateSwapBadPresentMode";
//static const char DECORATE_UNUSED *kVUID_Core_Swapchain_CreateSwapBadSharingMode = "UNASSIGNED-CoreValidation-Swapchain-CreateSwapBadSharingMode";
//static const char DECORATE_UNUSED *kVUID_Core_Swapchain_CreateSwapBadSharingValues = "UNASSIGNED-CoreValidation-Swapchain-CreateSwapBadSharingValues";
//static const char DECORATE_UNUSED *kVUID_Core_Swapchain_CreateSwapExtentsNoMatchWin = "UNASSIGNED-CoreValidation-Swapchain-CreateSwapExtentsNoMatchWin";
//static const char DECORATE_UNUSED *kVUID_Core_Swapchain_CreateSwapOutOfBoundsExtents = "UNASSIGNED-CoreValidation-Swapchain-CreateSwapOutOfBoundsExtents";
//static const char DECORATE_UNUSED *kVUID_Core_Swapchain_CreateSwapWithoutQuery = "UNASSIGNED-CoreValidation-Swapchain-CreateSwapWithoutQuery";
//static const char DECORATE_UNUSED *kVUID_Core_Swapchain_CreateUnsupportedSurface = "UNASSIGNED-CoreValidation-SwapchainCreateUnsupportedSurface";
//static const char DECORATE_UNUSED *kVUID_Core_Swapchain_DelObjectBeforeChildren = "UNASSIGNED-CoreValidation-Swapchain-DelObjectBeforeChildren";
//static const char DECORATE_UNUSED *kVUID_Core_Swapchain_ExtNotEnabledButUsed = "UNASSIGNED-CoreValidation-Swapchain-ExtNotEnabledButUsed";
//static const char DECORATE_UNUSED *kVUID_Core_Swapchain_InvalidHandle = "UNASSIGNED-CoreValidation-SwapchainInvalidHandle";
//static const char DECORATE_UNUSED *kVUID_Core_Swapchain_NullPointer = "UNASSIGNED-CoreValidation-SwapchainNullPointer";
//static const char DECORATE_UNUSED *kVUID_Core_Swapchain_PlaneIndexTooLarge = "UNASSIGNED-CoreValidation-Swapchain-PlaneIndexTooLarge";
//static const char DECORATE_UNUSED *kVUID_Core_Swapchain_WrongNext = "UNASSIGNED-CoreValidation-SwapchainWrongNext";
//static const char DECORATE_UNUSED *kVUID_Core_Swapchain_WrongStype = "UNASSIGNED-CoreValidation-SwapchainWrongStype";
//static const char DECORATE_UNUSED *kVUID_Core_Swapchain_ZeroValue = "UNASSIGNED-CoreValidation-SwapchainZeroValue";

static const char DECORATE_UNUSED *kVUID_Core_Image_InvalidFormatLimitsViolation = "UNASSIGNED-CoreValidation-Image-InvalidFormatLimitsViolation";
static const char DECORATE_UNUSED *kVUID_Core_Image_ZeroAreaSubregion = "UNASSIGNED-CoreValidation-Image-ZeroAreaSubregion";

static const char DECORATE_UNUSED *kVUID_Core_PushDescriptorUpdate_TemplateType = "UNASSIGNED-CoreValidation-vkCmdPushDescriptorSetWithTemplateKHR-descriptorUpdateTemplate-templateType";
static const char DECORATE_UNUSED *kVUID_Core_PushDescriptorUpdate_Template_SetMismatched = "UNASSIGNED-CoreValidation-vkCmdPushDescriptorSetWithTemplateKHR-set";
static const char DECORATE_UNUSED *kVUID_Core_PushDescriptorUpdate_Template_LayoutMismatched = "UNASSIGNED-CoreValidation-vkCmdPushDescriptorSetWithTemplateKHR-layout";

static const char DECORATE_UNUSED *kVUID_Core_BindImage_InvalidMemReqQuery = "UNASSIGNED-CoreValidation-vkBindImageMemory-invalid-requirements";
static const char DECORATE_UNUSED *kVUID_Core_BindImage_NoMemReqQuery = "UNASSIGNED-CoreValidation-vkBindImageMemory-memory-requirements";
static const char DECORATE_UNUSED *kVUID_Core_BindBuffer_NoMemReqQuery = "UNASSIGNED-CoreValidation-vkBindBufferMemory-memory-requirements";

static const char DECORATE_UNUSED *kVUID_Core_BindAccelNV_NoMemReqQuery = "UNASSIGNED-CoreValidation-vkBindAccelerationStructureMemoryNV-object-requirements";
static const char DECORATE_UNUSED *kVUID_Core_CmdBuildAccelNV_NoScratchMemReqQuery = "UNASSIGNED-CoreValidation-vkCmdBuildAccelerationStructureNV-scratch-requirements";
static const char DECORATE_UNUSED *kVUID_Core_CmdBuildAccelNV_NoUpdateMemReqQuery = "UNASSIGNED-CoreValidation-vkCmdBuildAccelerationStructureNV-update-requirements";
// clang-format on

#undef DECORATE_UNUSED

#if 0  // Preserve these comments for possible inclusion in the spec reference string database
enum SWAPCHAIN_ERROR {
    SWAPCHAIN_INVALID_HANDLE,                     // Handle used that isn't currently valid
    SWAPCHAIN_NULL_POINTER,                       // Pointer set to NULL, instead of being a valid pointer
    SWAPCHAIN_EXT_NOT_ENABLED_BUT_USED,           // Did not enable WSI extension, but called WSI function
    SWAPCHAIN_DEL_OBJECT_BEFORE_CHILDREN,         // Called vkDestroyDevice() before vkDestroySwapchainKHR()
    SWAPCHAIN_CREATE_UNSUPPORTED_SURFACE,         // Called vkCreateSwapchainKHR() with a pCreateInfo->surface that wasn't supported
    SWAPCHAIN_CREATE_SWAP_WITHOUT_QUERY,          // Called vkCreateSwapchainKHR() without calling a query
    SWAPCHAIN_CREATE_SWAP_OUT_OF_BOUNDS_EXTENTS,  // Called vkCreateSwapchainKHR() with out-of-bounds imageExtent
    SWAPCHAIN_CREATE_SWAP_EXTENTS_NO_MATCH_WIN,   // Called vkCreateSwapchainKHR w/imageExtent that doesn't match window's extent
    SWAPCHAIN_CREATE_SWAP_BAD_PRE_TRANSFORM,      // Called vkCreateSwapchainKHR() with a non-supported preTransform
    SWAPCHAIN_CREATE_SWAP_BAD_COMPOSITE_ALPHA,    // Called vkCreateSwapchainKHR() with a non-supported compositeAlpha
    SWAPCHAIN_CREATE_SWAP_BAD_IMG_ARRAY_LAYERS,   // Called vkCreateSwapchainKHR() with a non-supported imageArrayLayers
    SWAPCHAIN_CREATE_SWAP_BAD_IMG_USAGE_FLAGS,    // Called vkCreateSwapchainKHR() with a non-supported imageUsageFlags
    SWAPCHAIN_CREATE_SWAP_BAD_IMG_COLOR_SPACE,    // Called vkCreateSwapchainKHR() with a non-supported imageColorSpace
    SWAPCHAIN_CREATE_SWAP_BAD_IMG_FORMAT,         // Called vkCreateSwapchainKHR() with a non-supported imageFormat
    SWAPCHAIN_CREATE_SWAP_BAD_IMG_FMT_CLR_SP,     // Called vkCreateSwapchainKHR() with a non-supported imageColorSpace
    SWAPCHAIN_CREATE_SWAP_BAD_PRESENT_MODE,       // Called vkCreateSwapchainKHR() with a non-supported presentMode
    SWAPCHAIN_CREATE_SWAP_BAD_SHARING_MODE,       // Called vkCreateSwapchainKHR() with a non-supported imageSharingMode
    SWAPCHAIN_CREATE_SWAP_BAD_SHARING_VALUES,     // Called vkCreateSwapchainKHR() with bad values when imageSharingMode is
                                                  // VK_SHARING_MODE_CONCURRENT
    SWAPCHAIN_BAD_BOOL,       // VkBool32 that doesn't have value of VK_TRUE or VK_FALSE (e.g. is a non-zero form of true)
    SWAPCHAIN_PRIOR_COUNT,    // Query must be called first to get value of pCount, then called second time
    SWAPCHAIN_INVALID_COUNT,  // Second time a query called, the pCount value didn't match first time
    SWAPCHAIN_WRONG_STYPE,    // The sType for a struct has the wrong value
    SWAPCHAIN_WRONG_NEXT,     // The pNext for a struct is not NULL
    SWAPCHAIN_ZERO_VALUE,     // A value should be non-zero
    SWAPCHAIN_GET_SUPPORTED_DISPLAYS_WITHOUT_QUERY,  // vkGetDisplayPlaneSupportedDisplaysKHR should be called after querying
                                                     // device display plane properties
    SWAPCHAIN_PLANE_INDEX_TOO_LARGE,  // a planeIndex value is larger than what vkGetDisplayPlaneSupportedDisplaysKHR returns
};
#endif

#endif  // CORE_VALIDATION_ERROR_ENUMS_H_
