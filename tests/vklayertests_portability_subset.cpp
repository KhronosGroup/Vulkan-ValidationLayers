/*
 * Copyright (c) 2020 The Khronos Group Inc.
 * Copyright (c) 2020 Valve Corporation
 * Copyright (c) 2020 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Author: Nathaniel Cesario <nathaniel@lunarg.com>
 */

#include "cast_utils.h"
#include "layer_validation_tests.h"

class VkPortabilitySubsetTest : public VkLayerTest {
  public:
    void InitPortabilitySubsetFramework() {
        // VK_KHR_portability_subset extension dependencies
        instance_extensions_.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

        InitFramework(m_errorMonitor, nullptr);
    }
};

TEST_F(VkPortabilitySubsetTest, ValidatePortabilityCreateDevice) {
    TEST_DESCRIPTION("Portability: CreateDevice called and VK_KHR_portability_subset not enabled");

    ASSERT_NO_FATAL_FAILURE(InitPortabilitySubsetFramework());

    bool portability_supported = DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    if (!portability_supported) {
        printf("%s Test requires VK_KHR_portability_subset, skipping\n", kSkipPrefix);
        return;
    }

    vk_testing::PhysicalDevice phys_device(gpu());

    // request all queues
    const std::vector<VkQueueFamilyProperties> queue_props = phys_device.queue_properties();
    vk_testing::QueueCreateInfoArray queue_info(phys_device.queue_properties());

    // Only request creation with queuefamilies that have at least one queue
    std::vector<VkDeviceQueueCreateInfo> create_queue_infos;
    auto qci = queue_info.data();
    for (uint32_t j = 0; j < queue_info.size(); ++j) {
        if (qci[j].queueCount) {
            create_queue_infos.push_back(qci[j]);
        }
    }

    VkDeviceCreateInfo dev_info = {};
    dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dev_info.pNext = nullptr;
    dev_info.queueCreateInfoCount = create_queue_infos.size();
    dev_info.pQueueCreateInfos = create_queue_infos.data();
    dev_info.enabledLayerCount = 0;
    dev_info.ppEnabledLayerNames = NULL;
    dev_info.enabledExtensionCount = 0;
    dev_info.ppEnabledExtensionNames =
        nullptr;  // VK_KHR_portability_subset not included in enabled extensions should trigger 04451

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-pProperties-04451");
    VkDevice device;
    vk::CreateDevice(gpu(), &dev_info, nullptr, &device);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkPortabilitySubsetTest, PortabilityCreateEvent) {
    TEST_DESCRIPTION("Portability: CreateEvent when not supported");

    ASSERT_NO_FATAL_FAILURE(InitPortabilitySubsetFramework());

    bool portability_supported = DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    if (!portability_supported) {
        printf("%s Test requires VK_KHR_portability_subset, skipping\n", kSkipPrefix);
        return;
    }
    m_device_extension_names.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);

    auto portability_feature = lvl_init_struct<VkPhysicalDevicePortabilitySubsetFeaturesKHR>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&portability_feature);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    portability_feature.events = VK_FALSE;  // Make sure events are disabled

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateEvent-events-04468");
    VkEventCreateInfo eci = {VK_STRUCTURE_TYPE_EVENT_CREATE_INFO, nullptr, 0};
    VkEvent event;
    vk::CreateEvent(m_device->device(), &eci, nullptr, &event);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkPortabilitySubsetTest, CreateImage) {
    TEST_DESCRIPTION("Portability: CreateImage - VUIDs 04459, 04460");

    ASSERT_NO_FATAL_FAILURE(InitPortabilitySubsetFramework());

    bool portability_supported = DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    if (!portability_supported) {
        printf("%s Test requires VK_KHR_portability_subset, skipping\n", kSkipPrefix);
        return;
    }
    m_device_extension_names.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);

    auto portability_feature = lvl_init_struct<VkPhysicalDevicePortabilitySubsetFeaturesKHR>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&portability_feature);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    // Make sure image features are disabled via portability extension
    portability_feature.imageView2DOn3DImage = VK_FALSE;
    portability_feature.multisampleArrayImage = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkImageCreateInfo ci;
    ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ci.pNext = NULL;
    ci.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ci.extent.width = 512;
    ci.extent.height = 64;
    ci.extent.depth = 1;
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.queueFamilyIndexCount = 0;
    ci.pQueueFamilyIndices = nullptr;
    ci.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    CreateImageTest(*this, &ci, "VUID-VkImageCreateInfo-imageView2DOn3DImage-04459");

    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.flags = 0;
    ci.samples = VK_SAMPLE_COUNT_2_BIT;
    ci.arrayLayers = 2;
    CreateImageTest(*this, &ci, "VUID-VkImageCreateInfo-multisampleArrayImage-04460");
}

