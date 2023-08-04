/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 * Modifications Copyright (C) 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "utils/cast_utils.h"
#include "generated/enum_flag_bits.h"
#include "../framework/layer_validation_tests.h"
#include "utils/vk_layer_utils.h"
#include "generated/vk_validation_error_messages.h"

class MessageIdFilter {
  public:
    MessageIdFilter(const char *filter_string) {
        local_string = filter_string;
        filter_string_value.arrayString.pCharArray = local_string.data();
        filter_string_value.arrayString.count = local_string.size();

        strncpy(filter_setting_val.name, "message_id_filter", sizeof(filter_setting_val.name));
        filter_setting_val.type = VK_LAYER_SETTING_VALUE_TYPE_STRING_ARRAY_EXT;
        filter_setting_val.data = filter_string_value;
        filter_setting = {VK_STRUCTURE_TYPE_INSTANCE_LAYER_SETTINGS_EXT, nullptr, 1, &filter_setting_val};
    }
    VkLayerSettingsEXT *pnext{&filter_setting};

  private:
    VkLayerSettingValueDataEXT filter_string_value{};
    VkLayerSettingValueEXT filter_setting_val;
    VkLayerSettingsEXT filter_setting;
    std::string local_string;
};

class CustomStypeList {
  public:
    CustomStypeList(const char *stype_id_string) {
        local_string = stype_id_string;
        custom_stype_value.arrayString.pCharArray = local_string.data();
        custom_stype_value.arrayString.count = local_string.size();

        strncpy(custom_stype_setting_val.name, "custom_stype_list", sizeof(custom_stype_setting_val.name));
        custom_stype_setting_val.type = VK_LAYER_SETTING_VALUE_TYPE_STRING_ARRAY_EXT;
        custom_stype_setting_val.data = custom_stype_value;
        custom_stype_setting = {VK_STRUCTURE_TYPE_INSTANCE_LAYER_SETTINGS_EXT, nullptr, 1, &custom_stype_setting_val};
    }

    CustomStypeList(const std::vector<uint32_t> &stype_id_array) {
        local_vector = stype_id_array;
        custom_stype_value.arrayInt32.pInt32Array = local_vector.data();
        custom_stype_value.arrayInt32.count = local_vector.size();

        strncpy(custom_stype_setting_val.name, "custom_stype_list", sizeof(custom_stype_setting_val.name));
        custom_stype_setting_val.type = VK_LAYER_SETTING_VALUE_TYPE_UINT32_ARRAY_EXT;
        custom_stype_setting_val.data = custom_stype_value;
        custom_stype_setting = {VK_STRUCTURE_TYPE_INSTANCE_LAYER_SETTINGS_EXT, nullptr, 1, &custom_stype_setting_val};
    }
    VkLayerSettingsEXT *pnext{&custom_stype_setting};

  private:
    VkLayerSettingValueDataEXT custom_stype_value{};
    VkLayerSettingValueEXT custom_stype_setting_val;
    VkLayerSettingsEXT custom_stype_setting;
    std::string local_string;
    std::vector<uint32_t> local_vector;
};

class DuplicateMsgLimit {
  public:
    DuplicateMsgLimit(const uint32_t limit) {
        limit_value.value32 = limit;

        strncpy(limit_setting_val.name, "duplicate_message_limit", sizeof(limit_setting_val.name));
        limit_setting_val.type = VK_LAYER_SETTING_VALUE_TYPE_UINT32_EXT;
        limit_setting_val.data = limit_value;
        limit_setting = {VK_STRUCTURE_TYPE_INSTANCE_LAYER_SETTINGS_EXT, nullptr, 1, &limit_setting_val};
    }
    VkLayerSettingsEXT *pnext{&limit_setting};

  private:
    VkLayerSettingValueDataEXT limit_value{};
    VkLayerSettingValueEXT limit_setting_val;
    VkLayerSettingsEXT limit_setting;
};

TEST_F(VkLayerTest, VersionCheckPromotedAPIs) {
    TEST_DESCRIPTION("Validate that promoted APIs are not valid in old versions.");
    SetTargetApiVersion(VK_API_VERSION_1_0);

    ASSERT_NO_FATAL_FAILURE(Init());

    // TODO - Currently not working on MockICD with Profiles using 1.0
    // Seems API version is not being passed through correctly
    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    PFN_vkGetPhysicalDeviceProperties2 vkGetPhysicalDeviceProperties2 =
        (PFN_vkGetPhysicalDeviceProperties2)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2");

    VkPhysicalDeviceProperties2 phys_dev_props_2 = LvlInitStruct<VkPhysicalDeviceProperties2>();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-API-Version-Violation");
    vkGetPhysicalDeviceProperties2(gpu(), &phys_dev_props_2);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, UnsupportedPnextApiVersion) {
    TEST_DESCRIPTION("Validate that newer pnext structs are not valid for old Vulkan versions.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    ASSERT_NO_FATAL_FAILURE(Init());
    auto phys_dev_props_2 = LvlInitStruct<VkPhysicalDeviceProperties2>();
    auto bad_version_1_1_struct = LvlInitStruct<VkPhysicalDeviceVulkan12Properties>();
    phys_dev_props_2.pNext = &bad_version_1_1_struct;

    // VkPhysDevVulkan12Props was introduced in 1.2, so try adding it to a 1.1 pNext chain
    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPhysicalDeviceProperties2-pNext-pNext");
        vk::GetPhysicalDeviceProperties2(gpu(), &phys_dev_props_2);
        m_errorMonitor->VerifyFound();
    }

    // 1.1 context, VK_KHR_depth_stencil_resolve is NOT enabled, but using its struct is valid
    if (DeviceExtensionSupported(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME)) {
        auto unenabled_device_ext_struct = LvlInitStruct<VkPhysicalDeviceDepthStencilResolveProperties>();
        phys_dev_props_2.pNext = &unenabled_device_ext_struct;
        if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
            vk::GetPhysicalDeviceProperties2(gpu(), &phys_dev_props_2);
        } else {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-API-Version-Violation");
            vk::GetPhysicalDeviceProperties2(gpu(), &phys_dev_props_2);
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(VkLayerTest, PrivateDataExtTest) {
    TEST_DESCRIPTION("Test private data extension use.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_PRIVATE_DATA_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto private_data_features = LvlInitStruct<VkPhysicalDevicePrivateDataFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(private_data_features);
    if (private_data_features.privateData == VK_FALSE) {
        GTEST_SKIP() << "privateData feature is not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkPrivateDataSlotEXT data_slot;
    VkPrivateDataSlotCreateInfoEXT data_create_info = LvlInitStruct<VkPrivateDataSlotCreateInfoEXT>();
    data_create_info.flags = 0;
    VkResult err = vk::CreatePrivateDataSlotEXT(m_device->handle(), &data_create_info, NULL, &data_slot);
    if (err != VK_SUCCESS) {
        printf("Failed to create private data slot, VkResult %d.\n", err);
    }

    VkSamplerCreateInfo sampler_info = LvlInitStruct<VkSamplerCreateInfo>();
    sampler_info.flags = 0;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.anisotropyEnable = VK_FALSE;
    sampler_info.maxAnisotropy = 16;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    vk_testing::Sampler sampler(*m_device, sampler_info);

    static const uint64_t data_value = 0x70AD;
    err = vk::SetPrivateDataEXT(m_device->handle(), VK_OBJECT_TYPE_SAMPLER, (uint64_t)sampler.handle(), data_slot, data_value);
    if (err != VK_SUCCESS) {
        printf("Failed to set private data. VkResult = %d\n", err);
    }
    uint64_t data;
    vk::GetPrivateDataEXT(m_device->handle(), VK_OBJECT_TYPE_SAMPLER, (uint64_t)sampler.handle(), data_slot, &data);
    if (data != data_value) {
        m_errorMonitor->SetError("Got unexpected private data, %s.\n");
    }
    vk::DestroyPrivateDataSlotEXT(m_device->handle(), data_slot, NULL);
}

TEST_F(VkLayerTest, PrivateDataFeature) {
    TEST_DESCRIPTION("Test privateData feature not being enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_PRIVATE_DATA_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // feature not enabled
    ASSERT_NO_FATAL_FAILURE(InitState());

    bool vulkan_13 = (DeviceValidationVersion() >= VK_API_VERSION_1_3);

    VkPrivateDataSlotEXT data_slot;
    VkPrivateDataSlotCreateInfoEXT data_create_info = LvlInitStruct<VkPrivateDataSlotCreateInfoEXT>();
    data_create_info.flags = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreatePrivateDataSlot-privateData-04564");
    vk::CreatePrivateDataSlotEXT(m_device->handle(), &data_create_info, NULL, &data_slot);
    m_errorMonitor->VerifyFound();
    if (vulkan_13) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreatePrivateDataSlot-privateData-04564");
        vk::CreatePrivateDataSlot(m_device->handle(), &data_create_info, NULL, &data_slot);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, CustomStypeStructString) {
    TEST_DESCRIPTION("Positive Test for ability to specify custom pNext structs using a list (string)");

    // Create a custom structure
    typedef struct CustomStruct {
        VkStructureType sType;
        const void *pNext;
        uint32_t custom_data;
    } CustomStruct;

    uint32_t custom_stype = 3000300000;
    CustomStruct custom_struct;
    custom_struct.pNext = nullptr;
    custom_struct.sType = static_cast<VkStructureType>(custom_stype);
    custom_struct.custom_data = 44;

    // Communicate list of structinfo pairs to layers
    auto stype_list = CustomStypeList("3000300000,24");
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor, stype_list.pnext));
    ASSERT_NO_FATAL_FAILURE(InitState());

    uint32_t queue_family_index = 0;
    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.size = 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    buffer_create_info.queueFamilyIndexCount = 1;
    buffer_create_info.pQueueFamilyIndices = &queue_family_index;
    VkBufferObj buffer;
    buffer.init(*m_device, buffer_create_info);
    VkBufferViewCreateInfo bvci = LvlInitStruct<VkBufferViewCreateInfo>(&custom_struct);  // Add custom struct through pNext
    bvci.buffer = buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vk_testing::BufferView buffer_view(*m_device, bvci);
}

TEST_F(VkLayerTest, CustomStypeStructArray) {
    TEST_DESCRIPTION("Positive Test for ability to specify custom pNext structs using a vector of integers");

    // Create a custom structure
    typedef struct CustomStruct {
        VkStructureType sType;
        const void *pNext;
        uint32_t custom_data;
    } CustomStruct;

    const uint32_t custom_stype_a = 3000300000;
    CustomStruct custom_struct_a;
    custom_struct_a.pNext = nullptr;
    custom_struct_a.sType = static_cast<VkStructureType>(custom_stype_a);
    custom_struct_a.custom_data = 44;

    const uint32_t custom_stype_b = 3000300001;
    CustomStruct custom_struct_b;
    custom_struct_b.pNext = &custom_struct_a;
    custom_struct_b.sType = static_cast<VkStructureType>(custom_stype_b);
    custom_struct_b.custom_data = 88;

    // Communicate list of structinfo pairs to layers, including a duplicate which should get filtered out
    std::vector<uint32_t> custom_struct_info = {custom_stype_a,       sizeof(CustomStruct), custom_stype_b,
                                                sizeof(CustomStruct), custom_stype_a,       sizeof(CustomStruct)};
    auto stype_list = CustomStypeList(custom_struct_info);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor, stype_list.pnext));
    ASSERT_NO_FATAL_FAILURE(InitState());

    uint32_t queue_family_index = 0;
    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.size = 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    buffer_create_info.queueFamilyIndexCount = 1;
    buffer_create_info.pQueueFamilyIndices = &queue_family_index;
    VkBufferObj buffer;
    buffer.init(*m_device, buffer_create_info);
    VkBufferViewCreateInfo bvci = LvlInitStruct<VkBufferViewCreateInfo>(&custom_struct_b);  // Add custom struct through pNext
    bvci.buffer = buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vk_testing::BufferView buffer_view(*m_device, bvci);
}

TEST_F(VkLayerTest, DuplicateMessageLimit) {
    TEST_DESCRIPTION("Use the duplicate_message_id setting and verify correct operation");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    auto msg_limit = DuplicateMsgLimit(3);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor, msg_limit.pnext));
    ASSERT_NO_FATAL_FAILURE(InitState());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // Create an invalid pNext structure to trigger the stateless validation warning
    VkBaseOutStructure bogus_struct{};
    bogus_struct.sType = static_cast<VkStructureType>(0x33333333);
    auto properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&bogus_struct);

    // Should get the first three errors just fine
    m_errorMonitor->SetDesiredFailureMsg((kErrorBit | kWarningBit), "VUID-VkPhysicalDeviceProperties2-pNext-pNext");
    vk::GetPhysicalDeviceProperties2KHR(gpu(), &properties2);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg((kErrorBit | kWarningBit), "VUID-VkPhysicalDeviceProperties2-pNext-pNext");
    vk::GetPhysicalDeviceProperties2KHR(gpu(), &properties2);
    m_errorMonitor->VerifyFound();
    // VUID-VkPhysicalDeviceProperties2-pNext-pNext produces a massive ~3600 character log message, which hits a
    // complex string buffer reallocation path inside of the logging code. Make sure it successfully prints out
    // the very end of the message.
    m_errorMonitor->SetDesiredFailureMsg((kErrorBit | kWarningBit), "is undefined and may not work correctly with validation enabled");
    vk::GetPhysicalDeviceProperties2KHR(gpu(), &properties2);
    m_errorMonitor->VerifyFound();

    // Limit should prevent the message from coming through a fourth time
    vk::GetPhysicalDeviceProperties2KHR(gpu(), &properties2);
}

TEST_F(VkLayerTest, VuidCheckForHashCollisions) {
    TEST_DESCRIPTION("Ensure there are no VUID hash collisions");

    constexpr uint64_t num_vuids = sizeof(vuid_spec_text) / sizeof(vuid_spec_text[0]);
    std::vector<uint32_t> hashes;
    hashes.reserve(num_vuids);
    for (const auto &vuid_spec_text_pair : vuid_spec_text) {
        const uint32_t hash = vvl_vuid_hash(vuid_spec_text_pair.vuid);
        hashes.push_back(hash);
    }
    std::sort(hashes.begin(), hashes.end());
    const auto it = std::adjacent_find(hashes.begin(), hashes.end());
    ASSERT_TRUE(it == hashes.end());
}

