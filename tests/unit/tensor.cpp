/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (C) 2025 Arm Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "containers/container_utils.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include <vector>

class NegativeTensor : public TensorTest {};

TEST_F(NegativeTensor, ConcurrentTensor) {
    TEST_DESCRIPTION(
        "Try to create a tensor when sharingMode is VK_SHARING_MODE_CONCURRENT but queueFamilyIndexCount is 0 and "
        "pQueueFamilyIndices is nullptr");
    RETURN_IF_SKIP(InitBasicTensor());

    VkTensorDescriptionARM desc = DefaultDesc();
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);

    info.sharingMode = VK_SHARING_MODE_CONCURRENT;

    m_errorMonitor->SetDesiredError("VUID-VkTensorCreateInfoARM-sharingMode-09722");
    m_errorMonitor->SetDesiredError("VUID-VkTensorCreateInfoARM-sharingMode-09723");
    vkt::Tensor tensor(*m_device, info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, ConcurrentTensorNonUniqueIdx) {
    TEST_DESCRIPTION(
        "Try to create a tensor when sharingMode is VK_SHARING_MODE_CONCURRENT but pQueueFamilyIndices are not unique");
    RETURN_IF_SKIP(InitBasicTensor());

    VkTensorDescriptionARM desc = DefaultDesc();
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);

    std::vector<uint32_t> queue_family_indices{0, 0};
    info.sharingMode = VK_SHARING_MODE_CONCURRENT;
    info.queueFamilyIndexCount = 2;
    info.pQueueFamilyIndices = queue_family_indices.data();

    m_errorMonitor->SetDesiredError("VUID-VkTensorCreateInfoARM-sharingMode-09725");
    vkt::Tensor tensor(*m_device, info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, ConcurrentTensorLargeIdx) {
    TEST_DESCRIPTION(
        "Try to create a tensor when sharingMode is VK_SHARING_MODE_CONCURRENT and pQueueFamilyIndices are larger than what is "
        "reported by vkGetPhysicalDeviceQueueFamilyProperties");
    RETURN_IF_SKIP(InitBasicTensor());

    VkTensorDescriptionARM desc = DefaultDesc();
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);

    std::vector<uint32_t> queue_family_indices{UINT32_MAX - 1, 0};
    info.sharingMode = VK_SHARING_MODE_CONCURRENT;
    info.queueFamilyIndexCount = 2;
    info.pQueueFamilyIndices = queue_family_indices.data();
    m_errorMonitor->SetDesiredError("VUID-VkTensorCreateInfoARM-sharingMode-09725");
    vkt::Tensor tensor(*m_device, info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, OpaqueCaptureMissingFlag) {
    TEST_DESCRIPTION(
        "Try to create a tensor passing in the VkOpaqueCaptureDescriptorDataCreateInfoEXT struct without setting the appropriate "
        "flag");
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBufferTensorDescriptors);
    AddRequiredFeature(vkt::Feature::descriptorBufferCaptureReplay);
    RETURN_IF_SKIP(InitBasicTensor());

    VkTensorDescriptionARM desc = DefaultDesc();
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);

    std::vector<uint32_t> opaque_capture_descriptor_data(64, 0);
    VkOpaqueCaptureDescriptorDataCreateInfoEXT opaque_tensor = vku::InitStructHelper();
    opaque_tensor.opaqueCaptureDescriptorData = opaque_capture_descriptor_data.data();

    info.pNext = &opaque_tensor;

    m_errorMonitor->SetDesiredError("VUID-VkTensorCreateInfoARM-pNext-09727");
    vkt::Tensor tensor(*m_device, info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, ProtectedMemory) {
    TEST_DESCRIPTION("Try to create a protected memory tensor when the feature is not supported");
    RETURN_IF_SKIP(InitBasicTensor());

    VkTensorDescriptionARM desc = DefaultDesc();
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);
    info.flags = VK_TENSOR_CREATE_PROTECTED_BIT_ARM;

    m_errorMonitor->SetDesiredError("VUID-VkTensorCreateInfoARM-protectedMemory-09729");
    vkt::Tensor tensor(*m_device, info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, DescriptorBuffer) {
    TEST_DESCRIPTION("Try to create a descriptorBuffer tensor when the feature is not supported");
    RETURN_IF_SKIP(InitBasicTensor());

    VkTensorDescriptionARM desc = DefaultDesc();
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);

    info.flags = VK_TENSOR_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_ARM;
    m_errorMonitor->SetDesiredError("VUID-VkTensorCreateInfoARM-flags-09726");
    vkt::Tensor tensor(*m_device, info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, OptimalWithStrides) {
    TEST_DESCRIPTION("Try to create an optimal-tiled tensor with strides");
    RETURN_IF_SKIP(InitBasicTensor());

    VkTensorDescriptionARM desc = DefaultDesc();
    desc.tiling = VK_TENSOR_TILING_OPTIMAL_ARM;
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);

    m_errorMonitor->SetDesiredError("VUID-VkTensorCreateInfoARM-pDescription-09720");
    vkt::Tensor tensor(*m_device, info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, MaxTensorElements) {
    TEST_DESCRIPTION("Try to create a tensor larger than the limit");
    RETURN_IF_SKIP(InitBasicTensor());

    VkTensorDescriptionARM desc = DefaultDesc();
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);

    VkPhysicalDeviceTensorPropertiesARM tensor_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(tensor_props);

    std::vector<int64_t> dims(tensor_props.maxTensorDimensionCount, UINT32_MAX);
    desc.pDimensions = dims.data();
    desc.dimensionCount = tensor_props.maxTensorDimensionCount;
    desc.tiling = VK_TENSOR_TILING_OPTIMAL_ARM;
    desc.pStrides = nullptr;
    m_errorMonitor->SetDesiredError("VUID-VkTensorCreateInfoARM-tensorElements-09721");
    vkt::Tensor tensor(*m_device, info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, FormatFeatureLinUsage) {
    TEST_DESCRIPTION("Test creating a linear-tiled tensor where the format feature flags are not supported for the given usage");
    RETURN_IF_SKIP(InitBasicTensor());

    VkTensorDescriptionARM desc = DefaultDesc();
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);

    VkTensorFormatPropertiesARM tensor_fmt_props = vku::InitStructHelper();
    VkFormatProperties2 fmt_props_2 = vku::InitStructHelper(&tensor_fmt_props);
    vk::GetPhysicalDeviceFormatProperties2(Gpu(), desc.format, &fmt_props_2);

    auto lin_features = tensor_fmt_props.linearTilingTensorFeatures;

    desc.usage = VK_TENSOR_USAGE_TRANSFER_SRC_BIT_ARM | VK_TENSOR_USAGE_TRANSFER_DST_BIT_ARM;

    if (!(lin_features & VK_FORMAT_FEATURE_2_TRANSFER_SRC_BIT)) {
        m_errorMonitor->SetDesiredError("VUID-VkTensorCreateInfoARM-pDescription-09728");
    }
    if (!(lin_features & VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT)) {
        m_errorMonitor->SetDesiredError("VUID-VkTensorCreateInfoARM-pDescription-09728");
    }
    vkt::Tensor tensor(*m_device, info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, FormatFeatureOptUsage) {
    TEST_DESCRIPTION("Test creating an optimal-tiled tensor where the format feature flags are not supported for the given usage");
    RETURN_IF_SKIP(InitBasicTensor());

    VkTensorDescriptionARM desc = DefaultDesc();
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);

    VkTensorFormatPropertiesARM tensor_fmt_props = vku::InitStructHelper();
    VkFormatProperties2 fmt_props_2 = vku::InitStructHelper(&tensor_fmt_props);
    vk::GetPhysicalDeviceFormatProperties2(Gpu(), desc.format, &fmt_props_2);

    auto opt_features = tensor_fmt_props.optimalTilingTensorFeatures;

    desc.usage = VK_TENSOR_USAGE_TRANSFER_SRC_BIT_ARM | VK_TENSOR_USAGE_TRANSFER_DST_BIT_ARM;
    desc.tiling = VK_TENSOR_TILING_OPTIMAL_ARM;
    desc.pStrides = nullptr;

    if (!(opt_features & VK_FORMAT_FEATURE_2_TRANSFER_SRC_BIT)) {
        m_errorMonitor->SetDesiredError("VUID-VkTensorCreateInfoARM-pDescription-09728");
    }
    if (!(opt_features & VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT)) {
        m_errorMonitor->SetDesiredError("VUID-VkTensorCreateInfoARM-pDescription-09728");
    }
    vkt::Tensor tensor(*m_device, info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, MaxDimensionCount) {
    TEST_DESCRIPTION("Test creating a tensor where the dimensionCount is larger than the maximum");
    RETURN_IF_SKIP(InitBasicTensor());

    VkTensorDescriptionARM desc = DefaultDesc();
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);

    VkPhysicalDeviceTensorPropertiesARM tensor_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(tensor_props);
    std::vector<int64_t> dims(tensor_props.maxTensorDimensionCount + 1, 1);
    desc.dimensionCount = tensor_props.maxTensorDimensionCount + 1;
    desc.pDimensions = dims.data();
    desc.tiling = VK_TENSOR_TILING_OPTIMAL_ARM;
    desc.pStrides = nullptr;

    m_errorMonitor->SetDesiredError("VUID-VkTensorDescriptionARM-dimensionCount-09733");
    vkt::Tensor tensor(*m_device, info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, DimensionsHaveZeros) {
    TEST_DESCRIPTION("Test creating a tensor where the one of the dimensions is zero");
    RETURN_IF_SKIP(InitBasicTensor());

    VkTensorDescriptionARM desc = DefaultDesc();
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);

    std::vector<int64_t> dims(2, 1);
    dims[1] = 0;
    desc.dimensionCount = 2;
    desc.pDimensions = dims.data();
    desc.tiling = VK_TENSOR_TILING_OPTIMAL_ARM;
    desc.pStrides = nullptr;

    m_errorMonitor->SetDesiredError("VUID-VkTensorDescriptionARM-pDimensions-09734");
    vkt::Tensor tensor(*m_device, info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, FormatUndefined) {
    TEST_DESCRIPTION("Test creating a tensor where the format is VK_FORMAT_UNDEFINED");
    RETURN_IF_SKIP(InitBasicTensor());

    VkTensorDescriptionARM desc = DefaultDesc();
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);
    desc.format = VK_FORMAT_UNDEFINED;

    m_errorMonitor->SetDesiredError("VUID-VkTensorDescriptionARM-format-09735");
    vkt::Tensor tensor(*m_device, info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, FormatTwoComponent) {
    TEST_DESCRIPTION("Test creating a tensor where the format is two-component");
    RETURN_IF_SKIP(InitBasicTensor());

    VkTensorDescriptionARM desc = DefaultDesc();
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);
    desc.format = VK_FORMAT_R64G64_UINT;

    const std::vector<int64_t> strides{16l};
    desc.pStrides = strides.data();

    m_errorMonitor->SetDesiredError("VUID-VkTensorDescriptionARM-format-09735");
    vkt::Tensor tensor(*m_device, info);
    m_errorMonitor->VerifyFound();
}
TEST_F(NegativeTensor, FormatThreeComponent) {
    TEST_DESCRIPTION("Test creating a tensor where the format is three-component");
    RETURN_IF_SKIP(InitBasicTensor());

    VkTensorDescriptionARM desc = DefaultDesc();
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);
    desc.format = VK_FORMAT_R64G64B64_UINT;

    const std::vector<int64_t> strides{24l};
    desc.pStrides = strides.data();

    m_errorMonitor->SetDesiredError("VUID-VkTensorDescriptionARM-format-09735");
    vkt::Tensor tensor(*m_device, info);
    m_errorMonitor->VerifyFound();
}
TEST_F(NegativeTensor, FormatFourComponent) {
    TEST_DESCRIPTION("Test creating a tensor where the format is four-component");
    RETURN_IF_SKIP(InitBasicTensor());

    VkTensorDescriptionARM desc = DefaultDesc();
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);
    desc.format = VK_FORMAT_R64G64B64A64_UINT;

    const std::vector<int64_t> strides{32l};
    desc.pStrides = strides.data();

    m_errorMonitor->SetDesiredError("VUID-VkTensorDescriptionARM-format-09735");
    vkt::Tensor tensor(*m_device, info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, InnermostStrideWrongSize) {
    TEST_DESCRIPTION("Test creating a tensor where the innermost stride is not equal to the size of the tensor element in bytes");
    RETURN_IF_SKIP(InitBasicTensor());

    VkTensorDescriptionARM desc = DefaultDesc();
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);
    const std::vector<int64_t> strides{2l};
    desc.pStrides = strides.data();

    m_errorMonitor->SetDesiredError("VUID-VkTensorDescriptionARM-pStrides-09736");
    vkt::Tensor tensor(*m_device, info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, StridesNotMultiple) {
    TEST_DESCRIPTION("Test creating a tensor where the strides are not multiples of the size of tensor element in bytes");
    RETURN_IF_SKIP(InitBasicTensor());

    VkTensorDescriptionARM desc = DefaultDesc();
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);
    const std::vector<int64_t> dimensions{4ul, 1ul};
    const std::vector<int64_t> strides{3l, 2l};
    desc.pDimensions = dimensions.data();
    desc.dimensionCount = dimensions.size();
    desc.format = VK_FORMAT_R16_UINT;
    desc.pStrides = strides.data();

    m_errorMonitor->SetDesiredError("VUID-VkTensorDescriptionARM-pStrides-09737");
    m_errorMonitor->SetDesiredError("VUID-VkTensorDescriptionARM-None-09740");
    vkt::Tensor tensor(*m_device, info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, StridesExtremes) {
    TEST_DESCRIPTION("Test creating a tensor where the strides are less than or equal to zero, or greater than maxTensorStride");
    RETURN_IF_SKIP(InitBasicTensor());

    VkTensorDescriptionARM desc = DefaultDesc();
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);

    VkPhysicalDeviceTensorPropertiesARM tensor_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(tensor_props);

    const std::vector<int64_t> dimensions{2ul, 2ul, 2ul, 2ul};
    const std::vector<int64_t> strides{-1l, 0l, static_cast<int64_t>(tensor_props.maxTensorStride + 1), 1l};
    desc.pDimensions = dimensions.data();
    desc.dimensionCount = dimensions.size();
    desc.pStrides = strides.data();

    m_errorMonitor->SetDesiredError("VUID-VkTensorDescriptionARM-pStrides-09738");
    m_errorMonitor->SetDesiredError("VUID-VkTensorDescriptionARM-pStrides-09738");
    m_errorMonitor->SetDesiredError("VUID-VkTensorDescriptionARM-pStrides-09738");

    m_errorMonitor->SetDesiredError("VUID-VkTensorDescriptionARM-None-09740");
    m_errorMonitor->SetDesiredError("VUID-VkTensorDescriptionARM-pStrides-09739");
    m_errorMonitor->SetDesiredError("VUID-VkTensorDescriptionARM-pStrides-09739");
    vkt::Tensor tensor(*m_device, info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, StridesOverlap) {
    TEST_DESCRIPTION("Test creating a tensor where the strides overlap");
    RETURN_IF_SKIP(InitBasicTensor());

    VkTensorDescriptionARM desc = DefaultDesc();
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);

    const std::vector<int64_t> dimensions{1ul, 1ul, 1ul};
    const std::vector<int64_t> strides{1l, 4ul, 1l};
    desc.pDimensions = dimensions.data();
    desc.dimensionCount = dimensions.size();
    desc.pStrides = strides.data();

    m_errorMonitor->SetDesiredError("VUID-VkTensorDescriptionARM-pStrides-09739");
    m_errorMonitor->SetDesiredError("VUID-VkTensorDescriptionARM-None-09740");
    vkt::Tensor tensor(*m_device, info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, NonPacked) {
    TEST_DESCRIPTION("Test creating a non-packed tensor when it is not supported");
    RETURN_IF_SKIP(InitBasicTensor());

    VkTensorDescriptionARM desc = DefaultDesc();
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);

    const std::vector<int64_t> dimensions{1ul, 3ul, 1ul};
    const std::vector<int64_t> strides{4l, 1l, 1l};
    desc.pDimensions = dimensions.data();
    desc.dimensionCount = dimensions.size();
    desc.pStrides = strides.data();

    m_errorMonitor->SetDesiredError("VUID-VkTensorDescriptionARM-None-09740");
    vkt::Tensor tensor(*m_device, info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, OptimalAliased) {
    TEST_DESCRIPTION(
        "Test creating an optimal tiled tensor with the IMAGE_ALIASING bit set and the innermost dimension being larger than 4");
    RETURN_IF_SKIP(InitBasicTensor());

    VkTensorDescriptionARM desc = DefaultDesc();
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);

    desc.tiling = VK_TENSOR_TILING_OPTIMAL_ARM;
    desc.usage |= VK_TENSOR_USAGE_IMAGE_ALIASING_BIT_ARM;

    const std::vector<int64_t> dimensions{1ul, 1ul, 5ul};
    desc.pDimensions = dimensions.data();
    desc.dimensionCount = dimensions.size();
    desc.pStrides = nullptr;

    m_errorMonitor->SetDesiredError("VUID-VkTensorDescriptionARM-tiling-09741");
    vkt::Tensor tensor(*m_device, info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, LinearAliased) {
    TEST_DESCRIPTION("Test creating a linear tiled tensor with the IMAGE_ALIASING bit set");
    RETURN_IF_SKIP(InitBasicTensor());

    VkTensorDescriptionARM desc = DefaultDesc();
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);

    desc.usage |= VK_TENSOR_USAGE_IMAGE_ALIASING_BIT_ARM;

    m_errorMonitor->SetDesiredError("VUID-VkTensorDescriptionARM-tiling-09742");
    vkt::Tensor tensor(*m_device, info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, RebindTensor) {
    TEST_DESCRIPTION("Test binding a tensor which is already backed");
    RETURN_IF_SKIP(InitBasicTensor());

    vkt::Tensor tensor(*m_device);

    VkMemoryRequirements2 mem_reqs = tensor.GetMemoryReqs();

    VkMemoryAllocateInfo tensor_alloc_info = vku::InitStructHelper();
    tensor_alloc_info.allocationSize = mem_reqs.memoryRequirements.size;
    m_device->Physical().SetMemoryType(mem_reqs.memoryRequirements.memoryTypeBits, &tensor_alloc_info, 0);
    vkt::DeviceMemory memory_0(*m_device, tensor_alloc_info);
    vkt::DeviceMemory memory_1(*m_device, tensor_alloc_info);

    VkBindTensorMemoryInfoARM bind_info = vku::InitStructHelper();
    bind_info.tensor = tensor.handle();
    bind_info.memory = memory_0.handle();

    vk::BindTensorMemoryARM(*m_device, 1, &bind_info);
    bind_info.memory = memory_1.handle();
    m_errorMonitor->SetDesiredError("VUID-VkBindTensorMemoryInfoARM-tensor-09712");
    vk::BindTensorMemoryARM(*m_device, 1, &bind_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, BindTensorInvalidOffset) {
    TEST_DESCRIPTION("Test binding a tensor when the memory offset is larger than the size of the memory");
    RETURN_IF_SKIP(InitBasicTensor());

    vkt::Tensor tensor(*m_device);

    VkMemoryRequirements2 mem_reqs = tensor.GetMemoryReqs();

    VkMemoryAllocateInfo tensor_alloc_info = vku::InitStructHelper();
    tensor_alloc_info.allocationSize = mem_reqs.memoryRequirements.size;
    m_device->Physical().SetMemoryType(mem_reqs.memoryRequirements.memoryTypeBits, &tensor_alloc_info, 0);
    vkt::DeviceMemory memory(*m_device, tensor_alloc_info);

    VkBindTensorMemoryInfoARM bind_info = vku::InitStructHelper();
    bind_info.tensor = tensor.handle();
    bind_info.memory = memory.handle();
    bind_info.memoryOffset = mem_reqs.memoryRequirements.size * 2 *
                             mem_reqs.memoryRequirements.alignment; /* Multiply by alignment to ensure that the offset is correctly
                                                                       aligned while still being larger than memory*/

    m_errorMonitor->SetDesiredError("VUID-VkBindTensorMemoryInfoARM-memoryOffset-09713");
    vk::BindTensorMemoryARM(*m_device, 1, &bind_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, BindTensorInvalidMemoryBits) {
    TEST_DESCRIPTION("Test binding a tensor where the required memory type is incorrect");
    RETURN_IF_SKIP(InitBasicTensor());

    vkt::Tensor tensor(*m_device);

    VkMemoryRequirements mem_reqs = tensor.GetMemoryReqs().memoryRequirements;

    const auto unsupported_mem_type_bits = ~mem_reqs.memoryTypeBits;
    VkMemoryAllocateInfo tensor_alloc_info = vku::InitStructHelper();
    tensor_alloc_info.allocationSize = mem_reqs.size;
    m_device->Physical().SetMemoryType(unsupported_mem_type_bits, &tensor_alloc_info, 0);
    vkt::DeviceMemory memory(*m_device, tensor_alloc_info);

    VkBindTensorMemoryInfoARM bind_info = vku::InitStructHelper();
    bind_info.tensor = tensor.handle();
    bind_info.memory = memory.handle();

    m_errorMonitor->SetDesiredError("VUID-VkBindTensorMemoryInfoARM-memory-09714");
    vk::BindTensorMemoryARM(*m_device, 1, &bind_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, BindTensorOffsetNotAligned) {
    TEST_DESCRIPTION("Test binding a tensor when the memory offset is not aligned");
    RETURN_IF_SKIP(InitBasicTensor());

    vkt::Tensor tensor(*m_device);

    VkMemoryRequirements2 mem_reqs = tensor.GetMemoryReqs();

    VkMemoryAllocateInfo tensor_alloc_info = vku::InitStructHelper();
    tensor_alloc_info.allocationSize = 10 * mem_reqs.memoryRequirements.size;
    m_device->Physical().SetMemoryType(mem_reqs.memoryRequirements.memoryTypeBits, &tensor_alloc_info, 0);
    vkt::DeviceMemory memory(*m_device, tensor_alloc_info);

    VkBindTensorMemoryInfoARM bind_info = vku::InitStructHelper();
    bind_info.tensor = tensor.handle();
    bind_info.memory = memory.handle();
    bind_info.memoryOffset = 3;

    m_errorMonitor->SetDesiredError("VUID-VkBindTensorMemoryInfoARM-memoryOffset-09715");
    vk::BindTensorMemoryARM(*m_device, 1, &bind_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, BindTensorMemoryTooSmall) {
    TEST_DESCRIPTION("Test binding a tensor when the available memory is smaller than the requirements");
    RETURN_IF_SKIP(InitBasicTensor());

    vkt::Tensor tensor(*m_device);

    VkMemoryRequirements2 mem_reqs = tensor.GetMemoryReqs();

    {
        // allocated buffer is too small
        VkMemoryAllocateInfo tensor_alloc_info = vku::InitStructHelper();
        tensor_alloc_info.allocationSize = mem_reqs.memoryRequirements.size / 2;
        m_device->Physical().SetMemoryType(mem_reqs.memoryRequirements.memoryTypeBits, &tensor_alloc_info, 0);
        vkt::DeviceMemory memory(*m_device, tensor_alloc_info);

        VkBindTensorMemoryInfoARM bind_info = vku::InitStructHelper();
        bind_info.tensor = tensor.handle();
        bind_info.memory = memory.handle();

        m_errorMonitor->SetDesiredError("VUID-VkBindTensorMemoryInfoARM-size-09716");
        vk::BindTensorMemoryARM(*m_device, 1, &bind_info);
        m_errorMonitor->VerifyFound();
    }
    {
        // allocated buffer is big enough, but the binding offset doesn't leave enough space
        VkMemoryAllocateInfo tensor_alloc_info = vku::InitStructHelper();
        tensor_alloc_info.allocationSize = mem_reqs.memoryRequirements.size;
        m_device->Physical().SetMemoryType(mem_reqs.memoryRequirements.memoryTypeBits, &tensor_alloc_info, 0);
        vkt::DeviceMemory memory(*m_device, tensor_alloc_info);

        VkBindTensorMemoryInfoARM bind_info = vku::InitStructHelper();
        bind_info.tensor = tensor.handle();
        bind_info.memory = memory.handle();
        bind_info.memoryOffset = 64;

        m_errorMonitor->SetDesiredError("VUID-VkBindTensorMemoryInfoARM-size-09716");
        vk::BindTensorMemoryARM(*m_device, 1, &bind_info);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeTensor, BindTensorDedicatedMemoryDifferentTensor) {
    TEST_DESCRIPTION("Test binding a tensor to memory which is dedicated to a different tensor");
    RETURN_IF_SKIP(InitBasicTensor());

    vkt::Tensor tensor(*m_device);

    VkTensorMemoryRequirementsInfoARM req_info = vku::InitStructHelper();
    req_info.tensor = tensor.handle();

    VkMemoryDedicatedRequirements dedicated_reqs = vku::InitStructHelper();
    dedicated_reqs.requiresDedicatedAllocation = VK_TRUE;
    VkMemoryRequirements2 mem_reqs = vku::InitStructHelper();
    mem_reqs.pNext = &dedicated_reqs;
    vk::GetTensorMemoryRequirementsARM(*m_device, &req_info, &mem_reqs);

    VkMemoryDedicatedAllocateInfoTensorARM dedicated_tensor_alloc_info = vku::InitStructHelper();
    dedicated_tensor_alloc_info.tensor = tensor.handle();
    VkMemoryAllocateInfo tensor_alloc_info = vku::InitStructHelper();
    VkMemoryDedicatedAllocateInfo dedicated_alloc_info = vku::InitStructHelper();
    dedicated_alloc_info.pNext = &dedicated_tensor_alloc_info;
    tensor_alloc_info.pNext = &dedicated_alloc_info;
    tensor_alloc_info.allocationSize = mem_reqs.memoryRequirements.size;
    m_device->Physical().SetMemoryType(mem_reqs.memoryRequirements.memoryTypeBits, &tensor_alloc_info, 0);
    vkt::DeviceMemory memory(*m_device, tensor_alloc_info);

    vkt::Tensor wrong_tensor(*m_device);

    VkBindTensorMemoryInfoARM bind_info = vku::InitStructHelper();
    bind_info.tensor = wrong_tensor.handle();
    bind_info.memory = memory.handle();

    m_errorMonitor->SetDesiredError("VUID-VkBindTensorMemoryInfoARM-tensor-09717");
    vk::BindTensorMemoryARM(*m_device, 1, &bind_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, BindTensorNotProtectedToProtectedMemory) {
    TEST_DESCRIPTION("Test binding an unprotected tensor to protected memory");
    AddRequiredFeature(vkt::Feature::protectedMemory);
    RETURN_IF_SKIP(InitBasicTensor());

    auto tensor_desc = DefaultDesc();
    auto protected_tensor_info = DefaultCreateInfo(&tensor_desc);
    protected_tensor_info.flags |= VK_TENSOR_CREATE_PROTECTED_BIT_ARM;

    vkt::Tensor protected_tensor(*m_device, protected_tensor_info);

    auto unprotected_tensor_info = DefaultCreateInfo(&tensor_desc);
    vkt::Tensor unprotected_tensor(*m_device, unprotected_tensor_info);
    unprotected_tensor.GetMemoryReqs();

    /* get protected memory (use requirements for protected tensor) */
    VkMemoryRequirements2 mem_reqs = protected_tensor.GetMemoryReqs();
    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    m_device->Physical().SetMemoryType(mem_reqs.memoryRequirements.memoryTypeBits, &mem_alloc, VK_MEMORY_PROPERTY_PROTECTED_BIT);
    mem_alloc.allocationSize = mem_reqs.memoryRequirements.size;
    vkt::DeviceMemory memory(*m_device, mem_alloc);

    /* bind unprotected tensor with protected memory */
    VkBindTensorMemoryInfoARM bind_info = vku::InitStructHelper();
    bind_info.tensor = unprotected_tensor.handle();
    bind_info.memory = memory.handle();

    m_errorMonitor->SetDesiredError("VUID-VkBindTensorMemoryInfoARM-tensor-09719");
    // using the wrong type of memory also causes an error with memoryBits
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkBindTensorMemoryInfoARM-memory-09714");
    vk::BindTensorMemoryARM(*m_device, 1, &bind_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, BindTensorProtectedToNotProtectedMemory) {
    TEST_DESCRIPTION("Test binding an protected tensor to unprotected memory");
    AddRequiredFeature(vkt::Feature::protectedMemory);
    RETURN_IF_SKIP(InitBasicTensor());

    auto tensor_desc = DefaultDesc();
    auto protected_tensor_info = DefaultCreateInfo(&tensor_desc);
    protected_tensor_info.flags |= VK_TENSOR_CREATE_PROTECTED_BIT_ARM;
    vkt::Tensor protected_tensor(*m_device, protected_tensor_info);

    auto unprotected_tensor_info = DefaultCreateInfo(&tensor_desc);
    vkt::Tensor unprotected_tensor(*m_device, unprotected_tensor_info);

    /* get unprotected memory (use requirements for unprotected tensor) */
    VkMemoryRequirements2 mem_reqs = unprotected_tensor.GetMemoryReqs();
    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    m_device->Physical().SetMemoryType(mem_reqs.memoryRequirements.memoryTypeBits, &mem_alloc, 0);
    mem_alloc.allocationSize = mem_reqs.memoryRequirements.size;
    vkt::DeviceMemory memory(*m_device, mem_alloc);

    /* bind protected tensor with unprotected memory */
    VkBindTensorMemoryInfoARM bind_info = vku::InitStructHelper();
    bind_info.tensor = protected_tensor.handle();
    bind_info.memory = memory.handle();

    m_errorMonitor->SetDesiredError("VUID-VkBindTensorMemoryInfoARM-tensor-09718");
    // using the wrong type of memory also causes an error with memoryBits
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkBindTensorMemoryInfoARM-memory-09714");
    vk::BindTensorMemoryARM(*m_device, 1, &bind_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, BindTensorIncompatibleExportHandleType) {
    TEST_DESCRIPTION("Test binding exporting memory with mismatched handleTypes.");
    SetTargetApiVersion(VK_API_VERSION_1_4);
    AddRequiredExtensions(VK_ARM_TENSORS_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTERNAL_MEMORY_DMA_BUF_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tensors);
    RETURN_IF_SKIP(Init());

    // 2 external memory type handleTypes
    const auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
    const auto handle_type2 = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT;

    // Create a tensor with external memory using one of the handle types
    VkExternalMemoryTensorCreateInfoARM external_info = vku::InitStructHelper();
    external_info.handleTypes = handle_type;
    VkTensorDescriptionARM desc = DefaultDesc();
    desc.usage |= VK_TENSOR_USAGE_TRANSFER_SRC_BIT_ARM | VK_TENSOR_USAGE_TRANSFER_DST_BIT_ARM;
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);
    info.pNext = &external_info;
    vkt::Tensor tensor(*m_device, info);

    // Create export memory with the other handle type
    VkExportMemoryAllocateInfo export_memory_info = vku::InitStructHelper();
    export_memory_info.handleTypes = handle_type2;
    VkMemoryDedicatedAllocateInfoTensorARM dedicated_alloc_info = vku::InitStructHelper();
    dedicated_alloc_info.tensor = tensor.handle();
    VkMemoryDedicatedAllocateInfo dedicated_info = vku::InitStructHelper();
    dedicated_info.pNext = &dedicated_alloc_info;
    export_memory_info.pNext = &dedicated_info;

    VkTensorMemoryRequirementsInfoARM req_info = vku::InitStructHelper();
    req_info.tensor = tensor.handle();
    VkMemoryRequirements2 mem_reqs = vku::InitStructHelper();
    vk::GetTensorMemoryRequirementsARM(device(), &req_info, &mem_reqs);
    const auto alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, mem_reqs.memoryRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                                    &export_memory_info);
    vkt::DeviceMemory memory(*m_device, alloc_info);

    VkBindTensorMemoryInfoARM bind_info = vku::InitStructHelper();
    bind_info.tensor = tensor.handle();
    bind_info.memory = memory.handle();

    m_errorMonitor->SetDesiredError("VUID-VkBindTensorMemoryInfoARM-memory-09895");
    vk::BindTensorMemoryARM(device(), 1, &bind_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, BindTensorImportMemoryHandleType) {
    TEST_DESCRIPTION("Validate import memory handleType for tensor");
    SetTargetApiVersion(VK_API_VERSION_1_4);
    AddRequiredExtensions(VK_ARM_TENSORS_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTERNAL_MEMORY_DMA_BUF_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tensors);
    RETURN_IF_SKIP(Init());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "External tests are not supported by MockICD, skipping tests";
    }

    // mismatched handleTypes for tensor and memory
    const auto handle_type1 = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
    const auto handle_type2 = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT;

    // Create a tensor with external memory, use type #1
    VkTensorDescriptionARM desc = DefaultDesc();
    desc.usage |= VK_TENSOR_USAGE_TRANSFER_SRC_BIT_ARM | VK_TENSOR_USAGE_TRANSFER_DST_BIT_ARM;
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);
    VkExternalMemoryTensorCreateInfoARM tensor_external_info = vku::InitStructHelper();
    info.pNext = &tensor_external_info;
    tensor_external_info.handleTypes = handle_type1;
    vkt::Tensor tensor(*m_device, info);
    auto tensor_mem_reqs = tensor.GetMemoryReqs().memoryRequirements;

    // Get external memory buffer
    VkPhysicalDeviceExternalMemoryHostPropertiesEXT memory_host_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(memory_host_props);
    auto alignment = memory_host_props.minImportedHostPointerAlignment;
    auto alloc_size = ((tensor_mem_reqs.size + alignment - 1) / alignment) * alignment;
    void *host_memory = ::operator new((size_t)alloc_size, std::align_val_t(alignment));
    if (!host_memory) {
        GTEST_SKIP() << "Failed to allocate host memory";
    }
    VkImportMemoryHostPointerInfoEXT import_info = vku::InitStructHelper();
    import_info.handleType = handle_type2;
    import_info.pHostPointer = host_memory;

    // Allocate tensor in imported buffer
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.pNext = &import_info;
    alloc_info.allocationSize = alloc_size;
    m_device->Physical().SetMemoryType(tensor_mem_reqs.memoryTypeBits, &alloc_info,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT|VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    vkt::DeviceMemory memory(*m_device, alloc_info);

    // Bind tensor (with handle_type1) and memory (with handle_type2)
    VkBindTensorMemoryInfoARM bind_info = vku::InitStructHelper();
    bind_info.tensor = tensor.handle();
    bind_info.memory = memory.handle();
    m_errorMonitor->SetDesiredError("VUID-VkBindTensorMemoryInfoARM-memory-09896");
    vk::BindTensorMemoryARM(device(), 1, &bind_info);
    m_errorMonitor->VerifyFound();

    ::operator delete(host_memory, std::align_val_t(alloc_size));
}

TEST_F(NegativeTensor, TensorViewFormatMismatch) {
    TEST_DESCRIPTION("Test creating a tensor view with a different format than the tensor");
    RETURN_IF_SKIP(InitBasicTensor());
    vkt::Tensor tensor(*m_device);
    tensor.BindToMem();

    VkTensorViewCreateInfoARM tensor_view_create_info = vku::InitStructHelper();
    tensor_view_create_info.tensor = tensor.handle();
    tensor_view_create_info.format = VK_FORMAT_R16_UINT;

    m_errorMonitor->SetDesiredError("VUID-VkTensorViewCreateInfoARM-tensor-09743");
    vkt::TensorView(*m_device, tensor_view_create_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, TensorViewInvalidUsage) {
    TEST_DESCRIPTION("Test creating a tensor view where the usage flags for the tensor are incorrect");
    RETURN_IF_SKIP(InitBasicTensor());
    auto tensor_desc = DefaultDesc();
    tensor_desc.usage = VK_TENSOR_USAGE_TRANSFER_SRC_BIT_ARM;
    vkt::Tensor tensor(*m_device, tensor_desc);
    tensor.BindToMem();

    VkTensorViewCreateInfoARM tensor_view_create_info = vku::InitStructHelper();
    tensor_view_create_info.tensor = tensor.handle();
    tensor_view_create_info.format = tensor.Format();

    m_errorMonitor->SetDesiredError("VUID-VkTensorViewCreateInfoARM-usage-09747");
    vkt::TensorView(*m_device, tensor_view_create_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, TensorViewNonSparseNotBound) {
    TEST_DESCRIPTION("Test creating a tensor view where the tensor is non-sparse and is not bound to memory");
    RETURN_IF_SKIP(InitBasicTensor());
    vkt::Tensor tensor(*m_device);

    VkTensorViewCreateInfoARM tensor_view_create_info = vku::InitStructHelper();
    tensor_view_create_info.tensor = tensor.handle();
    tensor_view_create_info.format = tensor.Format();

    m_errorMonitor->SetDesiredError("VUID-VkTensorViewCreateInfoARM-tensor-09749");
    vkt::TensorView(*m_device, tensor_view_create_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, TensorViewMutableNotCompatible) {
    TEST_DESCRIPTION("Test creating a mutable-format tensor view where the format of the tensor view is not compatible");
    RETURN_IF_SKIP(InitBasicTensor());
    auto tensor_desc = DefaultDesc();
    auto tensor_info = DefaultCreateInfo(&tensor_desc);
    tensor_info.flags |= VK_TENSOR_CREATE_MUTABLE_FORMAT_BIT_ARM;
    vkt::Tensor tensor(*m_device, tensor_info);
    tensor.BindToMem();

    VkTensorViewCreateInfoARM tensor_view_create_info = vku::InitStructHelper();
    tensor_view_create_info.tensor = tensor.handle();
    tensor_view_create_info.format = VK_FORMAT_R32_SFLOAT;

    m_errorMonitor->SetDesiredError("VUID-VkTensorViewCreateInfoARM-tensor-09744");
    vkt::TensorView(*m_device, tensor_view_create_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, TensorViewDescriptorBuffer) {
    TEST_DESCRIPTION("Test creating a tensor view when the DESCRIPTOR_BUFFER is set but the feature is not available");
    RETURN_IF_SKIP(InitBasicTensor());

    vkt::Tensor tensor(*m_device);
    tensor.BindToMem();

    VkTensorViewCreateInfoARM tensor_view_create_info = vku::InitStructHelper();
    tensor_view_create_info.flags |= VK_TENSOR_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_ARM;
    tensor_view_create_info.tensor = tensor.handle();
    tensor_view_create_info.format = tensor.Format();

    m_errorMonitor->SetDesiredError("VUID-VkTensorViewCreateInfoARM-flags-09745");
    vkt::TensorView(*m_device, tensor_view_create_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, TensorViewOpaqueCaptureMissingFlag) {
    TEST_DESCRIPTION(
        "Try to create a tensor view passing in the VkOpaqueCaptureDescriptorDataCreateInfoEXT struct without setting the "
        "appropriate "
        "flag");
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBufferTensorDescriptors);
    AddRequiredFeature(vkt::Feature::descriptorBufferCaptureReplay);
    RETURN_IF_SKIP(InitBasicTensor());

    vkt::Tensor tensor(*m_device);
    tensor.BindToMem();

    std::vector<uint32_t> opaque_capture_descriptor_data(64, 0);
    VkOpaqueCaptureDescriptorDataCreateInfoEXT opaque_capture = vku::InitStructHelper();
    opaque_capture.opaqueCaptureDescriptorData = opaque_capture_descriptor_data.data();

    VkTensorViewCreateInfoARM tensor_view_create_info = vku::InitStructHelper();
    tensor_view_create_info.tensor = tensor.handle();
    tensor_view_create_info.format = tensor.Format();
    tensor_view_create_info.pNext = &opaque_capture;

    m_errorMonitor->SetDesiredError("VUID-VkTensorViewCreateInfoARM-pNext-09746");
    vkt::TensorView(*m_device, tensor_view_create_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, TensorViewLinearMissingFeatureFlags) {
    TEST_DESCRIPTION(
        "Test creating a linear-tiled tensor where the format feature flags are not supported for the given usage"
        "flag");
    RETURN_IF_SKIP(InitBasicTensor());

    VkTensorDescriptionARM desc = DefaultDesc();
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);

    VkTensorFormatPropertiesARM tensor_fmt_props = vku::InitStructHelper();
    VkFormatProperties2 fmt_props_2 = vku::InitStructHelper(&tensor_fmt_props);
    vk::GetPhysicalDeviceFormatProperties2(Gpu(), desc.format, &fmt_props_2);

    auto lin_features = tensor_fmt_props.linearTilingTensorFeatures;

    desc.usage |= VK_TENSOR_USAGE_TRANSFER_SRC_BIT_ARM | VK_TENSOR_USAGE_TRANSFER_DST_BIT_ARM;

    vkt::Tensor tensor(*m_device, info);
    tensor.BindToMem();

    VkTensorViewCreateInfoARM tensor_view_create_info = vku::InitStructHelper();
    tensor_view_create_info.tensor = tensor.handle();
    tensor_view_create_info.format = tensor.Format();

    if (!(lin_features & VK_FORMAT_FEATURE_2_TRANSFER_SRC_BIT)) {
        m_errorMonitor->SetDesiredError("VUID-VkTensorViewCreateInfoARM-usage-09748");
    }
    if (!(lin_features & VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT)) {
        m_errorMonitor->SetDesiredError("VUID-VkTensorViewCreateInfoARM-usage-09748");
    }
    vkt::TensorView(*m_device, tensor_view_create_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, TensorViewOptimalMissingFeatureFlags) {
    TEST_DESCRIPTION(
        "Test creating a optimal-tiled tensor where the format feature flags are not supported for the given usage"
        "flag");
    RETURN_IF_SKIP(InitBasicTensor());

    VkTensorDescriptionARM desc = DefaultDesc();
    VkTensorCreateInfoARM info = DefaultCreateInfo(&desc);

    VkTensorFormatPropertiesARM tensor_fmt_props = vku::InitStructHelper();
    VkFormatProperties2 fmt_props_2 = vku::InitStructHelper(&tensor_fmt_props);
    vk::GetPhysicalDeviceFormatProperties2(Gpu(), desc.format, &fmt_props_2);

    auto opt_features = tensor_fmt_props.optimalTilingTensorFeatures;

    desc.usage |= VK_TENSOR_USAGE_TRANSFER_SRC_BIT_ARM | VK_TENSOR_USAGE_TRANSFER_DST_BIT_ARM;
    desc.tiling = VK_TENSOR_TILING_OPTIMAL_ARM;
    desc.pStrides = nullptr;

    vkt::Tensor tensor(*m_device, info);
    tensor.BindToMem();

    VkTensorViewCreateInfoARM tensor_view_create_info = vku::InitStructHelper();
    tensor_view_create_info.tensor = tensor.handle();
    tensor_view_create_info.format = tensor.Format();

    if (!(opt_features & VK_FORMAT_FEATURE_2_TRANSFER_SRC_BIT)) {
        m_errorMonitor->SetDesiredError("VUID-VkTensorViewCreateInfoARM-usage-09748");
    }
    if (!(opt_features & VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT)) {
        m_errorMonitor->SetDesiredError("VUID-VkTensorViewCreateInfoARM-usage-09748");
    }
    vkt::TensorView(*m_device, tensor_view_create_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, CopyTensorDifferentDimensionCounts) {
    TEST_DESCRIPTION("Test copying 2 tensors with different values for dimensionCount");
    RETURN_IF_SKIP(InitBasicTensor());

    auto dst_desc = DefaultDesc();
    std::vector<int64_t> dst_dimensions{2ul, 2ul, 2ul};
    dst_desc.pDimensions = dst_dimensions.data();
    dst_desc.dimensionCount = dst_dimensions.size();
    dst_desc.pStrides = nullptr;
    dst_desc.usage |= VK_TENSOR_USAGE_TRANSFER_DST_BIT_ARM;
    auto dst_info = DefaultCreateInfo(&dst_desc);

    constexpr bool is_copy_tensor = true;
    vkt::Tensor src_tensor(*m_device, is_copy_tensor);
    vkt::Tensor dst_tensor(*m_device, dst_info);

    src_tensor.BindToMem();
    dst_tensor.BindToMem();

    VkTensorCopyARM regions = vku::InitStructHelper();
    regions.dimensionCount = dst_desc.dimensionCount;

    VkCopyTensorInfoARM copy_info = vku::InitStructHelper();
    copy_info.srcTensor = src_tensor.handle();
    copy_info.dstTensor = dst_tensor.handle();
    copy_info.regionCount = 1;
    copy_info.pRegions = &regions;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkCopyTensorInfoARM-dimensionCount-09684");
    vk::CmdCopyTensorARM(m_command_buffer.handle(), &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeTensor, CopyTensorDifferentDimensions) {
    TEST_DESCRIPTION("Test copying 2 tensors with different values in pDimensions");
    RETURN_IF_SKIP(InitBasicTensor());

    auto dst_desc = DefaultDesc();
    std::vector<int64_t> dst_dimensions{1, 4, 4, 4};
    dst_desc.pDimensions = dst_dimensions.data();
    dst_desc.dimensionCount = dst_dimensions.size();
    dst_desc.pStrides = nullptr;
    dst_desc.usage |= VK_TENSOR_USAGE_TRANSFER_DST_BIT_ARM;
    auto dst_info = DefaultCreateInfo(&dst_desc);

    constexpr bool is_copy_tensor = true;
    vkt::Tensor src_tensor(*m_device, is_copy_tensor);
    vkt::Tensor dst_tensor(*m_device, dst_info);

    src_tensor.BindToMem();
    dst_tensor.BindToMem();

    VkTensorCopyARM regions = vku::InitStructHelper();
    regions.dimensionCount = dst_desc.dimensionCount;

    VkCopyTensorInfoARM copy_info = vku::InitStructHelper();
    copy_info.srcTensor = src_tensor.handle();
    copy_info.dstTensor = dst_tensor.handle();
    copy_info.regionCount = 1;
    copy_info.pRegions = &regions;

    m_command_buffer.Begin();
    // There are 3 differences in dimensions so we expect this error 3 times
    m_errorMonitor->SetDesiredError("VUID-VkCopyTensorInfoARM-pDimensions-09685");
    m_errorMonitor->SetDesiredError("VUID-VkCopyTensorInfoARM-pDimensions-09685");
    m_errorMonitor->SetDesiredError("VUID-VkCopyTensorInfoARM-pDimensions-09685");
    vk::CmdCopyTensorARM(m_command_buffer.handle(), &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeTensor, CopyTensorRegionCountTooLarge) {
    TEST_DESCRIPTION("Test copying 2 tensors where regionCount is larger than 1");
    RETURN_IF_SKIP(InitBasicTensor());

    constexpr bool is_copy_tensor = true;
    vkt::Tensor src_tensor(*m_device, is_copy_tensor);
    vkt::Tensor dst_tensor(*m_device, is_copy_tensor);

    src_tensor.BindToMem();
    dst_tensor.BindToMem();

    VkTensorCopyARM regions = vku::InitStructHelper();
    regions.dimensionCount = src_tensor.DimensionCount();

    VkTensorCopyARM regions_arr[] = {regions, regions};

    VkCopyTensorInfoARM copy_info = vku::InitStructHelper();
    copy_info.srcTensor = src_tensor.handle();
    copy_info.dstTensor = dst_tensor.handle();
    copy_info.regionCount = 2;
    copy_info.pRegions = regions_arr;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkCopyTensorInfoARM-regionCount-09686");
    vk::CmdCopyTensorARM(m_command_buffer.handle(), &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeTensor, CopyTensorSrcOffsetNotAllZero) {
    TEST_DESCRIPTION("Test copying 2 tensors where pRegions->pSrcOffset is not null and not all 0's");
    RETURN_IF_SKIP(InitBasicTensor());

    constexpr bool is_copy_tensor = true;
    vkt::Tensor src_tensor(*m_device, is_copy_tensor);
    vkt::Tensor dst_tensor(*m_device, is_copy_tensor);

    src_tensor.BindToMem();
    dst_tensor.BindToMem();

    VkTensorCopyARM regions = vku::InitStructHelper();
    regions.dimensionCount = src_tensor.DimensionCount();
    std::vector<uint64_t> src_offset(src_tensor.DimensionCount(), 1);
    regions.pSrcOffset = src_offset.data();

    VkCopyTensorInfoARM copy_info = vku::InitStructHelper();
    copy_info.srcTensor = src_tensor.handle();
    copy_info.dstTensor = dst_tensor.handle();
    copy_info.regionCount = 1;
    copy_info.pRegions = &regions;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkCopyTensorInfoARM-pRegions-09687");
    vk::CmdCopyTensorARM(m_command_buffer.handle(), &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeTensor, CopyTensorDstOffsetNotAllZero) {
    TEST_DESCRIPTION("Test copying 2 tensors where pRegions->pDstOffset is not null and not all 0's");
    RETURN_IF_SKIP(InitBasicTensor());

    constexpr bool is_copy_tensor = true;
    vkt::Tensor src_tensor(*m_device, is_copy_tensor);
    vkt::Tensor dst_tensor(*m_device, is_copy_tensor);

    src_tensor.BindToMem();
    dst_tensor.BindToMem();

    VkTensorCopyARM regions = vku::InitStructHelper();
    regions.dimensionCount = src_tensor.DimensionCount();
    std::vector<uint64_t> dst_offset(dst_tensor.DimensionCount(), 1);
    regions.pDstOffset = dst_offset.data();

    VkCopyTensorInfoARM copy_info = vku::InitStructHelper();
    copy_info.srcTensor = src_tensor.handle();
    copy_info.dstTensor = dst_tensor.handle();
    copy_info.regionCount = 1;
    copy_info.pRegions = &regions;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkCopyTensorInfoARM-pRegions-09688");
    vk::CmdCopyTensorARM(m_command_buffer.handle(), &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeTensor, CopyTensorExtentDifferentToDimensions) {
    TEST_DESCRIPTION("Test copying 2 tensors where pRegions->pExtent does not equal the dimensions array of the tensors");
    RETURN_IF_SKIP(InitBasicTensor());

    constexpr bool is_copy_tensor = true;
    vkt::Tensor src_tensor(*m_device, is_copy_tensor);
    vkt::Tensor dst_tensor(*m_device, is_copy_tensor);

    src_tensor.BindToMem();
    dst_tensor.BindToMem();

    VkTensorCopyARM regions = vku::InitStructHelper();
    regions.dimensionCount = src_tensor.DimensionCount();
    std::vector<uint64_t> extent(src_tensor.DimensionCount(), 42);
    regions.pExtent = extent.data();

    VkCopyTensorInfoARM copy_info = vku::InitStructHelper();
    copy_info.srcTensor = src_tensor.handle();
    copy_info.dstTensor = dst_tensor.handle();
    copy_info.regionCount = 1;
    copy_info.pRegions = &regions;

    m_command_buffer.Begin();
    // The extent is wrong for each of the 4 dimensions
    m_errorMonitor->SetDesiredError("VUID-VkCopyTensorInfoARM-pRegions-09689");
    m_errorMonitor->SetDesiredError("VUID-VkCopyTensorInfoARM-pRegions-09689");
    m_errorMonitor->SetDesiredError("VUID-VkCopyTensorInfoARM-pRegions-09689");
    m_errorMonitor->SetDesiredError("VUID-VkCopyTensorInfoARM-pRegions-09689");
    vk::CmdCopyTensorARM(m_command_buffer.handle(), &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeTensor, CopyTensorSrcMissingFormatFeatures) {
    TEST_DESCRIPTION("Test copying 2 tensors where the src tensor format feature flags are not supported for the given usage");
    RETURN_IF_SKIP(InitBasicTensor());

    constexpr bool is_copy_tensor = true;
    vkt::Tensor src_tensor(*m_device, is_copy_tensor);
    vkt::Tensor dst_tensor(*m_device, is_copy_tensor);

    VkTensorFormatPropertiesARM tensor_fmt_props = vku::InitStructHelper();
    VkFormatProperties2 fmt_props_2 = vku::InitStructHelper(&tensor_fmt_props);
    vk::GetPhysicalDeviceFormatProperties2(Gpu(), src_tensor.Format(), &fmt_props_2);

    // Tensor is Linear tiling by default
    auto lin_features = tensor_fmt_props.linearTilingTensorFeatures;

    bool src_support = lin_features & VK_FORMAT_FEATURE_2_TRANSFER_SRC_BIT;
    if (src_support) {
        GTEST_SKIP() << "Src Tensor Linear Feature Flags supported";
    }

    src_tensor.BindToMem();
    dst_tensor.BindToMem();

    VkTensorCopyARM regions = vku::InitStructHelper();
    regions.dimensionCount = src_tensor.DimensionCount();

    VkCopyTensorInfoARM copy_info = vku::InitStructHelper();
    copy_info.srcTensor = src_tensor.handle();
    copy_info.dstTensor = dst_tensor.handle();
    copy_info.regionCount = 1;
    copy_info.pRegions = &regions;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkCopyTensorInfoARM-srcTensor-09690");
    vk::CmdCopyTensorARM(m_command_buffer.handle(), &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeTensor, CopyTensorSrcNoTransferBit) {
    TEST_DESCRIPTION("Test copying 2 tensors where the src tensor does not have the transfer bit set");
    RETURN_IF_SKIP(InitBasicTensor());

    constexpr bool is_copy_tensor = true;
    vkt::Tensor src_tensor(*m_device, !is_copy_tensor);
    vkt::Tensor dst_tensor(*m_device, is_copy_tensor);

    src_tensor.BindToMem();
    dst_tensor.BindToMem();

    VkTensorCopyARM regions = vku::InitStructHelper();
    regions.dimensionCount = src_tensor.DimensionCount();

    VkCopyTensorInfoARM copy_info = vku::InitStructHelper();
    copy_info.srcTensor = src_tensor.handle();
    copy_info.dstTensor = dst_tensor.handle();
    copy_info.regionCount = 1;
    copy_info.pRegions = &regions;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkCopyTensorInfoARM-srcTensor-09691");
    vk::CmdCopyTensorARM(m_command_buffer.handle(), &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeTensor, CopyTensorDstMissingFormatFeatures) {
    TEST_DESCRIPTION("Test copying 2 tensors where the dst tensor format feature flags are not supported for the given usage");
    RETURN_IF_SKIP(InitBasicTensor());

    constexpr bool is_copy_tensor = true;
    vkt::Tensor src_tensor(*m_device, is_copy_tensor);
    vkt::Tensor dst_tensor(*m_device, is_copy_tensor);

    VkTensorFormatPropertiesARM tensor_fmt_props = vku::InitStructHelper();
    VkFormatProperties2 fmt_props_2 = vku::InitStructHelper(&tensor_fmt_props);
    vk::GetPhysicalDeviceFormatProperties2(Gpu(), dst_tensor.Format(), &fmt_props_2);

    // Tensor is Linear tiling by default
    auto lin_features = tensor_fmt_props.linearTilingTensorFeatures;

    bool dst_support = lin_features & VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT;
    if (dst_support) {
        GTEST_SKIP() << "Dst Tensor Linear Feature Flags supported";
    }

    src_tensor.BindToMem();
    dst_tensor.BindToMem();

    VkTensorCopyARM regions = vku::InitStructHelper();
    regions.dimensionCount = src_tensor.DimensionCount();

    VkCopyTensorInfoARM copy_info = vku::InitStructHelper();
    copy_info.srcTensor = src_tensor.handle();
    copy_info.dstTensor = dst_tensor.handle();
    copy_info.regionCount = 1;
    copy_info.pRegions = &regions;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkCopyTensorInfoARM-dstTensor-09692");
    vk::CmdCopyTensorARM(m_command_buffer.handle(), &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeTensor, CopyTensorDstNoTransferBit) {
    TEST_DESCRIPTION("Test copying 2 tensors where the dst tensor does not have the transfer bit set");
    RETURN_IF_SKIP(InitBasicTensor());

    constexpr bool is_copy_tensor = true;
    vkt::Tensor src_tensor(*m_device, is_copy_tensor);
    vkt::Tensor dst_tensor(*m_device);

    src_tensor.BindToMem();
    dst_tensor.BindToMem();

    VkTensorCopyARM regions = vku::InitStructHelper();
    regions.dimensionCount = src_tensor.DimensionCount();

    VkCopyTensorInfoARM copy_info = vku::InitStructHelper();
    copy_info.srcTensor = src_tensor.handle();
    copy_info.dstTensor = dst_tensor.handle();
    copy_info.regionCount = 1;
    copy_info.pRegions = &regions;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkCopyTensorInfoARM-dstTensor-09693");
    vk::CmdCopyTensorARM(m_command_buffer.handle(), &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeTensor, CopyTensorSrcNotBound) {
    TEST_DESCRIPTION("Test copying 2 tensors where the src tensor is not bound to memory");
    RETURN_IF_SKIP(InitBasicTensor());

    constexpr bool is_copy_tensor = true;
    vkt::Tensor src_tensor(*m_device, is_copy_tensor);
    vkt::Tensor dst_tensor(*m_device, is_copy_tensor);

    dst_tensor.BindToMem();

    VkTensorCopyARM regions = vku::InitStructHelper();
    regions.dimensionCount = src_tensor.DimensionCount();

    VkCopyTensorInfoARM copy_info = vku::InitStructHelper();
    copy_info.srcTensor = src_tensor.handle();
    copy_info.dstTensor = dst_tensor.handle();
    copy_info.regionCount = 1;
    copy_info.pRegions = &regions;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkCopyTensorInfoARM-srcTensor-09694");
    vk::CmdCopyTensorARM(m_command_buffer.handle(), &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeTensor, CopyTensorDstNotBound) {
    TEST_DESCRIPTION("Test copying 2 tensors where the dst tensor is not bound to memory");
    RETURN_IF_SKIP(InitBasicTensor());

    constexpr bool is_copy_tensor = true;
    vkt::Tensor src_tensor(*m_device, is_copy_tensor);
    vkt::Tensor dst_tensor(*m_device, is_copy_tensor);

    src_tensor.BindToMem();

    VkTensorCopyARM regions = vku::InitStructHelper();
    regions.dimensionCount = src_tensor.DimensionCount();

    VkCopyTensorInfoARM copy_info = vku::InitStructHelper();
    copy_info.srcTensor = src_tensor.handle();
    copy_info.dstTensor = dst_tensor.handle();
    copy_info.regionCount = 1;
    copy_info.pRegions = &regions;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkCopyTensorInfoARM-dstTensor-09695");
    vk::CmdCopyTensorARM(m_command_buffer.handle(), &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeTensor, DestroyTensorInUse) {
    TEST_DESCRIPTION("Test destroying a tensor while it is being used");
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(InitBasicTensor());

    constexpr bool is_copy_tensor = true;
    vkt::Tensor src_tensor(*m_device, is_copy_tensor);
    vkt::Tensor dst_tensor(*m_device, is_copy_tensor);

    src_tensor.BindToMem();
    dst_tensor.BindToMem();

    VkTensorCopyARM regions = vku::InitStructHelper();
    regions.dimensionCount = src_tensor.DimensionCount();

    VkCopyTensorInfoARM copy_info = vku::InitStructHelper();
    copy_info.srcTensor = src_tensor.handle();
    copy_info.dstTensor = dst_tensor.handle();
    copy_info.regionCount = 1;
    copy_info.pRegions = &regions;

    m_command_buffer.Begin();
    vk::CmdCopyTensorARM(m_command_buffer.handle(), &copy_info);
    m_command_buffer.End();

    // Create a timeline semaphore block the command buffer
    VkSemaphoreTypeCreateInfo sem_type = vku::InitStructHelper();
    sem_type.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    sem_type.initialValue = 0;
    VkSemaphoreCreateInfo create_sem = vku::InitStructHelper();
    create_sem.pNext = &sem_type;

    vkt::Semaphore sem(*m_device, create_sem);
    VkTimelineSemaphoreSubmitInfo timeline_info = vku::InitStructHelper();
    const uint64_t wait_value = 1;
    timeline_info.waitSemaphoreValueCount = 1;
    timeline_info.pWaitSemaphoreValues = &wait_value;

    VkPipelineStageFlags dst_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.pNext = &timeline_info;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &sem.handle();
    submit_info.pWaitDstStageMask = &dst_stage_mask;
    submit_info.commandBufferCount = 1u;
    submit_info.pCommandBuffers = &m_command_buffer.handle();

    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);

    // Try destroying the tensor before signalling the semaphore
    m_errorMonitor->SetDesiredError("VUID-vkDestroyTensorARM-tensor-09730");
    vk::DestroyTensorARM(*m_device, src_tensor.handle(), nullptr);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredError("VUID-vkDestroyTensorARM-tensor-09730");
    vk::DestroyTensorARM(*m_device, dst_tensor.handle(), nullptr);
    m_errorMonitor->VerifyFound();

    // Signal semaphore to finish execution
    VkSemaphoreSignalInfo signal_sem = vku::InitStructHelper();
    signal_sem.semaphore = sem.handle();
    signal_sem.value = 1;
    vk::SignalSemaphore(*m_device, &signal_sem);

    m_default_queue->Wait();
}

TEST_F(NegativeTensor, DestroyTensorCreateWithDestroyWithoutCallbacks) {
    TEST_DESCRIPTION("Test destroying without callbacks a tensor which was created with callbacks");
    RETURN_IF_SKIP(InitBasicTensor());

    auto desc = DefaultDesc();
    auto info = DefaultCreateInfo(&desc);
    VkTensorARM tensor;
    vk::CreateTensorARM(*m_device, &info, vkt::DefaultAllocator(), &tensor);
    m_errorMonitor->SetDesiredError("VUID-vkDestroyTensorARM-tensor-09731");
    vk::DestroyTensorARM(*m_device, tensor, nullptr);
    m_errorMonitor->VerifyFound();
    // Clean up by correctly destroying the tensor
    vk::DestroyTensorARM(*m_device, tensor, vkt::DefaultAllocator());
}

TEST_F(NegativeTensor, DestroyTensorCreateWithoutDestroyWithCallbacks) {
    TEST_DESCRIPTION("Test destroying with callbacks a tensor which was not created with callbacks");
    RETURN_IF_SKIP(InitBasicTensor());

    auto desc = DefaultDesc();
    auto info = DefaultCreateInfo(&desc);
    VkTensorARM tensor;
    vk::CreateTensorARM(*m_device, &info, nullptr, &tensor);
    m_errorMonitor->SetDesiredError("VUID-vkDestroyTensorARM-tensor-09732");
    vk::DestroyTensorARM(*m_device, tensor, vkt::DefaultAllocator());
    m_errorMonitor->VerifyFound();
    // Clean up by correctly destroying the tensor
    vk::DestroyTensorARM(*m_device, tensor, nullptr);
}

TEST_F(NegativeTensor, DestroyTensorViewInUse) {
    TEST_DESCRIPTION("Test destroying a tensor view while it is in use");
    AddRequiredFeature(vkt::Feature::shaderTensorAccess);
    AddRequiredFeature(vkt::Feature::timelineSemaphore);
    RETURN_IF_SKIP(InitBasicTensor());

    vkt::Tensor tensor(*m_device);
    tensor.BindToMem();

    VkTensorViewCreateInfoARM tensor_view_create_info = vku::InitStructHelper();
    tensor_view_create_info.tensor = tensor.handle();
    tensor_view_create_info.format = tensor.Format();
    vkt::TensorView view(*m_device, tensor_view_create_info);

    vkt::Buffer buffer(*m_device, tensor.GetMemoryReqs().memoryRequirements.size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    CreateComputePipelineHelper pipe(*m_device);
    pipe.cs_ = VkShaderObj::CreateFromGLSL(this, tensor_shader_source, VK_SHADER_STAGE_COMPUTE_BIT);

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_TENSOR_ARM, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};

    pipe.dsl_bindings_.resize(bindings.size());
    memcpy(pipe.dsl_bindings_.data(), bindings.data(), bindings.size() * sizeof(VkDescriptorSetLayoutBinding));
    pipe.CreateComputePipeline();
    pipe.descriptor_set_.WriteDescriptorTensorInfo(0, &view.handle());
    pipe.descriptor_set_.WriteDescriptorBufferInfo(1, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1, &pipe.descriptor_set_.set_, 0,
                              nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.End();

    // Create a timeline semaphore block the command buffer
    VkSemaphoreTypeCreateInfo sem_type = vku::InitStructHelper();
    sem_type.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    sem_type.initialValue = 0;
    VkSemaphoreCreateInfo create_sem = vku::InitStructHelper();
    create_sem.pNext = &sem_type;

    vkt::Semaphore sem(*m_device, create_sem);
    VkTimelineSemaphoreSubmitInfo timeline_info = vku::InitStructHelper();
    const uint64_t wait_value = 1;
    timeline_info.waitSemaphoreValueCount = 1;
    timeline_info.pWaitSemaphoreValues = &wait_value;

    VkPipelineStageFlags dst_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.pNext = &timeline_info;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &sem.handle();
    submit_info.pWaitDstStageMask = &dst_stage_mask;
    submit_info.commandBufferCount = 1u;
    submit_info.pCommandBuffers = &m_command_buffer.handle();

    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->SetDesiredError("VUID-vkDestroyTensorViewARM-tensorView-09750");
    vk::DestroyTensorViewARM(*m_device, view, nullptr);
    m_errorMonitor->VerifyFound();

    // Signal semaphore to finish execution
    VkSemaphoreSignalInfo signal_sem = vku::InitStructHelper();
    signal_sem.semaphore = sem.handle();
    signal_sem.value = 1;
    vk::SignalSemaphore(*m_device, &signal_sem);

    m_default_queue->Wait();
}

TEST_F(NegativeTensor, DestroyTensorViewCreateWithDestroyWithoutCallbacks) {
    TEST_DESCRIPTION("Test destroying without callbacks a tensor view which was created with callbacks");
    RETURN_IF_SKIP(InitBasicTensor());

    vkt::Tensor tensor(*m_device);
    tensor.BindToMem();
    VkTensorViewCreateInfoARM tensor_view_create_info = vku::InitStructHelper();
    tensor_view_create_info.tensor = tensor.handle();
    tensor_view_create_info.format = tensor.Format();
    VkTensorViewARM view;
    vk::CreateTensorViewARM(*m_device, &tensor_view_create_info, vkt::DefaultAllocator(), &view);
    m_errorMonitor->SetDesiredError("VUID-vkDestroyTensorViewARM-tensorView-09751");
    vk::DestroyTensorViewARM(*m_device, view, nullptr);
    m_errorMonitor->VerifyFound();
    // Clean up by correctly destroying the tensor
    vk::DestroyTensorViewARM(*m_device, view, vkt::DefaultAllocator());
}

TEST_F(NegativeTensor, DestroyTensorViewCreateWithoutDestroyWithCallbacks) {
    TEST_DESCRIPTION("Test destroying with callbacks a tensor view which was not created with callbacks");
    RETURN_IF_SKIP(InitBasicTensor());

    vkt::Tensor tensor(*m_device);
    tensor.BindToMem();
    VkTensorViewCreateInfoARM tensor_view_create_info = vku::InitStructHelper();
    tensor_view_create_info.tensor = tensor.handle();
    tensor_view_create_info.format = tensor.Format();
    VkTensorViewARM view;
    vk::CreateTensorViewARM(*m_device, &tensor_view_create_info, nullptr, &view);
    m_errorMonitor->SetDesiredError("VUID-vkDestroyTensorViewARM-tensorView-09752");
    vk::DestroyTensorViewARM(*m_device, view, vkt::DefaultAllocator());
    m_errorMonitor->VerifyFound();
    // Clean up by correctly destroying the tensor view
    vk::DestroyTensorViewARM(*m_device, view, nullptr);
}

TEST_F(NegativeTensor, GetTensorOpaqueCaptureFeatureNotEnabled) {
    TEST_DESCRIPTION("Test getting descriptor data when the feature is not enabled");
    RETURN_IF_SKIP(InitBasicTensor());

    vkt::Tensor tensor(*m_device);
    VkTensorCaptureDescriptorDataInfoARM tensor_capture_desc_data_info = vku::InitStructHelper();
    tensor_capture_desc_data_info.tensor = tensor.handle();
    uint32_t data = 0;
    m_errorMonitor->SetDesiredError("VUID-vkGetTensorOpaqueCaptureDescriptorDataARM-descriptorBufferCaptureReplay-09702");
    vk::GetTensorOpaqueCaptureDescriptorDataARM(*m_device, &tensor_capture_desc_data_info, &data);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, GetTensorOpaqueCaptureMissingFlag) {
    TEST_DESCRIPTION("Test getting tensor descriptor data when the flag was not set on creation");
    AddRequiredFeature(vkt::Feature::descriptorBufferTensorDescriptors);
    AddRequiredFeature(vkt::Feature::descriptorBufferCaptureReplay);
    RETURN_IF_SKIP(InitBasicTensor());

    vkt::Tensor tensor(*m_device);
    VkTensorCaptureDescriptorDataInfoARM tensor_capture_desc_data_info = vku::InitStructHelper();
    tensor_capture_desc_data_info.tensor = tensor.handle();
    uint32_t data = 0;
    m_errorMonitor->SetDesiredError("VUID-VkTensorCaptureDescriptorDataInfoARM-tensor-09705");
    vk::GetTensorOpaqueCaptureDescriptorDataARM(*m_device, &tensor_capture_desc_data_info, &data);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, GetTensorViewOpaqueCaptureFeatureNotEnabled) {
    TEST_DESCRIPTION("Test getting descriptor data when the feature is not enabled");
    RETURN_IF_SKIP(InitBasicTensor());

    vkt::Tensor tensor(*m_device);
    tensor.BindToMem();

    VkTensorViewCreateInfoARM tensor_view_create_info = vku::InitStructHelper();
    tensor_view_create_info.tensor = tensor.handle();
    tensor_view_create_info.format = tensor.Format();

    vkt::TensorView view(*m_device, tensor_view_create_info);

    VkTensorViewCaptureDescriptorDataInfoARM tensor_capture_desc_data_info = vku::InitStructHelper();
    tensor_capture_desc_data_info.tensorView = view.handle();

    uint32_t data = 0;
    m_errorMonitor->SetDesiredError("VUID-vkGetTensorViewOpaqueCaptureDescriptorDataARM-descriptorBufferCaptureReplay-09706");
    vk::GetTensorViewOpaqueCaptureDescriptorDataARM(*m_device, &tensor_capture_desc_data_info, &data);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, GetTensorViewOpaqueCaptureMissingFlag) {
    TEST_DESCRIPTION("Test getting tensor view descriptor data when the flag was not set on creation");
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorBufferTensorDescriptors);
    AddRequiredFeature(vkt::Feature::descriptorBufferCaptureReplay);
    RETURN_IF_SKIP(InitBasicTensor());

    auto tensor_desc = DefaultDesc();
    auto tensor_ci = DefaultCreateInfo(&tensor_desc);
    tensor_ci.flags = VK_TENSOR_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_ARM;
    vkt::Tensor tensor(*m_device, tensor_ci);
    tensor.BindToMem();

    VkTensorViewCreateInfoARM tensor_view_create_info = vku::InitStructHelper();
    tensor_view_create_info.tensor = tensor.handle();
    tensor_view_create_info.format = tensor.Format();

    vkt::TensorView view(*m_device, tensor_view_create_info);

    VkTensorViewCaptureDescriptorDataInfoARM tensor_capture_desc_data_info = vku::InitStructHelper();
    tensor_capture_desc_data_info.tensorView = view.handle();

    uint32_t data = 0;
    m_errorMonitor->SetDesiredError("VUID-VkTensorViewCaptureDescriptorDataInfoARM-tensorView-09709");
    vk::GetTensorViewOpaqueCaptureDescriptorDataARM(*m_device, &tensor_capture_desc_data_info, &data);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, MemoryDedicatedAllocateInfoTensorWrongAllocationSize) {
    TEST_DESCRIPTION(
        "Test allocating dedicated device memory for tensor, with allocation size not matching the tensor size requirement");
    RETURN_IF_SKIP(InitBasicTensor());

    vkt::Tensor tensor(*m_device);

    VkMemoryRequirements2 mem_reqs = tensor.GetMemoryReqs();

    VkMemoryDedicatedAllocateInfoTensorARM dedicated_tensor_alloc_info = vku::InitStructHelper();
    dedicated_tensor_alloc_info.tensor = tensor.handle();
    VkMemoryAllocateInfo tensor_alloc_info = vku::InitStructHelper();
    VkMemoryDedicatedAllocateInfo dedicated_alloc_info = vku::InitStructHelper();
    dedicated_alloc_info.pNext = &dedicated_tensor_alloc_info;
    tensor_alloc_info.pNext = &dedicated_alloc_info;
    tensor_alloc_info.allocationSize = mem_reqs.memoryRequirements.size * 2;
    m_device->Physical().SetMemoryType(mem_reqs.memoryRequirements.memoryTypeBits, &tensor_alloc_info, 0);

    m_errorMonitor->SetDesiredError("VUID-VkMemoryDedicatedAllocateInfoTensorARM-allocationSize-09710");
    vkt::DeviceMemory memory(*m_device, tensor_alloc_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, WriteDescriptorSetTensorInfoMissing) {
    TEST_DESCRIPTION("Test trying to write to a descriptor set without a VkWriteDescriptorSetTensorARM structure");
    RETURN_IF_SKIP(InitBasicTensor());

    vkt::Tensor tensor(*m_device);
    tensor.BindToMem();

    VkTensorViewCreateInfoARM tensor_view_create_info = vku::InitStructHelper();
    tensor_view_create_info.tensor = tensor.handle();
    tensor_view_create_info.format = tensor.Format();

    vkt::TensorView view(*m_device, tensor_view_create_info);

    constexpr uint32_t tensor_binding_count = 5;

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_TENSOR_ARM, tensor_binding_count, VK_SHADER_STAGE_ALL, nullptr},
                                       });

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.pNext = nullptr;  // missing VkWriteDescriptorSetTensorARM, causes 9945
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = tensor_binding_count;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_TENSOR_ARM;

    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-09945");
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, WriteDescriptorSetTensorInfoWrongCount) {
    TEST_DESCRIPTION("Test trying to write to a descriptor set with the wrong descriptor count");
    RETURN_IF_SKIP(InitBasicTensor());

    vkt::Tensor tensor(*m_device);
    tensor.BindToMem();

    VkTensorViewCreateInfoARM tensor_view_create_info = vku::InitStructHelper();
    tensor_view_create_info.tensor = tensor.handle();
    tensor_view_create_info.format = tensor.Format();

    vkt::TensorView view(*m_device, tensor_view_create_info);

    constexpr uint32_t tensor_binding_count = 5;

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_TENSOR_ARM, tensor_binding_count, VK_SHADER_STAGE_ALL, nullptr},
                                       });

    const VkTensorViewARM view_writes[tensor_binding_count] = {view.handle(), view.handle(), view.handle(), view.handle(),
                                                               view.handle()};
    VkWriteDescriptorSetTensorARM tensor_descriptor_write = vku::InitStructHelper();
    tensor_descriptor_write.tensorViewCount = tensor_binding_count;
    tensor_descriptor_write.pTensorViews = view_writes;

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.pNext = &tensor_descriptor_write;
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = tensor_binding_count - 1;  // should be tensor_binding_count
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_TENSOR_ARM;

    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-09945");
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, TensorMemoryBarrierSharingModeConcurrentSrcQueueFamilyNotIgnored) {
    TEST_DESCRIPTION(
        "Test setting a tensor memory barrier when the tensor was created with VK_SHARING_MODE_CONCURRENT but the src "
        "QueueFamilyIndex is not VK_QUEUE_FAMILY_IGNORED ");
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBasicTensor());

    const uint32_t submit_family = m_device->graphics_queue_node_index_;
    const uint32_t queue_family_count = static_cast<uint32_t>(m_device->Physical().queue_properties_.size());
    const uint32_t other_family = submit_family != 0 ? 0 : 1;
    const bool only_one_family = (queue_family_count == 1) ||
                                 (m_device->Physical().queue_properties_[other_family].queueCount == 0) ||
                                 ((m_device->Physical().queue_properties_[other_family].queueFlags & VK_QUEUE_TRANSFER_BIT) == 0);
    if (only_one_family) {
        GTEST_SKIP() << "Only 1 queue family found. Skipping VK_SHARING_MODE_CONCURRENT tests";
    }

    auto desc = DefaultDesc();
    auto info = DefaultCreateInfo(&desc);
    std::vector<uint32_t> qf_indices{submit_family, other_family};
    info.sharingMode = VK_SHARING_MODE_CONCURRENT;
    info.queueFamilyIndexCount = qf_indices.size();
    info.pQueueFamilyIndices = qf_indices.data();

    vkt::Tensor tensor(*m_device, info);
    tensor.BindToMem();

    VkTensorMemoryBarrierARM barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    barrier.srcAccessMask =
        VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_HOST_BIT;
    barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    barrier.srcQueueFamilyIndex = submit_family;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.tensor = tensor.handle();
    VkTensorDependencyInfoARM barrier_dep_info = vku::InitStructHelper();
    barrier_dep_info.tensorMemoryBarrierCount = 1;
    barrier_dep_info.pTensorMemoryBarriers = &barrier;
    VkDependencyInfo dependency_info = vku::InitStructHelper(&barrier_dep_info);

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkTensorMemoryBarrierARM-tensor-09755");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeTensor, TensorMemoryBarrierSharingModeConcurrentDstQueueFamilyNotIgnored) {
    TEST_DESCRIPTION(
        "Test setting a tensor memory barrier when the tensor was created with VK_SHARING_MODE_CONCURRENT but the dst "
        "QueueFamilyIndex is not VK_QUEUE_FAMILY_IGNORED ");
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBasicTensor());

    const uint32_t submit_family = m_device->graphics_queue_node_index_;
    const uint32_t queue_family_count = static_cast<uint32_t>(m_device->Physical().queue_properties_.size());
    const uint32_t other_family = submit_family != 0 ? 0 : 1;
    const bool only_one_family = (queue_family_count == 1) ||
                                 (m_device->Physical().queue_properties_[other_family].queueCount == 0) ||
                                 ((m_device->Physical().queue_properties_[other_family].queueFlags & VK_QUEUE_TRANSFER_BIT) == 0);
    if (only_one_family) {
        GTEST_SKIP() << "Only 1 queue family found. Skipping VK_SHARING_MODE_CONCURRENT tests";
    }

    auto desc = DefaultDesc();
    auto info = DefaultCreateInfo(&desc);
    std::vector<uint32_t> qf_indices{submit_family, other_family};
    info.sharingMode = VK_SHARING_MODE_CONCURRENT;
    info.queueFamilyIndexCount = qf_indices.size();
    info.pQueueFamilyIndices = qf_indices.data();

    vkt::Tensor tensor(*m_device, info);
    tensor.BindToMem();

    VkTensorMemoryBarrierARM barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    barrier.srcAccessMask =
        VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_HOST_BIT;
    barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = other_family;
    barrier.tensor = tensor.handle();
    VkTensorDependencyInfoARM barrier_dep_info = vku::InitStructHelper();
    barrier_dep_info.tensorMemoryBarrierCount = 1;
    barrier_dep_info.pTensorMemoryBarriers = &barrier;
    VkDependencyInfo dependency_info = vku::InitStructHelper(&barrier_dep_info);

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkTensorMemoryBarrierARM-tensor-09755");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeTensor, TensorMemoryBarrierSrcQueueFamilyIgnoredDstSet) {
    TEST_DESCRIPTION(
        "Test setting a tensor memory barrier when the srcQueueFamilyIndex is VK_QUEUE_FAMILY_IGNORED but dstQueueFamilyIndex is "
        "set");
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBasicTensor());

    vkt::Tensor tensor(*m_device);
    tensor.BindToMem();

    VkTensorMemoryBarrierARM barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    barrier.srcAccessMask =
        VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_HOST_BIT;
    barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = UINT32_MAX - 1;
    barrier.tensor = tensor.handle();
    VkTensorDependencyInfoARM barrier_dep_info = vku::InitStructHelper();
    barrier_dep_info.tensorMemoryBarrierCount = 1;
    barrier_dep_info.pTensorMemoryBarriers = &barrier;
    VkDependencyInfo dependency_info = vku::InitStructHelper(&barrier_dep_info);

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkTensorMemoryBarrierARM-tensor-09756");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeTensor, TensorMemoryBarrierDstQueueFamilyIgnoredSrcSet) {
    TEST_DESCRIPTION(
        "Test setting a tensor memory barrier when the srcQueueFamilyIndex is set but dstQueueFamilyIndex is "
        "VK_QUEUE_FAMILY_IGNORED");
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBasicTensor());

    vkt::Tensor tensor(*m_device);
    tensor.BindToMem();

    VkTensorMemoryBarrierARM barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    barrier.srcAccessMask =
        VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_HOST_BIT;
    barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    barrier.srcQueueFamilyIndex = UINT32_MAX - 1;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.tensor = tensor.handle();
    VkTensorDependencyInfoARM barrier_dep_info = vku::InitStructHelper();
    barrier_dep_info.tensorMemoryBarrierCount = 1;
    barrier_dep_info.pTensorMemoryBarriers = &barrier;
    VkDependencyInfo dependency_info = vku::InitStructHelper(&barrier_dep_info);

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkTensorMemoryBarrierARM-tensor-09756");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeTensor, TensorMemoryBarrierWrongQueueFamilySets) {
    TEST_DESCRIPTION("Use a tensor memory barrier on one queue family, but the command buffer is allocated on another");
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBasicTensor());

    auto queue_family_properties = m_device->Physical().queue_properties_;
    std::vector<uint32_t> queue_families;
    for (uint32_t i = 0; i < queue_family_properties.size(); ++i) {
        if (queue_family_properties[i].queueCount > 0) {
            queue_families.push_back(i);
        }
    }

    if (queue_families.size() < 2) {
        GTEST_SKIP() << "Only 1 queue family found. Skipping VK_SHARING_MODE_CONCURRENT tests";
    }

    const uint32_t queue_family = queue_families[0];
    const uint32_t other_queue_family = queue_families[1];

    vkt::CommandPool cmd_pool(*m_device, other_queue_family);
    vkt::CommandBuffer cmd_buff(*m_device, cmd_pool);

    vkt::Tensor tensor(*m_device);
    tensor.BindToMem();

    VkTensorMemoryBarrierARM barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    barrier.srcAccessMask =
        VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_HOST_BIT;
    barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    barrier.srcQueueFamilyIndex = queue_family;
    barrier.dstQueueFamilyIndex = queue_family;
    barrier.tensor = tensor.handle();
    VkTensorDependencyInfoARM barrier_dep_info = vku::InitStructHelper();
    barrier_dep_info.tensorMemoryBarrierCount = 1;
    barrier_dep_info.pTensorMemoryBarriers = &barrier;
    VkDependencyInfo dependency_info = vku::InitStructHelper(&barrier_dep_info);

    cmd_buff.Begin();
    vk::CmdPipelineBarrier2(cmd_buff.handle(), &dependency_info);
    cmd_buff.End();

    // Submit on the wrong queue
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd_buff.handle();
    // Get a queue from the wrong family
    VkQueue queue;
    vk::GetDeviceQueue(device(), other_queue_family, 0, &queue);

    m_errorMonitor->SetDesiredError("VUID-VkTensorMemoryBarrierARM-tensor-09757");
    vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, TensorMemoryBarrierTensorNotBound) {
    TEST_DESCRIPTION("Test setting a tensor memory barrier when the tensor is not bound completely to memory");
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBasicTensor());

    vkt::Tensor tensor(*m_device);

    VkTensorMemoryBarrierARM barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    barrier.srcAccessMask =
        VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_HOST_BIT;
    barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.tensor = tensor.handle();
    VkTensorDependencyInfoARM barrier_dep_info = vku::InitStructHelper();
    barrier_dep_info.tensorMemoryBarrierCount = 1;
    barrier_dep_info.pTensorMemoryBarriers = &barrier;
    VkDependencyInfo dependency_info = vku::InitStructHelper(&barrier_dep_info);

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkTensorMemoryBarrierARM-tensor-09758");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeTensor, BindTensorDedicatedMemoryOffsetNotZero) {
    TEST_DESCRIPTION("Test binding a tensor to dedicated memory at a non-zero offset");
    RETURN_IF_SKIP(InitBasicTensor());

    vkt::Tensor tensor(*m_device);

    VkTensorMemoryRequirementsInfoARM req_info = vku::InitStructHelper();
    req_info.tensor = tensor.handle();

    VkMemoryDedicatedRequirements dedicated_reqs = vku::InitStructHelper();
    dedicated_reqs.requiresDedicatedAllocation = VK_TRUE;
    VkMemoryRequirements2 mem_reqs = vku::InitStructHelper();
    mem_reqs.pNext = &dedicated_reqs;
    vk::GetTensorMemoryRequirementsARM(*m_device, &req_info, &mem_reqs);

    VkMemoryDedicatedAllocateInfoTensorARM dedicated_tensor_alloc_info = vku::InitStructHelper();
    dedicated_tensor_alloc_info.tensor = tensor.handle();
    VkMemoryAllocateInfo tensor_alloc_info = vku::InitStructHelper();
    VkMemoryDedicatedAllocateInfo dedicated_alloc_info = vku::InitStructHelper();
    dedicated_alloc_info.pNext = &dedicated_tensor_alloc_info;
    tensor_alloc_info.pNext = &dedicated_alloc_info;
    tensor_alloc_info.allocationSize = mem_reqs.memoryRequirements.size;
    m_device->Physical().SetMemoryType(mem_reqs.memoryRequirements.memoryTypeBits, &tensor_alloc_info, 0);
    vkt::DeviceMemory memory(*m_device, tensor_alloc_info);

    VkBindTensorMemoryInfoARM bind_info = vku::InitStructHelper();
    bind_info.tensor = tensor.handle();
    bind_info.memory = memory.handle();
    bind_info.memoryOffset = mem_reqs.memoryRequirements.alignment;

    // We expect this VUID error as the dedicated allocation size MUST match the requirements
    // so adding an offset will inevitably make the available memory too small
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkBindTensorMemoryInfoARM-size-09716");

    m_errorMonitor->SetDesiredError("VUID-VkBindTensorMemoryInfoARM-memory-09806");
    vk::BindTensorMemoryARM(*m_device, 1, &bind_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, CreateTensorFeatureNotEnabled) {
    TEST_DESCRIPTION("Try creating a tensor when the feature is not available");
    SetTargetApiVersion(VK_API_VERSION_1_4);
    AddRequiredExtensions(VK_ARM_TENSORS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    m_errorMonitor->SetDesiredError("VUID-vkCreateTensorARM-tensors-09832");
    vkt::Tensor tensor(*m_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, GetDeviceMemoryReqsFeatureNotEnabled) {
    TEST_DESCRIPTION("Try getting the device tensor memory requirements when the feature is not available");
    SetTargetApiVersion(VK_API_VERSION_1_4);
    AddRequiredExtensions(VK_ARM_TENSORS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkTensorDescriptionARM desc = DefaultDesc();
    VkTensorCreateInfoARM create_info = DefaultCreateInfo(&desc);
    VkDeviceTensorMemoryRequirementsARM info = vku::InitStructHelper();
    info.pCreateInfo = &create_info;
    VkMemoryRequirements2 mem_reqs = vku::InitStructHelper();

    m_errorMonitor->SetDesiredError("VUID-vkGetDeviceTensorMemoryRequirementsARM-tensors-09831");
    vk::GetDeviceTensorMemoryRequirementsARM(*m_device, &info, &mem_reqs);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTensor, CreateTensorIncompatibleHandleTypes) {
    TEST_DESCRIPTION("Creating tensor with incompatible external memory handle types");
    AddRequiredExtensions(VK_EXT_EXTERNAL_MEMORY_DMA_BUF_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicTensor());

    // external memory handles and features supported for tensors (exclude graphics).
    constexpr auto allowed_handle_bits = static_cast<VkExternalMemoryHandleTypeFlags>(
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT | VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT |
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_MAPPED_FOREIGN_MEMORY_BIT_EXT);
    constexpr auto allowed_feature_flags = static_cast<VkExternalMemoryFeatureFlagBits>(
        VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT | VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT |
        VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT);
    // combine all supported types associated to all compatibility groups; keep only one compatible group (the last)
    VkTensorDescriptionARM desc = DefaultDesc();
    VkExternalMemoryHandleTypeFlags supported_handle_types = 0;
    VkExternalMemoryHandleTypeFlags any_compatible_group = 0;
    IterateFlags<VkExternalMemoryHandleTypeFlagBits>(allowed_handle_bits, [&](VkExternalMemoryHandleTypeFlagBits flag) {
        VkPhysicalDeviceExternalTensorInfoARM tensor_info = vku::InitStructHelper();
        tensor_info.pDescription = &desc;
        tensor_info.handleType = flag;
        VkExternalTensorPropertiesARM tensor_properties = vku::InitStructHelper();
        vk::GetPhysicalDeviceExternalTensorPropertiesARM(m_device->Physical(), &tensor_info, &tensor_properties);
        const auto features = tensor_properties.externalMemoryProperties.externalMemoryFeatures;
        if (features & allowed_feature_flags) {
            supported_handle_types |= flag;
            any_compatible_group = tensor_properties.externalMemoryProperties.compatibleHandleTypes;
        }
    });

    // The actual test. `supported_handle_types` NOT fully compatible with `any_compatible_group` means it CAN't be fully included
    // in `any_compatible_group`
    if ((supported_handle_types & any_compatible_group) != supported_handle_types) {
        VkExternalMemoryTensorCreateInfoARM ext_memory_info = vku::InitStructHelper();
        ext_memory_info.handleTypes = supported_handle_types;
        VkTensorCreateInfoARM tensor_info = vku::InitStructHelper();
        tensor_info.pDescription = &desc;
        tensor_info.pNext = &ext_memory_info;
        m_errorMonitor->SetDesiredError("VUID-VkTensorCreateInfoARM-pNext-09864");
        vkt::Tensor tensor(*m_device, tensor_info);
        m_errorMonitor->VerifyFound();
    } else {
        // all supported types are fully compatible: we can't test
        GTEST_SKIP() << "Couldn't find an incompatible memory flags combination on this platform.";
    }
}