TEST_F(VkPortabilitySubsetTest, CreateImageView) {
    TEST_DESCRIPTION("Portability: CreateImageView - VUIDs 04465, 04466");

    ASSERT_NO_FATAL_FAILURE(InitPortabilitySubsetFramework());

    bool portability_supported = DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    if (!portability_supported) {
        printf("%s Test requires VK_KHR_portability_subset, skipping\n", kSkipPrefix);
        return;
    }
    m_device_extension_names.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);

    auto portability_feature = lvl_init_struct<VkPhysicalDevicePortabilitySubsetFeaturesKHR>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&portability_feature);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    // Make sure image features are disabled via portability extension
    portability_feature.imageViewFormatSwizzle = VK_FALSE;
    portability_feature.imageViewFormatReinterpretation = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkImageCreateInfo imageCI;
    imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCI.pNext = NULL;
    imageCI.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    imageCI.imageType = VK_IMAGE_TYPE_2D;
    imageCI.format = VK_FORMAT_R4G4B4A4_UNORM_PACK16;
    imageCI.extent.width = 512;
    imageCI.extent.height = 64;
    imageCI.extent.depth = 1;
    imageCI.mipLevels = 1;
    imageCI.arrayLayers = 1;
    imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCI.queueFamilyIndexCount = 0;
    imageCI.pQueueFamilyIndices = nullptr;
    imageCI.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    VkImageObj image(m_device);
    image.init(&imageCI);

    VkImageViewCreateInfo ci;
    ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ci.pNext = nullptr;
    ci.flags = 0;
    ci.image = image.image();
    ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ci.format = VK_FORMAT_R4G4B4A4_UNORM_PACK16;
    // Incorrect swizzling due to portability
    ci.components.r = VK_COMPONENT_SWIZZLE_G;
    ci.components.g = VK_COMPONENT_SWIZZLE_G;
    ci.components.b = VK_COMPONENT_SWIZZLE_R;
    ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ci.subresourceRange.baseMipLevel = 0;
    ci.subresourceRange.levelCount = 1;
    ci.subresourceRange.baseArrayLayer = 0;
    ci.subresourceRange.layerCount = 1;
    CreateImageViewTest(*this, &ci, "VUID-VkImageViewCreateInfo-imageViewFormatSwizzle-04465");

    ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    ci.format = VK_FORMAT_R5G6B5_UNORM_PACK16;  // Wrong number of components
    CreateImageViewTest(*this, &ci, "VUID-VkImageViewCreateInfo-imageViewFormatReinterpretation-04466");

    ci.format = VK_FORMAT_R12X4G12X4_UNORM_2PACK16;  // Wrong number of bits per component
    CreateImageViewTest(*this, &ci, "VUID-VkImageViewCreateInfo-imageViewFormatReinterpretation-04466");
}

TEST_F(VkPortabilitySubsetTest, CreateSampler) {
    TEST_DESCRIPTION("Portability: CreateSampler - VUID 04467");

    ASSERT_NO_FATAL_FAILURE(InitPortabilitySubsetFramework());

    bool portability_supported = DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    if (!portability_supported) {
        printf("%s Test requires VK_KHR_portability_subset, skipping\n", kSkipPrefix);
        return;
    }
    m_device_extension_names.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);

    auto portability_feature = lvl_init_struct<VkPhysicalDevicePortabilitySubsetFeaturesKHR>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&portability_feature);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    // Make sure image features are disabled via portability extension
    portability_feature.samplerMipLodBias = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.mipLodBias = 1.0f;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-samplerMipLodBias-04467");
}