TEST_F(VkLayerTest, VuidHashStability) {
    TEST_DESCRIPTION("Ensure stability of VUID hashes clients rely on for filtering");
    ASSERT_TRUE(vvl_vuid_hash("VUID-VkRenderPassCreateInfo-pNext-01963") == 0xa19880e3);
    ASSERT_TRUE(vvl_vuid_hash("VUID-BaryCoordKHR-BaryCoordKHR-04154") == 0xcc72e520);
    ASSERT_TRUE(vvl_vuid_hash("VUID-FragDepth-FragDepth-04213") == 0x840af838);
    ASSERT_TRUE(vvl_vuid_hash("VUID-RayTmaxKHR-RayTmaxKHR-04349") == 0x8e67514c);
    ASSERT_TRUE(vvl_vuid_hash("VUID-RuntimeSpirv-SubgroupUniformControlFlowKHR-06379") == 0x2f574188);
    ASSERT_TRUE(vvl_vuid_hash("VUID-StandaloneSpirv-MeshEXT-07111") == 0xee813cd2);
}

TEST_F(VkLayerTest, VuidIdFilterString) {
    TEST_DESCRIPTION("Validate that message id string filtering is working");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    // This test would normally produce an unexpected error or two.  Use the message filter instead of
    // the error_monitor's SetUnexpectedError to test the filtering.
    auto filter_setting = MessageIdFilter("VUID-VkRenderPassCreateInfo-pNext-01963");
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor, filter_setting.pnext));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    VkAttachmentDescription attach = {0,
                                      VK_FORMAT_R8G8B8A8_UNORM,
                                      VK_SAMPLE_COUNT_1_BIT,
                                      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &ref, 0, nullptr, nullptr, nullptr, 0, nullptr};
    VkInputAttachmentAspectReference iaar = {0, 0, VK_IMAGE_ASPECT_METADATA_BIT};
    VkRenderPassInputAttachmentAspectCreateInfo rpiaaci = {VK_STRUCTURE_TYPE_RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO,
                                                           nullptr, 1, &iaar};
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, &rpiaaci, 0, 1, &attach, 1, &subpass, 0, nullptr};
    m_errorMonitor->SetUnexpectedError("VUID-VkRenderPassCreateInfo2-attachment-02525");
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false, "VUID-VkInputAttachmentAspectReference-aspectMask-01964", nullptr);
}

TEST_F(VkLayerTest, VuidFilterHexInt) {
    TEST_DESCRIPTION("Validate that message id hex int filtering is working");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    // This test would normally produce an unexpected error or two.  Use the message filter instead of
    // the error_monitor's SetUnexpectedError to test the filtering.
    auto filter_setting = MessageIdFilter("0xa19880e3");
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor, filter_setting.pnext));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    VkAttachmentDescription attach = {0,
                                      VK_FORMAT_R8G8B8A8_UNORM,
                                      VK_SAMPLE_COUNT_1_BIT,
                                      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &ref, 0, nullptr, nullptr, nullptr, 0, nullptr};
    VkInputAttachmentAspectReference iaar = {0, 0, VK_IMAGE_ASPECT_METADATA_BIT};
    VkRenderPassInputAttachmentAspectCreateInfo rpiaaci = {VK_STRUCTURE_TYPE_RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO,
                                                           nullptr, 1, &iaar};
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, &rpiaaci, 0, 1, &attach, 1, &subpass, 0, nullptr};
    m_errorMonitor->SetUnexpectedError("VUID-VkRenderPassCreateInfo2-attachment-02525");
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false, "VUID-VkInputAttachmentAspectReference-aspectMask-01964", nullptr);
}

TEST_F(VkLayerTest, VuidFilterInt) {
    TEST_DESCRIPTION("Validate that message id decimal int filtering is working");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    // This test would normally produce an unexpected error or two.  Use the message filter instead of
    // the error_monitor's SetUnexpectedError to test the filtering.
    auto filter_setting = MessageIdFilter("2711126243");
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor, filter_setting.pnext));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    VkAttachmentDescription attach = {0,
                                      VK_FORMAT_R8G8B8A8_UNORM,
                                      VK_SAMPLE_COUNT_1_BIT,
                                      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &ref, 0, nullptr, nullptr, nullptr, 0, nullptr};
    VkInputAttachmentAspectReference iaar = {0, 0, VK_IMAGE_ASPECT_METADATA_BIT};
    VkRenderPassInputAttachmentAspectCreateInfo rpiaaci = {VK_STRUCTURE_TYPE_RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO,
                                                           nullptr, 1, &iaar};
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, &rpiaaci, 0, 1, &attach, 1, &subpass, 0, nullptr};
    m_errorMonitor->SetUnexpectedError("VUID-VkRenderPassCreateInfo2-attachment-02525");
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false, "VUID-VkInputAttachmentAspectReference-aspectMask-01964", nullptr);
}

struct LayerStatusCheckData {
    std::function<void(const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, LayerStatusCheckData *)> callback;
    ErrorMonitor *error_monitor;
};

TEST_F(VkLayerTest, LayerInfoMessages) {
    TEST_DESCRIPTION("Ensure layer prints startup status messages.");

    auto ici = GetInstanceCreateInfo();
    LayerStatusCheckData callback_data;
    auto local_callback = [](const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, LayerStatusCheckData *data) {
        std::string message(pCallbackData->pMessage);
        if ((data->error_monitor->GetMessageFlags() & kInformationBit) &&
            (message.find("UNASSIGNED-khronos-validation-createinstance-status-message") == std::string::npos)) {
            data->error_monitor->SetError("UNASSIGNED-Khronos-validation-createinstance-status-message-not-found");
        } else if ((data->error_monitor->GetMessageFlags() & kPerformanceWarningBit) &&
                   (message.find("UNASSIGNED-khronos-Validation-debug-build-warning-message") == std::string::npos)) {
            data->error_monitor->SetError("UNASSIGNED-khronos-validation-createinstance-debug-warning-message-not-found");
        }
    };
    callback_data.error_monitor = m_errorMonitor;
    callback_data.callback = local_callback;

    VkInstance local_instance;

    auto callback_create_info = LvlInitStruct<VkDebugUtilsMessengerCreateInfoEXT>();
    callback_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    callback_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    callback_create_info.pfnUserCallback = DebugUtilsCallback;
    callback_create_info.pUserData = &callback_data;
    ici.pNext = &callback_create_info;

    // Create an instance, error if layer status INFO message not found
    ASSERT_VK_SUCCESS(vk::CreateInstance(&ici, nullptr, &local_instance));
    vk::DestroyInstance(local_instance, nullptr);

#ifndef NDEBUG
    // Create an instance, error if layer DEBUG_BUILD warning message not found
    callback_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    callback_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    ASSERT_VK_SUCCESS(vk::CreateInstance(&ici, nullptr, &local_instance));
    vk::DestroyInstance(local_instance, nullptr);
#endif
}

