/*
 * Copyright (C) 2025-2026 Advanced Micro Devices, Inc. All rights reserved.
 * Copyright (c) 2025-2026 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "utils/math_utils.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/shader_object_helper.h"
#include <cstdint>
#include <vulkan/utility/vk_struct_helper.hpp>

void DescriptorHeapTest::InitBasicDescriptorHeap() {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorHeap);
    RETURN_IF_SKIP(Init());
    GetPhysicalDeviceProperties2(heap_props);
}

void DescriptorHeapTest::CreateResourceHeap(VkDeviceSize app_size) {
    const VkDeviceSize heap_size = AlignResource(app_size + heap_props.minResourceHeapReservedRange);

    VkBufferUsageFlags2CreateInfo buffer_usage = vku::InitStructHelper();
    buffer_usage.usage = VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    resource_heap_.Init(*m_device, vkt::Buffer::CreateInfo(heap_size, 0, {}, &buffer_usage),
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocate_flag_info);
    resource_heap_data_ = static_cast<uint8_t*>(resource_heap_.Memory().Map());
}

void DescriptorHeapTest::CreateSamplerHeap(VkDeviceSize app_size, bool use_embedded_samplers) {
    embedded_samplers = use_embedded_samplers;
    const VkDeviceSize reserved_range =
        (embedded_samplers ? heap_props.minSamplerHeapReservedRangeWithEmbedded : heap_props.minSamplerHeapReservedRange);
    const VkDeviceSize heap_size = AlignSampler(app_size + reserved_range);

    VkBufferUsageFlags2CreateInfo buffer_usage = vku::InitStructHelper();
    buffer_usage.usage = VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    sampler_heap_.Init(*m_device, vkt::Buffer::CreateInfo(heap_size, 0, {}, &buffer_usage),
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocate_flag_info);
    sampler_heap_data_ = static_cast<uint8_t*>(sampler_heap_.Memory().Map());
}

void DescriptorHeapTest::BindResourceHeap() {
    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange.address = resource_heap_.Address();
    bind_resource_info.heapRange.size = resource_heap_.CreateInfo().size;
    bind_resource_info.reservedRangeOffset = resource_heap_.CreateInfo().size - heap_props.minResourceHeapReservedRange;
    bind_resource_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);
}

void DescriptorHeapTest::BindSamplerHeap() {
    const VkDeviceSize min_reserved_range =
        embedded_samplers ? heap_props.minSamplerHeapReservedRangeWithEmbedded : heap_props.minSamplerHeapReservedRange;
    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange.address = sampler_heap_.Address();
    bind_resource_info.heapRange.size = sampler_heap_.CreateInfo().size;
    bind_resource_info.reservedRangeOffset = sampler_heap_.CreateInfo().size - min_reserved_range;
    bind_resource_info.reservedRangeSize = min_reserved_range;
    vk::CmdBindSamplerHeapEXT(m_command_buffer, &bind_resource_info);
}

// Aligns end according to type alignment and sets the value to return as the begin of the type
// Then increments end by size * count and sets it as the new end
VkDeviceSize DescriptorHeapTest::AlignedAppend(VkDeviceSize& end, VkDescriptorType type, uint32_t count) {
    VkDeviceSize size = 0;
    VkDeviceSize align = 0;

    switch (type) {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
            size = heap_props.samplerDescriptorSize;
            align = heap_props.samplerDescriptorAlignment;
            break;

        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
            size = heap_props.bufferDescriptorSize;
            align = heap_props.bufferDescriptorAlignment;
            break;
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:
        case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            size = heap_props.imageDescriptorSize;
            align = heap_props.imageDescriptorAlignment;
            break;
        case VK_DESCRIPTOR_TYPE_TENSOR_ARM: {
            const auto tensor_props =
                vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorHeapTensorPropertiesARM>(heap_props.pNext);
            assert(tensor_props);
            size = tensor_props->tensorDescriptorSize;
            align = tensor_props->tensorDescriptorAlignment;
            break;
        }
        default:
            assert(0);
    }

    end = Align(end, align);

    const VkDeviceSize start = end;

    end += count * size;

    return start;
}

VkDeviceSize DescriptorHeapTest::AlignResource(VkDeviceSize offset) {
    return Align(Align(offset, heap_props.bufferDescriptorAlignment), heap_props.imageDescriptorAlignment);
}

VkDeviceSize DescriptorHeapTest::AlignSampler(VkDeviceSize offset) { return Align(offset, heap_props.samplerDescriptorAlignment); }

class NegativeDescriptorHeap : public DescriptorHeapTest {};

TEST_F(NegativeDescriptorHeap, NotEnabled) {
    TEST_DESCRIPTION("Tests for descriptor heap when feature is not enabled");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(Init());

    GetPhysicalDeviceProperties2(heap_props);

    {
        VkSamplerCreateInfo sampler_ci = vku::InitStructHelper();
        VkHostAddressRangeEXT descriptors = {&sampler_ci, static_cast<size_t>(heap_props.samplerDescriptorSize)};

        m_errorMonitor->SetDesiredError("VUID-vkWriteSamplerDescriptorsEXT-descriptorHeap-11202");
        vk::WriteSamplerDescriptorsEXT(device(), 1u, &sampler_ci, &descriptors);
        m_errorMonitor->VerifyFound();
    }
    {
        const VkDeviceSize buffer_size = heap_props.bufferDescriptorSize;
        vkt::Buffer buffer(*m_device, buffer_size, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, vkt::device_address);

        VkTexelBufferDescriptorInfoEXT texel_buffer_info = vku::InitStructHelper();
        texel_buffer_info.format = VK_FORMAT_R8G8B8A8_UNORM;
        texel_buffer_info.addressRange.address = buffer.Address();
        texel_buffer_info.addressRange.size = buffer_size;

        VkResourceDescriptorInfoEXT desc_info = vku::InitStructHelper();
        desc_info.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        desc_info.data.pTexelBuffer = &texel_buffer_info;

        VkHostAddressRangeEXT descriptors = {&desc_info, static_cast<size_t>(heap_props.imageDescriptorSize)};

        m_errorMonitor->SetDesiredError("VUID-vkWriteResourceDescriptorsEXT-descriptorHeap-11206");
        vk::WriteResourceDescriptorsEXT(device(), 1u, &desc_info, &descriptors);
        m_errorMonitor->VerifyFound();
    }
    {
        std::vector<uint8_t> data(heap_props.imageCaptureReplayOpaqueDataSize);

        VkImageCreateInfo image_create_info = vku::InitStructHelper();
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.extent.width = 128;
        image_create_info.extent.height = 128;
        image_create_info.extent.depth = 1;
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.format = VK_FORMAT_D32_SFLOAT;
        image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        vkt::Image image(*m_device, image_create_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VkHostAddressRangeEXT data2 = {data.data(), data.size()};

        m_errorMonitor->SetDesiredError("VUID-vkGetImageOpaqueCaptureDataEXT-descriptorHeapCaptureReplay-11282");
        m_errorMonitor->SetDesiredError("VUID-vkGetImageOpaqueCaptureDataEXT-pImages-11285");
        vk::GetImageOpaqueCaptureDataEXT(device(), 1, &image.handle(), &data2);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, SamplerParameterSize) {
    TEST_DESCRIPTION("Validate vkWriteSamplerDescriptorsEXT size parameter");
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize sampler_size = vk::GetPhysicalDeviceDescriptorSizeEXT(Gpu(), VK_DESCRIPTOR_TYPE_SAMPLER);
    std::vector<uint8_t> data(static_cast<size_t>(sampler_size));

    VkSamplerCreateInfo sampler_ci = vku::InitStructHelper();
    VkHostAddressRangeEXT descriptors = {data.data(), static_cast<size_t>(sampler_size) - 1};
    {
        m_errorMonitor->SetDesiredError("VUID-vkWriteSamplerDescriptorsEXT-size-11203");
        vk::WriteSamplerDescriptorsEXT(device(), 1u, &sampler_ci, &descriptors);
        m_errorMonitor->VerifyFound();
    }
    {
        descriptors.size = 0;
        m_errorMonitor->SetDesiredError("VUID-VkHostAddressRangeEXT-size-arraylength");
        vk::WriteSamplerDescriptorsEXT(device(), 1u, &sampler_ci, &descriptors);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, SamplerParameterYCbCr) {
    TEST_DESCRIPTION("Validate vkWriteSamplerDescriptorsEXT with YCbCr conversion");
    AddRequiredFeature(vkt::Feature::samplerYcbcrConversion);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize sampler_size = vk::GetPhysicalDeviceDescriptorSizeEXT(Gpu(), VK_DESCRIPTOR_TYPE_SAMPLER);
    std::vector<uint8_t> data(static_cast<size_t>(sampler_size));

    vkt::SamplerYcbcrConversion conversion(*m_device, VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM);
    VkSamplerYcbcrConversionInfoKHR ycbcr_conversion_info = vku::InitStructHelper();
    ycbcr_conversion_info.conversion = conversion;

    VkSamplerCreateInfo sampler_ci = vku::InitStructHelper(&ycbcr_conversion_info);
    sampler_ci.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_ci.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_ci.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    VkHostAddressRangeEXT descriptors = {data.data(), static_cast<size_t>(sampler_size)};

    m_errorMonitor->SetDesiredError("VUID-vkWriteSamplerDescriptorsEXT-pSamplers-11204");
    vk::WriteSamplerDescriptorsEXT(device(), 1u, &sampler_ci, &descriptors);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, SamplerParameterDebugUtilsObjectName) {
    TEST_DESCRIPTION("Validate vkWriteSamplerDescriptorsEXT with DebugUilsObjectName");
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize sampler_size = vk::GetPhysicalDeviceDescriptorSizeEXT(Gpu(), VK_DESCRIPTOR_TYPE_SAMPLER);
    std::vector<uint8_t> data(static_cast<size_t>(sampler_size));

    VkDebugUtilsObjectNameInfoEXT object_name_info = vku::InitStructHelper();
    object_name_info.objectType = VK_OBJECT_TYPE_DEVICE;
    VkSamplerCreateInfo sampler_ci = vku::InitStructHelper(&object_name_info);
    VkHostAddressRangeEXT descriptors = {data.data(), static_cast<size_t>(sampler_size)};

    m_errorMonitor->SetDesiredError("VUID-vkWriteSamplerDescriptorsEXT-pNext-11400");
    vk::WriteSamplerDescriptorsEXT(device(), 1u, &sampler_ci, &descriptors);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, SamplerParameterCustomBorderColor) {
    TEST_DESCRIPTION("Validate vkWriteSamplerDescriptorsEXT with custom border color");
    AddRequiredExtensions(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::customBorderColors);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize sampler_size = vk::GetPhysicalDeviceDescriptorSizeEXT(Gpu(), VK_DESCRIPTOR_TYPE_SAMPLER);
    std::vector<uint8_t> data(static_cast<size_t>(sampler_size));

    VkPhysicalDeviceCustomBorderColorPropertiesEXT border_color_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(border_color_props);

    VkSamplerCustomBorderColorCreateInfoEXT custom_border_color_ci = vku::InitStructHelper();
    custom_border_color_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    VkSamplerCustomBorderColorIndexCreateInfoEXT border_color_info = vku::InitStructHelper(&custom_border_color_ci);
    border_color_info.index = border_color_props.maxCustomBorderColorSamplers;
    VkSamplerCreateInfo sampler_ci = vku::InitStructHelper(&border_color_info);
    sampler_ci.borderColor = VK_BORDER_COLOR_FLOAT_CUSTOM_EXT;
    VkHostAddressRangeEXT descriptors = {data.data(), static_cast<size_t>(sampler_size)};

    {
        m_errorMonitor->SetDesiredError("VUID-vkWriteSamplerDescriptorsEXT-borderColor-11205");
        // Do not use SetDesiredError in next line, due to it ends validation after the error
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkSamplerCustomBorderColorIndexCreateInfoEXT-index-11289");
        vk::WriteSamplerDescriptorsEXT(device(), 1u, &sampler_ci, &descriptors);
        m_errorMonitor->VerifyFound();
    }
    {
        sampler_ci.pNext = &custom_border_color_ci;
        m_errorMonitor->SetDesiredError("VUID-vkWriteSamplerDescriptorsEXT-borderColor-11444");
        vk::WriteSamplerDescriptorsEXT(device(), 1u, &sampler_ci, &descriptors);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, SamplerParameterPSamplersStateless) {
    TEST_DESCRIPTION("Validate vkWriteSamplerDescriptorsEXT check stateless path");
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize sampler_size = vk::GetPhysicalDeviceDescriptorSizeEXT(Gpu(), VK_DESCRIPTOR_TYPE_SAMPLER);
    std::vector<uint8_t> data(static_cast<size_t>(sampler_size));

    VkSamplerCreateInfo sampler_ci = vku::InitStructHelper();
    VkHostAddressRangeEXT descriptors = {data.data(), static_cast<size_t>(sampler_size)};

    sampler_ci.minLod = 1.0f;
    sampler_ci.maxLod = 0.0f;
    m_errorMonitor->SetDesiredError("VUID-VkSamplerCreateInfo-maxLod-01973");
    vk::WriteSamplerDescriptorsEXT(device(), 1u, &sampler_ci, &descriptors);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, SamplerParameterPSamplersCore) {
    TEST_DESCRIPTION("Validate vkWriteSamplerDescriptorsEXT check core validation path");
    AddRequiredFeature(vkt::Feature::samplerYcbcrConversion);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize sampler_size = vk::GetPhysicalDeviceDescriptorSizeEXT(Gpu(), VK_DESCRIPTOR_TYPE_SAMPLER);
    std::vector<uint8_t> data(static_cast<size_t>(sampler_size));

    VkHostAddressRangeEXT descriptors = {data.data(), static_cast<size_t>(sampler_size)};

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkSamplerYcbcrConversion ycbcr_conv = VK_NULL_HANDLE;
    VkSamplerYcbcrConversionCreateInfo sycci = vku::InitStructHelper();
    sycci.format = VK_FORMAT_UNDEFINED;
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    sycci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    sycci.forceExplicitReconstruction = VK_FALSE;
    sycci.chromaFilter = VK_FILTER_NEAREST;
    sycci.xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    sycci.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;

    // test non external conversion with a VK_FORMAT_UNDEFINED
    m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-format-04061");
    vk::CreateSamplerYcbcrConversion(device(), &sycci, nullptr, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // test for non unorm
    sycci.format = VK_FORMAT_R8G8B8A8_SNORM;
    m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-format-04061");
    m_errorMonitor->SetUnexpectedError("VUID-VkSamplerYcbcrConversionCreateInfo-format-01650");
    vk::CreateSamplerYcbcrConversion(device(), &sycci, nullptr, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // Force the multi-planar format support desired format features
    VkFormat mp_format = VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM;
    VkFormatProperties formatProps;
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(Gpu(), mp_format, &formatProps);
    formatProps.linearTilingFeatures = 0;
    formatProps.optimalTilingFeatures = 0;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(Gpu(), mp_format, formatProps);

    // Check that errors are caught when format feature don't exist
    sycci.format = mp_format;

    // No Chroma Sampler Bit set
    m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-format-01650");
    // 01651 set off twice for both xChromaOffset and yChromaOffset
    m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01651");
    m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01651");
    vk::CreateSamplerYcbcrConversion(device(), &sycci, nullptr, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // Cosited feature supported, but midpoint samples set
    formatProps.linearTilingFeatures = 0;
    formatProps.optimalTilingFeatures = VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(Gpu(), mp_format, formatProps);
    sycci.xChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
    sycci.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01652");
    vk::CreateSamplerYcbcrConversion(device(), &sycci, nullptr, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // Moving support to Linear to test that it checks either linear or optimal
    formatProps.linearTilingFeatures = VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT;
    formatProps.optimalTilingFeatures = 0;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(Gpu(), mp_format, formatProps);
    sycci.xChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
    sycci.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01652");
    vk::CreateSamplerYcbcrConversion(device(), &sycci, nullptr, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // Using forceExplicitReconstruction without feature bit
    sycci.forceExplicitReconstruction = VK_TRUE;
    sycci.xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    sycci.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-forceExplicitReconstruction-01656");
    vk::CreateSamplerYcbcrConversion(device(), &sycci, nullptr, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // Linear chroma filtering without feature bit
    sycci.forceExplicitReconstruction = VK_FALSE;
    sycci.chromaFilter = VK_FILTER_LINEAR;
    m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-chromaFilter-01657");
    vk::CreateSamplerYcbcrConversion(device(), &sycci, nullptr, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // Add linear feature bit so can create valid SamplerYcbcrConversion
    formatProps.linearTilingFeatures = VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT;
    formatProps.optimalTilingFeatures = VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(Gpu(), mp_format, formatProps);
    vkt::SamplerYcbcrConversion conversion(*m_device, sycci);

    // Try to create a Sampler with non-matching filters without feature bit set
    VkSamplerYcbcrConversionInfo sampler_ycbcr_info = vku::InitStructHelper();
    sampler_ycbcr_info.conversion = conversion;
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.minFilter = VK_FILTER_NEAREST;  // Different than chromaFilter
    sampler_ci.magFilter = VK_FILTER_LINEAR;
    sampler_ci.pNext = (void*)&sampler_ycbcr_info;

    m_errorMonitor->SetDesiredError("VUID-vkWriteSamplerDescriptorsEXT-pSamplers-11204");
    vk::WriteSamplerDescriptorsEXT(device(), 1u, &sampler_ci, &descriptors);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, ResourceParameterTypeInvalid) {
    TEST_DESCRIPTION("Validate vkWriteResourceDescriptorsEXT with unacceptable type");
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize size = 1024;
    std::vector<uint8_t> data(size);

    VkResourceDescriptorInfoEXT resource_desc_info = vku::InitStructHelper();
    resource_desc_info.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    VkHostAddressRangeEXT descriptors = {&data[0], data.size()};

    m_errorMonitor->SetDesiredError("VUID-VkResourceDescriptorInfoEXT-type-11210");
    vk::WriteResourceDescriptorsEXT(device(), 1u, &resource_desc_info, &descriptors);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, ResourceParameterSize) {
    TEST_DESCRIPTION("Validate vkWriteResourceDescriptorsEXT size parameter");
    AddRequiredFeature(vkt::Feature::nullDescriptor);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDescriptorType types[] = {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
                                      VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE};

    for (auto t : types) {
        const VkDeviceSize size = vk::GetPhysicalDeviceDescriptorSizeEXT(Gpu(), t);
        if (size > 0) {
            std::vector<uint8_t> data(static_cast<size_t>(size));

            VkResourceDescriptorInfoEXT resource_desc_info = vku::InitStructHelper();
            resource_desc_info.type = t;
            VkHostAddressRangeEXT descriptors = {data.data(), static_cast<size_t>(size - 1)};

            {
                m_errorMonitor->SetDesiredError("VUID-vkWriteResourceDescriptorsEXT-size-11207");
                vk::WriteResourceDescriptorsEXT(device(), 1u, &resource_desc_info, &descriptors);
                m_errorMonitor->VerifyFound();
            }
            {
                descriptors.size = 0;
                m_errorMonitor->SetDesiredError("VUID-VkHostAddressRangeEXT-size-arraylength");
                vk::WriteResourceDescriptorsEXT(device(), 1u, &resource_desc_info, &descriptors);
                m_errorMonitor->VerifyFound();
            }
        }
    }
}

TEST_F(NegativeDescriptorHeap, ResourceParameterDataNull) {
    TEST_DESCRIPTION("Validate vkWriteResourceDescriptorsEXT null pointer");
    AddRequiredExtensions(VK_ARM_TENSORS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tensors);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const struct {
        const char* vuid;
        std::vector<VkDescriptorType> types;
    } subtests[] = {
        {
            "VUID-VkResourceDescriptorInfoEXT-None-11211",
            std::vector<VkDescriptorType>{
                VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            },
        },
        {"VUID-VkResourceDescriptorInfoEXT-None-11212",
         std::vector<VkDescriptorType>{VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER}},
        {"VUID-VkResourceDescriptorInfoEXT-None-11213",
         std::vector<VkDescriptorType>{// checked in separate test VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
                                       // VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV,
                                       VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER}},
        {"VUID-VkResourceDescriptorInfoEXT-None-11457",
         std::vector<VkDescriptorType>{VK_DESCRIPTOR_TYPE_TENSOR_ARM}},
    };
    for (const auto& s : subtests) {
        for (auto type : s.types) {
            const VkDeviceSize size = vk::GetPhysicalDeviceDescriptorSizeEXT(Gpu(), type);
            std::vector<uint8_t> data(static_cast<size_t>(size));

            VkResourceDescriptorInfoEXT resource_desc_info = vku::InitStructHelper();
            resource_desc_info.type = type;
            VkHostAddressRangeEXT descriptors = {data.data(), static_cast<size_t>(size)};

            m_errorMonitor->SetDesiredError(s.vuid);
            vk::WriteResourceDescriptorsEXT(device(), 1u, &resource_desc_info, &descriptors);
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(NegativeDescriptorHeap, ResourceParameterDataNullAS) {
    TEST_DESCRIPTION("Validate vkWriteResourceDescriptorsEXT nullDescriptor for acceleration structure");
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize size = vk::GetPhysicalDeviceDescriptorSizeEXT(Gpu(), VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR);
    std::vector<uint8_t> data(static_cast<size_t>(size));

    VkResourceDescriptorInfoEXT resource_desc_info = vku::InitStructHelper();
    resource_desc_info.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    VkHostAddressRangeEXT descriptors = {data.data(), static_cast<size_t>(size)};

    m_errorMonitor->SetDesiredError("VUID-VkResourceDescriptorInfoEXT-None-11213");
    vk::WriteResourceDescriptorsEXT(device(), 1u, &resource_desc_info, &descriptors);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, ResourceParameterDebugUtilsObjectName) {
    TEST_DESCRIPTION("Validate vkWriteResourceDescriptorsEXT with DebugUilsObjectName");
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize resource_size = vk::GetPhysicalDeviceDescriptorSizeEXT(Gpu(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    std::vector<uint8_t> data(static_cast<size_t>(resource_size));

    const uint32_t buffer_size = 16;
    vkt::Buffer dummy_buffer(*m_device, buffer_size, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT, vkt::device_address);

    VkDeviceAddressRangeEXT device_address_range = {dummy_buffer.Address(), buffer_size};

    VkDebugUtilsObjectNameInfoEXT object_name_info = vku::InitStructHelper();
    object_name_info.objectType = VK_OBJECT_TYPE_DEVICE;

    VkResourceDescriptorInfoEXT resource_desc_info = vku::InitStructHelper();
    resource_desc_info.pNext = &object_name_info;
    resource_desc_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    resource_desc_info.data.pAddressRange = &device_address_range;
    VkHostAddressRangeEXT descriptors = {data.data(), static_cast<size_t>(resource_size)};

    m_errorMonitor->SetDesiredError("VUID-VkResourceDescriptorInfoEXT-pNext-11401");
    vk::WriteResourceDescriptorsEXT(device(), 1u, &resource_desc_info, &descriptors);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, ResourceParameterDataAddressZero) {
    TEST_DESCRIPTION("Validate vkWriteResourceDescriptorsEXT case when address is 0, but size is not");
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    {
        const VkDeviceSize size = vk::GetPhysicalDeviceDescriptorSizeEXT(Gpu(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        std::vector<uint8_t> data(static_cast<size_t>(size));

        VkDeviceAddressRangeEXT invalid_device_address_range = {0, 16};

        VkResourceDescriptorInfoEXT resource_desc_info = vku::InitStructHelper();
        resource_desc_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        resource_desc_info.data.pAddressRange = &invalid_device_address_range;
        VkHostAddressRangeEXT descriptors = {data.data(), static_cast<size_t>(size)};

        m_errorMonitor->SetDesiredError("VUID-VkDeviceAddressRangeEXT-size-11411");
        vk::WriteResourceDescriptorsEXT(device(), 1u, &resource_desc_info, &descriptors);
        m_errorMonitor->VerifyFound();
    }
    {
        const VkDeviceSize size = vk::GetPhysicalDeviceDescriptorSizeEXT(Gpu(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
        std::vector<uint8_t> data(static_cast<size_t>(size));

        VkTexelBufferDescriptorInfoEXT texel_buffer_info = vku::InitStructHelper();
        texel_buffer_info.format = VK_FORMAT_R8G8B8A8_UNORM;
        texel_buffer_info.addressRange.address = 0;
        texel_buffer_info.addressRange.size = size;
        VkResourceDescriptorInfoEXT resource_desc_info = vku::InitStructHelper();
        resource_desc_info.type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        resource_desc_info.data.pTexelBuffer = &texel_buffer_info;
        VkHostAddressRangeEXT descriptors = {data.data(), static_cast<size_t>(size)};

        m_errorMonitor->SetDesiredError("VUID-VkDeviceAddressRangeEXT-size-11411");
        vk::WriteResourceDescriptorsEXT(device(), 1u, &resource_desc_info, &descriptors);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, ResourceParameterDataSizeZero) {
    TEST_DESCRIPTION("Validate vkWriteResourceDescriptorsEXT case when size is 0");
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    auto types = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER};

    const VkDeviceSize buffer_size = heap_props.bufferDescriptorSize;
    vkt::Buffer buffer(*m_device, buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                       vkt::device_address);

    for (auto type : types) {
        const VkDeviceSize size = vk::GetPhysicalDeviceDescriptorSizeEXT(Gpu(), type);
        std::vector<uint8_t> data(static_cast<size_t>(size));

        VkDeviceAddressRangeEXT invalid_device_address_range = {buffer.Address(), 0};
        VkResourceDescriptorInfoEXT resource_desc_info = vku::InitStructHelper();
        resource_desc_info.type = type;
        resource_desc_info.data.pAddressRange = &invalid_device_address_range;
        VkHostAddressRangeEXT descriptors = {data.data(), static_cast<size_t>(size)};

        m_errorMonitor->SetDesiredError("VUID-VkResourceDescriptorInfoEXT-type-11433");
        vk::WriteResourceDescriptorsEXT(device(), 1u, &resource_desc_info, &descriptors);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, ResourceParameterUniformAlign) {
    TEST_DESCRIPTION("Validate vkWriteResourceDescriptorsEXT for VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize align = m_device->Physical().limits_.minUniformBufferOffsetAlignment;
    const VkDeviceSize bdsize = heap_props.bufferDescriptorSize;
    const auto type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    EXPECT_GT(align, 0u);
    if (align == 1) {
        GTEST_SKIP() << "Cannot be unaligned with align equal 1";
    }

    vkt::Buffer buffer(*m_device, 2 * std::max(align, bdsize), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);

    const VkDeviceSize size = vk::GetPhysicalDeviceDescriptorSizeEXT(Gpu(), type);
    std::vector<uint8_t> data(static_cast<size_t>(size));

    VkDeviceAddressRangeEXT invalid_device_address_range = {0, 0};
    VkResourceDescriptorInfoEXT desc_info = vku::InitStructHelper();
    desc_info.type = type;
    desc_info.data.pAddressRange = &invalid_device_address_range;
    VkHostAddressRangeEXT descriptors = {data.data(), static_cast<size_t>(size)};

    invalid_device_address_range.address = buffer.Address() + 1;
    invalid_device_address_range.size = align;

    m_errorMonitor->SetDesiredError("VUID-VkResourceDescriptorInfoEXT-type-11452");
    vk::WriteResourceDescriptorsEXT(device(), 1u, &desc_info, &descriptors);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, ResourceParameterStorageAlign) {
    TEST_DESCRIPTION("Validate vkWriteResourceDescriptorsEXT for VK_DESCRIPTOR_TYPE_STORAGE_BUFFER");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize align = m_device->Physical().limits_.minStorageBufferOffsetAlignment;
    const VkDeviceSize bdsize = heap_props.bufferDescriptorSize;

    EXPECT_GT(align, 0u);
    if (align == 1) {
        GTEST_SKIP() << "Cannot be unaligned with align equal 1";
    }

    vkt::Buffer buffer(*m_device, 2 * std::max(align, bdsize), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);

    const VkDeviceSize size = vk::GetPhysicalDeviceDescriptorSizeEXT(Gpu(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    std::vector<uint8_t> data(static_cast<size_t>(size));

    VkDeviceAddressRangeEXT invalid_device_address_range = {0, 0};
    VkResourceDescriptorInfoEXT desc_info = vku::InitStructHelper();
    desc_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    desc_info.data.pAddressRange = &invalid_device_address_range;
    VkHostAddressRangeEXT descriptors = {data.data(), static_cast<size_t>(size)};

    invalid_device_address_range.address = buffer.Address() + 1;
    invalid_device_address_range.size = align;

    m_errorMonitor->SetDesiredError("VUID-VkResourceDescriptorInfoEXT-type-11453");
    vk::WriteResourceDescriptorsEXT(device(), 1u, &desc_info, &descriptors);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, ResourceParameterAccelerationStructureAlign) {
    TEST_DESCRIPTION("Validate vkWriteResourceDescriptorsEXT for VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize align = 256;
    const VkDeviceSize bdsize = heap_props.bufferDescriptorSize;

    vkt::Buffer buffer(*m_device, 2 * std::max(align, bdsize), VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
                       vkt::device_address);

    const VkDeviceSize size = vk::GetPhysicalDeviceDescriptorSizeEXT(Gpu(), VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR);
    std::vector<uint8_t> data(static_cast<size_t>(size));

    VkDeviceAddressRangeEXT invalid_device_address_range = {0, 0};
    VkResourceDescriptorInfoEXT desc_info = vku::InitStructHelper();
    desc_info.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    desc_info.data.pAddressRange = &invalid_device_address_range;
    VkHostAddressRangeEXT descriptors = {data.data(), static_cast<size_t>(size)};

    invalid_device_address_range.address = buffer.Address() + 1;
    invalid_device_address_range.size = align;

    m_errorMonitor->SetDesiredError("VUID-VkResourceDescriptorInfoEXT-type-11454");
    vk::WriteResourceDescriptorsEXT(device(), 1u, &desc_info, &descriptors);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, UniformTexelBufferOffsetSingleTexelAlignmentFalse) {
    TEST_DESCRIPTION("Validate vkWriteResourceDescriptorsEXT with uniformTexelBufferOffsetSingleTexelAlignment == VK_FALSE");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT align_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(align_props);

    if (align_props.uniformTexelBufferOffsetSingleTexelAlignment != VK_FALSE) {
        GTEST_SKIP() << "uniformTexelBufferOffsetSingleTexelAlignment required to be VK_FALSE";
    }

    const VkDeviceSize align = align_props.uniformTexelBufferOffsetAlignmentBytes;
    const VkDeviceSize bdsize = heap_props.bufferDescriptorSize;

    EXPECT_GT(align, 0u);
    if (align == 1) {
        GTEST_SKIP() << "Cannot be unaligned with align equal 1";
    }

    vkt::Buffer buffer(*m_device, 2 * std::max(align, bdsize), VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, vkt::device_address);

    VkTexelBufferDescriptorInfoEXT texel_buffer_info = vku::InitStructHelper();
    texel_buffer_info.format = VK_FORMAT_R8G8B8A8_UNORM;

    VkResourceDescriptorInfoEXT desc_info = vku::InitStructHelper();
    desc_info.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    desc_info.data.pTexelBuffer = &texel_buffer_info;

    auto data = std::vector<uint8_t>(static_cast<size_t>(bdsize));
    VkHostAddressRangeEXT descriptors = {&data[0], data.size()};

    texel_buffer_info.addressRange.address = buffer.Address() + 1;
    texel_buffer_info.addressRange.size = align;

    m_errorMonitor->SetDesiredError("VUID-VkResourceDescriptorInfoEXT-type-11214");
    vk::WriteResourceDescriptorsEXT(device(), 1u, &desc_info, &descriptors);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, UniformTexelBufferOffsetSingleTexelAlignmentTrue) {
    TEST_DESCRIPTION("Validate vkWriteResourceDescriptorsEXT with uniformTexelBufferOffsetSingleTexelAlignment == VK_TRUE");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT align_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(align_props);

    if (align_props.uniformTexelBufferOffsetSingleTexelAlignment != VK_TRUE) {
        GTEST_SKIP() << "uniformTexelBufferOffsetSingleTexelAlignment required to be VK_TRUE";
    }

    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    const VkDeviceSize format_size = GetSmallestGreaterOrEquallPowerOfTwo(vkuFormatTexelBlockSize(format));
    const VkDeviceSize align = std::min(align_props.uniformTexelBufferOffsetAlignmentBytes, format_size);
    const VkDeviceSize bdsize = heap_props.bufferDescriptorSize;

    EXPECT_GT(align, 0u);
    if (align == 1) {
        GTEST_SKIP() << "Cannot be unaligned with align equal 1";
    }

    vkt::Buffer buffer(*m_device, 2 * std::max(align, bdsize), VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, vkt::device_address);

    VkTexelBufferDescriptorInfoEXT texel_buffer_info = vku::InitStructHelper();
    texel_buffer_info.format = format;

    VkResourceDescriptorInfoEXT desc_info = vku::InitStructHelper();
    desc_info.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    desc_info.data.pTexelBuffer = &texel_buffer_info;

    auto data = std::vector<uint8_t>(static_cast<size_t>(bdsize));
    VkHostAddressRangeEXT descriptors = {&data[0], data.size()};

    texel_buffer_info.addressRange.address = buffer.Address() + 1;
    texel_buffer_info.addressRange.size = align;

    m_errorMonitor->SetDesiredError("VUID-VkResourceDescriptorInfoEXT-type-11214");
    vk::WriteResourceDescriptorsEXT(device(), 1u, &desc_info, &descriptors);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, StorageTexelBufferOffsetSingleTexelAlignmentFalse) {
    TEST_DESCRIPTION("Validate vkWriteResourceDescriptorsEXT with storageTexelBufferOffsetSingleTexelAlignment == VK_FALSE");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT align_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(align_props);

    if (align_props.storageTexelBufferOffsetSingleTexelAlignment != VK_FALSE) {
        GTEST_SKIP() << "storageTexelBufferOffsetSingleTexelAlignment required to be VK_FALSE";
    }

    const VkDeviceSize align = align_props.storageTexelBufferOffsetAlignmentBytes;
    const VkDeviceSize bdsize = heap_props.bufferDescriptorSize;

    EXPECT_GT(align, 0u);
    if (align == 1) {
        GTEST_SKIP() << "Cannot be unaligned with align equal 1";
    }

    vkt::Buffer buffer(*m_device, 2 * std::max(align, bdsize), VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, vkt::device_address);

    VkTexelBufferDescriptorInfoEXT texel_buffer_info = vku::InitStructHelper();
    texel_buffer_info.format = VK_FORMAT_R8G8B8A8_UNORM;

    VkResourceDescriptorInfoEXT desc_info = vku::InitStructHelper();
    desc_info.type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    desc_info.data.pTexelBuffer = &texel_buffer_info;

    auto data = std::vector<uint8_t>(static_cast<size_t>(bdsize));
    VkHostAddressRangeEXT descriptors = {&data[0], data.size()};

    texel_buffer_info.addressRange.address = buffer.Address() + 1;
    texel_buffer_info.addressRange.size = align;

    m_errorMonitor->SetDesiredError("VUID-VkResourceDescriptorInfoEXT-type-11215");
    vk::WriteResourceDescriptorsEXT(device(), 1u, &desc_info, &descriptors);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, StorageTexelBufferOffsetSingleTexelAlignmentTrue) {
    TEST_DESCRIPTION("Validate vkWriteResourceDescriptorsEXT with storageTexelBufferOffsetSingleTexelAlignment == VK_TRUE");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT align_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(align_props);

    if (align_props.storageTexelBufferOffsetSingleTexelAlignment != VK_TRUE) {
        GTEST_SKIP() << "storageTexelBufferOffsetSingleTexelAlignment required to be VK_TRUE";
    }

    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    const VkDeviceSize format_size = GetSmallestGreaterOrEquallPowerOfTwo(vkuFormatTexelBlockSize(format));
    const VkDeviceSize align = std::min(align_props.storageTexelBufferOffsetAlignmentBytes, format_size);
    const VkDeviceSize bdsize = heap_props.bufferDescriptorSize;

    EXPECT_GT(align, 0u);
    if (align == 1) {
        GTEST_SKIP() << "Cannot be unaligned with align equal 1";
    }

    vkt::Buffer buffer(*m_device, 2 * std::max(align, bdsize), VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, vkt::device_address);

    VkTexelBufferDescriptorInfoEXT texel_buffer_info = vku::InitStructHelper();
    texel_buffer_info.format = format;

    VkResourceDescriptorInfoEXT desc_info = vku::InitStructHelper();
    desc_info.type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    desc_info.data.pTexelBuffer = &texel_buffer_info;

    auto data = std::vector<uint8_t>(static_cast<size_t>(bdsize));
    VkHostAddressRangeEXT descriptors = {&data[0], data.size()};

    texel_buffer_info.addressRange.address = buffer.Address() + 1;
    texel_buffer_info.addressRange.size = align;

    m_errorMonitor->SetDesiredError("VUID-VkResourceDescriptorInfoEXT-type-11215");
    vk::WriteResourceDescriptorsEXT(device(), 1u, &desc_info, &descriptors);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, MinTexelBufferOffsetAlignment) {
    TEST_DESCRIPTION("Validate vkWriteResourceDescriptorsEXT with minTexelBufferOffsetAlignment");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorHeap);
    RETURN_IF_SKIP(Init());

    GetPhysicalDeviceProperties2(heap_props);

    const VkDeviceSize align = m_device->Physical().limits_.minTexelBufferOffsetAlignment;
    const VkDeviceSize bdsize = heap_props.bufferDescriptorSize;

    EXPECT_GT(align, 0u);
    if (align == 1) {
        GTEST_SKIP() << "Cannot be unaligned with align equal 1";
    }

    vkt::Buffer buffer(*m_device, 2 * std::max(align, bdsize), VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, vkt::device_address);

    VkTexelBufferDescriptorInfoEXT texel_buffer_info = vku::InitStructHelper();
    texel_buffer_info.format = VK_FORMAT_R8G8B8A8_UNORM;

    VkResourceDescriptorInfoEXT desc_info = vku::InitStructHelper();
    desc_info.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    desc_info.data.pTexelBuffer = &texel_buffer_info;

    auto data = std::vector<uint8_t>(static_cast<size_t>(bdsize));
    VkHostAddressRangeEXT descriptors = {&data[0], data.size()};

    texel_buffer_info.addressRange.address = buffer.Address() + 1;
    texel_buffer_info.addressRange.size = align;

    m_errorMonitor->SetDesiredError("VUID-VkTexelBufferDescriptorInfoEXT-None-11218");
    vk::WriteResourceDescriptorsEXT(device(), 1u, &desc_info, &descriptors);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, ResourceParameterYCbCr) {
    TEST_DESCRIPTION("Validate vkWriteResourceDescriptorsEXT with YCbCr conversion");
    AddRequiredFeature(vkt::Feature::samplerYcbcrConversion);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDescriptorType types[] = {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                      VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT};

    for (auto type : types) {
        const VkDeviceSize size = vk::GetPhysicalDeviceDescriptorSizeEXT(Gpu(), type);
        if (size == 0) {
            continue;
        }
        // Check if the device can make the image required for this test case.
        VkSamplerYcbcrConversionImageFormatProperties ycbcr_conversion_image_format_props = vku::InitStructHelper();
        VkImageFormatProperties2 format_props = vku::InitStructHelper(&ycbcr_conversion_image_format_props);

        VkPhysicalDeviceImageFormatInfo2 form_info = vku::InitStructHelper();
        form_info.format = VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM;
        form_info.type = VK_IMAGE_TYPE_2D;
        form_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        form_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        VkResult res = vk::GetPhysicalDeviceImageFormatProperties2(Gpu(), &form_info, &format_props);

        // If not, skip this part of the test.
        if (res || ycbcr_conversion_image_format_props.combinedImageSamplerDescriptorCount == 0) {
            continue;
        }

        vkt::Image image(*m_device, 32, 32, form_info.format, form_info.usage);

        auto size2 = size * ycbcr_conversion_image_format_props.combinedImageSamplerDescriptorCount;
        std::vector<uint8_t> data(static_cast<size_t>(size2));

        VkResourceDescriptorInfoEXT resource_desc_info = vku::InitStructHelper();
        resource_desc_info.type = type;

        vkt::SamplerYcbcrConversion conversion(*m_device, VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM);
        VkSamplerYcbcrConversionInfoKHR ycbcr_conversion_info = vku::InitStructHelper();
        ycbcr_conversion_info.conversion = conversion;
        VkImageViewCreateInfo image_view_ci = vku::InitStructHelper(&ycbcr_conversion_info);
        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_ci.format = form_info.format;
        image_view_ci.image = image;
        image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        image_view_ci.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};

        VkImageDescriptorInfoEXT image_info = vku::InitStructHelper();
        image_info.pView = &image_view_ci;
        image_info.layout = VK_IMAGE_LAYOUT_GENERAL;
        resource_desc_info.data.pImage = &image_info;

        VkHostAddressRangeEXT descriptors = {data.data(), static_cast<size_t>(size - 1)};

        m_errorMonitor->SetDesiredError("VUID-vkWriteResourceDescriptorsEXT-pResources-11208");
        if (ycbcr_conversion_image_format_props.combinedImageSamplerDescriptorCount == 1) {
            m_errorMonitor->SetDesiredError("VUID-vkWriteResourceDescriptorsEXT-size-11207");
        }
        vk::WriteResourceDescriptorsEXT(device(), 1u, &resource_desc_info, &descriptors);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, ResourceParameterSubsampledImage) {
    TEST_DESCRIPTION("Validate vkWriteResourceDescriptorsEXT with subsampled image");
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::fragmentDensityMap);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDescriptorType types[] = {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                      VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT};

    for (auto type : types) {
        const VkDeviceSize size = vk::GetPhysicalDeviceDescriptorSizeEXT(Gpu(), type);
        if (size == 0) {
            continue;
        }
        // Check if the device can make the image required for this test case.
        VkSubsampledImageFormatPropertiesEXT subsampled_image_format_props = vku::InitStructHelper();
        VkImageFormatProperties2 format_props = vku::InitStructHelper(&subsampled_image_format_props);

        VkPhysicalDeviceImageFormatInfo2 form_info = vku::InitStructHelper();
        form_info.format = VK_FORMAT_R8G8B8A8_UNORM;
        form_info.type = VK_IMAGE_TYPE_2D;
        form_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        form_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        form_info.flags = VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT;

        VkResult res = vk::GetPhysicalDeviceImageFormatProperties2(Gpu(), &form_info, &format_props);

        // If not, skip this part of the test.
        if (res || subsampled_image_format_props.subsampledImageDescriptorCount == 0) {
            continue;
        }

        VkImageCreateInfo image_create_info = vku::InitStructHelper();
        image_create_info.imageType = form_info.type;
        image_create_info.format = form_info.format;
        image_create_info.extent = {32, 32, 1};
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = form_info.tiling;
        image_create_info.usage = form_info.usage;
        image_create_info.flags = form_info.flags;
        vkt::Image image(*m_device, image_create_info);

        auto size2 = size * subsampled_image_format_props.subsampledImageDescriptorCount;
        std::vector<uint8_t> data(static_cast<size_t>(size2));

        VkResourceDescriptorInfoEXT resource_desc_info = vku::InitStructHelper();
        resource_desc_info.type = type;

        VkSamplerYcbcrConversionInfoKHR ycbcr_conversion_info = vku::InitStructHelper();
        VkImageViewCreateInfo image_view_ci = vku::InitStructHelper(&ycbcr_conversion_info);
        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_ci.format = form_info.format;
        image_view_ci.image = image;
        image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        image_view_ci.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};

        VkImageDescriptorInfoEXT image_info = vku::InitStructHelper();
        image_info.pView = &image_view_ci;
        image_info.layout = VK_IMAGE_LAYOUT_GENERAL;
        resource_desc_info.data.pImage = &image_info;

        VkHostAddressRangeEXT descriptors = {data.data(), static_cast<size_t>(size2 - 1)};

        m_errorMonitor->SetDesiredError("VUID-vkWriteResourceDescriptorsEXT-pResources-11209");
        if (subsampled_image_format_props.subsampledImageDescriptorCount == 1) {
            m_errorMonitor->SetDesiredError("VUID-vkWriteResourceDescriptorsEXT-size-11207");
        }
        vk::WriteResourceDescriptorsEXT(device(), 1u, &resource_desc_info, &descriptors);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, ResourceParameterPViewNull) {
    TEST_DESCRIPTION("Validate vkWriteResourceDescriptorsEXT with pView nullptr");
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDescriptorType types[] = {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                      VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT};

    for (auto type : types) {
        const VkDeviceSize size = vk::GetPhysicalDeviceDescriptorSizeEXT(Gpu(), type);
        ASSERT_TRUE(size != 0);
        std::vector<uint8_t> data(static_cast<size_t>(size));
        vkt::Image image(*m_device, 32, 32, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

        VkResourceDescriptorInfoEXT resource_desc_info = vku::InitStructHelper();
        resource_desc_info.type = type;

        VkImageDescriptorInfoEXT image_info = vku::InitStructHelper();
        image_info.pView = nullptr;
        image_info.layout = VK_IMAGE_LAYOUT_GENERAL;
        resource_desc_info.data.pImage = &image_info;

        VkHostAddressRangeEXT descriptors = {data.data(), static_cast<size_t>(size)};

        m_errorMonitor->SetDesiredError("VUID-VkImageDescriptorInfoEXT-pView-parameter");
        vk::WriteResourceDescriptorsEXT(device(), 1u, &resource_desc_info, &descriptors);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, ResourceParameterPView) {
    TEST_DESCRIPTION("Validate vkWriteResourceDescriptorsEXT triggers all validation paths for pView");
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    std::vector<uint8_t> data(static_cast<size_t>(heap_props.imageDescriptorSize));
    vkt::Image image(*m_device, 32, 32, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT);

    VkResourceDescriptorInfoEXT resource_desc_info = vku::InitStructHelper();
    resource_desc_info.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    VkImageViewCreateInfo image_view_ci = vku::InitStructHelper();
    image_view_ci.format = image.CreateInfo().format;
    image_view_ci.image = image;
    image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    image_view_ci.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};

    VkImageDescriptorInfoEXT image_info = vku::InitStructHelper();
    image_info.pView = &image_view_ci;
    image_info.layout = VK_IMAGE_LAYOUT_GENERAL;
    resource_desc_info.data.pImage = &image_info;

    VkHostAddressRangeEXT descriptors = {data.data(), data.size()};
    {
        // Validate stateless path
        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        image_view_ci.image = image;
        image_view_ci.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
        m_errorMonitor->SetDesiredError("VUID-VkImageViewCreateInfo-viewType-01004");
        vk::WriteResourceDescriptorsEXT(device(), 1u, &resource_desc_info, &descriptors);
        m_errorMonitor->VerifyFound();
    }
    {
        // Validate object tracker path
        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_ci.image = (VkImage)(((uint64_t)image.handle()) + 1);  // Make invalid handle to trigger error
        m_errorMonitor->SetDesiredError("VUID-VkImageViewCreateInfo-image-parameter");
        vk::WriteResourceDescriptorsEXT(device(), 1u, &resource_desc_info, &descriptors);
        m_errorMonitor->VerifyFound();
    }
    {
        // Validate core validation path
        image_view_ci.image = image;
        image.Memory().Destroy();
        m_errorMonitor->SetDesiredError("VUID-VkImageViewCreateInfo-image-01020");
        vk::WriteResourceDescriptorsEXT(device(), 1u, &resource_desc_info, &descriptors);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, CmdBindSamplerHeap) {
    TEST_DESCRIPTION("Validate vkCmdBindSamplerHeapEXT");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize descriptor_size = AlignSampler(2 * heap_props.samplerDescriptorSize);
    const VkDeviceSize heap_size = descriptor_size + heap_props.minSamplerHeapReservedRange;

    vkt::Buffer heap(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    if (heap_props.samplerDescriptorSize + 1 != 0) {
        // reservedRangeOffset check
        VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
        bind_info.heapRange = {heap.Address(), descriptor_size + heap_props.minSamplerHeapReservedRange};
        bind_info.reservedRangeOffset = descriptor_size * 2;
        bind_info.reservedRangeSize = heap_props.minSamplerHeapReservedRange;

        m_command_buffer.Begin();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBindSamplerHeapEXT-pBindInfo-11223");
        if (bind_info.heapRange.size > heap_props.maxSamplerHeapSize) {
            m_errorMonitor->SetDesiredError("VUID-vkCmdBindSamplerHeapEXT-pBindInfo-11225");
        }
        vk::CmdBindSamplerHeapEXT(m_command_buffer, &bind_info);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }

    const bool overflow = heap_props.maxSamplerHeapSize + heap_props.samplerDescriptorSize <
                          (heap_props.maxSamplerHeapSize | heap_props.samplerDescriptorSize);
    const VkDeviceSize max_buffer_size = descriptor_size + heap_props.maxSamplerHeapSize;
    VkPhysicalDeviceVulkan11Properties props11 = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(props11);
    if (!overflow && max_buffer_size <= props11.maxMemoryAllocationSize) {
        VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
        buffer_ci.usage = VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        buffer_ci.size = max_buffer_size;
        vkt::Buffer max_buffer(*m_device, buffer_ci, vkt::no_mem);

        VkMemoryRequirements mem_reqs;
        vk::GetBufferMemoryRequirements(device(), max_buffer, &mem_reqs);

        VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
        alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        auto mem_alloc_info =
            vkt::DeviceMemory::GetResourceAllocInfo(*m_device, mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_flags);
        vkt::DeviceMemory mem(*m_device, mem_alloc_info);
        if (!mem.initialized()) {
            GTEST_SKIP() << "Can't allocte memory over maxResourceHeapSize";
        }

        vk::BindBufferMemory(device(), max_buffer, mem, 0);

        // reservedRangeOffset check
        VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
        bind_info.heapRange = {max_buffer.Address(), max_buffer_size};
        bind_info.reservedRangeOffset = 0;
        bind_info.reservedRangeSize = heap_props.minSamplerHeapReservedRange;

        m_command_buffer.Begin();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBindSamplerHeapEXT-pBindInfo-11225");
        vk::CmdBindSamplerHeapEXT(m_command_buffer, &bind_info);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }
}

TEST_F(NegativeDescriptorHeap, CmdBindSamplerHeapReservedRangeSize) {
    TEST_DESCRIPTION("Validate vkCmdBindSamplerHeapEXT ReservedRangeSize is greater or equal minSamplerHeapReservedRange");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    if (heap_props.minSamplerHeapReservedRange == 0) {
        GTEST_SKIP() << "Cannot run when minSamplerHeapReservedRange is zero";
    }

    const VkDeviceSize heap_size = 2 * heap_props.samplerDescriptorSize + heap_props.minSamplerHeapReservedRange;
    vkt::Buffer heap(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
    bind_info.heapRange = {heap.Address(), heap_size};
    bind_info.reservedRangeSize = heap_props.minSamplerHeapReservedRange - 1;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindSamplerHeapEXT-pBindInfo-11224");
    vk::CmdBindSamplerHeapEXT(m_command_buffer, &bind_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptorHeap, CmdBindSamplerHeapAlign) {
    TEST_DESCRIPTION("Validate vkCmdBindSamplerHeapEXT addr alignement");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize heap_size = 2 * heap_props.samplerDescriptorSize + heap_props.minSamplerHeapReservedRange;

    EXPECT_GT(heap_props.samplerHeapAlignment, 0u);
    if (heap_props.samplerHeapAlignment == 1) {
        GTEST_SKIP() << "Cannot be unaligned with align equal 1";
    }

    vkt::Buffer heap(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
    bind_info.heapRange = {heap.Address() + 1, heap_size - 1};
    bind_info.reservedRangeOffset = 0;
    bind_info.reservedRangeSize = heap_props.minSamplerHeapReservedRange;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindSamplerHeapEXT-pBindInfo-11226");
    vk::CmdBindSamplerHeapEXT(m_command_buffer, &bind_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptorHeap, CmdBindSamplerHeapReservedRangeAlign) {
    TEST_DESCRIPTION("Validate vkCmdBindSamplerHeapEXT reservedRange alignement");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize heap_size = 2 * std::max(heap_props.samplerDescriptorSize, heap_props.minSamplerHeapReservedRange);

    vkt::Buffer heap(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
    bind_info.heapRange = {heap.Address(), heap_size};
    bind_info.reservedRangeOffset = 1;
    bind_info.reservedRangeSize = heap_size / 2;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindSamplerHeapEXT-pBindInfo-11434");
    vk::CmdBindSamplerHeapEXT(m_command_buffer, &bind_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptorHeap, CmdBindSamplerHeapSecondaryBuffer) {
    TEST_DESCRIPTION("Validate vkCmdBindSamplerHeapEXT command written to secondary buffer");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize heap_size = heap_props.samplerDescriptorSize + heap_props.minSamplerHeapReservedRange;
    vkt::Buffer heap(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceDescriptorHeapInfoEXT inh_desc_heap_info = vku::InitStructHelper();
    VkBindHeapInfoEXT samplerHeapBindInfo = vku::InitStructHelper();
    inh_desc_heap_info.pSamplerHeapBindInfo = &samplerHeapBindInfo;

    VkCommandBufferInheritanceInfo inh = vku::InitStructHelper(&inh_desc_heap_info);
    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    cbbi.pInheritanceInfo = &inh;

    secondary.Begin(&cbbi);

    VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
    bind_info.heapRange = {heap.Address(), heap_size};
    bind_info.reservedRangeOffset = heap_props.samplerDescriptorSize;
    bind_info.reservedRangeSize = heap_props.minSamplerHeapReservedRange;

    if (bind_info.reservedRangeOffset % heap_props.samplerDescriptorSize != 0) {
        m_errorMonitor->SetDesiredError("VUID-vkCmdBindSamplerHeapEXT-pBindInfo-11434");
    }

    m_errorMonitor->SetDesiredError("VUID-vkCmdBindSamplerHeapEXT-commandBuffer-11231");
    vk::CmdBindSamplerHeapEXT(secondary, &bind_info);
    m_errorMonitor->VerifyFound();

    secondary.End();
}

TEST_F(NegativeDescriptorHeap, CmdBindResourceHeap) {
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize app_size = AlignResource(heap_props.bufferDescriptorSize);
    const VkDeviceSize heap_size = app_size + heap_props.minResourceHeapReservedRange;
    vkt::Buffer heap(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    {
        // reservedRangeOffset check
        VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
        bind_info.heapRange = {heap.Address(), heap_size};
        bind_info.reservedRangeOffset = app_size;
        bind_info.reservedRangeSize = heap_size;

        m_command_buffer.Begin();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBindResourceHeapEXT-pBindInfo-11232");
        vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_info);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }

    if (heap_props.minResourceHeapReservedRange > 0) {
        // size check
        VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
        bind_info.heapRange = {heap.Address(), heap_size};
        bind_info.reservedRangeSize = heap_props.minResourceHeapReservedRange - 1;

        m_command_buffer.Begin();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBindResourceHeapEXT-pBindInfo-11233");
        vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_info);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }
    const bool overflow = heap_props.maxResourceHeapSize + heap_props.bufferDescriptorSize <
                          (heap_props.maxResourceHeapSize | heap_props.bufferDescriptorSize);
    const auto max_buffer_size = heap_props.maxResourceHeapSize + heap_props.bufferDescriptorSize;
    VkPhysicalDeviceVulkan11Properties props11 = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(props11);
    if (!overflow && max_buffer_size <= props11.maxMemoryAllocationSize) {
        VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
        buffer_ci.usage = VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        buffer_ci.size = max_buffer_size;
        vkt::Buffer max_buffer(*m_device, buffer_ci, vkt::no_mem);

        VkMemoryRequirements mem_reqs;
        vk::GetBufferMemoryRequirements(device(), max_buffer, &mem_reqs);

        VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
        alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        auto mem_alloc_info =
            vkt::DeviceMemory::GetResourceAllocInfo(*m_device, mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_flags);
        vkt::DeviceMemory mem(*m_device, mem_alloc_info);
        if (!mem.initialized()) {
            GTEST_SKIP() << "Can't allocte memory over maxResourceHeapSize";
        }

        vk::BindBufferMemory(device(), max_buffer, mem, 0);

        // reservedRangeOffset check
        VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
        bind_info.heapRange = {max_buffer.Address(), heap_props.maxResourceHeapSize + heap_props.bufferDescriptorSize};
        bind_info.reservedRangeOffset = 0;
        bind_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;

        m_command_buffer.Begin();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBindResourceHeapEXT-pBindInfo-11234");
        vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_info);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }
}

TEST_F(NegativeDescriptorHeap, SamplerInheritance) {
    TEST_DESCRIPTION("Validate that inherited ranges match primary buffer");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize heap_size = heap_props.minSamplerHeapReservedRange + 2 * heap_props.samplerDescriptorSize;
    vkt::Buffer heap(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    {
        VkCommandBufferInheritanceDescriptorHeapInfoEXT inh_desc_heap_info = vku::InitStructHelper();
        VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
        bind_info.heapRange = {heap.Address(), heap_size / 2};
        bind_info.reservedRangeOffset = 0;
        bind_info.reservedRangeSize = heap_props.minSamplerHeapReservedRange;
        inh_desc_heap_info.pSamplerHeapBindInfo = &bind_info;

        VkCommandBufferInheritanceInfo inh = vku::InitStructHelper(&inh_desc_heap_info);
        VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
        cbbi.pInheritanceInfo = &inh;

        secondary.Begin(&cbbi);
        secondary.End();
    }
    {
        m_command_buffer.Begin();

        VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
        bind_info.heapRange = {heap.Address(), heap_size};
        bind_info.reservedRangeOffset = 0;
        bind_info.reservedRangeSize = heap_props.minSamplerHeapReservedRange;

        vk::CmdBindSamplerHeapEXT(m_command_buffer, &bind_info);

        m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-commandBuffer-11351");
        vk::CmdExecuteCommands(m_command_buffer, 1, &secondary.handle());
        m_errorMonitor->VerifyFound();

        m_command_buffer.End();
    }
}

TEST_F(NegativeDescriptorHeap, ResourceInheritance) {
    TEST_DESCRIPTION("Validate that inherited ranges match primary buffer");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize heap_size = heap_props.minResourceHeapReservedRange + 2 * heap_props.bufferDescriptorSize;
    vkt::Buffer heap(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    {
        VkCommandBufferInheritanceDescriptorHeapInfoEXT inh_desc_heap_info = vku::InitStructHelper();
        VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
        bind_info.heapRange = {heap.Address(), heap_size / 2};
        bind_info.reservedRangeOffset = 0;
        bind_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;
        inh_desc_heap_info.pResourceHeapBindInfo = &bind_info;

        VkCommandBufferInheritanceInfo inh = vku::InitStructHelper(&inh_desc_heap_info);
        VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
        cbbi.pInheritanceInfo = &inh;

        secondary.Begin(&cbbi);
        secondary.End();
    }
    {
        m_command_buffer.Begin();

        VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
        bind_info.heapRange = {heap.Address(), heap_size};
        bind_info.reservedRangeOffset = 0;
        bind_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;

        vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_info);

        m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-commandBuffer-11352");
        vk::CmdExecuteCommands(m_command_buffer, 1, &secondary.handle());
        m_errorMonitor->VerifyFound();

        m_command_buffer.End();
    }
}

TEST_F(NegativeDescriptorHeap, CmdBindResourceHeapAlign) {
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    EXPECT_GT(heap_props.resourceHeapAlignment, 0u);
    if (heap_props.resourceHeapAlignment == 1) {
        GTEST_SKIP() << "Cannot be unaligned with align equal 1";
    }
    if (heap_props.minResourceHeapReservedRange == 0) {
        GTEST_SKIP() << "Test requires minResourceHeapReservedRange to not be 0";
    }

    const VkDeviceSize heap_size = heap_props.minResourceHeapReservedRange * 2;
    vkt::Buffer heap(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
    bind_info.heapRange = {heap.Address() + 1, heap_size - 1};
    bind_info.reservedRangeOffset = 0;
    bind_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindResourceHeapEXT-pBindInfo-11235");
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptorHeap, CmdBindResourceHeapReservedRangeAlign) {
    TEST_DESCRIPTION("Validate vkCmdBindResourceHeapEXT reservedRangeOffset alignment");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize buf_img_size = std::max(heap_props.bufferDescriptorAlignment, heap_props.imageDescriptorAlignment);
    const VkDeviceSize heap_size = 2 * std::max(buf_img_size, heap_props.minResourceHeapReservedRange);
    vkt::Buffer heap(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    // reservedRangeOffset alignment check
    VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
    bind_info.heapRange = {heap.Address(), heap_size};
    bind_info.reservedRangeOffset = 1;
    bind_info.reservedRangeSize = heap_size / 2;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindResourceHeapEXT-pBindInfo-11435");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindResourceHeapEXT-pBindInfo-11436");
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptorHeap, CmdBindResourceHeapSecondaryBuffer) {
    TEST_DESCRIPTION("Validate vkCmdBindResourceHeapEXT command written to secondary buffer");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize descriptor_size = AlignResource(heap_props.bufferDescriptorSize);
    const VkDeviceSize heap_size = descriptor_size + heap_props.minResourceHeapReservedRange;
    vkt::Buffer heap(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceDescriptorHeapInfoEXT inh_desc_heap_info = vku::InitStructHelper();
    VkBindHeapInfoEXT resourceHeapBindInfo = vku::InitStructHelper();
    inh_desc_heap_info.pResourceHeapBindInfo = &resourceHeapBindInfo;

    VkCommandBufferInheritanceInfo inh = vku::InitStructHelper(&inh_desc_heap_info);
    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    cbbi.pInheritanceInfo = &inh;

    secondary.Begin(&cbbi);

    VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
    bind_info.heapRange = {heap.Address(), heap_size};
    bind_info.reservedRangeOffset = descriptor_size;
    bind_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;

    m_errorMonitor->SetDesiredError("VUID-vkCmdBindResourceHeapEXT-commandBuffer-11238");
    vk::CmdBindResourceHeapEXT(secondary, &bind_info);
    m_errorMonitor->VerifyFound();

    secondary.End();
}

TEST_F(NegativeDescriptorHeap, CmdBindResourceHeapSecondaryBufferMemoryTests) {
    TEST_DESCRIPTION("Validate memory overflow corruption and binding written to secondary buffer");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceDescriptorHeapInfoEXT inh_desc_heap_info = vku::InitStructHelper();

    VkCommandBufferInheritanceInfo inh = vku::InitStructHelper(&inh_desc_heap_info);
    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    cbbi.pInheritanceInfo = &inh;

    {
        secondary.Begin(&cbbi);

        const VkDeviceSize heap_size = AlignSampler(heap_props.samplerDescriptorSize) + heap_props.minSamplerHeapReservedRange;
        vkt::Buffer heap(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

        VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
        bind_info.heapRange = {heap.Address(), heap_size + heap_props.samplerDescriptorAlignment};
        bind_info.reservedRangeOffset = heap_props.samplerDescriptorAlignment;
        bind_info.reservedRangeSize = heap_props.minSamplerHeapReservedRange;

        m_errorMonitor->SetDesiredError("VUID-VkDeviceAddressRangeEXT-address-11365");
        vk::CmdBindSamplerHeapEXT(secondary, &bind_info);
        m_errorMonitor->VerifyFound();

        secondary.End();
    }
    {
        secondary.Begin(&cbbi);

        const VkDeviceSize heap_size = AlignSampler(heap_props.samplerDescriptorSize) + heap_props.minSamplerHeapReservedRange;
        vkt::Buffer heap(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

        VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
        bind_info.heapRange = {heap.Address(), heap_size};
        bind_info.reservedRangeOffset = heap_props.samplerDescriptorAlignment;
        bind_info.reservedRangeSize = heap_props.minSamplerHeapReservedRange;

        heap.Memory().Destroy();

        m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-None-10894");
        vk::CmdBindSamplerHeapEXT(secondary, &bind_info);
        m_errorMonitor->VerifyFound();

        secondary.End();
    }

    {
        secondary.Begin(&cbbi);

        const VkDeviceSize heap_size = heap_props.bufferDescriptorSize + heap_props.minResourceHeapReservedRange;
        vkt::Buffer heap(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

        VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
        bind_info.heapRange = {heap.Address(), heap_size + heap_props.resourceHeapAlignment};
        bind_info.reservedRangeOffset = 0;
        bind_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;

        m_errorMonitor->SetDesiredError("VUID-VkDeviceAddressRangeEXT-address-11365");
        vk::CmdBindResourceHeapEXT(secondary, &bind_info);
        m_errorMonitor->VerifyFound();

        secondary.End();
    }
    {
        secondary.Begin(&cbbi);

        const VkDeviceSize heap_size = heap_props.bufferDescriptorSize + heap_props.minResourceHeapReservedRange;
        vkt::Buffer heap(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

        VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
        bind_info.heapRange = {heap.Address(), heap_size};
        bind_info.reservedRangeOffset = 0;
        bind_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;

        heap.Memory().Destroy();

        m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-None-10894");
        vk::CmdBindResourceHeapEXT(secondary, &bind_info);
        m_errorMonitor->VerifyFound();

        secondary.End();
    }
}

TEST_F(NegativeDescriptorHeap, CmdPushData) {
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    std::vector<uint8_t> payload(static_cast<size_t>(4 * heap_props.maxPushDataSize));
    VkPushDataInfoEXT push_data_info = vku::InitStructHelper();
    push_data_info.data.address = payload.data();
    m_command_buffer.Begin();
    {
        push_data_info.data.size = payload.size();
        push_data_info.offset = (uint32_t)heap_props.maxPushDataSize;
        m_errorMonitor->SetDesiredError("VUID-VkPushDataInfoEXT-offset-11243");
        vk::CmdPushDataEXT(m_command_buffer, &push_data_info);
        m_errorMonitor->VerifyFound();
    }
    {
        push_data_info.data.size = 0;
        push_data_info.offset = 0;
        m_errorMonitor->SetDesiredError("VUID-VkHostAddressRangeConstEXT-size-arraylength");
        vk::CmdPushDataEXT(m_command_buffer, &push_data_info);
        m_errorMonitor->VerifyFound();
    }
    {
        push_data_info.data.size = (uint32_t)heap_props.maxPushDataSize / 2;
        push_data_info.offset = 1;
        m_errorMonitor->SetDesiredError("VUID-VkPushDataInfoEXT-offset-11418");
        vk::CmdPushDataEXT(m_command_buffer, &push_data_info);
        m_errorMonitor->VerifyFound();
    }
    {
        push_data_info.data.size = 1 + (uint32_t)heap_props.maxPushDataSize / 2;
        push_data_info.offset = 0;
        m_errorMonitor->SetDesiredError("VUID-VkPushDataInfoEXT-data-11419");
        vk::CmdPushDataEXT(m_command_buffer, &push_data_info);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, ShaderDescriptorSetAndBindingMapping) {
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    InitRenderTarget();

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    VkDescriptorSetAndBindingMappingEXT mapping[2];
    mapping_info.mappingCount = 2;
    mapping_info.pMappings = mapping;

    {
        // Same range
        mapping[0] = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_UNIFORM_BUFFER_BIT_EXT);
        mapping[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
        mapping[1] = mapping[0];

        CreatePipelineHelper pipe(*this);

        pipe.shader_stages_[0].pNext = &mapping_info;

        m_errorMonitor->SetDesiredError("VUID-VkShaderDescriptorSetAndBindingMappingInfoEXT-pMappings-11244");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }

    {
        // Overlapping subrange
        mapping[0] = MakeSetAndBindingMapping(0, 1, 10, VK_SPIRV_RESOURCE_TYPE_SAMPLER_BIT_EXT);
        mapping[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
        mapping[1] = MakeSetAndBindingMapping(0, 2, 5, VK_SPIRV_RESOURCE_TYPE_SAMPLER_BIT_EXT);
        mapping[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;

        CreatePipelineHelper pipe(*this);

        pipe.shader_stages_[0].pNext = &mapping_info;

        m_errorMonitor->SetDesiredError("VUID-VkShaderDescriptorSetAndBindingMappingInfoEXT-pMappings-11244");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }

    {
        // Overlapping range
        mapping[0] = MakeSetAndBindingMapping(1, 0, 10, VK_SPIRV_RESOURCE_TYPE_SAMPLER_BIT_EXT);
        mapping[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
        mapping[1] = MakeSetAndBindingMapping(1, 2, 10, VK_SPIRV_RESOURCE_TYPE_SAMPLER_BIT_EXT);
        mapping[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;

        CreatePipelineHelper pipe(*this);

        pipe.shader_stages_[0].pNext = &mapping_info;

        m_errorMonitor->SetDesiredError("VUID-VkShaderDescriptorSetAndBindingMappingInfoEXT-pMappings-11244");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }

    {
        // Overlapping range with UINT32_MAX bindingCount
        mapping[0] = MakeSetAndBindingMapping(2, 20, UINT32_MAX, VK_SPIRV_RESOURCE_TYPE_SAMPLER_BIT_EXT);
        mapping[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
        mapping[1] = MakeSetAndBindingMapping(2, 30, 10, VK_SPIRV_RESOURCE_TYPE_SAMPLER_BIT_EXT);
        mapping[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;

        CreatePipelineHelper pipe(*this);

        pipe.shader_stages_[0].pNext = &mapping_info;

        m_errorMonitor->SetDesiredError("VUID-VkShaderDescriptorSetAndBindingMappingInfoEXT-pMappings-11244");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }

    {
        // No overlapping
        mapping[0] = MakeSetAndBindingMapping(1, 0, 1, VK_SPIRV_RESOURCE_TYPE_SAMPLER_BIT_EXT);
        mapping[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
        mapping[1] = MakeSetAndBindingMapping(1, 1, 1, VK_SPIRV_RESOURCE_TYPE_SAMPLER_BIT_EXT);
        mapping[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;

        CreateComputePipelineHelper pipe(*this);
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        m_errorMonitor->Reset();
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, DescriptorSetAndBindingMappingPipeline) {
    TEST_DESCRIPTION("Validate VkDescriptorSetAndBindingMappingEXT structure using pipeline");
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkDescriptorSetAndBindingMappingEXT mapping = {};

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    for (auto source : {VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_DATA_EXT, VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT,
                        VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT, VK_DESCRIPTOR_MAPPING_SOURCE_RESOURCE_HEAP_DATA_EXT}) {
        mapping = MakeSetAndBindingMapping(0, 0, 2, VK_SPIRV_RESOURCE_TYPE_UNIFORM_BUFFER_BIT_EXT);
        mapping.source = source;

        CreateComputePipelineHelper pipe(*this);
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetAndBindingMappingEXT-source-11245");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }

    for (auto source : {VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_DATA_EXT, VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT}) {
        bool data = source == VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_DATA_EXT;
        mapping = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_UNIFORM_BUFFER_BIT_EXT);
        mapping.source = source;
        if (data) {
            mapping.sourceData.pushDataOffset = 3;
        } else {
            mapping.sourceData.pushAddressOffset = 7;
        }

        CreateComputePipelineHelper pipe(*this);
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        m_errorMonitor->SetDesiredError(data ? "VUID-VkDescriptorSetAndBindingMappingEXT-source-11246"
                                             : "VUID-VkDescriptorSetAndBindingMappingEXT-source-11247");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }

    for (auto source :
         {VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_DATA_EXT, VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_ADDRESS_EXT}) {
        mapping = MakeSetAndBindingMapping(0, 0, 2, VK_SPIRV_RESOURCE_TYPE_UNIFORM_BUFFER_BIT_EXT);
        mapping.source = source;
        mapping.sourceData.shaderRecordAddressOffset = 8;

        CreateComputePipelineHelper pipe(*this);
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetAndBindingMappingEXT-source-11248");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }

    for (auto source :
         {VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_DATA_EXT, VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_ADDRESS_EXT}) {
        mapping = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_UNIFORM_BUFFER_BIT_EXT);
        bool data = source == VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_DATA_EXT;
        mapping.source = source;
        if (data) {
            mapping.sourceData.shaderRecordDataOffset = 3;
        } else {
            mapping.sourceData.shaderRecordAddressOffset = 7;
        }

        CreateComputePipelineHelper pipe(*this);
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        m_errorMonitor->SetDesiredError(data ? "VUID-VkDescriptorSetAndBindingMappingEXT-source-11249"
                                             : "VUID-VkDescriptorSetAndBindingMappingEXT-source-11250");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }

    for (auto source : {VK_DESCRIPTOR_MAPPING_SOURCE_RESOURCE_HEAP_DATA_EXT, VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_DATA_EXT,
                        VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_DATA_EXT}) {
        mapping = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_COMBINED_SAMPLED_IMAGE_BIT_EXT);
        mapping.source = source;
        mapping.sourceData.shaderRecordDataOffset = 0;

        CreateComputePipelineHelper pipe(*this);
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetAndBindingMappingEXT-source-11356");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }

    for (auto source : {VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_ADDRESS_EXT, VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT}) {
        mapping = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_COMBINED_SAMPLED_IMAGE_BIT_EXT);
        mapping.source = source;
        mapping.sourceData.shaderRecordDataOffset = 0;

        CreateComputePipelineHelper pipe(*this);
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetAndBindingMappingEXT-source-11357");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }

    for (auto source :
         {VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_SHADER_RECORD_INDEX_EXT, VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT,
          VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT,
          VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT}) {
        mapping = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_ONLY_IMAGE_BIT_EXT);
        mapping.source = source;

        switch (source) {
            case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_SHADER_RECORD_INDEX_EXT:
                mapping.sourceData.shaderRecordIndex.useCombinedImageSamplerIndex = VK_TRUE;
                break;
            case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT:
                mapping.sourceData.pushIndex.useCombinedImageSamplerIndex = VK_TRUE;
                break;
            case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT:
                mapping.sourceData.indirectIndex.useCombinedImageSamplerIndex = VK_TRUE;
                break;
            case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT:
                mapping.sourceData.indirectIndexArray.useCombinedImageSamplerIndex = VK_TRUE;
                break;
            default:
                assert(0);
        }

        CreateComputePipelineHelper pipe(*this);
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetAndBindingMappingEXT-source-11358");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }

    for (auto source :
         {VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT, VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT,
          VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_SHADER_RECORD_INDEX_EXT, VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT,
          VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT}) {
        mapping = MakeSetAndBindingMapping(0, 0, 2, VK_SPIRV_RESOURCE_TYPE_READ_ONLY_IMAGE_BIT_EXT);
        mapping.source = source;

        VkSamplerCreateInfo embedded_sampler = vku::InitStructHelper();

        switch (source) {
            case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT:
                mapping.sourceData.constantOffset.pEmbeddedSampler = &embedded_sampler;
                break;
            case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_SHADER_RECORD_INDEX_EXT:
                mapping.sourceData.shaderRecordIndex.pEmbeddedSampler = &embedded_sampler;
                break;
            case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT:
                mapping.sourceData.pushIndex.pEmbeddedSampler = &embedded_sampler;
                break;
            case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT:
                mapping.sourceData.indirectIndex.pEmbeddedSampler = &embedded_sampler;
                break;
            case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT:
                mapping.sourceData.indirectIndexArray.pEmbeddedSampler = &embedded_sampler;
                break;
            default:
                assert(0);
        }

        CreateComputePipelineHelper pipe(*this);
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetAndBindingMappingEXT-source-11389");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }

    for (auto source :
         {VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT, VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT,
          VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_SHADER_RECORD_INDEX_EXT, VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT,
          VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT}) {
        mapping = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_ONLY_IMAGE_BIT_EXT);
        mapping.source = source;

        VkDebugUtilsObjectNameInfoEXT object_name_info = vku::InitStructHelper();
        object_name_info.objectType = VK_OBJECT_TYPE_DEVICE;

        VkSamplerCreateInfo embedded_sampler = vku::InitStructHelper(&object_name_info);
        const char* vuid = nullptr;

        switch (source) {
            case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT:
                mapping.sourceData.constantOffset.pEmbeddedSampler = &embedded_sampler;
                vuid = "VUID-VkDescriptorMappingSourceConstantOffsetEXT-pEmbeddedSampler-11415";
                break;
            case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_SHADER_RECORD_INDEX_EXT:
                mapping.sourceData.shaderRecordIndex.pEmbeddedSampler = &embedded_sampler;
                vuid = "VUID-VkDescriptorMappingSourceShaderRecordIndexEXT-pEmbeddedSampler-11405";
                break;
            case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT:
                mapping.sourceData.pushIndex.pEmbeddedSampler = &embedded_sampler;
                vuid = "VUID-VkDescriptorMappingSourcePushIndexEXT-pEmbeddedSampler-11402";
                break;
            case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT:
                mapping.sourceData.indirectIndex.pEmbeddedSampler = &embedded_sampler;
                vuid = "VUID-VkDescriptorMappingSourceIndirectIndexEXT-pEmbeddedSampler-11403";
                break;
            case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT:
                mapping.sourceData.indirectIndexArray.pEmbeddedSampler = &embedded_sampler;
                vuid = "VUID-VkDescriptorMappingSourceIndirectIndexArrayEXT-pEmbeddedSampler-11404";
                break;
            default:
                assert(0);
        }

        CreateComputePipelineHelper pipe(*this);
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        m_errorMonitor->SetDesiredError(vuid);
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }

    for (auto source :
         {VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT, VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT,
          VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_SHADER_RECORD_INDEX_EXT, VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT,
          VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT}) {
        mapping = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_ONLY_IMAGE_BIT_EXT);
        mapping.source = source;

        for (const auto borderColor : {VK_BORDER_COLOR_FLOAT_CUSTOM_EXT, VK_BORDER_COLOR_INT_CUSTOM_EXT}) {
            VkSamplerCreateInfo embedded_sampler = vku::InitStructHelper();
            embedded_sampler.borderColor = borderColor;
            const char* vuid = nullptr;

            switch (source) {
                case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT:
                    mapping.sourceData.constantOffset.pEmbeddedSampler = &embedded_sampler;
                    vuid = "VUID-VkDescriptorMappingSourceConstantOffsetEXT-pEmbeddedSampler-11445";
                    break;
                case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_SHADER_RECORD_INDEX_EXT:
                    mapping.sourceData.shaderRecordIndex.pEmbeddedSampler = &embedded_sampler;
                    vuid = "VUID-VkDescriptorMappingSourceShaderRecordIndexEXT-pEmbeddedSampler-11449";
                    break;
                case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT:
                    mapping.sourceData.pushIndex.pEmbeddedSampler = &embedded_sampler;
                    vuid = "VUID-VkDescriptorMappingSourcePushIndexEXT-pEmbeddedSampler-11446";
                    break;
                case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT:
                    mapping.sourceData.indirectIndex.pEmbeddedSampler = &embedded_sampler;
                    vuid = "VUID-VkDescriptorMappingSourceIndirectIndexEXT-pEmbeddedSampler-11447";
                    break;
                case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT:
                    mapping.sourceData.indirectIndexArray.pEmbeddedSampler = &embedded_sampler;
                    vuid = "VUID-VkDescriptorMappingSourceIndirectIndexArrayEXT-pEmbeddedSampler-11448";
                    break;
                default:
                    assert(0);
            }

            CreateComputePipelineHelper pipe(*this);
            pipe.LateBindPipelineInfo();
            pipe.cp_ci_.stage.pNext = &mapping_info;

            m_errorMonitor->SetDesiredError(vuid);
            pipe.CreateComputePipeline(false);
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(NegativeDescriptorHeap, DescriptorSetAndBindingMappingShaderObject) {
    TEST_DESCRIPTION("Validate VkDescriptorSetAndBindingMappingEXT structure using shader object");
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkDescriptorSetAndBindingMappingEXT mapping = {};
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;
    for (auto source :
         {VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT, VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT,
          VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_SHADER_RECORD_INDEX_EXT, VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT,
          VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT}) {
        mapping = MakeSetAndBindingMapping(0, 0, 2, VK_SPIRV_RESOURCE_TYPE_READ_ONLY_IMAGE_BIT_EXT);
        mapping.source = source;

        VkSamplerCreateInfo embedded_sampler = vku::InitStructHelper();

        switch (source) {
            case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT:
                mapping.sourceData.constantOffset.pEmbeddedSampler = &embedded_sampler;
                break;
            case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_SHADER_RECORD_INDEX_EXT:
                mapping.sourceData.shaderRecordIndex.pEmbeddedSampler = &embedded_sampler;
                break;
            case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT:
                mapping.sourceData.pushIndex.pEmbeddedSampler = &embedded_sampler;
                break;
            case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT:
                mapping.sourceData.indirectIndex.pEmbeddedSampler = &embedded_sampler;
                break;
            case VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT:
                mapping.sourceData.indirectIndexArray.pEmbeddedSampler = &embedded_sampler;
                break;
            default:
                assert(0);
        }

        const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
        const VkShaderCreateFlagsEXT flags = VK_SHADER_CREATE_DESCRIPTOR_HEAP_BIT_EXT;
        VkShaderCreateInfoEXT vert_ci = ShaderCreateInfoFlag(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, flags);
        vert_ci.pNext = &mapping_info;

        m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetAndBindingMappingEXT-source-11389");
        const vkt::Shader vertShader(*m_device, vert_ci);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, DescriptorMappingSourcePushIndex) {
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    VkDescriptorSetAndBindingMappingEXT mapping =
        MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_ONLY_STORAGE_BUFFER_BIT_EXT);
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;
    {
        mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT;
        mapping.sourceData.pushIndex.pushOffset = 3;

        CreateComputePipelineHelper pipe(*this);
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        m_errorMonitor->SetDesiredError("VUID-VkDescriptorMappingSourcePushIndexEXT-pushOffset-11258");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }
    {
        mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT;
        mapping.sourceData.pushIndex.pushOffset = (uint32_t)heap_props.maxPushDataSize;

        CreateComputePipelineHelper pipe(*this);
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        m_errorMonitor->SetDesiredError("VUID-VkDescriptorMappingSourcePushIndexEXT-pushOffset-11259");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, DescriptorMappingSourceIndirectIndex) {
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkDescriptorSetAndBindingMappingEXT mapping = {};
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;
    {
        mapping = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_ONLY_STORAGE_BUFFER_BIT_EXT);
        mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT;
        mapping.sourceData.indirectIndex.pushOffset = 4;

        CreateComputePipelineHelper pipe(*this);
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        m_errorMonitor->SetDesiredError("VUID-VkDescriptorMappingSourceIndirectIndexEXT-pushOffset-11260");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }
    {
        mapping = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_ONLY_STORAGE_BUFFER_BIT_EXT);
        mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT;
        mapping.sourceData.indirectIndex.pushOffset = (uint32_t)heap_props.maxPushDataSize;

        CreateComputePipelineHelper pipe(*this);
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        m_errorMonitor->SetDesiredError("VUID-VkDescriptorMappingSourceIndirectIndexEXT-pushOffset-11261");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }
    {
        mapping = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_ONLY_STORAGE_BUFFER_BIT_EXT);
        mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT;
        mapping.sourceData.indirectIndex.addressOffset = 3;

        CreateComputePipelineHelper pipe(*this);
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        m_errorMonitor->SetDesiredError("VUID-VkDescriptorMappingSourceIndirectIndexEXT-addressOffset-11262");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, DescriptorMappingSourceHeapData) {
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkDescriptorSetAndBindingMappingEXT mapping = {};
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    if (m_device->Physical().limits_.minUniformBufferOffsetAlignment > 0) {
        mapping = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_UNIFORM_BUFFER_BIT_EXT);
        mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_RESOURCE_HEAP_DATA_EXT;
        mapping.sourceData.heapData.heapOffset = (uint32_t)m_device->Physical().limits_.minUniformBufferOffsetAlignment + 1;

        CreateComputePipelineHelper pipe(*this);
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        m_errorMonitor->SetDesiredError("VUID-VkDescriptorMappingSourceHeapDataEXT-heapOffset-11263");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }
    {
        mapping = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_UNIFORM_BUFFER_BIT_EXT);
        mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_RESOURCE_HEAP_DATA_EXT;
        mapping.sourceData.heapData.pushOffset = 3u;

        CreateComputePipelineHelper pipe(*this);
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        m_errorMonitor->SetDesiredError("VUID-VkDescriptorMappingSourceHeapDataEXT-pushOffset-11264");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }
    {
        mapping = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_UNIFORM_BUFFER_BIT_EXT);
        mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_RESOURCE_HEAP_DATA_EXT;
        mapping.sourceData.heapData.pushOffset = (uint32_t)heap_props.maxPushDataSize;

        CreateComputePipelineHelper pipe(*this);
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        m_errorMonitor->SetDesiredError("VUID-VkDescriptorMappingSourceHeapDataEXT-pushOffset-11265");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, DescriptorMappingSourceIndirectAddress) {
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkDescriptorSetAndBindingMappingEXT mapping = {};
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;
    {
        mapping = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_ONLY_STORAGE_BUFFER_BIT_EXT);
        mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT;
        mapping.sourceData.indirectAddress.pushOffset = 4;

        CreateComputePipelineHelper pipe(*this);
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        m_errorMonitor->SetDesiredError("VUID-VkDescriptorMappingSourceIndirectAddressEXT-pushOffset-11266");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }
    {
        mapping = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_ONLY_STORAGE_BUFFER_BIT_EXT);
        mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT;
        mapping.sourceData.indirectAddress.pushOffset = (uint32_t)heap_props.maxPushDataSize;

        CreateComputePipelineHelper pipe(*this);
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        m_errorMonitor->SetDesiredError("VUID-VkDescriptorMappingSourceIndirectAddressEXT-pushOffset-11267");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }
    {
        mapping = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_ONLY_STORAGE_BUFFER_BIT_EXT);
        mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT;
        mapping.sourceData.indirectAddress.addressOffset = 7;

        CreateComputePipelineHelper pipe(*this);
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        m_errorMonitor->SetDesiredError("VUID-VkDescriptorMappingSourceIndirectAddressEXT-addressOffset-11268");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, DescriptorMappingSourceShaderRecordIndexAlign) {
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkDescriptorSetAndBindingMappingEXT mapping =
        MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_ONLY_STORAGE_BUFFER_BIT_EXT);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_SHADER_RECORD_INDEX_EXT;
    mapping.sourceData.shaderRecordIndex.shaderRecordOffset = 3;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    CreateComputePipelineHelper pipe(*this);
    pipe.LateBindPipelineInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;

    m_errorMonitor->SetDesiredError("VUID-VkDescriptorMappingSourceShaderRecordIndexEXT-shaderRecordOffset-11269");
    pipe.CreateComputePipeline(false);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, DescriptorMappingSourceShaderRecordIndexSize) {
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR ray_tracing_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(ray_tracing_properties);

    VkDescriptorSetAndBindingMappingEXT mapping =
        MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_ONLY_STORAGE_BUFFER_BIT_EXT);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_SHADER_RECORD_INDEX_EXT;
    mapping.sourceData.shaderRecordIndex.shaderRecordOffset = Align(ray_tracing_properties.maxShaderGroupStride, 4u);

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    CreateComputePipelineHelper pipe(*this);
    pipe.LateBindPipelineInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;

    m_errorMonitor->SetDesiredError("VUID-VkDescriptorMappingSourceShaderRecordIndexEXT-shaderRecordOffset-11270");
    pipe.CreateComputePipeline(false);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, DescriptorMappingSourceIndirectIndexArray) {
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkDescriptorSetAndBindingMappingEXT mapping = {};
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;
    {
        mapping = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_ONLY_STORAGE_BUFFER_BIT_EXT);
        mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT;
        mapping.sourceData.indirectIndexArray.pushOffset = 4;

        CreateComputePipelineHelper pipe(*this);
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        m_errorMonitor->SetDesiredError("VUID-VkDescriptorMappingSourceIndirectIndexArrayEXT-pushOffset-11359");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }
    {
        mapping = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_ONLY_STORAGE_BUFFER_BIT_EXT);
        mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT;
        mapping.sourceData.indirectIndexArray.pushOffset = (uint32_t)heap_props.maxPushDataSize;

        CreateComputePipelineHelper pipe(*this);
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        m_errorMonitor->SetDesiredError("VUID-VkDescriptorMappingSourceIndirectIndexArrayEXT-pushOffset-11360");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }
    {
        mapping = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_ONLY_STORAGE_BUFFER_BIT_EXT);
        mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT;
        mapping.sourceData.indirectIndexArray.addressOffset = 7;

        CreateComputePipelineHelper pipe(*this);
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        m_errorMonitor->SetDesiredError("VUID-VkDescriptorMappingSourceIndirectIndexArrayEXT-addressOffset-11361");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, OpaqueCaptureDataCreateInfo) {
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    std::vector<uint8_t> data(heap_props.imageCaptureReplayOpaqueDataSize + 1);

    VkOpaqueCaptureDataCreateInfoEXT capture_data_create_info = vku::InitStructHelper();
    VkHostAddressRangeConstEXT data_range = {data.data(), data.size()};
    capture_data_create_info.pData = &data_range;

    for (int i = 0; i < 2; i++) {
        VkImageCreateInfo image_create_info = vku::InitStructHelper(&capture_data_create_info);
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.flags = (i == 0) ? 0u : (VkImageCreateFlags)VK_IMAGE_CREATE_DESCRIPTOR_HEAP_CAPTURE_REPLAY_BIT_EXT;
        image_create_info.extent.width = 128;
        image_create_info.extent.height = 128;
        image_create_info.extent.depth = 1;
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.format = VK_FORMAT_D32_SFLOAT;
        image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        if (i == 0) {
            m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-flags-11281");
        } else {
            m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-flags-08104");
        }
        vkt::Image image(*m_device, image_create_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, OpaqueCaptureDataCreateInfoSize) {
    AddRequiredFeature(vkt::Feature::descriptorHeapCaptureReplay);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    std::vector<uint8_t> data(heap_props.imageCaptureReplayOpaqueDataSize + 1);

    VkOpaqueCaptureDataCreateInfoEXT capture_data_create_info = vku::InitStructHelper();
    VkHostAddressRangeConstEXT data_range = {data.data(), data.size()};
    capture_data_create_info.pData = &data_range;

    VkImageCreateInfo image_create_info = vku::InitStructHelper(&capture_data_create_info);
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.flags = VK_IMAGE_CREATE_DESCRIPTOR_HEAP_CAPTURE_REPLAY_BIT_EXT;
    image_create_info.extent.width = 128;
    image_create_info.extent.height = 128;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.format = VK_FORMAT_D32_SFLOAT;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-pData-11286");
    vkt::Image image(*m_device, image_create_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, GetImageOpaqueCaptureData) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::descriptorHeapCaptureReplay);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddressCaptureReplay);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    std::vector<uint8_t> payload(heap_props.imageCaptureReplayOpaqueDataSize + 1);

    VkOpaqueCaptureDataCreateInfoEXT capture_data_create_info = vku::InitStructHelper();
    VkHostAddressRangeConstEXT range1 = {payload.data(), heap_props.imageCaptureReplayOpaqueDataSize};
    capture_data_create_info.pData = &range1;

    VkImageCreateInfo image_create_info = vku::InitStructHelper(&capture_data_create_info);
    image_create_info.flags = VK_IMAGE_CREATE_DESCRIPTOR_HEAP_CAPTURE_REPLAY_BIT_EXT;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.width = 128;
    image_create_info.extent.height = 128;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.format = VK_FORMAT_D32_SFLOAT;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkMemoryAllocateFlagsInfo alloc_info = vku::InitStructHelper();
    alloc_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT | VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT;
    vkt::Image image(*m_device, image_create_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_info);
    VkHostAddressRangeEXT range2 = {payload.data(), payload.size()};

    m_errorMonitor->SetDesiredError("VUID-vkGetImageOpaqueCaptureDataEXT-size-11283");
    vk::GetImageOpaqueCaptureDataEXT(device(), 1, &image.handle(), &range2);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, GetImageOpaqueCaptureDataMissingFlag) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::descriptorHeapCaptureReplay);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddressCaptureReplay);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkOpaqueCaptureDataCreateInfoEXT capture_data_create_info = vku::InitStructHelper();
    capture_data_create_info.pData = nullptr;

    VkImageCreateInfo image_create_info = vku::InitStructHelper(&capture_data_create_info);
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.width = 128;
    image_create_info.extent.height = 128;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.format = VK_FORMAT_D32_SFLOAT;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkMemoryAllocateFlagsInfo alloc_info = vku::InitStructHelper();
    alloc_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT | VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT;
    vkt::Image image(*m_device, image_create_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_info);

    std::vector<uint8_t> payload(heap_props.imageCaptureReplayOpaqueDataSize);
    VkHostAddressRangeEXT range2 = {payload.data(), payload.size()};
    {
        m_errorMonitor->SetDesiredError("VUID-vkGetImageOpaqueCaptureDataEXT-pImages-11285");
        vk::GetImageOpaqueCaptureDataEXT(device(), 1, &image.handle(), &range2);
        m_errorMonitor->VerifyFound();
    }
    {
        range2.size = 0;
        m_errorMonitor->SetDesiredError("VUID-VkHostAddressRangeEXT-size-arraylength");
        vk::GetImageOpaqueCaptureDataEXT(device(), 1, &image.handle(), &range2);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, RegisterCustomBorderColor) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::customBorderColors);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkPhysicalDeviceCustomBorderColorPropertiesEXT custom_border_color_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(custom_border_color_properties);

    VkSamplerCustomBorderColorCreateInfoEXT custom_border_color_create_info = vku::InitStructHelper();
    uint32_t index = custom_border_color_properties.maxCustomBorderColorSamplers;

    m_errorMonitor->SetDesiredError("VUID-vkRegisterCustomBorderColorEXT-requestIndex-11287");
    vk::RegisterCustomBorderColorEXT(device(), &custom_border_color_create_info, VK_TRUE, &index);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, UnregisterCustomBorderColor) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::customBorderColors);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkPhysicalDeviceCustomBorderColorPropertiesEXT custom_border_color_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(custom_border_color_properties);

    m_errorMonitor->SetDesiredError("VUID-vkUnregisterCustomBorderColorEXT-index-11288");
    vk::UnregisterCustomBorderColorEXT(device(), custom_border_color_properties.maxCustomBorderColorSamplers);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, SamplerCreateWithBorder) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::customBorderColors);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkPhysicalDeviceCustomBorderColorPropertiesEXT custom_border_color_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(custom_border_color_properties);

    VkSamplerCustomBorderColorIndexCreateInfoEXT custom_border_color_index_create_info = vku::InitStructHelper();
    custom_border_color_index_create_info.index = custom_border_color_properties.maxCustomBorderColorSamplers;

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo(&custom_border_color_index_create_info);
    VkSampler sampler = VK_NULL_HANDLE;

    m_errorMonitor->SetDesiredError("VUID-VkSamplerCustomBorderColorIndexCreateInfoEXT-index-11289");
    vk::CreateSampler(device(), &sampler_ci, nullptr, &sampler);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, DescriptorHeapPipelineCreateFlag) {
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkPipelineCreateFlags2CreateInfoKHR flags2_ci = vku::InitStructHelper();
    flags2_ci.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    {
        CreateComputePipelineHelper pipe(*this, &flags2_ci);
        pipe.LateBindPipelineInfo();

        m_errorMonitor->SetDesiredError("VUID-VkComputePipelineCreateInfo-flags-11311");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }
    {
        CreatePipelineHelper pipe(*this, &flags2_ci);

        m_errorMonitor->SetDesiredError("VUID-VkGraphicsPipelineCreateInfo-flags-11311");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }
    {
        CreateComputePipelineHelper pipe(*this);
        pipe.LateBindPipelineInfo();
        pipe.cp_ci_.layout = VK_NULL_HANDLE;

        m_errorMonitor->SetDesiredError("VUID-VkComputePipelineCreateInfo-None-11367");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, PipelineShaderStageCreateInfoNotEnabled) {
    TEST_DESCRIPTION("Validate VkPipelineShaderStageCreateInfo mappingCount to be zero when descriptorHeap is not enabled");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkDescriptorSetAndBindingMappingEXT mapping =
        MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_ONLY_STORAGE_BUFFER_BIT_EXT);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    CreateComputePipelineHelper pipe(*this);
    pipe.LateBindPipelineInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;

    m_errorMonitor->SetDesiredError("VUID-VkPipelineShaderStageCreateInfo-descriptorHeap-11314");
    pipe.CreateComputePipeline(false);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, IndirectExecutionSetShaderInfoInitialShaderFlagSame) {
    TEST_DESCRIPTION(
        "Validate that all VkIndirectExecutionSetShaderInfoEXT.pInitialShaders are created with same descriptor heap "
        "flag state");
    AddRequiredExtensions(VK_EXT_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::deviceGeneratedCommands);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT dgc_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(dgc_props);
    if ((dgc_props.supportedIndirectCommandsShaderStagesShaderBinding & VK_SHADER_STAGE_VERTEX_BIT) == 0) {
        GTEST_SKIP() << "VK_SHADER_STAGE_VERTEX_BIT is not supported.";
    }
    if ((dgc_props.supportedIndirectCommandsShaderStagesShaderBinding & VK_SHADER_STAGE_FRAGMENT_BIT) == 0) {
        GTEST_SKIP() << "VK_SHADER_STAGE_FRAGMENT_BIT is not supported.";
    }

    for (int i = 0; i < 2; i++) {
        const VkShaderCreateFlagsEXT flags1 = VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT | VK_SHADER_CREATE_DESCRIPTOR_HEAP_BIT_EXT;
        const VkShaderCreateFlagsEXT flags2 = VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT;

        const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
        VkShaderCreateInfoEXT vert_ci = ShaderCreateInfoFlag(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, (i == 0) ? flags1 : flags2);
        const vkt::Shader vertShader(*m_device, vert_ci);

        const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);
        VkShaderCreateInfoEXT frag_ci = ShaderCreateInfoFlag(frag_spv, VK_SHADER_STAGE_FRAGMENT_BIT, (i == 0) ? flags2 : flags1);
        const vkt::Shader fragShader(*m_device, frag_ci);

        VkIndirectExecutionSetEXT indirectExecutionSet = VK_NULL_HANDLE;

        VkShaderEXT shaders[2] = {vertShader, fragShader};

        VkIndirectExecutionSetShaderInfoEXT shader_info = vku::InitStructHelper();
        shader_info.maxShaderCount = 2;
        shader_info.shaderCount = 2;
        shader_info.pInitialShaders = shaders;
        shader_info.pSetLayoutInfos = nullptr;

        VkIndirectExecutionSetCreateInfoEXT indirect_execution_set_create_info = vku::InitStructHelper();

        indirect_execution_set_create_info.type = VK_INDIRECT_EXECUTION_SET_INFO_TYPE_SHADER_OBJECTS_EXT;
        indirect_execution_set_create_info.info.pShaderInfo = &shader_info;

        if (i == 0) {
            m_errorMonitor->SetDesiredError("VUID-VkIndirectExecutionSetShaderInfoEXT-pInitialShaders-11321");
        } else {
            m_errorMonitor->SetDesiredError("VUID-VkIndirectExecutionSetShaderInfoEXT-pInitialShaders-11322");
        }
        vk::CreateIndirectExecutionSetEXT(device(), &indirect_execution_set_create_info, nullptr, &indirectExecutionSet);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, IndirectExecutionSetShaderInfoSetLayoutNull) {
    TEST_DESCRIPTION("Validate that all VkIndirectExecutionSetShaderInfoEXT.pSetLayoutInfos is null");
    AddRequiredExtensions(VK_EXT_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::deviceGeneratedCommands);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT dgc_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(dgc_props);
    if ((dgc_props.supportedIndirectCommandsShaderStagesShaderBinding & VK_SHADER_STAGE_VERTEX_BIT) == 0) {
        GTEST_SKIP() << "VK_SHADER_STAGE_VERTEX_BIT is not supported.";
    }

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    const VkShaderCreateFlagsEXT flags = VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT | VK_SHADER_CREATE_DESCRIPTOR_HEAP_BIT_EXT;
    VkShaderCreateInfoEXT vert_ci = ShaderCreateInfoFlag(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, flags);
    const vkt::Shader vertShader(*m_device, vert_ci);

    VkIndirectExecutionSetEXT indirectExecutionSet = VK_NULL_HANDLE;
    VkDescriptorSetLayout set_layout = VK_NULL_HANDLE;

    VkIndirectExecutionSetShaderLayoutInfoEXT set_layout_info = vku::InitStructHelper();
    set_layout_info.setLayoutCount = 1;
    set_layout_info.pSetLayouts = &set_layout;

    VkIndirectExecutionSetShaderInfoEXT shader_info = vku::InitStructHelper();
    shader_info.maxShaderCount = 1;
    shader_info.shaderCount = 1;
    shader_info.pInitialShaders = &vertShader.handle();
    shader_info.pSetLayoutInfos = &set_layout_info;

    VkIndirectExecutionSetCreateInfoEXT indirect_execution_set_create_info = vku::InitStructHelper();

    indirect_execution_set_create_info.type = VK_INDIRECT_EXECUTION_SET_INFO_TYPE_SHADER_OBJECTS_EXT;
    indirect_execution_set_create_info.info.pShaderInfo = &shader_info;

    m_errorMonitor->SetDesiredError("VUID-VkIndirectExecutionSetShaderInfoEXT-pInitialShaders-11323");
    vk::CreateIndirectExecutionSetEXT(device(), &indirect_execution_set_create_info, nullptr, &indirectExecutionSet);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, ShaderCreateInfoPushConstant) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    {
        const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
        const VkShaderCreateFlagsEXT flags = VK_SHADER_CREATE_DESCRIPTOR_HEAP_BIT_EXT;
        VkShaderCreateInfoEXT vert_ci = ShaderCreateInfoFlag(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, flags);
        vert_ci.pushConstantRangeCount = 1;
        vert_ci.pPushConstantRanges = nullptr;

        m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-flags-11370");
        const vkt::Shader vertShader(*m_device, vert_ci);
        m_errorMonitor->VerifyFound();
    }
    {
        const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
        const VkShaderCreateFlagsEXT flags = VK_SHADER_CREATE_DESCRIPTOR_HEAP_BIT_EXT;
        VkShaderCreateInfoEXT vert_ci = ShaderCreateInfoFlag(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, flags);
        VkPushConstantRange push_constant_range = {};
        vert_ci.pushConstantRangeCount = 0;
        vert_ci.pPushConstantRanges = &push_constant_range;

        m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-flags-11371");
        const vkt::Shader vertShader(*m_device, vert_ci);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, DescriptorType) {
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceDescriptorSizeEXT-type-11362");
    vk::GetPhysicalDeviceDescriptorSizeEXT(Gpu(), VK_DESCRIPTOR_TYPE_MAX_ENUM);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, BindHeapInfoBufferHeapUsage) {
    TEST_DESCRIPTION("Validate VkBindHeapInfoEXT uses buffer address from buffer with heap usage and memory usage for the command");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    {
        const VkDeviceSize bdsize = heap_props.samplerDescriptorSize + heap_props.minSamplerHeapReservedRange;
        vkt::Buffer buffer(*m_device, bdsize, 0, vkt::device_address);

        VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
        bind_info.heapRange = {buffer.Address(), bdsize};
        bind_info.reservedRangeSize = heap_props.minSamplerHeapReservedRange;

        m_errorMonitor->SetDesiredError("VUID-vkCmdBindSamplerHeapEXT-heapRange-11230");
        m_command_buffer.Begin();
        vk::CmdBindSamplerHeapEXT(m_command_buffer, &bind_info);
        m_command_buffer.End();
        m_errorMonitor->VerifyFound();
    }
    {
        const VkDeviceSize bdsize = heap_props.bufferDescriptorSize + heap_props.minResourceHeapReservedRange;
        vkt::Buffer buffer(*m_device, bdsize, 0, vkt::device_address);

        VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
        bind_info.heapRange = {buffer.Address(), bdsize};
        bind_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;

        m_errorMonitor->SetDesiredError("VUID-vkCmdBindResourceHeapEXT-heapRange-11237");
        m_command_buffer.Begin();
        vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_info);
        m_command_buffer.End();
        m_errorMonitor->VerifyFound();
    }

    // Validate memory overflow corruption and binding
    {
        const VkDeviceSize heap_size = heap_props.samplerDescriptorSize + heap_props.minSamplerHeapReservedRange;
        vkt::Buffer heap(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

        VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
        bind_info.heapRange = {heap.Address(), heap_size + 1};
        bind_info.reservedRangeSize = heap_props.minSamplerHeapReservedRange;

        m_errorMonitor->SetDesiredError("VUID-VkDeviceAddressRangeEXT-address-11365");
        m_command_buffer.Begin();
        vk::CmdBindSamplerHeapEXT(m_command_buffer, &bind_info);
        m_command_buffer.End();
        m_errorMonitor->VerifyFound();
    }
    {
        const VkDeviceSize heap_size = heap_props.samplerDescriptorSize + heap_props.minSamplerHeapReservedRange;
        vkt::Buffer heap(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

        VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
        bind_info.heapRange = {heap.Address(), heap_size};
        bind_info.reservedRangeSize = heap_props.minSamplerHeapReservedRange;

        heap.Memory().Destroy();

        m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-None-10894");
        m_command_buffer.Begin();
        vk::CmdBindSamplerHeapEXT(m_command_buffer, &bind_info);
        m_command_buffer.End();
        m_errorMonitor->VerifyFound();
    }

    {
        const VkDeviceSize heap_size = heap_props.bufferDescriptorSize + heap_props.minResourceHeapReservedRange;
        vkt::Buffer heap(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

        VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
        bind_info.heapRange = {heap.Address(), heap_size + 1};
        bind_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;

        m_errorMonitor->SetDesiredError("VUID-VkDeviceAddressRangeEXT-address-11365");
        m_command_buffer.Begin();
        vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_info);
        m_command_buffer.End();
        m_errorMonitor->VerifyFound();
    }
    {
        const VkDeviceSize heap_size = heap_props.bufferDescriptorSize + heap_props.minResourceHeapReservedRange;
        vkt::Buffer heap(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

        VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
        bind_info.heapRange = {heap.Address(), heap_size};
        bind_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;

        heap.Memory().Destroy();

        m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-None-10894");
        m_command_buffer.Begin();
        vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_info);
        m_command_buffer.End();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, BindOverlappingRangesSampler) {
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    // std::max allows the test to run when minSamplerHeapReservedRange is zero or smaller than samplerDescriptorSize
    const VkDeviceSize heap_size = 4 * std::max(heap_props.samplerDescriptorSize, heap_props.minSamplerHeapReservedRange);
    vkt::Buffer heap(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    vkt::CommandPool command_pool(*m_device, m_device->graphics_queue_node_index_);
    vkt::CommandBuffer cmd_buffer1(*m_device, command_pool);
    vkt::CommandBuffer cmd_buffer2(*m_device, command_pool);
    vkt::CommandBuffer cmd_buffer3(*m_device, command_pool);

    cmd_buffer1.Begin();
    cmd_buffer2.Begin();
    cmd_buffer3.Begin();

    VkBindHeapInfoEXT bind_info1 = vku::InitStructHelper();
    bind_info1.heapRange.address = heap.Address();
    bind_info1.heapRange.size = heap_size;
    bind_info1.reservedRangeOffset = 0;
    bind_info1.reservedRangeSize = heap_size / 2;

    VkBindHeapInfoEXT bind_info2 = vku::InitStructHelper();
    bind_info2.heapRange.address = heap.Address();
    bind_info2.heapRange.size = heap_size;
    bind_info2.reservedRangeOffset = heap_size / 4;
    bind_info2.reservedRangeSize = heap_size / 2;

    vk::CmdBindSamplerHeapEXT(cmd_buffer1, &bind_info1);

    // Bind overlapping reserved areas: not allowed
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindSamplerHeapEXT-pBindInfo-11228");
    vk::CmdBindSamplerHeapEXT(cmd_buffer2, &bind_info2);
    m_errorMonitor->VerifyFound();

    // Bind same reserved area to different buffer: allowed
    vk::CmdBindSamplerHeapEXT(cmd_buffer3, &bind_info1);

    // Rebind same reserved area to same buffer: allowed
    vk::CmdBindSamplerHeapEXT(cmd_buffer1, &bind_info1);

    // Rebind same reserved area to different type buffer: not allowed
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindResourceHeapEXT-pBindInfo-11236");
    if (bind_info1.reservedRangeOffset < heap_props.minResourceHeapReservedRange) {
        // We use allow here, otherwise validation layers return before getting to 11236
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBindResourceHeapEXT-pBindInfo-11233");
    }
    vk::CmdBindResourceHeapEXT(cmd_buffer1, &bind_info1);
    m_errorMonitor->VerifyFound();

    cmd_buffer3.End();
    cmd_buffer2.End();
    cmd_buffer1.End();
}

TEST_F(NegativeDescriptorHeap, BindOverlappingRangesResource) {
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    // std::max allows the test to run when minResourceHeapReservedRange is zero or smaller than bufferDescriptorSize
    const VkDeviceSize heap_size = 4 * std::max(heap_props.imageDescriptorSize,
                                                std::max(heap_props.bufferDescriptorSize, heap_props.minResourceHeapReservedRange));
    vkt::Buffer heap(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    vkt::CommandPool command_pool(*m_device, m_device->graphics_queue_node_index_);
    vkt::CommandBuffer cmd_buffer1(*m_device, command_pool);
    vkt::CommandBuffer cmd_buffer2(*m_device, command_pool);
    vkt::CommandBuffer cmd_buffer3(*m_device, command_pool);

    cmd_buffer1.Begin();
    cmd_buffer2.Begin();
    cmd_buffer3.Begin();

    VkBindHeapInfoEXT bind_info1 = vku::InitStructHelper();
    bind_info1.heapRange.address = heap.Address();
    bind_info1.heapRange.size = heap_size;
    bind_info1.reservedRangeOffset = 0;
    bind_info1.reservedRangeSize = heap_size / 2;

    VkBindHeapInfoEXT bind_info2 = vku::InitStructHelper();
    bind_info2.heapRange.address = heap.Address();
    bind_info2.heapRange.size = heap_size;
    bind_info2.reservedRangeOffset = heap_size / 4;
    bind_info2.reservedRangeSize = heap_size / 2;

    vk::CmdBindResourceHeapEXT(cmd_buffer1, &bind_info1);

    // Bind overlapping reserved areas: not allowed
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindResourceHeapEXT-pBindInfo-11236");
    vk::CmdBindResourceHeapEXT(cmd_buffer2, &bind_info2);
    m_errorMonitor->VerifyFound();

    // Bind same reserved area to different buffer: allowed
    vk::CmdBindResourceHeapEXT(cmd_buffer3, &bind_info1);

    // Rebind same reserved area to same buffer: allowed
    vk::CmdBindResourceHeapEXT(cmd_buffer1, &bind_info1);

    // Rebind same reserved area to different type buffer: not allowed
    if (bind_info1.heapRange.size > heap_props.maxSamplerHeapSize) {
        // We use allow here, otherwise validation layers return before getting to 11236
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBindSamplerHeapEXT-pBindInfo-11225");
    }
    if (bind_info1.reservedRangeSize < heap_props.minSamplerHeapReservedRange) {
        m_errorMonitor->SetDesiredError("VUID-vkCmdBindSamplerHeapEXT-pBindInfo-11224");
    } else {
        m_errorMonitor->SetDesiredError("VUID-vkCmdBindSamplerHeapEXT-pBindInfo-11228");
    }
    vk::CmdBindSamplerHeapEXT(cmd_buffer1, &bind_info1);
    m_errorMonitor->VerifyFound();

    cmd_buffer3.End();
    cmd_buffer2.End();
    cmd_buffer1.End();
}

TEST_F(NegativeDescriptorHeap, WriteResourceDescriptorsMemoryTests) {
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT align_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(align_props);

    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    const VkDeviceSize buffer_size = std::max(heap_props.imageDescriptorSize, align_props.uniformTexelBufferOffsetAlignmentBytes);
    auto data = std::vector<uint8_t>(static_cast<size_t>(buffer_size));
    VkHostAddressRangeEXT descriptors = {&data[0], data.size()};

    {
        vkt::Buffer buffer(*m_device, buffer_size, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, vkt::device_address);

        VkTexelBufferDescriptorInfoEXT texel_buffer_info = vku::InitStructHelper();
        texel_buffer_info.addressRange = {buffer.Address(), 2 * buffer_size};
        texel_buffer_info.format = format;

        VkResourceDescriptorInfoEXT desc_info = vku::InitStructHelper();
        desc_info.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        desc_info.data.pTexelBuffer = &texel_buffer_info;

        m_errorMonitor->SetDesiredError("VUID-VkDeviceAddressRangeEXT-address-11365");
        vk::WriteResourceDescriptorsEXT(device(), 1u, &desc_info, &descriptors);
        m_errorMonitor->VerifyFound();
    }
    {
        vkt::Buffer buffer(*m_device, buffer_size, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, vkt::device_address);

        VkTexelBufferDescriptorInfoEXT texel_buffer_info = vku::InitStructHelper();
        texel_buffer_info.addressRange = {buffer.Address(), buffer_size};
        texel_buffer_info.format = format;

        VkResourceDescriptorInfoEXT desc_info = vku::InitStructHelper();
        desc_info.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        desc_info.data.pTexelBuffer = &texel_buffer_info;

        buffer.Memory().Destroy();

        m_errorMonitor->SetDesiredError("VUID-VkDeviceAddress-None-10894");
        vk::WriteResourceDescriptorsEXT(device(), 1u, &desc_info, &descriptors);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, WriteResourceDescriptorsMemoryTestsAS) {
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT align_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(align_props);

    const VkDeviceSize bdsize = std::max(heap_props.bufferDescriptorSize, align_props.uniformTexelBufferOffsetAlignmentBytes);
    const uint32_t as_size = 4096;

    {
        vkt::Buffer as_buffer(*m_device, as_size, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, vkt::device_address);

        VkAccelerationStructureKHR as = VK_NULL_HANDLE;
        VkAccelerationStructureCreateInfoKHR asci = vku::InitStructHelper();
        asci.buffer = as_buffer;
        vk::CreateAccelerationStructureKHR(device(), &asci, nullptr, &as);

        VkDeviceAddressRangeEXT address_range = {as_buffer.Address(), as_size + 256};

        VkResourceDescriptorInfoEXT desc_info = vku::InitStructHelper();
        desc_info.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        desc_info.data.pAddressRange = &address_range;

        auto data = std::vector<uint8_t>(static_cast<size_t>(bdsize));
        VkHostAddressRangeEXT descriptors = {&data[0], data.size()};

        m_errorMonitor->SetDesiredError("VUID-VkResourceDescriptorInfoEXT-type-11483");
        vk::WriteResourceDescriptorsEXT(device(), 1u, &desc_info, &descriptors);
        m_errorMonitor->VerifyFound();

        vk::DestroyAccelerationStructureKHR(device(), as, nullptr);
    }
    {
        vkt::Buffer as_buffer(*m_device, as_size, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, vkt::device_address);

        VkAccelerationStructureKHR as = VK_NULL_HANDLE;
        VkAccelerationStructureCreateInfoKHR asci = vku::InitStructHelper();
        asci.buffer = as_buffer;
        vk::CreateAccelerationStructureKHR(device(), &asci, nullptr, &as);

        VkDeviceAddressRangeEXT address_range = {as_buffer.Address(), as_size};

        VkResourceDescriptorInfoEXT desc_info = vku::InitStructHelper();
        desc_info.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        desc_info.data.pAddressRange = &address_range;

        auto data = std::vector<uint8_t>(static_cast<size_t>(bdsize));
        VkHostAddressRangeEXT descriptors = {&data[0], data.size()};
        as_buffer.Memory().Destroy();
        m_errorMonitor->SetDesiredError("VUID-VkResourceDescriptorInfoEXT-type-11483");
        vk::WriteResourceDescriptorsEXT(device(), 1u, &desc_info, &descriptors);
        m_errorMonitor->VerifyFound();

        vk::DestroyAccelerationStructureKHR(device(), as, nullptr);
    }
}

TEST_F(NegativeDescriptorHeap, OpTypeImage) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    RETURN_IF_SKIP(CheckSlangSupport());

    if (heap_props.imageDescriptorAlignment < 2) {
        GTEST_SKIP() << "Cannot be unaligned with imageDescriptorAlignment less than 2";
    }

    VkDescriptorSetAndBindingMappingEXT mappings = MakeSetAndBindingMapping(0, 0);
    mappings.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings.sourceData.constantOffset.heapOffset = 1;
    mappings.sourceData.constantOffset.heapArrayStride = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0, rgba8) uniform image2D color_out;
        void main() {
            vec4 color = gl_GlobalInvocationID.xxyy;
            ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
            imageStore(color_out, pixelCoords, color);
        }
    )glsl";

    const char* slang_shader = R"slang(
        [[vk::binding(0, 0)]]
        RWTexture2D<float4> color_out;
        [numthreads(1, 1, 1)]
        void main(uint3 dispatchThreadID : SV_DispatchThreadID) {
            float4 color = float4(dispatchThreadID.xxyy);
            int2 pixelCoords = int2(dispatchThreadID.xy);
            color_out[pixelCoords] = color;
        }
    )slang";

    VkShaderObj cs_modules[2];
    cs_modules[0] = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    cs_modules[1] = VkShaderObj(*m_device, slang_shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_SLANG);

    VkPipelineCreateFlags2CreateInfo pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    for (int i = 0; i < 4; i++) {
        CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
        pipe.cp_ci_.layout = VK_NULL_HANDLE;
        if (i < 2) {
            pipe.cp_ci_.stage = cs_modules[0].GetStageCreateInfo();
        } else {
            pipe.cp_ci_.stage = cs_modules[1].GetStageCreateInfo();
        }
        pipe.cp_ci_.stage.pNext = &mapping_info;

        mappings.sourceData.constantOffset.heapOffset = (i % 2 == 0) ? 1 : 0;
        mappings.sourceData.constantOffset.heapArrayStride = (i % 2 == 0) ? 0 : 1;

        m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetAndBindingMappingEXT-source-11251");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, MappedPushIsBlockUniform) {
    TEST_DESCRIPTION("Validate that mapped push data is backed by block uniform");
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    RETURN_IF_SKIP(CheckSlangSupport());

    VkDescriptorSetAndBindingMappingEXT mappings = MakeSetAndBindingMapping(0, 0);
    mappings.source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_DATA_EXT;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mappings;

    VkPipelineCreateFlags2CreateInfo pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    const char* cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer Output { uint a[2]; } x[];
        void main() {
            x[0].a[0] = 0;
        }
    )glsl";

    const char* slang_shader = R"slang(
        struct Output {
            uint a[2];
        };

        [[vk::binding(0, 0)]]
        RWStructuredBuffer<Output> x;

        [numthreads(1, 1, 1)]
        void main(uint3 dispatchThreadID : SV_DispatchThreadID) {
            x[0].a[dispatchThreadID.x] = 0;
        }
    )slang";

    VkShaderObj cs_modules[2];
    cs_modules[0] = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    cs_modules[1] = VkShaderObj(*m_device, slang_shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_SLANG);

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_modules[0].GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;
    m_errorMonitor->SetDesiredError("VUID-VkPipelineShaderStageCreateInfo-pNext-11315");
    pipe.CreateComputePipeline(false);
    m_errorMonitor->VerifyFound();

    pipe.cp_ci_.stage = cs_modules[1].GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;
    m_errorMonitor->SetDesiredError("VUID-VkPipelineShaderStageCreateInfo-pNext-11315");
    pipe.CreateComputePipeline(false);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, MappedStructLessThanMaxPushDataSize) {
    TEST_DESCRIPTION("Validate that mapped structure is less than maxPushDataSize");
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    RETURN_IF_SKIP(CheckSlangSupport());

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_DATA_EXT;
    mappings[0].sourceData.pushDataOffset = 0;
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2;
    mapping_info.pMappings = mappings;

    VkPipelineCreateFlags2CreateInfo pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    std::stringstream cs_source;
    cs_source << R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) uniform Input { uint a[)glsl";
    cs_source << heap_props.maxPushDataSize;
    cs_source << R"glsl(]; } x[];
        layout(set = 0, binding = 1) buffer Output { uint b[2]; } y[];
        void main() {
            y[0].b[gl_GlobalInvocationID.x] = x[0].a[0];
        }
    )glsl";
    std::stringstream slang_shader;
    slang_shader << R"slang(
        struct Input {
            uint a[)slang";
    slang_shader << heap_props.maxPushDataSize;
    slang_shader << R"slang(];
        };
        [[vk::binding(0, 0)]]
        StructuredBuffer<Input> x;

        struct Output {
            uint b[2];
        };
        [[vk::binding(1, 0)]]
        RWStructuredBuffer<Output> y;
        [numthreads(1, 1, 1)]
        void main(uint3 dispatchThreadID : SV_DispatchThreadID) {
            y[0].b[dispatchThreadID.x] = x[0].a[dispatchThreadID.x];
        }
    )slang";

    VkShaderObj cs_modules[2];
    cs_modules[0] = VkShaderObj(*m_device, cs_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT);
    cs_modules[1] =
        VkShaderObj(*m_device, slang_shader.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_SLANG);

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_modules[0].GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;
    m_errorMonitor->SetDesiredError("VUID-VkPipelineShaderStageCreateInfo-pNext-11316");
    m_errorMonitor->SetDesiredError("VUID-VkPipelineShaderStageCreateInfo-pNext-11315");
    pipe.CreateComputePipeline(false);
    m_errorMonitor->VerifyFound();

    pipe.cp_ci_.stage = cs_modules[1].GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;
    m_errorMonitor->SetDesiredError("VUID-VkPipelineShaderStageCreateInfo-pNext-11315");
    pipe.CreateComputePipeline(false);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, OpTypeStruct) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    RETURN_IF_SKIP(CheckSlangSupport());

    if (heap_props.bufferDescriptorAlignment < 2) {
        GTEST_SKIP() << "Cannot be unaligned with bufferDescriptorAlignment less than 2";
    }

    VkDescriptorSetAndBindingMappingEXT mappings = MakeSetAndBindingMapping(0, 0);
    mappings.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings.sourceData.constantOffset.heapOffset = 1;
    mappings.sourceData.constantOffset.heapArrayStride = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer storage_buffer1 { int result[]; };
        void main() {
            result[int(256.0 * gl_GlobalInvocationID.x)] = 1;
        }
    )glsl";

    const char* slang_shader = R"slang(
        [[vk::binding(0, 0)]]
        RWStructuredBuffer<int> result;

        [numthreads(1, 1, 1)]
        void main(uint3 dispatchThreadID : SV_DispatchThreadID)
        {
            result[int(256.0 * dispatchThreadID.x)] = 1;
        }
    )slang";

    VkShaderObj cs_modules[2];
    cs_modules[0] = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    cs_modules[1] = VkShaderObj(*m_device, slang_shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_SLANG);

    VkPipelineCreateFlags2CreateInfo pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    for (int i = 0; i < 4; i++) {
        CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
        pipe.cp_ci_.stage = cs_modules[i / 2].GetStageCreateInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        mappings.sourceData.constantOffset.heapOffset = (i % 2 == 0) ? 1 : 0;
        mappings.sourceData.constantOffset.heapArrayStride = (i % 2 == 0) ? 0 : 1;

        m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetAndBindingMappingEXT-source-11252");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, OpTypeSampler) {
    AddRequiredExtensions(VK_KHR_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::computeDerivativeGroupQuads);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    RETURN_IF_SKIP(CheckSlangSupport());

    if (heap_props.samplerDescriptorAlignment < 2) {
        GTEST_SKIP() << "Cannot be unaligned with samplerDescriptorAlignment less than 2";
    }

    VkDescriptorSetAndBindingMappingEXT mappings[3];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = 0u;
    mappings[0].sourceData.constantOffset.heapArrayStride = 0;

    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;

    mappings[2] = MakeSetAndBindingMapping(0, 2);
    mappings[2].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 3;
    mapping_info.pMappings = mappings;

    char const* cs_source0 = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer Output { int result[]; };
        layout(set = 0, binding = 1) uniform sampler samp;
        layout(set = 0, binding = 2) uniform texture2D tex;
        void main() {
            ivec2 uv = ivec2(gl_GlobalInvocationID.xy);
            vec2 texCoord = vec2(gl_GlobalInvocationID.xy) / 1.0;
            vec4 color = texture(sampler2D(tex, samp), texCoord);
            result[gl_LocalInvocationIndex] = int(color.r);
        }
    )glsl";
    char const* cs_source1 = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer Output { int result[]; };
        layout(set = 0, binding = 1) uniform sampler samp[];
        layout(set = 0, binding = 2) uniform texture2D tex;
        void main() {
            ivec2 uv = ivec2(gl_GlobalInvocationID.xy);
            vec2 texCoord = vec2(gl_GlobalInvocationID.xy) / 1.0;
            vec4 color = texture(sampler2D(tex, samp[0]), texCoord);
            result[gl_LocalInvocationIndex] = int(color.r);
        }
    )glsl";

    const char* slang_shader0 = R"slang(
        [[vk::binding(0, 0)]]
        RWStructuredBuffer<int> result;
        [[vk::binding(1, 0)]]
        SamplerState samp;
        [[vk::binding(2, 0)]]
        Texture2D tex;
        [numthreads(2, 2, 2)]
        void main(uint3 dispatchThreadID : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
        {
            int2 uv = int2(dispatchThreadID.xy);
            float2 texCoord = float2(dispatchThreadID.xy) / 1.0;
            float4 color = tex.Sample(samp, texCoord);
            result[groupIndex] = int(color.r);
        }
    )slang";

    const char* slang_shader1 = R"slang(
        [[vk::binding(0, 0)]]
        RWStructuredBuffer<int> result;
        [[vk::binding(1, 0)]]
        SamplerState samp[];
        [[vk::binding(2, 0)]]
        Texture2D tex;
        [numthreads(2, 2, 2)]
        void main(uint3 dispatchThreadID : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
        {
            int2 uv = int2(dispatchThreadID.xy);
            float2 texCoord = float2(dispatchThreadID.xy) / 1.0;
            float4 color = tex.Sample(samp[0], texCoord);
            result[groupIndex] = int(color.r);
        }
    )slang";

    VkShaderObj cs_modules[4];
    cs_modules[0] = VkShaderObj(*m_device, cs_source0, VK_SHADER_STAGE_COMPUTE_BIT);
    cs_modules[1] = VkShaderObj(*m_device, cs_source1, VK_SHADER_STAGE_COMPUTE_BIT);
    cs_modules[2] = VkShaderObj(*m_device, slang_shader0, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_SLANG);
    cs_modules[3] = VkShaderObj(*m_device, slang_shader1, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_SLANG);

    for (int j = 0; j < 4; j++) {
        VkPipelineCreateFlags2CreateInfo pipeline_create_flags_2_create_info = vku::InitStructHelper();
        pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

        for (int i = 0; i < 2; i++) {
            CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
            pipe.cp_ci_.layout = VK_NULL_HANDLE;
            pipe.cp_ci_.stage = cs_modules[j].GetStageCreateInfo();
            pipe.cp_ci_.stage.pNext = &mapping_info;

            mappings[1].sourceData.constantOffset.heapOffset = (i == 0) ? 1 : 0;
            mappings[1].sourceData.constantOffset.heapArrayStride = (i == 0) ? 0 : 1;

            m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetAndBindingMappingEXT-source-11253");
            pipe.CreateComputePipeline(false);
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(NegativeDescriptorHeap, OpTypeSampledImage) {
    TEST_DESCRIPTION("Validate that mapping is aligned for OpTypeSampledImage");
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    RETURN_IF_SKIP(CheckSlangSupport());

    if (heap_props.samplerDescriptorAlignment < 2) {
        GTEST_SKIP() << "Cannot be unaligned with samplerDescriptorAlignment less than 2";
    }

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = 0;
    mappings[0].sourceData.constantOffset.heapArrayStride = 0;

    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer Output { int result[]; };
        layout(set = 0, binding = 1) uniform sampler2D tex;
        void main() {
            vec4 color = textureLod(tex, vec2(gl_GlobalInvocationID.xy), 0);
            result[gl_LocalInvocationIndex] = int(color.r);
        }
    )glsl";

    const char* slang_shader = R"slang(
        [[vk::binding(0, 0)]]
        RWTexture2D<float4> result;
        [[vk::combinedImageSampler]]
        Sampler2D<float4> tex;
        [numthreads(1, 1, 1)]
        void main(uint3 dispatchThreadID : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex) {
            float4 color = tex.SampleLevel(float2(dispatchThreadID.xy), 0);
            result[groupIndex] = int(color.r);
        }
    )slang";

    VkShaderObj cs_modules[2];
    cs_modules[0] = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    cs_modules[1] = VkShaderObj(*m_device, slang_shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_SLANG);

    VkPipelineCreateFlags2CreateInfo pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    for (int i = 0; i < 4; i++) {
        CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
        pipe.cp_ci_.layout = VK_NULL_HANDLE;
        pipe.cp_ci_.stage = cs_modules[i / 2].GetStageCreateInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        mappings[1].sourceData.constantOffset.samplerHeapOffset = (i % 2 == 0) ? 1 : 0;
        mappings[1].sourceData.constantOffset.samplerHeapArrayStride = (i % 2 == 0) ? 0 : 1;

        m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetAndBindingMappingEXT-source-11254");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, NoMappingStruct) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    char const* cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer Output { int result[]; } x[];
        layout(set = 0, binding = 1) buffer Input0 { int data[]; } y[];
        void main() {
            x[0].result[gl_LocalInvocationID.x] = y[0].data[gl_LocalInvocationID.x];
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);

    VkPipelineCreateFlags2CreateInfo pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    m_errorMonitor->SetDesiredError("VUID-VkComputePipelineCreateInfo-flags-11312");
    pipe.CreateComputePipeline(false);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, MappingWithoutFlag) {
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkDescriptorSetAndBindingMappingEXT mappings = MakeSetAndBindingMapping(0, 0);
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A0 { int a_0; };
        void main() {
            a_0 = 0;
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;
    m_errorMonitor->SetDesiredWarning("WARNING-VkShaderDescriptorSetAndBindingMappingInfoEXT-ignored");
    pipe.CreateComputePipeline(false);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, NoMappingShaderObject) {
    TEST_DESCRIPTION("Check that descriptor set bindings have a mappings when using VkShaderCreateFlagsEXT");
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    char const* cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer Output { int result[]; } x[];
        void main() {
            x[0].result[gl_LocalInvocationID.x] = 0;
        }
    )glsl";

    VkDescriptorSetAndBindingMappingEXT mappings = MakeSetAndBindingMapping(1, 0);
    mappings.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings.sourceData.constantOffset.heapOffset = 0u;
    mappings.sourceData.constantOffset.heapArrayStride = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mappings;

    const auto spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, cs_source);
    const VkShaderCreateFlagsEXT flags = VK_SHADER_CREATE_DESCRIPTOR_HEAP_BIT_EXT;
    VkShaderCreateInfoEXT comp_ci = ShaderCreateInfoFlag(spv, VK_SHADER_STAGE_COMPUTE_BIT, flags);
    comp_ci.pNext = &mapping_info;

    m_errorMonitor->SetDesiredError("VUID-VkShaderCreateInfoEXT-flags-11292");
    const vkt::Shader compShader(*m_device, comp_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, NoMappingComputePipeline) {
    TEST_DESCRIPTION("Check that descriptor set bindings have a mappings");
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    RETURN_IF_SKIP(CheckSlangSupport());

    VkDescriptorSetAndBindingMappingEXT mappings = MakeSetAndBindingMapping(0, 0);
    mappings.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings.sourceData.constantOffset.heapOffset = 0;
    mappings.sourceData.constantOffset.heapArrayStride = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer Output { int result[]; } x[];
        layout(set = 0, binding = 1) buffer Input0 { int data[]; } y[];
        void main() {
            x[0].result[gl_LocalInvocationID.x] = y[0].data[gl_LocalInvocationID.x];
        }
    )glsl";

    const char* slang_shader = R"slang(
        [[vk::binding(0, 0)]]
        RWStructuredBuffer<int> x[];

        [[vk::binding(1, 0)]]
        RWStructuredBuffer<int> y[];

        [numthreads(1, 1, 1)]
        void main(uint3 dispatchThreadID : SV_DispatchThreadID) {
            x[0][dispatchThreadID.x] = y[0][dispatchThreadID.x];
        }
    )slang";

    VkShaderObj cs_modules[2];
    cs_modules[0] = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    cs_modules[1] = VkShaderObj(*m_device, slang_shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_SLANG);

    VkPipelineCreateFlags2CreateInfo pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;

    for (uint32_t i = 0; i < 2; ++i) {
        pipe.cp_ci_.stage = cs_modules[i].GetStageCreateInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;
        m_errorMonitor->SetDesiredError("VUID-VkComputePipelineCreateInfo-flags-11312");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, NoMappingMultipleMappings) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkDescriptorSetAndBindingMappingEXT mappings[5];
    mappings[0] = MakeSetAndBindingMapping(1, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = 0u;
    mappings[0].sourceData.constantOffset.heapArrayStride = 0;
    mappings[1] = MakeSetAndBindingMapping(0, 3);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = 0u;
    mappings[1].sourceData.constantOffset.heapArrayStride = 0;
    mappings[2] = MakeSetAndBindingMapping(0, 0, 2);
    mappings[2].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[2].sourceData.constantOffset.heapOffset = 0u;
    mappings[2].sourceData.constantOffset.heapArrayStride = 0;
    mappings[3] = MakeSetAndBindingMapping(0, 2, 1, VK_SPIRV_RESOURCE_TYPE_UNIFORM_BUFFER_BIT_EXT);
    mappings[3].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[3].sourceData.constantOffset.heapOffset = 0u;
    mappings[3].sourceData.constantOffset.heapArrayStride = 0;
    mappings[4] = MakeSetAndBindingMapping(0, 2, 1, VK_SPIRV_RESOURCE_TYPE_READ_ONLY_STORAGE_BUFFER_BIT_EXT);
    mappings[4].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[4].sourceData.constantOffset.heapOffset = 0u;
    mappings[4].sourceData.constantOffset.heapArrayStride = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 5;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 2) buffer Output { int result; };
        void main() {
            result = 0;
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);

    VkPipelineCreateFlags2CreateInfo pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;
    m_errorMonitor->SetDesiredError("VUID-VkComputePipelineCreateInfo-flags-11312");
    pipe.CreateComputePipeline(false);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, NoMappingMultipleMappingsAllUsed) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkDescriptorSetAndBindingMappingEXT mappings[3];
    for (uint32_t i = 0; i < 3; i++) {
        mappings[i] = MakeSetAndBindingMapping(0, i);
        mappings[i].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
        mappings[i].sourceData.constantOffset.heapOffset = 0u;
        mappings[i].sourceData.constantOffset.heapArrayStride = 0;
    }

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 3;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A0 { int a_0; };
        layout(set = 0, binding = 1) buffer A1 { int a_1; };
        layout(set = 0, binding = 2) buffer A2 { int a_2; };
        layout(set = 0, binding = 3) buffer A3 { int a_3; };
        void main() {
            a_0 = 0;
            a_1 = 0;
            a_2 = 0;
            a_3 = 0;
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);

    VkPipelineCreateFlags2CreateInfo pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;
    m_errorMonitor->SetDesiredError("VUID-VkComputePipelineCreateInfo-flags-11312");
    pipe.CreateComputePipeline(false);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, NoMappingMultipleMappingsMostUsed) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkDescriptorSetAndBindingMappingEXT mappings[9];
    for (uint32_t i = 0; i < 9; i++) {
        mappings[i] = MakeSetAndBindingMapping(0, i);
        mappings[i].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
        mappings[i].sourceData.constantOffset.heapOffset = 0u;
        mappings[i].sourceData.constantOffset.heapArrayStride = 0;
    }

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 9;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A0 { int a_0; };
        layout(set = 0, binding = 1) buffer A1 { int a_1; };
        layout(set = 0, binding = 2) buffer A2 { int a_2; };
        layout(set = 0, binding = 3) buffer A3 { int a_3; };
        layout(set = 0, binding = 4) buffer A4 { int a_4; };
        layout(set = 0, binding = 6) buffer A6 { int a_6; };
        layout(set = 0, binding = 7) buffer A7 { int a_7; };
        layout(set = 0, binding = 8) buffer A8 { int a_8; };
        layout(set = 0, binding = 9) buffer A9 { int a_9; };
        void main() {
            a_0 = 0;
            a_1 = 0;
            a_2 = 0;
            a_3 = 0;
            a_4 = 0;
            a_6 = 0;
            a_7 = 0;
            a_8 = 0;
            a_9 = 0;
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);

    VkPipelineCreateFlags2CreateInfo pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;
    m_errorMonitor->SetDesiredError("VUID-VkComputePipelineCreateInfo-flags-11312");
    pipe.CreateComputePipeline(false);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, NoMappingMultipleMappingsMany) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkDescriptorSetAndBindingMappingEXT mappings[512];
    for (uint32_t i = 0; i < 512; i++) {
        mappings[i] = MakeSetAndBindingMapping(0, i + 1);
        mappings[i].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
        mappings[i].sourceData.constantOffset.heapOffset = 0u;
        mappings[i].sourceData.constantOffset.heapArrayStride = 0;
    }

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 512;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A0 { int a_0; };
        layout(set = 0, binding = 10) buffer A1 { int a_1; };
        layout(set = 0, binding = 20) buffer A2 { int a_2; };
        layout(set = 0, binding = 30) buffer A3 { int a_3; };
        layout(set = 0, binding = 40) buffer A4 { int a_4; };
        layout(set = 0, binding = 60) buffer A6 { int a_6; };
        layout(set = 0, binding = 70) buffer A7 { int a_7; };
        layout(set = 0, binding = 80) buffer A8 { int a_8; };
        layout(set = 0, binding = 90) buffer A9 { int a_9; };
        void main() {
            a_0 = 0;
            a_1 = 0;
            a_2 = 0;
            a_3 = 0;
            a_4 = 0;
            a_6 = 0;
            a_7 = 0;
            a_8 = 0;
            a_9 = 0;
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);

    VkPipelineCreateFlags2CreateInfo pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;
    m_errorMonitor->SetDesiredError("VUID-VkComputePipelineCreateInfo-flags-11312");
    pipe.CreateComputePipeline(false);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, EmbeddedSamplerReservedArea) {
    TEST_DESCRIPTION("Validate that embedded sampler mapping have reserved area");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::computeDerivativeGroupQuads);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    RETURN_IF_SKIP(CheckSlangSupport());

    if (heap_props.minSamplerHeapReservedRangeWithEmbedded < 1) {
        GTEST_SKIP() << "Cannot test minSamplerHeapReservedRangeWithEmbedded less than 1";
    }

    const VkDeviceSize bdsize = 2 * heap_props.samplerDescriptorSize + heap_props.minSamplerHeapReservedRangeWithEmbedded;
    vkt::Buffer buffer(*m_device, bdsize, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    // reservedRangeOffset check
    VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
    bind_info.heapRange = {buffer.Address(), bdsize};
    bind_info.reservedRangeSize = heap_props.minSamplerHeapReservedRangeWithEmbedded - 1;

    // Resource descriptor heap buffer
    const VkDeviceSize resource_stride = Align(heap_props.bufferDescriptorSize, heap_props.samplerDescriptorAlignment);
    const VkDeviceSize resource_descriptor_count = 4;
    const VkDeviceSize resource_heap_size_app = AlignResource(resource_descriptor_count * resource_stride);
    const VkDeviceSize resource_heap_size = resource_heap_size_app + heap_props.minResourceHeapReservedRange;
    const VkDeviceSize out_data_buffer_size = 256;

    vkt::Buffer resource_heap(*m_device, resource_heap_size, VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);
    const auto resource_heap_ptr = static_cast<char*>(resource_heap.Memory().Map());

    // Sampler descriptor heap buffer
    const VkDeviceSize sampler_stride = heap_props.samplerDescriptorSize;
    const VkDeviceSize sampler_descriptor_count = 4;
    const VkDeviceSize sampler_heap_size_app = AlignResource(sampler_descriptor_count * sampler_stride);
    const VkDeviceSize sampler_heap_size = sampler_heap_size_app + heap_props.minSamplerHeapReservedRange;

    vkt::Buffer sampler_heap(*m_device, sampler_heap_size, VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);
    const auto sampler_heap_ptr = static_cast<char*>(sampler_heap.Memory().Map());

    // Output buffer descriptor
    vkt::Buffer out_buffer(*m_device, out_data_buffer_size, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    VkHostAddressRangeEXT out_descriptor{resource_heap_ptr, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT out_address_range = {out_buffer.Address(), out_data_buffer_size};
    VkResourceDescriptorInfoEXT out_descriptor_info = vku::InitStructHelper();
    out_descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    out_descriptor_info.data.pAddressRange = &out_address_range;
    vk::WriteResourceDescriptorsEXT(*m_device, 1, &out_descriptor_info, &out_descriptor);

    VkSamplerCreateInfo sampler_ci = vku::InitStructHelper();
    VkHostAddressRangeEXT input_descriptor{sampler_heap_ptr + sampler_stride, static_cast<size_t>(sampler_stride)};
    vk::WriteSamplerDescriptorsEXT(*m_device, 1, &sampler_ci, &input_descriptor);

    VkDescriptorSetAndBindingMappingEXT mappings[3];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = 0u * (uint32_t)resource_stride;
    mappings[0].sourceData.constantOffset.heapArrayStride = 0;

    VkSamplerCreateInfo embedded_sampler = vku::InitStructHelper();
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = 1u * (uint32_t)resource_stride;
    mappings[1].sourceData.constantOffset.heapArrayStride = 0;
    mappings[1].sourceData.constantOffset.pEmbeddedSampler = &embedded_sampler;

    mappings[2] = MakeSetAndBindingMapping(0, 2);
    mappings[2].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[2].sourceData.constantOffset.heapOffset = 2u * (uint32_t)resource_stride;
    mappings[2].sourceData.constantOffset.heapArrayStride = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 3;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer Output { int result[]; };
        layout(set = 0, binding = 1) uniform sampler samp;
        layout(set = 0, binding = 2) uniform texture2D tex;
        void main() {
            ivec2 uv = ivec2(gl_GlobalInvocationID.xy);
            vec2 texCoord = vec2(gl_GlobalInvocationID.xy) / 1.0;
            vec4 color = texture(sampler2D(tex, samp), texCoord);
            result[gl_LocalInvocationIndex] = int(color.r);
        }
    )glsl";

    const char* slang_shader = R"slang(
        [[vk::binding(0, 0)]]
        RWStructuredBuffer<int> result;

        [[vk::binding(1, 0)]]
        SamplerState samp;

        [[vk::binding(2, 0)]]
        Texture2D<float4> tex;

        [numthreads(2, 2, 2)]
        void main(uint3 dispatchThreadID : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex) {
            int2 uv = int2(dispatchThreadID.xy);
            float2 texCoord = float2(dispatchThreadID.xy) / 1.0;
            float4 color = tex.Sample(samp, texCoord);
            result[groupIndex] = int(color.r);
        }
    )slang";

    VkShaderObj cs_modules[2];
    cs_modules[0] = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    cs_modules[1] = VkShaderObj(*m_device, slang_shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_SLANG);

    for (int i = 0; i < 2; ++i) {
        VkPipelineCreateFlags2CreateInfo pipeline_create_flags_2_create_info = vku::InitStructHelper();
        pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

        CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
        pipe.cp_ci_.layout = VK_NULL_HANDLE;
        pipe.cp_ci_.stage = cs_modules[i].GetStageCreateInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        pipe.CreateComputePipeline(false);

        m_command_buffer.Begin();

        vk::CmdBindSamplerHeapEXT(m_command_buffer, &bind_info);
        vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
        m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-pBindInfo-11375");
        vk::CmdDispatch(m_command_buffer, 1, 1, 1);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
        m_default_queue->SubmitAndWait(m_command_buffer);
    }
}

TEST_F(NegativeDescriptorHeap, EmbeddedSamplerArray) {
    TEST_DESCRIPTION("Validate that embedded sampler mapping as an array");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::computeDerivativeGroupQuads);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    RETURN_IF_SKIP(CheckSlangSupport());

    char const* cs_source_glsl = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer Output { int result[]; };
        layout(set = 0, binding = 1) uniform sampler samp[2];
        layout(set = 0, binding = 2) uniform texture2D tex;
        void main() {
            ivec2 uv = ivec2(gl_GlobalInvocationID.xy);
            vec2 texCoord = vec2(uv) / 1.0;
            vec4 color = texture(sampler2D(tex, samp[0]), texCoord);
            result[gl_LocalInvocationIndex] = int(color.r);
        }
    )glsl";

    const char* cs_source_hlsl = R"slang(
        [[vk::binding(0, 0)]]
        RWStructuredBuffer<int> result[];

        [[vk::binding(1, 0)]]
        uniform SamplerState samp[2];

        [[vk::binding(2, 0)]]
        uniform Texture2D tex;

        [numthreads(2, 2, 2)]
        void main(uint3 dispatchThreadID : SV_DispatchThreadID) {
            int2 uv = int2(dispatchThreadID[0], dispatchThreadID[1]);
            float2 texCoord = float2(uv);
            float4 color = tex.Sample(samp[0], texCoord);
            result[dispatchThreadID.x][0] = int(color.r);
        }
    )slang";

    VkShaderObj cs_modules[2] = {};
    cs_modules[0] = VkShaderObj(*m_device, cs_source_glsl, VK_SHADER_STAGE_COMPUTE_BIT);
    cs_modules[1] = VkShaderObj(*m_device, cs_source_hlsl, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_SLANG);
    m_errorMonitor->VerifyFound();

    const VkDeviceSize bdsize = 2 * heap_props.samplerDescriptorSize + heap_props.minSamplerHeapReservedRangeWithEmbedded;
    vkt::Buffer buffer(*m_device, bdsize, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);

    // reservedRangeOffset check
    VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
    bind_info.heapRange = {buffer.Address(), bdsize};
    bind_info.reservedRangeSize = heap_props.minSamplerHeapReservedRangeWithEmbedded;
    m_command_buffer.Begin();
    vk::CmdBindSamplerHeapEXT(m_command_buffer, &bind_info);
    m_command_buffer.End();

    const VkDeviceSize resource_stride = Align(heap_props.bufferDescriptorSize, heap_props.samplerDescriptorAlignment);

    VkDescriptorSetAndBindingMappingEXT mappings[3];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = 0u * (uint32_t)resource_stride;
    mappings[0].sourceData.constantOffset.heapArrayStride = 0;

    VkSamplerCreateInfo embedded_sampler = vku::InitStructHelper();
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = 1u * (uint32_t)resource_stride;
    mappings[1].sourceData.constantOffset.heapArrayStride = 0;
    mappings[1].sourceData.constantOffset.pEmbeddedSampler = &embedded_sampler;

    mappings[2] = MakeSetAndBindingMapping(0, 2);
    mappings[2].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[2].sourceData.constantOffset.heapOffset = 2u * (uint32_t)resource_stride;
    mappings[2].sourceData.constantOffset.heapArrayStride = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 3;
    mapping_info.pMappings = mappings;

    VkPipelineCreateFlags2CreateInfo pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    for (int i = 0; i < 2; ++i) {
        CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
        pipe.cp_ci_.layout = VK_NULL_HANDLE;
        pipe.cp_ci_.stage = cs_modules[i].GetStageCreateInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        m_errorMonitor->SetDesiredError("VUID-VkPipelineShaderStageCreateInfo-pNext-11399");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, PushDataAssignedPipeline) {
    TEST_DESCRIPTION("Check that push data used in shader code has been set for pipeline based shaders");
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const char* cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(push_constant, std430) uniform foo { int x[4]; } constants;
        layout(set = 0, binding = 0) buffer Output { int result[]; };
        void main() {
            result[gl_LocalInvocationID.x] = constants.x[gl_LocalInvocationID.x];
        }
    )glsl";

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    const VkDeviceSize resource_descriptor_count = 4;
    const VkDeviceSize resource_heap_size_app = AlignResource(resource_descriptor_count * resource_stride);
    const VkDeviceSize resource_heap_size = resource_heap_size_app + heap_props.minResourceHeapReservedRange;
    const VkDeviceSize dataBufferSize = 256;
    for (int i = 0; i < 2; i++) {
        vkt::Buffer resource_heap(*m_device, resource_heap_size, VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);
        const auto resource_heap_ptr = static_cast<char*>(resource_heap.Memory().Map());

        // Output buffer descriptor
        vkt::Buffer out_buffer(*m_device, dataBufferSize, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT, vkt::device_address);

        VkHostAddressRangeEXT out_descriptor{resource_heap_ptr, static_cast<size_t>(resource_stride)};
        VkDeviceAddressRangeEXT out_address_range = {out_buffer.Address(), dataBufferSize};
        VkResourceDescriptorInfoEXT out_descriptor_info = vku::InitStructHelper();
        out_descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        out_descriptor_info.data.pAddressRange = &out_address_range;

        vk::WriteResourceDescriptorsEXT(*m_device, 1, &out_descriptor_info, &out_descriptor);

        VkDescriptorSetAndBindingMappingEXT mappings = MakeSetAndBindingMapping(0, 0);
        mappings.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
        mappings.sourceData.constantOffset.heapOffset = 0u * (uint32_t)resource_stride;
        mappings.sourceData.constantOffset.heapArrayStride = 0;

        VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
        mapping_info.mappingCount = 1;
        mapping_info.pMappings = &mappings;

        VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

        VkPipelineCreateFlags2CreateInfo pipeline_create_flags_2_create_info = vku::InitStructHelper();
        pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

        CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
        pipe.cp_ci_.layout = VK_NULL_HANDLE;
        pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;

        std::vector<uint8_t> payload(4);
        VkPushDataInfoEXT push_data_info = vku::InitStructHelper();
        push_data_info.data.address = payload.data();
        push_data_info.data.size = payload.size();
        push_data_info.offset = 0;

        pipe.CreateComputePipeline(false);

        m_command_buffer.Begin();
        vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
        if (i != 0) {
            vk::CmdPushDataEXT(m_command_buffer, &push_data_info);
        }
        m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11376");
        vk::CmdDispatch(m_command_buffer, 1, 1, 1);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }
}

TEST_F(NegativeDescriptorHeap, PushDataAssignedShaderObject) {
    TEST_DESCRIPTION("Check that push data used in shader code has been set using shader object");
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    static const char cs_source[] = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(push_constant, std430) uniform foo { int x[4]; } constants;
        layout(set = 0, binding = 0) buffer Output { int result[]; };
        void main() {
            result[gl_LocalInvocationID.x] = constants.x[gl_LocalInvocationID.x];
        }
        )glsl";
    const auto spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, cs_source);

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    const VkDeviceSize resource_descriptor_count = 4;
    const VkDeviceSize resource_heap_size_app = AlignResource(resource_descriptor_count * resource_stride);
    const VkDeviceSize resource_heap_size = resource_heap_size_app + heap_props.minResourceHeapReservedRange;
    const VkDeviceSize dataBufferSize = 256;

    for (int i = 0; i < 2; i++) {
        vkt::Buffer resource_heap(*m_device, resource_heap_size, VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);
        const auto resource_heap_ptr = static_cast<char*>(resource_heap.Memory().Map());

        // Output buffer descriptor
        vkt::Buffer out_buffer(*m_device, dataBufferSize, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT, vkt::device_address);

        VkHostAddressRangeEXT out_descriptor{resource_heap_ptr, static_cast<size_t>(resource_stride)};
        VkDeviceAddressRangeEXT out_address_range = {out_buffer.Address(), dataBufferSize};
        VkResourceDescriptorInfoEXT out_descriptor_info = vku::InitStructHelper();
        out_descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        out_descriptor_info.data.pAddressRange = &out_address_range;

        vk::WriteResourceDescriptorsEXT(*m_device, 1, &out_descriptor_info, &out_descriptor);

        VkDescriptorSetAndBindingMappingEXT mappings = MakeSetAndBindingMapping(0, 0);
        mappings.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
        mappings.sourceData.constantOffset.heapOffset = 0u * (uint32_t)resource_stride;
        mappings.sourceData.constantOffset.heapArrayStride = 0;

        VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
        mapping_info.mappingCount = 1;
        mapping_info.pMappings = &mappings;

        std::vector<uint8_t> payload(4);
        VkPushDataInfoEXT push_data_info = vku::InitStructHelper();
        push_data_info.data.address = payload.data();
        push_data_info.data.size = payload.size();
        push_data_info.offset = 0;

        auto shader_ci = ShaderCreateInfoFlag(spv, VK_SHADER_STAGE_COMPUTE_BIT, VK_SHADER_CREATE_DESCRIPTOR_HEAP_BIT_EXT);
        shader_ci.pNext = &mapping_info;
        const vkt::Shader comp_shader(*m_device, shader_ci);

        m_command_buffer.Begin();
        if (i != 0) {
            vk::CmdPushDataEXT(m_command_buffer, &push_data_info);
        }
        m_command_buffer.BindCompShader(comp_shader);

        m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11376");
        vk::CmdDispatch(m_command_buffer, 1u, 1u, 1u);
        m_errorMonitor->VerifyFound();

        m_command_buffer.End();
    }
}

TEST_F(NegativeDescriptorHeap, BufferDescriptorHeapUsageMemoryAllocationInfo) {
    TEST_DESCRIPTION("Validate that memory allocated for descriptor heap contains buffer device address");
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = 0;

    m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-buffer-11408");
    vkt::Buffer buffer(*m_device, heap_props.bufferDescriptorSize, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, kHostVisibleMemProps,
                       &allocate_flag_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, DuplicatedPushDataSequenceIndex) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::deviceGeneratedCommands);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkIndirectCommandsPushConstantTokenEXT pc_token_0;
    pc_token_0.updateRange = {VK_SHADER_STAGE_ALL, 0, 4};

    VkIndirectCommandsPushConstantTokenEXT pc_token_1;
    pc_token_1.updateRange = {VK_SHADER_STAGE_ALL, 4, 4};

    VkIndirectCommandsPushConstantTokenEXT pc_token_2;
    pc_token_2.updateRange = {VK_SHADER_STAGE_ALL, 8, 4};

    VkIndirectCommandsLayoutTokenEXT tokens[3];
    tokens[0] = vku::InitStructHelper();
    tokens[0].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_DATA_SEQUENCE_INDEX_EXT;
    tokens[0].data.pPushConstant = &pc_token_0;
    tokens[0].offset = 0;
    tokens[1] = vku::InitStructHelper();
    tokens[1].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_DATA_SEQUENCE_INDEX_EXT;
    tokens[1].data.pPushConstant = &pc_token_1;
    tokens[1].offset = 0;
    tokens[2] = vku::InitStructHelper();
    tokens[2].type = VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT;
    tokens[2].data.pPushConstant = &pc_token_2;
    tokens[2].offset = 0;

    const std::vector<VkPushConstantRange> pc_range = {{VK_SHADER_STAGE_VERTEX_BIT, 0, 64}};
    vkt::PipelineLayout pipeline_layout(*m_device, {}, pc_range);
    VkIndirectCommandsLayoutCreateInfoEXT command_layout_ci = vku::InitStructHelper();
    command_layout_ci.shaderStages = VK_SHADER_STAGE_VERTEX_BIT;
    command_layout_ci.pipelineLayout = pipeline_layout;
    command_layout_ci.tokenCount = sizeof(tokens) / sizeof(tokens[0]);
    command_layout_ci.pTokens = tokens;

    VkIndirectCommandsLayoutEXT command_layout;
    m_errorMonitor->SetDesiredError("VUID-VkIndirectCommandsLayoutCreateInfoEXT-pTokens-11145");
    vk::CreateIndirectCommandsLayoutEXT(device(), &command_layout_ci, nullptr, &command_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, CaptureReplayBit) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::descriptorHeapCaptureReplay);

    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.flags = VK_IMAGE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.width = 128;
    image_create_info.extent.height = 128;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.format = VK_FORMAT_D32_SFLOAT;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    vkt::Image temp_image(*m_device, image_create_info, vkt::no_mem);

    VkMemoryRequirements mem_reqs;
    vk::GetImageMemoryRequirements(device(), temp_image, &mem_reqs);

    auto mem_alloc_info = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkt::DeviceMemory mem(*m_device, mem_alloc_info);

    m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-image-08113");
    m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-image-09202");
    vk::BindImageMemory(device(), temp_image, mem, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, ImageType3DView3D) {
    TEST_DESCRIPTION("Validate that input attachment cannot be a 3D image");
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDescriptorType type = {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT};
    const VkDeviceSize size = vk::GetPhysicalDeviceDescriptorSizeEXT(Gpu(), type);
    if (size > 0) {
        VkImageCreateInfo image_ci = vku::InitStructHelper();
        image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
        image_ci.usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = 1;
        image_ci.extent = {8, 8, 8};
        image_ci.imageType = VK_IMAGE_TYPE_3D;
        vkt::Image image(*m_device, image_ci);

        VkResourceDescriptorInfoEXT resource_desc_info = vku::InitStructHelper();
        resource_desc_info.type = type;

        VkImageViewCreateInfo image_view_ci = vku::InitStructHelper();
        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_3D;
        image_view_ci.format = image_ci.format;
        image_view_ci.image = image;
        image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        image_view_ci.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};

        VkImageDescriptorInfoEXT image_info = vku::InitStructHelper();
        image_info.pView = &image_view_ci;
        image_info.layout = VK_IMAGE_LAYOUT_GENERAL;
        resource_desc_info.data.pImage = &image_info;

        auto data = std::make_unique<uint8_t[]>(static_cast<size_t>(size));
        VkHostAddressRangeEXT descriptors = {data.get(), static_cast<size_t>(size)};

        m_errorMonitor->SetDesiredError("VUID-VkResourceDescriptorInfoEXT-type-11422");
        vk::WriteResourceDescriptorsEXT(device(), 1u, &resource_desc_info, &descriptors);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, ImageType3DView2DArray) {
    TEST_DESCRIPTION("Validate that input attachment cannot be a 3D image even if viewtype is 2D array");
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDescriptorType types[] = {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                      VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT};

    for (auto type : types) {
        const VkDeviceSize size = vk::GetPhysicalDeviceDescriptorSizeEXT(Gpu(), type);
        if (size > 0) {
            const VkImageUsageFlagBits usage = {type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE   ? VK_IMAGE_USAGE_SAMPLED_BIT
                                                : type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ? VK_IMAGE_USAGE_STORAGE_BIT
                                                                                           : VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT};
            VkImageCreateInfo image_ci = vku::InitStructHelper();
            image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
            image_ci.usage = usage;
            image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
            image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
            image_ci.mipLevels = 1;
            image_ci.arrayLayers = 1;
            image_ci.extent = {8, 8, 8};
            image_ci.imageType = VK_IMAGE_TYPE_3D;
            vkt::Image image(*m_device, image_ci);

            VkResourceDescriptorInfoEXT resource_desc_info = vku::InitStructHelper();
            resource_desc_info.type = type;

            VkImageViewCreateInfo image_view_ci = vku::InitStructHelper();
            image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            image_view_ci.format = image_ci.format;
            image_view_ci.image = image;
            image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
            image_view_ci.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B,
                                        VK_COMPONENT_SWIZZLE_A};

            VkImageDescriptorInfoEXT image_info = vku::InitStructHelper();
            image_info.pView = &image_view_ci;
            image_info.layout = VK_IMAGE_LAYOUT_GENERAL;
            resource_desc_info.data.pImage = &image_info;

            auto data = std::make_unique<uint8_t[]>(static_cast<size_t>(size));
            VkHostAddressRangeEXT descriptors = {data.get(), static_cast<size_t>(size)};

            if (type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
                m_errorMonitor->SetDesiredError("VUID-VkResourceDescriptorInfoEXT-type-11422");
            }
            m_errorMonitor->SetDesiredError("VUID-VkImageDescriptorInfoEXT-pView-11426");
            vk::WriteResourceDescriptorsEXT(device(), 1u, &resource_desc_info, &descriptors);
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(NegativeDescriptorHeap, ImageType3DViewImage2DViewOf3D) {
    TEST_DESCRIPTION(
        "Validate that input attachment cannot be a 3D image even if view type 2D and image2DViewOf3D feature is not enabled");
    AddRequiredExtensions(VK_EXT_IMAGE_2D_VIEW_OF_3D_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDescriptorType type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    const VkDeviceSize size = vk::GetPhysicalDeviceDescriptorSizeEXT(Gpu(), type);
    if (size > 0) {
        VkImageCreateInfo image_ci = vku::InitStructHelper();
        image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
        image_ci.usage = VK_IMAGE_USAGE_STORAGE_BIT;
        image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = 1;
        image_ci.extent = {8, 8, 8};
        image_ci.imageType = VK_IMAGE_TYPE_3D;
        image_ci.flags = VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT;
        vkt::Image image(*m_device, image_ci);

        VkResourceDescriptorInfoEXT resource_desc_info = vku::InitStructHelper();
        resource_desc_info.type = type;

        VkImageViewCreateInfo image_view_ci = vku::InitStructHelper();
        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_ci.format = image_ci.format;
        image_view_ci.image = image;
        image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        image_view_ci.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};

        VkImageDescriptorInfoEXT image_info = vku::InitStructHelper();
        image_info.pView = &image_view_ci;
        image_info.layout = VK_IMAGE_LAYOUT_GENERAL;
        resource_desc_info.data.pImage = &image_info;

        auto data = std::make_unique<uint8_t[]>(static_cast<size_t>(size));
        VkHostAddressRangeEXT descriptors = {data.get(), static_cast<size_t>(size)};

        m_errorMonitor->SetDesiredError("VUID-VkResourceDescriptorInfoEXT-type-11424");
        vk::WriteResourceDescriptorsEXT(device(), 1u, &resource_desc_info, &descriptors);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, ImageType3DViewSampler2DViewOf3D) {
    TEST_DESCRIPTION(
        "Validate that input attachment cannot be a 3D image even if view type 2D and sampler2DViewOf3D feature is not enabled");
    AddRequiredExtensions(VK_EXT_IMAGE_2D_VIEW_OF_3D_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDescriptorType type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    const VkDeviceSize size = vk::GetPhysicalDeviceDescriptorSizeEXT(Gpu(), type);
    if (size > 0) {
        VkImageCreateInfo image_ci = vku::InitStructHelper();
        image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
        image_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
        image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = 1;
        image_ci.extent = {8, 8, 8};
        image_ci.imageType = VK_IMAGE_TYPE_3D;
        image_ci.flags = VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT;
        vkt::Image image(*m_device, image_ci);

        VkResourceDescriptorInfoEXT resource_desc_info = vku::InitStructHelper();
        resource_desc_info.type = type;

        VkImageViewCreateInfo image_view_ci = vku::InitStructHelper();
        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_ci.format = image_ci.format;
        image_view_ci.image = image;
        image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        image_view_ci.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};

        VkImageDescriptorInfoEXT image_info = vku::InitStructHelper();
        image_info.pView = &image_view_ci;
        image_info.layout = VK_IMAGE_LAYOUT_GENERAL;
        resource_desc_info.data.pImage = &image_info;

        auto data = std::make_unique<uint8_t[]>(static_cast<size_t>(size));
        VkHostAddressRangeEXT descriptors = {data.get(), static_cast<size_t>(size)};

        m_errorMonitor->SetDesiredError("VUID-VkResourceDescriptorInfoEXT-type-11425");
        vk::WriteResourceDescriptorsEXT(device(), 1u, &resource_desc_info, &descriptors);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, ImageType3DView2DWithoutCompatibility) {
    TEST_DESCRIPTION("Validate that image cannot be a 3D image if viewtype is 2D and no compatibility bit");
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize size = vk::GetPhysicalDeviceDescriptorSizeEXT(Gpu(), VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.extent = {8, 8, 8};
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_ci.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    vkt::Image image(*m_device, image_ci);

    VkResourceDescriptorInfoEXT resource_desc_info = vku::InitStructHelper();
    resource_desc_info.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

    VkImageViewCreateInfo image_view_ci = vku::InitStructHelper();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_ci.format = image_ci.format;
    image_view_ci.image = image;
    image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    image_view_ci.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};

    VkImageDescriptorInfoEXT image_info = vku::InitStructHelper();
    image_info.pView = &image_view_ci;
    image_info.layout = VK_IMAGE_LAYOUT_GENERAL;
    resource_desc_info.data.pImage = &image_info;

    auto data = std::make_unique<uint8_t[]>(static_cast<size_t>(size));
    VkHostAddressRangeEXT descriptors = {data.get(), static_cast<size_t>(size)};

    m_errorMonitor->SetDesiredError("VUID-VkImageDescriptorInfoEXT-pView-11427");
    m_errorMonitor->SetDesiredError("VUID-VkResourceDescriptorInfoEXT-type-11422");
    vk::WriteResourceDescriptorsEXT(device(), 1u, &resource_desc_info, &descriptors);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, ImageTypeDepthStencilAttachment) {
    TEST_DESCRIPTION("Validate that aspectMask contains depth or stencil if image is a depth stencil format");
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize size = vk::GetPhysicalDeviceDescriptorSizeEXT(Gpu(), VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);

    auto ds_format = FindSupportedDepthStencilFormat(Gpu());

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.format = ds_format;
    image_ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.extent = {8, 8, 1};
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    vkt::Image image(*m_device, image_ci);

    VkResourceDescriptorInfoEXT resource_desc_info = vku::InitStructHelper();
    resource_desc_info.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

    VkImageViewCreateInfo image_view_ci = vku::InitStructHelper();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_ci.format = image_ci.format;
    image_view_ci.image = image;
    image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1};
    image_view_ci.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};

    VkImageDescriptorInfoEXT image_info = vku::InitStructHelper();
    image_info.pView = &image_view_ci;
    image_info.layout = VK_IMAGE_LAYOUT_GENERAL;
    resource_desc_info.data.pImage = &image_info;

    auto data = std::make_unique<uint8_t[]>(static_cast<size_t>(size));
    VkHostAddressRangeEXT descriptors = {data.get(), static_cast<size_t>(size)};

    m_errorMonitor->SetDesiredError("VUID-VkImageDescriptorInfoEXT-pView-11430");
    vk::WriteResourceDescriptorsEXT(device(), 1u, &resource_desc_info, &descriptors);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, PushDataRange) {
    TEST_DESCRIPTION("Descriptor heap with VkPushDataInfoEXT but part of the range is missing");
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    const VkDeviceSize resource_descriptor_count = 1u;
    const VkDeviceSize resource_heap_size_app = AlignResource(resource_descriptor_count);
    const VkDeviceSize resource_heap_size = resource_heap_size_app + heap_props.minResourceHeapReservedRange;

    vkt::Buffer descriptor_heap(*m_device, resource_heap_size, VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);
    const auto descriptor_heap_ptr = static_cast<char*>(descriptor_heap.Memory().Map());

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    VkHostAddressRangeEXT descriptor_host = {descriptor_heap_ptr, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT device_range = {buffer.Address(), 16};
    VkResourceDescriptorInfoEXT descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info.data.pAddressRange = &device_range;

    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = 0;
    mapping.sourceData.constantOffset.heapArrayStride = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer A { vec4 a; };
        layout(push_constant) uniform PushConstant {
            vec4 b;
        };
        void main() {
            a = b;
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;
    pipe.CreateComputePipeline(false);

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange.address = descriptor_heap.Address();
    bind_resource_info.heapRange.size = resource_heap_size;
    bind_resource_info.reservedRangeOffset = resource_heap_size_app;
    bind_resource_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;

    float src_data[4] = {1.0f, 2.0f, 3.0f, 4.0f};

    VkPushDataInfoEXT push_data_info1 = vku::InitStructHelper();
    push_data_info1.offset = 0u;
    push_data_info1.data.size = 8u;
    push_data_info1.data.address = src_data;

    VkPushDataInfoEXT push_data_info2 = vku::InitStructHelper();
    push_data_info2.offset = 12u;
    push_data_info2.data.size = 4u;
    push_data_info2.data.address = src_data;

    // Set from 0-7 and 12-15, unset from 8-11
    m_command_buffer.Begin();
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);
    vk::CmdPushDataEXT(m_command_buffer, &push_data_info1);
    vk::CmdPushDataEXT(m_command_buffer, &push_data_info2);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11376");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    push_data_info1.offset = 4u;
    push_data_info1.data.size = 16u;

    // Set from 4-19, unset from 0-3
    m_command_buffer.Begin();
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);
    vk::CmdPushDataEXT(m_command_buffer, &push_data_info1);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11376");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    push_data_info1.offset = 0u;
    push_data_info1.data.size = 8u;
    push_data_info2.offset = 4u;
    push_data_info2.data.size = 8u;
    VkPushDataInfoEXT push_data_info3 = vku::InitStructHelper();
    push_data_info3.offset = 8u;
    push_data_info3.data.size = 8u;
    push_data_info3.data.address = src_data;

    // Set multiple times with overlap
    m_command_buffer.Begin();
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);
    vk::CmdPushDataEXT(m_command_buffer, &push_data_info1);
    vk::CmdPushDataEXT(m_command_buffer, &push_data_info2);
    vk::CmdPushDataEXT(m_command_buffer, &push_data_info3);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    // Unset after command buffer reset
    m_command_buffer.Begin();
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11376");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    push_data_info1.offset = 0u;
    push_data_info1.data.size = 16u;

    const std::vector<VkPushConstantRange> pc_range = {{VK_SHADER_STAGE_COMPUTE_BIT, 0, 16}};
    vkt::PipelineLayout pipeline_layout(*m_device, {}, pc_range);

    // Full range set, invalidated by vkCmdPushConstants, only part of the range set again
    m_command_buffer.Begin();
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);
    vk::CmdPushDataEXT(m_command_buffer, &push_data_info1);
    vk::CmdPushConstants(m_command_buffer, pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0u, 16u, src_data);
    push_data_info1.data.size = 8u;
    vk::CmdPushDataEXT(m_command_buffer, &push_data_info1);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11376");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(NegativeDescriptorHeap, NonConstantImageMemoryAccess) {
    TEST_DESCRIPTION("Non constant image memory access with incompatible mapping source");
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkDescriptorSetAndBindingMappingEXT mappings[3];
    mappings[0] = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_SAMPLED_IMAGE_BIT_EXT);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT;
    mappings[0].sourceData.pushAddressOffset = 0;
    mappings[1] = MakeSetAndBindingMapping(0, 1, 1, VK_SPIRV_RESOURCE_TYPE_SAMPLER_BIT_EXT);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset = {};
    mappings[1].sourceData.constantOffset.heapOffset = 1024;
    mappings[2] = MakeSetAndBindingMapping(1, 0);
    mappings[2].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[2].sourceData.constantOffset = {};
    mappings[2].sourceData.constantOffset.heapOffset = 2048;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 3u;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set = 0, binding = 0) uniform texture2D tex[];
        layout(set = 0, binding = 1) uniform sampler sampl;
        layout(set = 1, binding = 0) buffer ssbo {
	        uint index;
	        vec4 data;
        };
        void main() {
	        data = texture(sampler2D(tex[index], sampl), vec2(0.5f));
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    VkPipelineCreateFlags2CreateInfoKHR flags2_ci = vku::InitStructHelper();
    flags2_ci.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &flags2_ci);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-DescriptorSet-11385");
    pipe.CreateComputePipeline(false);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, ArrayLengthOnMappingSourceAddress) {
    TEST_DESCRIPTION("OpArrayLength instruction on incompatible mapping source type");
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT;
    mapping.sourceData.pushAddressOffset = 0u;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer Data { uint data[]; };
        void main() {
	        data[0] = data.length();
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    VkPipelineCreateFlags2CreateInfoKHR flags2_ci = vku::InitStructHelper();
    flags2_ci.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &flags2_ci);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;

    m_errorMonitor->SetDesiredError("VUID-VkPipelineShaderStageCreateInfo-pNext-11378");
    pipe.CreateComputePipeline(false);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, SamplerAllocationCount) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::descriptorHeap);
    RETURN_IF_SKIP(InitFramework());

    const int max_samplers = 8;
    VkSampler samplers[max_samplers + 1] = {};

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }
    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(Gpu(), &props.limits);
    if (props.limits.maxSamplerAllocationCount > max_samplers) {
        props.limits.maxSamplerAllocationCount = max_samplers;
        fpvkSetPhysicalDeviceLimitsEXT(Gpu(), &props.limits);
    }
    RETURN_IF_SKIP(InitState());

    GetPhysicalDeviceProperties2(heap_props);
    if (heap_props.minSamplerHeapReservedRangeWithEmbedded == 0) {
        GTEST_SKIP() << "minSamplerHeapReservedRangeWithEmbedded is 0";
    }
    if (heap_props.minSamplerHeapReservedRangeWithEmbedded / heap_props.samplerDescriptorSize > max_samplers) {
        GTEST_SKIP() << "minSamplerHeapReservedRangeWithEmbedded / samplerDescriptorSize is too large";
    }

    VkSamplerCreateInfo embedded_sampler = vku::InitStructHelper();
    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_ONLY_IMAGE_BIT_EXT);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.pEmbeddedSampler = &embedded_sampler;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    VkPipelineCreateFlags2CreateInfoKHR flags2_ci = vku::InitStructHelper();
    flags2_ci.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;
    CreateComputePipelineHelper pipe(*this, &flags2_ci);
    pipe.LateBindPipelineInfo();
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage.pNext = &mapping_info;
    pipe.CreateComputePipeline(false);

    m_errorMonitor->SetDesiredError("VUID-vkCreateSampler-maxSamplerAllocationCount-11412");

    VkSamplerCreateInfo sampler_create_info = SafeSaneSamplerCreateInfo();

    VkResult err = VK_SUCCESS;
    int i;
    for (i = 0; i < max_samplers; i++) {
        err = vk::CreateSampler(device(), &sampler_create_info, NULL, &samplers[i]);
        if (err != VK_SUCCESS) {
            break;
        }
    }
    m_errorMonitor->VerifyFound();

    for (int j = 0; j < i; j++) {
        vk::DestroySampler(device(), samplers[j], NULL);
    }
}

TEST_F(NegativeDescriptorHeap, SamplerAllocationCountPipeline) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(InitFramework());

    const int max_samplers = 8;
    VkSampler samplers[max_samplers + 1] = {};

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }
    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(Gpu(), &props.limits);
    if (props.limits.maxSamplerAllocationCount > max_samplers) {
        props.limits.maxSamplerAllocationCount = max_samplers;
        fpvkSetPhysicalDeviceLimitsEXT(Gpu(), &props.limits);
    }
    RETURN_IF_SKIP(InitState());

    GetPhysicalDeviceProperties2(heap_props);
    if (heap_props.minSamplerHeapReservedRangeWithEmbedded == 0) {
        GTEST_SKIP() << "minSamplerHeapReservedRangeWithEmbedded is 0";
    }
    if (heap_props.minSamplerHeapReservedRangeWithEmbedded / heap_props.samplerDescriptorSize > max_samplers) {
        GTEST_SKIP() << "minSamplerHeapReservedRangeWithEmbedded / samplerDescriptorSize is too large";
    }

    VkSamplerCreateInfo sampler_create_info = SafeSaneSamplerCreateInfo();

    VkResult err = VK_SUCCESS;
    int i;
    for (i = 0; i < max_samplers; i++) {
        err = vk::CreateSampler(device(), &sampler_create_info, NULL, &samplers[i]);
        if (err != VK_SUCCESS) {
            break;
        }
    }

    m_errorMonitor->SetDesiredError("VUID-vkCreateComputePipelines-pCreateInfos-11414");

    VkSamplerCreateInfo embedded_sampler = vku::InitStructHelper();
    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_ONLY_IMAGE_BIT_EXT);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.pEmbeddedSampler = &embedded_sampler;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    VkPipelineCreateFlags2CreateInfo pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.LateBindPipelineInfo();
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage.pNext = &mapping_info;
    pipe.CreateComputePipeline(false);

    m_errorMonitor->VerifyFound();

    for (int j = 0; j < i; j++) {
        vk::DestroySampler(device(), samplers[j], NULL);
    }
}

TEST_F(NegativeDescriptorHeap, SamplerAllocationCountShaderObject) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitFramework());

    const int max_samplers = 8;
    VkSampler samplers[max_samplers + 1] = {};

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }
    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(Gpu(), &props.limits);
    if (props.limits.maxSamplerAllocationCount > max_samplers) {
        props.limits.maxSamplerAllocationCount = max_samplers;
        fpvkSetPhysicalDeviceLimitsEXT(Gpu(), &props.limits);
    }
    RETURN_IF_SKIP(InitState());

    GetPhysicalDeviceProperties2(heap_props);
    if (heap_props.minSamplerHeapReservedRangeWithEmbedded == 0) {
        GTEST_SKIP() << "minSamplerHeapReservedRangeWithEmbedded is 0";
    }
    if (heap_props.minSamplerHeapReservedRangeWithEmbedded / heap_props.samplerDescriptorSize > max_samplers) {
        GTEST_SKIP() << "minSamplerHeapReservedRangeWithEmbedded / samplerDescriptorSize is too large";
    }

    VkSamplerCreateInfo sampler_create_info = SafeSaneSamplerCreateInfo();

    VkResult err = VK_SUCCESS;
    int i;
    for (i = 0; i < max_samplers; i++) {
        err = vk::CreateSampler(device(), &sampler_create_info, NULL, &samplers[i]);
        if (err != VK_SUCCESS) {
            break;
        }
    }

    VkSamplerCreateInfo embedded_sampler = vku::InitStructHelper();
    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_ONLY_IMAGE_BIT_EXT);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.pEmbeddedSampler = &embedded_sampler;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    const VkShaderCreateFlagsEXT flags = VK_SHADER_CREATE_DESCRIPTOR_HEAP_BIT_EXT;
    VkShaderCreateInfoEXT vert_ci = ShaderCreateInfoFlag(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, flags);
    vert_ci.pNext = &mapping_info;

    m_errorMonitor->SetDesiredError("VUID-vkCreateShadersEXT-pCreateInfos-11413");
    const vkt::Shader vertShader(*m_device, vert_ci);
    m_errorMonitor->VerifyFound();

    for (int j = 0; j < i; j++) {
        vk::DestroySampler(device(), samplers[j], NULL);
    }
}

TEST_F(NegativeDescriptorHeap, SamplerAllocationTotalCountPipeline) {
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkPhysicalDeviceLimits& limits = m_device->Physical().limits_;
    if (limits.maxSamplerAllocationCount < heap_props.maxDescriptorHeapEmbeddedSamplers) {
        GTEST_SKIP() << "maxSamplerAllocationCount < maxDescriptorHeapEmbeddedSamplers";
    }
    if (heap_props.maxDescriptorHeapEmbeddedSamplers > 2048) {
        GTEST_SKIP() << "maxDescriptorHeapEmbeddedSamplers too large to run the test";
    }

    const size_t max_samplers = heap_props.maxDescriptorHeapEmbeddedSamplers - 1;
    std::vector<VkSampler> samplers(max_samplers);

    VkSamplerCreateInfo sampler_create_info = SafeSaneSamplerCreateInfo();

    VkResult err = VK_SUCCESS;
    for (size_t i = 0; i < max_samplers; i++) {
        err = vk::CreateSampler(device(), &sampler_create_info, nullptr, &samplers[i]);
        if (err != VK_SUCCESS) {
            break;
        }
    }

    m_errorMonitor->SetDesiredError("VUID-vkCreateComputePipelines-pCreateInfos-11429");

    const size_t maxSamplerAllocationCount =
        static_cast<size_t>(limits.maxSamplerAllocationCount -
                            SafeDivision(heap_props.minSamplerHeapReservedRangeWithEmbedded, heap_props.samplerDescriptorSize));
    if (max_samplers >= maxSamplerAllocationCount) {
        m_errorMonitor->SetDesiredError("VUID-vkCreateComputePipelines-pCreateInfos-11414");
    }

    VkSamplerCreateInfo embedded_sampler = vku::InitStructHelper();
    VkDescriptorSetAndBindingMappingEXT mapping[2];
    mapping[0] = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_ONLY_IMAGE_BIT_EXT);
    mapping[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping[0].sourceData.constantOffset.pEmbeddedSampler = &embedded_sampler;
    mapping[1] = MakeSetAndBindingMapping(0, 1, 1, VK_SPIRV_RESOURCE_TYPE_READ_ONLY_IMAGE_BIT_EXT);
    mapping[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping[1].sourceData.constantOffset.pEmbeddedSampler = &embedded_sampler;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2;
    mapping_info.pMappings = mapping;

    VkPipelineCreateFlags2CreateInfo pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.LateBindPipelineInfo();
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage.pNext = &mapping_info;
    pipe.CreateComputePipeline(false);

    m_errorMonitor->VerifyFound();

    for (auto& sampler : samplers) {
        vk::DestroySampler(device(), sampler, nullptr);
    }
}

TEST_F(NegativeDescriptorHeap, PipelineLayoutNotNull) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    char const* cs_source = R"glsl(
        #version 450
        #extension GL_EXT_descriptor_heap : require
        layout(local_size_x = 1) in;
        layout(descriptor_heap) buffer A { uint a; } heap[];
        void main() {
            heap[0].a = 0;
        }
    )glsl";
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    m_errorMonitor->SetDesiredError("VUID-VkComputePipelineCreateInfo-layout-07988");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, SamplerAllocationTotalCountShaderObject) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkPhysicalDeviceLimits& limits = m_device->Physical().limits_;
    if (limits.maxSamplerAllocationCount < heap_props.maxDescriptorHeapEmbeddedSamplers) {
        GTEST_SKIP() << "maxSamplerAllocationCount < maxDescriptorHeapEmbeddedSamplers";
    }
    if (heap_props.maxDescriptorHeapEmbeddedSamplers > 2048) {
        GTEST_SKIP() << "maxDescriptorHeapEmbeddedSamplers too large to run the test";
    }

    const size_t max_samplers = heap_props.maxDescriptorHeapEmbeddedSamplers - 1;
    std::vector<VkSampler> samplers(max_samplers);

    VkSamplerCreateInfo sampler_create_info = SafeSaneSamplerCreateInfo();

    VkResult err = VK_SUCCESS;
    for (size_t i = 0; i < max_samplers; i++) {
        err = vk::CreateSampler(device(), &sampler_create_info, nullptr, &samplers[i]);
        if (err != VK_SUCCESS) {
            break;
        }
    }

    VkSamplerCreateInfo embedded_sampler = vku::InitStructHelper();
    VkDescriptorSetAndBindingMappingEXT mapping[2];
    mapping[0] = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_ONLY_IMAGE_BIT_EXT);
    mapping[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping[0].sourceData.constantOffset.pEmbeddedSampler = &embedded_sampler;
    mapping[1] = MakeSetAndBindingMapping(0, 1, 1, VK_SPIRV_RESOURCE_TYPE_READ_ONLY_IMAGE_BIT_EXT);
    mapping[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping[1].sourceData.constantOffset.pEmbeddedSampler = &embedded_sampler;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2u;
    mapping_info.pMappings = mapping;

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexMinimalGlsl);
    const VkShaderCreateFlagsEXT flags = VK_SHADER_CREATE_DESCRIPTOR_HEAP_BIT_EXT;
    VkShaderCreateInfoEXT vert_ci = ShaderCreateInfoFlag(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, flags);
    vert_ci.pNext = &mapping_info;

    m_errorMonitor->SetDesiredError("VUID-vkCreateShadersEXT-pCreateInfos-11428");

    const size_t maxSamplerAllocationCount =
        static_cast<size_t>(limits.maxSamplerAllocationCount -
                            SafeDivision(heap_props.minSamplerHeapReservedRangeWithEmbedded, heap_props.samplerDescriptorSize));
    if (max_samplers >= maxSamplerAllocationCount) {
        m_errorMonitor->SetDesiredError("VUID-vkCreateShadersEXT-pCreateInfos-11413");
    }

    const vkt::Shader vertShader(*m_device, vert_ci);
    m_errorMonitor->VerifyFound();

    for (auto& sampler : samplers) {
        vk::DestroySampler(device(), sampler, nullptr);
    }
}

TEST_F(NegativeDescriptorHeap, NonConstantMemoryAccess) {
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT;
    mappings[0].sourceData.pushAddressOffset = 0u;
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT;
    mappings[1].sourceData.pushAddressOffset = 8u;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2u;
    mapping_info.pMappings = mappings;

    char const* cs_source1 = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) uniform Index {
            uint index;
        };
        layout(set = 0, binding = 1) buffer Data {
            uint data[];
        } ssbos[];
        void main() {
	        ssbos[index].data[4] = 4u;
        }
    )glsl";

    char const* cs_source2 = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) uniform Index {
            uint index;
        };
        layout(set = 0, binding = 1) buffer Data {
            uint data[];
        } ssbos[];
        void main() {
	        ssbos[index].data[index] = 4u;
        }
    )glsl";

    for (uint32_t i = 0; i < 2; ++i) {
        VkShaderObj cs_module = VkShaderObj(*m_device, i == 0 ? cs_source1 : cs_source2, VK_SHADER_STAGE_COMPUTE_BIT);

        VkPipelineCreateFlags2CreateInfoKHR flags2_ci = vku::InitStructHelper();
        flags2_ci.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

        CreateComputePipelineHelper pipe(*this, &flags2_ci);
        pipe.cp_ci_.layout = VK_NULL_HANDLE;
        pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;
        m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-DescriptorSet-11385");
        pipe.CreateComputePipeline(false);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, ResourceHeapNotBound) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    vkt::Buffer buffer_a(*m_device, 32, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = 0;
    mapping.sourceData.constantOffset.heapArrayStride = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        #extension GL_EXT_descriptor_heap : require
        layout(local_size_x = 1) in;
        layout(descriptor_heap) buffer A { uint a; } heap[];
        void main() {
            heap[73].a = 2;
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;
    pipe.CreateComputePipeline(false);

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11308");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptorHeap, SamplerHeapNotBound) {
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    InitRenderTarget();

    const uint32_t buffer_index = 16u;

    VkDeviceSize resource_heap_tracker = 0u;
    const VkDeviceSize image_offset = AlignedAppend(resource_heap_tracker, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    const VkDeviceSize image_size = resource_heap_tracker - image_offset;
    const VkDeviceSize buffer_offset = sizeof(float) * 4u * buffer_index;
    const VkDeviceSize buffer_size = resource_heap_tracker - buffer_offset + 256;  // 256 padding
    const VkDeviceSize resource_heap_app_size = resource_heap_tracker;

    CreateResourceHeap(resource_heap_app_size);

    vkt::Buffer buffer(*m_device, sizeof(float) * 4u, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

    VkHostAddressRangeEXT resource_host[2];
    resource_host[0].address = resource_heap_data_ + image_offset;
    resource_host[0].size = static_cast<size_t>(image_size);
    resource_host[1].address = resource_heap_data_ + buffer_offset;
    resource_host[1].size = static_cast<size_t>(buffer_size);

    VkImageViewCreateInfo view_info = image.BasicViewCreatInfo(VK_IMAGE_ASPECT_COLOR_BIT);

    VkImageDescriptorInfoEXT image_info = vku::InitStructHelper();
    image_info.pView = &view_info;
    image_info.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDeviceAddressRangeEXT buffer_address_range = {buffer.Address(), 16};

    VkResourceDescriptorInfoEXT descriptor_info[2];
    descriptor_info[0] = vku::InitStructHelper();
    descriptor_info[0].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    descriptor_info[0].data.pImage = &image_info;
    descriptor_info[1] = vku::InitStructHelper();
    descriptor_info[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info[1].data.pAddressRange = &buffer_address_range;
    vk::WriteResourceDescriptorsEXT(*m_device, 2u, descriptor_info, resource_host);

    VkDeviceSize sampler_desc_heap_size_tracker = 0u;
    const VkDeviceSize sampler_offset = AlignedAppend(sampler_desc_heap_size_tracker, VK_DESCRIPTOR_TYPE_SAMPLER);
    const VkDeviceSize sampler_size = sampler_desc_heap_size_tracker - sampler_offset;

    CreateSamplerHeap(sampler_desc_heap_size_tracker);

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();

    VkHostAddressRangeEXT sampler_host = {sampler_heap_data_ + sampler_offset, static_cast<size_t>(sampler_size)};
    vk::WriteSamplerDescriptorsEXT(*m_device, 1u, &sampler_info, &sampler_host);

    char const* cs_source = R"glsl(
        #version 450
        #extension GL_EXT_descriptor_heap : require
        layout(descriptor_heap) uniform texture2D heapTextures[];
        layout(descriptor_heap) uniform sampler heapSamplers[];
        layout(descriptor_heap) buffer ssbo {
            vec4 data;
        } heapBuffer[];
        void main() {
            heapBuffer[16].data = texture(sampler2D(heapTextures[0], heapSamplers[0]), vec2(0.5f));
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.CreateComputePipeline(false);

    m_command_buffer.Begin();

    VkImageMemoryBarrier image_barrier = vku::InitStructHelper();
    image_barrier.srcAccessMask = VK_ACCESS_NONE;
    image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    image_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier.image = image;
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, 0u, 1u};
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0u, 0u, nullptr, 0u,
                           nullptr, 1u, &image_barrier);

    VkClearColorValue color = {};
    color.float32[0] = 0.2f;
    color.float32[1] = 0.4f;
    color.float32[2] = 0.6f;
    color.float32[3] = 0.8f;
    vk::CmdClearColorImage(m_command_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &color, 1u,
                           &image_barrier.subresourceRange);

    image_barrier.srcAccessMask = image_barrier.dstAccessMask;
    image_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    image_barrier.oldLayout = image_barrier.newLayout;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0u, 0u, nullptr,
                           0u, nullptr, 1u, &image_barrier);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    BindResourceHeap();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11308");
    vk::CmdDispatch(m_command_buffer, 1u, 1u, 1u);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptorHeap, UsagesValidation) {
    TEST_DESCRIPTION("Tests that buffers and images were created with appropriate usage flags");
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    {
        std::vector<uint8_t> data(static_cast<size_t>(heap_props.imageDescriptorSize));
        vkt::Image image(*m_device, 32, 32, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

        struct {
            VkDescriptorType type;
            const char* vuid;
        } tests[] = {
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, "VUID-VkResourceDescriptorInfoEXT-type-11458"},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, "VUID-VkResourceDescriptorInfoEXT-type-11459"},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, "VUID-VkResourceDescriptorInfoEXT-type-11460"},
        };

        for (const auto test : tests) {
            VkResourceDescriptorInfoEXT resource_desc_info = vku::InitStructHelper();
            resource_desc_info.type = test.type;

            VkImageViewCreateInfo image_view_ci = vku::InitStructHelper();
            image_view_ci.image = image;
            image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
            image_view_ci.format = image.CreateInfo().format;
            image_view_ci.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B,
                                        VK_COMPONENT_SWIZZLE_A};
            image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

            VkImageDescriptorInfoEXT image_info = vku::InitStructHelper();
            image_info.pView = &image_view_ci;
            image_info.layout = VK_IMAGE_LAYOUT_GENERAL;
            resource_desc_info.data.pImage = &image_info;

            VkHostAddressRangeEXT descriptors = {data.data(), data.size()};
            m_errorMonitor->SetDesiredError(test.vuid);
            vk::WriteResourceDescriptorsEXT(device(), 1u, &resource_desc_info, &descriptors);
            m_errorMonitor->VerifyFound();
        }
    }
    {
        const VkDeviceSize buffer_size = 256;
        vkt::Buffer buffer(*m_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vkt::device_address);

        VkDeviceAddressRangeEXT address_range{buffer.Address(), buffer_size};
        VkResourceDescriptorInfoEXT desc_info = vku::InitStructHelper();
        desc_info.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        desc_info.data.pAddressRange = &address_range;

        VkHostAddressRangeEXT descriptors = {&desc_info, static_cast<size_t>(heap_props.bufferDescriptorSize)};

        m_errorMonitor->SetDesiredError("VUID-VkResourceDescriptorInfoEXT-type-11461");
        vk::WriteResourceDescriptorsEXT(device(), 1u, &desc_info, &descriptors);
        m_errorMonitor->VerifyFound();

        desc_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        m_errorMonitor->SetDesiredError("VUID-VkResourceDescriptorInfoEXT-type-11462");
        vk::WriteResourceDescriptorsEXT(device(), 1u, &desc_info, &descriptors);
        m_errorMonitor->VerifyFound();
    }
    {
        const VkDeviceSize buffer_size = 256;
        vkt::Buffer buffer(*m_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vkt::device_address);

        VkTexelBufferDescriptorInfoEXT texel_buffer_info = vku::InitStructHelper();
        texel_buffer_info.format = VK_FORMAT_R8G8B8A8_UNORM;
        texel_buffer_info.addressRange.address = buffer.Address();
        texel_buffer_info.addressRange.size = buffer_size;

        VkResourceDescriptorInfoEXT desc_info = vku::InitStructHelper();
        desc_info.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        desc_info.data.pTexelBuffer = &texel_buffer_info;

        VkHostAddressRangeEXT descriptors = {&desc_info, static_cast<size_t>(heap_props.imageDescriptorSize)};

        m_errorMonitor->SetDesiredError("VUID-VkResourceDescriptorInfoEXT-type-11463");
        vk::WriteResourceDescriptorsEXT(device(), 1u, &desc_info, &descriptors);
        m_errorMonitor->VerifyFound();

        desc_info.type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        m_errorMonitor->SetDesiredError("VUID-VkResourceDescriptorInfoEXT-type-11464");
        vk::WriteResourceDescriptorsEXT(device(), 1u, &desc_info, &descriptors);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, InputAttachmentIsNotNull) {
    TEST_DESCRIPTION("Validate VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT cannot be nullDescriptor");
    AddRequiredFeature(vkt::Feature::nullDescriptor);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDescriptorType type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    const VkDeviceSize size = vk::GetPhysicalDeviceDescriptorSizeEXT(Gpu(), type);
    if (size > 0) {
        std::vector<uint8_t> data(static_cast<size_t>(size));

        VkResourceDescriptorInfoEXT resource_desc_info = vku::InitStructHelper();
        resource_desc_info.type = type;
        VkHostAddressRangeEXT descriptors = {data.data(), static_cast<size_t>(size)};

        m_errorMonitor->SetDesiredError("VUID-VkResourceDescriptorInfoEXT-type-11469");
        vk::WriteResourceDescriptorsEXT(device(), 1u, &resource_desc_info, &descriptors);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorHeap, MappedPushIsBlockUniformArray) {
    TEST_DESCRIPTION("Validate that mapped push data is backed by block uniform");
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkDescriptorSetAndBindingMappingEXT mappings = MakeSetAndBindingMapping(0, 0);
    mappings.source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_DATA_EXT;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mappings;

    VkPipelineCreateFlags2CreateInfo pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    const char* cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) uniform Output {
            uvec4 x[2];
        } y[2];
        void main() {
            uvec4 a = y[0].x[0];
            uvec4 b = y[1].x[0];
        }
    )glsl";

    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;
    m_errorMonitor->SetDesiredError("VUID-VkPipelineShaderStageCreateInfo-pNext-11315");
    pipe.CreateComputePipeline(false);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, InvalidateComputeBoundDescriptorSetsBindDescriptorSets) {
    TEST_DESCRIPTION("Descriptor heap cmd functions do reset previously bound with CmdBindDescriptorSets descriptor sets");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO { uint x; };
        void main() { x = 0; }
    )glsl";

    for (int i = 0; i < 3; i++) {
        const auto ds_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        OneOffDescriptorSet ds(m_device, {{0, ds_type, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}}, 0, nullptr);
        const vkt::PipelineLayout pipeline_layout(*m_device, {&ds.layout_});

        vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

        VkDescriptorBufferInfo buffer_info{buffer, 0, VK_WHOLE_SIZE};
        VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
        descriptor_write.dstSet = ds.set_;
        descriptor_write.dstBinding = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = ds_type;
        descriptor_write.pBufferInfo = &buffer_info;
        vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);

        CreateComputePipelineHelper pipe(*this);
        pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
        pipe.cp_ci_.layout = pipeline_layout;
        pipe.CreateComputePipeline();

        m_command_buffer.Begin();
        vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
        vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &ds.set_, 0, nullptr);

        // The following descriptor heap-based command invalidates previously bound descriptor set and causes an error in
        // CmdDispatch
        if (i == 0) {
            CreateResourceHeap(heap_props.bufferDescriptorSize);
            BindResourceHeap();
        } else if (i == 1) {
            CreateSamplerHeap(heap_props.samplerDescriptorSize);
            BindSamplerHeap();
        } else {
            std::vector<uint8_t> payload(static_cast<size_t>(heap_props.maxPushDataSize / 2));
            VkPushDataInfoEXT push_data_info = vku::InitStructHelper();
            push_data_info.data.address = payload.data();
            push_data_info.data.size = payload.size();
            push_data_info.offset = 0;
            vk::CmdPushDataEXT(m_command_buffer, &push_data_info);
        }

        m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08600");
        vk::CmdDispatch(m_command_buffer, 1, 1, 1);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }
}

TEST_F(NegativeDescriptorHeap, InvalidateComputeBoundDescriptorSetsPushDescriptor) {
    TEST_DESCRIPTION("Descriptor heap cmd functions do reset previously bound with CmdPushDataEXT descriptor sets");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer SSBO { uint x; };
        void main() { x = 0; }
    )glsl";

    for (int i = 0; i < 3; i++) {
        const auto ds_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        OneOffDescriptorSet ds(m_device, {{0, ds_type, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}},
                               VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT);
        const vkt::PipelineLayout pipeline_layout(*m_device, {&ds.layout_});

        vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

        VkDescriptorBufferInfo buffer_info{buffer, 0, VK_WHOLE_SIZE};
        VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
        descriptor_write.dstSet = ds.set_;
        descriptor_write.dstBinding = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = ds_type;
        descriptor_write.pBufferInfo = &buffer_info;

        CreateComputePipelineHelper pipe(*this);
        pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
        pipe.cp_ci_.layout = pipeline_layout;
        pipe.CreateComputePipeline();

        m_command_buffer.Begin();
        vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
        vk::CmdPushDescriptorSetKHR(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0u, 1u, &descriptor_write);

        // The following descriptor heap-based command invalidates previously bound descriptor set and causes an error
        if (i == 0) {
            CreateResourceHeap(heap_props.bufferDescriptorSize);
            BindResourceHeap();
        } else if (i == 1) {
            CreateSamplerHeap(heap_props.samplerDescriptorSize);
            BindSamplerHeap();
        } else {
            std::vector<uint8_t> payload(static_cast<size_t>(heap_props.maxPushDataSize / 2));
            VkPushDataInfoEXT push_data_info = vku::InitStructHelper();
            push_data_info.data.address = payload.data();
            push_data_info.data.size = payload.size();
            push_data_info.offset = 0;
            vk::CmdPushDataEXT(m_command_buffer, &push_data_info);
        }

        m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-08600");
        vk::CmdDispatch(m_command_buffer, 1, 1, 1);
        m_errorMonitor->VerifyFound();
        m_command_buffer.End();
    }
}

TEST_F(NegativeDescriptorHeap, SecondaryCmdBufferHeapMissingInheritance) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    InitRenderTarget();

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    VkHostAddressRangeEXT descriptor_host = {resource_heap_data_, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT device_range = {buffer.Address(), 256};
    VkResourceDescriptorInfoEXT descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info.data.pAddressRange = &device_range;

    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);

    char const* vs_source = R"glsl(
        #version 450
        #extension GL_EXT_descriptor_heap : require

        layout(descriptor_heap) buffer A { uint a; } heap[];
        layout(push_constant) uniform PushConstant {
            uint b;
        };
        void main() {
            heap[0].a = b;
            gl_Position = vec4(1.0f);
        }
    )glsl";
    VkShaderObj vert_module = VkShaderObj(*m_device, vs_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_2);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    VkPipelineShaderStageCreateInfo stage;
    stage = vert_module.GetStageCreateInfo();

    CreatePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.gp_ci_.layout = VK_NULL_HANDLE;
    pipe.gp_ci_.stageCount = 1;
    pipe.gp_ci_.pStages = &stage;
    pipe.CreateGraphicsPipeline(false);

    uint32_t src_data = 4321u;

    VkPushDataInfoEXT push_data_info = vku::InitStructHelper();
    push_data_info.offset = 0u;
    push_data_info.data.size = sizeof(uint32_t);
    push_data_info.data.address = &src_data;

    VkPushConstantRange push_const_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t)};
    VkPipelineLayoutCreateInfo pipeline_layout_info = vku::InitStructHelper();
    pipeline_layout_info.pushConstantRangeCount = 1u;
    pipeline_layout_info.pPushConstantRanges = &push_const_range;
    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_info);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceDescriptorHeapInfoEXT inheritance_heap_info = vku::InitStructHelper();

    VkCommandBufferInheritanceInfo inheritance_info = vku::InitStructHelper(&inheritance_heap_info);
    inheritance_info.renderPass = RenderPass();
    inheritance_info.framebuffer = Framebuffer();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    begin_info.pInheritanceInfo = &inheritance_info;

    secondary.Begin(&begin_info);
    vk::CmdBindPipeline(secondary, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdPushDataEXT(secondary, &push_data_info);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-11308");
    vk::CmdDraw(secondary, 3u, 1u, 0u, 0u);
    m_errorMonitor->VerifyFound();
    secondary.End();
}

TEST_F(NegativeDescriptorHeap, SecondaryCmdBufferResourceHeapUnbound) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    InitRenderTarget();

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    VkHostAddressRangeEXT descriptor_host = {resource_heap_data_, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT device_range = {buffer.Address(), 256};
    VkResourceDescriptorInfoEXT descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info.data.pAddressRange = &device_range;

    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);

    char const* vs_source = R"glsl(
        #version 450
        #extension GL_EXT_descriptor_heap : require

        layout(descriptor_heap) buffer A { uint a; } heap[];
        layout(push_constant) uniform PushConstant {
            uint b;
        };
        void main() {
            heap[0].a = b;
            gl_Position = vec4(1.0f);
        }
    )glsl";
    VkShaderObj vert_module = VkShaderObj(*m_device, vs_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_2);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    VkPipelineShaderStageCreateInfo stage;
    stage = vert_module.GetStageCreateInfo();

    CreatePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.gp_ci_.layout = VK_NULL_HANDLE;
    pipe.gp_ci_.stageCount = 1;
    pipe.gp_ci_.pStages = &stage;
    pipe.CreateGraphicsPipeline(false);

    uint32_t src_data = 4321u;

    VkPushDataInfoEXT push_data_info = vku::InitStructHelper();
    push_data_info.offset = 0u;
    push_data_info.data.size = sizeof(uint32_t);
    push_data_info.data.address = &src_data;

    VkPushConstantRange push_const_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t)};
    VkPipelineLayoutCreateInfo pipeline_layout_info = vku::InitStructHelper();
    pipeline_layout_info.pushConstantRangeCount = 1u;
    pipeline_layout_info.pPushConstantRanges = &push_const_range;
    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_info);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange.address = resource_heap_.Address();
    bind_resource_info.heapRange.size = resource_heap_.CreateInfo().size;
    bind_resource_info.reservedRangeOffset = resource_heap_.CreateInfo().size - heap_props.minResourceHeapReservedRange;
    bind_resource_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;

    VkCommandBufferInheritanceDescriptorHeapInfoEXT inheritance_heap_info = vku::InitStructHelper();
    inheritance_heap_info.pResourceHeapBindInfo = &bind_resource_info;

    VkCommandBufferInheritanceInfo inheritance_info = vku::InitStructHelper(&inheritance_heap_info);
    inheritance_info.renderPass = RenderPass();
    inheritance_info.framebuffer = Framebuffer();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    begin_info.pInheritanceInfo = &inheritance_info;

    secondary.Begin(&begin_info);
    vk::CmdBindPipeline(secondary, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdPushDataEXT(secondary, &push_data_info);
    vk::CmdDraw(secondary, 3u, 1u, 0u, 0u);
    secondary.End();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-commandBuffer-11474");
    vk::CmdExecuteCommands(m_command_buffer, 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptorHeap, SecondaryCmdBufferSamplerHeapUnbound) {
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkDeviceSize resource_heap_tracker = 0u;
    const VkDeviceSize image_offset = AlignedAppend(resource_heap_tracker, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    const VkDeviceSize image_size = resource_heap_tracker - image_offset;
    const VkDeviceSize buffer_offset = AlignedAppend(resource_heap_tracker, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    const VkDeviceSize buffer_size = resource_heap_tracker - buffer_offset;
    const VkDeviceSize resource_heap_size = resource_heap_tracker;

    CreateResourceHeap(resource_heap_size);

    vkt::Buffer buffer(*m_device, sizeof(float) * 4u, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

    VkHostAddressRangeEXT resource_host[2];
    resource_host[0].address = resource_heap_data_ + image_offset;
    resource_host[0].size = static_cast<size_t>(image_size);
    resource_host[1].address = resource_heap_data_ + buffer_offset;
    resource_host[1].size = static_cast<size_t>(buffer_size);

    VkImageViewCreateInfo view_info = image.BasicViewCreatInfo(VK_IMAGE_ASPECT_COLOR_BIT);

    VkImageDescriptorInfoEXT image_info = vku::InitStructHelper();
    image_info.pView = &view_info;
    image_info.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDeviceAddressRangeEXT buffer_address_range = {buffer.Address(), 16};

    VkResourceDescriptorInfoEXT descriptor_info[2];
    descriptor_info[0] = vku::InitStructHelper();
    descriptor_info[0].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    descriptor_info[0].data.pImage = &image_info;
    descriptor_info[1] = vku::InitStructHelper();
    descriptor_info[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info[1].data.pAddressRange = &buffer_address_range;
    vk::WriteResourceDescriptorsEXT(*m_device, 2u, descriptor_info, resource_host);

    VkDeviceSize sampler_desc_heap_size_tracker =
        Align(heap_props.minSamplerHeapReservedRange, heap_props.samplerDescriptorAlignment);
    const VkDeviceSize sampler_offset = AlignedAppend(sampler_desc_heap_size_tracker, VK_DESCRIPTOR_TYPE_SAMPLER);
    const VkDeviceSize sampler_size = sampler_desc_heap_size_tracker - sampler_offset;
    const VkDeviceSize sampler_heap_size = sampler_desc_heap_size_tracker;

    vkt::Buffer sampler_heap(*m_device, sampler_heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);
    uint8_t* sampler_heap_data = static_cast<uint8_t*>(sampler_heap.Memory().Map());

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();

    VkHostAddressRangeEXT sampler_host = {sampler_heap_data + sampler_offset, static_cast<size_t>(sampler_size)};
    vk::WriteSamplerDescriptorsEXT(*m_device, 1u, &sampler_info, &sampler_host);

    VkBindHeapInfoEXT sampler_bind_info = vku::InitStructHelper();
    sampler_bind_info.heapRange = {sampler_heap.Address(), sampler_heap_size};
    sampler_bind_info.reservedRangeOffset = 0;
    sampler_bind_info.reservedRangeSize = heap_props.minSamplerHeapReservedRange;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform texture2D tex;
        layout(set = 0, binding = 1) uniform sampler sampl;
        layout(set = 1, binding = 0) buffer ssbo {
            vec4 data;
        };
        void main() {
            data = texture(sampler2D(tex, sampl), vec2(0.5f));
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    VkDescriptorSetAndBindingMappingEXT mappings[3];
    mappings[0] = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_SAMPLED_IMAGE_BIT_EXT);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset = {};
    mappings[0].sourceData.constantOffset.heapOffset = static_cast<uint32_t>(image_offset);
    mappings[1] = MakeSetAndBindingMapping(0, 1, 1, VK_SPIRV_RESOURCE_TYPE_SAMPLER_BIT_EXT);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset = {};
    mappings[1].sourceData.constantOffset.heapOffset = static_cast<uint32_t>(sampler_offset);
    mappings[2] = MakeSetAndBindingMapping(1, 0);
    mappings[2].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[2].sourceData.constantOffset = {};
    mappings[2].sourceData.constantOffset.heapOffset = static_cast<uint32_t>(buffer_offset);

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 3u;
    mapping_info.pMappings = mappings;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;
    pipe.CreateComputePipeline(false);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkBindHeapInfoEXT resource_bind_info = vku::InitStructHelper();
    resource_bind_info.heapRange = {resource_heap_.Address(), resource_heap_.CreateInfo().size};
    resource_bind_info.reservedRangeOffset = resource_heap_.CreateInfo().size - heap_props.minResourceHeapReservedRange;
    resource_bind_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;

    VkCommandBufferInheritanceDescriptorHeapInfoEXT inheritance_heap_info = vku::InitStructHelper();
    inheritance_heap_info.pResourceHeapBindInfo = &resource_bind_info;
    inheritance_heap_info.pSamplerHeapBindInfo = &sampler_bind_info;

    VkCommandBufferInheritanceInfo inheritance_info = vku::InitStructHelper(&inheritance_heap_info);

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    begin_info.pInheritanceInfo = &inheritance_info;

    secondary.Begin(&begin_info);
    vk::CmdBindPipeline(secondary, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(secondary, 1u, 1u, 1u);
    secondary.End();

    m_command_buffer.Begin();
    BindResourceHeap();
    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-commandBuffer-11473");
    vk::CmdExecuteCommands(m_command_buffer, 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptorHeap, OffsetIdNotAlignedBuffer) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    // What the shader looks like
    //
    // layout(descriptor_heap) struct {
    //    layout(offset = 0) buffer a { uint data; } x;
    //    layout(offset = 5) buffer a { uint data; } y;
    // };
    char const* cs_source = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %resource_heap %_
               OpExecutionMode %main LocalSize 1 1 1
               OpName %heap_struct "heap_struct"
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %A Block
               OpMemberDecorate %A 0 Offset 0
               OpDecorate %PushConstant Block
               OpMemberDecorate %PushConstant 0 Offset 0
               OpMemberDecorate %heap_struct 0 Offset 0
               OpMemberDecorate %heap_struct 1 Offset 5
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
       %uint = OpTypeInt 32 0
          %A = OpTypeStruct %uint
      %int_0 = OpConstant %int 0
%PushConstant = OpTypeStruct %uint
%_ptr_PushConstant_PushConstant = OpTypePointer PushConstant %PushConstant
          %_ = OpVariable %_ptr_PushConstant_PushConstant PushConstant
%_ptr_PushConstant_uint = OpTypePointer PushConstant %uint
%_ptr_StorageBuffer = OpTypeUntypedPointerKHR StorageBuffer
         %21 = OpTypeBufferEXT StorageBuffer
%heap_struct = OpTypeStruct %21 %21
       %main = OpFunction %void None %3
          %5 = OpLabel
         %17 = OpAccessChain %_ptr_PushConstant_uint %_ %int_0
         %18 = OpLoad %uint %17
         %20 = OpUntypedAccessChainKHR %_ptr_UniformConstant %heap_struct %resource_heap %int_1
         %24 = OpBufferPointerEXT %_ptr_StorageBuffer %20
         %25 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %A %24 %int_0
               OpStore %25 %18
               OpReturn
               OpFunctionEnd

    )";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-bufferDescriptorAlignment-11478");
    pipe.CreateComputePipeline(false);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, OffsetIdNotAlignedImageAndSampler) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    char const* cs_source = R"(
            OpCapability Shader
            OpCapability UntypedPointersKHR
            OpCapability DescriptorHeapEXT
            OpExtension "SPV_EXT_descriptor_heap"
            OpExtension "SPV_KHR_untyped_pointers"
            OpMemoryModel Logical GLSL450
            OpEntryPoint GLCompute %main "main"
            OpExecutionMode %main LocalSize 1 1 1
            OpName %struct_a "struct_a"
            OpName %struct_b "struct_b"
            OpMemberDecorate %struct_a 0 Offset 0
            OpMemberDecorate %struct_a 1 Offset 7
            OpMemberDecorate %struct_b 0 Offset 0
            OpMemberDecorate %struct_b 1 Offset 32
            OpMemberDecorate %struct_b 2 Offset 39
    %void = OpTypeVoid
       %3 = OpTypeFunction %void
     %int = OpTypeInt 32 1
   %float = OpTypeFloat 32
   %image = OpTypeImage %float 2D 0 0 0 1 Unknown
 %sampler = OpTypeSampler
%struct_a = OpTypeStruct %int %image
%struct_b = OpTypeStruct %struct_a %int %sampler
    %main = OpFunction %void None %3
       %5 = OpLabel
            OpReturn
            OpFunctionEnd

    )";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-samplerDescriptorAlignment-11476");
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-imageDescriptorAlignment-11477");
    pipe.CreateComputePipeline(false);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, OffsetIdNotAlignedConstant) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    char const* cs_source = R"(
             OpCapability Shader
             OpCapability UntypedPointersKHR
             OpCapability DescriptorHeapEXT
             OpExtension "SPV_EXT_descriptor_heap"
             OpExtension "SPV_KHR_untyped_pointers"
             OpMemoryModel Logical GLSL450
             OpEntryPoint GLCompute %main "main"
             OpExecutionMode %main LocalSize 1 1 1
             OpName %struct_a "struct_a"
             OpMemberDecorateIdEXT %struct_a 0 OffsetIdEXT %int_0
             OpMemberDecorateIdEXT %struct_a 1 OffsetIdEXT %int_7
     %void = OpTypeVoid
        %3 = OpTypeFunction %void
     %int = OpTypeInt 32 1
   %int_0 = OpConstant %int 0
   %int_7 = OpConstant %int 7
   %float = OpTypeFloat 32
   %image = OpTypeImage %float 2D 0 0 0 1 Unknown
%struct_a = OpTypeStruct %int %image
    %main = OpFunction %void None %3
       %5 = OpLabel
            OpReturn
            OpFunctionEnd
    )";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-imageDescriptorAlignment-11477");
    pipe.CreateComputePipeline(false);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeap, OffsetIdNotAlignedSpecConstantDefault) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    char const* cs_source = R"(
             OpCapability Shader
             OpCapability UntypedPointersKHR
             OpCapability DescriptorHeapEXT
             OpExtension "SPV_EXT_descriptor_heap"
             OpExtension "SPV_KHR_untyped_pointers"
             OpMemoryModel Logical GLSL450
             OpEntryPoint GLCompute %main "main"
             OpExecutionMode %main LocalSize 1 1 1
             OpName %struct_a "struct_a"
             OpMemberDecorateIdEXT %struct_a 0 OffsetIdEXT %uint_0
             OpMemberDecorateIdEXT %struct_a 1 OffsetIdEXT %result
     %void = OpTypeVoid
        %3 = OpTypeFunction %void
    %uint = OpTypeInt 32 0
  %uint_0 = OpConstant %uint 0
  %uint_1 = OpConstant %uint 1
  %uint_4 = OpConstant %uint 4
  %uint_8 = OpConstant %uint 8
     %mul = OpSpecConstantOp %uint IMul %uint_4 %uint_8
  %result = OpSpecConstantOp %uint IAdd %mul %uint_1
   %float = OpTypeFloat 32
   %image = OpTypeImage %float 2D 0 0 0 1 Unknown
%struct_a = OpTypeStruct %uint %image
    %main = OpFunction %void None %3
       %5 = OpLabel
            OpReturn
            OpFunctionEnd
    )";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-imageDescriptorAlignment-11477");
    pipe.CreateComputePipeline(false);
    m_errorMonitor->VerifyFound();
}