TEST_F(VkPortabilitySubsetTest, CreateGraphicsPipelinesTriangleFans) {
    TEST_DESCRIPTION("Portability: CreateGraphicsPipelines - VUID 04452");

    ASSERT_NO_FATAL_FAILURE(InitPortabilitySubsetFramework());

    bool portability_supported = DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    if (!portability_supported) {
        printf("%s Test requires VK_KHR_portability_subset, skipping\n", kSkipPrefix);
        return;
    }
    m_device_extension_names.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);

    auto portability_feature = lvl_init_struct<VkPhysicalDevicePortabilitySubsetFeaturesKHR>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&portability_feature);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    // Make sure image features are disabled via portability extension
    portability_feature.triangleFans = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    ASSERT_TRUE(m_depth_stencil_fmt != 0);
    m_depthStencil->Init(m_device, static_cast<int32_t>(m_width), static_cast<int32_t>(m_height), m_depth_stencil_fmt);
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(m_depthStencil->BindInfo()));

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.ia_ci_ = VkPipelineInputAssemblyStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
                                                         VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, VK_FALSE};
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineInputAssemblyStateCreateInfo-triangleFans-04452");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkPortabilitySubsetTest, CreateGraphicsPipelinesVertexInputStride) {
    TEST_DESCRIPTION("Portability: CreateGraphicsPipelines - VUID 04456");

    ASSERT_NO_FATAL_FAILURE(InitPortabilitySubsetFramework());

    bool portability_supported = DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    if (!portability_supported) {
        printf("%s Test requires VK_KHR_portability_subset, skipping\n", kSkipPrefix);
        return;
    }
    m_device_extension_names.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);

    auto portability_feature = lvl_init_struct<VkPhysicalDevicePortabilitySubsetFeaturesKHR>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&portability_feature);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);

    // Get the current vertex stride to ensure we pass an incorrect value when creating the graphics pipeline
    auto portability_properties = lvl_init_struct<VkPhysicalDevicePortabilitySubsetPropertiesKHR>();
    auto prop2 = lvl_init_struct<VkPhysicalDeviceProperties2KHR>(&portability_properties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &prop2);
    ASSERT_TRUE(portability_properties.minVertexInputBindingStrideAlignment > 0);
    auto vertex_stride = portability_properties.minVertexInputBindingStrideAlignment - 1;

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    ASSERT_TRUE(m_depth_stencil_fmt != 0);
    m_depthStencil->Init(m_device, static_cast<int32_t>(m_width), static_cast<int32_t>(m_height), m_depth_stencil_fmt);
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(m_depthStencil->BindInfo()));

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    VkVertexInputBindingDescription vertex_desc{
        0,                            // binding
        vertex_stride,                // stride
        VK_VERTEX_INPUT_RATE_VERTEX,  // inputRate
    };
    pipe.vi_ci_ = VkPipelineVertexInputStateCreateInfo{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, 0, 1, &vertex_desc, 0, nullptr};
    pipe.ia_ci_ = VkPipelineInputAssemblyStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
                                                         VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE};
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVertexInputBindingDescription-stride-04456");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkPortabilitySubsetTest, CreateGraphicsPipelinesVertexAttributes) {
    TEST_DESCRIPTION("Portability: CreateGraphicsPipelines - VUID 04457");

    ASSERT_NO_FATAL_FAILURE(InitPortabilitySubsetFramework());

    bool portability_supported = DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    if (!portability_supported) {
        printf("%s Test requires VK_KHR_portability_subset, skipping\n", kSkipPrefix);
        return;
    }
    m_device_extension_names.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);

    auto portability_feature = lvl_init_struct<VkPhysicalDevicePortabilitySubsetFeaturesKHR>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&portability_feature);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    // Make sure image features are disabled via portability extension
    portability_feature.vertexAttributeAccessBeyondStride = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    ASSERT_TRUE(m_depth_stencil_fmt != 0);
    m_depthStencil->Init(m_device, static_cast<int32_t>(m_width), static_cast<int32_t>(m_height), m_depth_stencil_fmt);
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(m_depthStencil->BindInfo()));

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    VkVertexInputBindingDescription vertex_desc{
        0,                            // binding
        4,                            // stride
        VK_VERTEX_INPUT_RATE_VERTEX,  // inputRate
    };
    VkVertexInputAttributeDescription vertex_attrib{
        0,                     // location
        0,                     // binding
        VK_FORMAT_R32_SFLOAT,  // format; size == 4
        4,                     // offset; size(format) + offset > description.stride, so this should trigger 04457
    };
    pipe.vi_ci_ = VkPipelineVertexInputStateCreateInfo{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, 0, 1, &vertex_desc, 1, &vertex_attrib};
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkVertexInputAttributeDescription-vertexAttributeAccessBeyondStride-04457");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkPortabilitySubsetTest, CreateGraphicsPipelinesRasterizationState) {
    TEST_DESCRIPTION("Portability: CreateGraphicsPipelines - VUID 04458");

    ASSERT_NO_FATAL_FAILURE(InitPortabilitySubsetFramework());

    bool portability_supported = DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    if (!portability_supported) {
        printf("%s Test requires VK_KHR_portability_subset, skipping\n", kSkipPrefix);
        return;
    }
    m_device_extension_names.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);

    auto portability_feature = lvl_init_struct<VkPhysicalDevicePortabilitySubsetFeaturesKHR>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&portability_feature);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    // Make sure point polygons are disabled
    portability_feature.pointPolygons = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkAttachmentDescription attachment{};
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.format = VK_FORMAT_B8G8R8A8_SRGB;

    VkAttachmentReference color_ref{};
    color_ref.attachment = 0;
    color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_ref;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkRenderPassCreateInfo rp_info{};
    rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_info.attachmentCount = 1;
    rp_info.pAttachments = &attachment;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &subpass;

    vk::CreateRenderPass(device(), &rp_info, nullptr, &m_renderPass);
    m_renderPass_info = rp_info;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.rs_state_ci_.polygonMode = VK_POLYGON_MODE_POINT;
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineRasterizationStateCreateInfo-pointPolygons-04458");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkPortabilitySubsetTest, CreateGraphicsPipelinesDepthStencilState) {
    TEST_DESCRIPTION("Portability: CreateGraphicsPipelines - VUID 04453");

    ASSERT_NO_FATAL_FAILURE(InitPortabilitySubsetFramework());

    bool portability_supported = DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    if (!portability_supported) {
        printf("%s Test requires VK_KHR_portability_subset, skipping\n", kSkipPrefix);
        return;
    }
    m_device_extension_names.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);

    auto portability_feature = lvl_init_struct<VkPhysicalDevicePortabilitySubsetFeaturesKHR>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&portability_feature);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    portability_feature.separateStencilMaskRef = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    ASSERT_TRUE(m_depth_stencil_fmt != 0);
    m_depthStencil->Init(m_device, static_cast<int32_t>(m_width), static_cast<int32_t>(m_height), m_depth_stencil_fmt);
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(m_depthStencil->BindInfo()));

    auto depth_stencil_ci = lvl_init_struct<VkPipelineDepthStencilStateCreateInfo>();
    depth_stencil_ci.stencilTestEnable = VK_TRUE;
    depth_stencil_ci.front.reference = 1;
    depth_stencil_ci.back.reference = depth_stencil_ci.front.reference + 1;
    //  front.reference != back.reference should trigger 04453

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.pDepthStencilState = &depth_stencil_ci;
    pipe.rs_state_ci_.cullMode = VK_CULL_MODE_NONE;
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineDepthStencilStateCreateInfo-separateStencilMaskRef-04453");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkPortabilitySubsetTest, CreateGraphicsPipelinesColorBlendAttachmentState) {
    TEST_DESCRIPTION("Portability: CreateGraphicsPipelines - VUIDs 04454, 04455");

    ASSERT_NO_FATAL_FAILURE(InitPortabilitySubsetFramework());

    bool portability_supported = DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    if (!portability_supported) {
        printf("%s Test requires VK_KHR_portability_subset, skipping\n", kSkipPrefix);
        return;
    }
    m_device_extension_names.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);

    auto portability_feature = lvl_init_struct<VkPhysicalDevicePortabilitySubsetFeaturesKHR>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&portability_feature);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    portability_feature.constantAlphaColorBlendFactors = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    ASSERT_TRUE(m_depth_stencil_fmt != 0);
    m_depthStencil->Init(m_device, static_cast<int32_t>(m_width), static_cast<int32_t>(m_height), m_depth_stencil_fmt);
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(m_depthStencil->BindInfo()));

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cb_attachments_.srcColorBlendFactor = VK_BLEND_FACTOR_CONSTANT_ALPHA;
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkPipelineColorBlendAttachmentState-constantAlphaColorBlendFactors-04454");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    pipe.cb_attachments_.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkPipelineColorBlendAttachmentState-constantAlphaColorBlendFactors-04454");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    pipe.cb_attachments_.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    pipe.cb_attachments_.dstColorBlendFactor = VK_BLEND_FACTOR_CONSTANT_ALPHA;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkPipelineColorBlendAttachmentState-constantAlphaColorBlendFactors-04455");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    pipe.cb_attachments_.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkPipelineColorBlendAttachmentState-constantAlphaColorBlendFactors-04455");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}