TEST_F(VkLayerTest, RequiredParameter) {
    TEST_DESCRIPTION("Specify VK_NULL_HANDLE, NULL, and 0 for required handle, pointer, array, and array count parameters");

    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "required parameter pFeatures specified as NULL");
    // Specify NULL for a pointer to a handle
    // Expected to trigger an error with
    // StatelessValidation::ValidateRequiredPointer
    vk::GetPhysicalDeviceFeatures(gpu(), NULL);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "required parameter pQueueFamilyPropertyCount specified as NULL");
    // Specify NULL for pointer to array count
    // Expected to trigger an error with StatelessValidation::ValidateArray
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), NULL, NULL);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-viewportCount-arraylength");
    // Specify 0 for a required array count
    // Expected to trigger an error with StatelessValidation::ValidateArray
    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    m_commandBuffer->SetViewport(0, 0, &viewport);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateImage-pCreateInfo-parameter");
    // Specify a null pImageCreateInfo struct pointer
    VkImage test_image;
    vk::CreateImage(device(), NULL, NULL, &test_image);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-pViewports-parameter");
    // Specify NULL for a required array
    // Expected to trigger an error with StatelessValidation::ValidateArray
    m_commandBuffer->SetViewport(0, 1, NULL);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "required parameter memory specified as VK_NULL_HANDLE");
    // Specify VK_NULL_HANDLE for a required handle
    // Expected to trigger an error with
    // StatelessValidation::ValidateRequiredHandle
    vk::UnmapMemory(device(), VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "required parameter pFences[0] specified as VK_NULL_HANDLE");
    // Specify VK_NULL_HANDLE for a required handle array entry
    // Expected to trigger an error with
    // StatelessValidation::ValidateRequiredHandleArray
    VkFence fence = VK_NULL_HANDLE;
    vk::ResetFences(device(), 1, &fence);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "required parameter pAllocateInfo specified as NULL");
    // Specify NULL for a required struct pointer
    // Expected to trigger an error with
    // StatelessValidation::ValidateStructType
    VkDeviceMemory memory = VK_NULL_HANDLE;
    vk::AllocateMemory(device(), NULL, NULL, &memory);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "value of faceMask must not be 0");
    // Specify 0 for a required VkFlags parameter
    // Expected to trigger an error with StatelessValidation::ValidateFlags
    m_commandBuffer->SetStencilReference(0, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-sType-sType");
    // Set a bogus sType and see what happens
    VkSemaphore semaphore = VK_NULL_HANDLE;
    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSubmitInfo submitInfo = LvlInitStruct<VkSubmitInfo>();
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &semaphore;
    submitInfo.pWaitDstStageMask = &stageFlags;
    submitInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vk::QueueSubmit(m_device->m_queue, 1, &submitInfo, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pWaitSemaphores-parameter");
    stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    // Set a null pointer for pWaitSemaphores
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = &stageFlags;
    vk::QueueSubmit(m_device->m_queue, 1, &submitInfo, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pWaitDstStageMask-parameter");
    submitInfo.pWaitSemaphores = &semaphore;
    submitInfo.pWaitDstStageMask = nullptr;
    vk::QueueSubmit(m_device->m_queue, 1, &submitInfo, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateRenderPass-pCreateInfo-parameter");
    VkRenderPass render_pass;
    vk::CreateRenderPass(device(), nullptr, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, SpecLinks) {
    TEST_DESCRIPTION("Test that spec links in a typical error message are well-formed");
    AddOptionalExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());

#ifdef ANNOTATED_SPEC_LINK
    bool test_annotated_spec_link = true;
#else   // ANNOTATED_SPEC_LINK
    bool test_annotated_spec_link = false;
#endif  // ANNOTATED_SPEC_LINK

    std::string spec_version;
    if (test_annotated_spec_link) {
        std::string major_version = std::to_string(VK_VERSION_MAJOR(VK_HEADER_VERSION_COMPLETE));
        std::string minor_version = std::to_string(VK_VERSION_MINOR(VK_HEADER_VERSION_COMPLETE));
        std::string patch_version = std::to_string(VK_VERSION_PATCH(VK_HEADER_VERSION_COMPLETE));
        spec_version = "doc/view/" + major_version + "." + minor_version + "." + patch_version + ".0/windows";
    } else {
        spec_version = "registry/vulkan/specs";
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, spec_version);
    vk::GetPhysicalDeviceFeatures(gpu(), NULL);
    m_errorMonitor->VerifyFound();

    // Now generate a 'default' message and check the link
    bool ycbcr_support =
        (IsExtensionsEnabled(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME) || (DeviceValidationVersion() >= VK_API_VERSION_1_1));
    bool maintenance2_support =
        (IsExtensionsEnabled(VK_KHR_MAINTENANCE_2_EXTENSION_NAME) || (DeviceValidationVersion() >= VK_API_VERSION_1_1));

    if (!((m_device->format_properties(VK_FORMAT_R8_UINT).optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) &&
          (ycbcr_support ^ maintenance2_support))) {
        GTEST_SKIP() << "Device does not support format and extensions required";
    }

    VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                   nullptr,
                                   0,
                                   VK_IMAGE_TYPE_2D,
                                   VK_FORMAT_R8_UINT,
                                   {128, 128, 1},
                                   1,
                                   1,
                                   VK_SAMPLE_COUNT_1_BIT,
                                   VK_IMAGE_TILING_OPTIMAL,
                                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                   VK_SHARING_MODE_EXCLUSIVE,
                                   0,
                                   nullptr,
                                   VK_IMAGE_LAYOUT_UNDEFINED};
    imageInfo.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    VkImageObj mutImage(m_device);
    mutImage.init(&imageInfo);
    ASSERT_TRUE(mutImage.initialized());

    VkImageViewCreateInfo imgViewInfo = LvlInitStruct<VkImageViewCreateInfo>();
    imgViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imgViewInfo.format = VK_FORMAT_B8G8R8A8_UNORM;  // different than createImage
    imgViewInfo.subresourceRange.layerCount = 1;
    imgViewInfo.subresourceRange.baseMipLevel = 0;
    imgViewInfo.subresourceRange.levelCount = 1;
    imgViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imgViewInfo.image = mutImage.handle();

    // VUIDs 01759 and 01760 should generate 'default' spec URLs, to search the registry
    CreateImageViewTest(*this, &imgViewInfo, "Vulkan-Docs/search");
}

TEST_F(VkLayerTest, UsePnextOnlyStructWithoutExtensionEnabled) {
    TEST_DESCRIPTION(
        "Validate that using VkPipelineTessellationDomainOriginStateCreateInfo in VkPipelineTessellationStateCreateInfo.pNext "
        "in a 1.0 context will generate an error message.");

    SetTargetApiVersion(VK_API_VERSION_1_0);

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (!m_device->phy().features().tessellationShader) {
        GTEST_SKIP() << "Device does not support tessellation shaders";
    }
    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj tcs(this, kTessellationControlMinimalGlsl, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    VkShaderObj tes(this, kTessellationEvalMinimalGlsl, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkPipelineInputAssemblyStateCreateInfo iasci{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
                                                 VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, VK_FALSE};
    VkPipelineTessellationDomainOriginStateCreateInfo tessellationDomainOriginStateInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO, VK_NULL_HANDLE,
        VK_TESSELLATION_DOMAIN_ORIGIN_UPPER_LEFT};
    VkPipelineTessellationStateCreateInfo tsci{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
                                               &tessellationDomainOriginStateInfo, 0, 3};
    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.pTessellationState = &tsci;
    pipe.gp_ci_.pInputAssemblyState = &iasci;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), tcs.GetStageCreateInfo(), tes.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineTessellationStateCreateInfo-pNext-pNext");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, PnextOnlyStructValidation) {
    TEST_DESCRIPTION("See if checks occur on structs ONLY used in pnext chains.");

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    // Create a device passing in a bad PdevFeatures2 value
    auto indexing_features = LvlInitStruct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(indexing_features);
    // Set one of the features values to an invalid boolean value
    indexing_features.descriptorBindingUniformBufferUpdateAfterBind = 800;

    uint32_t queue_node_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_node_count, NULL);
    std::vector<VkQueueFamilyProperties> queue_props;
    queue_props.resize(queue_node_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_node_count, queue_props.data());
    float priorities[] = {1.0f};
    VkDeviceQueueCreateInfo queue_info = LvlInitStruct<VkDeviceQueueCreateInfo>();
    queue_info.flags = 0;
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &priorities[0];
    VkDeviceCreateInfo dev_info = LvlInitStruct<VkDeviceCreateInfo>();
    dev_info.queueCreateInfoCount = 1;
    dev_info.pQueueCreateInfos = &queue_info;
    dev_info.enabledLayerCount = 0;
    dev_info.ppEnabledLayerNames = NULL;
    dev_info.enabledExtensionCount = m_device_extension_names.size();
    dev_info.ppEnabledExtensionNames = m_device_extension_names.data();
    dev_info.pNext = &features2;
    VkDevice dev;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "is neither VK_TRUE nor VK_FALSE");
    m_errorMonitor->SetUnexpectedError("Failed to create");
    vk::CreateDevice(gpu(), &dev_info, NULL, &dev);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ReservedParameter) {
    TEST_DESCRIPTION("Specify a non-zero value for a reserved parameter");

    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, " must be 0");
    // Specify 0 for a reserved VkFlags parameter
    // Expected to trigger an error with
    // StatelessValidation::ValidateReservedFlags
    VkSemaphore sem_handle = VK_NULL_HANDLE;
    VkSemaphoreCreateInfo sem_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    sem_info.flags = 1;
    vk::CreateSemaphore(device(), &sem_info, NULL, &sem_handle);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DebugMarkerNameTest) {
    TEST_DESCRIPTION("Ensure debug marker object names are printed in debug report output");

    AddRequiredExtensions(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Skipping object naming test with MockICD.";
    }

    VkBuffer buffer;
    VkDeviceMemory memory_1, memory_2;
    std::string memory_name = "memory_name";

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_create_info.size = 1;

    vk::CreateBuffer(device(), &buffer_create_info, nullptr, &buffer);

    VkMemoryRequirements memRequirements;
    vk::GetBufferMemoryRequirements(device(), buffer, &memRequirements);

    VkMemoryAllocateInfo memory_allocate_info = LvlInitStruct<VkMemoryAllocateInfo>();
    memory_allocate_info.allocationSize = memRequirements.size;
    memory_allocate_info.memoryTypeIndex = 0;

    vk::AllocateMemory(device(), &memory_allocate_info, nullptr, &memory_1);
    vk::AllocateMemory(device(), &memory_allocate_info, nullptr, &memory_2);

    VkDebugMarkerObjectNameInfoEXT name_info = LvlInitStruct<VkDebugMarkerObjectNameInfoEXT>();
    name_info.object = (uint64_t)memory_2;
    name_info.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT;
    name_info.pObjectName = memory_name.c_str();
    vk::DebugMarkerSetObjectNameEXT(device(), &name_info);

    vk::BindBufferMemory(device(), buffer, memory_1, 0);

    // Test core_validation layer
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, memory_name);
    vk::BindBufferMemory(device(), buffer, memory_2, 0);
    m_errorMonitor->VerifyFound();

    vk::FreeMemory(device(), memory_1, nullptr);
    memory_1 = VK_NULL_HANDLE;
    vk::FreeMemory(device(), memory_2, nullptr);
    memory_2 = VK_NULL_HANDLE;
    vk::DestroyBuffer(device(), buffer, nullptr);
    buffer = VK_NULL_HANDLE;

    VkCommandBuffer commandBuffer;
    std::string commandBuffer_name = "command_buffer_name";
    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk_testing::CommandPool command_pool_1(*m_device, pool_create_info);
    vk_testing::CommandPool command_pool_2(*m_device, pool_create_info);

    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool_1.handle();
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(device(), &command_buffer_allocate_info, &commandBuffer);

    name_info.object = (uint64_t)commandBuffer;
    name_info.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT;
    name_info.pObjectName = commandBuffer_name.c_str();
    vk::DebugMarkerSetObjectNameEXT(device(), &name_info);

    VkCommandBufferBeginInfo cb_begin_Info = LvlInitStruct<VkCommandBufferBeginInfo>();
    cb_begin_Info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vk::BeginCommandBuffer(commandBuffer, &cb_begin_Info);

    const VkRect2D scissor = {{-1, 0}, {16, 16}};
    const VkRect2D scissors[] = {scissor, scissor};

    // Test parameter_validation layer
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, commandBuffer_name);
    vk::CmdSetScissor(commandBuffer, 0, 1, scissors);
    m_errorMonitor->VerifyFound();

    // Test object_tracker layer
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, commandBuffer_name);
    vk::FreeCommandBuffers(device(), command_pool_2.handle(), 1, &commandBuffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DebugUtilsNameTest) {
    TEST_DESCRIPTION("Ensure debug utils object names are printed in debug messenger output");

    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());

    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Skipping object naming test with MockICD.";
    }

    DebugUtilsLabelCheckData callback_data;
    auto empty_callback = [](const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, DebugUtilsLabelCheckData *data) {
        data->count++;
    };
    callback_data.count = 0;
    callback_data.callback = empty_callback;

    auto callback_create_info = LvlInitStruct<VkDebugUtilsMessengerCreateInfoEXT>();
    callback_create_info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    callback_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    callback_create_info.pfnUserCallback = DebugUtilsCallback;
    callback_create_info.pUserData = &callback_data;
    VkDebugUtilsMessengerEXT my_messenger = VK_NULL_HANDLE;
    vk::CreateDebugUtilsMessengerEXT(instance(), &callback_create_info, nullptr, &my_messenger);

    VkBuffer buffer;
    VkDeviceMemory memory_1, memory_2;
    std::string memory_name = "memory_name";

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_create_info.size = 1;

    vk::CreateBuffer(device(), &buffer_create_info, nullptr, &buffer);

    VkMemoryRequirements memRequirements;
    vk::GetBufferMemoryRequirements(device(), buffer, &memRequirements);

    VkMemoryAllocateInfo memory_allocate_info = LvlInitStruct<VkMemoryAllocateInfo>();
    memory_allocate_info.allocationSize = memRequirements.size;
    memory_allocate_info.memoryTypeIndex = 0;

    vk::AllocateMemory(device(), &memory_allocate_info, nullptr, &memory_1);
    vk::AllocateMemory(device(), &memory_allocate_info, nullptr, &memory_2);

    VkDebugUtilsObjectNameInfoEXT name_info = LvlInitStruct<VkDebugUtilsObjectNameInfoEXT>();
    name_info.objectType = VK_OBJECT_TYPE_DEVICE_MEMORY;
    name_info.pObjectName = memory_name.c_str();

    // Pass in bad handle make sure ObjectTracker catches it
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDebugUtilsObjectNameInfoEXT-objectType-02590");
    name_info.objectHandle = (uint64_t)0xcadecade;
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);
    m_errorMonitor->VerifyFound();

    // Pass in null handle
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkSetDebugUtilsObjectNameEXT-pNameInfo-02588");
    name_info.objectHandle = 0;
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);
    m_errorMonitor->VerifyFound();

    // Pass in 'unknown' object type and see if parameter validation catches it
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkSetDebugUtilsObjectNameEXT-pNameInfo-02587");
    name_info.objectHandle = (uint64_t)memory_2;
    name_info.objectType = VK_OBJECT_TYPE_UNKNOWN;
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);
    m_errorMonitor->VerifyFound();

    name_info.objectType = VK_OBJECT_TYPE_DEVICE_MEMORY;
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    vk::BindBufferMemory(device(), buffer, memory_1, 0);

    // Test core_validation layer
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, memory_name);
    vk::BindBufferMemory(device(), buffer, memory_2, 0);
    m_errorMonitor->VerifyFound();

    vk::FreeMemory(device(), memory_1, nullptr);
    memory_1 = VK_NULL_HANDLE;
    vk::FreeMemory(device(), memory_2, nullptr);
    memory_2 = VK_NULL_HANDLE;
    vk::DestroyBuffer(device(), buffer, nullptr);
    buffer = VK_NULL_HANDLE;

    VkCommandBuffer commandBuffer;
    std::string commandBuffer_name = "command_buffer_name";
    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk_testing::CommandPool command_pool_1(*m_device, pool_create_info);
    vk_testing::CommandPool command_pool_2(*m_device, pool_create_info);

    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool_1.handle();
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(device(), &command_buffer_allocate_info, &commandBuffer);

    name_info.objectHandle = (uint64_t)commandBuffer;
    name_info.objectType = VK_OBJECT_TYPE_COMMAND_BUFFER;
    name_info.pObjectName = commandBuffer_name.c_str();
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    VkCommandBufferBeginInfo cb_begin_Info = LvlInitStruct<VkCommandBufferBeginInfo>();
    cb_begin_Info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vk::BeginCommandBuffer(commandBuffer, &cb_begin_Info);

    const VkRect2D scissor = {{-1, 0}, {16, 16}};
    const VkRect2D scissors[] = {scissor, scissor};

    auto command_label = LvlInitStruct<VkDebugUtilsLabelEXT>();
    command_label.pLabelName = "Command Label 0123";
    command_label.color[0] = 0.;
    command_label.color[1] = 1.;
    command_label.color[2] = 2.;
    command_label.color[3] = 3.0;
    bool command_label_test = false;
    auto command_label_callback = [command_label, &command_label_test](const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                                       DebugUtilsLabelCheckData *data) {
        data->count++;
        command_label_test = false;
        if (pCallbackData->cmdBufLabelCount == 1) {
            command_label_test = pCallbackData->pCmdBufLabels[0] == command_label;
        }
    };
    callback_data.callback = command_label_callback;

    vk::CmdInsertDebugUtilsLabelEXT(commandBuffer, &command_label);
    // Test parameter_validation layer
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, commandBuffer_name);
    vk::CmdSetScissor(commandBuffer, 0, 1, scissors);
    m_errorMonitor->VerifyFound();

    // Check the label test
    if (!command_label_test) {
        ADD_FAILURE() << "Command label '" << command_label.pLabelName << "' not passed to callback.";
    }

    // Test object_tracker layer
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, commandBuffer_name);
    vk::FreeCommandBuffers(device(), command_pool_2.handle(), 1, &commandBuffer);
    m_errorMonitor->VerifyFound();

    vk::DestroyDebugUtilsMessengerEXT(instance(), my_messenger, nullptr);
}

TEST_F(VkLayerTest, InvalidStructSType) {
    TEST_DESCRIPTION("Specify an invalid VkStructureType for a Vulkan structure's sType field");

    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "parameter pAllocateInfo->sType must be");
    // Zero struct memory, effectively setting sType to
    // VK_STRUCTURE_TYPE_APPLICATION_INFO
    // Expected to trigger an error with
    // StatelessValidation::ValidateStructType
    VkMemoryAllocateInfo alloc_info = {};
    VkDeviceMemory memory = VK_NULL_HANDLE;
    vk::AllocateMemory(device(), &alloc_info, NULL, &memory);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "parameter pSubmits[0].sType must be");
    // Zero struct memory, effectively setting sType to
    // VK_STRUCTURE_TYPE_APPLICATION_INFO
    // Expected to trigger an error with
    // StatelessValidation::ValidateStructTypeArray
    VkSubmitInfo submit_info = {};
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidStructPNext) {
    TEST_DESCRIPTION("Specify an invalid value for a Vulkan structure's pNext field");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    m_errorMonitor->SetDesiredFailureMsg((kErrorBit | kWarningBit), "VUID-VkCommandPoolCreateInfo-pNext-pNext");
    // Set VkCommandPoolCreateInfo::pNext to a non-NULL value, when pNext must be NULL.
    // Need to pick a function that has no allowed pNext structure types.
    // Expected to trigger an error with StatelessValidation::ValidateStructPnext
    VkCommandPool pool = VK_NULL_HANDLE;
    auto pool_ci = LvlInitStruct<VkCommandPoolCreateInfo>();
    auto app_info = LvlInitStruct<VkApplicationInfo>();
    pool_ci.pNext = &app_info;
    vk::CreateCommandPool(device(), &pool_ci, NULL, &pool);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg((kErrorBit | kWarningBit), " chain includes a structure with unexpected VkStructureType ");
    // Set VkMemoryAllocateInfo::pNext to a non-NULL value, but use
    // a function that has allowed pNext structure types and specify
    // a structure type that is not allowed.
    // Expected to trigger an error with StatelessValidation::ValidateStructPnext
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkMemoryAllocateInfo memory_alloc_info = LvlInitStruct<VkMemoryAllocateInfo>(&app_info);
    vk::AllocateMemory(device(), &memory_alloc_info, NULL, &memory);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg((kErrorBit | kWarningBit), " chain includes a structure with unexpected VkStructureType ");
    // Same concept as above, but unlike vkAllocateMemory where VkMemoryAllocateInfo is a const
    // in vkGetPhysicalDeviceProperties2, VkPhysicalDeviceProperties2 is not a const
    VkPhysicalDeviceProperties2 physical_device_properties2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&app_info);

    vk::GetPhysicalDeviceProperties2KHR(gpu(), &physical_device_properties2);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, UnrecognizedValueOutOfRange) {
    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "does not fall within the begin..end range of the core VkFormat enumeration tokens");
    // Specify an invalid VkFormat value
    // Expected to trigger an error with
    // StatelessValidation::ValidateRangedEnum
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(gpu(), static_cast<VkFormat>(8000), &format_properties);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, UnrecognizedValueBadMask) {
    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "contains flag bits that are not recognized members of");
    // Specify an invalid VkFlags bitmask value
    // Expected to trigger an error with StatelessValidation::ValidateFlags
    VkImageFormatProperties image_format_properties;
    vk::GetPhysicalDeviceImageFormatProperties(gpu(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                               static_cast<VkImageUsageFlags>(1 << 25), 0, &image_format_properties);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, UnrecognizedValueBadFlag) {
    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "contains flag bits that are not recognized members of");
    // Specify an invalid VkFlags array entry
    // Expected to trigger an error with StatelessValidation::ValidateFlagsArray
    VkSemaphore semaphore;
    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);
    // `stage_flags` is set to a value which, currently, is not a defined stage flag
    // `VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM` works well for this
    VkPipelineStageFlags stage_flags = VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
    // `waitSemaphoreCount` *must* be greater than 0 to perform this check
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore;
    submit_info.pWaitDstStageMask = &stage_flags;
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::DestroySemaphore(m_device->device(), semaphore, nullptr);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, UnrecognizedValueBadBool) {
    // Make sure using VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE doesn't trigger a false positive.
    AddRequiredExtensions(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Specify an invalid VkBool32 value, expecting a warning with StatelessValidation::ValidateBool32
    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;

    // Not VK_TRUE or VK_FALSE
    sampler_info.anisotropyEnable = 3;
    CreateSamplerTest(*this, &sampler_info, "is neither VK_TRUE nor VK_FALSE");
}

TEST_F(VkLayerTest, UnrecognizedValueMaxEnum) {
    ASSERT_NO_FATAL_FAILURE(Init());

    // Specify MAX_ENUM
    VkFormatProperties format_properties;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "does not fall within the begin..end range");
    vk::GetPhysicalDeviceFormatProperties(gpu(), VK_FORMAT_MAX_ENUM, &format_properties);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, LeakAnObject) {
    TEST_DESCRIPTION("Create a fence and destroy its device without first destroying the fence.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!IsPlatform(kMockICD)) {
        // This test leaks a fence (on purpose) and should not be run on a real driver
        GTEST_SKIP() << "This test only runs on the mock ICD";
    }

    // Workaround for overzealous layers checking even the guaranteed 0th queue family
    const auto q_props = vk_testing::PhysicalDevice(gpu()).queue_properties();
    ASSERT_TRUE(q_props.size() > 0);
    ASSERT_TRUE(q_props[0].queueCount > 0);

    const float q_priority[] = {1.0f};
    VkDeviceQueueCreateInfo queue_ci = LvlInitStruct<VkDeviceQueueCreateInfo>();
    queue_ci.queueFamilyIndex = 0;
    queue_ci.queueCount = 1;
    queue_ci.pQueuePriorities = q_priority;

    VkDeviceCreateInfo device_ci = LvlInitStruct<VkDeviceCreateInfo>();
    device_ci.queueCreateInfoCount = 1;
    device_ci.pQueueCreateInfos = &queue_ci;

    VkDevice leaky_device;
    ASSERT_VK_SUCCESS(vk::CreateDevice(gpu(), &device_ci, nullptr, &leaky_device));

    const VkFenceCreateInfo fence_ci = LvlInitStruct<VkFenceCreateInfo>();
    VkFence leaked_fence;
    ASSERT_VK_SUCCESS(vk::CreateFence(leaky_device, &fence_ci, nullptr, &leaked_fence));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyDevice-device-00378");
    vk::DestroyDevice(leaky_device, nullptr);
    m_errorMonitor->VerifyFound();

    // There's no way we can destroy the fence at this point. Even though DestroyDevice failed, the loader has already removed
    // references to the device
    m_errorMonitor->SetUnexpectedError("VUID-vkDestroyDevice-device-00378");
    m_errorMonitor->SetUnexpectedError("UNASSIGNED-ObjectTracker-ObjectLeak");
}

TEST_F(VkLayerTest, UseObjectWithWrongDevice) {
    TEST_DESCRIPTION(
        "Try to destroy a render pass object using a device other than the one it was created on. This should generate a distinct "
        "error from the invalid handle error.");
    // Create first device and renderpass
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Create second device
    float priorities[] = {1.0f};
    VkDeviceQueueCreateInfo queue_info = LvlInitStruct<VkDeviceQueueCreateInfo>();
    queue_info.flags = 0;
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &priorities[0];

    VkDeviceCreateInfo device_create_info = LvlInitStruct<VkDeviceCreateInfo>();
    auto features = m_device->phy().features();
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pQueueCreateInfos = &queue_info;
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = NULL;
    device_create_info.pEnabledFeatures = &features;

    VkDevice second_device;
    ASSERT_VK_SUCCESS(vk::CreateDevice(gpu(), &device_create_info, NULL, &second_device));

    // Try to destroy the renderpass from the first device using the second device
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyRenderPass-renderPass-parent");
    vk::DestroyRenderPass(second_device, m_renderPass, NULL);
    m_errorMonitor->VerifyFound();

    vk::DestroyDevice(second_device, NULL);
}

TEST_F(VkLayerTest, InvalidAllocationCallbacks) {
    TEST_DESCRIPTION("Test with invalid VkAllocationCallbacks");

    ASSERT_NO_FATAL_FAILURE(Init());

    const std::optional queueFamilyIndex = DeviceObj()->QueueFamilyMatching(0, 0, true);
    if (!queueFamilyIndex) {
        GTEST_SKIP() << "Required queue families not present";
    }

    // vk::CreateInstance, and vk::CreateDevice tend to crash in the Loader Trampoline ATM, so choosing vk::CreateCommandPool
    const VkCommandPoolCreateInfo cpci = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr, 0, queueFamilyIndex.value()};
    VkCommandPool cmdPool;

    struct Alloc {
        static VKAPI_ATTR void *VKAPI_CALL alloc(void *, size_t, size_t, VkSystemAllocationScope) { return nullptr; };
        static VKAPI_ATTR void *VKAPI_CALL realloc(void *, void *, size_t, size_t, VkSystemAllocationScope) { return nullptr; };
        static VKAPI_ATTR void VKAPI_CALL free(void *, void *){};
        static VKAPI_ATTR void VKAPI_CALL internalAlloc(void *, size_t, VkInternalAllocationType, VkSystemAllocationScope){};
        static VKAPI_ATTR void VKAPI_CALL internalFree(void *, size_t, VkInternalAllocationType, VkSystemAllocationScope){};
    };

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAllocationCallbacks-pfnAllocation-00632");
        const VkAllocationCallbacks allocator = {nullptr, nullptr, Alloc::realloc, Alloc::free, nullptr, nullptr};
        vk::CreateCommandPool(device(), &cpci, &allocator, &cmdPool);
        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAllocationCallbacks-pfnReallocation-00633");
        const VkAllocationCallbacks allocator = {nullptr, Alloc::alloc, nullptr, Alloc::free, nullptr, nullptr};
        vk::CreateCommandPool(device(), &cpci, &allocator, &cmdPool);
        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAllocationCallbacks-pfnFree-00634");
        const VkAllocationCallbacks allocator = {nullptr, Alloc::alloc, Alloc::realloc, nullptr, nullptr, nullptr};
        vk::CreateCommandPool(device(), &cpci, &allocator, &cmdPool);
        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAllocationCallbacks-pfnInternalAllocation-00635");
        const VkAllocationCallbacks allocator = {nullptr, Alloc::alloc, Alloc::realloc, Alloc::free, nullptr, Alloc::internalFree};
        vk::CreateCommandPool(device(), &cpci, &allocator, &cmdPool);
        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAllocationCallbacks-pfnInternalAllocation-00635");
        const VkAllocationCallbacks allocator = {nullptr, Alloc::alloc, Alloc::realloc, Alloc::free, Alloc::internalAlloc, nullptr};
        vk::CreateCommandPool(device(), &cpci, &allocator, &cmdPool);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, MismatchedQueueFamiliesOnSubmit) {
    TEST_DESCRIPTION(
        "Submit command buffer created using one queue family and attempt to submit them on a queue created in a different queue "
        "family.");

    ASSERT_NO_FATAL_FAILURE(Init());  // assumes it initializes all queue families on vk::CreateDevice

    // This test is meaningless unless we have multiple queue families
    auto queue_family_properties = m_device->phy().queue_properties();
    std::vector<uint32_t> queue_families;
    for (uint32_t i = 0; i < queue_family_properties.size(); ++i)
        if (queue_family_properties[i].queueCount > 0) queue_families.push_back(i);

    if (queue_families.size() < 2) {
        GTEST_SKIP() << "Device only has one queue family";
    }

    const uint32_t queue_family = queue_families[0];

    const uint32_t other_queue_family = queue_families[1];
    VkQueue other_queue;
    vk::GetDeviceQueue(m_device->device(), other_queue_family, 0, &other_queue);

    VkCommandPoolObj cmd_pool(m_device, queue_family);
    VkCommandBufferObj cmd_buff(m_device, &cmd_pool);

    cmd_buff.begin();
    cmd_buff.end();

    // Submit on the wrong queue
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd_buff.handle();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit-pCommandBuffers-00074");
    vk::QueueSubmit(other_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DeviceFeature2AndVertexAttributeDivisorExtensionUnenabled) {
    TEST_DESCRIPTION(
        "Test unenabled VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME & "
        "VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME.");

    VkPhysicalDeviceFeatures2 pd_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>();

    ASSERT_NO_FATAL_FAILURE(Init());
    vk_testing::QueueCreateInfoArray queue_info(m_device->queue_props);
    VkDeviceCreateInfo device_create_info = LvlInitStruct<VkDeviceCreateInfo>(&pd_features2);
    device_create_info.queueCreateInfoCount = queue_info.size();
    device_create_info.pQueueCreateInfos = queue_info.data();
    VkDevice testDevice;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-pNext-pNext");
    m_errorMonitor->SetUnexpectedError("Failed to create device chain");
    vk::CreateDevice(gpu(), &device_create_info, NULL, &testDevice);
    m_errorMonitor->VerifyFound();

    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT vadf = LvlInitStruct<VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT>();
    device_create_info.pNext = &vadf;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VK_EXT_vertex_attribute_divisor must be enabled when it creates a device");
    m_errorMonitor->SetUnexpectedError("Failed to create device chain");
    vk::CreateDevice(gpu(), &device_create_info, NULL, &testDevice);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, Features12Features13AndpNext) {
    TEST_DESCRIPTION("Test VkPhysicalDeviceVulkan12Features and illegal struct in pNext");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_8BIT_STORAGE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_16BIT_STORAGE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    VkPhysicalDevice16BitStorageFeatures sixteen_bit = LvlInitStruct<VkPhysicalDevice16BitStorageFeatures>();
    sixteen_bit.storageBuffer16BitAccess = true;
    VkPhysicalDeviceVulkan11Features features11 = LvlInitStruct<VkPhysicalDeviceVulkan11Features>(&sixteen_bit);
    features11.storageBuffer16BitAccess = true;

    VkPhysicalDevice8BitStorageFeatures eight_bit = LvlInitStruct<VkPhysicalDevice8BitStorageFeatures>(&features11);
    eight_bit.storageBuffer8BitAccess = true;
    VkPhysicalDeviceVulkan12Features features12 = LvlInitStruct<VkPhysicalDeviceVulkan12Features>(&eight_bit);
    features12.storageBuffer8BitAccess = true;

    VkPhysicalDeviceVulkan13Features features13 = {};
    VkPhysicalDeviceDynamicRenderingFeatures dyn_rendering_features = {};
    if (DeviceValidationVersion() >= VK_API_VERSION_1_3) {
        dyn_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
        dyn_rendering_features.dynamicRendering = true;
        dyn_rendering_features.pNext = &eight_bit;
        features13 = LvlInitStruct<VkPhysicalDeviceVulkan13Features>(&dyn_rendering_features);
        features13.dynamicRendering = true;
        features12.pNext = &features13;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-pNext-06532");
    }

    vk_testing::PhysicalDevice physical_device(gpu());
    vk_testing::QueueCreateInfoArray queue_info(physical_device.queue_properties());
    std::vector<VkDeviceQueueCreateInfo> create_queue_infos;
    auto qci = queue_info.data();
    for (uint32_t i = 0; i < queue_info.size(); ++i) {
        if (qci[i].queueCount) {
            create_queue_infos.push_back(qci[i]);
        }
    }

    VkDeviceCreateInfo device_create_info = LvlInitStruct<VkDeviceCreateInfo>(&features12);
    device_create_info.queueCreateInfoCount = queue_info.size();
    device_create_info.pQueueCreateInfos = queue_info.data();
    VkDevice testDevice;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-pNext-02829");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-pNext-02830");
    m_errorMonitor->SetUnexpectedError("Failed to create device chain");
    vk::CreateDevice(gpu(), &device_create_info, NULL, &testDevice);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, RequiredPromotedFeaturesExtensions) {
    TEST_DESCRIPTION("Checks that features are enabled if extension is passed in for promoted extensions with requirement.");

    // targets  each possible version
    SetTargetApiVersion(VK_API_VERSION_1_1);
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    const bool test_1_2 = (DeviceValidationVersion() >= VK_API_VERSION_1_2);

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    vk_testing::PhysicalDevice physical_device(gpu());
    vk_testing::QueueCreateInfoArray queue_info(physical_device.queue_properties());
    std::vector<VkDeviceQueueCreateInfo> create_queue_infos;
    auto qci = queue_info.data();
    for (uint32_t i = 0; i < queue_info.size(); ++i) {
        if (qci[i].queueCount) {
            create_queue_infos.push_back(qci[i]);
        }
    }

    // Explicity set all tested features to false
    VkPhysicalDeviceVulkan12Features features12 = LvlInitStruct<VkPhysicalDeviceVulkan12Features>();
    features12.drawIndirectCount = VK_FALSE;
    features12.samplerMirrorClampToEdge = VK_FALSE;
    features12.descriptorIndexing = VK_FALSE;
    features12.samplerFilterMinmax = VK_FALSE;
    features12.shaderOutputViewportIndex = VK_FALSE;
    features12.shaderOutputLayer = VK_TRUE;  // Set true since both shader_viewport features need to true

    VkPhysicalDeviceVulkan11Features features11 = LvlInitStruct<VkPhysicalDeviceVulkan11Features>();
    features11.pNext = (test_1_2 == true) ? &features12 : nullptr;
    features11.shaderDrawParameters = VK_FALSE;

    std::vector<const char *> device_extensions;

    // Go through each extension and if supported add to list and add failure to check for
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME)) {
        device_extensions.push_back(VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-04476");
    }
    if (test_1_2 == true) {
        if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME)) {
            device_extensions.push_back(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-02831");
        }
        if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME)) {
            device_extensions.push_back(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME);
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-02832");
        }
        if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME)) {
            device_extensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
            device_extensions.push_back(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-02833");
        }
        if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME)) {
            device_extensions.push_back(VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME);
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-02834");
        }
        if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME)) {
            device_extensions.push_back(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-02835");
        }
    } else {
        // VkPhysicalDeviceVulkan11Features was not added until Vulkan 1.2
        m_errorMonitor->SetUnexpectedError("VUID-VkDeviceCreateInfo-pNext-pNext");
    }

    VkDeviceCreateInfo device_create_info = LvlInitStruct<VkDeviceCreateInfo>(&features11);
    device_create_info.queueCreateInfoCount = queue_info.size();
    device_create_info.pQueueCreateInfos = queue_info.data();
    device_create_info.ppEnabledExtensionNames = device_extensions.data();
    device_create_info.enabledExtensionCount = device_extensions.size();
    VkDevice testDevice;

    m_errorMonitor->SetUnexpectedError("Failed to create device chain");
    vk::CreateDevice(gpu(), &device_create_info, NULL, &testDevice);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, FeaturesVariablePointer) {
    TEST_DESCRIPTION("Checks VK_KHR_variable_pointers features.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_VARIABLE_POINTERS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    std::vector<const char *> device_extensions;
    device_extensions.push_back(VK_KHR_VARIABLE_POINTERS_EXTENSION_NAME);
    device_extensions.push_back(VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME);

    // Create a device that enables variablePointers but not variablePointersStorageBuffer
    auto variable_features = LvlInitStruct<VkPhysicalDeviceVariablePointersFeatures>();
    auto features2 = GetPhysicalDeviceFeatures2(variable_features);
    if (variable_features.variablePointers == VK_FALSE) {
        GTEST_SKIP() << "variablePointer feature not supported";
    }

    variable_features.variablePointersStorageBuffer = VK_FALSE;

    vk_testing::PhysicalDevice physical_device(gpu());
    vk_testing::QueueCreateInfoArray queue_info(physical_device.queue_properties());
    std::vector<VkDeviceQueueCreateInfo> create_queue_infos;
    auto qci = queue_info.data();
    for (uint32_t i = 0; i < queue_info.size(); ++i) {
        if (qci[i].queueCount) {
            create_queue_infos.push_back(qci[i]);
        }
    }

    VkDeviceCreateInfo device_create_info = LvlInitStruct<VkDeviceCreateInfo>(&features2);
    device_create_info.queueCreateInfoCount = queue_info.size();
    device_create_info.pQueueCreateInfos = queue_info.data();
    device_create_info.ppEnabledExtensionNames = device_extensions.data();
    device_create_info.enabledExtensionCount = device_extensions.size();
    VkDevice testDevice;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPhysicalDeviceVariablePointersFeatures-variablePointers-01431");
    vk::CreateDevice(gpu(), &device_create_info, NULL, &testDevice);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ValidationCacheTestBadMerge) {
    AddRequiredExtensions(VK_EXT_VALIDATION_CACHE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkValidationCacheCreateInfoEXT validationCacheCreateInfo = LvlInitStruct<VkValidationCacheCreateInfoEXT>();
    validationCacheCreateInfo.initialDataSize = 0;
    validationCacheCreateInfo.pInitialData = NULL;
    validationCacheCreateInfo.flags = 0;
    VkValidationCacheEXT validationCache = VK_NULL_HANDLE;
    VkResult res = vk::CreateValidationCacheEXT(m_device->device(), &validationCacheCreateInfo, nullptr, &validationCache);
    ASSERT_VK_SUCCESS(res);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkMergeValidationCachesEXT-dstCache-01536");
    res = vk::MergeValidationCachesEXT(m_device->device(), validationCache, 1, &validationCache);
    m_errorMonitor->VerifyFound();

    vk::DestroyValidationCacheEXT(m_device->device(), validationCache, nullptr);
}

TEST_F(VkLayerTest, UnclosedAndDuplicateQueries) {
    TEST_DESCRIPTION("End a command buffer with a query still in progress, create nested queries.");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 0, &queue);

    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_create_info.queryCount = 5;
    vk_testing::QueryPool query_pool(*m_device, query_pool_create_info);
    m_commandBuffer->begin();
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool.handle(), 0, 5);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryPool-01922");
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 1, 0);
    // Attempt to begin a query that has the same type as an active query
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 3, 0);
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 1);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkEndCommandBuffer-commandBuffer-00061");
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
    vk::EndCommandBuffer(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, StageMaskHost) {
    TEST_DESCRIPTION("Test invalid usage of VK_PIPELINE_STAGE_HOST_BIT.");
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkEventCreateInfo event_create_info = LvlInitStruct<VkEventCreateInfo>();
    vk_testing::Event event(*m_device, event_create_info);
    ASSERT_TRUE(event.initialized());

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetEvent-stageMask-01149");
    vk::CmdSetEvent(m_commandBuffer->handle(), event.handle(), VK_PIPELINE_STAGE_HOST_BIT);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResetEvent-stageMask-01153");
    vk::CmdResetEvent(m_commandBuffer->handle(), event.handle(), VK_PIPELINE_STAGE_HOST_BIT);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();

    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    vk_testing::Semaphore semaphore(*m_device, semaphore_create_info);
    ASSERT_TRUE(semaphore.initialized());

    VkPipelineStageFlags stage_flags = VK_PIPELINE_STAGE_HOST_BIT;
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();

    // Signal the semaphore so the next test can wait on it.
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore.handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = nullptr;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore.handle();
    submit_info.pWaitDstStageMask = &stage_flags;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pWaitDstStageMask-00078");
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // Need to ensure semaphore is not in use before the test ends and it gets destroyed
    vk::QueueWaitIdle(m_device->m_queue);
}

#if GTEST_IS_THREADSAFE
TEST_F(VkLayerTest, ThreadCommandBufferCollision) {
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "THREADING ERROR");
    m_errorMonitor->SetAllowedFailureMsg("THREADING ERROR");  // Ignore any extra threading errors found beyond the first one

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Test takes magnitude of time longer for profiles and slows down testing
    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    // Calls AllocateCommandBuffers
    VkCommandBufferObj commandBuffer(m_device, m_commandPool);

    commandBuffer.begin();

    VkEventCreateInfo event_info = LvlInitStruct<VkEventCreateInfo>();
    vk_testing::Event event(*m_device, event_info);
    VkResult err;

    err = vk::ResetEvent(device(), event.handle());
    ASSERT_VK_SUCCESS(err);

    ThreadTestData data;
    data.commandBuffer = commandBuffer.handle();
    data.event = event.handle();
    std::atomic<bool> bailout{false};
    data.bailout = &bailout;
    m_errorMonitor->SetBailout(data.bailout);

    // First do some correct operations using multiple threads.
    // Add many entries to command buffer from another thread.
    std::thread thread1(AddToCommandBuffer, &data);
    // Make non-conflicting calls from this thread at the same time.
    for (int i = 0; i < 80000; i++) {
        uint32_t count;
        vk::EnumeratePhysicalDevices(instance(), &count, NULL);
    }
    thread1.join();

    // Then do some incorrect operations using multiple threads.
    // Add many entries to command buffer from another thread.
    std::thread thread2(AddToCommandBuffer, &data);
    // Add many entries to command buffer from this thread at the same time.
    AddToCommandBuffer(&data);

    thread2.join();
    commandBuffer.end();

    m_errorMonitor->SetBailout(NULL);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ThreadUpdateDescriptorCollision) {
    TEST_DESCRIPTION("Two threads updating the same descriptor set, expected to generate a threading error");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "THREADING ERROR : vkUpdateDescriptorSets");
    m_errorMonitor->SetAllowedFailureMsg("THREADING ERROR");  // Ignore any extra threading errors found beyond the first one

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet normal_descriptor_set(m_device,
                                              {
                                                  {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                              },
                                              0);

    VkBufferObj buffer(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    ThreadTestData data;
    data.device = device();
    data.descriptorSet = normal_descriptor_set.set_;
    data.binding = 0;
    data.buffer = buffer.handle();
    std::atomic<bool> bailout{false};
    data.bailout = &bailout;
    m_errorMonitor->SetBailout(data.bailout);

    // Update descriptors from another thread.
    std::thread thread(UpdateDescriptor, &data);
    // Update descriptors from this thread at the same time.

    ThreadTestData data2;
    data2.device = device();
    data2.descriptorSet = normal_descriptor_set.set_;
    data2.binding = 1;
    data2.buffer = buffer.handle();
    data2.bailout = &bailout;

    UpdateDescriptor(&data2);

    thread.join();

    m_errorMonitor->SetBailout(NULL);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ThreadUpdateDescriptorUpdateAfterBindNoCollision) {
    TEST_DESCRIPTION("Two threads updating the same UAB descriptor set, expected not to generate a threading error");

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // Create a device that enables descriptorBindingStorageBufferUpdateAfterBind
    auto indexing_features = LvlInitStruct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(indexing_features);
    if (VK_FALSE == indexing_features.descriptorBindingStorageBufferUpdateAfterBind) {
        GTEST_SKIP() << "Test requires (unsupported) descriptorBindingStorageBufferUpdateAfterBind";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    std::array<VkDescriptorBindingFlagsEXT, 2> flags = {
        {VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT}};
    auto flags_create_info = LvlInitStruct<VkDescriptorSetLayoutBindingFlagsCreateInfoEXT>();
    flags_create_info.bindingCount = (uint32_t)flags.size();
    flags_create_info.pBindingFlags = flags.data();

    OneOffDescriptorSet normal_descriptor_set(m_device,
                                              {
                                                  {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                              },
                                              VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT, &flags_create_info,
                                              VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT);

    VkBufferObj buffer(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    ThreadTestData data;
    data.device = device();
    data.descriptorSet = normal_descriptor_set.set_;
    data.binding = 0;
    data.buffer = buffer.handle();
    std::atomic<bool> bailout{false};
    data.bailout = &bailout;
    m_errorMonitor->SetBailout(data.bailout);

    // Update descriptors from another thread.
    std::thread thread(UpdateDescriptor, &data);
    // Update descriptors from this thread at the same time.

    ThreadTestData data2;
    data2.device = device();
    data2.descriptorSet = normal_descriptor_set.set_;
    data2.binding = 1;
    data2.buffer = buffer.handle();
    data2.bailout = &bailout;

    UpdateDescriptor(&data2);

    thread.join();

    m_errorMonitor->SetBailout(NULL);
}
#endif  // GTEST_IS_THREADSAFE

TEST_F(VkLayerTest, ExecuteUnrecordedCB) {
    TEST_DESCRIPTION("Attempt vkQueueSubmit with a CB in the initial state");

    ASSERT_NO_FATAL_FAILURE(Init());
    // never record m_commandBuffer

    VkSubmitInfo si = LvlInitStruct<VkSubmitInfo>();
    si.commandBufferCount = 1;
    si.pCommandBuffers = &m_commandBuffer->handle();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit-pCommandBuffers-00070");
    vk::QueueSubmit(m_device->m_queue, 1, &si, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // Testing an "unfinished secondary CB" crashes on some HW/drivers (notably Pixel 3 and RADV)
    // VkCommandBufferObj cb(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    // m_commandBuffer->begin();
    // vk::CmdExecuteCommands(m_commandBuffer->handle(), 1u, &cb.handle());
    // m_commandBuffer->end();

    // m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit-pCommandBuffers-00072");
    // vk::QueueSubmit(m_device->m_queue, 1, &si, VK_NULL_HANDLE);
    // m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, Maintenance1AndNegativeViewport) {
    TEST_DESCRIPTION("Attempt to enable AMD_negative_viewport_height and Maintenance1_KHR extension simultaneously");

    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!((DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME)) &&
          (DeviceExtensionSupported(gpu(), nullptr, VK_AMD_NEGATIVE_VIEWPORT_HEIGHT_EXTENSION_NAME)))) {
        GTEST_SKIP() << "Maintenance1 and AMD_negative viewport height extensions not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    vk_testing::QueueCreateInfoArray queue_info(m_device->queue_props);
    const char *extension_names[2] = {"VK_KHR_maintenance1", "VK_AMD_negative_viewport_height"};
    VkDevice testDevice;
    VkDeviceCreateInfo device_create_info = LvlInitStruct<VkDeviceCreateInfo>();
    auto features = m_device->phy().features();
    device_create_info.queueCreateInfoCount = queue_info.size();
    device_create_info.pQueueCreateInfos = queue_info.data();
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = NULL;
    device_create_info.enabledExtensionCount = 2;
    device_create_info.ppEnabledExtensionNames = (const char *const *)extension_names;
    device_create_info.pEnabledFeatures = &features;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-00374");
    // The following unexpected error is coming from the LunarG loader. Do not make it a desired message because platforms that do
    // not use the LunarG loader (e.g. Android) will not see the message and the test will fail.
    m_errorMonitor->SetUnexpectedError("Failed to create device chain.");
    vk::CreateDevice(gpu(), &device_create_info, NULL, &testDevice);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ApiVersion1_1AndNegativeViewport) {
    TEST_DESCRIPTION("Attempt to enable AMD_negative_viewport_height with api version 1.1");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_AMD_NEGATIVE_VIEWPORT_HEIGHT_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    vk_testing::PhysicalDevice physical_device(gpu_);
    VkPhysicalDeviceFeatures features = physical_device.features();
    vk_testing::QueueCreateInfoArray queue_info(physical_device.queue_properties());
    const char *extension_names[1] = {VK_AMD_NEGATIVE_VIEWPORT_HEIGHT_EXTENSION_NAME};
    VkDevice testDevice;
    VkDeviceCreateInfo device_create_info = LvlInitStruct<VkDeviceCreateInfo>();
    device_create_info.queueCreateInfoCount = queue_info.size();
    device_create_info.pQueueCreateInfos = queue_info.data();
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = NULL;
    device_create_info.enabledExtensionCount = 1;
    device_create_info.ppEnabledExtensionNames = (const char *const *)extension_names;
    device_create_info.pEnabledFeatures = &features;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-01840");
    // The following unexpected error is coming from the LunarG loader. Do not make it a desired message because platforms that do
    // not use the LunarG loader (e.g. Android) will not see the message and the test will fail.
    m_errorMonitor->SetUnexpectedError("Failed to create device chain.");
    vk::CreateDevice(gpu(), &device_create_info, NULL, &testDevice);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ResetEventThenSet) {
    TEST_DESCRIPTION("Reset an event then set it after the reset has been submitted.");

    ASSERT_NO_FATAL_FAILURE(Init());
    VkEventCreateInfo event_create_info = LvlInitStruct<VkEventCreateInfo>();
    vk_testing::Event event(*m_device, event_create_info);

    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk_testing::CommandPool command_pool(*m_device, pool_create_info);

    VkCommandBuffer command_buffer;
    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool.handle();
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &command_buffer);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 0, &queue);

    {
        VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        vk::BeginCommandBuffer(command_buffer, &begin_info);

        vk::CmdResetEvent(command_buffer, event.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
        vk::EndCommandBuffer(command_buffer);
    }
    {
        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = nullptr;
        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "that is already in use by a command buffer.");
        vk::SetEvent(m_device->device(), event.handle());
        m_errorMonitor->VerifyFound();
    }

    vk::QueueWaitIdle(queue);

    vk::FreeCommandBuffers(m_device->device(), command_pool.handle(), 1, &command_buffer);
}

TEST_F(VkLayerTest, FreeCommandBuffersNull) {
    TEST_DESCRIPTION("Can pass NULL for vkFreeCommandBuffers");
    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkFreeCommandBuffers-pCommandBuffers-00048");
    vk::FreeCommandBuffers(m_device->device(), m_commandPool->handle(), 2, nullptr);
    m_errorMonitor->VerifyFound();

    VkCommandBuffer invalid_cb = CastToHandle<VkCommandBuffer, uintptr_t>(0xbaadbeef);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkFreeCommandBuffers-pCommandBuffers-00048");
    vk::FreeCommandBuffers(m_device->device(), m_commandPool->handle(), 1, &invalid_cb);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, FreeDescriptorSetsNull) {
    TEST_DESCRIPTION("Can pass NULL for vkFreeDescriptorSets");
    ASSERT_NO_FATAL_FAILURE(Init());

    VkDescriptorPoolSize ds_type_count = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1};
    auto ds_pool_ci = LvlInitStruct<VkDescriptorPoolCreateInfo>();
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    ds_pool_ci.pPoolSizes = &ds_type_count;
    vk_testing::DescriptorPool ds_pool(*m_device, ds_pool_ci);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkFreeDescriptorSets-pDescriptorSets-00310");
    vk::FreeDescriptorSets(m_device->device(), ds_pool.handle(), 2, nullptr);
    m_errorMonitor->VerifyFound();

    VkDescriptorSet invalid_set = CastToHandle<VkDescriptorSet, uintptr_t>(0xbaadbeef);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkFreeDescriptorSets-pDescriptorSets-00310");
    vk::FreeDescriptorSets(m_device->device(), ds_pool.handle(), 1, &invalid_set);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ValidateStride) {
    TEST_DESCRIPTION("Validate Stride.");
    ASSERT_NO_FATAL_FAILURE(Init(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_props(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, queue_props.data());
    if (queue_props[m_device->graphics_queue_node_index_].timestampValidBits == 0) {
        GTEST_SKIP() << " Device graphic queue has timestampValidBits of 0, skipping.";
    }

    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_ci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_ci.queryCount = 1;
    vk_testing::QueryPool query_pool(*m_device, query_pool_ci);

    m_commandBuffer->begin();
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool.handle(), 0, 1);
    vk::CmdWriteTimestamp(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, query_pool.handle(), 0);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    char data_space;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-flags-02828");
    vk::GetQueryPoolResults(m_device->handle(), query_pool.handle(), 0, 1, sizeof(data_space), &data_space, 1,
                            VK_QUERY_RESULT_WAIT_BIT);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-flags-00815");
    vk::GetQueryPoolResults(m_device->handle(), query_pool.handle(), 0, 1, sizeof(data_space), &data_space, 1,
                            (VK_QUERY_RESULT_WAIT_BIT | VK_QUERY_RESULT_64_BIT));
    m_errorMonitor->VerifyFound();

    char data_space4[4] = "";
    vk::GetQueryPoolResults(m_device->handle(), query_pool.handle(), 0, 1, sizeof(data_space4), &data_space4, 4,
                            VK_QUERY_RESULT_WAIT_BIT);

    char data_space8[8] = "";
    vk::GetQueryPoolResults(m_device->handle(), query_pool.handle(), 0, 1, sizeof(data_space8), &data_space8, 8,
                            (VK_QUERY_RESULT_WAIT_BIT | VK_QUERY_RESULT_64_BIT));

    uint32_t qfi = 0;
    VkBufferCreateInfo buff_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buff_create_info.size = 128;
    buff_create_info.usage =
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    buff_create_info.queueFamilyIndexCount = 1;
    buff_create_info.pQueueFamilyIndices = &qfi;
    VkBufferObj buffer;
    buffer.init(*m_device, buff_create_info);

    m_commandBuffer->reset();
    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-flags-00822");
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool.handle(), 0, 1, buffer.handle(), 1, 1, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-flags-00823");
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool.handle(), 0, 1, buffer.handle(), 1, 1,
                                VK_QUERY_RESULT_64_BIT);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool.handle(), 0, 1, buffer.handle(), 4, 4, 0);

    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool.handle(), 0, 1, buffer.handle(), 8, 8,
                                VK_QUERY_RESULT_64_BIT);

    if (m_device->phy().features().multiDrawIndirect) {
        auto buffer_memory_barrier = buffer.buffer_memory_barrier(
            VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_INDEX_READ_BIT, 0, VK_WHOLE_SIZE);
        m_commandBuffer->PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT,
                                         VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr, 1,
                                         &buffer_memory_barrier, 0, nullptr);

        CreatePipelineHelper helper(*this);
        helper.InitInfo();
        helper.InitState();
        helper.CreateGraphicsPipeline();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, helper.pipeline_);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirect-drawCount-00476");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirect-drawCount-00488");
        vk::CmdDrawIndirect(m_commandBuffer->handle(), buffer.handle(), 0, 100, 2);
        m_errorMonitor->VerifyFound();

        vk::CmdDrawIndirect(m_commandBuffer->handle(), buffer.handle(), 0, 2, 24);

        vk::CmdBindIndexBuffer(m_commandBuffer->handle(), buffer.handle(), 0, VK_INDEX_TYPE_UINT16);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirect-drawCount-00528");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirect-drawCount-00540");
        vk::CmdDrawIndexedIndirect(m_commandBuffer->handle(), buffer.handle(), 0, 100, 2);
        m_errorMonitor->VerifyFound();

        auto draw_count = m_device->phy().properties().limits.maxDrawIndirectCount;
        if (draw_count != vvl::kU32Max) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirect-drawCount-02719");
            vk::CmdDrawIndirect(m_commandBuffer->handle(), buffer.handle(), 0, draw_count + 1, 2);
            m_errorMonitor->VerifyFound();
        }

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirect-drawCount-00487");
        vk::CmdDrawIndirect(m_commandBuffer->handle(), buffer.handle(), buff_create_info.size, 1, 2);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndexedIndirect-drawCount-00539");
        vk::CmdDrawIndexedIndirect(m_commandBuffer->handle(), buffer.handle(), buff_create_info.size, 1, 2);
        m_errorMonitor->VerifyFound();

        vk::CmdDrawIndexedIndirect(m_commandBuffer->handle(), buffer.handle(), 0, 2, 24);

        vk::CmdEndRenderPass(m_commandBuffer->handle());
        m_commandBuffer->end();

    } else {
        printf("Test requires unsupported multiDrawIndirect feature. Skipped.\n");
    }
}

TEST_F(VkLayerTest, ValidateNVDeviceDiagnosticCheckpoints) {
    TEST_DESCRIPTION("General testing of VK_NV_device_diagnostic_checkpoints");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    uint32_t data = 100;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetCheckpointNV-commandBuffer-recording");
    vk::CmdSetCheckpointNV(m_commandBuffer->handle(), &data);
    m_errorMonitor->VerifyFound();
}

template <typename ExtType, typename Parm>
void ExtendedDynStateCalls(ErrorMonitor *error_monitor, VkCommandBuffer cmd_buf, ExtType ext_call, const char *vuid, Parm parm) {
    error_monitor->SetDesiredFailureMsg(kErrorBit, vuid);
    ext_call(cmd_buf, parm);
    error_monitor->VerifyFound();
}

TEST_F(VkLayerTest, ValidateArrayLength) {
    TEST_DESCRIPTION("Validate arraylength VUs");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Used to have a valid pointed to set object too
    VkCommandBuffer unused_command_buffer;
    VkDescriptorSet unused_descriptor_set;

    VkDescriptorSetObj descriptor_set_obj(m_device);
    descriptor_set_obj.AppendDummy();
    descriptor_set_obj.CreateVKDescriptorSet(m_commandBuffer);
    VkDescriptorSet descriptor_set = descriptor_set_obj.GetDescriptorSetHandle();

    VkFenceCreateInfo fence_create_info = LvlInitStruct<VkFenceCreateInfo>();
    vk_testing::Fence fence(*m_device, fence_create_info);

    VkEventCreateInfo event_create_info = LvlInitStruct<VkEventCreateInfo>();
    vk_testing::Event event(*m_device, event_create_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkAllocateCommandBuffers-pAllocateInfo::commandBufferCount-arraylength");
    {
        VkCommandBufferAllocateInfo info = LvlInitStruct<VkCommandBufferAllocateInfo>();
        info.commandPool = m_commandPool->handle();
        info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        info.commandBufferCount = 0;  // invalid
        vk::AllocateCommandBuffers(device(), &info, &unused_command_buffer);
    }
    m_errorMonitor->VerifyFound();

    // One exception in spec where the size of a field is used in both the function call it and the struct
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkAllocateDescriptorSets-pAllocateInfo::descriptorSetCount-arraylength");
    // TODO - Figure out why  VUID-VkDescriptorSetAllocateInfo-descriptorSetCount-arraylength is not being generated, very low
    // priority since it is already caught with the above implicit VU. There was an internal MR and WG decided to keep both
    // len='descriptorSetCount' for anyone relying on it
    m_errorMonitor->SetUnexpectedError("VUID_Undefined");
    {
        VkDescriptorSetLayout set_layout = descriptor_set_obj.GetDescriptorSetLayout();
        VkDescriptorSetAllocateInfo info = LvlInitStruct<VkDescriptorSetAllocateInfo>();
        info.descriptorPool = descriptor_set_obj.handle();
        info.descriptorSetCount = 0;  // invalid
        info.pSetLayouts = &set_layout;
        vk::AllocateDescriptorSets(device(), &info, &unused_descriptor_set);
    }
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkFreeCommandBuffers-commandBufferCount-arraylength");
    vk::FreeCommandBuffers(device(), m_commandPool->handle(), 0, &unused_command_buffer);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkFreeDescriptorSets-descriptorSetCount-arraylength");
    vk::FreeDescriptorSets(device(), descriptor_set_obj.handle(), 0, &descriptor_set);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkResetFences-fenceCount-arraylength");
    vk::ResetFences(device(), 0, &fence.handle());
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkWaitForFences-fenceCount-arraylength");
    vk::WaitForFences(device(), 0, &fence.handle(), true, 1);
    m_errorMonitor->VerifyFound();

    VkCommandBufferObj command_buffer(m_device, m_commandPool);
    command_buffer.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorSets-descriptorSetCount-arraylength");
    vk::CmdBindDescriptorSets(command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, descriptor_set_obj.GetPipelineLayout(), 0,
                              0, &(descriptor_set), 0, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-commandBufferCount-arraylength");
    vk::CmdExecuteCommands(command_buffer.handle(), 0, &unused_command_buffer);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-eventCount-arraylength");
    vk::CmdWaitEvents(command_buffer.handle(), 0, &event.handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                      0, nullptr, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();

    command_buffer.end();
}

TEST_F(VkLayerTest, InvalidSpirvExtension) {
    TEST_DESCRIPTION("Use an invalid SPIR-V extension in OpExtension.");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const char *vertex_source = R"spirv(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %4 "main"
               OpSource GLSL 450
               OpExtension "GL_EXT_scalar_block_layout"
               OpName %4 "main"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )spirv";
    VkShaderObj vs(this, vertex_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM_TRY);
    m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-01379");
    if (!vs.InitFromASMTry()) {
        GTEST_SKIP() << "Failed to compile shader";
    }
    const VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08741");
    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ValidateViewportStateScissorOverflow) {
    TEST_DESCRIPTION("Validate sum of offset and width of viewport state scissor");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    VkRect2D scissor_x = {{vvl::kI32Max / 2, 0}, {vvl::kI32Max / 2 + 64, 64}};
    VkRect2D scissor_y = {{0, vvl::kI32Max / 2}, {64, vvl::kI32Max / 2 + 64}};

    const auto break_vp_x = [&](CreatePipelineHelper &helper) {
        helper.vp_state_ci_.viewportCount = 1;
        helper.vp_state_ci_.pViewports = &viewport;
        helper.vp_state_ci_.scissorCount = 1;
        helper.vp_state_ci_.pScissors = &scissor_x;
    };
    CreatePipelineHelper::OneshotTest(*this, break_vp_x, kErrorBit,
                                      vector<std::string>({"VUID-VkPipelineViewportStateCreateInfo-offset-02822"}));

    const auto break_vp_y = [&](CreatePipelineHelper &helper) {
        helper.vp_state_ci_.viewportCount = 1;
        helper.vp_state_ci_.pViewports = &viewport;
        helper.vp_state_ci_.scissorCount = 1;
        helper.vp_state_ci_.pScissors = &scissor_y;
    };
    CreatePipelineHelper::OneshotTest(*this, break_vp_y, kErrorBit,
                                      vector<std::string>({"VUID-VkPipelineViewportStateCreateInfo-offset-02823"}));
}

TEST_F(VkLayerTest, ValidateViewportStateScissorNegative) {
    TEST_DESCRIPTION("Validate offset of viewport state scissor");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    VkRect2D scissor_x = {{-64, 0}, {256, 256}};
    VkRect2D scissor_y = {{0, -64}, {256, 256}};

    const auto break_vp_x = [&](CreatePipelineHelper &helper) {
        helper.vp_state_ci_.viewportCount = 1;
        helper.vp_state_ci_.pViewports = &viewport;
        helper.vp_state_ci_.scissorCount = 1;
        helper.vp_state_ci_.pScissors = &scissor_x;
    };
    CreatePipelineHelper::OneshotTest(*this, break_vp_x, kErrorBit, "VUID-VkPipelineViewportStateCreateInfo-x-02821");

    const auto break_vp_y = [&](CreatePipelineHelper &helper) {
        helper.vp_state_ci_.viewportCount = 1;
        helper.vp_state_ci_.pViewports = &viewport;
        helper.vp_state_ci_.scissorCount = 1;
        helper.vp_state_ci_.pScissors = &scissor_y;
    };
    CreatePipelineHelper::OneshotTest(*this, break_vp_y, kErrorBit, "VUID-VkPipelineViewportStateCreateInfo-x-02821");
}

TEST_F(VkLayerTest, DuplicatePhysicalDevices) {
    TEST_DESCRIPTION("Duplicated physical devices in DeviceGroupDeviceCreateInfo.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    uint32_t physical_device_group_count = 0;
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, nullptr);

    if (physical_device_group_count == 0) {
        GTEST_SKIP() << "physical_device_group_count is 0";
    }

    std::vector<VkPhysicalDeviceGroupProperties> physical_device_group(physical_device_group_count,
                                                                       {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, physical_device_group.data());
    VkPhysicalDevice physicalDevices[2] = {physical_device_group[0].physicalDevices[0],
                                           physical_device_group[0].physicalDevices[0]};

    VkDeviceGroupDeviceCreateInfo create_device_pnext = LvlInitStruct<VkDeviceGroupDeviceCreateInfo>();
    create_device_pnext.physicalDeviceCount = 2;
    create_device_pnext.pPhysicalDevices = physicalDevices;

    ASSERT_NO_FATAL_FAILURE(InitState());

    vk_testing::QueueCreateInfoArray queue_info(m_device->queue_props);

    VkDeviceCreateInfo create_info = LvlInitStruct<VkDeviceCreateInfo>();
    create_info.pNext = &create_device_pnext;
    create_info.queueCreateInfoCount = queue_info.size();
    create_info.pQueueCreateInfos = queue_info.data();
    create_info.enabledLayerCount = 0;
    create_info.ppEnabledLayerNames = nullptr;
    create_info.enabledExtensionCount = m_device_extension_names.size();
    create_info.ppEnabledExtensionNames = m_device_extension_names.data();

    VkDevice device;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupDeviceCreateInfo-pPhysicalDevices-00375");
    vk::CreateDevice(gpu(), &create_info, nullptr, &device);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidCombinationOfDeviceFeatures) {
    TEST_DESCRIPTION("Test invalid combinations of device features.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT shader_image_atomic_int64_feature =
        LvlInitStruct<VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT>();
    shader_image_atomic_int64_feature.sparseImageInt64Atomics = VK_TRUE;
    shader_image_atomic_int64_feature.shaderImageInt64Atomics = VK_FALSE;

    VkPhysicalDeviceShaderAtomicFloatFeaturesEXT shader_atomic_float_feature =
        LvlInitStruct<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT>();
    shader_atomic_float_feature.sparseImageFloat32Atomics = VK_TRUE;
    shader_atomic_float_feature.shaderImageFloat32Atomics = VK_FALSE;
    shader_atomic_float_feature.sparseImageFloat32AtomicAdd = VK_TRUE;
    shader_atomic_float_feature.shaderImageFloat32AtomicAdd = VK_FALSE;

    VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT shader_atomic_float_feature2 =
        LvlInitStruct<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT>();
    shader_atomic_float_feature2.sparseImageFloat32AtomicMinMax = VK_TRUE;
    shader_atomic_float_feature2.shaderImageFloat32AtomicMinMax = VK_FALSE;

    VkPhysicalDeviceFeatures2 pd_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&shader_image_atomic_int64_feature);

    ASSERT_NO_FATAL_FAILURE(Init());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    vk_testing::QueueCreateInfoArray queue_info(m_device->queue_props);
    VkDeviceCreateInfo device_create_info = LvlInitStruct<VkDeviceCreateInfo>();
    device_create_info.pNext = &pd_features2;
    device_create_info.queueCreateInfoCount = queue_info.size();
    device_create_info.pQueueCreateInfos = queue_info.data();

    {
        VkDevice testDevice;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-None-04896");
        vk::CreateDevice(gpu(), &device_create_info, NULL, &testDevice);
        m_errorMonitor->VerifyFound();
    }
    {
        pd_features2.pNext = &shader_atomic_float_feature;

        VkDevice testDevice;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-None-04897");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-None-04898");
        vk::CreateDevice(gpu(), &device_create_info, NULL, &testDevice);
        m_errorMonitor->VerifyFound();
    }
    {
        pd_features2.pNext = &shader_atomic_float_feature2;

        VkDevice testDevice;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-sparseImageFloat32AtomicMinMax-04975");
        vk::CreateDevice(gpu(), &device_create_info, NULL, &testDevice);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, InvalidImageCreateFlagWithPhysicalDeviceCount) {
    TEST_DESCRIPTION("Test for invalid imageCreate flags bit with physicalDeviceCount.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    uint32_t physical_device_group_count = 0;
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, nullptr);

    if (physical_device_group_count == 0) {
        GTEST_SKIP() << "physical_device_group_count is 0";
    }

    std::vector<VkPhysicalDeviceGroupProperties> physical_device_group(physical_device_group_count,
                                                                       {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, physical_device_group.data());
    VkDeviceGroupDeviceCreateInfo create_device_pnext = LvlInitStruct<VkDeviceGroupDeviceCreateInfo>();
    create_device_pnext.physicalDeviceCount = 1;
    create_device_pnext.pPhysicalDevices = physical_device_group[0].physicalDevices;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &create_device_pnext, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    VkImageCreateInfo ici = LvlInitStruct<VkImageCreateInfo>();
    ici.flags = VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT;
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_R8G8B8A8_UNORM;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    VkImageFormatProperties imageFormatProperties;
    VkResult result =
        vk::GetPhysicalDeviceImageFormatProperties(physical_device_group[0].physicalDevices[0], ici.format, ici.imageType,
                                                   ici.tiling, ici.usage, ici.flags, &imageFormatProperties);
    if (result == VK_ERROR_FORMAT_NOT_SUPPORTED) {
        GTEST_SKIP() << "image format is not supported";
    }

    CreateImageTest(*this, &ici, "VUID-VkImageCreateInfo-physicalDeviceCount-01421");
}

TEST_F(VkLayerTest, Features12AndppEnabledExtensionNames) {
    TEST_DESCRIPTION("Test VkPhysicalDeviceVulkan12Features and illegal extension in ppEnabledExtensionNames");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    VkPhysicalDeviceVulkan12Features features12 = LvlInitStruct<VkPhysicalDeviceVulkan12Features>();
    features12.bufferDeviceAddress = VK_TRUE;

    float priority = 1.0f;
    VkDeviceQueueCreateInfo queue_info = LvlInitStruct<VkDeviceQueueCreateInfo>();
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &priority;

    const char *enabled_ext = VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME;

    VkDeviceCreateInfo device_create_info = LvlInitStruct<VkDeviceCreateInfo>(&features12);
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pQueueCreateInfos = &queue_info;
    device_create_info.enabledExtensionCount = 1;
    device_create_info.ppEnabledExtensionNames = &enabled_ext;

    VkDevice testDevice;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-pNext-04748");
    vk::CreateDevice(gpu(), &device_create_info, NULL, &testDevice);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ImageSubresourceOverlapBetweenCurrentRenderPassAndDescriptorSets) {
    TEST_DESCRIPTION("Validate if attachments in render pass and descriptor set use the same image subresources");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidRenderpass");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassBeginInfo-renderPass-00904");

    const uint32_t width = 16;
    const uint32_t height = 16;
    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    VkAttachmentReference attach_ref = {};
    attach_ref.attachment = 0;
    attach_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attach_ref;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = format;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    VkAttachmentDescription attach_desc2[] = {attach_desc, attach_desc};

    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 2;
    rpci.pAttachments = attach_desc2;

    vk_testing::RenderPass render_pass(*m_device, rpci);

    VkClearValue clear_values[2] = {m_renderPassClearValues[0], m_renderPassClearValues[0]};

    VkRenderPassBeginInfo rpbi = LvlInitStruct<VkRenderPassBeginInfo>();
    rpbi.framebuffer = m_framebuffer;
    rpbi.renderPass = render_pass.handle();
    rpbi.renderArea.extent.width = width;
    rpbi.renderArea.extent.height = height;
    rpbi.clearValueCount = 2;
    rpbi.pClearValues = clear_values;

    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ZeroBitmask) {
    TEST_DESCRIPTION("Test a reserved flags field set to a non-zero value");

    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreCreateInfo-flags-zerobitmask");
    auto semaphore_ci = LvlInitStruct<VkSemaphoreCreateInfo>();
    semaphore_ci.flags = 1;
    VkSemaphore semaphore = VK_NULL_HANDLE;
    vk::CreateSemaphore(m_device->device(), &semaphore_ci, nullptr, &semaphore);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InstanceCreateEnumeratePortability) {
    TEST_DESCRIPTION("Validate creating instances with VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR.");

    auto ici = GetInstanceCreateInfo();
    ici.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    ici.enabledExtensionCount = 0;

    VkInstance local_instance;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkInstanceCreateInfo-flags-06559");
    vk::CreateInstance(&ici, nullptr, &local_instance);
    m_errorMonitor->VerifyFound();

    if (InstanceExtensionSupported(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
        std::vector<const char *> enabled_extensions = {VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME};
        ici.enabledExtensionCount = 1;
        ici.ppEnabledExtensionNames = enabled_extensions.data();

        ASSERT_VK_SUCCESS(vk::CreateInstance(&ici, nullptr, &local_instance));
        vk::DestroyInstance(local_instance, nullptr);
    }
}

#ifdef VK_USE_PLATFORM_METAL_EXT
TEST_F(VkLayerTest, ExportMetalObjects) {
    TEST_DESCRIPTION("Test VK_EXT_metal_objects VUIDs");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_METAL_OBJECTS_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    const bool ycbcr_conversion_extension = IsExtensionsEnabled(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    auto portability_features = LvlInitStruct<VkPhysicalDevicePortabilitySubsetFeaturesKHR>();
    auto features2 = GetPhysicalDeviceFeatures2(portability_features);

    if (ycbcr_conversion_extension) {
        auto ycbcr_features = LvlInitStruct<VkPhysicalDeviceSamplerYcbcrConversionFeatures>();
        ycbcr_features.samplerYcbcrConversion = VK_TRUE;
        portability_features.pNext = &ycbcr_features;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto metal_object_create_info = LvlInitStruct<VkExportMetalObjectCreateInfoEXT>();
    auto instance_ci = GetInstanceCreateInfo();
    metal_object_create_info.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_SHARED_EVENT_BIT_EXT;
    metal_object_create_info.pNext = instance_ci.pNext;
    instance_ci.pNext = &metal_object_create_info;

    VkInstance instance = {};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkInstanceCreateInfo-pNext-06779");
    vk::CreateInstance(&instance_ci, nullptr, &instance);
    m_errorMonitor->VerifyFound();
    metal_object_create_info.pNext = nullptr;

    auto alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
    alloc_info.pNext = &metal_object_create_info;
    alloc_info.allocationSize = 1024;
    VkDeviceMemory memory;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-pNext-06780");
    vk::AllocateMemory(device(), &alloc_info, nullptr, &memory);
    m_errorMonitor->VerifyFound();

    VkImageCreateInfo ici = LvlInitStruct<VkImageCreateInfo>();
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.format = VK_FORMAT_B8G8R8A8_UNORM;
    ici.extent = {128, 128, 1};
    ici.mipLevels = 1;
    ici.arrayLayers = 1;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_LINEAR;
    ici.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.pNext = &metal_object_create_info;
    CreateImageTest(*this, &ici, "VUID-VkImageCreateInfo-pNext-06783");

    auto import_metal_texture_info = LvlInitStruct<VkImportMetalTextureInfoEXT>();
    import_metal_texture_info.plane = VK_IMAGE_ASPECT_COLOR_BIT;
    ici.pNext = &import_metal_texture_info;
    ici.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    CreateImageTest(*this, &ici, "VUID-VkImageCreateInfo-pNext-06784");

    ici.format = VK_FORMAT_B8G8R8A8_UNORM;
    import_metal_texture_info.plane = VK_IMAGE_ASPECT_PLANE_1_BIT;
    CreateImageTest(*this, &ici, "VUID-VkImageCreateInfo-pNext-06785");

    ici.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    import_metal_texture_info.plane = VK_IMAGE_ASPECT_PLANE_2_BIT;
    CreateImageTest(*this, &ici, "VUID-VkImageCreateInfo-pNext-06786");

    uint32_t queue_family_index = 0;
    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.size = 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    buffer_create_info.queueFamilyIndexCount = 1;
    buffer_create_info.pQueueFamilyIndices = &queue_family_index;

    VkBufferObj buffer;
    buffer.init(*m_device, buffer_create_info);
    VkBufferViewCreateInfo buff_view_ci = LvlInitStruct<VkBufferViewCreateInfo>();
    buff_view_ci.buffer = buffer.handle();
    buff_view_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    buff_view_ci.range = VK_WHOLE_SIZE;
    buff_view_ci.pNext = &metal_object_create_info;
    CreateBufferViewTest(*this, &buff_view_ci, {"VUID-VkBufferViewCreateInfo-pNext-06782"});

    VkImageObj image_obj(m_device);
    image_obj.Init(256, 256, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>();
    ivci.image = image_obj.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_B8G8R8A8_UNORM;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ivci.pNext = &metal_object_create_info;
    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-pNext-06787");

    auto sem_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    sem_info.pNext = &metal_object_create_info;
    VkSemaphore semaphore;
    metal_object_create_info.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_BUFFER_BIT_EXT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreCreateInfo-pNext-06789");
    vk::CreateSemaphore(device(), &sem_info, NULL, &semaphore);
    m_errorMonitor->VerifyFound();

    auto event_info = LvlInitStruct<VkEventCreateInfo>();
    if (portability_features.events) {
        event_info.pNext = &metal_object_create_info;
        VkEvent event;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkEventCreateInfo-pNext-06790");
        vk::CreateEvent(device(), &event_info, nullptr, &event);
        m_errorMonitor->VerifyFound();
    }

    auto export_metal_objects_info = LvlInitStruct<VkExportMetalObjectsInfoEXT>();
    auto metal_device_info = LvlInitStruct<VkExportMetalDeviceInfoEXT>();
    auto metal_command_queue_info = LvlInitStruct<VkExportMetalCommandQueueInfoEXT>();
    metal_command_queue_info.queue = m_device->m_queue;
    export_metal_objects_info.pNext = &metal_device_info;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMetalObjectsInfoEXT-pNext-06791");
    vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    export_metal_objects_info.pNext = &metal_command_queue_info;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMetalObjectsInfoEXT-pNext-06792");
    vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    alloc_info.pNext = nullptr;
    VkResult err = vk::AllocateMemory(device(), &alloc_info, nullptr, &memory);
    ASSERT_VK_SUCCESS(err);
    auto metal_buffer_info = LvlInitStruct<VkExportMetalBufferInfoEXT>();
    metal_buffer_info.memory = memory;
    export_metal_objects_info.pNext = &metal_buffer_info;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMetalObjectsInfoEXT-pNext-06793");
    vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();
    vk::FreeMemory(device(), memory, nullptr);

    auto export_metal_object_create_info = LvlInitStruct<VkExportMetalObjectCreateInfoEXT>();
    export_metal_object_create_info.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT;
    ici.pNext = &export_metal_object_create_info;
    VkImageObj export_image_obj(m_device);
    export_image_obj.Init(ici);
    vk_testing::BufferView export_buffer_view;
    buff_view_ci.pNext = &export_metal_object_create_info;
    export_buffer_view.init(*m_device, buff_view_ci);
    auto metal_texture_info = LvlInitStruct<VkExportMetalTextureInfoEXT>();
    metal_texture_info.bufferView = export_buffer_view.handle();
    metal_texture_info.image = export_image_obj.handle();
    metal_texture_info.plane = VK_IMAGE_ASPECT_PLANE_0_BIT;
    export_metal_objects_info.pNext = &metal_texture_info;

    // Only one of image, bufferView, imageView
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMetalObjectsInfoEXT-pNext-06794");
    vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    // Image not created with struct in pNext
    metal_texture_info.bufferView = VK_NULL_HANDLE;
    metal_texture_info.image = image_obj.handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMetalObjectsInfoEXT-pNext-06795");
    vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    metal_texture_info.image = VK_NULL_HANDLE;
    vk_testing::ImageView image_view_no_struct;
    auto image_view_ci = image_obj.BasicViewCreatInfo();
    image_view_no_struct.init(*m_device, image_view_ci);
    metal_texture_info.imageView = image_view_no_struct.handle();
    // ImageView not created with struct in pNext
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMetalObjectsInfoEXT-pNext-06796");
    vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    buff_view_ci.pNext = nullptr;
    vk_testing::BufferView buffer_view_no_struct;
    buffer_view_no_struct.init(*m_device, buff_view_ci);
    metal_texture_info.imageView = VK_NULL_HANDLE;
    metal_texture_info.bufferView = buffer_view_no_struct.handle();
    // BufferView not created with struct in pNext
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMetalObjectsInfoEXT-pNext-06797");
    vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    metal_texture_info.bufferView = VK_NULL_HANDLE;
    metal_texture_info.image = export_image_obj.handle();
    metal_texture_info.plane = VK_IMAGE_ASPECT_COLOR_BIT;
    // metal_texture_info.plane not plane 0, 1 or 2
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMetalObjectsInfoEXT-pNext-06798");
    vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    ici.format = VK_FORMAT_B8G8R8A8_UNORM;
    VkImageObj single_plane_export_image_obj(m_device);
    single_plane_export_image_obj.Init(ici);
    metal_texture_info.plane = VK_IMAGE_ASPECT_PLANE_1_BIT;
    metal_texture_info.image = single_plane_export_image_obj.handle();
    // metal_texture_info.plane not plane_0 for single plane image
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMetalObjectsInfoEXT-pNext-06799");
    vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    image_view_ci.pNext = &export_metal_object_create_info;
    export_metal_object_create_info.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT;
    vk_testing::ImageView single_plane_export_image_view;
    single_plane_export_image_view.init(*m_device, image_view_ci);
    metal_texture_info.image = VK_NULL_HANDLE;
    metal_texture_info.imageView = single_plane_export_image_view.handle();
    // metal_texture_info.plane not plane_0 for single plane imageView
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMetalObjectsInfoEXT-pNext-06801");
    vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    auto metal_iosurface_info = LvlInitStruct<VkExportMetalIOSurfaceInfoEXT>();
    metal_iosurface_info.image = image_obj.handle();
    export_metal_objects_info.pNext = &metal_iosurface_info;
    // metal_iosurface_info.image not created with struct in pNext
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMetalObjectsInfoEXT-pNext-06803");
    vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    auto metal_shared_event_info = LvlInitStruct<VkExportMetalSharedEventInfoEXT>();
    export_metal_objects_info.pNext = &metal_shared_event_info;
    // metal_shared_event_info event and semaphore both VK_NULL_HANDLE
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMetalObjectsInfoEXT-pNext-06804");
    vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    sem_info.pNext = nullptr;
    vk_testing::Semaphore semaphore_no_struct;
    semaphore_no_struct.init(*m_device, sem_info);
    metal_shared_event_info.semaphore = semaphore_no_struct.handle();
    export_metal_objects_info.pNext = &metal_shared_event_info;
    // Semaphore not created with struct in pNext
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMetalObjectsInfoEXT-pNext-06805");
    vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    if (portability_features.events) {
        event_info.pNext = nullptr;
        vk_testing::Event event_no_struct;
        event_no_struct.init(*m_device, event_info);
        metal_shared_event_info.event = event_no_struct.handle();
        metal_shared_event_info.semaphore = VK_NULL_HANDLE;
        // Event not created with struct in pNext
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMetalObjectsInfoEXT-pNext-06806");
        vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
        m_errorMonitor->VerifyFound();
    }

    const VkFormat mp_format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    if (ImageFormatIsSupported(gpu(), mp_format)) {
        export_metal_object_create_info = LvlInitStruct<VkExportMetalObjectCreateInfoEXT>();
        export_metal_object_create_info.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT;
        ici.format = mp_format;
        ici.tiling = VK_IMAGE_TILING_OPTIMAL;
        ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
        ici.pNext = &export_metal_object_create_info;
        VkImageObj mp_image_obj(m_device);
        mp_image_obj.init(&ici);

        metal_texture_info.bufferView = VK_NULL_HANDLE;
        metal_texture_info.imageView = VK_NULL_HANDLE;
        metal_texture_info.image = mp_image_obj.handle();
        metal_texture_info.plane = VK_IMAGE_ASPECT_PLANE_2_BIT;
        export_metal_objects_info.pNext = &metal_texture_info;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMetalObjectsInfoEXT-pNext-06800");
        vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
        m_errorMonitor->VerifyFound();

        if (ycbcr_conversion_extension) {
            VkSamplerYcbcrConversionCreateInfo ycbcr_create_info = LvlInitStruct<VkSamplerYcbcrConversionCreateInfo>();
            ycbcr_create_info.format = mp_format;
            ycbcr_create_info.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
            ycbcr_create_info.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
            ycbcr_create_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                            VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
            ycbcr_create_info.xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
            ycbcr_create_info.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
            ycbcr_create_info.chromaFilter = VK_FILTER_NEAREST;
            ycbcr_create_info.forceExplicitReconstruction = false;

            VkSamplerYcbcrConversion conversion;
            err = vk::CreateSamplerYcbcrConversionKHR(m_device->device(), &ycbcr_create_info, nullptr, &conversion);
            ASSERT_VK_SUCCESS(err);

            VkSamplerYcbcrConversionInfo ycbcr_info = LvlInitStruct<VkSamplerYcbcrConversionInfo>();
            ycbcr_info.conversion = conversion;
            ycbcr_info.pNext = &export_metal_object_create_info;
            ivci.image = mp_image_obj.handle();
            ivci.format = mp_format;
            ivci.pNext = &ycbcr_info;
            vk_testing::ImageView mp_image_view;
            mp_image_view.init(*m_device, ivci);
            metal_texture_info.image = VK_NULL_HANDLE;
            metal_texture_info.imageView = mp_image_view.handle();
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMetalObjectsInfoEXT-pNext-06802");
            vk::ExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
            m_errorMonitor->VerifyFound();
            vk::DestroySamplerYcbcrConversionKHR(m_device->device(), conversion, nullptr);
        }
    }
}
#endif  // VK_USE_PLATFORM_METAL_EXT

TEST_F(VkLayerTest, InvalidExtEnum) {
    TEST_DESCRIPTION("Use an enum from an extension that is not enabled.");
    ASSERT_NO_FATAL_FAILURE(Init());

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.magFilter = VK_FILTER_CUBIC_EXT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerCreateInfo-magFilter-parameter");
    vk_testing::Sampler sampler(*m_device, sampler_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ExtensionNotEnabledYCbCr) {
    TEST_DESCRIPTION("Validate that using an API from an unenabled extension returns an error");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // Require YCbCr dependencies extensions except VK_KHR_GET_MEMORY_REQUIREMENTS_2 -- to create the needed error
    std::vector<const char *> required_device_extensions = {
        VK_KHR_MAINTENANCE_1_EXTENSION_NAME, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME};
    for (auto dev_ext : required_device_extensions) {
        if (DeviceExtensionSupported(dev_ext)) {
            m_device_extension_names.push_back(dev_ext);
        } else {
            // Need to get out of the test now so that the subsequent code doesn't try to use an extension that isn't enabled.
            GTEST_SKIP() << "Did not find required device extension: " << dev_ext;
        }
    }

    // Need to ignore this error to get to the one we're testing
    m_errorMonitor->SetUnexpectedError("VUID-vkCreateDevice-ppEnabledExtensionNames-01387");
    ASSERT_NO_FATAL_FAILURE(InitState());

    // The feature bit samplerYcbcrConversion prevents the function from being called even in Vulkan 1.0
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateSamplerYcbcrConversion-None-01648");
    VkSamplerYcbcrConversionCreateInfo ycbcr_create_info = LvlInitStruct<VkSamplerYcbcrConversionCreateInfo>();
    ycbcr_create_info.format = VK_FORMAT_UNDEFINED;
    ycbcr_create_info.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    ycbcr_create_info.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    ycbcr_create_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                    VK_COMPONENT_SWIZZLE_IDENTITY};
    ycbcr_create_info.xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    ycbcr_create_info.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    ycbcr_create_info.chromaFilter = VK_FILTER_NEAREST;
    ycbcr_create_info.forceExplicitReconstruction = false;
    VkSamplerYcbcrConversion conversion;
    vk::CreateSamplerYcbcrConversionKHR(m_device->handle(), &ycbcr_create_info, nullptr, &conversion);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DuplicateValidPNextStructures) {
    TEST_DESCRIPTION("Create a pNext chain containing valid structures, but with a duplicate structure type");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    // VK_KHR_get_physical_device_properties2 promoted to 1.1
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPhysicalDeviceProperties2-sType-unique");
    // in VkPhysicalDeviceProperties2 create a chain of pNext of type A -> B -> A
    // Also using different instance of struct to not trip the cycle checkings
    VkPhysicalDeviceProtectedMemoryProperties protected_memory_properties_0 =
        LvlInitStruct<VkPhysicalDeviceProtectedMemoryProperties>();

    VkPhysicalDeviceIDProperties id_properties = LvlInitStruct<VkPhysicalDeviceIDProperties>(&protected_memory_properties_0);

    VkPhysicalDeviceProtectedMemoryProperties protected_memory_properties_1 =
        LvlInitStruct<VkPhysicalDeviceProtectedMemoryProperties>(&id_properties);

    VkPhysicalDeviceProperties2 physical_device_properties2 =
        LvlInitStruct<VkPhysicalDeviceProperties2>(&protected_memory_properties_1);

    vk::GetPhysicalDeviceProperties2(gpu(), &physical_device_properties2);
    m_errorMonitor->VerifyFound();
}
